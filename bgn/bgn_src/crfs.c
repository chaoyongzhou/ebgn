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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <sys/mman.h>

#include <sys/stat.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "carray.h"
#include "cvector.h"

#include "cbc.h"
#include "ctimer.h"
#include "cbtimer.h"
#include "cmisc.h"

#include "task.h"

#include "csocket.h"

#include "cmpie.h"

#include "crfs.h"
#include "crfshttp.h"
#include "crfsmc.h"
#include "crfsbk.h"

#include "cload.h"

#include "ccurl.h"

#include "cfuse.h"
#include "cmd5.h"
#include "crfsdt.h"
#include "crfsc.h"

#include "findex.inc"

#define CRFS_MD_CAPACITY()                  (cbc_md_capacity(MD_CRFS))

#define CRFS_MD_GET(crfs_md_id)     ((CRFS_MD *)cbc_md_get(MD_CRFS, (crfs_md_id)))

#define CRFS_MD_ID_CHECK_INVALID(crfs_md_id)  \
    ((CMPI_ANY_MODI != (crfs_md_id)) && ((NULL_PTR == CRFS_MD_GET(crfs_md_id)) || (0 == (CRFS_MD_GET(crfs_md_id)->usedcounter))))

static CRFSNP_FNODE * __crfs_reserve_npp(const UINT32 crfs_md_id, const CSTRING *file_path, const uint32_t expire_nsec);
static EC_BOOL __crfs_release_npp(const UINT32 crfs_md_id, const CSTRING *file_path);
static EC_BOOL __crfs_collect_neighbors(const UINT32 crfs_md_id);
static EC_BOOL __crfs_recycle_of_np(const UINT32 crfs_md_id, const uint32_t crfsnp_id);

/*when found missed segment in bnode*/
static EC_BOOL __crfs_write_b_seg_dn_miss(const UINT32 crfs_md_id, 
                                              CRFSNP *crfsnp, 
                                              const uint32_t parent_pos, 
                                              UINT32 *offset, const CBYTES *cbytes,
                                              const uint32_t key_2nd_hash, 
                                              const uint32_t klen, const uint8_t *key,
                                              uint32_t *node_pos);
static EC_BOOL __crfs_write_b_seg_dn_hit(const UINT32 crfs_md_id, 
                                           CRFSNP *crfsnp,
                                           const uint32_t node_pos, 
                                           UINT32 *offset, const CBYTES *cbytes,
                                           const uint32_t key_2nd_hash, 
                                           const uint32_t klen, const uint8_t *key);
static EC_BOOL __crfs_write_b_seg_dn(const UINT32 crfs_md_id, 
                                             CRFSNP *crfsnp, const uint32_t parent_pos, 
                                             const uint32_t seg_no, UINT32 *offset, 
                                             const CBYTES *cbytes);
static EC_BOOL __crfs_write_b_dn(const UINT32 crfs_md_id, const uint32_t crfsnp_id, const uint32_t parent_pos, uint64_t *offset, const CBYTES *cbytes);

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

static EC_BOOL __crfs_fetch_block_fd_b_seg_dn_hit(const UINT32 crfs_md_id, 
                                                CRFSNP *crfsnp, 
                                                const uint32_t node_pos, 
                                                uint32_t *block_size,
                                                int *block_fd);

static EC_BOOL __crfs_fetch_block_fd_b_seg_dn(const UINT32 crfs_md_id, 
                                            CRFSNP *crfsnp, 
                                            const uint32_t parent_pos, 
                                            const uint32_t seg_no, 
                                            uint32_t *block_size,
                                            int *block_fd);

static EC_BOOL __crfs_fetch_block_fd_b_dn(const UINT32 crfs_md_id, 
                                             CRFSNP *crfsnp, 
                                             const uint32_t parent_pos, 
                                             const uint64_t offset, 
                                             uint32_t *block_size, 
                                             int *block_fd);                                            
                                            
/**
*
*  read data node at offset from the specific big file
*
**/
static EC_BOOL __crfs_read_b_dn(const UINT32 crfs_md_id, CRFSNP *crfsnp, const uint32_t parent_pos, uint64_t *offset, const UINT32 max_len, CBYTES *cbytes);


/**
*   for test only
*
*   to query the status of CRFS Module
*
**/
void crfs_print_module_status(const UINT32 crfs_md_id, LOG *log)
{
    CRFS_MD *crfs_md;
    UINT32 this_crfs_md_id;

    for( this_crfs_md_id = 0; this_crfs_md_id < CRFS_MD_CAPACITY(); this_crfs_md_id ++ )
    {
        crfs_md = CRFS_MD_GET(this_crfs_md_id);

        if ( NULL_PTR != crfs_md && 0 < crfs_md->usedcounter )
        {
            sys_log(log,"CRFS Module # %u : %u refered\n",
                    this_crfs_md_id,
                    crfs_md->usedcounter);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed CRFS module
*
*
**/
UINT32 crfs_free_module_static_mem(const UINT32 crfs_md_id)
{
    CRFS_MD  *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_free_module_static_mem: crfs module #0x%lx not started.\n",
                crfs_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    free_module_static_mem(MD_CRFS, crfs_md_id);

    return 0;
}

/**
*
* start CRFS module
*
**/
UINT32 crfs_start(const CSTRING *crfs_root_dir)
{
    CRFS_MD *crfs_md;
    UINT32   crfs_md_id;

    TASK_BRD *task_brd;
    EC_BOOL   ret;

    CSTRING *crfs_dir;
    CSTRING *crfsnp_root_dir;
    CSTRING *crfsdn_root_dir;    

    task_brd = task_brd_default_get();
    
    crfs_md_id = cbc_md_new(MD_CRFS, sizeof(CRFS_MD));
    if(ERR_MODULE_ID == crfs_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /*check validity*/
    if(CRFS_MAX_MODI < crfs_md_id) /*limited to 2-digital*/
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: crfs_md_id %ld overflow\n", crfs_md_id);

        cbc_md_free(MD_CRFS, crfs_md_id);
        return (ERR_MODULE_ID);
    }

    crfs_dir = cstring_make("%s/rfs%02ld", (char *)cstring_get_str(crfs_root_dir), crfs_md_id);
    if(NULL_PTR == crfs_dir)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: new crfs_dir failed\n");

        cbc_md_free(MD_CRFS, crfs_md_id);
        return (ERR_MODULE_ID);
    }

    if(EC_FALSE == c_file_access((char *)cstring_get_str(crfs_dir), F_OK))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: RFS %ld dir %s not exist\n", 
                           crfs_md_id, (char *)cstring_get_str(crfs_dir));

        cbc_md_free(MD_CRFS, crfs_md_id);
        cstring_free(crfs_dir);
        return (ERR_MODULE_ID);
    }
    cstring_free(crfs_dir);

    crfsnp_root_dir = cstring_make("%s/rfs%02ld", (char *)cstring_get_str(crfs_root_dir), crfs_md_id);
    if(NULL_PTR == crfsnp_root_dir)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: new crfsnp_root_dir failed\n");

        cbc_md_free(MD_CRFS, crfs_md_id);
        return (ERR_MODULE_ID);
    }
    
    crfsdn_root_dir = cstring_make("%s/rfs%02ld", (char *)cstring_get_str(crfs_root_dir), crfs_md_id);
    if(NULL_PTR == crfsdn_root_dir)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: new crfsdn_root_dir failed\n");

        cbc_md_free(MD_CRFS, crfs_md_id);

        cstring_free(crfsnp_root_dir);
        return (ERR_MODULE_ID);
    }    

    
    /* initilize new one CRFS module */
    crfs_md = (CRFS_MD *)cbc_md_get(MD_CRFS, crfs_md_id);
    crfs_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();    

    CRFS_MD_DN_MOD_MGR(crfs_md)  = mod_mgr_new(crfs_md_id, LOAD_BALANCING_QUE);
    CRFS_MD_NPP_MOD_MGR(crfs_md) = mod_mgr_new(crfs_md_id, LOAD_BALANCING_QUE);
    
    CRFS_MD_DN(crfs_md)  = NULL_PTR;
    CRFS_MD_NPP(crfs_md) = NULL_PTR;

    ret = EC_TRUE;
    if(EC_TRUE  == ret && NULL_PTR != crfsnp_root_dir 
    && EC_FALSE == cstring_is_empty(crfsnp_root_dir) 
    && EC_TRUE  == crfsnp_mgr_exist(crfsnp_root_dir))
    {
        CRFS_MD_NPP(crfs_md) = crfsnp_mgr_open(crfsnp_root_dir);
        if(NULL_PTR == CRFS_MD_NPP(crfs_md))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: open npp from root dir %s failed\n", 
                               (char *)cstring_get_str(crfsnp_root_dir));
            ret = EC_FALSE;
        }
    }

    if(EC_TRUE  == ret && NULL_PTR != crfsdn_root_dir 
    && EC_FALSE == cstring_is_empty(crfsdn_root_dir) 
    && EC_TRUE  == crfsdn_exist((char *)cstring_get_str(crfsdn_root_dir)))
    {
        CRFS_MD_DN(crfs_md) = crfsdn_open((char *)cstring_get_str(crfsdn_root_dir));
        if(NULL_PTR == CRFS_MD_DN(crfs_md))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: open dn with root dir %s failed\n", 
                               (char *)cstring_get_str(crfsdn_root_dir));
            ret = EC_FALSE;
        }    
    }

    cstring_free(crfsnp_root_dir);    
    cstring_free(crfsdn_root_dir);

    if(SWITCH_ON == CRFS_MEMC_SWITCH && EC_TRUE == ret)
    {
        CRFS_MD_MCACHE(crfs_md) = crfsmc_new(crfs_md_id, 
                                             CRFSMC_NP_ID, CHFSNP_512M_MODEL, 
                                             CHASH_JS_ALGO_ID, 
                                             CPGD_001GB_BLOCK_NUM);
        if(NULL_PTR == CRFS_MD_MCACHE(crfs_md))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: new memcache failed\n");
            ret = EC_FALSE;
        }
    }
    else
    {
        CRFS_MD_MCACHE(crfs_md) = NULL_PTR;
    }

    CRFS_MD_BACKUP(crfs_md) = NULL_PTR;

    if(EC_FALSE == ret)
    {
        if(NULL_PTR != CRFS_MD_MCACHE(crfs_md))
        {
            crfsmc_free(CRFS_MD_MCACHE(crfs_md));
            CRFS_MD_MCACHE(crfs_md) = NULL_PTR;
        }

        if(NULL_PTR != CRFS_MD_BACKUP(crfs_md))
        {
            crfsbk_free(CRFS_MD_BACKUP(crfs_md));
            CRFS_MD_BACKUP(crfs_md) = NULL_PTR;
        }
        
        if(NULL_PTR != CRFS_MD_DN(crfs_md))
        {
            crfsdn_close(CRFS_MD_DN(crfs_md));
            CRFS_MD_DN(crfs_md) = NULL_PTR;
        }

        if(NULL_PTR != CRFS_MD_NPP(crfs_md))
        {
            crfsnp_mgr_close(CRFS_MD_NPP(crfs_md));
            CRFS_MD_NPP(crfs_md) = NULL_PTR;
        }
        
        cbc_md_free(MD_CRFS, crfs_md_id);

        return (ERR_MODULE_ID);
    }

    CRFS_MD_CBTIMER_NODE(crfs_md) = NULL_PTR;
    
    if(NULL_PTR != CRFS_MD_DN(crfs_md))
    {
        CBTIMER_NODE *cbtimer_node;
        
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_start: crfs %ld try to add crfsdn cached nodes expire event\n", crfs_md_id);
        cbtimer_node = cbtimer_add(TASK_BRD_CBTIMER_LIST(task_brd_default_get()), 
                                   (UINT8 *)"CRFS_EXPIRE_DN", 
                                   CBTIMER_NEVER_EXPIRE, 
                                   CRFS_CHECK_DN_EXPIRE_IN_NSEC, 
                                   FI_crfs_expire_dn, crfs_md_id);
                                   
        CRFS_MD_CBTIMER_NODE(crfs_md) = cbtimer_node;
    }

    cvector_init(CRFS_MD_NEIGHBOR_VEC(crfs_md), 0, MM_MOD_NODE, CVECTOR_LOCK_ENABLE, LOC_CRFS_0001);

    CRFS_MD_STATE(crfs_md) = CRFS_WORK_STATE;

    crfs_md->usedcounter = 1;

    csig_atexit_register((CSIG_ATEXIT_HANDLER)crfs_end, crfs_md_id);

    __crfs_collect_neighbors(crfs_md_id);

    dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "crfs_start: start CRFS module #%u\n", crfs_md_id);

    CRFS_INIT_LOCK(crfs_md, LOC_CRFS_0002);

    if(SWITCH_ON == CRFS_DN_DEFER_WRITE_SWITCH)
    {
        UINT32 core_max_num;
        UINT32 flush_thread_idx;

        CRFS_MD_TERMINATE_FLAG(crfs_md) = EC_FALSE;
        core_max_num = sysconf(_SC_NPROCESSORS_ONLN);

        ASSERT(0 < CRFS_DN_DEFER_WRITE_THREAD_NUM);

        for(flush_thread_idx = 0; flush_thread_idx < CRFS_DN_DEFER_WRITE_THREAD_NUM; flush_thread_idx ++)
        {
            cthread_new(CTHREAD_DETACHABLE | CTHREAD_SYSTEM_LEVEL,
                    (UINT32)crfsdn_flush_cache_nodes,
                    (UINT32)(TASK_BRD_RANK(task_brd) % core_max_num), /*core #*/
                    (UINT32)2,/*para num*/
                    (UINT32)(&(CRFS_MD_DN(crfs_md))),
                    (UINT32)&(CRFS_MD_TERMINATE_FLAG(crfs_md))
                    );    
        }
    }

    if(SWITCH_ON == CRFSNGX_SWITCH && CMPI_FWD_RANK == CMPI_LOCAL_RANK)
    {
        if(EC_TRUE == task_brd_default_check_csrv_enabled())
        {
            task_brd_default_start_crfsngx_srv(crfs_md_id, task_brd_default_get_srv_ipaddr(), task_brd_default_get_csrv_port());
        }
    }

    if(SWITCH_ON == CRFSHTTP_SWITCH && CMPI_FWD_RANK == CMPI_LOCAL_RANK)
    {
        if(EC_TRUE == task_brd_default_check_csrv_enabled())
        {
            if(EC_FALSE == crfshttp_defer_request_queue_init())
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: init crfshttp defer request queue failed\n");
                crfs_end(crfs_md_id);
                return (ERR_MODULE_ID);
            }

            if(EC_FALSE == crfshttp_csocket_cnode_defer_close_list_init())
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start: init crfshttp node defer clean list failed\n");
                crfs_end(crfs_md_id);
                return (ERR_MODULE_ID);
            }            
            
            task_brd_default_start_crfshttp_srv(crfs_md_id, task_brd_default_get_srv_ipaddr(), task_brd_default_get_csrv_port());
        }
    }    

    return ( crfs_md_id );
}

/**
*
* end CRFS module
*
**/
void crfs_end(const UINT32 crfs_md_id)
{
    CRFS_MD *crfs_md;

    csig_atexit_unregister((CSIG_ATEXIT_HANDLER)crfs_end, crfs_md_id);

    crfs_md = CRFS_MD_GET(crfs_md_id);
    if(NULL_PTR == crfs_md)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_end: crfs_md_id = %u not exist.\n", crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
    
    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < crfs_md->usedcounter )
    {
        crfs_md->usedcounter --;
        return ;
    }

    if ( 0 == crfs_md->usedcounter )
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_end: crfs_md_id = %u is not started.\n", crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }

    if(NULL_PTR != CRFS_MD_CBTIMER_NODE(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_start: crfs %ld try to add crfsdn cached nodes expire event\n", crfs_md_id);
        cbtimer_unregister(TASK_BRD_CBTIMER_LIST(task_brd_default_get()), CRFS_MD_CBTIMER_NODE(crfs_md));
        CRFS_MD_CBTIMER_NODE(crfs_md) = NULL_PTR;
    }    

    CRFS_MD_STATE(crfs_md) = CRFS_ERR_STATE;

    /* if nobody else occupied the module,then free its resource */
    if(NULL_PTR != CRFS_MD_MCACHE(crfs_md))
    {
        crfsmc_free(CRFS_MD_MCACHE(crfs_md));
        CRFS_MD_MCACHE(crfs_md) = NULL_PTR;
    } 

    if(NULL_PTR != CRFS_MD_BACKUP(crfs_md))
    {
        crfsbk_free(CRFS_MD_BACKUP(crfs_md));
        CRFS_MD_BACKUP(crfs_md) = NULL_PTR;
    }    
    
    if(NULL_PTR != CRFS_MD_DN(crfs_md))
    {
        crfsdn_close(CRFS_MD_DN(crfs_md));
        CRFS_MD_DN(crfs_md) = NULL_PTR;
    }

    if(NULL_PTR != CRFS_MD_NPP(crfs_md))
    {
        crfsnp_mgr_close(CRFS_MD_NPP(crfs_md));
        CRFS_MD_NPP(crfs_md) = NULL_PTR;
    }

    if(NULL_PTR != CRFS_MD_DN_MOD_MGR(crfs_md))
    {
        mod_mgr_free(CRFS_MD_DN_MOD_MGR(crfs_md));
        CRFS_MD_DN_MOD_MGR(crfs_md)  = NULL_PTR;
    }

    if(NULL_PTR != CRFS_MD_NPP_MOD_MGR(crfs_md))
    {
        mod_mgr_free(CRFS_MD_NPP_MOD_MGR(crfs_md));
        CRFS_MD_NPP_MOD_MGR(crfs_md)  = NULL_PTR;
    }   

    cvector_clean(CRFS_MD_NEIGHBOR_VEC(crfs_md), (CVECTOR_DATA_CLEANER)mod_node_free, LOC_CRFS_0003);
    
    /* free module : */
    //crfs_free_module_static_mem(crfs_md_id);

    crfs_md->usedcounter = 0;
    CRFS_CLEAN_LOCK(crfs_md, LOC_CRFS_0004);

    dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "crfs_end: stop CRFS module #%u\n", crfs_md_id);
    cbc_md_free(MD_CRFS, crfs_md_id);

    return ;
}

EC_BOOL crfs_create_backup(const UINT32 crfs_md_id, const CSTRING *crfsnp_root_dir_bk, const CSTRING *crfsdn_root_dir_bk, const CSTRING *crfs_op_fname)
{
    CRFS_MD  *crfs_md;
    CRFSBK   *crfsbk;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_create_backup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        crfs_print_module_status(crfs_md_id, LOGSTDOUT);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR != CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_backup: give up due to backup RFS already exist!\n");
        return (EC_FALSE);
    }

    /*create np and dn of backup RFS*/
    crfsbk = crfsbk_new(crfs_md_id, 
                        (char *)cstring_get_str(crfsnp_root_dir_bk), 
                        (char *)cstring_get_str(crfsdn_root_dir_bk), 
                        CRFSBK_NP_ID, CRFSBK_NP_MODEL, CRFSBK_HASH_ALGO_ID, /*to simplify, fix parameters here*/
                        (char *)cstring_get_str(crfs_op_fname));
    if(NULL_PTR == crfsbk)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_backup: create backup RFS of np dir %s, dn dir %s, op file %s failed\n", 
                            (char *)cstring_get_str(crfsnp_root_dir_bk), 
                            (char *)cstring_get_str(crfsdn_root_dir_bk),
                            (char *)cstring_get_str(crfs_op_fname));
        return (EC_FALSE);
    }

    /*add 1TB disk to dn of backup RFS*/
    if(EC_FALSE == crfsbk_add_disk(crfsbk, CRFSBK_DISK_NO))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_backup: add disk to backup RFS of np dir %s and dn dir %s failed\n", 
                            (char *)cstring_get_str(crfsnp_root_dir_bk), (char *)cstring_get_str(crfsdn_root_dir_bk));
        crfsbk_free(crfsbk);
        return (EC_FALSE);
    }

    CRFS_MD_BACKUP(crfs_md) = crfsbk;

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_create_backup: create backup RFS of np dir %s and dn dir %s done\n", 
                        (char *)cstring_get_str(crfsnp_root_dir_bk), (char *)cstring_get_str(crfsdn_root_dir_bk));    

    return (EC_TRUE);
}

EC_BOOL crfs_open_backup(const UINT32 crfs_md_id, const CSTRING *crfsnp_root_dir_bk, const CSTRING *crfsdn_root_dir_bk, const CSTRING *crfs_op_fname)
{
    CRFS_MD  *crfs_md;
    CRFSBK   *crfsbk;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_open_backup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        crfs_print_module_status(crfs_md_id, LOGSTDOUT);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR != CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_open_backup: backup RFS is already open\n");
        return (EC_TRUE);
    }

    /*create np and dn of backup RFS*/
    crfsbk = crfsbk_open(crfs_md_id, 
                        (char *)cstring_get_str(crfsnp_root_dir_bk), 
                        (char *)cstring_get_str(crfsdn_root_dir_bk), 
                        CRFSBK_NP_ID, /*to simplify, fix parameters here*/
                        (char *)cstring_get_str(crfs_op_fname));
    if(NULL_PTR == crfsbk)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_open_backup: open backup RFS of np dir %s and dn dir %s failed\n", 
                            (char *)cstring_get_str(crfsnp_root_dir_bk), (char *)cstring_get_str(crfsdn_root_dir_bk));
        return (EC_FALSE);
    }

    CRFS_MD_BACKUP(crfs_md) = crfsbk;

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_open_backup: open backup RFS of np dir %s and dn dir %s done\n", 
                        (char *)cstring_get_str(crfsnp_root_dir_bk), (char *)cstring_get_str(crfsdn_root_dir_bk));    

    return (EC_TRUE);
}

EC_BOOL crfs_close_backup(const UINT32 crfs_md_id)
{
    CRFS_MD  *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_close_backup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        crfs_print_module_status(crfs_md_id, LOGSTDOUT);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_close_backup: no backup RFS exist\n");
        return (EC_TRUE);
    }

    crfsbk_free(CRFS_MD_BACKUP(crfs_md));
    CRFS_MD_BACKUP(crfs_md) = NULL_PTR;
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_close_backup: backup RFS was closed\n");
    return (EC_TRUE);
}

EC_BOOL crfs_start_sync(const UINT32 crfs_md_id)
{
    CRFS_MD  *crfs_md;
    UINT32    crfs_state;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_start_sync: crfs module #0x%lx not started.\n",
                crfs_md_id);
        crfs_print_module_status(crfs_md_id, LOGSTDOUT);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfs_state = crfs_get_state(crfs_md_id);
    if(EC_FALSE == crfs_is_state(crfs_md_id, CRFS_WORK_STATE))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start_sync: RFS is not in WORK state\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_start_sync: backup RFS not created or open yet\n");
        return (EC_FALSE);
    }

    crfs_set_state(crfs_md_id, CRFS_SYNC_STATE);
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_start_sync: RFS enter SYNC state\n");

    return (EC_TRUE);
}

EC_BOOL crfs_end_sync(const UINT32 crfs_md_id)
{
    CRFS_MD  *crfs_md;
    UINT32    crfs_state;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_end_sync: crfs module #0x%lx not started.\n",
                crfs_md_id);
        crfs_print_module_status(crfs_md_id, LOGSTDOUT);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfs_state = crfs_get_state(crfs_md_id);
    if(EC_FALSE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_end_sync: RFS is not in SYNC state\n");
        return (EC_FALSE);
    }

    crfs_set_state(crfs_md_id, CRFS_WORK_STATE);
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_end_sync: RFS enter WORK state\n");
    
    return (EC_TRUE);
}

EC_BOOL crfs_show_backup(const UINT32 crfs_md_id, LOG *log)
{
    CRFS_MD  *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_show_backup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        crfs_print_module_status(crfs_md_id, LOGSTDOUT);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    if(NULL_PTR == CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_show_backup: backup RFS not open yet\n");
        return (EC_FALSE);
    }

    crfsbk_print(log, CRFS_MD_BACKUP(crfs_md));
    
    return (EC_TRUE);
}

static EC_BOOL __crfs_add_neighbor(const UINT32 crfs_md_id, TASKS_CFG *remote_tasks_cfg)
{
    CRFS_MD  *crfs_md;
    CVECTOR  *crfs_neighbor_vec;
    MOD_NODE *mod_node;

    crfs_md = CRFS_MD_GET(crfs_md_id);
    crfs_neighbor_vec = CRFS_MD_NEIGHBOR_VEC(crfs_md);

    mod_node = mod_node_new();
    if(NULL_PTR == mod_node)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_add_neighbor: new mod_node failed\n");
        return (EC_FALSE);
    }

    MOD_NODE_TCID(mod_node) = TASKS_CFG_TCID(remote_tasks_cfg);
    MOD_NODE_COMM(mod_node) = CMPI_COMM_NULL;
    MOD_NODE_RANK(mod_node) = CMPI_CRFS_RANK;
    MOD_NODE_MODI(mod_node) = 0;   

    if(CVECTOR_ERR_POS != cvector_search_front(crfs_neighbor_vec, (void *)mod_node, (CVECTOR_DATA_CMP)mod_node_cmp))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_add_neighbor: tcid %s already in neighbors\n", MOD_NODE_TCID_STR(mod_node));
        mod_node_free(mod_node);
        return (EC_FALSE);
    }

    cvector_push(crfs_neighbor_vec, (void *)mod_node);

    return (EC_TRUE);
}

