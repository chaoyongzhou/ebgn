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

#ifndef _CRFSNGX_INC
#define _CRFSNGX_INC

#include "type.h"
#include "csocket.inc"

#define CRFSNGX_KEY_MAX_LEN       ((uint32_t) 4 * 1024) /*4KB*/
#define CRFSNGX_CSOCKET_BUFF_LEN  ((uint32_t) 4 * 1024) /*4KB*/

#define CRFSNGX_GET_TAG           ((uint32_t) 1)
#define CRFSNGX_SET_TAG           ((uint32_t) 2)
#define CRFSNGX_DEL_TAG           ((uint32_t) 3)
#define CRFSNGX_SELECT_TAG        ((uint32_t) 4)
#define CRFSNGX_UNDEF_TAG         ((uint32_t)-1)

#if 1
#define CRFSNGX_END_STR           ((const char *)"\r\n")
#define CRFSNGX_SUCC_STATUS_STR   ((const char *)"SUCC\r\n")
#define CRFSNGX_FAIL_STATUS_STR   ((const char *)"FAIL\r\n")

#define CRFSNGX_END_SIZE          ((uint32_t)2)
#define CRFSNGX_SUCC_STATUS_SIZE  ((uint32_t)6)
#define CRFSNGX_FAIL_STATUS_SIZE  ((uint32_t)6)

#define CRFSNGX_GET_OP_STR        ((const char *)"GET\r\n")
#define CRFSNGX_SET_OP_STR        ((const char *)"SET\r\n")
#define CRFSNGX_DEL_OP_STR        ((const char *)"DEL\r\n")
#define CRFSNGX_SELECT_OP_STR     ((const char *)"SELECT\r\n")


#define CRFSNGX_GET_OP_SIZE       ((uint32_t)5)
#define CRFSNGX_SET_OP_SIZE       ((uint32_t)5)
#define CRFSNGX_DEL_OP_SIZE       ((uint32_t)5)
#define CRFSNGX_SELECT_OP_SIZE    ((uint32_t)8)

#define CRFSNGX_FETCH_OP_SIZE     ((uint32_t)8)
#define CRFSNGX_MAX_OP_SIZE       ((uint32_t)8)
#define CRFSNGX_MIN_OP_SIZE       ((uint32_t)5)

#else
#define CRFSNGX_END_STR           ((const char *)"\n")
#define CRFSNGX_SUCC_STATUS_STR   ((const char *)"SUCC\n")
#define CRFSNGX_FAIL_STATUS_STR   ((const char *)"FAIL\n")

#define CRFSNGX_END_SIZE          ((uint32_t)1)
#define CRFSNGX_SUCC_STATUS_SIZE  ((uint32_t)5)
#define CRFSNGX_FAIL_STATUS_SIZE  ((uint32_t)5)

#define CRFSNGX_GET_OP_STR        ((const char *)"GET\n")
#define CRFSNGX_SET_OP_STR        ((const char *)"SET\n")
#define CRFSNGX_DEL_OP_STR        ((const char *)"DEL\n")
#define CRFSNGX_SELECT_OP_STR     ((const char *)"SELECT\n")

#define CRFSNGX_GET_OP_SIZE       ((uint32_t)4)
#define CRFSNGX_SET_OP_SIZE       ((uint32_t)4)
#define CRFSNGX_DEL_OP_SIZE       ((uint32_t)4)
#define CRFSNGX_SELECT_OP_SIZE    ((uint32_t)7)

#define CRFSNGX_FETCH_OP_SIZE     ((uint32_t)7)
#define CRFSNGX_MAX_OP_SIZE       ((uint32_t)7)
#define CRFSNGX_MIN_OP_SIZE       ((uint32_t)4)
#endif


typedef struct
{
    uint32_t     crfsngx_op_len;  /*ngx op str len  */
    uint32_t     crfsngx_op_tag;  /*ngx op tag val  */  
    uint32_t     crfsngx_key_len; /*ngx key str len */
    uint32_t     crfsngx_val_len; /*ngx val str len */
    uint32_t     crfsngx_vlen;    /*ngx val byte len*/
    uint8_t      pad[ CSOCKET_CNODE_PKT_HDR_SIZE -  4 * 5];
}CRFSNGX_PKT_HDR;

#define CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr)          ((crfsngx_pkt_hdr)->crfsngx_op_len)
#define CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr)          ((crfsngx_pkt_hdr)->crfsngx_op_tag)
#define CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr)         ((crfsngx_pkt_hdr)->crfsngx_key_len)
#define CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr)         ((crfsngx_pkt_hdr)->crfsngx_val_len)
#define CRFSNGX_PKT_HDR_VLEN(crfsngx_pkt_hdr)            ((crfsngx_pkt_hdr)->crfsngx_vlen)

typedef struct
{
    const uint8_t   *op_str;
    uint32_t         op_size;
    uint32_t         op_tag;
}CRFSNGX_OP_DEF;

#endif /*_CRFSNGX_INC*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

