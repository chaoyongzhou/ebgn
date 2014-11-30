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

#include "chfsnp.h"
#include "chfsnprb.h"
#include "chfsnpmgr.h"
#include "chashalgo.h"
#include "cfuse.h"

#include "findex.inc"


CHFSNP_MGR *chfsnp_mgr_new()
{
    CHFSNP_MGR *chfsnp_mgr;

    alloc_static_mem(MD_CHFS, CMPI_ANY_MODI, MM_CHFSNP_MGR, &chfsnp_mgr, LOC_CHFSNPMGR_0001);
    if(NULL_PTR != chfsnp_mgr)
    {
        chfsnp_mgr_init(chfsnp_mgr);
    }

    return (chfsnp_mgr);
}

EC_BOOL chfsnp_mgr_init(CHFSNP_MGR *chfsnp_mgr)
{
    CHFSNP_MGR_CRWLOCK_INIT(chfsnp_mgr, LOC_CHFSNPMGR_0002);
    CHFSNP_MGR_CMUTEX_INIT(chfsnp_mgr, LOC_CHFSNPMGR_0003);
    
    cstring_init(CHFSNP_MGR_DB_ROOT_DIR(chfsnp_mgr), NULL_PTR);    

    CHFSNP_MGR_NP_MODEL(chfsnp_mgr) = CHFSNP_ERR_MODEL;
    CHFSNP_MGR_NP_1ST_CHASH_ALGO_ID(chfsnp_mgr) = (uint8_t)CHASH_ERR_ALGO_ID;
    CHFSNP_MGR_NP_2ND_CHASH_ALGO_ID(chfsnp_mgr) = (uint8_t)CHASH_ERR_ALGO_ID;
    CHFSNP_MGR_NP_ITEM_MAX_NUM(chfsnp_mgr)      = 0;
    CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr)           = 0;

    cvector_init(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), 0, MM_CSTRING, CVECTOR_LOCK_ENABLE, LOC_CHFSNPMGR_0004);
    cvector_init(CHFSNP_MGR_NP_VEC(chfsnp_mgr), 0, MM_CHFSNP, CVECTOR_LOCK_ENABLE, LOC_CHFSNPMGR_0005);   
    
    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_clean(CHFSNP_MGR *chfsnp_mgr)
{
    CHFSNP_MGR_CRWLOCK_CLEAN(chfsnp_mgr, LOC_CHFSNPMGR_0006);
    CHFSNP_MGR_CMUTEX_CLEAN(chfsnp_mgr, LOC_CHFSNPMGR_0007);
    
    cstring_clean(CHFSNP_MGR_DB_ROOT_DIR(chfsnp_mgr));    

    CHFSNP_MGR_NP_MODEL(chfsnp_mgr) = CHFSNP_ERR_MODEL;
    CHFSNP_MGR_NP_1ST_CHASH_ALGO_ID(chfsnp_mgr) = (uint8_t)CHASH_ERR_ALGO_ID;
    CHFSNP_MGR_NP_2ND_CHASH_ALGO_ID(chfsnp_mgr) = (uint8_t)CHASH_ERR_ALGO_ID;
    CHFSNP_MGR_NP_ITEM_MAX_NUM(chfsnp_mgr)      = 0;
    CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr)           = 0;

    cvector_clean(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), (CVECTOR_DATA_CLEANER)cstring_free, LOC_CHFSNPMGR_0008);
    cvector_clean(CHFSNP_MGR_NP_VEC(chfsnp_mgr), (CVECTOR_DATA_CLEANER)chfsnp_free, LOC_CHFSNPMGR_0009);       

    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_free(CHFSNP_MGR *chfsnp_mgr)
{
    if(NULL_PTR != chfsnp_mgr)
    {
        chfsnp_mgr_clean(chfsnp_mgr);
        free_static_mem(MD_CHFS, CMPI_ANY_MODI, MM_CHFSNP_MGR, chfsnp_mgr, LOC_CHFSNPMGR_0010);
    }
    return (EC_TRUE);
}

CHFSNP *chfsnp_mgr_open_np(CHFSNP_MGR *chfsnp_mgr, const uint32_t chfsnp_id)
{
    CHFSNP *chfsnp;

    chfsnp = CHFSNP_MGR_NP_GET_NO_LOCK(chfsnp_mgr, chfsnp_id);
    if(NULL_PTR != chfsnp)
    {
        return (chfsnp);
    }

    chfsnp = chfsnp_open((char *)CHFSNP_MGR_DB_ROOT_DIR_STR(chfsnp_mgr), chfsnp_id);
    if(NULL_PTR == chfsnp)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_open_np: open np %u from %s failed\n", 
                           chfsnp_id, (char *)CHFSNP_MGR_DB_ROOT_DIR_STR(chfsnp_mgr));
        return (NULL_PTR);
    }

    CHFSNP_MGR_NP_SET_NO_LOCK(chfsnp_mgr, chfsnp_id, chfsnp);
    return (chfsnp);
}

EC_BOOL chfsnp_mgr_close_np(CHFSNP_MGR *chfsnp_mgr, const uint32_t chfsnp_id)
{
    CHFSNP *chfsnp;

    chfsnp = CHFSNP_MGR_NP_GET_NO_LOCK(chfsnp_mgr, chfsnp_id);
    if(NULL_PTR == chfsnp)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 1)(LOGSTDOUT, "warn:chfsnp_mgr_close_np: np %u not open yet\n", chfsnp_id);
        return (EC_TRUE);
    }

    CHFSNP_MGR_NP_SET_NO_LOCK(chfsnp_mgr, chfsnp_id, NULL_PTR);
    chfsnp_close(chfsnp);
    return (EC_TRUE);
}

