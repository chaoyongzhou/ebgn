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

#ifndef _CRFS_H
#define _CRFS_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "type.h"
#include "cstring.h"
#include "clist.h"
#include "cvector.h"

#include "csocket.h"

#include "mod.inc"

#include "crfsnp.h"
#include "crfsdn.h"
#include "crfsnpmgr.h"

#define CRFS_FILE_PAD_CHAR                  (0x00)
//#define CRFS_FILE_PAD_CHAR                  ((uint8_t)'.')

#define CRFS_BIGFILE_MAX_SIZE               ((uint64_t)(((uint64_t)1) << 46))/*64TB*/

#define CRFS_OP_WRITE                       ((UINT8)  1)
#define CRFS_OP_READ                        ((UINT8)  2)
#define CRFS_OP_GET_WORDSIZE                ((UINT8)  3)
#define CRFS_OP_QLIST_PATH                  ((UINT8)  4)
#define CRFS_OP_MKDIR                       ((UINT8)  5)
#define CRFS_OP_EXISTS                      ((UINT8)  6)
#define CRFS_OP_IS_FILE                     ((UINT8)  7)
#define CRFS_OP_IS_DIR                      ((UINT8)  8)
#define CRFS_OP_IS_QFILE                    ((UINT8)  9)
#define CRFS_OP_IS_QDIR                     ((UINT8) 10)

typedef struct
{
    /* used counter >= 0 */
    UINT32      usedcounter;
    EC_BOOL     terminate_flag;

    MOD_MGR    *crfsdn_mod_mgr;
    MOD_MGR    *crfsnpp_mod_mgr;

    CRFSDN     *crfsdn;
    CRFSNP_MGR *crfsnpmgr;/*namespace pool*/    

    CROUTINE_RWLOCK     crwlock;
}CRFS_MD;

#define CRFS_MD_TERMINATE_FLAG(crfs_md)  ((crfs_md)->terminate_flag)
#define CRFS_MD_DN_MOD_MGR(crfs_md)      ((crfs_md)->crfsdn_mod_mgr)
#define CRFS_MD_NPP_MOD_MGR(crfs_md)     ((crfs_md)->crfsnpp_mod_mgr)
#define CRFS_MD_DN(crfs_md)              ((crfs_md)->crfsdn)
#define CRFS_MD_NPP(crfs_md)             ((crfs_md)->crfsnpmgr)
#define CRFS_CRWLOCK(crfs_md)            (&((crfs_md)->crwlock))

#if 0
#define CRFS_INIT_LOCK(crfs_md, location)  (croutine_rwlock_init(CRFS_CRWLOCK(crfs_md), CMUTEX_PROCESS_PRIVATE, location))
#define CRFS_CLEAN_LOCK(crfs_md, location) (croutine_rwlock_clean(CRFS_CRWLOCK(crfs_md), location))

#define CRFS_RDLOCK(crfs_md, location)     (croutine_rwlock_rdlock(CRFS_CRWLOCK(crfs_md), location))
#define CRFS_WRLOCK(crfs_md, location)     (croutine_rwlock_wrlock(CRFS_CRWLOCK(crfs_md), location))
#define CRFS_UNLOCK(crfs_md, location)     (croutine_rwlock_unlock(CRFS_CRWLOCK(crfs_md), location))
#endif

#if 1
#define CRFS_INIT_LOCK(crfs_md, location)  do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFS_INIT_LOCK: CRFS_CRWLOCK %p, at %s:%ld\n", CRFS_CRWLOCK(crfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_init(CRFS_CRWLOCK(crfs_md), CMUTEX_PROCESS_PRIVATE, location);\
}while(0)

#define CRFS_CLEAN_LOCK(crfs_md, location) do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFS_CLEAN_LOCK: CRFS_CRWLOCK %p, at %s:%ld\n", CRFS_CRWLOCK(crfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_clean(CRFS_CRWLOCK(crfs_md), location);\
}while(0)    

