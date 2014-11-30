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

#include <pcre.h>

#include "type.h"
#include "mm.h"
#include "log.h"

#include "cbc.h"

#include "cmisc.h"
#include "clist.h"
#include "cmutex.h"
#include "cbytes.h"
#include "cstring.h"

#include "mod.inc"
#include "cmpic.inc"
#include "task.h"
#include "cbtimer.h"
#include "chashalgo.h"

#include "cbgt.h"
#include "cscore.h"

#include "findex.inc"

#define CSCORE_MD_CAPACITY()                  (cbc_md_capacity(MD_CSCORE))

#define CSCORE_MD_GET(cscore_md_id)         ((CSCORE_MD *)cbc_md_get(MD_CSCORE, (cscore_md_id)))

#define CSCORE_MD_ID_CHECK_INVALID(cscore_md_id)  \
    ((CMPI_ANY_MODI != (cscore_md_id)) && ((NULL_PTR == CSCORE_MD_GET(cscore_md_id)) || (0 == (CSCORE_MD_GET(cscore_md_id)->usedcounter))))

static UINT32 * __cscore_new_uint32(const UINT32 cscore_md_id);

static EC_BOOL  __cscore_free_uint32(const UINT32 cscore_md_id, UINT32 *data);

static CBYTES *__cscore_new_row_cbytes_by_csword(const UINT32 cscore_md_id, const CSWORD *csword);

static CBYTES *__cscore_new_ts_cbytes_by_ctimet(const UINT32 cscore_md_id, const CTIMET *ctimet);

static CSTRING *__cscore_new_row_cstring_by_csword(const UINT32 cscore_md_id, const CSWORD *csword);

static CSTRING *__cscore_new_ts_cstring_by_ctimet(const UINT32 cscore_md_id, const CTIMET *ctimet);

static EC_BOOL __cscore_csword_import_doc_id_plain(const UINT32 cscore_md_id, const CSWORD *csword, const CSDOC *csdoc,
                                                                const CBYTES *row_bytes,
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *cached_bytes_vec);

static EC_BOOL __cscore_csword_import_doc_type_plain(const UINT32 cscore_md_id, const CSWORD *csword, const CSDOC *csdoc,
                                                                const CBYTES *row_bytes,
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *cached_bytes_vec);

static EC_BOOL __cscore_csword_import_doc_code_plain(const UINT32 cscore_md_id, const CSWORD *csword, const CSDOC *csdoc,
                                                                const CBYTES *row_bytes,
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *cached_bytes_vec);

static EC_BOOL __cscore_csword_import_plain(const UINT32 cscore_md_id, const CSWORD *csword, const CSDOC *csdoc,
                                                        CTIMET *ctimet,
                                                        TASK_MGR *task_mgr,
                                                        CVECTOR *ret_vec, CVECTOR *cached_bytes_vec);

static EC_BOOL __cscore_csword_list_import_plain(const UINT32 cscore_md_id, const CLIST *csword_list, const CSDOC *csdoc,
                                                            TASK_MGR *task_mgr,
                                                            CVECTOR *ret_vec, CVECTOR *cached_bytes_vec);


static EC_BOOL __cscore_csdoc_export_doc_id_plain(const UINT32 cscore_md_id, const CBYTES *kv_bytes, 
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *doc_id_cbytes_vec,
                                                                CVECTOR *cached_cbytes_vec);

static EC_BOOL __cscore_csdoc_export_doc_type_plain(const UINT32 cscore_md_id, const CBYTES *kv_bytes,
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *doc_type_cbytes_vec,
                                                                CVECTOR *cached_cbytes_vec);

static EC_BOOL __cscore_csdoc_export_doc_code_plain(const UINT32 cscore_md_id, const CBYTES *kv_bytes,
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *doc_code_cbytes_vec,
                                                                CVECTOR *cached_cbytes_vec);

static EC_BOOL __cscore_csdoc_export_doc_plain(const UINT32 cscore_md_id, const CBYTES *kv_bytes,
                                                            TASK_MGR *task_mgr,
                                                            CVECTOR *ret_vec, 
                                                            CVECTOR *doc_id_cbytes_vec,
                                                            CVECTOR *doc_type_cbytes_vec,
                                                            CVECTOR *doc_code_cbytes_vec,
                                                            CVECTOR *cached_cbytes_vec
                                                            );
                                                            
static EC_BOOL __cscore_csdoc_list_export_plain(const UINT32 cscore_md_id, const CVECTOR *kv_vec, CSWORD_DOCS *csword_docs);

static EC_BOOL __cscore_row_no_pool_init(CSCORE_ROW_NO_POOL *cscore_row_no_pool, const UINT32 row_max_num);

static EC_BOOL __cscore_csdoc_merge(const UINT32 cscore_md_id, const CSDOC *csdoc, CLIST *csdoc_list);

static EC_BOOL __cscore_csword_docs_merge(const UINT32 cscore_md_id, const CSWORD_DOCS *csword_docs, CLIST *csdoc_list);

static EC_BOOL __cscore_csword_docs_list_make_one(const UINT32 cscore_md_id, CLIST *csword_docs_list, const CSWORD *csword);

static EC_BOOL __cscore_csdoc_merge_one(const CSDOC *csdoc, CSDOC *cur_csdoc);

static const UINT8 *g_cscore_bgt_root = (const UINT8 *)"/home/ezhocha/hsbgt";

/**
*   for test only
*
*   to query the status of CSCORE Module
*
**/
void cscore_print_module_status(const UINT32 cscore_md_id, LOG *log)
{
    CSCORE_MD *cscore_md;
    UINT32 this_cscore_md_id;

    for( this_cscore_md_id = 0; this_cscore_md_id < CSCORE_MD_CAPACITY(); this_cscore_md_id ++ )
    {
        cscore_md = CSCORE_MD_GET(this_cscore_md_id);

        if ( NULL_PTR != cscore_md && 0 < cscore_md->usedcounter )
        {
            sys_log(log,"CSCORE Module # %ld : %ld refered, cbgt %ld\n",
                    this_cscore_md_id,
                    cscore_md->usedcounter,
                    CSCORE_MD_CBGT_MD_ID(cscore_md));
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed CSCORE module
*
*
**/
UINT32 cscore_free_module_static_mem(const UINT32 cscore_md_id)
{
    CSCORE_MD  *cscore_md;

#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_free_module_static_mem: cscore module #0x%lx not started.\n",
                cscore_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    free_module_static_mem(MD_CSCORE, cscore_md_id);

    return 0;
}

/**
*
* start CSCORE module
*
**/
UINT32 cscore_start()
{
    CSCORE_MD *cscore_md;
    UINT32 cscore_md_id;
    UINT32 cbgt_md_id;

    CSTRING  *cbgt_db_root_dir;

    TASK_BRD *task_brd;

    cscore_md_id = cbc_md_new(MD_CSCORE, sizeof(CSCORE_MD));
    if(ERR_MODULE_ID == cscore_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one CSCORE module */
    cscore_md = (CSCORE_MD *)cbc_md_get(MD_CSCORE, cscore_md_id);
    cscore_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();

    task_brd = task_brd_default_get();

    cbgt_db_root_dir = cstring_new(g_cscore_bgt_root, LOC_CSCORE_0001);
    if(NULL_PTR == cbgt_db_root_dir)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_start: new cstring for bgt root %s failed\n", (char *)g_cscore_bgt_root);
        cbc_md_free(MD_CSCORE, cscore_md_id);
        return (ERR_MODULE_ID);
    }

    cbgt_md_id = cbgt_start(CBGT_TYPE_USER_CLIENT, CBGT_ERR_TABLE_ID, NULL_PTR, NULL_PTR, cbgt_db_root_dir, CBGT_O_UNDEF);
    if(ERR_MODULE_ID == cbgt_md_id)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_start: start cbgt user client failed\n");
        cstring_free(cbgt_db_root_dir);
        cbc_md_free(MD_CSCORE, cscore_md_id);
        return (ERR_MODULE_ID);
    }
    cstring_free(cbgt_db_root_dir);

    CSCORE_MD_MOD_MGR(cscore_md) = mod_mgr_new(cscore_md_id, LOAD_BALANCING_LOOP);
    CSCORE_MD_CBGT_MD_ID(cscore_md)   = cbgt_md_id;
    CSCORE_CHASH_ALGO_ID(cscore_md)   = CSCORE_MD_DEFAULT_CHASH_ALGO_ID;
    CSCORE_CHASH_ALGO_FUNC(cscore_md) = chash_algo_fetch(CSCORE_CHASH_ALGO_ID(cscore_md));

    __cscore_row_no_pool_init(CSCORE_MD_ROW_NO_POOL(cscore_md), CSCORE_MD_DEFAULT_MAX_ROW_NUM);

    cbytes_set(CSCORE_MD_TABLE_NAME(cscore_md)              , (UINT8 *)CSCORE_MD_DEFAULT_TABLE_NAME    , CSCORE_MD_DEFAULT_TABLE_NAME_LEN);
    cbytes_set(CSCORE_MD_TABLE_COLF_NAME(cscore_md)          , (UINT8 *)CSCORE_MD_DEFAULT_COLF_NAME    , CSCORE_MD_DEFAULT_COLF_NAME_LEN );

    cbytes_set(CSCORE_MD_TABLE_COLQ_WORD_TEXT(cscore_md), (UINT8 *)CSCORE_MD_DEFAULT_COLQ_WORD_TEXT, CSCORE_MD_DEFAULT_COLQ_WORD_TEXT_LEN);
    cbytes_set(CSCORE_MD_TABLE_COLQ_DOC_ID(cscore_md)   , (UINT8 *)CSCORE_MD_DEFAULT_COLQ_DOC_ID   , CSCORE_MD_DEFAULT_COLQ_DOC_ID_LEN   );
    cbytes_set(CSCORE_MD_TABLE_COLQ_DOC_TYPE(cscore_md) , (UINT8 *)CSCORE_MD_DEFAULT_COLQ_DOC_TYPE , CSCORE_MD_DEFAULT_COLQ_DOC_TYPE_LEN );
    cbytes_set(CSCORE_MD_TABLE_COLQ_DOC_CODE(cscore_md) , (UINT8 *)CSCORE_MD_DEFAULT_COLQ_DOC_CODE , CSCORE_MD_DEFAULT_COLQ_DOC_CODE_LEN );


    cscore_md->usedcounter = 1;

    dbg_log(SEC_0051_CSCORE, 5)(LOGSTDOUT, "cscore_start: start CSCORE module #%ld\n", cscore_md_id);
    //dbg_log(SEC_0051_CSCORE, 3)(LOGSTDOUT, "========================= cscore_start: CSCORE table info:\n");
    //cscore_print_module_status(cscore_md_id, LOGSTDOUT);
    //cbc_print();

    return ( cscore_md_id );
}

/**
*
* end CSCORE module
*
**/
void cscore_end(const UINT32 cscore_md_id)
{
    CSCORE_MD *cscore_md;

    cscore_md = CSCORE_MD_GET(cscore_md_id);
    if(NULL_PTR == cscore_md)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT,"error:cscore_end: cscore_md_id = %ld not exist.\n", cscore_md_id);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < cscore_md->usedcounter )
    {
        cscore_md->usedcounter --;
        return ;
    }

    if ( 0 == cscore_md->usedcounter )
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT,"error:cscore_end: cscore_md_id = %ld is not started.\n", cscore_md_id);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    if(ERR_MODULE_ID != CSCORE_MD_CBGT_MD_ID(cscore_md))
    {
        cbgt_end(CSCORE_MD_CBGT_MD_ID(cscore_md));
        CSCORE_MD_CBGT_MD_ID(cscore_md) = ERR_MODULE_ID;
    }

    CSCORE_CHASH_ALGO_ID(cscore_md)   = CHASH_ERR_ALGO_ID;
    CSCORE_CHASH_ALGO_FUNC(cscore_md) = NULL_PTR;

    cbytes_clean(CSCORE_MD_TABLE_NAME(cscore_md)          , LOC_CSCORE_0002);
    cbytes_clean(CSCORE_MD_TABLE_COLF_NAME(cscore_md)     , LOC_CSCORE_0003);

    cbytes_clean(CSCORE_MD_TABLE_COLQ_WORD_TEXT(cscore_md), LOC_CSCORE_0004);
    cbytes_clean(CSCORE_MD_TABLE_COLQ_DOC_ID(cscore_md)   , LOC_CSCORE_0005);
    cbytes_clean(CSCORE_MD_TABLE_COLQ_DOC_TYPE(cscore_md) , LOC_CSCORE_0006);
    cbytes_clean(CSCORE_MD_TABLE_COLQ_DOC_CODE(cscore_md) , LOC_CSCORE_0007);

    if(NULL_PTR != CSCORE_MD_MOD_MGR(cscore_md))
    {
        mod_mgr_free(CSCORE_MD_MOD_MGR(cscore_md));
        CSCORE_MD_MOD_MGR(cscore_md)  = NULL_PTR;
    }

    /* free module : */
    //cscore_free_module_static_mem(cscore_md_id);

    cscore_md->usedcounter = 0;

    dbg_log(SEC_0051_CSCORE, 5)(LOGSTDOUT, "cscore_end: stop CSCORE module #%ld\n", cscore_md_id);
    cbc_md_free(MD_CSCORE, cscore_md_id);

    breathing_static_mem();

    //dbg_log(SEC_0051_CSCORE, 3)(LOGSTDOUT, "========================= cscore_end: CSCORE table info:\n");
    //cscore_print_module_status(cscore_md_id, LOGSTDOUT);
    //cbc_print();

    return ;
}