static char *__chfsnp_mgr_gen_db_name(const char *root_dir)
{
    const char *fields[ 2 ];
    
    fields[ 0 ] = root_dir;
    fields[ 1 ] = CHFSNP_DB_NAME;
    
    return c_str_join((char *)"/", fields, 2);
}

static EC_BOOL __chfsnp_mgr_load_db(CHFSNP_MGR *chfsnp_mgr, int chfsnp_mgr_fd)
{
    UINT32 chfsnp_mgr_db_size;
    UINT8* chfsnp_mgr_db_buff;
    UINT32 chfsnp_mgr_db_offset;
    UINT32 chfsnp_home_dir_num;
    UINT32 chfsnp_home_dir_pos;

    uint32_t chfsnp_id;
    
    /*init offset*/
    chfsnp_mgr_db_offset = 0;

    /*CHFSNP_MGR_NP_MODEL*/
    chfsnp_mgr_db_size   = sizeof(uint8_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_MODEL(chfsnp_mgr));    
    if(EC_FALSE == c_file_load(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_load_db: load np model failed\n");
        return (EC_FALSE);
    }

    /*CHFSNP_MGR_NP_1ST_CHASH_ALGO_ID*/
    chfsnp_mgr_db_size   = sizeof(uint8_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_1ST_CHASH_ALGO_ID(chfsnp_mgr));    
    if(EC_FALSE == c_file_load(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_load_db: load 1st chash algo id failed\n");
        return (EC_FALSE);
    }    

    /*CHFSNP_MGR_NP_2ND_CHASH_ALGO_ID*/
    chfsnp_mgr_db_size   = sizeof(uint8_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_2ND_CHASH_ALGO_ID(chfsnp_mgr));    
    if(EC_FALSE == c_file_load(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_load_db: load 2nd chash algo id failed\n");
        return (EC_FALSE);
    }     

    /*CHFSNP_MGR_NP_ITEM_MAX_NUM*/
    chfsnp_mgr_db_size   = sizeof(uint32_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_ITEM_MAX_NUM(chfsnp_mgr));    
    if(EC_FALSE == c_file_load(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_load_db: load item max num failed\n");
        return (EC_FALSE);
    }     

    /*CHFSNP_MGR_NP_MAX_NUM*/
    chfsnp_mgr_db_size   = sizeof(uint32_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr));    
    if(EC_FALSE == c_file_load(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_load_db: load disk max num failed\n");
        return (EC_FALSE);
    }

    for(chfsnp_id = cvector_size(CHFSNP_MGR_NP_VEC(chfsnp_mgr)); chfsnp_id < CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr); chfsnp_id ++)
    {
        cvector_push_no_lock(CHFSNP_MGR_NP_VEC(chfsnp_mgr), NULL_PTR);
    }

    /*CHFSNP_MGR_NP_HOME_DIR_VEC*/
    chfsnp_mgr_db_size   = sizeof(UINT32);
    chfsnp_mgr_db_buff   = (UINT8 *)&(chfsnp_home_dir_num);    
    if(EC_FALSE == c_file_load(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_load_db: load home dir vec size failed\n");
        return (EC_FALSE);
    }    

    for(chfsnp_home_dir_pos = 0; chfsnp_home_dir_pos < chfsnp_home_dir_num; chfsnp_home_dir_pos ++)
    {
        CSTRING *chfsnp_home_dir_cstr;

        chfsnp_home_dir_cstr = cstring_new(NULL_PTR, LOC_CHFSNPMGR_0011);
        if(NULL_PTR == chfsnp_home_dir_cstr)
        {
            dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_load_db: new home dir %u # failed\n", chfsnp_home_dir_pos);
            return (EC_FALSE);
        }
                
        if(EC_FALSE == cstring_load(chfsnp_home_dir_cstr, chfsnp_mgr_fd, &chfsnp_mgr_db_offset))
        {
            dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_load_db: load home dir %u # failed\n", chfsnp_home_dir_pos);
            cstring_free(chfsnp_home_dir_cstr);
            return (EC_FALSE);
        }

        if(EC_TRUE == cstring_is_empty(chfsnp_home_dir_cstr))
        {
            cstring_free(chfsnp_home_dir_cstr);
            cvector_push_no_lock(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), NULL_PTR);
        }
        else
        {
            cvector_push_no_lock(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), (void *)chfsnp_home_dir_cstr);
        }
    }

    return (EC_TRUE);
}

static EC_BOOL __chfsnp_mgr_flush_db(CHFSNP_MGR *chfsnp_mgr, int chfsnp_mgr_fd)
{
    UINT32 chfsnp_mgr_db_size;
    UINT8* chfsnp_mgr_db_buff;
    UINT32 chfsnp_mgr_db_offset;
    UINT32 chfsnp_home_dir_num;
    UINT32 chfsnp_home_dir_pos;

    /*init offset*/
    chfsnp_mgr_db_offset = 0;

    /*CHFSNP_MGR_NP_MODEL*/
    chfsnp_mgr_db_size   = sizeof(uint8_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_MODEL(chfsnp_mgr));    
    if(EC_FALSE == c_file_flush(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_flush_db: flush np model failed");
        return (EC_FALSE);
    }

    /*CHFSNP_MGR_NP_1ST_CHASH_ALGO_ID*/
    chfsnp_mgr_db_size   = sizeof(uint8_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_1ST_CHASH_ALGO_ID(chfsnp_mgr));    
    if(EC_FALSE == c_file_flush(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_flush_db: flush 1st chash algo id failed");
        return (EC_FALSE);
    }    

    /*CHFSNP_MGR_NP_2ND_CHASH_ALGO_ID*/
    chfsnp_mgr_db_size   = sizeof(uint8_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_2ND_CHASH_ALGO_ID(chfsnp_mgr));    
    if(EC_FALSE == c_file_flush(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_flush_db: flush 2nd chash algo id failed");
        return (EC_FALSE);
    }     

    /*CHFSNP_MGR_NP_ITEM_MAX_NUM*/
    chfsnp_mgr_db_size   = sizeof(uint32_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_ITEM_MAX_NUM(chfsnp_mgr));    
    if(EC_FALSE == c_file_flush(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_flush_db: flush item max num failed");
        return (EC_FALSE);
    }     

    /*CHFSNP_MGR_NP_MAX_NUM*/
    chfsnp_mgr_db_size   = sizeof(uint32_t);
    chfsnp_mgr_db_buff   = (UINT8 *)&(CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr));    
    if(EC_FALSE == c_file_flush(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_flush_db: flush disk max num failed");
        return (EC_FALSE);
    }

    /*CHFSNP_MGR_NP_HOME_DIR_VEC*/
    chfsnp_home_dir_num  = cvector_size(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr));
    chfsnp_mgr_db_size   = sizeof(UINT32);
    chfsnp_mgr_db_buff   = (UINT8 *)&(chfsnp_home_dir_num);    
    if(EC_FALSE == c_file_flush(chfsnp_mgr_fd, &chfsnp_mgr_db_offset, chfsnp_mgr_db_size, chfsnp_mgr_db_buff))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_flush_db: flush home dir vec size failed");
        return (EC_FALSE);
    }    

    dbg_log(SEC_0065_CHFSNPMGR, 9)(LOGSTDOUT, "[DEBUG] __chfsnp_mgr_flush_db: np max num = %u\n", CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr));
    dbg_log(SEC_0065_CHFSNPMGR, 9)(LOGSTDOUT, "[DEBUG] __chfsnp_mgr_flush_db: np home dir vec size = %u\n", cvector_size(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr)));

    for(chfsnp_home_dir_pos = 0; chfsnp_home_dir_pos < chfsnp_home_dir_num; chfsnp_home_dir_pos ++)
    {
        CSTRING *chfsnp_home_dir_cstr;

        chfsnp_home_dir_cstr = cvector_get_no_lock(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), chfsnp_home_dir_pos);
        if(EC_FALSE == cstring_flush(chfsnp_home_dir_cstr, chfsnp_mgr_fd, &chfsnp_mgr_db_offset))
        {
            dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_flush_db: flush home dir %u # failed\n", chfsnp_home_dir_pos);
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_load_db(CHFSNP_MGR *chfsnp_mgr)
{
    char  *chfsnp_mgr_db_name;
    int    chfsnp_mgr_fd;

    chfsnp_mgr_db_name = __chfsnp_mgr_gen_db_name((char *)CHFSNP_MGR_DB_ROOT_DIR_STR(chfsnp_mgr));
    if(NULL_PTR == chfsnp_mgr_db_name)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_load_db: new str %s/%s failed\n", 
                            (char *)CHFSNP_MGR_DB_ROOT_DIR_STR(chfsnp_mgr), CHFSNP_DB_NAME);
        return (EC_FALSE);
    }

    if(EC_FALSE == c_file_access(chfsnp_mgr_db_name, F_OK))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_load_db: chfsnp mgr db %s not exist\n", chfsnp_mgr_db_name);
        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0012);
        return (EC_FALSE);
    }

    chfsnp_mgr_fd = c_file_open(chfsnp_mgr_db_name, O_RDONLY, 0666);
    if(ERR_FD == chfsnp_mgr_fd)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_load_db: open chfsnp mgr db %s failed\n", chfsnp_mgr_db_name);
        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0013);
        return (EC_FALSE);
    }

    if(EC_FALSE == __chfsnp_mgr_load_db(chfsnp_mgr, chfsnp_mgr_fd))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_load_db: load db from chfsnp mgr db %s\n", chfsnp_mgr_db_name);
        c_file_close(chfsnp_mgr_fd);
        chfsnp_mgr_fd = ERR_FD;

        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0014);
        return (EC_FALSE);
    }

    c_file_close(chfsnp_mgr_fd);
    chfsnp_mgr_fd = ERR_FD;

    dbg_log(SEC_0065_CHFSNPMGR, 9)(LOGSTDOUT, "[DEBUG] chfsnp_mgr_load_db: load db from chfsnp mgr db %s done\n", chfsnp_mgr_db_name);

    safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0015);
    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_create_db(CHFSNP_MGR *chfsnp_mgr, const CSTRING *chfsnp_db_root_dir)
{
    char  *chfsnp_mgr_db_name;
    int    chfsnp_mgr_fd;

    chfsnp_mgr_db_name = __chfsnp_mgr_gen_db_name((char *)cstring_get_str(chfsnp_db_root_dir));
    if(NULL_PTR == chfsnp_mgr_db_name)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_create_db: new str %s/%s failed\n", 
                            (char *)cstring_get_str(chfsnp_db_root_dir), CHFSNP_DB_NAME);
        return (EC_FALSE);
    }

    if(EC_TRUE == c_file_access(chfsnp_mgr_db_name, F_OK))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_create_db: chfsnp mgr db %s already exist\n", chfsnp_mgr_db_name);
        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0016);
        return (EC_FALSE);
    }

    chfsnp_mgr_fd = c_file_open(chfsnp_mgr_db_name, O_RDWR | O_CREAT, 0666);
    if(ERR_FD == chfsnp_mgr_fd)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_create_db: open chfsnp mgr db %s failed\n", chfsnp_mgr_db_name);
        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0017);
        return (EC_FALSE);
    }

    if(EC_FALSE == __chfsnp_mgr_flush_db(chfsnp_mgr, chfsnp_mgr_fd))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_create_db: flush db to chfsnp mgr db %s\n", chfsnp_mgr_db_name);
        c_file_close(chfsnp_mgr_fd);
        chfsnp_mgr_fd = ERR_FD;

        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0018);
        return (EC_FALSE);
    }    

    c_file_close(chfsnp_mgr_fd);
    chfsnp_mgr_fd = ERR_FD;

    safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0019);
    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_flush_db(CHFSNP_MGR *chfsnp_mgr)
{
    char  *chfsnp_mgr_db_name;
    int    chfsnp_mgr_fd;

    chfsnp_mgr_db_name = __chfsnp_mgr_gen_db_name((char *)CHFSNP_MGR_DB_ROOT_DIR_STR(chfsnp_mgr));
    if(NULL_PTR == chfsnp_mgr_db_name)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_flush_db: new str %s/%s failed\n", 
                            (char *)CHFSNP_MGR_DB_ROOT_DIR_STR(chfsnp_mgr), CHFSNP_DB_NAME);
        return (EC_FALSE);
    }

    if(EC_FALSE == c_file_access(chfsnp_mgr_db_name, F_OK))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_flush_db: chfsnp mgr db %s not exist\n", chfsnp_mgr_db_name);
        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0020);
        return (EC_FALSE);
    }

    chfsnp_mgr_fd = c_file_open(chfsnp_mgr_db_name, O_RDWR, 0666);
    if(ERR_FD == chfsnp_mgr_fd)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_flush_db: open chfsnp mgr db %s failed\n", chfsnp_mgr_db_name);
        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0021);
        return (EC_FALSE);
    }

    if(EC_FALSE == __chfsnp_mgr_flush_db(chfsnp_mgr, chfsnp_mgr_fd))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_flush_db: flush db to chfsnp mgr db %s\n", chfsnp_mgr_db_name);
        c_file_close(chfsnp_mgr_fd);
        chfsnp_mgr_fd = ERR_FD;

        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0022);
        return (EC_FALSE);
    }

    c_file_close(chfsnp_mgr_fd);
    chfsnp_mgr_fd = ERR_FD;

    safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0023);
    return (EC_TRUE);
}

