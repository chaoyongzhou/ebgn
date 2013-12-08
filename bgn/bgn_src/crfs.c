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
#include <malloc.h>
#include <errno.h>

#include <sys/stat.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "carray.h"
#include "cvector.h"

#include "cbc.h"

#include "cmisc.h"

#include "task.h"

#include "csocket.h"

#include "cmpie.h"

#include "crfs.h"
#include "cload.h"

#include "cfuse.h"

#include "findex.inc"

#define CRFS_MD_CAPACITY()                  (cbc_md_capacity(MD_CRFS))

#define CRFS_MD_GET(crfs_md_id)     ((CRFS_MD *)cbc_md_get(MD_CRFS, (crfs_md_id)))

#define CRFS_MD_ID_CHECK_INVALID(crfs_md_id)  \
    ((CMPI_ANY_MODI != (crfs_md_id)) && ((NULL_PTR == CRFS_MD_GET(crfs_md_id)) || (0 == (CRFS_MD_GET(crfs_md_id)->usedcounter))))

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
UINT32 crfs_start(const CSTRING *crfsnp_root_dir, const CSTRING *crfsdn_root_dir)
{
    CRFS_MD *crfs_md;
    UINT32   crfs_md_id;

    TASK_BRD *task_brd;
    EC_BOOL   ret;

    task_brd = task_brd_default_get();
    
    crfs_md_id = cbc_md_new(MD_CRFS, sizeof(CRFS_MD));
    if(ERR_MODULE_ID == crfs_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one CRFS module */
    crfs_md = (CRFS_MD *)cbc_md_get(MD_CRFS, crfs_md_id);
    crfs_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();    

    CRFS_MD_DN_MOD_MGR(crfs_md)  = mod_mgr_new(crfs_md_id, /*LOAD_BALANCING_LOOP*//*LOAD_BALANCING_MOD*/LOAD_BALANCING_QUE);
    CRFS_MD_NPP_MOD_MGR(crfs_md) = mod_mgr_new(crfs_md_id, /*LOAD_BALANCING_LOOP*//*LOAD_BALANCING_MOD*/LOAD_BALANCING_QUE);
    
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
            sys_log(LOGSTDOUT, "error:crfs_start: open npp from root dir %s failed\n", (char *)cstring_get_str(crfsnp_root_dir));
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
            sys_log(LOGSTDOUT, "error:crfs_start: open dn with root dir %s failed\n", (char *)cstring_get_str(crfsdn_root_dir));
            ret = EC_FALSE;
        }    
    }

    if(EC_FALSE == ret)
    {
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

    crfs_md->usedcounter = 1;

    sys_log(LOGSTDOUT, "crfs_start: start CRFS module #%u\n", crfs_md_id);

    CRFS_INIT_LOCK(crfs_md, LOC_CRFS_0001);

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

    crfs_md = CRFS_MD_GET(crfs_md_id);
    if(NULL_PTR == crfs_md)
    {
        sys_log(LOGSTDOUT,"error:crfs_end: crfs_md_id = %u not exist.\n", crfs_md_id);
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
        sys_log(LOGSTDOUT,"error:crfs_end: crfs_md_id = %u is not started.\n", crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
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
    
    /* free module : */
    //crfs_free_module_static_mem(crfs_md_id);

    crfs_md->usedcounter = 0;
    CRFS_CLEAN_LOCK(crfs_md, LOC_CRFS_0002);

    sys_log(LOGSTDOUT, "crfs_end: stop CRFS module #%u\n", crfs_md_id);
    cbc_md_free(MD_CRFS, crfs_md_id);

    return ;
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

    sys_log(LOGSTDOUT, "crfs_set_npp_mod_mgr: md_id %d, input src_mod_mgr %lx\n", crfs_md_id, src_mod_mgr);
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    /*figure out mod_nodes with tcid belong to set of crfsnp_tcid_vec and crfsnp_tcid_vec*/
    mod_mgr_limited_clone(crfs_md_id, src_mod_mgr, des_mod_mgr);

    sys_log(LOGSTDOUT, "====================================crfs_set_npp_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    sys_log(LOGSTDOUT, "====================================crfs_set_npp_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);

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

    sys_log(LOGSTDOUT, "crfs_set_dn_mod_mgr: md_id %d, input src_mod_mgr %lx\n", crfs_md_id, src_mod_mgr);
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    /*figure out mod_nodes with tcid belong to set of crfsnp_tcid_vec and crfsnp_tcid_vec*/
    mod_mgr_limited_clone(crfs_md_id, src_mod_mgr, des_mod_mgr);

    sys_log(LOGSTDOUT, "====================================crfs_set_dn_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    sys_log(LOGSTDOUT, "====================================crfs_set_dn_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);

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
        sys_log(LOGSTDOUT, "error:crfs_open_npp: npp was open\n");
        return (EC_FALSE);
    }

    CRFS_MD_NPP(crfs_md) = crfsnp_mgr_open(crfsnp_db_root_dir);
    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:crfs_open_npp: open npp from root dir %s failed\n", (char *)cstring_get_str(crfsnp_db_root_dir));
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
        sys_log(LOGSTDOUT, "warn:crfs_close_npp: npp was not open\n");
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

static EC_BOOL __crfs_check_is_uint8_t(const UINT32 num)
{
    if(0 == (num >> 8))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

static EC_BOOL __crfs_check_is_uint16_t(const UINT32 num)
{
    if(0 == (num >> 16))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

static EC_BOOL __crfs_check_is_uint32_t(const UINT32 num)
{
#if (32 == WORDSIZE)
    return (EC_TRUE);
#endif /*(32 == WORDSIZE)*/

#if (64 == WORDSIZE)
    if(32 == WORDSIZE || 0 == (num >> 32))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
#endif /*(64 == WORDSIZE)*/
}
 
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
        sys_log(LOGSTDOUT, "error:crfs_create_npp: npp already exist\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfs_check_is_uint8_t(crfsnp_model))
    {
        sys_log(LOGSTDOUT, "error:crfs_create_npp: crfsnp_model %u is invalid\n", crfsnp_model);
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfs_check_is_uint32_t(crfsnp_max_num))
    {
        sys_log(LOGSTDOUT, "error:crfs_create_npp: crfsnp_disk_max_num %u is invalid\n", crfsnp_max_num);
        return (EC_FALSE);
    }   

    if(EC_FALSE == __crfs_check_is_uint8_t(crfsnp_1st_chash_algo_id))
    {
        sys_log(LOGSTDOUT, "error:crfs_create_npp: crfsnp_1st_chash_algo_id %u is invalid\n", crfsnp_1st_chash_algo_id);
        return (EC_FALSE);
    }    

    if(EC_FALSE == __crfs_check_is_uint8_t(crfsnp_2nd_chash_algo_id))
    {
        sys_log(LOGSTDOUT, "error:crfs_create_npp: crfsnp_2nd_chash_algo_id %u is invalid\n", crfsnp_2nd_chash_algo_id);
        return (EC_FALSE);
    }    

    CRFS_MD_NPP(crfs_md) = crfsnp_mgr_create((uint8_t ) crfsnp_model, 
                                             (uint32_t) crfsnp_max_num, 
                                             (uint8_t ) crfsnp_1st_chash_algo_id, 
                                             (uint8_t ) crfsnp_2nd_chash_algo_id, 
                                             crfsnp_db_root_dir);
    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:crfs_create_npp: create npp failed\n");
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
        sys_log(LOGSTDOUT, "warn:crfs_add_npp: crfsnpp_tcid %s not connected\n", c_word_to_ipv4(crfsnpp_tcid));
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
        sys_log(LOGSTDOUT, "warn:crfs_add_dn: crfsdn_tcid %s not connected\n", c_word_to_ipv4(crfsdn_tcid));
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
        sys_log(LOGSTDOUT, "warn:crfs_find_dir: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0003);
    ret = crfsnp_mgr_find_dir(CRFS_MD_NPP(crfs_md), dir_path);
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0004);

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
        sys_log(LOGSTDOUT, "warn:crfs_find_file: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0005);
    ret = crfsnp_mgr_find_file(CRFS_MD_NPP(crfs_md), file_path);
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0006);
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
        sys_log(LOGSTDOUT, "warn:crfs_find: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0007);
    ret = crfsnp_mgr_find(CRFS_MD_NPP(crfs_md), path, CRFSNP_ITEM_FILE_IS_ANY);
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0008);

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
*  set/bind home dir of a name node
*
**/
EC_BOOL crfs_set(const UINT32 crfs_md_id, const CSTRING *home_dir, const UINT32 crfsnp_id)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_set: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(EC_FALSE == __crfs_check_is_uint32_t(crfsnp_id))
    {
        sys_log(LOGSTDOUT, "error:crfs_set: crfsnp_id %u is invalid\n", crfsnp_id);
        return (EC_FALSE);
    } 

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:crfs_set: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0009);
    if(EC_FALSE == crfsnp_mgr_bind(CRFS_MD_NPP(crfs_md), home_dir, crfsnp_id))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0010);
        sys_log(LOGSTDOUT, "error:crfs_set: bind home '%s' to np %u failed\n", (char *)cstring_get_str(home_dir), crfsnp_id);
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0011);
    return (EC_TRUE);
}

/**
*
*  write a file
*
**/
EC_BOOL crfs_write(const UINT32 crfs_md_id, const CSTRING *file_path, const CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;
    CRFSNP_FNODE  crfsnp_fnode;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_write: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    crfsnp_fnode_init(&crfsnp_fnode);

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0012);

    if(EC_FALSE == crfs_write_dn(crfs_md_id, cbytes, &crfsnp_fnode))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0013);
        sys_log(LOGSTDOUT, "error:crfs_write: write file %s content to dn failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfs_write: write file %s to dn where fnode is \n", (char *)cstring_get_str(file_path));
    crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);   

    sys_log(LOGSTDOUT, "[DEBUG] crfs_write: write file %s is %.*s\n", (char *)cstring_get_str(file_path), DMIN(16, cbytes_len(cbytes)), cbytes_buf(cbytes));

    if(EC_FALSE == crfs_write_npp(crfs_md_id, file_path, &crfsnp_fnode))
    {   
        __crfs_delete_dn(crfs_md_id, &crfsnp_fnode);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0014);
        sys_log(LOGSTDOUT, "error:crfs_write: write file %s to npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0015);
    return (EC_TRUE);
}

