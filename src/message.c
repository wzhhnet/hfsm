/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : hfsm.h
*   CREATE DATE : 2021-12-17
*   MODULE      :
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#include "log.h"
#include "list.h"
#include "bitops.h"
#include "thread.h"
#include "message.h"

struct msginfo_t {
    struct listnode node;
    int bit;
    message msg;
};

struct msg_pool_t {
    uint32_t max;   /*!> max sum of unit */
    //uint32_t unit;  /*!> unit buffer size(max size of payload for message) */
    uint32_t *bits;
    struct msginfo_t *buff;
};

struct queueinfo_t {
    struct thread_ctrl_t ctrl;
    struct listnode queue;
    uint32_t pool_size;
    struct msg_pool_t pool;
};

static uint32_t queinfo_mem_calc(uint32_t max) {

    uint32_t bits_size, buff_size, total;

    /*! calculate size of msg_pool_t->bits */
    bits_size = sizeof(uint32_t) * BITS_TO_WORDS(max);
    /*! calculate size of msg_pool_t->buff */
    buff_size = sizeof(struct msginfo_t) * max;

    total = sizeof(struct queueinfo_t) + bits_size + buff_size;
    return total;
}

static bool queinfo_mem_assign(
    struct queueinfo_t* queinfo, void *block, uint32_t blksize) {

    struct msg_pool_t *pool;
    uint32_t bits_size, buff_size, unit_size, total;
    char *p = block;

    if (!queinfo || !block)
        return false;

    if (blksize < sizeof(struct queueinfo_t))
        return false;

    pool = &queinfo->pool;

    p += sizeof(struct queueinfo_t);
    total -= sizeof(struct queueinfo_t);

    /*! calculate size of msg_pool_t->bits */
    bits_size = sizeof(uint32_t) * BITS_TO_WORDS(pool->max);
    /*! calculate size of msg_pool_t->buff */
    buff_size = sizeof(struct msginfo_t) * pool->max;

    if (total < (bits_size + buff_size))
        return false;

    pool->bits = (uint32_t*)p;
    p += bits_size;
    pool->buff = (struct msginfo_t*)p;

    return true;
}

static struct msginfo_t* queinfo_allc_message(struct msg_pool_t *pool) {

    if (!pool)
        return NULL;

    const int max_buf_size = (int)pool->max;
    int bit = bitmask_ffz(pool->bits, max_buf_size);

    if (bit >= 0 && bit < max_buf_size) {
        pool->buff[bit].bit = bit;
        bitmask_set(pool->bits, bit);
        return &pool->buff[bit];
    }
    return NULL;
}

static void queinfo_release_message(
    struct msg_pool_t *pool, struct msginfo_t* msg) {

    if (pool == NULL || msg == NULL)
        return;

    if (msg->bit >= 0 && msg->bit < (int)pool->max) {
        bitmask_clear(pool->bits, msg->bit);
    }
}


#if 0 // removed
static void queue_signal(que_handle handle) {

    struct queueinfo_t *pqueinfo;
    if (!handle)
        return;

    pqueinfo = (struct queueinfo_t*)handle;
    thread_ctrl_wait(&pqueinfo->ctrl);
}

static void queue_lock(que_handle handle) {

    struct queueinfo_t *pqueinfo;
    if (!handle)
        return;

    pqueinfo = (struct queueinfo_t*)handle;
    thread_ctrl_signal(&pqueinfo->ctrl);
}

static void queue_unlock(que_handle handle) {

    struct queueinfo_t *pqueinfo;
    if (!handle)
        return;

    pqueinfo = (struct queueinfo_t*)handle;
    thread_ctrl_unlock(&pqueinfo->ctrl);
}
#endif

static message* queue_front(que_handle handle) {

    message* ret;
    struct listnode *n;
    struct msginfo_t *msginfo;
    struct queueinfo_t *pqueinfo;

    if (!handle)
        return NULL;

    pqueinfo = (struct queueinfo_t*)handle;
    if (list_empty(&pqueinfo->queue))
        return NULL;

    n = list_head(&pqueinfo->queue);
    msginfo = list_entry(n, struct msginfo_t, node);
    return &msginfo->msg;
}

void queue_create(uint32_t max, que_handle *handle) {

    bool ret;
    char *mem;
    uint32_t total;
    struct queueinfo_t* pqueinfo;

    if (!handle || !max)
        return;

    total = queinfo_mem_calc(max);
    mem = (char*)malloc(total);
    if (!mem || total < sizeof(struct queueinfo_t))
        return;

    /*! assignment  */
    memset(mem, 0, total);
    pqueinfo = (struct queueinfo_t*)mem;
    pqueinfo->pool.max = max;
    ret = queinfo_mem_assign(pqueinfo, mem, total);
    if (!ret)
        goto release;

    /*! initialize */
    list_init(&pqueinfo->queue);
    thread_ctrl_init(&pqueinfo->ctrl);
    pqueinfo->pool_size = max;
    *handle = (que_handle)pqueinfo;

    return;

release:

    if (pqueinfo)
        free(pqueinfo);
}