static EC_BOOL __cscore_row_no_pool_init(CSCORE_ROW_NO_POOL *cscore_row_no_pool, const UINT32 row_max_num)
{
    /*reserve row no pool from remote*/
    //TODO

    CSCORE_ROW_NO_POOL_INIT_CMUTEX(cscore_row_no_pool, LOC_CSCORE_0008);

    CSCORE_ROW_NO_POOL_BEG(cscore_row_no_pool) = CSCORE_MD_DEFAULT_BEG_ROW_NO;
    CSCORE_ROW_NO_POOL_END(cscore_row_no_pool) = CSCORE_ROW_NO_POOL_BEG(cscore_row_no_pool) + row_max_num;
    CSCORE_ROW_NO_POOL_CUR(cscore_row_no_pool) = CSCORE_ROW_NO_POOL_BEG(cscore_row_no_pool);

    return (EC_TRUE);
}

UINT32 cscore_reserve_row_no(const UINT32 cscore_md_id)
{
    CSCORE_MD  *cscore_md;
    CSCORE_ROW_NO_POOL *cscore_row_no_pool;
    UINT32 row_no;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_reserve_row_no: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_md  = CSCORE_MD_GET(cscore_md_id);
    cscore_row_no_pool = CSCORE_MD_ROW_NO_POOL(cscore_md);
    CSCORE_ROW_NO_POOL_CMUTEX_LOCK(cscore_row_no_pool, LOC_CSCORE_0009);
    if(CSCORE_ROW_NO_POOL_CUR(cscore_row_no_pool) >= CSCORE_ROW_NO_POOL_END(cscore_row_no_pool))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_reserve_row_no: row no pool is burn out\n");
        CSCORE_ROW_NO_POOL_CMUTEX_UNLOCK(cscore_row_no_pool, LOC_CSCORE_0010);
        return (CSCORE_MD_DEFAULT_ERR_ROW_NO);
    }
    row_no = CSCORE_ROW_NO_POOL_CUR(cscore_row_no_pool) ++;
    CSCORE_ROW_NO_POOL_CMUTEX_UNLOCK(cscore_row_no_pool, LOC_CSCORE_0011);

    return (row_no);
}

EC_BOOL cscore_release_row_no(const UINT32 cscore_md_id, const UINT32 row_no)
{
    CSCORE_MD  *cscore_md;
    CSCORE_ROW_NO_POOL *cscore_row_no_pool;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_release_row_no: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_md  = CSCORE_MD_GET(cscore_md_id);
    cscore_row_no_pool = CSCORE_MD_ROW_NO_POOL(cscore_md);

    if(CSCORE_MD_DEFAULT_ERR_ROW_NO == row_no)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_release_row_no: invalid row no\n");
        return (EC_FALSE);
    }
    
    CSCORE_ROW_NO_POOL_CMUTEX_LOCK(cscore_row_no_pool, LOC_CSCORE_0012);
    if(row_no < CSCORE_ROW_NO_POOL_BEG(cscore_row_no_pool) || row_no >= CSCORE_ROW_NO_POOL_END(cscore_row_no_pool))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_release_row_no: row no %ld not in range [%ld, %ld)\n",
                            row_no,
                            CSCORE_ROW_NO_POOL_BEG(cscore_row_no_pool),
                            CSCORE_ROW_NO_POOL_END(cscore_row_no_pool));
        CSCORE_ROW_NO_POOL_CMUTEX_UNLOCK(cscore_row_no_pool, LOC_CSCORE_0013);
        return (EC_FALSE);
    }
    //TODO:
    dbg_log(SEC_0051_CSCORE, 1)(LOGSTDOUT, "warn:cscore_release_row_no: not implemented yet\n");
    CSCORE_ROW_NO_POOL_CMUTEX_UNLOCK(cscore_row_no_pool, LOC_CSCORE_0014);

    return (EC_TRUE);
}

CSDOC *cscore_csdoc_new(const UINT32 cscore_md_id)
{
    CSDOC *csdoc;

#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_new: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    alloc_static_mem(MD_CSCORE, cscore_md_id, MM_CSDOC, &csdoc, LOC_CSCORE_0015);
    if(NULL_PTR == csdoc)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_new:alloc csdoc failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cscore_csdoc_init(cscore_md_id, csdoc))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_new: init csdoc failed\n");
        free_static_mem(MD_CSCORE, cscore_md_id, MM_CSDOC, csdoc, LOC_CSCORE_0016);
        return (NULL_PTR);
    }

    return (csdoc);
}

