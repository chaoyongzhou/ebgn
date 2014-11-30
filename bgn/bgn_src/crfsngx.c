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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>

#include <sys/stat.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "carray.h"
#include "cvector.h"

#include "cbc.h"

#include "cmisc.h"

#include "task.h"

#include "csocket.h"

#include "cmpie.h"

#include "crfs.h"
#include "crfsngx.h"
#include "cload.h"

#include "cfuse.h"

#include "findex.inc"

/**
protocol
=========
    (case sensitive)

    0. STATUS string = {SUCC, FAIL}

    1. GET format: 
    request: GET\r\n | key (path, EOL = "\r\n")

    reply: STATUS\r\n[| bin body len (dec string) "\r\n" | <bin body>]

    2.SET format:
    request: SET\r\n | key (path, EOL = "\r\n") | bin body len (dec string) "\r\n" | <bin body>

    reply: STATUS\r\n

    3.DEL format:
    request DEL\r\n | key (path, EOL = "\r\n")

    reply: STATUS\r\n

    4.SELECT format:
    request SELECT\r\n | key (path, EOL = "\r\n")

    reply: STATUS\r\n
**/

#if 0
#define CRFSNGX_PRINT_UINT8(info, buff, len) do{\
    uint32_t __pos;\
    dbg_log(SEC_0036_CRFSNGX, 5)(LOGSTDOUT, "%s: ", info);\
    for(__pos = 0; __pos < len; __pos ++)\
    {\
        sys_print(LOGSTDOUT, "%02x,", ((uint8_t *)buff)[ __pos ]);\
    }\
    sys_print(LOGSTDOUT, "\n");\
}while(0)

#define CRFSNGX_PRINT_CHARS(info, buff, len) do{\
    uint32_t __pos;\
    dbg_log(SEC_0036_CRFSNGX, 5)(LOGSTDOUT, "%s: ", info);\
    for(__pos = 0; __pos < len; __pos ++)\
    {\
        sys_print(LOGSTDOUT, "%c", ((uint8_t *)buff)[ __pos ]);\
    }\
    sys_print(LOGSTDOUT, "\n");\
}while(0)
#else
#define CRFSNGX_PRINT_UINT8(info, buff, len) do{}while(0)
#define CRFSNGX_PRINT_CHARS(info, buff, len) do{}while(0)
#endif

static const CRFSNGX_OP_DEF g_crfsngx_op_tab[ ] = {
    {CRFSNGX_GET_OP_STR   ,  CRFSNGX_GET_OP_SIZE   , CRFSNGX_GET_TAG   },
    {CRFSNGX_SET_OP_STR   ,  CRFSNGX_SET_OP_SIZE   , CRFSNGX_SET_TAG   },
    {CRFSNGX_DEL_OP_STR   ,  CRFSNGX_DEL_OP_SIZE   , CRFSNGX_DEL_TAG   },
    {CRFSNGX_SELECT_OP_STR,  CRFSNGX_SELECT_OP_SIZE, CRFSNGX_SELECT_TAG},
};
static const uint32_t g_crfsngx_op_tab_len = sizeof(g_crfsngx_op_tab)/sizeof(g_crfsngx_op_tab[0]);

EC_BOOL crfsngx_csocket_check_str_end(const uint8_t *buff, const uint32_t size, uint32_t *str_len)
{
    uint32_t  pos;

    for(pos = 0; pos + 1 < size; pos ++)
    {
        if('\r' != buff[ pos ])
        {
            continue;
        }

        if('\n' == buff[ pos + 1 ])
        {
            (*str_len) = pos; /*end_pos pointer to "\r\n"*/
            return (EC_TRUE);
        }        
    }

    return (EC_FALSE);
}

/**
    1. GET format: 
    request: GET\r\n | key (path, EOL = "\r\n")
**/
EC_BOOL crfsngx_csocket_fetch_ngx_get_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag)
{
    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);

    if(EC_FALSE == crfsngx_csocket_fetch_ngx_key(csocket_cnode, task_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_get_request: fetch ngx key on sockfd %d failed\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }    

    if(0 == CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr)) /*key is not ready*/
    {
        return (EC_TRUE);
    }  
    
    (*done_flag) = EC_TRUE;
    return (EC_TRUE);
}

/**
    2.SET format:
    request: SET\r\n | key (path, EOL = "\r\n") | bin body len (dec string) "\r\n" | <bin body>
**/

EC_BOOL crfsngx_csocket_fetch_ngx_set_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag)
{
    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;
    uint32_t offset;
    uint32_t end_pos;    

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);

    if(EC_FALSE == crfsngx_csocket_fetch_ngx_key(csocket_cnode, task_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_set_request: fetch ngx key on sockfd %d failed\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }    

    if(0 == CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr)) /*key is not ready*/
    {
        return (EC_TRUE);
    }  

    if(EC_FALSE == crfsngx_csocket_fetch_ngx_vlen(csocket_cnode, task_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_set_request: fetch ngx vlen on sockfd %d failed\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }    

    if(0 == CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr)) /*vlen is not ready*/
    {
        return (EC_TRUE);
    }    

    if(EC_FALSE == crfsngx_csocket_fetch_ngx_val(csocket_cnode, task_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_set_request: fetch ngx val on sockfd %d failed\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    } 
        
    offset = CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr)  + CRFSNGX_END_SIZE /*op end*/
           + CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE /*key end*/
           + CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE /*val end*/;

    end_pos = offset + CRFSNGX_PKT_HDR_VLEN(crfsngx_pkt_hdr);           
    if(TASK_NODE_BUFF_POS(task_node) < end_pos) /*val is not ready*/
    {
        return (EC_TRUE);
    }

    (*done_flag) = EC_TRUE;
    return (EC_TRUE);
}

/**
    3.DEL format:
    request DEL\r\n | key (path, EOL = "\r\n")
**/
EC_BOOL crfsngx_csocket_fetch_ngx_del_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag)
{
    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);

    if(EC_FALSE == crfsngx_csocket_fetch_ngx_key(csocket_cnode, task_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_del_request: fetch ngx key on sockfd %d failed\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }    

    if(0 == CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr)) /*key is not ready*/
    {
        return (EC_TRUE);
    }  
    
    (*done_flag) = EC_TRUE;
    return (EC_TRUE);
}

