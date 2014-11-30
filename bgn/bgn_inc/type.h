/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#ifndef _TYPE_H
#define _TYPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#include "bgnctrl.h"
#include "typeconst.h"

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

#ifndef _UINT64_
#define _UINT64_
typedef unsigned long long UINT64;
#endif /* _UINT32_ */

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

#ifndef _RWSIZE_
#define _RWSIZE_
typedef ssize_t RWSIZE;
#endif/* _RWSIZE_ */

#ifndef _UINT64_T_
#define _UINT64_T_
#if (32 == WORDSIZE)
typedef unsigned long long  uint64_t;
#endif/*(32 == WORDSIZE)*/
#if (64 == WORDSIZE)
typedef unsigned long   uint64_t;
#endif/*(64 == WORDSIZE)*/
#endif/*_UINT32_T_*/

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

#ifndef _C_TIME_T_
#define _C_TIME_T_
typedef time_t         ctime_t;
#endif/*_C_TIME_T_*/

#ifndef _UUID_
#define _UUID_
typedef UINT8 UUID[16];
#endif/* _UUID_ */

#define NULL_PTR 0

typedef void (*F_CLEANER)(void *);

#define ASSERT(condition) do{\
    if(!(condition)) {\
        sys_log(LOGSTDOUT, "error: assert failed at %s:%d\n", __FUNCTION__, __LINE__);\
        exit(EXIT_FAILURE);\
    }\
}while(0)

#define CHOICE(condition, chose_true, chose_false) ((condition)? (chose_true) : chose_false)

#define CHAR_IS_VISIBLE(ch)     (0x20 < (ch) && (ch) < 0x7f)

#if(64 == WORDSIZE)
/*#define CHECK_UINT32_HIGH_BITS_IS_ZERO(num)  ASSERT(0 == ((num) >> (WORDSIZE/2)))*/
#define CHECK_UINT32_HIGH_BITS_IS_ZERO(num)  do{}while(0)
#define UINT32_TO_INT32(num)                (((num) << (WORDSIZE/2)) >> (WORDSIZE/2))
#define INT32_TO_UINT32(num)                ((UINT32)(num))
#else
#define CHECK_UINT32_HIGH_BITS_IS_ZERO(num) do{}while(0)
#define UINT32_TO_INT32(num)                (num)
#define INT32_TO_UINT32(num)                ((UINT32)(num))
#endif

#if(64 == WORDSIZE)
#define MAJOR(num64bits)    ((UINT32)((num64bits) >> 32))
#define MINOR(num64bits)    ((UINT32)((num64bits) & 0xffffffff))
#else
#define MAJOR(num64bits)    ((UINT32)(((UINT64)(num64bits)) >> 32))
#define MINOR(num64bits)    ((UINT32)(((UINT64)(num64bits)) & 0xffffffff))
#endif

#if 0
netinet/in.h
======
/* We can optimize calls to the conversion functions.  Either nothing has
   to be done or we are using directly the byte-swapping functions which
   often can be inlined.  */
# if __BYTE_ORDER == __BIG_ENDIAN
/* The host byte order is the same as network byte order,
   so these functions are all just identity.  */
# define ntohl(x)       (x)
# define ntohs(x)       (x)
# define htonl(x)       (x)
# define htons(x)       (x)
# else
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define ntohl(x)     __bswap_32 (x)
#   define ntohs(x)     __bswap_16 (x)
#   define htonl(x)     __bswap_32 (x)
#   define htons(x)     __bswap_16 (x)
#  endif
# endif
#endif

#if (__BYTE_ORDER == __BIG_ENDIAN)
#define ntoh_uint64(x)       (x)
#define hton_uint64(x)       (x)

#define ntoh_uint32(x)       (x)
#define hton_uint32(x)       (x)

#define ntoh_uint32_t(x)     (x)
#define hton_uint32_t(x)     (x)

#define ntoh_uint16(x)       (x)
#define hton_uint16(x)       (x)

#endif/*(__BYTE_ORDER == __BIG_ENDIAN)*/