EC_BOOL cscore_csdoc_init(const UINT32 cscore_md_id, CSDOC *csdoc)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_init: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    CSDOC_NAME(csdoc)    = NULL_PTR;
    CSDOC_ID(csdoc)      = CSDOC_ERROR_ID;
    CSDOC_TYPE(csdoc)    = CSDOC_ERROR_TYPE;
    CSDOC_CODE(csdoc)    = CSDOC_ERROR_CODE;
    CSDOC_RATE(csdoc)    = 0;
    CSDOC_CONTENT(csdoc) = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_clean(const UINT32 cscore_md_id, CSDOC *csdoc)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_clean: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(NULL_PTR != CSDOC_NAME(csdoc))
    {
        cstring_free(CSDOC_NAME(csdoc));
        CSDOC_NAME(csdoc) = NULL_PTR;
    }

    if(NULL_PTR != CSDOC_CONTENT(csdoc))
    {
        cbytes_free(CSDOC_CONTENT(csdoc), LOC_CSCORE_0017);
        CSDOC_CONTENT(csdoc) = NULL_PTR;
    }

    CSDOC_ID(csdoc)      = CSDOC_ERROR_ID;
    CSDOC_TYPE(csdoc)    = CSDOC_ERROR_TYPE;
    CSDOC_CODE(csdoc)    = CSDOC_ERROR_CODE;
    CSDOC_RATE(csdoc)    = 0;

    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_free(const UINT32 cscore_md_id, CSDOC *csdoc)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_free: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(NULL_PTR != csdoc)
    {
        cscore_csdoc_clean(cscore_md_id, csdoc);
        free_static_mem(MD_CSCORE, cscore_md_id, MM_CSDOC, csdoc, LOC_CSCORE_0018);
    }
    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_clone(const UINT32 cscore_md_id, const CSDOC *csdoc_src, CSDOC *csdoc_des)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_clone: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    CSDOC_ID(csdoc_des)      = CSDOC_ID(csdoc_src);
    CSDOC_TYPE(csdoc_des)    = CSDOC_TYPE(csdoc_src);
    CSDOC_CODE(csdoc_des)    = CSDOC_CODE(csdoc_src);
    CSDOC_RATE(csdoc_des)    = CSDOC_RATE(csdoc_src);

    if(NULL_PTR != CSDOC_CONTENT(csdoc_src))
    {
        CSDOC_CONTENT(csdoc_des) = cbytes_dup(CSDOC_CONTENT(csdoc_src));
        if(NULL_PTR == CSDOC_CONTENT(csdoc_des))
        {
            dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_clone: dup csdoc content failed\n");
            return (EC_FALSE);
        }
    }    
    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_rate_lt(const UINT32 cscore_md_id, const CSDOC *csdoc_1st, const CSDOC *csdoc_2nd)
{
    if(CSDOC_RATE(csdoc_1st) < CSDOC_RATE(csdoc_2nd))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cscore_csdoc_rate_le(const UINT32 cscore_md_id, const CSDOC *csdoc_1st, const CSDOC *csdoc_2nd)
{
    if(CSDOC_RATE(csdoc_1st) <= CSDOC_RATE(csdoc_2nd))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cscore_csdoc_rate_gt(const UINT32 cscore_md_id, const CSDOC *csdoc_1st, const CSDOC *csdoc_2nd)
{
    if(CSDOC_RATE(csdoc_1st) > CSDOC_RATE(csdoc_2nd))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cscore_csdoc_rate_ge(const UINT32 cscore_md_id, const CSDOC *csdoc_1st, const CSDOC *csdoc_2nd)
{
    if(CSDOC_RATE(csdoc_1st) >= CSDOC_RATE(csdoc_2nd))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cscore_csdoc_set_name(const UINT32 cscore_md_id, CSDOC *csdoc, const CSTRING *doc_name)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_set_name: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(NULL_PTR != doc_name)
    {
        CSTRING *name_cstr;

        name_cstr = cstring_new(cstring_get_str(doc_name), LOC_CSCORE_0019);
        if(NULL_PTR == name_cstr)
        {
            dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_set_name: new cstring failed\n");
            return (EC_FALSE);
        }

        if(NULL_PTR != CSDOC_NAME(csdoc))
        {
            cstring_free(CSDOC_NAME(csdoc));
            CSDOC_NAME(csdoc) = NULL_PTR;
        }

        CSDOC_NAME(csdoc) = name_cstr;
    }

    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_set_doc_id(const UINT32 cscore_md_id, CSDOC *csdoc, const UINT32 doc_id)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_set_doc_id: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    CSDOC_ID(csdoc) = doc_id;
    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_set_doc_type(const UINT32 cscore_md_id, CSDOC *csdoc, const UINT32 doc_type)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_set_doc_type: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    CSDOC_TYPE(csdoc) = doc_type;
    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_set_doc_code(const UINT32 cscore_md_id, CSDOC *csdoc, const UINT32 doc_code)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_set_doc_code: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    CSDOC_CODE(csdoc) = doc_code;
    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_set_doc_content(const UINT32 cscore_md_id, CSDOC *csdoc, const CBYTES *doc_content)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_set_doc_content: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(NULL_PTR != doc_content)
    {
        CBYTES *content_bytes;

        content_bytes = cbytes_new(0);
        if(NULL_PTR == content_bytes)
        {
            dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_set_doc_content: new cbytes failed\n");
            return (EC_FALSE);
        }

        if(NULL_PTR != CSDOC_CONTENT(csdoc))
        {
            cbytes_free(CSDOC_CONTENT(csdoc), LOC_CSCORE_0020);
            CSDOC_CONTENT(csdoc) = NULL_PTR;
        }

        CSDOC_CONTENT(csdoc) = content_bytes;
    }
    return (EC_TRUE);
}

void   cscore_csdoc_print(LOG *log, const CSDOC *csdoc)
{
    if(NULL_PTR != CSDOC_NAME(csdoc))
    {
        sys_print(log, "doc name: %s\n", (char *)cstring_get_str(CSDOC_NAME(csdoc)));
    }
    else
    {
        sys_print(log, "doc name: <null>\n");
    }

    sys_print(log, "doc id  : %ld\n", CSDOC_ID(csdoc));
    sys_print(log, "doc type: %ld\n", CSDOC_TYPE(csdoc));
    sys_print(log, "doc code: %ld\n", CSDOC_CODE(csdoc));
    sys_print(log, "doc rate: %ld\n", CSDOC_RATE(csdoc));

    if(NULL_PTR != CSDOC_CONTENT(csdoc))
    {
        sys_print(log, "doc content:\n");
        cbytes_print_str(log, CSDOC_CONTENT(csdoc));
    }
    else
    {
        sys_print(log, "doc content: <null>\n");
    }

    return;
}

CSWORD *cscore_csword_new(const UINT32 cscore_md_id, const UINT32 location)
{
    CSWORD *csword;

#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_new: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    alloc_static_mem(MD_CSCORE, cscore_md_id, MM_CSWORD, &csword, location);
    if(NULL_PTR == csword)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_new:alloc csword failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cscore_csword_init(cscore_md_id, csword))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_new: init csword failed\n");
        free_static_mem(MD_CSCORE, cscore_md_id, MM_CSWORD, csword, location);
        return (NULL_PTR);
    }

    return (csword);
}

EC_BOOL cscore_csword_init(const UINT32 cscore_md_id, CSWORD *csword)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_init: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cbytes_init(CSWORD_CONTENT(csword));

    return (EC_TRUE);
}

EC_BOOL cscore_csword_clean(const UINT32 cscore_md_id, CSWORD *csword)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_clean: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cbytes_clean(CSWORD_CONTENT(csword), LOC_CSCORE_0021);
    return (EC_TRUE);
}

EC_BOOL cscore_csword_free(const UINT32 cscore_md_id, CSWORD *csword)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_free: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(NULL_PTR != csword)
    {
        cscore_csword_clean(cscore_md_id, csword);
        free_static_mem(MD_CSCORE, cscore_md_id, MM_CSWORD, csword, LOC_CSCORE_0022);
    }
    return (EC_TRUE);
}

EC_BOOL cscore_csword_make_by_cbytes_content(const UINT32 cscore_md_id, CSWORD *csword, const CBYTES *word_content)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_make_by_cbytes_content: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cbytes_clean(CSWORD_CONTENT(csword), LOC_CSCORE_0023);
    if(EC_FALSE == cbytes_clone(word_content, CSWORD_CONTENT(csword)))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_make_by_cbytes_content: clone content failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/*make csword from string word content*/
CSWORD *cscore_csword_make_by_str_content(const UINT32 cscore_md_id, const UINT8 *word_content_str)
{
    CSWORD *csword;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_make_by_str_content: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    csword = cscore_csword_new(cscore_md_id, LOC_CSCORE_0024);
    if(NULL_PTR == csword)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_make_by_str_content: new csword failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cbytes_set(CSWORD_CONTENT(csword), word_content_str, strlen((char *)word_content_str)))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_make_by_str_content: set word content cbytes from str %s failed\n", 
                            (char *)word_content_str);
        cscore_csword_free(cscore_md_id, csword);
        return (NULL_PTR);
    }

    return (csword);
}

/*make csword from cstring word content*/
CSWORD *cscore_csword_make_by_cstr_content(const UINT32 cscore_md_id, const CSTRING *word_content_cstr)
{
    CSWORD *csword;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_make_by_cstr_content: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    csword = cscore_csword_new(cscore_md_id, LOC_CSCORE_0025);
    if(NULL_PTR == csword)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_make_by_cstr_content: new csword failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cbytes_set(CSWORD_CONTENT(csword), cstring_get_str(word_content_cstr), cstring_get_len(word_content_cstr)))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_make_by_cstr_content: set word content cbytes from cstr %.s failed\n", 
                            cstring_get_len(word_content_cstr), cstring_get_str(word_content_cstr));
        cscore_csword_free(cscore_md_id, csword);
        return (NULL_PTR);
    }

    return (csword);
}

EC_BOOL cscore_csword_clone(const UINT32 cscore_md_id, const CSWORD *csword_src, CSWORD *csword_des)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_clone: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cbytes_clean(CSWORD_CONTENT(csword_des), LOC_CSCORE_0026);
    cbytes_clone(CSWORD_CONTENT(csword_src), CSWORD_CONTENT(csword_des));
    return (EC_TRUE);
}

void   cscore_csword_print(LOG *log, const CSWORD *csword)
{
    sys_print(log, "word content:\n");
    cbytes_print_str(log, CSWORD_CONTENT(csword));

    return;
}

CSDOC_WORDS *cscore_csdoc_words_new(const UINT32 cscore_md_id)
{
    CSDOC_WORDS *csdoc_words;

#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_new: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    alloc_static_mem(MD_CSCORE, cscore_md_id, MM_CSDOC_WORDS, &csdoc_words, LOC_CSCORE_0027);
    if(NULL_PTR == csdoc_words)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_words_new:alloc csdoc_words failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cscore_csdoc_words_init(cscore_md_id, csdoc_words))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_words_new: init csdoc_words failed\n");
        free_static_mem(MD_CSCORE, cscore_md_id, MM_CSDOC_WORDS, csdoc_words, LOC_CSCORE_0028);
        return (NULL_PTR);
    }

    return (csdoc_words);
}

EC_BOOL cscore_csdoc_words_init(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_init: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_csdoc_init(cscore_md_id, CSDOC_WORDS_DOC(csdoc_words));
    clist_init(CSDOC_WORDS_LIST(csdoc_words), MM_CSWORD, LOC_CSCORE_0029);

    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_words_clean(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_clean: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_csdoc_clean(cscore_md_id, CSDOC_WORDS_DOC(csdoc_words));
    clist_clean_with_modi(CSDOC_WORDS_LIST(csdoc_words), cscore_md_id, (CLIST_DATA_CLEAN)cscore_csword_free);

    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_words_free(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_free: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(NULL_PTR != csdoc_words)
    {
        cscore_csdoc_words_clean(cscore_md_id, csdoc_words);
        free_static_mem(MD_CSCORE, cscore_md_id, MM_CSDOC_WORDS, csdoc_words, LOC_CSCORE_0030);
    }
    return (EC_TRUE);
}

void   cscore_csdoc_words_print(LOG *log, const CSDOC_WORDS *csdoc_words)
{
    cscore_csdoc_print(log, CSDOC_WORDS_DOC(csdoc_words));
    clist_print(log, CSDOC_WORDS_LIST(csdoc_words), (CLIST_DATA_DATA_PRINT)cscore_csword_print);

    return;
}

const CSDOC *cscore_csdoc_words_get_doc(const UINT32 cscore_md_id, const CSDOC_WORDS *csdoc_words)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_get_doc: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    return (CSDOC_WORDS_DOC(csdoc_words));
}

const CLIST *cscore_csdoc_words_get_word_list(const UINT32 cscore_md_id, const CSDOC_WORDS *csdoc_words)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_get_word_list: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    return (CSDOC_WORDS_LIST(csdoc_words));
}

EC_BOOL cscore_csdoc_words_push_word(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words, const CSWORD *csword)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_push_word: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    clist_push_back(CSDOC_WORDS_LIST(csdoc_words), (void *)csword);
    return (EC_TRUE);
}

EC_BOOL cscore_csdoc_words_add_word(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words, const CSWORD *csword)
{
    CSWORD *csword_des;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_add_word: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    csword_des = cscore_csword_new(cscore_md_id, LOC_CSCORE_0031);
    if(NULL_PTR == csword_des)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_words_add_word: new csword failed\n");
        return (EC_FALSE);
    }
    
    cscore_csword_clone(cscore_md_id, csword, csword_des);
    clist_push_back(CSDOC_WORDS_LIST(csdoc_words), (void *)csword_des);
    return (EC_TRUE);
}


