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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cmpic.inc"
#include "cmutex.h"
#include "clist.h"
#include "cstring.h"
#include "cmisc.h"

#include "task.inc"
#include "task.h"

#include "crfsnp.h"
#include "crfsnprb.h"
#include "crfsnpmgr.h"
#include "chashalgo.h"
#include "cfuse.h"

#include "findex.inc"


static EC_BOOL __crfsnp_home_dirs_init(CRFSNP_HOME_DIRS *crfsnp_home_dirs)
{
    CLIST *home_dir_list;
    home_dir_list = CRFSNP_HOME_DIR_LIST(crfsnp_home_dirs);
    clist_init(home_dir_list, MM_CSTRING, LOC_CRFSNPMGR_0001);
    return (EC_TRUE);
}

static CRFSNP_HOME_DIRS *__crfsnp_home_dirs_new()
{
    CRFSNP_HOME_DIRS *crfsnp_home_dirs;

    alloc_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSNP_HOME_DIRS, &crfsnp_home_dirs, LOC_CRFSNPMGR_0002);
    if(NULL_PTR != crfsnp_home_dirs)
    {
        __crfsnp_home_dirs_init(crfsnp_home_dirs);
        return (crfsnp_home_dirs);
    }
    return (NULL_PTR);
}

static EC_BOOL __crfsnp_home_dirs_clean(CRFSNP_HOME_DIRS *crfsnp_home_dirs)
{
    CLIST *home_dir_list;
    home_dir_list = CRFSNP_HOME_DIR_LIST(crfsnp_home_dirs);
    clist_clean(home_dir_list, (CLIST_DATA_DATA_CLEANER)cstring_free);
    return (EC_TRUE);
}

static EC_BOOL __crfsnp_home_dirs_free(CRFSNP_HOME_DIRS *crfsnp_home_dirs)
{
    if(NULL_PTR != crfsnp_home_dirs)
    {
        __crfsnp_home_dirs_clean(crfsnp_home_dirs);
        free_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSNP_HOME_DIRS, crfsnp_home_dirs, LOC_CRFSNPMGR_0003);
    }
    return (EC_TRUE);
}

static EC_BOOL __crfsnp_home_dirs_print(LOG *log, const CRFSNP_HOME_DIRS *crfsnp_home_dirs)
{
    const CLIST *home_dir_list;
    CLIST_DATA  *clist_data;    
    UINT32       pos;

    home_dir_list = CRFSNP_HOME_DIR_LIST(crfsnp_home_dirs);

    CLIST_LOCK(home_dir_list, LOC_CRFSNPMGR_0004);
    if(EC_TRUE == CLIST_IS_EMPTY(home_dir_list))
    {
        sys_log(log, "(null)\n");

        CLIST_UNLOCK(home_dir_list, LOC_CRFSNPMGR_0005);
        return (EC_TRUE);
    }
    
    pos = 0;
    CLIST_LOOP_NEXT(home_dir_list, clist_data)
    {
        CSTRING     *crfsnp_home_dir;
        crfsnp_home_dir = (CSTRING *)CLIST_DATA_DATA(clist_data);
        if(NULL_PTR == crfsnp_home_dir)
        {
            sys_log(log, "No. %u: home dir: (null)\n", pos);
        }
        else
        {
            sys_log(log, "No. %u: home dir: %s\n", pos, (char *)cstring_get_str(crfsnp_home_dir));
        }  
        
        pos ++;
    }
    CLIST_UNLOCK(home_dir_list, LOC_CRFSNPMGR_0006);
    return (EC_TRUE);
}

static CSTRING *__crfsnp_home_dirs_match_str(const CRFSNP_HOME_DIRS *crfsnp_home_dirs, const uint32_t path_len, const uint8_t *path)
{
    const CLIST *home_dir_list;
    CLIST_DATA  *clist_data;

    home_dir_list = CRFSNP_HOME_DIR_LIST(crfsnp_home_dirs);

    CLIST_LOCK(home_dir_list, LOC_CRFSNPMGR_0007);
    CLIST_LOOP_NEXT(home_dir_list, clist_data)
    {
        CSTRING *crfsnp_home_dir_cstr;
        uint32_t crfsnp_home_dir_len;
        uint8_t *crfsnp_home_dir_str;
        
        crfsnp_home_dir_cstr = (CSTRING *)CLIST_DATA_DATA(clist_data);

        if(NULL_PTR == crfsnp_home_dir_cstr)
        {
            continue;
        }

        if(EC_TRUE == cstring_is_empty(crfsnp_home_dir_cstr))
        {
            continue;
        }

        crfsnp_home_dir_len = (uint32_t)cstring_get_len(crfsnp_home_dir_cstr);
        crfsnp_home_dir_str = cstring_get_str(crfsnp_home_dir_cstr);

        sys_log(LOGSTDOUT, "[DEBUG] __crfsnp_home_dirs_match_str: %.*s vs %s\n", path_len, (char *)path, (char *)cstring_get_str(crfsnp_home_dir_cstr));
        sys_log(LOGSTDOUT, "[DEBUG] __crfsnp_home_dirs_match_str: path_len %u, crfsnp_home_dir_len %u\n", path_len, crfsnp_home_dir_len);
       
        if(path_len < crfsnp_home_dir_len)
        {
            continue;
        }

        /*now path_len >= crfsnp_home_dir_len*/
        if(path_len != crfsnp_home_dir_len && '/' != path[ crfsnp_home_dir_len ])
        {
            continue;
        }

        if(0 == BCMP(path, crfsnp_home_dir_str, crfsnp_home_dir_len))
        {
            CLIST_UNLOCK(home_dir_list, LOC_CRFSNPMGR_0008);    
            return (crfsnp_home_dir_cstr);
        }        
    }
    CLIST_UNLOCK(home_dir_list, LOC_CRFSNPMGR_0009);    

    return (NULL_PTR);
}

static EC_BOOL __crfsnp_home_dirs_bind(CRFSNP_HOME_DIRS *crfsnp_home_dirs, CSTRING *crfsnp_home_dir_cstr)
{
    CLIST *home_dir_list;

    home_dir_list = CRFSNP_HOME_DIR_LIST(crfsnp_home_dirs);
    clist_push_back(home_dir_list, (void *)crfsnp_home_dir_cstr);
    return (EC_TRUE);
}

static CSTRING * __crfsnp_home_dirs_unbind(CRFSNP_HOME_DIRS *crfsnp_home_dirs, const CSTRING *unbind_home_dir_cstr)
{
    CLIST *home_dir_list;
    CLIST_DATA  *clist_data;

    home_dir_list = CRFSNP_HOME_DIR_LIST(crfsnp_home_dirs);

    CLIST_LOCK(home_dir_list, LOC_CRFSNPMGR_0010);
    CLIST_LOOP_NEXT(home_dir_list, clist_data)
    {
        CSTRING *crfsnp_home_dir_cstr;
        
        crfsnp_home_dir_cstr = (CSTRING *)CLIST_DATA_DATA(clist_data);
        if(NULL_PTR == crfsnp_home_dir_cstr)
        {
            continue;
        }

        if(EC_TRUE == cstring_is_empty(crfsnp_home_dir_cstr))
        {
            continue;
        }

        if(EC_TRUE == cstring_cmp(crfsnp_home_dir_cstr, unbind_home_dir_cstr))
        {
            clist_rmv_no_lock(home_dir_list, clist_data);
            CLIST_UNLOCK(home_dir_list, LOC_CRFSNPMGR_0011);    
            return (crfsnp_home_dir_cstr);
        }
    }
    CLIST_UNLOCK(home_dir_list, LOC_CRFSNPMGR_0012);
    return (NULL_PTR);
}