/**
    4.SELECT format:
    request SELECT\r\n | key (path, EOL = "\r\n")
**/
EC_BOOL crfsngx_csocket_fetch_ngx_select_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag)
{
    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);

    if(EC_FALSE == crfsngx_csocket_fetch_ngx_key(csocket_cnode, task_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_select_request: fetch ngx key on sockfd %d failed\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }    

    if(0 == CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr)) /*key is not ready*/
    {
        return (EC_TRUE);
    }  
    
    (*done_flag) = EC_TRUE;
    return (EC_TRUE);
}

static uint32_t __crfsngx_csocket_fetch_ngx_op(const char *buff, const uint32_t len)
{
    uint32_t crfsngx_op_idx;

    CRFSNGX_PRINT_CHARS("[DEBUG] __crfsngx_csocket_fetch_ngx_op:CHARS:", buff, len);
    CRFSNGX_PRINT_UINT8("[DEBUG] __crfsngx_csocket_fetch_ngx_op:UINT8:", buff, len);

    for(crfsngx_op_idx = 0; crfsngx_op_idx < g_crfsngx_op_tab_len; crfsngx_op_idx ++)
    {
        const CRFSNGX_OP_DEF *crfsngx_op_def;

        crfsngx_op_def = &(g_crfsngx_op_tab[ crfsngx_op_idx ]);

        if(len >= crfsngx_op_def->op_size
        && 0 == BCMP(buff, crfsngx_op_def->op_str, crfsngx_op_def->op_size)
        )
        {
            return (crfsngx_op_def->op_tag);
        }
    }

    return (CRFSNGX_UNDEF_TAG);
}

EC_BOOL crfsngx_csocket_fetch_ngx_op(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node)
{
    uint32_t    offset;
    uint32_t    op_str_len; 
    uint32_t    op_tag;

    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);
    if(0 < CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr)) /*op is ready*/
    {
        return (EC_TRUE);
    }

    /*when buff room is not available, double it*/
    if(TASK_NODE_BUFF_POS(task_node) >= TASK_NODE_BUFF_LEN(task_node))
    {
        UINT32 new_size;

        new_size = 2 * TASK_NODE_BUFF_LEN(task_node);
        if(EC_FALSE == task_node_expand_to(task_node, new_size))
        {
            dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_op: task_node %p expand to len %ld failed\n",
                               task_node, new_size);
            CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);                               
            return (EC_FALSE);
        }
    }

    if(EC_FALSE == csocket_read(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                  TASK_NODE_BUFF(task_node),
                                  TASK_NODE_BUFF_LEN(task_node), 
                                  &(TASK_NODE_BUFF_POS(task_node)))
      )
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_op: csocket irecv failed on socket %d where pos %ld\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode), TASK_NODE_BUFF_POS(task_node));
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);
        return (EC_FALSE);
    }

    offset = 0;

    if(offset + CRFSNGX_END_SIZE/*op end*/ >= TASK_NODE_BUFF_POS(task_node))
    {
        /*obviously, buff length is not enough to fetch ngx op*/
        return (EC_TRUE);
    }
    
    if(EC_FALSE == crfsngx_csocket_check_str_end(TASK_NODE_BUFF(task_node) + offset, 
                                                    TASK_NODE_BUFF_POS(task_node) - offset, 
                                                    &op_str_len))
    {
        /*okay, ngx op is incompleted yet*/
        return (EC_TRUE);
    }

    op_tag = __crfsngx_csocket_fetch_ngx_op(TASK_NODE_BUFF(task_node) + offset, op_str_len + CRFSNGX_END_SIZE);
    if(CRFSNGX_UNDEF_TAG == op_tag)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_op: invalid crfsngx tag %.*s\n",
                            op_str_len, TASK_NODE_BUFF(task_node) + offset);
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);                            
        return (EC_FALSE);                            
    }

    /*now, ngx op is ok*/
    CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr) = op_str_len;
    CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr) = op_tag;

    dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_csocket_fetch_ngx_op: op len %d, op tag %d, op str: [%.*s]\n",
                        CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr), CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr),
                        CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr), TASK_NODE_BUFF(task_node) + offset);

    return (EC_TRUE);
}

EC_BOOL crfsngx_csocket_fetch_ngx_key(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node)
{
    uint32_t    offset;
    uint32_t    key_str_len; 

    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);
    if(0 < CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr)) /*key is ready*/
    {
        return (EC_TRUE);
    }

    /*when buff room is not available, double it*/
    if(TASK_NODE_BUFF_POS(task_node) >= TASK_NODE_BUFF_LEN(task_node))
    {
        UINT32 new_size;

        new_size = 2 * TASK_NODE_BUFF_LEN(task_node);
        if(EC_FALSE == task_node_expand_to(task_node, new_size))
        {
            dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_key: task_node %p expand to len %ld failed\n",
                               task_node, new_size);
            CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);                               
            return (EC_FALSE);
        }
    }

    if(EC_FALSE == csocket_read(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                  TASK_NODE_BUFF(task_node),
                                  TASK_NODE_BUFF_LEN(task_node), 
                                  &(TASK_NODE_BUFF_POS(task_node)))
      )
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_key: csocket irecv failed on socket %d where pos %ld\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode), TASK_NODE_BUFF_POS(task_node));
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);                            
        return (EC_FALSE);
    }

    offset = CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE /*op end*/;

    if(offset + CRFSNGX_END_SIZE /*key end*/ >= TASK_NODE_BUFF_POS(task_node))
    {
        /*obviously, buff length is not enough to fetch ngx key*/
        return (EC_TRUE);
    }

    CRFSNGX_PRINT_UINT8("[DEBUG] crfsngx_csocket_fetch_ngx_key:UINT8", TASK_NODE_BUFF(task_node) + offset, TASK_NODE_BUFF_POS(task_node) - offset);
    CRFSNGX_PRINT_CHARS("[DEBUG] crfsngx_csocket_fetch_ngx_key:CHARS", TASK_NODE_BUFF(task_node) + offset, TASK_NODE_BUFF_POS(task_node) - offset);
    
    if(EC_FALSE == crfsngx_csocket_check_str_end(TASK_NODE_BUFF(task_node) + offset, 
                                                    TASK_NODE_BUFF_POS(task_node) - offset, 
                                                    &key_str_len))
    {
        /*okay, ngx key is incompleted yet*/
        return (EC_TRUE);
    }
    dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_csocket_fetch_ngx_key: offset %d, buff pos %ld, key_str_len %d\n",
                        offset, TASK_NODE_BUFF_POS(task_node), key_str_len);

    /*now, ngx key is ok*/
    CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr) = key_str_len;

    dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_csocket_fetch_ngx_key: key_len %d, key: [%.*s]\n",
                        CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr), 
                        CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr), TASK_NODE_BUFF(task_node) + offset);

    return (EC_TRUE);
}