/**
*
*  read a file
*
**/
EC_BOOL crfs_read(const UINT32 crfs_md_id, const CSTRING *file_path, CBYTES *cbytes)
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
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0016);
    if(EC_FALSE == crfs_read_npp(crfs_md_id, file_path, &crfsnp_fnode))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0017);
        sys_log(LOGSTDOUT, "error:crfs_read: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfs_read: read file %s from npp and fnode %p is \n", (char *)cstring_get_str(file_path), &crfsnp_fnode);
    crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);

    if(EC_FALSE == crfs_read_dn(crfs_md_id, &crfsnp_fnode, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0018);
        sys_log(LOGSTDOUT, "error:crfs_read: read file %s from dn failed where fnode is ", (char *)cstring_get_str(file_path));
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfs_read: read file %s is %.*s\n", (char *)cstring_get_str(file_path), DMIN(16, cbytes_len(cbytes)), cbytes_buf(cbytes));
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0019);
    return (EC_TRUE);
}

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
    uint32_t file_old_size;

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

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0020);

    if(EC_FALSE == crfs_read_npp(crfs_md_id, file_path, &crfsnp_fnode))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0021);
        sys_log(LOGSTDOUT, "error:crfs_write_e: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    file_old_size = CRFSNP_FNODE_FILESZ(&crfsnp_fnode);

    if(EC_FALSE == crfs_write_e_dn(crfs_md_id, &crfsnp_fnode, offset, max_len, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0022);
        sys_log(LOGSTDOUT, "error:crfs_write_e: offset write to dn failed\n");
        return (EC_FALSE);
    }

    if(file_old_size != CRFSNP_FNODE_FILESZ(&crfsnp_fnode))
    {
        if(EC_FALSE == crfs_update_npp(crfs_md_id, file_path, &crfsnp_fnode))
        {
            CRFS_UNLOCK(crfs_md, LOC_CRFS_0023);
            sys_log(LOGSTDOUT, "error:crfs_write_e: offset write file %s to npp failed\n", (char *)cstring_get_str(file_path));
            return (EC_FALSE);
        }
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0024);
    return (EC_TRUE);
}

/**
*
*  read a file from offset
*
**/
EC_BOOL crfs_read_e(const UINT32 crfs_md_id, const CSTRING *file_path, UINT32 *offset, const UINT32 max_len, CBYTES *cbytes)
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

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0025);
    
    if(EC_FALSE == crfs_read_npp(crfs_md_id, file_path, &crfsnp_fnode))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0026);
        sys_log(LOGSTDOUT, "error:crfs_read_e: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    if(EC_FALSE == crfs_read_e_dn(crfs_md_id, &crfsnp_fnode, offset, max_len, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0027);
        sys_log(LOGSTDOUT, "error:crfs_read_e: offset read file %s from dn failed where fnode is\n", (char *)cstring_get_str(file_path));
        crfsnp_fnode_print(LOGSTDOUT, &crfsnp_fnode);
        return (EC_FALSE);
    }
    
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0028);
    return (EC_TRUE);
}

/*----------------------------------- BIG FILE interface -----------------------------------*/
/**
*
*  create a big file at offset
*
**/
EC_BOOL crfs_create_b(const UINT32 crfs_md_id, const CSTRING *file_path)
{
    CRFS_MD      *crfs_md;
    
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

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0029);

    if(EC_FALSE == __crfs_write_b_npp(crfs_md_id, file_path))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0030);
        sys_log(LOGSTDOUT, "error:crfs_create_b: create big file %s to npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0031);
    return (EC_TRUE);
}

/**
*
*  write a big file at offset
*
**/
EC_BOOL crfs_write_b(const UINT32 crfs_md_id, const CSTRING *file_path, uint64_t *offset, const UINT32 max_len, const CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;
    
    uint32_t crfsnp_id;
    uint32_t parent_pos;

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

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0032);

    if(EC_FALSE == __crfs_read_b_npp(crfs_md_id, file_path, &crfsnp_id, &parent_pos))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0033);
        sys_log(LOGSTDOUT, "error:crfs_write_b: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfs_write_b_dn(crfs_md_id, crfsnp_id, parent_pos, offset, max_len, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0034);
        sys_log(LOGSTDOUT, "error:crfs_write_b: offset write to dn failed\n");
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0035);
    return (EC_TRUE);
}

