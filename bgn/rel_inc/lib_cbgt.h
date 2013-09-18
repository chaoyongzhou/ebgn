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

#ifndef _LIB_CBGT_H
#define _LIB_CBGT_H

#include "lib_type.h"
#include "lib_mm.h"
#include "lib_log.h"

#include "lib_cbytes.h"

/**
*   for test only
*
*   to query the status of CBGT Module
*
**/
void cbgt_print_module_status(const UINT32 cbgt_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CBGT module
*
*
**/
UINT32 cbgt_free_module_static_mem(const UINT32 cbgt_md_id);

/**
*
* start CBGT module
*
**/
UINT32 cbgt_start(const UINT32 server_type,
                    const UINT32 table_id,
                    const CBYTES *table_name,
                    const void *parent,
                    const CSTRING *root_path,
                    const UINT32 open_flags);

/**
*
* end CBGT module
*
**/
void    cbgt_end(const UINT32 cbgt_md_id);

EC_BOOL cbgt_aging_handle(const UINT32 cbgt_md_id);

UINT32  cbgt_set_mod_mgr(const UINT32 cbgt_md_id, const void * src_mod_mgr);

void *  cbgt_get_mod_mgr(const UINT32 cbgt_md_id);

void    cbgt_close_mod_mgr(const UINT32 cbgt_md_id);

void    cbgt_print_status(const UINT32 cbgt_md_id, LOG *log);

EC_BOOL cbgt_is_root_server(const UINT32 cbgt_md_id);

EC_BOOL cbgt_is_meta_server(const UINT32 cbgt_md_id);

EC_BOOL cbgt_is_colf_server(const UINT32 cbgt_md_id);

EC_BOOL cbgt_is_user_server(const UINT32 cbgt_md_id);

EC_BOOL cbgt_is_user_client(const UINT32 cbgt_md_id);

EC_BOOL cbgt_check_exist(const UINT32 cbgt_md_id, const UINT32 table_id, const void *mod_node);

UINT32  cbgt_fetch_table_id(const UINT32 cbgt_md_id);

EC_BOOL cbgt_merge(const UINT32 cbgt_md_id);

EC_BOOL cbgt_split(const UINT32 cbgt_md_id);

EC_BOOL cbgt_exist_table(const UINT32 cbgt_md_id, const CBYTES *table_name);

EC_BOOL cbgt_create_table_on_root(const UINT32 cbgt_md_id, const CBYTES *table_name, const void *col_family_name_vec);

EC_BOOL cbgt_create_colf_on_meta(const UINT32 cbgt_md_id, const CBYTES  *colf_name);

EC_BOOL cbgt_create_table_on_meta(const UINT32 cbgt_md_id, const void *col_family_name_vec);

EC_BOOL cbgt_create_table_on_colf(const UINT32 cbgt_md_id, const CBYTES *colf_row);

EC_BOOL cbgt_get_colf_table_from_root(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES *colf, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_get_colf_table_from_meta(const UINT32 cbgt_md_id, const CBYTES *colf, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_get_user_table_from_root(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_get_user_table_from_meta(const UINT32 cbgt_md_id, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_get_user_table_from_colf(const UINT32 cbgt_md_id, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_open_colf_table_from_root(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES *colf, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_open_colf_table_from_meta(const UINT32 cbgt_md_id, const CBYTES *colf, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_open_user_table_from_root(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES * row, const CBYTES *colf, const CBYTES * colq, CBYTES *user_table_name, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_open_user_table_from_meta(const UINT32 cbgt_md_id, const CBYTES * row, const CBYTES *colf, const CBYTES * colq, CBYTES *user_table_name, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_open_user_table_from_colf(const UINT32 cbgt_md_id, const CBYTES * row, const CBYTES *colf, const CBYTES * colq, CBYTES *user_table_name, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_close_rmc_table(const UINT32 cbgt_md_id, const CBYTES *table_name, const UINT32 table_id);

EC_BOOL cbgt_close_user_table(const UINT32 cbgt_md_id, const CBYTES *table_name, const UINT32 table_id);

EC_BOOL cbgt_insert_colf(const UINT32 cbgt_md_id, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, const CBYTES *val);

EC_BOOL cbgt_insert(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, const CBYTES *val);

EC_BOOL cbgt_delete_from_user(const UINT32 cbgt_md_id, const CBYTES *row, const CBYTES *colf, const CBYTES *colq);

EC_BOOL cbgt_delete_from_colf(const UINT32 cbgt_md_id, const CBYTES *row, const CBYTES *colf, const CBYTES *colq);

EC_BOOL cbgt_delete(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES *row, const CBYTES *colf, const CBYTES *colq);

EC_BOOL cbgt_search_from_colf(const UINT32 cbgt_md_id, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, CBYTES *val);

EC_BOOL cbgt_search_from_user(const UINT32 cbgt_md_id, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, CBYTES *val);

EC_BOOL cbgt_search(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, CBYTES *val);

EC_BOOL cbgt_fetch_key(const UINT32 cbgt_md_id, const CBYTES *kv_bytes, CBYTES *key_bytes);

EC_BOOL cbgt_fetch_row(const UINT32 cbgt_md_id, const CBYTES *kv_bytes, CBYTES *row_bytes);

EC_BOOL cbgt_fetch_user_table(const UINT32 cbgt_md_id, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, UINT32 *user_table_id, void *user_mod_node);

EC_BOOL cbgt_fetch_from_rmc(const UINT32 cbgt_md_id, const CBYTES *kv_bytes, UINT32 *table_id, void *mod_node);

EC_BOOL cbgt_fetch_from_user(const UINT32 cbgt_md_id, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, CBYTES *val);

EC_BOOL cbgt_fetch(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES *row, const CBYTES *colf, const CBYTES *colq, CBYTES *val);

EC_BOOL cbgt_cleanup_colf_table(const UINT32 cbgt_md_id, const CBYTES *table_name);

EC_BOOL cbgt_cleanup_meta_table(const UINT32 cbgt_md_id, const CBYTES *table_name);

EC_BOOL cbgt_delete_user_table(const UINT32 cbgt_md_id, const CBYTES *table_name);

EC_BOOL cbgt_delete_colf_table(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES *colf_name);

EC_BOOL cbgt_add_colf_table(const UINT32 cbgt_md_id, const CBYTES *table_name, const CBYTES *colf_name);

EC_BOOL cbgt_select_from_user(const UINT32 cbgt_md_id, const CSTRING *row_pattern, const CSTRING *colf_pattern, const CSTRING *colq_pattern, const CSTRING *val_pattern, void *ret_kv_vec);

EC_BOOL cbgt_select_from_colf(const UINT32 cbgt_md_id, const UINT32 cached_mode, const CSTRING *row_pattern, const CSTRING *colf_pattern, const CSTRING *colq_pattern, const CSTRING *val_pattern, void *ret_kv_vec);

EC_BOOL cbgt_select_from_meta(const UINT32 cbgt_md_id, const UINT32 cached_mode, const CSTRING *row_pattern, const CSTRING *colf_pattern, const CSTRING *colq_pattern, const CSTRING *val_pattern, void *ret_kv_vec);

EC_BOOL cbgt_select_from_root(const UINT32 cbgt_md_id, const UINT32 cached_mode, const CSTRING *table_pattern, const CSTRING *row_pattern, const CSTRING *colf_pattern, const CSTRING *colq_pattern, const CSTRING *val_pattern, void *ret_kv_vec);

EC_BOOL cbgt_select_in_meta(const UINT32 cbgt_md_id, const UINT32 cached_mode, const CBYTES *table_name, const CSTRING *row_pattern, const CSTRING *colf_pattern, const CSTRING *colq_pattern, const CSTRING *val_pattern, void *ret_kv_vec);

EC_BOOL cbgt_select_in_colf(const UINT32 cbgt_md_id, const UINT32 cached_mode, const CBYTES *table_name, const CBYTES *colf_name, const CSTRING *row_pattern,  const CSTRING *colq_pattern, const CSTRING *val_pattern, void *ret_kv_vec);

EC_BOOL cbgt_select(const UINT32 cbgt_md_id, const UINT32 cached_mode, const CSTRING *table_pattern, const CSTRING *row_pattern, const CSTRING *colf_pattern, const CSTRING *colq_pattern, const CSTRING *val_pattern, void *ret_kv_vec);

void cbgt_traversal_no_lock(const UINT32 cbgt_md_id, LOG *log);

void cbgt_traversal(const UINT32 cbgt_md_id, LOG *log);

void cbgt_runthrough_no_lock(const UINT32 cbgt_md_id, LOG *log);

void cbgt_runthrough(const UINT32 cbgt_md_id, LOG *log);

void cbgt_traversal_depth(const UINT32 cbgt_md_id, LOG *log);

void cbgt_runthrough_depth(const UINT32 cbgt_md_id, LOG *log);
#endif/* _LIB_CBGT_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/
