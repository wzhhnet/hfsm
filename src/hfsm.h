/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : hfsm.h
*   CREATE DATE : 2021-12-16
*   MODULE      :
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#ifndef HIERARCHICAL_FINITE_STATE_MACHINE_H
#define HIERARCHICAL_FINITE_STATE_MACHINE_H


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "types.h"

#define MODULE_NAME_HFSM        "HFSM"
#define HFSM_STATE_NAME_LEN     (64)
#define MESSAGE_PAYLOAD_LEN     (512)

enum hfsm_result_e {
    HFSM_OK = 0,
    HFSM_ERR,
    HFSM_ERR_MALLOC,
    HFSM_ERR_THREAD,
    HFSM_ERR_NULLPTR,
    HFSM_ERR_STATE_EXIST,
    HFSM_ERR_STATE_NOT_EXIST,
    HFSM_ERR_STATE_SIZE_OVERFLOW,
    HFSM_ERR_STATE_EXCEED_MAX_CNT,
    HFSM_ERR_ROOT_STATE_EXIST,
    HFSM_ERR_MSG_HANDLE,
    HFSM_ERR_MSG_OVERFLOW
};

enum hfsm_proc_msg_result_e {
    HFSM_PROC_MSG_OK = 0,
    HFSM_PROC_MSG_UNHANDLER
};


typedef void* hfsm_handle;

typedef struct tag_msg_t {
    int32_t what;
    int32_t arg1;
    int32_t arg2;
    int32_t result;
    uint64_t timeStamp;
    int8_t payload[MESSAGE_PAYLOAD_LEN];
} hfsm_message;

typedef uint32_t (*hfsm_msg_process_f)(
    hfsm_message* /* msg */, void* /* argv */, uint32_t* /* stateID */);

typedef struct hfsm_state_t {
    uint32_t stateID;                        /* state id */
    uint8_t name[HFSM_STATE_NAME_LEN];       /* state name */
    uint32_t (*enter)(void* argv);           /* call when state enter */
    uint32_t (*exit)(void* argv);            /* call when state exit */
    /* call when process a message */
    hfsm_msg_process_f process;
    void* argv;                              /* User private, used by processMessage */
} hfsm_state;

/**
  *    @brief create HFSM
  *
  *    create HFSM
  *    @param[in]  pstFsmAttr: attribute of HFSM
  *    @param[out]  phfsm: point of FHSM handle
  *    @return     0 success, non-zero error code
 */
int hfsm_create(hfsm_handle *hfsm);

/**
  *    @brief destroy HFSM
  *
  *    destroy HFSM
  *    @param[in]  hfsm handle
  *    @return     0 success, non-zero error code
 */
int hfsm_destroy(hfsm_handle hfsm);

/**
  *    @brief add state
  *
  *    add state for HFSM
  *    @param[in]  hfsm: point of FHSM handle
  *    @param[in]  state: state to add
  *    @param[in]  parent: parent of state to add
  *    @return     0 success, non-zero error code
 */
int hfsm_add_state(hfsm_handle hfsm, hfsm_state *target, hfsm_state *parent);

/**
  *    @brief set initial state
  *
  *    set initial state for HFSM
  *    @param[in]  hfsm: point of FHSM handle
  *    @param[in]  stateName: the name of initial state
  *    @return     0 success, non-zero error code
 */
int hfsm_set_init_state(hfsm_handle hfsm, uint32_t stateid);

/**
  *    @brief start
  *
  *    start
  *    @param[in]  hfsm: point of FHSM handle
  *    @return     0 success, non-zero error code
 */
int hfsm_start(hfsm_handle hfsm);

/**
  *    @brief stop
  *
  *    stop
  *    @param[in]  hfsm: point of FHSM handle
  *    @return     0 success, non-zero error code
 */
int hfsm_stop(hfsm_handle hfsm);

/**
  *    @brief send an asynchronous message
  *
  *    send an asynchronous message to HFSM
  *    @param[in]  hfsm: point of FHSM handle
  *    @param[in]  msg: message point to send
  *    @return     0 success, non-zero error code
 */
int hfsm_send_async_message(hfsm_handle hfsm, hfsm_message *msg);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif

