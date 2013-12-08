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

#include "bgnctrl.h"
#include "type.h"
#include "moduleconst.h"
#include "mm.h"
#include "log.h"

#include "real.h"

#include "vector.h"
#include "vectorr.h"

#include "matrix.h"
#include "matrixr.h"
#include "dmatrixr.h"

#include "cmpic.inc"

#include "debug.h"

#include "print.h"

#include "mod.h"

#include "task.h"

#include "findex.inc"

#include "cbc.h"

#include "croutine.h"

#define DMATRIXR_MD_CAPACITY()                  (cbc_md_capacity(MD_DMATRIXR))

#define DMATRIXR_MD_GET(dmatrixr_md_id)     ((DMATRIXR_MD *)cbc_md_get(MD_DMATRIXR, (dmatrixr_md_id)))

#define DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id)  \
    ( NULL_PTR == DMATRIXR_MD_GET(dmatrixr_md_id) || 0 == (DMATRIXR_MD_GET(dmatrixr_md_id)->usedcounter) )

/**
*   for test only
*
*   to query the status of DMATRIXR Module
*
**/
void dmatrix_r_print_module_status(const UINT32 dmatrixr_md_id, LOG *log)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 this_dmatrixr_md_id;

    for( this_dmatrixr_md_id = 0; this_dmatrixr_md_id < DMATRIXR_MD_CAPACITY(); this_dmatrixr_md_id ++ )
    {
        dmatrixr_md = DMATRIXR_MD_GET(this_dmatrixr_md_id);

        if ( NULL_PTR != dmatrixr_md && 0 < dmatrixr_md->usedcounter )
        {
            sys_log(log,"DMATRIXR Module # %ld : %ld refered, refer REAL Module : %ld, refer MATRIXR Module : %ld\n",
                    this_dmatrixr_md_id,
                    dmatrixr_md->usedcounter,
                    dmatrixr_md->real_md_id,
                    dmatrixr_md->matrixr_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed DMATRIXR module
*
*
**/
UINT32 dmatrix_r_free_module_static_mem(const UINT32 dmatrixr_md_id)
{
    DMATRIXR_MD  *dmatrixr_md;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_free_module_static_mem: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);

    free_module_static_mem(MD_DMATRIXR, dmatrixr_md_id);

    return 0;
}

/**
*
* start DMATRIXR module
*
**/
UINT32 dmatrix_r_start( )
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 dmatrixr_md_id;

    UINT32 real_md_id;
    UINT32 matrixr_md_id;

    dmatrixr_md_id = cbc_md_new(MD_DMATRIXR, sizeof(DMATRIXR_MD));
    if(ERR_MODULE_ID == dmatrixr_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one DMATRIXR module */
    dmatrixr_md = (DMATRIXR_MD *)cbc_md_get(MD_DMATRIXR, dmatrixr_md_id);
    dmatrixr_md->usedcounter   = 0;
    dmatrixr_md->real_md_id    = ERR_MODULE_ID;
    dmatrixr_md->matrixr_md_id = ERR_MODULE_ID;

    /* create a new module node */
    real_md_id = ERR_MODULE_ID;
    matrixr_md_id = ERR_MODULE_ID;

    init_static_mem();

    /*default setting which will be override after dmatrix_r_set_mod_mgr calling*/
    dmatrixr_md->mod_mgr = mod_mgr_new(dmatrixr_md_id, LOAD_BALANCING_LOOP);

    real_md_id = real_start(); /*no REAL_MD need to alloc, here is one trick at present*/
    matrixr_md_id = matrix_r_start();

    matrix_r_new_matrix(matrixr_md_id, 0, 0, &(dmatrixr_md->src_matrixr));
    matrix_r_new_matrix(matrixr_md_id, 0, 0, &(dmatrixr_md->matrixr_of_mul));
    matrix_r_new_block(matrixr_md_id, &(dmatrixr_md->matrixr_row_block_of_adc));

    dmatrixr_md->real_md_id = real_md_id;
    dmatrixr_md->matrixr_md_id = matrixr_md_id;
    dmatrixr_md->usedcounter = 1;

    croutine_mutex_init(&(dmatrixr_md->cmutex), CMUTEX_PROCESS_PRIVATE, LOC_DMATRIXR_0001);

    croutine_cond_init(&(dmatrixr_md->ccond), LOC_DMATRIXR_0002);
    return ( dmatrixr_md_id );
}

/**
*
* end DMATRIXR module
*
**/
void dmatrix_r_end(const UINT32 dmatrixr_md_id)
{
    DMATRIXR_MD *dmatrixr_md;

    UINT32 real_md_id;
    UINT32 matrixr_md_id;

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    if(NULL_PTR == dmatrixr_md)
    {
        sys_log(LOGSTDOUT,"error:dmatrix_r_end: dmatrixr_md_id = %ld not exist.\n", dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < dmatrixr_md->usedcounter )
    {
        dmatrixr_md->usedcounter --;
        return ;
    }

    if ( 0 == dmatrixr_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:dmatrix_r_end: dmatrixr_md_id = %ld is not started.\n", dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }

    //task_brd_mod_mgr_rmv(dmatrixr_md->task_brd, dmatrixr_md->mod_mgr);
    mod_mgr_free(dmatrixr_md->mod_mgr);
    dmatrixr_md->mod_mgr  = NULL_PTR;

    /* if nobody else occupied the module,then free its resource */
    real_md_id = dmatrixr_md->real_md_id;
    matrixr_md_id = dmatrixr_md->matrixr_md_id;

    matrix_r_destroy_matrix(matrixr_md_id, dmatrixr_md->src_matrixr);
    matrix_r_destroy_matrix(matrixr_md_id, dmatrixr_md->matrixr_of_mul);
    matrix_r_destroy_block(matrixr_md_id, dmatrixr_md->matrixr_row_block_of_adc);

    real_end(real_md_id);
    matrix_r_end(matrixr_md_id);

    dmatrixr_md->real_md_id = ERR_MODULE_ID;
    dmatrixr_md->matrixr_md_id = ERR_MODULE_ID;
    dmatrixr_md->usedcounter = 0;

    croutine_mutex_clean(&(dmatrixr_md->cmutex), LOC_DMATRIXR_0003);
    croutine_cond_clean(&(dmatrixr_md->ccond), LOC_DMATRIXR_0004);

    cbc_md_free(MD_DMATRIXR, dmatrixr_md_id);

    breathing_static_mem();
    return ;
}


/**
*
* initialize mod mgr of DMATRIXR module
*
**/
UINT32 dmatrix_r_set_mod_mgr(const UINT32 dmatrixr_md_id, const MOD_MGR * src_mod_mgr)
{
    DMATRIXR_MD *dmatrixr_md;
    MOD_MGR * des_mod_mgr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_set_mod_mgr: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dmatrix_r_print_module_status(dmatrixr_md_id, LOGSTDOUT);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    des_mod_mgr = dmatrixr_md->mod_mgr;

    //sys_log(LOGSTDOUT, "dmatrix_r_set_mod_mgr: md_id %d, src_mod_mgr %lx\n", dmatrixr_md_id, src_mod_mgr);
    //mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    mod_mgr_limited_clone(dmatrixr_md_id, src_mod_mgr, des_mod_mgr);

    //sys_log(LOGSTDOUT, "dmatrix_r_set_mod_mgr: md_id %d, des_mod_mgr %lx\n", dmatrixr_md_id, des_mod_mgr);
    //mod_mgr_print(LOGSTDOUT, des_mod_mgr);

    //sys_log(LOGSTDOUT, "====================================dmatrix_r_set_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    //mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    //sys_log(LOGSTDOUT, "====================================dmatrix_r_set_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);


    return (0);
}

/**
*
* get mod mgr of MATRIXR module
*
**/
MOD_MGR * dmatrix_r_get_mod_mgr(const UINT32 dmatrixr_md_id)
{
    DMATRIXR_MD *dmatrixr_md;

    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        return (MOD_MGR *)0;
    }

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    return (dmatrixr_md->mod_mgr);
}

UINT32 dmatrix_r_get_matrixr_md_id(const UINT32 dmatrixr_md_id, UINT32 * matrixr_md_id)
{
    DMATRIXR_MD *dmatrixr_md;
#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_append_row_block: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    *matrixr_md_id = dmatrixr_md->matrixr_md_id;
    return (0);
}

MATRIX_BLOCK *dmatrix_r_get_row_block(const UINT32 dmatrixr_md_id, const MATRIX *matrixr, const UINT32 pos)
{
    if(pos >= MATRIX_GET_COL_BLOCKS_NUM(matrixr))
    {
        sys_log(LOGSTDOUT, "error:dmatrix_r_get_row_block: pos %ld overflow where col blocks num of matrix is %ld\n", pos, MATRIX_GET_COL_BLOCKS_NUM(matrixr));
        return (NULL_PTR);
    }

    return MATRIX_GET_BLOCK(matrixr, 0, pos);
}

UINT32 dmatrix_r_clean_adc_cache(const UINT32 dmatrixr_md_id, const UINT32 row_num_of_src_matrix_row_block)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 matrixr_md_id;

    MATRIX_BLOCK *local_matrix_row_block;
    UINT32 col_num_of_local_matrix_row_block;

    MOD_MGR *mod_mgr;

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;
    mod_mgr = dmatrixr_md->mod_mgr;

    //sys_log(LOGSTDOUT, "dmatrix_r_clean_adc_cache: %ld clean up\n", MOD_MGR_LOCAL_MOD_POS(mod_mgr));

    local_matrix_row_block = dmatrix_r_get_row_block(dmatrixr_md_id, dmatrixr_md->src_matrixr, MOD_MGR_LOCAL_MOD_POS(mod_mgr));
    matrix_r_block_get_col_num(matrixr_md_id, local_matrix_row_block, &col_num_of_local_matrix_row_block);

    matrix_r_clean_block(matrixr_md_id, dmatrixr_md->matrixr_row_block_of_adc);
    MATRIX_BLOCK_SET_ROTATED_FLAG(dmatrixr_md->matrixr_row_block_of_adc, 0);
    matrix_r_block_set_type(matrixr_md_id, row_num_of_src_matrix_row_block, col_num_of_local_matrix_row_block, dmatrixr_md->matrixr_row_block_of_adc);

    return (0);
}

UINT32 dmatrix_r_clean_mul_cache(const UINT32 dmatrixr_md_id, const UINT32 row_num_of_src_matrix_row_block)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 matrixr_md_id;

    UINT32 col_num_of_src_matrixr;

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;

    matrix_r_get_col_num(matrixr_md_id, dmatrixr_md->src_matrixr, &col_num_of_src_matrixr);

    /*refresh mul matrix to zero and align type*/
    matrix_r_clean_matrix(matrixr_md_id, dmatrixr_md->matrixr_of_mul);
    matrix_r_new_matrix_skeleton(matrixr_md_id, row_num_of_src_matrix_row_block, col_num_of_src_matrixr, dmatrixr_md->matrixr_of_mul);
    return (0);
}

UINT32 dmatrix_r_clean_cache(const UINT32 dmatrixr_md_id, const UINT32 row_num_of_src_matrix_row_block, const UINT32 col_blocks_num)
{
    DMATRIXR_MD *dmatrixr_md;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_clean_cache: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);

    dmatrix_r_clean_adc_cache(dmatrixr_md_id, row_num_of_src_matrix_row_block);
    dmatrix_r_clean_mul_cache(dmatrixr_md_id, row_num_of_src_matrix_row_block);

    //sys_log(LOGSTDOUT, "dmatrix_r_clean_cache: dmatrixr_md_id = %ld, spy on ccond times = %ld\n", dmatrixr_md_id, croutine_cond_spy(&(dmatrixr_md->ccond), LOC_DMATRIXR_0005);
    croutine_cond_reserve(&(dmatrixr_md->ccond), col_blocks_num, LOC_DMATRIXR_0006);
    return (0);
}

