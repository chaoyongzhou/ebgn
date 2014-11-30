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

#ifndef _CHFS_H
#define _CHFS_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "type.h"
#include "cstring.h"
#include "clist.h"
#include "cvector.h"

#include "csocket.h"

#include "mod.inc"

#include "chfsnp.h"
#include "crfsdn.h"
#include "chfsnpmgr.h"

#define CHFS_MOD_NODE_WRITE_DISABLE         ((UINT32) ((MD_CHFS << (WORDSIZE/2)) + 0x1000))

#define CHFS_OP_WRITE                       ((UINT8)  1)
#define CHFS_OP_READ                        ((UINT8)  2)
#define CHFS_OP_GET_WORDSIZE                ((UINT8)  3)
#define CHFS_OP_QLIST_PATH                  ((UINT8)  4)
#define CHFS_OP_MKDIR                       ((UINT8)  5)
#define CHFS_OP_EXISTS                      ((UINT8)  6)
#define CHFS_OP_IS_FILE                     ((UINT8)  7)
#define CHFS_OP_IS_DIR                      ((UINT8)  8)
#define CHFS_OP_IS_QFILE                    ((UINT8)  9)
#define CHFS_OP_IS_QDIR                     ((UINT8) 10)

typedef struct
{
    /* used counter >= 0 */
    UINT32      usedcounter;
    EC_BOOL     terminate_flag;

    MOD_MGR    *chfsdn_mod_mgr;
    MOD_MGR    *chfsnpp_mod_mgr;

    CRFSDN     *crfsdn;
    CHFSNP_MGR *chfsnpmgr;/*namespace pool*/   

    CROUTINE_RWLOCK     crwlock;
}CHFS_MD;

#define CHFS_MD_TERMINATE_FLAG(chfs_md)  ((chfs_md)->terminate_flag)
#define CHFS_MD_DN_MOD_MGR(chfs_md)      ((chfs_md)->chfsdn_mod_mgr)
#define CHFS_MD_NPP_MOD_MGR(chfs_md)     ((chfs_md)->chfsnpp_mod_mgr)
#define CHFS_MD_DN(chfs_md)              ((chfs_md)->crfsdn)
#define CHFS_MD_NPP(chfs_md)             ((chfs_md)->chfsnpmgr)
#define CHFS_CRWLOCK(chfs_md)            (&((chfs_md)->crwlock))

#if 0
#define CHFS_INIT_LOCK(chfs_md, location)  (croutine_rwlock_init(CHFS_CRWLOCK(chfs_md), CMUTEX_PROCESS_PRIVATE, location))
#define CHFS_CLEAN_LOCK(chfs_md, location) (croutine_rwlock_clean(CHFS_CRWLOCK(chfs_md), location))

#define CHFS_RDLOCK(chfs_md, location)     (croutine_rwlock_rdlock(CHFS_CRWLOCK(chfs_md), location))
#define CHFS_WRLOCK(chfs_md, location)     (croutine_rwlock_wrlock(CHFS_CRWLOCK(chfs_md), location))
#define CHFS_UNLOCK(chfs_md, location)     (croutine_rwlock_unlock(CHFS_CRWLOCK(chfs_md), location))
#endif