static EC_BOOL __crfsnp_home_dirs_load(CRFSNP_HOME_DIRS *crfsnp_home_dirs, int crfsnp_mgr_fd, UINT32 *crfsnp_mgr_db_offset)
{
    UINT32 crfsnp_mgr_db_size;
    UINT8* crfsnp_mgr_db_buff;
    
    CLIST   *home_dir_list;
    UINT32   home_dir_vec_size;
    UINT32   home_dir_vec_pos;

    crfsnp_mgr_db_size   = sizeof(UINT32);
    crfsnp_mgr_db_buff   = (UINT8 *)&(home_dir_vec_size);    
    sys_log(LOGSTDOUT, "[DEBUG] __crfsnp_home_dirs_load: load home_dir_vec_size from offset %u\n", (*crfsnp_mgr_db_offset));
    if(EC_FALSE == c_file_load(crfsnp_mgr_fd, crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_home_dirs_load: load home_dir_vec_size failed\n");
        return (EC_FALSE);
    }    

    sys_log(LOGSTDOUT, "[DEBUG] __crfsnp_home_dirs_load: home_dir_vec_size = %u\n", home_dir_vec_size);

    home_dir_list = CRFSNP_HOME_DIR_LIST(crfsnp_home_dirs);
    sys_log(LOGSTDOUT, "[DEBUG] __crfsnp_home_dirs_load: home_dir_list size = %u\n", clist_size(home_dir_list));

    
    for(home_dir_vec_pos = 0; home_dir_vec_pos < home_dir_vec_size; home_dir_vec_pos ++)
    {
        CSTRING *crfsnp_home_dir_cstr;
        
        crfsnp_home_dir_cstr = cstring_load(crfsnp_mgr_fd, crfsnp_mgr_db_offset);
        if(NULL_PTR == crfsnp_home_dir_cstr)
        {
            sys_log(LOGSTDOUT, "error:__crfsnp_home_dirs_load: load home dir %u # failed\n", home_dir_vec_pos);
            return (EC_FALSE);
        }

        if(EC_TRUE == cstring_is_empty(crfsnp_home_dir_cstr))
        {
            cstring_free(crfsnp_home_dir_cstr);
            continue;
        }

        clist_push_back(home_dir_list, (void *)crfsnp_home_dir_cstr);
    }

    ASSERT(clist_size(home_dir_list) == home_dir_vec_size);
    return (EC_TRUE);
}

static EC_BOOL __crfsnp_home_dirs_flush(const CRFSNP_HOME_DIRS *crfsnp_home_dirs, int crfsnp_mgr_fd, UINT32 *crfsnp_mgr_db_offset)
{
    UINT32 crfsnp_mgr_db_size;
    UINT8* crfsnp_mgr_db_buff;
    
    const CLIST *home_dir_list;
    CLIST_DATA  *clist_data;
    UINT32   home_dir_vec_size;
    UINT32   home_dir_vec_pos;

    home_dir_list     = CRFSNP_HOME_DIR_LIST(crfsnp_home_dirs);
    home_dir_vec_size = clist_size(home_dir_list);

    crfsnp_mgr_db_size   = sizeof(UINT32);
    crfsnp_mgr_db_buff   = (UINT8 *)&(home_dir_vec_size);    
    sys_log(LOGSTDOUT, "[DEBUG] __crfsnp_home_dirs_flush: flush home_dir_vec_size %u at offset %u\n", home_dir_vec_size, (*crfsnp_mgr_db_offset));
    if(EC_FALSE == c_file_flush(crfsnp_mgr_fd, crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_home_dirs_flush: load home_dir_vec_size failed\n");
        return (EC_FALSE);
    }

    home_dir_vec_pos = 0;

    CLIST_LOCK(home_dir_list, LOC_CRFSNPMGR_0013);
    CLIST_LOOP_NEXT(home_dir_list, clist_data)
    {
        CSTRING *crfsnp_home_dir_cstr;

        crfsnp_home_dir_cstr = (CSTRING *)CLIST_DATA_DATA(clist_data);
        if(NULL_PTR == crfsnp_home_dir_cstr)
        {
            sys_log(LOGSTDOUT, "warn:__crfsnp_home_dirs_flush: find null crfsnp_home_dir_cstr\n");
            continue;
        }

        if(EC_FALSE == cstring_flush(crfsnp_mgr_fd, crfsnp_mgr_db_offset, crfsnp_home_dir_cstr))
        {
            CLIST_UNLOCK(home_dir_list, LOC_CRFSNPMGR_0014);   
            sys_log(LOGSTDOUT, "error:__crfsnp_home_dirs_flush: flush home dir %u # %s failed\n", 
                               home_dir_vec_pos, (char *)cstring_get_str(crfsnp_home_dir_cstr));
            return (EC_FALSE);
        }
    }
    CLIST_UNLOCK(home_dir_list, LOC_CRFSNPMGR_0015);    
    
    return (EC_TRUE);
}

CRFSNP_MGR *crfsnp_mgr_new()
{
    CRFSNP_MGR *crfsnp_mgr;

    alloc_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSNP_MGR, &crfsnp_mgr, LOC_CRFSNPMGR_0016);
    if(NULL_PTR != crfsnp_mgr)
    {
        crfsnp_mgr_init(crfsnp_mgr);
    }

    return (crfsnp_mgr);
}

EC_BOOL crfsnp_mgr_init(CRFSNP_MGR *crfsnp_mgr)
{
    CRFSNP_MGR_CRWLOCK_INIT(crfsnp_mgr, LOC_CRFSNPMGR_0017);
    CRFSNP_MGR_CMUTEX_INIT(crfsnp_mgr, LOC_CRFSNPMGR_0018);
    
    cstring_init(CRFSNP_MGR_DB_ROOT_DIR(crfsnp_mgr), NULL_PTR);    

    CRFSNP_MGR_NP_MODEL(crfsnp_mgr) = CRFSNP_ERR_MODEL;
    CRFSNP_MGR_NP_1ST_CHASH_ALGO_ID(crfsnp_mgr) = (uint8_t)CHASH_ERR_ALGO_ID;
    CRFSNP_MGR_NP_2ND_CHASH_ALGO_ID(crfsnp_mgr) = (uint8_t)CHASH_ERR_ALGO_ID;
    CRFSNP_MGR_NP_ITEM_MAX_NUM(crfsnp_mgr)      = 0;
    CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr)           = 0;

    cvector_init(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr), 0, MM_CRFSNP_HOME_DIRS, CVECTOR_LOCK_ENABLE, LOC_CRFSNPMGR_0019);
    cvector_init(CRFSNP_MGR_NP_VEC(crfsnp_mgr), 0, MM_CRFSNP, CVECTOR_LOCK_ENABLE, LOC_CRFSNPMGR_0020);   
    
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_clean(CRFSNP_MGR *crfsnp_mgr)
{
    CRFSNP_MGR_CRWLOCK_CLEAN(crfsnp_mgr, LOC_CRFSNPMGR_0021);
    CRFSNP_MGR_CMUTEX_CLEAN(crfsnp_mgr, LOC_CRFSNPMGR_0022);
    
    cstring_clean(CRFSNP_MGR_DB_ROOT_DIR(crfsnp_mgr));    

    CRFSNP_MGR_NP_MODEL(crfsnp_mgr) = CRFSNP_ERR_MODEL;
    CRFSNP_MGR_NP_1ST_CHASH_ALGO_ID(crfsnp_mgr) = (uint8_t)CHASH_ERR_ALGO_ID;
    CRFSNP_MGR_NP_2ND_CHASH_ALGO_ID(crfsnp_mgr) = (uint8_t)CHASH_ERR_ALGO_ID;
    CRFSNP_MGR_NP_ITEM_MAX_NUM(crfsnp_mgr)      = 0;
    CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr)           = 0;

    cvector_clean(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr), (CVECTOR_DATA_CLEANER)__crfsnp_home_dirs_free, LOC_CRFSNPMGR_0023);
    cvector_clean(CRFSNP_MGR_NP_VEC(crfsnp_mgr), (CVECTOR_DATA_CLEANER)crfsnp_free, LOC_CRFSNPMGR_0024);       

    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_free(CRFSNP_MGR *crfsnp_mgr)
{
    if(NULL_PTR != crfsnp_mgr)
    {
        crfsnp_mgr_clean(crfsnp_mgr);
        free_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSNP_MGR, crfsnp_mgr, LOC_CRFSNPMGR_0025);
    }
    return (EC_TRUE);
}

CRFSNP *crfsnp_mgr_open_np(CRFSNP_MGR *crfsnp_mgr, const uint32_t crfsnp_id)
{
    CRFSNP *crfsnp;

    crfsnp = (CRFSNP *)cvector_get_no_lock(CRFSNP_MGR_NP_VEC(crfsnp_mgr), crfsnp_id);
    if(NULL_PTR != crfsnp)
    {
        return (crfsnp);
    }

    crfsnp = crfsnp_open((char *)CRFSNP_MGR_DB_ROOT_DIR_STR(crfsnp_mgr), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_open_np: open np %u from %s failed\n", crfsnp_id, (char *)CRFSNP_MGR_DB_ROOT_DIR_STR(crfsnp_mgr));
        return (NULL_PTR);
    }

    if(NULL_PTR != cvector_set_no_lock(CRFSNP_MGR_NP_VEC(crfsnp_mgr), (crfsnp_id), (crfsnp)))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_open_np: set np %u to vector but found old existence\n", crfsnp_id);
        return (crfsnp);
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfsnp_mgr_open_np: set np %u to vector done\n", crfsnp_id);
    return (crfsnp);
}