UINT32 dmatrix_r_append_row_block(const UINT32 dmatrixr_md_id, const MATRIX_BLOCK *matrixr_row_block)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 matrixr_md_id;

    MATRIX *matrixr;
    MATRIX_BLOCK *matrixr_row_block_t;

    UINT32 row_num;
    UINT32 col_num;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    if( NULL_PTR == matrixr_row_block )
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_append_row_block: matrixr_row_block is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_append_row_block: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;
    matrixr = dmatrixr_md->src_matrixr;

    row_num = MATRIX_GET_ROW_NUM(matrixr);
    col_num = MATRIX_GET_COL_NUM(matrixr);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(matrixr_row_block);
    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(matrixr_row_block);

    /*when matrix is empty, have to alloc row header and col header. otherwise, alloc row header is enough*/
    if(0 == row_num && 0 == col_num)
    {
        /*clone matrixr_block and do not change anything of inputed matrixr_block*/
        matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &matrixr_row_block_t);
        matrix_r_clone_block(matrixr_md_id, matrixr_row_block, matrixr_row_block_t);

        cvector_push((CVECTOR *)MATRIX_GET_BLOCKS(matrixr), matrixr_row_block_t);

        MATRIX_SET_ROW_NUM(matrixr, sub_row_num);
        MATRIX_SET_COL_NUM(matrixr, sub_col_num);

    }
    else
    {
        /*validity checking*/
        if(sub_col_num != MATRIX_GET_COL_NUM(matrixr))
        {
            sys_log(LOGSTDERR, "error:dmatrix_r_append_row_block: col num mismatched: matrixr_block (%ld, %ld), matrixr (%ld, %ld)\n",
                            sub_row_num, sub_col_num, row_num, col_num);
            return ((UINT32)(-1));
        }

        /*clone matrixr_block and do not change anything of inputed matrixr_block*/
        matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &matrixr_row_block_t);
        matrix_r_clone_block(matrixr_md_id, matrixr_row_block, matrixr_row_block_t);

        cvector_push((CVECTOR *)MATRIX_GET_BLOCKS(matrixr), matrixr_row_block_t);

        MATRIX_SET_ROW_NUM(matrixr, row_num + sub_row_num);
    }

    return (0);
}