#if (__BYTE_ORDER == __LITTLE_ENDIAN)

#define ntoh_uint64(x)       __bswap_64(x)
#define hton_uint64(x)       __bswap_64(x)

#if 1
#if (32 == WORDSIZE)
#define ntoh_uint32(x)       __bswap_32(x)
#define hton_uint32(x)       __bswap_32(x)
#define ntoh_uint32_t(x)     __bswap_32(x)
#define hton_uint32_t(x)     __bswap_32(x)
#endif/*(32 == WORDSIZE)*/

#if (64 == WORDSIZE)
#define ntoh_uint32(x)       __bswap_64(x)
#define hton_uint32(x)       __bswap_64(x)
#define ntoh_uint32_t(x)     __bswap_32(x)
#define hton_uint32_t(x)     __bswap_32(x)
#endif/*(64 == WORDSIZE)*/

#define ntoh_uint16(x)       __bswap_16(x)
#define hton_uint16(x)       __bswap_16(x)

#endif
#if 0
#define ntoh_uint32(x)       dbg_ntoh_uint32(x)
#define hton_uint32(x)       dbg_hton_uint32(x)

#define ntoh_uint16(x)       dbg_ntoh_uint16(x)
#define hton_uint16(x)       dbg_hton_uint16(x)

#define ntoh_uint8(x)        (x)
#define hton_uint8(x)        (x)
#endif

#endif/*(__BYTE_ORDER == __LITTLE_ENDIAN)*/

#ifndef PRIx64
#if (32 == __WORDSIZE)
#define PRIx64 "lld" 
#endif
#if (64 == __WORDSIZE)
#define PRIx64 "lu" 
#endif
#endif/*PRIx64*/


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

#define GET_BIGINT_LEN(a) ((a)->len)
#define SET_BIGINT_LEN(a,len) ((a)->len = (len))

typedef UINT8  STRCHAR;

#define ERR_MODULE_ID   ((UINT32)(~((UINT32)0)))

/*EC_BOOL value*/
#define EC_TRUE      ((UINT32) 0)
#define EC_FALSE     ((UINT32) 1)
#define EC_OBSCURE   ((UINT32) 2)
#define EC_ERROR     ((UINT32)-1)

#define EC_EQ     ((UINT32) 0) /* == */
#define EC_GT     ((UINT32) 1) /* >  */
#define EC_LT     ((UINT32) 2) /* <  */
#define EC_GE     ((UINT32) 3) /* >= */
#define EC_LE     ((UINT32) 4) /* <= */

#define UINT32_ZERO ((UINT32) 0)
#define UINT32_ONE  ((UINT32) 1)

#define ERR_FD      ((int)-1)
#define ERR_SEEK    (-1)

#define FILE_PAD_CHAR (0x00)

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
    UINT32      len;         /*number of BIGINT in EBGN*/
    UINT32      sgn;         /*positive or negative sign of the EBGN*/
    //UINT32      pos;             /*radix point position*/
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

/**
*   About poly_deg, constant, poly_var_deg, poly_var_coe
*
*   Example:
*       let P(x,y,z) = a_n(y,z)*x^n + .... + a_1(y,z)*x, + a_0(y,z)
*   and its principal is x,
*   then the items of P(x,y,z) are
*       a_n(y,z)*x^n, ..., a_1(y,z)*x, a_0(y,z)
*
*   for each item, e.g., a_i(y,z) * x^i,
*       poly_var_deg of item a_i(y,z) * x^i = deg( a_i(y,z) )
*       poly_var_coe of item a_i(y,z) * x^i = a_i(y,z)
*   i.e,
*       item a_i(y,z) * x^i = poly_var_coe * x^{poly_var_deg}
*
*   define the degree of item a_i(y,z) * x^i as
*       i + deg(a_i(y,z))
*   i.e, the item degree is
*       deg(poly_item)  = poly_var_deg + deg( poly_var_coe)
*
*   how to define the degree of polynomial?
*   it's the max of all deg(poly_item), i.e,
*       deg( poly ) = max{ deg(poly_item) }
*    where deg( constant ) = 0 if the poly is a constant.
*
*   note:
*       the constant is represented by a poly but not a poly item.
*
**/
typedef struct
{
    LIST_NODE   poly_item_head;  /*the hang point of the head item and tail item in the poly*/
}POLY;

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
typedef BIGINT DEGREE;
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
typedef UINT32 DEGREE;
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

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
     UINT32       num[ 2 ];    /*the row num or col num of block, depending on rotated_flag*/