CSWORD_DOCS *cscore_csword_docs_new(const UINT32 cscore_md_id)
{
    CSWORD_DOCS *csword_docs;

#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_docs_new: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    alloc_static_mem(MD_CSCORE, cscore_md_id, MM_CSWORD_DOCS, &csword_docs, LOC_CSCORE_0032);
    if(NULL_PTR == csword_docs)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_new:alloc csword_docs failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cscore_csword_docs_init(cscore_md_id, csword_docs))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_new: init csword_docs failed\n");
        free_static_mem(MD_CSCORE, cscore_md_id, MM_CSWORD_DOCS, csword_docs, LOC_CSCORE_0033);
        return (NULL_PTR);
    }

    return (csword_docs);
}

EC_BOOL cscore_csword_docs_init(const UINT32 cscore_md_id, CSWORD_DOCS *csword_docs)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_docs_init: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_csword_init(cscore_md_id, CSWORD_DOCS_WORD(csword_docs));
    clist_init(CSWORD_DOCS_LIST(csword_docs), MM_CSDOC, LOC_CSCORE_0034);

    return (EC_TRUE);
}

EC_BOOL cscore_csword_docs_clean(const UINT32 cscore_md_id, CSWORD_DOCS *csword_docs)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_docs_clean: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_csword_clean(cscore_md_id, CSWORD_DOCS_WORD(csword_docs));
    clist_clean_with_modi(CSWORD_DOCS_LIST(csword_docs), cscore_md_id, (CLIST_DATA_CLEAN)cscore_csdoc_free);

    return (EC_TRUE);
}

EC_BOOL cscore_csword_docs_free(const UINT32 cscore_md_id, CSWORD_DOCS *csword_docs)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_docs_free: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(NULL_PTR != csword_docs)
    {
        cscore_csword_docs_clean(cscore_md_id, csword_docs);
        free_static_mem(MD_CSCORE, cscore_md_id, MM_CSWORD_DOCS, csword_docs, LOC_CSCORE_0035);
    }
    return (EC_TRUE);
}

EC_BOOL cscore_csword_docs_set_word(const UINT32 cscore_md_id, CSWORD_DOCS *csword_docs, const CSWORD *csword)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_docs_set_word: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_csword_clone(cscore_md_id, csword, CSWORD_DOCS_WORD(csword_docs));
    return (EC_TRUE);
}

void   cscore_csword_docs_print(LOG *log, const CSWORD_DOCS *csword_docs)
{
    cscore_csword_print(log, CSWORD_DOCS_WORD(csword_docs));
    clist_print(log, CSWORD_DOCS_LIST(csword_docs), (CLIST_DATA_DATA_PRINT)cscore_csdoc_print);

    return;
}

static UINT32 * __cscore_new_uint32(const UINT32 cscore_md_id)
{
    UINT32 *data;
    alloc_static_mem(MD_CSCORE, cscore_md_id, MM_UINT32, &data, LOC_CSCORE_0036);
    return (data);
}

static EC_BOOL __cscore_free_uint32(const UINT32 cscore_md_id, UINT32 *data)
{
    if(NULL_PTR != data)
    {
        free_static_mem(MD_CSCORE, cscore_md_id, MM_UINT32, data, LOC_CSCORE_0037);
    }
    return (EC_TRUE);
}

static CVECTOR *__cscore_new_cbytes_vec(const UINT32 cscore_md_id)
{
    CVECTOR *cbytes_vec;
    cbytes_vec = cvector_new(0, MM_CBYTES, LOC_CSCORE_0038);
    return (cbytes_vec);
}

static EC_BOOL __cscore_free_cbytes_vec(const UINT32 cscore_md_id, CVECTOR *cbytes_vec)
{
    if(NULL_PTR != cbytes_vec)
    {
        cvector_clean_with_location(cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0039);
        cvector_free(cbytes_vec, LOC_CSCORE_0040);
    }
    return (EC_TRUE);
}

static CBYTES *__cscore_new_row_cbytes_by_csword_0(const UINT32 cscore_md_id, const CSWORD *csword)
{
    CSCORE_MD  *cscore_md;
    CBYTES     *row_bytes;
    UINT32      csword_hash_val;
    CHASH_ALGO  chash_algo;

    cscore_md  = CSCORE_MD_GET(cscore_md_id);
    chash_algo = CSCORE_CHASH_ALGO_FUNC(cscore_md);

    csword_hash_val = chash_algo(cbytes_len(CSWORD_CONTENT(csword)), cbytes_buf(CSWORD_CONTENT(csword)));
    dbg_log(SEC_0051_CSCORE, 9)(LOGSTDOUT, "[DEBUG] __cscore_new_row_cbytes_by_csword: word = %.*s => hash %ld\n",
                        cbytes_len(CSWORD_CONTENT(csword)), 
                        cbytes_buf(CSWORD_CONTENT(csword)),
                        csword_hash_val);
    row_bytes = cbytes_make_by_word(csword_hash_val);
    if(NULL_PTR == row_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_new_row_cbytes_by_csword: make cbytes by csword hash val %ld failed\n", csword_hash_val);
        return (NULL_PTR);
    }
    return (row_bytes);
}

static CSTRING *__cscore_new_row_cstring_by_csword_0(const UINT32 cscore_md_id, const CSWORD *csword)
{
    CSCORE_MD  *cscore_md;
    CSTRING    *row_cstring;
    UINT32      csword_hash_val;
    CHASH_ALGO  chash_algo;

    cscore_md  = CSCORE_MD_GET(cscore_md_id);
    chash_algo = CSCORE_CHASH_ALGO_FUNC(cscore_md);

    csword_hash_val = chash_algo(cbytes_len(CSWORD_CONTENT(csword)), cbytes_buf(CSWORD_CONTENT(csword)));
    dbg_log(SEC_0051_CSCORE, 9)(LOGSTDOUT, "[DEBUG] __cscore_new_row_cstring_by_csword: word %.*s => hash %ld\n",
                        cbytes_len(CSWORD_CONTENT(csword)),
                        cbytes_buf(CSWORD_CONTENT(csword)),
                        csword_hash_val);

    row_cstring = cstring_make_by_word(csword_hash_val);
    if(NULL_PTR == row_cstring)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_new_row_cstring_by_csword: make cstring by csword hash val %ld failed\n", csword_hash_val);
        return (NULL_PTR);
    }
    return (row_cstring);
}

static CBYTES *__cscore_new_row_cbytes_by_csword(const UINT32 cscore_md_id, const CSWORD *csword)
{
    CBYTES     *row_bytes;
    
    row_bytes = cbytes_dup(CSWORD_CONTENT(csword));
    if(NULL_PTR == row_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_new_row_cbytes_by_csword: dup cbytes failed\n");
        return (NULL_PTR);
    }
    return (row_bytes);
}

static CSTRING *__cscore_new_row_cstring_by_csword(const UINT32 cscore_md_id, const CSWORD *csword)
{
    CSTRING    *row_cstring;

    row_cstring = cstring_make_by_bytes(cbytes_len(CSWORD_CONTENT(csword)), cbytes_buf(CSWORD_CONTENT(csword)));
    if(NULL_PTR == row_cstring)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_new_row_cstring_by_csword: make cstring by bytes failed\n");
        return (NULL_PTR);
    }
    return (row_cstring);
}

static CBYTES *__cscore_new_ts_cbytes_by_ctimet(const UINT32 cscore_md_id, const CTIMET *ctimet)
{
    CBYTES *ts_cbytes;

    ts_cbytes = cbytes_make_by_ctimet(ctimet);
    if(NULL_PTR == ts_cbytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_new_ts_cbytes_by_ctimet: make cbytes for ts failed\n");
        return (NULL_PTR);
    }

    return (ts_cbytes);
}

static CSTRING *__cscore_new_ts_cstring_by_ctimet(const UINT32 cscore_md_id, const CTIMET *ctimet)
{
    CSTRING *ts_cstring;

    ts_cstring = cstring_make_by_ctimet(ctimet);
    if(NULL_PTR == ts_cstring)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_new_ts_cstring_by_ctimet: new cstring for ts failed\n");
        return (NULL_PTR);
    }

    return (ts_cstring);
}

static EC_BOOL __cscore_csword_import_word_content_plain(const UINT32 cscore_md_id, const CSWORD *csword,
                                                                const CBYTES *row_bytes, 
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *cached_bytes_vec)
{
    CSCORE_MD *cscore_md;

    CBYTES *cword_content_bytes;
    UINT32 *ret;

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    /*make cdoc_id num bytes*/
    cword_content_bytes = cbytes_dup(CSWORD_CONTENT(csword));
    if(NULL_PTR == cword_content_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_word_content_plain: dup cbytes of csword content failed\n");
        return (EC_FALSE);
    }

    /*make ret ready*/
    ret = __cscore_new_uint32(cscore_md_id);
    if(NULL_PTR == ret)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_word_content_plain: new UINT32 failed\n");
        cbytes_free(cword_content_bytes, LOC_CSCORE_0041);
        return (EC_FALSE);
    }
    (*ret) = EC_FALSE;

    cvector_push_no_lock(cached_bytes_vec, (void *)cword_content_bytes);
    cvector_push_no_lock(ret_vec         , (void *)ret              );

    /*make task*/
    /*table name        = def table name */
    /*row key           = <row no>       */
    /*column family     = "docwords"     */
    /*column qualifier  = "wordtext"     */
    /*timestamp,version = <current times>*/
    /*value             = <word text>    */
    task_inc(task_mgr, ret, FI_cbgt_insert, ERR_MODULE_ID,
              CSCORE_MD_TABLE_NAME(cscore_md),        
              row_bytes,                              
              CSCORE_MD_TABLE_COLF_NAME(cscore_md),   
              CSCORE_MD_TABLE_COLQ_WORD_TEXT(cscore_md),
              cword_content_bytes,                      
              EC_FALSE);

    return (EC_TRUE);
}