UINT32 dmatrix_r_append_col_block(const UINT32 dmatrixr_md_id, const MATRIX_BLOCK *matrixr_col_block)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 matrixr_md_id;

    MATRIX *matrixr;
    MATRIX_BLOCK *matrixr_col_block_t;

    UINT32 row_num;
    UINT32 col_num;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    if( NULL_PTR == matrixr_col_block )
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_append_col_block: matrixr_col_block is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_append_col_block: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;
    matrixr = dmatrixr_md->src_matrixr;

    row_num = MATRIX_GET_ROW_NUM(matrixr);
    col_num = MATRIX_GET_COL_NUM(matrixr);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(matrixr_col_block);
    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(matrixr_col_block);

    /*when matrix is empty, have to alloc row header and col header. otherwise, alloc col header is enough*/
    if(0 == row_num && 0 == col_num)
    {
        /*clone matrixr_block and do not change anything of inputed matrixr_block*/
        matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &matrixr_col_block_t);
        matrix_r_clone_block(matrixr_md_id, matrixr_col_block, matrixr_col_block_t);

        cvector_push((CVECTOR *)MATRIX_GET_BLOCKS(matrixr), matrixr_col_block_t);

        MATRIX_SET_ROW_NUM(matrixr, sub_row_num);
        MATRIX_SET_COL_NUM(matrixr, sub_col_num);

    }
    else
    {
        /*validity checking*/
        if(sub_row_num != MATRIX_GET_ROW_NUM(matrixr))
        {
            sys_log(LOGSTDERR, "error:dmatrix_r_append_col_block: row num mismatched: matrixr_block (%ld, %ld), matrixr (%ld, %ld)\n",
                            sub_row_num, sub_col_num, row_num, col_num);
            return ((UINT32)(-1));
        }

        /*clone matrixr_block and do not change anything of inputed matrixr_block*/
        matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &matrixr_col_block_t);
        matrix_r_clone_block(matrixr_md_id, matrixr_col_block, matrixr_col_block_t);

        cvector_push((CVECTOR *)MATRIX_GET_BLOCKS(matrixr), matrixr_col_block_t);

        MATRIX_SET_COL_NUM(matrixr, col_num + sub_col_num);
    }

    return (0);
}

