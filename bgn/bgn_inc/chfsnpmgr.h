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

#ifndef _CHFSNPMGR_H
#define _CHFSNPMGR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "type.h"
#include "cvector.h"
#include "cmutex.h"
#include "cstring.h"

#include "chashalgo.h"
#include "chfsnp.h"
#include "chfsnprb.h"

#define CHFSNP_DB_NAME      ((const char *)"hfsnp_cfg.db")

typedef struct
{
    CSTRING          chfsnp_db_root_dir;           /*chfsnp database root dir*/
    CRWLOCK          crwlock;
    CMUTEX           cmutex;

    uint8_t          chfsnp_model;                  /*chfsnp model, e.g, CHFSNP_001G_MODEL*/
    uint8_t          chfsnp_1st_chash_algo_id;
    uint8_t          chfsnp_2nd_chash_algo_id;
    uint8_t          rsvd1;
    uint32_t         chfsnp_item_max_num;
    uint32_t         chfsnp_max_num;                /*max np num*/
    uint32_t         rsvd2;
    CVECTOR          chfsnp_home_dir_vec;           /*home directories in chfsnp(s)*/    
    CVECTOR          chfsnp_vec;                    /*item is CHFSNP*/
}CHFSNP_MGR;

#define CHFSNP_MGR_DB_ROOT_DIR(chfsnp_mgr)              (&((chfsnp_mgr)->chfsnp_db_root_dir))
#define CHFSNP_MGR_DB_ROOT_DIR_STR(chfsnp_mgr)          (cstring_get_str(CHFSNP_MGR_DB_ROOT_DIR(chfsnp_mgr)))

#define CHFSNP_MGR_NP_MODEL(chfsnp_mgr)                 ((chfsnp_mgr)->chfsnp_model)
#define CHFSNP_MGR_NP_1ST_CHASH_ALGO_ID(chfsnp_mgr)     ((chfsnp_mgr)->chfsnp_1st_chash_algo_id)
#define CHFSNP_MGR_NP_2ND_CHASH_ALGO_ID(chfsnp_mgr)     ((chfsnp_mgr)->chfsnp_2nd_chash_algo_id)
#define CHFSNP_MGR_NP_ITEM_MAX_NUM(chfsnp_mgr)          ((chfsnp_mgr)->chfsnp_item_max_num)
#define CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr)               ((chfsnp_mgr)->chfsnp_max_num)
#define CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr)          (&((chfsnp_mgr)->chfsnp_home_dir_vec))
#define CHFSNP_MGR_NP_HOME_DIR(chfsnp_mgr, chfsnp_id)   ((CSTRING *)cvector_get_no_lock(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), chfsnp_id))
#define CHFSNP_MGR_NP_VEC(chfsnp_mgr)                   (&((chfsnp_mgr)->chfsnp_vec))
#define CHFSNP_MGR_NP(chfsnp_mgr, chfsnp_id)            ((CHFSNP *)cvector_get(CHFSNP_MGR_NP_VEC(chfsnp_mgr), chfsnp_id))

#define CHFSNP_MGR_NP_HOME_DIR_VEC_LOCK(chfsnp_mgr, location)   CVECTOR_LOCK(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), location)
#define CHFSNP_MGR_NP_HOME_DIR_VEC_UNLOCK(chfsnp_mgr, location) CVECTOR_UNLOCK(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), location)

/*to reduce lock operation in name node*/
#define CHFSNP_MGR_NP_GET_NO_LOCK(chfsnp_mgr, chfsnp_id) \
        ((CHFSNP *)cvector_get_no_lock(CHFSNP_MGR_NP_VEC(chfsnp_mgr), (chfsnp_id)))
        
#define CHFSNP_MGR_NP_SET_NO_LOCK(chfsnp_mgr, chfsnp_id, __chfsnp) \
        (cvector_set_no_lock(CHFSNP_MGR_NP_VEC(chfsnp_mgr), (chfsnp_id), (__chfsnp)))

#define CHFSNP_MGR_CRWLOCK(chfsnp_mgr)                          (&((chfsnp_mgr)->crwlock))
#define CHFSNP_MGR_CRWLOCK_INIT(chfsnp_mgr, location)           (crwlock_init(CHFSNP_MGR_CRWLOCK(chfsnp_mgr), CMUTEX_PROCESS_PRIVATE, location))
#define CHFSNP_MGR_CRWLOCK_CLEAN(chfsnp_mgr, location)          (crwlock_clean(CHFSNP_MGR_CRWLOCK(chfsnp_mgr), location))
#define CHFSNP_MGR_CRWLOCK_RDLOCK(chfsnp_mgr, location)         (crwlock_rdlock(CHFSNP_MGR_CRWLOCK(chfsnp_mgr), location))
#define CHFSNP_MGR_CRWLOCK_WRLOCK(chfsnp_mgr, location)         (crwlock_wrlock(CHFSNP_MGR_CRWLOCK(chfsnp_mgr), location))
#define CHFSNP_MGR_CRWLOCK_UNLOCK(chfsnp_mgr, location)         (crwlock_unlock(CHFSNP_MGR_CRWLOCK(chfsnp_mgr), location))