static EC_BOOL __cscore_csword_import_doc_id_plain(const UINT32 cscore_md_id, const CSWORD *csword, const CSDOC *csdoc,
                                                                const CBYTES *row_bytes, 
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *cached_bytes_vec)
{
    CSCORE_MD *cscore_md;

    CBYTES *cdoc_id_num_bytes;
    UINT32 *ret;

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    /*make cdoc_id num bytes*/
    cdoc_id_num_bytes = cbytes_make_by_word(CSDOC_ID(csdoc));
    if(NULL_PTR == cdoc_id_num_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_doc_id_plain: make cbytes for cdoc_id failed\n");
        return (EC_FALSE);
    }

    /*make ret ready*/
    ret = __cscore_new_uint32(cscore_md_id);
    if(NULL_PTR == ret)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_doc_id_plain: new UINT32 failed\n");
        cbytes_free(cdoc_id_num_bytes, LOC_CSCORE_0042);
        return (EC_FALSE);
    }
    (*ret) = EC_FALSE;

    cvector_push_no_lock(cached_bytes_vec, (void *)cdoc_id_num_bytes);
    cvector_push_no_lock(ret_vec         , (void *)ret              );

    /*make task*/
    /*table name        = def table name */
    /*row key           = <row no>       */
    /*column family     = "docwords"     */
    /*column qualifier  = "docid"        */
    /*timestamp,version = <current times>*/
    /*value             = <cdoc_id>      */
    task_inc(task_mgr, ret, FI_cbgt_insert, ERR_MODULE_ID,
              CSCORE_MD_TABLE_NAME(cscore_md),        
              row_bytes,                              
              CSCORE_MD_TABLE_COLF_NAME(cscore_md),   
              CSCORE_MD_TABLE_COLQ_DOC_ID(cscore_md),
              cdoc_id_num_bytes,                      
              EC_FALSE);

    return (EC_TRUE);
}

static EC_BOOL __cscore_csword_import_doc_type_plain(const UINT32 cscore_md_id, const CSWORD *csword, const CSDOC *csdoc,
                                                                const CBYTES *row_bytes,
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *cached_bytes_vec)
{
    CSCORE_MD *cscore_md;

    CBYTES *cdoc_type_num_bytes;
    UINT32 *ret;

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    /*make cdoc_type num bytes*/
    cdoc_type_num_bytes = cbytes_make_by_word(CSDOC_TYPE(csdoc));
    if(NULL_PTR == cdoc_type_num_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_doc_type_plain: make cbytes for cdoc_type failed\n");
        return (EC_FALSE);
    }

    /*make ret ready*/
    ret = __cscore_new_uint32(cscore_md_id);
    if(NULL_PTR == ret)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_doc_type_plain: new UINT32 failed\n");
        cbytes_free(cdoc_type_num_bytes, LOC_CSCORE_0043);
        return (EC_FALSE);
    }
    (*ret) = EC_FALSE;

    cvector_push_no_lock(cached_bytes_vec, (void *)cdoc_type_num_bytes);
    cvector_push_no_lock(ret_vec         , (void *)ret                );

    /*make task*/
    /*table name        = def table name */
    /*row key           = <row no>       */
    /*column family     = "docwords"     */
    /*column qualifier  = "doctype"        */
    /*timestamp,version = <current times>*/
    /*value             = <cdoc_type>      */    
    task_inc(task_mgr, ret, FI_cbgt_insert, ERR_MODULE_ID,
              CSCORE_MD_TABLE_NAME(cscore_md),
              row_bytes,
              CSCORE_MD_TABLE_COLF_NAME(cscore_md),
              CSCORE_MD_TABLE_COLQ_DOC_TYPE(cscore_md),
              cdoc_type_num_bytes,
              EC_FALSE);

    return (EC_TRUE);
}

static EC_BOOL __cscore_csword_import_doc_code_plain(const UINT32 cscore_md_id, const CSWORD *csword, const CSDOC *csdoc,
                                                                const CBYTES *row_bytes,
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *cached_bytes_vec)
{
    CSCORE_MD *cscore_md;

    CBYTES *cdoc_code_num_bytes;
    UINT32 *ret;

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    /*make cdoc_code num bytes*/
    cdoc_code_num_bytes = cbytes_make_by_word(CSDOC_CODE(csdoc));
    if(NULL_PTR == cdoc_code_num_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_doc_code_plain: make cbytes for cdoc_code failed\n");
        return (EC_FALSE);
    }

    /*make ret ready*/
    ret = __cscore_new_uint32(cscore_md_id);
    if(NULL_PTR == ret)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_doc_code_plain: new UINT32 failed\n");
        cbytes_free(cdoc_code_num_bytes, LOC_CSCORE_0044);
        return (EC_FALSE);
    }
    (*ret) = EC_FALSE;

    cvector_push_no_lock(cached_bytes_vec, (void *)cdoc_code_num_bytes);
    cvector_push_no_lock(ret_vec         , (void *)ret                );

    /*make task*/
    /*table name        = def table name */
    /*row key           = <row no>       */
    /*column family     = "docwords"     */
    /*column qualifier  = "doccode"        */
    /*timestamp,version = <current times>*/
    /*value             = <cdoc_code>      */       
    task_inc(task_mgr, ret, FI_cbgt_insert, ERR_MODULE_ID,
              CSCORE_MD_TABLE_NAME(cscore_md),
              row_bytes,
              CSCORE_MD_TABLE_COLF_NAME(cscore_md),
              CSCORE_MD_TABLE_COLQ_DOC_CODE(cscore_md),
              cdoc_code_num_bytes,
              EC_FALSE);

    return (EC_TRUE);
}

