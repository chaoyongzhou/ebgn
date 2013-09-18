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

#ifndef _LIB_CDIR_H
#define _LIB_CDIR_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "lib_type.h"
#include "lib_cstring.h"
#include "lib_log.h"


#define CDIR_DIR_SEG          ((UINT32) 1)
#define CDIR_FILE_SEG         ((UINT32) 2)
#define CDIR_ERR_SEG          ((UINT32)-1)

/**
*   for test only
*
*   to query the status of CDIR Module
*
**/
void print_cdir_status(LOG *log);

/**
*
*   free all static memory occupied by the appointed CDIR module
*
*
**/
UINT32 cdir_free_module_static_mem(const UINT32 cdir_md_id);

/**
*
* start CDIR module
*
**/
UINT32 cdir_start(const void *node_tcid_vec);

/**
*
* end CDIR module
*
**/
void cdir_end(const UINT32 cdir_md_id);

/**
*
* initialize mod mgr of CDIR module
*
**/
UINT32 cdir_set_mod_mgr(const UINT32 cdir_md_id, const void * src_mod_mgr);

/**
*
* get mod mgr of CDIR module
*
**/
void * cdir_get_mod_mgr(const UINT32 cdir_md_id);


void * cdir_seg_new(const UINT32 cdir_md_id);

UINT32 cdir_seg_init(const UINT32 cdir_md_id, void *cdir_seg);

UINT32 cdir_seg_clean(const UINT32 cdir_md_id, void *cdir_seg);

UINT32 cdir_seg_free(const UINT32 cdir_md_id, void *cdir_seg);

UINT32 cdir_seg_set(const UINT32 cdir_md_id, const UINT32 seg_type, const CSTRING *seg_name, void *cdir_seg);

UINT32 cdir_seg_rmv(const UINT32 cdir_md_id, const void *cdir_seg);

EC_BOOL cdir_seg_is_dir(const UINT32 cdir_md_id, const void *cdir_seg);

UINT32 cdir_seg_print(LOG *log, const void *cdir_seg, const UINT32 level);
UINT32 cdir_seg_xml_print(LOG *log, const void *cdir_seg, const UINT32 level);

void * cdir_node_new(const UINT32 cdir_md_id);

UINT32 cdir_node_init(const UINT32 cdir_md_id, void *cdir_node);

UINT32 cdir_node_clean(const UINT32 cdir_md_id, void *cdir_node);

UINT32 cdir_node_free(const UINT32 cdir_md_id, void *cdir_node);

UINT32 cdir_node_print(LOG *log, const void *cdir_node, const UINT32 level);
UINT32 cdir_node_xml_print(LOG *log, const void *cdir_node, const UINT32 level);

EC_BOOL cdir_is_dir_on_node_tcid(const UINT32 cdir_md_id, const CSTRING *path_name, const UINT32 node_tcid);

UINT32 cdir_create_on_node_tcid(const UINT32 cdir_md_id, const CSTRING *dir_name, const UINT32 mode, const UINT32 node_tcid);

UINT32 cdir_read_on_node_tcid(const UINT32 cdir_md_id, const CSTRING *dir_name, void * cdir_node, const UINT32 node_tcid);

UINT32 cdir_clean_on_node_tcid(const UINT32 cdir_md_id, const void *cdir_node, const UINT32 node_tcid);

UINT32 cdir_rmv_on_node_tcid(const UINT32 cdir_md_id, const void * cdir_node, const UINT32 node_tcid);

EC_BOOL cdir_search_trans(const UINT32 cdir_md_id, const CSTRING *dir_name, UINT32 *node_tcid);

EC_BOOL cdir_is_dir_trans(const UINT32 cdir_md_id, const CSTRING *path_name, UINT32 *node_tcid);

UINT32 cdir_create_trans(const UINT32 cdir_md_id, const CSTRING *dir_name, const UINT32 mode);

UINT32 cdir_read_trans(const UINT32 cdir_md_id, const CSTRING *dir_name, void * cdir_node);

UINT32 cdir_clean_trans(const UINT32 cdir_md_id, const void *cdir_node);;

UINT32 cdir_rmv_trans(const UINT32 cdir_md_id, const void * cdir_node);

UINT32 cdir_cwd(const UINT32 cdir_md_id, const void * cdir_node, CSTRING *cwd);

#endif /*_LIB_CDIR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