static EC_BOOL __crfs_collect_neighbors_from_cluster(const UINT32 crfs_md_id, const UINT32 cluster_id)
{
    TASK_BRD    *task_brd;    
    TASKS_CFG   *local_tasks_cfg;
    CLUSTER_CFG *cluster_cfg;

    task_brd = task_brd_default_get();
    local_tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);

    cluster_cfg = sys_cfg_get_cluster_cfg_by_id(TASK_BRD_SYS_CFG(task_brd), cluster_id);
    if(NULL_PTR == cluster_cfg)
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:__crfs_collect_neighbors_from_cluster: not found cluter %ld definition\n", cluster_id);
        return (EC_TRUE);
    }

    if(MODEL_TYPE_HSRFS_CONNEC == CLUSTER_CFG_MODEL(cluster_cfg))
    {        
        CVECTOR  *cluster_nodes;
        UINT32    pos;
        
        cluster_nodes = CLUSTER_CFG_NODES(cluster_cfg);
        
        CVECTOR_LOCK(cluster_nodes, LOC_CRFS_0005);
        for(pos = 0; pos < cvector_size(cluster_nodes); pos ++)
        {
            CLUSTER_NODE_CFG *cluster_node_cfg;
            TASKS_CFG *remote_tasks_cfg;
            
            cluster_node_cfg = (CLUSTER_NODE_CFG *)cvector_get_no_lock(cluster_nodes, pos);
            if(NULL_PTR == cluster_node_cfg)
            {
                continue;
            }

            remote_tasks_cfg = sys_cfg_search_tasks_cfg(TASK_BRD_SYS_CFG(task_brd), CLUSTER_NODE_CFG_TCID(cluster_node_cfg), CMPI_ANY_MASK, CMPI_ANY_MASK);
            if(NULL_PTR == remote_tasks_cfg)
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_collect_neighbors_from_cluster: not found tasks_cfg of cluster node %s\n", CLUSTER_NODE_CFG_TCID_STR(cluster_node_cfg));
                continue;
            }

            if(EC_TRUE == tasks_cfg_cmp(local_tasks_cfg, remote_tasks_cfg))
            {
                dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] __crfs_collect_neighbors_from_cluster: skip local tcid %s\n", CLUSTER_NODE_CFG_TCID_STR(cluster_node_cfg));
                continue;
            }

            /*check whether remote_tasks_cfg belong to the intranet of local_tasks_cfg*/
            if(EC_FALSE == tasks_cfg_is_intranet(TASK_BRD_TASKS_CFG(task_brd), remote_tasks_cfg)
            && EC_FALSE == tasks_cfg_is_externet(TASK_BRD_TASKS_CFG(task_brd), remote_tasks_cfg)
            && EC_FALSE == tasks_cfg_is_lannet(TASK_BRD_TASKS_CFG(task_brd), remote_tasks_cfg)
            && EC_FALSE == tasks_cfg_is_dbgnet(TASK_BRD_TASKS_CFG(task_brd), remote_tasks_cfg)
            && EC_FALSE == tasks_cfg_is_monnet(TASK_BRD_TASKS_CFG(task_brd), remote_tasks_cfg)
            )
            {
                continue;
            }

            if(EC_FALSE == __crfs_add_neighbor(crfs_md_id, remote_tasks_cfg))
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_collect_neighbors_from_cluster: add neighbor tcid %s failed\n", CLUSTER_NODE_CFG_TCID_STR(cluster_node_cfg));
                continue;
            }
            dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG]__crfs_collect_neighbors_from_cluster: add neighbor tcid %s done\n", CLUSTER_NODE_CFG_TCID_STR(cluster_node_cfg));
        }
        CVECTOR_UNLOCK(cluster_nodes, LOC_CRFS_0006);
    }    
    else
    {
        dbg_log(SEC_0031_CRFS, 3)(LOGSTDOUT, "info:__crfs_collect_neighbors_from_cluster: skip cluster %ld due to mismatched model %ld\n",
                           cluster_id, CLUSTER_CFG_MODEL(cluster_cfg));
    }
    
    return (EC_TRUE);
}

static EC_BOOL __crfs_collect_neighbors(const UINT32 crfs_md_id)
{
    CRFS_MD     *crfs_md;

    TASK_BRD    *task_brd;
    TASKS_CFG   *tasks_cfg;

    EC_BOOL      ret;

    crfs_md = CRFS_MD_GET(crfs_md_id);
    if(NULL_PTR == crfs_md)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_collect_neighbors: crfs_md_id = %u not exist.\n", crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }

    task_brd  = task_brd_default_get();
    tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);

    if(NULL_PTR != tasks_cfg)
    {
        cvector_loop(TASKS_CFG_CLUSTER_VEC(tasks_cfg), &ret, NULL_PTR, 
                                (UINT32)2, 
                                (UINT32)1, 
                                (UINT32)__crfs_collect_neighbors_from_cluster,
                                crfs_md_id,
                                NULL_PTR);
    }
    return (EC_TRUE);
}

/**
*
* initialize mod mgr of CRFS module
*
**/
UINT32 crfs_set_npp_mod_mgr(const UINT32 crfs_md_id, const MOD_MGR * src_mod_mgr)
{
    CRFS_MD *crfs_md;
    MOD_MGR  *des_mod_mgr;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_set_npp_mod_mgr: crfs module #0x%lx not started.\n",
                crfs_md_id);
        crfs_print_module_status(crfs_md_id, LOGSTDOUT);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    des_mod_mgr = CRFS_MD_NPP_MOD_MGR(crfs_md);

    dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "crfs_set_npp_mod_mgr: md_id %d, input src_mod_mgr %lx\n", crfs_md_id, src_mod_mgr);
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    /*figure out mod_nodes with tcid belong to set of crfsnp_tcid_vec and crfsnp_tcid_vec*/
    mod_mgr_limited_clone(crfs_md_id, src_mod_mgr, des_mod_mgr);

    dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "====================================crfs_set_npp_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "====================================crfs_set_npp_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);

    return (0);
}

UINT32 crfs_set_dn_mod_mgr(const UINT32 crfs_md_id, const MOD_MGR * src_mod_mgr)
{
    CRFS_MD *crfs_md;
    MOD_MGR  *des_mod_mgr;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_set_dn_mod_mgr: crfs module #0x%lx not started.\n",
                crfs_md_id);
        crfs_print_module_status(crfs_md_id, LOGSTDOUT);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    des_mod_mgr = CRFS_MD_DN_MOD_MGR(crfs_md);

    dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "crfs_set_dn_mod_mgr: md_id %d, input src_mod_mgr %lx\n", crfs_md_id, src_mod_mgr);
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    /*figure out mod_nodes with tcid belong to set of crfsnp_tcid_vec and crfsnp_tcid_vec*/
    mod_mgr_limited_clone(crfs_md_id, src_mod_mgr, des_mod_mgr);

    dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "====================================crfs_set_dn_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "====================================crfs_set_dn_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);

    return (0);
}

/**
*
* get mod mgr of CRFS module
*
**/
MOD_MGR * crfs_get_npp_mod_mgr(const UINT32 crfs_md_id)
{
    CRFS_MD *crfs_md;

    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        return (MOD_MGR *)0;
    }

    crfs_md = CRFS_MD_GET(crfs_md_id);
    return CRFS_MD_NPP_MOD_MGR(crfs_md);
}

MOD_MGR * crfs_get_dn_mod_mgr(const UINT32 crfs_md_id)
{
    CRFS_MD *crfs_md;

    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        return (MOD_MGR *)0;
    }

    crfs_md = CRFS_MD_GET(crfs_md_id);
    return CRFS_MD_DN_MOD_MGR(crfs_md);
}

CRFSNP_FNODE *crfs_fnode_new(const UINT32 crfs_md_id)
{
    return crfsnp_fnode_new();
}

EC_BOOL crfs_fnode_init(const UINT32 crfs_md_id, CRFSNP_FNODE *crfsnp_fnode)
{
    return crfsnp_fnode_init(crfsnp_fnode);
}

EC_BOOL crfs_fnode_clean(const UINT32 crfs_md_id, CRFSNP_FNODE *crfsnp_fnode)
{
    return crfsnp_fnode_clean(crfsnp_fnode);
}

EC_BOOL crfs_fnode_free(const UINT32 crfs_md_id, CRFSNP_FNODE *crfsnp_fnode)
{
    return crfsnp_fnode_free(crfsnp_fnode);
}

EC_BOOL crfs_set_state(const UINT32 crfs_md_id, const UINT32 crfs_state)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_set_state: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id); 

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_set_state: crfs module #0x%lx: state %lx -> %lx\n", 
                        CRFS_MD_STATE(crfs_md), crfs_state);
    
    CRFS_MD_STATE(crfs_md) = crfs_state;

    return (EC_TRUE);
}

UINT32 crfs_get_state(const UINT32 crfs_md_id)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_get_state: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id); 
    
    return CRFS_MD_STATE(crfs_md);
}    

EC_BOOL crfs_is_state(const UINT32 crfs_md_id, const UINT32 crfs_state)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_is_state: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id); 
    if(CRFS_MD_STATE(crfs_md) == crfs_state)
    {
        return (EC_TRUE);
    }
    
    return (EC_FALSE);
}

/**
*
*  get name node pool of the module
*
**/
CRFSNP_MGR *crfs_get_npp(const UINT32 crfs_md_id)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_get_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    return CRFS_MD_NPP(crfs_md);
}

/**
*
*  get data node of the module
*
**/
CRFSDN *crfs_get_dn(const UINT32 crfs_md_id)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_get_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    return CRFS_MD_DN(crfs_md);
}

/**
*
*  open name node pool
*
**/
EC_BOOL crfs_open_npp(const UINT32 crfs_md_id, const CSTRING *crfsnp_db_root_dir)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_open_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR != CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_open_npp: npp was open\n");
        return (EC_FALSE);
    }

    CRFS_MD_NPP(crfs_md) = crfsnp_mgr_open(crfsnp_db_root_dir);
    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_open_npp: open npp from root dir %s failed\n", (char *)cstring_get_str(crfsnp_db_root_dir));
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

/**
*
*  close name node pool
*
**/
EC_BOOL crfs_close_npp(const UINT32 crfs_md_id)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_close_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_close_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_close(CRFS_MD_NPP(crfs_md));
    CRFS_MD_NPP(crfs_md) = NULL_PTR;
    return (EC_TRUE);
}

/**
*
*  check this CRFS is name node pool or not
*
*
**/
EC_BOOL crfs_is_npp(const UINT32 crfs_md_id)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_is_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/**
*
*  check this CRFS is data node or not
*
*
**/
EC_BOOL crfs_is_dn(const UINT32 crfs_md_id)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_is_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/**
*
*  check this CRFS is data node and namenode or not
*
*
**/
EC_BOOL crfs_is_npp_and_dn(const UINT32 crfs_md_id)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_is_npp_and_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md) || NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/**
*
*  create name node pool
*
**/
EC_BOOL crfs_create_npp(const UINT32 crfs_md_id, 
                             const UINT32 crfsnp_model, 
                             const UINT32 crfsnp_max_num, 
                             const UINT32 crfsnp_2nd_chash_algo_id, 
                             const CSTRING *crfsnp_db_root_dir)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_create_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR != CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_npp: npp already exist\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == c_check_is_uint8_t(crfsnp_model))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_npp: crfsnp_model %u is invalid\n", crfsnp_model);
        return (EC_FALSE);
    }

    if(EC_FALSE == c_check_is_uint32_t(crfsnp_max_num))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_npp: crfsnp_disk_max_num %u is invalid\n", crfsnp_max_num);
        return (EC_FALSE);
    }   

    if(EC_FALSE == c_check_is_uint8_t(crfsnp_2nd_chash_algo_id))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_npp: crfsnp_2nd_chash_algo_id %u is invalid\n", crfsnp_2nd_chash_algo_id);
        return (EC_FALSE);
    }    

    CRFS_MD_NPP(crfs_md) = crfsnp_mgr_create((uint8_t ) crfsnp_model, 
                                             (uint32_t) crfsnp_max_num, 
                                             (uint8_t ) crfsnp_2nd_chash_algo_id, 
                                             crfsnp_db_root_dir);
    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_npp: create npp failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfs_add_npp(const UINT32 crfs_md_id, const UINT32 crfsnpp_tcid, const UINT32 crfsnpp_rank)
{
    CRFS_MD   *crfs_md;

    TASK_BRD *task_brd;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_add_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    task_brd = task_brd_default_get();
#if 1
    if(EC_FALSE == task_brd_check_tcid_connected(task_brd, crfsnpp_tcid))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_add_npp: crfsnpp_tcid %s not connected\n", c_word_to_ipv4(crfsnpp_tcid));
        return (EC_FALSE);
    }
#endif
    mod_mgr_incl(crfsnpp_tcid, CMPI_ANY_COMM, crfsnpp_rank, 0, CRFS_MD_NPP_MOD_MGR(crfs_md));
    cload_mgr_set_que(TASK_BRD_CLOAD_MGR(task_brd), crfsnpp_tcid, crfsnpp_rank, 0);

    return (EC_TRUE);
}

EC_BOOL crfs_add_dn(const UINT32 crfs_md_id, const UINT32 crfsdn_tcid, const UINT32 crfsdn_rank)
{
    CRFS_MD   *crfs_md;

    TASK_BRD *task_brd;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_add_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    task_brd = task_brd_default_get();
#if 1
    if(EC_FALSE == task_brd_check_tcid_connected(task_brd, crfsdn_tcid))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_add_dn: crfsdn_tcid %s not connected\n", c_word_to_ipv4(crfsdn_tcid));
        return (EC_FALSE);
    }
#endif
    mod_mgr_incl(crfsdn_tcid, CMPI_ANY_COMM, crfsdn_rank, (UINT32)0, CRFS_MD_DN_MOD_MGR(crfs_md));
    cload_mgr_set_que(TASK_BRD_CLOAD_MGR(task_brd), crfsdn_tcid, crfsdn_rank, 0);

    return (EC_TRUE);
}

/**
*
*  check existing of a dir
*
**/
EC_BOOL crfs_find_dir(const UINT32 crfs_md_id, const CSTRING *dir_path)
{
    CRFS_MD   *crfs_md;
    EC_BOOL    ret;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_find_dir: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_find_dir: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0007);
    ret = crfsnp_mgr_find_dir(CRFS_MD_NPP(crfs_md), dir_path);
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0008);

    return (ret);
}

/**
*
*  check existing of a file
*
**/
EC_BOOL crfs_find_file(const UINT32 crfs_md_id, const CSTRING *file_path)
{
    CRFS_MD   *crfs_md;
    EC_BOOL    ret;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_find_file: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_find_file: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0009);
    ret = crfsnp_mgr_find_file(CRFS_MD_NPP(crfs_md), file_path);
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0010);
    return (ret);
}

/**
*
*  check existing of a big file
*
**/
EC_BOOL crfs_find_file_b(const UINT32 crfs_md_id, const CSTRING *file_path)
{
    CRFS_MD   *crfs_md;
    EC_BOOL    ret;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_find_file_b: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_find_file_b: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0011);
    ret = crfsnp_mgr_find_file_b(CRFS_MD_NPP(crfs_md), file_path);
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0012);
    return (ret);
}

/**
*
*  check existing of a file or a dir
*
**/
EC_BOOL crfs_find(const UINT32 crfs_md_id, const CSTRING *path)
{
    CRFS_MD   *crfs_md;
    EC_BOOL    ret;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_find: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_find: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0013);
    ret = crfsnp_mgr_find(CRFS_MD_NPP(crfs_md), path, CRFSNP_ITEM_FILE_IS_ANY/*xxx*/);
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0014);

    return (ret);
}

/**
*
*  check existing of a file or a dir
*
**/
EC_BOOL crfs_exists(const UINT32 crfs_md_id, const CSTRING *path)
{    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_exists: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    return crfs_find(crfs_md_id, path);
}

/**
*
*  check existing of a file
*
**/
EC_BOOL crfs_is_file(const UINT32 crfs_md_id, const CSTRING *file_path)
{
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_is_file: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    return crfs_find_file(crfs_md_id, file_path);;
}

/**
*
*  check existing of a dir
*
**/
EC_BOOL crfs_is_dir(const UINT32 crfs_md_id, const CSTRING *dir_path)
{
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_is_dir: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    return crfs_find_dir(crfs_md_id, dir_path);
}

/**
*
*  reserve space from dn
*
**/
static EC_BOOL __crfs_reserve_hash_dn(const UINT32 crfs_md_id, const UINT32 data_len, const uint32_t path_hash, CRFSNP_FNODE *crfsnp_fnode)
{
    CRFS_MD      *crfs_md;
    CRFSNP_INODE *crfsnp_inode;
    CPGV         *cpgv;

    uint32_t size;
    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;    

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_reserve_hash_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CPGB_CACHE_MAX_BYTE_SIZE <= data_len)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_reserve_hash_dn: data_len %u overflow\n", data_len);
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_reserve_hash_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    if(NULL_PTR == CRFSDN_CPGV(CRFS_MD_DN(crfs_md)))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_reserve_hash_dn: no pgv exist\n");
        return (EC_FALSE);
    }
    
    cpgv = CRFSDN_CPGV(CRFS_MD_DN(crfs_md));
    if(NULL_PTR == CPGV_HEADER(cpgv))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_reserve_hash_dn: pgv header is null\n");
        return (EC_FALSE);
    }

    if(0 == CPGV_PAGE_DISK_NUM(cpgv))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_reserve_hash_dn: pgv has no disk yet\n");
        return (EC_FALSE);
    }

    size    = (uint32_t)(data_len);
    disk_no = (uint16_t)(path_hash % CPGV_PAGE_DISK_NUM(cpgv));
    
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0015);
    if(EC_FALSE == cpgv_new_space_from_disk(cpgv, size, disk_no, &block_no, &page_no))
    {
        /*try again*/
        if(EC_FALSE == cpgv_new_space(cpgv, size, &disk_no, &block_no, &page_no))
        {
            crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0016);
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_reserve_hash_dn: new %u bytes space from vol failed\n", data_len);
            return (EC_FALSE);
        }
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0017);

    crfsnp_fnode_init(crfsnp_fnode);
    CRFSNP_FNODE_FILESZ(crfsnp_fnode) = size;
    CRFSNP_FNODE_REPNUM(crfsnp_fnode) = 1;   
    
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    CRFSNP_INODE_CACHE_FLAG(crfsnp_inode) = CRFSDN_DATA_NOT_IN_CACHE;
    CRFSNP_INODE_DISK_NO(crfsnp_inode)    = disk_no;
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode)   = block_no;
    CRFSNP_INODE_PAGE_NO(crfsnp_inode)    = page_no;

    return (EC_TRUE);
}

/**
*
*  reserve space from dn
*
**/
EC_BOOL crfs_reserve_dn(const UINT32 crfs_md_id, const UINT32 data_len, CRFSNP_FNODE *crfsnp_fnode)
{
    CRFS_MD      *crfs_md;
    CRFSNP_INODE *crfsnp_inode;

    uint32_t size;
    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;    

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_reserve_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CPGB_CACHE_MAX_BYTE_SIZE <= data_len)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_reserve_dn: data_len %u overflow\n", data_len);
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_reserve_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    size = (uint32_t)(data_len);
    
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0018);
    if(EC_FALSE == cpgv_new_space(CRFSDN_CPGV(CRFS_MD_DN(crfs_md)), size, &disk_no, &block_no, &page_no))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0019);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_reserve_dn: new %u bytes space from vol failed\n", data_len);
        return (EC_FALSE);
    }    
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0020);

    crfsnp_fnode_init(crfsnp_fnode);
    CRFSNP_FNODE_FILESZ(crfsnp_fnode) = size;
    CRFSNP_FNODE_REPNUM(crfsnp_fnode) = 1;   
    
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    CRFSNP_INODE_CACHE_FLAG(crfsnp_inode) = CRFSDN_DATA_NOT_IN_CACHE;
    CRFSNP_INODE_DISK_NO(crfsnp_inode)    = disk_no;
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode)   = block_no;
    CRFSNP_INODE_PAGE_NO(crfsnp_inode)    = page_no;

    return (EC_TRUE);
}

/**
*
*  release space to dn
*
**/
EC_BOOL crfs_release_dn(const UINT32 crfs_md_id, const CRFSNP_FNODE *crfsnp_fnode)
{
    CRFS_MD *crfs_md;
    const CRFSNP_INODE *crfsnp_inode;

    uint32_t file_size;
    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_release_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_release_dn: no dn was open\n");
        return (EC_FALSE);
    }

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);

    if(CPGB_CACHE_MAX_BYTE_SIZE < file_size)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_release_dn: file_size %u overflow\n", file_size);
        return (EC_FALSE);
    }

    if(0 == file_size)
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_release_dn: file_size is zero\n");
        return (EC_FALSE);
    }

    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;
    
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0021);
    if(EC_FALSE == cpgv_free_space(CRFSDN_CPGV(CRFS_MD_DN(crfs_md)), disk_no, block_no, page_no, file_size))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0022);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_release_dn: free %u bytes to vol failed where disk %u, block %u, page %u\n", 
                            file_size, disk_no, block_no, page_no);
        return (EC_FALSE);
    }    
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0023);

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_release_dn: remove file fsize %u, disk %u, block %u, page %u done\n", 
                       file_size, disk_no, block_no, page_no);
    
    return (EC_TRUE);
}

/**
*
*  write a file (version 0.1)
*
**/
static EC_BOOL __crfs_write_v01(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_write: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_fnode_init(&crfsnp_fnode);

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0024);

    if(EC_FALSE == crfs_write_dn(crfs_md_id, cbytes, &crfsnp_fnode))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0025);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write: write file %s content to dn failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    if(do_log(SEC_0031_CRFS, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] __crfs_write: write file %s to dn where fnode is \n", (char *)cstring_get_str(file_path));
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);   
    }
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] __crfs_write: write file %s is %.*s\n", 
                        (char *)cstring_get_str(file_path), DMIN(16, cbytes_len(cbytes)), cbytes_buf(cbytes));

    if(EC_FALSE == crfs_write_npp(crfs_md_id, file_path, &crfsnp_fnode))
    {   
        __crfs_delete_dn(crfs_md_id, &crfsnp_fnode);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0026);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write: write file %s to npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0027);
    return (EC_TRUE);
}

/**
*
*  write a file (version 0.2)
*
**/
static EC_BOOL __crfs_write_v02(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_write: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_fnode_init(&crfsnp_fnode);

    if(EC_FALSE == crfs_reserve_dn(crfs_md_id, CBYTES_LEN(cbytes), &crfsnp_fnode))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write: reserve dn %u bytes for file %s failed\n", 
                            CBYTES_LEN(cbytes), (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }    

    if(EC_FALSE == crfs_export_dn(crfs_md_id, cbytes, &crfsnp_fnode))
    {
        crfs_release_dn(crfs_md_id, &crfsnp_fnode);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write: export file %s content to dn failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    if(do_log(SEC_0031_CRFS, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] __crfs_write: write file %s to dn where fnode is \n", (char *)cstring_get_str(file_path));
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);   
    }
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] __crfs_write: write file %s is %.*s\n", 
                        (char *)cstring_get_str(file_path), DMIN(16, cbytes_len(cbytes)), cbytes_buf(cbytes));

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0028);
    if(EC_FALSE == crfs_write_npp(crfs_md_id, file_path, &crfsnp_fnode))
    {   
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0029);
        crfs_release_dn(crfs_md_id, &crfsnp_fnode);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write: write file %s to npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0030);

    return (EC_TRUE);
}

/**
*
*  write a file (version 0.3)
*
**/
static EC_BOOL __crfs_write(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes, const UINT32 expire_nsec)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE *crfsnp_fnode;
    uint32_t      path_hash;
    uint32_t      expire_nsec_t;
    uint8_t       md5sum[ CMD5_DIGEST_LEN ];

    crfs_md = CRFS_MD_GET(crfs_md_id);

    expire_nsec_t = (uint32_t)expire_nsec;

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0031);
    crfsnp_fnode = __crfs_reserve_npp(crfs_md_id, file_path, expire_nsec_t);
    if(NULL_PTR == crfsnp_fnode)
    {   
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0032);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write: file %s reserve npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0033);

    /*calculate hash value of file_path*/
    path_hash = (uint32_t)MD5_hash(cstring_get_len(file_path), cstring_get_str(file_path));

    /*exception*/
    if(0 == CBYTES_LEN(cbytes))
    {
        crfsnp_fnode_init(crfsnp_fnode);
        CRFSNP_FNODE_HASH(crfsnp_fnode) = path_hash;

        if(do_log(SEC_0031_CRFS, 1))
        {
            sys_log(LOGSTDOUT, "warn:__crfs_write: write file %s with zero len to dn where fnode is \n", (char *)cstring_get_str(file_path));
            crfsnp_fnode_print(LOGSTDOUT, crfsnp_fnode);   
        }
        
        return (EC_TRUE);
    }
        
    if(EC_FALSE == __crfs_reserve_hash_dn(crfs_md_id, CBYTES_LEN(cbytes), path_hash, crfsnp_fnode))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write: reserve dn %u bytes for file %s failed\n", 
                            CBYTES_LEN(cbytes), (char *)cstring_get_str(file_path));
                            
        CRFS_WRLOCK(crfs_md, LOC_CRFS_0034);                            
        __crfs_release_npp(crfs_md_id, file_path);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0035);
        return (EC_FALSE);
    }

    if(EC_FALSE == crfs_export_dn(crfs_md_id, cbytes, crfsnp_fnode))
    {
        crfs_release_dn(crfs_md_id, crfsnp_fnode);
        
        CRFS_WRLOCK(crfs_md, LOC_CRFS_0036);                            
        __crfs_release_npp(crfs_md_id, file_path);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0037);        
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write: export file %s content to dn failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    CRFSNP_FNODE_HASH(crfsnp_fnode) = path_hash;

    if(SWITCH_ON == CRFS_MD5_SWITCH)
    {
        cmd5_sum((uint32_t)CBYTES_LEN(cbytes), CBYTES_BUF(cbytes), md5sum);
        BCOPY(md5sum, CRFSNP_FNODE_MD5SUM(crfsnp_fnode), CMD5_DIGEST_LEN);
    }
    
    if(do_log(SEC_0031_CRFS, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] __crfs_write: write file %s to dn where fnode is \n", (char *)cstring_get_str(file_path));
        crfsnp_fnode_print(LOGSTDOUT, crfsnp_fnode);   
    }

    return (EC_TRUE);
}