/**
*
*  read a file from offset
*
**/
EC_BOOL crfs_read_b(const UINT32 crfs_md_id, const CSTRING *file_path, uint64_t *offset, const UINT32 max_len, CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;
    
    uint32_t crfsnp_id;
    uint32_t parent_pos;

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

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0036);
  
    if(EC_FALSE == __crfs_read_b_npp(crfs_md_id, file_path, &crfsnp_id, &parent_pos))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0037);
        sys_log(LOGSTDOUT, "error:crfs_read_b: read file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfs_read_b_dn(crfs_md_id, crfsnp_id, parent_pos, offset, max_len, cbytes))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0038);
        sys_log(LOGSTDOUT, "error:crfs_read_b: offset read file %s from dn failed where bnode is\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0039);
    return (EC_TRUE);
}


/**
*
*  create data node
*
**/
EC_BOOL crfs_create_dn(const UINT32 crfs_md_id, const CSTRING *root_dir, const UINT32 max_gb_num_of_disk_space)
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
        sys_log(LOGSTDOUT, "error:crfs_create_dn: dn already exist\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfs_check_is_uint16_t(max_gb_num_of_disk_space))
    {
        sys_log(LOGSTDOUT, "error:crfs_create_dn: max_gb_num_of_disk_space %u is invalid\n", max_gb_num_of_disk_space);
        return (EC_FALSE);
    }

    CRFS_MD_DN(crfs_md) = crfsdn_create((char *)cstring_get_str(root_dir), max_gb_num_of_disk_space);
    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:crfs_create_dn: create dn failed\n");
        return (EC_FALSE);
    }

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
    sys_log(LOGSTDOUT, "[DEBUG] crfs_open_dn: try to open dn %s  ...\n", (char *)cstring_get_str(root_dir));

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR != CRFS_MD_DN(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:crfs_open_dn: dn was open\n");
        return (EC_FALSE);
    }

    CRFS_MD_DN(crfs_md) = crfsdn_open((char *)cstring_get_str(root_dir));
    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:crfs_open_dn: open dn with root dir %s failed\n", (char *)cstring_get_str(root_dir));
        return (EC_FALSE);
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfs_open_dn: open dn %s\n", (char *)cstring_get_str(root_dir));
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
        sys_log(LOGSTDOUT, "error:crfs_close_dn: no dn was open\n");
        return (EC_FALSE);
    }

    crfsdn_close(CRFS_MD_DN(crfs_md));
    CRFS_MD_DN(crfs_md) = NULL_PTR;
    sys_log(LOGSTDOUT, "[DEBUG] crfs_close_dn: dn was closed\n");

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
        sys_log(LOGSTDOUT, "error:crfs_write_dn: buff len (or file size) %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:crfs_write_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0040);
    if(EC_FALSE == crfsdn_write_p(CRFS_MD_DN(crfs_md), cbytes_len(cbytes), cbytes_buf(cbytes), &disk_no, &block_no, &page_no))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0041);
        
        sys_log(LOGSTDOUT, "error:crfs_write_dn: write %u bytes to dn failed\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0042);

    crfsnp_fnode_init(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    CRFSNP_INODE_DISK_NO(crfsnp_inode)  = disk_no;
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode) = block_no;
    CRFSNP_INODE_PAGE_NO(crfsnp_inode)  = page_no;

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
        sys_log(LOGSTDOUT, "error:crfs_read_dn: dn is null\n");
        return (EC_FALSE);
    }

    if(0 == CRFSNP_FNODE_REPNUM(crfsnp_fnode))
    {
        sys_log(LOGSTDOUT, "error:crfs_read_dn: no replica\n");
        return (EC_FALSE);
    }

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;

    sys_log(LOGSTDOUT, "[DEBUG] crfs_read_dn: file size %u, disk %u, block %u, page %u\n", file_size, disk_no, block_no, page_no);

    if(CBYTES_LEN(cbytes) < file_size)
    {
        if(NULL_PTR != CBYTES_BUF(cbytes))
        {
            SAFE_FREE(CBYTES_BUF(cbytes), LOC_CRFS_0043);
        }
        CBYTES_BUF(cbytes) = (UINT8 *)SAFE_MALLOC(file_size, LOC_CRFS_0044);
        CBYTES_LEN(cbytes) = 0;
    }

    crfsdn_rdlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0045);
    if(EC_FALSE == crfsdn_read_p(CRFS_MD_DN(crfs_md), disk_no, block_no, page_no, file_size, CBYTES_BUF(cbytes), &(CBYTES_LEN(cbytes))))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0046);
        
        sys_log(LOGSTDOUT, "error:crfs_read_dn: read %u bytes from disk %u, block %u, page %u failed\n", 
                           file_size, disk_no, block_no, page_no);
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0047);
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
        sys_log(LOGSTDOUT, "error:crfs_write_e_dn: offset %u + buff len (or file size) %u = %u overflow\n", 
                            (*offset), CBYTES_LEN(cbytes), (*offset) + CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:crfs_write_e_dn: no dn was open\n");
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
        sys_log(LOGSTDOUT, "error:crfs_write_e_dn: offset %u overflow due to file max size is %u\n", (*offset), file_max_size);
        return (EC_FALSE);
    }    
    
    offset_t  = (uint32_t)(*offset);
    max_len_t = DMIN(DMIN(max_len, file_max_size - offset_t), cbytes_len(cbytes));    

    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0048);
    if(EC_FALSE == crfsdn_write_e(CRFS_MD_DN(crfs_md), max_len_t, cbytes_buf(cbytes), disk_no, block_no, page_no, offset_t))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0049);
        
        sys_log(LOGSTDOUT, "error:crfs_write_e_dn: write %u bytes to dn failed\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0050);

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
        sys_log(LOGSTDOUT, "error:crfs_read_e_dn: dn is null\n");
        return (EC_FALSE);
    }

    if(0 == CRFSNP_FNODE_REPNUM(crfsnp_fnode))
    {
        sys_log(LOGSTDOUT, "error:crfs_read_e_dn: no replica\n");
        return (EC_FALSE);
    }

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;    

    if((*offset) >= file_size)
    {
        sys_log(LOGSTDOUT, "error:crfs_read_e_dn: due to offset %u >= file size %u\n", (*offset), file_size);
        return (EC_FALSE);
    }

    offset_t = (uint32_t)(*offset);
    max_len_t = DMIN(max_len, file_size - offset_t);

    sys_log(LOGSTDOUT, "[DEBUG] crfs_read_e_dn: file size %u, disk %u, block %u, page %u, offset %u, max len %u\n", 
                        file_size, disk_no, block_no, page_no, offset_t, max_len_t);

    if(CBYTES_LEN(cbytes) < max_len_t)
    {
        if(NULL_PTR != CBYTES_BUF(cbytes))
        {
            SAFE_FREE(CBYTES_BUF(cbytes), LOC_CRFS_0051);
        }
        CBYTES_BUF(cbytes) = (UINT8 *)SAFE_MALLOC(max_len_t, LOC_CRFS_0052);
        CBYTES_LEN(cbytes) = 0;
    }    

    crfsdn_rdlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0053);
    if(EC_FALSE == crfsdn_read_e(CRFS_MD_DN(crfs_md), disk_no, block_no, page_no, offset_t, max_len_t, CBYTES_BUF(cbytes), &(CBYTES_LEN(cbytes))))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0054);
        
        sys_log(LOGSTDOUT, "error:crfs_read_e_dn: read %u bytes from disk %u, block %u, offset %u failed\n", 
                           max_len_t, disk_no, block_no, offset_t);
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0055);

    (*offset) += CBYTES_LEN(cbytes);
    return (EC_TRUE);
}

