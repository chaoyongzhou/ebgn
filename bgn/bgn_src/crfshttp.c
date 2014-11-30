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
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>

#include <sys/stat.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"
#include "cqueue.h"

#include "cbc.h"

#include "cmisc.h"

#include "task.h"

#include "csocket.h"

#include "cmpie.h"

#include "cepoll.h"

#include "crfs.h"
#include "crfshttp.h"

#include "cbuffer.h"
#include "cstrkv.h"
#include "chunk.h"

#include "findex.inc"

/**
protocol
=========
    (case sensitive)

    1. Read File
    REQUEST:
        GET /get/<cache_key> HTTP/1.1
        Host: <host ipaddr | hostname>
        //Date: <date in second>
        store_offset: <file offset>
        store_size: <read length>
    RESPONSE:
        status: 200(HTTP_OK), 400(HTTP_BAD_REQUEST), 404(HTTP_NOT_FOUND)
        body: <file content>
            
    2. Write File    
    REQUEST:
        POST /set/<cache_key> HTTP/1.1
        Host: <host ipaddr | hostname>
        Content-Length: <file length>
        Expires: <date in second>
        body: <file content>
    RESPONSE:
        status: 200(HTTP_OK), 400(HTTP_BAD_REQUEST), 404(HTTP_NOT_FOUND, HTTP_ERROR)
        body: <null>

    3. Update File
    REQUEST:    
        POST /update/<cache_key> HTTP/1.1
        Host: <host ipaddr | hostname>
        Content-Length: <file length>
        Expires: <date in second>
        [body: <file content>
    RESPONSE:
        status: 200(HTTP_OK), 400(HTTP_BAD_REQUEST), 404(HTTP_NOT_FOUND, HTTP_ERROR)
        body: <null>

    4. Delete File
    REQUEST:    
        GET /delete/<cache_key> HTTP/1.1
        Host: <host ipaddr | hostname>
        //Date: <date in second>
    RESPONSE:
        status: 200(HTTP_OK), 400(HTTP_BAD_REQUEST), 404(HTTP_NOT_FOUND, HTTP_ERROR)
        body: <null>        
**/

#if 0
#define CRFSHTTP_PRINT_UINT8(info, buff, len) do{\
    uint32_t __pos;\
    dbg_log(SEC_0049_CRFSHTTP, 5)(LOGSTDOUT, "%s: ", info);\
    for(__pos = 0; __pos < len; __pos ++)\
    {\
        sys_print(LOGSTDOUT, "%02x,", ((uint8_t *)buff)[ __pos ]);\
    }\
    sys_print(LOGSTDOUT, "\n");\
}while(0)

#define CRFSHTTP_PRINT_CHARS(info, buff, len) do{\
    uint32_t __pos;\
    dbg_log(SEC_0049_CRFSHTTP, 5)(LOGSTDOUT, "%s: ", info);\
    for(__pos = 0; __pos < len; __pos ++)\
    {\
        sys_print(LOGSTDOUT, "%c", ((uint8_t *)buff)[ __pos ]);\
    }\
    sys_print(LOGSTDOUT, "\n");\
}while(0)
#else
#define CRFSHTTP_PRINT_UINT8(info, buff, len) do{}while(0)
#define CRFSHTTP_PRINT_CHARS(info, buff, len) do{}while(0)
#endif

static const CRFSHTTP_KV g_crfshttp_status_kvs[] = {
    { 100, "Continue" },
    { 101, "Switching Protocols" },
    { 102, "Processing" }, /* WebDAV */
    { 200, "OK" },
    { 201, "Created" },
    { 202, "Accepted" },
    { 203, "Non-Authoritative Information" },
    { 204, "No Content" },
    { 205, "Reset Content" },
    { 206, "Partial Content" },
    { 207, "Multi-status" }, /* WebDAV */
    { 300, "Multiple Choices" },
    { 301, "Moved Permanently" },
    { 302, "Found" },
    { 303, "See Other" },
    { 304, "Not Modified" },
    { 305, "Use Proxy" },
    { 306, "(Unused)" },
    { 307, "Temporary Redirect" },
    { 400, "Bad Request" },
    { 401, "Unauthorized" },
    { 402, "Payment Required" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
    { 405, "Method Not Allowed" },
    { 406, "Not Acceptable" },
    { 407, "Proxy Authentication Required" },
    { 408, "Request Timeout" },
    { 409, "Conflict" },
    { 410, "Gone" },
    { 411, "Length Required" },
    { 412, "Precondition Failed" },
    { 413, "Request Entity Too Large" },
    { 414, "Request-URI Too Long" },
    { 415, "Unsupported Media Type" },
    { 416, "Requested Range Not Satisfiable" },
    { 417, "Expectation Failed" },
    { 422, "Unprocessable Entity" }, /* WebDAV */
    { 423, "Locked" }, /* WebDAV */
    { 424, "Failed Dependency" }, /* WebDAV */
    { 426, "Upgrade Required" }, /* TLS */
    { 500, "Internal Server Error" },
    { 501, "Not Implemented" },
    { 502, "Bad Gateway" },
    { 503, "Service Not Available" },
    { 504, "Gateway Timeout" },
    { 505, "HTTP Version Not Supported" },
    { 507, "Insufficient Storage" }, /* WebDAV */

    { -1, NULL }
};

static const uint32_t g_crfshttp_status_kvs_num = sizeof(g_crfshttp_status_kvs)/sizeof(g_crfshttp_status_kvs[0]);

static const char *__crfshttp_status_str_get(const uint32_t http_status)
{
    uint32_t idx;

    for(idx = 0; idx < g_crfshttp_status_kvs_num; idx ++)
    {
        const CRFSHTTP_KV *crfshttp_kv;
        crfshttp_kv = &(g_crfshttp_status_kvs[ idx ]);
        if(http_status == CRFSHTTP_KV_KEY(crfshttp_kv))
        {
            return (CRFSHTTP_KV_VAL(crfshttp_kv));
        }
    }
    return ((const char *)"unknown");
}

static CQUEUE g_crfshttp_defer_request_queue;

static CLIST  g_csocket_cnode_defer_close_list;

static int __crfshttp_on_message_begin(http_parser_t* http_parser) 
{
    CRFSHTTP_NODE *crfshttp_node;

    crfshttp_node = (CRFSHTTP_NODE *)http_parser->data;
    if(NULL_PTR == crfshttp_node)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_on_message_begin: http_parser->data is null\n");
        return (-1);
    }

    CRFSHTTP_NODE_HEADER_FSM(crfshttp_node) = CRFSHTTP_NODE_HEADER_PARSING;
    //dbg_log(SEC_0049_CRFSHTTP, 5)(LOGSTDOUT, "\n***MESSAGE BEGIN***\n\n");
    return (0);
}

static int __crfshttp_on_headers_complete(http_parser_t* http_parser) 
{
    CRFSHTTP_NODE *crfshttp_node;

    crfshttp_node = (CRFSHTTP_NODE *)http_parser->data;
    if(NULL_PTR == crfshttp_node)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_on_headers_complete: http_parser->data is null\n");
        return (-1);
    }

    CRFSHTTP_NODE_HEADER_FSM(crfshttp_node) = CRFSHTTP_NODE_HEADER_PARSED;
   
    crfshttp_parse_host(crfshttp_node);
    crfshttp_parse_uri(crfshttp_node);
    crfshttp_parse_content_length(crfshttp_node);

    //crfshttp_node_print(LOGSTDOUT, crfshttp_node);
    
    //dbg_log(SEC_0049_CRFSHTTP, 5)(LOGSTDOUT, "\n***HEADERS COMPLETE***\n\n");
    return (0);
}

static int __crfshttp_on_message_complete(http_parser_t* http_parser) 
{
    (void)http_parser;
    //dbg_log(SEC_0049_CRFSHTTP, 5)(LOGSTDOUT, "\n***MESSAGE COMPLETE***\n\n");
    return (0);
}

static int __crfshttp_on_url(http_parser_t* http_parser, const char* at, size_t length) 
{
    CRFSHTTP_NODE *crfshttp_node;

    crfshttp_node = (CRFSHTTP_NODE *)http_parser->data;
    cbuffer_set(CRFSHTTP_NODE_URL(crfshttp_node), (uint8_t *)at, length);

    //dbg_log(SEC_0049_CRFSHTTP, 5)(LOGSTDOUT, "Url: %.*s\n", (int)length, at);

    return (0);
}

static int __crfshttp_on_header_field(http_parser_t* http_parser, const char* at, size_t length) 
{
    CRFSHTTP_NODE *crfshttp_node;
    CSTRKV *cstrkv;

    crfshttp_node = (CRFSHTTP_NODE *)(http_parser->data);

    cstrkv = cstrkv_new(NULL_PTR, NULL_PTR);
    if(NULL_PTR == cstrkv)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_on_header_field: new cstrkv failed where header field: %.*s\n", 
                           (int)length, at);
        return (-1);
    }

    cstrkv_set_key_bytes(cstrkv, (const uint8_t *)at, (uint32_t)length);
    cstrkv_mgr_add_kv(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), cstrkv);

    //dbg_log(SEC_0049_CRFSHTTP, 5)(LOGSTDOUT, "Header field: %.*s\n", (int)length, at);
    return (0);
}

static int __crfshttp_on_header_value(http_parser_t* http_parser, const char* at, size_t length) 
{
    CRFSHTTP_NODE *crfshttp_node;
    CSTRKV *cstrkv;

    crfshttp_node = (CRFSHTTP_NODE *)(http_parser->data);

    cstrkv = cstrkv_mgr_last_kv(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node));
    if(NULL_PTR == cstrkv)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_on_header_value: no cstrkv existing where value field: %.*s\n", 
                           (int)length, at);
        return (-1);
    }

    cstrkv_set_val_bytes(cstrkv, (const uint8_t *)at, (uint32_t)length);

    //dbg_log(SEC_0049_CRFSHTTP, 5)(LOGSTDOUT, "Header value: %.*s\n", (int)length, at);
    return (0);
}

static int __crfshttp_on_body(http_parser_t* http_parser, const char* at, size_t length) 
{
    CRFSHTTP_NODE *crfshttp_node;
    CHUNK_MGR     *body_chunks;

    //dbg_log(SEC_0049_CRFSHTTP, 5)(LOGSTDOUT, "Body [ignore]: length %d\n\n", length);
    
    crfshttp_node = (CRFSHTTP_NODE *)(http_parser->data);  
    body_chunks   = CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node);

    if(EC_FALSE == chunk_mgr_append_data(body_chunks, (uint8_t *)at, length))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_on_body: append %d bytes failed\n", length);
        return (-1);
    }

    CRFSHTTP_NODE_BODY_PARSED_LEN(crfshttp_node) += length;
    return (0);
}