EC_BOOL crfsnp_mgr_close_np(CRFSNP_MGR *crfsnp_mgr, const uint32_t crfsnp_id)
{
    CRFSNP *crfsnp;

    crfsnp = (CRFSNP *)cvector_get_no_lock(CRFSNP_MGR_NP_VEC(crfsnp_mgr), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "warn:crfsnp_mgr_close_np: np %u not open yet\n", crfsnp_id);
        return (EC_TRUE);
    }

    cvector_set_no_lock(CRFSNP_MGR_NP_VEC(crfsnp_mgr), crfsnp_id, NULL_PTR);
    crfsnp_close(crfsnp);
    return (EC_TRUE);
}

static char *__crfsnp_mgr_gen_db_name(const char *root_dir)
{
    const char *fields[ 2 ];
    
    fields[ 0 ] = root_dir;
    fields[ 1 ] = CRFSNP_DB_NAME;
    
    return c_str_join((char *)"/", fields, 2);
}

static EC_BOOL __crfsnp_mgr_load_db(CRFSNP_MGR *crfsnp_mgr, int crfsnp_mgr_fd)
{
    UINT32 crfsnp_mgr_db_size;
    UINT8* crfsnp_mgr_db_buff;
    UINT32 crfsnp_mgr_db_offset;
    UINT32 crfsnp_num;
    UINT32 crfsnp_id;
    
    /*init offset*/
    crfsnp_mgr_db_offset = 0;

    /*CRFSNP_MGR_NP_MODEL*/
    crfsnp_mgr_db_size   = sizeof(uint8_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_MODEL(crfsnp_mgr));    
    if(EC_FALSE == c_file_load(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_load_db: load np model failed\n");
        return (EC_FALSE);
    }

    /*CRFSNP_MGR_NP_1ST_CHASH_ALGO_ID*/
    crfsnp_mgr_db_size   = sizeof(uint8_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_1ST_CHASH_ALGO_ID(crfsnp_mgr));    
    if(EC_FALSE == c_file_load(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_load_db: load 1st chash algo id failed\n");
        return (EC_FALSE);
    }    

    /*CRFSNP_MGR_NP_2ND_CHASH_ALGO_ID*/
    crfsnp_mgr_db_size   = sizeof(uint8_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_2ND_CHASH_ALGO_ID(crfsnp_mgr));    
    if(EC_FALSE == c_file_load(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_load_db: load 2nd chash algo id failed\n");
        return (EC_FALSE);
    }     

    /*CRFSNP_MGR_NP_ITEM_MAX_NUM*/
    crfsnp_mgr_db_size   = sizeof(uint32_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_ITEM_MAX_NUM(crfsnp_mgr));    
    if(EC_FALSE == c_file_load(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_load_db: load item max num failed\n");
        return (EC_FALSE);
    }     

    /*CRFSNP_MGR_NP_MAX_NUM*/
    crfsnp_mgr_db_size   = sizeof(uint32_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr));    
    if(EC_FALSE == c_file_load(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_load_db: load disk max num failed\n");
        return (EC_FALSE);
    }

    for(crfsnp_id = cvector_size(CRFSNP_MGR_NP_VEC(crfsnp_mgr)); crfsnp_id < CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr); crfsnp_id ++)
    {
        cvector_push_no_lock(CRFSNP_MGR_NP_VEC(crfsnp_mgr), NULL_PTR);
    }

    /*CRFSNP_MGR_NP_HOME_DIRS_VEC*/
    crfsnp_mgr_db_size   = sizeof(UINT32);
    crfsnp_mgr_db_buff   = (UINT8 *)&(crfsnp_num);    
    if(EC_FALSE == c_file_load(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_load_db: load home dir vec size failed\n");
        return (EC_FALSE);
    }    

    ASSERT(crfsnp_num == CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr));
    ASSERT(0 == cvector_size(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr)));

    for(crfsnp_id = cvector_size(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr)); crfsnp_id < crfsnp_num; crfsnp_id ++)
    {
        CRFSNP_HOME_DIRS * crfsnp_home_dirs;

        crfsnp_home_dirs = __crfsnp_home_dirs_new();
        if(NULL_PTR == crfsnp_home_dirs)
        {
            sys_log(LOGSTDOUT, "error:__crfsnp_mgr_load_db: new crfsnp_home_dirs %u # failed\n", crfsnp_id);
            return (EC_FALSE);
        }    
        cvector_push_no_lock(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr), (void *)crfsnp_home_dirs);
    }    

    for(crfsnp_id = 0; crfsnp_id < crfsnp_num; crfsnp_id ++)
    {   
        CRFSNP_HOME_DIRS * crfsnp_home_dirs;

        crfsnp_home_dirs = CRFSNP_MGR_NP_HOME_DIRS(crfsnp_mgr, crfsnp_id);
        if(NULL_PTR == crfsnp_home_dirs)
        {
            sys_log(LOGSTDOUT, "error:__crfsnp_mgr_load_db: crfsnp_home_dirs %u # is null\n", crfsnp_id);
            return (EC_FALSE);
        }
        
        if(EC_FALSE == __crfsnp_home_dirs_load(crfsnp_home_dirs, crfsnp_mgr_fd, &crfsnp_mgr_db_offset))
        {
            sys_log(LOGSTDOUT, "error:__crfsnp_mgr_load_db: load crfsnp_home_dirs %u # failed\n", crfsnp_id);
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

static EC_BOOL __crfsnp_mgr_flush_db(CRFSNP_MGR *crfsnp_mgr, int crfsnp_mgr_fd)
{
    UINT32 crfsnp_mgr_db_size;
    UINT8* crfsnp_mgr_db_buff;
    UINT32 crfsnp_mgr_db_offset;
    UINT32 crfsnp_num;
    UINT32 crfsnp_id;

    /*init offset*/
    crfsnp_mgr_db_offset = 0;

    /*CRFSNP_MGR_NP_MODEL*/
    crfsnp_mgr_db_size   = sizeof(uint8_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_MODEL(crfsnp_mgr));    
    if(EC_FALSE == c_file_flush(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_flush_db: flush np model failed");
        return (EC_FALSE);
    }

    /*CRFSNP_MGR_NP_1ST_CHASH_ALGO_ID*/
    crfsnp_mgr_db_size   = sizeof(uint8_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_1ST_CHASH_ALGO_ID(crfsnp_mgr));    
    if(EC_FALSE == c_file_flush(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_flush_db: flush 1st chash algo id failed");
        return (EC_FALSE);
    }    

    /*CRFSNP_MGR_NP_2ND_CHASH_ALGO_ID*/
    crfsnp_mgr_db_size   = sizeof(uint8_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_2ND_CHASH_ALGO_ID(crfsnp_mgr));    
    if(EC_FALSE == c_file_flush(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_flush_db: flush 2nd chash algo id failed");
        return (EC_FALSE);
    }     
    
    /*CRFSNP_MGR_NP_ITEM_MAX_NUM*/
    crfsnp_mgr_db_size   = sizeof(uint32_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_ITEM_MAX_NUM(crfsnp_mgr));    
    if(EC_FALSE == c_file_flush(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_flush_db: flush item max num failed");
        return (EC_FALSE);
    }     

    /*CRFSNP_MGR_NP_MAX_NUM*/
    crfsnp_mgr_db_size   = sizeof(uint32_t);
    crfsnp_mgr_db_buff   = (UINT8 *)&(CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr));    
    if(EC_FALSE == c_file_flush(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_flush_db: flush disk max num failed");
        return (EC_FALSE);
    }

    /*CRFSNP_MGR_NP_HOME_DIRS_VEC*/
    crfsnp_num           = cvector_size(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr));
    crfsnp_mgr_db_size   = sizeof(UINT32);
    crfsnp_mgr_db_buff   = (UINT8 *)&(crfsnp_num);    
    if(EC_FALSE == c_file_flush(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_flush_db: flush home dir vec size failed");
        return (EC_FALSE);
    }    

    sys_log(LOGSTDOUT, "[DEBUG] __crfsnp_mgr_flush_db: np max num = %u\n", CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr));
    sys_log(LOGSTDOUT, "[DEBUG] __crfsnp_mgr_flush_db: np home dir vec size = %u\n", cvector_size(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr)));

    for(crfsnp_id = 0; crfsnp_id < crfsnp_num; crfsnp_id ++)
    {
        CRFSNP_HOME_DIRS * crfsnp_home_dirs;

        crfsnp_home_dirs = CRFSNP_MGR_NP_HOME_DIRS(crfsnp_mgr, crfsnp_id);
        if(NULL_PTR == crfsnp_home_dirs)
        {
            UINT32 home_dir_vec_size;
            home_dir_vec_size = 0;
            
            crfsnp_mgr_db_size   = sizeof(UINT32);
            crfsnp_mgr_db_buff   = (UINT8 *)&(home_dir_vec_size);    
            if(EC_FALSE == c_file_flush(crfsnp_mgr_fd, &crfsnp_mgr_db_offset, crfsnp_mgr_db_size, crfsnp_mgr_db_buff))
            {
                sys_log(LOGSTDOUT, "error:__crfsnp_mgr_flush_db: load home_dir_vec_size failed\n");
                return (EC_FALSE);
            }
            continue;
        }
        
        if(EC_FALSE == __crfsnp_home_dirs_flush(crfsnp_home_dirs, crfsnp_mgr_fd, &crfsnp_mgr_db_offset))
        {
            sys_log(LOGSTDOUT, "error:__crfsnp_mgr_flush_db: flush crfsnp_home_dirs %u # failed\n", crfsnp_id);
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_load_db(CRFSNP_MGR *crfsnp_mgr)
{
    char  *crfsnp_mgr_db_name;
    int    crfsnp_mgr_fd;

    crfsnp_mgr_db_name = __crfsnp_mgr_gen_db_name((char *)CRFSNP_MGR_DB_ROOT_DIR_STR(crfsnp_mgr));
    if(NULL_PTR == crfsnp_mgr_db_name)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_load_db: new str %s/%s failed\n", 
                            (char *)CRFSNP_MGR_DB_ROOT_DIR_STR(crfsnp_mgr), CRFSNP_DB_NAME);
        return (EC_FALSE);
    }

    if(EC_FALSE == c_file_access(crfsnp_mgr_db_name, F_OK))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_load_db: crfsnp mgr db %s not exist\n", crfsnp_mgr_db_name);
        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0026);
        return (EC_FALSE);
    }

    crfsnp_mgr_fd = c_file_open(crfsnp_mgr_db_name, O_RDONLY, 0666);
    if(ERR_FD == crfsnp_mgr_fd)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_load_db: open crfsnp mgr db %s failed\n", crfsnp_mgr_db_name);
        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0027);
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfsnp_mgr_load_db(crfsnp_mgr, crfsnp_mgr_fd))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_load_db: load db from crfsnp mgr db %s\n", crfsnp_mgr_db_name);
        c_file_close(crfsnp_mgr_fd);
        crfsnp_mgr_fd = ERR_FD;

        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0028);
        return (EC_FALSE);
    }

    c_file_close(crfsnp_mgr_fd);
    crfsnp_mgr_fd = ERR_FD;

    safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0029);
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_create_db(CRFSNP_MGR *crfsnp_mgr, const CSTRING *crfsnp_db_root_dir)
{
    char  *crfsnp_mgr_db_name;
    int    crfsnp_mgr_fd;

    crfsnp_mgr_db_name = __crfsnp_mgr_gen_db_name((char *)cstring_get_str(crfsnp_db_root_dir));
    if(NULL_PTR == crfsnp_mgr_db_name)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_create_db: new str %s/%s failed\n", 
                            (char *)cstring_get_str(crfsnp_db_root_dir), CRFSNP_DB_NAME);
        return (EC_FALSE);
    }

    if(EC_TRUE == c_file_access(crfsnp_mgr_db_name, F_OK))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_create_db: crfsnp mgr db %s already exist\n", crfsnp_mgr_db_name);
        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0030);
        return (EC_FALSE);
    }

    crfsnp_mgr_fd = c_file_open(crfsnp_mgr_db_name, O_RDWR | O_CREAT, 0666);
    if(ERR_FD == crfsnp_mgr_fd)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_create_db: open crfsnp mgr db %s failed\n", crfsnp_mgr_db_name);
        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0031);
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfsnp_mgr_flush_db(crfsnp_mgr, crfsnp_mgr_fd))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_create_db: flush db to crfsnp mgr db %s\n", crfsnp_mgr_db_name);
        c_file_close(crfsnp_mgr_fd);
        crfsnp_mgr_fd = ERR_FD;

        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0032);
        return (EC_FALSE);
    }    

    c_file_close(crfsnp_mgr_fd);
    crfsnp_mgr_fd = ERR_FD;

    safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0033);
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_flush_db(CRFSNP_MGR *crfsnp_mgr)
{
    char  *crfsnp_mgr_db_name;
    int    crfsnp_mgr_fd;

    crfsnp_mgr_db_name = __crfsnp_mgr_gen_db_name((char *)CRFSNP_MGR_DB_ROOT_DIR_STR(crfsnp_mgr));
    if(NULL_PTR == crfsnp_mgr_db_name)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_flush_db: new str %s/%s failed\n", 
                            (char *)CRFSNP_MGR_DB_ROOT_DIR_STR(crfsnp_mgr), CRFSNP_DB_NAME);
        return (EC_FALSE);
    }

    if(EC_FALSE == c_file_access(crfsnp_mgr_db_name, F_OK))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_flush_db: crfsnp mgr db %s not exist\n", crfsnp_mgr_db_name);
        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0034);
        return (EC_FALSE);
    }

    crfsnp_mgr_fd = c_file_open(crfsnp_mgr_db_name, O_RDWR, 0666);
    if(ERR_FD == crfsnp_mgr_fd)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_flush_db: open crfsnp mgr db %s failed\n", crfsnp_mgr_db_name);
        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0035);
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfsnp_mgr_flush_db(crfsnp_mgr, crfsnp_mgr_fd))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_flush_db: flush db to crfsnp mgr db %s\n", crfsnp_mgr_db_name);
        c_file_close(crfsnp_mgr_fd);
        crfsnp_mgr_fd = ERR_FD;

        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0036);
        return (EC_FALSE);
    }

    c_file_close(crfsnp_mgr_fd);
    crfsnp_mgr_fd = ERR_FD;

    sys_log(LOGSTDOUT, "[DEBUG] crfsnp_mgr_flush_db: flush db to crfsnp mgr db %s done\n", crfsnp_mgr_db_name);

    safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0037);
    return (EC_TRUE);
}

