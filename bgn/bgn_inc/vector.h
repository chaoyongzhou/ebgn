/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#ifndef _VECTOR_H
#define _VECTOR_H

#include "type.h"
#include "list_base.h"
#include "mm.h"

/* -----------------------------------------------  vector structer interface  ----------------------------------------------------*/
#define VECTOR_GET_ROTATED_FLAG(vector) ((vector)->rotated_flag)
#define VECTOR_SET_ROTATED_FLAG(vector, flg) ((vector)->rotated_flag = (flg))
#define VECTOR_XCHG_ROTATED_FLAG(vector) ((vector)->rotated_flag ^= 1 )

#define VECTOR_HEAD(vector) (&((vector)->head))

#define VECTOR_HEAD_NEXT(vector) ((vector)->head.next)
#define VECTOR_HEAD_PREV(vector) ((vector)->head.prev)

#define VECTOR_HEAD_INIT(vector) INIT_LIST_BASE_HEAD(VECTOR_HEAD(vector))

#define VECTOR_GET_NUM(vector)         (((vector)->num))
#define VECTOR_INC_NUM(vector)         (((vector)->num) ++)
#define VECTOR_DEC_NUM(vector)         (((vector)->num) --)
#define VECTOR_SET_NUM(vector, val)     (((vector)->num) = (val))

#define VECTOR_FIRST_BLOCK(vector)     list_base_entry(VECTOR_HEAD_NEXT(vector), VECTOR_BLOCK, node)

#define VECTOR_LAST_BLOCK(vector)      list_base_entry(VECTOR_HEAD_PREV(vector), VECTOR_BLOCK, node)

#define VECTOR_NULL_BLOCK(vector)     list_base_entry(VECTOR_HEAD(vector), VECTOR_BLOCK, node)

#define VECTOR_IS_EMPTY(vector)      list_base_empty(VECTOR_HEAD(vector))

#define VECTOR_ADD_BLOCK_TAIL(this_vector, vector_block_new)     \
    list_base_add_tail(VECTOR_BLOCK_NODE(vector_block_new), VECTOR_HEAD(this_vector))

#define VECTOR_ADD_BLOCK(this_vector, vector_block_new)      \
    list_base_add(VECTOR_BLOCK_NODE(vector_block_new), VECTOR_HEAD(this_vector))

#define VECTOR_HEAD_MOVE(src_vector, des_vector) \
    list_base_add_tail(VECTOR_HEAD(des_vector), VECTOR_HEAD(src_vector)); \
    list_base_del_init(VECTOR_HEAD(src_vector)); \
    VECTOR_SET_NUM(des_vector, VECTOR_GET_NUM(src_vector)); \
    VECTOR_SET_NUM(src_vector, 0);

/* -----------------------------------------------  vector block structer interface  ----------------------------------------------------*/
#define VECTOR_BLOCK_GET_NUM(vector_block)         (((vector_block)->num))
#define VECTOR_BLOCK_INC_NUM(vector_block)         (((vector_block)->num) ++)
#define VECTOR_BLOCK_DEC_NUM(vector_block)         (((vector_block)->num) --)
#define VECTOR_BLOCK_SET_NUM(vector_block, val)     (((vector_block)->num) = (val))

#define VECTOR_BLOCK_NODE(vector_block)                 (&((vector_block)->node))

#define VECTOR_BLOCK_NODE_NEXT(vector_block)                 ((vector_block)->node.next)

#define VECTOR_BLOCK_NODE_PREV(vector_block)                 ((vector_block)->node.prev)

#define VECTOR_BLOCK_NODE_INIT(vector_block)                 INIT_LIST_BASE_HEAD(VECTOR_BLOCK_NODE(vector_block))

#define VECTOR_BLOCK_DATA_ADDR(vector_block, sub_idx)             ((vector_block)->data_area[ sub_idx ])

#define VECTOR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx)         (VECTOR_BLOCK_DATA_ADDR(vector_block, sub_idx))

#define VECTOR_BLOCK_SET_DATA_ADDR(vector_block, sub_idx, addr)     (VECTOR_BLOCK_DATA_ADDR(vector_block, sub_idx) = (UINT32)(addr))

#define VECTOR_BLOCK_GET_DATA_VAL(vector_block, sub_idx, type)         (*(type *)VECTOR_BLOCK_DATA_ADDR(vector_block, sub_idx))

#define VECTOR_BLOCK_SET_DATA_VAL(vector_block, sub_idx, type, val)     (*(type *)VECTOR_BLOCK_DATA_ADDR(vector_block, sub_idx) = (val))

/*insert a new vector block ahead the current block*/
#define VECTOR_BLOCK_ADD_PREV(vector_block_cur, vector_block_new) \
    list_base_add_tail(VECTOR_BLOCK_NODE(vector_block_new), VECTOR_BLOCK_NODE(vector_block_cur))

/*insert a new vector node after the current node*/
#define VECTOR_BLOCK_ADD_NEXT(vector_block_cur, vector_block_new) \
    list_base_add(VECTOR_BLOCK_NODE(vector_block_new), VECTOR_BLOCK_NODE(vector_block_cur))