static EC_BOOL __crfshttp_uri_is_setsmf_op(const CBUFFER *uri_cbuffer)
{
    const uint8_t *uri_str;
    uint32_t       uri_len;
    
    uri_str      = CBUFFER_DATA(uri_cbuffer);
    uri_len      = CBUFFER_USED(uri_cbuffer);

    if(CONST_STR_LEN("/setsmf/") < uri_len 
    && EC_TRUE == c_memcmp(uri_str, CONST_UINT8_STR_AND_LEN("/setsmf/")))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

static EC_BOOL __crfshttp_uri_is_getrgf_op(const CBUFFER *uri_cbuffer)
{
    const uint8_t *uri_str;
    uint32_t       uri_len;
    
    uri_str      = CBUFFER_DATA(uri_cbuffer);
    uri_len      = CBUFFER_USED(uri_cbuffer);

    if(CONST_STR_LEN("/getrgf/") < uri_len 
    && EC_TRUE == c_memcmp(uri_str, CONST_UINT8_STR_AND_LEN("/getrgf/")))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

static EC_BOOL __crfshttp_uri_is_getsmf_op(const CBUFFER *uri_cbuffer)
{
    const uint8_t *uri_str;
    uint32_t       uri_len;
    
    uri_str      = CBUFFER_DATA(uri_cbuffer);
    uri_len      = CBUFFER_USED(uri_cbuffer);

    if(CONST_STR_LEN("/getsmf/") < uri_len 
    && EC_TRUE == c_memcmp(uri_str, CONST_UINT8_STR_AND_LEN("/getsmf/")))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

static EC_BOOL __crfshttp_uri_is_getbgf_op(const CBUFFER *uri_cbuffer)
{
    const uint8_t *uri_str;
    uint32_t       uri_len;
    
    uri_str      = CBUFFER_DATA(uri_cbuffer);
    uri_len      = CBUFFER_USED(uri_cbuffer);

    if(CONST_STR_LEN("/getbgf/") < uri_len 
    && EC_TRUE == c_memcmp(uri_str, CONST_UINT8_STR_AND_LEN("/getbgf/")))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}


static EC_BOOL __crfshttp_uri_is_update_op(const CBUFFER *uri_cbuffer)
{
    const uint8_t *uri_str;
    uint32_t       uri_len;
    
    uri_str      = CBUFFER_DATA(uri_cbuffer);
    uri_len      = CBUFFER_USED(uri_cbuffer);

    if(CONST_STR_LEN("/update/") < uri_len 
    && EC_TRUE == c_memcmp(uri_str, CONST_UINT8_STR_AND_LEN("/update/")))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

/*renew expires setting*/
static EC_BOOL __crfshttp_uri_is_renew_op(const CBUFFER *uri_cbuffer)
{
    const uint8_t *uri_str;
    uint32_t       uri_len;
    
    uri_str      = CBUFFER_DATA(uri_cbuffer);
    uri_len      = CBUFFER_USED(uri_cbuffer);

    if(CONST_STR_LEN("/renew/") < uri_len 
    && EC_TRUE == c_memcmp(uri_str, CONST_UINT8_STR_AND_LEN("/renew/")))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

static EC_BOOL __crfshttp_uri_is_dsmf_op(const CBUFFER *uri_cbuffer)
{
    const uint8_t *uri_str;
    uint32_t       uri_len;
    
    uri_str      = CBUFFER_DATA(uri_cbuffer);
    uri_len      = CBUFFER_USED(uri_cbuffer);

    if(CONST_STR_LEN("/dsmf/") < uri_len 
    && EC_TRUE == c_memcmp(uri_str, CONST_UINT8_STR_AND_LEN("/dsmf/")))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

static EC_BOOL __crfshttp_uri_is_ddir_op(const CBUFFER *uri_cbuffer)
{
    const uint8_t *uri_str;
    uint32_t       uri_len;
    
    uri_str      = CBUFFER_DATA(uri_cbuffer);
    uri_len      = CBUFFER_USED(uri_cbuffer);

    if(CONST_STR_LEN("/ddir/") < uri_len 
    && EC_TRUE == c_memcmp(uri_str, CONST_UINT8_STR_AND_LEN("/ddir/")))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

void crfshttp_csocket_cnode_close0(CSOCKET_CNODE *csocket_cnode)
{
    if(NULL_PTR != csocket_cnode && ERR_FD != CSOCKET_CNODE_SOCKFD(csocket_cnode))
    {
        CRFSHTTP_NODE *crfshttp_node;

        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_csocket_cnode_close: try to close sockfd %d\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        
//#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)
//        cepoll_del_all(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode));
//#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)*/

        crfshttp_node = CSOCKET_CNODE_CRFSHTTP_NODE(csocket_cnode);
        CSOCKET_CNODE_CRFSHTTP_NODE(csocket_cnode) = NULL_PTR;

        //csocket_close(CSOCKET_CNODE_SOCKFD(csocket_cnode));
        //csocket_cnode_free(csocket_cnode);

        crfshttp_node_free(crfshttp_node);
    }
    return;
}

void crfshttp_csocket_cnode_close(CSOCKET_CNODE *csocket_cnode)
{
    if(NULL_PTR != csocket_cnode)
    {     
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
    }
    return;
}

EC_BOOL crfshttp_csocket_cnode_defer_close_list_init()
{
    clist_init(&g_csocket_cnode_defer_close_list, MM_CSOCKET_CNODE, LOC_CRFSHTTP_0001);
    return (EC_TRUE);
}

EC_BOOL crfshttp_csocket_cnode_defer_close_list_push(CSOCKET_CNODE *csocket_cnode)
{
    CLIST_LOCK(&g_csocket_cnode_defer_close_list, LOC_CRFSHTTP_0002);
    if(NULL_PTR == clist_search_front_no_lock(&g_csocket_cnode_defer_close_list, (void *)csocket_cnode, NULL_PTR))
    {
        clist_push_back_no_lock(&g_csocket_cnode_defer_close_list, (void *)csocket_cnode);
    }
    CLIST_UNLOCK(&g_csocket_cnode_defer_close_list, LOC_CRFSHTTP_0003);
    return (EC_TRUE);
}

CSOCKET_CNODE *crfshttp_csocket_cnode_defer_close_list_pop()
{
    return (CSOCKET_CNODE *)clist_pop_front(&g_csocket_cnode_defer_close_list);
}

EC_BOOL crfshttp_csocket_cnode_defer_close_list_is_empty()
{
    return clist_is_empty(&g_csocket_cnode_defer_close_list);
}

EC_BOOL crfshttp_csocket_cnode_defer_close_handle()
{
    while(EC_FALSE == crfshttp_csocket_cnode_defer_close_list_is_empty())
    {
        CSOCKET_CNODE *csocket_cnode;
        CRFSHTTP_NODE *crfshttp_node;
        
        csocket_cnode = crfshttp_csocket_cnode_defer_close_list_pop();
        crfshttp_node = CSOCKET_CNODE_CRFSHTTP_NODE(csocket_cnode);
        if(NULL_PTR != crfshttp_node)
        {
            crfshttp_node_free(crfshttp_node);
        }
        else
        {
            csocket_cnode_close(csocket_cnode);
        }
    }
    return (EC_TRUE);
}

EC_BOOL crfshttp_defer_request_queue_init()
{
    cqueue_init(&g_crfshttp_defer_request_queue, MM_CRFSHTTP_NODE, LOC_CRFSHTTP_0004);

    if(EC_FALSE == cepoll_set_loop_handler(task_brd_default_get_cepoll(), 
                                           (CEPOLL_LOOP_HANDLER)crfshttp_defer_launch, 
                                           NULL_PTR))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_defer_request_queue_init: set cepoll loop handler failed\n");
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL crfshttp_defer_request_queue_clean()
{
    cqueue_clean(&g_crfshttp_defer_request_queue, (CQUEUE_DATA_DATA_CLEANER)crfshttp_node_free);
    return (EC_TRUE);
}

EC_BOOL crfshttp_defer_request_queue_push(CRFSHTTP_NODE *crfshttp_node)
{
    cqueue_push(&g_crfshttp_defer_request_queue, (void *)crfshttp_node);
    return (EC_TRUE);
}

CRFSHTTP_NODE *crfshttp_defer_request_queue_pop()
{
    return (CRFSHTTP_NODE *)cqueue_pop(&g_crfshttp_defer_request_queue);
}

CRFSHTTP_NODE *crfshttp_defer_request_queue_peek()
{
    return (CRFSHTTP_NODE *)cqueue_front(&g_crfshttp_defer_request_queue);
}

EC_BOOL crfshttp_defer_request_queue_launch()
{
    CRFSHTTP_NODE *crfshttp_node;

    for(;;)
    {
        crfshttp_node = crfshttp_defer_request_queue_peek();
        if(NULL_PTR == crfshttp_node)/*no more*/
        {
            break;
        }

        if(EC_FALSE == crfshttp_commit_request(crfshttp_node))
        {
            break;
        }

        crfshttp_defer_request_queue_pop();
    }
    return (EC_TRUE);
}

EC_BOOL crfshttp_defer_launch()
{
    crfshttp_defer_request_queue_launch();
    crfshttp_csocket_cnode_defer_close_handle();
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_response_header_protocol(CRFSHTTP_NODE *crfshttp_node, const uint16_t major, const uint16_t minor, const uint32_t status)
{
    uint8_t  header_protocol[64];
    uint32_t len;

    len = snprintf(((char *)header_protocol), sizeof(header_protocol), "HTTP/%d.%d %d %s\r\n", 
                   major, minor, status, __crfshttp_status_str_get(status));
                   
    if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), header_protocol, len))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header_protocol: append '%.*s' to chunks failed\n", 
                           len, (char *)header_protocol);
        return (EC_FALSE);                            
    }
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_response_header_date(CRFSHTTP_NODE *crfshttp_node)
{
    uint8_t  header_date[64];
    uint32_t len;
    ctime_t  time_in_sec;

    time_in_sec = task_brd_default_get_time();

    /*e.g., Date:Thu, 01 May 2014 12:12:16 GMT*/
	len = strftime(((char *)header_date), sizeof(header_date), "Date:%a, %d %b %Y %H:%M:%S GMT\r\n", gmtime(&time_in_sec));    
	//dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_make_response_header_date: [%.*s] len = %u\n", len - 1, (char *)header_date, len - 1);
    if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), header_date, len))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header_date: append '%.*s' to chunks failed\n", len, header_date);
        return (EC_FALSE);
    }     
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_response_header_expires(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER *expires;

    expires = CRFSHTTP_NODE_EXPIRES(crfshttp_node);
        
    if(0 < CBUFFER_USED(expires))
    {
        //dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_make_response_header_expires: [%.*s] len = %u\n", CBUFFER_USED(expires), (char *)CBUFFER_DATA(expires), CBUFFER_USED(expires));
        if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), CBUFFER_DATA(expires), CBUFFER_USED(expires)))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header_expires: append '%.*s' to chunks failed\n", 
                               CBUFFER_USED(expires), CBUFFER_DATA(expires));
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_response_header_elapsed(CRFSHTTP_NODE *crfshttp_node)
{
    uint8_t  header_date[128];
    uint32_t len;
    CTMV    *s_tmv;
    CTMV    *e_tmv;
    
    uint32_t elapsed_msec;

    s_tmv = CRFSHTTP_NODE_START_TMV(crfshttp_node);
    e_tmv = task_brd_default_get_daytime();

    ASSERT(CTMV_NSEC(e_tmv) >= CTMV_NSEC(s_tmv));
    elapsed_msec = (CTMV_NSEC(e_tmv) - CTMV_NSEC(s_tmv)) * 1000 + CTMV_MSEC(e_tmv) - CTMV_MSEC(s_tmv);

    len = 0;
    len += snprintf(((char *)header_date) + len, sizeof(header_date) - len, "BegTime:%u.%03u\r\n", (uint32_t)CTMV_NSEC(s_tmv), (uint32_t)CTMV_MSEC(s_tmv));
    len += snprintf(((char *)header_date) + len, sizeof(header_date) - len, "EndTime:%u.%03u\r\n", (uint32_t)CTMV_NSEC(e_tmv), (uint32_t)CTMV_MSEC(e_tmv));
    len += snprintf(((char *)header_date) + len, sizeof(header_date) - len, "Elapsed:%u micro seconds\r\n", elapsed_msec);
 
    if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), header_date, len))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header_elapsed: append '%.*s' to chunks failed\n", len, header_date);
        return (EC_FALSE);
    }     
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_response_header_content_type(CRFSHTTP_NODE *crfshttp_node, const uint8_t *data, const uint32_t size)
{
    if(NULL_PTR == data || 0 == size)
    {
        return (EC_TRUE);
    }

    if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), CONST_UINT8_STR_AND_LEN("Content-Type:")))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header_content_type: append 'Content-Type:' to chunks failed\n");
        return (EC_FALSE);
    }    

    if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), data, size))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header_content_type: append %d bytes to chunks failed\n", size);
        return (EC_FALSE);
    }

    if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), CONST_UINT8_STR_AND_LEN("\r\n")))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header_content_type: append EOL to chunks failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_response_header_content_length(CRFSHTTP_NODE *crfshttp_node, const uint64_t size)
{
    uint8_t  content_length[64];
    uint32_t len;

    len = snprintf(((char *)content_length), sizeof(content_length), "Content-Length:%ld\r\n", size);

    if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), content_length, len))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header_content_length: append '%.*s' to chunks failed\n", 
                           len, (char *)content_length);
        return (EC_FALSE);                            
    }
    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_make_response_header_content_length: append '%.*s' to chunks done\n", 
                       len, (char *)content_length);    
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_response_header_end(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), CONST_UINT8_STR_AND_LEN("\r\n")))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header_content_length: append '\r\n' to chunks failed\n");
        return (EC_FALSE);                            
    }
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_response_body(CRFSHTTP_NODE *crfshttp_node, const uint8_t *data, const uint32_t size)
{
    if(NULL_PTR == data || 0 == size)
    {
        return (EC_TRUE);
    }

    //dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_make_response_body: body: '%.*s'\n", size, data);
    if(EC_FALSE == chunk_mgr_append_data(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node), data, size))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_body: append %d bytes to chunks failed\n", size);
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_response_header(CRFSHTTP_NODE *crfshttp_node, const uint64_t body_len)
{
    if(EC_FALSE == crfshttp_make_response_header_protocol(crfshttp_node, 
                                                          CRFSHTTP_VERSION_MAJOR, CRFSHTTP_VERSION_MINOR, 
                                                          CRFSHTTP_NODE_RSP_STATUS(crfshttp_node)))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header: make header protocol failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == crfshttp_make_response_header_date(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header: make header date failed\n");
        return (EC_FALSE);
    }  

    if(EC_FALSE == crfshttp_make_response_header_expires(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header: make header expires failed\n");
        return (EC_FALSE);
    }  

    if(EC_FALSE == crfshttp_make_response_header_elapsed(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header: make header elapsed failed\n");
        return (EC_FALSE);
    }      

    if(EC_FALSE == crfshttp_make_response_header_content_type(crfshttp_node, NULL_PTR, 0))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header: make header content type failed\n");
        return (EC_FALSE);
    }    

    if(EC_FALSE == crfshttp_make_response_header_content_length(crfshttp_node, body_len))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header: make header content length failed\n");        
        return (EC_FALSE);
    }      

    if(EC_FALSE == crfshttp_make_response_header_end(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_response_header: make header end failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfshttp_make_error_response(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_make_response_header(crfshttp_node, (uint64_t)0))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_error_response: make error response header failed\n");
        
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfshttp_make_put_response(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_make_response_header(crfshttp_node, (uint64_t)0))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_put_response: make response header failed\n");
        
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfshttp_make_post_response(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_make_response_header(crfshttp_node, (uint64_t)0))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_post_response: make response header failed\n");
        
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfshttp_make_getrgf_response(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;
    uint64_t       store_size;
       
    uri_cbuffer   = CRFSHTTP_NODE_URI(crfshttp_node);
    store_size    = CRFSHTTP_NODE_STORE_SIZE(crfshttp_node);

    if(do_log(SEC_0049_CRFSHTTP, 9))
    {
        uint8_t       *cache_key;
        uint32_t       cache_len;
    
        cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/getrgf");
        cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/getrgf");
        
        sys_log(LOGSTDOUT, "[DEBUG] crfshttp_make_getrgf_response: path %.*s\n", cache_len, cache_key);
        sys_log(LOGSTDOUT, "[DEBUG] crfshttp_make_getrgf_response: store_size %ld\n", store_size);
    }    
  
    if(EC_FALSE == crfshttp_make_response_header(crfshttp_node, store_size))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_getrgf_response: make response header failed\n");
        return (EC_FALSE);
    } 
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_getsmf_response(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;

    CBYTES        *content_cbytes;
    uint64_t       content_len;
    
    uri_cbuffer    = CRFSHTTP_NODE_URI(crfshttp_node);
    content_cbytes = CRFSHTTP_NODE_CONTENT_CBYTES(crfshttp_node);
    content_len    = CBYTES_LEN(content_cbytes);

    if(do_log(SEC_0049_CRFSHTTP, 9))
    {
        uint8_t       *cache_key;
        uint32_t       cache_len;
    
        cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/getsmf");
        cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/getsmf");
        
        sys_log(LOGSTDOUT, "[DEBUG] crfshttp_make_getsmf_response: path %.*s\n", cache_len, cache_key);
    }
  
    if(EC_FALSE == crfshttp_make_response_header(crfshttp_node, content_len))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_getsmf_response: make response header failed\n");
        return (EC_FALSE);
    } 

    if(EC_FALSE == crfshttp_make_response_body(crfshttp_node, 
                                              CBYTES_BUF(content_cbytes), 
                                              (uint32_t)CBYTES_LEN(content_cbytes)))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_getsmf_response: make body with len %d failed\n", 
                           (uint32_t)CBYTES_LEN(content_cbytes));
        return (EC_FALSE);
    } 

    return (EC_TRUE);
}

