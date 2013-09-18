/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/
#ifndef _LIB_TYPE_H
#define _LIB_TYPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#include "lib_typeconst.h"

#undef TRUE
#undef FALSE

#undef UINT32
#undef UINT16

#undef UINT8
#undef INT32
#undef UINT32FIXED

#if ( INTMAX < 2)
#error "fatal error: INTMAX is too small!"
#endif /* ( INTMAX < 2) */

#ifndef _UINT32FIXED_
#define _UINT32FIXED_
typedef unsigned int UINT32FIXED;
#endif /* _UINT32FIXED_ */

#ifndef _UINT32_
#define _UINT32_
typedef unsigned long UINT32;
#endif /* _UINT32_ */

#ifndef _INT32_
#define _INT32_
typedef long INT32;
#endif /* _INT32_ */

#ifndef _UINT16_
#define _UINT16_
typedef unsigned short UINT16;
#endif /* _UINT16_ */

#ifndef _UINT8_
#define _UINT8_
typedef unsigned char UINT8;
#endif /* _UINT8_ */

#ifndef _REAL_
#define _REAL_
typedef double REAL;
#endif/* _REAL_ */

#ifndef _UINT32_T_
#define _UINT32_T_
typedef unsigned int   uint32_t;
#endif/*_UINT32_T_*/

#ifndef _UINT16_T_
#define _UINT16_T_
typedef unsigned short  uint16_t;
#endif/*_UINT16_T_*/

#ifndef _UINT8_T_
#define _UINT8_T_
typedef unsigned char   uint8_t;
#endif/*_UINT8_T_*/

#ifndef _WORD_T_
#define _WORD_T_
typedef unsigned long   word_t;
#endif/*_WORD_T_*/

#define NULL_PTR (0)

#define ASSERT(condition) do{\
    if(!(condition)) {\
        sys_log(LOGSTDOUT, "warning: assert failed at %s:%d\n", __FUNCTION__, __LINE__);\
        exit(EXIT_FAILURE);\
    }\
}while(0)

/**
 *  data : support max integer up to BIGINTSIZE bits
 *  The BIGINT struct represent Data:
 *      data[ INTMAX - 1] || ... ||  data[0]
 *  where
 *      Data = SUM( data[ i ] * M ^ i, where i = 0.. INTMAX-1 and M = 2 ^ WORDSIZE )
 **/
typedef struct
{
    UINT32  len;     /* len = 0.. INTMAX */

    UINT32  data[INTMAX];

}BIGINT;


typedef UINT8  STRCHAR;
#define ERR_MODULE_ID   ((UINT32)(~((UINT32)0)))

/*EC_BOOL value*/
#define EC_TRUE  ((UINT32)0)
#define EC_FALSE ((UINT32)1)

typedef UINT32 EC_BOOL;

typedef struct
{
  BIGINT x;
  BIGINT y;
}EC_CURVE_POINT;

typedef struct
{
    BIGINT x;
    BIGINT y;
    BIGINT z;
}EC_CURVE_AFF_POINT;

typedef struct
{
  BIGINT a;
  BIGINT b;
}ECF2N_CURVE;

typedef struct
{
  BIGINT a;
  BIGINT b;
}ECFP_CURVE;

typedef struct
{
    BIGINT private_key;
    EC_CURVE_POINT public_key;
}ECC_KEYPAIR;

typedef struct
{
    BIGINT r;
    BIGINT s;
}ECC_SIGNATURE;

typedef struct
{
    BIGINT hash;
    BIGINT sn;
}SOFTSERIAL;

typedef struct _LIST_NODE
{
    struct _LIST_NODE *next;
    struct _LIST_NODE *prev;
}LIST_NODE;


/**
 *
 *   data = SUM( data[ i ] * M ^ i, where i from 0 to len - 1 and M = 2 ^ BIGINTSIZE )
 *
 *  where data[ 0 ] mounted at head of EBGN, data[ len - 1 ] mounted at tail of EBGN
**/
typedef struct
{
    LIST_NODE   ebgn_item_head;  /*the hang point of the head item and tail item in the EBGN*/
    UINT32      len;             /*number of BIGINT in EBGN*/
    UINT32      sgn;             /*positive or negative sign of the EBGN*/
}EBGN;

#define POS_SGN EC_TRUE
#define NEG_SGN EC_FALSE

typedef EBGN EBIGINT;

typedef struct
{
     LIST_NODE   item_node;   /*the hang point of this item in the EBGN*/

     BIGINT      *bgn;   /*the data of this item is BIGINT*/
}EBGN_ITEM;

typedef EBGN_ITEM EBIGINT_ITEM;
typedef struct
{
    LIST_NODE   poly_item_head;  /*the hang point of the head item and tail item in the poly*/
}POLY;
typedef BIGINT DEGREE;

typedef struct
{
     LIST_NODE    item_node;   /*the hang point of this item in the poly*/

     DEGREE       item_deg;    /*the degree of the principal variable in this item*/
     EC_BOOL      bgn_coe_flag;/*TRUE means the coe of this item is a bgn constant.*/
                               /*FALSE means the coe of this item is a coe.*/
     union
     {
        BIGINT      *bgn_coe;   /*the coe of this item is a bgn constant*/
        POLY        *poly_coe;  /*the coe of this item is a coe*/
     }coe;
}POLY_ITEM;