#if 1
#define CHFS_INIT_LOCK(chfs_md, location)  do{\
    sys_log(LOGSTDOUT, "[DEBUG] CHFS_INIT_LOCK: CHFS_CRWLOCK %p, at %s:%ld\n", CHFS_CRWLOCK(chfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_init(CHFS_CRWLOCK(chfs_md), CMUTEX_PROCESS_PRIVATE, location);\
}while(0)

#define CHFS_CLEAN_LOCK(chfs_md, location) do{\
    sys_log(LOGSTDOUT, "[DEBUG] CHFS_CLEAN_LOCK: CHFS_CRWLOCK %p, at %s:%ld\n", CHFS_CRWLOCK(chfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_clean(CHFS_CRWLOCK(chfs_md), location);\
}while(0)    

#define CHFS_RDLOCK(chfs_md, location)     do{\
    sys_log(LOGSTDOUT, "[DEBUG] CHFS_RDLOCK: CHFS_CRWLOCK %p, at %s:%ld\n", CHFS_CRWLOCK(chfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_rdlock(CHFS_CRWLOCK(chfs_md), location);\
    sys_log(LOGSTDOUT, "[DEBUG] CHFS_RDLOCK: CHFS_CRWLOCK %p, at %s:%ld done\n", CHFS_CRWLOCK(chfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
}while(0)

#define CHFS_WRLOCK(chfs_md, location)     do{\
    sys_log(LOGSTDOUT, "[DEBUG] CHFS_WRLOCK: CHFS_CRWLOCK %p, at %s:%ld\n", CHFS_CRWLOCK(chfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_wrlock(CHFS_CRWLOCK(chfs_md), location);\
    sys_log(LOGSTDOUT, "[DEBUG] CHFS_WRLOCK: CHFS_CRWLOCK %p, at %s:%ld done\n", CHFS_CRWLOCK(chfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
}while(0)
#define CHFS_UNLOCK(chfs_md, location)     do{\
    sys_log(LOGSTDOUT, "[DEBUG] CHFS_UNLOCK: CHFS_CRWLOCK %p, at %s:%ld\n", CHFS_CRWLOCK(chfs_md), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_unlock(CHFS_CRWLOCK(chfs_md), location);\
}while(0)
#endif


/**
*   for test only
*
*   to query the status of CHFS Module
*
**/
void chfs_print_module_status(const UINT32 chfs_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CHFS module
*
*
**/
UINT32 chfs_free_module_static_mem(const UINT32 chfs_md_id);

/**
*
* start CHFS module
*
**/
UINT32 chfs_start(const CSTRING *chfsnp_root_dir, const CSTRING *crfsdn_root_dir);

/**
*
* end CHFS module
*
**/
void chfs_end(const UINT32 chfs_md_id);

/**
*
* initialize mod mgr of CHFS module
*
**/
UINT32 chfs_set_npp_mod_mgr(const UINT32 chfs_md_id, const MOD_MGR * src_mod_mgr);

UINT32 chfs_set_dn_mod_mgr(const UINT32 chfs_md_id, const MOD_MGR * src_mod_mgr);

/**
*
* get mod mgr of CHFS module
*
**/
MOD_MGR * chfs_get_npp_mod_mgr(const UINT32 chfs_md_id);

MOD_MGR * chfs_get_dn_mod_mgr(const UINT32 chfs_md_id);

CHFSNP_FNODE *chfs_fnode_new(const UINT32 chfs_md_id);

EC_BOOL chfs_fnode_init(const UINT32 chfs_md_id, CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfs_fnode_clean(const UINT32 chfs_md_id, CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfs_fnode_free(const UINT32 chfs_md_id, CHFSNP_FNODE *chfsnp_fnode);


/**
*
*  get name node pool of the module
*
**/
CHFSNP_MGR *chfs_get_npp(const UINT32 chfs_md_id);

/**
*
*  get data node of the module
*
**/
CRFSDN *chfs_get_dn(const UINT32 chfs_md_id);

/**
*
*  open name node pool
*
**/
EC_BOOL chfs_open_npp(const UINT32 chfs_md_id, const CSTRING *chfsnp_db_root_dir);

/**
*
*  flush and close name node pool
*
**/
EC_BOOL chfs_close_npp(const UINT32 chfs_md_id);

/**
*
*  check this CHFS is name node pool or not
*
*
**/
EC_BOOL chfs_is_npp(const UINT32 chfs_md_id);

/**
*
*  check this CHFS is data node or not
*
*
**/
EC_BOOL chfs_is_dn(const UINT32 chfs_md_id);

/**
*
*  check this CHFS is data node and namenode or not
*
*
**/
EC_BOOL chfs_is_npp_and_dn(const UINT32 chfs_md_id);

/**
*
*  create name node pool
*
**/
EC_BOOL chfs_create_npp(const UINT32 chfs_md_id, 
                             const UINT32 chfsnp_model, 
                             const UINT32 chfsnp_max_num, 
                             const UINT32 chfsnp_1st_chash_algo_id, 
                             const UINT32 chfsnp_2nd_chash_algo_id, 
                             const UINT32 chfsnp_bucket_max_num, 
                             const CSTRING *chfsnp_db_root_dir);

EC_BOOL chfs_add_npp(const UINT32 chfs_md_id, const UINT32 chfsnpp_tcid, const UINT32 chfsnpp_rank);

EC_BOOL chfs_add_dn(const UINT32 chfs_md_id, const UINT32 chfsdn_tcid, const UINT32 chfsdn_rank);

/**
*
*  check existing of a file
*
**/
EC_BOOL chfs_find_file(const UINT32 chfs_md_id, const CSTRING *file_path);

/**
*
*  check existing of a file
*
**/
EC_BOOL chfs_find(const UINT32 chfs_md_id, const CSTRING *path);

/**
*
*  check existing of a file
*
**/
EC_BOOL chfs_exists(const UINT32 chfs_md_id, const CSTRING *path);

/**
*
*  check existing of a file
*
**/
EC_BOOL chfs_is_file(const UINT32 chfs_md_id, const CSTRING *file_path);

/**
*
*  set home dir of a name node
*
**/
EC_BOOL chfs_set(const UINT32 chfs_md_id, const CSTRING *home_dir, const UINT32 chfsnp_id);

/**
*
*  reserve space from dn
*
**/
EC_BOOL chfs_reserve_dn(const UINT32 chfs_md_id, const UINT32 data_len, CHFSNP_FNODE *chfsnp_fnode);

/**
*
*  release space to dn
*
**/
EC_BOOL chfs_release_dn(const UINT32 chfs_md_id, const CHFSNP_FNODE *chfsnp_fnode);

/**
*
*  write a file
*
**/
EC_BOOL chfs_write(const UINT32 chfs_md_id, const CSTRING *file_path, const CBYTES *cbytes);

/**
*
*  read a file
*
**/
EC_BOOL chfs_read(const UINT32 chfs_md_id, const CSTRING *file_path, CBYTES *cbytes);

/**
*
*  create data node
*
**/
EC_BOOL chfs_create_dn(const UINT32 chfs_md_id, const CSTRING *root_dir);

/**
*
*  add a disk to data node
*
**/
EC_BOOL chfs_add_disk(const UINT32 chfs_md_id, const UINT32 disk_no);

/**
*
*  delete a disk from data node
*
**/
EC_BOOL chfs_del_disk(const UINT32 chfs_md_id, const UINT32 disk_no);

/**
*
*  mount a disk to data node
*
**/
EC_BOOL chfs_mount_disk(const UINT32 chfs_md_id, const UINT32 disk_no);

/**
*
*  umount a disk from data node
*
**/
EC_BOOL chfs_umount_disk(const UINT32 chfs_md_id, const UINT32 disk_no);

/**
*
*  open data node
*
**/
EC_BOOL chfs_open_dn(const UINT32 chfs_md_id, const CSTRING *root_dir);

/**
*
*  close data node
*
**/
EC_BOOL chfs_close_dn(const UINT32 chfs_md_id);

/**
*
*  export data into data node
*
**/
EC_BOOL chfs_export_dn(const UINT32 chfs_md_id, const CBYTES *cbytes, const CHFSNP_FNODE *chfsnp_fnode);

/**
*
*  write data node
*
**/
EC_BOOL chfs_write_dn(const UINT32 chfs_md_id, const CBYTES *cbytes, CHFSNP_FNODE *chfsnp_fnode);

/**
*
*  write data node in cache
*
**/
EC_BOOL chfs_write_dn_cache(const UINT32 chfs_md_id, const CBYTES *cbytes, CHFSNP_FNODE *chfsnp_fnode);

/**
*
*  read data node
*
**/
EC_BOOL chfs_read_dn(const UINT32 chfs_md_id, const CHFSNP_FNODE *chfsnp_fnode, CBYTES *cbytes);

/**
*
*  write a fnode to name node
*
**/
EC_BOOL chfs_write_npp(const UINT32 chfs_md_id, const CSTRING *file_path, const CHFSNP_FNODE *chfsnp_fnode);

/**
*
*  read a fnode from name node
*
**/
EC_BOOL chfs_read_npp(const UINT32 chfs_md_id, const CSTRING *file_path, CHFSNP_FNODE *chfsnp_fnode);

/**
*
*  delete a file from current npp
*
**/
EC_BOOL chfs_delete_npp(const UINT32 chfs_md_id, const CSTRING *path, CVECTOR *chfsnp_fnode_vec);

/**
*
*  delete file data from current dn
*
**/
static EC_BOOL __chfs_delete_dn(const UINT32 chfs_md_id, const CHFSNP_FNODE *chfsnp_fnode);;
EC_BOOL chfs_delete_dn(const UINT32 chfs_md_id, const CHFSNP_FNODE *chfsnp_fnode);

/**
*
*  delete a file from all npp and all dn
*
**/
EC_BOOL chfs_delete(const UINT32 chfs_md_id, const CSTRING *path);

/**
*
*  query a file
*
**/
EC_BOOL chfs_qfile(const UINT32 chfs_md_id, const CSTRING *file_path, CHFSNP_ITEM  *chfsnp_item);

/**
*
*  flush name node pool
*
**/
EC_BOOL chfs_flush_npp(const UINT32 chfs_md_id);

/**
*
*  flush data node
*
*
**/
EC_BOOL chfs_flush_dn(const UINT32 chfs_md_id);

/**
*
*  count file num under specific path
*  if path is regular file, return file_num 1
*  if path is directory, return file num under it
*
**/
EC_BOOL chfs_file_num(const UINT32 chfs_md_id, UINT32 *file_num);

/**
*
*  get file size of specific file given full path name
*
**/
EC_BOOL chfs_file_size(const UINT32 chfs_md_id, UINT32 *file_size);

/**
*
*  search in current name node pool
*
**/
EC_BOOL chfs_search(const UINT32 chfs_md_id, const CSTRING *path_cstr);

/**
*
*  check file content on data node
*
**/
EC_BOOL chfs_check_file_content(const UINT32 chfs_md_id, const UINT32 disk_no, const UINT32 block_no, const UINT32 page_no, const UINT32 file_size, const CSTRING *file_content_cstr);

/**
*
*  check file content on data node
*
**/
EC_BOOL chfs_check_file_is(const UINT32 chfs_md_id, const CSTRING *file_path, const CBYTES *file_content);

/**
*
*  show name node pool info if it is npp
*
*
**/
EC_BOOL chfs_show_npp(const UINT32 chfs_md_id, LOG *log);

/**
*
*  show crfsdn info if it is dn
*
*
**/
EC_BOOL chfs_show_dn(const UINT32 chfs_md_id, LOG *log);

/*debug*/
EC_BOOL chfs_show_cached_np(const UINT32 chfs_md_id, LOG *log);

EC_BOOL chfs_show_specific_np(const UINT32 chfs_md_id, const UINT32 chfsnp_id, LOG *log);

#endif /*_CHFS_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