EC_BOOL crfshttp_make_getbgf_response(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;
    uint64_t       store_size;
       
    uri_cbuffer   = CRFSHTTP_NODE_URI(crfshttp_node);
    store_size    = CRFSHTTP_NODE_STORE_SIZE(crfshttp_node);

    if(do_log(SEC_0049_CRFSHTTP, 9))
    {
        uint8_t       *cache_key;
        uint32_t       cache_len;
    
        cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/getbgf");
        cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/getbgf");
        
        sys_log(LOGSTDOUT, "[DEBUG] crfshttp_make_getbgf_response: path %.*s\n", cache_len, cache_key);
        sys_log(LOGSTDOUT, "[DEBUG] crfshttp_make_getbgf_response: store_size %ld\n", store_size);
    }    
  
    if(EC_FALSE == crfshttp_make_response_header(crfshttp_node, store_size))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_getbgf_response: make response header failed\n");
        return (EC_FALSE);
    } 
    return (EC_TRUE);
}

EC_BOOL crfshttp_make_dsmf_response(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_make_response_header(crfshttp_node, (uint64_t)0))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_dsmf_response: make response header failed\n");
        return (EC_FALSE);
    } 

    return (EC_TRUE);
}

EC_BOOL crfshttp_make_ddir_response(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_make_response_header(crfshttp_node, (uint64_t)0))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_make_ddir_response: make response header failed\n");
        return (EC_FALSE);
    } 

    return (EC_TRUE);
}

EC_BOOL crfshttp_handle_put_request(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;
        
    uint8_t       *cache_key;
    uint32_t       cache_len;

    CSTRING        path_cstr;
    CBYTES        *content_cbytes;

    uint64_t       body_len;
    uint64_t       content_len;
   
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);
    content_len  = CRFSHTTP_NODE_CONTENT_LENGTH(crfshttp_node);
    ASSERT((uint64_t)0x100000000 > content_len);/*not consider this scenario yet*/

    cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/setsmf");
    cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/setsmf");
    
    cstring_init(&path_cstr, NULL_PTR);
    cstring_append_chars(&path_cstr, cache_len, cache_key);

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_handle_put_request: path %s\n", (char *)cstring_get_str(&path_cstr));

    body_len = chunk_mgr_total_length(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));
    ASSERT((uint64_t)0x100000000 > body_len);/*not consider this scenario yet*/

    if(content_len > body_len)
    {
        dbg_log(SEC_0049_CRFSHTTP, 1)(LOGSTDOUT, "warn:crfshttp_handle_put_request: content_len %lld > body_len %lld\n", content_len, body_len);

        //chunk_mgr_clean(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));/*recycle space asap*/
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_PARTIAL_CONTENT;
        
        cstring_clean(&path_cstr);
        return (EC_TRUE);
    }

    content_cbytes = cbytes_new((UINT32)body_len);
    if(NULL_PTR == content_cbytes)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_put_request: new cbytes with len %d failed\n", (UINT32)body_len);
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_INSUFFICIENT_STORAGE;
        cstring_clean(&path_cstr);
        return (EC_TRUE);
    }

    if(EC_FALSE == chunk_mgr_export_to_cbytes(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node), content_cbytes))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_put_request: export body with len %ld to cbytes failed\n", 
                            cbytes_len(content_cbytes));
                            
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_INTERNAL_SERVER_ERROR;
        
        cstring_clean(&path_cstr);
        cbytes_free(content_cbytes, LOC_CRFSHTTP_0005);
        return (EC_TRUE);
    }    
    
    if(EC_TRUE == __crfshttp_uri_is_setsmf_op(uri_cbuffer))
    {
        CSOCKET_CNODE * csocket_cnode;
        char    *expired_str;
        uint32_t expired_nsec;

        expired_str  = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"Expires");
        expired_nsec = c_str_to_uint32(expired_str);
        
        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        if(EC_FALSE == crfs_write_r(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, content_cbytes, expired_nsec, CRFS_MAX_REPLICA_NUM))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_put_request: crfs write %s failed\n", 
                                (char *)cstring_get_str(&path_cstr));

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_FORBIDDEN;
            
            cstring_clean(&path_cstr);
            cbytes_free(content_cbytes, LOC_CRFSHTTP_0006);
            return (EC_TRUE);
        }
    } 
    else if(EC_TRUE == __crfshttp_uri_is_update_op(uri_cbuffer))
    {
        CSOCKET_CNODE * csocket_cnode;
        char    *expired_str;
        uint32_t expired_nsec;

        expired_str  = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"Expires");
        expired_nsec = c_str_to_uint32(expired_str);
        
        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        if(EC_FALSE == crfs_update_r(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, content_cbytes, expired_nsec, CRFS_MAX_REPLICA_NUM))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_put_request: crfs update %s failed\n", 
                                (char *)cstring_get_str(&path_cstr));

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_FORBIDDEN;
            
            cstring_clean(&path_cstr);
            cbytes_free(content_cbytes, LOC_CRFSHTTP_0007);
            return (EC_TRUE);
        }
    }    
    else if(EC_TRUE == __crfshttp_uri_is_renew_op(uri_cbuffer))
    {
        CSOCKET_CNODE * csocket_cnode;
        char    *expired_str;
        uint32_t expired_nsec;

        expired_str  = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"Expires");
        expired_nsec = c_str_to_uint32(expired_str);
        
        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        if(EC_FALSE == crfs_renew_r(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, expired_nsec, CRFS_MAX_REPLICA_NUM))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_put_request: crfs renew %s failed\n", 
                                (char *)cstring_get_str(&path_cstr));

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_FORBIDDEN;
            
            cstring_clean(&path_cstr);
            cbytes_free(content_cbytes, LOC_CRFSHTTP_0008);
            return (EC_TRUE);
        }
    }    
    else
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_put_request: should never reach here!\n");        
        task_brd_default_abort();
    }

    cstring_clean(&path_cstr);
    cbytes_free(content_cbytes, LOC_CRFSHTTP_0009);    
    
    /*clean body chunks*/
    chunk_mgr_clean(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));

    CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_OK;

    return (EC_TRUE);
}

EC_BOOL crfshttp_handle_post_request(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;
        
    uint8_t       *cache_key;
    uint32_t       cache_len;

    CSTRING        path_cstr;
    CBYTES        *content_cbytes;

    uint64_t       body_len;
    uint64_t       content_len;
   
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);
    content_len  = CRFSHTTP_NODE_CONTENT_LENGTH(crfshttp_node);
    ASSERT((uint64_t)0x100000000 > content_len);/*not consider this scenario yet*/

    cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/setsmf");
    cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/setsmf");
    
    cstring_init(&path_cstr, NULL_PTR);
    cstring_append_chars(&path_cstr, cache_len, cache_key);

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_handle_post_request: path %s\n", (char *)cstring_get_str(&path_cstr));

    body_len = chunk_mgr_total_length(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));
    ASSERT((uint64_t)0x100000000 > body_len);/*not consider this scenario yet*/

    if(content_len > body_len)
    {
        dbg_log(SEC_0049_CRFSHTTP, 1)(LOGSTDOUT, "warn:crfshttp_handle_post_request: content_len %lld > body_len %lld\n", content_len, body_len);

        //chunk_mgr_clean(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));/*recycle space asap*/
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_PARTIAL_CONTENT;
        
        cstring_clean(&path_cstr);
        return (EC_TRUE);
    }

    content_cbytes = cbytes_new((UINT32)body_len);
    if(NULL_PTR == content_cbytes)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_post_request: new cbytes with len %d failed\n", (UINT32)body_len);
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_INSUFFICIENT_STORAGE;
        cstring_clean(&path_cstr);
        return (EC_TRUE);
    }

    if(EC_FALSE == chunk_mgr_export_to_cbytes(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node), content_cbytes))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_post_request: export body with len %ld to cbytes failed\n", 
                            cbytes_len(content_cbytes));
                            
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_INTERNAL_SERVER_ERROR;
        
        cstring_clean(&path_cstr);
        cbytes_free(content_cbytes, LOC_CRFSHTTP_0010);
        return (EC_TRUE);
    }    
    
    if(EC_TRUE == __crfshttp_uri_is_setsmf_op(uri_cbuffer))
    {
        CSOCKET_CNODE * csocket_cnode;
        char    *expired_str;
        uint32_t expired_nsec;

        expired_str  = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"Expires");
        expired_nsec = c_str_to_uint32(expired_str);
        
        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        if(EC_FALSE == crfs_write_r(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, content_cbytes, expired_nsec, CRFS_MAX_REPLICA_NUM))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_post_request: crfs write %s failed\n", 
                                (char *)cstring_get_str(&path_cstr));

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_FORBIDDEN;
            
            cstring_clean(&path_cstr);
            cbytes_free(content_cbytes, LOC_CRFSHTTP_0011);
            return (EC_TRUE);
        }
    } 

    else if(EC_TRUE == __crfshttp_uri_is_update_op(uri_cbuffer))
    {
        CSOCKET_CNODE * csocket_cnode;
        char    *expired_str;
        uint32_t expired_nsec;

        expired_str  = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"Expires");
        expired_nsec = c_str_to_uint32(expired_str);
        
        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        if(EC_FALSE == crfs_update_r(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, content_cbytes, expired_nsec, CRFS_MAX_REPLICA_NUM))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_post_request: crfs update %s failed\n", 
                                (char *)cstring_get_str(&path_cstr));

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_FORBIDDEN;
            
            cstring_clean(&path_cstr);
            cbytes_free(content_cbytes, LOC_CRFSHTTP_0012);
            return (EC_TRUE);
        }
    }
    else if(EC_TRUE == __crfshttp_uri_is_renew_op(uri_cbuffer))
    {
        CSOCKET_CNODE * csocket_cnode;
        char    *expired_str;
        uint32_t expired_nsec;

        expired_str  = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"Expires");
        expired_nsec = c_str_to_uint32(expired_str);
        
        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        if(EC_FALSE == crfs_renew_r(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, expired_nsec, CRFS_MAX_REPLICA_NUM))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_post_request: crfs renew %s failed\n", 
                                (char *)cstring_get_str(&path_cstr));

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_FORBIDDEN;
            
            cstring_clean(&path_cstr);
            cbytes_free(content_cbytes, LOC_CRFSHTTP_0013);
            return (EC_TRUE);
        }
    }    
    else
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_post_request: should never reach here!\n");        
        task_brd_default_abort();
    }
    
    cstring_clean(&path_cstr);
    cbytes_free(content_cbytes, LOC_CRFSHTTP_0014);    
    
    /*clean body chunks*/
    chunk_mgr_clean(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));

    CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_OK;

    return (EC_TRUE);
}

static EC_BOOL __crfshttp_handle_getrgf_request_send_block(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);

    if(CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node) < CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node))
    {    
        UINT32 pos;

        pos = CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node);
        if(EC_FALSE == csocket_sendfile(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                   CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node), 
                                   CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node),
                                   &pos))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getrgf_request_send_block: sockfd %d sendfile %ld bytes failed\n",
                               CSOCKET_CNODE_SOCKFD(csocket_cnode),
                               CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node) - CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)
                               );
            return (EC_FALSE);                           
        }
