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

#ifndef _VMATRIXR_H
#define _VMATRIXR_H

#include "type.h"
#include "mm.h"
#include "real.h"
#include "matrixr.h"
#include "task.inc"
#include "task.h"
#include "vmm.h"

#include "cmutex.h"

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    MOD_MGR  *mod_mgr;
    TASK_BRD *task_brd;
    MOD_NODE  local_mod_node;

    UINT32 matrixr_md_id;
    UINT32        vmm_md_id;

}VMATRIXR_MD;

typedef struct
{
     UINT32       num[ 2 ];    /*the row num or col num of block, depending on rotated_flag*/

     VMM_NODE     vmm_node;

     UINT32       rotated_flag; /*rotated_flag inheritd from matrix*/
}VMATRIX_BLOCK;


/* -----------------------------------------------  vmatrix structer interface  ----------------------------------------------------*/
#define VMATRIX_GET_ROTATED_FLAG(matrix)                 ((matrix)->rotated_flag)
#define VMATRIX_SET_ROTATED_FLAG(matrix, flg)            ((matrix)->rotated_flag = (flg))
#define VMATRIX_XCHG_ROTATED_FLAG(matrix)                ((matrix)->rotated_flag ^= 1 )

#define VMATRIX_GET_ROW_NUM(matrix)                      (((matrix)->num[ 0 ^ (VMATRIX_GET_ROTATED_FLAG(matrix)) ]))
#define VMATRIX_SET_ROW_NUM(matrix, val)                 (((matrix)->num[ 0 ^ (VMATRIX_GET_ROTATED_FLAG(matrix)) ]) = (val))

#define VMATRIX_GET_COL_NUM(matrix)                      (((matrix)->num[ 1 ^ (VMATRIX_GET_ROTATED_FLAG(matrix)) ]))
#define VMATRIX_SET_COL_NUM(matrix, val)                 (((matrix)->num[ 1 ^ (VMATRIX_GET_ROTATED_FLAG(matrix)) ]) = (val))

#define VMATRIX_GET_BLOCKS(matrix)                       ((matrix)->blocks)
#define VMATRIX_SET_BLOCKS(matrix, this_blocks)          ((matrix)->blocks = (this_blocks))
#define VMATRIX_BLOCKS_INIT(matrix)                      ((matrix)->blocks = (0))

#define VMATRIX_GET_ROW_BLOCKS_NUM(matrix)               ((VMATRIX_GET_ROW_NUM(matrix) + MATRIX_VECTOR_WIDTH - 1) / MATRIX_VECTOR_WIDTH)
#define VMATRIX_GET_COL_BLOCKS_NUM(matrix)               ((VMATRIX_GET_COL_NUM(matrix) + MATRIX_VECTOR_WIDTH - 1) / MATRIX_VECTOR_WIDTH)

#define VMATRIX_BLOCK_OFFSET(rotated_flag, block_row_num, block_col_num, block_row_idx, block_col_idx) \
        ( ((1 ^ (rotated_flag)) * (block_col_num) + (0 ^ (rotated_flag)))*(block_row_idx) + \
          ((0 ^ (rotated_flag)) * (block_row_num) + (1 ^ (rotated_flag))) *(block_col_idx))

#define VMATRIX_BLOCK_POS(matrix, block_row_idx, block_col_idx)\
        VMATRIX_BLOCK_OFFSET(VMATRIX_GET_ROTATED_FLAG(matrix), VMATRIX_GET_ROW_BLOCKS_NUM(matrix), VMATRIX_GET_COL_BLOCKS_NUM(matrix), block_row_idx, block_col_idx)

#define VMATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx) \
    ((VMATRIX_BLOCK *)cvector_get((CVECTOR *)VMATRIX_GET_BLOCKS(matrix), VMATRIX_BLOCK_POS(matrix, block_row_idx, block_col_idx)))

#define VMATRIX_SET_BLOCK(matrix, block_row_idx, block_col_idx, matrix_block) \
    (cvector_set((CVECTOR *)VMATRIX_GET_BLOCKS(matrix), VMATRIX_BLOCK_POS(matrix, block_row_idx, block_col_idx), matrix_block))

/* -----------------------------------------------  matrix block structer interface  ----------------------------------------------------*/
#define VMATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)                  ((matrix_block)->rotated_flag)
#define VMATRIX_BLOCK_SET_ROTATED_FLAG(matrix_block, flg)             ((matrix_block)->rotated_flag = (flg))
#define VMATRIX_BLOCK_XCHG_ROTATED_FLAG(matrix_block)                 ((matrix_block)->rotated_flag ^= 1 )

#define VMATRIX_BLOCK_GET_ROW_NUM(matrix_block)                       (((matrix_block)->num[ 0 ^ (VMATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)) ]))
#define VMATRIX_BLOCK_SET_ROW_NUM(matrix_block, val)                  (((matrix_block)->num[ 0 ^ (VMATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)) ]) = (val))

#define VMATRIX_BLOCK_GET_COL_NUM(matrix_block)                       (((matrix_block)->num[ 1 ^ (VMATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)) ]))
#define VMATRIX_BLOCK_SET_COL_NUM(matrix_block, val)                  (((matrix_block)->num[ 1 ^ (VMATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)) ]) = (val))


