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

#ifndef _CRFSHTTP_H
#define _CRFSHTTP_H

#include "type.h"
#include "debug.h"

#include "cstring.h"

#include "csocket.inc"
#include "task.inc"

#include "cbuffer.h"
#include "cstrkv.h"
#include "chunk.h"

#include "http_parser.h"

/*HTTP 1.1*/
#define CRFSHTTP_VERSION_MAJOR       ((uint16_t) 1)
#define CRFSHTTP_VERSION_MINOR       ((uint16_t) 1)

#define CRFSHTTP_HEADER_MAX_SIZE     ((uint32_t)(4 * 1024))
#define CRFSHTTP_CBUFFER_SIZE        ((uint32_t)(8 * 1024))


typedef struct 
{
	uint32_t    key;
	const char *val;
}CRFSHTTP_KV;

#define CRFSHTTP_KV_KEY(crfshttp_kv)        ((crfshttp_kv)->key)
#define CRFSHTTP_KV_VAL(crfshttp_kv)        ((crfshttp_kv)->val)

#define CRFSHTTP_NODE_HEADER_UNDEF          ((uint32_t) 0)
#define CRFSHTTP_NODE_HEADER_PARSING        ((uint32_t) 1)
#define CRFSHTTP_NODE_HEADER_PARSED         ((uint32_t) 2)
#define CRFSHTTP_NODE_HEADER_HANDLING       ((uint32_t) 3)
#define CRFSHTTP_NODE_HEADER_HANDLED        ((uint32_t) 4)

#define CRFSHTTP_NODE_UNDEF_OP              ((uint32_t) 1)
#define CRFSHTTP_NODE_GET_OP                ((uint32_t) 1)
#define CRFSHTTP_NODE_POST_SET_OP           ((uint32_t) 2)
#define CRFSHTTP_NODE_POST_UPDATE_OP        ((uint32_t) 3)
#define CRFSHTTP_NODE_POST_RENEW_OP         ((uint32_t) 4)
#define CRFSHTTP_NODE_PUT_SET_OP            ((uint32_t) 5)
#define CRFSHTTP_NODE_PUT_UPDATE_OP         ((uint32_t) 6)
#define CRFSHTTP_NODE_PUT_RENEW_OP          ((uint32_t) 7)

typedef struct _CRFSHTTP_NODE
{
    http_parser_t            http_parser;
    http_parser_settings_t   http_parser_setting;
    
    CBUFFER                  cbuffer;
    uint32_t                 http_header_parsed_len;
    uint32_t                 http_rsp_status;
    uint32_t                 http_header_fsm;/*finite state machine*/
    uint32_t                 http_header_op;/*operation*/
    
    CSOCKET_CNODE           *csocket_cnode;
    
    CBUFFER                  url;   /*string*/
    CBUFFER                  host;  /*string*/
    CBUFFER                  uri;   /*string*/
    CBUFFER                  expires;/*optional header in response*/
    CSTRKV_MGR               header_kvs;
    CHUNK_MGR                req_body_chunks;
    CHUNK_MGR                rsp_body_chunks;
    CTIMET                   c_time;/*create time*/
    CTIMET                   e_time;/*expire time*/
    CTMV                     s_tmv;/*timeval when start for debug or stats*/
    //CTMV                     e_tmv;/*timeval when end for debug or stats*/
    CBYTES                   content_cbytes;/*response content*/
    uint64_t                 content_length;
    uint64_t                 http_body_parsed_len;

    CSTRING                 *store_path;
    EC_BOOL                  expired_body_need;/*flag*/
    EC_BOOL                 (*send_data_more)(struct _CRFSHTTP_NODE *);

    /*buff mode*/
    uint8_t                  *data_buff;
    UINT32                    data_total_len;
    UINT32                    data_sent_len;

    /*sendfile mode*/
    int                       block_fd;
    int                       rsvd1;
    UINT32                    block_size;
    UINT32                    block_pos;
    
    uint64_t                  store_beg_offset;
    uint64_t                  store_end_offset;
    uint64_t                  store_cur_offset;
}CRFSHTTP_NODE;