#if 0
        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getrgf_request_send_body: write %ld bytes from %ld to %ld\n", 
                           pos - CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node),
                           CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node), pos);
#endif
        CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node) += (pos - CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node));
        CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)    = pos;
    }

    if(CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node) < CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node))
    {
        /*wait for next writing*/
   
        return (EC_TRUE);
    }

    CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node) = 0;
    CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)  = 0;

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getrgf_request_send_block: write offset reach %ld\n", 
                       CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node));
    return (EC_TRUE);
}

static EC_BOOL __crfshttp_handle_getrgf_request_send_block_more(CRFSHTTP_NODE *crfshttp_node)
{
    /*send data*/
    if(EC_FALSE == __crfshttp_handle_getrgf_request_send_block(crfshttp_node))
    {
        CSOCKET_CNODE * csocket_cnode;

        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getrgf_request_send_block_more: sockfd %d send body failed where store [%ld, %ld) and reached %ld\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode),
                           CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node)
                           );
        /*cleanup possible WR epoll event*/
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);            
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
        return (EC_FALSE);                           
    }

    if(CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node) >= CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node))
    {
        CSOCKET_CNODE * csocket_cnode;

        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);

        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getrgf_request_send_block_more: sockfd %d send [%ld, %ld) and len %ld done\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode),
                           CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_SIZE(crfshttp_node)
                           );

        /*cleanup possible WR epoll event*/
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);            
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);

        if(ERR_FD != CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node))
        {
            c_file_close(CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node));
            CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node) = ERR_FD;
        }
        
        return (EC_TRUE);                           
    }

    /*wait for next sending*/

    return (EC_TRUE);
}

static EC_BOOL __crfshttp_handle_getrgf_request_fetch_path(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;
    CSTRING       *store_path;    
    uint8_t       *cache_key;
    uint32_t       cache_len;

    uri_cbuffer   = CRFSHTTP_NODE_URI(crfshttp_node);

    cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/getrgf");
    cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/getrgf");
    
    store_path = cstring_new(NULL_PTR, LOC_CRFSHTTP_0015);
    if(NULL_PTR == store_path)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getrgf_request_fetch_path: new cstring for store path %.*s failed\n", 
                            cache_len, cache_key);
        return (EC_FALSE);
    }   
    cstring_append_chars(store_path, cache_len, cache_key);
    CRFSHTTP_NODE_STORE_PATH(crfshttp_node) = store_path;

    return (EC_TRUE);
}

EC_BOOL crfshttp_handle_getrgf_request(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE *csocket_cnode;

    CSTRING       *store_path;

    CBYTES        *content_cbytes;

    char          *data_offset_str;
    char          *data_size_str;

    uint64_t       store_size_of_file;

    int            fd;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    if(EC_FALSE == __crfshttp_handle_getrgf_request_fetch_path(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getrgf_request: fetch store path failed\n");

        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
                             
        //return (EC_FALSE);
        return (EC_TRUE);
    }
    store_path = CRFSHTTP_NODE_STORE_PATH(crfshttp_node);
    
    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_handle_getrgf_request: path %s\n", (char *)cstring_get_str(store_path));

    /*clean content which will never be used*/
    ASSERT(0 == chunk_mgr_count_chunks(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node)));
    content_cbytes = CRFSHTTP_NODE_CONTENT_CBYTES(crfshttp_node);
    cbytes_clean(content_cbytes, LOC_CRFSHTTP_0016);

    /*open file*/
    fd = c_file_open((char *)cstring_get_str(store_path), O_RDONLY, 0666);
    if(ERR_FD == fd)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getrgf_request: open file %s failed\n", 
                            (char *)cstring_get_str(store_path));

        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
       
        //return (EC_FALSE);
        return (EC_TRUE);
    }

    /*get file size*/
    if(EC_FALSE == c_file_size_b(fd, &store_size_of_file))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getrgf_request: get size of file %s failed\n", 
                            (char *)cstring_get_str(store_path));

        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
        c_file_close(fd);
        //return (EC_FALSE);
        return (EC_TRUE);
    }

    CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node)   = fd;

    /*set default [beg, end)*/
    CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) = 0;
    CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node) = store_size_of_file;
#if 0
    /*check validity: only support < 4G regural file*/
    if(0 < (store_size_of_file >> 32))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getrgf_request: file %s size %ld overflow\n", 
                            (char *)cstring_get_str(store_path), store_size_of_file);

        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_FORBIDDEN;
        c_file_close(fd);
        //return (EC_FALSE);
        return (EC_TRUE);
    }
#endif    
    data_offset_str = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"store_offset");
    data_size_str   = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"store_size");

    if(NULL_PTR != data_offset_str)
    {
        uint64_t   data_offset;
        
        data_offset = c_str_to_uint64(data_offset_str);
        if(data_offset >= store_size_of_file) /*invalid offset*/
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getrgf_request: crfs file %s, data_offset %ld >= store_size_of_file %ld\n", 
                                (char *)cstring_get_str(store_path), data_offset, store_size_of_file);

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
           
            //return (EC_FALSE);
            return (EC_TRUE);
        }

        CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) = data_offset;
    }

    if(NULL_PTR != data_size_str)
    {
        uint64_t  data_len;
        
        data_len  = c_str_to_uint64(data_size_str);/*note: when data_size_str is null, data_len is zero*/

        /*if 0 == data_len, from offset to end of file*/
        /*else if data_len + beg > end, from offset to end of file*/
        /*else, from offset to beg + end*/

        if(0 == data_len) /*ok, from offset to end of file*/
        {
            /*nothing to do*/
        }

        else if (data_len + CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) > CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node))
        {
            /*nothing to do*/
        }

        else 
        {
            CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node) = CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) + data_len;
        }
    }

    /*set cur*/
    CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node) = CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node);
    
    /*set http status code: OK*/
    CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_OK;

    CRFSHTTP_NODE_SEND_DATA_MORE(crfshttp_node)      = __crfshttp_handle_getrgf_request_send_block_more;
    CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node)      = NULL_PTR;
    CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node) = 0;
    CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node)  = 0;

    CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)      = (UINT32)CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node);
    CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node)     = (UINT32)CRFSHTTP_NODE_STORE_SIZE(crfshttp_node);

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_handle_getrgf_request: to read file %s in range [%ld, %ld) and len %ld\n", 
                       (char *)cstring_get_str(store_path), 
                       CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node),
                       CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node),
                       CRFSHTTP_NODE_STORE_SIZE(crfshttp_node));

    return (EC_TRUE);
}

EC_BOOL crfshttp_handle_getsmf_request(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;
        
    uint8_t       *cache_key;
    uint32_t       cache_len;

    CSTRING        path_cstr;
    CBYTES        *content_cbytes;

    char          *expired_body_str;
    char          *store_offset_str;
    char          *store_size_str;

    EC_BOOL        expired_body_needed;
    UINT32         expires_timestamp;
    char           expires_str[64];
    uint32_t       expires_str_len;
   
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);

    cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/getsmf");
    cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/getsmf");
    
    cstring_init(&path_cstr, NULL_PTR);
    cstring_append_chars(&path_cstr, cache_len, cache_key);

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_handle_getsmf_request: path %s\n", (char *)cstring_get_str(&path_cstr));

    ASSERT(0 == chunk_mgr_count_chunks(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node)));

    content_cbytes = CRFSHTTP_NODE_CONTENT_CBYTES(crfshttp_node);
    cbytes_clean(content_cbytes, LOC_CRFSHTTP_0017);

    expired_body_str = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"Expired_body");
    if(NULL_PTR == expired_body_str || 0 == c_str_to_uint32(expired_body_str))
    {
        expired_body_needed = EC_TRUE;/*even if file expired, return file content*/
    }
    else
    {
        expired_body_needed = EC_FALSE;/*if file expired, NOT return file content*/
    }

    store_offset_str = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"store_offset");
    if(NULL_PTR != store_offset_str)
    {
        CSOCKET_CNODE * csocket_cnode;
        
        uint32_t store_offset;
        uint32_t store_size;
        
        UINT32   offset;
        UINT32   max_len;
        
        store_size_str   = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"store_size");

        store_offset = c_str_to_uint32(store_offset_str);
        store_size   = c_str_to_uint32(store_size_str);/*note: when store_size_str is null, store_size is zero*/

        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        offset        = store_offset;
        max_len       = store_size;
        
        if(EC_FALSE == crfs_read_e(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, &offset, max_len, content_cbytes, &expires_timestamp, expired_body_needed))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getsmf_request: crfs read %s with offset %u, size %u failed\n", 
                                (char *)cstring_get_str(&path_cstr), store_offset, store_size);

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
            
            cstring_clean(&path_cstr);
            cbytes_clean(content_cbytes, LOC_CRFSHTTP_0018);                                
            //return (EC_FALSE);
            return (EC_TRUE);
        }
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_OK;        
        
    }
    else/*read whole file content*/
    {
        CSOCKET_CNODE * csocket_cnode;        
        
        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        if(EC_FALSE == crfs_read(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, content_cbytes, &expires_timestamp, expired_body_needed))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getsmf_request: crfs read %s failed\n", 
                                (char *)cstring_get_str(&path_cstr));

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
            
            cstring_clean(&path_cstr);
            cbytes_clean(content_cbytes, LOC_CRFSHTTP_0019);                                
            //return (EC_FALSE);
            return (EC_TRUE);
        }
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_OK;
    }

    expires_str_len = snprintf(expires_str, sizeof(expires_str), "Expires:%ld\r\n", expires_timestamp);
    cbuffer_set(CRFSHTTP_NODE_EXPIRES(crfshttp_node), (uint8_t *)expires_str, expires_str_len);

    cstring_clean(&path_cstr);

    return (EC_TRUE);
}

static EC_BOOL __crfshttp_handle_getbgf_request_read_body(CRFSHTTP_NODE *crfshttp_node)
{       
    CSOCKET_CNODE *csocket_cnode;
   
    uint64_t   offset;
    uint64_t   offset_save;
    UINT32     max_len;

    EC_BOOL    expires_timestamp;
    EC_BOOL    expired_body_needed;

    CSTRING   *store_path;
    CBYTES     data_cbytes;
    

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);    

    offset        = CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node);
    offset_save   = offset;     
    max_len       = CPGB_CACHE_MAX_BYTE_SIZE;/*adjust later*/

    store_path    = CRFSHTTP_NODE_STORE_PATH(crfshttp_node);
    
    if(NULL_PTR == CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node))
    {
        uint8_t *data_buff;

        data_buff = safe_malloc(max_len, LOC_CRFSHTTP_0020);
        if(NULL_PTR == data_buff)
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_read_body: malloc %ld bytes failed before read path %s from offset %ld\n", 
                                max_len, (char *)cstring_get_str(store_path), offset_save);

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_INTERNAL_SERVER_ERROR;
            
            //return (EC_FALSE);
            return (EC_TRUE);
        }    
        CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node) = data_buff;
    }

    /*adjust max_len*/
    if(CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node) < CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) + CPGB_CACHE_MAX_BYTE_SIZE)
    {
        max_len = (UINT32)(CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node) - CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node));
    }
    cbytes_mount(&data_cbytes, max_len, CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node));
    

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getbgf_request_read_body: read offset reach %ld\n", offset);

    expired_body_needed = CRFSHTTP_NODE_EXPIRED_BODY_NEED(crfshttp_node);    
    if(EC_FALSE == crfs_read_b(CSOCKET_CNODE_MODI(csocket_cnode), store_path, &offset, max_len, &data_cbytes, &expires_timestamp, expired_body_needed))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_read_body: crfs read %s from offset %ld, max_len %u failed\n", 
                            (char *)cstring_get_str(store_path), offset_save, max_len);
       
        return (EC_FALSE);
    }

    CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node)      = cbytes_buf(&data_cbytes);
    CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node) = (offset - offset_save);
    CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node)  = 0;

    return (EC_TRUE);
}

static EC_BOOL __crfshttp_handle_getbgf_request_send_body(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);

    if(CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node) < CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node))
    {    
        UINT32 pos;

        pos = CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node);
        if(EC_FALSE == csocket_write(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                   CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node), 
                                   CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node),
                                   &pos))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_send_body: sockfd %d send %ld bytes failed\n",
                               CSOCKET_CNODE_SOCKFD(csocket_cnode),
                               CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node) - CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node)
                               );
            return (EC_FALSE);                           
        }
#if 0
        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getbgf_request_send_body: write %ld bytes from %ld to %ld\n", 
                           pos - CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node),
                           CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node), pos);