static EC_BOOL __crfs_write_cache(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_fnode_init(&crfsnp_fnode);

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0038);

    if(EC_FALSE == crfs_write_dn_cache(crfs_md_id, cbytes, &crfsnp_fnode))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0039);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_cache: write file %s content to dn failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    if(do_log(SEC_0031_CRFS, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] __crfs_write_cache: write file %s to dn where fnode is \n", (char *)cstring_get_str(file_path));
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);   
    }
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] __crfs_write_cache: write file %s is %.*s\n", (char *)cstring_get_str(file_path), DMIN(16, cbytes_len(cbytes)), cbytes_buf(cbytes));

    if(EC_FALSE == crfs_write_npp(crfs_md_id, file_path, &crfsnp_fnode))
    {   
        __crfs_delete_dn(crfs_md_id, &crfsnp_fnode);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0040);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_cache: write file %s to npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0041);
    return (EC_TRUE);
}

EC_BOOL crfs_write_backup(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes, const UINT32 expire_nsec)
{
    CRFS_MD      *crfs_md;

    uint8_t       md5sum[ CMD5_DIGEST_LEN ];
    uint32_t      expire_nsec_t;    
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write_backup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error: crfs_write_backup: RFS in SYNC but backup RFS is null\n");
        return (EC_FALSE);
    }
    
    if(SWITCH_ON == CRFS_MD5_SWITCH)
    {
        cmd5_sum((uint32_t)CBYTES_LEN(cbytes), CBYTES_BUF(cbytes), md5sum);
    }        

    expire_nsec_t = (uint32_t)expire_nsec;

    if(EC_FALSE == crfsbk_write(CRFS_MD_BACKUP(crfs_md), file_path, cbytes, expire_nsec_t, md5sum))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error: crfs_write_backup: write file %s with size %ld to backup RFS failed\n", 
                           (char *)cstring_get_str(file_path), cbytes_len(cbytes)); 
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_write_backup: write file %s with size %ld to backup RFS done\n", 
                           (char *)cstring_get_str(file_path), cbytes_len(cbytes)); 

    return (EC_TRUE);
}

EC_BOOL crfs_write(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes, const UINT32 expire_nsec)
{
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    /*in SYNC state*/
    if(EC_TRUE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        if(EC_TRUE == crfs_write_backup(crfs_md_id, file_path, cbytes, expire_nsec))
        {
            dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_write: write file %s with size %ld to backup RFS done\n", 
                                   (char *)cstring_get_str(file_path), cbytes_len(cbytes));
            return (EC_TRUE);
        }

        /*fall through to RFS*/
        dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "warn:crfs_write: write file %s with size %ld to backup RFS failed, try to write to RFS\n", 
                               (char *)cstring_get_str(file_path), cbytes_len(cbytes));        
    }
    
    if(SWITCH_ON == CRFS_DN_DEFER_WRITE_SWITCH)
    {
        return __crfs_write_cache(crfs_md_id, file_path, cbytes);
    }

    return __crfs_write(crfs_md_id, file_path, cbytes, expire_nsec);
}

/**
*
*  read a file
*
**/
EC_BOOL crfs_read_safe(const UINT32 crfs_md_id, const CSTRING *file_path, CBYTES *cbytes, UINT32 *expires_timestamp, const EC_BOOL need_expired_content)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_read: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfsnp_fnode_init(&crfsnp_fnode);

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0042);
    if(EC_FALSE == crfs_read_npp(crfs_md_id, file_path, &crfsnp_fnode, expires_timestamp))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0043);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    if(NULL_PTR != expires_timestamp && (*expires_timestamp) > task_brd_default_get_time())
    {    
        if(do_log(SEC_0031_CRFS, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] crfs_read: read file %s [EXPIRED: %ld > %u] from npp and fnode %p is \n", 
                               (char *)cstring_get_str(file_path),
                               (*expires_timestamp), task_brd_default_get_time(),
                               &crfsnp_fnode);  
            crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);  
        }
        
        if(EC_FALSE == need_expired_content)
        {
            CRFS_UNLOCK(crfs_md, LOC_CRFS_0044);
            dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "crfs_read: not need to read expired file %s from dn\n", 
                               (char *)cstring_get_str(file_path));
            return (EC_FALSE);
        }        
    }
#if 0    
    else
    {    
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_read: read file %s from npp and fnode %p is \n", 
                           (char *)cstring_get_str(file_path),
                           &crfsnp_fnode);  
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);                           
    }
#endif    

    /*exception*/
    if(0 == CRFSNP_FNODE_FILESZ(&crfsnp_fnode))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0045);
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_read: read file %s with zero len from npp and fnode %p is \n", (char *)cstring_get_str(file_path), &crfsnp_fnode);
        return (EC_TRUE);
    }

    if(EC_FALSE == crfs_read_dn(crfs_md_id, &crfsnp_fnode, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0046);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read: read file %s from dn failed where fnode is \n", (char *)cstring_get_str(file_path));
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);
        return (EC_FALSE);
    }

    //dbg_log(SEC_0031_CRFS, 9)(LOGSTDNULL, "[DEBUG] crfs_read: read file %s is %.*s\n", (char *)cstring_get_str(file_path), DMIN(16, cbytes_len(cbytes)), cbytes_buf(cbytes));
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0047);
    return (EC_TRUE);
}

EC_BOOL crfs_read_backup(const UINT32 crfs_md_id, const CSTRING *file_path, CBYTES *cbytes, UINT32 *expires_timestamp, const EC_BOOL need_expired_content)
{
    CRFS_MD      *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_read_backup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    if(NULL_PTR == CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error: crfs_read_backup: RFS in SYNC but backup RFS is null\n");
        return (EC_FALSE);
    }        

    if(EC_FALSE == crfsbk_read(CRFS_MD_BACKUP(crfs_md), file_path, cbytes, expires_timestamp, need_expired_content))
    {
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_read_backup: read file %s with size %ld from backup RFS failed\n", 
                           (char *)cstring_get_str(file_path), cbytes_len(cbytes));     
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_read_backup: read file %s with size %ld from backup RFS done\n", 
                       (char *)cstring_get_str(file_path), cbytes_len(cbytes));  
    
    return (EC_TRUE);
}

EC_BOOL crfs_read(const UINT32 crfs_md_id, const CSTRING *file_path, CBYTES *cbytes, UINT32 *expires_timestamp, const EC_BOOL need_expired_content)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_read: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfsnp_fnode_init(&crfsnp_fnode);

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(SWITCH_ON == CRFS_MEMC_SWITCH)
    {
        if(EC_TRUE == crfsmc_read(CRFS_MD_MCACHE(crfs_md), file_path, cbytes))
        {
            dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_read: read file %s with size %ld from memcache done\n", 
                               (char *)cstring_get_str(file_path), cbytes_len(cbytes));    
            return (EC_TRUE);
        }
    }

    /*in SYNC state*/
    if(EC_TRUE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        if(EC_TRUE == crfs_read_backup(crfs_md_id, file_path, cbytes, expires_timestamp, need_expired_content))
        {
            dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_read: read file %s with from backup RFS done\n", 
                                   (char *)cstring_get_str(file_path));            
            return (EC_TRUE);
        }

        /*fall through to RFS*/
        dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "warn:crfs_read: read file %s with from backup RFS failed, try to read from RFS\n", 
                               (char *)cstring_get_str(file_path));            
    }
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0048);
    if(EC_FALSE == crfs_read_npp(crfs_md_id, file_path, &crfsnp_fnode, expires_timestamp))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0049);
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_read: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0050);

    /**
    *
    * WARNING:
    * after unlock, crfsnp_fnode read from npp will be dangerous due to someone may delete the file without
    * notifying the reader, thus reader would read "dirty" data which is deleted yet in logical.
    *
    * but we can ignore and tolerant the short-term "dirty" data
    *
    **/

    if(NULL_PTR != expires_timestamp && (*expires_timestamp) > task_brd_default_get_time())
    {    
        if(do_log(SEC_0031_CRFS, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] crfs_read: read file %s [EXPIRED: %ld > %u] from npp and fnode %p is \n", 
                               (char *)cstring_get_str(file_path),
                               (*expires_timestamp), task_brd_default_get_time(),
                               &crfsnp_fnode);  
            crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);  
        }
        
        if(EC_FALSE == need_expired_content)
        {
            dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "crfs_read: not need to read expired file %s from dn\n", 
                               (char *)cstring_get_str(file_path));
            return (EC_FALSE);
        }        
    }
#if 0    
    else
    {    
        if(do_log(SEC_0031_CRFS, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] crfs_read: read file %s from npp and fnode %p is \n", 
                               (char *)cstring_get_str(file_path),
                               &crfsnp_fnode);  
            crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);
        }
    }
#endif    

    /*exception*/
    if(0 == CRFSNP_FNODE_FILESZ(&crfsnp_fnode))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_read: read file %s with zero len from npp and fnode %p is \n", (char *)cstring_get_str(file_path), &crfsnp_fnode);
        return (EC_TRUE);
    }

    if(EC_FALSE == crfs_read_dn(crfs_md_id, &crfsnp_fnode, cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read: read file %s from dn failed where fnode is \n", (char *)cstring_get_str(file_path));
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);
        return (EC_FALSE);
    }

    if(SWITCH_ON == CRFS_MEMC_SWITCH)
    {
        crfsmc_ensure_room_safe_level(CRFS_MD_MCACHE(crfs_md));/*LRU retire & recycle*/
        crfsmc_write(CRFS_MD_MCACHE(crfs_md), file_path, cbytes, CRFSNP_FNODE_MD5SUM(&crfsnp_fnode));
    }

    if(do_log(SEC_0031_CRFS, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] crfs_read: read file %s with size %ld done\n", 
                            (char *)cstring_get_str(file_path), cbytes_len(cbytes));
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);                            
    }
    return (EC_TRUE);
}

/**
*
*  write a file in cache
*
**/


/*----------------------------------- POSIX interface -----------------------------------*/
/**
*
*  write a file at offset
*
**/
EC_BOOL crfs_write_e(const UINT32 crfs_md_id, const CSTRING *file_path, UINT32 *offset, const UINT32 max_len, const CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;
    UINT32        expires_timestamp;
    uint32_t      file_old_size;    

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write_e: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfsnp_fnode_init(&crfsnp_fnode);

    crfs_md = CRFS_MD_GET(crfs_md_id);

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0051);

    if(EC_FALSE == crfs_read_npp(crfs_md_id, file_path, &crfsnp_fnode, &expires_timestamp))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0052);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_e: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    file_old_size = CRFSNP_FNODE_FILESZ(&crfsnp_fnode);

    if(EC_FALSE == crfs_write_e_dn(crfs_md_id, &crfsnp_fnode, offset, max_len, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0053);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_e: offset write to dn failed\n");
        return (EC_FALSE);
    }

    if(file_old_size != CRFSNP_FNODE_FILESZ(&crfsnp_fnode))
    {
        if(EC_FALSE == crfs_update_npp(crfs_md_id, file_path, &crfsnp_fnode))
        {
            CRFS_UNLOCK(crfs_md, LOC_CRFS_0054);
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_e: offset write file %s to npp failed\n", (char *)cstring_get_str(file_path));
            return (EC_FALSE);
        }
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0055);
    return (EC_TRUE);
}

/**
*
*  read a file from offset
*
*  when max_len = 0, return the partial content from offset to EOF (end of file) 
*
**/
EC_BOOL crfs_read_e(const UINT32 crfs_md_id, const CSTRING *file_path, UINT32 *offset, const UINT32 max_len, CBYTES *cbytes, UINT32 *expires_timestamp, const EC_BOOL need_expired_content)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_read_e: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_fnode_init(&crfsnp_fnode);

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0056);
    
    if(EC_FALSE == crfs_read_npp(crfs_md_id, file_path, &crfsnp_fnode, expires_timestamp))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0057);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_e: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    if(NULL_PTR != expires_timestamp && (*expires_timestamp) > task_brd_default_get_time())
    {    
        if(do_log(SEC_0031_CRFS, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] crfs_read_e: read file %s [EXPIRED: %ld > %u] from npp and fnode %p is \n", 
                               (char *)cstring_get_str(file_path),
                               (*expires_timestamp), task_brd_default_get_time(),
                               &crfsnp_fnode);  
            crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);  
        }
        
        if(EC_FALSE == need_expired_content)
        {
            CRFS_UNLOCK(crfs_md, LOC_CRFS_0058);
            dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "crfs_read_e: not need to read expired file %s from dn\n", 
                               (char *)cstring_get_str(file_path));
            return (EC_FALSE);
        }        
    }
    else
    {    
        if(do_log(SEC_0031_CRFS, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] crfs_read_e: read file %s from npp and fnode %p is \n", 
                               (char *)cstring_get_str(file_path),
                               &crfsnp_fnode);  
            crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);
        }
    }    

    if(EC_FALSE == crfs_read_e_dn(crfs_md_id, &crfsnp_fnode, offset, max_len, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0059);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_e: offset read file %s from dn failed where fnode is\n", (char *)cstring_get_str(file_path));
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);
        return (EC_FALSE);
    }
    
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0060);
    return (EC_TRUE);
}

/*----------------------------------- BIG FILE interface -----------------------------------*/
/**
*
*  create a bnode in name node
*
**/
static EC_BOOL __crfs_create_b_npp(const UINT32 crfs_md_id, const CSTRING *file_path, const uint64_t *file_size)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_create_b_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:__crfs_create_b_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0061);
    /*note: the reason of transfering hash func MD5_hash but not the path hash value to crfsnp_mgr_write_b */
    /*is to reduce some hash computation cost: if write failed, hash computation will not happen :-)*/
    if(EC_FALSE == crfsnp_mgr_write_b(CRFS_MD_NPP(crfs_md), file_path, file_size, MD5_hash))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0062);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_create_b_npp: crfsnp mgr read %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0063);

    return (EC_TRUE);
}

/**
*
*  create a big file at offset
*
**/
EC_BOOL crfs_create_b(const UINT32 crfs_md_id, const CSTRING *file_path, const uint64_t *file_size)
{
    CRFS_MD  *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_create_b: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    sys_log(LOGSTDOUT, "[DEBUG] crfs_create_b: file %s, size %ld\n", (char *)cstring_get_str(file_path), (*file_size));

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0064);

    if(EC_FALSE == __crfs_create_b_npp(crfs_md_id, file_path, file_size))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0065);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_b: create big file %s with size %ld to npp failed\n", 
                           (char *)cstring_get_str(file_path), (*file_size));
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0066);

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_create_b: create big file %s with size %ld to npp done\n", 
                       (char *)cstring_get_str(file_path), (*file_size));
    
    return (EC_TRUE);
}

/**
*
*  write a big file at offset
*
**/
EC_BOOL crfs_write_b(const UINT32 crfs_md_id, const CSTRING *file_path, uint64_t *offset, const CBYTES *cbytes)
{
    CRFS_MD  *crfs_md;
    
    uint32_t  crfsnp_id;
    uint32_t  parent_pos;
    UINT32    expires_timestamp;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write_b: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_write_b: file %s, offset %ld, data len %ld\n", 
                        (char *)cstring_get_str(file_path), (*offset), cbytes_len(cbytes));    

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0067);

    if(EC_FALSE == __crfs_read_b_npp(crfs_md_id, file_path, &crfsnp_id, &parent_pos, &expires_timestamp))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0068);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_b: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    /*parent_pos is the bnode*/
    
    if(EC_FALSE == __crfs_write_b_dn(crfs_md_id, crfsnp_id, parent_pos, offset, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0069);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_b: write dn at offset %ld failed\n", (*offset));
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0070);

    return (EC_TRUE);
}

/**
*
*  read a file from offset
*
**/
EC_BOOL crfs_read_b(const UINT32 crfs_md_id, const CSTRING *file_path, uint64_t *offset, const UINT32 max_len, CBYTES *cbytes, UINT32 *expires_timestamp, const EC_BOOL need_expired_content)
{
    CRFS_MD  *crfs_md;

    CRFSNP   *crfsnp;
    uint32_t  crfsnp_id;
    uint32_t  parent_pos;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_read_b: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0071);
  
    if(EC_FALSE == __crfs_read_b_npp(crfs_md_id, file_path, &crfsnp_id, &parent_pos, expires_timestamp))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0072);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_b: read file %s from npp failed\n", 
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    /*parent_pos is the bnode*/

    /**
    *
    * WARNING:
    * after unlock, crfsnp_fnode read from npp will be dangerous due to someone may delete the file without
    * notifying the reader, thus reader would read "dirty" data which is deleted yet in logical.
    *
    * but we can ignore and tolerant the short-term "dirty" data
    *
    **/

    if(NULL_PTR != expires_timestamp && (*expires_timestamp) > task_brd_default_get_time())
    {    
        if(do_log(SEC_0031_CRFS, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] crfs_read_b: read file %s [EXPIRED: %ld > %u] from npp\n", 
                               (char *)cstring_get_str(file_path),
                               (*expires_timestamp), task_brd_default_get_time());  
        }
        
        if(EC_FALSE == need_expired_content)
        {
            CRFS_UNLOCK(crfs_md, LOC_CRFS_0073);
            dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "crfs_read_b: not need to read expired file %s from dn\n", 
                               (char *)cstring_get_str(file_path));
            return (EC_FALSE);
        }        
    }    

    crfsnp = crfsnp_mgr_open_np(CRFS_MD_NPP(crfs_md), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0074);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_b: open crfsnp %u failed\n", crfsnp_id);
        return (EC_FALSE);
    }     

    if(EC_FALSE == __crfs_read_b_dn(crfs_md_id, crfsnp, parent_pos, offset, max_len, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0075);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_b: offset read file %s from dn failed\n", 
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0076);
    return (EC_TRUE);
}

/**
*
*  fetch block description from offset
*
**/
EC_BOOL crfs_fetch_block_fd_b(const UINT32 crfs_md_id, const CSTRING *file_path, const uint64_t offset, UINT32 *expires_timestamp, const EC_BOOL need_expired_content, uint32_t *block_size, int *block_fd)
{
    CRFS_MD  *crfs_md;

    CRFSNP   *crfsnp;
    uint32_t  crfsnp_id;
    uint32_t  parent_pos;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_fetch_block_fd_b: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0077);
  
    if(EC_FALSE == __crfs_read_b_npp(crfs_md_id, file_path, &crfsnp_id, &parent_pos, expires_timestamp))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0078);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_fetch_block_fd_b: read file %s from npp failed\n", 
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    /*parent_pos is the bnode*/

    /**
    *
    * WARNING:
    * after unlock, crfsnp_fnode read from npp will be dangerous due to someone may delete the file without
    * notifying the reader, thus reader would read "dirty" data which is deleted yet in logical.
    *
    * but we can ignore and tolerant the short-term "dirty" data
    *
    **/

    if(NULL_PTR != expires_timestamp && (*expires_timestamp) > task_brd_default_get_time())
    {    
        if(do_log(SEC_0031_CRFS, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] crfs_fetch_block_fd_b: fetch block of file %s [EXPIRED: %ld > %u] from npp\n", 
                               (char *)cstring_get_str(file_path),
                               (*expires_timestamp), task_brd_default_get_time());  
        }
        
        if(EC_FALSE == need_expired_content)
        {
            CRFS_UNLOCK(crfs_md, LOC_CRFS_0079);
            dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "crfs_fetch_block_fd_b: not need to fetch block of expired file %s from dn\n", 
                               (char *)cstring_get_str(file_path));
            return (EC_FALSE);
        }        
    }    

    crfsnp = crfsnp_mgr_open_np(CRFS_MD_NPP(crfs_md), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0080);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_fetch_block_fd_b: open crfsnp %u failed\n", crfsnp_id);
        return (EC_FALSE);
    }     

    if(EC_FALSE == __crfs_fetch_block_fd_b_dn(crfs_md_id, crfsnp, parent_pos, offset, block_size, block_fd))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0081);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_fetch_block_fd_b: offset fetch block of %s from dn failed\n", 
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0082);
    return (EC_TRUE);
}

EC_BOOL crfs_np_transfer(const UINT32 crfs_md_id, const UINT32 crfsnp_id, const CSTRING *remote_path)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_np_transfer: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    return (EC_TRUE);
}

EC_BOOL crfs_npp_transfer(const UINT32 crfs_md_id, const CSTRING *remote_path)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_npp_transfer: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    return (EC_TRUE);
}

EC_BOOL crfs_block_transfer(const UINT32 crfs_md_id, const UINT32 block_no, const CSTRING *remote_path)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_block_transfer: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    return (EC_TRUE);
}

EC_BOOL crfs_disk_transfer(const UINT32 crfs_md_id, const UINT32 disk_no, const CSTRING *remote_path)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_disk_transfer: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    return (EC_TRUE);
}

EC_BOOL crfs_dn_transfer(const UINT32 crfs_md_id, const CSTRING *remote_path)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_dn_transfer: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    return (EC_TRUE);
}

EC_BOOL crfs_vol_transfer(const UINT32 crfs_md_id, const CSTRING *remote_path)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_vol_transfer: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    return (EC_TRUE);
}


/**
*
*  transfer files from one RFS to another RFS based on file name hash value in consistency hash table
*
**/
static EC_BOOL __crfs_transfer_pre_of_np(const UINT32 crfs_md_id, const uint32_t crfsnp_id, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode)
{
    CRFS_MD      *crfs_md;

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0083);
    if(EC_FALSE == crfsnp_mgr_transfer_pre_np(CRFS_MD_NPP(crfs_md), crfsnp_id, dir_path, crfsdt_pnode))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0084);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_transfer_pre_of_np: transfer np %u prepare failed\n", crfsnp_id);
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0085);
    
    return (EC_TRUE);
}

static EC_BOOL __crfs_transfer_handle_of_np(const UINT32 crfs_md_id, const uint32_t crfsnp_id, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn)
{
    CRFS_MD      *crfs_md;

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    //CRFS_RDLOCK(crfs_md, LOC_CRFS_0086);
    if(EC_FALSE == crfsnp_mgr_transfer_handle_np(CRFS_MD_NPP(crfs_md), crfsnp_id, dir_path, crfsdt_pnode, crfsnp_trans_dn))
    {
        //CRFS_UNLOCK(crfs_md, LOC_CRFS_0087);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_transfer_handle_of_np: transfer np %u handle failed\n", crfsnp_id);
        return (EC_FALSE);
    }
    //CRFS_UNLOCK(crfs_md, LOC_CRFS_0088);
    
    return (EC_TRUE);
}

static EC_BOOL __crfs_transfer_post_of_np(const UINT32 crfs_md_id, const uint32_t crfsnp_id, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn)
{
    CRFS_MD      *crfs_md;

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    //CRFS_WRLOCK(crfs_md, LOC_CRFS_0089);
    if(EC_FALSE == crfsnp_mgr_transfer_post_np(CRFS_MD_NPP(crfs_md), crfsnp_id, dir_path, crfsdt_pnode, crfsnp_trans_dn))
    {
        //CRFS_UNLOCK(crfs_md, LOC_CRFS_0090);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_transfer_post_of_np: transfer np %u post clean failed\n", crfsnp_id);
        return (EC_FALSE);
    }
    //CRFS_UNLOCK(crfs_md, LOC_CRFS_0091);
    
    return (EC_TRUE);
}

