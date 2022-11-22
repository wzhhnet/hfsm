/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : log.h
*   CREATE DATE : 2021-12-27
*   MODULE      :
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#ifndef _HFSM_LOG_H
#define _HFSM_LOG_H

#ifndef LOG_TAG
#define LOG_TAG "hfsm"
#endif

#define RED "\033[31m"
#define GRN "\033[32m"
#define BRN "\033[33m"
#define BLU "\033[34m"
#define MAT "\033[345"
#define RST "\033[0m"

void hfsm_trace(const char *format, ...);

#define LOGD(...)   ((void)hfsm_trace(BRN LOG_TAG "-d: " __VA_ARGS__))
#define LOGI(...)   ((void)hfsm_trace(GRN LOG_TAG "-i: " __VA_ARGS__))
#define LOGW(...)   ((void)hfsm_trace(BLU LOG_TAG "-w: " __VA_ARGS__))
#define LOGE(...)   ((void)hfsm_trace(RED LOG_TAG "-e: " __VA_ARGS__))

#define LOGE_IF(condition, ...) do { if (condition) {  (void)LOGE(__VA_ARGS__); } }while(0)

#define ASSERT(condition)   LOGE_IF(!(condition), "assert failed at: %s() @line %d", __FUNCTION__, __LINE__)

#endif /*! _HFSM_LOG_H */