#endif
        CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node)  += (pos - CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node));
        CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node) = pos;
    }

    if(CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node) < CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node))
    {
        /*wait for next writing*/
   
        return (EC_TRUE);
    }

    CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node) = 0;
    CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node)  = 0;

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getbgf_request_send_body: write offset reach %ld\n", 
                       CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node));
    return (EC_TRUE);
}
static EC_BOOL __crfshttp_handle_getbgf_request_send_body_more(CRFSHTTP_NODE *crfshttp_node)
{
    /*send data*/
    if(EC_FALSE == __crfshttp_handle_getbgf_request_send_body(crfshttp_node))
    {
        CSOCKET_CNODE * csocket_cnode;

        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_send_body_more: sockfd %d send body failed where store [%ld, %ld) and reached %ld\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode),
                           CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node)
                           );
        /*cleanup possible WR epoll event*/
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);            
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
        return (EC_FALSE);                           
    }

    if(CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node) >= CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node))
    {
        CSOCKET_CNODE * csocket_cnode;

        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);

        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getbgf_request_send_body_more: sockfd %d send [%ld, %ld) and len %ld done\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode),
                           CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_SIZE(crfshttp_node)
                           );

        /*cleanup possible WR epoll event*/
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);            
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
        return (EC_TRUE);                           
    }

    /*read data for next sending*/
    if(EC_FALSE ==__crfshttp_handle_getbgf_request_read_body(crfshttp_node))
    {
        CSTRING *store_path;

        store_path = CRFSHTTP_NODE_STORE_PATH(crfshttp_node);
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_send_body_more: crfs read %s failed\n", 
                            (char *)cstring_get_str(store_path));

        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/*get block fd and pos*/
static EC_BOOL __crfshttp_handle_getbgf_request_read_block(CRFSHTTP_NODE *crfshttp_node)
{       
    CSOCKET_CNODE *csocket_cnode;
   
    uint64_t   offset;
    uint32_t   max_len;

    EC_BOOL    expires_timestamp;
    EC_BOOL    expired_body_needed;

    CSTRING   *store_path;

    uint32_t   block_size;
    int        block_fd;
    
    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);    

    offset        = CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node);
    max_len       = CPGB_CACHE_MAX_BYTE_SIZE;/*adjust later*/

    store_path    = CRFSHTTP_NODE_STORE_PATH(crfshttp_node);

    /*adjust max_len*/
    if(CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node) < CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) + CPGB_CACHE_MAX_BYTE_SIZE)
    {
        max_len = (UINT32)(CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node) - CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node));
    }

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getbgf_request_read_block: read offset reach %ld\n", offset);

    expired_body_needed = CRFSHTTP_NODE_EXPIRED_BODY_NEED(crfshttp_node);
    
    if(EC_FALSE == crfs_fetch_block_fd_b(CSOCKET_CNODE_MODI(csocket_cnode), store_path, 
                                          offset, &expires_timestamp, expired_body_needed, 
                                          &block_size, &block_fd))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_read_block: crfs fetch %s from offset %ld, max_len %u failed\n", 
                            (char *)cstring_get_str(store_path), offset, max_len);

        return (EC_FALSE);
    }

    CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node)   = block_fd;
    CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node) = block_size;
    CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)  = (UINT32)(offset % CPGB_CACHE_MAX_BYTE_SIZE);

    return (EC_TRUE);
}

static EC_BOOL __crfshttp_handle_getbgf_request_send_block(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);

    if(CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node) < CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node))
    {    
        UINT32 pos;

        pos = CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node);
        if(EC_FALSE == csocket_sendfile(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                   CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node), 
                                   CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node),
                                   &pos))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_send_block: sockfd %d sendfile %ld bytes failed\n",
                               CSOCKET_CNODE_SOCKFD(csocket_cnode),
                               CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node) - CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)
                               );
            return (EC_FALSE);                           
        }
#if 0
        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getbgf_request_send_body: write %ld bytes from %ld to %ld\n", 
                           pos - CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node),
                           CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node), pos);
#endif
        CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node) += (pos - CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node));
        CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)    = pos;
    }

    if(CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node) < CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node))
    {
        /*wait for next writing*/
   
        return (EC_TRUE);
    }

    CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node) = 0;
    CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)  = 0;

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getbgf_request_send_block: write offset reach %ld\n", 
                       CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node));
    return (EC_TRUE);
}

static EC_BOOL __crfshttp_handle_getbgf_request_send_block_more(CRFSHTTP_NODE *crfshttp_node)
{
    /*send data*/
    if(EC_FALSE == __crfshttp_handle_getbgf_request_send_block(crfshttp_node))
    {
        CSOCKET_CNODE * csocket_cnode;

        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_send_block_more: sockfd %d send body failed where store [%ld, %ld) and reached %ld\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode),
                           CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node)
                           );
        /*cleanup possible WR epoll event*/
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);            
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
        return (EC_FALSE);                           
    }

    if(CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node) >= CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node))
    {
        CSOCKET_CNODE * csocket_cnode;

        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);

        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_handle_getbgf_request_send_block_more: sockfd %d send [%ld, %ld) and len %ld done\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode),
                           CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node),
                           CRFSHTTP_NODE_STORE_SIZE(crfshttp_node)
                           );

        /*cleanup possible WR epoll event*/
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);            
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
        return (EC_TRUE);                           
    }

    /*read block (fd) for next sending*/
    if(EC_FALSE ==__crfshttp_handle_getbgf_request_read_block(crfshttp_node))
    {
        CSTRING *store_path;

        store_path = CRFSHTTP_NODE_STORE_PATH(crfshttp_node);
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_send_block_more: crfs read %s failed\n", 
                            (char *)cstring_get_str(store_path));

        return (EC_FALSE);
    }

    return (EC_TRUE);
}

static EC_BOOL __crfshttp_handle_getbgf_request_fetch_path(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;
    CSTRING       *store_path;    
    uint8_t       *cache_key;
    uint32_t       cache_len;

    uri_cbuffer   = CRFSHTTP_NODE_URI(crfshttp_node);

    cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/getbgf");
    cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/getbgf");
    
    store_path = cstring_new(NULL_PTR, LOC_CRFSHTTP_0021);
    if(NULL_PTR == store_path)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_handle_getbgf_request_fetch_path: new cstring for store path %.*s failed\n", 
                            cache_len, cache_key);
        return (EC_FALSE);
    }   
    cstring_append_chars(store_path, cache_len, cache_key);
    CRFSHTTP_NODE_STORE_PATH(crfshttp_node) = store_path;

    return (EC_TRUE);
}

static EC_BOOL __crfshttp_handle_getbgf_request_fetch_expired_body_need(CRFSHTTP_NODE *crfshttp_node)
{
    char          *expired_body_str;
    EC_BOOL        expired_body_needed;
    
    expired_body_str = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"Expired_body");
    if(NULL_PTR == expired_body_str || 0 == c_str_to_uint32(expired_body_str))
    {
        expired_body_needed = EC_TRUE;/*even if file expired, return file content*/
    }
    else
    {
        expired_body_needed = EC_FALSE;/*if file expired, NOT return file content*/
    }
    CRFSHTTP_NODE_EXPIRED_BODY_NEED(crfshttp_node) = expired_body_needed;

    return (EC_TRUE);
}

EC_BOOL crfshttp_handle_getbgf_request(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE *csocket_cnode;

    CSTRING       *store_path;

    CBYTES        *content_cbytes;

    char          *data_offset_str;
    char          *data_size_str;

    EC_BOOL        expired_body_needed;
    UINT32         expires_timestamp;
    char           expires_str[64];
    uint32_t       expires_str_len;
    //int            sock_sendbuf_size;
    uint64_t       store_size_of_file;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    if(EC_FALSE == __crfshttp_handle_getbgf_request_fetch_path(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getbgf_request: fetch store path failed\n");

        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
                             
        //return (EC_FALSE);
        return (EC_TRUE);
    }
    store_path = CRFSHTTP_NODE_STORE_PATH(crfshttp_node);
    
    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_handle_getbgf_request: path %s\n", (char *)cstring_get_str(store_path));
#if 0
    sock_sendbuf_size = 128 * 1024;
    if(EC_FALSE == csocket_set_sendbuf_size(CSOCKET_CNODE_SOCKFD(csocket_cnode), sock_sendbuf_size))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getbgf_request: sockfd %d set sendbuf size %d failed\n",
                            CSOCKET_CNODE_SOCKFD(csocket_cnode), sock_sendbuf_size);

        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_INTERNAL_SERVER_ERROR;
                             
        //return (EC_FALSE);
        return (EC_TRUE);
    }
#endif
    /*clean content which will never be used*/
    ASSERT(0 == chunk_mgr_count_chunks(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node)));
    content_cbytes = CRFSHTTP_NODE_CONTENT_CBYTES(crfshttp_node);
    cbytes_clean(content_cbytes, LOC_CRFSHTTP_0022);

    /*get expired_body_need flag*/
    __crfshttp_handle_getbgf_request_fetch_expired_body_need(crfshttp_node);
    expired_body_needed = CRFSHTTP_NODE_EXPIRED_BODY_NEED(crfshttp_node);

    /*get file store_size*/
    if(EC_FALSE == crfs_store_size_b(CSOCKET_CNODE_MODI(csocket_cnode), store_path, &store_size_of_file, &expires_timestamp))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getbgf_request: crfs get store size of file %s failed\n", 
                            (char *)cstring_get_str(store_path));

        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
       
        //return (EC_FALSE);
        return (EC_TRUE);
    }    

    /*set default [beg, end)*/
    CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) = 0;
    CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node) = store_size_of_file;
    
    data_offset_str = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"store_offset");
    data_size_str   = cstrkv_mgr_get_val_str(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), (const char *)"store_size");

    if(NULL_PTR != data_offset_str)
    {
        uint64_t   data_offset;
        
        data_offset = c_str_to_uint64(data_offset_str);
        if(data_offset >= store_size_of_file) /*invalid offset*/
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_getbgf_request: crfs file %s, data_offset %ld >= store_size_of_file %ld\n", 
                                (char *)cstring_get_str(store_path), data_offset, store_size_of_file);

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
           
            //return (EC_FALSE);
            return (EC_TRUE);
        }

        CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) = data_offset;
    }

    if(NULL_PTR != data_size_str)
    {
        uint64_t  data_len;
        
        data_len  = c_str_to_uint64(data_size_str);/*note: when data_size_str is null, data_len is zero*/

        /*if 0 == data_len, from offset to end of file*/
        /*else if data_len + beg > end, from offset to end of file*/
        /*else, from offset to beg + end*/

        if(0 == data_len) /*ok, from offset to end of file*/
        {
            /*nothing to do*/
        }

        else if (data_len + CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) > CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node))
        {
            /*nothing to do*/
        }

        else 
        {
            CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node) = CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node) + data_len;
        }
    }

    /*set cur*/
    CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node) = CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node);
    
    /*set http status code: OK*/
    CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_OK;

    /*set Expires header*/
    expires_str_len = snprintf(expires_str, sizeof(expires_str), "Expires:%ld\r\n", expires_timestamp);
    cbuffer_set(CRFSHTTP_NODE_EXPIRES(crfshttp_node), (uint8_t *)expires_str, expires_str_len);

    CRFSHTTP_NODE_SEND_DATA_MORE(crfshttp_node)      = __crfshttp_handle_getbgf_request_send_block_more;
    CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node)      = NULL_PTR;
    CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node) = 0;
    CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node)  = 0;

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_handle_getbgf_request: to read file %s in range [%ld, %ld) and len %ld\n", 
                       (char *)cstring_get_str(store_path), 
                       CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node),
                       CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node),
                       CRFSHTTP_NODE_STORE_SIZE(crfshttp_node));

    return (EC_TRUE);
}

EC_BOOL crfshttp_handle_dsmf_request(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;
        
    uint8_t       *cache_key;
    uint32_t       cache_len;

    CSTRING        path_cstr;
   
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);

    cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/dsmf");
    cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/dsmf");
    
    cstring_init(&path_cstr, NULL_PTR);
    cstring_append_chars(&path_cstr, cache_len, cache_key);

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_handle_dsmf_request: path %s\n", (char *)cstring_get_str(&path_cstr));

    ASSERT(0 == chunk_mgr_count_chunks(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node)));
   
    if(EC_TRUE == __crfshttp_uri_is_dsmf_op(uri_cbuffer))
    {
        CSOCKET_CNODE * csocket_cnode;
        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        if(EC_FALSE == crfs_delete_r(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, CRFSNP_ITEM_FILE_IS_REG, CRFS_MAX_REPLICA_NUM))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_dsmf_request: crfs delete file %s failed\n", 
                                (char *)cstring_get_str(&path_cstr));

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
            
            cstring_clean(&path_cstr);
            return (EC_TRUE);
        }
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_OK;
    }
    else
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_dsmf_request: should never reach here!\n");        
        task_brd_default_abort();
    }

    cstring_clean(&path_cstr);

    return (EC_TRUE);
}

EC_BOOL crfshttp_handle_ddir_request(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *uri_cbuffer;
        
    uint8_t       *cache_key;
    uint32_t       cache_len;

    CSTRING        path_cstr;
   
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);

    cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/ddir");
    cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/ddir");
    
    cstring_init(&path_cstr, NULL_PTR);
    cstring_append_chars(&path_cstr, cache_len, cache_key);

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_handle_ddir_request: path %s\n", (char *)cstring_get_str(&path_cstr));

    ASSERT(0 == chunk_mgr_count_chunks(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node)));
   
    if(EC_TRUE == __crfshttp_uri_is_ddir_op(uri_cbuffer))
    {
        CSOCKET_CNODE * csocket_cnode;
        csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
        if(EC_FALSE == crfs_delete_r(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr, CRFSNP_ITEM_FILE_IS_DIR, CRFS_MAX_REPLICA_NUM))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_ddir_request: crfs delete dir %s failed\n", 
                                (char *)cstring_get_str(&path_cstr));

            CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_FOUND;
            
            cstring_clean(&path_cstr);
            return (EC_TRUE);
        }
        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_OK;
    }
    else
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_handle_ddir_request: should never reach here!\n");        
        task_brd_default_abort();
    }    

    cstring_clean(&path_cstr);

    return (EC_TRUE);
}