/*when found missed segment in bnode*/
static EC_BOOL __crfs_write_b_seg_dn_miss(const UINT32 crfs_md_id, 
                                              CRFSNP *crfsnp, 
                                              const uint32_t parent_pos, 
                                              UINT32 *offset, const CBYTES *cbytes,
                                              const uint32_t key_1st_hash, const uint32_t key_2nd_hash, 
                                              const uint32_t klen, const uint8_t *key)
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
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn_miss: buff len %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }   

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn_miss: offset %u overflow\n", (*offset));
        return (EC_FALSE);
    }     
    
    zero_len = (*offset);/*save offset*/
    data_len = DMIN(CPGB_CACHE_MAX_BYTE_SIZE - zero_len, cbytes_len(cbytes));
    
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0056);
    if(EC_FALSE == crfsdn_write_b(CRFS_MD_DN(crfs_md), data_len, cbytes_buf(cbytes), &disk_no, &block_no, offset))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0057);
        
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn_miss: write %u bytes to dn failed\n", data_len);
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0058);

    insert_offset = crfsnp_bnode_insert(crfsnp, parent_pos, key_1st_hash, key_2nd_hash, klen, key, CRFSNP_ITEM_FILE_IS_REG);
    if(CRFSNPRB_ERR_POS == insert_offset)
    {        
        page_no = 0;
        
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn_miss: write %s to bnode in npp failed\n", (char *)key);
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
    
    return (EC_TRUE);
}

static EC_BOOL __crfs_write_b_seg_dn_hit(const UINT32 crfs_md_id, 
                                           CRFSNP *crfsnp, 
                                           const uint32_t node_pos, 
                                           UINT32 *offset, const CBYTES *cbytes,
                                           const uint32_t key_1st_hash, const uint32_t key_2nd_hash, 
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
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn_hit: buff len %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }   

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn_hit: offset %u overflow\n", (*offset));
        return (EC_FALSE);
    }     

    skip_len = (*offset);/*save offset*/
    data_len = DMIN(CPGB_CACHE_MAX_BYTE_SIZE - skip_len, cbytes_len(cbytes));
        
    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    ASSERT(NULL_PTR != crfsnp_item);
    ASSERT(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item));    

    crfsnp_fnode = CRFSNP_ITEM_FNODE(crfsnp_item);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);    

    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode);
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0059);            
    if(EC_FALSE == crfsdn_update_b(CRFS_MD_DN(crfs_md), data_len, cbytes_buf(cbytes), disk_no, block_no, offset))
    {  
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0060);  
        sys_log(LOGSTDOUT, "warn:__crfs_write_b_seg_dn_hit: write %u bytes of disk %u block %u offset %u failed\n", data_len, disk_no, block_no, skip_len);
        return (EC_FALSE);
    }
    else
    {
        sys_log(LOGSTDOUT, "[DEBUG]__crfs_write_b_seg_dn_hit: write %u bytes of disk %u block %u offset %u done\n", data_len, disk_no, block_no, skip_len);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0061);  

    if((*offset) > CRFSNP_FNODE_FILESZ(crfsnp_fnode))
    {
        /*update file size info*/
        CRFSNP_FNODE_FILESZ(crfsnp_fnode) = (uint32_t)(*offset);
    } 
    return (EC_TRUE);
}

