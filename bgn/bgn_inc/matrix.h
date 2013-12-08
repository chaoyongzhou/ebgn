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

#ifndef _MATRIX_H
#define _MATRIX_H

#include "type.h"
#include "list_base.h"
#include "mm.h"

/* -----------------------------------------------  matrix structer interface  ----------------------------------------------------*/
#define MATRIX_GET_ROTATED_FLAG(matrix)                 ((matrix)->rotated_flag)
#define MATRIX_SET_ROTATED_FLAG(matrix, flg)            ((matrix)->rotated_flag = (flg))
#define MATRIX_XCHG_ROTATED_FLAG(matrix)                ((matrix)->rotated_flag ^= 1 )

#define MATRIX_GET_ROW_NUM(matrix)                      (((matrix)->num[ 0 ^ (MATRIX_GET_ROTATED_FLAG(matrix)) ]))
#define MATRIX_SET_ROW_NUM(matrix, val)                 (((matrix)->num[ 0 ^ (MATRIX_GET_ROTATED_FLAG(matrix)) ]) = (val))

#define MATRIX_GET_COL_NUM(matrix)                      (((matrix)->num[ 1 ^ (MATRIX_GET_ROTATED_FLAG(matrix)) ]))
#define MATRIX_SET_COL_NUM(matrix, val)                 (((matrix)->num[ 1 ^ (MATRIX_GET_ROTATED_FLAG(matrix)) ]) = (val))

#define MATRIX_GET_BLOCKS(matrix)                       ((matrix)->blocks)
#define MATRIX_SET_BLOCKS(matrix, this_blocks)          ((matrix)->blocks = (this_blocks))
#define MATRIX_BLOCKS_INIT(matrix)                      ((matrix)->blocks = (0))

#define MATRIX_GET_ROW_BLOCKS_NUM(matrix)               ((MATRIX_GET_ROW_NUM(matrix) + MATRIX_VECTOR_WIDTH - 1) / MATRIX_VECTOR_WIDTH)
#define MATRIX_GET_COL_BLOCKS_NUM(matrix)               ((MATRIX_GET_COL_NUM(matrix) + MATRIX_VECTOR_WIDTH - 1) / MATRIX_VECTOR_WIDTH)

#define MATRIX_BLOCK_OFFSET(rotated_flag, block_row_num, block_col_num, block_row_idx, block_col_idx) \
        ( ((1 ^ (rotated_flag)) * (block_col_num) + (0 ^ (rotated_flag)))*(block_row_idx) + \
          ((0 ^ (rotated_flag)) * (block_row_num) + (1 ^ (rotated_flag))) *(block_col_idx))

#define MATRIX_BLOCK_POS(matrix, block_row_idx, block_col_idx)\
        MATRIX_BLOCK_OFFSET(MATRIX_GET_ROTATED_FLAG(matrix), MATRIX_GET_ROW_BLOCKS_NUM(matrix), MATRIX_GET_COL_BLOCKS_NUM(matrix), block_row_idx, block_col_idx)

#define MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx) \
    ((MATRIX_BLOCK *)cvector_get((CVECTOR *)MATRIX_GET_BLOCKS(matrix), MATRIX_BLOCK_POS(matrix, block_row_idx, block_col_idx)))

#define MATRIX_SET_BLOCK(matrix, block_row_idx, block_col_idx, matrix_block) \
    (cvector_set((CVECTOR *)MATRIX_GET_BLOCKS(matrix), MATRIX_BLOCK_POS(matrix, block_row_idx, block_col_idx), matrix_block))

/* -----------------------------------------------  matrix block structer interface  ----------------------------------------------------*/
#define MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)                  ((matrix_block)->rotated_flag)
#define MATRIX_BLOCK_SET_ROTATED_FLAG(matrix_block, flg)             ((matrix_block)->rotated_flag = (flg))
#define MATRIX_BLOCK_XCHG_ROTATED_FLAG(matrix_block)                 ((matrix_block)->rotated_flag ^= 1 )

#define MATRIX_BLOCK_GET_ROW_NUM(matrix_block)                       (((matrix_block)->num[ 0 ^ (MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)) ]))
#define MATRIX_BLOCK_SET_ROW_NUM(matrix_block, val)                  (((matrix_block)->num[ 0 ^ (MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)) ]) = (val))

