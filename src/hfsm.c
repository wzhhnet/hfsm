/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : hfsm.h
*   CREATE DATE : 2021-12-16
*   MODULE      :
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#include "log.h"
#include "hfsm.h"
#include "state.h"
#include "message.h"
#include <pthread.h>

#define MAX_MESSAGE_NUM     (64)
#define EXIT_THREAD  (1)

enum hfsm_msg_e {
    HFSM_SYS_START,
    HFSM_SYS_STOP,
    HFSM_USER
};

struct hfsm_sys_t {
    uint32_t what;
    int32_t arg1;
    int32_t arg2;
};

struct hfsm_thread_t {
    int exit;
    pthread_t id;
};

struct hfsm_t {
    que_handle msg_handle;
    state_handle sta_handle;
    uint32_t cur_state;     /*!< 0 as default */
    struct hfsm_thread_t thread;
};

static bool hfsm_transit_cb(hfsm_state* pstate, void* arg) {

    int ret;
    if (!pstate)
        return false;

    ret = pstate->enter(pstate->argv);
    if (ret != HFSM_OK)
        return false;

    return true;
}

static void hfsm_invoke_transit(
    struct hfsm_t *phfsm, hfsm_state *src, hfsm_state *dst) {

    int ret;
    hfsm_state *pstate, *s;
    traverse tra_param = {};

    if (!phfsm || !dst)
        return;

    if (!src) {
        pstate = state_root(phfsm->sta_handle);
    } else {
        pstate = state_forefathers(
            phfsm->sta_handle, src, dst);
    }

    if (!pstate)
        goto err;

    /*! exit original state to forefathers */
    s = src;
    while (s && s->stateID != pstate->stateID) {
        ret = s->exit(s->argv);
        if (ret != HFSM_OK)
            goto err;
        s = state_parent(phfsm->sta_handle, s);
        if (ret != HFSM_OK)
            goto err;
    }

    /*! enter destination state from forefathers */
    tra_param.func = hfsm_transit_cb;
    tra_param.arg = (void*)phfsm;
    ret = state_downward(
        phfsm->sta_handle, pstate, dst, &tra_param);
    if (ret == HFSM_OK) {
        ret = state_set(phfsm->sta_handle, dst->stateID);
        if (ret == HFSM_OK)
            return;
    }

err:
    LOGE("%s failed errcode = %d", __FUNCTION__, ret);
}

static void hfsm_state_transit(
    struct hfsm_t *phfsm, uint32_t stateid) {

    int ret;
    hfsm_state *tar, *cur;

    if (!phfsm)
        return;

    /*! find target state */
    tar = state_find(phfsm->sta_handle, stateid);
    if (!tar)
        return;

    /*! find current state */
    cur = state_get(phfsm->sta_handle);
    if (cur) {
        if (tar->stateID != cur->stateID) {
            hfsm_invoke_transit(phfsm, cur, tar);
        } else {
            /*! no need transition */
            LOGD("%s no transition");
        }
    } else {
        hfsm_invoke_transit(phfsm, NULL, tar);
    }
}

static void hfms_msg_process(
    struct hfsm_t *phfsm, hfsm_message *msg) {

    int ret;
    int msg_rc = HFSM_PROC_MSG_UNHANDLER;
    hfsm_state *pstate;

    /*! get current state */
    pstate = state_get(phfsm->sta_handle);

    /*! process message on proper state */
    while (pstate) {
        uint32_t state_id = pstate->stateID;

        /*! process message on current state */
        msg_rc = pstate->process(msg, pstate->argv, &state_id);
        if (msg_rc == HFSM_PROC_MSG_UNHANDLER) {
            /*! process message on parent state */
            pstate = state_parent(phfsm->sta_handle, pstate);
        } else {
            if (pstate->stateID != state_id) {
                phfsm->cur_state = state_id;
                /*! try to transition before next message process */
                hfsm_state_transit(phfsm, phfsm->cur_state);
            }
            break;
        }
    }
}

static void *hfsm_loop(void *arg) {
    if (arg == NULL) goto exit;

    bool actived = false;
    struct hfsm_t* phfsm = (struct hfsm_t*)arg;
    struct hfsm_thread_t* thread = &phfsm->thread;

    while (!thread->exit) {
        message m = {};
        LOGD("waiting for message ...");

        /*! thread hangs if queue is empty */
        queue_wait(phfsm->msg_handle);
        LOGD("message is comming");

        /*! do message if anyone pushed it to queue */
        queue_pop(phfsm->msg_handle, &m);
        if (m.type == HFSM_SYS_START) {
            hfsm_state_transit(phfsm, phfsm->cur_state);
            actived = true;
        } else if (m.type == HFSM_SYS_STOP) {
            actived = false;
        } else if (m.type == HFSM_USER) {
            if (actived) {
                hfsm_message *cur_msg = (hfsm_message*)m.payload;
                /*! process message on proper state */
                hfms_msg_process(phfsm, cur_msg);
            }
        }
    }

    return NULL; /*!< exit normaly */

exit:

    return NULL;
}