void chfsnp_mgr_print_db(LOG *log, const CHFSNP_MGR *chfsnp_mgr)
{
    uint32_t chfsnp_home_dir_num;
    uint32_t chfsnp_home_dir_pos;
    uint32_t chfsnp_num;
    uint32_t chfsnp_id;

    sys_log(log, "chfsnp mgr db root dir  : %s\n", (char *)CHFSNP_MGR_DB_ROOT_DIR_STR(chfsnp_mgr));
    sys_log(log, "chfsnp model            : %u\n", CHFSNP_MGR_NP_MODEL(chfsnp_mgr));
    sys_log(log, "chfsnp 1st hash algo id : %u\n", CHFSNP_MGR_NP_1ST_CHASH_ALGO_ID(chfsnp_mgr));
    sys_log(log, "chfsnp 2nd hash algo id : %u\n", CHFSNP_MGR_NP_2ND_CHASH_ALGO_ID(chfsnp_mgr));
    sys_log(log, "chfsnp item max num     : %u\n", CHFSNP_MGR_NP_ITEM_MAX_NUM(chfsnp_mgr));
    sys_log(log, "chfsnp max num          : %u\n", CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr));

    chfsnp_home_dir_num = (uint32_t)cvector_size(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr));
    for(chfsnp_home_dir_pos = 0; chfsnp_home_dir_pos < chfsnp_home_dir_num; chfsnp_home_dir_pos ++)
    {
        CSTRING *chfsnp_home_dir_cstr;

        chfsnp_home_dir_cstr = cvector_get_no_lock(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), chfsnp_home_dir_pos);
        if(NULL_PTR == chfsnp_home_dir_cstr || EC_TRUE == cstring_is_empty(chfsnp_home_dir_cstr))
        {
            sys_log(log, "home dir %u #: (null)\n", chfsnp_home_dir_pos);
        }
        else
        {
            sys_log(log, "home dir %u #: %.*s\n", chfsnp_home_dir_pos,
                        cstring_get_len(chfsnp_home_dir_cstr), (char *)cstring_get_str(chfsnp_home_dir_cstr));
        }
    }

    chfsnp_num = (uint32_t)cvector_size(CHFSNP_MGR_NP_VEC(chfsnp_mgr));
    for(chfsnp_id = 0; chfsnp_id < chfsnp_num; chfsnp_id ++)
    {
        CHFSNP *chfsnp;

        chfsnp = CHFSNP_MGR_NP(chfsnp_mgr, chfsnp_id);
        if(NULL_PTR == chfsnp)
        {
            sys_log(log, "np %u #: (null)\n", chfsnp_id);
        }
        else
        {
            chfsnp_print(log, chfsnp);
        }
    }
    return;
}