static EC_BOOL __cscore_csword_import_plain(const UINT32 cscore_md_id, const CSWORD *csword, const CSDOC *csdoc,
                                                        CTIMET *ctimet,
                                                        TASK_MGR *task_mgr,
                                                        CVECTOR *ret_vec, CVECTOR *cached_bytes_vec)
{
    CBYTES *row_bytes;

    UINT32  row_no;

    row_no = cscore_reserve_row_no(cscore_md_id);
    if(CSCORE_MD_DEFAULT_ERR_ROW_NO == row_no)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_plain: reserve row no failed\n");
        return (EC_FALSE);
    }

    row_bytes = cbytes_make_by_word(row_no);
    if(NULL_PTR == row_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_plain: new row cbytes by row no %ld failed\n", row_no);
        cscore_release_row_no(cscore_md_id, row_no);
        return (EC_FALSE);
    }

    if(EC_FALSE == __cscore_csword_import_word_content_plain(cscore_md_id, csword,
                                                        row_bytes,
                                                        task_mgr,
                                                        ret_vec, cached_bytes_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_plain: import word text failed\n");
        cscore_release_row_no(cscore_md_id, row_no);
        cbytes_free(row_bytes, LOC_CSCORE_0045);
        return (EC_FALSE);
    }    

    if(EC_FALSE == __cscore_csword_import_doc_id_plain(cscore_md_id, csword, csdoc,
                                                        row_bytes, 
                                                        task_mgr,
                                                        ret_vec, cached_bytes_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_plain: import doc id failed\n");
        cscore_release_row_no(cscore_md_id, row_no);
        cbytes_free(row_bytes, LOC_CSCORE_0046);
        return (EC_FALSE);
    }

    if(EC_FALSE == __cscore_csword_import_doc_type_plain(cscore_md_id, csword, csdoc,
                                                            row_bytes, 
                                                            task_mgr,
                                                            ret_vec, cached_bytes_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_plain: import doc type failed\n");
        cscore_release_row_no(cscore_md_id, row_no);
        cbytes_free(row_bytes, LOC_CSCORE_0047);
        return (EC_FALSE);
    }

    if(EC_FALSE == __cscore_csword_import_doc_code_plain(cscore_md_id, csword, csdoc,
                                                            row_bytes, 
                                                            task_mgr,
                                                            ret_vec, cached_bytes_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_import_plain: import doc code failed\n");
        cscore_release_row_no(cscore_md_id, row_no);
        cbytes_free(row_bytes, LOC_CSCORE_0048);
        return (EC_FALSE);
    }

    cvector_push_no_lock(cached_bytes_vec, (void *)row_bytes);
    return (EC_TRUE);
}

static EC_BOOL __cscore_csword_list_import_plain(const UINT32 cscore_md_id, const CLIST *csword_list, const CSDOC *csdoc,
                                                            TASK_MGR *task_mgr,
                                                            CVECTOR *ret_vec, CVECTOR *cached_bytes_vec)
{
    CTIMET      cur_time;
    EC_BOOL     ret;

    CTIMET_GET(cur_time);
    
    if(EC_FALSE == clist_loop((CLIST *)csword_list, (void *)&ret, CLIST_CHECKER_DEFAULT, 
                               (UINT32)7, 
                               (UINT32)1,
                               (UINT32)__cscore_csword_import_plain, 
                                cscore_md_id, 
                                NULL_PTR, 
                                csdoc,
                                &cur_time,
                                task_mgr,
                                ret_vec, 
                                cached_bytes_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_list_import_plain: csword import plain failed\n");
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

/*import doc and its word tokens into bigtable*/
EC_BOOL cscore_csdoc_words_import(const UINT32 cscore_md_id, const CSDOC_WORDS *csdoc_words)
{
    CSCORE_MD *cscore_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    CVECTOR    ret_vec;
    CVECTOR    cached_bytes_vec;

#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_import: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    cvector_init(&ret_vec, 0, MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CSCORE_0049);
    cvector_init(&cached_bytes_vec, 0, MM_CBYTES, CVECTOR_LOCK_ENABLE, LOC_CSCORE_0050);

    mod_mgr = mod_mgr_new(CSCORE_MD_CBGT_MD_ID(cscore_md), LOAD_BALANCING_OBJ);
    if(NULL_PTR == mod_mgr)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_words_import: new mod_mgr failed\n");

        cvector_clean_with_modi(&ret_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32, LOC_CSCORE_0051);
        cvector_clean_with_location(&cached_bytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0052);
   
        return (EC_FALSE);
    }
    
    mod_mgr_incl(CMPI_LOCAL_TCID, CMPI_LOCAL_COMM, CMPI_LOCAL_RANK, CSCORE_MD_CBGT_MD_ID(cscore_md), mod_mgr);
    dbg_log(SEC_0051_CSCORE, 0)(LOGCONSOLE, "[DEBUG] cscore_csdoc_words_import: mod_mgr is\n");
    mod_mgr_print(LOGCONSOLE, mod_mgr);

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    if(NULL_PTR == task_mgr)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_words_import: new task_mgr failed\n");

        cvector_clean_with_modi(&ret_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32, LOC_CSCORE_0053);
        cvector_clean_with_location(&cached_bytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0054);
        
        mod_mgr_free(mod_mgr);
        
        return (EC_FALSE);
    }

    /*import word list*/
    if(EC_FALSE == __cscore_csword_list_import_plain(cscore_md_id, CSDOC_WORDS_LIST(csdoc_words), CSDOC_WORDS_DOC(csdoc_words),
                                                      task_mgr,
                                                      &ret_vec, &cached_bytes_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_words_import: import csword list failed\n");

        cvector_clean_with_modi(&ret_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32, LOC_CSCORE_0055);
        cvector_clean_with_location(&cached_bytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0056);

        mod_mgr_free(mod_mgr);
        task_mgr_free(task_mgr);

        return (EC_FALSE);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    mod_mgr_free(mod_mgr);

    /*check result*/
    if(EC_FALSE == cvector_check_all_is_true(&ret_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_words_import: import csdoc_words failed\n");

        cvector_clean_with_modi(&ret_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32, LOC_CSCORE_0057);
        cvector_clean_with_location(&cached_bytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0058);

        return (EC_FALSE);
    }

    /*clean up*/
    cvector_clean_with_modi(&ret_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32, LOC_CSCORE_0059);
    cvector_clean_with_location(&cached_bytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0060);

    return (EC_TRUE);
}

/*csdoc_words_clist item type is CSDOC_WORDS*/
EC_BOOL cscore_csdoc_words_list_import(const UINT32 cscore_md_id, const CLIST *csdoc_words_clist)
{
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csdoc_words_list_import: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(EC_FALSE == clist_loop_front_with_modi(csdoc_words_clist, cscore_md_id, (CLIST_DATA_MODI_HANDLER)cscore_csdoc_words_import))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csdoc_words_list_import: import csdoc_words list failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

static EC_BOOL __cscore_csdoc_export_doc_id_plain(const UINT32 cscore_md_id, const CBYTES *kv_bytes, 
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *doc_id_cbytes_vec,
                                                                CVECTOR *cached_cbytes_vec)
{
    CSCORE_MD *cscore_md;
    
    const uint8_t *kv;
    const uint8_t *colq_str;
    uint16_t       colq_len;

    CBYTES *row_bytes;
    CBYTES *colf_bytes;
    CBYTES *colq_bytes;

    UINT32 *ret;
    CBYTES *val_bytes;

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    ret = __cscore_new_uint32(cscore_md_id);
    if(NULL_PTR == ret)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_id_plain: new ret uint32 failed\n");
        return (EC_FALSE);
    }
    cvector_push_no_lock(ret_vec, (void *)ret);    

    val_bytes = cbytes_new(0);
    if(NULL_PTR == val_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_id_plain: new val cbytes failed\n");
        return (EC_FALSE);
    }
    cvector_push_no_lock(doc_id_cbytes_vec, (void *)val_bytes);

    kv = cbytes_buf(kv_bytes);

    colq_len = CSCORE_MD_DEFAULT_COLQ_DOC_ID_LEN;
    colq_str = (UINT8 *)CSCORE_MD_DEFAULT_COLQ_DOC_ID;

    row_bytes  = cbytes_make_by_bytes(kvGetrLenHs(kv)  , kvGetRowHs(kv)      );
    if(NULL_PTR == row_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_id_plain: new row cbytes failed\n");
        return (EC_FALSE);
    }
    
    colf_bytes = cbytes_make_by_bytes(kvGetcfLenHs(kv) , kvGetColFamilyHs(kv));
    if(NULL_PTR == colf_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_id_plain: new colf cbytes failed\n");
        cbytes_free(row_bytes, LOC_CSCORE_0061);
        return (EC_FALSE);
    }    
    colq_bytes = cbytes_make_by_bytes(colq_len         , colq_str            );
    if(NULL_PTR == colq_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_id_plain: new colq cbytes failed\n");
        cbytes_free(row_bytes, LOC_CSCORE_0062);
        cbytes_free(colf_bytes, LOC_CSCORE_0063);
        return (EC_FALSE);
    }    
   
    cvector_push_no_lock(cached_cbytes_vec, (void *)row_bytes );
    cvector_push_no_lock(cached_cbytes_vec, (void *)colf_bytes);
    cvector_push_no_lock(cached_cbytes_vec, (void *)colq_bytes);

    task_inc(task_mgr, ret, FI_cbgt_fetch, ERR_MODULE_ID, 
            CSCORE_MD_TABLE_NAME(cscore_md), 
            row_bytes, colf_bytes, colq_bytes,  
            val_bytes);

    return (EC_TRUE);
}

static EC_BOOL __cscore_csdoc_export_doc_type_plain(const UINT32 cscore_md_id, const CBYTES *kv_bytes,
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *doc_type_cbytes_vec,
                                                                CVECTOR *cached_cbytes_vec)
{
    CSCORE_MD *cscore_md;
    
    const uint8_t *kv;
    const uint8_t *colq_str;
    uint16_t       colq_len;

    CBYTES *row_bytes;
    CBYTES *colf_bytes;
    CBYTES *colq_bytes;

    UINT32 *ret;
    CBYTES *val_bytes;

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    ret = __cscore_new_uint32(cscore_md_id);
    if(NULL_PTR == ret)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_type_plain: new ret uint32 failed\n");
        return (EC_FALSE);
    }
    cvector_push_no_lock(ret_vec, (void *)ret);    

    val_bytes = cbytes_new(0);
    if(NULL_PTR == val_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_type_plain: new val cbytes failed\n");
        return (EC_FALSE);
    }
    cvector_push_no_lock(doc_type_cbytes_vec, (void *)val_bytes);

    kv = cbytes_buf(kv_bytes);

    colq_len = CSCORE_MD_DEFAULT_COLQ_DOC_TYPE_LEN;
    colq_str = (UINT8 *)CSCORE_MD_DEFAULT_COLQ_DOC_TYPE;

    row_bytes  = cbytes_make_by_bytes(kvGetrLenHs(kv)  , kvGetRowHs(kv)      );
    if(NULL_PTR == row_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_type_plain: new row cbytes failed\n");
        return (EC_FALSE);
    }
    
    colf_bytes = cbytes_make_by_bytes(kvGetcfLenHs(kv) , kvGetColFamilyHs(kv));
    if(NULL_PTR == colf_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_type_plain: new colf cbytes failed\n");
        cbytes_free(row_bytes, LOC_CSCORE_0064);
        return (EC_FALSE);
    }    
    colq_bytes = cbytes_make_by_bytes(colq_len         , colq_str            );
    if(NULL_PTR == colq_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_type_plain: new colq cbytes failed\n");
        cbytes_free(row_bytes, LOC_CSCORE_0065);
        cbytes_free(colf_bytes, LOC_CSCORE_0066);
        return (EC_FALSE);
    }    

    cvector_push_no_lock(cached_cbytes_vec, (void *)row_bytes );
    cvector_push_no_lock(cached_cbytes_vec, (void *)colf_bytes);
    cvector_push_no_lock(cached_cbytes_vec, (void *)colq_bytes);

    task_inc(task_mgr, ret, FI_cbgt_fetch, ERR_MODULE_ID, 
            CSCORE_MD_TABLE_NAME(cscore_md), 
            row_bytes, colf_bytes, colq_bytes,  
            val_bytes);

    return (EC_TRUE);
}

static EC_BOOL __cscore_csdoc_export_doc_code_plain(const UINT32 cscore_md_id, const CBYTES *kv_bytes,
                                                                TASK_MGR *task_mgr,
                                                                CVECTOR *ret_vec, CVECTOR *doc_code_cbytes_vec,
                                                                CVECTOR *cached_cbytes_vec)
{
    CSCORE_MD *cscore_md;
    
    const uint8_t *kv;
    const uint8_t *colq_str;
    uint16_t       colq_len;

    CBYTES *row_bytes;
    CBYTES *colf_bytes;
    CBYTES *colq_bytes;

    UINT32 *ret;
    CBYTES *val_bytes;

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    ret = __cscore_new_uint32(cscore_md_id);
    if(NULL_PTR == ret)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_code_plain: new ret uint32 failed\n");
        return (EC_FALSE);
    }
    cvector_push_no_lock(ret_vec, (void *)ret);    

    val_bytes = cbytes_new(0);
    if(NULL_PTR == val_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_code_plain: new val cbytes failed\n");
        return (EC_FALSE);
    }
    cvector_push_no_lock(doc_code_cbytes_vec, (void *)val_bytes);

    kv = cbytes_buf(kv_bytes);

    colq_len = CSCORE_MD_DEFAULT_COLQ_DOC_CODE_LEN;
    colq_str = (UINT8 *)CSCORE_MD_DEFAULT_COLQ_DOC_CODE;

    row_bytes  = cbytes_make_by_bytes(kvGetrLenHs(kv)  , kvGetRowHs(kv)      );
    if(NULL_PTR == row_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_code_plain: new row cbytes failed\n");
        return (EC_FALSE);
    }
    
    colf_bytes = cbytes_make_by_bytes(kvGetcfLenHs(kv) , kvGetColFamilyHs(kv));
    if(NULL_PTR == colf_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_code_plain: new colf cbytes failed\n");
        cbytes_free(row_bytes, LOC_CSCORE_0067);
        return (EC_FALSE);
    }    
    colq_bytes = cbytes_make_by_bytes(colq_len         , colq_str            );
    if(NULL_PTR == colq_bytes)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_code_plain: new colq cbytes failed\n");
        cbytes_free(row_bytes, LOC_CSCORE_0068);
        cbytes_free(colf_bytes, LOC_CSCORE_0069);
        return (EC_FALSE);
    }    
   
    cvector_push_no_lock(cached_cbytes_vec, (void *)row_bytes );
    cvector_push_no_lock(cached_cbytes_vec, (void *)colf_bytes);
    cvector_push_no_lock(cached_cbytes_vec, (void *)colq_bytes);

    task_inc(task_mgr, ret, FI_cbgt_fetch, ERR_MODULE_ID, 
            CSCORE_MD_TABLE_NAME(cscore_md), 
            row_bytes, colf_bytes, colq_bytes,  
            val_bytes);

    return (EC_TRUE);
}

static EC_BOOL __cscore_csdoc_export_doc_plain(const UINT32 cscore_md_id, const CBYTES *kv_bytes,
                                                            TASK_MGR *task_mgr,
                                                            CVECTOR *ret_vec, 
                                                            CVECTOR *doc_id_cbytes_vec,
                                                            CVECTOR *doc_type_cbytes_vec,
                                                            CVECTOR *doc_code_cbytes_vec,
                                                            CVECTOR *cached_cbytes_vec
                                                            )
{
    CSCORE_MD *cscore_md;
    
    const uint8_t *kv;
    const uint8_t *colq_str;
    uint16_t       colq_len;

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    kv = cbytes_buf(kv_bytes);
    colq_len = kvGetcqLenHs(kv);
    colq_str = kvGetColQualifierHs(kv);

    if(!(colq_len == CSCORE_MD_DEFAULT_COLQ_WORD_TEXT_LEN && 0 == strncmp((char *)colq_str, CSCORE_MD_DEFAULT_COLQ_WORD_TEXT, colq_len)))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_plain: colq in kv is %.*s which not matched with %.*s\n",
                           colq_len, colq_str,
                           CSCORE_MD_DEFAULT_COLQ_WORD_TEXT_LEN, CSCORE_MD_DEFAULT_COLQ_WORD_TEXT);
        return (EC_FALSE);
    }    

    if(EC_FALSE == __cscore_csdoc_export_doc_id_plain(cscore_md_id, kv_bytes, task_mgr, ret_vec, doc_id_cbytes_vec, cached_cbytes_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_plain: export cdoc id failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == __cscore_csdoc_export_doc_type_plain(cscore_md_id, kv_bytes, task_mgr, ret_vec, doc_type_cbytes_vec, cached_cbytes_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_plain: export cdoc type failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == __cscore_csdoc_export_doc_code_plain(cscore_md_id, kv_bytes, task_mgr, ret_vec, doc_code_cbytes_vec, cached_cbytes_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_export_doc_plain: export cdoc code failed\n");
        return (EC_FALSE);
    }    

    return (EC_TRUE);
}

static EC_BOOL __cscore_csdoc_list_export_plain(const UINT32 cscore_md_id, const CVECTOR *kv_vec, CSWORD_DOCS *csword_docs)
{    
    CSCORE_MD *cscore_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    CVECTOR   ret_vec;   /*item is UINT32*/
    CVECTOR   cached_vec;/*item is vector which item is CBYTES*/
    CVECTOR   cbytes_vec;

    UINT32 pos;
    UINT32 idx;

    if(0 == cvector_size(kv_vec))
    {
        dbg_log(SEC_0051_CSCORE, 1)(LOGSTDOUT, "warn:__cscore_csdoc_list_export_plain: kv_vec is null, export nothing\n");
        return (EC_TRUE);
    }

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    cvector_init(&ret_vec   , 0, MM_UINT32 , CVECTOR_LOCK_ENABLE, LOC_CSCORE_0070);
    cvector_init(&cached_vec, 0, MM_CVECTOR, CVECTOR_LOCK_ENABLE, LOC_CSCORE_0071);
    cvector_init(&cbytes_vec, 0, MM_CBYTES , CVECTOR_LOCK_ENABLE, LOC_CSCORE_0072);

    for(idx = 0; idx < CSCORE_COLQ_END_IDX; idx ++)
    {
        CVECTOR *cached_cbytes_vec;

        cached_cbytes_vec = __cscore_new_cbytes_vec(cscore_md_id);
        if(NULL_PTR == cached_cbytes_vec)
        {
            dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_list_export_plain: new cbytes vector failed\n");
            
            cvector_clean_with_modi(&ret_vec   , cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32    , LOC_CSCORE_0073);
            cvector_clean_with_modi(&cached_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_cbytes_vec, LOC_CSCORE_0074);
            cvector_clean_with_location(&cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0075);
            return (EC_FALSE);
        }
        cvector_push_no_lock(&cached_vec, (void *)cached_cbytes_vec);
    }

    mod_mgr = mod_mgr_new(CSCORE_MD_CBGT_MD_ID(cscore_md), LOAD_BALANCING_OBJ);
    if(NULL_PTR == mod_mgr)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_list_export_plain: new mod_mgr failed\n");

        cvector_clean_with_modi(&ret_vec   , cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32    , LOC_CSCORE_0076);
        cvector_clean_with_modi(&cached_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_cbytes_vec, LOC_CSCORE_0077);
        cvector_clean_with_location(&cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0078);
   
        return (EC_FALSE);
    }
    
    mod_mgr_incl(CMPI_LOCAL_TCID, CMPI_LOCAL_COMM, CMPI_LOCAL_RANK, CSCORE_MD_CBGT_MD_ID(cscore_md), mod_mgr);
    dbg_log(SEC_0051_CSCORE, 0)(LOGCONSOLE, "[DEBUG] __cscore_csdoc_list_export_plain: mod_mgr is\n");
    mod_mgr_print(LOGCONSOLE, mod_mgr);

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    if(NULL_PTR == task_mgr)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_list_export_plain: new task_mgr failed\n");

        cvector_clean_with_modi(&ret_vec   , cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32    , LOC_CSCORE_0079);
        cvector_clean_with_modi(&cached_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_cbytes_vec, LOC_CSCORE_0080);
        cvector_clean_with_location(&cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0081);
        
        mod_mgr_free(mod_mgr);
        
        return (EC_FALSE);
    }

    for(pos = 0; pos < cvector_size(kv_vec); pos ++)
    {
        const CBYTES *kv_bytes;

        kv_bytes = (const CBYTES *)cvector_get_no_lock(kv_vec, pos);
        dbg_log(SEC_0051_CSCORE, 9)(LOGSTDOUT, "[DEBUG] [A]__cscore_csdocs_list_export_plain: %ld: ", pos);
        kvPrintHs(LOGSTDOUT, cbytes_buf(kv_bytes));
        
        if(EC_FALSE == __cscore_csdoc_export_doc_plain(cscore_md_id, kv_bytes, 
                                                        task_mgr,
                                                        &ret_vec, 
                                                        (CVECTOR *)cvector_get_no_lock(&cached_vec, CSCORE_COLQ_DOC_ID_IDX),
                                                        (CVECTOR *)cvector_get_no_lock(&cached_vec, CSCORE_COLQ_DOC_TYPE_IDX),
                                                        (CVECTOR *)cvector_get_no_lock(&cached_vec, CSCORE_COLQ_DOC_CODE_IDX),
                                                        &cbytes_vec
                                                        ))
        {
            dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdocs_list_export_plain: export %ld # cdoc failed\n", pos);

            cvector_clean_with_modi(&ret_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32, LOC_CSCORE_0082);
            cvector_clean_with_modi(&cached_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_cbytes_vec, LOC_CSCORE_0083);
            cvector_clean_with_location(&cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0084);

            mod_mgr_free(mod_mgr);
            task_mgr_free(task_mgr);
            
            return (EC_FALSE);
        }
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    mod_mgr_free(mod_mgr);

    /*check result*/
    if(EC_FALSE == cvector_check_all_is_true(&ret_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdocs_list_export_plain: export csword_docs failed\n");

        cvector_clean_with_modi(&ret_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32, LOC_CSCORE_0085);
        cvector_clean_with_modi(&cached_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_cbytes_vec, LOC_CSCORE_0086);
        cvector_clean_with_location(&cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0087);

        return (EC_FALSE);
    }

    //TODO: get doc id,type,code from fetched val bytes
    for(pos = 0; pos < cvector_size(kv_vec); pos ++)
    {
        CSDOC  *csdoc;
        CBYTES *doc_id_val_cbytes;
        CBYTES *doc_type_val_cbytes;
        CBYTES *doc_code_val_cbytes;
        
        const CBYTES *kv_bytes;

        kv_bytes = (const CBYTES *)cvector_get_no_lock(kv_vec, pos);

        dbg_log(SEC_0051_CSCORE, 9)(LOGSTDOUT, "[DEBUG] [B]__cscore_csdocs_list_export_plain: %ld: ", pos);
        kvPrintHs(LOGSTDOUT, cbytes_buf(kv_bytes));

        csdoc = cscore_csdoc_new(cscore_md_id);
        if(NULL_PTR == csdoc)
        {
            dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_list_export_plain: new csdoc failed\n");
            cvector_clean_with_modi(&ret_vec   , cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32    , LOC_CSCORE_0088);
            cvector_clean_with_modi(&cached_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_cbytes_vec, LOC_CSCORE_0089);
            cvector_clean_with_location(&cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0090);
            
            return (EC_FALSE);
        }

        doc_id_val_cbytes   = (CBYTES *)cvector_get_no_lock((CVECTOR *)cvector_get_no_lock(&cached_vec, CSCORE_COLQ_DOC_ID_IDX)  , pos);
        doc_type_val_cbytes = (CBYTES *)cvector_get_no_lock((CVECTOR *)cvector_get_no_lock(&cached_vec, CSCORE_COLQ_DOC_TYPE_IDX), pos);
        doc_code_val_cbytes = (CBYTES *)cvector_get_no_lock((CVECTOR *)cvector_get_no_lock(&cached_vec, CSCORE_COLQ_DOC_CODE_IDX), pos);

        CSDOC_ID(csdoc)   = c_chars_to_word((char *)cbytes_buf(doc_id_val_cbytes)  , cbytes_len(doc_id_val_cbytes)  );
        CSDOC_TYPE(csdoc) = c_chars_to_word((char *)cbytes_buf(doc_type_val_cbytes), cbytes_len(doc_type_val_cbytes));
        CSDOC_CODE(csdoc) = c_chars_to_word((char *)cbytes_buf(doc_code_val_cbytes), cbytes_len(doc_code_val_cbytes));
        
        clist_push_back(CSWORD_DOCS_LIST(csword_docs), (void *)csdoc);
    }    

    cvector_clean_with_modi(&ret_vec   , cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_uint32    , LOC_CSCORE_0091);
    cvector_clean_with_modi(&cached_vec, cscore_md_id, (CVECTOR_DATA_CLEAN)__cscore_free_cbytes_vec, LOC_CSCORE_0092);
    cvector_clean_with_location(&cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0093);
    
    return (EC_TRUE);
}

/*export word and its docs from bigtable*/
/*note: csword_docs is IO parameter*/
EC_BOOL cscore_csword_docs_export(const UINT32 cscore_md_id, const UINT32 cached_mode, CSWORD_DOCS *csword_docs)
{
    CSCORE_MD *cscore_md;

    CSTRING   *table_pattern;
    CSTRING   *row_pattern;
    CSTRING   *colf_pattern;
    CSTRING   *colq_pattern;
    CSTRING   *val_pattern;

    CVECTOR   *ret_kv_vec;

    CSWORD    *csword;

#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_docs_export: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    cscore_md = CSCORE_MD_GET(cscore_md_id);

    if(NULL_PTR == csword_docs)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: csword_docs is null\n");
        return (EC_FALSE);
    }

    csword = CSWORD_DOCS_WORD(csword_docs);
    if(0 == CBYTES_LEN(CSWORD_CONTENT(csword)))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: csword content is empty\n");
        return (EC_FALSE);
    }

    /*table pattern*/
    table_pattern = cstring_new((UINT8 *)CSCORE_MD_DEFAULT_TABLE_NAME, LOC_CSCORE_0094);
    if(NULL_PTR == table_pattern)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: new table pattern failed\n");
        return (EC_FALSE);
    }

    /*row pattern*/
    row_pattern = cstring_new((UINT8 *)".*", LOC_CSCORE_0095);
    if(NULL_PTR == row_pattern)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: new row pattern failed\n");
        cstring_free(table_pattern);
        return (EC_FALSE);
    }

    /*colum family pattern*/
    colf_pattern = cstring_new((UINT8 *)CSCORE_MD_DEFAULT_COLF_NAME, LOC_CSCORE_0096);
    if(NULL_PTR == colf_pattern)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: new colf pattern failed\n");
        cstring_free(table_pattern);
        cstring_free(row_pattern);
        return (EC_FALSE);
    }

    /*colum qualifier pattern*/
    colq_pattern = cstring_new((UINT8 *)CSCORE_MD_DEFAULT_COLQ_WORD_TEXT, LOC_CSCORE_0097);
    if(NULL_PTR == colq_pattern)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: new colq pattern failed\n");
        cstring_free(table_pattern);
        cstring_free(row_pattern);
        cstring_free(colf_pattern);
        return (EC_FALSE);
    }

    /*value pattern*/
    val_pattern = cstring_make_by_bytes(cbytes_len(CSWORD_CONTENT(csword)), cbytes_buf(CSWORD_CONTENT(csword)));
    if(NULL_PTR == val_pattern)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: new val pattern failed\n");

        cstring_free(table_pattern);
        cstring_free(row_pattern);
        cstring_free(colf_pattern);
        cstring_free(colq_pattern);
        return (EC_FALSE);
    }

    /*make ret kv vector ready*/
    ret_kv_vec = cvector_new(0, MM_CBYTES, LOC_CSCORE_0098);
    if(NULL_PTR == ret_kv_vec)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: new ret kv vec failed\n");

        cstring_free(table_pattern);
        cstring_free(row_pattern);
        cstring_free(colf_pattern);
        cstring_free(colq_pattern);
        cstring_free(val_pattern);
        return (EC_FALSE);
    }

    if(EC_FALSE == cbgt_select(CSCORE_MD_CBGT_MD_ID(cscore_md), cached_mode,
                                table_pattern, row_pattern, colf_pattern, colq_pattern, val_pattern,
                                ret_kv_vec))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: select failed\n");

        cstring_free(table_pattern);
        cstring_free(row_pattern);
        cstring_free(colf_pattern);
        cstring_free(colq_pattern);
        cstring_free(val_pattern);

        cvector_clean_with_location(ret_kv_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0099);
        cvector_free(ret_kv_vec, LOC_CSCORE_0100);
        return (EC_FALSE);
    }

    if(1)
    {
        UINT32 pos;
        
        for(pos = 0; pos < cvector_size(ret_kv_vec); pos ++)
        {
            CBYTES *kv_bytes;
            kv_bytes = (CBYTES *)cvector_get_no_lock(ret_kv_vec, pos);
            if(NULL_PTR == kv_bytes)
            {
                continue;
            }

            dbg_log(SEC_0051_CSCORE, 5)(LOGSTDOUT, "%ld: ", pos);
            kvPrintHs(LOGSTDOUT, cbytes_buf(kv_bytes));
        }    
    }

    /*export ret_kv_vec to csword_docs*/
    if(EC_FALSE == __cscore_csdoc_list_export_plain(cscore_md_id, ret_kv_vec, csword_docs))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_export: export csdoc list failed\n");

        cstring_free(table_pattern);
        cstring_free(row_pattern);
        cstring_free(colf_pattern);
        cstring_free(colq_pattern);
        cstring_free(val_pattern);

        cvector_clean_with_location(ret_kv_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0101);
        cvector_free(ret_kv_vec, LOC_CSCORE_0102);
        return (EC_FALSE);
    }

    cstring_free(table_pattern);
    cstring_free(row_pattern);
    cstring_free(colf_pattern);
    cstring_free(colq_pattern);
    cstring_free(val_pattern);

    cvector_clean_with_location(ret_kv_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_CSCORE_0103);
    cvector_free(ret_kv_vec, LOC_CSCORE_0104);

    return (EC_TRUE);
}

