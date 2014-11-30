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
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <sys/mman.h>

#include <sys/stat.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "carray.h"
#include "cvector.h"

#include "cbc.h"
#include "ctimer.h"
#include "cbtimer.h"
#include "cmisc.h"

#include "task.h"

#include "cmpie.h"

#include "cxmpp.inc"
#include "cxmpp.h"
#include "cexpat.h"
#include "cxmppc2s.h"

#include "cbase64code.h"
#include "findex.inc"

static const uint8_t *g_cxmpp_domain = (const uint8_t *)"example.com";

static uint8_t g_cxmpp_id[ 16 ]; /*generated randomly*/
static uint8_t g_cxmpp_push_id[ 32 ]; /*generated randomly*/

static uint8_t *cxmpp_id_new()
{
    uint8_t *id;
    int i;
    int r;

    id = (uint8_t *)g_cxmpp_id;
    
    /* as we are not using ids for tracking purposes, these can be generated randomly */
    for(i = 0; i < 10; i++) 
    {
        r = (int) (36.0 * rand() / RAND_MAX);
        id[i] = (r >= 0 && r <= 9) ? (r + 48) : (r + 87);
    }
    id[ i ] = '\0';
    return (g_cxmpp_id);
}

static uint8_t *cxmpp_push_id_new()
{
    uint8_t *id;
    int i;
    int r;

    id = (uint8_t *)g_cxmpp_push_id;
    id += sprintf((char *)id, "push");
    
    /* as we are not using ids for tracking purposes, these can be generated randomly */
    for(i = 0; i < 10; i++) 
    {
        r = (int) (36.0 * rand() / RAND_MAX);
        id[i] = (r >= 0 && r <= 9) ? (r + 48) : (r + 87);
    }
    id[ i ] = '\0';
    return (g_cxmpp_push_id);
}

CXMPP_NODE *cxmpp_node_new(const uint32_t size)
{
    CXMPP_NODE *cxmpp_node;

    dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_node_new: size = %d\n", size);

    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CXMPP_NODE, &cxmpp_node, LOC_CXMPP_0001);
    if(NULL_PTR == cxmpp_node)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_node_new: new cxmpp_node failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cxmpp_node_init(cxmpp_node, size))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_node_new: init cxmpp_node failed\n");
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CXMPP_NODE, cxmpp_node, LOC_CXMPP_0002);
        return (NULL_PTR);
    }

    return (cxmpp_node);
}

EC_BOOL cxmpp_node_init(CXMPP_NODE *cxmpp_node, const uint32_t size)
{
    if(EC_FALSE == cbuffer_init(CXMPP_NODE_RECV_CBUFFER(cxmpp_node), size))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_node_init: init recv cbuffer with size %d failed\n", size);
        return (EC_FALSE);
    }

    if(EC_FALSE == cbuffer_init(CXMPP_NODE_SEND_CBUFFER(cxmpp_node), 0))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_node_init: init send cbuffer with size 0 failed\n");
        return (EC_FALSE);
    }    

    cexpat_parser_init(CXMPP_NODE_XML_PARSER(cxmpp_node));

    CXMPP_NODE_USERNAME(cxmpp_node)            = NULL_PTR;
    
    CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node)    = NULL_PTR;
    CXMPP_NODE_SEND_CEXPAT_NODE(cxmpp_node)    = NULL_PTR;
    
    CXMPP_NODE_CSOCKET_CNODE(cxmpp_node)       = NULL_PTR;
    CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node)       = NULL_PTR;

    CXMPP_NODE_SEND_DATA_MORE_FUNC(cxmpp_node) = NULL_PTR;
    CXMPP_NODE_STATE(cxmpp_node)               = CXMPP_NODE_STATE_NONE;    

    CXMPP_NODE_SHAKEHAND(cxmpp_node)           = 0;

    return (EC_TRUE);
}

EC_BOOL cxmpp_node_clean(CXMPP_NODE *cxmpp_node)
{
    CSOCKET_CNODE *csocket_cnode;

    dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_node_clean: try to clean cxmpp_node %p\n", cxmpp_node);

    csocket_cnode = CXMPP_NODE_CSOCKET_CNODE(cxmpp_node);
    if(NULL_PTR != csocket_cnode)
    {
        CEPOLL *cepoll;

        cepoll = task_brd_default_get_cepoll();
        cepoll_del_all(cepoll, CSOCKET_CNODE_SOCKFD(csocket_cnode));
        cepoll_set_not_used(cepoll, CSOCKET_CNODE_SOCKFD(csocket_cnode));
    }

    if(NULL_PTR != CXMPP_NODE_USERNAME(cxmpp_node))
    {
        cstring_free(CXMPP_NODE_USERNAME(cxmpp_node));
        CXMPP_NODE_USERNAME(cxmpp_node) = NULL_PTR;
    }

    cexpat_parser_close(CXMPP_NODE_XML_PARSER(cxmpp_node));
    cexpat_parser_clean(CXMPP_NODE_XML_PARSER(cxmpp_node));

    if(NULL_PTR != CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node))
    {
        cexpat_node_free(CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node));
        CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node) = NULL_PTR;
    }

    if(NULL_PTR != CXMPP_NODE_SEND_CEXPAT_NODE(cxmpp_node))
    {
        cexpat_node_free(CXMPP_NODE_SEND_CEXPAT_NODE(cxmpp_node));
        CXMPP_NODE_SEND_CEXPAT_NODE(cxmpp_node) = NULL_PTR;
    }    

    cbuffer_clean(CXMPP_NODE_RECV_CBUFFER(cxmpp_node));
    cbuffer_clean(CXMPP_NODE_SEND_CBUFFER(cxmpp_node));
    
    CXMPP_NODE_CSOCKET_CNODE(cxmpp_node)       = NULL_PTR;

    CXMPP_NODE_SEND_DATA_MORE_FUNC(cxmpp_node) = NULL_PTR;
    CXMPP_NODE_STATE(cxmpp_node)               = CXMPP_NODE_STATE_NONE;
    CXMPP_NODE_SHAKEHAND(cxmpp_node)           = 0;

    if(NULL_PTR != CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node))
    {
        CXMPPC2S_CONN *cxmppc2s_conn;

        cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
        CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node) = NULL_PTR;

        cxmppc2s_rm_conn(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
        cxmppc2s_conn_free(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
    }

    if(NULL_PTR != csocket_cnode)
    {
        //not sure ....
        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_node_clean: try to close socket %d\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        csocket_cnode_close(csocket_cnode);/*when socket is closed, it may be reused at once*/
    }

    return (EC_TRUE);
}

EC_BOOL cxmpp_node_free(CXMPP_NODE *cxmpp_node)
{
    if(NULL_PTR != cxmpp_node)
    {
        cxmpp_node_clean(cxmpp_node);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CXMPP_NODE, cxmpp_node, LOC_CXMPP_0004);
    }

    return (EC_TRUE);
}

void cxmpp_csocket_cnode_epoll_close(CSOCKET_CNODE *csocket_cnode)
{
    CEPOLL *cepoll;
    
    cepoll = task_brd_default_get_cepoll();    
    cepoll_del_all(cepoll, CSOCKET_CNODE_SOCKFD(csocket_cnode));
    cepoll_set_not_used(cepoll, CSOCKET_CNODE_SOCKFD(csocket_cnode));
    
    if(NULL_PTR != CSOCKET_CNODE_CXMPP_NODE(csocket_cnode))
    {
        CXMPP_NODE *cxmpp_node;        
        cxmpp_node = CSOCKET_CNODE_CXMPP_NODE(csocket_cnode);  
        CXMPP_NODE_CSOCKET_CNODE(cxmpp_node)    = NULL_PTR;/*clean*/
        CSOCKET_CNODE_CXMPP_NODE(csocket_cnode) = NULL_PTR;/*clean*/
        cxmpp_node_free(cxmpp_node);
    }
    csocket_cnode_close(csocket_cnode);
    return;
}

void cxmpp_node_defer_close(CXMPP_NODE *cxmpp_node)
{
    CSOCKET_CNODE *csocket_cnode;
    CEPOLL *cepoll;
    
    cepoll = task_brd_default_get_cepoll();
    csocket_cnode = CXMPP_NODE_CSOCKET_CNODE(cxmpp_node);
    
    CXMPP_NODE_CSOCKET_CNODE(cxmpp_node) = NULL_PTR;/*clean*/
    CSOCKET_CNODE_CXMPP_NODE(csocket_cnode)  = NULL_PTR;/*clean*/

    cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_RD_EVENT);                      
    cepoll_set_event(cepoll, CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                      (CEPOLL_EVENT_HANDLER)cxmpp_csocket_cnode_epoll_close,(void *)csocket_cnode);    
    
    cxmpp_node_free(cxmpp_node);
    return;
}

void cxmpp_csocket_cnode_timeout(CSOCKET_CNODE *csocket_cnode)
{
#if 1
    dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_csocket_cnode_timeout: csocket_cnode %p sockfd %d node %p timeout, set defer close\n",
                        csocket_cnode, CSOCKET_CNODE_SOCKFD(csocket_cnode), CSOCKET_CNODE_CXMPP_NODE(csocket_cnode));
                        
    cxmpp_node_defer_close(CSOCKET_CNODE_CXMPP_NODE(csocket_cnode));
#endif
    /*TODO: send L7 heartbeat*/
    return;
}

void cxmpp_csocket_cnode_shutdown(CSOCKET_CNODE *csocket_cnode)
{
#if 1
    dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_csocket_cnode_close: csocket_cnode %p sockfd %d node %p shutdown, set defer close\n",
                        csocket_cnode, CSOCKET_CNODE_SOCKFD(csocket_cnode), CSOCKET_CNODE_CXMPP_NODE(csocket_cnode));
                        
    cxmpp_node_defer_close(CSOCKET_CNODE_CXMPP_NODE(csocket_cnode));
#endif
    /*TODO: send L7 heartbeat*/
    return;
}

void cxmpp_node_print(LOG *log, const CXMPP_NODE *cxmpp_node)
{
    //TODO:

    return;
}

/*
    <mechanism>PLAIN</mechanism>
*/
static CEXPAT_NODE *__cxmpp_make_mechanism_cexpat_node(const uint8_t *mechanism)
{
    CEXPAT_NODE *cexpat_node;
    CBYTES      *cdata;

    cdata = cbytes_make_by_str(mechanism);

    cexpat_node = cexpat_node_make((const uint8_t *)"mechanism");
    cexpat_node_set_cdata(cexpat_node, cdata);

    return (cexpat_node);
}

/*
   <mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>
       <mechanism>PLAIN</mechanism>
       <mechanism>DIGEST-MD5</mechanism>
       <mechanism>SCRAM-SHA-1</mechanism>
   </mechanisms>
*/

static CEXPAT_NODE *__cxmpp_make_mechanisms_cexpat_node()
{
    CEXPAT_NODE *cexpat_node;
    CEXPAT_NODE *cexpat_node_child;

    cexpat_node = cexpat_node_make((const uint8_t *)"mechanisms");
    cexpat_node_add_attr(cexpat_node, (const uint8_t *)"xmlns", (const uint8_t *)"urn:ietf:params:xml:ns:xmpp-sasl");

    cexpat_node_child = __cxmpp_make_mechanism_cexpat_node((const uint8_t *)"PLAIN");
    cexpat_node_add_child(cexpat_node, cexpat_node_child);

    //cexpat_node_child = __cxmpp_make_mechanism_cexpat_node((const uint8_t *)"DIGEST-MD5");
    //cexpat_node_add_child(cexpat_node, cexpat_node_child);    

    //cexpat_node_child = __cxmpp_make_mechanism_cexpat_node((const uint8_t *)"SCRAM-SHA-1");
    //cexpat_node_add_child(cexpat_node, cexpat_node_child);  

    return (cexpat_node);
}

/*
<c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='/nWL9StXSXhEsL2wg0+s4xo/UdA='/>
*/
static CEXPAT_NODE *__cxmpp_make_c_cexpat_node()
{
    CEXPAT_NODE *cexpat_node;

    cexpat_node = cexpat_node_make((const uint8_t *)"c");
    cexpat_node_add_attr(cexpat_node, (const uint8_t *)"xmlns", (const uint8_t *)"http://jabber.org/protocol/caps");
    cexpat_node_add_attr(cexpat_node, (const uint8_t *)"hash", (const uint8_t *)"sha-1");
    cexpat_node_add_attr(cexpat_node, (const uint8_t *)"node", (const uint8_t *)"http://www.process-one.net/en/ejabberd/");
    cexpat_node_add_attr(cexpat_node, (const uint8_t *)"ver", (const uint8_t *)"/nWL9StXSXhEsL2wg0+s4xo/UdA=");

    return (cexpat_node);
}