EC_BOOL crfs_transfer(const UINT32 crfs_md_id, const UINT32 crfsc_md_id, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode)
{
    CRFS_MD      *crfs_md;
    CRFSNP_MGR   *crfsnp_mgr;
    uint32_t      crfsnp_id;

    CRFSNP_TRANS_DN crfsnp_trans_dn;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_transfer: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer: transfer beg\n");

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_mgr = CRFS_MD_NPP(crfs_md);
    if(NULL_PTR == crfsnp_mgr)
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_transfer: npp was not open\n");
        return (EC_FALSE);
    }    

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer: dn was not open\n");
        return (EC_FALSE);
    }  

    CRFSNP_TRANS_CRFS_MODI(&crfsnp_trans_dn)           = crfs_md_id;
    CRFSNP_TRANS_CRFSC_MODI(&crfsnp_trans_dn)          = crfsc_md_id;
    CRFSNP_TRANS_CRFS_READ_FILE(&crfsnp_trans_dn)      = crfs_read;
    CRFSNP_TRANS_CRFS_READ_FILE_B(&crfsnp_trans_dn)    = crfs_read_b;
    CRFSNP_TRANS_CRFSC_DELETE_FILE(&crfsnp_trans_dn)   = crfsc_delete_file;
    CRFSNP_TRANS_CRFSC_DELETE_FILE_B(&crfsnp_trans_dn) = crfsc_delete_file_b;    

    for(crfsnp_id = 0; crfsnp_id < CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr); crfsnp_id ++)
    {
        /*prepare*/
        if(EC_FALSE == __crfs_transfer_pre_of_np(crfs_md_id, crfsnp_id, dir_path, crfsdt_pnode))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer: transfer dir %s prepare in np %u failed\n", 
                               (char *)cstring_get_str(dir_path), crfsnp_id);
            return (EC_FALSE);                               
        }

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer: transfer dir %s prepare in np %u done\n", 
                           (char *)cstring_get_str(dir_path), crfsnp_id);

        /*handle*/
        if(EC_FALSE == __crfs_transfer_handle_of_np(crfs_md_id, crfsnp_id, dir_path, crfsdt_pnode, &crfsnp_trans_dn))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer: transfer dir %s handle in np %u failed\n", 
                               (char *)cstring_get_str(dir_path), crfsnp_id);
            return (EC_FALSE);                               
        }

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer: transfer dir %s handle in np %u done\n", 
                           (char *)cstring_get_str(dir_path), crfsnp_id);   

        /*post clean*/
        if(EC_FALSE == __crfs_transfer_post_of_np(crfs_md_id, crfsnp_id, dir_path, crfsdt_pnode, &crfsnp_trans_dn))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer: transfer dir %s post clean in np %u failed\n", 
                               (char *)cstring_get_str(dir_path), crfsnp_id);
            return (EC_FALSE);                               
        }

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer: transfer dir %s post clean in np %u done\n", 
                           (char *)cstring_get_str(dir_path), crfsnp_id);

        /*recycle*/
        if(EC_FALSE == __crfs_recycle_of_np(crfs_md_id, crfsnp_id))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer: transfer dir %s recycle in np %u failed\n", 
                               (char *)cstring_get_str(dir_path), crfsnp_id);
            return (EC_FALSE);
        }
        
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer: transfer dir %s recycle in np %u done\n", 
                           (char *)cstring_get_str(dir_path), crfsnp_id);        

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer: transfer dir %s in np %u done\n", 
                           (char *)cstring_get_str(dir_path), crfsnp_id);                           
    }
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer: transfer dir %s end\n", (char *)cstring_get_str(dir_path));
    
    return (EC_TRUE);
}

EC_BOOL crfs_transfer_pre(const UINT32 crfs_md_id, const UINT32 crfsc_md_id, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode)
{
    CRFS_MD      *crfs_md;
    CRFSNP_MGR   *crfsnp_mgr;
    uint32_t      crfsnp_id;
   
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_transfer_pre: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_pre: transfer beg\n");

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_mgr = CRFS_MD_NPP(crfs_md);
    if(NULL_PTR == crfsnp_mgr)
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_transfer_pre: npp was not open\n");
        return (EC_FALSE);
    }    

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer_pre: dn was not open\n");
        return (EC_FALSE);
    }  

    for(crfsnp_id = 0; crfsnp_id < CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr); crfsnp_id ++)
    {
        /*prepare*/
        if(EC_FALSE == __crfs_transfer_pre_of_np(crfs_md_id, crfsnp_id, dir_path, crfsdt_pnode))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer_pre: transfer dir %s prepare in np %u failed\n", 
                               (char *)cstring_get_str(dir_path), crfsnp_id);
            return (EC_FALSE);                               
        }

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_pre: transfer dir %s prepare in np %u done\n", 
                           (char *)cstring_get_str(dir_path), crfsnp_id);
     
    }
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_pre: transfer dir %s prepare end\n", (char *)cstring_get_str(dir_path));
    
    return (EC_TRUE);
}

EC_BOOL crfs_transfer_handle(const UINT32 crfs_md_id, const UINT32 crfsc_md_id, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode)
{
    CRFS_MD      *crfs_md;
    CRFSNP_MGR   *crfsnp_mgr;
    uint32_t      crfsnp_id;

    CRFSNP_TRANS_DN crfsnp_trans_dn;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_transfer_handle: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_handle: transfer beg\n");

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_mgr = CRFS_MD_NPP(crfs_md);
    if(NULL_PTR == crfsnp_mgr)
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_transfer_handle: npp was not open\n");
        return (EC_FALSE);
    }    

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer_handle: dn was not open\n");
        return (EC_FALSE);
    }  

    CRFSNP_TRANS_CRFS_MODI(&crfsnp_trans_dn)           = crfs_md_id;
    CRFSNP_TRANS_CRFSC_MODI(&crfsnp_trans_dn)          = crfsc_md_id;
    CRFSNP_TRANS_CRFS_READ_FILE(&crfsnp_trans_dn)      = crfs_read;
    CRFSNP_TRANS_CRFS_READ_FILE_B(&crfsnp_trans_dn)    = crfs_read_b;
    CRFSNP_TRANS_CRFSC_DELETE_FILE(&crfsnp_trans_dn)   = NULL_PTR;
    CRFSNP_TRANS_CRFSC_DELETE_FILE_B(&crfsnp_trans_dn) = NULL_PTR;    

    for(crfsnp_id = 0; crfsnp_id < CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr); crfsnp_id ++)
    {
        /*handle*/
        if(EC_FALSE == __crfs_transfer_handle_of_np(crfs_md_id, crfsnp_id, dir_path, crfsdt_pnode, &crfsnp_trans_dn))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer_handle: transfer dir %s handle in np %u failed\n", 
                               (char *)cstring_get_str(dir_path), crfsnp_id);
            return (EC_FALSE);                               
        }

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_handle: transfer dir %s handle in np %u done\n", 
                           (char *)cstring_get_str(dir_path), crfsnp_id);
         
    }
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_handle: transfer dir %s handle end\n", (char *)cstring_get_str(dir_path));
    
    return (EC_TRUE);
}

EC_BOOL crfs_transfer_post(const UINT32 crfs_md_id, const UINT32 crfsc_md_id, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode)
{
    CRFS_MD      *crfs_md;
    CRFSNP_MGR   *crfsnp_mgr;
    uint32_t      crfsnp_id;

    CRFSNP_TRANS_DN crfsnp_trans_dn;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_transfer_post: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_post: transfer beg\n");

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_mgr = CRFS_MD_NPP(crfs_md);
    if(NULL_PTR == crfsnp_mgr)
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_transfer_post: npp was not open\n");
        return (EC_FALSE);
    }    

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer_post: dn was not open\n");
        return (EC_FALSE);
    }  

    CRFSNP_TRANS_CRFS_MODI(&crfsnp_trans_dn)           = crfs_md_id;
    CRFSNP_TRANS_CRFSC_MODI(&crfsnp_trans_dn)          = crfsc_md_id;
    CRFSNP_TRANS_CRFS_READ_FILE(&crfsnp_trans_dn)      = NULL_PTR;
    CRFSNP_TRANS_CRFS_READ_FILE_B(&crfsnp_trans_dn)    = NULL_PTR;
    CRFSNP_TRANS_CRFSC_DELETE_FILE(&crfsnp_trans_dn)   = crfsc_delete_file;
    CRFSNP_TRANS_CRFSC_DELETE_FILE_B(&crfsnp_trans_dn) = crfsc_delete_file_b;    

    for(crfsnp_id = 0; crfsnp_id < CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr); crfsnp_id ++)
    {
        /*post clean*/
        if(EC_FALSE == __crfs_transfer_post_of_np(crfs_md_id, crfsnp_id, dir_path, crfsdt_pnode, &crfsnp_trans_dn))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer_post: transfer dir %s post clean in np %u failed\n", 
                               (char *)cstring_get_str(dir_path), crfsnp_id);
            return (EC_FALSE);                               
        }

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_post: transfer dir %s post clean in np %u done\n", 
                           (char *)cstring_get_str(dir_path), crfsnp_id);
    }
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_post: transfer dir %s post clean end\n", (char *)cstring_get_str(dir_path));
    
    return (EC_TRUE);
}

EC_BOOL crfs_transfer_recycle(const UINT32 crfs_md_id, const UINT32 crfsc_md_id, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode)
{
    CRFS_MD      *crfs_md;
    CRFSNP_MGR   *crfsnp_mgr;
    uint32_t      crfsnp_id;
   
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_transfer_recycle: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_recycle: transfer beg\n");

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_mgr = CRFS_MD_NPP(crfs_md);
    if(NULL_PTR == crfsnp_mgr)
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_transfer_recycle: npp was not open\n");
        return (EC_FALSE);
    }    

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer_recycle: dn was not open\n");
        return (EC_FALSE);
    }  

    for(crfsnp_id = 0; crfsnp_id < CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr); crfsnp_id ++)
    {
        /*recycle*/
        if(EC_FALSE == __crfs_recycle_of_np(crfs_md_id, crfsnp_id))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_transfer_recycle: transfer dir %s recycle in np %u failed\n", 
                               (char *)cstring_get_str(dir_path), crfsnp_id);
            return (EC_FALSE);
        }
        
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_recycle: transfer dir %s recycle in np %u done\n", 
                           (char *)cstring_get_str(dir_path), crfsnp_id);                           
    }
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_transfer_recycle: transfer dir %s recycle end\n", (char *)cstring_get_str(dir_path));
    
    return (EC_TRUE);
}

/**
*
*  create data node
*
**/
EC_BOOL crfs_create_dn(const UINT32 crfs_md_id, const CSTRING *root_dir)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_create_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    if(NULL_PTR != CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_dn: dn already exist\n");
        return (EC_FALSE);
    }

    CRFS_MD_DN(crfs_md) = crfsdn_create((char *)cstring_get_str(root_dir));
    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_create_dn: create dn failed\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_create_dn: crfs %ld try to add crfsdn cached nodes expirer\n", crfs_md_id);
    cbtimer_add(TASK_BRD_CBTIMER_LIST(task_brd_default_get()), 
               (UINT8 *)"CRFS_EXPIRE_DN", 
               CBTIMER_NEVER_EXPIRE, 
               CRFS_CHECK_DN_EXPIRE_IN_NSEC, 
               FI_crfs_expire_dn, crfs_md_id);

    return (EC_TRUE);
}

/**
*
*  add a disk to data node
*
**/
EC_BOOL crfs_add_disk(const UINT32 crfs_md_id, const UINT32 disk_no)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_add_disk: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_add_disk: dn not created yet\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == c_check_is_uint16_t(disk_no))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_add_disk: disk_no %u is invalid\n", disk_no);
        return (EC_FALSE);
    }

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0092);
    if(EC_FALSE == crfsdn_add_disk(CRFS_MD_DN(crfs_md), (uint16_t)disk_no))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0093);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_add_disk: add disk %u to dn failed\n", disk_no);
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0094);
    return (EC_TRUE);
}

/**
*
*  delete a disk from data node
*
**/
EC_BOOL crfs_del_disk(const UINT32 crfs_md_id, const UINT32 disk_no)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_del_disk: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_del_disk: dn not created yet\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == c_check_is_uint16_t(disk_no))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_del_disk: disk_no %u is invalid\n", disk_no);
        return (EC_FALSE);
    }

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0095);
    if(EC_FALSE == crfsdn_del_disk(CRFS_MD_DN(crfs_md), (uint16_t)disk_no))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0096);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_del_disk: del disk %u from dn failed\n", disk_no);
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0097);
    return (EC_TRUE);
}

/**
*
*  mount a disk to data node
*
**/
EC_BOOL crfs_mount_disk(const UINT32 crfs_md_id, const UINT32 disk_no)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_mount_disk: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_mount_disk: dn not created yet\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == c_check_is_uint16_t(disk_no))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_mount_disk: disk_no %u is invalid\n", disk_no);
        return (EC_FALSE);
    }

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0098);
    if(EC_FALSE == crfsdn_mount_disk(CRFS_MD_DN(crfs_md), (uint16_t)disk_no))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0099);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_mount_disk: mount disk %u to dn failed\n", disk_no);
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0100);
    return (EC_TRUE);
}

/**
*
*  umount a disk from data node
*
**/
EC_BOOL crfs_umount_disk(const UINT32 crfs_md_id, const UINT32 disk_no)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_umount_disk: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_umount_disk: dn not created yet\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == c_check_is_uint16_t(disk_no))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_umount_disk: disk_no %u is invalid\n", disk_no);
        return (EC_FALSE);
    }

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0101);
    if(EC_FALSE == crfsdn_umount_disk(CRFS_MD_DN(crfs_md), (uint16_t)disk_no))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0102);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_umount_disk: umount disk %u from dn failed\n", disk_no);
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0103);
    return (EC_TRUE);
}

/**
*
*  open data node
*
**/
EC_BOOL crfs_open_dn(const UINT32 crfs_md_id, const CSTRING *root_dir)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_open_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_open_dn: try to open dn %s  ...\n", (char *)cstring_get_str(root_dir));

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR != CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_open_dn: dn was open\n");
        return (EC_FALSE);
    }

    CRFS_MD_DN(crfs_md) = crfsdn_open((char *)cstring_get_str(root_dir));
    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_open_dn: open dn with root dir %s failed\n", (char *)cstring_get_str(root_dir));
        return (EC_FALSE);
    }
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_open_dn: open dn %s\n", (char *)cstring_get_str(root_dir));
    return (EC_TRUE);
}

/**
*
*  close data node
*
**/
EC_BOOL crfs_close_dn(const UINT32 crfs_md_id)
{
    CRFS_MD   *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_close_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_close_dn: no dn was open\n");
        return (EC_FALSE);
    }

    crfsdn_close(CRFS_MD_DN(crfs_md));
    CRFS_MD_DN(crfs_md) = NULL_PTR;
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_close_dn: dn was closed\n");

    return (EC_TRUE);
}

/**
*
*  export data into data node
*
**/
EC_BOOL crfs_export_dn(const UINT32 crfs_md_id, const CBYTES *cbytes, const CRFSNP_FNODE *crfsnp_fnode)
{
    CRFS_MD      *crfs_md;
    const CRFSNP_INODE *crfsnp_inode;

    UINT32   offset;
    UINT32   data_len;
    uint32_t size;

    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;    

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_export_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    data_len = DMIN(CBYTES_LEN(cbytes), CRFSNP_FNODE_FILESZ(crfsnp_fnode));

    if(CPGB_CACHE_MAX_BYTE_SIZE <= data_len)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_export_dn: CBYTES_LEN %u or CRFSNP_FNODE_FILESZ %u overflow\n", 
                            CBYTES_LEN(cbytes), CRFSNP_FNODE_FILESZ(crfsnp_fnode));
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_export_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    size = (uint32_t)data_len;

    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;    

    offset  = (((UINT32)(page_no)) << (CPGB_PAGE_4K_BIT_SIZE));
    if(EC_FALSE == crfsdn_write_o(CRFS_MD_DN(crfs_md), data_len, CBYTES_BUF(cbytes), disk_no, block_no, &offset))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_export_dn: write %u bytes to disk %u block %u page %u failed\n", 
                            data_len, disk_no, block_no, page_no);
        return (EC_FALSE);
    }
    //dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_export_dn: write %u bytes to disk %u block %u page %u done\n", 
    //                    data_len, disk_no, block_no, page_no);

    return (EC_TRUE);
}

/**
*
*  write data node
*
**/
EC_BOOL crfs_write_dn(const UINT32 crfs_md_id, const CBYTES *cbytes, CRFSNP_FNODE *crfsnp_fnode)
{
    CRFS_MD      *crfs_md;
    CRFSNP_INODE *crfsnp_inode;

    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;    

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CPGB_CACHE_MAX_BYTE_SIZE <= CBYTES_LEN(cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_dn: buff len (or file size) %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    crfsnp_fnode_init(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);    

    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0104);
    if(EC_FALSE == crfsdn_write_p(CRFS_MD_DN(crfs_md), cbytes_len(cbytes), cbytes_buf(cbytes), &disk_no, &block_no, &page_no))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0105);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_dn: write %u bytes to dn failed\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0106);

    CRFSNP_INODE_CACHE_FLAG(crfsnp_inode) = CRFSDN_DATA_NOT_IN_CACHE;
    CRFSNP_INODE_DISK_NO(crfsnp_inode)    = disk_no;
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode)   = block_no;
    CRFSNP_INODE_PAGE_NO(crfsnp_inode)    = page_no;

    CRFSNP_FNODE_FILESZ(crfsnp_fnode) = CBYTES_LEN(cbytes);
    CRFSNP_FNODE_REPNUM(crfsnp_fnode) = 1;

    return (EC_TRUE);
}

/**
*
*  write data node in cache
*
**/
EC_BOOL crfs_write_dn_cache(const UINT32 crfs_md_id, const CBYTES *cbytes, CRFSNP_FNODE *crfsnp_fnode)
{
    CRFS_MD      *crfs_md;
    CRFSNP_INODE *crfsnp_inode;

    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;    

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write_dn_cache: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CPGB_CACHE_MAX_BYTE_SIZE <= CBYTES_LEN(cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_dn_cache: buff len (or file size) %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_dn_cache: no dn was open\n");
        return (EC_FALSE);
    }    

    crfsnp_fnode_init(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    
    if(EC_FALSE == crfsdn_write_p_cache(CRFS_MD_DN(crfs_md), cbytes_len(cbytes), cbytes_buf(cbytes), &disk_no, &block_no, &page_no))
    {       
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_dn_cache: write %u bytes to dn failed\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }
    
    CRFSNP_INODE_CACHE_FLAG(crfsnp_inode) = CRFSDN_DATA_IS_IN_CACHE;
    CRFSNP_INODE_DISK_NO(crfsnp_inode)    = disk_no;
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode)   = block_no;
    CRFSNP_INODE_PAGE_NO(crfsnp_inode)    = page_no;

    CRFSNP_FNODE_FILESZ(crfsnp_fnode) = CBYTES_LEN(cbytes);
    CRFSNP_FNODE_REPNUM(crfsnp_fnode) = 1;

    return (EC_TRUE);
}

/**
*
*  read data node
*
**/
EC_BOOL crfs_read_dn(const UINT32 crfs_md_id, const CRFSNP_FNODE *crfsnp_fnode, CBYTES *cbytes)
{
    CRFS_MD *crfs_md;
    const CRFSNP_INODE *crfsnp_inode;

    uint32_t file_size;
    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_read_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_dn: dn is null\n");
        return (EC_FALSE);
    }

    if(0 == CRFSNP_FNODE_REPNUM(crfsnp_fnode))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_dn: no replica\n");
        return (EC_FALSE);
    }

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;

    //dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_read_dn: file size %u, disk %u, block %u, page %u\n", file_size, disk_no, block_no, page_no);

    if(CBYTES_LEN(cbytes) < file_size)
    {
        if(NULL_PTR != CBYTES_BUF(cbytes))
        {
            SAFE_FREE(CBYTES_BUF(cbytes), LOC_CRFS_0107);
        }
        CBYTES_BUF(cbytes) = (UINT8 *)SAFE_MALLOC(file_size, LOC_CRFS_0108);
        ASSERT(NULL_PTR != CBYTES_BUF(cbytes));
        CBYTES_LEN(cbytes) = 0;
    }

    crfsdn_rdlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0109);
    if(EC_FALSE == crfsdn_read_p(CRFS_MD_DN(crfs_md), disk_no, block_no, page_no, file_size, CBYTES_BUF(cbytes), &(CBYTES_LEN(cbytes))))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0110);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_dn: read %u bytes from disk %u, block %u, page %u failed\n", 
                           file_size, disk_no, block_no, page_no);
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0111);
    return (EC_TRUE);
}

/**
*
*  write data node at offset in the specific file
*
**/
EC_BOOL crfs_write_e_dn(const UINT32 crfs_md_id, CRFSNP_FNODE *crfsnp_fnode, UINT32 *offset, const UINT32 max_len, const CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;
    CRFSNP_INODE *crfsnp_inode;

    uint32_t file_size;
    uint32_t file_max_size;
    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;    
    uint32_t offset_t;

    UINT32   max_len_t;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write_e_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CPGB_CACHE_MAX_BYTE_SIZE <= (*offset) + CBYTES_LEN(cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_e_dn: offset %u + buff len (or file size) %u = %u overflow\n", 
                            (*offset), CBYTES_LEN(cbytes), (*offset) + CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_e_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;

    /*file_max_size = file_size alignment to 4KB*/
    file_max_size = (((file_size + CPGB_PAGE_4K_BYTE_SIZE - 1) >> CPGB_PAGE_4K_BIT_SIZE) << CPGB_PAGE_4K_BIT_SIZE);
    
    if(((UINT32)file_max_size) <= (*offset))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_e_dn: offset %u overflow due to file max size is %u\n", (*offset), file_max_size);
        return (EC_FALSE);
    }    
    
    offset_t  = (uint32_t)(*offset);
    max_len_t = DMIN(DMIN(max_len, file_max_size - offset_t), cbytes_len(cbytes));    

    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0112);
    if(EC_FALSE == crfsdn_write_e(CRFS_MD_DN(crfs_md), max_len_t, cbytes_buf(cbytes), disk_no, block_no, page_no, offset_t))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0113);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_e_dn: write %u bytes to dn failed\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0114);

    (*offset) += max_len_t;
    if((*offset) > file_size)
    {
        /*update file size info*/
        CRFSNP_FNODE_FILESZ(crfsnp_fnode) = (uint32_t)(*offset);
    }    

    return (EC_TRUE);
}

/**
*
*  read data node from offset in the specific file
*
**/
EC_BOOL crfs_read_e_dn(const UINT32 crfs_md_id, const CRFSNP_FNODE *crfsnp_fnode, UINT32 *offset, const UINT32 max_len, CBYTES *cbytes)
{
    CRFS_MD *crfs_md;
    const CRFSNP_INODE *crfsnp_inode;

    uint32_t file_size;
    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;
    uint32_t offset_t;
    
    UINT32   max_len_t;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_read_e_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_e_dn: dn is null\n");
        return (EC_FALSE);
    }

    if(0 == CRFSNP_FNODE_REPNUM(crfsnp_fnode))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_e_dn: no replica\n");
        return (EC_FALSE);
    }

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;    

    if((*offset) >= file_size)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_e_dn: due to offset %u >= file size %u\n", (*offset), file_size);
        return (EC_FALSE);
    }

    offset_t = (uint32_t)(*offset);
    if(0 == max_len)
    {
        max_len_t = file_size - offset_t;
    }
    else
    {
        max_len_t = DMIN(max_len, file_size - offset_t);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_read_e_dn: file size %u, disk %u, block %u, page %u, offset %u, max len %u\n", 
                        file_size, disk_no, block_no, page_no, offset_t, max_len_t);

    if(CBYTES_LEN(cbytes) < max_len_t)
    {
        if(NULL_PTR != CBYTES_BUF(cbytes))
        {
            SAFE_FREE(CBYTES_BUF(cbytes), LOC_CRFS_0115);
        }
        CBYTES_BUF(cbytes) = (UINT8 *)SAFE_MALLOC(max_len_t, LOC_CRFS_0116);
        CBYTES_LEN(cbytes) = 0;
    }    

    crfsdn_rdlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0117);
    if(EC_FALSE == crfsdn_read_e(CRFS_MD_DN(crfs_md), disk_no, block_no, page_no, offset_t, max_len_t, CBYTES_BUF(cbytes), &(CBYTES_LEN(cbytes))))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0118);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_read_e_dn: read %u bytes from disk %u, block %u, offset %u failed\n", 
                           max_len_t, disk_no, block_no, offset_t);
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0119);

    (*offset) += CBYTES_LEN(cbytes);
    return (EC_TRUE);
}

/*when found missed segment in bnode*/
static EC_BOOL __crfs_write_b_seg_dn_miss(const UINT32 crfs_md_id, 
                                              CRFSNP *crfsnp, 
                                              const uint32_t parent_pos, 
                                              UINT32 *offset, const CBYTES *cbytes,
                                              const uint32_t key_2nd_hash, 
                                              const uint32_t klen, const uint8_t *key,
                                              uint32_t *node_pos)
{
    CRFS_MD      *crfs_md;
    
    uint16_t      disk_no;
    uint16_t      block_no;
    uint16_t      page_no;
    uint32_t      insert_offset;

    UINT32        data_len;
    UINT32        zero_len;

    CRFSNP_ITEM  *crfsnp_item_insert;
    CRFSNP_FNODE *crfsnp_fnode;    
    CRFSNP_INODE *crfsnp_inode;    

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_write_b_seg_dn_miss: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CPGB_CACHE_MAX_BYTE_SIZE < CBYTES_LEN(cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn_miss: buff len %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }   

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn_miss: offset %u overflow\n", (*offset));
        return (EC_FALSE);
    }     
    
    zero_len = (*offset);/*save offset*/
    data_len = DMIN(CPGB_CACHE_MAX_BYTE_SIZE - zero_len, cbytes_len(cbytes));
    
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0120);
    if(EC_FALSE == crfsdn_write_b(CRFS_MD_DN(crfs_md), data_len, cbytes_buf(cbytes), &disk_no, &block_no, offset))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0121);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn_miss: write %u bytes to dn failed\n", data_len);
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0122);

    insert_offset = crfsnp_bnode_insert(crfsnp, parent_pos, key_2nd_hash, klen, key, CRFSNP_ITEM_FILE_IS_REG);
    if(CRFSNPRB_ERR_POS == insert_offset)
    {        
        page_no = 0;
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn_miss: write %s to bnode in npp failed\n", (char *)key);
        crfsdn_remove(CRFS_MD_DN(crfs_md), disk_no, block_no, page_no, CPGB_CACHE_MAX_BYTE_SIZE);
        return (EC_FALSE);
    }

    crfsnp_item_insert = crfsnp_fetch(crfsnp, insert_offset);
    crfsnp_fnode = CRFSNP_ITEM_FNODE(crfsnp_item_insert);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    page_no      = 0;
        
    CRFSNP_INODE_DISK_NO(crfsnp_inode)  = disk_no;
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode) = block_no;
    CRFSNP_INODE_PAGE_NO(crfsnp_inode)  = page_no;

    CRFSNP_FNODE_FILESZ(crfsnp_fnode) = (*offset);
    CRFSNP_FNODE_REPNUM(crfsnp_fnode) = 1;

    (*node_pos) = insert_offset;
    
    return (EC_TRUE);
}