/*csword_docs_list item type is CSWORD_DOCS and csword_docs_list is IO parameter*/
EC_BOOL cscore_csword_docs_list_export(const UINT32 cscore_md_id, const UINT32 cached_mode, CLIST *csword_docs_list)
{
    EC_BOOL     ret;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_docs_list_export: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(EC_FALSE == clist_loop(csword_docs_list, (void *)&ret, CLIST_CHECKER_DEFAULT, 
                              (UINT32)3, (UINT32)2,
                              (UINT32)cscore_csword_docs_export, cscore_md_id, cached_mode, NULL_PTR))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_list_export:export docs of csword failed\n");
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

static EC_BOOL __cscore_csword_docs_list_make_one(const UINT32 cscore_md_id, CLIST *csword_docs_list, const CSWORD *csword)
{
    CSWORD_DOCS *csword_docs;
    
    csword_docs = cscore_csword_docs_new(cscore_md_id);
    if(NULL_PTR == csword_docs)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_docs_list_make_one: new csword_docs failed\n");
        return (EC_FALSE);
    }
    cscore_csword_docs_set_word(cscore_md_id, csword_docs, csword);
    clist_push_back(csword_docs_list, (void *)csword_docs);
    return (EC_TRUE);
}

EC_BOOL cscore_csword_docs_list_export_docs(const UINT32 cscore_md_id, const UINT32 cached_mode, const CLIST *csword_list, CLIST *csdoc_list)
{    
    CLIST *csword_docs_list;
    EC_BOOL ret;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_docs_list_export_docs: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    csword_docs_list = clist_new(MM_CSWORD_DOCS, LOC_CSCORE_0105);
    if(NULL_PTR == csword_docs_list)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_list_export_docs: new csword_docs list failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == clist_loop((CLIST *)csword_list, (void *)&ret, CLIST_CHECKER_DEFAULT, 
                                (UINT32)3, (UINT32)2, 
                                (UINT32)__cscore_csword_docs_list_make_one, cscore_md_id, csword_docs_list, NULL_PTR))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_list_export_docs: new csword_docs failed\n");
        clist_clean_with_modi(csword_docs_list, cscore_md_id, (CLIST_DATA_MODI_CLEANER)cscore_csword_docs_free);
        clist_free(csword_docs_list, LOC_CSCORE_0106);
        return (EC_FALSE);
    }

    dbg_log(SEC_0051_CSCORE, 0)(LOGCONSOLE, "[DEBUG] cscore_csword_docs_list_export_docs: csword_docs list is\n");
    clist_print(LOGCONSOLE, csword_docs_list, (CLIST_DATA_DATA_PRINT)cscore_csword_docs_print);

    if(EC_FALSE == cscore_csword_docs_list_export(cscore_md_id, cached_mode, csword_docs_list))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_list_export_docs: export csword_docs list failed\n");
        clist_clean_with_modi(csword_docs_list, cscore_md_id, (CLIST_DATA_MODI_CLEANER)cscore_csword_docs_free);
        clist_free(csword_docs_list, LOC_CSCORE_0107);        
        return (EC_FALSE);
    }

    cscore_csword_docs_list_merge(cscore_md_id, csword_docs_list, csdoc_list);

    clist_clean_with_modi(csword_docs_list, cscore_md_id, (CLIST_DATA_MODI_CLEANER)cscore_csword_docs_free);
    clist_free(csword_docs_list, LOC_CSCORE_0108);    
    return (EC_TRUE);
}

