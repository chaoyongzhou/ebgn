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

#ifndef _CXMPP_INC
#define _CXMPP_INC

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
#include "cexpat.h"
#include "cxmppc2s.h"


#define CXMPP_RECV_CBUFFER_SIZE                       ((uint32_t)(8 * 1024))
#define CXMPP_SEND_CBUFFER_SIZE                       ((uint32_t)(8 * 1024))
                                                      
#define CXMPP_NODE_STATE_NONE                         ((uint32_t)0) /* pre-init */
#define CXMPP_NODE_STATE_STREAM_LIST_FEATURE          ((uint32_t)1) /* stream start received (server) */
#define CXMPP_NODE_STATE_STREAM_AUTH                  ((uint32_t)2) /* stream start sent (client) */
#define CXMPP_NODE_STATE_STREAM_BIND_FEATURE          ((uint32_t)3) /* stream established */
#define CXMPP_NODE_STATE_STREAM_ONGOING               ((uint32_t)4) /* auth completed (normal stream operation) */
#define CXMPP_NODE_STATE_STREAM_CLOSING               ((uint32_t)5) /* ready to close (send event_CLOSED to app) */
#define CXMPP_NODE_STATE_STREAM_CLOSED                ((uint32_t)6) /* closed (same as NONE, but can't be used any more) */

#define CXMPP_NODE_STATE_STREAM_ERR                   ((uint32_t)-1)

typedef struct _CXMPP_NODE
{   
    CEXPAT_PARSER            cexpat_parser;

    CSTRING                 *username;
    
    CSOCKET_CNODE           *csocket_cnode;
    CXMPPC2S_CONN           *cxmppc2s_conn;

    CEXPAT_NODE             *recv_cexpat_node;  /*recvd cexpat_node*/ 
    CEXPAT_NODE             *send_cexpat_node;  /*send cexpat_node */ 
    
    CBUFFER                  recv_cbuffer;
    CBUFFER                  send_cbuffer;      /*note: send buffer will shift-out the sent data at once*/

    EC_BOOL                 (*send_data_more)(struct _CXMPP_NODE *);

    uint32_t                 state;
    uint32_t                 shakehand;
    UINT32                   cxmppc2s_md_id;

}CXMPP_NODE;

#define CXMPP_NODE_XML_PARSER(cxmpp_node)               (&((cxmpp_node)->cexpat_parser))
#define CXMPP_NODE_USERNAME(cxmpp_node)                 ((cxmpp_node)->username)
#define CXMPP_NODE_USERNAME_STR(cxmpp_node)             ((char *)cstring_get_str(CXMPP_NODE_USERNAME(cxmpp_node)))
#define CXMPP_NODE_CSOCKET_CNODE(cxmpp_node)            ((cxmpp_node)->csocket_cnode)
#define CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node)            ((cxmpp_node)->cxmppc2s_conn)
#define CXMPP_NODE_RECV_CBUFFER(cxmpp_node)             (&((cxmpp_node)->recv_cbuffer))
#define CXMPP_NODE_SEND_CBUFFER(cxmpp_node)             (&((cxmpp_node)->send_cbuffer))
#define CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node)         ((cxmpp_node)->recv_cexpat_node)
#define CXMPP_NODE_SEND_CEXPAT_NODE(cxmpp_node)         ((cxmpp_node)->send_cexpat_node)
#define CXMPP_NODE_SEND_DATA_MORE_FUNC(cxmpp_node)      ((cxmpp_node)->send_data_more)
#define CXMPP_NODE_STATE(cxmpp_node)                    ((cxmpp_node)->state)
#define CXMPP_NODE_SHAKEHAND(cxmpp_node)                ((cxmpp_node)->shakehand)

#define CXMPP_NODE_PARSE_STATE(cxmpp_node)
#define CXMPP_NODE_PARSE_SET_FROM(cxmpp_node, from_str)
#define CXMPP_NODE_PARSE_SET_TO(cxmpp_node, to_str)
#define CXMPP_NODE_PARSE_SET_VERSION(cxmpp_node, ver_str)
#define CXMPP_NODE_PARSE_SET_ID(cxmpp_node, id_str)
#define CXMPP_NODE_PARSE_DEPTH(cxmpp_node)
#define CXMPP_NODE_PARSE_SET_READ(cxmpp_node)
#define CXMPP_NODE_PARSE_SET_WRITE(cxmpp_node)
#define CXMPP_NODE_PARSE_SET_FAIL(cxmpp_node)

#define CXMPP_NODE_PARSE_IS_FAIL(cxmpp_node)

#define CSOCKET_CNODE_CXMPP_NODE(csocket_cnode)         (CSOCKET_CNODE_PTR(csocket_cnode))


#endif /*_CXMPP_INC*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/


