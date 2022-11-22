/********** Copyright(C) 2021 MXNavi Co.,Ltd. ALL RIGHTS RESERVED **********/
/****************************************************************************
*   FILE NAME   : types.h
*   CREATE DATE : 2021-12-16
*   MODULE      :
*   AUTHOR      : wanch
*---------------------------------------------------------------------------*
*   MEMO        :
*****************************************************************************/
#ifndef _HFSM_TYPES_H
#define _HFSM_TYPES_H


#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef __cplusplus
#ifndef bool
#define bool int
#define true    1
#define false   0
#endif
#endif


typedef signed char             int8_t;
typedef short int               int16_t;
typedef int                     int32_t;
typedef long long int           int64_t;

/* Unsigned.  */
typedef unsigned char           uint8_t;
typedef unsigned short int      uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long int  uint64_t;


#endif /*! _HFSM_TYPES_H */