EC_BOOL crfsngx_csocket_fetch_ngx_vlen(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node)
{
    uint32_t    offset;
    uint32_t    vlen_str_len; 
    uint32_t    vlen;

    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);
    if(0 < CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr)) /*vlen is ready*/
    {
        return (EC_TRUE);
    }

    /*when buff room is not available, double it*/
    if(TASK_NODE_BUFF_POS(task_node) >= TASK_NODE_BUFF_LEN(task_node))
    {
        UINT32 new_size;

        new_size = 2 * TASK_NODE_BUFF_LEN(task_node);
        if(EC_FALSE == task_node_expand_to(task_node, new_size))
        {
            dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_vlen: task_node %p expand to len %ld failed\n",
                               task_node, new_size);
            CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);                               
            return (EC_FALSE);
        }
    }

    if(EC_FALSE == csocket_read(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                  TASK_NODE_BUFF(task_node),
                                  TASK_NODE_BUFF_LEN(task_node), 
                                  &(TASK_NODE_BUFF_POS(task_node)))
      )
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_vlen: csocket irecv failed on socket %d where pos %ld\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode), TASK_NODE_BUFF_POS(task_node));
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);                            
        return (EC_FALSE);
    }

    offset = CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr)  + CRFSNGX_END_SIZE /*op end*/
           + CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE /*key end*/;

    if(offset + CRFSNGX_END_SIZE /*vlen end*/ >= TASK_NODE_BUFF_POS(task_node))
    {
        /*obviously, buff length is not enough to fetch ngx vlen*/
        return (EC_TRUE);
    }
    
    if(EC_FALSE == crfsngx_csocket_check_str_end(TASK_NODE_BUFF(task_node) + offset, 
                                                    TASK_NODE_BUFF_POS(task_node) - offset, 
                                                    &vlen_str_len))
    {
        /*okay, ngx vlen is incompleted yet*/
        return (EC_TRUE);
    }

    /*now, ngx vlen string is available*/
    vlen = c_chars_to_uint32(TASK_NODE_BUFF(task_node) + offset, vlen_str_len);
    if(0 == vlen)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_vlen:invalid vlen str %.*s\n", 
                            vlen_str_len, TASK_NODE_BUFF(task_node) + offset);
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);                            
        return (EC_FALSE);                            
    }

    /*now, ngx vlen is ok*/
    
    CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr) = vlen_str_len;
    CRFSNGX_PKT_HDR_VLEN(crfsngx_pkt_hdr)    = vlen;

    dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_csocket_fetch_ngx_vlen: val len %d, vlen %d, vlen str: [%.*s]\n",
                        CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr), CRFSNGX_PKT_HDR_VLEN(crfsngx_pkt_hdr),
                        CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr), TASK_NODE_BUFF(task_node) + offset);

    return (EC_TRUE);
}

EC_BOOL crfsngx_csocket_fetch_ngx_val(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node)
{
    uint32_t    offset;
    uint32_t    end_pos; 

    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);

    offset = CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr)  + CRFSNGX_END_SIZE /*op end*/
           + CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE /*key end*/
           + CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE /*val end*/;

    end_pos = offset + CRFSNGX_PKT_HDR_VLEN(crfsngx_pkt_hdr);           
    if(TASK_NODE_BUFF_POS(task_node) >= end_pos) /*val is ready*/
    {
        dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_csocket_fetch_ngx_val: "
                           "val is ready where op len %d, key len %d "
                           "val_len %d, vlen %d, offset %d, buff_pos %d\n",
                           CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr), CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr),
                           CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr), CRFSNGX_PKT_HDR_VLEN(crfsngx_pkt_hdr),
                           offset, TASK_NODE_BUFF_POS(task_node));
        return (EC_TRUE);
    }

    /*when buff room is not available, expand it*/
    if(end_pos >= TASK_NODE_BUFF_LEN(task_node))
    {
        UINT32 new_size;

        new_size = end_pos;
        if(EC_FALSE == task_node_expand_to(task_node, new_size))
        {
            dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_val: task_node %p expand to len %ld failed\n",
                               task_node, new_size);
            CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);                               
            return (EC_FALSE);
        }
    }

    if(EC_FALSE == csocket_read(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                  TASK_NODE_BUFF(task_node),
                                  TASK_NODE_BUFF_LEN(task_node), 
                                  &(TASK_NODE_BUFF_POS(task_node)))
      )
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx_val: csocket irecv failed on socket %d where pos %ld\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode), TASK_NODE_BUFF_POS(task_node));
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);                            
        return (EC_FALSE);
    }

    if(TASK_NODE_BUFF_POS(task_node) < end_pos)
    {
        /*ngx val is incompleted yet*/
        return (EC_TRUE);
    }

    dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_csocket_fetch_ngx_val: val len %d, val str: [%.*s]\n",
                        TASK_NODE_BUFF_POS(task_node) - offset, 
                        TASK_NODE_BUFF_POS(task_node) - offset, TASK_NODE_BUFF(task_node) + offset);

    /*now, ngx val is ok*/
    return (EC_TRUE);
}

EC_BOOL crfsngx_csocket_fetch_ngx(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag)
{
    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;
    uint32_t    op_tag;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);

    if(EC_FALSE == crfsngx_csocket_fetch_ngx_op(csocket_cnode, task_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx: fetch ngx op on sockfd %d failed\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    if(0 == CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr)) /*op is not ready*/
    {
        return (EC_TRUE);
    }

    op_tag = CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr);
    if(CRFSNGX_GET_TAG == op_tag)
    {
        return crfsngx_csocket_fetch_ngx_get_request(csocket_cnode, task_node, done_flag);
    }

    if(CRFSNGX_SET_TAG == op_tag)
    {
        return crfsngx_csocket_fetch_ngx_set_request(csocket_cnode, task_node, done_flag);
    } 

    if(CRFSNGX_DEL_TAG == op_tag)
    {
        return crfsngx_csocket_fetch_ngx_del_request(csocket_cnode, task_node, done_flag);
    }   

    if(CRFSNGX_SELECT_TAG == op_tag)
    {
        return crfsngx_csocket_fetch_ngx_select_request(csocket_cnode, task_node, done_flag);
    }      

    /*otherwise, op is invalid*/    
    dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_csocket_fetch_ngx:invalid op %d\n", op_tag);
    CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);
    
    return (EC_FALSE);
}

