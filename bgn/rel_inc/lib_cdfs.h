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

#ifndef _LIB_CDFS_H
#define _LIB_CDFS_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "lib_type.h"
#include "lib_cstring.h"
#include "lib_cvector.h"

/**
*   for test only
*
*   to query the status of CDFS Module
*
**/
void print_cdfs_status(LOG *log);

/**
*
*   free all static memory occupied by the appointed CDFS module
*
*
**/
UINT32 cdfs_free_module_static_mem(const UINT32 cdfs_md_id);

/**
*
* start CDFS module
*
**/
UINT32 cdfs_start(const UINT32 cdfsnp_min_num);

/**
*
* end CDFS module
*
**/
void cdfs_end(const UINT32 cdfs_md_id);

void * cdfs_get_npp_mod_mgr(const UINT32 cdfs_md_id);

void * cdfs_get_dn_mod_mgr(const UINT32 cdfs_md_id);

/**
*
* initialize mod mgr of CDFS module
*
**/
UINT32 cdfs_set_npp_mod_mgr(const UINT32 cdfs_md_id, const void * src_mod_mgr);

UINT32 cdfs_set_dn_mod_mgr(const UINT32 cdfs_md_id, const void * src_mod_mgr);

/**
*
* get mod mgr of CDFS module
*
**/
void * cdfs_get_npp_mod_mgr(const UINT32 cdfs_md_id);

void * cdfs_get_dn_mod_mgr(const UINT32 cdfs_md_id);

/**
*
*  open name node pool
*
**/
EC_BOOL cdfs_open_npp(const UINT32 cdfs_md_id, const CSTRING *cdfsnp_db_root_dir, const UINT32 cdfsnp_cached_max_num);

/**
*
*  close name node pool
*
**/
EC_BOOL cdfs_close_npp(const UINT32 cdfs_md_id);

/**
*
*  flush and close name node pool
*
**/
EC_BOOL cdfs_close_with_flush_npp(const UINT32 cdfs_md_id);

EC_BOOL cdfs_collect_dn_tcid_vec(const UINT32 cdfs_md_id, void *cdfsdn_tcid_vec);

EC_BOOL cdfs_collect_npp_tcid_vec(const UINT32 cdfs_md_id, void *cdfsnpp_tcid_vec);

/**
*
*  create name node pool
*
**/
EC_BOOL cdfs_create_npp(const UINT32 cdfs_md_id, const UINT32 cdfsnp_mode, const UINT32 cdfsnp_disk_max_num, const UINT32 cdfsnp_support_max_num, const UINT32 cdfsnp_first_chash_algo_id, const UINT32 cdfsnp_second_chash_algo_id, const CSTRING *cdfsnp_db_root_dir);

EC_BOOL cdfs_add_npp(const UINT32 cdfs_md_id, const UINT32 cdfsnpp_tcid);

EC_BOOL cdfs_add_dn(const UINT32 cdfs_md_id, const UINT32 cdfsdn_tcid);

EC_BOOL cdfs_add_dn_vec(const UINT32 cdfs_md_id);

EC_BOOL cdfs_add_npp_vec(const UINT32 cdfs_md_id);

EC_BOOL cdfs_reg_npp(const UINT32 cdfs_md_id, const UINT32 cdfsnpp_tcid);

EC_BOOL cdfs_reg_dn(const UINT32 cdfs_md_id, const UINT32 cdfsdn_tcid);

EC_BOOL cdfs_reg_dn_vec(const UINT32 cdfs_md_id);

EC_BOOL cdfs_reg_npp_vec(const UINT32 cdfs_md_id);

/**
*
*  check existing of a dir
*
**/
EC_BOOL cdfs_find_dir(const UINT32 cdfs_md_id, const CSTRING *dir_path);

/**
*
*  check existing of a file
*
**/
EC_BOOL cdfs_find_file(const UINT32 cdfs_md_id, const CSTRING *file_path);

/**
*
*  check existing of a file or a dir
*
**/
EC_BOOL cdfs_find(const UINT32 cdfs_md_id, const CSTRING *path);

/**
*
*  write a file
*
**/
EC_BOOL cdfs_write(const UINT32 cdfs_md_id, const CSTRING *file_path, const CBYTES *cbytes, const UINT32 replica_num);

/**
*
*  read a file
*
**/
EC_BOOL cdfs_read(const UINT32 cdfs_md_id, const CSTRING *file_path, CBYTES *cbytes);

/**
*
*  create data node
*
**/
EC_BOOL cdfs_create_dn(const UINT32 cdfs_md_id, const CSTRING *root_dir, const UINT32 disk_num, const UINT32 max_gb_num_of_disk_space);

/**
*
*  open data node
*
**/
EC_BOOL cdfs_open_dn(const UINT32 cdfs_md_id, const CSTRING *root_dir);