void queue_destroy(que_handle handle) {

    struct queueinfo_t* pqueinfo;
    if (!handle)
        return;

    pqueinfo = (struct queueinfo_t*)handle;
    thread_ctrl_uninit(&pqueinfo->ctrl);

    free(handle);
}

void queue_push(que_handle handle, message *msg) {

    struct queueinfo_t *pqueinfo;
    struct msginfo_t *msginfo;
    struct listnode *n;

    if (!handle || !msg)
        return;

    pqueinfo = (struct queueinfo_t*)handle;
    thread_ctrl_lock(&pqueinfo->ctrl);   /*!< lock for queue */
    LOGD("%s before push list_count=%d, item[%d, %d]", __FUNCTION__, list_count(&pqueinfo->queue), msg->type, msg->pri);

    /*! allocate a msginfo from pool*/
    msginfo = queinfo_allc_message(&pqueinfo->pool);
    if (!msginfo)
        goto unlock;

    /*! initialize msginfo */
    list_init(&msginfo->node);
    msginfo->msg.size = msg->size;
    msginfo->msg.pri = msg->pri;
    msginfo->msg.type = msg->type;
    memcpy(msginfo->msg.payload, msg->payload, msg->size);
    if (list_empty(&pqueinfo->queue)) {
        list_add_head(&pqueinfo->queue, &msginfo->node);
    } else {
        /*! find proper position and insert node */
        list_for_each_reverse(n, &pqueinfo->queue) {
            struct msginfo_t *tmp;
            tmp = list_entry(n, struct msginfo_t, node);
            if (tmp && tmp->msg.pri >= msg->pri) {
                list_add_head(n, &msginfo->node);
                break;
            }
        }
    }

    LOGD("%s after push list_count=%d", __FUNCTION__, list_count(&pqueinfo->queue));
    thread_ctrl_signal(&pqueinfo->ctrl);

unlock:
    thread_ctrl_unlock(&pqueinfo->ctrl);   /*!< unlock for queue */

}

void queue_pop(que_handle handle, message *msg) {

    struct listnode *n;
    struct msginfo_t *msginfo;
    struct queueinfo_t *pqueinfo;

    if (!handle || !msg)
        return;

    pqueinfo = (struct queueinfo_t*)handle;
    thread_ctrl_lock(&pqueinfo->ctrl);   /*!< lock for queue */
    LOGD("%s before pop list_count=%d", __FUNCTION__, list_count(&pqueinfo->queue));

    if (list_empty(&pqueinfo->queue))
        goto unlock;

    n = list_head(&pqueinfo->queue);
    list_remove(n);

    msginfo = list_entry(n, struct msginfo_t, node);
    memcpy(msg, &msginfo->msg, sizeof(message));
    queinfo_release_message(&pqueinfo->pool, msginfo);
    LOGD("%s after pop list_count=%d, item[%d, %d]", __FUNCTION__, list_count(&pqueinfo->queue), msg->type, msg->pri);
unlock:
    thread_ctrl_unlock(&pqueinfo->ctrl);   /*!< unlock for queue */

}

void queue_wait(que_handle handle) {

    struct queueinfo_t *pqueinfo;
    if (!handle)
        return;

    pqueinfo = (struct queueinfo_t*)handle;
    thread_ctrl_lock(&pqueinfo->ctrl);
    /*! wait if queue is empty */
    if (!list_count(&pqueinfo->queue)) {
        thread_ctrl_wait(&pqueinfo->ctrl);
    }
    thread_ctrl_unlock(&pqueinfo->ctrl);
}

void queue_shutdown(que_handle handle) {
    struct listnode *n, *node;
    struct queueinfo_t *pqueinfo;
    struct msginfo_t *msg;

    if (!handle)
        return;

    pqueinfo = (struct queueinfo_t*)handle;
    thread_ctrl_lock(&pqueinfo->ctrl);
    list_for_each_safe(node, n, &pqueinfo->queue) {
        msg = list_entry(node,
                struct msginfo_t, node);
        list_remove(node);
        queinfo_release_message(&pqueinfo->pool, msg);
    }
    thread_ctrl_broadcast(&pqueinfo->ctrl);
    thread_ctrl_unlock(&pqueinfo->ctrl);
}

uint32_t queue_size(que_handle handle) {

    uint32_t size;
    struct queueinfo_t *pqueinfo;
    if (!handle)
        return;

    thread_ctrl_lock(&pqueinfo->ctrl);   /*!< lock for queue */
    size = list_count(&pqueinfo->queue);
    thread_ctrl_unlock(&pqueinfo->ctrl); /*!< unlock for queue */

    return size;
}