static EC_BOOL __crfs_write_b_seg_dn(const UINT32 crfs_md_id, 
                                             CRFSNP *crfsnp, const uint32_t parent_pos, 
                                             const uint32_t seg_no, UINT32 *offset, 
                                             const UINT32 max_len, const CBYTES *cbytes)
{
    CRFS_MD *crfs_md;

    CRFSNP_ITEM  *crfsnp_item_parent;
    CRFSNP_BNODE *crfsnp_bnode;    
    
    UINT32   max_len_t;
    CBYTES   cbytes_t;

    uint8_t  key[32];
    uint32_t klen;    

    uint32_t key_1st_hash;
    uint32_t key_2nd_hash;

    uint32_t node_pos;

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
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn: no npp was open\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn: offset %u overflow\n", (*offset));
        return (EC_FALSE);
    }      

    /*okay, limit buff len <= 64MB*/
    if(CPGB_CACHE_MAX_BYTE_SIZE < CBYTES_LEN(cbytes))
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn: buff len %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }    

    max_len_t = DMIN(max_len, CBYTES_LEN(cbytes));
    if(0 == max_len_t)
    {
        return (EC_TRUE);
    }     

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn: fetch parent item failed where parent offset %u\n", parent_pos);
        return (EC_FALSE);
    }

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item_parent) 
    || CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item_parent))
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DFLG(crfsnp_item_parent),
                            CRFSNP_ITEM_STAT(crfsnp_item_parent));
        return (EC_FALSE);
    }     

    /*init cbytes_t*/
    cbytes_mount(&cbytes_t, max_len_t, CBYTES_BUF(cbytes));

    snprintf((char *)key, sizeof(key), "%d", seg_no);
    klen = strlen((char *)key);

    key_1st_hash = CRFSNP_1ST_CHASH_ALGO_COMPUTE(crfsnp, klen, key);
    key_2nd_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, klen, key);
    node_pos     = crfsnp_bnode_search(crfsnp, crfsnp_bnode, key_1st_hash, key_2nd_hash, klen, key);    

    if(CRFSNPRB_ERR_POS == node_pos)/*Oops! the segment missed*/
    {
        UINT32 offset_t1;
        UINT32 offset_t2;
        UINT32 burn_len;

        offset_t1 = (*offset);
        offset_t2 = offset_t1;
        
        if(EC_FALSE == __crfs_write_b_seg_dn_miss(crfs_md_id, crfsnp, 
                                              parent_pos, 
                                              &offset_t1, &cbytes_t,
                                              key_1st_hash, key_2nd_hash, 
                                              klen, key))
        {
            sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn: write data to missed segment failed\n");
            return (EC_FALSE);
        }

        burn_len = (offset_t1 - offset_t2);
        (*offset) += burn_len; /*update offset*/
    }
    else/*the segment hit*/
    {
        UINT32 offset_t1;
        UINT32 offset_t2;
        UINT32 burn_len;

        offset_t1 = (*offset);
        offset_t2 = offset_t1;
        
        if(EC_FALSE == __crfs_write_b_seg_dn_hit(crfs_md_id, crfsnp, 
                                              node_pos, 
                                              &offset_t1, &cbytes_t,
                                              key_1st_hash, key_2nd_hash, 
                                              klen, key))
        {
            sys_log(LOGSTDOUT, "error:__crfs_write_b_seg_dn: write data to hit segment failed\n");
            return (EC_FALSE);
        }

        burn_len = (offset_t1 - offset_t2);
        (*offset) += burn_len; /*update offset*/
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
                                      const UINT32 max_len, const CBYTES *cbytes)
{
    CRFS_MD  *crfs_md;
    CRFSNP   *crfsnp;

    CRFSNP_ITEM  *crfsnp_item_parent;
    CRFSNP_BNODE *crfsnp_bnode;

    UINT32   max_len_t;
    UINT8   *content_buf;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_write_b_dn: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(CRFS_BIGFILE_MAX_SIZE <= (*offset))
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_dn: offset %llu overflow\n", (*offset));
        return (EC_FALSE);
    }

    if(CPGB_CACHE_MAX_BYTE_SIZE < max_len)
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_dn: max_len %u overflow\n", max_len);
        return (EC_FALSE);
    }    

    crfsnp = crfsnp_mgr_open_np(CRFS_MD_NPP(crfs_md), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_dn: open crfsnp %u failed\n", crfsnp_id);
        return (EC_FALSE);
    }     

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_dn: fetch parent item failed where parent offset %u\n", parent_pos);
        return (EC_FALSE);
    }

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item_parent) 
    || CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item_parent))
    {
        sys_log(LOGSTDOUT, "error:__crfs_write_b_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DFLG(crfsnp_item_parent),
                            CRFSNP_ITEM_STAT(crfsnp_item_parent));
        return (EC_FALSE);
    }     

    max_len_t   = DMIN(max_len, CBYTES_LEN(cbytes));
    content_buf = CBYTES_BUF(cbytes);

    for(; 0 < max_len_t;)
    {
        CBYTES   cbytes_t;
        UINT32   offset_t1;
        UINT32   offset_t2;
        UINT32   burn_len;
        uint32_t seg_no;

        offset_t1 = (UINT32)((*offset) % CPGB_CACHE_MAX_BYTE_SIZE);
        offset_t2 = offset_t1;
        seg_no    = (uint32_t)((*offset) >> CPGB_CACHE_BIT_SIZE);
        
        cbytes_mount(&cbytes_t, max_len_t, content_buf);
        
        if(EC_FALSE == __crfs_write_b_seg_dn(crfs_md_id, crfsnp, parent_pos, seg_no, &offset_t1, max_len_t, &cbytes_t))
        {
            sys_log(LOGSTDOUT, "error:__crfs_write_b_dn: write to seg_no %u at offset %u failed\n", seg_no, offset_t2);
            return (EC_FALSE);
        }

        burn_len     = (offset_t1 - offset_t2);
        max_len_t   -= burn_len;
        content_buf += burn_len;
        (*offset)   += burn_len;

        CRFSNP_BNODE_FILESZ_UPDATE_IF_NEED(crfsnp_bnode, (*offset));
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
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn_miss: max_len %u overflow\n", max_len);
        return (EC_FALSE);
    }

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn_miss: offset %u overflow\n", (*offset));
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
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn_hit: buff len %u overflow\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }   

    if(CPGB_CACHE_MAX_BYTE_SIZE < (*offset))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn_hit: offset %u overflow\n", (*offset));
        return (EC_FALSE);
    }     
       
    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    ASSERT(NULL_PTR != crfsnp_item);
    ASSERT(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item));    

    crfsnp_fnode = CRFSNP_ITEM_FNODE(crfsnp_item);

    file_size = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    if(file_size < (*offset))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn_hit: offset %u > file_size %u\n", (*offset), file_size);
        return (EC_FALSE);
    }     

    read_len = DMIN(file_size - (*offset), cbytes_len(cbytes));/*the buffer to accept the read data was ready in cbytes*/

    /*note: CRFS_MD_DN(crfs_md) will be locked in crfs_read_e_dn*/
    if(0 < read_len && EC_FALSE == crfs_read_e_dn(crfs_md_id, crfsnp_fnode, offset, read_len, cbytes))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn_hit: read %u bytes from dn failed\n", read_len);
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

    uint32_t key_1st_hash;
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
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn: max_len %u overflow\n", max_len);
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn: no npp was open\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn: fetch parent item failed where parent offset %u\n", parent_pos);
        return (EC_FALSE);
    }

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item_parent) 
    || CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item_parent))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DFLG(crfsnp_item_parent),
                            CRFSNP_ITEM_STAT(crfsnp_item_parent));
        return (EC_FALSE);
    }    

    max_len_t = DMIN(max_len, CBYTES_LEN(cbytes));

    /*init cbytes_t*/
    cbytes_mount(&cbytes_t, max_len_t, CBYTES_BUF(cbytes));    

    snprintf((char *)key, sizeof(key), "%d", seg_no);
    klen = strlen((char *)key);

    key_1st_hash = CRFSNP_1ST_CHASH_ALGO_COMPUTE(crfsnp, klen, key);
    key_2nd_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, klen, key);
    node_pos     = crfsnp_bnode_search(crfsnp, crfsnp_bnode, key_1st_hash, key_2nd_hash, klen, key);

    if(CRFSNPRB_ERR_POS == node_pos)/*Oops! the segment missed*/
    {
        UINT32 offset_t1;
        UINT32 offset_t2;
        UINT32 read_len;

        offset_t1 = (*offset);
        offset_t2 = offset_t1;

        if(EC_FALSE == __crfs_read_b_seg_dn_miss(crfs_md_id, &offset_t1, max_len_t, &cbytes_t))
        {
            sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn: read data from missed segment failed\n");
            return (EC_FALSE);
        }        

        read_len  = (offset_t1 - offset_t2);
        (*offset) += read_len; 
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
            sys_log(LOGSTDOUT, "error:__crfs_read_b_seg_dn: write data to hit segment failed\n");
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
static EC_BOOL __crfs_read_b_dn(const UINT32 crfs_md_id, const uint32_t crfsnp_id, const uint32_t parent_pos, uint64_t *offset, const UINT32 max_len, CBYTES *cbytes)
{
    CRFS_MD      *crfs_md;

    CRFSNP *crfsnp;

    CRFSNP_ITEM  *crfsnp_item_parent;
    CRFSNP_BNODE *crfsnp_bnode;

    uint64_t file_size;

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
        sys_log(LOGSTDOUT, "error:__crfs_read_b_dn: max_len %u overflow\n", max_len);
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_dn: no npp was open\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == CRFS_MD_DN(crfs_md))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_dn: no dn was open\n");
        return (EC_FALSE);
    }    

    crfsnp = crfsnp_mgr_open_np(CRFS_MD_NPP(crfs_md), crfsnp_id);
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_dn: open crfsnp %u failed\n", crfsnp_id);
        return (EC_FALSE);
    }    

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);
    if(NULL_PTR == crfsnp_item_parent)
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_dn: fetch parent item failed where parent offset %u\n", parent_pos);
        return (EC_FALSE);
    }

    crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item_parent) 
    || CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item_parent))
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_dn: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DFLG(crfsnp_item_parent),
                            CRFSNP_ITEM_STAT(crfsnp_item_parent));
        return (EC_FALSE);
    }    

    file_size = CRFSNP_BNODE_FILESZ(crfsnp_bnode);
    if((*offset) >= file_size)
    {
        sys_log(LOGSTDOUT, "error:__crfs_read_b_dn: offset %llu overflow due to file_size = %llu\n", (*offset), file_size);
        return (EC_FALSE);
    }       

    if((*offset) + max_len >= file_size)
    {
        max_len_t = (UINT32)(file_size - (*offset));
    }
    else
    {
        max_len_t = max_len;
    } 

    sys_log(LOGSTDOUT, "[DEBUG] __crfs_read_b_dn: file_size %llu, max_len %u, max_len_t %u, offset %llu\n", 
                        file_size, max_len, max_len_t, (*offset));

    if(CBYTES_LEN(cbytes) < max_len_t)
    {
        if(NULL_PTR != CBYTES_BUF(cbytes))
        {
            SAFE_FREE(CBYTES_BUF(cbytes), LOC_CRFS_0062);
        }
        CBYTES_BUF(cbytes) = (UINT8 *)SAFE_MALLOC(max_len_t, LOC_CRFS_0063);
        CBYTES_LEN(cbytes) = max_len_t;/*initialize to the final size ...*/

        if(NULL_PTR == CBYTES_BUF(cbytes))
        {
            sys_log(LOGSTDOUT, "error:__crfs_read_b_dn: malloc %u bytes for cbytes failed\n", max_len_t);
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
            sys_log(LOGSTDOUT, "error:__crfs_read_b_dn: read from seg_no %u at offset %u failed\n", seg_no, offset_t2);
            return (EC_FALSE);
        }

        read_len     = (offset_t1 - offset_t2);
        max_len_t   -= read_len;
        content_buf += read_len;
        (*offset)   += read_len;
    }

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
        sys_log(LOGSTDOUT, "error:crfs_write_npp: npp was not open\n");
        return (EC_FALSE);
    }

    if(0 == CRFSNP_FNODE_REPNUM(crfsnp_fnode))
    {
        sys_log(LOGSTDOUT, "error:crfs_write_npp: no valid replica in fnode\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0064);
    if(EC_FALSE == crfsnp_mgr_write(CRFS_MD_NPP(crfs_md), file_path, crfsnp_fnode))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0065);
        
        sys_log(LOGSTDOUT, "error:crfs_write_npp: no name node accept file %s with %u replicas writting\n",
                            (char *)cstring_get_str(file_path), CRFSNP_FNODE_REPNUM(crfsnp_fnode));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0066);
    return (EC_TRUE);
}