#define MATRIX_BLOCK_GET_COL_NUM(matrix_block)                       (((matrix_block)->num[ 1 ^ (MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)) ]))
#define MATRIX_BLOCK_SET_COL_NUM(matrix_block, val)                  (((matrix_block)->num[ 1 ^ (MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block)) ]) = (val))


#if (MATRIXR_PATCH_01_SWITCH == SWITCH_ON)
#define MATRIX_BLOCK_DATA_BUFF(matrix_block) ((matrix_block)->data_buff_addr)
#define MATRIX_BLOCK_DATA_BUFF_SET(matrix_block, data_buff_ptr) ((matrix_block)->data_buff_addr = ((UINT32)(data_buff_ptr)))
#endif/*(MATRIXR_PATCH_01_SWITCH == SWITCH_ON)*/

#define MATRIX_BLOCK_DATA_ROW_NO(rotated_flag, sub_row_idx, sub_col_idx) \
    ((sub_row_idx) * ( 1 ^ (rotated_flag))  + (sub_col_idx) * (rotated_flag))

#define MATRIX_BLOCK_DATA_COL_NO(rotated_flag, sub_row_idx, sub_col_idx) \
    ((sub_col_idx) * ( 1 ^ (rotated_flag))  + (sub_row_idx) * (rotated_flag))

#if (MATRIXR_PATCH_SWITCH == SWITCH_OFF || MATRIXR_PATCH_01_SWITCH == SWITCH_ON)
#define MATRIX_BLOCK_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx) \
       ((matrix_block)->data_area[ MATRIX_BLOCK_DATA_ROW_NO(MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block), sub_row_idx, sub_col_idx) ][ MATRIX_BLOCK_DATA_COL_NO(MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block), sub_row_idx, sub_col_idx) ])

#define MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx, addr) \
    (MATRIX_BLOCK_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx) = (UINT32)(addr))

#endif/*(MATRIXR_PATCH_SWITCH == SWITCH_OFF || MATRIXR_PATCH_01_SWITCH == SWITCH_ON)*/

#if (MATRIXR_PATCH_02_SWITCH == SWITCH_ON)
#define MATRIX_BLOCK_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx) \
    (&((matrix_block)->data_area[ MATRIX_BLOCK_DATA_ROW_NO(MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block), sub_row_idx, sub_col_idx) ][ MATRIX_BLOCK_DATA_COL_NO(MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block), sub_row_idx, sub_col_idx) ]))

#define MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx, addr) do{}while(0)

#endif/*(MATRIXR_PATCH_02_SWITCH == SWITCH_ON)*/

#define MATRIX_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx) \
    (MATRIX_BLOCK_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx))

#define MATRIX_BLOCK_GET_DATA_VAL(matrix_block, sub_row_idx, sub_col_idx, type) \
    (*(type *)MATRIX_BLOCK_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx))

#define MATRIX_BLOCK_SET_DATA_VAL(matrix_block, sub_row_idx, sub_col_idx, type, val) \
    (*(type *)MATRIX_BLOCK_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx) = (val))

/* -----------------------------------------------  single loop interface  ----------------------------------------------------*/
#define MATRIX_BLOCK_ROW_LOOP_NEXT(blocks_row_num, block_row_idx) \
    for( (block_row_idx) = 0; (block_row_idx) < (blocks_row_num); (block_row_idx) ++)

#define MATRIX_BLOCK_COL_LOOP_NEXT(blocks_col_num, block_col_idx) \
    for( (block_col_idx) = 0; (block_col_idx) < (blocks_col_num); (block_col_idx) ++)

#define MATRIX_BLOCK_ROW_COL_LOOP_NEXT(blocks_row_num, block_row_idx, blocks_col_num, block_col_idx) \
    MATRIX_BLOCK_ROW_LOOP_NEXT(blocks_row_num, block_row_idx) \
    MATRIX_BLOCK_COL_LOOP_NEXT(blocks_col_num, block_col_idx)

#define MATRIX_ROW_BLOCKS_LOOP_NEXT(matrix, block_row_idx) \
    MATRIX_BLOCK_ROW_LOOP_NEXT(MATRIX_GET_ROW_BLOCKS_NUM(matrix), (block_row_idx))

#define MATRIX_COL_BLOCKS_LOOP_NEXT(matrix, block_col_idx) \
    MATRIX_BLOCK_COL_LOOP_NEXT(MATRIX_GET_COL_BLOCKS_NUM(matrix), (block_col_idx))

