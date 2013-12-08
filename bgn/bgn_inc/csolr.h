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

#ifndef _CSOLR_H
#define _CSOLR_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "type.h"
#include "cstring.h"
#include "clist.h"
#include "cvector.h"

#include "csocket.h"
#include "cmutex.h"

#include "mod.inc"

#define CSOLR_OP_REQ        ((UINT8) 1)
#define CSOLR_OP_RSP        ((UINT8) 2)

typedef struct
{
    /* used counter >= 0 */
    UINT32      usedcounter;

    MOD_MGR    *mod_mgr;

    CSTRING    *solr_rsp;
    CROUTINE_COND       ccond;/*waiting for solr_rsp*/

    CSOCKET_CNODE *csocket_cnode;/*socket connection to SOLR*/

}CSOLR_MD;

#define CSOLR_MD_MOD_MGR(csolr_md)              ((csolr_md)->mod_mgr)
#define CSOLR_MD_CSOCKET_CNODE(csolr_md)        ((csolr_md)->csocket_cnode)
#define CSOLR_MD_WAITING_RSP(csolr_md)          ((csolr_md)->solr_rsp)
#define CSOLR_MD_RSP_CCOND(csolr_md)            (&((csolr_md)->ccond))

#define CSOLR_MD_RSP_CCOND_INIT(csolr_md, location)       croutine_cond_init(CSOLR_MD_RSP_CCOND(csolr_md), location);
#define CSOLR_MD_RSP_CCOND_CLEAN(csolr_md, location)      croutine_cond_clean(CSOLR_MD_RSP_CCOND(csolr_md), location);
#define CSOLR_MD_RSP_CCOND_RESERVE(csolr_md, location)    croutine_cond_reserve(CSOLR_MD_RSP_CCOND(csolr_md), 1, location);
#define CSOLR_MD_RSP_CCOND_RELEASE(csolr_md, location)    croutine_cond_release(CSOLR_MD_RSP_CCOND(csolr_md), location);
#define CSOLR_MD_RSP_CCOND_WAIT(csolr_md, location)       croutine_cond_wait(CSOLR_MD_RSP_CCOND(csolr_md), location);

/**
*   for test only
*
*   to query the status of CSOLR Module
*
**/
void csolr_print_module_status(const UINT32 csolr_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CSOLR module
*
*
**/
UINT32 csolr_free_module_static_mem(const UINT32 csolr_md_id);

/**
*
* start CSOLR module
*
**/
UINT32 csolr_start();

/**
*
* end CSOLR module
*
**/
void csolr_end(const UINT32 csolr_md_id);

/**
*
* initialize mod mgr of CSOLR module
*
**/
UINT32 csolr_set_mod_mgr(const UINT32 csolr_md_id, const MOD_MGR * src_mod_mgr);

/**
*
* get mod mgr of CSOLR module
*
**/
MOD_MGR * csolr_get_mod_mgr(const UINT32 csolr_md_id);

EC_BOOL csolr_add_mod(const UINT32 csolr_md_id, const UINT32 csolr_tcid);

EC_BOOL csolr_add_csocket_cnode(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode);

EC_BOOL csolr_del_csocket_cnode(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode);

EC_BOOL csolr_send_req(const UINT32 csolr_md_id, const CSTRING *solr_req);

EC_BOOL csolr_recv_req(const UINT32 csolr_md_id, CSTRING *solr_req);

EC_BOOL csolr_send_rsp(const UINT32 csolr_md_id, const CSTRING *solr_rsp);

EC_BOOL csolr_recv_rsp(const UINT32 csolr_md_id, CSTRING *solr_rsp);

EC_BOOL csolr_do_req(const UINT32 csolr_md_id, const CSTRING *solr_req, CSTRING *solr_rsp);

EC_BOOL csolr_srv_fwd_req(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode);

EC_BOOL csolr_srv_hit_rsp(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode);

EC_BOOL csolr_srv_handle_once(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode);

#endif /*_CSOLR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