static EC_BOOL __crfs_write_b_seg_dn_hit(const UINT32 crfs_md_id, 
                                           CRFSNP *crfsnp, 
                                           const uint32_t node_pos, 
                                           UINT32 *offset, const CBYTES *cbytes,
                                           const uint32_t key_2nd_hash, 
                                           const uint32_t klen, const uint8_t *key)
{
    CRFS_MD      *crfs_md;

    UINT32        data_len;
    UINT32        skip_len;

    CRFSNP_ITEM  *crfsnp_item;
    CRFSNP_FNODE *crfsnp_fnode;    
    CRFSNP_INODE *crfsnp_inode;    

    uint16_t      disk_no;
    uint16_t      block_no;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_write_b_seg_dn_hit: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CPGB_CACHE_MAX_BYTE_SIZE < CBYTES_LEN(cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn_hit: buff len %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }   

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn_hit: offset %u overflow\n", (*offset));
        return (EC_FALSE);
    }     

    skip_len = (*offset);/*save offset*/
    data_len = DMIN(CPGB_CACHE_MAX_BYTE_SIZE - skip_len, cbytes_len(cbytes));
        
    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    ASSERT(NULL_PTR != crfsnp_item);
    ASSERT(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item));    

    crfsnp_fnode = CRFSNP_ITEM_FNODE(crfsnp_item);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);    

    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode);
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0123);            
    if(EC_FALSE == crfsdn_update_b(CRFS_MD_DN(crfs_md), data_len, cbytes_buf(cbytes), disk_no, block_no, offset))
    {  
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0124);  
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:__crfs_write_b_seg_dn_hit: write %u bytes of disk %u block %u offset %u failed\n", data_len, disk_no, block_no, skip_len);
        return (EC_FALSE);
    }
    else
    {
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG]__crfs_write_b_seg_dn_hit: write %u bytes of disk %u block %u offset %u done\n", data_len, disk_no, block_no, skip_len);
    }    
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0125);  

    if(CRFSNP_FNODE_FILESZ(crfsnp_fnode) < (*offset))
    {
        CRFSNP_FNODE_FILESZ(crfsnp_fnode) = (*offset);/*adjust file size*/
    }

    return (EC_TRUE);
}

static EC_BOOL __crfs_write_b_seg_dn(const UINT32 crfs_md_id, 
                                             CRFSNP *crfsnp, const uint32_t parent_pos, 
                                             const uint32_t seg_no, UINT32 *offset, 
                                             const CBYTES *cbytes)
{
    CRFS_MD *crfs_md;

    CRFSNP_ITEM  *crfsnp_item_parent;
    CRFSNP_BNODE *crfsnp_bnode;    
    
    uint8_t  key[32];
    uint32_t klen;    

    uint32_t key_2nd_hash;

    uint32_t node_pos;

    UINT32 offset_t1;
    UINT32 offset_t2;
    UINT32 burn_len;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_write_b_seg_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);    

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn: no npp was open\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn: offset %u overflow\n", (*offset));
        return (EC_FALSE);
    }      

    /*okay, limit buff len <= 64MB*/
    if(CPGB_CACHE_MAX_BYTE_SIZE < CBYTES_LEN(cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn: buff len %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }    

    if(0 == CBYTES_LEN(cbytes))
    {
        return (EC_TRUE);
    }     

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn: fetch parent item failed where parent offset %u\n", parent_pos);
        return (EC_FALSE);
    }

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent) 
    || CRFSNP_ITEM_IS_NOT_USED == CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent),
                            CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent));
        return (EC_FALSE);
    }     

    crfsnp_make_b_seg_key(seg_no, key, sizeof(key), &klen);

    //key_2nd_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, klen, key);
    key_2nd_hash = 0;/*trick*/
    node_pos     = crfsnp_bnode_search(crfsnp, crfsnp_bnode, key_2nd_hash, klen, key);    

    if(CRFSNPRB_ERR_POS == node_pos)/*Oops! the segment missed*/
    {
        offset_t1 = (*offset);
        offset_t2 = offset_t1;
        
        if(EC_FALSE == __crfs_write_b_seg_dn_miss(crfs_md_id, crfsnp, 
                                              parent_pos, 
                                              &offset_t1, cbytes,
                                              key_2nd_hash, 
                                              klen, key,
                                              &node_pos))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn: write data to missed segment failed\n");
            return (EC_FALSE);
        }

        burn_len   = (offset_t1 - offset_t2);
        (*offset) += burn_len; /*update offset*/
        CRFSNP_BNODE_STORESZ(crfsnp_bnode) += burn_len;
    }
    else/*the segment hit*/
    {
        offset_t1 = (*offset);
        offset_t2 = offset_t1;
        
        if(EC_FALSE == __crfs_write_b_seg_dn_hit(crfs_md_id, crfsnp, 
                                              node_pos, 
                                              &offset_t1, cbytes,
                                              key_2nd_hash, 
                                              klen, key))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn: write data to hit segment failed\n");
            return (EC_FALSE);
        }

        burn_len   = (offset_t1 - offset_t2);
        (*offset) += burn_len; /*update offset*/
        CRFSNP_BNODE_STORESZ(crfsnp_bnode) += burn_len;
    }

    if(SWITCH_ON == CRFS_MD5_SWITCH)
    {
        if(0 < burn_len 
        && (
              CRFSNP_BNODE_STORESZ(crfsnp_bnode) == CRFSNP_BNODE_FILESZ(crfsnp_bnode)
           || 0 == (CRFSNP_BNODE_STORESZ(crfsnp_bnode) % CPGB_CACHE_MAX_BYTE_SIZE )
           )
         )
        {
            CRFSNP_ITEM *crfsnp_item;
            CRFSNP_FNODE*crfsnp_fnode;
            CBYTES       cbytes_t;
            uint64_t     offset_t3;
            uint32_t     data_len;            
            
            crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
            crfsnp_fnode = CRFSNP_ITEM_FNODE(crfsnp_item);

            data_len = (uint32_t)(CRFSNP_BNODE_STORESZ(crfsnp_bnode) & (CPGB_CACHE_MAX_BYTE_SIZE - 1));
            if(0 == data_len)
            {
                data_len = CPGB_CACHE_MAX_BYTE_SIZE;
            }
            ASSERT(data_len == CRFSNP_FNODE_FILESZ(crfsnp_fnode));
            offset_t3 = CRFSNP_BNODE_STORESZ(crfsnp_bnode) - data_len;
            cbytes_init(&cbytes_t);

            if(EC_FALSE == __crfs_read_b_dn(crfs_md_id, crfsnp, parent_pos, &offset_t3, data_len, &cbytes_t))
            {
                cbytes_clean(&cbytes_t, LOC_CRFS_0126);
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_seg_dn: offset read file from dn failed where seg_no %u\n", 
                                    seg_no);
                return (EC_FALSE);
            }

            ASSERT(data_len == CBYTES_LEN(&cbytes_t));
            cmd5_sum(data_len, CBYTES_BUF(&cbytes_t), CRFSNP_FNODE_MD5SUM(crfsnp_fnode));
            cbytes_clean(&cbytes_t, LOC_CRFS_0127);
        }
    }
    
    return (EC_TRUE);        
}

/**
*
*  write data node at offset in the specific big file
*
**/
static EC_BOOL __crfs_write_b_dn(const UINT32 crfs_md_id, 
                                      const uint32_t crfsnp_id, 
                                      const uint32_t parent_pos, 
                                      uint64_t *offset, 
                                      const CBYTES *cbytes)
{
    CRFS_MD  *crfs_md;
    CRFSNP   *crfsnp;

    CRFSNP_ITEM  *crfsnp_item_parent;
    CRFSNP_BNODE *crfsnp_bnode;

    UINT32   content_len;    
    UINT8   *content_buf;
    UINT32   offset_diff;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_write_b_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md     = CRFS_MD_GET(crfs_md_id);

    if(CRFS_BIGFILE_MAX_SIZE <= (*offset))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_dn: offset %ld overflow\n", (*offset));
        return (EC_FALSE);
    }    

    crfsnp = crfsnp_mgr_open_np(CRFS_MD_NPP(crfs_md), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_dn: open crfsnp %u failed\n", crfsnp_id);
        return (EC_FALSE);
    }     

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_dn: fetch parent item failed where parent offset %u\n", parent_pos);
        return (EC_FALSE);
    }

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent) 
    || CRFSNP_ITEM_IS_NOT_USED == CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent),
                            CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent));
        return (EC_FALSE);
    }     

    content_buf = CBYTES_BUF(cbytes);
    content_len = cbytes_len(cbytes);

    if(CPGB_CACHE_MAX_BYTE_SIZE < content_len)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_dn: input content len %ld overflow\n", content_len);
        return (EC_FALSE);
    }

    /*now content_len <= 64MB was granted*/

    /*adjust offset and content_len*/
    if((*offset) > CRFSNP_BNODE_STORESZ(crfsnp_bnode))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_dn: offset %ld > store size %ld\n", 
                            (*offset), CRFSNP_BNODE_STORESZ(crfsnp_bnode));
        return (EC_FALSE);
    }

    if((*offset) + content_len <= CRFSNP_BNODE_STORESZ(crfsnp_bnode))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_dn: content was ready, offset %ld + content len %ld <= store size %ld\n", 
                            (*offset), content_len, CRFSNP_BNODE_STORESZ(crfsnp_bnode));
        return (EC_FALSE);
    }

    /*now offset <= store size < offset + content_len*/
    offset_diff  = (UINT32)(CRFSNP_BNODE_STORESZ(crfsnp_bnode) - (*offset));
    (*offset)    = CRFSNP_BNODE_STORESZ(crfsnp_bnode);
    content_len -= offset_diff;
    content_buf += offset_diff;
    
    if(CPGB_CACHE_MAX_BYTE_SIZE < content_len)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_dn: after adjust, content len %ld overflow\n", content_len);
        return (EC_FALSE);
    }     

    for(; 0 < content_len;)
    {
        CBYTES   cbytes_t;
        UINT32   offset_t1;
        UINT32   offset_t2;
        UINT32   burn_len;
        uint32_t seg_no;

        offset_t1 = (UINT32)((*offset) % CPGB_CACHE_MAX_BYTE_SIZE);
        offset_t2 = offset_t1;
        seg_no    = (uint32_t)((*offset) >> CPGB_CACHE_BIT_SIZE);
        
        cbytes_mount(&cbytes_t, content_len, content_buf);
        
        if(EC_FALSE == __crfs_write_b_seg_dn(crfs_md_id, crfsnp, parent_pos, seg_no, &offset_t1, &cbytes_t))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_write_b_dn: write to seg_no %u at offset %u failed\n", seg_no, offset_t2);
            return (EC_FALSE);
        }

        burn_len     = (offset_t1 - offset_t2);
        content_len -= burn_len;
        content_buf += burn_len;
        (*offset)   += burn_len;
    }     
    
    return (EC_TRUE);
}

static EC_BOOL __crfs_read_b_seg_dn_miss(const UINT32 crfs_md_id, 
                                                    UINT32 *offset, 
                                                    const UINT32 max_len, CBYTES *cbytes)
{
    UINT32 read_len;
    
    if(CPGB_CACHE_MAX_BYTE_SIZE < max_len)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn_miss: max_len %u overflow\n", max_len);
        return (EC_FALSE);
    }

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn_miss: offset %u overflow\n", (*offset));
        return (EC_FALSE);
    }    
    
    read_len = DMIN(max_len, CPGB_CACHE_MAX_BYTE_SIZE - (*offset));
    BSET(CBYTES_BUF(cbytes), CRFS_FILE_PAD_CHAR, read_len);

    (*offset) += read_len;
    return (EC_TRUE);
}

static EC_BOOL __crfs_read_b_seg_dn_hit(const UINT32 crfs_md_id, 
                                                CRFSNP *crfsnp, 
                                                const uint32_t node_pos, 
                                                UINT32 *offset, 
                                                CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;

    UINT32        file_size;
    UINT32        read_len;
 
    CRFSNP_ITEM  *crfsnp_item;
    CRFSNP_FNODE *crfsnp_fnode;    

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_read_b_seg_dn_hit: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CPGB_CACHE_MAX_BYTE_SIZE < CBYTES_LEN(cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn_hit: buff len %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }   

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn_hit: offset %u overflow\n", (*offset));
        return (EC_FALSE);
    }     
       
    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    ASSERT(NULL_PTR != crfsnp_item);
    ASSERT(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item));    

    crfsnp_fnode = CRFSNP_ITEM_FNODE(crfsnp_item);

    file_size = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    if(file_size < (*offset))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn_hit: offset %u > file_size %u\n", (*offset), file_size);
        return (EC_FALSE);
    }     

    read_len = DMIN(file_size - (*offset), cbytes_len(cbytes));/*the buffer to accept the read data was ready in cbytes*/

    /*note: CRFS_MD_DN(crfs_md) will be locked in crfs_read_e_dn*/
    if(0 < read_len && EC_FALSE == crfs_read_e_dn(crfs_md_id, crfsnp_fnode, offset, read_len, cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn_hit: read %u bytes from dn failed\n", read_len);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

static EC_BOOL __crfs_read_b_seg_dn(const UINT32 crfs_md_id, 
                                            CRFSNP *crfsnp, 
                                            const uint32_t parent_pos, 
                                            const uint32_t seg_no, UINT32 *offset, 
                                            const UINT32 max_len, CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;

    CRFSNP_ITEM  *crfsnp_item_parent;
    CRFSNP_BNODE *crfsnp_bnode;

    UINT32   max_len_t;

    uint8_t  key[32];
    uint32_t klen;

    uint32_t key_2nd_hash;

    uint32_t node_pos;

    CBYTES   cbytes_t;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_read_b_seg_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    /*okay, limit buff len <= 64MB*/
    if(CPGB_CACHE_MAX_BYTE_SIZE < max_len)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn: max_len %u overflow\n", max_len);
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn: no npp was open\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn: fetch parent item failed where parent offset %u\n", parent_pos);
        return (EC_FALSE);
    }
    
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent) 
    || CRFSNP_ITEM_IS_NOT_USED == CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent),
                            CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent));
        return (EC_FALSE);
    }    

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);

    max_len_t = DMIN(max_len, CBYTES_LEN(cbytes));

    /*init cbytes_t*/
    cbytes_mount(&cbytes_t, max_len_t, CBYTES_BUF(cbytes));    

    crfsnp_make_b_seg_key(seg_no, key, sizeof(key), &klen);

    //key_2nd_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, klen, key);
    key_2nd_hash = 0;/*trick*/
    node_pos     = crfsnp_bnode_search(crfsnp, crfsnp_bnode, key_2nd_hash, klen, key);

    if(CRFSNPRB_ERR_POS == node_pos)/*Oops! the segment missed*/
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn: read data from seg %u but missed\n", seg_no);
        return (EC_FALSE);
    }
    else/*the segment hit*/
    {
        UINT32 offset_t1;
        UINT32 offset_t2;
        UINT32 read_len;

        offset_t1 = (*offset);
        offset_t2 = offset_t1;
        
        if(EC_FALSE == __crfs_read_b_seg_dn_hit(crfs_md_id, crfsnp, node_pos, &offset_t1, &cbytes_t))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_seg_dn: write data to hit segment failed\n");
            return (EC_FALSE);
        }

        read_len = (offset_t1 - offset_t2);
        (*offset) += read_len; /*update offset*/
    }

    return (EC_TRUE);
}

/**
*
*  read data node at offset from the specific big file
*
**/
static EC_BOOL __crfs_read_b_dn(const UINT32 crfs_md_id, CRFSNP *crfsnp, const uint32_t parent_pos, uint64_t *offset, const UINT32 max_len, CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;

    CRFSNP_ITEM  *crfsnp_item_parent;
    CRFSNP_BNODE *crfsnp_bnode;

    uint64_t file_size;
    uint64_t store_size;

    UINT32   max_len_t;
    UINT8   *content_buf;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_read_b_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    /*okay, limit buff len <= 64MB*/
    if(CPGB_CACHE_MAX_BYTE_SIZE < max_len)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_dn: max_len %u overflow\n", max_len);
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_dn: no npp was open\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_dn: fetch parent item failed where parent pos %u\n", parent_pos);
        return (EC_FALSE);
    }

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent) 
    || CRFSNP_ITEM_IS_NOT_USED == CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent),
                            CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent));
        return (EC_FALSE);
    }    

    file_size  = CRFSNP_BNODE_FILESZ(crfsnp_bnode);
    store_size = CRFSNP_BNODE_STORESZ(crfsnp_bnode);

    if((*offset) == store_size)
    {
        return (EC_TRUE);
    }
    
    if((*offset) > store_size)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_dn: offset %ld >= store_size = %ld, file_size = %ld\n", 
                            (*offset), store_size, file_size);
        return (EC_FALSE);
    }       

    if((*offset) + max_len >= store_size)
    {
        max_len_t = (UINT32)(store_size - (*offset));
    }
    else
    {
        max_len_t = max_len;
    } 

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] __crfs_read_b_dn: file_size %ld, store_size %ld, max_len %u, max_len_t %u, offset %ld\n", 
                        file_size, store_size, max_len, max_len_t, (*offset));

    if(CBYTES_LEN(cbytes) < max_len_t)
    {
        if(NULL_PTR != CBYTES_BUF(cbytes))
        {
            SAFE_FREE(CBYTES_BUF(cbytes), LOC_CRFS_0128);
        }
        CBYTES_BUF(cbytes) = (UINT8 *)SAFE_MALLOC(max_len_t, LOC_CRFS_0129);
        CBYTES_LEN(cbytes) = max_len_t;/*initialize to the final size ...*/

        if(NULL_PTR == CBYTES_BUF(cbytes))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_dn: malloc %u bytes for cbytes failed\n", max_len_t);
            return (EC_FALSE);
        }
    }     

    content_buf = CBYTES_BUF(cbytes);

    for(; 0 < max_len_t;)
    {
        CBYTES   cbytes_t;
        UINT32   offset_t1;
        UINT32   offset_t2;
        UINT32   read_len;
        uint32_t seg_no;

        offset_t1 = (UINT32)((*offset) % CPGB_CACHE_MAX_BYTE_SIZE);
        offset_t2 = offset_t1;
        seg_no    = (uint32_t)((*offset) >> CPGB_CACHE_BIT_SIZE);
        
        cbytes_mount(&cbytes_t, max_len_t, content_buf);

        if(EC_FALSE == __crfs_read_b_seg_dn(crfs_md_id, crfsnp, parent_pos, seg_no, &offset_t1, max_len_t, &cbytes_t))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_dn: read from seg_no %u at offset %u failed\n", 
                                seg_no, offset_t2);
            return (EC_FALSE);
        }

        read_len     = (offset_t1 - offset_t2);
        max_len_t   -= read_len;
        content_buf += read_len;
        (*offset)   += read_len;
    }

    return (EC_TRUE);
}

static EC_BOOL __crfs_fetch_block_fd_b_seg_dn_hit(const UINT32 crfs_md_id, 
                                                CRFSNP *crfsnp, 
                                                const uint32_t node_pos, 
                                                uint32_t *block_size,
                                                int *block_fd)
{
    CRFS_MD      *crfs_md;
 
    CRFSNP_ITEM  *crfsnp_item;
    CRFSNP_FNODE *crfsnp_fnode;    
    CRFSNP_INODE *crfsnp_inode;

    uint32_t file_size;
    uint16_t disk_no;
    uint16_t block_no;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_fetch_block_fd_b_seg_dn_hit: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
       
    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    ASSERT(NULL_PTR != crfsnp_item);
    ASSERT(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item));    

    crfsnp_fnode = CRFSNP_ITEM_FNODE(crfsnp_item);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    disk_no      = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no     = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);

    /*note: CRFS_MD_DN(crfs_md) will be locked in crfs_read_e_dn*/
    if(EC_FALSE == crfsdn_fetch_block_fd(CRFS_MD_DN(crfs_md), disk_no, block_no, block_fd))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_seg_dn_hit: fetch block fd of disk %u block %u failed\n", disk_no, block_no);
        return (EC_FALSE);
    }

    (*block_size) = file_size;
    return (EC_TRUE);
}

static EC_BOOL __crfs_fetch_block_fd_b_seg_dn(const UINT32 crfs_md_id, 
                                            CRFSNP *crfsnp, 
                                            const uint32_t parent_pos, 
                                            const uint32_t seg_no, 
                                            uint32_t *block_size,
                                            int *block_fd
                                            )
{
    CRFS_MD      *crfs_md;

    CRFSNP_ITEM  *crfsnp_item_parent;
    CRFSNP_BNODE *crfsnp_bnode;

    uint8_t  key[32];
    uint32_t klen;

    uint32_t key_2nd_hash;

    uint32_t node_pos;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_fetch_block_fd_b_seg_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_seg_dn: no npp was open\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_seg_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_seg_dn: fetch parent item failed where parent offset %u\n", parent_pos);
        return (EC_FALSE);
    }

    
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent) 
    || CRFSNP_ITEM_IS_NOT_USED == CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_seg_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent),
                            CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent));
        return (EC_FALSE);
    }    

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    
    crfsnp_make_b_seg_key(seg_no, key, sizeof(key), &klen);

    //key_2nd_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, klen, key);
    key_2nd_hash = 0;/*trick*/
    node_pos     = crfsnp_bnode_search(crfsnp, crfsnp_bnode, key_2nd_hash, klen, key);
    if(CRFSNPRB_ERR_POS == node_pos)/*Oops! the segment missed*/
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_seg_dn: fetch block from seg %u but missed\n", seg_no);
        return (EC_FALSE);
    }
    /*the segment hit*/
    if(EC_FALSE == __crfs_fetch_block_fd_b_seg_dn_hit(crfs_md_id, crfsnp, node_pos, block_size, block_fd))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_seg_dn: fetch block from hit seg %u failed\n", seg_no);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

static EC_BOOL __crfs_fetch_block_fd_b_dn(const UINT32 crfs_md_id, 
                                             CRFSNP *crfsnp, 
                                             const uint32_t parent_pos, 
                                             const uint64_t offset, 
                                             uint32_t *block_size, 
                                             int *block_fd)
{
    CRFS_MD      *crfs_md;

    CRFSNP_ITEM  *crfsnp_item_parent;
    CRFSNP_BNODE *crfsnp_bnode;

    uint64_t file_size;
    uint64_t store_size;

    uint32_t seg_no;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_fetch_block_fd_b_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_dn: no npp was open\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_dn: fetch parent item failed where parent pos %u\n", parent_pos);
        return (EC_FALSE);
    }

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent) 
    || CRFSNP_ITEM_IS_NOT_USED == CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DIR_FLAG(crfsnp_item_parent),
                            CRFSNP_ITEM_USED_FLAG(crfsnp_item_parent));
        return (EC_FALSE);
    }    

    file_size  = CRFSNP_BNODE_FILESZ(crfsnp_bnode);
    store_size = CRFSNP_BNODE_STORESZ(crfsnp_bnode);
    if(offset >= store_size)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_dn: offset %ld >= store_size = %ld, file_size = %ld\n", 
                            offset, store_size, file_size);
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] __crfs_fetch_block_fd_b_dn: file_size %ld, store_size %ld, offset %ld\n", 
                        file_size, store_size, offset);

    seg_no = (uint32_t)(offset >> CPGB_CACHE_BIT_SIZE);
    
    if(EC_FALSE == __crfs_fetch_block_fd_b_seg_dn(crfs_md_id, crfsnp, parent_pos, seg_no, block_size, block_fd))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_block_fd_b_dn: fetch block fd of seg_no %u failed\n", 
                            seg_no);
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] __crfs_fetch_block_fd_b_dn: fetch block fd %d and size %u from seg %u done\n", 
                        (*block_fd), (*block_size), seg_no);
    
    return (EC_TRUE);
}

/**
*
*  reserve a fnode from name node
*
**/
static CRFSNP_FNODE * __crfs_reserve_npp(const UINT32 crfs_md_id, const CSTRING *file_path, const uint32_t expire_nsec)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE *crfsnp_fnode;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_reserve_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_reserve_npp: npp was not open\n");
        return (NULL_PTR);
    }

    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0130);
    crfsnp_fnode = crfsnp_mgr_reserve(CRFS_MD_NPP(crfs_md), file_path, expire_nsec);
    if(NULL_PTR == crfsnp_fnode)
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0131);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_reserve_npp: no name node accept file %s\n",
                            (char *)cstring_get_str(file_path));
        return (NULL_PTR);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0132);
    return (crfsnp_fnode);
}


/**
*
*  release a fnode from name node
*
**/
static EC_BOOL __crfs_release_npp(const UINT32 crfs_md_id, const CSTRING *file_path)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_release_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_release_npp: npp was not open\n");
        return (NULL_PTR);
    }

    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0133);
    if(EC_FALSE == crfsnp_mgr_release(CRFS_MD_NPP(crfs_md), file_path))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0134);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_release_npp: release file %s from npp failed\n",
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0135);
    return (EC_TRUE);
}

/**
*
*  write a fnode to name node
*
**/
EC_BOOL crfs_write_npp(const UINT32 crfs_md_id, const CSTRING *file_path, const CRFSNP_FNODE *crfsnp_fnode)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_npp: npp was not open\n");
        return (EC_FALSE);
    }

    if(0 == CRFSNP_FNODE_REPNUM(crfsnp_fnode))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_npp: no valid replica in fnode\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0136);
    if(EC_FALSE == crfsnp_mgr_write(CRFS_MD_NPP(crfs_md), file_path, crfsnp_fnode))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0137);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_npp: no name node accept file %s with %u replicas writting\n",
                            (char *)cstring_get_str(file_path), CRFSNP_FNODE_REPNUM(crfsnp_fnode));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0138);
    return (EC_TRUE);
}