void chfsnp_mgr_print(LOG *log, const CHFSNP_MGR *chfsnp_mgr)
{
    sys_log(log, "chfsnp mgr:\n");
    chfsnp_mgr_print_db(log, chfsnp_mgr);
    return;
}

EC_BOOL chfsnp_mgr_load(CHFSNP_MGR *chfsnp_mgr, const CSTRING *chfsnp_db_root_dir)
{
    cstring_clean(CHFSNP_MGR_DB_ROOT_DIR(chfsnp_mgr));
    cstring_clone(chfsnp_db_root_dir, CHFSNP_MGR_DB_ROOT_DIR(chfsnp_mgr));

    if(EC_FALSE == chfsnp_mgr_load_db(chfsnp_mgr))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_load: load cfg db failed from dir %s\n", (char *)cstring_get_str(chfsnp_db_root_dir));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_sync_np(CHFSNP_MGR *chfsnp_mgr, const uint32_t chfsnp_id)
{
    CHFSNP *chfsnp;
    
    chfsnp = CHFSNP_MGR_NP_GET_NO_LOCK(chfsnp_mgr, chfsnp_id);
    if(NULL_PTR != chfsnp)
    {
        return chfsnp_sync(chfsnp);
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_flush(CHFSNP_MGR *chfsnp_mgr)
{
    uint32_t chfsnp_num;
    uint32_t chfsnp_id;
    EC_BOOL ret;

    ret = EC_TRUE;

    if(EC_FALSE == chfsnp_mgr_flush_db(chfsnp_mgr))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_flush: flush cfg db failed\n");
        ret = EC_FALSE;
    }

    chfsnp_num = CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr);
    for(chfsnp_id = 0; chfsnp_id < chfsnp_num; chfsnp_id ++)
    {
        chfsnp_mgr_sync_np(chfsnp_mgr, chfsnp_id);
    }
    return (ret);
}

EC_BOOL chfsnp_mgr_show_np(LOG *log, CHFSNP_MGR *chfsnp_mgr, const uint32_t chfsnp_id)
{
    CHFSNP *chfsnp;

    chfsnp = CHFSNP_MGR_NP_GET_NO_LOCK(chfsnp_mgr, chfsnp_id);
    if(NULL_PTR == chfsnp)
    {
        CSTRING *home_dir;
        
        /*try to open the np and print it*/
        chfsnp = chfsnp_mgr_open_np(chfsnp_mgr, chfsnp_id);
        if(NULL_PTR == chfsnp)
        {
            dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_show_np: open np %u failed\n", chfsnp_id);
            return (EC_FALSE);
        } 

        home_dir = CHFSNP_MGR_NP_HOME_DIR(chfsnp_mgr, chfsnp_id);
        if(NULL_PTR == home_dir)
        {
            sys_log(log, "home dir: (null)\n");
        }
        else
        {
            sys_log(log, "home dir: %s\n", (char *)cstring_get_str(home_dir));
        }        

        chfsnp_print(log, chfsnp);

        chfsnp_mgr_close_np(chfsnp_mgr, chfsnp_id);
    }
    else
    {
        CSTRING *home_dir;
        
        home_dir = CHFSNP_MGR_NP_HOME_DIR(chfsnp_mgr, chfsnp_id);
        if(NULL_PTR == home_dir)
        {
            sys_log(log, "home dir: (null)\n");
        }
        else
        {
            sys_log(log, "home dir: %s\n", (char *)cstring_get_str(home_dir));
        }  
        
        chfsnp_print(log, chfsnp);
    }

    return (EC_TRUE);
}

static uint32_t __chfsnp_mgr_get_np_id_of_path(const CHFSNP_MGR *chfsnp_mgr, const uint32_t path_len, const uint8_t *path)
{
    uint32_t chfsnp_num;
    uint32_t chfsnp_id;

    ASSERT(CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr) == cvector_size(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr)));

    CHFSNP_MGR_NP_HOME_DIR_VEC_LOCK(chfsnp_mgr, LOC_CHFSNPMGR_0024);
    chfsnp_num = CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr);
    for(chfsnp_id = 0; chfsnp_id < chfsnp_num; chfsnp_id ++)
    {
        CSTRING *chfsnp_home_dir_cstr;
        uint32_t chfsnp_home_dir_len;
        uint8_t *chfsnp_home_dir_str;

        chfsnp_home_dir_cstr = CHFSNP_MGR_NP_HOME_DIR(chfsnp_mgr, chfsnp_id);        
        if(NULL_PTR == chfsnp_home_dir_cstr || EC_TRUE == cstring_is_empty(chfsnp_home_dir_cstr))
        {
            continue;
        }

        chfsnp_home_dir_len = (uint32_t)cstring_get_len(chfsnp_home_dir_cstr);
        chfsnp_home_dir_str = cstring_get_str(chfsnp_home_dir_cstr);

        dbg_log(SEC_0065_CHFSNPMGR, 9)(LOGSTDOUT, "[DEBUG] __chfsnp_mgr_get_np_id_of_path: %.*s vs %s\n", path_len, (char *)path, (char *)cstring_get_str(chfsnp_home_dir_cstr));
        dbg_log(SEC_0065_CHFSNPMGR, 9)(LOGSTDOUT, "[DEBUG] __chfsnp_mgr_get_np_id_of_path: path_len %u, chfsnp_home_dir_len %u\n", path_len, chfsnp_home_dir_len);
       
        if(path_len < chfsnp_home_dir_len)
        {
            continue;
        }

        /*now path_len >= chfsnp_home_dir_len*/
        if(path_len != chfsnp_home_dir_len && '/' != path[ chfsnp_home_dir_len ])
        {
            continue;
        }

        if(0 == BCMP(path, chfsnp_home_dir_str, chfsnp_home_dir_len))
        {
            CHFSNP_MGR_NP_HOME_DIR_VEC_UNLOCK(chfsnp_mgr, LOC_CHFSNPMGR_0025);
            return (chfsnp_id);
        }        
    }
    CHFSNP_MGR_NP_HOME_DIR_VEC_UNLOCK(chfsnp_mgr, LOC_CHFSNPMGR_0026);
    return (CHFSNP_ERR_ID);
}