/**
*
*  read a fnode from name node
*
**/
EC_BOOL crfs_read_npp(const UINT32 crfs_md_id, const CSTRING *file_path, CRFSNP_FNODE *crfsnp_fnode)
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
        sys_log(LOGSTDOUT, "warn:crfs_read_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0067);
    if(EC_FALSE == crfsnp_mgr_read(CRFS_MD_NPP(crfs_md), file_path, crfsnp_fnode))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0068);
        
        sys_log(LOGSTDOUT, "error:crfs_read_npp: crfsnp mgr read %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0069);

    return (EC_TRUE);
}

/**
*
*  write a bnode to name node
*
**/
static EC_BOOL __crfs_write_b_npp(const UINT32 crfs_md_id, const CSTRING *file_path)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__crfs_write_b_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(LOGSTDOUT, "warn:__crfs_write_b_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0070);
    if(EC_FALSE == crfsnp_mgr_write_b(CRFS_MD_NPP(crfs_md), file_path))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0071);
        
        sys_log(LOGSTDOUT, "error:__crfs_write_b_npp: crfsnp mgr read %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0072);

    return (EC_TRUE);
}

/**
*
*  read a bnode from name node
*
**/
static EC_BOOL __crfs_read_b_npp(const UINT32 crfs_md_id, const CSTRING *file_path, uint32_t *crfsnp_id, uint32_t *parent_pos)
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
        sys_log(LOGSTDOUT, "warn:__crfs_read_b_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0073);
    if(EC_FALSE == crfsnp_mgr_read_b(CRFS_MD_NPP(crfs_md), file_path, crfsnp_id, parent_pos))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0074);
        
        sys_log(LOGSTDOUT, "error:__crfs_read_b_npp: crfsnp mgr read %s failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0075);

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
        sys_log(LOGSTDOUT, "error:crfs_update_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0076);
    if(EC_FALSE == crfsnp_mgr_update(CRFS_MD_NPP(crfs_md), file_path, crfsnp_fnode))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0077);
        
        sys_log(LOGSTDOUT, "error:crfs_update_npp: no name node accept file %s with %u replicas updating\n",
                            (char *)cstring_get_str(file_path), CRFSNP_FNODE_REPNUM(crfsnp_fnode));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0078);
    return (EC_TRUE);
}


/**
*
*  delete a file or dir from current npp
*
**/
EC_BOOL crfs_delete_npp(const UINT32 crfs_md_id, const CSTRING *path, const UINT32 dflag, uint32_t *crfsnp_id, CVECTOR *crfsnp_item_vec)
{
    CRFS_MD      *crfs_md;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete_npp: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(LOGSTDOUT, "warn:crfs_delete_npp: npp was not open\n");
        return (EC_FALSE);
    }

    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0079);
    if(EC_FALSE == crfsnp_mgr_delete(CRFS_MD_NPP(crfs_md), path, dflag, crfsnp_id, crfsnp_item_vec))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0080);
        
        sys_log(LOGSTDOUT, "error:crfs_delete_npp: delete %s, dflag %lx failed\n", (char *)cstring_get_str(path), dflag);
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0081);

    return (EC_TRUE);
}

/**
*
*  delete file data from current dn
*
**/
static EC_BOOL __crfs_delete_dn(const UINT32 crfs_md_id, CRFSNP_FNODE *crfsnp_fnode)
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
        sys_log(LOGSTDOUT, "error:__crfs_delete_dn: no dn was open\n");
        return (EC_FALSE);
    }

    if(0 == CRFSNP_FNODE_REPNUM(crfsnp_fnode))
    {
        sys_log(LOGSTDOUT, "error:__crfs_delete_dn: no replica\n");
        return (EC_FALSE);
    }    

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    disk_no  = CRFSNP_INODE_DISK_NO(crfsnp_inode) ;
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;
    
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0082);
    if(EC_FALSE == crfsdn_remove(CRFS_MD_DN(crfs_md), disk_no, block_no, page_no, file_size))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0083);
        sys_log(LOGSTDOUT, "error:__crfs_delete_dn: remove file fsize %u, disk %u, block %u, page %u failed\n", file_size, disk_no, block_no, page_no);
        return (EC_FALSE);
    }    
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0084);

    sys_log(LOGSTDOUT, "[DEBUG] __crfs_delete_dn: remove file fsize %u, disk %u, block %u, page %u done\n", file_size, disk_no, block_no, page_no);
    
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
        sys_log(LOGSTDOUT, "error:__crfs_delete_b_dn: no dn was open\n");
        return (EC_FALSE);
    }

    if(0 == CRFSNP_BNODE_SEG_NUM(crfsnp_bnode))
    {
        sys_log(LOGSTDOUT, "[DEBUG] __crfs_delete_b_dn: no segment to delete\n");
        return (EC_TRUE);
    } 

    if(EC_FALSE == crfsnp_bnode_delete_dir_son(crfsnp, crfsnp_bnode, NULL_PTR))
    {
        sys_log(LOGSTDOUT, "error:__crfs_delete_b_dn:delete bnode sons failed\n");
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
        if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {        
            if(EC_FALSE == __crfs_delete_dn(crfs_md_id, CRFSNP_ITEM_FNODE(crfsnp_item)))
            {
                sys_log(LOGSTDOUT, "error:crfs_delete: delete regular file from dn failed\n");
                return (EC_FALSE);
            }
            return (EC_TRUE);
        }
        
        if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            CRFS_MD *crfs_md;
            CRFSNP  *crfsnp;
            uint32_t crfsnp_id_t;

            crfs_md = CRFS_MD_GET(crfs_md_id);
            if(NULL_PTR == CRFS_MD_NPP(crfs_md))
            {
                sys_log(LOGSTDOUT, "warn:crfs_delete: npp was not open\n");
                return (EC_FALSE);
            }            

            crfsnp_id_t = (uint32_t)crfsnp_id;
            
            CRFSNP_MGR_CMUTEX_LOCK(CRFS_MD_NPP(crfs_md), LOC_CRFS_0085);
            crfsnp = crfsnp_mgr_open_np(CRFS_MD_NPP(crfs_md), crfsnp_id_t);
            if(NULL_PTR == crfsnp)
            {
                CRFSNP_MGR_CMUTEX_UNLOCK(CRFS_MD_NPP(crfs_md), LOC_CRFS_0086);
                sys_log(LOGSTDOUT, "error:crfs_delete: open np %u failed\n", crfsnp_id_t);
                return (EC_FALSE);
            }
            CRFSNP_MGR_CMUTEX_UNLOCK(CRFS_MD_NPP(crfs_md), LOC_CRFS_0087);
    
            if(EC_FALSE == __crfs_delete_b_dn(crfs_md_id, crfsnp, (CRFSNP_BNODE *)CRFSNP_ITEM_BNODE(crfsnp_item)))
            {
                sys_log(LOGSTDOUT, "error:crfs_delete: delete big file from dn failed\n");
                return (EC_FALSE);
            }
            return (EC_TRUE);
        }
    }
    return (EC_TRUE);
}

