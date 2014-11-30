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

#ifndef _CXMPP_H
#define _CXMPP_H

#include "type.h"
#include "cstring.h"
#include "clist.h"
#include "cvector.h"

#include "csocket.h"
#include "cbtimer.h"
#include "mod.inc"

#include "cbuffer.h"
#include "cstrkv.h"
#include "chunk.h"

#include "cxmpp.inc"


CXMPP_NODE *cxmpp_node_new(const uint32_t size);

EC_BOOL cxmpp_node_init(CXMPP_NODE *cxmpp_node, const uint32_t size);

EC_BOOL cxmpp_node_clean(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_node_free(CXMPP_NODE *cxmpp_node);

//EC_BOOL cxmpp_csocket_cnode_close(CSOCKET_CNODE *csocket_cnode);

void cxmpp_csocket_cnode_epoll_close(CSOCKET_CNODE *csocket_cnode);

void cxmpp_node_defer_close(CXMPP_NODE *cxmpp_node);

void cxmpp_csocket_cnode_timeout(CSOCKET_CNODE *csocket_cnode);

void cxmpp_csocket_cnode_shutdown(CSOCKET_CNODE *csocket_cnode);

void cxmpp_node_print(LOG *log, const CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_handle_state_none(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_handle_state_stream_list_feature(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_handle_state_stream_auth(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_handle_state_stream_bind_feature(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_handle_state_stream_ongoing(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_handle_state_stream_closing(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_handle_state_stream_closed(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_handle_state(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_handle(CXMPP_NODE *cxmpp_node);

EC_BOOL cxmpp_recv_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode);

EC_BOOL cxmpp_send_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode);

EC_BOOL cxmpp_recv_on_csocket_cnode_thread(CSOCKET_CNODE *csocket_cnode);

EC_BOOL cxmpp_send_on_csocket_cnode_thread(CSOCKET_CNODE *csocket_cnode);

#endif /*_CXMPP_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/