#define VECTOR_BLOCK_DEL(vector_block)          list_base_del(VECTOR_BLOCK_NODE(vector_block))

#define VECTOR_BLOCK_INIT(vector_block)        INIT_LIST_BASE_HEAD(VECTOR_BLOCK_NODE(vector_block))

#define VECTOR_BLOCK_NEXT(vector_block) \
    list_base_entry(VECTOR_BLOCK_NODE_NEXT(vector_block), VECTOR_BLOCK, node)

#define VECTOR_BLOCK_PREV(vector_block) \
    list_base_entry(VECTOR_BLOCK_NODE_PREV(vector_block), VECTOR_BLOCK, node)

/* -----------------------------------------------  single loop interface  ----------------------------------------------------*/
#define VECTOR_BLOCK_LOOP_NEXT(vector_block, this_vector) \
    for( (vector_block)  = VECTOR_FIRST_BLOCK(this_vector); \
         (vector_block) != VECTOR_NULL_BLOCK(this_vector); \
         (vector_block)  = VECTOR_BLOCK_NEXT(vector_block) )

#define VECTOR_BLOCK_LOOP_PREV(vector_block, this_vector) \
    for( (vector_block)  = VECTOR_LAST_BLOCK(this_vector); \
         (vector_block) != VECTOR_NULL_BLOCK(this_vector); \
         (vector_block)  = VECTOR_BLOCK_PREV(vector_block) )

/* -----------------------------------------------  double loop interface  ----------------------------------------------------*/
#define VECTOR_BLOCK_DOUBLE_LOOP_NEXT(vector_block_1st, vector_1st, vector_block_2nd, vector_2nd) \
    for( (vector_block_1st)  = VECTOR_FIRST_BLOCK(vector_1st), \
         (vector_block_2nd)  = VECTOR_FIRST_BLOCK(vector_2nd); \
         (vector_block_1st) != VECTOR_NULL_BLOCK(vector_1st); \
         (vector_block_1st)  = VECTOR_BLOCK_NEXT(vector_block_1st), \
         (vector_block_2nd)  = VECTOR_BLOCK_NEXT(vector_block_2nd) )

#define VECTOR_BLOCK_DOUBLE_LOOP_PREV(vector_block_1st, vector_1st, vector_block_2nd, vector_2nd) \
    for( (vector_block_1st)  = VECTOR_LAST_BLOCK(vector_1st), \
         (vector_block_2nd)  = VECTOR_LAST_BLOCK(vector_2nd); \
         (vector_block_1st) != VECTOR_NULL_BLOCK(vector_1st); \
         (vector_block_1st)  = VECTOR_BLOCK_PREV(vector_block_1st), \
         (vector_block_2nd)  = VECTOR_BLOCK_PREV(vector_block_2nd) )

/* -----------------------------------------------  triple loop interface  ----------------------------------------------------*/
#define VECTOR_BLOCK_TRIPLE_LOOP_NEXT(vector_block_1st, vector_1st, vector_block_2nd, vector_2nd, vector_block_3rd, vector_3rd) \
    for( (vector_block_1st)  = VECTOR_FIRST_BLOCK(vector_1st), \
         (vector_block_2nd)  = VECTOR_FIRST_BLOCK(vector_2nd), \
         (vector_block_3rd)  = VECTOR_FIRST_BLOCK(vector_3rd); \
         (vector_block_1st) != VECTOR_NULL_BLOCK(vector_1st); \
         (vector_block_1st)  = VECTOR_BLOCK_NEXT(vector_block_1st), \
         (vector_block_2nd)  = VECTOR_BLOCK_NEXT(vector_block_2nd), \
         (vector_block_3rd)  = VECTOR_BLOCK_NEXT(vector_block_3rd) )

#define VECTOR_BLOCK_TRIPLE_LOOP_PREV(vector_block_1st, vector_1st, vector_block_2nd, vector_2nd, vector_block_3rd, vector_3rd) \
    for( (vector_block_1st)  = VECTOR_LAST_BLOCK(vector_1st), \
         (vector_block_2nd)  = VECTOR_LAST_BLOCK(vector_2nd), \
         (vector_block_3rd)  = VECTOR_LAST_BLOCK(vector_3rd); \
         (vector_block_1st) != VECTOR_NULL_BLOCK(vector_1st); \
         (vector_block_1st)  = VECTOR_BLOCK_PREV(vector_block_1st), \
         (vector_block_2nd)  = VECTOR_BLOCK_PREV(vector_block_2nd), \
         (vector_block_3rd)  = VECTOR_BLOCK_PREV(vector_block_3rd) )

/* -----------------------------------------------  data area runthrough interface  ----------------------------------------------------*/
#define VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num) \
    for( (sub_idx) = 0; (sub_idx) < (sub_num); (sub_idx) ++ )


#define VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_PREV(sub_idx, sub_num) \
    for( (sub_idx) = sub_num; (sub_idx)-- > 0; )

#endif /*_VECTOR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
