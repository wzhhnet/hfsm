/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : state.c
*   CREATE DATE : 2021-12-16
*   MODULE      :
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#include "log.h"
#include "list.h"
#include "hfsm.h"
#include "htree.h"
#include "bitops.h"
#include "thread.h"
#include "state.h"


/****************************************************************************
*                       Macro Definition Section                            *
*****************************************************************************/


/****************************************************************************
*                       Type Definition Section                             *
*****************************************************************************/
struct state_info_t {
    struct treenode node;
    int bit;
    hfsm_state info;
};

struct state_pool_t {
    struct state_info_t buff[HFSM_MAX_STATES];
    uint32_t bits[BITS_TO_WORDS(HFSM_MAX_STATES)];
};

struct state_handle_t {
    struct treeroot tree;
    struct treenode *cur;       /*!< current state */
    struct thread_ctrl_t ctrl;
    struct state_pool_t pool;
};

static bool traverse_callback(struct treenode *node, void *arg) {

    uint32_t key;
    struct state_info_t *info;

    if (!node || !arg)
        return false;

    key = *(uint32_t*)arg;
    info = container_of(node, struct state_info_t, node);
    if (!info)
        return false;

    if (info->info.stateID == key)
        return true;

    return false;
}

static bool downward_callback(struct treenode *node, void *arg) {

    struct traverse_t *p;
    struct state_info_t *s;

    if (!node || !arg)
        return false;

    p = (struct traverse_t*)arg;
    s = container_of(node, struct state_info_t, node);

    return p->func(&s->info, p->arg);
}

struct state_info_t* state_alloc(struct state_pool_t *pool) {

    if (!pool)
        return NULL;

    int bit = bitmask_ffz(pool->bits, HFSM_MAX_STATES);
    if (bit >= 0 && bit < HFSM_MAX_STATES) {
        pool->buff[bit].bit = bit;
        bitmask_set(pool->bits, bit);
        return &pool->buff[bit];
    }
    return NULL;   
}

void state_release(struct state_pool_t *pool, struct state_info_t *pstate) {

    if (pool == NULL || pstate == NULL)
        return;

    if (pstate->bit >= 0 && pstate->bit < HFSM_MAX_STATES) {
        bitmask_clear(pool->bits, pstate->bit);
    }
}

int state_create(state_handle *handle) {

    bool ret;
    char *mem;
    uint32_t total;
    struct state_handle_t* phandle;

    if (!handle)
        return HFSM_ERR_NULLPTR;

    phandle = (struct state_handle_t*)malloc(sizeof(struct state_handle_t));
    if (!phandle)
        return HFSM_ERR_MALLOC;

    /*! initialize  */
    memset(phandle, 0, sizeof(struct state_handle_t));
    thread_ctrl_init(&phandle->ctrl);
    *handle = (state_handle)phandle;

    return HFSM_OK;
}

int state_destroy(state_handle handle) {

    struct state_handle_t* phandle;
    if (!handle)
        return HFSM_ERR_NULLPTR;

    phandle = (struct state_handle_t*)handle;
    thread_ctrl_uninit(&phandle->ctrl);
    free(phandle);

    return HFSM_OK;
}

int state_add(state_handle handle, hfsm_state *parent, hfsm_state *pstate) {

    int ret;
    struct treenode *node;
    struct state_info_t* state;
    struct state_handle_t* phandle;

    if (!handle || !pstate)
        return HFSM_ERR_NULLPTR;

    phandle = (struct state_handle_t*)handle;
    state = state_alloc(&phandle->pool);
    if (!state) {
        ret = HFSM_ERR_STATE_EXCEED_MAX_CNT;
        goto err;
    }

    tree_init(&state->node);
    memcpy(&state->info, pstate, sizeof(hfsm_state));

    if (!parent) {
        if (!is_empty_tree(&phandle->tree)) {
            ret = HFSM_ERR_ROOT_STATE_EXIST;
            goto err;
        }
        phandle->tree.root = &state->node;
    } else {
        /*! try to find target state */
        node = tree_find_node(
            &phandle->tree, (callback_fn)traverse_callback, (void*)&pstate->stateID);
        if (node) {
            ret = HFSM_ERR_STATE_EXIST;
            goto err;
        }

        /*! try to find parent state */
        node = tree_find_node(
            &phandle->tree, (callback_fn)traverse_callback, (void*)&parent->stateID);
        if (!node) {
            ret = HFSM_ERR_STATE_NOT_EXIST;
            goto err;
        }

        tree_add_node(node, &state->node);
    }

    return HFSM_OK;

err:
    state_release(&phandle->pool, state);
    LOGE("%s faild ret = %d", __FUNCTION__, ret);
    return ret;
}