#if (MATRIXR_PATCH_SWITCH == SWITCH_OFF)
    UINT32       data_area[ MATRIX_VECTOR_WIDTH ][ MATRIX_VECTOR_WIDTH ]; /*data address references*/
#endif/*(MATRIXR_PATCH_SWITCH == SWITCH_OFF)*/

#if (MATRIXR_PATCH_01_SWITCH == SWITCH_ON)
     UINT32       data_buff_addr;
     UINT32       data_area[ MATRIX_VECTOR_WIDTH ][ MATRIX_VECTOR_WIDTH ]; /*data address references*/
#endif/*(MATRIXR_PATCH_01_SWITCH == SWITCH_ON)*/

#if (MATRIXR_PATCH_02_SWITCH == SWITCH_ON)
     REAL         data_area[ MATRIX_VECTOR_WIDTH ][ MATRIX_VECTOR_WIDTH ]; /*data zone*/
#endif/*(MATRIXR_PATCH_02_SWITCH == SWITCH_ON)*/

     UINT32       rotated_flag; /*rotated_flag inheritd from matrix*/
}MATRIX_BLOCK;

typedef struct
{
     UINT32      num[ 2 ];    /*the row num or col num of matrix, depending on rotated_flag*/

     UINT32      rotated_flag; /*when rotated_flag = 0(not rotated), row row_idx and col col_idx data is data_area[ row_idx ][ col_idx ]*/
                               /*when rotated_flag = 1(rotated)    , row row_idx and col col_idx data is data_area[ col_idx ][ row_idx ]*/

     void *      blocks;       /*list of MATRIX_BLOCKs*/
}MATRIX;

typedef struct
{
     LIST_NODE    node;                      /*the hang point of this node in vector block*/
     UINT32       num;                       /*the num of data item in vector block*/
     UINT32       data_area[ MATRIX_VECTOR_WIDTH ]; /*data address references*/
}VECTOR_BLOCK;

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

/*WARNING: depend on actual hardware & software environment*/
typedef struct
{
    unsigned long m0:32;
    unsigned long m1:20;
    unsigned long p :11;
    unsigned long s : 1;
}REAL_FORMAT;

#define LOGD_FILE_RECORD_LIMIT_ENABLED  ((UINT32) 1) /*record limit set and enabled*/
#define LOGD_FILE_RECORD_LIMIT_DISABLED ((UINT32) 2) /*record unlimit*/

#define LOGD_FILE_MAX_RECORDS_LIMIT     ((UINT32)-1)

#define LOGD_SWITCH_OFF_ENABLE          ((UINT32) 1) /*log device can be switch off*/
#define LOGD_SWITCH_OFF_DISABLE         ((UINT32) 2) /*log device can NOT be switch off*/

#define LOGD_PID_INFO_ENABLE            ((UINT32) 1) /*log device record pid info*/
#define LOGD_PID_INFO_DISABLE           ((UINT32) 2) /*log device NOT record pid info*/

typedef struct _LOG
{
    UINT32 type;              /*type of log device*/
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

            pthread_mutex_t *mutex;  /*lock for freopen*/

            UINT32   record_limit_enabled; /*record limit enabled or disabled*/
            UINT32   max_records;          /*record limit*/
            UINT32   cur_records;          /*current records*/

            UINT32   tcid;
            UINT32   rank;
        }file;

        CSTRING *cstring;
    }logd;/*log device*/
}LOG;

typedef time_t CTIMET;  /*32 bits for 32bit OS, 64 bits for 64bit OS*/
//typedef time_t ctime_t; /*32 bits for 32bit OS, 64 bits for 64bit OS*/
typedef struct tm CTM;
typedef struct timeval CTMV;