/*
<register xmlns='http://jabber.org/features/iq-register'/>
*/
static CEXPAT_NODE *__cxmpp_make_register_cexpat_node()
{
    CEXPAT_NODE *cexpat_node;

    cexpat_node = cexpat_node_make((const uint8_t *)"register");
    cexpat_node_add_attr(cexpat_node, (const uint8_t *)"xmlns", (const uint8_t *)"http://jabber.org/features/iq-register");

    return (cexpat_node);
}


/*
   <stream:features>
       <mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>
           <mechanism>PLAIN</mechanism>
           <mechanism>DIGEST-MD5</mechanism>
           <mechanism>SCRAM-SHA-1</mechanism>
       </mechanisms>
   <c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='/nWL9StXSXhEsL2wg0+s4xo/UdA='/>
   <register xmlns='http://jabber.org/features/iq-register'/>
   </stream:features>
*/
static CEXPAT_NODE *__cxmpp_make_features_list_cexpat_node()
{
    CEXPAT_NODE *cexpat_node;
    CEXPAT_NODE *cexpat_node_child;

    cexpat_node = cexpat_node_make((const uint8_t *)"stream:features");
    
    cexpat_node_child = __cxmpp_make_mechanisms_cexpat_node();
    cexpat_node_add_child(cexpat_node, cexpat_node_child);  

    cexpat_node_child = __cxmpp_make_c_cexpat_node();
    cexpat_node_add_child(cexpat_node, cexpat_node_child);  

    cexpat_node_child = __cxmpp_make_register_cexpat_node();
    cexpat_node_add_child(cexpat_node, cexpat_node_child);  
    return (cexpat_node);
}

/*
<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'/>
*/
static CEXPAT_NODE *__cxmpp_make_bind_cexpat_node()
{
    CEXPAT_NODE *cexpat_node;

    cexpat_node = cexpat_node_make((const uint8_t *)"bind");
    cexpat_node_add_attr(cexpat_node, (const uint8_t *)"xmlns", (const uint8_t *)"urn:ietf:params:xml:ns:xmpp-bind");
    return (cexpat_node);
}

/*
<jid>user1000@example.com/pandion</jid>
*/
static CEXPAT_NODE *__cxmpp_make_jid_cexpat_node(CXMPPC2S_CONN *cxmppc2s_conn)
{
    CEXPAT_NODE *cexpat_node;
    CSTRING     *jid_cstr;
    CBYTES      *jid_cdata;
    
    cexpat_node = cexpat_node_make((const uint8_t *)"jid");

    jid_cstr = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
   
    jid_cdata = cbytes_make_by_cstr(jid_cstr);  

    cstring_free(jid_cstr);

    cexpat_node_set_cdata(cexpat_node, jid_cdata);
    return (cexpat_node);
}

static CEXPAT_NODE *__cxmpp_make_jid_cexpat_node0(const uint8_t *username, const uint32_t resource_len, const uint8_t *resource_bytes)
{
    CEXPAT_NODE *cexpat_node;
    uint8_t      jid_str[256];
    UINT32       len;
    CBYTES      *jid_cdata;

    cexpat_node = cexpat_node_make((const uint8_t *)"jid");

    len = snprintf((char *)jid_str, sizeof(jid_str)/sizeof(jid_str[0]), "%s/%.*s", (const char *)username, resource_len, (const char *)resource_bytes);
    jid_cdata = cbytes_make_by_bytes(len, jid_str);  

    cexpat_node_set_cdata(cexpat_node, jid_cdata);
    return (cexpat_node);
}

/*
<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>
*/
static CEXPAT_NODE *__cxmpp_make_session_cexpat_node()
{
    CEXPAT_NODE *cexpat_node;

    cexpat_node = cexpat_node_make((const uint8_t *)"session");
    cexpat_node_add_attr(cexpat_node, (const uint8_t *)"xmlns", (const uint8_t *)"urn:ietf:params:xml:ns:xmpp-session");
    return (cexpat_node);
}

/*
<sm xmlns='urn:xmpp:sm:2'/>
<sm xmlns='urn:xmpp:sm:3'/>
*/
static CEXPAT_NODE *__cxmpp_make_sm_cexpat_node(const UINT32 sm_id)
{
    CEXPAT_NODE *cexpat_node;
    uint8_t attr_val[64];

    snprintf((char *)attr_val, sizeof(attr_val)/sizeof(attr_val[0]), "urn:xmpp:sm:%ld", sm_id);

    cexpat_node = cexpat_node_make((const uint8_t *)"sm");
    cexpat_node_add_attr(cexpat_node, (const uint8_t *)"xmlns", (const uint8_t *)attr_val);
    return (cexpat_node);
}

/*
     <stream:features>
       <bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'/>
       <session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>
       <sm xmlns='urn:xmpp:sm:2'/>
       <sm xmlns='urn:xmpp:sm:3'/>
       <c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='/nWL9StXSXhEsL2wg0+s4xo/UdA='/>
       <register xmlns='http://jabber.org/features/iq-register'/>
      </stream:features>
*/
static CEXPAT_NODE *__cxmpp_make_features_bind_cexpat_node()
{
    CEXPAT_NODE *cexpat_node;
    CEXPAT_NODE *cexpat_node_child;

    cexpat_node = cexpat_node_make((const uint8_t *)"stream:features");
    
    cexpat_node_child = __cxmpp_make_bind_cexpat_node();
    cexpat_node_add_child(cexpat_node, cexpat_node_child);  

    cexpat_node_child = __cxmpp_make_session_cexpat_node();
    cexpat_node_add_child(cexpat_node, cexpat_node_child);      

    cexpat_node_child = __cxmpp_make_sm_cexpat_node(2);
    cexpat_node_add_child(cexpat_node, cexpat_node_child);   

    cexpat_node_child = __cxmpp_make_sm_cexpat_node(3);
    cexpat_node_add_child(cexpat_node, cexpat_node_child);      

    cexpat_node_child = __cxmpp_make_c_cexpat_node();
    cexpat_node_add_child(cexpat_node, cexpat_node_child);  

    cexpat_node_child = __cxmpp_make_register_cexpat_node();
    cexpat_node_add_child(cexpat_node, cexpat_node_child);  
    return (cexpat_node);
}

EC_BOOL cxmpp_handle_state_none(CXMPP_NODE *cxmpp_node)
{
    CEXPAT_PARSER  *cexpat_parser;

    cexpat_parser = CXMPP_NODE_XML_PARSER(cxmpp_node);
    if(CEXPAT_PARSE_HEADER_NOT_DONE == CEXPAT_PARSER_HEADER_DONE(cexpat_parser))
    {
        /*nothing to do, wait for next reading*/
        return (EC_TRUE);
    }

    /*else*/
    CXMPP_NODE_STATE(cxmpp_node) = CXMPP_NODE_STATE_STREAM_LIST_FEATURE;
    return (EC_TRUE);
}

