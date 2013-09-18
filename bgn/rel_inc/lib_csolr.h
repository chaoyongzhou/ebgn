/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#ifndef _LIB_CSOLR_H
#define _LIB_CSOLR_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "lib_type.h"
#include "lib_cstring.h"


/**
*   for test only
*
*   to query the status of CSOLR Module
*
**/
void print_csolr_status(LOG *log);

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

EC_BOOL csolr_add_mod(const UINT32 csolr_md_id, const UINT32 csolr_tcid);

EC_BOOL csolr_send_req(const UINT32 csolr_md_id, const CSTRING *solr_req);

EC_BOOL csolr_recv_req(const UINT32 csolr_md_id, CSTRING *solr_req);

EC_BOOL csolr_send_rsp(const UINT32 csolr_md_id, const CSTRING *solr_rsp);

EC_BOOL csolr_recv_rsp(const UINT32 csolr_md_id, CSTRING *solr_rsp);

EC_BOOL csolr_do_req(const UINT32 csolr_md_id, const CSTRING *solr_req, CSTRING *solr_rsp);

#endif /*_LIB_CSOLR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