#define CRFS_RDLOCK(crfs_md, location)     do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFS_RDLOCK: CRFS_CRWLOCK %p, at %s:%ld\n", CRFS_CRWLOCK(crfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_rdlock(CRFS_CRWLOCK(crfs_md), location);\
    sys_log(LOGSTDOUT, "[DEBUG] CRFS_RDLOCK: CRFS_CRWLOCK %p, at %s:%ld done\n", CRFS_CRWLOCK(crfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
}while(0)

#define CRFS_WRLOCK(crfs_md, location)     do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFS_WRLOCK: CRFS_CRWLOCK %p, at %s:%ld\n", CRFS_CRWLOCK(crfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_wrlock(CRFS_CRWLOCK(crfs_md), location);\
    sys_log(LOGSTDOUT, "[DEBUG] CRFS_WRLOCK: CRFS_CRWLOCK %p, at %s:%ld done\n", CRFS_CRWLOCK(crfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
}while(0)
#define CRFS_UNLOCK(crfs_md, location)     do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFS_UNLOCK: CRFS_CRWLOCK %p, at %s:%ld\n", CRFS_CRWLOCK(crfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_unlock(CRFS_CRWLOCK(crfs_md), location);\
}while(0)
#endif



/**
*   for test only
*
*   to query the status of CRFS Module
*
**/
void crfs_print_module_status(const UINT32 crfs_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CRFS module
*
*
**/
UINT32 crfs_free_module_static_mem(const UINT32 crfs_md_id);

/**
*
* start CRFS module
*
**/
UINT32 crfs_start(const CSTRING *crfsnp_root_dir, const CSTRING *crfsdn_root_dir);

/**
*
* end CRFS module
*
**/
void crfs_end(const UINT32 crfs_md_id);


/**
*
* initialize mod mgr of CRFS module
*
**/
UINT32 crfs_set_npp_mod_mgr(const UINT32 crfs_md_id, const MOD_MGR * src_mod_mgr);

UINT32 crfs_set_dn_mod_mgr(const UINT32 crfs_md_id, const MOD_MGR * src_mod_mgr);

/**
*
* get mod mgr of CRFS module
*
**/
MOD_MGR * crfs_get_npp_mod_mgr(const UINT32 crfs_md_id);

MOD_MGR * crfs_get_dn_mod_mgr(const UINT32 crfs_md_id);

CRFSNP_FNODE *crfs_fnode_new(const UINT32 crfs_md_id);

EC_BOOL crfs_fnode_init(const UINT32 crfs_md_id, CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfs_fnode_clean(const UINT32 crfs_md_id, CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfs_fnode_free(const UINT32 crfs_md_id, CRFSNP_FNODE *crfsnp_fnode);


/**
*
*  get name node pool of the module
*
**/
CRFSNP_MGR *crfs_get_npp(const UINT32 crfs_md_id);

/**
*
*  get data node of the module
*
**/
CRFSDN *crfs_get_dn(const UINT32 crfs_md_id);

/**
*
*  open name node pool
*
**/
EC_BOOL crfs_open_npp(const UINT32 crfs_md_id, const CSTRING *crfsnp_db_root_dir);

/**
*
*  flush and close name node pool
*
**/
EC_BOOL crfs_close_npp(const UINT32 crfs_md_id);

/**
*
*  check this CRFS is name node pool or not
*
*
**/
EC_BOOL crfs_is_npp(const UINT32 crfs_md_id);

/**
*
*  check this CRFS is data node or not
*
*
**/
EC_BOOL crfs_is_dn(const UINT32 crfs_md_id);

/**
*
*  check this CRFS is data node and namenode or not
*
*
**/
EC_BOOL crfs_is_npp_and_dn(const UINT32 crfs_md_id);

/**
*
*  create name node pool
*
**/
EC_BOOL crfs_create_npp(const UINT32 crfs_md_id, 
                             const UINT32 crfsnp_model, 
                             const UINT32 crfsnp_max_num, 
                             const UINT32 crfsnp_1st_chash_algo_id, 
                             const UINT32 crfsnp_2nd_chash_algo_id, 
                             const CSTRING *crfsnp_db_root_dir);

EC_BOOL crfs_add_npp(const UINT32 crfs_md_id, const UINT32 crfsnpp_tcid, const UINT32 crfsnpp_rank);

EC_BOOL crfs_add_dn(const UINT32 crfs_md_id, const UINT32 crfsdn_tcid, const UINT32 crfsdn_rank);

/**
*
*  check existing of a dir
*
**/
EC_BOOL crfs_find_dir(const UINT32 crfs_md_id, const CSTRING *dir_path);

/**
*
*  check existing of a file
*
**/
EC_BOOL crfs_find_file(const UINT32 crfs_md_id, const CSTRING *file_path);

/**
*
*  check existing of a file or a dir
*
**/
EC_BOOL crfs_find(const UINT32 crfs_md_id, const CSTRING *path);

/**
*
*  check existing of a file or a dir
*
**/
EC_BOOL crfs_exists(const UINT32 crfs_md_id, const CSTRING *path);

/**
*
*  check existing of a file
*
**/
EC_BOOL crfs_is_file(const UINT32 crfs_md_id, const CSTRING *file_path);

/**
*
*  check existing of a dir
*
**/
EC_BOOL crfs_is_dir(const UINT32 crfs_md_id, const CSTRING *dir_path);

/**
*
*  set home dir of a name node
*
**/
EC_BOOL crfs_set(const UINT32 crfs_md_id, const CSTRING *home_dir, const UINT32 crfsnp_id);

/**
*
*  unset/unbind home dir of a name node
*
**/
EC_BOOL crfs_unset(const UINT32 crfs_md_id, const CSTRING *home_dir, const UINT32 crfsnp_id);

/**
*
*  reserve space from dn
*
**/
EC_BOOL crfs_reserve_dn(const UINT32 crfs_md_id, const UINT32 data_len, CRFSNP_FNODE *crfsnp_fnode);

/**
*
*  release space to dn
*
**/
EC_BOOL crfs_release_dn(const UINT32 crfs_md_id, const CRFSNP_FNODE *crfsnp_fnode);

/**
*
*  write a file
*
**/
EC_BOOL crfs_write(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes);

#if 0
/**
*
*  write a file in cache
*
**/
EC_BOOL crfs_write_cache(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes);
#endif
/**
*
*  read a file
*
**/
EC_BOOL crfs_read(const UINT32 crfs_md_id, const CSTRING *file_path, CBYTES *cbytes);

/**
*
*  write a file at offset
*
**/
EC_BOOL crfs_write_e(const UINT32 crfs_md_id, const CSTRING *file_path, UINT32 *offset, const UINT32 max_len, const CBYTES *cbytes);

/**
*
*  read a file from offset
*
**/
EC_BOOL crfs_read_e(const UINT32 crfs_md_id, const CSTRING *file_path, UINT32 *offset, const UINT32 max_len, CBYTES *cbytes);

/**
*
*  create a big file at offset
*
**/
EC_BOOL crfs_create_b(const UINT32 crfs_md_id, const CSTRING *file_path);

/**
*
*  write a big file at offset
*
**/
EC_BOOL crfs_write_b(const UINT32 crfs_md_id, const CSTRING *file_path, uint64_t *offset, const UINT32 max_len, const CBYTES *cbytes);

/**
*
*  read a file from offset
*
**/
EC_BOOL crfs_read_b(const UINT32 crfs_md_id, const CSTRING *file_path, uint64_t *offset, const UINT32 max_len, CBYTES *cbytes);

/**
*
*  create data node
*
**/
EC_BOOL crfs_create_dn(const UINT32 crfs_md_id, const CSTRING *root_dir);

/**
*
*  add a disk to data node
*
**/
EC_BOOL crfs_add_disk(const UINT32 crfs_md_id, const UINT32 disk_no);

/**
*
*  delete a disk from data node
*
**/
EC_BOOL crfs_del_disk(const UINT32 crfs_md_id, const UINT32 disk_no);

/**
*
*  mount a disk to data node
*
**/
EC_BOOL crfs_mount_disk(const UINT32 crfs_md_id, const UINT32 disk_no);

/**
*
*  umount a disk from data node
*
**/
EC_BOOL crfs_umount_disk(const UINT32 crfs_md_id, const UINT32 disk_no);

/**
*
*  open data node
*
**/
EC_BOOL crfs_open_dn(const UINT32 crfs_md_id, const CSTRING *root_dir);

/**
*
*  close data node
*
**/
EC_BOOL crfs_close_dn(const UINT32 crfs_md_id);

/**
*
*  export data into data node
*
**/
EC_BOOL crfs_export_dn(const UINT32 crfs_md_id, const CBYTES *cbytes, const CRFSNP_FNODE *crfsnp_fnode);

/**
*
*  write data node
*
**/
EC_BOOL crfs_write_dn(const UINT32 crfs_md_id, const CBYTES *cbytes, CRFSNP_FNODE *crfsnp_fnode);

/**
*
*  write data node in cache
*
**/
EC_BOOL crfs_write_dn_cache(const UINT32 crfs_md_id, const CBYTES *cbytes, CRFSNP_FNODE *crfsnp_fnode);

/**
*
*  read data node
*
**/
EC_BOOL crfs_read_dn(const UINT32 crfs_md_id, const CRFSNP_FNODE *crfsnp_fnode, CBYTES *cbytes);

/**
*
*  write data node at offset in the specific file
*
**/
EC_BOOL crfs_write_e_dn(const UINT32 crfs_md_id, CRFSNP_FNODE *crfsnp_fnode, UINT32 *offset, const UINT32 max_len, const CBYTES *cbytes);

/**
*
*  read data node from offset in the specific file
*
**/
EC_BOOL crfs_read_e_dn(const UINT32 crfs_md_id, const CRFSNP_FNODE *crfsnp_fnode, UINT32 *offset, const UINT32 max_len, CBYTES *cbytes);

/*when found missed segment in bnode*/
static EC_BOOL __crfs_write_b_seg_dn_miss(const UINT32 crfs_md_id, 
                                              CRFSNP *crfsnp, 
                                              const uint32_t parent_pos, 
                                              UINT32 *offset, const CBYTES *cbytes,
                                              const uint32_t key_1st_hash, const uint32_t key_2nd_hash, 
                                              const uint32_t klen, const uint8_t *key);
static EC_BOOL __crfs_write_b_seg_dn_hit(const UINT32 crfs_md_id, 
                                           CRFSNP *crfsnp,
                                           const uint32_t node_pos, 
                                           UINT32 *offset, const CBYTES *cbytes,
                                           const uint32_t key_1st_hash, const uint32_t key_2nd_hash, 
                                           const uint32_t klen, const uint8_t *key);
static EC_BOOL __crfs_write_b_seg_dn(const UINT32 crfs_md_id, 
                                             CRFSNP *crfsnp, const uint32_t parent_pos, 
                                             const uint32_t seg_no, UINT32 *offset, 
                                             const UINT32 max_len, const CBYTES *cbytes);
static EC_BOOL __crfs_write_b_dn(const UINT32 crfs_md_id, const uint32_t crfsnp_id, const uint32_t parent_pos, uint64_t *offset, const UINT32 max_len, const CBYTES *cbytes);

static EC_BOOL __crfs_read_b_seg_dn_miss(const UINT32 crfs_md_id, 
                                                    UINT32 *offset, 
                                                    const UINT32 max_len, CBYTES *cbytes);
static EC_BOOL __crfs_read_b_seg_dn_hit(const UINT32 crfs_md_id, 
                                                CRFSNP *crfsnp, 
                                                const uint32_t node_pos, 
                                                UINT32 *offset, 
                                                CBYTES *cbytes);

static EC_BOOL __crfs_read_b_seg_dn(const UINT32 crfs_md_id, 
                                            CRFSNP *crfsnp, 
                                            const uint32_t parent_pos, 
                                            const uint32_t seg_no, UINT32 *offset, 
                                            const UINT32 max_len, CBYTES *cbytes);                                                
/**
*
*  read data node at offset from the specific big file
*
**/
static EC_BOOL __crfs_read_b_dn(const UINT32 crfs_md_id, const uint32_t crfsnp_id, const uint32_t parent_pos, uint64_t *offset, const UINT32 max_len, CBYTES *cbytes);

/**
*
*  write a fnode to name node
*
**/
EC_BOOL crfs_write_npp(const UINT32 crfs_md_id, const CSTRING *file_path, const CRFSNP_FNODE *crfsnp_fnode);

/**
*
*  read a fnode from name node
*
**/
EC_BOOL crfs_read_npp(const UINT32 crfs_md_id, const CSTRING *file_path, CRFSNP_FNODE *crfsnp_fnode);

/**
*
*  write a bnode to name node
*
**/
static EC_BOOL __crfs_write_b_npp(const UINT32 crfs_md_id, const CSTRING *file_path);

/**
*
*  read a bnode from name node
*
**/
static EC_BOOL __crfs_read_b_npp(const UINT32 crfs_md_id, const CSTRING *file_path, uint32_t *crfsnp_id, uint32_t *parent_pos);

/**
*
*  update a fnode to name node
*
**/
EC_BOOL crfs_update_npp(const UINT32 crfs_md_id, const CSTRING *file_path, const CRFSNP_FNODE *crfsnp_fnode);

/**
*
*  delete a file or dir from current npp
*
**/
EC_BOOL crfs_delete_npp(const UINT32 crfs_md_id, const CSTRING *path, const UINT32 dflag, uint32_t *crfsnp_id, CVECTOR *crfsnp_item_vec);

/**
*
*  delete file data from current dn
*
**/
static EC_BOOL __crfs_delete_dn(const UINT32 crfs_md_id, const CRFSNP_FNODE *crfsnp_fnode);
static EC_BOOL __crfs_delete_b_dn(const UINT32 crfs_md_id, CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode);
EC_BOOL crfs_delete_dn(const UINT32 crfs_md_id, const UINT32 crfsnp_id, const CRFSNP_ITEM *crfsnp_item);

/**
*
*  delete a file or dir from all npp and all dn
*
**/
EC_BOOL crfs_delete(const UINT32 crfs_md_id, const CSTRING *path, const UINT32 dflag);

/**
*
*  query a file
*
**/
EC_BOOL crfs_qfile(const UINT32 crfs_md_id, const CSTRING *file_path, CRFSNP_ITEM  *crfsnp_item);

/**
*
*  query a dir
*
**/
EC_BOOL crfs_qdir(const UINT32 crfs_md_id, const CSTRING *dir_path, CVECTOR  *crfsnp_item_vec);

/**
*
*  query and list full path of a file or dir
*
**/
EC_BOOL crfs_qlist_path(const UINT32 crfs_md_id, const CSTRING *file_path, CVECTOR  *path_cstr_vec);

/**
*
*  query and list short name of a file or dir
*
**/
EC_BOOL crfs_qlist_seg(const UINT32 crfs_md_id, const CSTRING *file_path, CVECTOR  *seg_cstr_vec);

/**
*
*  flush name node pool
*
**/
EC_BOOL crfs_flush_npp(const UINT32 crfs_md_id);

/**
*
*  flush data node
*
*
**/
EC_BOOL crfs_flush_dn(const UINT32 crfs_md_id);

/**
*
*  count file num under specific path
*  if path is regular file, return file_num 1
*  if path is directory, return file num under it
*
**/
EC_BOOL crfs_file_num(const UINT32 crfs_md_id, const CSTRING *path_cstr, UINT32 *file_num);

/**
*
*  get file size of specific file given full path name
*
**/
EC_BOOL crfs_file_size(const UINT32 crfs_md_id, const CSTRING *path_cstr, uint64_t *file_size);

/**
*
*  mkdir in current name node pool
*
**/
EC_BOOL crfs_mkdir(const UINT32 crfs_md_id, const CSTRING *path_cstr);

/**
*
*  search in current name node pool
*
**/
EC_BOOL crfs_search(const UINT32 crfs_md_id, const CSTRING *path_cstr, const UINT32 dflag);

/**
*
*  check file content on data node
*
**/
EC_BOOL crfs_check_file_content(const UINT32 crfs_md_id, const UINT32 disk_no, const UINT32 block_no, const UINT32 page_no, const UINT32 file_size, const CSTRING *file_content_cstr);

/**
*
*  check file content on data node
*
**/
EC_BOOL crfs_check_file_is(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *file_content);

/**
*
*  show name node pool info if it is npp
*
*
**/
EC_BOOL crfs_show_npp(const UINT32 crfs_md_id, LOG *log);

/**
*
*  show crfsdn info if it is dn
*
*
**/
EC_BOOL crfs_show_dn(const UINT32 crfs_md_id, LOG *log);

/*debug*/
EC_BOOL crfs_show_cached_np(const UINT32 crfs_md_id, LOG *log);

EC_BOOL crfs_show_specific_np(const UINT32 crfs_md_id, const UINT32 crfsnp_id, LOG *log);

EC_BOOL crfs_show_path_depth(const UINT32 crfs_md_id, const CSTRING *path, LOG *log);

EC_BOOL crfs_show_path(const UINT32 crfs_md_id, const CSTRING *path, LOG *log);

EC_BOOL crfs_rdlock(const UINT32 crfs_md_id, const UINT32 location);
EC_BOOL crfs_wrlock(const UINT32 crfs_md_id, const UINT32 location);
EC_BOOL crfs_unlock(const UINT32 crfs_md_id, const UINT32 location);

#endif /*_CRFS_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