/**
C: <stream:stream to="example.com" xml:lang="zh-cn" xmlns="jabber:client" xmlns:stream="http://etherx.jabber.org/streams" version="1.0">
S: <?xml version='1.0'?>
   <stream:stream id='2533490523' from='example.com' xml:lang='zh-cn' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='1.0' >
       <stream:features>
           <mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>
               <mechanism>PLAIN</mechanism>
               <mechanism>DIGEST-MD5</mechanism>
               <mechanism>SCRAM-SHA-1</mechanism>
           </mechanisms>
       <c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='/nWL9StXSXhEsL2wg0+s4xo/UdA='/>
       <register xmlns='http://jabber.org/features/iq-register'/>
       </stream:features>
**/
EC_BOOL cxmpp_make_state_stream_list_feature(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_stream_node)
{
    CEXPAT_NODE       *send_stream_node;
    CEXPAT_NODE       *features_stream_node;

    send_stream_node = cexpat_node_make((const uint8_t *)"stream:stream");
    if(NULL_PTR == send_stream_node)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_make_state_stream_list_feature: make send_stream_node failed\n");
        return (EC_FALSE);
    }
    
    cexpat_node_add_attr(send_stream_node, (const uint8_t *)"id", (const uint8_t *)cxmpp_id_new());
    cexpat_node_xclone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"to", (const uint8_t *)"from"); /*clone 'to' to 'from'*/
    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"xml:lang");
    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"xmlns");
    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"xmlns:stream");
    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"version");

    features_stream_node = __cxmpp_make_features_list_cexpat_node();
    cexpat_node_add_child(send_stream_node, features_stream_node);  

    cexpat_node_encode_xml(send_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_FALSE/*not closed scope*/);

    cexpat_node_free(send_stream_node);

    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_state_stream_list_feature(CXMPP_NODE *cxmpp_node)
{
    CEXPAT_NODE       *recv_cexpat_node;
    const CEXPAT_NODE *recv_stream_node;
    const CSTRING     *to;
    
    uint32_t           len;

    recv_cexpat_node = CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node);
    if(NULL_PTR == recv_cexpat_node)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_stream_list_feature: recv_cexpat_node is null\n");
        return (EC_FALSE);
    }

    recv_stream_node = cexpat_find_node_by_path(recv_cexpat_node, (const uint8_t *)"stream:stream", (const uint8_t *)"|");
    if(NULL_PTR == recv_stream_node)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_handle_state_stream_list_feature: stream:stream not reached yet\n");
        return (EC_TRUE);
    }

    to = cexpat_find_attr(recv_stream_node, (const uint8_t *)"to");
    if(NULL_PTR == to)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_handle_state_stream_list_feature: stream:stream|to not reached yet\n");
        return (EC_TRUE);
    }

    /*check I am the target server*/
    //TODO:

    /*yes*/
    len = cbuffer_append(CXMPP_NODE_SEND_CBUFFER(cxmpp_node), CONST_UINT8_STR_AND_UINT32T_LEN("<?xml version='1.0'?>"));

    if(EC_FALSE == cxmpp_make_state_stream_list_feature(cxmpp_node, recv_stream_node))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_stream_list_feature: make list features failed\n");
        cbuffer_left_shift_out(CXMPP_NODE_SEND_CBUFFER(cxmpp_node), NULL_PTR, (uint32_t)len);
        return (EC_FALSE);
    }

    if(do_log(SEC_0147_CXMPP, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_state_stream_list_feature: send buffer is\n");
        cbuffer_print_str(LOGSTDOUT, CXMPP_NODE_SEND_CBUFFER(cxmpp_node));
    }
   
    if(EC_FALSE == cbuffer_is_empty(CXMPP_NODE_SEND_CBUFFER(cxmpp_node)))
    {
        CSOCKET_CNODE     *csocket_cnode;
        
        csocket_cnode = CXMPP_NODE_CSOCKET_CNODE(cxmpp_node);
        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)cxmpp_send_on_csocket_cnode_thread,(void *)csocket_cnode);   

        CXMPP_NODE_STATE(cxmpp_node) = CXMPP_NODE_STATE_STREAM_AUTH;
    }
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_auth_plain(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_stream_node)
{
    CEXPAT_NODE       *send_stream_node;
    CBYTES            *cdata;    
    UINT8              out[256];
    UINT32             out_len;

/**
C: <auth xmlns="urn:ietf:params:xml:ns:xmpp-sasl" mechanism="SCRAM-SHA-1">biwsbj11c2VyMTAwMCxyPUNJVkk0dHdXRnZVVHZsM0h1aXNRSGVmcmVZL3FOeWFI</auth>
S: <success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>dj1razBpSUUzNzA1Zm9aUVhMalVzTFlRNjRoeG89</success>
**/
    cdata = CEXPAT_NODE_CDATA(recv_stream_node);
    if(NULL_PTR == cdata)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_auth_plain: no cdata\n");
        return (EC_FALSE);
    }

    cbase64_decode(CBYTES_BUF(cdata), CBYTES_LEN(cdata), out, sizeof(out)/sizeof(out[0]), &out_len);
    dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle_auth_plain: decoded result: %.*s\n", out_len - 2, (char *)(out + 1));

    CXMPP_NODE_USERNAME(cxmpp_node) = cstring_make_by_bytes(out_len - 2, &(out[ 1 ]));
    if(NULL_PTR == CXMPP_NODE_USERNAME(cxmpp_node))
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_auth_plain: make username '%.*s' failed\n", out_len - 2, &(out[ 1 ]));
        return (EC_FALSE);
    }

    send_stream_node = cexpat_node_make((const uint8_t *)"success");
    if(NULL_PTR == send_stream_node)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_auth_plain: make send_stream_node failed\n");
        return (EC_FALSE);
    }

    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"xmlns");
    //cexpat_node_set_cdata(send_stream_node, cdata);
    
    cexpat_node_encode_xml(send_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*not closed scope*/);

    cexpat_node_free(send_stream_node);

    if(do_log(SEC_0147_CXMPP, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_auth_plain: send buffer is\n");
        cbuffer_print_str(LOGSTDOUT, CXMPP_NODE_SEND_CBUFFER(cxmpp_node));
    }
 
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_auth_md5(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_stream_node)
{
    CEXPAT_NODE       *send_stream_node;
    CBYTES            *cdata;

/**
C: <auth xmlns="urn:ietf:params:xml:ns:xmpp-sasl" mechanism="SCRAM-SHA-1">biwsbj11c2VyMTAwMCxyPUNJVkk0dHdXRnZVVHZsM0h1aXNRSGVmcmVZL3FOeWFI</auth>
S: <challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>cj1OM2FjY2svOWFuam1HbldRSG84alhDSisweUl4YW5MTFJvVEpQMEd5TkNqMmhvcWhiL0lmdnc9PSxzPU8waDVzUWNSZXpxMzRZdWlvWlpVNXc9PSxpPTQwOTY=</challenge>
**/
    send_stream_node = cexpat_node_make((const uint8_t *)"challenge");
    if(NULL_PTR == send_stream_node)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_auth_md5: make send_stream_node failed\n");
        return (EC_FALSE);
    }

    //cdata = cbytes_make_by_str((const uint8_t *)"cj1OM2FjY2svOWFuam1HbldRSG84alhDSisweUl4YW5MTFJvVEpQMEd5TkNqMmhvcWhiL0lmdnc9PSxzPU8waDVzUWNSZXpxMzRZdWlvWlpVNXc9PSxpPTQwOTY=");
    cdata = cbytes_make_by_str((const uint8_t *)"AHVzZXIxMDAwAHBhc3N3ZDEwMDA=");
    if(NULL_PTR == cdata)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_auth_md5: make cdata failed\n");
        cexpat_node_free(send_stream_node);
        return (EC_FALSE);
    }

    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"xmlns");
    cexpat_node_set_cdata(send_stream_node, cdata);
    
    cexpat_node_encode_xml(send_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*not closed scope*/);

    cexpat_node_free(send_stream_node);

    if(do_log(SEC_0147_CXMPP, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_auth_md5: send buffer is\n");
        cbuffer_print_str(LOGSTDOUT, CXMPP_NODE_SEND_CBUFFER(cxmpp_node));
    }
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_auth(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_stream_node, const CSTRING *mechanism)
{
    if(EC_TRUE == cstring_is_str_ignore_case(mechanism, (const uint8_t *)"PLAIN"))
    {
        if(EC_FALSE == cxmpp_handle_auth_plain(cxmpp_node, recv_stream_node))
        {
            /*not reached yet, wait for next reading*/
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_auth: handle auth plain failed\n");
            return (EC_FALSE);
        }

        return (EC_TRUE);
    }
    
    if(EC_TRUE == cstring_is_str_ignore_case(mechanism, (const uint8_t *)"DIGEST-MD5"))
    {
        if(EC_FALSE == cxmpp_handle_auth_md5(cxmpp_node, recv_stream_node))
        {
            /*not reached yet, wait for next reading*/
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_auth: handle auth md5 failed\n");
            return (EC_FALSE);
        } 
        
        return (EC_TRUE);
    }
    
    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_auth: unknown mechanism %s\n", (char *)cstring_get_str(mechanism));
    return (EC_FALSE);
}

/*
C: <auth xmlns="urn:ietf:params:xml:ns:xmpp-sasl" mechanism="SCRAM-SHA-1">biwsbj11c2VyMTAwMCxyPUNJVkk0dHdXRnZVVHZsM0h1aXNRSGVmcmVZL3FOeWFI</auth>
S: <challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>cj1OM2FjY2svOWFuam1HbldRSG84alhDSisweUl4YW5MTFJvVEpQMEd5TkNqMmhvcWhiL0lmdnc9PSxzPU8waDVzUWNSZXpxMzRZdWlvWlpVNXc9PSxpPTQwOTY=</challenge>
*/
EC_BOOL cxmpp_handle_state_stream_auth(CXMPP_NODE *cxmpp_node)
{
    CEXPAT_NODE       *recv_cexpat_node;
    const CEXPAT_NODE *recv_stream_node;
    const CSTRING     *mechanism;
        
    recv_cexpat_node = CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node);
    if(NULL_PTR == recv_cexpat_node)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_stream_auth: recv_cexpat_node is null\n");
        return (EC_FALSE);
    }

    recv_stream_node = cexpat_find_node_by_path(recv_cexpat_node, (const uint8_t *)"stream:stream|auth", (const uint8_t *)"|");
    if(NULL_PTR == recv_stream_node)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_handle_state_stream_auth: stream:stream|auth not reached yet\n");
        return (EC_TRUE);
    }

    mechanism = cexpat_find_attr(recv_stream_node, (const uint8_t *)"mechanism");
    if(NULL_PTR == mechanism)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_handle_state_stream_auth: stream:stream|auth|mechanism not reached yet\n");
        return (EC_TRUE);
    }

    if(EC_FALSE == cxmpp_handle_auth(cxmpp_node, recv_stream_node, mechanism))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_stream_auth: handle auth '%s' failed\n", 
                           (char *)cstring_get_str(mechanism));
        return (EC_FALSE);
    }
   
    if(EC_FALSE == cbuffer_is_empty(CXMPP_NODE_SEND_CBUFFER(cxmpp_node)))
    {
        CSOCKET_CNODE     *csocket_cnode;
        
        csocket_cnode = CXMPP_NODE_CSOCKET_CNODE(cxmpp_node);
        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)cxmpp_send_on_csocket_cnode_thread,(void *)csocket_cnode);   

        /*after auth is OK*/
        CXMPP_NODE_STATE(cxmpp_node) = CXMPP_NODE_STATE_STREAM_BIND_FEATURE;

        /*clean old scenario*/
        cexpat_node_free(CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node));
        CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node) = NULL_PTR;
        cexpat_parser_clear(CXMPP_NODE_XML_PARSER(cxmpp_node));
    }
    return (EC_TRUE);
}

/**
C: <stream:stream to="example.com" xml:lang="zh-cn" xmlns="jabber:client" xmlns:stream="http://etherx.jabber.org/streams" version="1.0">
S: <?xml version='1.0'?>
   <stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' id='4181238451' from='example.com' version='1.0' xml:lang='en'>
     <stream:features>
       <bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'/>
       <session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>
       <sm xmlns='urn:xmpp:sm:2'/><sm xmlns='urn:xmpp:sm:3'/>
       <c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://www.process-one.net/en/ejabberd/' ver='/nWL9StXSXhEsL2wg0+s4xo/UdA='/>
       <register xmlns='http://jabber.org/features/iq-register'/>
      </stream:features>
**/

EC_BOOL cxmpp_make_state_stream_bind_feature(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_stream_node)
{
    CEXPAT_NODE       *send_stream_node;
    CEXPAT_NODE       *features_bind_node;
    
    send_stream_node = cexpat_node_make((const uint8_t *)"stream:stream");
    if(NULL_PTR == send_stream_node)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_make_state_stream_bind_feature: make send_stream_node failed\n");
        return (EC_FALSE);
    }   

    cexpat_node_add_attr(send_stream_node, (const uint8_t *)"id", (const uint8_t *)cxmpp_id_new());
    cexpat_node_xclone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"to", (const uint8_t *)"from");
    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"xml:lang");
    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"xmlns");
    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"xmlns:stream");
    cexpat_node_clone_attr(recv_stream_node, send_stream_node, (const uint8_t *)"version");

    features_bind_node = __cxmpp_make_features_bind_cexpat_node();
    cexpat_node_add_child(send_stream_node, features_bind_node);
    
    cexpat_node_encode_xml(send_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_FALSE/*not closed scope*/);

    cexpat_node_free(send_stream_node);

    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_state_stream_bind_feature(CXMPP_NODE *cxmpp_node)
{
    CEXPAT_NODE       *recv_cexpat_node;
    const CEXPAT_NODE *recv_stream_node;
    const CSTRING     *to;

    uint32_t           len;

    recv_cexpat_node = CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node);
    if(NULL_PTR == recv_cexpat_node)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_stream_bind_feature: recv_cexpat_node is null\n");
        return (EC_FALSE);
    }

    recv_stream_node = cexpat_find_node_by_path(recv_cexpat_node, (const uint8_t *)"stream:stream", (const uint8_t *)"|");
    if(NULL_PTR == recv_stream_node)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_handle_state_stream_bind_feature: stream:stream not reached yet\n");
        return (EC_TRUE);
    }

    to = cexpat_find_attr(recv_stream_node, (const uint8_t *)"to");
    if(NULL_PTR == to)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_handle_state_stream_bind_feature: stream:stream|to not reached yet\n");
        return (EC_TRUE);
    }

    /*check I am the target server*/
    //TODO:

    /*yes*/
    len = cbuffer_append(CXMPP_NODE_SEND_CBUFFER(cxmpp_node), CONST_UINT8_STR_AND_UINT32T_LEN("<?xml version='1.0'?>"));

    if(EC_FALSE == cxmpp_make_state_stream_bind_feature(cxmpp_node, recv_stream_node))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_stream_bind_feature: make bind features failed\n");
        cbuffer_left_shift_out(CXMPP_NODE_SEND_CBUFFER(cxmpp_node), NULL_PTR, (uint32_t)len);
        return (EC_FALSE);
    }

    if(do_log(SEC_0147_CXMPP, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_state_stream_bind_feature: send buffer is\n");
        cbuffer_print_str(LOGSTDOUT, CXMPP_NODE_SEND_CBUFFER(cxmpp_node));
    }
   
    if(EC_FALSE == cbuffer_is_empty(CXMPP_NODE_SEND_CBUFFER(cxmpp_node)))
    {
        CSOCKET_CNODE     *csocket_cnode;
        
        csocket_cnode = CXMPP_NODE_CSOCKET_CNODE(cxmpp_node);
        /*cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_RD_EVENT);*//*disable read*/
        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)cxmpp_send_on_csocket_cnode_thread,(void *)csocket_cnode);   

        CXMPP_NODE_STATE(cxmpp_node) = CXMPP_NODE_STATE_STREAM_ONGOING;
    }
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_state_stream_ongoing(CXMPP_NODE *cxmpp_node)
{   
    return cxmpp_handle_state(cxmpp_node);
}

EC_BOOL cxmpp_handle_state_stream_closing(CXMPP_NODE *cxmpp_node)
{
    cxmpp_csocket_cnode_epoll_close(CXMPP_NODE_CSOCKET_CNODE(cxmpp_node));
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_state_stream_closed(CXMPP_NODE *cxmpp_node)
{
    return (EC_TRUE);
}

/**
C: <iq type="get" id="sd3" to="example.com"><query xmlns="http://jabber.org/protocol/disco#items"/></iq>
S: <iq from='example.com' to='user1000@example.com/\xe6\xbd\x98\xe8\xbf\xaa\xe5\xae\x89' id='sd3' type='result'>
     <query xmlns='http://jabber.org/protocol/disco#items'>
        <item jid='conference.example.com'/>
        <item jid='irc.example.com'/>
        <item jid='pubsub.example.com'/>
        <item jid='vjud.example.com'/>
     </query>
   </iq>
**/

EC_BOOL cxmpp_make_iq_get_disco(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *query_stream_node, CEXPAT_NODE **send_stream_node)
{
    CEXPAT_NODE       *send_query_stream_node;
    CEXPAT_NODE       *send_item_stream_node;

    send_query_stream_node = cexpat_node_make((const uint8_t *)"query");
    cexpat_node_clone_attr(query_stream_node, send_query_stream_node, (const uint8_t *)"xmlns");

    send_item_stream_node = cexpat_node_make((const uint8_t *)"item");
    cexpat_node_add_attr(send_item_stream_node, (const uint8_t *)"jid", (const uint8_t *)"pubsub.example.com");
    
    cexpat_node_add_child(send_query_stream_node, send_item_stream_node);    

    (*send_stream_node) = send_query_stream_node;
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_iq_get_query_disco(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node)
{
    const CEXPAT_NODE *query_stream_node;
    const CSTRING     *query_xmlns_attr;

    query_stream_node = cexpat_find_node_by_path(recv_iq_stream_node, (const uint8_t *)"iq|query", (const uint8_t *)"|");
    if(NULL_PTR == query_stream_node)
    {
        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_get_query_disco: query node not reached yet\n");
        return (EC_TRUE);
    }

    query_xmlns_attr = cexpat_find_attr(query_stream_node, (const uint8_t *)"xmlns");
    if(NULL_PTR == query_xmlns_attr)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_disco: query attr 'xmlns' not reached yet\n");
        return (EC_TRUE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(query_xmlns_attr, (const uint8_t *)"http://jabber.org/protocol/disco#items"))
    {
        CEXPAT_NODE *send_query_stream_node;
        CEXPAT_NODE *send_iq_stream_node;

        CXMPPC2S_CONN *cxmppc2s_conn;
        CSTRING       *jid;
    
        send_query_stream_node = NULL_PTR;
        if(EC_FALSE == cxmpp_make_iq_get_disco(cxmpp_node, query_stream_node, &send_query_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_disco: make iq|set|query failed\n");
            return (EC_FALSE);
        }

        cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
        jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);

        send_iq_stream_node = cexpat_node_make((const uint8_t *)"iq");
        if(NULL_PTR == send_iq_stream_node)
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_disco: make iq node failed\n");
            cexpat_node_free(send_query_stream_node);
            return (EC_FALSE);
        }
        
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"type", (const uint8_t *)"result");
        cexpat_node_clone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"id");

        cexpat_node_xclone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"to", (const uint8_t *)"from");
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"to", (const uint8_t *)cstring_get_str(jid));

        cexpat_node_add_child(send_iq_stream_node, send_query_stream_node);

        cstring_free(jid);

        cexpat_node_encode_xml(send_iq_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*closed scope*/);
        cexpat_node_free(send_iq_stream_node);        
        
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_disco: query attr xmlns='%s' is unknown\n",
                       (char *)cstring_get_str(query_xmlns_attr));
    return (EC_FALSE);
}