EC_BOOL crfsngx_handle_ngx_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);    

    /*handle request*/
    task_caller(task_req_func, func_addr_node);

    /*commit reply*/
    if(EC_FALSE == crfsngx_commit_ngx_reply(csocket_cnode, task_req_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_handle_ngx_request: commit ngx reply failed\n");
        return (EC_FALSE);
    }

    /*send reply*/
    cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                      (CSRV_WRITE_HANDLER)crfsngx_send_on_csocket_cnode, csocket_cnode);
    return (EC_TRUE);
}

/*crfsngx_make_xxx: to make memory of parameters ready*/
EC_BOOL crfsngx_make_ngx_get_request(TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;
    
    CSTRING *file_path;
    CBYTES  *file_bytes;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);    

    file_path = cstring_new(NULL_PTR, LOC_CRFSNGX_0001);
    if(NULL_PTR == file_path)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_make_ngx_get_request: new cstring failed\n");
        return (EC_FALSE);
    }

    file_bytes = cbytes_new(0);
    if(NULL_PTR == file_bytes)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_make_ngx_get_request: new cbytes failed\n");
        cstring_free(file_path);
        return (EC_FALSE);
    }
    
    TASK_REQ_TIME_TO_LIVE(task_req) = TASK_DEFAULT_LIVE;

    TASK_REQ_FUNC_ID(task_req)       = func_addr_node->func_index;
    TASK_REQ_FUNC_PARA_NUM(task_req) = func_addr_node->func_para_num;
    TASK_REQ_NEED_RSP_FLAG(task_req) = TASK_NEED_RSP_FLAG;

    TASK_REQ_FUNC_ADDR_NODE(task_req) = func_addr_node;/*mount func_addr_node to task req*/

    task_req_func->func_ret_val   = EC_FALSE; /*init ret value*/
    
    task_req_func->func_para[ 0 ].para_dir = func_addr_node->func_para_direction[ 0 ];
    task_req_func->func_para[ 0 ].para_val = 0; /*CRFS module id*/

    task_req_func->func_para[ 1 ].para_dir = func_addr_node->func_para_direction[ 1 ];
    task_req_func->func_para[ 1 ].para_val = (UINT32)file_path;

    task_req_func->func_para[ 2 ].para_dir = func_addr_node->func_para_direction[ 2 ];
    task_req_func->func_para[ 2 ].para_val = (UINT32)file_bytes;

    return (EC_TRUE);
}

/*crfsngx_make_xxx: to make memory of parameters ready*/
EC_BOOL crfsngx_make_ngx_set_request(TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;
    
    CSTRING *file_path;
    CBYTES  *file_bytes;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);    

    file_path = cstring_new(NULL_PTR, LOC_CRFSNGX_0002);
    if(NULL_PTR == file_path)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_make_ngx_set_request: new cstring failed\n");
        return (EC_FALSE);
    }

    file_bytes = cbytes_new(0);
    if(NULL_PTR == file_bytes)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_make_ngx_set_request: new cbytes failed\n");
        cstring_free(file_path);
        return (EC_FALSE);
    }
    
    TASK_REQ_TIME_TO_LIVE(task_req) = TASK_DEFAULT_LIVE;

    TASK_REQ_FUNC_ID(task_req)       = func_addr_node->func_index;
    TASK_REQ_FUNC_PARA_NUM(task_req) = func_addr_node->func_para_num;
    TASK_REQ_NEED_RSP_FLAG(task_req) = TASK_NEED_RSP_FLAG;

    TASK_REQ_FUNC_ADDR_NODE(task_req) = func_addr_node;/*mount func_addr_node to task req*/

    task_req_func->func_ret_val   = EC_FALSE; /*init ret value*/
    
    task_req_func->func_para[ 0 ].para_dir = func_addr_node->func_para_direction[ 0 ];
    task_req_func->func_para[ 0 ].para_val = 0; /*CRFS module id*/

    task_req_func->func_para[ 1 ].para_dir = func_addr_node->func_para_direction[ 1 ];
    task_req_func->func_para[ 1 ].para_val = (UINT32)file_path;

    task_req_func->func_para[ 2 ].para_dir = func_addr_node->func_para_direction[ 2 ];
    task_req_func->func_para[ 2 ].para_val = (UINT32)file_bytes;

    return (EC_TRUE);
}

/*crfsngx_make_xxx: to make memory of parameters ready*/
EC_BOOL crfsngx_make_ngx_del_request(TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;
    
    CSTRING *file_path;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);    

    file_path = cstring_new(NULL_PTR, LOC_CRFSNGX_0003);
    if(NULL_PTR == file_path)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_make_ngx_del_request: new cstring failed\n");
        return (EC_FALSE);
    }
    
    TASK_REQ_TIME_TO_LIVE(task_req) = TASK_DEFAULT_LIVE;

    TASK_REQ_FUNC_ID(task_req)       = func_addr_node->func_index;
    TASK_REQ_FUNC_PARA_NUM(task_req) = func_addr_node->func_para_num;
    TASK_REQ_NEED_RSP_FLAG(task_req) = TASK_NEED_RSP_FLAG;

    TASK_REQ_FUNC_ADDR_NODE(task_req) = func_addr_node;/*mount func_addr_node to task req*/

    task_req_func->func_ret_val   = EC_FALSE; /*init ret value*/
    
    task_req_func->func_para[ 0 ].para_dir = func_addr_node->func_para_direction[ 0 ];
    task_req_func->func_para[ 0 ].para_val = 0; /*CRFS module id*/

    task_req_func->func_para[ 1 ].para_dir = func_addr_node->func_para_direction[ 1 ];
    task_req_func->func_para[ 1 ].para_val = (UINT32)file_path;

    task_req_func->func_para[ 2 ].para_dir = func_addr_node->func_para_direction[ 2 ];
    task_req_func->func_para[ 2 ].para_val = CRFSNP_ITEM_FILE_IS_ANY;

    return (EC_TRUE);
}

