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

#ifndef _CXMPPC2S_H
#define _CXMPPC2S_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "type.h"
#include "cstring.h"
#include "clist.h"
#include "cvector.h"

#include "csocket.h"
#include "cbtimer.h"
#include "mod.inc"

/*jid = usrname@domain/resource*/
typedef struct
{
    UINT32                  cxmppc2s_md_id;
    CSTRING                 domain;
    CSTRING                 resource;
    CSOCKET_CNODE          *csocket_cnode;
}CXMPPC2S_CONN;

#define CXMPPC2S_CONN_MD_ID(cxmppc2s_conn)         ((cxmppc2s_conn)->cxmppc2s_md_id)
#define CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn)        (&((cxmppc2s_conn)->domain))
#define CXMPPC2S_CONN_RESOURCE(cxmppc2s_conn)      (&((cxmppc2s_conn)->resource))
#define CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn) ((cxmppc2s_conn)->csocket_cnode)

#define CXMPPC2S_CONN_DOMAIN_STR(cxmppc2s_conn)    ((char *)cstring_get_str(CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn)))
#define CXMPPC2S_CONN_RESOURCE_STR(cxmppc2s_conn)  ((char *)cstring_get_str(CXMPPC2S_CONN_RESOURCE(cxmppc2s_conn)))

typedef struct
{
    /* used counter >= 0 */
    UINT32              usedcounter;

    CROUTINE_RWLOCK     crwlock;

    CSTRING             usrname;
    CLIST               conn_list;/*item is CXMPPC2S_CONN*/    
}CXMPPC2S_MD;

#define CXMPPC2S_MD_CRWLOCK(cxmppc2s_md)            (&((cxmppc2s_md)->crwlock))
#define CXMPPC2S_MD_USRNAME(cxmppc2s_md)            (&((cxmppc2s_md)->usrname))
#define CXMPPC2S_MD_CONN_LIST(cxmppc2s_md)          (&((cxmppc2s_md)->conn_list))

#define CXMPPC2S_MD_USRNAME_STR(cxmppc2s_md)        ((char *)cstring_get_str(CXMPPC2S_MD_USRNAME(cxmppc2s_md)))

#if 0
#define CXMPPC2S_INIT_LOCK(cxmppc2s_md, location)  (croutine_rwlock_init(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), CMUTEX_PROCESS_PRIVATE, location))
#define CXMPPC2S_CLEAN_LOCK(cxmppc2s_md, location) (croutine_rwlock_clean(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), location))

#define CXMPPC2S_RDLOCK(cxmppc2s_md, location)     (croutine_rwlock_rdlock(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), location))
#define CXMPPC2S_WRLOCK(cxmppc2s_md, location)     (croutine_rwlock_wrlock(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), location))
#define CXMPPC2S_UNLOCK(cxmppc2s_md, location)     (croutine_rwlock_unlock(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), location))
#endif

#if 1
#define CXMPPC2S_MD_INIT_LOCK(cxmppc2s_md, location)  (croutine_rwlock_init(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), CMUTEX_PROCESS_PRIVATE, location))
#define CXMPPC2S_MD_CLEAN_LOCK(cxmppc2s_md, location) (croutine_rwlock_clean(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), location))
#define CXMPPC2S_MD_RDLOCK(cxmppc2s_md, location)     (croutine_rwlock_rdlock(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), location))
#define CXMPPC2S_MD_WRLOCK(cxmppc2s_md, location)     (croutine_rwlock_wrlock(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), location))
#define CXMPPC2S_MD_UNLOCK(cxmppc2s_md, location)     (croutine_rwlock_unlock(CXMPPC2S_MD_CRWLOCK(cxmppc2s_md), location))
#endif

/**
 * *   for test only
 * *
 * *   to query the status of CXMPPC2S Module
 * *
 * **/
void cxmppc2s_print_module_status(const UINT32 cxmppc2s_md_id, LOG *log);

/**
 * *
 * *   free all static memory occupied by the appointed CXMPPC2S module
 * *
 * *
 * **/
UINT32 cxmppc2s_free_module_static_mem(const UINT32 cxmppc2s_md_id);

UINT32 cxmppc2s_find_module(const uint8_t *usrname);

/**
 * *
 * * start CXMPPC2S module
 * *
 * **/
UINT32 cxmppc2s_start();

/**
 * *
 * * end CXMPPC2S module
 * *
 * **/
void cxmppc2s_end(const UINT32 cxmppc2s_md_id);

CXMPPC2S_CONN *cxmppc2s_conn_new(const UINT32 cxmppc2s_md_id);

EC_BOOL cxmppc2s_conn_init(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn);

EC_BOOL cxmppc2s_conn_clean(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn);

EC_BOOL cxmppc2s_conn_free(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn);

CSTRING *cxmppc2s_conn_make_jid(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn);

CSTRING *cxmppc2s_conn_make_jid_no_resource(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn);

EC_BOOL cxmppc2s_set_usrname(const UINT32 cxmppc2s_md_id, const CSTRING *usrname);

/*internal interface*/
CSTRING *cxmppc2s_get_usrname(const UINT32 cxmppc2s_md_id);

CXMPPC2S_CONN *cxmppc2s_add_conn(const UINT32 cxmppc2s_md_id, const CSTRING *domain, const CSTRING *resource, const CSOCKET_CNODE *csocket_cnode);

CXMPPC2S_CONN *cxmppc2s_del_conn(const UINT32 cxmppc2s_md_id, const CSTRING *domain, const CSTRING *resource);

CXMPPC2S_CONN *cxmppc2s_rm_conn(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn);

CXMPPC2S_CONN *cxmppc2s_find_conn(const UINT32 cxmppc2s_md_id, const CSTRING *domain, const CSTRING *resource);

EC_BOOL cxmppc2s_is_me(const UINT32 cxmppc2s_md_id, const CSTRING *to, CXMPPC2S_CONN **cxmppc2s_conn);

EC_BOOL cxmppc2s_get_mod_node(const UINT32 cxmppc2s_md_id, const CSTRING *to, MOD_NODE *mod_node);

EC_BOOL cxmppc2s_send_message(const UINT32 cxmppc2s_md_id, const CSTRING *to, const CBUFFER *send_message_buffer);

EC_BOOL cxmppc2s_presence_subscribe(const UINT32 cxmppc2s_md_id, const CSTRING *to, const CBUFFER *send_subscribe_buffer);

EC_BOOL cxmppc2s_presence_subscribed(const UINT32 cxmppc2s_md_id, const CSTRING *to, const CBUFFER *send_subscribed_buffer);


#endif /*_CXMPPC2S_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/