/**
C: <iq type="get" id="sd4"><query xmlns="jabber:iq:roster"/>
S: <iq from='user1000@example.com' to='user1000@example.com/.........' id='sd4' type='result'>
      <query xmlns='jabber:iq:roster'>
         <item ask='subscribe' subscription='none' jid='user1001@example.com'/>
      </query>
   </iq>
**/

EC_BOOL cxmpp_make_iq_get_roster(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *query_stream_node, CEXPAT_NODE **send_stream_node)
{
    CEXPAT_NODE   *send_query_stream_node;
    CEXPAT_NODE   *send_item_stream_node;

    CXMPPC2S_CONN *cxmppc2s_conn;
    CSTRING       *jid;    

    cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
    jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
        
    send_query_stream_node = cexpat_node_make((const uint8_t *)"query");
    cexpat_node_clone_attr(query_stream_node, send_query_stream_node, (const uint8_t *)"xmlns");
   
    send_item_stream_node = cexpat_node_make((const uint8_t *)"item");
    cexpat_node_add_attr(send_item_stream_node, (const uint8_t *)"ask", (const uint8_t *)"subscribe");
    cexpat_node_add_attr(send_item_stream_node, (const uint8_t *)"subscription", (const uint8_t *)"none");
    cexpat_node_add_attr(send_item_stream_node, (const uint8_t *)"jid", (const uint8_t *)cstring_get_str(jid));
    
    cexpat_node_add_child(send_query_stream_node, send_item_stream_node);    

    cstring_free(jid);

    (*send_stream_node) = send_query_stream_node;
    return (EC_TRUE);
}


EC_BOOL cxmpp_handle_iq_get_query_roster(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node)
{
    const CEXPAT_NODE *query_stream_node;
    const CSTRING     *query_xmlns_attr;

    query_stream_node = cexpat_find_node_by_path(recv_iq_stream_node, (const uint8_t *)"iq|query", (const uint8_t *)"|");
    if(NULL_PTR == query_stream_node)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_roster: query node not reached yet\n");
        return (EC_TRUE);
    }

    query_xmlns_attr = cexpat_find_attr(query_stream_node, (const uint8_t *)"xmlns");
    if(NULL_PTR == query_xmlns_attr)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_roster: query attr 'xmlns' not reached yet\n");
        return (EC_TRUE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(query_xmlns_attr, (const uint8_t *)"jabber:iq:roster"))
    {
        CEXPAT_NODE *send_query_stream_node;
        CEXPAT_NODE *send_iq_stream_node;

        CXMPPC2S_CONN *cxmppc2s_conn;
        CSTRING       *jid;    
        CSTRING       *jid_no_resource;    
    
        send_query_stream_node = NULL_PTR;
        if(EC_FALSE == cxmpp_make_iq_get_roster(cxmpp_node, query_stream_node, &send_query_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_roster: make iq|set|query failed\n");
            return (EC_FALSE);
        }
    
        send_iq_stream_node = cexpat_node_make((const uint8_t *)"iq");
        if(NULL_PTR == send_iq_stream_node)
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_roster: make iq node failed\n");
            cexpat_node_free(send_query_stream_node);
            return (EC_FALSE);
        }

        cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
        jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);        
        jid_no_resource = cxmppc2s_conn_make_jid_no_resource(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);

        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"type", (const uint8_t *)"result");
        cexpat_node_clone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"id");
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"from", (const uint8_t *)cstring_get_str(jid_no_resource));
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"to", (const uint8_t *)cstring_get_str(jid));

        cexpat_node_add_child(send_iq_stream_node, send_query_stream_node);
        
        cstring_free(jid);
        cstring_free(jid_no_resource);
        
        cexpat_node_encode_xml(send_iq_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*closed scope*/);
        cexpat_node_free(send_iq_stream_node);
        
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_roster: query attr xmlns='%s' is unknown\n",
                       (char *)cstring_get_str(query_xmlns_attr));
    return (EC_FALSE);
}

/**
C: <iq type="get" id="sd5"><query xmlns="jabber:iq:privacy"/></iq>
S: <iq from='user1000@example.com' to='user1000@example.com/.........' id='sd5' type='result'>
      <query xmlns='jabber:iq:privacy'>
        <list name='invisible'/>
      </query>
   </iq>
**/

EC_BOOL cxmpp_make_iq_get_privacy(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *query_stream_node, CEXPAT_NODE **send_stream_node)
{
    CEXPAT_NODE       *send_query_stream_node;
    CEXPAT_NODE       *send_list_stream_node;

    send_query_stream_node = cexpat_node_make((const uint8_t *)"query");
    cexpat_node_clone_attr(query_stream_node, send_query_stream_node, (const uint8_t *)"xmlns");
   
    send_list_stream_node = cexpat_node_make((const uint8_t *)"list");
    cexpat_node_add_attr(send_list_stream_node, (const uint8_t *)"name", (const uint8_t *)"invisible");

    cexpat_node_add_child(send_query_stream_node, send_list_stream_node);    

    (*send_stream_node) = send_query_stream_node;
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_iq_get_query_privacy(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node)
{
    const CEXPAT_NODE *query_stream_node;
    const CSTRING     *query_xmlns_attr;

    query_stream_node = cexpat_find_node_by_path(recv_iq_stream_node, (const uint8_t *)"iq|query", (const uint8_t *)"|");
    if(NULL_PTR == query_stream_node)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_privacy: query node not reached yet\n");
        return (EC_TRUE);
    }

    query_xmlns_attr = cexpat_find_attr(query_stream_node, (const uint8_t *)"xmlns");
    if(NULL_PTR == query_xmlns_attr)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_privacy: query attr 'xmlns' not reached yet\n");
        return (EC_TRUE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(query_xmlns_attr, (const uint8_t *)"jabber:iq:privacy"))
    {
        CEXPAT_NODE *send_query_stream_node;
        CEXPAT_NODE *send_iq_stream_node;

        CXMPPC2S_CONN *cxmppc2s_conn;
        CSTRING       *jid;    
        CSTRING       *jid_no_resource;   
        
        send_query_stream_node = NULL_PTR;
        if(EC_FALSE == cxmpp_make_iq_get_privacy(cxmpp_node, query_stream_node, &send_query_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_privacy: make iq|get|query failed\n");
            return (EC_FALSE);
        }

        send_iq_stream_node = cexpat_node_make((const uint8_t *)"iq");
        if(NULL_PTR == send_iq_stream_node)
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_privacy: make iq node failed\n");
            cexpat_node_free(send_query_stream_node);
            return (EC_FALSE);
        }

        cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
        jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);        
        jid_no_resource = cxmppc2s_conn_make_jid_no_resource(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);

        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"type", (const uint8_t *)"result");
        cexpat_node_clone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"id");
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"from", (const uint8_t *)cstring_get_str(jid_no_resource));
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"to", (const uint8_t *)cstring_get_str(jid));

        cexpat_node_add_child(send_iq_stream_node, send_query_stream_node);

        cstring_free(jid);
        cstring_free(jid_no_resource);
        
        cexpat_node_encode_xml(send_iq_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*closed scope*/);
        cexpat_node_free(send_iq_stream_node);
        
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_privacy: query attr xmlns='%s' is unknown\n",
                       (char *)cstring_get_str(query_xmlns_attr));
    return (EC_FALSE);
}

/**
C: <iq type="get" id="sd59" to="example.com"><query xmlns="jabber:iq:browse"/>
S: <iq type="result" from="example.com" id="sd59"/>
**/
EC_BOOL cxmpp_make_iq_get_browse(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *query_stream_node, CEXPAT_NODE **send_stream_node)
{
    CEXPAT_NODE       *send_query_stream_node;

    send_query_stream_node = cexpat_node_make((const uint8_t *)"query");
    cexpat_node_clone_attr(query_stream_node, send_query_stream_node, (const uint8_t *)"xmlns");
    
    (*send_stream_node) = send_query_stream_node;
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_iq_get_query_browse(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node)
{
    const CEXPAT_NODE *query_stream_node;
    const CSTRING     *query_xmlns_attr;

    query_stream_node = cexpat_find_node_by_path(recv_iq_stream_node, (const uint8_t *)"iq|query", (const uint8_t *)"|");
    if(NULL_PTR == query_stream_node)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_browse: query node not reached yet\n");
        return (EC_TRUE);
    }

    query_xmlns_attr = cexpat_find_attr(query_stream_node, (const uint8_t *)"xmlns");
    if(NULL_PTR == query_xmlns_attr)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_browse: query attr 'xmlns' not reached yet\n");
        return (EC_TRUE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(query_xmlns_attr, (const uint8_t *)"jabber:iq:browse"))
    {
        CEXPAT_NODE *send_query_stream_node;
        CEXPAT_NODE *send_iq_stream_node;

        CXMPPC2S_CONN *cxmppc2s_conn;
        CSTRING       *jid;    
        CSTRING       *jid_no_resource;  
        
        send_query_stream_node = NULL_PTR;
        if(EC_FALSE == cxmpp_make_iq_get_browse(cxmpp_node, query_stream_node, &send_query_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_browse: make iq|get|query failed\n");
            return (EC_FALSE);
        }

        send_iq_stream_node = cexpat_node_make((const uint8_t *)"iq");
        if(NULL_PTR == send_iq_stream_node)
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_browse: make iq node failed\n");
            cexpat_node_free(send_query_stream_node);
            return (EC_FALSE);
        }

        cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
        jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);        
        jid_no_resource = cxmppc2s_conn_make_jid_no_resource(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
        
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"type", (const uint8_t *)"result");
        cexpat_node_clone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"id");
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"from", (const uint8_t *)cstring_get_str(jid_no_resource));
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"to", (const uint8_t *)cstring_get_str(jid));

        cexpat_node_add_child(send_iq_stream_node, send_query_stream_node);

        cstring_free(jid);
        cstring_free(jid_no_resource);
        
        cexpat_node_encode_xml(send_iq_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*closed scope*/);
        cexpat_node_free(send_iq_stream_node);
        
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query_browse: query attr xmlns='%s' is unknown\n",
                       (char *)cstring_get_str(query_xmlns_attr));
    return (EC_FALSE);
}