/**
*
*  close data node
*
**/
EC_BOOL cdfs_close_dn(const UINT32 cdfs_md_id);

/**
*
*  close and flush data node
*
**/
EC_BOOL cdfs_close_with_flush_dn(const UINT32 cdfs_md_id);

/**
*
*  write data node in pipe line
*
**/
EC_BOOL cdfs_write_dn_ppl(const UINT32 cdfs_md_id, const CBYTES *cbytes, const UINT32 cdfsnp_inode_pos, void *cdfsnp_fnode, void *cdfsdn_stat);

/**
*
*  read data node in pipe line
*
**/
EC_BOOL cdfs_read_dn_ppl(const UINT32 cdfs_md_id, const UINT32 cdfsnp_inode_pos, const void *cdfsnp_fnode, CBYTES *cbytes);

/**
*
*  write data node
*
**/
EC_BOOL cdfs_write_dn_p(const UINT32 cdfs_md_id, const CBYTES *cbytes, UINT32 *path_layout, UINT32 *offset);

/**
*
*  read data node
*
**/
EC_BOOL cdfs_read_dn(const UINT32 cdfs_md_id, const void *cdfsnp_fnode, CBYTES *cbytes);

/**
*
*  query a file
*
**/
EC_BOOL cdfs_qfile(const UINT32 cdfs_md_id, const CSTRING *file_path, void  *cdfsnp_item);

/**
*
*  query a dir
*
**/
EC_BOOL cdfs_qdir(const UINT32 cdfs_md_id, const CSTRING *file_path, void  *cdfsnp_item_vec);

/**
*
*  query and list full path of a file or dir
*
**/
EC_BOOL cdfs_qlist_path(const UINT32 cdfs_md_id, const CSTRING *file_path, void  *path_cstr_vec);

/**
*
*  query and list short name of a file or dir
*
**/
EC_BOOL cdfs_qlist_seg(const UINT32 cdfs_md_id, const CSTRING *file_path, void  *seg_cstr_vec);

/**
*
*  flush name node pool
*
**/
EC_BOOL cdfs_flush_npp(const UINT32 cdfs_md_id, const UINT32 cdfsnpp_tcid);

/**
*
*  flush data node
*
*
**/
EC_BOOL cdfs_flush_dn(const UINT32 cdfs_md_id, const UINT32 cdfsdn_tcid);

/**
*
*  check this CDFS is name node pool or not
*
*
**/
EC_BOOL cdfs_is_npp(const UINT32 cdfs_md_id);

/**
*
*  check this CDFS is data node or not
*
*
**/
EC_BOOL cdfs_is_dn(const UINT32 cdfs_md_id);

/**
*
*  list all added or registed name node pool to this CDFS
*
*
**/
EC_BOOL cdfs_list_npp(const UINT32 cdfs_md_id, LOG *log);

/**
*
*  list all added or registed data nodes to this CDFS
*
*
**/
EC_BOOL cdfs_list_dn(const UINT32 cdfs_md_id, LOG *log);

/**
*
*  count file num under specific path
*  if path is regular file, return file_num 1
*  if path is directory, return file num under it
*
**/
EC_BOOL cdfs_file_num(const UINT32 cdfs_md_id, const CSTRING *path_cstr, UINT32 *file_num);

/**
*
*  get file size of specific file given full path name
*
**/
EC_BOOL cdfs_file_size(const UINT32 cdfs_md_id, const CSTRING *path_cstr, UINT32 *file_size);

/**
*
*  check replica num and tcid set and path layout validity
*
**/
EC_BOOL cdfs_check_replicas(const UINT32 cdfs_md_id, const CSTRING *file_path, const UINT32 replica_num, const void *tcid_vec);

/**
*
*  check file content on data node
*
**/
EC_BOOL cdfs_check_file_content(const UINT32 cdfs_md_id, const UINT32 path_layout, const UINT32 offset, const UINT32 file_size, const CSTRING *file_content_cstr);

/**
*
*  check content with sepcific len of all replica files
*
**/
EC_BOOL cdfs_check_replica_files_content(const UINT32 cdfs_md_id, const CSTRING *file_path, const UINT32 file_size, const CSTRING *file_content_cstr);

/**
*
*  check inode info belong to specific cdfsdn block on some tcid
*
**/
EC_BOOL cdfs_figure_out_block(const UINT32 cdfs_md_id, const UINT32 tcid, const UINT32 path_layout, LOG *log);

/**
*
*  show name node pool info if it is npp
*
*
**/
EC_BOOL cdfs_show_npp(const UINT32 cdfs_md_id, LOG *log);

/**
*
*  show cdfsdn info if it is dn
*
*
**/
EC_BOOL cdfs_show_dn(const UINT32 cdfs_md_id, LOG *log);


#endif /*_LIB_LIB_CDFS_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