/**
*
*  read a fnode from name node
*
**/
EC_BOOL crfs_read_npp(const UINT32 crfs_md_id, const CSTRING *file_path, CRFSNP_FNODE *crfsnp_fnode, UINT32 *expires_timestamp)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_read_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_read_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0139);
    if(EC_FALSE == crfsnp_mgr_read(CRFS_MD_NPP(crfs_md), file_path, crfsnp_fnode, expires_timestamp))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0140);
        
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_read_npp: crfsnp mgr read %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0141);

    return (EC_TRUE);
}

/**
*
*  read a bnode from name node
*
**/
static EC_BOOL __crfs_read_b_npp(const UINT32 crfs_md_id, const CSTRING *file_path, uint32_t *crfsnp_id, uint32_t *parent_pos, UINT32 *expires_timestamp)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_read_b_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:__crfs_read_b_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0142);
    if(EC_FALSE == crfsnp_mgr_read_b(CRFS_MD_NPP(crfs_md), file_path, crfsnp_id, parent_pos, expires_timestamp))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0143);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_read_b_npp: crfsnp mgr read %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0144);

    return (EC_TRUE);
}

/**
*
*  update a fnode to name node
*
**/
EC_BOOL crfs_update_npp(const UINT32 crfs_md_id, const CSTRING *file_path, const CRFSNP_FNODE *crfsnp_fnode)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_update_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_update_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0145);
    if(EC_FALSE == crfsnp_mgr_update(CRFS_MD_NPP(crfs_md), file_path, crfsnp_fnode))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0146);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_update_npp: no name node accept file %s with %u replicas updating\n",
                            (char *)cstring_get_str(file_path), CRFSNP_FNODE_REPNUM(crfsnp_fnode));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0147);
    return (EC_TRUE);
}

/**
*
*  renew a fnode to name node
*
**/
EC_BOOL crfs_renew(const UINT32 crfs_md_id, const CSTRING *file_path, const UINT32 expires_timestamp)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_renew: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_renew: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0148);
    if(EC_FALSE == crfsnp_mgr_update_expires(CRFS_MD_NPP(crfs_md), file_path, expires_timestamp))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0149);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_renew: no np has file %s to update\n",
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0150);
    return (EC_TRUE);
}

/**
*
*  delete file data from current dn
*
**/
static EC_BOOL __crfs_delete_dn(const UINT32 crfs_md_id, const CRFSNP_FNODE *crfsnp_fnode)
{
    CRFS_MD *crfs_md;
    const CRFSNP_INODE *crfsnp_inode;

    uint32_t file_size;
    uint16_t disk_no;
    uint16_t block_no;
    uint16_t page_no;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_delete_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_delete_dn: no dn was open\n");
        return (EC_FALSE);
    }

    if(0 == CRFSNP_FNODE_REPNUM(crfsnp_fnode))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_delete_dn: no replica\n");
        return (EC_FALSE);
    }    

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;
    
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0151);
    if(EC_FALSE == crfsdn_remove(CRFS_MD_DN(crfs_md), disk_no, block_no, page_no, file_size))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0152);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_delete_dn: remove file fsize %u, disk %u, block %u, page %u failed\n", file_size, disk_no, block_no, page_no);
        return (EC_FALSE);
    }    
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0153);

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] __crfs_delete_dn: remove file fsize %u, disk %u, block %u, page %u done\n", file_size, disk_no, block_no, page_no);
    
    return (EC_TRUE);
}

static EC_BOOL __crfs_delete_b_dn(const UINT32 crfs_md_id, CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_delete_b_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_delete_b_dn: no dn was open\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsnp_bnode_delete_dir_son(crfsnp, crfsnp_bnode))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_delete_b_dn:delete bnode sons failed\n");
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL crfs_delete_dn(const UINT32 crfs_md_id, const UINT32 crfsnp_id, const CRFSNP_ITEM *crfsnp_item)
{
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/


    if(NULL_PTR != crfsnp_item)
    {
        if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
        {        
            if(EC_FALSE == __crfs_delete_dn(crfs_md_id, CRFSNP_ITEM_FNODE(crfsnp_item)))
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_dn: delete regular file from dn failed\n");
                return (EC_FALSE);
            }
            return (EC_TRUE);
        }
        
        if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
        {
            CRFS_MD *crfs_md;
            CRFSNP  *crfsnp;
            uint32_t crfsnp_id_t;

            crfs_md = CRFS_MD_GET(crfs_md_id);
            if(NULL_PTR == CRFS_MD_NPP(crfs_md))
            {
                dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_delete_dn: npp was not open\n");
                return (EC_FALSE);
            }            

            crfsnp_id_t = (uint32_t)crfsnp_id;
            
            CRFSNP_MGR_CMUTEX_LOCK(CRFS_MD_NPP(crfs_md), LOC_CRFS_0154);
            crfsnp = crfsnp_mgr_open_np(CRFS_MD_NPP(crfs_md), crfsnp_id_t);
            if(NULL_PTR == crfsnp)
            {
                CRFSNP_MGR_CMUTEX_UNLOCK(CRFS_MD_NPP(crfs_md), LOC_CRFS_0155);
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_dn: open np %u failed\n", crfsnp_id_t);
                return (EC_FALSE);
            }
            CRFSNP_MGR_CMUTEX_UNLOCK(CRFS_MD_NPP(crfs_md), LOC_CRFS_0156);
    
            if(EC_FALSE == __crfs_delete_b_dn(crfs_md_id, crfsnp, (CRFSNP_BNODE *)CRFSNP_ITEM_BNODE(crfsnp_item)))
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_dn: delete big file from dn failed\n");
                return (EC_FALSE);
            }
            return (EC_TRUE);
        }

        /*Oops! not implement or not support yet ...*/
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_dn: crfsnp_item %p dflag flag 0x%x is unknown\n",
                            crfsnp_item, CRFSNP_ITEM_DIR_FLAG(crfsnp_item));
    }
    return (EC_TRUE);
}

EC_BOOL crfs_delete_file_backup(const UINT32 crfs_md_id, const CSTRING *path)
{
    CRFS_MD      *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete_file_backup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    if(NULL_PTR == CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error: crfs_delete_file_backup: RFS in SYNC but backup RFS is null\n");
        return (EC_FALSE);
    }        

    if(EC_FALSE == crfsbk_delete_file(CRFS_MD_BACKUP(crfs_md), path))
    {
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file_backup: delete file %s with size %ld from backup RFS failed\n", 
                           (char *)cstring_get_str(path));     
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file_backup: delete file %s from backup RFS done\n", 
                       (char *)cstring_get_str(path));  
    
    return (EC_TRUE);
}

EC_BOOL crfs_delete_file_b_backup(const UINT32 crfs_md_id, const CSTRING *path)
{
    CRFS_MD      *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete_file_b_backup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    if(NULL_PTR == CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error: crfs_delete_file_b_backup: RFS in SYNC but backup RFS is null\n");
        return (EC_FALSE);
    }        

    if(EC_FALSE == crfsbk_delete_file_b(CRFS_MD_BACKUP(crfs_md), path))
    {
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file_b_backup: delete file %s with size %ld from backup RFS failed\n", 
                           (char *)cstring_get_str(path));     
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file_b_backup: delete file %s from backup RFS done\n", 
                       (char *)cstring_get_str(path));  
    
    return (EC_TRUE);
}

EC_BOOL crfs_delete_dir_backup(const UINT32 crfs_md_id, const CSTRING *path)
{
    CRFS_MD      *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete_dir_backup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    if(NULL_PTR == CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error: crfs_delete_dir_backup: RFS in SYNC but backup RFS is null\n");
        return (EC_FALSE);
    }        

    if(EC_FALSE == crfsbk_delete_dir(CRFS_MD_BACKUP(crfs_md), path))
    {
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_dir_backup: delete file %s with size %ld from backup RFS failed\n", 
                           (char *)cstring_get_str(path));     
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_dir_backup: delete file %s from backup RFS done\n", 
                       (char *)cstring_get_str(path));  
    
    return (EC_TRUE);
}

/**
*
*  delete a file
*
**/
EC_BOOL crfs_delete_file(const UINT32 crfs_md_id, const CSTRING *path)
{
    CRFS_MD      *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete_file: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    /*in SYNC state*/
    if(EC_TRUE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        if(EC_TRUE == crfs_delete_file_backup(crfs_md_id, path))
        {
            dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file: delete file %s with from backup RFS done\n", 
                                   (char *)cstring_get_str(path));            
            return (EC_TRUE);
        }

        /*fall through to RFS*/
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_file: delete file %s with from backup RFS failed\n", 
                               (char *)cstring_get_str(path));            
        return (EC_FALSE);/*terminate, not change RFS*/
    } 
    
    crfs_md = CRFS_MD_GET(crfs_md_id);   

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_delete_file: npp was not open\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file: crfs_md_id %u, path %s ...\n", 
                        crfs_md_id, (char *)cstring_get_str(path));

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0157);
    if(EC_FALSE == crfsnp_mgr_umount(CRFS_MD_NPP(crfs_md), path, CRFSNP_ITEM_FILE_IS_REG))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0158);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_file: umount %.*s failed\n", 
                            cstring_get_len(path), cstring_get_str(path));
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0159);
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file: crfs_md_id %u, path %s done\n", 
                        crfs_md_id, (char *)cstring_get_str(path));
    
    return (EC_TRUE);
}

/**
*
*  delete a big file
*
**/
EC_BOOL crfs_delete_file_b(const UINT32 crfs_md_id, const CSTRING *path)
{
    CRFS_MD      *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete_file_b: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    /*in SYNC state*/
    if(EC_TRUE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        if(EC_TRUE == crfs_delete_file_b_backup(crfs_md_id, path))
        {
            dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file_b: delete file %s with from backup RFS done\n", 
                                   (char *)cstring_get_str(path));            
            return (EC_TRUE);
        }

        /*fall through to RFS*/
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_file_b: delete file %s with from backup RFS failed\n", 
                               (char *)cstring_get_str(path));            
        return (EC_FALSE);/*terminate, not change RFS*/
    }   

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_delete_file_b: npp was not open\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file_b: crfs_md_id %u, path %s ...\n", 
                        crfs_md_id, (char *)cstring_get_str(path));

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0160);
    if(EC_FALSE == crfsnp_mgr_umount(CRFS_MD_NPP(crfs_md), path, CRFSNP_ITEM_FILE_IS_BIG))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0161);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_file_b: umount %.*s failed\n", 
                            cstring_get_len(path), cstring_get_str(path));
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0162);
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_file_b: crfs_md_id %u, path %s done\n", 
                        crfs_md_id, (char *)cstring_get_str(path));
    
    return (EC_TRUE);
}

/**
*
*  delete a dir from all npp and all dn
*
**/
EC_BOOL crfs_delete_dir(const UINT32 crfs_md_id, const CSTRING *path)
{
    CRFS_MD      *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete_dir: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    /*in SYNC state*/
    if(EC_TRUE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        if(EC_TRUE == crfs_delete_dir_backup(crfs_md_id, path))
        {
            dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_dir: delete dir %s with from backup RFS done\n", 
                                   (char *)cstring_get_str(path));            
            return (EC_TRUE);
        }

        /*fall through to RFS*/
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_dir: delete dir %s with from backup RFS failed\n", 
                               (char *)cstring_get_str(path));            
        return (EC_FALSE);/*terminate, not change RFS*/
    }  

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_delete_dir: npp was not open\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_dir: crfs_md_id %u, path %s ...\n", 
                        crfs_md_id, (char *)cstring_get_str(path));

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0163);
    if(EC_FALSE == crfsnp_mgr_umount(CRFS_MD_NPP(crfs_md), path, CRFSNP_ITEM_FILE_IS_DIR))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0164);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_dir: umount %.*s failed\n", 
                            cstring_get_len(path), cstring_get_str(path));
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0165);
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_delete_dir: crfs_md_id %u, path %s done\n", 
                        crfs_md_id, (char *)cstring_get_str(path));
    
    return (EC_TRUE);
}

/**
*
*  delete a file or dir from all npp and all dn
*
**/
EC_BOOL crfs_delete(const UINT32 crfs_md_id, const CSTRING *path, const UINT32 dflag)
{   
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    if(CRFSNP_ITEM_FILE_IS_REG == dflag)
    {
        return crfs_delete_file(crfs_md_id, path);
    }

    if(CRFSNP_ITEM_FILE_IS_BIG == dflag)
    {
        return crfs_delete_file_b(crfs_md_id, path);
    }    

    if(CRFSNP_ITEM_FILE_IS_DIR == dflag)
    {   
        return crfs_delete_dir(crfs_md_id, path);
    }

    if(CRFSNP_ITEM_FILE_IS_ANY == dflag)
    {
        crfs_delete_file(crfs_md_id, path);
        crfs_delete_file_b(crfs_md_id, path);  
        crfs_delete_dir(crfs_md_id, path);

        return (EC_TRUE);
    }

    dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete: crfs_md_id %u, path [invalid 0x%x] %s\n", 
                        crfs_md_id, dflag, (char *)cstring_get_str(path));
  
    return (EC_FALSE);
}

/**
*
*  update a file 
*
**/
EC_BOOL crfs_update(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes, const UINT32 expire_nsec)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;
    UINT32        expires_timestamp;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_update: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfsnp_fnode_init(&crfsnp_fnode);

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0166);
    if(EC_FALSE == crfs_read_npp(crfs_md_id, file_path, &crfsnp_fnode, &expires_timestamp))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0167);

        /*file not exist, write as new file*/
        if(EC_FALSE == crfs_write(crfs_md_id, file_path, cbytes, expire_nsec))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_update: write file %s failed\n", (char *)cstring_get_str(file_path));
            return (EC_FALSE);
        }
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_update: write file %s done\n", (char *)cstring_get_str(file_path));
        return (EC_TRUE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0168);

    /*file exist, update it*/
    if(EC_FALSE == crfs_delete(crfs_md_id, file_path, CRFSNP_ITEM_FILE_IS_REG))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_update: delete old file %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }   
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_update: delete old file %s done\n", (char *)cstring_get_str(file_path));

    if(EC_FALSE == crfs_write(crfs_md_id, file_path, cbytes, expire_nsec))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_update: write new file %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_update: write new file %s done\n", (char *)cstring_get_str(file_path));

    return (EC_TRUE);
}

/**
*
*  query a file
*
**/
EC_BOOL crfs_qfile(const UINT32 crfs_md_id, const CSTRING *file_path, CRFSNP_ITEM  *crfsnp_item)
{
    CRFS_MD      *crfs_md;
    CRFSNP_ITEM  *crfsnp_item_src;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_qfile: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_qfile: npp was not open\n");
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0169);    

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0170);
    crfsnp_item_src = crfsnp_mgr_search_item(CRFS_MD_NPP(crfs_md), 
                                             (uint32_t)cstring_get_len(file_path), 
                                             cstring_get_str(file_path), 
                                             CRFSNP_ITEM_FILE_IS_REG);
    if(NULL_PTR == crfsnp_item_src)
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0171);

        CRFS_UNLOCK(crfs_md, LOC_CRFS_0172);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_qfile: query file %s from npp failed\n", 
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    /*clone*/
    crfsnp_item_clone(crfsnp_item_src, crfsnp_item);
    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0173);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0174);

    return (EC_TRUE);
}

/**
*
*  query a dir
*
**/
EC_BOOL crfs_qdir(const UINT32 crfs_md_id, const CSTRING *dir_path, CRFSNP_ITEM  *crfsnp_item)
{
    CRFS_MD      *crfs_md;
    CRFSNP_ITEM  *crfsnp_item_src;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_qdir: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_qdir: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0175);    

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0176);
    crfsnp_item_src = crfsnp_mgr_search_item(CRFS_MD_NPP(crfs_md), 
                                             (uint32_t)cstring_get_len(dir_path), 
                                             cstring_get_str(dir_path), 
                                             CRFSNP_ITEM_FILE_IS_DIR);    
    if(NULL_PTR == crfsnp_item_src)
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0177);

        CRFS_UNLOCK(crfs_md, LOC_CRFS_0178);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_qdir: query dir %s from npp failed\n", 
                            (char *)cstring_get_str(dir_path));
        return (EC_FALSE);
    }               
    /*clone*/
    crfsnp_item_clone(crfsnp_item_src, crfsnp_item);
    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0179);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0180);

    return (EC_TRUE);
}

/**
*
*  query and list full path of a file or dir
*
**/
EC_BOOL crfs_qlist_path(const UINT32 crfs_md_id, const CSTRING *file_path, CVECTOR  *path_cstr_vec)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_qlist_path: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_qlist_path: npp was not open\n");
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0181);    
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0182);
    if(EC_FALSE == crfsnp_mgr_list_path(CRFS_MD_NPP(crfs_md), file_path, path_cstr_vec))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0183);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0184);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_qlist_path: list path '%s' failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0185);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0186);

    return (EC_TRUE);
}

/**
*
*  query and list full path of a file or dir of one np
*
**/
EC_BOOL crfs_qlist_path_of_np(const UINT32 crfs_md_id, const CSTRING *file_path, const UINT32 crfsnp_id, CVECTOR  *path_cstr_vec)
{
    CRFS_MD      *crfs_md;
    uint32_t      crfsnp_id_t;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_qlist_path_of_np: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_qlist_path_of_np: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_id_t = (uint32_t)crfsnp_id;

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0187);    
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0188);
    if(EC_FALSE == crfsnp_mgr_list_path_of_np(CRFS_MD_NPP(crfs_md), file_path, crfsnp_id_t, path_cstr_vec))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0189);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0190);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_qlist_path_of_np: list path '%s' of np %u failed\n", 
                            (char *)cstring_get_str(file_path), crfsnp_id_t);
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0191);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0192);

    return (EC_TRUE);
}

/**
*
*  query and list short name of a file or dir
*
**/
EC_BOOL crfs_qlist_seg(const UINT32 crfs_md_id, const CSTRING *file_path, CVECTOR  *seg_cstr_vec)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_qlist_seg: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_qlist_seg: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0193);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0194);
    if(EC_FALSE == crfsnp_mgr_list_seg(CRFS_MD_NPP(crfs_md), file_path, seg_cstr_vec))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0195);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0196);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_qlist_seg: list seg of path '%s' failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0197);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0198);
    return (EC_TRUE);
}

/**
*
*  query and list short name of a file or dir of one np
*
**/
EC_BOOL crfs_qlist_seg_of_np(const UINT32 crfs_md_id, const CSTRING *file_path, const UINT32 crfsnp_id, CVECTOR  *seg_cstr_vec)
{
    CRFS_MD      *crfs_md;
    uint32_t      crfsnp_id_t;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_qlist_seg_of_np: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_qlist_seg_of_np: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_id_t = (uint32_t)crfsnp_id;
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0199);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0200);
    if(EC_FALSE == crfsnp_mgr_list_seg_of_np(CRFS_MD_NPP(crfs_md), file_path, crfsnp_id_t, seg_cstr_vec))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0201);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0202);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_qlist_seg_of_np: list seg of path '%s' failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0203);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0204);
    return (EC_TRUE);
}

/**
*
*  flush name node pool
*
**/
EC_BOOL crfs_flush_npp(const UINT32 crfs_md_id)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_flush_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_flush_npp: npp was not open\n");
        return (EC_TRUE);
    }
    
    CRFS_WRLOCK(crfs_md, LOC_CRFS_0205);
    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0206);
    if(EC_FALSE == crfsnp_mgr_flush(CRFS_MD_NPP(crfs_md)))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0207);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0208);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_flush_npp: flush failed\n");
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0209);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0210);
    return (EC_TRUE);
}

/**
*
*  flush data node
*
*
**/
EC_BOOL crfs_flush_dn(const UINT32 crfs_md_id)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_flush_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_flush_dn: dn is null\n");
        return (EC_FALSE);
    }

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0211);
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0212);
    if(EC_FALSE == crfsdn_flush(CRFS_MD_DN(crfs_md)))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0213);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0214);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_flush_dn: flush dn failed\n");
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0215);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0216);

    return (EC_TRUE);
}

/**
*
*  count file num under specific path
*  if path is regular file, return file_num 1
*  if path is directory, return file num under it
*
**/
EC_BOOL crfs_file_num(const UINT32 crfs_md_id, const CSTRING *path_cstr, UINT32 *file_num)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_file_num: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_file_num: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0217);
    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0218);
    if(EC_FALSE == crfsnp_mgr_file_num(CRFS_MD_NPP(crfs_md), path_cstr, file_num))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0219);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0220);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_file_num: get file num of path '%s' failed\n", (char *)cstring_get_str(path_cstr));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0221);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0222);

    return (EC_TRUE);
}

/**
*
*  get file size of specific file given full path name
*
**/
EC_BOOL crfs_file_size(const UINT32 crfs_md_id, const CSTRING *path_cstr, uint64_t *file_size)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_file_size: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_file_size: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0223);
    if(EC_FALSE == crfsnp_mgr_file_size(CRFS_MD_NPP(crfs_md), path_cstr, file_size))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0224);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_file_size: crfsnp mgr get size of %s failed\n", (char *)cstring_get_str(path_cstr));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0225);    
    return (EC_TRUE);
}

/**
*
*  get bigfile store size of specific file given full path name
*
**/
EC_BOOL crfs_store_size_b(const UINT32 crfs_md_id, const CSTRING *path_cstr, uint64_t *store_size, UINT32 *expires_timestamp)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_store_size_b: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_store_size_b: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0226);
    if(EC_FALSE == crfsnp_mgr_store_size_b(CRFS_MD_NPP(crfs_md), path_cstr, store_size, expires_timestamp))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0227);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_store_size_b: crfsnp mgr get store size of %s failed\n", (char *)cstring_get_str(path_cstr));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0228);    
    return (EC_TRUE);
}

/**
*
*  get file md5sum of specific file given full path name
*
**/
EC_BOOL crfs_file_md5sum(const UINT32 crfs_md_id, const CSTRING *path_cstr, CMD5_DIGEST *md5sum)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_file_md5sum: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_file_md5sum: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0229);
    if(EC_FALSE == crfsnp_mgr_file_md5sum(CRFS_MD_NPP(crfs_md), path_cstr, md5sum))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0230);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_file_md5sum: crfsnp mgr get md5sum of %s failed\n", (char *)cstring_get_str(path_cstr));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0231);    
    return (EC_TRUE);
}

/**
*
*  get a seg md5sum of specific bigfile given full path name
*
**/
EC_BOOL crfs_file_md5sum_b(const UINT32 crfs_md_id, const CSTRING *path_cstr, const UINT32 seg_no, CMD5_DIGEST *md5sum)
{
    CRFS_MD      *crfs_md;
    uint32_t      seg_no_t;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_file_md5sum_b: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_file_md5sum_b: npp was not open\n");
        return (EC_FALSE);
    }

    seg_no_t = (uint32_t)seg_no;
    if(seg_no != seg_no_t)
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_file_md5sum_b: seg %u overflow\n", seg_no);
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0232);
    if(EC_FALSE == crfsnp_mgr_file_md5sum_b(CRFS_MD_NPP(crfs_md), path_cstr, seg_no_t, md5sum))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0233);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_file_md5sum_b: crfsnp mgr get seg %u md5sum of bigfile %s failed\n", 
                            seg_no, (char *)cstring_get_str(path_cstr));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0234);    
    return (EC_TRUE);
}

/**
*
*  mkdir in current name node pool
*
**/
EC_BOOL crfs_mkdir(const UINT32 crfs_md_id, const CSTRING *path_cstr)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_mkdir: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_mkdir: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_WRLOCK(crfs_md, LOC_CRFS_0235);
    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0236);
    if(EC_FALSE == crfsnp_mgr_mkdir(CRFS_MD_NPP(crfs_md), path_cstr))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0237);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0238);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_mkdir: mkdir '%s' failed\n", (char *)cstring_get_str(path_cstr));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0239);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0240);

    return (EC_TRUE);
}

/**
*
*  search in current name node pool
*
**/
EC_BOOL crfs_search(const UINT32 crfs_md_id, const CSTRING *path_cstr, const UINT32 dflag)
{
    CRFS_MD      *crfs_md;
    uint32_t      crfsnp_id;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_search: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_search: crfs_md_id %u, path %s, dflag %x\n", crfs_md_id, (char *)cstring_get_str(path_cstr), dflag);

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_search: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0241);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0242);
    if(EC_FALSE == crfsnp_mgr_search(CRFS_MD_NPP(crfs_md), (uint32_t)cstring_get_len(path_cstr), cstring_get_str(path_cstr), dflag, &crfsnp_id))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0243);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0244);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_search: search '%s' with dflag %x failed\n", (char *)cstring_get_str(path_cstr), dflag);
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0245);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0246);

    return (EC_TRUE);
}

static EC_BOOL __crfs_recycle_of_np(const UINT32 crfs_md_id, const uint32_t crfsnp_id)
{
    CRFS_MD      *crfs_md;
    CRFSNP_RECYCLE_DN crfsnp_recycle_dn;

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:__crfs_recycle_of_np: npp was not open\n");
        return (EC_FALSE);
    } 

    CRFSNP_RECYCLE_DN_ARG1(&crfsnp_recycle_dn)   = crfs_md_id;
    CRFSNP_RECYCLE_DN_FUNC(&crfsnp_recycle_dn)   = crfs_release_dn;
    //CRFSNP_RECYCLE_DN_WRLOCK(&crfsnp_recycle_dn) = crfs_wrlock;
    //CRFSNP_RECYCLE_DN_UNLOCK(&crfsnp_recycle_dn) = crfs_unlock;

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0247);
    if(EC_FALSE == crfsnp_mgr_recycle_np(CRFS_MD_NPP(crfs_md), crfsnp_id, &crfsnp_recycle_dn))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0248);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_recycle_of_np: recycle np %u failed\n", crfsnp_id);
        return (EC_FALSE);
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0249);
    return (EC_TRUE);
}