void crfsnp_mgr_print_db(LOG *log, const CRFSNP_MGR *crfsnp_mgr)
{
    uint32_t crfsnp_num;
    uint32_t crfsnp_id;

    sys_log(log, "crfsnp mgr db root dir  : %s\n", (char *)CRFSNP_MGR_DB_ROOT_DIR_STR(crfsnp_mgr));
    sys_log(log, "crfsnp model            : %u\n", CRFSNP_MGR_NP_MODEL(crfsnp_mgr));
    sys_log(log, "crfsnp 1st hash algo id : %u\n", CRFSNP_MGR_NP_1ST_CHASH_ALGO_ID(crfsnp_mgr));
    sys_log(log, "crfsnp 2nd hash algo id : %u\n", CRFSNP_MGR_NP_2ND_CHASH_ALGO_ID(crfsnp_mgr));
    sys_log(log, "crfsnp item max num     : %u\n", CRFSNP_MGR_NP_ITEM_MAX_NUM(crfsnp_mgr));
    sys_log(log, "crfsnp max num          : %u\n", CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr));

    crfsnp_num = (uint32_t)cvector_size(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr));
    for(crfsnp_id = 0; crfsnp_id < crfsnp_num; crfsnp_id ++)
    {
        sys_log(log, "np %u #: home dirs are\n", crfsnp_id);
        __crfsnp_home_dirs_print(log, CRFSNP_MGR_NP_HOME_DIRS(crfsnp_mgr, crfsnp_id));
    }

    crfsnp_num = (uint32_t)cvector_size(CRFSNP_MGR_NP_VEC(crfsnp_mgr));
    for(crfsnp_id = 0; crfsnp_id < crfsnp_num; crfsnp_id ++)
    {
        CRFSNP *crfsnp;

        crfsnp = CRFSNP_MGR_NP(crfsnp_mgr, crfsnp_id);
        if(NULL_PTR == crfsnp)
        {
            sys_log(log, "np %u #: (null)\n", crfsnp_id);
        }
        else
        {
            crfsnp_print(log, crfsnp);
        }
    }
    return;
}

void crfsnp_mgr_print(LOG *log, const CRFSNP_MGR *crfsnp_mgr)
{
    sys_log(log, "crfsnp mgr:\n");
    crfsnp_mgr_print_db(log, crfsnp_mgr);
    return;
}

