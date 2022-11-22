/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : message.h
*   CREATE DATE : 2021-12-16
*   MODULE      : priority queue
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#ifndef _HFSM_MESSAGE_H
#define _HFSM_MESSAGE_H

/****************************************************************************
*                       Include File Section                                *
*****************************************************************************/
#include "types.h"

/****************************************************************************
*                       Macro Definition Section                            *
*****************************************************************************/
#define MESSAGE_MAX_SIZE        (768)

typedef void* que_handle;

typedef enum priority_e {
    PRIORITY_LOW,
    PRIORITY_MID,
    PRIORITY_HIGH
} priority;

typedef struct message_t {
    uint16_t type;
    priority pri;
    uint32_t size; /*!> message length for user */
    char payload[MESSAGE_MAX_SIZE];
} message;

void queue_create(uint32_t max, que_handle *handle);
void queue_destroy(que_handle handle);
void queue_push(que_handle handle, message *msg);
void queue_pop(que_handle handle, message *msg);
void queue_wait(que_handle handle);
void queue_shutdown(que_handle handle);
uint32_t queue_size(que_handle handle);


#endif /*! _HFSM_MESSAGE_H */

