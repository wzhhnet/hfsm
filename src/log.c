/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : log.c
*   CREATE DATE : 2021-12-27
*   MODULE      : Hierarchy tree
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#include <log.h>
#include <stdarg.h>
#include <stdio.h>

void hfsm_trace(const char *format, ...) {
    int n;
    char buf[4096];
    
    va_list ap;
    va_start(ap, format);

    n = vsnprintf(buf, sizeof(buf)-1, format, ap);
    if (n > 0 && n < (int)(sizeof(buf)-1)) {
        fprintf(stderr, "%s\n" RST, buf);
    }
    va_end (ap);
}