UINT32 dmatrix_r_deliver_row_blocks(const UINT32 dmatrixr_md_id, const MATRIX *matrixr)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 matrixr_md_id;

    MATRIX_BLOCK *matrixr_row_block;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    if( NULL_PTR == matrixr )
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_deliver_row_blocks: matrixr is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_deliver_row_blocks: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;

    MATRIX_ROW_BLOCKS_LOOP_NEXT(matrixr, block_row_idx)
    {
        MATRIX_COL_BLOCKS_LOOP_NEXT(matrixr, block_col_idx)
        {
            matrixr_row_block = MATRIX_GET_BLOCK(matrixr, block_row_idx, block_col_idx);
            dmatrix_r_append_col_block(dmatrixr_md_id, matrixr_row_block);
        }
        matrix_r_clean_matrix(matrixr_md_id, dmatrixr_md->src_matrixr);
    }
    return (0);
}

UINT32 dmatrix_r_deliver_col_blocks(const UINT32 dmatrixr_md_id, const MATRIX *matrixr)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 matrixr_md_id;

    MATRIX_BLOCK *matrixr_col_block;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    if( NULL_PTR == matrixr )
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_deliver_col_blocks: matrixr is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_deliver_col_blocks: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;

    MATRIX_COL_BLOCKS_LOOP_NEXT(matrixr, block_col_idx)
    {
        MATRIX_ROW_BLOCKS_LOOP_NEXT(matrixr, block_row_idx)
        {
            matrixr_col_block = MATRIX_GET_BLOCK(matrixr, block_row_idx, block_col_idx);
            dmatrix_r_append_row_block(dmatrixr_md_id, matrixr_col_block);
        }
        matrix_r_clean_matrix(matrixr_md_id, dmatrixr_md->src_matrixr);
    }
    return (0);
}