/*crfsngx_make_xxx: to make memory of parameters ready*/
EC_BOOL crfsngx_make_ngx_select_request(TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;
    
    CSTRING *file_path;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);    

    file_path = cstring_new(NULL_PTR, LOC_CRFSNGX_0004);
    if(NULL_PTR == file_path)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_make_ngx_select_request: new cstring failed\n");
        return (EC_FALSE);
    }
    
    TASK_REQ_TIME_TO_LIVE(task_req) = TASK_DEFAULT_LIVE;

    TASK_REQ_FUNC_ID(task_req)       = func_addr_node->func_index;
    TASK_REQ_FUNC_PARA_NUM(task_req) = func_addr_node->func_para_num;
    TASK_REQ_NEED_RSP_FLAG(task_req) = TASK_NEED_RSP_FLAG;

    TASK_REQ_FUNC_ADDR_NODE(task_req) = func_addr_node;/*mount func_addr_node to task req*/

    task_req_func->func_ret_val   = EC_FALSE; /*init ret value*/
    
    task_req_func->func_para[ 0 ].para_dir = func_addr_node->func_para_direction[ 0 ];
    task_req_func->func_para[ 0 ].para_val = 0; /*CRFS module id*/

    task_req_func->func_para[ 1 ].para_dir = func_addr_node->func_para_direction[ 1 ];
    task_req_func->func_para[ 1 ].para_val = (UINT32)file_path;

    return (EC_TRUE);
}


EC_BOOL crfsngx_clean_ngx_get_request(TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;
    
    CSTRING *file_path;
    CBYTES  *file_bytes;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);    

    file_path  = (CSTRING *)task_req_func->func_para[ 1 ].para_val;
    file_bytes = (CBYTES  *)task_req_func->func_para[ 2 ].para_val;

    cstring_free(file_path);
    cbytes_free(file_bytes, LOC_CRFSNGX_0005);

    task_req_func->func_para[ 1 ].para_val = 0;
    task_req_func->func_para[ 2 ].para_val = 0;

    return (EC_TRUE);
}

EC_BOOL crfsngx_clean_ngx_set_request(TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;
    
    CSTRING *file_path;
    CBYTES  *file_bytes;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);    

    file_path  = (CSTRING *)task_req_func->func_para[ 1 ].para_val;
    file_bytes = (CBYTES  *)task_req_func->func_para[ 2 ].para_val;

    cstring_free(file_path);
    cbytes_free(file_bytes, LOC_CRFSNGX_0006);

    task_req_func->func_para[ 1 ].para_val = 0;
    task_req_func->func_para[ 2 ].para_val = 0;

    return (EC_TRUE);
}

EC_BOOL crfsngx_clean_ngx_del_request(TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;
    
    CSTRING *file_path;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);    

    file_path  = (CSTRING *)task_req_func->func_para[ 1 ].para_val;

    cstring_free(file_path);

    task_req_func->func_para[ 1 ].para_val = 0;

    return (EC_TRUE);
}

EC_BOOL crfsngx_clean_ngx_select_request(TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;
    
    CSTRING *file_path;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);    

    file_path  = (CSTRING *)task_req_func->func_para[ 1 ].para_val;

    cstring_free(file_path);

    task_req_func->func_para[ 1 ].para_val = 0;

    return (EC_TRUE);
}

EC_BOOL crfsngx_commit_ngx_get_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;

    FUNC_ADDR_NODE *func_addr_node;

    CSTRING  *file_path;

    TASK_BRD *task_brd;
    
    CROUTINE_NODE  *croutine_node;

    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;
    UINT32           offset;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);
    
    task_brd = task_brd_default_get();

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);

    if(0 != dbg_fetch_func_addr_node_by_index(FI_crfs_read, &func_addr_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_get_request: failed to fetch func addr node by func id %lx\n", 
                            FI_crfs_read);
        return (EC_FALSE);
    } 
    
    if(EC_FALSE == crfsngx_make_ngx_get_request(task_req_node, func_addr_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_get_request: make ngx GET request failed\n");
        return (EC_FALSE);
    }

    offset = CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE;
    file_path = (CSTRING *)task_req_func->func_para[ 1 ].para_val;
    cstring_append_chars(file_path, CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr), TASK_NODE_BUFF(task_req_node) + offset);

    task_node_buff_free(task_req_node); /*free buff memory after decoding*/

    croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd), 
                                       (UINT32)crfsngx_handle_ngx_request, 3, csocket_cnode, task_req_node, func_addr_node);
    if(NULL_PTR == croutine_node)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_get_request: cthread load failed\n");
        return (EC_FALSE);
    }
    CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CRFSNGX_0007);


    dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_commit_ngx_get_request: commit done!\n");

    return (EC_TRUE);
}

EC_BOOL crfsngx_commit_ngx_set_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;

    FUNC_ADDR_NODE *func_addr_node;

    CSTRING  *file_path;
    CBYTES   *file_body;

    TASK_BRD *task_brd;
    
    CROUTINE_NODE  *croutine_node;

    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;
    UINT32           offset;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);
    
    task_brd = task_brd_default_get();

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);

    if(0 != dbg_fetch_func_addr_node_by_index(FI_crfs_write, &func_addr_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_set_request: failed to fetch func addr node by func id %lx\n", 
                            FI_crfs_write);
        return (EC_FALSE);
    } 
    
    if(EC_FALSE == crfsngx_make_ngx_set_request(task_req_node, func_addr_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_set_request: make ngx GET request failed\n");
        return (EC_FALSE);
    }

    offset = CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE;
    file_path = (CSTRING *)task_req_func->func_para[ 1 ].para_val;
    cstring_append_chars(file_path, CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr), TASK_NODE_BUFF(task_req_node) + offset);

    offset = CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE
           + CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE
           + CRFSNGX_PKT_HDR_VAL_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE;
    file_body = (CBYTES *)task_req_func->func_para[ 2 ].para_val;
    cbytes_set(file_body, TASK_NODE_BUFF(task_req_node) + offset, CRFSNGX_PKT_HDR_VLEN(crfsngx_pkt_hdr));

    task_node_buff_free(task_req_node); /*free buff memory after decoding*/

    croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd), 
                                       (UINT32)crfsngx_handle_ngx_request, 3, csocket_cnode, task_req_node, func_addr_node);
    if(NULL_PTR == croutine_node)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_set_request: cthread load failed\n");
        return (EC_FALSE);
    }
    CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CRFSNGX_0008);


    dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_commit_ngx_set_request: commit done!\n");

    return (EC_TRUE);
}