/**
*
*  delete a file or dir from all npp and all dn
*
**/
EC_BOOL crfs_delete(const UINT32 crfs_md_id, const CSTRING *path, const UINT32 dflag)
{
    CRFS_MD      *crfs_md;
    
    CVECTOR *crfsnp_item_vec;
    UINT32   crfsnp_item_num;
    UINT32   crfsnp_item_pos;
    EC_BOOL  ret;

    uint32_t crfsnp_id;

#if ( SWITCH_ON == CRFS_DEBUG_SWITCH )
    if ( CRFS_MD_ID_CHECK_INVALID(crfs_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:crfs_delete: crfs module #0x%lx not started.\n",
                crfs_md_id);
        dbg_exit(MD_CRFS, crfs_md_id);
    }
#endif/*CRFS_DEBUG_SWITCH*/

    sys_log(LOGSTDOUT, "[DEBUG] crfs_delete: crfs_md_id %u, path %s, dflag %x\n", crfs_md_id, (char *)cstring_get_str(path), dflag);

    crfsnp_item_vec = cvector_new(0, MM_CRFSNP_ITEM, LOC_CRFS_0088);
    if(NULL_PTR == crfsnp_item_vec)
    {
        sys_log(LOGSTDOUT, "error:crfs_delete: new vector failed\n");
        return (EC_FALSE);
    }

    crfs_md = CRFS_MD_GET(crfs_md_id);
    
    CRFS_WRLOCK(crfs_md, LOC_CRFS_0089);    

    /*delete inodes*/
    if(EC_FALSE == crfs_delete_npp(crfs_md_id, path, dflag, &crfsnp_id, crfsnp_item_vec))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0090);    
        
        sys_log(LOGSTDOUT, "error:crfs_delete: delete %s from npp failed\n", (char *)cstring_get_str(path));
        cvector_free(crfsnp_item_vec, LOC_CRFS_0091);
        return (EC_FALSE);
    }

    /*delete data*/
    ret = EC_TRUE;
    
    crfsnp_item_num = cvector_size(crfsnp_item_vec);
    for(crfsnp_item_pos = 0; crfsnp_item_pos < crfsnp_item_num; crfsnp_item_pos ++)
    {
        CRFSNP_ITEM *crfsnp_item;
        
        crfsnp_item = (CRFSNP_ITEM *)cvector_get_no_lock(crfsnp_item_vec, crfsnp_item_pos);
        if(EC_FALSE == crfs_delete_dn(crfs_md_id, (UINT32)crfsnp_id, crfsnp_item))
        {
            sys_log(LOGSTDOUT, "error:crfs_delete: delete %s from dn failed\n", (char *)cstring_get_str(path));
            ret = EC_FALSE;
        }   
    }

    CRFS_UNLOCK(crfs_md, LOC_CRFS_0092);    

    cvector_clean(crfsnp_item_vec, NULL_PTR, LOC_CRFS_0093);
    cvector_free(crfsnp_item_vec, LOC_CRFS_0094);    
    
    return (ret);
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
        sys_log(LOGSTDOUT, "warn:crfs_qfile: npp was not open\n");
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0095);    

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0096);
    crfsnp_item_src = crfsnp_mgr_search_item(CRFS_MD_NPP(crfs_md), 
                                             (uint32_t)cstring_get_len(file_path), 
                                             cstring_get_str(file_path), 
                                             CRFSNP_ITEM_FILE_IS_REG);
    if(NULL_PTR == crfsnp_item_src)
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0097);

        CRFS_UNLOCK(crfs_md, LOC_CRFS_0098);
        sys_log(LOGSTDOUT, "error:crfs_qfile: query file %s from npp failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0099);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0100);

    crfsnp_item_clone(crfsnp_item_src, crfsnp_item);

    return (EC_TRUE);
}