UINT32 dmatrix_r_deliver_rows_p(const UINT32 dmatrixr_md_id, const MATRIX *matrixr)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 matrixr_md_id;

    MATRIX_BLOCK *matrixr_col_block;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MOD_MGR *mod_mgr;
    UINT32 remote_mod_node_num;
    UINT32 recv_mod_node_pos;

    if( NULL_PTR == matrixr )
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_deliver_rows_p: matrixr is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_deliver_rows_p: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;
    mod_mgr = dmatrixr_md->mod_mgr;

    remote_mod_node_num = mod_mgr_remote_mod_node_num(mod_mgr);
    if(0 == remote_mod_node_num)
    {
        sys_log(LOGSTDERR, "error:dmatrix_r_deliver_rows_p: remote mod node num of mod_mgr %lx is 0!\n", mod_mgr);
        return ((UINT32)(-1));
    }

    MATRIX_COL_BLOCKS_LOOP_NEXT(matrixr, block_col_idx)
    {
        UINT32 ret;
        TASK_MGR *task_mgr;

        task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        mod_mgr_first_remote(mod_mgr, &recv_mod_node_pos);

        MATRIX_ROW_BLOCKS_LOOP_NEXT(matrixr, block_row_idx)
        {
            matrixr_col_block = MATRIX_GET_BLOCK(matrixr, block_row_idx, block_col_idx);
            task_pos_inc(task_mgr, recv_mod_node_pos, &ret, FI_dmatrix_r_append_col_block, ERR_MODULE_ID, matrixr_col_block);
            mod_mgr_next_remote(mod_mgr, &recv_mod_node_pos);
        }

        task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
    }
    return (0);
}

UINT32 dmatrix_r_deliver_cols_p(const UINT32 dmatrixr_md_id, const MATRIX *matrixr)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 matrixr_md_id;

    MATRIX_BLOCK *matrixr_row_block;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MOD_MGR *mod_mgr;
    UINT32 remote_mod_node_num;
    UINT32 recv_mod_node_pos;

    if( NULL_PTR == matrixr )
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_deliver_cols_p: matrixr is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_deliver_cols_p: matrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;
    mod_mgr = dmatrixr_md->mod_mgr;

    remote_mod_node_num = mod_mgr_remote_mod_node_num(mod_mgr);
    if(0 == remote_mod_node_num)
    {
        sys_log(LOGSTDERR, "error:dmatrix_r_deliver_cols_p: remote mod node num of mod_mgr %lx is 0!\n", mod_mgr);
        return ((UINT32)(-1));
    }

    MATRIX_ROW_BLOCKS_LOOP_NEXT(matrixr, block_row_idx)
    {
        UINT32 ret;
        TASK_MGR *task_mgr;

        task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

        mod_mgr_first_remote(mod_mgr, &recv_mod_node_pos);
        MATRIX_COL_BLOCKS_LOOP_NEXT(matrixr, block_col_idx)
        {
            matrixr_row_block = MATRIX_GET_BLOCK(matrixr, block_row_idx, block_col_idx);
            task_pos_inc(task_mgr, recv_mod_node_pos, &ret, FI_dmatrix_r_append_row_block, ERR_MODULE_ID, matrixr_row_block);
            mod_mgr_next_remote(mod_mgr, &recv_mod_node_pos);
        }

        task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
    }
    return (0);
}