EC_BOOL cxmpp_handle_iq_get_query(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node, const CSTRING *recv_query_xmlns_attr)
{  
    if(EC_TRUE == cstring_is_str_ignore_case(recv_query_xmlns_attr, (const uint8_t *)"http://jabber.org/protocol/disco#items"))
    {
        if(EC_TRUE == cxmpp_handle_iq_get_query_disco(cxmpp_node, recv_iq_stream_node))
        {
            return (EC_TRUE);    
        }
        
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query: handle iq|query disco#item failed\n");
        return (EC_FALSE);        
    }

    if(EC_TRUE == cstring_is_str_ignore_case(recv_query_xmlns_attr, (const uint8_t *)"http://jabber.org/protocol/disco#info"))
    {
        //TODO:
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "warn:cxmpp_handle_iq_get_query: handle iq|query disco#info failed\n");
        return (EC_TRUE);        
    }    

    if(EC_TRUE == cstring_is_str_ignore_case(recv_query_xmlns_attr, (const uint8_t *)"jabber:iq:roster"))
    {
        if(EC_TRUE == cxmpp_handle_iq_get_query_roster(cxmpp_node, recv_iq_stream_node))
        {
            return (EC_TRUE);    
        }
        
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query: handle iq|query roster failed\n");
        return (EC_FALSE);           
    }

    if(EC_TRUE == cstring_is_str_ignore_case(recv_query_xmlns_attr, (const uint8_t *)"jabber:iq:privacy"))
    {
        if(EC_TRUE == cxmpp_handle_iq_get_query_privacy(cxmpp_node, recv_iq_stream_node))
        {
            return (EC_TRUE);    
        }
        
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query: handle iq|query privacy failed\n");
        return (EC_FALSE);         
    }

    if(EC_TRUE == cstring_is_str_ignore_case(recv_query_xmlns_attr, (const uint8_t *)"jabber:iq:browse"))
    {
        if(EC_TRUE == cxmpp_handle_iq_get_query_browse(cxmpp_node, recv_iq_stream_node))
        {
            return (EC_TRUE);    
        }
        
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query: handle iq|query browse failed\n");
        return (EC_FALSE);        
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_query: unknown xmlns '%s' failed\n",
                        (char *)cstring_get_str(recv_query_xmlns_attr));
    return (EC_FALSE);
}

/*
[USER1003]<iq type="get" id="sd66" to="user1004@example.com"><vCard xmlns="vcard-temp"/></iq>
[S]<iq from='user1004@example.com' to='user1003@example.com/.........' id='sd66' type='result'/>
*/
EC_BOOL cxmpp_make_iq_get_vcard(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node)
{
    CEXPAT_NODE   *send_iq_stream_node;

    CXMPPC2S_CONN *cxmppc2s_conn;
    CSTRING       *jid;

    cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
    jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);

    send_iq_stream_node = cexpat_node_make((const uint8_t *)"iq");
    if(NULL_PTR == send_iq_stream_node)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_make_iq_get_vcard: make iq node failed\n");
        return (EC_FALSE);
    }

    cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"type", (const uint8_t *)"result");
    cexpat_node_clone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"id");
    cexpat_node_xclone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"to", (const uint8_t *)"from");
    cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"to", (const uint8_t *)cstring_get_str(jid));

    cstring_free(jid);

    cexpat_node_encode_xml(send_iq_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*closed scope*/);
    cexpat_node_free(send_iq_stream_node);  

    return (EC_TRUE);
}
EC_BOOL cxmpp_handle_iq_get_vcard(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node, const CSTRING *recv_query_xmlns_attr)
{
    if(EC_TRUE == cstring_is_str_ignore_case(recv_query_xmlns_attr, (const uint8_t *)"vcard-temp"))
    {
        //to="user1004@example.com"
        //TODO: send 'iq(get)|query|vcard' to server of user1004
        if(EC_FALSE == cxmpp_make_iq_get_vcard(cxmpp_node, recv_iq_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_make_iq_get_vcard: make iq(get)|query|vcard result node failed\n");
            return (EC_FALSE);
        }
        
        return (EC_TRUE);
    }
    
    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_get_vcard_disco: unknown xmlns '%s' failed\n",
                        (char *)cstring_get_str(recv_query_xmlns_attr));    
    return (EC_FALSE);
}

EC_BOOL cxmpp_handle_iq_get(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node)
{
    CSTRING     *recv_query_xmlns_attr;
    CSTRING     *recv_vcard_xmlns_attr;

    recv_query_xmlns_attr = cexpat_find_attr_by_path(recv_iq_stream_node, (const uint8_t *)"iq|query|xmlns", (const uint8_t *)"|");
    if(NULL_PTR != recv_query_xmlns_attr)
    {
        if(EC_FALSE == cxmpp_handle_iq_get_query(cxmpp_node, recv_iq_stream_node, recv_query_xmlns_attr))
        {
            dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_get: handle 'iq|query|xmlns' failed\n");
            return (EC_FALSE);
        }
        return (EC_TRUE);
    }

    recv_vcard_xmlns_attr = cexpat_find_attr_by_path(recv_iq_stream_node, (const uint8_t *)"iq|vCard|xmlns", (const uint8_t *)"|");
    if(NULL_PTR != recv_vcard_xmlns_attr)
    {
        if(EC_FALSE == cxmpp_handle_iq_get_vcard(cxmpp_node, recv_iq_stream_node, recv_vcard_xmlns_attr))
        {
            dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_get: handle 'iq|vcard|xmlns' failed\n");
            return (EC_FALSE);
        }
        return (EC_TRUE);
    }    

    dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_get:  'iq(get)|{query|vcard}|xmlns' not reached yet\n");

    return (EC_TRUE);    
}

EC_BOOL cxmpp_bind_c2s(CXMPP_NODE *cxmpp_node, const CBYTES *bind_resource_cdata)
{
    UINT32   cxmppc2s_md_id;

    CXMPPC2S_CONN *cxmppc2s_conn;
    CSTRING       *domain;
    CSTRING       *resource;

    cxmppc2s_md_id = cxmppc2s_start();
    if(ERR_MODULE_ID == cxmppc2s_md_id)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_bind_c2s: start cxmppc2s module failed\n");
        return (EC_FALSE);
    }
   
    cxmppc2s_set_usrname(cxmppc2s_md_id, CXMPP_NODE_USERNAME(cxmpp_node));   

    dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_bind_c2s: start cxmppc2s module %ld and bind usrname %s done\n",
                      cxmppc2s_md_id, CXMPP_NODE_USERNAME_STR(cxmpp_node));    

    domain   = cstring_new(g_cxmpp_domain, LOC_CXMPP_0001);
    resource = cstring_make_by_bytes(cbytes_len(bind_resource_cdata), cbytes_buf(bind_resource_cdata));

    if(1 && do_log(SEC_0147_CXMPP, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_bind_c2s: resource cdata is\n");
        cbytes_print_str(LOGSTDOUT, bind_resource_cdata);
        cbytes_print_chars(LOGSTDOUT, bind_resource_cdata);
    }

    cxmppc2s_conn = cxmppc2s_add_conn(cxmppc2s_md_id, domain, resource, CXMPP_NODE_CSOCKET_CNODE(cxmpp_node));
    if(NULL_PTR == cxmppc2s_conn)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_bind_c2s: add cxmppc2s_conn failed\n");
        
        cstring_free(domain);
        cstring_free(resource);
        cxmppc2s_end(cxmppc2s_md_id);
        return (EC_FALSE);
    }
    
    cstring_free(domain);
    cstring_free(resource);    

    CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node) = cxmppc2s_conn;
    return (EC_TRUE);
}

