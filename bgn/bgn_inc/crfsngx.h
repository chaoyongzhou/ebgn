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

#ifndef _CRFSNGX_H
#define _CRFSNGX_H

#include "type.h"
#include "debug.h"

#include "cstring.h"

#include "csocket.inc"
#include "task.inc"


#include "crfsngx.inc"

EC_BOOL crfsngx_csocket_fetch_ngx_get_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag);

EC_BOOL crfsngx_csocket_fetch_ngx_set_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag);

EC_BOOL crfsngx_csocket_fetch_ngx_del_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag);

EC_BOOL crfsngx_csocket_fetch_ngx_select_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag);

EC_BOOL crfsngx_csocket_fetch_ngx_op(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node);

EC_BOOL crfsngx_csocket_fetch_ngx_key(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node);

EC_BOOL crfsngx_csocket_fetch_ngx_vlen(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node);

EC_BOOL crfsngx_csocket_fetch_ngx_val(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node);

EC_BOOL crfsngx_csocket_fetch_ngx(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node, EC_BOOL *done_flag);

EC_BOOL crfsngx_handle_ngx_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node);

EC_BOOL crfsngx_make_ngx_get_request(TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node);

EC_BOOL crfsngx_make_ngx_set_request(TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node);

EC_BOOL crfsngx_make_ngx_del_request(TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node);

EC_BOOL crfsngx_make_ngx_select_request(TASK_NODE *task_req_node, FUNC_ADDR_NODE *func_addr_node);

EC_BOOL crfsngx_clean_ngx_get_request(TASK_NODE *task_req_node);

EC_BOOL crfsngx_clean_ngx_set_request(TASK_NODE *task_req_node);

EC_BOOL crfsngx_clean_ngx_del_request(TASK_NODE *task_req_node);

EC_BOOL crfsngx_clean_ngx_select_request(TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_get_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_set_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_del_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_select_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_request(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_get_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_set_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_del_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_select_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_commit_ngx_reply(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_req_node);

EC_BOOL crfsngx_recv_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode);

EC_BOOL crfsngx_send_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode);



#endif /*_CRFSNGX_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