#define VMATRIX_BLOCK_DATA_ROW_NO(rotated_flag, sub_row_idx, sub_col_idx) \
    ((sub_row_idx) * ( 1 ^ (rotated_flag))  + (sub_col_idx) * (rotated_flag))

#define VMATRIX_BLOCK_DATA_COL_NO(rotated_flag, sub_row_idx, sub_col_idx) \
    ((sub_col_idx) * ( 1 ^ (rotated_flag))  + (sub_row_idx) * (rotated_flag))


/* -----------------------------------------------  single loop interface  ----------------------------------------------------*/
#define VMATRIX_BLOCK_ROW_LOOP_NEXT(blocks_row_num, block_row_idx) \
    for( (block_row_idx) = 0; (block_row_idx) < (blocks_row_num); (block_row_idx) ++)

#define VMATRIX_BLOCK_COL_LOOP_NEXT(blocks_col_num, block_col_idx) \
    for( (block_col_idx) = 0; (block_col_idx) < (blocks_col_num); (block_col_idx) ++)

#define VMATRIX_BLOCK_ROW_COL_LOOP_NEXT(blocks_row_num, block_row_idx, blocks_col_num, block_col_idx) \
    VMATRIX_BLOCK_ROW_LOOP_NEXT(blocks_row_num, block_row_idx) \
    VMATRIX_BLOCK_COL_LOOP_NEXT(blocks_col_num, block_col_idx)

#define VMATRIX_ROW_BLOCKS_LOOP_NEXT(matrix, block_row_idx) \
    VMATRIX_BLOCK_ROW_LOOP_NEXT(VMATRIX_GET_ROW_BLOCKS_NUM(matrix), (block_row_idx))

#define VMATRIX_COL_BLOCKS_LOOP_NEXT(matrix, block_col_idx) \
    VMATRIX_BLOCK_COL_LOOP_NEXT(VMATRIX_GET_COL_BLOCKS_NUM(matrix), (block_col_idx))

#define VMATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx) \
    VMATRIX_ROW_BLOCKS_LOOP_NEXT(matrix, block_row_idx) \
    VMATRIX_COL_BLOCKS_LOOP_NEXT(matrix, block_col_idx)

/* -----------------------------------------------  double loop interface  ----------------------------------------------------*/
#define VMATRIX_BLOCK_ROW_DOUBLE_LOOP_NEXT(blocks_row_num_1st, blocks_row_num_2nd, block_row_idx)\
    for((block_row_idx) = 0; (block_row_idx) < (blocks_row_num_1st) && (block_row_idx) < (blocks_row_num_2nd) ; (block_row_idx) ++)

#define VMATRIX_BLOCK_COL_DOUBLE_LOOP_NEXT(blocks_col_num_1st, blocks_col_num_2nd, block_col_idx)\
    for((block_col_idx) = 0; (block_col_idx) < (blocks_col_num_1st) && (block_col_idx) < (blocks_col_num_2nd) ; (block_col_idx) ++)

#define VMATRIX_ROW_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_row_idx) \
    VMATRIX_BLOCK_ROW_DOUBLE_LOOP_NEXT(VMATRIX_GET_ROW_BLOCKS_NUM(matrix_1st), VMATRIX_GET_ROW_BLOCKS_NUM(matrix_2nd), (block_row_idx))

#define VMATRIX_COL_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_col_idx) \
    VMATRIX_BLOCK_COL_DOUBLE_LOOP_NEXT(VMATRIX_GET_COL_BLOCKS_NUM(matrix_1st), VMATRIX_GET_COL_BLOCKS_NUM(matrix_2nd), (block_col_idx))

#define VMATRIX_ROW_COL_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_row_idx, block_col_idx) \
    VMATRIX_ROW_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_row_idx) \
    VMATRIX_COL_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_col_idx)

/* -----------------------------------------------  triple loop interface  ----------------------------------------------------*/
#define VMATRIX_BLOCK_ROW_TRIPLE_LOOP_NEXT(blocks_row_num_1st, blocks_row_num_2nd, blocks_row_num_3rd, block_row_idx)\
    for((block_row_idx) = 0; (block_row_idx) < (blocks_row_num_1st) && (block_row_idx) < (blocks_row_num_2nd) && (block_row_idx) < (blocks_row_num_3rd); (block_row_idx) ++)

#define VMATRIX_BLOCK_COL_TRIPLE_LOOP_NEXT(blocks_col_num_1st, blocks_col_num_2nd, blocks_col_num_3rd, block_col_idx)\
    for((block_col_idx) = 0; (block_col_idx) < (blocks_col_num_1st) && (block_col_idx) < (blocks_col_num_2nd) && (block_col_idx) < (blocks_col_num_3rd); (block_col_idx) ++)

#define VMATRIX_ROW_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_row_idx) \
    VMATRIX_BLOCK_ROW_TRIPLE_LOOP_NEXT(VMATRIX_GET_ROW_BLOCKS_NUM(matrix_1st), VMATRIX_GET_ROW_BLOCKS_NUM(matrix_2nd), VMATRIX_GET_ROW_BLOCKS_NUM(matrix_3rd), (block_row_idx))

