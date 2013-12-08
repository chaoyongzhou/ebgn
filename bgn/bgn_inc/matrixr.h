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

#ifndef _MATRIXR_H
#define _MATRIXR_H

#include "type.h"
#include "mm.h"
#include "real.h"
#include "vectorr.h"
#include "task.h"

#define MATRIXR_BLOCK_GET_DATA_VAL(matrix_block, row_idx, col_idx ) \
        (MATRIX_BLOCK_GET_DATA_VAL(matrix_block, row_idx, col_idx, REAL))

#define MATRIXR_BLOCK_SET_DATA_VAL(matrix_block, row_idx, col_idx, val) \
        (MATRIX_BLOCK_SET_DATA_VAL(matrix_block, row_idx, col_idx, REAL, val))

#define MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx) \
        ((REAL *)MATRIX_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx))

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    MOD_MGR  *mod_mgr;

    UINT32 real_md_id;
    UINT32 vectorr_md_id;

}MATRIXR_MD;

/**
*   for test only
*
*   to query the status of MATRIXR Module
*
**/
void matrix_r_print_module_status(const UINT32 matrixr_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed MATRIXR module
*
*
**/
UINT32 matrix_r_free_module_static_mem(const UINT32 matrixr_md_id);
/**
*
* start MATRIXR module
*
**/
UINT32 matrix_r_start( );

/**
*
* end MATRIXR module
*
**/
void matrix_r_end(const UINT32 matrixr_md_id);

/**
*
* set mod mgr of MATRIXR module
*
**/
UINT32 matrix_r_set_mod_mgr(const UINT32 matrixr_md_id, const MOD_MGR * src_task_mod_mgr);

/**
*
* get mod mgr of MATRIXR module
*
**/
MOD_MGR * matrix_r_get_mod_mgr(const UINT32 matrixr_md_id);

/**
*
* new skeleton of a matrix
* note: all matrix item pointer is null without allocing memory space
*
**/
UINT32 matrix_r_new_matrix_skeleton(const UINT32 matrixr_md_id, const UINT32 row_num, const UINT32 col_num, MATRIX *matrix);

/**
*
* new a matrix
* new a matrix node itsele and its skeleton
* note: all matrix item pointer is null without allocing memory space
*
**/
UINT32 matrix_r_new_matrix(const UINT32 matrixr_md_id, const UINT32 row_num, const UINT32 col_num, MATRIX **ppmatrix);

/**
*
* intialize a matrix to a 0 x 0 matrix
*
**/
UINT32 matrix_r_init_matrix(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* move src_matrix skeleton and data area to des_matrix
* note:
*     des_matrix must have no skeleton or data area. otherwise, skeleton and data area will be lost without free
*
**/
UINT32 matrix_r_move_matrix(const UINT32 matrixr_md_id, MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* clean a matrix
* all data area pointers, block pointers, header pointers, except matrix itself pointer,  will be destroyed.
* note:
*       matrix pointer can be reused after calling
*
**/
UINT32 matrix_r_clean_matrix(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* destroy a matrix
* all data area pointers, block pointers, header pointers, and matrix itself pointer will be destroyed.
* note:
*       matrix pointer cannot be reused after calling
*
**/
UINT32 matrix_r_destroy_matrix(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* insert datas into matrix row by row
* note:
*     matrix must be empty with skeleton only. any pointers mounted on data_area will be lost without notification
*     pdata[] is one-dimension array carrying on REAL pointer
*
**/
UINT32 matrix_r_insert_data_by_row(const UINT32 matrixr_md_id, const REAL *pdata[], const UINT32 data_num, MATRIX *matrix);

/**
*
* here not consider the insertion row into one existing matrix with data
* just insert the row into one empty matrix with skeleton only including matrix node itself, headers, blocks
* note: ppdata[] is two-dimension array carrying on REAL pointer
*
**/
UINT32 matrix_r_insert_data_by_tbl(const UINT32 matrixr_md_id, const REAL **ppdata[], const UINT32 row_num, const UINT32 col_num, MATRIX *matrix);

/**
*
* get row num of matrix block
*
**/
UINT32 matrix_r_block_get_row_num(const UINT32 matrixr_md_id, const MATRIX_BLOCK *matrix_block, UINT32 *row_num);


/**
*
* get col num of matrix block
*
**/
UINT32 matrix_r_block_get_col_num(const UINT32 matrixr_md_id, const MATRIX_BLOCK *matrix_block, UINT32 *col_num);

/**
*
* get type of matrix block
*
**/
UINT32 matrix_r_block_get_type(const UINT32 matrixr_md_id, const MATRIX_BLOCK *matrix_block, UINT32 *row_num, UINT32 *col_num);

/**
*
* set type of matrix block
*
**/
UINT32 matrix_r_block_set_type(const UINT32 matrixr_md_id, const UINT32 row_num, const UINT32 col_num, MATRIX_BLOCK *matrix_block);

/**
*
* get row num of matrix
*
**/
UINT32 matrix_r_get_row_num(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *row_num);

/**
*
* get col num of matrix
*
**/
UINT32 matrix_r_get_col_num(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *col_num);

/**
*
* get type of matrix
* if matrix type is (m x n), then return row_num = m, and col_num = n
*
**/
UINT32 matrix_r_get_type(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *row_num, UINT32 *col_num);

/**
*
* exchange two rows of the matrix
* if row no is overflow, then report error and do nothing
*
**/
UINT32 matrix_r_xchg_rows(const UINT32 matrixr_md_id, const UINT32 row_no_1, const UINT32 row_no_2, MATRIX *matrix);

/**
*
* exchange two cols of the matrix
* if col no is overflow, then report error and do nothing
*
**/
UINT32 matrix_r_xchg_cols(const UINT32 matrixr_md_id, const UINT32 col_no_1, const UINT32 col_no_2, MATRIX *matrix);

/**
*
* get col blocks num of matrix
*
**/
UINT32 matrix_r_get_col_blocks_num(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *col_blocks_num);

/**
*
* clone src matrix to des matrix
*       des_matrix = src_matrix
* note:
*    here des_matrix must be empty matrix, otherwise, its skeleton will be lost without notification
*
**/
UINT32 matrix_r_clone(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix is_zero operation
*    if all data_area is zero or null pointer, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL matrix_r_is_zero(const UINT32 matrixr_md_id, const MATRIX *matrix);

/**
*
* matrix is_one operation
*    if matrix is unit matrix, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL matrix_r_is_one(const UINT32 matrixr_md_id, const MATRIX *matrix);

/**
*
* matrix = 0
*
**/
UINT32 matrix_r_set_zero(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* matrix = 1
* note:
*       1. row num of matrix must be same as col num
*       2. must row num > 0
*
**/
UINT32 matrix_r_set_one(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* matrix cmp operation
*    if matrix_1 == matrix_2, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL matrix_r_cmp(const UINT32 matrixr_md_id, const MATRIX *matrix_1, const MATRIX *matrix_2);

/**
*
* rotate a matrix
*       matrix = T(matrix)
*
**/
UINT32 matrix_r_rotate(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* matrix neg operation
*     des_matrix = - src_matrix
*
**/
UINT32 matrix_r_neg(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix add operation
*     des_matrix = des_matrix + src_matrix
* matrix type: (m x n) = (m x n) + (m x n)
*
**/
UINT32 matrix_r_adc(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix add operation
*     des_matrix = src_matrix_1 + src_matrix_2
* matrix type: (m x n) = (m x n) + (m x n)
*
**/
UINT32 matrix_r_add(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);

/**
*
* matrix sbb operation
*     des_matrix = des_matrix - src_matrix
* matrix type: (m x n) = (m x n) - (m x n)
*
**/
UINT32 matrix_r_sbb(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix sub operation
*     des_matrix = src_matrix_1 - src_matrix_2
* matrix type: (m x n) = (m x n) - (m x n)
*
**/
UINT32 matrix_r_sub(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);

/**
*
* matrix smul operation
*     des_matrix = s_data * src_matrix
*
**/
UINT32 matrix_r_s_mul(const UINT32 matrixr_md_id, const REAL *s_data_addr, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix squ operation
*     matrix = matrix ^ 2
* matrix type: (m x m) = (m x m) * (m x m)
*
**/
UINT32 matrix_r_squ_self(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* matrix squ operation
*     des_matrix = src_matrix ^ 2
* matrix type: (m x m) = (m x m) * (m x m)
*
**/
UINT32 matrix_r_squ(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix mul operation
*     des_matrix = des_matrix * src_matrix
* matrix type: (m x n) = (m x k) * (k x n)
*
**/
UINT32 matrix_r_mul_self_rear(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix mul operation
*     des_matrix = src_matrix * des_matrix
* matrix type: (m x n) = (m x k) * (k x n)
*
**/
UINT32 matrix_r_mul_self_front(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix mul operation
*     des_matrix = src_matrix_1 * src_matrix_2
* matrix type: (m x n) = (m x k) * (k x n)
*
**/
UINT32 matrix_r_mul(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);
UINT32 matrix_r_mul_p(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);



/*----------------------------------------------------- matrix block ---------------------------------------------------*/
UINT32 matrix_r_new_block(const UINT32 matrixr_md_id, MATRIX_BLOCK **matrix_block);
UINT32 matrix_r_init_block(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block);
/**
*
* clean data_area but do not free block itself
*
**/
UINT32 matrix_r_clean_block(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block);

UINT32 matrix_r_destroy_block(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block);

UINT32 matrix_r_block_adc(const UINT32 matrixr_md_id, const MATRIX_BLOCK *src_matrix_block, MATRIX_BLOCK *des_matrix_block);
UINT32 matrix_r_block_mul(const UINT32 matrixr_md_id, const MATRIX_BLOCK *src_matrix_block_1, const MATRIX_BLOCK *src_matrix_block_2,MATRIX_BLOCK *des_matrix_block);

/*----------------------------------------------------- matrix & vector operation ---------------------------------------------------*/

UINT32 matrix_r_block_mul_vector_block(const UINT32 matrixr_md_id, const MATRIX_BLOCK *src_matrix_block, const VECTOR_BLOCK *src_vector_block, VECTOR_BLOCK *des_vector_block);

/**
*
* matrix and vector mul operation
*    des_vector = src_matrix * src_vector
* type: (m x 1) = (m x n) * (n x 1)
*
*
**/
UINT32 matrix_r_matrix_mul_vector(const UINT32 matrixr_md_id, const MATRIX *src_matrix,const VECTOR *src_vector, VECTOR *des_vector);
UINT32 matrix_r_matrix_mul_vector_p(const UINT32 matrixr_md_id, const MATRIX *src_matrix,const VECTOR *src_vector, VECTOR *des_vector);


/* ---------------------------------------- internal interface ------------------------------------------------ */
UINT32 matrix_r_alloc_block( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, const UINT32 type, MATRIX_BLOCK **matrix_block);
UINT32 matrix_r_free_block( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, const UINT32 type, MATRIX_BLOCK *matrix_block);
UINT32 matrix_r_clone_block(const UINT32 matrixr_md_id, const MATRIX_BLOCK *src_matrix_block, MATRIX_BLOCK *des_matrix_block);


/* ---------------------------------------- output interface ------------------------------------------------ */
UINT32 matrix_r_print_matrix_addr_info(const UINT32 matrixr_md_id, MATRIX *matrix);
UINT32 matrix_r_print_matrix_data_info(const UINT32 matrixr_md_id, MATRIX *matrix);
UINT32 matrix_r_print_matrix_block_addr_info(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block);
UINT32 matrix_r_print_matrix_block_data_info(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block);

#endif /*_MATRIXR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