static CHFSNP *__chfsnp_mgr_get_np(CHFSNP_MGR *chfsnp_mgr, const uint32_t path_len, const uint8_t *path, uint32_t *np_id)
{
    CHFSNP  * chfsnp;
    uint32_t  chfsnp_id;
    
    chfsnp_id = __chfsnp_mgr_get_np_id_of_path(chfsnp_mgr, path_len, path);
    if(CHFSNP_ERR_ID == chfsnp_id)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_get_np: no np for path %.*s\n", path_len, (char *)path);
        return (NULL_PTR);
    }

    CHFSNP_MGR_CMUTEX_LOCK(chfsnp_mgr, LOC_CHFSNPMGR_0027);
    chfsnp = chfsnp_mgr_open_np(chfsnp_mgr, chfsnp_id);
    if(NULL_PTR == chfsnp)
    {
        CHFSNP_MGR_CMUTEX_UNLOCK(chfsnp_mgr, LOC_CHFSNPMGR_0028);
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:__chfsnp_mgr_get_np: path %.*s in np %u but cannot open\n", path_len, path, chfsnp_id);
        return (NULL_PTR);
    }
    CHFSNP_MGR_CMUTEX_UNLOCK(chfsnp_mgr, LOC_CHFSNPMGR_0029);

    if(NULL_PTR != np_id)
    {
        (*np_id) = chfsnp_id;
    }
    
    return (chfsnp);           
}