#define CRFSHTTP_NODE_PARSER(crfshttp_node)              (&((crfshttp_node)->http_parser))
#define CRFSHTTP_NODE_SETTING(crfshttp_node)             (&((crfshttp_node)->http_parser_setting))
#define CRFSHTTP_NODE_CBUFFER(crfshttp_node)             (&((crfshttp_node)->cbuffer))
#define CRFSHTTP_NODE_HEADER_PARSED_LEN(crfshttp_node)   ((crfshttp_node)->http_header_parsed_len)
#define CRFSHTTP_NODE_HEADER_FSM(crfshttp_node)          ((crfshttp_node)->http_header_fsm)
#define CRFSHTTP_NODE_HEADER_OP(crfshttp_node)           ((crfshttp_node)->http_header_op)
#define CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node)       ((crfshttp_node)->csocket_cnode)
#define CRFSHTTP_NODE_URL(crfshttp_node)                 (&((crfshttp_node)->url))
#define CRFSHTTP_NODE_HOST(crfshttp_node)                (&((crfshttp_node)->host))
#define CRFSHTTP_NODE_URI(crfshttp_node)                 (&((crfshttp_node)->uri))
#define CRFSHTTP_NODE_EXPIRES(crfshttp_node)             (&((crfshttp_node)->expires))
#define CRFSHTTP_NODE_HEADER_KVS(crfshttp_node)          (&((crfshttp_node)->header_kvs))
#define CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node)     (&((crfshttp_node)->req_body_chunks))
#define CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node)     (&((crfshttp_node)->rsp_body_chunks))
#define CRFSHTTP_NODE_CREATE_TIME(crfshttp_node)         ((crfshttp_node)->c_time)
#define CRFSHTTP_NODE_EXPIRE_TIME(crfshttp_node)         ((crfshttp_node)->e_time)
#define CRFSHTTP_NODE_START_TMV(crfshttp_node)           (&((crfshttp_node)->s_tmv))
//#define CRFSHTTP_NODE_END_TMV(crfshttp_node)             (&((crfshttp_node)->e_tmv))
#define CRFSHTTP_NODE_CONTENT_CBYTES(crfshttp_node)      (&((crfshttp_node)->content_cbytes))
#define CRFSHTTP_NODE_CONTENT_LENGTH(crfshttp_node)      ((crfshttp_node)->content_length)
#define CRFSHTTP_NODE_BODY_PARSED_LEN(crfshttp_node)     ((crfshttp_node)->http_body_parsed_len)
#define CRFSHTTP_NODE_RSP_STATUS(crfshttp_node)          ((crfshttp_node)->http_rsp_status)

#define CRFSHTTP_NODE_STORE_PATH(crfshttp_node)          ((crfshttp_node)->store_path)
#define CRFSHTTP_NODE_EXPIRED_BODY_NEED(crfshttp_node)   ((crfshttp_node)->expired_body_need)

#define CRFSHTTP_NODE_SEND_DATA_MORE(crfshttp_node)      ((crfshttp_node)->send_data_more)
#define CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node)      ((crfshttp_node)->data_buff)
#define CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node) ((crfshttp_node)->data_total_len)
#define CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node)  ((crfshttp_node)->data_sent_len)
#define CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node)       ((crfshttp_node)->block_fd)
#define CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node)     ((crfshttp_node)->block_size)
#define CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)      ((crfshttp_node)->block_pos)
#define CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node)    ((crfshttp_node)->store_beg_offset)
#define CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node)    ((crfshttp_node)->store_end_offset)
#define CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node)    ((crfshttp_node)->store_cur_offset)
#define CRFSHTTP_NODE_STORE_SIZE(crfshttp_node)          (CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node) - CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node))
                                                         
#define CSOCKET_CNODE_CRFSHTTP_NODE(csocket_cnode)       (CSOCKET_CNODE_PTR(csocket_cnode))