/**
C: <iq type="set" id="sd1"><bind xmlns="urn:ietf:params:xml:ns:xmpp-bind"><resource>pandion</resource></bind></iq>
S: <iq id='sd1' type='result'><bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'><jid>user1000@example.com/pandion</jid></bind></iq>
**/ 
EC_BOOL cxmpp_make_iq_set_bind(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *bind_stream_node, CEXPAT_NODE **send_stream_node)
{
    const CBYTES      *bind_resource_cdata;
    CEXPAT_NODE       *send_jid_stream_node;
    CEXPAT_NODE       *send_bind_stream_node;
    
    bind_resource_cdata = cexpat_find_cdata_by_path(bind_stream_node, (const uint8_t *)"bind|resource", (const uint8_t *)"|");
    if(NULL_PTR == bind_resource_cdata)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_make_iq_set_bind: bind|resource not reached yet\n");
        (*send_stream_node) = NULL_PTR;
        return (EC_TRUE);
    }

    /*start CXMPPC2S module*/

    if(0 && do_log(SEC_0147_CXMPP, 9))
    {
        UINT32 idx;
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_make_iq_set_bind: resouce is %.*s\n", 
                           CBYTES_LEN(bind_resource_cdata), CBYTES_BUF(bind_resource_cdata));
        for(idx = 0; idx < CBYTES_LEN(bind_resource_cdata); idx ++)
        {
            sys_print(LOGSTDOUT, "\\x%x", (uint8_t)CBYTES_BUF(bind_resource_cdata)[ idx ]);
        }
        sys_print(LOGSTDOUT, "\n");
    }

    if(EC_FALSE == cxmpp_bind_c2s(cxmpp_node, bind_resource_cdata))
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 1)(LOGSTDOUT, "warn:cxmpp_make_iq_set_bind: bind c2s failed\n");
        (*send_stream_node) = NULL_PTR;
        return (EC_TRUE);
    }
       
    send_jid_stream_node = __cxmpp_make_jid_cexpat_node(CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node));

    send_bind_stream_node = __cxmpp_make_bind_cexpat_node();
    cexpat_node_add_child(send_bind_stream_node, send_jid_stream_node);

    (*send_stream_node) = send_bind_stream_node;
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_iq_set_bind(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node, const CEXPAT_NODE *recv_bind_stream_node)
{
    const CSTRING     *recv_bind_xmlns_attr;

    recv_bind_xmlns_attr = cexpat_find_attr(recv_bind_stream_node, (const uint8_t *)"xmlns");
    if(NULL_PTR == recv_bind_xmlns_attr)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_bind: bind attr 'xmlns' not reached yet\n");
        return (EC_TRUE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(recv_bind_xmlns_attr, (const uint8_t *)"urn:ietf:params:xml:ns:xmpp-bind"))
    {
        CEXPAT_NODE *send_bind_stream_node;
        CEXPAT_NODE *send_iq_stream_node;
    
        send_bind_stream_node = NULL_PTR;
        if(EC_FALSE == cxmpp_make_iq_set_bind(cxmpp_node, recv_bind_stream_node, &send_bind_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_bind: make iq|set|bind failed\n");
            return (EC_FALSE);
        }

        if(do_log(SEC_0147_CXMPP, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_set_bind: send_bind_stream_node is\n");
            cexpat_node_depth_print_xml_level(LOGSTDOUT, send_bind_stream_node, 0);
        }        

        send_iq_stream_node = cexpat_node_make((const uint8_t *)"iq");
        if(NULL_PTR == send_iq_stream_node)
        {
            /*not reached yet, wait for next reading*/
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_bind: make iq node failed\n");
            cexpat_node_free(send_bind_stream_node);
            return (EC_FALSE);
        }
        
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"type", (const uint8_t *)"result");
        cexpat_node_clone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"id");

        cexpat_node_add_child(send_iq_stream_node, send_bind_stream_node);

        if(do_log(SEC_0147_CXMPP, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_set_bind: send_iq_stream_node is\n");
            cexpat_node_depth_print_xml_level(LOGSTDOUT, send_iq_stream_node, 0);
        }         

        cexpat_node_encode_xml(send_iq_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*closed scope*/);
        cexpat_node_free(send_iq_stream_node);
        
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_bind: bind attr xmlns='%s' is unknown\n",
                       (char *)cstring_get_str(recv_bind_xmlns_attr));
    return (EC_FALSE);
}

/**
C: <iq type="set" id="sd2" to="example.com"><session xmlns="urn:ietf:params:xml:ns:xmpp-session"/></iq>
S: <iq type='result' from='example.com' id='sd2'/>
**/
EC_BOOL cxmpp_make_iq_set_session(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *session_stream_node, CEXPAT_NODE **send_stream_node)
{
    CEXPAT_NODE       *send_session_stream_node;

    send_session_stream_node = cexpat_node_make((const uint8_t *)"session");
    cexpat_node_clone_attr(session_stream_node, send_session_stream_node, (const uint8_t *)"xmlns");

    (*send_stream_node) = send_session_stream_node;
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_iq_set_session(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node, const CEXPAT_NODE *recv_session_stream_node)
{
    const CSTRING     *recv_session_xmlns;
    
    recv_session_xmlns = cexpat_find_attr(recv_session_stream_node, (const uint8_t *)"xmlns");
    if(NULL_PTR == recv_session_xmlns)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_session: session attr 'xmlns' not reached yet\n");
        return (EC_TRUE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(recv_session_xmlns, (const uint8_t *)"urn:ietf:params:xml:ns:xmpp-session"))
    {
        CEXPAT_NODE *send_session_stream_node;
        CEXPAT_NODE *send_iq_stream_node;
    
        send_session_stream_node = NULL_PTR;
        if(EC_FALSE == cxmpp_make_iq_set_session(cxmpp_node, recv_session_stream_node, &send_session_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_session: make iq|set|session failed\n");
            return (EC_FALSE);
        }

        if(do_log(SEC_0147_CXMPP, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_set_session: send_session_stream_node is\n");
            cexpat_node_depth_print_xml_level(LOGSTDOUT, send_session_stream_node, 0);
        }         

        send_iq_stream_node = cexpat_node_make((const uint8_t *)"iq");
        if(NULL_PTR == send_iq_stream_node)
        {
            /*not reached yet, wait for next reading*/
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_session: make iq node failed\n");
            cexpat_node_free(send_session_stream_node);
            return (EC_FALSE);
        }        
        
        cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"type", (const uint8_t *)"result");
        cexpat_node_clone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"id");

        cexpat_node_add_child(send_iq_stream_node, send_session_stream_node);

        if(do_log(SEC_0147_CXMPP, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_set_session: send_iq_stream_node is\n");
            cexpat_node_depth_print_xml_level(LOGSTDOUT, send_iq_stream_node, 0);
        }         

        cexpat_node_encode_xml(send_iq_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*closed scope*/);
        cexpat_node_free(send_iq_stream_node);
        
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_session: session attr xmlns='%s' is unknown\n",
                       (char *)cstring_get_str(recv_session_xmlns));
    return (EC_FALSE);
}

/* 
user1003 invite user1000
--------------------------------------------
[USER1003]<iq type="set" id="sd65"><query xmlns="jabber:iq:roster"><item jid="user1004@example.com"/></query></iq>
[S]<iq from='user1003@example.com' to='user1003@example.com/.........' id='push2188776968' type='set'>
      <query xmlns='jabber:iq:roster'>
         <item subscription='none' jid='user1004@example.com'/>
      </query>
    </iq>
[S]<iq from='user1003@example.com' to='user1003@example.com/.........' id='sd65' type='result'/>
*/

EC_BOOL cxmpp_make_iq_set_query_roster_item(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *query_stream_node, CEXPAT_NODE **send_stream_node)
{
    CEXPAT_NODE       *send_roster_stream_node;
    CEXPAT_NODE       *send_item_stream_node;
    const CSTRING     *to_jid;
    //const char        *subscription[] = {"none", "to", "both"};
    //uint32_t           shakehand;

    //shakehand = CXMPP_NODE_SHAKEHAND(cxmpp_node);
    //CXMPP_NODE_SHAKEHAND(cxmpp_node) = (shakehand + 1) % 3;

    to_jid = cexpat_find_attr_by_path(query_stream_node, (const uint8_t *)"query|item|jid", (const uint8_t *)"|");

    send_roster_stream_node = cexpat_node_make((const uint8_t *)"query");
    cexpat_node_clone_attr(query_stream_node, send_roster_stream_node, (const uint8_t *)"xmlns");

    send_item_stream_node = cexpat_node_make((const uint8_t *)"item");
    cexpat_node_add_attr(send_item_stream_node, (const uint8_t *)"subscription", (const uint8_t *)/*subscription[ shakehand ]*/"none");
    cexpat_node_add_attr(send_item_stream_node, (const uint8_t *)"jid", cstring_get_str(to_jid));

    cexpat_node_add_child(send_roster_stream_node, send_item_stream_node);    

    (*send_stream_node) = send_roster_stream_node;
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_iq_set_query_roster_id(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node, const CEXPAT_NODE *recv_query_stream_node)
{
    CEXPAT_NODE *send_roster_stream_node;
    CEXPAT_NODE *send_iq_stream_node;

    CXMPPC2S_CONN  *cxmppc2s_conn;
    CSTRING        *from_jid;
    CSTRING        *from_jid_no_resource;
    
    /*<iq from='user1003@example.com' to='user1003@example.com/.........' id='push2188776968' type='set'>*/
    send_iq_stream_node = cexpat_node_make((const uint8_t *)"iq");
    if(NULL_PTR == send_iq_stream_node)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_query_roster_id: make iq node failed\n");
        return (EC_FALSE);
    }

    cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
    from_jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
    from_jid_no_resource = cxmppc2s_conn_make_jid_no_resource(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
   
    cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"type", (const uint8_t *)"set");
    cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"from", (const uint8_t *)cstring_get_str(from_jid));
    cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"to"  , (const uint8_t *)cstring_get_str(from_jid_no_resource));
    cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"id"  , cxmpp_push_id_new());

    send_roster_stream_node = NULL_PTR;
    if(EC_FALSE == cxmpp_make_iq_set_query_roster_item(cxmpp_node, recv_query_stream_node, &send_roster_stream_node))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_query_roster_id: make iq|query roster failed\n");
        cexpat_node_free(send_iq_stream_node);
        return (EC_FALSE);
    }

    cexpat_node_add_child(send_iq_stream_node, send_roster_stream_node);

    cstring_free(from_jid);
    cstring_free(from_jid_no_resource);
    
    if(do_log(SEC_0147_CXMPP, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_set_query_roster_id: send_iq_stream_node is\n");
        cexpat_node_depth_print_xml_level(LOGSTDOUT, send_iq_stream_node, 0);
    }         

    cexpat_node_encode_xml(send_iq_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*closed scope*/);
    cexpat_node_free(send_iq_stream_node);
    
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_iq_set_query_roster_result(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node, const CEXPAT_NODE *recv_query_stream_node)
{
    CEXPAT_NODE *send_iq_stream_node;

    CXMPPC2S_CONN  *cxmppc2s_conn;
    CSTRING        *from_jid;
    CSTRING        *from_jid_no_resource;
    
    /*<iq from='user1003@example.com' to='user1003@example.com/.........' id='sd65' type='result'/>*/
    send_iq_stream_node = cexpat_node_make((const uint8_t *)"iq");
    if(NULL_PTR == send_iq_stream_node)
    {
        /*not reached yet, wait for next reading*/
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_query_roster_result: make iq node failed\n");
        return (EC_FALSE);
    }

    cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
    from_jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
    from_jid_no_resource = cxmppc2s_conn_make_jid_no_resource(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
   
    cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"type", (const uint8_t *)"result");
    cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"from", (const uint8_t *)cstring_get_str(from_jid));
    cexpat_node_add_attr(send_iq_stream_node, (const uint8_t *)"to"  , (const uint8_t *)cstring_get_str(from_jid_no_resource));
    cexpat_node_clone_attr(recv_iq_stream_node, send_iq_stream_node, (const uint8_t *)"id");

    cstring_free(from_jid);
    cstring_free(from_jid_no_resource);    
  
    if(do_log(SEC_0147_CXMPP, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_iq_set_query_roster_result: send_iq_stream_node is\n");
        cexpat_node_depth_print_xml_level(LOGSTDOUT, send_iq_stream_node, 0);
    }         

    cexpat_node_encode_xml(send_iq_stream_node, CXMPP_NODE_SEND_CBUFFER(cxmpp_node), EC_TRUE/*closed scope*/);
    cexpat_node_free(send_iq_stream_node);
    
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_iq_set_query(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node, const CEXPAT_NODE *recv_query_stream_node)
{
    const CSTRING     *recv_query_xmlns;
    
    recv_query_xmlns = cexpat_find_attr(recv_query_stream_node, (const uint8_t *)"xmlns");
    if(NULL_PTR == recv_query_xmlns)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_query: query attr 'xmlns' not reached yet\n");
        return (EC_TRUE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(recv_query_xmlns, (const uint8_t *)"jabber:iq:roster"))
    {
        if(EC_FALSE == cxmpp_handle_iq_set_query_roster_id(cxmpp_node, recv_iq_stream_node, recv_query_stream_node))
        {
            /*not reached yet, wait for next reading*/
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_query: handle iq(set)|query|roster failed\n");
            return (EC_FALSE);
        }

        if(EC_FALSE == cxmpp_handle_iq_set_query_roster_result(cxmpp_node, recv_iq_stream_node, recv_query_stream_node))
        {
            /*not reached yet, wait for next reading*/
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_query: handle iq(set)|query|result failed\n");
            return (EC_FALSE);
        }        
        
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_query: query attr xmlns='%s' is unknown\n",
                       (char *)cstring_get_str(recv_query_xmlns));
    return (EC_FALSE);
}

EC_BOOL cxmpp_handle_iq_set(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node)
{
    CEXPAT_NODE *recv_iq_sub_node;

    recv_iq_sub_node = cexpat_find_node_by_path(recv_iq_stream_node, (const uint8_t *)"iq|bind", (const uint8_t *)"|");
    if(NULL_PTR != recv_iq_sub_node)
    {
        if(EC_TRUE == cxmpp_handle_iq_set_bind(cxmpp_node, recv_iq_stream_node, recv_iq_sub_node))
        {
            return (EC_TRUE);    
        }
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_bind: handle iq|bind failed\n");
        return (EC_FALSE);
    }

    recv_iq_sub_node = cexpat_find_node_by_path(recv_iq_stream_node, (const uint8_t *)"iq|session", (const uint8_t *)"|");
    if(NULL_PTR != recv_iq_sub_node)
    {
        if(EC_TRUE == cxmpp_handle_iq_set_session(cxmpp_node, recv_iq_stream_node, recv_iq_sub_node))
        {
            return (EC_TRUE);    
        }
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_bind: handle iq|session failed\n");
        return (EC_FALSE);
    }

    recv_iq_sub_node = cexpat_find_node_by_path(recv_iq_stream_node, (const uint8_t *)"iq|query", (const uint8_t *)"|");
    if(NULL_PTR != recv_iq_sub_node)
    {
        if(EC_TRUE == cxmpp_handle_iq_set_query(cxmpp_node, recv_iq_stream_node, recv_iq_sub_node))
        {
            return (EC_TRUE);    
        }
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_bind: handle iq|query failed\n");
        return (EC_FALSE);
    }    

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq_set_bind: unknown iq subnode\n");
    return (EC_FALSE);    
}

EC_BOOL cxmpp_handle_iq(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_iq_stream_node)
{
    const CSTRING *type;

    type = cexpat_find_attr(recv_iq_stream_node, (const uint8_t *)"type");
    if(NULL_PTR == type)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq: attr type not reached yet\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(type, (const uint8_t *)"set"))
    {
        if(EC_FALSE == cxmpp_handle_iq_set(cxmpp_node, recv_iq_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq: iq set failed\n");
            return (EC_FALSE);
        }
        
        return (EC_TRUE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(type, (const uint8_t *)"get"))
    {        
        if(EC_FALSE == cxmpp_handle_iq_get(cxmpp_node, recv_iq_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq: iq get failed\n");
            return (EC_FALSE);
        }

        return (EC_TRUE);
    }  

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_iq: unknown iq type '%s'\n", (char *)cstring_get_str(type));
    return (EC_FALSE);
}

/*
[USER1003]<presence type="subscribe" to="user1004@example.com"/>
[S]<iq from='user1003@example.com' to='user1003@example.com/.........' id='push3377869454' type='set'>
      <query xmlns='jabber:iq:roster'><item ask='subscribe' subscription='none' jid='user1004@example.com'/></query>
   </iq>
*/

EC_BOOL cxmpp_make_presence_subscribe(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_query_stream_node, CBUFFER *send_buffer)
{
    CEXPAT_NODE   *send_presence_stream_node;
    CSTRING       *jid;
    CXMPPC2S_CONN *cxmppc2s_conn;

    cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
    jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
       
    send_presence_stream_node = cexpat_node_make((const uint8_t *)"presence");
    cexpat_node_add_attr(send_presence_stream_node, (const uint8_t *)"type", (const uint8_t *)"subscribe");
    cexpat_node_add_attr(send_presence_stream_node, (const uint8_t *)"from", cstring_get_str(jid));
    cexpat_node_clone_attr(recv_query_stream_node, send_presence_stream_node, (const uint8_t *)"to");
    
    cexpat_node_encode_xml(send_presence_stream_node, send_buffer, EC_TRUE/*closed scope*/);
    cexpat_node_free(send_presence_stream_node);     

    cstring_free(jid);

    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_presence_subscribe(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_presence_stream_node)
{
    const CSTRING *to;

    CXMPPC2S_CONN *cxmppc2s_conn;

    cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
    
    to = cexpat_find_attr(recv_presence_stream_node, (const uint8_t *)"to");
    if(NULL_PTR != to)
    {
        CBUFFER * send_subscribe_buffer;

        send_subscribe_buffer = cbuffer_new(CXMPP_SEND_CBUFFER_SIZE);
        if(NULL_PTR == send_subscribe_buffer)
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence_subscribe: new send buffer failed\n");
            return (EC_FALSE);
        }

        if(EC_FALSE == cxmpp_make_presence_subscribe(cxmpp_node, recv_presence_stream_node, send_subscribe_buffer))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence_subscribe: make presence|query failed\n");
            cbuffer_free(send_subscribe_buffer);
            return (EC_FALSE);
        }
    
        cxmppc2s_presence_subscribe(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), to, send_subscribe_buffer);

        if(do_log(SEC_0147_CXMPP, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_presence_subscribe: send_subscribe_buffer is [%.*s]\n",
                               CBUFFER_USED(send_subscribe_buffer), CBUFFER_DATA(send_subscribe_buffer));
        } 
        
        cbuffer_free(send_subscribe_buffer);
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence_subscribe: 'presence|to' not reached yet\n");

    return (EC_TRUE);
}

/*
<presence from='user1004@example.com' to='user1003@example.com/.........' xml:lang='zh-cn' type='subscribed'/>
*/
EC_BOOL cxmpp_make_presence_subscribed(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_presence_stream_node, CBUFFER *send_buffer)
{
    CEXPAT_NODE   *send_presence_stream_node;    
    CXMPPC2S_CONN *cxmppc2s_conn;
    const CSTRING *from;

    cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
       
    send_presence_stream_node = cexpat_node_make((const uint8_t *)"presence");
    cexpat_node_add_attr(send_presence_stream_node, (const uint8_t *)"type", (const uint8_t *)"subscribed");    
    cexpat_node_clone_attr(recv_presence_stream_node, send_presence_stream_node, (const uint8_t *)"to");

    from = cexpat_find_attr(recv_presence_stream_node, (const uint8_t *)"from");
    if(NULL_PTR != from)
    {
        cexpat_node_add_attr(send_presence_stream_node, (const uint8_t *)"from", cstring_get_str(from));
    }
    else/*add 'from' attr*/
    {
        CSTRING       *jid;
        jid = cxmppc2s_conn_make_jid(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), cxmppc2s_conn);
        cexpat_node_add_attr(send_presence_stream_node, (const uint8_t *)"from", cstring_get_str(jid));
        cstring_free(jid);
    }    
    
    cexpat_node_encode_xml(send_presence_stream_node, send_buffer, EC_TRUE/*closed scope*/);
    cexpat_node_free(send_presence_stream_node);

    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_presence_subscribed(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_presence_stream_node)
{
    const CSTRING *to;

    CXMPPC2S_CONN *cxmppc2s_conn;

    cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
    
    to = cexpat_find_attr(recv_presence_stream_node, (const uint8_t *)"to");
    if(NULL_PTR != to)
    {
        CBUFFER * send_subscribed_buffer;

        send_subscribed_buffer = cbuffer_new(CXMPP_SEND_CBUFFER_SIZE);
        if(NULL_PTR == send_subscribed_buffer)
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence_subscribed: new send buffer failed\n");
            return (EC_FALSE);
        }

        if(EC_FALSE == cxmpp_make_presence_subscribed(cxmpp_node, recv_presence_stream_node, send_subscribed_buffer))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence_subscribed: make presence|subscribed failed\n");
            cbuffer_free(send_subscribed_buffer);
            return (EC_FALSE);
        }        

        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle_presence_subscribed: to is '%s'\n", (char *)cstring_get_str(to));
    
        cxmppc2s_presence_subscribed(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), to, send_subscribed_buffer);
        cbuffer_free(send_subscribed_buffer);
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence_subscribed: 'presence|to' not reached yet\n");

    return (EC_TRUE);
}

/**
C: <presence><x xmlns="jabber:x:avatar"><hash>8846ea523d1185eb903bfc2218c153a7cf3bbf70</hash></x><priority>8</priority></presence>
S: 
**/
EC_BOOL cxmpp_make_presence(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *query_stream_node)
{
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_presence(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_presence_stream_node)
{
    const CSTRING *type;
    const CSTRING *xmlns;

    if(do_log(SEC_0147_CXMPP, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_presence: recv_presence_stream_node is\n");
        cexpat_node_depth_print_xml_level(LOGSTDOUT, recv_presence_stream_node, 0);
    }

    type = cexpat_find_attr_by_path(recv_presence_stream_node, (const uint8_t *)"presence|type", (const uint8_t *)"|");
    if(NULL_PTR != type)
    {
        if(EC_TRUE == cstring_is_str_ignore_case(type, (const uint8_t *)"unavailable"))
        {
            CXMPP_NODE_STATE(cxmpp_node) = CXMPP_NODE_STATE_STREAM_CLOSING;
            return (EC_TRUE);
        }

        if(EC_TRUE == cstring_is_str_ignore_case(type, (const uint8_t *)"subscribe"))
        {
            if(2 <= CXMPP_NODE_SHAKEHAND(cxmpp_node))
            {
                return (EC_TRUE);
            }
            
            if(EC_FALSE == cxmpp_handle_presence_subscribe(cxmpp_node, recv_presence_stream_node))
            {
                dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence: presence subscribe failed\n");
                return (EC_FALSE);
            }

            CXMPP_NODE_SHAKEHAND(cxmpp_node) ++;
            return (EC_TRUE);
        }

        if(EC_TRUE == cstring_is_str_ignore_case(type, (const uint8_t *)"subscribed"))
        {
            if(2 <= CXMPP_NODE_SHAKEHAND(cxmpp_node))
            {
                return (EC_TRUE);
            }
            
            if(EC_FALSE == cxmpp_handle_presence_subscribed(cxmpp_node, recv_presence_stream_node))
            {
                dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence: presence subscribed failed\n");
                return (EC_FALSE);
            }
            CXMPP_NODE_SHAKEHAND(cxmpp_node) ++;
            return (EC_TRUE);
        }        
        
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence: unknown attr presence|type '%s'\n", 
                           (char *)cstring_get_str(type));
        return (EC_FALSE);
    }

    xmlns = cexpat_find_attr_by_path(recv_presence_stream_node, (const uint8_t *)"presence|x|xmlns", (const uint8_t *)"|");
    if(NULL_PTR == xmlns)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence: attr presence|x|xmlns not reached yet\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_str_ignore_case(xmlns, (const uint8_t *)"jabber:x:avatar"))
    {
        if(EC_FALSE == cxmpp_make_presence(cxmpp_node, recv_presence_stream_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence: presence failed\n");
            return (EC_FALSE);
        }
        
        return (EC_TRUE);
    }

    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_presence: unknown xmlns '%s'\n", 
                      (char *)cstring_get_str(xmlns));
    return (EC_FALSE);
}

EC_BOOL cxmpp_make_message(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_message_stream_node, CBUFFER *message_buffer)
{
    if(NULL_PTR != recv_message_stream_node)
    {
        cexpat_node_encode_xml(recv_message_stream_node, message_buffer, EC_TRUE/*closed scope*/);
    }
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_message(CXMPP_NODE *cxmpp_node, const CEXPAT_NODE *recv_message_stream_node)
{
    const CSTRING *from;
    const CSTRING *to;
    
    const CBYTES  *body_cdata;
    CXMPPC2S_CONN *cxmppc2s_conn;
    CBUFFER       *send_message_buffer;

    if(do_log(SEC_0147_CXMPP, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_message: recv_message_stream_node is\n");
        cexpat_node_depth_print_xml_level(LOGSTDOUT, recv_message_stream_node, 0);
    }

    //from = cexpat_find_attr(recv_message_stream_node, (const uint8_t *)"from", (const uint8_t *)"|");
    from = cexpat_find_attr(recv_message_stream_node, (const uint8_t *)"from");
    if(NULL_PTR == from)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_message: attr message|from not reached yet\n");
        return (EC_TRUE);
    }

    //to = cexpat_find_attr(recv_message_stream_node, (const uint8_t *)"to", (const uint8_t *)"|");
    to = cexpat_find_attr(recv_message_stream_node, (const uint8_t *)"to");
    if(NULL_PTR == to)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_message: attr message|to not reached yet\n");
        return (EC_TRUE);
    }    

    body_cdata = cexpat_find_cdata_by_path(recv_message_stream_node, (const uint8_t *)"message|body", (const uint8_t *)"|");
    if(NULL_PTR == body_cdata)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_message: cdata message|body not reached yet\n");
        return (EC_TRUE);
    }

    send_message_buffer = cbuffer_new(CXMPP_SEND_CBUFFER_SIZE);
    if(NULL_PTR == send_message_buffer)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_message: new cbuffer with size %u failed\n", CXMPP_SEND_CBUFFER_SIZE);
        return (EC_FALSE);
    }

    if(EC_FALSE == cxmpp_make_message(cxmpp_node, recv_message_stream_node, send_message_buffer))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_message: make send_message_buffer failed\n");
        cbuffer_free(send_message_buffer);
        return (EC_FALSE);
    }

    cxmppc2s_conn = CXMPP_NODE_CXMPPC2S_CONN(cxmpp_node);
    if(EC_FALSE == cxmppc2s_send_message(CXMPPC2S_CONN_MD_ID(cxmppc2s_conn), to, send_message_buffer))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_message: send message from '%s' to '%s' failed\n",
                            (char *)cstring_get_str(from), (char *)cstring_get_str(to));
        cbuffer_free(send_message_buffer);                            
        return (EC_FALSE);
    }

    dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle_message: send message from '%s' to '%s' done\n",
                        (char *)cstring_get_str(from), (char *)cstring_get_str(to));
    cbuffer_free(send_message_buffer);
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle_state_once(CXMPP_NODE *cxmpp_node, CEXPAT_NODE *recv_parent_node, CEXPAT_NODE *recv_child_node)
{
    if(EC_TRUE == cexpat_node_match(recv_child_node, (const uint8_t *)"iq"))
    {
        if(EC_FALSE == cxmpp_handle_iq(cxmpp_node, recv_child_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_once: handle iq failed\n");
            return (EC_FALSE);
        } 
        
        return (EC_TRUE);
    }

    if(EC_TRUE == cexpat_node_match(recv_child_node, (const uint8_t *)"presence"))
    {
        if(EC_FALSE == cxmpp_handle_presence(cxmpp_node, recv_child_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_once: handle presence failed\n");
            return (EC_FALSE);
        } 
        
        return (EC_TRUE);
    }    

    if(EC_TRUE == cexpat_node_match(recv_child_node, (const uint8_t *)"message"))
    {
        if(EC_FALSE == cxmpp_handle_message(cxmpp_node, recv_child_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_once: handle message failed\n");
            return (EC_FALSE);
        } 
        
        return (EC_TRUE);
    } 
    
    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state_once: unknow node '%s'\n",
                       CEXPAT_NODE_NAME_STR(recv_child_node));
    return (EC_FALSE);
}

EC_BOOL cxmpp_handle_state(CXMPP_NODE *cxmpp_node)
{
    CEXPAT_NODE       *recv_cexpat_node;    
    EC_BOOL            ret;
    
    recv_cexpat_node = CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node);
    if(NULL_PTR == recv_cexpat_node)
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state: recv_cexpat_node is null\n");
        return (EC_FALSE);
    }

    ret = EC_TRUE;

    while(EC_FALSE == clist_is_empty(CEXPAT_NODE_CHILDREN(recv_cexpat_node)))
    {
        CEXPAT_NODE *recv_child_node;
        recv_child_node = (CEXPAT_NODE *)clist_pop_front(CEXPAT_NODE_CHILDREN(recv_cexpat_node));

        if(EC_FALSE == cxmpp_handle_state_once(cxmpp_node, recv_cexpat_node, recv_child_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle_state: handle node '%s' faild\n",
                               CEXPAT_NODE_NAME_STR(recv_child_node));
            cexpat_node_free(recv_child_node);
            return (EC_FALSE);
        }

        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle_state: handle node '%s' done\n",
                           CEXPAT_NODE_NAME_STR(recv_child_node));
                                   
        cexpat_node_free(recv_child_node);
    }

    if(EC_FALSE == cbuffer_is_empty(CXMPP_NODE_SEND_CBUFFER(cxmpp_node)))
    {
        CSOCKET_CNODE     *csocket_cnode;
        
        /*when send buffer is not empty*/
        csocket_cnode = CXMPP_NODE_CSOCKET_CNODE(cxmpp_node);
        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)cxmpp_send_on_csocket_cnode_thread,(void *)csocket_cnode);   

        //CXMPP_NODE_STATE(cxmpp_node) = CXMPP_NODE_STATE_STREAM_ONGOING;

        if(do_log(SEC_0147_CXMPP, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] cxmpp_handle_state: send buffer is\n");
            cbuffer_print_str(LOGSTDOUT, CXMPP_NODE_SEND_CBUFFER(cxmpp_node));
        }        
    }
    return (EC_TRUE);
}