EC_BOOL chfsnp_mgr_search(CHFSNP_MGR *chfsnp_mgr, const uint32_t path_len, const uint8_t *path, uint32_t *searched_chfsnp_id)
{
    CHFSNP   *chfsnp;
    uint32_t  chfsnp_id;    
    uint32_t  node_pos;
    
    chfsnp = __chfsnp_mgr_get_np(chfsnp_mgr, path_len, path, &chfsnp_id);
    if(NULL_PTR == chfsnp)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_search: path %.*s in np %u but cannot open\n", path_len, path, chfsnp_id);
        return (EC_FALSE);
    }

    node_pos = chfsnp_search(chfsnp, path_len, path);
    if(CHFSNPRB_ERR_POS == node_pos)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 9)(LOGSTDOUT, "[DEBUG] chfsnp_mgr_search: path %.*s in np %u but not found indeed\n", path_len, path, chfsnp_id);
        return (EC_FALSE);
    }

    if(NULL_PTR != searched_chfsnp_id)
    {
        (*searched_chfsnp_id) = chfsnp_id;
    }

    return (EC_TRUE);
}

CHFSNP_ITEM *chfsnp_mgr_search_item(CHFSNP_MGR *chfsnp_mgr, const uint32_t path_len, const uint8_t *path)
{
    CHFSNP   *chfsnp;
    uint32_t  chfsnp_id;    
    uint32_t  node_pos;
    
    chfsnp = __chfsnp_mgr_get_np(chfsnp_mgr, path_len, path, &chfsnp_id);
    if(NULL_PTR == chfsnp)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_search_item: path %.*s in np %u but cannot open\n", path_len, path, chfsnp_id);
        return (NULL_PTR);
    }

    node_pos = chfsnp_search(chfsnp, path_len, path);
    if(CHFSNPRB_ERR_POS == node_pos)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 9)(LOGSTDOUT, "[DEBUG] chfsnp_mgr_search_item: path %.*s in np %u but not found indeed\n", path_len, path, chfsnp_id);
        return (NULL_PTR);
    }

    return chfsnp_fetch(chfsnp, node_pos);
}

CHFSNP_MGR *chfsnp_mgr_create(const uint8_t chfsnp_model, 
                                const uint32_t chfsnp_max_num, 
                                const uint8_t  chfsnp_1st_chash_algo_id, 
                                const uint8_t  chfsnp_2nd_chash_algo_id, 
                                const uint32_t chfsnp_bucket_max_num,
                                const CSTRING *chfsnp_db_root_dir)
{
    CHFSNP_MGR *chfsnp_mgr;
    uint32_t chfsnp_item_max_num;
    uint32_t chfsnp_id;
    
    if(EC_FALSE == chfsnp_model_item_max_num(chfsnp_model , &chfsnp_item_max_num))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_create: invalid chfsnp model %u\n", chfsnp_model);
        return (NULL_PTR);
    }

    chfsnp_mgr = chfsnp_mgr_new();

    CHFSNP_MGR_NP_MODEL(chfsnp_mgr)                = chfsnp_model;
    CHFSNP_MGR_NP_1ST_CHASH_ALGO_ID(chfsnp_mgr)    = chfsnp_1st_chash_algo_id;
    CHFSNP_MGR_NP_2ND_CHASH_ALGO_ID(chfsnp_mgr)    = chfsnp_2nd_chash_algo_id;
    CHFSNP_MGR_NP_ITEM_MAX_NUM(chfsnp_mgr)         = chfsnp_item_max_num;
    CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr)              = chfsnp_max_num;

    cstring_clone(chfsnp_db_root_dir, CHFSNP_MGR_DB_ROOT_DIR(chfsnp_mgr));

    for(chfsnp_id = 0; chfsnp_id < chfsnp_max_num; chfsnp_id ++)
    {
        const char *np_root_dir;
        CHFSNP *chfsnp;

        np_root_dir = (const char *)cstring_get_str(chfsnp_db_root_dir);/*Oops! int the same dire*/
        chfsnp = chfsnp_create(np_root_dir, chfsnp_id, chfsnp_model, chfsnp_1st_chash_algo_id, chfsnp_2nd_chash_algo_id, chfsnp_bucket_max_num);
        if(NULL_PTR == chfsnp)
        {
            dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_create: create np %u failed\n", chfsnp_id);
            return (NULL_PTR);
        }
        chfsnp_close(chfsnp);
        
        cvector_push_no_lock(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), (void *)NULL_PTR);
        cvector_push_no_lock(CHFSNP_MGR_NP_VEC(chfsnp_mgr), (void *)NULL_PTR);
    }

    if(EC_FALSE == chfsnp_mgr_create_db(chfsnp_mgr, chfsnp_db_root_dir))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_create: create cfg db failed in root dir %s\n",
                            (char *)cstring_get_str(chfsnp_db_root_dir));
        chfsnp_mgr_free(chfsnp_mgr);
        return (NULL_PTR);
    }

    //chfsnp_mgr_free(chfsnp_mgr);
    return (chfsnp_mgr);
}