EC_BOOL crfshttp_commit_error_response(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_RD_EVENT);        
    return crfshttp_send_on_csocket_cnode(csocket_cnode);
}

EC_BOOL crfshttp_commit_put_response(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    //dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_commit_put_response: chunks are\n");
    //chunk_mgr_print_str(LOGSTDOUT, CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    return crfshttp_send_on_csocket_cnode(csocket_cnode);
}

EC_BOOL crfshttp_commit_post_response(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    return crfshttp_send_on_csocket_cnode(csocket_cnode);
}

EC_BOOL crfshttp_commit_getrgf_response(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    return crfshttp_send_on_csocket_cnode(csocket_cnode);
}

EC_BOOL crfshttp_commit_getsmf_response(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    return crfshttp_send_on_csocket_cnode(csocket_cnode);
}

EC_BOOL crfshttp_commit_getbgf_response(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    return crfshttp_send_on_csocket_cnode(csocket_cnode);
}

EC_BOOL crfshttp_commit_dsmf_response(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    return crfshttp_send_on_csocket_cnode(csocket_cnode);
}

EC_BOOL crfshttp_commit_ddir_response(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE * csocket_cnode;

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    return crfshttp_send_on_csocket_cnode(csocket_cnode);
}

EC_BOOL crfshttp_commit_error_request(CRFSHTTP_NODE *crfshttp_node)
{    
    /*cleanup request body and response body*/
    chunk_mgr_clean(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));
    cbytes_clean(CRFSHTTP_NODE_CONTENT_CBYTES(crfshttp_node), LOC_CRFSHTTP_0023);

    if(EC_FALSE == crfshttp_make_error_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_error_request: make error response failed\n");
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_commit_error_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_error_request: commit error response failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_commit_put_request(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_handle_put_request(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_put_request: handle 'SET' request failed\n");
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_make_put_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_put_request: make 'SET' response failed\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_commit_put_request: make 'SET' response done\n");

    if(EC_FALSE == crfshttp_commit_put_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_put_request: commit 'SET' response failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_commit_post_request(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_handle_post_request(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_post_request: handle 'SET' request failed\n");
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_make_post_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_post_request: make 'SET' response failed\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_commit_post_request: make 'SET' response done\n");

    if(EC_FALSE == crfshttp_commit_post_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_post_request: commit 'SET' response failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_commit_getrgf_request(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_handle_getrgf_request(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_getrgf_request: handle 'GET' request failed\n");        
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_make_getrgf_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_getrgf_request: make 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_commit_getrgf_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_getrgf_request: commit 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_commit_getsmf_request(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_handle_getsmf_request(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_getsmf_request: handle 'GET' request failed\n");        
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_make_getsmf_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_getsmf_request: make 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_commit_getsmf_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_getsmf_request: commit 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_commit_getbgf_request(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_handle_getbgf_request(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_getbgf_request: handle 'GET' request failed\n");        
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_make_getbgf_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_getbgf_request: make 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_commit_getbgf_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_getbgf_request: commit 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}


EC_BOOL crfshttp_commit_dsmf_request(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_handle_dsmf_request(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_dsmf_request: handle 'GET' request failed\n");        
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_make_dsmf_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_dsmf_request: make 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_commit_dsmf_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_dsmf_request: commit 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_commit_ddir_request(CRFSHTTP_NODE *crfshttp_node)
{
    if(EC_FALSE == crfshttp_handle_ddir_request(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_ddir_request: handle 'GET' request failed\n");        
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_make_ddir_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_ddir_request: make 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfshttp_commit_ddir_response(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_ddir_request: commit 'GET' response failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_is_http_put(const CRFSHTTP_NODE *crfshttp_node)
{
    const CBUFFER *uri_cbuffer;
    
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);    

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_is_http_put: uri: '%.*s' [len %d]\n", 
                        CBUFFER_USED(uri_cbuffer), 
                        CBUFFER_DATA(uri_cbuffer), 
                        CBUFFER_USED(uri_cbuffer));

    if(EC_TRUE == __crfshttp_uri_is_setsmf_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }

    if(EC_TRUE == __crfshttp_uri_is_update_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }    

    if(EC_TRUE == __crfshttp_uri_is_renew_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL crfshttp_is_http_post(const CRFSHTTP_NODE *crfshttp_node)
{
    const CBUFFER *uri_cbuffer;
    
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);    

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_is_http_post: uri: '%.*s' [len %d]\n", 
                        CBUFFER_USED(uri_cbuffer), 
                        CBUFFER_DATA(uri_cbuffer), 
                        CBUFFER_USED(uri_cbuffer));
                        
    if(EC_TRUE == __crfshttp_uri_is_setsmf_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }

    if(EC_TRUE == __crfshttp_uri_is_update_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }    

    if(EC_TRUE == __crfshttp_uri_is_renew_op(uri_cbuffer))
    {
        return (EC_TRUE);
    } 
    
    return (EC_FALSE);
}

/*regular file*/
EC_BOOL crfshttp_is_http_getrgf(const CRFSHTTP_NODE *crfshttp_node)
{
    const CBUFFER *uri_cbuffer;
    
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_is_http_getrgf: uri: '%.*s' [len %d]\n", 
                        CBUFFER_USED(uri_cbuffer), 
                        CBUFFER_DATA(uri_cbuffer), 
                        CBUFFER_USED(uri_cbuffer));

    if(EC_TRUE == __crfshttp_uri_is_getrgf_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }
    
    return (EC_FALSE);
}

EC_BOOL crfshttp_is_http_getsmf(const CRFSHTTP_NODE *crfshttp_node)
{
    const CBUFFER *uri_cbuffer;
    
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_is_http_getsmf: uri: '%.*s' [len %d]\n", 
                        CBUFFER_USED(uri_cbuffer), 
                        CBUFFER_DATA(uri_cbuffer), 
                        CBUFFER_USED(uri_cbuffer));

    if(EC_TRUE == __crfshttp_uri_is_getsmf_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }
    
    return (EC_FALSE);
}

EC_BOOL crfshttp_is_http_getbgf(const CRFSHTTP_NODE *crfshttp_node)
{
    const CBUFFER *uri_cbuffer;
    
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_is_http_getbgf: uri: '%.*s' [len %d]\n", 
                        CBUFFER_USED(uri_cbuffer), 
                        CBUFFER_DATA(uri_cbuffer), 
                        CBUFFER_USED(uri_cbuffer));

    if(EC_TRUE == __crfshttp_uri_is_getbgf_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }
    
    return (EC_FALSE);
}

/*delete small/regular file*/
EC_BOOL crfshttp_is_http_dsmf(const CRFSHTTP_NODE *crfshttp_node)
{
    const CBUFFER *uri_cbuffer;
   
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);    

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_is_http_dsmf: uri: '%.*s' [len %d]\n", 
                        CBUFFER_USED(uri_cbuffer), 
                        CBUFFER_DATA(uri_cbuffer), 
                        CBUFFER_USED(uri_cbuffer));

    if(EC_TRUE == __crfshttp_uri_is_dsmf_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }    

    return (EC_FALSE);
}

/*delete dir*/
EC_BOOL crfshttp_is_http_ddir(const CRFSHTTP_NODE *crfshttp_node)
{
    const CBUFFER *uri_cbuffer;
    
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);  

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_is_http_ddir: uri: '%.*s' [len %d]\n", 
                        CBUFFER_USED(uri_cbuffer), 
                        CBUFFER_DATA(uri_cbuffer), 
                        CBUFFER_USED(uri_cbuffer));

    if(EC_TRUE == __crfshttp_uri_is_ddir_op(uri_cbuffer))
    {
        return (EC_TRUE);
    }    

    return (EC_FALSE);
}

EC_BOOL crfshttp_commit_http_put(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER *uri_cbuffer;
    
    if(EC_TRUE == crfshttp_is_http_put(crfshttp_node))
    {
        return crfshttp_commit_put_request(crfshttp_node);
    }
    
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);    
    dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_http_put: invalid uri %.*s\n", CBUFFER_USED(uri_cbuffer), CBUFFER_DATA(uri_cbuffer));

    return crfshttp_commit_error_request(crfshttp_node);
}

EC_BOOL crfshttp_commit_http_post(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER *uri_cbuffer;
    
    if(EC_TRUE == crfshttp_is_http_post(crfshttp_node))
    {
        return crfshttp_commit_post_request(crfshttp_node);
    }
    
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);    
    dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_http_post: invalid uri %.*s\n", CBUFFER_USED(uri_cbuffer), CBUFFER_DATA(uri_cbuffer));

    return crfshttp_commit_error_request(crfshttp_node);
}

EC_BOOL crfshttp_commit_http_get(CRFSHTTP_NODE *crfshttp_node)
{
    EC_BOOL ret;

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_commit_http_get: uri: '%.*s' [len %d]\n", 
                        CBUFFER_USED(CRFSHTTP_NODE_URI(crfshttp_node)), 
                        CBUFFER_DATA(CRFSHTTP_NODE_URI(crfshttp_node)), 
                        CBUFFER_USED(CRFSHTTP_NODE_URI(crfshttp_node)));
                        
    if(EC_TRUE == crfshttp_is_http_getsmf(crfshttp_node))
    {
        ret = crfshttp_commit_getsmf_request(crfshttp_node);
    }  
    else if(EC_TRUE == crfshttp_is_http_getbgf(crfshttp_node))
    {
        ret = crfshttp_commit_getbgf_request(crfshttp_node);
    }      
    else if (EC_TRUE == crfshttp_is_http_dsmf(crfshttp_node))
    {
        ret = crfshttp_commit_dsmf_request(crfshttp_node);
    }
    else if (EC_TRUE == crfshttp_is_http_ddir(crfshttp_node))
    {
        ret = crfshttp_commit_ddir_request(crfshttp_node);
    }
    else if (EC_TRUE == crfshttp_is_http_getrgf(crfshttp_node))
    {
        ret = crfshttp_commit_getrgf_request(crfshttp_node);
    }    
    else
    {
        CBUFFER *uri_cbuffer;
        
        uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);    
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_http_get: invalid uri %.*s\n", CBUFFER_USED(uri_cbuffer), CBUFFER_DATA(uri_cbuffer));

        CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_NOT_ACCEPTABLE;
        ret = EC_FALSE;
    }

    if(EC_FALSE == ret)
    {
        return crfshttp_commit_error_request(crfshttp_node);
    }

    return (EC_TRUE);
}

EC_BOOL crfshttp_parse_host(CRFSHTTP_NODE *crfshttp_node)
{
    CSTRING       *host_cstr;
    CBUFFER       *url;
    uint8_t       *data;
    uint8_t       *host_str;
    uint32_t       offset;
    uint32_t       host_len;
    
    host_cstr = cstrkv_mgr_get_val_cstr(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), "Host");
    if(NULL_PTR != host_cstr)
    {
        cbuffer_set(CRFSHTTP_NODE_HOST(crfshttp_node), cstring_get_str(host_cstr), (uint32_t)cstring_get_len(host_cstr));
        return (EC_TRUE);
    }

    dbg_log(SEC_0049_CRFSHTTP, 3)(LOGSTDOUT, "info:crfshttp_parse_host: not found 'Host' in http header\n");

    url  = CRFSHTTP_NODE_URL(crfshttp_node);
    data = CBUFFER_DATA(url);    
    
    for(offset = CONST_STR_LEN("http://"); offset < CBUFFER_USED(url); offset ++)
    {
        if('/' == data [ offset ])
        {
            break;
        }
    }
    
    host_str = CBUFFER_DATA(url) + CONST_STR_LEN("http://");
    host_len = offset - CONST_STR_LEN("http://");

    dbg_log(SEC_0049_CRFSHTTP, 3)(LOGSTDOUT, "info:crfshttp_parse_host: fetch domain %.*s as 'Host' in http header\n", host_len, host_str);
    
    cbuffer_set(CRFSHTTP_NODE_HOST(crfshttp_node), host_str, host_len);    

    return (EC_TRUE);
}

EC_BOOL crfshttp_parse_content_length(CRFSHTTP_NODE *crfshttp_node)
{
    CSTRING       *content_length_cstr;
    
    content_length_cstr = cstrkv_mgr_get_val_cstr(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node), "Content-Length");
    if(NULL_PTR == content_length_cstr)
    {
        dbg_log(SEC_0049_CRFSHTTP, 3)(LOGSTDOUT, "info:crfshttp_parse_content_length: not found 'Content-Length' in http header\n");
        CRFSHTTP_NODE_CONTENT_LENGTH(crfshttp_node) = 0;
        return (EC_TRUE);
    }
    CRFSHTTP_NODE_CONTENT_LENGTH(crfshttp_node) = c_chars_to_uint64((char *)cstring_get_str(content_length_cstr), 
                                                                    (uint32_t)cstring_get_len(content_length_cstr));

    return (EC_TRUE);
}

EC_BOOL crfshttp_parse_uri(CRFSHTTP_NODE *crfshttp_node)
{
    CBUFFER       *url_cbuffer;
    CBUFFER       *host_cbuffer;
    CBUFFER       *uri_cbuffer;
   
    uint8_t       *uri_str;    
    uint32_t       uri_len;
    uint32_t       skip_len;
    
    url_cbuffer  = CRFSHTTP_NODE_URL(crfshttp_node);
    host_cbuffer = CRFSHTTP_NODE_HOST(crfshttp_node);
    uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);

    if(EC_FALSE == cbuffer_cmp_bytes(url_cbuffer, 0, CONST_UINT8_STR_AND_LEN("http://")))
    {
        cbuffer_clone(url_cbuffer, uri_cbuffer);
        return (EC_TRUE);
    }

    skip_len = sizeof("http://") - 1 + CBUFFER_USED(host_cbuffer);
    uri_str  = CBUFFER_DATA(url_cbuffer) + skip_len;
    uri_len  = CBUFFER_USED(url_cbuffer) - skip_len;

    if(EC_FALSE == cbuffer_set(uri_cbuffer, uri_str, uri_len))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_parse_uri: set uri %.*s failed\n", uri_len, uri_str);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfshttp_commit_request(CRFSHTTP_NODE *crfshttp_node)
{  
    http_parser_t *http_parser;
    
    http_parser = CRFSHTTP_NODE_PARSER(crfshttp_node);

    if(HTTP_GET == http_parser->method)
    {
        CROUTINE_NODE  *croutine_node;
        
        croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd_default_get()), 
                                           (UINT32)crfshttp_commit_http_get, 1, crfshttp_node);
        if(NULL_PTR == croutine_node)
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_request: cthread load for HTTP_GET failed\n");
            return (EC_FALSE);
        }
        CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CRFSHTTP_0024);    
        
        return (EC_TRUE);
    }

    if(HTTP_PUT == http_parser->method)
    {
        CROUTINE_NODE  *croutine_node;
        
        croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd_default_get()), 
                                           (UINT32)crfshttp_commit_http_put, 1, crfshttp_node);
        if(NULL_PTR == croutine_node)
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_request: cthread load for HTTP_PUT failed\n");
            return (EC_FALSE);
        }
        CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CRFSHTTP_0025);    
        
        return (EC_TRUE);
    }    

    if(HTTP_POST == http_parser->method)
    {
        CROUTINE_NODE  *croutine_node;
        
        croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd_default_get()), 
                                           (UINT32)crfshttp_commit_http_post, 1, crfshttp_node);
        if(NULL_PTR == croutine_node)
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_request: cthread load for HTTP_POST failed\n");
            return (EC_FALSE);
        }
        CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CRFSHTTP_0026);    

    
        return (EC_TRUE);
    }

    dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_commit_request: not support http method %d yet\n", http_parser->method);
    return (EC_TRUE);
}