EC_BOOL cxmpp_handle(CXMPP_NODE *cxmpp_node)
{
    if(CXMPP_NODE_STATE_NONE == CXMPP_NODE_STATE(cxmpp_node))
    {
        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle: handle CXMPP_NODE_STATE_NONE\n");
        if(EC_FALSE == cxmpp_handle_state_none(cxmpp_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle: handle CXMPP_NODE_STATE_NONE failed\n");
            return (EC_FALSE);
        }
        
        if(NULL_PTR == CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node))
        {
            dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle_state_stream_list_feature: recv_cexpat_node is null, wait more data\n");
            return (EC_TRUE);
        }
        
        /*fall through*/
    }

    if(CXMPP_NODE_STATE_STREAM_LIST_FEATURE == CXMPP_NODE_STATE(cxmpp_node))
    {
        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle: handle CXMPP_NODE_STATE_STREAM_LIST_FEATURE\n");
        if(EC_FALSE == cxmpp_handle_state_stream_list_feature(cxmpp_node))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle: handle CXMPP_NODE_STATE_STREAM_LIST_FEATURE failed\n");
            return (EC_FALSE);
        }
        /*fall through*/
    }    

    if(CXMPP_NODE_STATE_STREAM_AUTH == CXMPP_NODE_STATE(cxmpp_node))
    {
        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle: handle CXMPP_NODE_STATE_STREAM_AUTH\n");
        return cxmpp_handle_state_stream_auth(cxmpp_node);
    }     

    if(CXMPP_NODE_STATE_STREAM_BIND_FEATURE == CXMPP_NODE_STATE(cxmpp_node))
    {
        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle: handle CXMPP_NODE_STATE_STREAM_BIND_FEATURE\n");
        return cxmpp_handle_state_stream_bind_feature(cxmpp_node);
    } 

    if(CXMPP_NODE_STATE_STREAM_ONGOING == CXMPP_NODE_STATE(cxmpp_node))
    {
        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle: handle CXMPP_NODE_STATE_STREAM_ONGOING\n");
        return cxmpp_handle_state_stream_ongoing(cxmpp_node);
    } 

    if(CXMPP_NODE_STATE_STREAM_CLOSING == CXMPP_NODE_STATE(cxmpp_node))
    {
        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle: handle CXMPP_NODE_STATE_STREAM_CLOSING\n");
        return cxmpp_handle_state_stream_closing(cxmpp_node);
    }  

    if(CXMPP_NODE_STATE_STREAM_CLOSED == CXMPP_NODE_STATE(cxmpp_node))
    {
        dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_handle: handle CXMPP_NODE_STATE_STREAM_CLOSED\n");
        return cxmpp_handle_state_stream_closed(cxmpp_node);
    }    

    /*should never reach here*/
    dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_handle: cxmpp_node %p, invalid state is %d\n", 
                       cxmpp_node, CXMPP_NODE_STATE(cxmpp_node));
    return (EC_FALSE);
}