/**
*
*  query a dir
*
**/
EC_BOOL crfs_qdir(const UINT32 crfs_md_id, const CSTRING *dir_path, CVECTOR  *crfsnp_item_vec)
{
    CRFS_MD      *crfs_md;
    uint32_t      crfsnp_id;

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
        sys_log(LOGSTDOUT, "warn:crfs_qdir: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0101);    

    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0102);
    if(EC_FALSE == crfsnp_mgr_collect_items(CRFS_MD_NPP(crfs_md), dir_path, CRFSNP_ITEM_FILE_IS_DIR, &crfsnp_id, crfsnp_item_vec))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0103);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0104);
        
        sys_log(LOGSTDOUT, "error:crfs_qfile: query dir %s from npp failed\n", (char *)cstring_get_str(dir_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0105);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0106);

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
        sys_log(LOGSTDOUT, "warn:crfs_qlist_path: npp was not open\n");
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0107);    
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0108);
    if(EC_FALSE == crfsnp_mgr_list_path(CRFS_MD_NPP(crfs_md), file_path, path_cstr_vec))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0109);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0110);
        
        sys_log(LOGSTDOUT, "error:crfs_qlist_path: list path '%s' failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0111);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0112);

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
        sys_log(LOGSTDOUT, "warn:crfs_qlist_seg: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0113);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0114);
    if(EC_FALSE == crfsnp_mgr_list_seg(CRFS_MD_NPP(crfs_md), file_path, seg_cstr_vec))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0115);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0116);
        
        sys_log(LOGSTDOUT, "error:crfs_qlist_seg: list seg of path '%s' failed\n", (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0117);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0118);
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
        sys_log(LOGSTDOUT, "warn:crfs_flush_npp: npp was not open\n");
        return (EC_TRUE);
    }
    
    CRFS_WRLOCK(crfs_md, LOC_CRFS_0119);
    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0120);
    if(EC_FALSE == crfsnp_mgr_flush(CRFS_MD_NPP(crfs_md)))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0121);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0122);
        
        sys_log(LOGSTDOUT, "error:crfs_flush_npp: flush failed\n");
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0123);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0124);
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
        sys_log(LOGSTDOUT, "error:crfs_flush_dn: dn is null\n");
        return (EC_FALSE);
    }

    CRFS_WRLOCK(crfs_md, LOC_CRFS_0125);
    crfsdn_wrlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0126);
    if(EC_FALSE == crfsdn_flush(CRFS_MD_DN(crfs_md)))
    {
        crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0127);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0128);
        
        sys_log(LOGSTDOUT, "error:crfs_flush_dn: flush dn failed\n");
        return (EC_FALSE);
    }
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0129);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0130);

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
        sys_log(LOGSTDOUT, "warn:crfs_file_num: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0131);
    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0132);
    if(EC_FALSE == crfsnp_mgr_file_num(CRFS_MD_NPP(crfs_md), path_cstr, file_num))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0133);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0134);
        
        sys_log(LOGSTDOUT, "error:crfs_file_num: get file num of path '%s' failed\n", (char *)cstring_get_str(path_cstr));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0135);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0136);

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
        sys_log(LOGSTDOUT, "warn:crfs_file_size: npp was not open\n");
        return (EC_FALSE);
    }
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0137);
    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0138);
    if(EC_FALSE == crfsnp_mgr_file_size(CRFS_MD_NPP(crfs_md), path_cstr, file_size))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0139);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0140);
        
        sys_log(LOGSTDOUT, "error:crfs_file_size: get file size of path '%s' failed\n", (char *)cstring_get_str(path_cstr));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0141);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0142);
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
        sys_log(LOGSTDOUT, "warn:crfs_mkdir: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_WRLOCK(crfs_md, LOC_CRFS_0143);
    crfsnp_mgr_wrlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0144);
    if(EC_FALSE == crfsnp_mgr_mkdir(CRFS_MD_NPP(crfs_md), path_cstr))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0145);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0146);
        sys_log(LOGSTDOUT, "error:crfs_mkdir: mkdir '%s' failed\n", (char *)cstring_get_str(path_cstr));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0147);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0148);

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

    sys_log(LOGSTDOUT, "[DEBUG] crfs_search: crfs_md_id %u, path %s, dflag %x\n", crfs_md_id, (char *)cstring_get_str(path_cstr), dflag);

    crfs_md = CRFS_MD_GET(crfs_md_id);

    if(NULL_PTR == CRFS_MD_NPP(crfs_md))
    {
        sys_log(LOGSTDOUT, "warn:crfs_search: npp was not open\n");
        return (EC_FALSE);
    }
    
    CRFS_RDLOCK(crfs_md, LOC_CRFS_0149);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0150);
    if(EC_FALSE == crfsnp_mgr_search(CRFS_MD_NPP(crfs_md), (uint32_t)cstring_get_len(path_cstr), cstring_get_str(path_cstr), dflag, &crfsnp_id))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0151);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0152);
        
        sys_log(LOGSTDOUT, "error:crfs_search: search '%s' with dflag %x failed\n", (char *)cstring_get_str(path_cstr), dflag);
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0153);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0154);

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
        sys_log(LOGSTDOUT, "error:crfs_check_file_content: dn is null\n");
        return (EC_FALSE);
    }

    ASSERT(EC_TRUE == __crfs_check_is_uint16_t(disk_no));
    ASSERT(EC_TRUE == __crfs_check_is_uint16_t(block_no));
    ASSERT(EC_TRUE == __crfs_check_is_uint16_t(page_no));

    cbytes = cbytes_new(file_size);
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:crfs_check_file_content: new crfs buff with len %u failed\n", file_size);
        return (EC_FALSE);
    }

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0155);
    if(EC_FALSE == crfsdn_read_p(CRFS_MD_DN(crfs_md), (uint16_t)disk_no, (uint16_t)block_no, (uint16_t)page_no, file_size, 
                                  CBYTES_BUF(cbytes), &(CBYTES_LEN(cbytes))))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0156);
        sys_log(LOGSTDOUT, "error:crfs_check_file_content: read %u bytes from disk %u, block %u, page %u failed\n", 
                            file_size, disk_no, block_no, page_no);
        cbytes_free(cbytes, LOC_CRFS_0157);
        return (EC_FALSE);
    }

    if(CBYTES_LEN(cbytes) < cstring_get_len(file_content_cstr))
    {
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0158);
        sys_log(LOGSTDOUT, "error:crfs_check_file_content: read %u bytes from disk %u, block %u, page %u to buff len %u less than cstring len %u to compare\n",
                            file_size, disk_no, block_no, page_no,
                            CBYTES_LEN(cbytes), cstring_get_len(file_content_cstr));
        cbytes_free(cbytes, LOC_CRFS_0159);
        return (EC_FALSE);
    }

    len = cstring_get_len(file_content_cstr);

    buff = CBYTES_BUF(cbytes);
    str  = cstring_get_str(file_content_cstr);

    for(pos = 0; pos < len; pos ++)
    {
        if(buff[ pos ] != str[ pos ])
        {
            CRFS_UNLOCK(crfs_md, LOC_CRFS_0160);
            sys_log(LOGSTDOUT, "error:crfs_check_file_content: char at pos %u not matched\n", pos);
            sys_print(LOGSTDOUT, "read buff: %.*s\n", len, buff);
            sys_print(LOGSTDOUT, "expected : %.*s\n", len, str);

            cbytes_free(cbytes, LOC_CRFS_0161);
            return (EC_FALSE);
        }
    }
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0162);

    cbytes_free(cbytes, LOC_CRFS_0163);
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
        sys_log(LOGSTDOUT, "error:crfs_check_file_is: dn is null\n");
        return (EC_FALSE);
    }
    
    cbytes = cbytes_new(0);
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:crfs_check_file_is: new cbytes failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == crfs_read(crfs_md_id, file_path, cbytes))
    {
        sys_log(LOGSTDOUT, "error:crfs_check_file_is: read file %s failed\n", (char *)cstring_get_str(file_path));
        cbytes_free(cbytes, LOC_CRFS_0164);
        return (EC_FALSE);
    }

    if(CBYTES_LEN(cbytes) != CBYTES_LEN(file_content))
    {
        sys_log(LOGSTDOUT, "error:crfs_check_file_is: mismatched len: file %s read len %u which should be %u\n",
                            (char *)cstring_get_str(file_path),                            
                            CBYTES_LEN(cbytes), CBYTES_LEN(file_content));
        cbytes_free(cbytes, LOC_CRFS_0165);
        return (EC_FALSE);
    }

    len  = CBYTES_LEN(file_content);

    buff = CBYTES_BUF(cbytes);
    str  = CBYTES_BUF(file_content);

    for(pos = 0; pos < len; pos ++)
    {
        if(buff[ pos ] != str[ pos ])
        {
            sys_log(LOGSTDOUT, "error:crfs_check_file_is: char at pos %u not matched\n", pos);
            sys_print(LOGSTDOUT, "read buff: %.*s\n", len, buff);
            sys_print(LOGSTDOUT, "expected : %.*s\n", len, str);

            cbytes_free(cbytes, LOC_CRFS_0166);
            return (EC_FALSE);
        }
    }

    cbytes_free(cbytes, LOC_CRFS_0167);
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

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0168);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0169);
    
    crfsnp_mgr_print(log, CRFS_MD_NPP(crfs_md));
    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0170);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0171);
    
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

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0172);
    crfsdn_rdlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0173);
    crfsdn_print(log, CRFS_MD_DN(crfs_md));
    crfsdn_unlock(CRFS_MD_DN(crfs_md), LOC_CRFS_0174);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0175);

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

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0176);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0177);    
    if(EC_FALSE == crfsnp_mgr_show_cached_np(log, CRFS_MD_NPP(crfs_md)))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0178);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0179);
        
        sys_log(LOGSTDOUT, "error:crfs_show_cached_np: show cached np but failed\n");
        return (EC_FALSE);
    }    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0180);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0181);

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

    if(EC_FALSE == __crfs_check_is_uint32_t(crfsnp_id))
    {
        sys_log(LOGSTDOUT, "error:crfs_show_specific_np: crfsnp_id %u is invalid\n", crfsnp_id);
        return (EC_FALSE);
    }    

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0182);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0183);    
    if(EC_FALSE == crfsnp_mgr_show_np(log, CRFS_MD_NPP(crfs_md), (uint32_t)crfsnp_id))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0184);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0185);
        
        sys_log(LOGSTDOUT, "error:crfs_show_cached_np: show np %u but failed\n", crfsnp_id);
        return (EC_FALSE);
    }    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0186);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0187);

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

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0188);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0189);    
    if(EC_FALSE == crfsnp_mgr_show_path_depth(log, CRFS_MD_NPP(crfs_md), path))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0190);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0191);
        
        sys_log(log, "error:crfs_show_path_depth: show path %s in depth failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0192);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0193);

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

    CRFS_RDLOCK(crfs_md, LOC_CRFS_0194);
    crfsnp_mgr_rdlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0195);    
    if(EC_FALSE == crfsnp_mgr_show_path(log, CRFS_MD_NPP(crfs_md), path))
    {
        crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0196);
        CRFS_UNLOCK(crfs_md, LOC_CRFS_0197);
        sys_log(log, "error:crfs_show_path: show path %s failed\n", (char *)cstring_get_str(path));
        return (EC_FALSE);
    }    
    crfsnp_mgr_unlock(CRFS_MD_NPP(crfs_md), LOC_CRFS_0198);
    CRFS_UNLOCK(crfs_md, LOC_CRFS_0199);
    
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

#ifdef __cplusplus
}
#endif/*__cplusplus*/