static EC_BOOL __cscore_csdoc_merge_one(const CSDOC *csdoc, CSDOC *cur_csdoc)
{
     if(CSDOC_ID(csdoc) == CSDOC_ID(cur_csdoc))
     {
        CSDOC_RATE(cur_csdoc) ++;
        return (EC_FALSE);/*terminate clist loop*/
     }
     return (EC_TRUE);
}

static EC_BOOL __cscore_csdoc_merge(const UINT32 cscore_md_id, const CSDOC *csdoc, CLIST *csdoc_list)
{
    CSDOC *new_csdoc;
    EC_BOOL ret;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__cscore_csdoc_merge: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    clist_loop(csdoc_list, (void *)&ret, CLIST_CHECKER_DEFAULT, 
               (UINT32)2, (UINT32)1, 
               (UINT32)__cscore_csdoc_merge_one, csdoc, NULL_PTR);

    new_csdoc = cscore_csdoc_new(cscore_md_id);
    if(NULL_PTR == new_csdoc)
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_merge: new csdoc failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == cscore_csdoc_clone(cscore_md_id, csdoc, new_csdoc))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csdoc_merge: clone csdoc failed\n");
        cscore_csdoc_free(cscore_md_id, new_csdoc);
        return (EC_FALSE);
    }

    CSDOC_RATE(new_csdoc) = 1;
    clist_push_back(csdoc_list, new_csdoc);

    return (EC_TRUE);
}

static EC_BOOL __cscore_csword_docs_merge(const UINT32 cscore_md_id, const CSWORD_DOCS *csword_docs, CLIST *csdoc_list)
{
    EC_BOOL ret;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:__cscore_csword_docs_merge: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(EC_FALSE == clist_loop((CLIST *)CSWORD_DOCS_LIST(csword_docs), &ret, CLIST_CHECKER_DEFAULT, 
                              (UINT32)3, (UINT32)1, 
                              (UINT32)__cscore_csdoc_merge, cscore_md_id, NULL_PTR, csdoc_list))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:__cscore_csword_docs_merge: merge csdoc to csdoc list failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/*csword_docs_list item type is CSWORD_DOCS*/
/*docs_list item type is CSDOC which is OUT parameter*/
/*NOTE: here is the most natvie implementation*/
EC_BOOL cscore_csword_docs_list_merge(const UINT32 cscore_md_id, const CLIST *csword_docs_list, CLIST *csdoc_list)
{
    EC_BOOL ret;
    
#if ( SWITCH_ON == CSCORE_DEBUG_SWITCH )
    if ( CSCORE_MD_ID_CHECK_INVALID(cscore_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cscore_csword_docs_list_merge: cscore module #0x%lx not started.\n",
                cscore_md_id);
        cscore_print_module_status(cscore_md_id, LOGSTDOUT);
        dbg_exit(MD_CSCORE, cscore_md_id);
    }
#endif/*CSCORE_DEBUG_SWITCH*/

    if(EC_FALSE == clist_loop((CLIST *)csword_docs_list, (void *)&ret, CLIST_CHECKER_DEFAULT, 
                               (UINT32)3, (UINT32)1, 
                               (UINT32)__cscore_csword_docs_merge, cscore_md_id, NULL_PTR, csdoc_list))
    {
        dbg_log(SEC_0051_CSCORE, 0)(LOGSTDOUT, "error:cscore_csword_docs_list_merge: merge csword_csdoc_list to docs list failed\n");
        return (EC_FALSE);
     }

    /*sort csdoc from high rate to low rate*/
    clist_bubble_sort_with_modi(csdoc_list, cscore_md_id, (CLIST_DATA_MODI_CMP)cscore_csdoc_rate_gt);

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