/**
*
*  empty recycle
*
**/
EC_BOOL crfs_recycle(const UINT32 crfs_md_id)
{
    CRFS_MD      *crfs_md;
    CRFSNP_MGR   *crfsnp_mgr;
    uint32_t      crfsnp_id;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_recycle: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_recycle: recycle beg\n");

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_mgr = CRFS_MD_NPP(crfs_md);
    if(NULL_PTR == crfsnp_mgr)
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "warn:crfs_recycle: npp was not open\n");
        return (EC_FALSE);
    }    

    for(crfsnp_id = 0; crfsnp_id < CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr); crfsnp_id ++)
    {
        __crfs_recycle_of_np(crfs_md_id, crfsnp_id);
        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_recycle: recycle np %u done\n", crfsnp_id);
    }
    
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_recycle: recycle end\n");
    
    return (EC_TRUE);
}

/**
*
*  check file content on data node
*
**/
EC_BOOL crfs_check_file_content(const UINT32 crfs_md_id, const UINT32 disk_no, const UINT32 block_no, const UINT32 page_no, const UINT32 file_size, const CSTRING *file_content_cstr)
{
    CRFS_MD *crfs_md;

    CBYTES *cbytes;

    UINT8 *buff;
    UINT8 *str;

    UINT32 len;
    UINT32 pos;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_check_file_content: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_content: dn is null\n");
        return (EC_FALSE);
    }

    ASSERT(EC_TRUE == c_check_is_uint16_t(disk_no));
    ASSERT(EC_TRUE == c_check_is_uint16_t(block_no));
    ASSERT(EC_TRUE == c_check_is_uint16_t(page_no));

    cbytes = cbytes_new(file_size);
    if(NULL_PTR == cbytes)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_content: new crfs buff with len %u failed\n", file_size);
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0250);
    if(EC_FALSE == crfsdn_read_p(CRFS_MD_DN(crfs_md), (uint16_t)disk_no, (uint16_t)block_no, (uint16_t)page_no, file_size, 
                                  CBYTES_BUF(cbytes), &(CBYTES_LEN(cbytes))))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0251);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_content: read %u bytes from disk %u, block %u, page %u failed\n", 
                            file_size, disk_no, block_no, page_no);
        cbytes_free(cbytes, LOC_CRFS_0252);
        return (EC_FALSE);
    }

    if(CBYTES_LEN(cbytes) < cstring_get_len(file_content_cstr))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0253);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_content: read %u bytes from disk %u, block %u, page %u to buff len %u less than cstring len %u to compare\n",
                            file_size, disk_no, block_no, page_no,
                            CBYTES_LEN(cbytes), cstring_get_len(file_content_cstr));
        cbytes_free(cbytes, LOC_CRFS_0254);
        return (EC_FALSE);
    }

    len = cstring_get_len(file_content_cstr);

    buff = CBYTES_BUF(cbytes);
    str  = cstring_get_str(file_content_cstr);

    for(pos = 0; pos < len; pos ++)
    {
        if(buff[ pos ] != str[ pos ])
        {
            CRFS_UNLOCK(crfs_md, LOC_CRFS_0255);
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_content: char at pos %u not matched\n", pos);
            sys_print(LOGSTDOUT, "read buff: %.*s\n", len, buff);
            sys_print(LOGSTDOUT, "expected : %.*s\n", len, str);

            cbytes_free(cbytes, LOC_CRFS_0256);
            return (EC_FALSE);
        }
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0257);

    cbytes_free(cbytes, LOC_CRFS_0258);
    return (EC_TRUE);
}

/**
*
*  check file content on data node
*
**/
EC_BOOL crfs_check_file_is(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *file_content)
{
    CRFS_MD *crfs_md;

    CBYTES *cbytes;

    UINT8 *buff;
    UINT8 *str;

    UINT32 len;
    UINT32 pos;

    UINT32 expires_timestamp;
    UINT32 need_expired_content;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_check_file_is: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_is: dn is null\n");
        return (EC_FALSE);
    }
    
    cbytes = cbytes_new(0);
    if(NULL_PTR == cbytes)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_is: new cbytes failed\n");
        return (EC_FALSE);
    }

    need_expired_content = EC_TRUE;
    if(EC_FALSE == crfs_read(crfs_md_id, file_path, cbytes, &expires_timestamp, need_expired_content))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_is: read file %s failed\n", (char *)cstring_get_str(file_path));
        cbytes_free(cbytes, LOC_CRFS_0259);
        return (EC_FALSE);
    }

    if(CBYTES_LEN(cbytes) != CBYTES_LEN(file_content))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_is: mismatched len: file %s read len %u which should be %u\n",
                            (char *)cstring_get_str(file_path),                            
                            CBYTES_LEN(cbytes), CBYTES_LEN(file_content));
        cbytes_free(cbytes, LOC_CRFS_0260);
        return (EC_FALSE);
    }

    len  = CBYTES_LEN(file_content);

    buff = CBYTES_BUF(cbytes);
    str  = CBYTES_BUF(file_content);

    for(pos = 0; pos < len; pos ++)
    {
        if(buff[ pos ] != str[ pos ])
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_check_file_is: char at pos %u not matched\n", pos);
            sys_print(LOGSTDOUT, "read buff: %.*s\n", len, buff);
            sys_print(LOGSTDOUT, "expected : %.*s\n", len, str);

            cbytes_free(cbytes, LOC_CRFS_0261);
            return (EC_FALSE);
        }
    }

    cbytes_free(cbytes, LOC_CRFS_0262);
    return (EC_TRUE);
}

/**
*
*  show name node pool info if it is npp
*
*
**/
EC_BOOL crfs_show_npp(const UINT32 crfs_md_id, LOG *log)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_show_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(log, "(null)\n");
        return (EC_TRUE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0263);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0264);
    
    crfsnp_mgr_print(log, CRFS_MD_NPP(crfs_md));
    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0265);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0266);
    
    return (EC_TRUE);
}

/*for debug only*/
EC_BOOL crfs_show_dn_no_lock(const UINT32 crfs_md_id, LOG *log)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_show_dn_no_lock: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        sys_log(log, "(null)\n");
        return (EC_TRUE);
    }

    //CRFS_RDLOCK(crfs_md, LOC_CRFS_0267);
    //crfsdn_rdlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0268);
    crfsdn_print(log, CRFS_MD_DN(crfs_md));
    //crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0269);
    //CRFS_UNLOCK(crfs_md, LOC_CRFS_0270);

    return (EC_TRUE);
}

/**
*
*  show crfsdn info if it is dn
*
*
**/
EC_BOOL crfs_show_dn(const UINT32 crfs_md_id, LOG *log)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_show_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        sys_log(log, "(null)\n");
        return (EC_TRUE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0271);
    crfsdn_rdlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0272);
    crfsdn_print(log, CRFS_MD_DN(crfs_md));
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0273);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0274);

    return (EC_TRUE);
}

/*debug*/
EC_BOOL crfs_show_cached_np(const UINT32 crfs_md_id, LOG *log)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_show_cached_np: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(log, "(null)\n");
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0275);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0276);    
    if(EC_FALSE == crfsnp_mgr_show_cached_np(log, CRFS_MD_NPP(crfs_md)))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0277);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0278);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_show_cached_np: show cached np but failed\n");
        return (EC_FALSE);
    }    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0279);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0280);

    return (EC_TRUE);
}

EC_BOOL crfs_show_specific_np(const UINT32 crfs_md_id, const UINT32 crfsnp_id, LOG *log)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_show_specific_np: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(log, "(null)\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == c_check_is_uint32_t(crfsnp_id))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_show_specific_np: crfsnp_id %u is invalid\n", crfsnp_id);
        return (EC_FALSE);
    }    

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0281);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0282);    
    if(EC_FALSE == crfsnp_mgr_show_np(log, CRFS_MD_NPP(crfs_md), (uint32_t)crfsnp_id))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0283);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0284);
        
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_show_cached_np: show np %u but failed\n", crfsnp_id);
        return (EC_FALSE);
    }    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0285);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0286);

    return (EC_TRUE);
}

EC_BOOL crfs_show_path_depth(const UINT32 crfs_md_id, const CSTRING *path, LOG *log)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_show_path_depth: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(log, "error:crfs_show_path_depth: npp was not open\n");
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0287);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0288);    
    if(EC_FALSE == crfsnp_mgr_show_path_depth(log, CRFS_MD_NPP(crfs_md), path))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0289);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0290);
        
        sys_log(log, "error:crfs_show_path_depth: show path %s in depth failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0291);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0292);

    return (EC_TRUE);
}

EC_BOOL crfs_show_path(const UINT32 crfs_md_id, const CSTRING *path, LOG *log)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_show_path: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(log, "error:crfs_show_path: npp was not open\n");
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0293);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0294);    
    if(EC_FALSE == crfsnp_mgr_show_path(log, CRFS_MD_NPP(crfs_md), path))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0295);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0296);
        sys_log(log, "error:crfs_show_path: show path %s failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0297);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0298);
    
    return (EC_TRUE);
}

EC_BOOL crfs_rdlock(const UINT32 crfs_md_id, const UINT32 location)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_rdlock: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    CRFS_RDLOCK(crfs_md, location);
    return (EC_TRUE);
}

EC_BOOL crfs_expire_dn(const UINT32 crfs_md_id)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_expire_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_expire_dn: no dn was open\n");
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfsdn_expire_open_nodes(CRFS_MD_DN(crfs_md)))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_expire_dn: expire open nodes failed\n");
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL crfs_wrlock(const UINT32 crfs_md_id, const UINT32 location)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_wrlock: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    CRFS_WRLOCK(crfs_md, location);
    return (EC_TRUE);
}

EC_BOOL crfs_unlock(const UINT32 crfs_md_id, const UINT32 location)
{
    CRFS_MD *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_unlock: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);
    CRFS_UNLOCK(crfs_md, location);
    return (EC_TRUE);
}

/*------------------------------------------------ interface for replica ------------------------------------------------*/
EC_BOOL crfs_write_r(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes, const UINT32 expire_nsec, const UINT32 replica_num)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write_r: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);    

    if(CRFS_MAX_REPLICA_NUM < replica_num)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_r: reject to write file %s with invalid replica %ld\n", 
                           (char *)cstring_get_str(file_path), replica_num);
        return (EC_FALSE);
    }

    if(EC_FALSE == crfs_write(crfs_md_id, file_path, cbytes, expire_nsec))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_r: write file %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);                           
    }

    if(1 >= replica_num)/*at least one replica. zero means default replicas*/
    {
        return (EC_TRUE);
    }

    if(EC_FALSE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        TASK_MGR *task_mgr;
        UINT32    mod_node_num;
        UINT32    mod_node_idx;  
        EC_BOOL   ret[CRFS_MAX_REPLICA_NUM];

        mod_node_num = DMIN(replica_num, cvector_size(CRFS_MD_NEIGHBOR_VEC(crfs_md)));

        task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            ret[ mod_node_idx ] = EC_FALSE;
            task_p2p_inc(task_mgr, crfs_md_id, recv_mod_node, 
                        &(ret[ mod_node_idx ]), FI_crfs_write,ERR_MODULE_ID, file_path, cbytes, expire_nsec);
        }
        task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            if(EC_FALSE == ret[ mod_node_idx ])
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_r: write file %s to tcid %s rank %ld failed\n", 
                                   (char *)cstring_get_str(file_path), 
                                   MOD_NODE_TCID_STR(recv_mod_node),MOD_NODE_RANK(recv_mod_node));
            }
        }        
    }
    return (EC_TRUE);
}

/**
*
*  update a file 
*
**/
EC_BOOL crfs_update_r(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes, const UINT32 expire_nsec, const UINT32 replica_num)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_update_r: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfsnp_fnode_init(&crfsnp_fnode);

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CRFS_MAX_REPLICA_NUM < replica_num)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_update_r: reject to update file %s with invalid replica %ld\n", 
                           (char *)cstring_get_str(file_path), replica_num);
        return (EC_FALSE);
    }

    if(EC_FALSE == crfs_update(crfs_md_id, file_path, cbytes, expire_nsec))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_update_r: update file %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);                           
    }

    if(1 >= replica_num)/*at least one replica. zero means default replicas*/
    {
        return (EC_TRUE);
    }
    
    if(EC_FALSE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        TASK_MGR *task_mgr;
        UINT32    mod_node_num;
        UINT32    mod_node_idx;  
        EC_BOOL   ret_vec[CRFS_MAX_REPLICA_NUM];

        mod_node_num = DMIN(CRFS_MAX_REPLICA_NUM, cvector_size(CRFS_MD_NEIGHBOR_VEC(crfs_md)));

        task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            ret_vec[ mod_node_idx ] = EC_FALSE;
            task_p2p_inc(task_mgr, crfs_md_id, recv_mod_node, 
                        &(ret_vec[ mod_node_idx ]), FI_crfs_update,ERR_MODULE_ID, file_path, cbytes, expire_nsec);
        }
        task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            if(EC_FALSE == ret_vec[ mod_node_idx ])
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_update_r: update file %s at tcid %s rank %ld failed\n", 
                                   (char *)cstring_get_str(file_path), 
                                   MOD_NODE_TCID_STR(recv_mod_node), MOD_NODE_RANK(recv_mod_node));
            }
        }        
    }    

    return (EC_TRUE);
}

EC_BOOL crfs_delete_r(const UINT32 crfs_md_id, const CSTRING *path, const UINT32 dflag, const UINT32 replica_num)
{
    CRFS_MD      *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete_r: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CRFS_MAX_REPLICA_NUM < replica_num)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_r: reject to remove file %s with invalid replica %ld\n", 
                           (char *)cstring_get_str(path), replica_num);
        return (EC_FALSE);
    }

    if(EC_FALSE == crfs_delete(crfs_md_id, path, dflag))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_r: remove file %s failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);                           
    }

    if(1 >= replica_num)/*at least one replica. zero means default replicas*/
    {
        return (EC_TRUE);
    }

    if(EC_FALSE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        TASK_MGR *task_mgr;
        UINT32    mod_node_num;
        UINT32    mod_node_idx;  
        EC_BOOL   ret_vec[CRFS_MAX_REPLICA_NUM];

        mod_node_num = DMIN(CRFS_MAX_REPLICA_NUM, cvector_size(CRFS_MD_NEIGHBOR_VEC(crfs_md)));

        task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            ret_vec[ mod_node_idx ] = EC_FALSE;
            task_p2p_inc(task_mgr, crfs_md_id, recv_mod_node, 
                         &(ret_vec[ mod_node_idx ]), FI_crfs_delete,ERR_MODULE_ID, path, dflag);
        }
        task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            if(EC_FALSE == ret_vec[ mod_node_idx ])
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_delete_r: remove file %s at tcid %s rank %ld failed\n", 
                                   (char *)cstring_get_str(path), 
                                   MOD_NODE_TCID_STR(recv_mod_node), MOD_NODE_RANK(recv_mod_node));
            }
        }        
    }
    
    return (EC_TRUE);
}

EC_BOOL crfs_renew_r(const UINT32 crfs_md_id, const CSTRING *file_path, const UINT32 expires_timestamp, const UINT32 replica_num)
{
    CRFS_MD      *crfs_md;
    
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_renew_r: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CRFS_MAX_REPLICA_NUM < replica_num)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_renew_r: reject to renew file %s with invalid replica %ld\n", 
                           (char *)cstring_get_str(file_path), replica_num);
        return (EC_FALSE);
    }

    if(EC_FALSE == crfs_renew(crfs_md_id, file_path, expires_timestamp))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_renew_r: renew file %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);                           
    }

    if(1 >= replica_num)/*at least one replica. zero means default replicas*/
    {
        return (EC_TRUE);
    }

    if(EC_FALSE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        TASK_MGR *task_mgr;
        UINT32    mod_node_num;
        UINT32    mod_node_idx;  
        EC_BOOL   ret_vec[CRFS_MAX_REPLICA_NUM];

        mod_node_num = DMIN(CRFS_MAX_REPLICA_NUM, cvector_size(CRFS_MD_NEIGHBOR_VEC(crfs_md)));

        task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            ret_vec[ mod_node_idx ] = EC_FALSE;
            task_p2p_inc(task_mgr, crfs_md_id, recv_mod_node, 
                         &(ret_vec[ mod_node_idx ]), FI_crfs_renew,ERR_MODULE_ID, file_path, expires_timestamp);
        }
        task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            if(EC_FALSE == ret_vec[ mod_node_idx ])
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_renew_r: renew file %s at tcid %s rank %ld failed\n", 
                                   (char *)cstring_get_str(file_path), 
                                   MOD_NODE_TCID_STR(recv_mod_node), MOD_NODE_RANK(recv_mod_node));
            }
        }        
    }
    
    return (EC_TRUE);
}

EC_BOOL crfs_write_b_r(const UINT32 crfs_md_id, const CSTRING *file_path, uint64_t *offset, const CBYTES *cbytes, const UINT32 replica_num)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write_b_r: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);    

    if(CRFS_MAX_REPLICA_NUM < replica_num)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_b_r: reject to write file %s with invalid replica %ld\n", 
                           (char *)cstring_get_str(file_path), replica_num);
        return (EC_FALSE);
    }

    if(EC_FALSE == crfs_write_b(crfs_md_id, file_path, offset, cbytes))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_b_r: write file %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);                           
    }

    if(1 >= replica_num)/*at least one replica. zero means default replicas*/
    {
        return (EC_TRUE);
    }

    if(EC_FALSE == crfs_is_state(crfs_md_id, CRFS_SYNC_STATE))
    {
        TASK_MGR *task_mgr;
        UINT32    mod_node_num;
        UINT32    mod_node_idx;  
        EC_BOOL   ret[CRFS_MAX_REPLICA_NUM];

        mod_node_num = DMIN(replica_num, cvector_size(CRFS_MD_NEIGHBOR_VEC(crfs_md)));

        task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            ret[ mod_node_idx ] = EC_FALSE;
            task_p2p_inc(task_mgr, crfs_md_id, recv_mod_node, 
                        &(ret[ mod_node_idx ]), FI_crfs_write_b,ERR_MODULE_ID, file_path, offset, cbytes);
        }
        task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

        for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
        {
            MOD_NODE *recv_mod_node;

            recv_mod_node = cvector_get(CRFS_MD_NEIGHBOR_VEC(crfs_md), mod_node_idx);
            if(NULL_PTR == recv_mod_node)
            {
                continue;
            }

            if(EC_FALSE == ret[ mod_node_idx ])
            {
                dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_write_b_r: write file %s to tcid %s rank %ld failed\n", 
                                   (char *)cstring_get_str(file_path), 
                                   MOD_NODE_TCID_STR(recv_mod_node),MOD_NODE_RANK(recv_mod_node));
            }
        }        
    }
    return (EC_TRUE);
}

EC_BOOL crfs_np_snapshot(const UINT32 crfs_md_id, const UINT32 crfsnp_id, const CSTRING *des_path)
{
    CRFS_MD      *crfs_md;
    CSTRING       des_file;
    int           des_fd;

    uint32_t      crfsnp_id_t;
    CRFSNP       *crfsnp;
    UINT32        offset;

    TASK_BRD     *task_brd;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_np_snapshot: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_np_snapshot: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_id_t = (uint32_t)crfsnp_id;

    task_brd = task_brd_default_get();
    
    cstring_init(&des_file, NULL_PTR);
    cstring_format(&des_file, "%s/rfsnp_%04d_%4d%02d%02d_%02d%02d%02d.dat", 
                              (char *)cstring_get_str(des_path), 
                              crfsnp_id_t, 
                              TIME_IN_YMDHMS(TASK_BRD_CTM(task_brd)));
                              
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_np_snapshot: des file is %s\n", (char *)cstring_get_str(&des_file));

    des_fd = c_file_open((char *)cstring_get_str(&des_file), O_RDWR | O_CREAT, 0666);
    if(ERR_FD == des_fd)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_np_snapshot: open des file %s failed\n", (char *)cstring_get_str(&des_file));
        cstring_clean(&des_file);
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0299);
    crfsnp = crfsnp_mgr_open_np(CRFS_MD_NPP(crfs_md), crfsnp_id_t);
    if(NULL_PTR == crfsnp)
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0300);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_np_snapshot: open crfsnp %u failed\n", crfsnp_id_t);
        
        cstring_clean(&des_file);
        c_file_close(des_fd);
        return (EC_FALSE);
    }

    offset = 0;
    if(EC_FALSE == c_file_flush(des_fd, &offset, CRFSNP_FSIZE(crfsnp), (const uint8_t *)CRFSNP_HDR(crfsnp)))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0301);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_np_snapshot: flush crfsnp %u to file %s failed\n", 
                            crfsnp_id_t, (char *)cstring_get_str(&des_file));

        cstring_clean(&des_file);
        c_file_close(des_fd);
        return (EC_FALSE);
    }

    if(offset != CRFSNP_FSIZE(crfsnp))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0302);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_np_snapshot: flush crfsnp %u to file %s failed due to completed size %u != fsize %u\n", 
                            crfsnp_id_t, (char *)cstring_get_str(&des_file), offset, CRFSNP_FSIZE(crfsnp));

        cstring_clean(&des_file);
        c_file_close(des_fd);
        return (EC_FALSE);
    }
    
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0303);

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_np_snapshot: flush crfsnp %u to file %s done\n", 
                        crfsnp_id_t, (char *)cstring_get_str(&des_file));

    cstring_clean(&des_file);
    c_file_close(des_fd);

    return (EC_TRUE);
}

EC_BOOL crfs_npp_snapshot(const UINT32 crfs_md_id, const CSTRING *des_path)
{
    CRFS_MD      *crfs_md;

    CRFSNP_MGR   *crfsnp_mgr;
    uint32_t      crfsnp_id;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_npp_snapshot: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_npp_snapshot: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr = CRFS_MD_NPP(crfs_md);
    for(crfsnp_id = 0; crfsnp_id < CRFSNP_MGR_NP_MAX_NUM(crfsnp_mgr); crfsnp_id ++)
    {
        if(EC_FALSE == crfs_np_snapshot(crfs_md_id, crfsnp_id, des_path))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_npp_snapshot: snapshot crfsnp %u to path %s failed\n", 
                                crfsnp_id, (char *)cstring_get_str(des_path));
                                
            /*ignore error, snapshot next np*/
        }
    }    

    return (EC_TRUE);
}

EC_BOOL crfs_disk_snapshot(const UINT32 crfs_md_id, const UINT32 disk_no, const CSTRING *des_path)
{
    CRFS_MD      *crfs_md;

    CRFSDN       *crfsdn;
    CPGV         *cpgv;
    CPGD         *cpgd;
    
    CSTRING       des_file;
    int           des_fd;

    UINT32        offset;

    TASK_BRD     *task_brd;

    uint16_t      disk_no_t;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_disk_snapshot: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_disk_snapshot: dn was not open\n");
        return (EC_FALSE);
    }

    crfsdn  = CRFS_MD_DN(crfs_md);
    if(NULL_PTR == CRFSDN_CPGV(crfsdn))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_disk_snapshot: pgv is null\n");
        return (EC_FALSE);
    }
    
    cpgv      = CRFSDN_CPGV(crfsdn);

    disk_no_t = (uint16_t)disk_no;

    task_brd  = task_brd_default_get();

    cpgd = CPGV_DISK_CPGD(cpgv, disk_no_t);
    if(NULL_PTR == cpgd)
    {
        return (EC_TRUE);
    }

    cstring_init(&des_file, NULL_PTR);

    cstring_format(&des_file, "%s/dsk%04d_%4d%02d%02d_%02d%02d%02d.dat", 
                              (char *)cstring_get_str(des_path), 
                              disk_no_t,
                              TIME_IN_YMDHMS(TASK_BRD_CTM(task_brd)));
                              
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_disk_snapshot: des file is %s\n", (char *)cstring_get_str(&des_file));

    des_fd = c_file_open((char *)cstring_get_str(&des_file), O_RDWR | O_CREAT, 0666);
    if(ERR_FD == des_fd)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_disk_snapshot: open des file %s failed\n", (char *)cstring_get_str(&des_file));
        cstring_clean(&des_file);
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0304);
    offset = 0;
    if(EC_FALSE == c_file_flush(des_fd, &offset, CPGD_FSIZE(cpgd), (const uint8_t *)CPGD_HEADER(cpgd)))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0305);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_disk_snapshot: flush disk to file %s failed\n", 
                            (char *)cstring_get_str(&des_file));

        cstring_clean(&des_file);
        c_file_close(des_fd);
        return (EC_FALSE);
    }

    if(offset != CPGD_FSIZE(cpgd))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0306);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_disk_snapshot: flush disk to file %s failed due to completed size %u != fsize %u\n", 
                            (char *)cstring_get_str(&des_file), offset, CPGD_FSIZE(cpgd));

        cstring_clean(&des_file);
        c_file_close(des_fd);
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0307);

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_disk_snapshot: flush disk to file %s done\n", 
                        (char *)cstring_get_str(&des_file));

    cstring_clean(&des_file);
    c_file_close(des_fd);    

    return (EC_TRUE);
}

