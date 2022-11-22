/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : state.h
*   CREATE DATE : 2021-12-16
*   MODULE      :
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#ifndef _HFSM_STATE_H
#define _HFSM_STATE_H

#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************
*                       Include File Section                                *
*****************************************************************************/


/****************************************************************************
*                       Macro Definition Section                            *
*****************************************************************************/
#define HFSM_MAX_STATES     (64)
#define HFSM_STATE_SIZE          (128)

typedef void* state_handle;
typedef bool (*traverse_fn)(hfsm_state*, void*);

typedef struct traverse_t {
    traverse_fn func;
    void *arg;
} traverse;

int state_create(state_handle *handle);
int state_destroy(state_handle handle);
int state_add(state_handle handle, hfsm_state *parent, hfsm_state *pstate);
int state_set(state_handle handle, uint32_t key);
hfsm_state* state_find(state_handle handle, uint32_t key);
hfsm_state* state_get(state_handle handle);
hfsm_state* state_parent(state_handle handle, hfsm_state *pstate);
hfsm_state* state_forefathers(state_handle handle, hfsm_state *p1, hfsm_state *p2);
hfsm_state* state_root(state_handle handle);
int state_downward(state_handle handle, hfsm_state *src, hfsm_state *dst, traverse *tra);

#ifdef __cplusplus
}
#endif

#endif /*! _HFSM_STATE_H */