EC_BOOL chfsnp_mgr_exist(const CSTRING *chfsnp_db_root_dir)
{
    char  *chfsnp_mgr_db_name;

    chfsnp_mgr_db_name = __chfsnp_mgr_gen_db_name((char *)cstring_get_str(chfsnp_db_root_dir));
    if(NULL_PTR == chfsnp_mgr_db_name)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_exist: new str %s/%s failed\n", 
                            (char *)cstring_get_str(chfsnp_db_root_dir), CHFSNP_DB_NAME);
        return (EC_FALSE);
    }

    if(EC_FALSE == c_file_access(chfsnp_mgr_db_name, F_OK))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_exist: chfsnp mgr db %s not exist\n", chfsnp_mgr_db_name);
        safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0030);
        return (EC_FALSE);
    }
    safe_free(chfsnp_mgr_db_name, LOC_CHFSNPMGR_0031);
    return (EC_TRUE);
}

CHFSNP_MGR * chfsnp_mgr_open(const CSTRING *chfsnp_db_root_dir)
{
    CHFSNP_MGR *chfsnp_mgr;

    chfsnp_mgr = chfsnp_mgr_new();
    if(NULL_PTR == chfsnp_mgr)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_open: new chfsnp mgr failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == chfsnp_mgr_load(chfsnp_mgr, chfsnp_db_root_dir))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_open: load failed\n");
        chfsnp_mgr_free(chfsnp_mgr);
        return (NULL_PTR);
    }
    return (chfsnp_mgr);
}

EC_BOOL chfsnp_mgr_close(CHFSNP_MGR *chfsnp_mgr)
{    
    if(NULL_PTR != chfsnp_mgr)
    {
        CHFSNP_MGR_CMUTEX_LOCK(chfsnp_mgr, LOC_CHFSNPMGR_0032);
        chfsnp_mgr_flush(chfsnp_mgr);
        CHFSNP_MGR_CMUTEX_UNLOCK(chfsnp_mgr, LOC_CHFSNPMGR_0033);
        chfsnp_mgr_free(chfsnp_mgr);
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_find(CHFSNP_MGR *chfsnp_mgr, const CSTRING *path)
{
    return chfsnp_mgr_search(chfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), NULL_PTR);
}

/*bind home_dir and name node, i.e., one name node owns unique home dir*/
EC_BOOL chfsnp_mgr_bind(CHFSNP_MGR *chfsnp_mgr, const CSTRING *path, const UINT32 chfsnp_id)
{
    CSTRING *home_dir;
    CSTRING *home_dir_old;
    uint32_t home_dir_pos;

    ASSERT(cvector_size(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr)) == CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr));
    if(CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr) <= chfsnp_id)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_bind: max np num %u but chfsnp id %u overflow\n", CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr), chfsnp_id);
        return (EC_FALSE);
    }

    home_dir_pos = __chfsnp_mgr_get_np_id_of_path(chfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path));
    if(CHFSNP_ERR_ID != home_dir_pos)
    {
        home_dir = cvector_get_no_lock(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), home_dir_pos);
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_bind: some dir %s already bound to np %u, thus cannot accept binding %s\n", 
                            (char *)cstring_get_str(home_dir), home_dir_pos, (char *)cstring_get_str(path));
        return (EC_FALSE);
    }    

    home_dir = cstring_dup(path);
    if(NULL_PTR == home_dir)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_bind: dup %s failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    home_dir_old = (CSTRING *)cvector_set(CHFSNP_MGR_NP_HOME_DIR_VEC(chfsnp_mgr), chfsnp_id, (void *)home_dir);
    if(NULL_PTR != home_dir_old)
    {
        cstring_free(home_dir_old);
    }
    return (EC_TRUE);
}