static EC_BOOL __cxmpp_node_prepare_for_csocket_cnode(CSOCKET_CNODE *csocket_cnode)
{
    CXMPP_NODE *cxmpp_node;
   
    cxmpp_node = CSOCKET_CNODE_CXMPP_NODE(csocket_cnode);
    if(NULL_PTR == cxmpp_node)
    {  
        cxmpp_node = cxmpp_node_new(CXMPP_RECV_CBUFFER_SIZE);
        if(NULL_PTR == cxmpp_node)
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:__cxmpp_node_prepare_for_csocket_cnode: new cxmpp_node failed\n");
            return (EC_FALSE);
        }

        if(EC_FALSE == cexpat_parser_open(CXMPP_NODE_XML_PARSER(cxmpp_node)))
        {
            dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:__cxmpp_node_prepare_for_csocket_cnode: open expat parser failed\n");
            cxmpp_node_free(cxmpp_node);
            return (EC_FALSE);
        }        
        
        CSOCKET_CNODE_CXMPP_NODE(csocket_cnode) = cxmpp_node;
        CXMPP_NODE_CSOCKET_CNODE(cxmpp_node)    = csocket_cnode;
    }

    return (EC_TRUE);
}

/*
* Q: recv and send will happen at same time (once epoll_wait)? if so, exception should
*    trigger defer close but not epoll close immediately?
*
*/
EC_BOOL cxmpp_recv_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode)
{
    CXMPP_NODE     *cxmpp_node;
    CBUFFER        *recv_buffer;
    CEXPAT_PARSER  *cexpat_parser;
    
    UINT32      pos;
    
    if(EC_FALSE == CSOCKET_CNODE_IS_CONNECTED(csocket_cnode))
    {
        cxmpp_csocket_cnode_epoll_close(csocket_cnode);
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_recv_on_csocket_cnode: sockfd %d is not connected\n", 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    if(EC_FALSE == __cxmpp_node_prepare_for_csocket_cnode(csocket_cnode))
    {        
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_recv_on_csocket_cnode: sockfd %d prepare cxmpp_node failed\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode));
        cxmpp_csocket_cnode_epoll_close(csocket_cnode);
        return (EC_FALSE);
    }

    cxmpp_node   = CSOCKET_CNODE_CXMPP_NODE(csocket_cnode);
    recv_buffer  = CXMPP_NODE_RECV_CBUFFER(cxmpp_node);
    
    pos = CBUFFER_USED(recv_buffer);
    if(EC_FALSE == csocket_read(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                CBUFFER_DATA(recv_buffer), 
                                CBUFFER_ROOM(recv_buffer), 
                                &pos))
    {
        cxmpp_csocket_cnode_epoll_close(csocket_cnode);
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_recv_on_csocket_cnode: read on sockfd %d failed where size %d and used %d\n",
                            CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                            CBUFFER_SIZE(recv_buffer), 
                            CBUFFER_USED(recv_buffer));
        return (EC_FALSE);                            
    }

    CBUFFER_USED(recv_buffer) = (uint32_t)pos;

    dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_recv_on_csocket_cnode: sockfd %d, recv_buffer is [%.*s]\n", 
                        CSOCKET_CNODE_SOCKFD(csocket_cnode), pos, (char *)CBUFFER_DATA(recv_buffer));

    cexpat_parser = CXMPP_NODE_XML_PARSER(cxmpp_node);
    if(EC_FALSE == cexpat_node_parse(cexpat_parser, CBUFFER_DATA(recv_buffer), CBUFFER_USED(recv_buffer)))
    {        
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_recv_on_csocket_cnode: sockfd %d, parse [%.*s] failed\n",
                            CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                            CBUFFER_USED(recv_buffer),
                            CBUFFER_DATA(recv_buffer));
        cxmpp_csocket_cnode_epoll_close(csocket_cnode);
        return (EC_FALSE);                            
    }

    cbuffer_left_shift_out(recv_buffer, NULL_PTR, CBUFFER_USED(recv_buffer));

    if(do_log(SEC_0147_CXMPP, 9))
    {
        if(NULL_PTR != CEXPAT_PARSER_ROOT_NODE(cexpat_parser))
        {
            sys_log(LOGSTDOUT, "[DEBUG] cxmpp_recv_on_csocket_cnode: sockfd %d, parsed root node %p is\n", 
                               CSOCKET_CNODE_SOCKFD(csocket_cnode), CEXPAT_PARSER_ROOT_NODE(cexpat_parser));
            cexpat_node_depth_print_xml_level(LOGSTDOUT, CEXPAT_PARSER_ROOT_NODE(cexpat_parser), 0);
        }
    }
    
    CXMPP_NODE_RECV_CEXPAT_NODE(cxmpp_node) = CEXPAT_PARSER_ROOT_NODE(cexpat_parser);

    /*main handle process*/
    if(EC_FALSE == cxmpp_handle(cxmpp_node))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_recv_on_csocket_cnode: sockfd %d, handle cxmpp_node %p failed\n",
                            CSOCKET_CNODE_SOCKFD(csocket_cnode), cxmpp_node);
        cxmpp_csocket_cnode_epoll_close(csocket_cnode);
        return (EC_FALSE);
    }    
    
    return (EC_TRUE);
}

EC_BOOL cxmpp_send_on_csocket_cnode(CSOCKET_CNODE *csocket_cnode)
{
    CXMPP_NODE *cxmpp_node;
    CBUFFER    *send_cbuffer;
    UINT32      pos;
    
    if(EC_FALSE == CSOCKET_CNODE_IS_CONNECTED(csocket_cnode))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_send_on_csocket_cnode: sockfd %d is not connected\n", 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode));    
        cxmpp_csocket_cnode_epoll_close(csocket_cnode);
        return (EC_FALSE);
    }

    cxmpp_node = CSOCKET_CNODE_CXMPP_NODE(csocket_cnode);
    if(NULL_PTR == cxmpp_node)
    {
        /*nothing to do ??*/    
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_send_on_csocket_cnode: sockfd %d find node is null\n",
                           CSOCKET_CNODE_SOCKFD(csocket_cnode));
        cxmpp_csocket_cnode_epoll_close(csocket_cnode);
        return (EC_FALSE);
    }    

    send_cbuffer = CXMPP_NODE_SEND_CBUFFER(cxmpp_node);

    pos = 0;
    if(EC_FALSE == csocket_write(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                   CBUFFER_DATA(send_cbuffer), 
                                   CBUFFER_USED(send_cbuffer),
                                   &pos))
    {
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_send_on_csocket_cnode: node %p, sockfd %d send %ld bytes failed\n",
                           cxmpp_node,
                           CSOCKET_CNODE_SOCKFD(csocket_cnode),
                           CBUFFER_USED(send_cbuffer)
                           );
        cxmpp_csocket_cnode_epoll_close(csocket_cnode);
        return (EC_FALSE);                           
    }

    dbg_log(SEC_0147_CXMPP, 9)(LOGSTDOUT, "[DEBUG] cxmpp_send_on_csocket_cnode: node %p, sockfd %d, send_buffer [%.*s] done\n",
                       cxmpp_node,
                       CSOCKET_CNODE_SOCKFD(csocket_cnode),
                       pos, CBUFFER_DATA(send_cbuffer)
                       );    

    cbuffer_left_shift_out(send_cbuffer, NULL_PTR, (uint32_t)pos);

    if(0 < CBUFFER_USED(send_cbuffer))
    {
        /*wait for next writing*/
        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)cxmpp_send_on_csocket_cnode_thread, (void *)csocket_cnode);  
    }
    
    return (EC_TRUE);
}

EC_BOOL cxmpp_recv_on_csocket_cnode_thread(CSOCKET_CNODE *csocket_cnode)
{
    CROUTINE_NODE  *croutine_node;
    
    croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd_default_get()), 
                                       (UINT32)cxmpp_recv_on_csocket_cnode, 1, csocket_cnode);
    if(NULL_PTR == croutine_node)
    {
        CSOCKET_CNODE_RETRIES(csocket_cnode) ++;
        
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "warn:cxmpp_recv_on_csocket_cnode_thread: cthread load failed where retried %ld\n", CSOCKET_CNODE_RETRIES(csocket_cnode));
                
        if(CXMPP_OVERLOAD_MAX_RETIRES <= CSOCKET_CNODE_RETRIES(csocket_cnode))
        {
            cxmpp_csocket_cnode_epoll_close(csocket_cnode);
            return (EC_FALSE);
        }
        
        return (EC_TRUE);/*wait for next chance to load*/
    }
    CSOCKET_CNODE_RETRIES(csocket_cnode) = 0;

    //never disable RD event
    //cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_RD_EVENT);
    CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CXMPP_0005);    
    
    return (EC_TRUE);
}

EC_BOOL cxmpp_send_on_csocket_cnode_thread(CSOCKET_CNODE *csocket_cnode)
{
    CROUTINE_NODE  *croutine_node;

    croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd_default_get()), 
                                       (UINT32)cxmpp_send_on_csocket_cnode, 1, csocket_cnode);
    if(NULL_PTR == croutine_node)
    {
        CSOCKET_CNODE_RETRIES(csocket_cnode) ++;
        dbg_log(SEC_0147_CXMPP, 0)(LOGSTDOUT, "error:cxmpp_send_on_csocket_cnode: cthread load failed where retried %ld\n", CSOCKET_CNODE_RETRIES(csocket_cnode));

        if(CXMPP_OVERLOAD_MAX_RETIRES <= CSOCKET_CNODE_RETRIES(csocket_cnode))
        {
            cxmpp_csocket_cnode_epoll_close(csocket_cnode);
            return (EC_FALSE);
        }        
        return (EC_TRUE);/*wait for next chance to load*/
    }
    CSOCKET_CNODE_RETRIES(csocket_cnode) = 0;

    /* note: when load sender in thread, have to prevent same sender was loaded twice.*/
    /* e.g., the previous sender is on-going without return back, WR event was trigger*/
    /*       and try to load the sender again. Thus conflict happen*/
    /*solution: before launch thread, remove WR event, after sender complete, add back*/
    cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);
    
    CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CXMPP_0006);    
    
    return (EC_TRUE);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/
    