#define    CRFSHTTP_STATUS_NONE                          ((uint32_t)   0)
#define    CRFSHTTP_CONTINUE                             ((uint32_t) 100)
#define    CRFSHTTP_SWITCHING_PROTOCOLS                  ((uint32_t) 101)
#define    CRFSHTTP_PROCESSING                           ((uint32_t) 102)   /* RFC2518 section 10.1 */
#define    CRFSHTTP_OK                                   ((uint32_t) 200)
#define    CRFSHTTP_CREATED                              ((uint32_t) 201)
#define    CRFSHTTP_ACCEPTED                             ((uint32_t) 202)
#define    CRFSHTTP_NON_AUTHORITATIVE_INFORMATION        ((uint32_t) 203)
#define    CRFSHTTP_NO_CONTENT                           ((uint32_t) 204)
#define    CRFSHTTP_RESET_CONTENT                        ((uint32_t) 205)
#define    CRFSHTTP_PARTIAL_CONTENT                      ((uint32_t) 206)
#define    CRFSHTTP_MULTI_STATUS                         ((uint32_t) 207)    /* RFC2518 section 10.2 */
#define    CRFSHTTP_MULTIPLE_CHOICES                     ((uint32_t) 300)
#define    CRFSHTTP_MOVED_PERMANENTLY                    ((uint32_t) 301)
#define    CRFSHTTP_MOVED_TEMPORARILY                    ((uint32_t) 302)
#define    CRFSHTTP_SEE_OTHER                            ((uint32_t) 303)
#define    CRFSHTTP_NOT_MODIFIED                         ((uint32_t) 304)
#define    CRFSHTTP_USE_PROXY                            ((uint32_t) 305)
#define    CRFSHTTP_TEMPORARY_REDIRECT                   ((uint32_t) 307)
#define    CRFSHTTP_BAD_REQUEST                          ((uint32_t) 400)
#define    CRFSHTTP_UNAUTHORIZED                         ((uint32_t) 401)
#define    CRFSHTTP_PAYMENT_REQUIRED                     ((uint32_t) 402)
#define    CRFSHTTP_FORBIDDEN                            ((uint32_t) 403)
#define    CRFSHTTP_NOT_FOUND                            ((uint32_t) 404)
#define    CRFSHTTP_METHOD_NOT_ALLOWED                   ((uint32_t) 405)
#define    CRFSHTTP_NOT_ACCEPTABLE                       ((uint32_t) 406)
#define    CRFSHTTP_PROXY_AUTHENTICATION_REQUIRED        ((uint32_t) 407)
#define    CRFSHTTP_REQUEST_TIMEOUT                      ((uint32_t) 408)
#define    CRFSHTTP_CONFLICT                             ((uint32_t) 409)
#define    CRFSHTTP_GONE                                 ((uint32_t) 410)
#define    CRFSHTTP_LENGTH_REQUIRED                      ((uint32_t) 411)
#define    CRFSHTTP_PRECONDITION_FAILED                  ((uint32_t) 412)
#define    CRFSHTTP_REQUEST_ENTITY_TOO_LARGE             ((uint32_t) 413)
#define    CRFSHTTP_REQUEST_URI_TOO_LONG                 ((uint32_t) 414)
#define    CRFSHTTP_UNSUPPORTED_MEDIA_TYPE               ((uint32_t) 415)
#define    CRFSHTTP_EXPECTATION_FAILED                   ((uint32_t) 417)
#define    CRFSHTTP_UNPROCESSABLE_ENTITY                 ((uint32_t) 422)    /* RFC2518 section 10.3 */
#define    CRFSHTTP_LOCKED                               ((uint32_t) 423)    /* RFC2518 section 10.4 */
#define    CRFSHTTP_FAILED_DEPENDENCY                    ((uint32_t) 424)    /* RFC2518 section 10.5 */
                                                              