EC_BOOL crfsnp_mgr_load(CRFSNP_MGR *crfsnp_mgr, const CSTRING *crfsnp_db_root_dir)
{
    cstring_clean(CRFSNP_MGR_DB_ROOT_DIR(crfsnp_mgr));
    cstring_clone(crfsnp_db_root_dir, CRFSNP_MGR_DB_ROOT_DIR(crfsnp_mgr));

    if(EC_FALSE == crfsnp_mgr_load_db(crfsnp_mgr))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_load: load cfg db failed from dir %s\n", (char *)cstring_get_str(crfsnp_db_root_dir));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_sync_np(CRFSNP_MGR *crfsnp_mgr, const uint32_t crfsnp_id)
{
    CRFSNP *crfsnp;
    
    crfsnp = (CRFSNP *)cvector_get_no_lock(CRFSNP_MGR_NP_VEC(crfsnp_mgr), crfsnp_id);
    if(NULL_PTR != crfsnp)
    {
        return crfsnp_sync(crfsnp);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_flush(CRFSNP_MGR *crfsnp_mgr)
{
    uint32_t crfsnp_num;
    uint32_t crfsnp_id;
    EC_BOOL ret;

    ret = EC_TRUE;

    if(EC_FALSE == crfsnp_mgr_flush_db(crfsnp_mgr))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_flush: flush cfg db failed\n");
        ret = EC_FALSE;
    }

    crfsnp_num = CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr);
    for(crfsnp_id = 0; crfsnp_id < crfsnp_num; crfsnp_id ++)
    {
        crfsnp_mgr_sync_np(crfsnp_mgr, crfsnp_id);
    }
    return (ret);
}

EC_BOOL crfsnp_mgr_show_np(LOG *log, CRFSNP_MGR *crfsnp_mgr, const uint32_t crfsnp_id)
{
    CRFSNP *crfsnp;

    crfsnp = (CRFSNP *)cvector_get_no_lock(CRFSNP_MGR_NP_VEC(crfsnp_mgr), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {        
        /*try to open the np and print it*/
        crfsnp = crfsnp_mgr_open_np(crfsnp_mgr, crfsnp_id);
        if(NULL_PTR == crfsnp)
        {
            sys_log(LOGSTDOUT, "error:crfsnp_mgr_show_np: open np %u failed\n", crfsnp_id);
            return (EC_FALSE);
        } 

        __crfsnp_home_dirs_print(log, CRFSNP_MGR_NP_HOME_DIRS(crfsnp_mgr, crfsnp_id));

        crfsnp_print(log, crfsnp);

        crfsnp_mgr_close_np(crfsnp_mgr, crfsnp_id);
    }
    else
    {
        __crfsnp_home_dirs_print(log, CRFSNP_MGR_NP_HOME_DIRS(crfsnp_mgr, crfsnp_id));
        
        crfsnp_print(log, crfsnp);
    }

    return (EC_TRUE);
}

static uint32_t __crfsnp_mgr_get_np_id_of_path(const CRFSNP_MGR *crfsnp_mgr, const uint32_t path_len, const uint8_t *path, CSTRING **home_dir_cstr)
{
    uint32_t crfsnp_num;
    uint32_t crfsnp_id;

    ASSERT(CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr) == cvector_size(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr)));

    CRFSNP_MGR_NP_HOME_DIRS_VEC_LOCK(crfsnp_mgr, LOC_CRFSNPMGR_0038);
    crfsnp_num = CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr);
    for(crfsnp_id = 0; crfsnp_id < crfsnp_num; crfsnp_id ++)
    {
        CSTRING *home_dir_cstr_t;
        home_dir_cstr_t = __crfsnp_home_dirs_match_str(CRFSNP_MGR_NP_HOME_DIRS(crfsnp_mgr, crfsnp_id), path_len, path); 
        if(NULL_PTR != home_dir_cstr_t)
        {
            CRFSNP_MGR_NP_HOME_DIRS_VEC_UNLOCK(crfsnp_mgr, LOC_CRFSNPMGR_0039);
            if(NULL_PTR != home_dir_cstr)
            {
                (*home_dir_cstr) = home_dir_cstr_t;
            }
            return (crfsnp_id);
        }
    }
    CRFSNP_MGR_NP_HOME_DIRS_VEC_UNLOCK(crfsnp_mgr, LOC_CRFSNPMGR_0040);
    return (CRFSNP_ERR_ID);
}

static CRFSNP *__crfsnp_mgr_get_np(CRFSNP_MGR *crfsnp_mgr, const uint32_t path_len, const uint8_t *path, uint32_t *np_id)
{
    CRFSNP  * crfsnp;
    uint32_t  crfsnp_id;

    crfsnp_id = __crfsnp_mgr_get_np_id_of_path(crfsnp_mgr, path_len, path, NULL_PTR);
    if(CRFSNP_ERR_ID == crfsnp_id)
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_get_np: no np for path %.*s\n", path_len, (char *)path);
        return (NULL_PTR);
    }

    CRFSNP_MGR_CMUTEX_LOCK(crfsnp_mgr, LOC_CRFSNPMGR_0041);
    crfsnp = crfsnp_mgr_open_np(crfsnp_mgr, crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        CRFSNP_MGR_CMUTEX_UNLOCK(crfsnp_mgr, LOC_CRFSNPMGR_0042);
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_get_np: path %.*s in np %u but cannot open\n", path_len, path, crfsnp_id);
        return (NULL_PTR);
    }
    CRFSNP_MGR_CMUTEX_UNLOCK(crfsnp_mgr, LOC_CRFSNPMGR_0043);

    if(NULL_PTR != np_id)
    {
        (*np_id) = crfsnp_id;
    }
    
    return (crfsnp);           
}

EC_BOOL crfsnp_mgr_search(CRFSNP_MGR *crfsnp_mgr, const uint32_t path_len, const uint8_t *path, const uint32_t dflag, uint32_t *searched_crfsnp_id)
{
    CRFSNP   *crfsnp;
    uint32_t  crfsnp_id;    
    uint32_t  node_pos;
    
    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, path_len, path, &crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_search: path %.*s in np %u but cannot open\n", path_len, path, crfsnp_id);
        return (EC_FALSE);
    }

    node_pos = crfsnp_search(crfsnp, path_len, path, dflag);
    if(CRFSNPRB_ERR_POS == node_pos)
    {
        sys_log(LOGSTDOUT, "[DEBUG] crfsnp_mgr_search: path %.*s in np %u but not found indeed\n", path_len, path, crfsnp_id);
        return (EC_FALSE);
    }

    if(NULL_PTR != searched_crfsnp_id)
    {
        (*searched_crfsnp_id) = crfsnp_id;
    }

    return (EC_TRUE);
}

CRFSNP_ITEM *crfsnp_mgr_search_item(CRFSNP_MGR *crfsnp_mgr, const uint32_t path_len, const uint8_t *path, const uint32_t dflag)
{
    CRFSNP   *crfsnp;
    uint32_t  crfsnp_id;    
    uint32_t  node_pos;
    
    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, path_len, path, &crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_search_item: path %.*s in np %u but cannot open\n", path_len, path, crfsnp_id);
        return (NULL_PTR);
    }

    node_pos = crfsnp_search(crfsnp, path_len, path, dflag);
    if(CRFSNPRB_ERR_POS == node_pos)
    {
        sys_log(LOGSTDOUT, "[DEBUG] crfsnp_mgr_search_item: path %.*s in np %u but not found indeed\n", path_len, path, crfsnp_id);
        return (NULL_PTR);
    }

    return crfsnp_fetch(crfsnp, node_pos);
}

CRFSNP_MGR *crfsnp_mgr_create(const uint8_t crfsnp_model, 
                                const uint32_t crfsnp_max_num, 
                                const uint8_t  crfsnp_1st_chash_algo_id, 
                                const uint8_t  crfsnp_2nd_chash_algo_id, 
                                const CSTRING *crfsnp_db_root_dir)
{
    CRFSNP_MGR *crfsnp_mgr;
    uint32_t crfsnp_item_max_num;
    uint32_t crfsnp_id;
    
    if(EC_FALSE == crfsnp_model_item_max_num(crfsnp_model , &crfsnp_item_max_num))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_create: invalid crfsnp model %u\n", crfsnp_model);
        return (NULL_PTR);
    }

    crfsnp_mgr = crfsnp_mgr_new();

    CRFSNP_MGR_NP_MODEL(crfsnp_mgr)                = crfsnp_model;
    CRFSNP_MGR_NP_1ST_CHASH_ALGO_ID(crfsnp_mgr)    = crfsnp_1st_chash_algo_id;
    CRFSNP_MGR_NP_2ND_CHASH_ALGO_ID(crfsnp_mgr)    = crfsnp_2nd_chash_algo_id;
    CRFSNP_MGR_NP_ITEM_MAX_NUM(crfsnp_mgr)         = crfsnp_item_max_num;
    CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr)              = crfsnp_max_num;

    cstring_clone(crfsnp_db_root_dir, CRFSNP_MGR_DB_ROOT_DIR(crfsnp_mgr));

    for(crfsnp_id = 0; crfsnp_id < crfsnp_max_num; crfsnp_id ++)
    {
        const char *np_root_dir;
        CRFSNP *crfsnp;
        CRFSNP_HOME_DIRS *crfsnp_home_dirs;

        np_root_dir = (const char *)cstring_get_str(crfsnp_db_root_dir);/*Oops! int the same dire*/
        crfsnp = crfsnp_create(np_root_dir, crfsnp_id, crfsnp_model, crfsnp_1st_chash_algo_id, crfsnp_2nd_chash_algo_id);
        if(NULL_PTR == crfsnp)
        {
            sys_log(LOGSTDOUT, "error:crfsnp_mgr_create: create np %u failed\n", crfsnp_id);
            return (NULL_PTR);
        }
        crfsnp_close(crfsnp);

        crfsnp_home_dirs = __crfsnp_home_dirs_new();
        if(NULL_PTR == crfsnp_home_dirs)
        {
            sys_log(LOGSTDOUT, "error:crfsnp_mgr_create: new crfsnp_home_dirs for np %u failed\n", crfsnp_id);
            return (NULL_PTR);
        }
        
        cvector_push_no_lock(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr), (void *)crfsnp_home_dirs);
        cvector_push_no_lock(CRFSNP_MGR_NP_VEC(crfsnp_mgr), (void *)NULL_PTR);
    }

    if(EC_FALSE == crfsnp_mgr_create_db(crfsnp_mgr, crfsnp_db_root_dir))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_create: create cfg db failed in root dir %s\n",
                            (char *)cstring_get_str(crfsnp_db_root_dir));
        crfsnp_mgr_free(crfsnp_mgr);
        return (NULL_PTR);
    }

    //crfsnp_mgr_free(crfsnp_mgr);
    return (crfsnp_mgr);
}

