/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : thread.h
*   CREATE DATE : 2021-12-20
*   MODULE      : thread control
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#ifndef _HFSM_THREAD_H
#define _HFSM_THREAD_H

/****************************************************************************
*                       Include File Section                                *
*****************************************************************************/
#include <pthread.h>
#include "types.h"

/****************************************************************************
*                       Macro Definition Section                            *
*****************************************************************************/

struct thread_ctrl_t {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
};

static inline void thread_ctrl_init(struct thread_ctrl_t *ctrl) {

    pthread_condattr_t *pattr = NULL;
    if (!ctrl)
        return;

#if !defined(__ANDROID__) && !defined(HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE)
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pattr = &attr;
#endif
    pthread_cond_init(&ctrl->cond, pattr);
    if (pattr)
        pthread_condattr_destroy(pattr);

    pthread_mutex_init(&ctrl->mutex, NULL);
}

static inline void thread_ctrl_uninit(struct thread_ctrl_t *ctrl) {

    if (!ctrl)
        return;

    pthread_cond_destroy(&ctrl->cond);
    pthread_mutex_unlock(&ctrl->mutex);
}

static inline void thread_ctrl_lock(struct thread_ctrl_t *ctrl) {
    if (ctrl)
        pthread_mutex_lock(&ctrl->mutex);
}

static inline void thread_ctrl_unlock(struct thread_ctrl_t *ctrl) {
    if (ctrl)
        pthread_mutex_unlock(&ctrl->mutex);
}

static inline void thread_ctrl_wait(struct thread_ctrl_t *ctrl) {
    if (ctrl)
        pthread_cond_wait(&ctrl->cond, &ctrl->mutex);
}

static inline void thread_ctrl_signal(struct thread_ctrl_t *ctrl) {
    if (ctrl)
        pthread_cond_signal(&ctrl->cond);
}

static inline void thread_ctrl_broadcast(struct thread_ctrl_t *ctrl) {
    if (ctrl)
        pthread_cond_broadcast(&ctrl->cond);
}

#endif //_HFSM_THREAD_H