EC_BOOL crfsngx_commit_ngx_del_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;

    FUNC_ADDR_NODE *func_addr_node;

    CSTRING  *file_path;

    TASK_BRD *task_brd;
    
    CROUTINE_NODE  *croutine_node;

    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;
    UINT32           offset;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);
    
    task_brd = task_brd_default_get();

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);

    if(0 != dbg_fetch_func_addr_node_by_index(FI_crfs_delete, &func_addr_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_del_request: failed to fetch func addr node by func id %lx\n", 
                            FI_crfs_delete);
        return (EC_FALSE);
    } 
    
    if(EC_FALSE == crfsngx_make_ngx_del_request(task_req_node, func_addr_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_del_request: make ngx GET request failed\n");
        return (EC_FALSE);
    }

    offset = CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE;
    file_path = (CSTRING *)task_req_func->func_para[ 1 ].para_val;
    cstring_append_chars(file_path, CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr), TASK_NODE_BUFF(task_req_node) + offset);

    task_node_buff_free(task_req_node); /*free buff memory after decoding*/

    croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd), 
                                       (UINT32)crfsngx_handle_ngx_request, 3, csocket_cnode, task_req_node, func_addr_node);
    if(NULL_PTR == croutine_node)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_del_request: cthread load failed\n");
        return (EC_FALSE);
    }
    CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CRFSNGX_0009);

    dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_commit_ngx_del_request: commit done!\n");

    return (EC_TRUE);
}

EC_BOOL crfsngx_commit_ngx_select_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;

    FUNC_ADDR_NODE *func_addr_node;

    CSTRING  *file_path;

    TASK_BRD *task_brd;
    
    CROUTINE_NODE  *croutine_node;

    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;
    UINT32           offset;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);
    
    task_brd = task_brd_default_get();

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);

    if(0 != dbg_fetch_func_addr_node_by_index(FI_crfs_exists, &func_addr_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_select_request: failed to fetch func addr node by func id %lx\n", 
                            FI_crfs_exists);
        return (EC_FALSE);
    } 
    
    if(EC_FALSE == crfsngx_make_ngx_select_request(task_req_node, func_addr_node))
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_select_request: make ngx GET request failed\n");
        return (EC_FALSE);
    }

    offset = CRFSNGX_PKT_HDR_OP_LEN(crfsngx_pkt_hdr) + CRFSNGX_END_SIZE;
    file_path = (CSTRING *)task_req_func->func_para[ 1 ].para_val;
    cstring_append_chars(file_path, CRFSNGX_PKT_HDR_KEY_LEN(crfsngx_pkt_hdr), TASK_NODE_BUFF(task_req_node) + offset);

    task_node_buff_free(task_req_node); /*free buff memory after decoding*/

    croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd), 
                                       (UINT32)crfsngx_handle_ngx_request, 3, csocket_cnode, task_req_node, func_addr_node);
    if(NULL_PTR == croutine_node)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_select_request: cthread load failed\n");
        return (EC_FALSE);
    }
    CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CRFSNGX_0010);

    dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDOUT, "[DEBUG] crfsngx_commit_ngx_select_request: commit done!\n");

    return (EC_TRUE);
}

EC_BOOL crfsngx_commit_ngx_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{  
    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);
    
    if(CRFSNGX_GET_TAG == CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr))
    {
        return crfsngx_commit_ngx_get_request(csocket_cnode, task_req_node);
    }

    if(CRFSNGX_SET_TAG == CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr))
    {
        return crfsngx_commit_ngx_set_request(csocket_cnode, task_req_node);
    }    

    if(CRFSNGX_DEL_TAG == CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr))
    {
        return crfsngx_commit_ngx_del_request(csocket_cnode, task_req_node);
    } 

    if(CRFSNGX_SELECT_TAG == CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr))
    {
        return crfsngx_commit_ngx_select_request(csocket_cnode, task_req_node);
    }   
    dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_request:unknown ngx tag %d on sockfd %d\n", 
                        CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr), CSOCKET_CNODE_SOCKFD(csocket_cnode));
    return (EC_FALSE);
}

EC_BOOL crfsngx_commit_ngx_get_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;

    EC_BOOL    ret;
    CSTRING   *file_path;
    CBYTES    *file_bytes;

    TASK_RSP  *task_rsp;
    TASK_NODE *task_rsp_node;
    UINT32     max_len;
    UINT32     send_len;
    UINT32     file_len;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);   

    ret = task_req_func->func_ret_val;
    file_path  = (CSTRING *)task_req_func->func_para[ 1 ].para_val;
    file_bytes = (CBYTES  *)task_req_func->func_para[ 2 ].para_val;

    task_req_func->func_para[ 1 ].para_val = 0;
    task_req_func->func_para[ 2 ].para_val = 0;

    /*reply: STATUS\r\n[| bin body len (dec string) "\r\n" | <bin body>]*/
    
    task_rsp = task_rsp_new(0, LOC_CRFSNGX_0011);
    task_rsp_node = TASK_RSP_NODE(task_rsp);

    TASK_NODE_TAG(task_rsp_node) = TAG_TASK_RSP;
    
    if(EC_FALSE == ret)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_get_reply: get %s failed\n", (char *)cstring_get_str(file_path));

        max_len = CRFSNGX_FAIL_STATUS_SIZE;/*FAIL*/
        task_node_buff_alloc(task_rsp_node, max_len + 1);
        send_len = snprintf(TASK_NODE_BUFF(task_rsp_node), TASK_NODE_BUFF_LEN(task_rsp_node), CRFSNGX_FAIL_STATUS_STR);
        TASK_NODE_BUFF_LEN(task_rsp_node) = send_len;

        crfsngx_clean_ngx_get_request(task_req_node);
        task_node_free(task_req_node);

        clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), task_rsp_node);    
        return (EC_TRUE);                            
    }

    file_len = cbytes_len(file_bytes);

    max_len = CRFSNGX_SUCC_STATUS_SIZE/*SUCC*/
            + 32/*max body len*/
            + file_len;
            
    task_node_buff_alloc(task_rsp_node, max_len);  

    send_len  = 0;
    send_len += snprintf(TASK_NODE_BUFF(task_rsp_node) + send_len, TASK_NODE_BUFF_LEN(task_rsp_node) - send_len, CRFSNGX_SUCC_STATUS_STR);
    send_len += snprintf(TASK_NODE_BUFF(task_rsp_node) + send_len, TASK_NODE_BUFF_LEN(task_rsp_node) - send_len, "%ld\r\n", file_len);

    ASSERT(TASK_NODE_BUFF_LEN(task_rsp_node) >= file_len + send_len);
    BCOPY(cbytes_buf(file_bytes), TASK_NODE_BUFF(task_rsp_node) + send_len, file_len);
    send_len += file_len;
    
    TASK_NODE_BUFF_LEN(task_rsp_node) = send_len;

    crfsngx_clean_ngx_get_request(task_req_node);
    task_node_free(task_req_node); 

    clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), task_rsp_node);    
    return (EC_TRUE);
}