EC_BOOL crfsnp_mgr_exist(const CSTRING *crfsnp_db_root_dir)
{
    char  *crfsnp_mgr_db_name;

    crfsnp_mgr_db_name = __crfsnp_mgr_gen_db_name((char *)cstring_get_str(crfsnp_db_root_dir));
    if(NULL_PTR == crfsnp_mgr_db_name)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_exist: new str %s/%s failed\n", 
                            (char *)cstring_get_str(crfsnp_db_root_dir), CRFSNP_DB_NAME);
        return (EC_FALSE);
    }

    if(EC_FALSE == c_file_access(crfsnp_mgr_db_name, F_OK))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_exist: crfsnp mgr db %s not exist\n", crfsnp_mgr_db_name);
        safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0044);
        return (EC_FALSE);
    }
    safe_free(crfsnp_mgr_db_name, LOC_CRFSNPMGR_0045);
    return (EC_TRUE);
}

CRFSNP_MGR * crfsnp_mgr_open(const CSTRING *crfsnp_db_root_dir)
{
    CRFSNP_MGR *crfsnp_mgr;

    crfsnp_mgr = crfsnp_mgr_new();
    if(NULL_PTR == crfsnp_mgr)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_open: new crfsnp mgr failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == crfsnp_mgr_load(crfsnp_mgr, crfsnp_db_root_dir))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_open: load failed\n");
        crfsnp_mgr_free(crfsnp_mgr);
        return (NULL_PTR);
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfsnp_mgr_open: crfsnp mgr loaded from %s\n", (char *)cstring_get_str(crfsnp_db_root_dir));
    return (crfsnp_mgr);
}

EC_BOOL crfsnp_mgr_close(CRFSNP_MGR *crfsnp_mgr)
{    
    if(NULL_PTR != crfsnp_mgr)
    {
        CRFSNP_MGR_CMUTEX_LOCK(crfsnp_mgr, LOC_CRFSNPMGR_0046);
        crfsnp_mgr_flush(crfsnp_mgr);
        CRFSNP_MGR_CMUTEX_UNLOCK(crfsnp_mgr, LOC_CRFSNPMGR_0047);
        crfsnp_mgr_free(crfsnp_mgr);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_collect_items(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path, const UINT32 dflag, uint32_t *crfsnp_id, CVECTOR *crfsnp_item_vec)
{
    CRFSNP  *crfsnp;
    uint32_t node_pos;
    CRFSNP_ITEM *crfsnp_item;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_collect_items: no np for path %s\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    node_pos = crfsnp_search_no_lock(crfsnp, cstring_get_len(path), cstring_get_str(path), dflag);
    if(CRFSNPRB_ERR_POS != node_pos)
    {
        CRFSNP_ITEM *crfsnp_item_collected;

        crfsnp_item = crfsnp_fetch(crfsnp, node_pos);

        crfsnp_item_collected = crfsnp_item_new();
        crfsnp_item_clone(crfsnp_item, crfsnp_item_collected);

        cvector_push_no_lock(crfsnp_item_vec, (void *)crfsnp_item_collected);
    }    

    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_find_dir(CRFSNP_MGR *crfsnp_mgr, const CSTRING *dir_path)
{
    return crfsnp_mgr_search(crfsnp_mgr, (uint32_t)cstring_get_len(dir_path), cstring_get_str(dir_path), CRFSNP_ITEM_FILE_IS_DIR, NULL_PTR);
}

EC_BOOL crfsnp_mgr_find_file(CRFSNP_MGR *crfsnp_mgr, const CSTRING *file_path)
{
    return crfsnp_mgr_search(crfsnp_mgr, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), CRFSNP_ITEM_FILE_IS_REG, NULL_PTR);
}

EC_BOOL crfsnp_mgr_find(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path, const UINT32 dflag)
{
    if(0 == strcmp("/", (char *)cstring_get_str(path)))/*patch*/
    {
        if(CRFSNP_ITEM_FILE_IS_ANY == dflag || CRFSNP_ITEM_FILE_IS_DIR == dflag)
        {
            return (EC_TRUE);
        }
        return (EC_FALSE);
    }

    return crfsnp_mgr_search(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), dflag, NULL_PTR);
}

/*bind a home_dir to specific name node*/
EC_BOOL crfsnp_mgr_bind(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path, const UINT32 crfsnp_id)
{
    CSTRING *home_dir;
    uint32_t crfsnp_id_t;

    sys_log(LOGSTDOUT, "[DEBUG] crfsnp_mgr_bind: CRFSNP_MGR_NP_HOME_DIRS_VEC = %u, CRFSNP_MGR_NP_MAX_NUM = %u\n", 
                        cvector_size(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr)), CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr));
    ASSERT(cvector_size(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr)) == CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr));
    //TODO: check validity of path and crfsnp_id
    if(CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr) <= crfsnp_id)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_bind: max np num %u but crfsnp id %u overflow\n", CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr), crfsnp_id);
        return (EC_FALSE);
    }

    crfsnp_id_t = __crfsnp_mgr_get_np_id_of_path(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), &home_dir);
    if(CRFSNP_ERR_ID != crfsnp_id_t)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_bind: some dir %s already bound to np %u, thus cannot accept binding %s\n", 
                            (char *)cstring_get_str(home_dir), crfsnp_id_t, (char *)cstring_get_str(path));
        return (EC_FALSE);
    }    

    home_dir = cstring_dup(path);
    if(NULL_PTR == home_dir)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_bind: dup %s failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    __crfsnp_home_dirs_bind(CRFSNP_MGR_NP_HOME_DIRS(crfsnp_mgr, crfsnp_id), home_dir);

    /*flush new home_dir to disk*/
    if(EC_FALSE == crfsnp_mgr_flush_db(crfsnp_mgr))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_bind: flush db failed\n");
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