#define VMATRIX_COL_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_col_idx) \
    VMATRIX_BLOCK_COL_TRIPLE_LOOP_NEXT(VMATRIX_GET_COL_BLOCKS_NUM(matrix_1st), VMATRIX_GET_COL_BLOCKS_NUM(matrix_2nd), VMATRIX_GET_COL_BLOCKS_NUM(matrix_3rd), (block_col_idx))

#define VMATRIX_ROW_COL_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_row_idx, block_col_idx) \
    VMATRIX_ROW_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_row_idx) \
    VMATRIX_COL_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_col_idx)

/* -----------------------------------------------  data area runthrough interface  ----------------------------------------------------*/
#define VMATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num) \
    for( (sub_idx) = 0; (sub_idx) < (sub_num); (sub_idx) ++ )

#define VMATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, row_num, sub_col_idx, col_num) \
    VMATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_row_idx, row_num)\
    VMATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_col_idx, col_num)

/**
*   for test only
*
*   to query the status of VMATRIXR Module
*
**/
void vmatrix_r_print_module_status(const UINT32 vmatrixr_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed VMATRIXR module
*
*
**/
UINT32 vmatrix_r_free_module_static_mem(const UINT32 vmatrixr_md_id);

/**
*
* start VMATRIXR module
*
**/
UINT32 vmatrix_r_start( );

/**
*
* end VMATRIXR module
*
**/
void vmatrix_r_end(const UINT32 vmatrixr_md_id);

/**
*
* initialize mod mgr of VMATRIXR module
*
**/
UINT32 vmatrix_r_set_mod_mgr(const UINT32 vmatrixr_md_id, const MOD_MGR * src_mod_mgr);

/**
*
* get mod mgr of MATRIXR module
*
**/
MOD_MGR * vmatrix_r_get_mod_mgr(const UINT32 vmatrixr_md_id);

VMM_NODE * vmatrix_r_alloc_vmm_node(const UINT32 vmatrixr_md_id);

EC_BOOL vmatrix_r_free_vmm_node(const UINT32 vmatrixr_md_id, VMM_NODE *vmm_node);

UINT32 vmatrix_r_alloc_block(const UINT32 vmatrixr_md_id, VMM_NODE *matrix_block_vmm);

UINT32 vmatrix_r_free_block(const UINT32 vmatrixr_md_id, VMM_NODE *matrix_block_vmm);

EC_BOOL vmatrix_r_local_mod_node(const UINT32 vmatrixr_md_id, MOD_NODE *mod_node);

UINT32 vmatrix_r_block_set_row_num(const UINT32 vmatrixr_md_id, const UINT32 row_num, VMM_NODE *matrix_block_vmm);

UINT32 vmatrix_r_block_set_col_num(const UINT32 vmatrixr_md_id, const UINT32 col_num, VMM_NODE *matrix_block_vmm);

UINT32 vmatrix_r_new_matrix_skeleton(const UINT32 vmatrixr_md_id, const UINT32 row_num, const UINT32 col_num, MATRIX *matrix);

UINT32 vmatrix_r_new_matrix(const UINT32 vmatrixr_md_id, const UINT32 row_num, const UINT32 col_num, VMM_NODE *matrix_vmm);

UINT32 vmatrix_r_get_block(const UINT32 vmatrixr_md_id, const VMM_NODE *vmm_node, MATRIX_BLOCK *matrix_block);

UINT32 vmatrix_r_set_block(const UINT32 vmatrixr_md_id, const MATRIX_BLOCK *matrix_block, VMM_NODE *vmm_node);

UINT32 vmatrix_r_block_mul(const UINT32 vmatrixr_md_id, const VMM_NODE *src_matrix_row_block_vmm_1, const VMM_NODE *src_matrix_row_block_vmm_2, VMM_NODE *des_matrix_row_block_vmm);

UINT32 vmatrix_r_block_adc(const UINT32 vmatrixr_md_id, const VMM_NODE *src_matrix_row_block_vmm, VMM_NODE *des_matrix_row_block_vmm);

UINT32 vmatrix_r_clean_matrix(const UINT32 vmatrixr_md_id, VMM_NODE *matrix_vmm);

UINT32 vmatrix_r_move_matrix(const UINT32 vmatrixr_md_id, VMM_NODE *src_matrix_vmm, VMM_NODE *des_matrix_vmm);

UINT32 vmatrix_r_free_matrix( const UINT32 vmatrixr_md_id, VMM_NODE *matrix_vmm);

UINT32 vmatrix_r_destroy_matrix(const UINT32 vmatrixr_md_id, VMM_NODE *matrix_vmm);
/**
*
* matrix mul operation
*     des_matrix = src_matrix_1 * src_matrix_2
*
**/
UINT32 vmatrix_r_mul_p(const UINT32 vmatrixr_md_id, const VMM_NODE *src_matrix_vmm_1, const VMM_NODE *src_matrix_vmm_2, VMM_NODE *des_matrix_vmm);


#endif /*_VMATRIXR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

