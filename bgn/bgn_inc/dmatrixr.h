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

#ifndef _DMATRIXR_H
#define _DMATRIXR_H

#include "type.h"
#include "mm.h"
#include "real.h"
#include "matrixr.h"
#include "task.h"

#include "croutine.h"

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    MOD_MGR  *mod_mgr;

    UINT32 real_md_id;
    UINT32 matrixr_md_id;

    MATRIX *src_matrixr; /*row-blocks*/

    MATRIX *matrixr_of_mul; /*cache of mul result*/
    MATRIX_BLOCK *matrixr_row_block_of_adc;/*cache of adc result*/

    CROUTINE_MUTEX       cmutex;
    CROUTINE_COND        ccond;
}DMATRIXR_MD;


/**
*   for test only
*
*   to query the status of DMATRIXR Module
*
**/
void dmatrix_r_print_module_status(const UINT32 dmatrixr_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed DMATRIXR module
*
*
**/
UINT32 dmatrix_r_free_module_static_mem(const UINT32 dmatrixr_md_id);

/**
*
* start DMATRIXR module
*
**/
UINT32 dmatrix_r_start( );

/**
*
* end DMATRIXR module
*
**/
void dmatrix_r_end(const UINT32 dmatrixr_md_id);


/**
*
* initialize mod mgr of DMATRIXR module
*
**/
UINT32 dmatrix_r_set_mod_mgr(const UINT32 dmatrixr_md_id, const MOD_MGR * src_mod_mgr);

/**
*
* get mod mgr of DMATRIXR module
*
**/
MOD_MGR * dmatrix_r_get_mod_mgr(const UINT32 dmatrixr_md_id);

/**
*
* get matrixr_md_id of DMATRIXR module
*
**/
UINT32 dmatrix_r_get_matrixr_md_id(const UINT32 dmatrixr_md_id, UINT32 * matrixr_md_id);

UINT32 dmatrix_r_clean_adc_cache(const UINT32 dmatrixr_md_id, const UINT32 row_num_of_src_matrix_row_block);
UINT32 dmatrix_r_clean_mul_cache(const UINT32 dmatrixr_md_id, const UINT32 row_num_of_src_matrix_row_block);
UINT32 dmatrix_r_clean_cache(const UINT32 dmatrixr_md_id, const UINT32 row_num_of_src_matrix_row_block, const UINT32 col_blocks_num);

UINT32 dmatrix_r_append_row_block(const UINT32 dmatrixr_md_id, const MATRIX_BLOCK *matrixr_row_block);

UINT32 dmatrix_r_append_col_block(const UINT32 dmatrixr_md_id, const MATRIX_BLOCK *matrixr_col_block);

UINT32 dmatrix_r_deliver_row_blocks(const UINT32 dmatrixr_md_id, const MATRIX *matrixr);

UINT32 dmatrix_r_deliver_col_blocks(const UINT32 dmatrixr_md_id, const MATRIX *matrixr);

UINT32 dmatrix_r_deliver_rows_p(const UINT32 dmatrixr_md_id, const MATRIX *matrixr);
UINT32 dmatrix_r_deliver_cols_p(const UINT32 dmatrixr_md_id, const MATRIX *matrixr);;

UINT32 dmatrix_r_clone_row_block(const UINT32 dmatrixr_md_id, MATRIX_BLOCK *des_matrix_block);
UINT32 dmatrix_r_adc_row_block(const UINT32 dmatrixr_md_id, const MATRIX_BLOCK *src_matrix_row_block);
UINT32 dmatrix_r_mul_row_block(const UINT32 dmatrixr_md_id, const MATRIX_BLOCK *src_matrix_row_block, MATRIX_BLOCK *ret_matrix_row_block);
UINT32 dmatrix_r_mul_p(const UINT32 dmatrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);

#endif /*_DMATRIXR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