/*unbind one home_dir from specific name node*/
EC_BOOL crfsnp_mgr_unbind(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path, const UINT32 crfsnp_id)
{
    CSTRING *home_dir_old;

    ASSERT(cvector_size(CRFSNP_MGR_NP_HOME_DIRS_VEC(crfsnp_mgr)) == CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr));
    //TODO: check validity of path and crfsnp_id
    if(CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr) <= crfsnp_id)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_unbind: max np num %u but crfsnp id %u overflow\n", CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr), crfsnp_id);
        return (EC_FALSE);
    }

    home_dir_old = __crfsnp_home_dirs_unbind(CRFSNP_MGR_NP_HOME_DIRS(crfsnp_mgr, crfsnp_id), path);
    if(NULL_PTR != home_dir_old)
    {
        cstring_free(home_dir_old);
        
        /*flush new home_dir to disk*/
        if(EC_FALSE == crfsnp_mgr_flush_db(crfsnp_mgr))
        {
            sys_log(LOGSTDOUT, "error:crfsnp_mgr_unbind: flush db failed\n");
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_write(CRFSNP_MGR *crfsnp_mgr, const CSTRING *file_path, const CRFSNP_FNODE *crfsnp_fnode)
{
    CRFSNP *crfsnp;
    CRFSNP_ITEM *crfsnp_item;
    uint32_t crfsnp_id;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), &crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_write: no np for path %s\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    crfsnp_item = crfsnp_set(crfsnp, cstring_get_len(file_path), cstring_get_str(file_path), CRFSNP_ITEM_FILE_IS_REG);
    if(NULL_PTR == crfsnp_item)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_write: set file %s to np %u failed\n",
                            (char *)cstring_get_str(file_path), crfsnp_id);
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_REG != CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_write: file path %s is not regular file\n", 
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfsnp_fnode_import(crfsnp_fnode, CRFSNP_ITEM_FNODE(crfsnp_item)))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_write: import fnode to item failed where path %s\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfsnp_mgr_write: import fnode to item successfully where path %s\n", (char *)cstring_get_str(file_path));
    crfsnp_item_print(LOGSTDOUT, crfsnp_item);
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_read(CRFSNP_MGR *crfsnp_mgr, const CSTRING *file_path, CRFSNP_FNODE *crfsnp_fnode)
{
    CRFSNP *crfsnp;
    uint32_t crfsnp_id;
    uint32_t node_pos;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), &crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_read: no np for path %s\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    node_pos = crfsnp_search_no_lock(crfsnp, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), CRFSNP_ITEM_FILE_IS_REG);
    if(CRFSNPRB_ERR_POS != node_pos)
    {
        CRFSNP_ITEM *crfsnp_item;

        crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
        return crfsnp_fnode_import(CRFSNP_ITEM_FNODE(crfsnp_item), crfsnp_fnode);
    }
    return (EC_FALSE);    
}

EC_BOOL crfsnp_mgr_write_b(CRFSNP_MGR *crfsnp_mgr, const CSTRING *file_path)
{
    CRFSNP *crfsnp;
    CRFSNP_ITEM *crfsnp_item;
    uint32_t crfsnp_id;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), &crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_write_b: no np for path %s\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    crfsnp_item = crfsnp_set(crfsnp, cstring_get_len(file_path), cstring_get_str(file_path), CRFSNP_ITEM_FILE_IS_BIG);
    if(NULL_PTR == crfsnp_item)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_write_b: set file %s to np %u failed\n",
                            (char *)cstring_get_str(file_path), crfsnp_id);
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_write_b: file path %s is not big file\n", 
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_read_b(CRFSNP_MGR *crfsnp_mgr, const CSTRING *file_path, uint32_t *crfsnp_id, uint32_t *parent_pos)
{
    CRFSNP *crfsnp;
    uint32_t node_pos;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_read_b: no np for path %s\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    node_pos = crfsnp_search_no_lock(crfsnp, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), CRFSNP_ITEM_FILE_IS_BIG);
    if(CRFSNPRB_ERR_POS != node_pos)
    {
        if(NULL_PTR != parent_pos)
        {
            (*parent_pos) = node_pos;
        }
        return (EC_TRUE);
    }
    return (EC_FALSE);    
}

EC_BOOL crfsnp_mgr_update(CRFSNP_MGR *crfsnp_mgr, const CSTRING *file_path, const CRFSNP_FNODE *crfsnp_fnode)
{
    CRFSNP *crfsnp;
    uint32_t crfsnp_id;
    uint32_t node_pos;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), &crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_update: no np for path %s\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    node_pos = crfsnp_search_no_lock(crfsnp, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), CRFSNP_ITEM_FILE_IS_REG);
    if(CRFSNPRB_ERR_POS != node_pos)
    {
        CRFSNP_ITEM *crfsnp_item;

        crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
        return crfsnp_fnode_import(crfsnp_fnode, CRFSNP_ITEM_FNODE(crfsnp_item));
    }
    return (EC_FALSE);    
}

EC_BOOL crfsnp_mgr_delete(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path, const UINT32 dflag, uint32_t *crfsnp_id, CVECTOR *crfsnp_item_vec)
{
    CRFSNP *crfsnp;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_delete: no np for path %s\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    return crfsnp_delete(crfsnp, (uint32_t)cstring_get_len(path), cstring_get_str(path), dflag, crfsnp_item_vec);
}

EC_BOOL crfsnp_mgr_mkdir(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path)
{
    CRFSNP *crfsnp;
    CRFSNP_ITEM *crfsnp_item;
    uint32_t crfsnp_id;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), &crfsnp_id);;
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_mkdir: no np for path %s failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    crfsnp_item = crfsnp_set(crfsnp, cstring_get_len(path), cstring_get_str(path), CRFSNP_ITEM_FILE_IS_DIR);
    if(NULL_PTR == crfsnp_item)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_mkdir: mkdir %s in np %u failed\n",
                            (char *)cstring_get_str(path), crfsnp_id);
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_DIR != CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_mkdir: path %s is not dir in np %u\n", (char *)cstring_get_str(path), crfsnp_id);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_list_path(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path, CVECTOR  *path_cstr_vec)
{
    CRFSNP *crfsnp;
    uint32_t crfsnp_id;
    uint32_t node_pos;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), &crfsnp_id);;
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_list_path: no np for path %s failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    node_pos = crfsnp_search_no_lock(crfsnp, cstring_get_len(path), cstring_get_str(path), CRFSNP_ITEM_FILE_IS_ANY);
    if(CRFSNPRB_ERR_POS == node_pos)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_list_path: not found path %s in np %u\n", 
                            (char *)cstring_get_str(path), crfsnp_id);
        return (EC_FALSE);
    }

    return crfsnp_list_path_vec(crfsnp, node_pos, path_cstr_vec);
}

EC_BOOL crfsnp_mgr_list_seg(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path, CVECTOR  *seg_cstr_vec)
{
    CRFSNP *crfsnp;
    uint32_t crfsnp_id;
    uint32_t node_pos;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), &crfsnp_id);;
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_list_seg: no np for path %s failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    node_pos = crfsnp_search_no_lock(crfsnp, cstring_get_len(path), cstring_get_str(path), CRFSNP_ITEM_FILE_IS_ANY);
    if(CRFSNPRB_ERR_POS == node_pos)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_list_seg: not found path %s in np %u\n", 
                            (char *)cstring_get_str(path), crfsnp_id);
        return (EC_FALSE);
    }

    return crfsnp_list_seg_vec(crfsnp, node_pos, seg_cstr_vec);
}

EC_BOOL crfsnp_mgr_file_num(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path_cstr, UINT32 *file_num)
{
    CVECTOR *crfsnp_item_vec;
    UINT32   crfsnp_item_pos;
    uint32_t crfsnp_id;

    (*file_num) = 0;

    crfsnp_item_vec = cvector_new(0, MM_CRFSNP_ITEM, LOC_CRFSNPMGR_0048);
    if(NULL_PTR == crfsnp_item_vec)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_file_num: new cvector failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsnp_mgr_collect_items(crfsnp_mgr, path_cstr, CRFSNP_ITEM_FILE_IS_ANY, &crfsnp_id, crfsnp_item_vec))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_file_num: collect item of path %s failed\n", (char *)cstring_get_str(path_cstr));

        cvector_clean_no_lock(crfsnp_item_vec, (CVECTOR_DATA_CLEANER)crfsnp_item_free, LOC_CRFSNPMGR_0049);
        cvector_free_no_lock(crfsnp_item_vec, LOC_CRFSNPMGR_0050);
        return (EC_FALSE);
    }

    for(crfsnp_item_pos = 0; crfsnp_item_pos < cvector_size(crfsnp_item_vec); crfsnp_item_pos ++)
    {
        CRFSNP_ITEM *crfsnp_item;

        crfsnp_item = (CRFSNP_ITEM *)cvector_get_no_lock(crfsnp_item_vec, crfsnp_item_pos);
        if(NULL_PTR == crfsnp_item)
        {
            continue;
        }

        if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            (*file_num) ++;
            continue;
        }

        if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            (*file_num) ++;
            continue;
        }        

        if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            CRFSNP_DNODE *crfsnp_dnode;
            crfsnp_dnode = CRFSNP_ITEM_DNODE(crfsnp_item);

            (*file_num) += CRFSNP_DNODE_FILE_NUM(crfsnp_dnode);
            continue;
        }

        sys_log(LOGSTDOUT, "error:crfsnp_mgr_file_num: invalid dflg %lx\n", CRFSNP_ITEM_DFLG(crfsnp_item));
    }

    cvector_clean_no_lock(crfsnp_item_vec, (CVECTOR_DATA_CLEANER)crfsnp_item_free, LOC_CRFSNPMGR_0051);
    cvector_free_no_lock(crfsnp_item_vec, LOC_CRFSNPMGR_0052);
    return (EC_TRUE);
}