EC_BOOL crfs_dn_snapshot(const UINT32 crfs_md_id, const CSTRING *des_path)
{
    CRFS_MD      *crfs_md;

    CRFSDN       *crfsdn;
    
    uint16_t      disk_no;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_dn_snapshot: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_dn_snapshot: dn was not open\n");
        return (EC_FALSE);
    }

    crfsdn  = CRFS_MD_DN(crfs_md);
    if(NULL_PTR == CRFSDN_CPGV(crfsdn))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_dn_snapshot: pgv is null\n");
        return (EC_FALSE);
    }
    
    for(disk_no = 0; disk_no < CPGV_MAX_DISK_NUM; disk_no ++)
    {
        UINT32 disk_no_t;

        disk_no_t = disk_no;
        if(EC_FALSE == crfs_disk_snapshot(crfs_md_id, disk_no_t, des_path))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_dn_snapshot: snapshot disk %u to path %s failed\n", 
                                disk_no, (char *)cstring_get_str(des_path));
                                
            /*ignore error, snapshot next np*/
        }
    }    

    return (EC_TRUE);
}

EC_BOOL crfs_vol_snapshot(const UINT32 crfs_md_id, const CSTRING *des_path)
{
    CRFS_MD      *crfs_md;

    CRFSDN       *crfsdn;
    CPGV         *cpgv;
    
    CSTRING       des_file;
    int           des_fd;

    UINT32        offset;

    TASK_BRD     *task_brd;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_vol_snapshot: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_vol_snapshot: dn was not open\n");
        return (EC_FALSE);
    }

    crfsdn  = CRFS_MD_DN(crfs_md);
    if(NULL_PTR == CRFSDN_CPGV(crfsdn))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_vol_snapshot: pgv is null\n");
        return (EC_FALSE);
    }
    
    cpgv     = CRFSDN_CPGV(crfsdn);

    task_brd = task_brd_default_get();
    
    cstring_init(&des_file, NULL_PTR);

    cstring_format(&des_file, "%s/vol_%4d%02d%02d_%02d%02d%02d.dat", 
                              (char *)cstring_get_str(des_path), 
                              TIME_IN_YMDHMS(TASK_BRD_CTM(task_brd)));
                              
    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_vol_snapshot: des file is %s\n", (char *)cstring_get_str(&des_file));

    des_fd = c_file_open((char *)cstring_get_str(&des_file), O_RDWR | O_CREAT, 0666);
    if(ERR_FD == des_fd)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_vol_snapshot: open des file %s failed\n", (char *)cstring_get_str(&des_file));
        cstring_clean(&des_file);
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0308);
    offset = 0;
    if(EC_FALSE == c_file_flush(des_fd, &offset, CPGV_FSIZE(cpgv), (const uint8_t *)CPGV_HEADER(cpgv)))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0309);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_vol_snapshot: flush dn to file %s failed\n", 
                            (char *)cstring_get_str(&des_file));

        cstring_clean(&des_file);
        c_file_close(des_fd);
        return (EC_FALSE);
    }

    if(offset != CPGV_FSIZE(cpgv))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0310);
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_vol_snapshot: flush dn to file %s failed due to completed size %u != fsize %u\n", 
                            (char *)cstring_get_str(&des_file), offset, CPGV_FSIZE(cpgv));

        cstring_clean(&des_file);
        c_file_close(des_fd);
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0311);

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_vol_snapshot: flush dn to file %s done\n", 
                        (char *)cstring_get_str(&des_file));

    cstring_clean(&des_file);
    c_file_close(des_fd);
    
    return (EC_TRUE);
}

/*snapshot npp, dn, vol*/
EC_BOOL crfs_all_snapshot(const UINT32 crfs_md_id, const CSTRING *des_path)
{
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_vol_snapshot: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    if(EC_FALSE == crfs_npp_snapshot(crfs_md_id, des_path))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_all_snapshot: snapshot npp failed\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_all_snapshot: snapshot npp done\n");

    if(EC_FALSE == crfs_vol_snapshot(crfs_md_id, des_path))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_all_snapshot: snapshot vol failed\n");
        return (EC_FALSE);
    }    

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_all_snapshot: snapshot vol done\n");

    if(EC_FALSE == crfs_dn_snapshot(crfs_md_id, des_path))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_all_snapshot: snapshot vol failed\n");
        return (EC_FALSE);
    }   

    dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_all_snapshot: snapshot dn done\n");
    
    return (EC_TRUE);
}

EC_BOOL crfs_replay(const UINT32 crfs_md_id)
{
    CRFS_MD      *crfs_md;
    CRFSBK       *crfsbk;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_replay: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_replay: dn was not open\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "error:crfs_replay: npp was not open\n");
        return (EC_FALSE);
    }     

    if(NULL_PTR == CRFS_MD_BACKUP(crfs_md))
    {
        dbg_log(SEC_0031_CRFS, 1)(LOGSTDOUT, "error:crfs_replay: backup RFS was not open\n");
        return (EC_FALSE);
    }

    crfsbk = CRFS_MD_BACKUP(crfs_md);

    if(EC_FALSE == crfs_is_state(crfs_md_id, CRFS_WORK_STATE))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_replay: backup RFS is not in WORK state\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsbk_replay(crfsbk))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_replay: backup RFS replay failed\n");
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

/*------------------------------------------------ interface for liburl ------------------------------------------------*/
static EC_BOOL __crfs_open_url_list_file(const char *fname, char **fmem, UINT32 *fsize, int *fd)
{  
    char *cur_fmem;
    int   cur_fd;
    UINT32 cur_fsize;
    
    cur_fd = c_file_open(fname, O_RDONLY, 0666);
    if(ERR_FD == cur_fd)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_open_url_list_file: open url list file %s failed\n", fname);
        return (EC_FALSE);
    }

    if(EC_FALSE == c_file_size(cur_fd, &cur_fsize))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_open_url_list_file: get size of url list file %s failed\n", fname);
        c_file_close(cur_fd);
        return (EC_FALSE);
    }

    cur_fmem = (char *)mmap(NULL_PTR, cur_fsize, PROT_READ, MAP_SHARED, cur_fd, 0);
    if(MAP_FAILED == cur_fmem)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_open_url_list_file: mmap url list file %s with cur_fd %d failed, errno = %d, errorstr = %s\n", 
                           fname, cur_fd, errno, strerror(errno));
        return (EC_FALSE);
    }        

    (*fd)    = cur_fd;
    (*fmem)  = cur_fmem;
    (*fsize) = cur_fsize;

    return (EC_TRUE);
}

static EC_BOOL __crfs_close_url_list_file(char *fmem, const UINT32 fsize, const int fd)
{   
    if(ERR_FD != fd)
    {
        close(fd);
    }

    if(NULL_PTR != fmem)
    {
        munmap(fmem, fsize);
    }

    return (EC_TRUE);
}

static EC_BOOL __crfs_fetch_url_cstr(const char *fmem, const UINT32 fsize, UINT32 *offset, UINT32 *idx,CSTRING *url_cstr)
{
    UINT32 old_offset;
    UINT32 line_len;

    old_offset = (*offset);
    if(fsize <= old_offset)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:__crfs_fetch_url_cstr: offset %ld overflow fsize %ld\n", old_offset, fsize);
        return (EC_FALSE);
    }
    
    line_len = c_line_len(fmem + old_offset);    
    cstring_append_chars(url_cstr, line_len, (UINT8 *)fmem + old_offset);
    cstring_append_char(url_cstr, '\0');

    (*offset) += line_len + 1;
    (*idx) ++;

    dbg_log(SEC_0031_CRFS, 0)(LOGCONSOLE, "[DEBUG] __crfs_fetch_url_cstr: [%8d] %s\n", (*idx), (char *)cstring_get_str(url_cstr));

    return (EC_TRUE);
}

/**
*
*
* pull file from proxy_srv and forward(push) to rfs_srv, here url_list_fname feed url list:-)
*
* where rfs_srv provided by crfs module 
* and proxy_srv cstring format is <ip>:<port>
*
**/
EC_BOOL crfs_forward(const UINT32 crfs_md_id, const CSTRING *url_list_fname, const UINT32 rfs_srv_tcid, const CSTRING *proxy_srv)
{
    MOD_NODE rfs_srv_mod_node;
    UINT32   ccurl_md_id;
    UINT32   fail_counter;

    char   *fmem;
    UINT32  fsize;
    UINT32  offset;
    UINT32  url_idx;
    int     fd;
        
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_forward: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    ccurl_md_id = ccurl_start();
    if(ERR_MODULE_ID == ccurl_md_id)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: start ccurl module failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfs_open_url_list_file((char *)cstring_get_str(url_list_fname), &fmem, &fsize, &fd))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: open url list file '%s' failed\n", cstring_get_str(url_list_fname));
        ccurl_end(ccurl_md_id);
        return (EC_FALSE);
    }
    
    MOD_NODE_TCID(&rfs_srv_mod_node) = rfs_srv_tcid;
    MOD_NODE_COMM(&rfs_srv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&rfs_srv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&rfs_srv_mod_node) = 0;    

    offset  = 0;
    url_idx = 0;
    fail_counter = 0; /*init*/
    
    for(; fail_counter < 10; )
    {
        EC_BOOL  ret;

        CSTRING  url_cstr;   /*url cstring               */ 
        CSTRING  body_cstr;  /*reply body in cstring     */

        CSTRING  path_cstr;  /*path cstring from url_cstr*/ 
        CBYTES   body_bytes; /*reply body in cbytes      */

        cstring_init(&url_cstr, NULL_PTR);
        cstring_init(&body_cstr, NULL_PTR);
        cstring_init(&path_cstr, NULL_PTR);
        cbytes_init(&body_bytes);

        /*get url*/
        ret = __crfs_fetch_url_cstr(fmem, fsize, &offset, &url_idx, &url_cstr);                 
        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: get url from url list file %s failed\n", 
                               (char *)cstring_get_str(url_list_fname));   
                               
            c_usleep(1000, LOC_CRFS_0312);/*sleep 1 second*/

            fail_counter ++;
            continue;
        }

        /*when succ, reset counter*/
        fail_counter = 0;
#if 1

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_forward: curl '%s' from proxy %s beg\n", 
                           (char *)cstring_get_str(&url_cstr),
                           (char *)cstring_get_str(proxy_srv));
                           
        /*curl do*/
        if(EC_FALSE == ccurl_get(ccurl_md_id, &url_cstr, proxy_srv, &body_cstr))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: curl '%s' from proxy %s failed\n", 
                               (char *)cstring_get_str(&url_cstr),
                               (char *)cstring_get_str(proxy_srv));
                               
            cstring_clean(&body_cstr);
            cstring_clean(&url_cstr);

            continue;
        }

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_forward: curl '%s' [len %ld] from proxy %s done\n", 
                           (char *)cstring_get_str(&url_cstr),
                           cstring_get_len(&body_cstr),
                           (char *)cstring_get_str(proxy_srv));

        /*body: CSTRING -> CBYTES*/
        cbytes_mount(&body_bytes, cstring_get_len(&body_cstr), cstring_get_str(&body_cstr));

        /*url_cstr -> path_cstr*/
        cstring_set_str(&path_cstr, cstring_get_str(&url_cstr) + strlen("http://") - 1);

        /*rfs write*/
        ret = EC_FALSE;
        task_p2p(crfs_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, 
                 &rfs_srv_mod_node, 
                 &ret, FI_crfs_write, ERR_MODULE_ID, &path_cstr, &body_bytes, (UINT32)0);
        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: write '%s' with len %ld to rfs failed\n", 
                               (char *)cstring_get_str(&path_cstr), 
                               cbytes_len(&body_bytes));

            cbytes_umount(&body_bytes);
            cstring_clean(&body_cstr);
            cstring_clean(&url_cstr);

            continue;
        }      

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_forward: forward '%s' with body len %ld to rfs done\n", 
                           (char *)cstring_get_str(&url_cstr), 
                           cstring_get_len(&body_cstr));
#endif
        cbytes_umount(&body_bytes);
        cstring_clean(&body_cstr);
        cstring_clean(&url_cstr);        
    }

    __crfs_close_url_list_file(fmem, fsize, fd);

    ccurl_end(ccurl_md_id);

    return (EC_TRUE);
}

EC_BOOL crfs_forward1(const UINT32 crfs_md_id, const CSTRING *url_list_fname, const UINT32 rfs_srv_tcid, const CSTRING *proxy_srv)
{
    MOD_MGR *mod_mgr;
    MOD_NODE rfs_srv_mod_node;
    UINT32   ccurl_md_id;
    UINT32   fail_counter;

    char   *fmem;
    UINT32  fsize;
    UINT32  offset;
    UINT32  url_idx;
    int     fd;
        
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_forward: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    ccurl_md_id = ccurl_start();
    if(ERR_MODULE_ID == ccurl_md_id)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: start ccurl module failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfs_open_url_list_file((char *)cstring_get_str(url_list_fname), &fmem, &fsize, &fd))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: open url list file '%s' failed\n", cstring_get_str(url_list_fname));
        ccurl_end(ccurl_md_id);
        return (EC_FALSE);
    }

    mod_mgr = mod_mgr_new(crfs_md_id, LOAD_BALANCING_QUE);
    if(NULL_PTR == mod_mgr)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: new mod_mgr failed\n");
        ccurl_end(ccurl_md_id);
        return (EC_FALSE);
    }    

    mod_mgr_incl(rfs_srv_tcid, CMPI_ANY_COMM, 0, 0, mod_mgr);
    mod_mgr_incl(rfs_srv_tcid, CMPI_ANY_COMM, 1, 0, mod_mgr);
    
    MOD_NODE_TCID(&rfs_srv_mod_node) = rfs_srv_tcid;
    MOD_NODE_COMM(&rfs_srv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&rfs_srv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&rfs_srv_mod_node) = 0;    

    offset  = 0;
    url_idx = 0;
    fail_counter = 0; /*init*/
    
    for(; fail_counter < 10; )
    {
        EC_BOOL  ret;

        CSTRING  url_cstr;   /*url cstring               */ 
        CSTRING  body_cstr;  /*reply body in cstring     */

        CSTRING  path_cstr;  /*path cstring from url_cstr*/ 
        CBYTES   body_bytes; /*reply body in cbytes      */
        
        cstring_init(&url_cstr, NULL_PTR);
        cstring_init(&body_cstr, NULL_PTR);
        cstring_init(&path_cstr, NULL_PTR);
        cbytes_init(&body_bytes);

        /*get url*/
        ret = __crfs_fetch_url_cstr(fmem, fsize, &offset, &url_idx, &url_cstr);                 
        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: get url from url list file %s failed\n", 
                               (char *)cstring_get_str(url_list_fname));   
                               
            c_usleep(1000, LOC_CRFS_0313);/*sleep 1 second*/

            fail_counter ++;
            continue;
        }

        /*when succ, reset counter*/
        fail_counter = 0;
#if 1
        /*curl do*/
        if(EC_FALSE == ccurl_get(ccurl_md_id, &url_cstr, proxy_srv, &body_cstr))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: curl '%s' from proxy %s failed\n", 
                               (char *)cstring_get_str(&url_cstr),
                               (char *)cstring_get_str(proxy_srv));
                               
            cstring_clean(&body_cstr);
            cstring_clean(&url_cstr);

            continue;
        }

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_forward: curl '%s' [len %ld] from proxy %s done\n", 
                           (char *)cstring_get_str(&url_cstr),
                           cstring_get_len(&body_cstr),
                           (char *)cstring_get_str(proxy_srv));

        /*body: CSTRING -> CBYTES*/
        cbytes_mount(&body_bytes, cstring_get_len(&body_cstr), cstring_get_str(&body_cstr));

        /*url_cstr -> path_cstr*/
        cstring_set_str(&path_cstr, cstring_get_str(&url_cstr) + strlen("http://") - 1);

        /*rfs write*/
        ret = EC_FALSE;

        task_mono(mod_mgr,
                  TASK_DEFAULT_LIVE, TASK_PRIO_HIGH,
                  TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP, TASK_NOT_NEED_RESCHEDULE_FLAG,
                  &ret, FI_crfs_write, ERR_MODULE_ID, &path_cstr, &body_bytes, (UINT32)0);
        
        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_forward: write '%s' with len %ld to rfs failed\n", 
                               (char *)cstring_get_str(&path_cstr), 
                               cbytes_len(&body_bytes));

            cbytes_umount(&body_bytes);
            cstring_clean(&body_cstr);
            cstring_clean(&url_cstr);

            continue;
        }      

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_forward: forward '%s' with body len %ld to rfs done\n", 
                           (char *)cstring_get_str(&url_cstr), 
                           cstring_get_len(&body_cstr));
#endif
        cbytes_umount(&body_bytes);
        cstring_clean(&body_cstr);
        cstring_clean(&url_cstr);        
    }

    __crfs_close_url_list_file(fmem, fsize, fd);

    mod_mgr_free(mod_mgr);
    ccurl_end(ccurl_md_id);

    return (EC_TRUE);
}

EC_BOOL crfs_cmp_finger(const UINT32 crfs_md_id, const CSTRING *url_list_fname, const UINT32 rfs_srv_tcid, const CSTRING *proxy_srv)
{
    MOD_NODE rfs_srv_mod_node;
    UINT32   ccurl_md_id;
    UINT32   fail_counter;

    char   *fmem;
    UINT32  fsize;
    UINT32  offset;
    UINT32  url_idx;
    int     fd;
        
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_cmp_finger: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    ccurl_md_id = ccurl_start();
    if(ERR_MODULE_ID == ccurl_md_id)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_cmp_finger: start ccurl module failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfs_open_url_list_file((char *)cstring_get_str(url_list_fname), &fmem, &fsize, &fd))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_cmp_finger: open url list file '%s' failed\n", cstring_get_str(url_list_fname));
        ccurl_end(ccurl_md_id);
        return (EC_FALSE);
    }
    
    MOD_NODE_TCID(&rfs_srv_mod_node) = rfs_srv_tcid;
    MOD_NODE_COMM(&rfs_srv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&rfs_srv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&rfs_srv_mod_node) = 0;    

    offset  = 0;
    url_idx = 0;
    fail_counter = 0; /*init*/
    
    for(; fail_counter < 10; )
    {
        EC_BOOL  ret;

        CSTRING  url_cstr;   /*url cstring               */ 
        CSTRING  body_cstr;  /*reply body in cstring     */

        CSTRING  path_cstr;  /*path cstring from url_cstr*/ 
        CBYTES   body_bytes; /*reply body in cbytes      */
        UINT32   expires_timestamp;
        EC_BOOL  need_expired_content;

        cstring_init(&url_cstr, NULL_PTR);
        cstring_init(&body_cstr, NULL_PTR);
        cstring_init(&path_cstr, NULL_PTR);
        cbytes_init(&body_bytes);

        /*get url*/
        ret = __crfs_fetch_url_cstr(fmem, fsize, &offset, &url_idx, &url_cstr);                 
        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_cmp_finger: get url from url list file %s failed\n", 
                               (char *)cstring_get_str(url_list_fname));   
                               
            c_usleep(1000, LOC_CRFS_0314);/*sleep 1 second*/

            fail_counter ++;
            continue;
        }

        /*when succ, reset counter*/
        fail_counter = 0;
#if 1

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_cmp_finger: curl '%s' from proxy %s beg\n", 
                           (char *)cstring_get_str(&url_cstr),
                           (char *)cstring_get_str(proxy_srv));
                           
        /*curl do*/
        if(EC_FALSE == ccurl_get(ccurl_md_id, &url_cstr, proxy_srv, &body_cstr))
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_cmp_finger: curl '%s' from proxy %s failed\n", 
                               (char *)cstring_get_str(&url_cstr),
                               (char *)cstring_get_str(proxy_srv));
                               
            cstring_clean(&body_cstr);
            cstring_clean(&url_cstr);

            continue;
        }

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_cmp_finger: curl '%s' [len %ld] from proxy %s done\n", 
                           (char *)cstring_get_str(&url_cstr),
                           cstring_get_len(&body_cstr),
                           (char *)cstring_get_str(proxy_srv));

        /*url_cstr -> path_cstr*/
        cstring_set_str(&path_cstr, cstring_get_str(&url_cstr) + strlen("http://") - 1);

        /*rfs read*/
        ret = EC_FALSE;
        need_expired_content = EC_TRUE;
        task_p2p(crfs_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, 
                 &rfs_srv_mod_node, 
                 &ret, FI_crfs_read, ERR_MODULE_ID, &path_cstr, &body_bytes, &expires_timestamp, need_expired_content);
        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_cmp_finger: read '%s' from rfs failed\n", 
                               (char *)cstring_get_str(&path_cstr));

            cbytes_clean(&body_bytes, LOC_CRFS_0315);
            cstring_clean(&body_cstr);
            cstring_clean(&url_cstr);

            continue;
        }      

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_cmp_finger: finger '%s' with body len %ld from rfs done\n", 
                           (char *)cstring_get_str(&url_cstr), 
                           cbytes_len(&body_bytes));

        if(1)
        {
            uint8_t   digest_from_curl[ CMD5_DIGEST_LEN ];
            uint8_t   digest_from_crfs[ CMD5_DIGEST_LEN ];
            
            cmd5_sum(cstring_get_len(&body_cstr), cstring_get_str(&body_cstr), digest_from_curl);
            cmd5_sum(cbytes_len(&body_bytes), cbytes_buf(&body_bytes), digest_from_crfs);

            if(0 != BCMP(digest_from_curl, digest_from_crfs, CMD5_DIGEST_LEN))
            {
                sys_log(LOGUSER05, "[DEBUG] crfs_cmp_finger: [FAIL] %s\n", (char *)cstring_get_str(&url_cstr));
            }
            else
            {
                sys_log(LOGUSER05, "[DEBUG] crfs_cmp_finger: [SUCC] %s\n", (char *)cstring_get_str(&url_cstr));
            }
        }
#endif
        cbytes_clean(&body_bytes, LOC_CRFS_0316);
        cstring_clean(&body_cstr);
        cstring_clean(&url_cstr);        
    }

    __crfs_close_url_list_file(fmem, fsize, fd);

    ccurl_end(ccurl_md_id);

    return (EC_TRUE);
}

/*debug only*/
EC_BOOL crfs_cleanup(const UINT32 crfs_md_id, const CSTRING *url_list_fname, const UINT32 rfs_srv_tcid)
{
    MOD_NODE rfs_srv_mod_node;
    UINT32   ccurl_md_id;
    UINT32   fail_counter;

    char   *fmem;
    UINT32  fsize;
    UINT32  offset;
    UINT32  url_idx;
    int     fd;
        
#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_cleanup: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    ccurl_md_id = ccurl_start();
    if(ERR_MODULE_ID == ccurl_md_id)
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_cleanup: start ccurl module failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfs_open_url_list_file((char *)cstring_get_str(url_list_fname), &fmem, &fsize, &fd))
    {
        dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_cleanup: open url list file '%s' failed\n", cstring_get_str(url_list_fname));
        ccurl_end(ccurl_md_id);
        return (EC_FALSE);
    }
    
    MOD_NODE_TCID(&rfs_srv_mod_node) = rfs_srv_tcid;
    MOD_NODE_COMM(&rfs_srv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&rfs_srv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&rfs_srv_mod_node) = 0;    

    offset  = 0;
    url_idx = 0;
    fail_counter = 0; /*init*/
    
    for(; fail_counter < 10; )
    {
        EC_BOOL  ret;

        CSTRING  url_cstr;   /*url cstring               */ 
        CSTRING  path_cstr;  /*path cstring from url_cstr*/ 

        cstring_init(&url_cstr, NULL_PTR);
        cstring_init(&path_cstr, NULL_PTR);

        /*get url*/
        ret = __crfs_fetch_url_cstr(fmem, fsize, &offset, &url_idx, &url_cstr);                 
        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_cleanup: get url from url list file %s failed\n", 
                               (char *)cstring_get_str(url_list_fname));   
                               
            c_usleep(1000, LOC_CRFS_0317);/*sleep 1 second*/

            fail_counter ++;
            continue;
        }

        /*when succ, reset counter*/
        fail_counter = 0;

        dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_cleanup: delete '%s' from rfs beg\n", (char *)cstring_get_str(&url_cstr));

        /*url_cstr -> path_cstr*/
        cstring_set_str(&path_cstr, cstring_get_str(&url_cstr) + strlen("http://") - 1);

        /*rfs delete*/
        ret = EC_FALSE;
        task_p2p(crfs_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, 
                 &rfs_srv_mod_node, 
                 &ret, FI_crfs_delete, ERR_MODULE_ID, &path_cstr, (UINT32)CRFSNP_ITEM_FILE_IS_REG);
#if 1
        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0031_CRFS, 0)(LOGSTDOUT, "error:crfs_cleanup: delete '%s' from rfs failed\n", 
                               (char *)cstring_get_str(&path_cstr));

            //cstring_clean(&url_cstr);
            //continue;
        }
        else
        {
            dbg_log(SEC_0031_CRFS, 9)(LOGSTDOUT, "[DEBUG] crfs_cleanup: delete '%s' from rfs done\n", (char *)cstring_get_str(&url_cstr));    
        }
#endif

        if(EC_FALSE == ret)
        {
            sys_log(LOGUSER05, "[DEBUG] crfs_cleanup: [FAIL] %s\n", (char *)cstring_get_str(&url_cstr));
        }
        else
        {
            sys_log(LOGUSER05, "[DEBUG] crfs_cleanup: [SUCC] %s\n", (char *)cstring_get_str(&url_cstr));
        }

        cstring_clean(&url_cstr);        
    }

    __crfs_close_url_list_file(fmem, fsize, fd);

    ccurl_end(ccurl_md_id);

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