UINT32 dmatrix_r_clone_row_block(const UINT32 dmatrixr_md_id, MATRIX_BLOCK *des_matrix_block)
{
    /*locate the adc matrix block and clone it to des matrix block*/
    DMATRIXR_MD *dmatrixr_md;

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);

    //sys_log(LOGSTDOUT, "dmatrix_r_clone_row_block: dmatrixr_md_id = %ld, spy on ccond times = %ld\n", dmatrixr_md_id, croutine_cond_spy(&(dmatrixr_md->ccond), LOC_DMATRIXR_0007);

    matrix_r_clone_block(dmatrixr_md->matrixr_md_id, dmatrixr_md->matrixr_row_block_of_adc, des_matrix_block);

    return (0);
}

UINT32 dmatrix_r_adc_row_block(const UINT32 dmatrixr_md_id, const MATRIX_BLOCK *src_matrix_row_block)
{
    /*locate the src matrix block and adc with mul matrix block*/
    DMATRIXR_MD *dmatrixr_md;

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);

    /*adc buff is boundary resource, hence lock it when adc*/
    croutine_mutex_lock(&(dmatrixr_md->cmutex), LOC_DMATRIXR_0008);

#if 0
    sys_log(LOGSTDOUT, "dmatrix_r_adc_row_block: type checking: (%ld, %ld), (%ld, %ld)\n",
                    MATRIX_GET_ROW_NUM(MATRIX_GET_ROTATED_FLAG(src_matrix_row_block), src_matrix_row_block),
                    MATRIX_GET_COL_NUM(MATRIX_GET_ROTATED_FLAG(src_matrix_row_block), src_matrix_row_block),
                    MATRIX_GET_ROW_NUM(MATRIX_GET_ROTATED_FLAG(dmatrixr_md->matrixr_row_block_of_adc), dmatrixr_md->matrixr_row_block_of_adc),
                    MATRIX_GET_COL_NUM(MATRIX_GET_ROTATED_FLAG(dmatrixr_md->matrixr_row_block_of_adc), dmatrixr_md->matrixr_row_block_of_adc));
#endif

    /*warn: matrix_r_block_adc is internal function, it does not check matrix block type validity!*/
    matrix_r_block_adc(dmatrixr_md->matrixr_md_id, src_matrix_row_block, dmatrixr_md->matrixr_row_block_of_adc);

    croutine_cond_release(&(dmatrixr_md->ccond), LOC_DMATRIXR_0009);
    //sys_log(LOGSTDOUT, "dmatrix_r_adc_row_block: dmatrixr_md_id = %ld, spy on ccond times = %ld\n", dmatrixr_md_id, croutine_cond_spy(&(dmatrixr_md->ccond), LOC_DMATRIXR_0010);

    croutine_mutex_unlock(&(dmatrixr_md->cmutex), LOC_DMATRIXR_0011);
    return (0);
}