static void __crfshttp_parser_setting_init(http_parser_settings_t   *http_parser_setting)
{
    BSET(http_parser_setting, 0, sizeof(http_parser_settings_t));
    
    http_parser_setting->on_message_begin    = __crfshttp_on_message_begin;
    http_parser_setting->on_url              = __crfshttp_on_url;
    http_parser_setting->on_header_field     = __crfshttp_on_header_field;
    http_parser_setting->on_header_value     = __crfshttp_on_header_value;
    http_parser_setting->on_headers_complete = __crfshttp_on_headers_complete;
    http_parser_setting->on_body             = __crfshttp_on_body;
    http_parser_setting->on_message_complete = __crfshttp_on_message_complete;

    return;
}

CRFSHTTP_NODE *crfshttp_node_new(const uint32_t size)
{
    CRFSHTTP_NODE *crfshttp_node;

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_node_new: size = %d\n", size);

    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CRFSHTTP_NODE, &crfshttp_node, LOC_CRFSHTTP_0027);
    if(NULL_PTR == crfshttp_node)
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_node_new: new crfshttp_node failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == crfshttp_node_init(crfshttp_node, size))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_node_new: init crfshttp_node failed\n");
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CRFSHTTP_NODE, crfshttp_node, LOC_CRFSHTTP_0028);
        return (NULL_PTR);
    }

    return (crfshttp_node);
}

EC_BOOL crfshttp_node_init(CRFSHTTP_NODE *crfshttp_node, const uint32_t size)
{
    if(EC_FALSE == cbuffer_init(CRFSHTTP_NODE_CBUFFER(crfshttp_node), size))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_node_init: init cbuffer with size %d failed\n", size);
        return (EC_FALSE);
    }

    CRFSHTTP_NODE_HEADER_PARSED_LEN(crfshttp_node) = 0;
    CRFSHTTP_NODE_HEADER_FSM(crfshttp_node)        = CRFSHTTP_NODE_HEADER_UNDEF;
    CRFSHTTP_NODE_HEADER_OP(crfshttp_node)         = CRFSHTTP_NODE_UNDEF_OP;
    CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node)     = NULL_PTR;

    cbuffer_init(CRFSHTTP_NODE_URL(crfshttp_node) , 0);
    cbuffer_init(CRFSHTTP_NODE_HOST(crfshttp_node), 0);
    cbuffer_init(CRFSHTTP_NODE_URI(crfshttp_node) , 0);
    cbuffer_init(CRFSHTTP_NODE_EXPIRES(crfshttp_node) , 0);

    cstrkv_mgr_init(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node));
    chunk_mgr_init(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));
    chunk_mgr_init(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node));

    cbytes_init(CRFSHTTP_NODE_CONTENT_CBYTES(crfshttp_node));

    CRFSHTTP_NODE_CREATE_TIME(crfshttp_node) = 0;
    CRFSHTTP_NODE_EXPIRE_TIME(crfshttp_node) = 0;

    CTMV_INIT(CRFSHTTP_NODE_START_TMV(crfshttp_node));

    CRFSHTTP_NODE_CONTENT_LENGTH(crfshttp_node)  = 0;
    CRFSHTTP_NODE_BODY_PARSED_LEN(crfshttp_node) = 0;
    CRFSHTTP_NODE_RSP_STATUS(crfshttp_node)      = CRFSHTTP_STATUS_NONE;

    CRFSHTTP_NODE_STORE_PATH(crfshttp_node)         = NULL_PTR;
    CRFSHTTP_NODE_EXPIRED_BODY_NEED(crfshttp_node)  = EC_TRUE;

    CRFSHTTP_NODE_SEND_DATA_MORE(crfshttp_node)      = NULL_PTR;
    CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node)      = NULL_PTR;
    CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node) = 0;
    CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node)  = 0;

    CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node)       = ERR_FD;
    CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node)     = 0;
    CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)      = 0;

    CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node)    = 0;
    CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node)    = 0;
    CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node)    = 0;

    http_parser_init(CRFSHTTP_NODE_PARSER(crfshttp_node), HTTP_REQUEST);    
    __crfshttp_parser_setting_init(CRFSHTTP_NODE_SETTING(crfshttp_node));

    return (EC_TRUE);
}

EC_BOOL crfshttp_node_clean(CRFSHTTP_NODE *crfshttp_node)
{
    CSOCKET_CNODE *csocket_cnode;

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_node_clean: try to clean crfshttp_node %p\n", crfshttp_node);

    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);
    if(NULL_PTR != csocket_cnode)
    {
        CEPOLL *cepoll;

        cepoll = task_brd_default_get_cepoll();
        cepoll_del_all(cepoll, CSOCKET_CNODE_SOCKFD(csocket_cnode));
        cepoll_set_not_used(cepoll, CSOCKET_CNODE_SOCKFD(csocket_cnode));
    }
    
    cbuffer_clean(CRFSHTTP_NODE_CBUFFER(crfshttp_node));

    CRFSHTTP_NODE_HEADER_PARSED_LEN(crfshttp_node) = 0;
    CRFSHTTP_NODE_HEADER_FSM(crfshttp_node)        = CRFSHTTP_NODE_HEADER_UNDEF;
    CRFSHTTP_NODE_HEADER_OP(crfshttp_node)         = CRFSHTTP_NODE_UNDEF_OP;
    CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node)     = NULL_PTR;
    
    cbuffer_clean(CRFSHTTP_NODE_URL(crfshttp_node));
    cbuffer_clean(CRFSHTTP_NODE_HOST(crfshttp_node));
    cbuffer_clean(CRFSHTTP_NODE_URI(crfshttp_node));
    cbuffer_clean(CRFSHTTP_NODE_EXPIRES(crfshttp_node));

    cstrkv_mgr_clean(CRFSHTTP_NODE_HEADER_KVS(crfshttp_node));
    chunk_mgr_clean(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));    
    chunk_mgr_clean(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node));    

    cbytes_clean(CRFSHTTP_NODE_CONTENT_CBYTES(crfshttp_node), LOC_CRFSHTTP_0029);

    CRFSHTTP_NODE_CREATE_TIME(crfshttp_node) = 0;
    CRFSHTTP_NODE_EXPIRE_TIME(crfshttp_node) = 0;

    CTMV_CLEAN(CRFSHTTP_NODE_START_TMV(crfshttp_node));

    CRFSHTTP_NODE_CONTENT_LENGTH(crfshttp_node)  = 0;
    CRFSHTTP_NODE_BODY_PARSED_LEN(crfshttp_node) = 0;
    CRFSHTTP_NODE_RSP_STATUS(crfshttp_node)      = CRFSHTTP_STATUS_NONE;

    if(NULL_PTR != CRFSHTTP_NODE_STORE_PATH(crfshttp_node))
    {
        cstring_free(CRFSHTTP_NODE_STORE_PATH(crfshttp_node));
        CRFSHTTP_NODE_STORE_PATH(crfshttp_node) = NULL_PTR;
    }
    CRFSHTTP_NODE_STORE_PATH(crfshttp_node) = EC_TRUE;

    CRFSHTTP_NODE_SEND_DATA_MORE(crfshttp_node)      = NULL_PTR;
    if(NULL_PTR != CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node))
    {
        safe_free(CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node), LOC_CRFSHTTP_0030);
        CRFSHTTP_NODE_SEND_DATA_BUFF(crfshttp_node) = NULL_PTR;
    }
    CRFSHTTP_NODE_SEND_DATA_TOTAL_LEN(crfshttp_node) = 0;
    CRFSHTTP_NODE_SEND_DATA_SENT_LEN(crfshttp_node)  = 0; 

    CRFSHTTP_NODE_SEND_BLOCK_FD(crfshttp_node)       = ERR_FD;
    CRFSHTTP_NODE_SEND_BLOCK_SIZE(crfshttp_node)     = 0;
    CRFSHTTP_NODE_SEND_BLOCK_POS(crfshttp_node)      = 0;  
    
    CRFSHTTP_NODE_STORE_BEG_OFFSET(crfshttp_node)    = 0;
    CRFSHTTP_NODE_STORE_END_OFFSET(crfshttp_node)    = 0;
    CRFSHTTP_NODE_STORE_CUR_OFFSET(crfshttp_node)    = 0;    

    if(NULL_PTR != csocket_cnode)
    {
        //not sure ....
        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_node_clean: try to close socket %d\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        csocket_cnode_close(csocket_cnode);/*when socket is closed, it may be reused at once*/
    }

    return (EC_TRUE);
}

EC_BOOL crfshttp_node_free(CRFSHTTP_NODE *crfshttp_node)
{
    if(NULL_PTR != crfshttp_node)
    {
        crfshttp_node_clean(crfshttp_node);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CRFSHTTP_NODE, crfshttp_node, LOC_CRFSHTTP_0031);
    }

    return (EC_TRUE);
}

void crfshttp_node_print(LOG *log, const CRFSHTTP_NODE *crfshttp_node)
{
    sys_log(LOGSTDOUT, "crfshttp_node_print:url : \n");
    cbuffer_print_str(LOGSTDOUT, CRFSHTTP_NODE_URL(crfshttp_node));
    
    sys_log(LOGSTDOUT, "crfshttp_node_print:host : \n");
    cbuffer_print_str(LOGSTDOUT, CRFSHTTP_NODE_HOST(crfshttp_node));

    sys_log(LOGSTDOUT, "crfshttp_node_print:uri : \n");
    cbuffer_print_str(LOGSTDOUT, CRFSHTTP_NODE_URI(crfshttp_node));

    sys_log(LOGSTDOUT, "crfshttp_node_print:header kvs: \n");
    cstrkv_mgr_print(LOGSTDOUT, CRFSHTTP_NODE_HEADER_KVS(crfshttp_node));

    sys_log(LOGSTDOUT, "crfshttp_node_print:header content length: "PRIx64"\n", CRFSHTTP_NODE_CONTENT_LENGTH(crfshttp_node));

    sys_log(LOGSTDOUT, "crfshttp_node_print:req body chunks: total length %"PRIx64"\n", chunk_mgr_total_length(CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node)));
    sys_log(LOGSTDOUT, "crfshttp_node_print:rsp body chunks: total length %"PRIx64"\n", chunk_mgr_total_length(CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node)));
    //chunk_mgr_print_str(LOGSTDOUT, CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));
    //chunk_mgr_print_info(LOGSTDOUT, CRFSHTTP_NODE_REQ_BODY_CHUNKS(crfshttp_node));

    sys_log(LOGSTDOUT, "crfshttp_node_print:create time: %d\n", CRFSHTTP_NODE_CREATE_TIME(crfshttp_node));
    sys_log(LOGSTDOUT, "crfshttp_node_print:expire time: %d\n", CRFSHTTP_NODE_EXPIRE_TIME(crfshttp_node));
    sys_log(LOGSTDOUT, "crfshttp_node_print:header fsm : %u\n", CRFSHTTP_NODE_HEADER_FSM(crfshttp_node));

    sys_log(LOGSTDOUT, "crfshttp_node_print:store url : %s\n", (char *)cstring_get_str(CRFSHTTP_NODE_STORE_PATH(crfshttp_node)));

    return;
}