EC_BOOL crfsngx_commit_ngx_set_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;

    EC_BOOL    ret;
    CSTRING   *file_path;

    TASK_RSP  *task_rsp;
    TASK_NODE *task_rsp_node;
    UINT32     max_len;
    UINT32     send_len;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);   

    ret = task_req_func->func_ret_val;
    file_path  = (CSTRING *)task_req_func->func_para[ 1 ].para_val;

    /*reply: STATUS\r\n*/
    
    task_rsp = task_rsp_new(0, LOC_CRFSNGX_0012);
    task_rsp_node = TASK_RSP_NODE(task_rsp);

    TASK_NODE_TAG(task_rsp_node) = TAG_TASK_RSP;
    
    if(EC_FALSE == ret)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_set_reply: set %s failed\n", (char *)cstring_get_str(file_path));

        max_len = CRFSNGX_FAIL_STATUS_SIZE;/*FAIL*/
        task_node_buff_alloc(task_rsp_node, max_len + 1);
        send_len = snprintf(TASK_NODE_BUFF(task_rsp_node), TASK_NODE_BUFF_LEN(task_rsp_node), CRFSNGX_FAIL_STATUS_STR);
        TASK_NODE_BUFF_LEN(task_rsp_node) = send_len;

        crfsngx_clean_ngx_set_request(task_req_node);
        task_node_free(task_req_node);

        clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), task_rsp_node);    
        return (EC_TRUE);                            
    }

    max_len = CRFSNGX_SUCC_STATUS_SIZE/*SUCC*/;
            
    task_node_buff_alloc(task_rsp_node, max_len + 1);  

    send_len  = 0;
    send_len += snprintf(TASK_NODE_BUFF(task_rsp_node) + send_len, TASK_NODE_BUFF_LEN(task_rsp_node) - send_len, CRFSNGX_SUCC_STATUS_STR);
    
    TASK_NODE_BUFF_LEN(task_rsp_node) = send_len;

    crfsngx_clean_ngx_set_request(task_req_node);
    task_node_free(task_req_node); 

    clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), task_rsp_node);    
    return (EC_TRUE);
}

EC_BOOL crfsngx_commit_ngx_del_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;

    EC_BOOL    ret;
    CSTRING   *file_path;

    TASK_RSP  *task_rsp;
    TASK_NODE *task_rsp_node;
    UINT32     max_len;
    UINT32     send_len;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);   

    ret = task_req_func->func_ret_val;
    file_path  = (CSTRING *)task_req_func->func_para[ 1 ].para_val;

    /*reply: STATUS\r\n*/
    
    task_rsp = task_rsp_new(0, LOC_CRFSNGX_0013);
    task_rsp_node = TASK_RSP_NODE(task_rsp);

    TASK_NODE_TAG(task_rsp_node) = TAG_TASK_RSP;
    
    if(EC_FALSE == ret)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_del_reply: set %s failed\n", (char *)cstring_get_str(file_path));

        max_len = CRFSNGX_FAIL_STATUS_SIZE;/*FAIL*/
        task_node_buff_alloc(task_rsp_node, max_len + 1);
        send_len = snprintf(TASK_NODE_BUFF(task_rsp_node), TASK_NODE_BUFF_LEN(task_rsp_node), CRFSNGX_FAIL_STATUS_STR);
        TASK_NODE_BUFF_LEN(task_rsp_node) = send_len;

        crfsngx_clean_ngx_del_request(task_req_node);
        task_node_free(task_req_node);

        clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), task_rsp_node);    
        return (EC_TRUE);                            
    }

    max_len = CRFSNGX_SUCC_STATUS_SIZE/*SUCC*/;
            
    task_node_buff_alloc(task_rsp_node, max_len + 1);  

    send_len  = 0;
    send_len += snprintf(TASK_NODE_BUFF(task_rsp_node) + send_len, TASK_NODE_BUFF_LEN(task_rsp_node) - send_len, CRFSNGX_SUCC_STATUS_STR);
    
    TASK_NODE_BUFF_LEN(task_rsp_node) = send_len;

    crfsngx_clean_ngx_del_request(task_req_node);
    task_node_free(task_req_node); 

    clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), task_rsp_node);    
    return (EC_TRUE);
}

EC_BOOL crfsngx_commit_ngx_select_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{
    TASK_REQ  *task_req;
    TASK_FUNC *task_req_func;

    EC_BOOL    ret;
    CSTRING   *file_path;

    TASK_RSP  *task_rsp;
    TASK_NODE *task_rsp_node;
    UINT32     max_len;
    UINT32     send_len;

    task_req = TASK_NODE_REQ(task_req_node);
    task_req_func = TASK_REQ_FUNC(task_req);   

    ret = task_req_func->func_ret_val;
    file_path  = (CSTRING *)task_req_func->func_para[ 1 ].para_val;

    /*reply: STATUS\r\n*/
    
    task_rsp = task_rsp_new(0, LOC_CRFSNGX_0014);
    task_rsp_node = TASK_RSP_NODE(task_rsp);

    TASK_NODE_TAG(task_rsp_node) = TAG_TASK_RSP;
    
    if(EC_FALSE == ret)
    {
        dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_select_reply: set %s failed\n", (char *)cstring_get_str(file_path));

        max_len = CRFSNGX_FAIL_STATUS_SIZE;/*FAIL*/
        task_node_buff_alloc(task_rsp_node, max_len + 1);
        send_len = snprintf(TASK_NODE_BUFF(task_rsp_node), TASK_NODE_BUFF_LEN(task_rsp_node), CRFSNGX_FAIL_STATUS_STR);
        TASK_NODE_BUFF_LEN(task_rsp_node) = send_len;

        crfsngx_clean_ngx_select_request(task_req_node);
        task_node_free(task_req_node);

        clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), task_rsp_node);    
        return (EC_TRUE);                            
    }

    max_len = CRFSNGX_SUCC_STATUS_SIZE/*SUCC*/;
            
    task_node_buff_alloc(task_rsp_node, max_len + 1);  

    send_len  = 0;
    send_len += snprintf(TASK_NODE_BUFF(task_rsp_node) + send_len, TASK_NODE_BUFF_LEN(task_rsp_node) - send_len, CRFSNGX_SUCC_STATUS_STR);
    
    TASK_NODE_BUFF_LEN(task_rsp_node) = send_len;

    crfsngx_clean_ngx_select_request(task_req_node);
    task_node_free(task_req_node); 

    clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), task_rsp_node);    
    return (EC_TRUE);
}

