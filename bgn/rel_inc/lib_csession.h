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

#ifndef _LIB_CSESSION_H
#define _LIB_CSESSION_H

#include "lib_type.h"
#include "lib_mm.h"
#include "lib_log.h"

#include "lib_cstring.h"
#include "lib_cbytes.h"

/**
*   for test only
*
*   to query the status of CSESSION Module
*
**/
void csession_print_module_status(const UINT32 csession_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CSESSION module
*
*
**/
UINT32 csession_free_module_static_mem(const UINT32 csession_md_id);

/**
*
* start CSESSION module
*
**/
UINT32 csession_start();

/**
*
* end CSESSION module
*
**/
void csession_end(const UINT32 csession_md_id);

void csession_print(LOG *log, const UINT32 csession_md_id, const UINT32 level);

void *csession_node_new(const UINT32 csession_md_id, const CSTRING *name, const UINT32 expire_nsec);

EC_BOOL csession_node_init(const UINT32 csession_md_id, void *csession_node);

EC_BOOL csession_node_clean(const UINT32 csession_md_id, void *csession_node);

EC_BOOL csession_node_free(const UINT32 csession_md_id, void *csession_node);

EC_BOOL csession_node_is_expired(const UINT32 csession_md_id, const void *csession_node, const void *cur_time);

EC_BOOL csession_node_match_name(const void *csession_node, const CSTRING *name);

EC_BOOL csession_node_match_id(const void *csession_node, const UINT32 session_id);

void csession_node_print(LOG *log, const void *csession_node, const UINT32 level);

void *csession_item_new(const UINT32 csession_md_id, const CSTRING *key, const CBYTES *val);

EC_BOOL csession_item_init(const UINT32 csession_md_id, void *csession_item);

EC_BOOL csession_item_clean(const UINT32 csession_md_id, void *csession_item);

EC_BOOL csession_item_free(const UINT32 csession_md_id, void *csession_item);

EC_BOOL csession_item_match_key(const void *csession_item, const CSTRING *key);

EC_BOOL csession_item_match_val(const void *csession_item, const CBYTES *val);

void csession_item_print(LOG *log, const void *csession_item, const UINT32 level);

void csession_item_list_print(LOG *log, const void *csession_item_list, const UINT32 level);

void *csession_search_by_name(const UINT32 csession_md_id, const CSTRING *name);

void *csession_search_by_id(const UINT32 csession_md_id, const UINT32 session_id);

EC_BOOL csession_add(const UINT32 csession_md_id, const CSTRING *name, const UINT32 expire_nsec);

EC_BOOL csession_rmv_by_name(const UINT32 csession_md_id, const CSTRING *name);

EC_BOOL csession_rmv_by_id(const UINT32 csession_md_id, const UINT32 session_id);

EC_BOOL csession_get_name(const UINT32 csession_md_id, const UINT32 session_id, CSTRING *session_name);

EC_BOOL csession_get_id(const UINT32 csession_md_id, const CSTRING *session_name, UINT32 *session_id);

/*note: path is the full path of key. e.g., top=root&level1=b&level2=c*/
EC_BOOL csession_set(const UINT32 csession_md_id, void *csession_node, const CSTRING *path, const CBYTES *val);

EC_BOOL csession_set_by_name(const UINT32 csession_md_id, const CSTRING *session_name, const CSTRING *path, const CBYTES *val);

EC_BOOL csession_set_by_id(const UINT32 csession_md_id, const UINT32 session_id, const CSTRING *path, const CBYTES *val);

/*note: path is the full path of key with wildcards. e.g., top=root&level1=*&level2=c*x*/
EC_BOOL csession_get(const UINT32 csession_md_id, const void *csession_node, const CSTRING *path, void *csession_item_list);

EC_BOOL csession_get_by_name(const UINT32 csession_md_id, const CSTRING *session_name, const CSTRING *path, void *csession_item_list);

EC_BOOL csession_get_by_id(const UINT32 csession_md_id, const UINT32 session_id, const CSTRING *path, void *csession_item_list);


#endif/*_LIB_CSESSION_H*/
#ifdef __cplusplus
}
#endif/*__cplusplus*/