static EC_BOOL __crfshttp_node_prepare_for_csocket_cnode(CSOCKET_CNODE *csocket_cnode)
{
    CRFSHTTP_NODE *crfshttp_node;
   
    crfshttp_node = CSOCKET_CNODE_CRFSHTTP_NODE(csocket_cnode);
    if(NULL_PTR == crfshttp_node)
    {
        http_parser_t  *http_parser;
        
        crfshttp_node = crfshttp_node_new(CRFSHTTP_CBUFFER_SIZE);
        if(NULL_PTR == crfshttp_node)
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_node_prepare_for_csocket_cnode: new crfshttp_node failed\n");
            return (EC_FALSE);
        }
        
        CSOCKET_CNODE_CRFSHTTP_NODE(csocket_cnode) = crfshttp_node;
        CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node) = csocket_cnode;

        CTMV_CLONE(task_brd_default_get_daytime(), CRFSHTTP_NODE_START_TMV(crfshttp_node));
        
        http_parser = CRFSHTTP_NODE_PARSER(crfshttp_node);
        http_parser->data = (void *)crfshttp_node;/*xxx*/
    }

    return (EC_TRUE);
}

static EC_BOOL __crfshttp_node_pre_handle_header(CRFSHTTP_NODE  *crfshttp_node)
{
    CSOCKET_CNODE *csocket_cnode;
    
    csocket_cnode = CRFSHTTP_NODE_CSOCKET_CNODE(crfshttp_node);    
    
    /*check header validity*/
    if(CRFSHTTP_HEADER_MAX_SIZE < CRFSHTTP_NODE_HEADER_PARSED_LEN(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:__crfshttp_node_pre_handle_header: sockfd %d header too large where pased len %d\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode),
                           CRFSHTTP_NODE_HEADER_PARSED_LEN(crfshttp_node));
        return (EC_FALSE);
    }

    //dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_node_pre_handle_header: FSM = %u\n", CRFSHTTP_NODE_HEADER_FSM(crfshttp_node));

    /*FSM transition*/
    if(CRFSHTTP_NODE_HEADER_HANDLING == CRFSHTTP_NODE_HEADER_FSM(crfshttp_node))
    {
        CBUFFER        *uri_cbuffer;
        http_parser_t  *http_parser;        
        
        CRFSHTTP_NODE_HEADER_FSM(crfshttp_node) = CRFSHTTP_NODE_HEADER_HANDLED;

        uri_cbuffer  = CRFSHTTP_NODE_URI(crfshttp_node);        
        http_parser = CRFSHTTP_NODE_PARSER(crfshttp_node);

        if(HTTP_PUT == http_parser->method && EC_TRUE == __crfshttp_uri_is_setsmf_op(uri_cbuffer))
        {   
            uint8_t       *cache_key;
            uint32_t       cache_len;

            CSTRING        path_cstr;

            cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/setsmf");
            cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/setsmf");
            
            cstring_init(&path_cstr, NULL_PTR);
            cstring_append_chars(&path_cstr, cache_len, cache_key);            
            
            if(EC_TRUE == crfs_is_file(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr))
            {
                dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_node_pre_handle_header: file '%s' already exist\n", 
                                   (char *)cstring_get_str(&path_cstr));
                cstring_clean(&path_cstr);

                CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_FORBIDDEN;

                return crfshttp_commit_error_request(crfshttp_node);
            }   
            
            cstring_clean(&path_cstr);
        }        

        else if(HTTP_POST == http_parser->method && EC_TRUE == __crfshttp_uri_is_setsmf_op(uri_cbuffer))
        {           
            uint8_t       *cache_key;
            uint32_t       cache_len;

            CSTRING        path_cstr;

            cache_key = CBUFFER_DATA(uri_cbuffer) + CONST_STR_LEN("/setsmf");
            cache_len = CBUFFER_USED(uri_cbuffer) - CONST_STR_LEN("/setsmf");
            
            cstring_init(&path_cstr, NULL_PTR);
            cstring_append_chars(&path_cstr, cache_len, cache_key);            
            
            if(EC_TRUE == crfs_is_file(CSOCKET_CNODE_MODI(csocket_cnode), &path_cstr))
            {
                dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] __crfshttp_node_pre_handle_header: file '%s' already exist\n", 
                                   (char *)cstring_get_str(&path_cstr));
                cstring_clean(&path_cstr);

                CRFSHTTP_NODE_RSP_STATUS(crfshttp_node) = CRFSHTTP_FORBIDDEN;
                return crfshttp_commit_error_request(crfshttp_node);
            }   
            
            cstring_clean(&path_cstr);
        }        
    }    

    return (EC_TRUE);
}

EC_BOOL crfshttp_recv_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode)
{
    CRFSHTTP_NODE            *crfshttp_node;
    http_parser_t            *http_parser;
    http_parser_settings_t   *http_parser_setting;
    CBUFFER                  *http_buffer;
    
    UINT32   pos;
    uint32_t parsed_len;
    
    if(EC_FALSE == CSOCKET_CNODE_IS_CONNECTED(csocket_cnode))
    {
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_recv_on_csocket_cnode: sockfd %d is not connected\n", 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfshttp_node_prepare_for_csocket_cnode(csocket_cnode))
    {
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_recv_on_csocket_cnode: sockfd %d prepare crfshttp_node failed\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    crfshttp_node       = CSOCKET_CNODE_CRFSHTTP_NODE(csocket_cnode);
    http_parser         = CRFSHTTP_NODE_PARSER(crfshttp_node);
    http_parser_setting = CRFSHTTP_NODE_SETTING(crfshttp_node);
    http_buffer         = CRFSHTTP_NODE_CBUFFER(crfshttp_node);
    
    pos = CBUFFER_USED(http_buffer);
    if(EC_FALSE == csocket_read(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                CBUFFER_DATA(http_buffer), 
                                CBUFFER_ROOM(http_buffer), 
                                &pos))
    {
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_recv_on_csocket_cnode: read on sockfd %d failed where size %d and used %d\n",
                            CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                            CBUFFER_SIZE(http_buffer), 
                            CBUFFER_USED(http_buffer));
        return (EC_FALSE);                            
    }

    CBUFFER_USED(http_buffer) = (uint32_t)pos;

    parsed_len = http_parser_execute(http_parser, http_parser_setting, (char *)CBUFFER_DATA(http_buffer), CBUFFER_USED(http_buffer));
    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_recv_on_csocket_cnode: parsed_len = %u, http state %s\n", parsed_len, http_state_str(http_parser->state));
    cbuffer_left_shift_out(http_buffer, NULL_PTR, parsed_len);

    /*count header len*/
    switch(CRFSHTTP_NODE_HEADER_FSM(crfshttp_node))
    {
        case CRFSHTTP_NODE_HEADER_PARSING:
            CRFSHTTP_NODE_HEADER_PARSED_LEN(crfshttp_node) += parsed_len;
            break;
        case CRFSHTTP_NODE_HEADER_PARSED:
            CRFSHTTP_NODE_HEADER_PARSED_LEN(crfshttp_node) += parsed_len;
            CRFSHTTP_NODE_HEADER_FSM(crfshttp_node) = CRFSHTTP_NODE_HEADER_HANDLING;
            break;
        default:
            /*do nothing*/
            break;
    }

    if(EC_FALSE == __crfshttp_node_pre_handle_header(crfshttp_node))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_recv_on_csocket_cnode: sockfd %d header pre-handle failed\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDNULL, "[DEBUG] crfshttp_recv_on_csocket_cnode: sockfd %d parsed_len %d "
                        "=> header parsed %d, body parsed %ld\n", 
                       CSOCKET_CNODE_SOCKFD(csocket_cnode),
                       parsed_len, 
                       CRFSHTTP_NODE_HEADER_PARSED_LEN(crfshttp_node), CRFSHTTP_NODE_BODY_PARSED_LEN(crfshttp_node));
    dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDNULL, "[DEBUG] crfshttp_recv_on_csocket_cnode: sockfd %d http state %s\n", 
                       CSOCKET_CNODE_SOCKFD(csocket_cnode),
                       http_state_str(http_parser->state));

    if(s_start_req == http_parser->state || s_dead == http_parser->state)
    {
        /*note: http request is ready now. stop read from socket to prevent recving during handling request*/
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_RD_EVENT);
        /*commit*/
        /*return crfshttp_commit_request(crfshttp_node);*/
        return crfshttp_defer_request_queue_push(crfshttp_node);
    }
    /*TODO: other scenarios*/

    /*launch crountines for request*/
    //crfshttp_defer_request_queue_launch();
    
    return (EC_TRUE);
}

EC_BOOL crfshttp_send_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode)
{
    CRFSHTTP_NODE            *crfshttp_node;
    CHUNK_MGR                *body_chunks;
    
    if(EC_FALSE == CSOCKET_CNODE_IS_CONNECTED(csocket_cnode))
    {
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_send_on_csocket_cnode: sockfd %d is not connected\n", 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode));    
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);                           
        return (EC_FALSE);
    }

    crfshttp_node = CSOCKET_CNODE_CRFSHTTP_NODE(csocket_cnode);
    if(NULL_PTR == crfshttp_node)
    {
        /*nothing to do ??*/    
        crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
        dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_send_on_csocket_cnode: sockfd %d CSOCKET_CNODE_CRFSHTTP_NODE is null\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    body_chunks = CRFSHTTP_NODE_RSP_BODY_CHUNKS(crfshttp_node);

    while(EC_FALSE == chunk_mgr_is_empty(body_chunks))
    {
        CHUNK *chunk;
        UINT32 pos;
        
        chunk = chunk_mgr_first_chunk(body_chunks);
        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDNULL, "[DEBUG] crfshttp_send_on_csocket_cnode: sockfd %d chunk offset %d, buffer used %d\n",
                            CSOCKET_CNODE_SOCKFD(csocket_cnode),
                            CHUNK_OFFSET(chunk), CBUFFER_USED(CHUNK_BUFFER(chunk)));
        if(CHUNK_OFFSET(chunk) >= CBUFFER_USED(CHUNK_BUFFER(chunk)))
        {
            /*send completely*/
            chunk_mgr_pop_first_chunk(body_chunks);
            chunk_free(chunk);
            continue;
        }

        pos = CHUNK_OFFSET(chunk);
        if(EC_FALSE == csocket_write(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                   CBUFFER_DATA(CHUNK_BUFFER(chunk)), 
                                   CBUFFER_USED(CHUNK_BUFFER(chunk)),
                                   &pos))
        {
            dbg_log(SEC_0049_CRFSHTTP, 0)(LOGSTDOUT, "error:crfshttp_send_on_csocket_cnode: sockfd %d send %ld bytes failed\n",
                               CSOCKET_CNODE_SOCKFD(csocket_cnode),
                               CBUFFER_USED(CHUNK_BUFFER(chunk)) - CHUNK_OFFSET(chunk)
                               );
            /*cleanup possible WR epoll event*/
            cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);            
            crfshttp_csocket_cnode_defer_close_list_push(csocket_cnode);
            return (EC_FALSE);                           
        }

        CHUNK_OFFSET(chunk) = (uint32_t)pos;
        if(CHUNK_OFFSET(chunk) < CBUFFER_USED(CHUNK_BUFFER(chunk)))
        {
            /*wait for next writing*/
            cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                             (CSRV_WRITE_HANDLER)crfshttp_send_on_csocket_cnode, csocket_cnode);
            return (EC_TRUE);
        }

        dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_send_on_csocket_cnode: sockfd %d pop chunk and clean it, size %u\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode), CBUFFER_USED(CHUNK_BUFFER(chunk)));    
        
        /*chunk is sent completely*/
        chunk_mgr_pop_first_chunk(body_chunks);
        chunk_free(chunk);
    }

    if(EC_TRUE == chunk_mgr_is_empty(body_chunks))
    {
        if(NULL_PTR != CRFSHTTP_NODE_SEND_DATA_MORE(crfshttp_node))
        {
            cepoll_set_writer(task_brd_default_get_cepoll(), 
                              CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                              (CEPOLL_EVENT_HANDLER)CRFSHTTP_NODE_SEND_DATA_MORE(crfshttp_node), 
                              (void *)crfshttp_node);     
            /*wait for next writing*/
            cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                             (CSRV_WRITE_HANDLER)crfshttp_send_on_csocket_cnode, csocket_cnode);                              
        }
        else
        {
            dbg_log(SEC_0049_CRFSHTTP, 9)(LOGSTDOUT, "[DEBUG] crfshttp_send_on_csocket_cnode: sockfd %d del WR event and clean crfshttp_node\n",
                               CSOCKET_CNODE_SOCKFD(csocket_cnode));    
            /*cleanup possible WR epoll event*/
            crfshttp_node_free(crfshttp_node);    /*2014.09.14: when access non-existing file, crfshttp_node would not be free*/
            CSOCKET_CNODE_CRFSHTTP_NODE(csocket_cnode) = NULL_PTR;
        }
    }
    
    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