EC_BOOL crfsngx_commit_ngx_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node)
{
    CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

    crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);

    if(CRFSNGX_GET_TAG == CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr))
    {
        return crfsngx_commit_ngx_get_reply(csocket_cnode, task_req_node);
    }

    if(CRFSNGX_SET_TAG == CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr))
    {
        return crfsngx_commit_ngx_set_reply(csocket_cnode, task_req_node);
    }    

    if(CRFSNGX_DEL_TAG == CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr))
    {
        return crfsngx_commit_ngx_del_reply(csocket_cnode, task_req_node);
    } 

    if(CRFSNGX_SELECT_TAG == CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr))
    {
        return crfsngx_commit_ngx_select_reply(csocket_cnode, task_req_node);
    } 
    
    dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_commit_ngx_reply:unknown ngx tag %d on sockfd %d\n", 
                        CRFSNGX_PKT_HDR_OP_TAG(crfsngx_pkt_hdr), CSOCKET_CNODE_SOCKFD(csocket_cnode));    

    return (EC_FALSE);
}

EC_BOOL crfsngx_recv_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode)
{
    if(EC_FALSE == csocket_cnode_is_connected(csocket_cnode))
    {
        return (EC_FALSE);
    }
    
    for(;;)
    {
        TASK_NODE  *task_node;

        EC_BOOL   done_flag; /*EC_TRUE: all data recved completely, EC_FALSE: incomplete*/

        /*handle incoming tas_node*/
        task_node = CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode);
        if(NULL_PTR != task_node)
        {   
            done_flag = EC_FALSE;
            
            /*if fix cannot complete the csocket_request, BUFF has no data to handle, so terminate*/            
            if(EC_FALSE == crfsngx_csocket_fetch_ngx(csocket_cnode, task_node, &done_flag))
            {
                /*terminate*/
                return (EC_FALSE);
            }

            if(EC_FALSE == done_flag)
            {
                return (EC_TRUE);
            }

            /*otherwise, remove it from INCOMING list and push it to INCOMED list*/
            //cepoll_mod_events(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);
            crfsngx_commit_ngx_request(csocket_cnode, task_node);
            TASK_NODE_COMP(task_node) = TASK_WAS_RECV;
            CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode) = NULL_PTR;/*clean*/

            return (EC_TRUE);/*support one request on one socket connection/session*/
        }  

        done_flag = EC_FALSE;
        
        /*handle next task_node*/
        task_node = task_node_new(CRFSNGX_CSOCKET_BUFF_LEN, LOC_CRFSNGX_0015);
        if(NULL_PTR == task_node)
        {
            dbg_log(SEC_0036_CRFSNGX, 0)(LOGSTDOUT, "error:crfsngx_recv_on_csocket_cnode: new task_node with %d bytes failed\n", 
                               CRFSNGX_CSOCKET_BUFF_LEN);
            return (EC_FALSE);
        } 
        TASK_NODE_TAG(task_node) = TAG_TASK_REQ;
        
        if(EC_FALSE == crfsngx_csocket_fetch_ngx(csocket_cnode, task_node, &done_flag))
        {
            return (EC_FALSE);
        }
        
        if(EC_TRUE == done_flag)
        {
            /*push complete csocket_request to INCOMED list*/
            crfsngx_commit_ngx_request(csocket_cnode, task_node);
            TASK_NODE_COMP(task_node) = TASK_WAS_RECV;

            dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDNULL, "[DEBUG] crfsngx_recv_on_csocket_cnode: push csocket_request to incomed list\n");

            break;
        }
        else
        {
            /*push incomplete csocket_request to INCOMING list*/
            CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode) = task_node;
            dbg_log(SEC_0036_CRFSNGX, 9)(LOGSTDNULL, "[DEBUG] crfsngx_recv_on_csocket_cnode: push csocket_request to incoming list\n");
            /*terminate this loop*/
            break;
        }       
    }
    return (EC_TRUE);
}

EC_BOOL crfsngx_send_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode)
{
    CLIST_DATA *clist_data;

    if(EC_FALSE == csocket_cnode_is_connected(csocket_cnode))
    {
        return (EC_FALSE);
    }

    /*actually, crfsngx handle one request /reply per cycle only*/
    /*send data as much as possible on the csocket_cnode*/
    CLIST_LOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_CRFSNGX_0016);
    CLIST_LOOP_NEXT(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), clist_data)
    {
        TASK_NODE *task_node;

        task_node = (TASK_NODE *)CLIST_DATA_DATA(clist_data);

        if( TASK_NODE_BUFF_POS(task_node) != TASK_NODE_BUFF_LEN(task_node))
        {
            if(EC_FALSE == csocket_isend_task_node(csocket_cnode, task_node))
            {
                CLIST_UNLOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_CRFSNGX_0017);
                //return (EC_FALSE);
                return (EC_TRUE);
            }
        }

        /*when data sending incomplete, end this sending loop*/
        if( TASK_NODE_BUFF_POS(task_node) != TASK_NODE_BUFF_LEN(task_node))
        {
            break;
        }

        /*whend data sending complete, rmv csocket_request from SENDING_REQUEST list*/
        else
        {
            CLIST_DATA *clist_data_rmv;

            clist_data_rmv = clist_data;
            clist_data = CLIST_DATA_PREV(clist_data);

            clist_rmv_no_lock(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), clist_data_rmv);
            TASK_NODE_COMP(task_node) = TASK_WAS_SENT;

            task_node_free(task_node);/*for crfngx*/
            break;
        }
        /*continue next sending*/
    }
    CLIST_UNLOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_CRFSNGX_0018);

    if(EC_TRUE == clist_is_empty(CSOCKET_CNODE_SENDING_LIST(csocket_cnode)))
    {
        CRFSNGX_PKT_HDR *crfsngx_pkt_hdr;

        crfsngx_pkt_hdr = (CRFSNGX_PKT_HDR *)CSOCKET_CNODE_PKT_HDR(csocket_cnode);    
        BSET(crfsngx_pkt_hdr, 0, sizeof(CRFSNGX_PKT_HDR));/*clean up*/
    
#if 0/*this branch is for scenario: one request + reply cycle per connection*/     
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);
        return (EC_FALSE);/*return false will trigger socket shutdown, due to __cepoll_write_handle check return val*/
#else/*this branch is for scenario: multiple request + reply cycles per connection*/
        /*WARNING: the defect is client should never send next request before previous reply reached*/
        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_RD_EVENT,
                          (CSRV_WRITE_HANDLER)crfsngx_recv_on_csocket_cnode, csocket_cnode);
        return (EC_TRUE);/*return true will be ready for next request + reply cycle*/
#endif        
        
    }
    return (EC_TRUE);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