int state_set(state_handle handle, uint32_t key) {

    int ret;
    struct treenode *node;
    struct state_handle_t* phandle;

    if (!handle) {
        ret = HFSM_ERR_NULLPTR;
        goto err;
    }

    phandle = (struct state_handle_t*)handle;
    /*! try to find target state */
    node = tree_find_node(
        &phandle->tree, (callback_fn)traverse_callback, (void*)&key);
    if (!node) {
        ret = HFSM_ERR_STATE_NOT_EXIST;
        goto err;
    }

    phandle->cur = node;
    return HFSM_OK;

 err:
     LOGE("%s faild ret = %d", __FUNCTION__, ret);
     return ret;
}

hfsm_state* state_find(state_handle handle, uint32_t key) {

    struct treenode* n;
    struct state_info_t* s;
    struct state_handle_t* phandle;
    if (!handle)
        goto err;

    phandle = (struct state_handle_t*)handle;
    n = tree_find_node(
        &phandle->tree, (callback_fn)traverse_callback, (void*)&key);
    if (!n)
        goto err;

    s = list_entry(n, struct state_info_t, node);
    return &s->info;

err:
    LOGE("%s failed", __FUNCTION__);
    return NULL;
}

hfsm_state* state_get(state_handle handle) {

    struct state_info_t* s;
    struct state_handle_t* phandle;

    if (!handle)
        return NULL;

    phandle = (struct state_handle_t*)handle;

    if (!phandle->cur)
        return NULL;

    s = container_of(phandle->cur, struct state_info_t, node);
    return &s->info;
}

hfsm_state* state_parent(state_handle handle, hfsm_state *pstate) {

    struct treenode *n;
    struct state_info_t* s;
    struct state_handle_t* phandle;

    if (!handle || !pstate)
        goto err;

    phandle = (struct state_handle_t*)handle;
    n = tree_find_node(
        &phandle->tree, (callback_fn)traverse_callback, (void*)&pstate->stateID);
    if (!n)
        goto err;

    n = n->parent;
    if (!n)
        goto err;

    s = container_of(n, struct state_info_t, node);
    return &s->info;

err:
    LOGE("%s failed", __FUNCTION__);
    return NULL;
}

hfsm_state* state_forefathers(state_handle handle, hfsm_state *p1, hfsm_state *p2) {

    struct treenode *n1, *n2, *nf;
    struct state_info_t* s;
    struct state_handle_t* phandle;

    if (!handle || !p1 || !p2)
        goto err;

    phandle = (struct state_handle_t*)handle;
    if (is_empty_tree(&phandle->tree))
        goto err;

    n1 = tree_find_node(
        &phandle->tree, (callback_fn)traverse_callback, (void*)&p1->stateID);
    if (!n1)
        goto err;

    n2 = tree_find_node(
        &phandle->tree, (callback_fn)traverse_callback, (void*)&p2->stateID);
    if (!n2)
        goto err;

    nf = tree_forefathers(&phandle->tree, n1, n2);
    if (!nf)
        goto err;

    s = container_of(nf, struct state_info_t, node);
    return &s->info;

err:
    LOGE("%s failed", __FUNCTION__);
    return NULL;
}

hfsm_state* state_root(state_handle handle) {

    struct treenode *n;
    struct state_info_t* s;
    struct state_handle_t* phandle;

    if (!handle)
        return NULL;

    phandle = (struct state_handle_t*)handle;
    if (is_empty_tree(&phandle->tree))
        return NULL;

    s = container_of(&phandle->tree.root, struct state_info_t, node);
    return &s->info;
}

int state_downward(
    state_handle handle, hfsm_state *src, hfsm_state *dst, traverse *tra) {

    struct treenode *n;
    struct state_info_t *s, *d;
    struct state_handle_t* phandle;

    if (!handle || !src || !dst || !tra)
        return HFSM_ERR_NULLPTR;

    phandle = (struct state_handle_t*)handle;
    if (is_empty_tree(&phandle->tree))
        return HFSM_ERR_ROOT_STATE_EXIST;

    s = tree_find_node(
        &phandle->tree, (callback_fn)traverse_callback, (void*)&src->stateID);
    if (!s)
        return HFSM_ERR_STATE_NOT_EXIST;

    d = tree_find_node(
        &phandle->tree, (callback_fn)traverse_callback, (void*)&dst->stateID);
    if (!d)
        return HFSM_ERR_STATE_NOT_EXIST;

    tree_traverse_downward(s, d, downward_callback, (void*)tra);
    return HFSM_OK;
}