#define CHFSNP_MGR_CMUTEX(chfsnp_mgr)                          (&((chfsnp_mgr)->cmutex))
#define CHFSNP_MGR_CMUTEX_INIT(chfsnp_mgr, location)           (cmutex_init(CHFSNP_MGR_CMUTEX(chfsnp_mgr), CMUTEX_PROCESS_PRIVATE, location))
#define CHFSNP_MGR_CMUTEX_CLEAN(chfsnp_mgr, location)          (cmutex_clean(CHFSNP_MGR_CMUTEX(chfsnp_mgr), location))
#define CHFSNP_MGR_CMUTEX_LOCK(chfsnp_mgr, location)           (cmutex_lock(CHFSNP_MGR_CMUTEX(chfsnp_mgr), location))
#define CHFSNP_MGR_CMUTEX_UNLOCK(chfsnp_mgr, location)         (cmutex_unlock(CHFSNP_MGR_CMUTEX(chfsnp_mgr), location))


CHFSNP_MGR *chfsnp_mgr_new();

EC_BOOL chfsnp_mgr_init(CHFSNP_MGR *chfsnp_mgr);

EC_BOOL chfsnp_mgr_clean(CHFSNP_MGR *chfsnp_mgr);

EC_BOOL chfsnp_mgr_free(CHFSNP_MGR *chfsnp_mgr);

CHFSNP *chfsnp_mgr_open_np(CHFSNP_MGR *chfsnp_mgr, const uint32_t chfsnp_id);

EC_BOOL chfsnp_mgr_close_np(CHFSNP_MGR *chfsnp_mgr, const uint32_t chfsnp_id);

EC_BOOL chfsnp_mgr_load_db(CHFSNP_MGR *chfsnp_mgr);

EC_BOOL chfsnp_mgr_create_db(CHFSNP_MGR *chfsnp_mgr, const CSTRING *chfsnp_db_root_dir);

EC_BOOL chfsnp_mgr_flush_db(CHFSNP_MGR *chfsnp_mgr);

void chfsnp_mgr_print_db(LOG *log, const CHFSNP_MGR *chfsnp_mgr);

void chfsnp_mgr_print(LOG *log, const CHFSNP_MGR *chfsnp_mgr);

EC_BOOL chfsnp_mgr_load(CHFSNP_MGR *chfsnp_mgr, const CSTRING *chfsnp_db_root_dir);

EC_BOOL chfsnp_mgr_flush(CHFSNP_MGR *chfsnp_mgr);

EC_BOOL chfsnp_mgr_show_np(LOG *log, CHFSNP_MGR *chfsnp_mgr, const uint32_t chfsnp_id);

EC_BOOL chfsnp_mgr_search(CHFSNP_MGR *chfsnp_mgr, const uint32_t path_len, const uint8_t *path, uint32_t *searched_chfsnp_id);

CHFSNP_ITEM *chfsnp_mgr_search_item(CHFSNP_MGR *chfsnp_mgr, const uint32_t path_len, const uint8_t *path);

CHFSNP_MGR *chfsnp_mgr_create(const uint8_t chfsnp_model, 
                                const uint32_t chfsnp_disk_max_num, 
                                const uint8_t  chfsnp_1st_chash_algo_id, 
                                const uint8_t  chfsnp_2nd_chash_algo_id, 
                                const uint32_t chfsnp_bucket_max_num,
                                const CSTRING *chfsnp_db_root_dir);

EC_BOOL chfsnp_mgr_exist(const CSTRING *chfsnp_db_root_dir);

CHFSNP_MGR * chfsnp_mgr_open(const CSTRING *chfsnp_db_root_dir);

EC_BOOL chfsnp_mgr_close(CHFSNP_MGR *chfsnp_mgr);

EC_BOOL chfsnp_mgr_find(CHFSNP_MGR *chfsnp_mgr, const CSTRING *path);

EC_BOOL chfsnp_mgr_bind(CHFSNP_MGR *chfsnp_mgr, const CSTRING *path, const UINT32 chfsnp_id);

EC_BOOL chfsnp_mgr_write(CHFSNP_MGR *chfsnp_mgr, const CSTRING *file_path, const CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfsnp_mgr_read(CHFSNP_MGR *chfsnp_mgr, const CSTRING *file_path, CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfsnp_mgr_delete(CHFSNP_MGR *chfsnp_mgr, const CSTRING *path, CVECTOR *chfsnp_fnode_vec);

EC_BOOL chfsnp_mgr_file_num(CHFSNP_MGR *chfsnp_mgr, UINT32 *file_num);

EC_BOOL chfsnp_mgr_file_size(CHFSNP_MGR *chfsnp_mgr, UINT32 *file_size);

EC_BOOL chfsnp_mgr_show_cached_np(LOG *log, const CHFSNP_MGR *chfsnp_mgr);

EC_BOOL chfsnp_mgr_rdlock(CHFSNP_MGR *chfsnp_mgr, const UINT32 location);

EC_BOOL chfsnp_mgr_wrlock(CHFSNP_MGR *chfsnp_mgr, const UINT32 location);

EC_BOOL chfsnp_mgr_unlock(CHFSNP_MGR *chfsnp_mgr, const UINT32 location);

#endif/* _CHFSNPMGR_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