UINT32 dmatrix_r_mul_row_block(const UINT32 dmatrixr_md_id, const MATRIX_BLOCK *src_matrix_row_block, MATRIX_BLOCK *ret_matrix_row_block)
{
    DMATRIXR_MD *dmatrixr_md;
    UINT32 matrixr_md_id;


    MOD_MGR *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT32 local_mod_node_pos;
    UINT32 recv_mod_node_pos;
    UINT32 ret;

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;
    mod_mgr = dmatrixr_md->mod_mgr;

    //sys_log(LOGSTDOUT, "====================================dmatrix_r_mul_row_block: mod_mgr %lx beg====================================\n", mod_mgr);
    //mod_mgr_print(LOGSTDOUT, mod_mgr);
    //sys_log(LOGSTDOUT, "====================================dmatrix_r_mul_row_block: mod_mgr %lx end====================================\n", mod_mgr);

    local_mod_node_pos = MOD_MGR_LOCAL_MOD_POS(mod_mgr);

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    /*loop remote mod_node from pos 0 and skip local mod_node, so that we will no leave hole due to function mod_mgr_xxx_remote_mod_node_pos*/
    /*assume no repeat remote mod node*/
    mod_mgr_first_remote(mod_mgr, &recv_mod_node_pos);

    MATRIX_ROW_COL_BLOCKS_DOUBLE_LOOP_NEXT(dmatrixr_md->src_matrixr, dmatrixr_md->matrixr_of_mul, block_row_idx, block_col_idx)
    {
        MATRIX_BLOCK *matrix_row_block_of_src;
        MATRIX_BLOCK *matrix_row_block_of_mul;

        matrix_row_block_of_src = MATRIX_GET_BLOCK(dmatrixr_md->src_matrixr   , block_row_idx, block_col_idx);
        matrix_row_block_of_mul = MATRIX_GET_BLOCK(dmatrixr_md->matrixr_of_mul, block_row_idx, block_col_idx);

        matrix_r_block_mul(matrixr_md_id, src_matrix_row_block, matrix_row_block_of_src, matrix_row_block_of_mul);

        if(recv_mod_node_pos == local_mod_node_pos)
        {
            dmatrix_r_adc_row_block(dmatrixr_md_id, matrix_row_block_of_mul);
        }
        else
        {
            task_pos_inc(task_mgr, recv_mod_node_pos, &ret, FI_dmatrix_r_adc_row_block, ERR_MODULE_ID, matrix_row_block_of_mul);
        }

        mod_mgr_next_remote(mod_mgr, &recv_mod_node_pos);
    }

    /*when task_wait come back, all mul result blocks adc to remote*/
    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    //sys_log(LOGSTDOUT, "dmatrix_r_mul_row_block: dmatrixr_md_id = %ld, spy on ccond times = %ld\n", dmatrixr_md_id, croutine_cond_spy(&(dmatrixr_md->ccond), LOC_DMATRIXR_0012);

    /*when croutine_cond_wait come back, all remote result blocks adc to local*/
    croutine_cond_wait(&(dmatrixr_md->ccond), LOC_DMATRIXR_0013);

    dmatrix_r_clone_row_block(dmatrixr_md_id, ret_matrix_row_block);

    return (0);
}