#define    CRFSHTTP_INTERNAL_SERVER_ERROR                ((uint32_t) 500)
#define    CRFSHTTP_NOT_IMPLEMENTED                      ((uint32_t) 501)
#define    CRFSHTTP_BAD_GATEWAY                          ((uint32_t) 502)
#define    CRFSHTTP_SERVICE_UNAVAILABLE                  ((uint32_t) 503)
#define    CRFSHTTP_GATEWAY_TIMEOUT                      ((uint32_t) 504)
#define    CRFSHTTP_VERSION_NOT_SUPPORTED                ((uint32_t) 505)
#define    CRFSHTTP_INSUFFICIENT_STORAGE                 ((uint32_t) 507)   /* RFC2518 section 10.6 */
#define    CRFSHTTP_INVALID_HEADER                       ((uint32_t) 600)   /* Squid header parsing error */
#define    CRFSHTTP_HEADER_TOO_LARGE                     ((uint32_t) 601)   /* Header too large to process */

void crfshttp_csocket_cnode_close(CSOCKET_CNODE *csocket_cnode);

EC_BOOL crfshttp_csocket_cnode_defer_close_list_init();

EC_BOOL crfshttp_csocket_cnode_defer_close_list_push(CSOCKET_CNODE *csocket_cnode);

CSOCKET_CNODE *crfshttp_csocket_cnode_defer_close_list_pop();

EC_BOOL crfshttp_csocket_cnode_defer_close_list_is_empty();

EC_BOOL crfshttp_csocket_cnode_defer_close_handle();

EC_BOOL crfshttp_defer_request_queue_init();

EC_BOOL crfshttp_defer_request_queue_clean();

EC_BOOL crfshttp_defer_request_queue_push(CRFSHTTP_NODE *crfshttp_node);

CRFSHTTP_NODE *crfshttp_defer_request_queue_pop();

CRFSHTTP_NODE *crfshttp_defer_request_queue_peek();

EC_BOOL crfshttp_defer_request_queue_launch();

EC_BOOL crfshttp_defer_launch();

EC_BOOL crfshttp_make_response_header_protocol(CRFSHTTP_NODE *crfshttp_node, const uint16_t major, const uint16_t minor, const uint32_t status);

EC_BOOL crfshttp_make_response_header_date(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_response_header_elapsed(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_response_header_content_type(CRFSHTTP_NODE *crfshttp_node, const uint8_t *data, const uint32_t size);

EC_BOOL crfshttp_make_response_header_content_length(CRFSHTTP_NODE *crfshttp_node, const uint64_t size);

EC_BOOL crfshttp_make_response_header_end(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_response_body(CRFSHTTP_NODE *crfshttp_node, const uint8_t *data, const uint32_t size);

EC_BOOL crfshttp_make_response_header(CRFSHTTP_NODE *crfshttp_node, const uint64_t body_len);

EC_BOOL crfshttp_make_error_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_put_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_post_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_getrgf_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_getsmf_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_getbgf_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_dsmf_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_make_ddir_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_handle_put_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_handle_post_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_handle_getrgf_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_handle_getsmf_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_handle_getbgf_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_handle_dsmf_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_handle_ddir_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_error_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_put_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_post_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_getrgf_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_getsmf_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_getbgf_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_dsmf_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_ddir_response(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_error_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_put_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_post_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_getrgf_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_getsmf_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_getbgf_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_dsmf_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_ddir_request(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_is_http_put(const CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_is_http_post(const CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_is_http_getrgf(const CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_is_http_getsmf(const CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_is_http_getbgf(const CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_is_http_dsmf(const CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_is_http_ddir(const CRFSHTTP_NODE *crfshttp_node);


EC_BOOL crfshttp_commit_http_put(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_http_post(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_http_get(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_parse_host(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_parse_content_length(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_parse_uri(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_commit_request(CRFSHTTP_NODE *crfshttp_node);

CRFSHTTP_NODE *crfshttp_node_new(const uint32_t size);

EC_BOOL crfshttp_node_init(CRFSHTTP_NODE *crfshttp_node, const uint32_t size);

EC_BOOL crfshttp_node_clean(CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_node_free(CRFSHTTP_NODE *crfshttp_node);

void    crfshttp_node_print(LOG *log, const CRFSHTTP_NODE *crfshttp_node);

EC_BOOL crfshttp_recv_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode);

EC_BOOL crfshttp_send_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode);


#endif /*_CRFSHTTP_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