static EC_BOOL __crfsnp_mgr_node_size(CRFSNP_MGR *crfsnp_mgr, CRFSNP *crfsnp, uint32_t node_pos, uint64_t *file_size)
{
    CRFSNPRB_POOL *pool;
    CRFSNPRB_NODE *node;
    CRFSNP_ITEM   *item;

    if(CRFSNPRB_ERR_POS == node_pos)
    {
        return (EC_TRUE);
    }

    pool = CRFSNP_ITEMS_POOL(crfsnp);
    node  = CRFSNPRB_POOL_NODE(pool, node_pos);   

    item = (CRFSNP_ITEM *)CRFSNP_RB_NODE_ITEM(node);
    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(item))
    {
        CRFSNP_FNODE *crfsnp_fnode;
        crfsnp_fnode = CRFSNP_ITEM_FNODE(item);

        (*file_size) += CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    }

    else if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(item))
    {
        CRFSNP_BNODE *crfsnp_bnode;
        crfsnp_bnode = CRFSNP_ITEM_BNODE(item);

        sys_log(LOGSTDOUT, "[DEBUG] __crfsnp_mgr_node_size: bnode fsize = %llu\n", CRFSNP_BNODE_FILESZ(crfsnp_bnode));

        (*file_size) += CRFSNP_BNODE_FILESZ(crfsnp_bnode);
    }
    else if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(item))
    {
        /*skip it, never step down*/
    }
    else
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_node_size: invalid dflg %lx\n", CRFSNP_ITEM_DFLG(item));
        return (EC_FALSE);
    }

    /*run through left subtree*/
    __crfsnp_mgr_node_size(crfsnp_mgr, crfsnp, CRFSNPRB_NODE_LEFT_POS(node), file_size);

    /*run through right subtree*/
    __crfsnp_mgr_node_size(crfsnp_mgr, crfsnp, CRFSNPRB_NODE_RIGHT_POS(node), file_size);

    return (EC_TRUE);
}

/*total file size under the directory, never search the directory in depth*/
static EC_BOOL __crfsnp_mgr_dir_size(CRFSNP_MGR *crfsnp_mgr, uint32_t crfsnp_id, const CRFSNP_DNODE *crfsnp_dnode, uint64_t *file_size)
{
    CRFSNP  *crfsnp;
    uint32_t bucket_pos;

    crfsnp = crfsnp_mgr_open_np(crfsnp_mgr, crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_mgr_dir_size: open np %u failed\n", crfsnp_id);
        return (EC_FALSE);
    }

    for(bucket_pos = 0; bucket_pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; bucket_pos ++)
    {
        uint32_t node_pos;

        node_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, bucket_pos);
        if(CRFSNPRB_ERR_POS != node_pos)
        {
            __crfsnp_mgr_node_size(crfsnp_mgr, crfsnp, node_pos, file_size);
        }
    }

    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_file_size(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path_cstr, uint64_t *file_size)
{
    CVECTOR *crfsnp_item_vec;
    UINT32   crfsnp_item_pos;
    uint32_t crfsnp_id;

    (*file_size) = 0;

    crfsnp_item_vec = cvector_new(0, MM_CRFSNP_ITEM, LOC_CRFSNPMGR_0053);
    if(NULL_PTR == crfsnp_item_vec)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_file_size: new cvector failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsnp_mgr_collect_items(crfsnp_mgr, path_cstr, CRFSNP_ITEM_FILE_IS_ANY, &crfsnp_id, crfsnp_item_vec))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_file_size: collect item of path %s failed\n", (char *)cstring_get_str(path_cstr));
        cvector_clean_no_lock(crfsnp_item_vec, (CVECTOR_DATA_CLEANER)crfsnp_item_free, LOC_CRFSNPMGR_0054);
        cvector_free_no_lock(crfsnp_item_vec, LOC_CRFSNPMGR_0055);
        return (EC_FALSE);
    }

    for(crfsnp_item_pos = 0; crfsnp_item_pos < cvector_size(crfsnp_item_vec); crfsnp_item_pos ++)
    {
        CRFSNP_ITEM *crfsnp_item;

        crfsnp_item = (CRFSNP_ITEM *)cvector_get_no_lock(crfsnp_item_vec, crfsnp_item_pos);
        if(NULL_PTR == crfsnp_item)
        {
            continue;
        }

        if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            CRFSNP_FNODE *crfsnp_fnode;
            crfsnp_fnode = CRFSNP_ITEM_FNODE(crfsnp_item);

            (*file_size) += CRFSNP_FNODE_FILESZ(crfsnp_fnode);
            continue;
        }

        if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            CRFSNP_BNODE *crfsnp_bnode;
            crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item);

            sys_log(LOGSTDOUT, "[DEBUG] crfsnp_mgr_file_size: bnode fsize = %llu\n", CRFSNP_BNODE_FILESZ(crfsnp_bnode));

            (*file_size) += CRFSNP_BNODE_FILESZ(crfsnp_bnode);
            continue;
        }        

        if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            CRFSNP_DNODE *crfsnp_dnode;
            crfsnp_dnode = CRFSNP_ITEM_DNODE(crfsnp_item);
            __crfsnp_mgr_dir_size(crfsnp_mgr, crfsnp_id, crfsnp_dnode, file_size);
            continue;
        }

        sys_log(LOGSTDOUT, "error:crfsnp_mgr_file_size: invalid dflg %lx\n", CRFSNP_ITEM_DFLG(crfsnp_item));
    }

    cvector_clean_no_lock(crfsnp_item_vec, (CVECTOR_DATA_CLEANER)crfsnp_item_free, LOC_CRFSNPMGR_0056);
    cvector_free_no_lock(crfsnp_item_vec, LOC_CRFSNPMGR_0057);
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_show_cached_np(LOG *log, const CRFSNP_MGR *crfsnp_mgr)
{
    uint32_t crfsnp_num;
    uint32_t crfsnp_pos;

    crfsnp_num = cvector_size(CRFSNP_MGR_NP_VEC(crfsnp_mgr));
    for(crfsnp_pos = 0; crfsnp_pos < crfsnp_num; crfsnp_pos ++)
    {
        CRFSNP *crfsnp;

        crfsnp = CRFSNP_MGR_NP(crfsnp_mgr, crfsnp_pos);
        if(NULL_PTR != crfsnp)
        {
            __crfsnp_home_dirs_print(log, CRFSNP_MGR_NP_HOME_DIRS(crfsnp_mgr, crfsnp_pos));
            
            crfsnp_print(log, crfsnp);
        }
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_show_path_depth(LOG *log, CRFSNP_MGR *crfsnp_mgr, const CSTRING *path)
{
    CRFSNP *crfsnp;
    uint32_t crfsnp_id;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), &crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_show_path_depth: no np for path %s\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfsnp_mgr_show_path_depth: crfsnp %p, id %u\n", crfsnp, crfsnp_id);

    return crfsnp_show_path_depth(log, crfsnp, (uint32_t)cstring_get_len(path), cstring_get_str(path));
}

EC_BOOL crfsnp_mgr_show_path(LOG *log, CRFSNP_MGR *crfsnp_mgr, const CSTRING *path)
{
    CRFSNP *crfsnp;
    uint32_t crfsnp_id;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), &crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_show_path: no np for path %s\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    return crfsnp_show_path(log, crfsnp, (uint32_t)cstring_get_len(path), cstring_get_str(path));
}

EC_BOOL crfsnp_mgr_get_first_fname_of_path(CRFSNP_MGR *crfsnp_mgr, const CSTRING *path, CSTRING *fname, uint32_t *dflag)
{
    CRFSNP *crfsnp;
    uint32_t crfsnp_id;
    uint8_t *fname_str;

    crfsnp = __crfsnp_mgr_get_np(crfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), &crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_get_first_fname_of_path: no np for path %s\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsnp_get_first_fname_of_path(crfsnp, (uint32_t)cstring_get_len(path), cstring_get_str(path), &fname_str, dflag))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mgr_get_first_fname_of_path: get first fname of path %s failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    cstring_set_str(fname, fname_str);

    return (EC_TRUE);
}

EC_BOOL crfsnp_mgr_rdlock(CRFSNP_MGR *crfsnp_mgr, const UINT32 location)
{
    return CRFSNP_MGR_CRWLOCK_RDLOCK(crfsnp_mgr, location);
}

EC_BOOL crfsnp_mgr_wrlock(CRFSNP_MGR *crfsnp_mgr, const UINT32 location)
{
    return CRFSNP_MGR_CRWLOCK_WRLOCK(crfsnp_mgr, location);
}

EC_BOOL crfsnp_mgr_unlock(CRFSNP_MGR *crfsnp_mgr, const UINT32 location)
{
    return CRFSNP_MGR_CRWLOCK_UNLOCK(crfsnp_mgr, location);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