EC_BOOL chfsnp_mgr_write(CHFSNP_MGR *chfsnp_mgr, const CSTRING *file_path, const CHFSNP_FNODE *chfsnp_fnode)
{
    CHFSNP *chfsnp;
    CHFSNP_ITEM *chfsnp_item;
    uint32_t chfsnp_id;

    chfsnp = __chfsnp_mgr_get_np(chfsnp_mgr, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), &chfsnp_id);
    if(NULL_PTR == chfsnp)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_write: no np for path %s\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    chfsnp_item = chfsnp_set(chfsnp, cstring_get_len(file_path), cstring_get_str(file_path));
    if(NULL_PTR == chfsnp_item)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_write: set file %s to np %u failed\n",
                            (char *)cstring_get_str(file_path), chfsnp_id);
        return (EC_FALSE);
    }
    
    if(EC_FALSE == chfsnp_fnode_import(chfsnp_fnode, CHFSNP_ITEM_FNODE(chfsnp_item)))
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_write: import fnode to item failed where path %s\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    dbg_log(SEC_0065_CHFSNPMGR, 9)(LOGSTDOUT, "[DEBUG] chfsnp_mgr_write: import fnode to item successfully where path %s\n", (char *)cstring_get_str(file_path));
    chfsnp_item_print(LOGSTDOUT, chfsnp_item);
    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_read(CHFSNP_MGR *chfsnp_mgr, const CSTRING *file_path, CHFSNP_FNODE *chfsnp_fnode)
{  
    CHFSNP *chfsnp;
    uint32_t chfsnp_id;
    uint32_t node_pos;

    chfsnp = __chfsnp_mgr_get_np(chfsnp_mgr, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), &chfsnp_id);
    if(NULL_PTR == chfsnp)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_read: no np for path %s\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    node_pos = chfsnp_search_no_lock(chfsnp, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path));
    if(CHFSNPRB_ERR_POS != node_pos)
    {
        CHFSNP_ITEM *chfsnp_item;

        chfsnp_item = chfsnp_fetch(chfsnp, node_pos);
        return chfsnp_fnode_import(CHFSNP_ITEM_FNODE(chfsnp_item), chfsnp_fnode);
    }
    
    dbg_log(SEC_0065_CHFSNPMGR, 9)(LOGSTDOUT, "[DEBUG] chfsnp_mgr_read: search nothing for path '%s'\n", (char *)cstring_get_str(file_path));
    return (EC_FALSE);    
}

EC_BOOL chfsnp_mgr_delete(CHFSNP_MGR *chfsnp_mgr, const CSTRING *path, CVECTOR *chfsnp_fnode_vec)
{
    CHFSNP *chfsnp;
    uint32_t chfsnp_id;

    chfsnp = __chfsnp_mgr_get_np(chfsnp_mgr, (uint32_t)cstring_get_len(path), cstring_get_str(path), &chfsnp_id);
    if(NULL_PTR == chfsnp)
    {
        dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_delete: no np for path %s\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }

    return chfsnp_delete(chfsnp, (uint32_t)cstring_get_len(path), cstring_get_str(path), chfsnp_fnode_vec);
}

EC_BOOL chfsnp_mgr_file_num(CHFSNP_MGR *chfsnp_mgr, UINT32 *file_num)
{
    uint32_t chfsnp_id;

    (*file_num) = 0;

    for(chfsnp_id = 0; chfsnp_id < CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr); chfsnp_id ++)
    {
        CHFSNP*chfsnp;
        
        chfsnp = chfsnp_mgr_open_np(chfsnp_mgr, chfsnp_id);
        if(NULL_PTR == chfsnp)
        {
            dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_file_num: open np %u failed\n", chfsnp_id);
            return (EC_FALSE);
        }
        (*file_num) += chfsnp_count_file_num(chfsnp);
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_file_size(CHFSNP_MGR *chfsnp_mgr, UINT32 *file_size)
{
    uint32_t chfsnp_id;

    (*file_size) = 0;

    for(chfsnp_id = 0; chfsnp_id < CHFSNP_MGR_NP_MAX_NUM(chfsnp_mgr); chfsnp_id ++)
    {
        CHFSNP*chfsnp;
        
        chfsnp = chfsnp_mgr_open_np(chfsnp_mgr, chfsnp_id);
        if(NULL_PTR == chfsnp)
        {
            dbg_log(SEC_0065_CHFSNPMGR, 0)(LOGSTDOUT, "error:chfsnp_mgr_file_size: open np %u failed\n", chfsnp_id);
            return (EC_FALSE);
        }
        chfsnp_count_file_size(chfsnp, file_size);
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_show_cached_np(LOG *log, const CHFSNP_MGR *chfsnp_mgr)
{
    uint32_t chfsnp_num;
    uint32_t chfsnp_pos;

    chfsnp_num = cvector_size(CHFSNP_MGR_NP_VEC(chfsnp_mgr));
    for(chfsnp_pos = 0; chfsnp_pos < chfsnp_num; chfsnp_pos ++)
    {
        CHFSNP *chfsnp;

        chfsnp = CHFSNP_MGR_NP(chfsnp_mgr, chfsnp_pos);
        if(NULL_PTR != chfsnp)
        {
            CSTRING *home_dir;
            home_dir = CHFSNP_MGR_NP_HOME_DIR(chfsnp_mgr, chfsnp_pos);
            if(NULL_PTR == home_dir)
            {
                sys_log(log, "home dir: (null)\n");
            }
            else
            {
                sys_log(log, "home dir: %s\n", (char *)cstring_get_str(home_dir));
            }
            chfsnp_print(log, chfsnp);
        }
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_mgr_rdlock(CHFSNP_MGR *chfsnp_mgr, const UINT32 location)
{
    return CHFSNP_MGR_CRWLOCK_RDLOCK(chfsnp_mgr, location);
}

EC_BOOL chfsnp_mgr_wrlock(CHFSNP_MGR *chfsnp_mgr, const UINT32 location)
{
    return CHFSNP_MGR_CRWLOCK_WRLOCK(chfsnp_mgr, location);
}

EC_BOOL chfsnp_mgr_unlock(CHFSNP_MGR *chfsnp_mgr, const UINT32 location)
{
    return CHFSNP_MGR_CRWLOCK_UNLOCK(chfsnp_mgr, location);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