extern long int lrint(double x);

/*---------------------------------------- interface of ctimet ----------------------------------------------*/
//#define CTIMET_GET(ctimet)                                 (c_time(&(ctimet)))
#define CTIMET_GET(ctimet)                                 do{(ctimet) = task_brd_default_get_time();}while(0)
#define CTIMET_DIFF(ctimet_start, ctimet_end)              (difftime(ctimet_end, ctimet_start))
//#define CTIMET_DIFF(ctimet_start, ctimet_end)              ((double)((ctimet_end) - (ctimet_start)))
#define CTIMEOFDAY_GET(ctimev)                             do{gettimeofday(&(ctimev), NULL_PTR);}while(0)

#define CTIMET_TO_LOCAL_TIME(ctimet) (c_localtime_r(&(ctimet)))
#define CTIMET_TO_TM(ctimet)         (CTIMET_TO_LOCAL_TIME(ctimet))

#define CTM_YEAR(ctm)          (((ctm)->tm_year) + 1900)
#define CTM_MONTH(ctm)         (((ctm)->tm_mon) + 1)
#define CTM_MDAY(ctm)          (((ctm)->tm_mday))
#define CTM_HOUR(ctm)          (((ctm)->tm_hour))
#define CTM_MIN(ctm)           (((ctm)->tm_min))
#define CTM_SEC(ctm)           (((ctm)->tm_sec))

#define CTMV_INIT(tmv)         do{(tmv)->tv_sec = 0; (tmv)->tv_usec = 0;}while(0)
#define CTMV_CLEAN(tmv)        do{(tmv)->tv_sec = 0; (tmv)->tv_usec = 0;}while(0)
#define CTMV_CLONE(src, des)   do{(des)->tv_sec = (src)->tv_sec; (des)->tv_usec = (src)->tv_usec;}while(0)

#define CTMV_NSEC(tmv)         ((tmv)->tv_sec)
#define CTMV_MSEC(tmv)         ((tmv)->tv_usec / 1000)

#define BCOPY(src, des, len)           memcpy(des, src, len)
#define BMOVE(src, des, len)           memmove(des, src, len)
#define BSET(pstr, ch, len)            memset(pstr, ch, len)
#define BCMP(pstr1, pstr2, len)        memcmp(pstr1, pstr2, len)
#define STRCMP(pstr1, pstr2)           strcmp(pstr1, pstr2)
#define STRCASECMP(pstr1, pstr2)       strcasecmp(pstr1, pstr2)
#define STRNCMP(pstr1,pstr2, len)      strncmp(pstr1, pstr2, len)
#define STRNCASECMP(pstr1,pstr2, len)  strncasecmp(pstr1, pstr2, len)

#define DMIN(a, b)      ((a) <= (b) ? (a) : (b))
#define DMAX(a, b)      ((a) <= (b) ? (b) : (a))

/*feed src to des and take back des by src at last*/
#define XCHG(type, des, src)  do{type __t__; (__t__) = (des); (des)=(src); (src)=(__t__);}while(0)

/*note, sizeof trap here ....*/
//#define CONST_STR_LEN(str)     ((str) ? (sizeof(str) - 1) : 0)
#define CONST_STR_LEN(str)     ((str) ? strlen(str) : 0)

#define CONST_STR_AND_LEN(str) (str), CONST_STR_LEN(str)

#define CONST_UINT8_STR_AND_LEN(str) ((uint8_t *)(str)), CONST_STR_LEN((char *)str)

#define CONST_UINT8_STR_AND_UINT32T_LEN(str) ((uint8_t *)(str)), (uint32_t)CONST_STR_LEN((char *)str)


#define BASE_ENTRY(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))


#define ISSPACE(c) (( ' ' == (c) || '\n' == (c) || '\t' == (c) || '\f' == (c) || '\v' == (c) || '\r' == (c)))

//#define DEBUG(x) x
#define DEBUG(x) do{}while(0)

#include "loc_macro.inc"
#include "sec_macro.inc"

#endif /*_TYPE_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