/*called by master!*/
UINT32 dmatrix_r_mul_p(const UINT32 dmatrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix)
{
    DMATRIXR_MD  *dmatrixr_md;
    UINT32 matrixr_md_id;

    MOD_MGR *mod_mgr;

    MATRIX *tmp_matrix;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT32 row_num;
    UINT32 col_num;

    UINT32 col_blocks_num_of_src_matrix_2;

    if( NULL_PTR == src_matrix_1 )
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_mul_p: src_matrix_1 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_matrix_2 )
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_mul_p: src_matrix_2 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix)
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_mul_p: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( DMATRIXR_MD_ID_CHECK_INVALID(dmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:dmatrix_r_mul_p: dmatrixr module #0x%lx not started.\n",
                dmatrixr_md_id);
        dbg_exit(MD_DMATRIXR, dmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    dmatrixr_md = DMATRIXR_MD_GET(dmatrixr_md_id);
    matrixr_md_id = dmatrixr_md->matrixr_md_id;
    mod_mgr = dmatrixr_md->mod_mgr;

    if(  MATRIX_GET_COL_NUM(src_matrix_1)
      != MATRIX_GET_ROW_NUM(src_matrix_2) )
    {
        sys_log(LOGSTDERR,"error:dmatrix_r_mul_p: not matchable matrix: col num of src_matrix_1 = %ld, row num of src_matrix_2 = %ld\n",
                        MATRIX_GET_COL_NUM(src_matrix_1),
                        MATRIX_GET_ROW_NUM(src_matrix_2));
        return ((UINT32)(-1));
    }

    dmatrix_r_deliver_rows_p(dmatrixr_md_id, src_matrix_2);
    matrix_r_get_col_blocks_num(matrixr_md_id, src_matrix_2, &col_blocks_num_of_src_matrix_2);

    row_num = MATRIX_GET_ROW_NUM(src_matrix_1);
    col_num = MATRIX_GET_COL_NUM(src_matrix_2);

    //sys_log(LOGSTDOUT, "dmatrix_r_mul_p:matrix_r_new_matrix beg ============================================================\n");
    matrix_r_new_matrix(matrixr_md_id, row_num, col_num, &tmp_matrix);
    //sys_log(LOGSTDOUT, "dmatrix_r_mul_p:matrix_r_new_matrix end ============================================================\n");

    MATRIX_ROW_BLOCKS_DOUBLE_LOOP_NEXT(src_matrix_1, tmp_matrix, block_row_idx)
    {
        TASK_MGR *task_mgr;
        UINT32 ret;
        UINT32 recv_mod_node_pos;

        /*assmue local mod_node not in remote mod_node list*/
        task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        mod_mgr_first_remote(mod_mgr, &recv_mod_node_pos);

        MATRIX_COL_BLOCKS_LOOP_NEXT(src_matrix_1, block_col_idx)
        {
            MATRIX_BLOCK  *src_matrix_row_block_1;
            UINT32         row_num_of_src_matrix_row_block_1;

            src_matrix_row_block_1 = MATRIX_GET_BLOCK(src_matrix_1, block_row_idx, block_col_idx);
            matrix_r_block_get_row_num(matrixr_md_id, src_matrix_row_block_1, &row_num_of_src_matrix_row_block_1);

            task_pos_inc(task_mgr, recv_mod_node_pos, &ret, FI_dmatrix_r_clean_cache, ERR_MODULE_ID, row_num_of_src_matrix_row_block_1, col_blocks_num_of_src_matrix_2);
            mod_mgr_next_remote(mod_mgr, &recv_mod_node_pos);
        }
        task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

        /*assmue local mod_node not in remote mod_node list*/
        task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        mod_mgr_first_remote(mod_mgr, &recv_mod_node_pos);

        MATRIX_COL_BLOCKS_DOUBLE_LOOP_NEXT(src_matrix_1, tmp_matrix, block_col_idx)
        {
            MATRIX_BLOCK  *src_matrix_row_block_1;
            MATRIX_BLOCK  *tmp_matrix_row_block;

            src_matrix_row_block_1 = MATRIX_GET_BLOCK(src_matrix_1, block_row_idx, block_col_idx);
            tmp_matrix_row_block = MATRIX_GET_BLOCK(tmp_matrix, block_row_idx, block_col_idx);

            task_pos_inc(task_mgr, recv_mod_node_pos, &ret, FI_dmatrix_r_mul_row_block, ERR_MODULE_ID, src_matrix_row_block_1, tmp_matrix_row_block);
            mod_mgr_next_remote(mod_mgr, &recv_mod_node_pos);
        }
        task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

#if 0
        task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, &task_mgr);
        mod_mgr_first_remote(mod_mgr, &recv_mod_node_pos);

        MATRIX_COL_BLOCKS_LOOP_NEXT(src_matrix_1, block_col_idx)
        {
            MATRIX_BLOCK  *src_matrix_row_block_1;

            src_matrix_row_block_1 = MATRIX_GET_BLOCK(src_matrix_1, block_row_idx, block_col_idx);
            task_pos_inc(task_mgr, recv_mod_node_pos, &ret, FI_dmatrix_r_mul_row_block, ERR_MODULE_ID, src_matrix_row_block_1);
            mod_mgr_next_remote(mod_mgr, &recv_mod_node_pos);
        }
        task_wait(task_mgr);

        /*when come back, all result blocks in row are available. so that we can clone back the result*/
        task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, &task_mgr);
        mod_mgr_first_remote(mod_mgr, &recv_mod_node_pos);

        MATRIX_COL_BLOCKS_LOOP_NEXT(tmp_matrix, block_col_idx)
        {
            MATRIX_BLOCK  *tmp_matrix_row_block;

            tmp_matrix_row_block = MATRIX_GET_BLOCK(tmp_matrix, block_row_idx, block_col_idx);
            task_pos_inc(task_mgr, recv_mod_node_pos, &ret, FI_dmatrix_r_clone_row_block, ERR_MODULE_ID, tmp_matrix_row_block);
            mod_mgr_next_remote(mod_mgr, &recv_mod_node_pos);
        }
        task_wait(task_mgr);
#endif
    }

    matrix_r_clean_matrix(matrixr_md_id, des_matrix);
    matrix_r_move_matrix(matrixr_md_id, tmp_matrix, des_matrix);
    matrix_r_destroy_matrix(matrixr_md_id, tmp_matrix);
    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