#define MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx) \
    MATRIX_ROW_BLOCKS_LOOP_NEXT(matrix, block_row_idx) \
    MATRIX_COL_BLOCKS_LOOP_NEXT(matrix, block_col_idx)

/* -----------------------------------------------  double loop interface  ----------------------------------------------------*/
#define MATRIX_BLOCK_ROW_DOUBLE_LOOP_NEXT(blocks_row_num_1st, blocks_row_num_2nd, block_row_idx)\
    for((block_row_idx) = 0; (block_row_idx) < (blocks_row_num_1st) && (block_row_idx) < (blocks_row_num_2nd) ; (block_row_idx) ++)

#define MATRIX_BLOCK_COL_DOUBLE_LOOP_NEXT(blocks_col_num_1st, blocks_col_num_2nd, block_col_idx)\
    for((block_col_idx) = 0; (block_col_idx) < (blocks_col_num_1st) && (block_col_idx) < (blocks_col_num_2nd) ; (block_col_idx) ++)

#define MATRIX_ROW_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_row_idx) \
    MATRIX_BLOCK_ROW_DOUBLE_LOOP_NEXT(MATRIX_GET_ROW_BLOCKS_NUM(matrix_1st), MATRIX_GET_ROW_BLOCKS_NUM(matrix_2nd), (block_row_idx))

#define MATRIX_COL_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_col_idx) \
    MATRIX_BLOCK_COL_DOUBLE_LOOP_NEXT(MATRIX_GET_COL_BLOCKS_NUM(matrix_1st), MATRIX_GET_COL_BLOCKS_NUM(matrix_2nd), (block_col_idx))

#define MATRIX_ROW_COL_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_row_idx, block_col_idx) \
    MATRIX_ROW_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_row_idx) \
    MATRIX_COL_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1st, matrix_2nd, block_col_idx)

/* -----------------------------------------------  triple loop interface  ----------------------------------------------------*/
#define MATRIX_BLOCK_ROW_TRIPLE_LOOP_NEXT(blocks_row_num_1st, blocks_row_num_2nd, blocks_row_num_3rd, block_row_idx)\
    for((block_row_idx) = 0; (block_row_idx) < (blocks_row_num_1st) && (block_row_idx) < (blocks_row_num_2nd) && (block_row_idx) < (blocks_row_num_3rd); (block_row_idx) ++)

#define MATRIX_BLOCK_COL_TRIPLE_LOOP_NEXT(blocks_col_num_1st, blocks_col_num_2nd, blocks_col_num_3rd, block_col_idx)\
    for((block_col_idx) = 0; (block_col_idx) < (blocks_col_num_1st) && (block_col_idx) < (blocks_col_num_2nd) && (block_col_idx) < (blocks_col_num_3rd); (block_col_idx) ++)

#define MATRIX_ROW_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_row_idx) \
    MATRIX_BLOCK_ROW_TRIPLE_LOOP_NEXT(MATRIX_GET_ROW_BLOCKS_NUM(matrix_1st), MATRIX_GET_ROW_BLOCKS_NUM(matrix_2nd), MATRIX_GET_ROW_BLOCKS_NUM(matrix_3rd), (block_row_idx))

#define MATRIX_COL_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_col_idx) \
    MATRIX_BLOCK_COL_TRIPLE_LOOP_NEXT(MATRIX_GET_COL_BLOCKS_NUM(matrix_1st), MATRIX_GET_COL_BLOCKS_NUM(matrix_2nd), MATRIX_GET_COL_BLOCKS_NUM(matrix_3rd), (block_col_idx))

#define MATRIX_ROW_COL_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_row_idx, block_col_idx) \
    MATRIX_ROW_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_row_idx) \
    MATRIX_COL_BLOCKS_TRIPLE_LOOP_NEXT(matrix_1st, matrix_2nd, matrix_3rd, block_col_idx)

/* -----------------------------------------------  data area runthrough interface  ----------------------------------------------------*/
#define MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num) \
    for( (sub_idx) = 0; (sub_idx) < (sub_num); (sub_idx) ++ )

#define MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, row_num, sub_col_idx, col_num) \
    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_row_idx, row_num)\
    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_col_idx, col_num)


#endif /*_MATRIX_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