typedef  UINT32 ( * FUNC_RAND_GEN )(BIGINT * random);
typedef  UINT32 ( * FUNC_HASH )(const UINT8 *message,const UINT32 messagelen, BIGINT *hash);

typedef struct
{
     UINT32      num[ 2 ];    /*the row num or col num of matrix, depending on rotated_flag*/

     UINT32      rotated_flag; /*when rotated_flag = 0(not rotated), row row_idx and col col_idx data is data_area[ row_idx ][ col_idx ]*/
                               /*when rotated_flag = 1(rotated)    , row row_idx and col col_idx data is data_area[ col_idx ][ row_idx ]*/

     void *        blocks;       /*list of MATRIX_BLOCKs*/
}MATRIX;

typedef struct
{
    LIST_NODE head;          /*the head of vector in row or column direction, depending on rotated_flag*/
    UINT32    num;           /*the row num or col num of vector, depending on rotated_flag*/
    UINT32    rotated_flag;  /*when rotated_flag = 0(not rotated), the vector is organized as row vector*/
                             /*when rotated_flag = 1             , the vector is organized as col vector*/
}VECTOR;

typedef struct
{
    UINT32 capacity;
    UINT32 len;
    UINT8 *str;
}CSTRING;

typedef struct
{
    UINT32 len;
    UINT8 *buf;
}CBYTES;

#define LOGD_FILE_RECORD_LIMIT_ENABLED  ((UINT32) 1) /*record limit set and enabled*/
#define LOGD_FILE_RECORD_LIMIT_DISABLED ((UINT32) 2) /*record unlimit*/

#define LOGD_FILE_MAX_RECORDS_LIMIT     ((UINT32)-1)

#define LOGD_SWITCH_OFF_ENABLE          ((UINT32) 1) /*log device can be switch off*/
#define LOGD_SWITCH_OFF_DISABLE         ((UINT32) 2) /*log device can NOT be switch off*/

#define LOGD_PID_INFO_ENABLE            ((UINT32) 1) /*log device record pid info*/
#define LOGD_PID_INFO_DISABLE           ((UINT32) 2) /*log device NOT record pid info*/

typedef struct _LOG
{
    UINT32 type;/*type of log device*/
    UINT32 switch_off_enable; /*enable or disable switch_off*/
    UINT32 pid_info_enable;   /*enable or disable pid info at the first line*/
    
    struct _LOG *redirect_log;/*current log is rediect to*/
    
    union
    {
        struct
        {
            UINT32   fname_with_date_switch;
            CSTRING *fname;   /*one part of log file name. */
                              /*when fname_with_date_switch = SWITCH_ON , log file name = {fname}_{date/time}.log*/
                              /*when fname_with_date_switch = SWITCH_OFF, log file name = {fname}.log*/
            CSTRING *mode;    /*log file open mode*/
            FILE    *fp;
            
            pthread_mutex_t   *mutex;   /*lock for freopen*/

            UINT32   record_limit_enabled; /*record limit enabled or disabled*/
            UINT32   max_records;          /*record limit*/
            UINT32   cur_records;          /*current records*/

            UINT32   tcid;
            UINT32   rank;            
        }file;

        CSTRING *cstring;
    }logd;/*log device*/    
}LOG;

/*---------------------------------------- interface of ctimet ----------------------------------------------*/
#define CTIMET_GET(ctimet)                                 (time(&(ctimet)))
#define CTIMET_DIFF(ctimet_start, ctimet_end)              (difftime(ctimet_end, ctimet_start))

#define CTIMET_TO_LOCAL_TIME(ctimet) (c_localtime_r(&(ctimet)))
#define CTIMET_TO_TM(ctimet)         (CTIMET_TO_LOCAL_TIME(ctimet))
#define CTIMET_YEAR(ctimet)          ((CTIMET_TO_LOCAL_TIME(ctimet)->tm_year) + 1900)
#define CTIMET_MONTH(ctimet)         ((CTIMET_TO_LOCAL_TIME(ctimet)->tm_mon) + 1)
#define CTIMET_MDAY(ctimet)          ((CTIMET_TO_LOCAL_TIME(ctimet)->tm_mday))
#define CTIMET_HOUR(ctimet)          ((CTIMET_TO_LOCAL_TIME(ctimet)->tm_hour))
#define CTIMET_MIN(ctimet)           ((CTIMET_TO_LOCAL_TIME(ctimet)->tm_min))
#define CTIMET_SEC(ctimet)           ((CTIMET_TO_LOCAL_TIME(ctimet)->tm_sec))

#define BCOPY(src, des, len)    memcpy(des, src, len)
#define BSET(pstr, ch, len)     memset(pstr, ch, len)
#define BCMP(pstr1, pstr2, len) memcmp(pstr1, pstr2, len)
#define STRCMP(pstr1, pstr2)    strcmp(pstr1, pstr2)

#define DMIN(a, b)      ((a) <= (b) ? (a) : (b))
#define DMAX(a, b)      ((a) <= (b) ? (b) : (a))

/*feed src to des and take back des by src at last*/
#define XCHG(type, des, src)  do{type __t__; (__t__) = (des); (des)=(src); (src)=(__t__);}while(0)

#endif /*_LIB_TYPE_H*/
#ifdef __cplusplus
}
#endif/*__cplusplus*/