int hfsm_create(hfsm_handle *hfsm) {

    int ret;
    struct hfsm_t* phfsm = (struct hfsm_t*)malloc(sizeof(struct hfsm_t));
    if (!phfsm)
        return HFSM_ERR_MALLOC;

    memset(phfsm, 0, sizeof(struct hfsm_t));

    /*! create message handler */
    queue_create(MAX_MESSAGE_NUM, &phfsm->msg_handle);
    if (!phfsm->msg_handle) {
        ret = HFSM_ERR_MSG_HANDLE;
        goto err;
    }

    /*! create state handler */
    ret = state_create(&phfsm->sta_handle);
    if (HFSM_OK != ret)
        goto err;

    /*! create hfsm thread */
    ret = pthread_create(
        &phfsm->thread.id, 0, hfsm_loop, (void*)phfsm);
    if (ret) {
        ret = HFSM_ERR_THREAD;
        goto err;
    }

    *hfsm = (hfsm_handle)phfsm;
    return HFSM_OK;

err:
    if (phfsm)
        free(phfsm);
    LOGE("%s failed", __FUNCTION__);
    return ret;
}

int hfsm_destroy(hfsm_handle hfsm) {

    int ret;
    struct hfsm_t* phfsm = (struct hfsm_t*)hfsm;

    if (!phfsm)
        return HFSM_ERR_NULLPTR;

    phfsm->thread.exit = EXIT_THREAD;
    ret = pthread_cancel(phfsm->thread.id);
    if (ret) {
        ret = HFSM_ERR_THREAD;
        goto exit;
    }

    ret = pthread_join(phfsm->thread.id, 0);
    if (ret) {
        ret = HFSM_ERR_THREAD;
        goto exit;
    }

    ret = HFSM_OK;

exit:

    if (phfsm) {
        queue_destroy(phfsm->msg_handle);
        state_destroy(phfsm->sta_handle);
        free((void*)phfsm);
    }

    return ret;
}

int hfsm_add_state(hfsm_handle hfsm, hfsm_state *target, hfsm_state *parent) {

    int ret = HFSM_OK;
    hfsm_state *p;
    struct treenode* found;

    struct hfsm_t* phfsm = (struct hfsm_t*)hfsm;
    if (!phfsm || !target)
        return HFSM_ERR_NULLPTR;

    if (sizeof(hfsm_state) > HFSM_STATE_SIZE)
        return HFSM_ERR_STATE_SIZE_OVERFLOW;

    if (parent) {
        p = state_find(phfsm->sta_handle, parent->stateID);
        if (!p)
            goto err;

        ret = state_add(phfsm->sta_handle, parent, target);
        if (HFSM_OK != ret)
            goto err;
    } else {
        ret = state_add(phfsm->sta_handle, NULL, target);
        if (HFSM_OK != ret)
            goto err;
    }

err:
    return ret;
}

int hfsm_set_init_state(hfsm_handle hfsm, uint32_t stateid) {

    int ret;
    struct treenode* found;
    struct hfsm_t* phfsm = (struct hfsm_t*)hfsm;

    if (!phfsm)
        return HFSM_ERR_NULLPTR;

    phfsm->cur_state = stateid;
    return HFSM_OK;
}

int hfsm_start(hfsm_handle hfsm) {

    message msginfo = {};
    struct hfsm_t* phfsm = (struct hfsm_t*)hfsm;

    if (!phfsm)
        return HFSM_ERR_NULLPTR;

    msginfo.type = HFSM_SYS_START;
    msginfo.size = sizeof(struct hfsm_sys_t);
    msginfo.pri = PRIORITY_HIGH;
    /*! TODO: msginfo.payload */

    queue_push(phfsm->msg_handle, &msginfo);
    return HFSM_OK;
}

int hfsm_stop(hfsm_handle hfsm) {

    message msginfo = {};
    struct hfsm_t* phfsm = (struct hfsm_t*)hfsm;

    if (!phfsm)
        return HFSM_ERR_NULLPTR;

    msginfo.type = HFSM_SYS_STOP;
    msginfo.size = sizeof(struct hfsm_sys_t);
    msginfo.pri = PRIORITY_HIGH;
    /*! TODO: msginfo.payload */

    queue_push(phfsm->msg_handle, &msginfo);
    return HFSM_OK;
}

int hfsm_send_async_message(hfsm_handle hfsm, hfsm_message *msg) {

    message msginfo = {};
    struct hfsm_t* phfsm = (struct hfsm_t*)hfsm;

    if (!phfsm)
        return HFSM_ERR_NULLPTR;

    if (sizeof(hfsm_message) > MESSAGE_MAX_SIZE)
        return HFSM_ERR_MSG_OVERFLOW;

    msginfo.type = HFSM_USER;
    msginfo.size = sizeof(hfsm_message);
    msginfo.pri = PRIORITY_MID;
    memcpy(msginfo.payload, msg, msginfo.size);

    queue_push(phfsm->msg_handle, &msginfo);
    return HFSM_OK;
}


