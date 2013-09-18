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

#ifndef _POLY_H
#define _POLY_H

#include "type.h"
#include "list_base.h"

#define MAX_POLY_DEG_SUPPORTED_FOR_MOD 128

#define POLY_ITEM_HEAD(poly)  (&((poly)->poly_item_head))

/*the lowest degree item in the poly*/
#define POLY_FIRST_ITEM(poly) list_base_entry((poly)->poly_item_head.next, POLY_ITEM, item_node)

/*the highest degree item in the poly*/
#define POLY_LAST_ITEM(poly)  list_base_entry((poly)->poly_item_head.prev, POLY_ITEM, item_node)

/*the null item in the poly which is not any real item*/
#define POLY_NULL_ITEM(poly) list_base_entry(&((poly)->poly_item_head), POLY_ITEM, item_node)

#define POLY_IS_EMPTY(poly)  list_base_empty(POLY_ITEM_HEAD(poly))

#define POLY_ADD_ITEM_TAIL(poly,item_new) list_base_add_tail(POLY_ITEM_NODE(item_new), POLY_ITEM_HEAD(poly))

#define POLY_ADD_ITEM_HEAD(poly,item_new)  list_base_add(POLY_ITEM_NODE(item_new), POLY_ITEM_HEAD(poly))

#define POLY_INIT(poly)     INIT_LIST_BASE_HEAD(POLY_ITEM_HEAD(poly))

//#define POLY_MOVE(poly_src, poly_des) list_base_move(POLY_ITEM_HEAD(poly_src),POLY_ITEM_HEAD(poly_des))

#define POLY_ITEM_DEG(item)    (&((item)->item_deg))

#define POLY_DEG(poly)          POLY_ITEM_DEG(POLY_LAST_ITEM(poly))

#define POLY_ITEM_NODE(item)    (&((item)->item_node))

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
#define POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, deg) bgn_z_is_zero(bgnz_md_id, deg)

#define POLY_ITEM_DEG_IS_N(bgnz_md_id, deg, n) bgn_z_is_n(bgnz_md_id, deg, n)

#define POLY_ITEM_DEG_IS_ODD(bgnz_md_id, deg) bgn_z_is_odd(bgnz_md_id, deg)

#define POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, deg) bgn_z_set_zero(bgnz_md_id, deg)

#define POLY_ITEM_DEG_SET_ONE(bgnz_md_id, deg) bgn_z_set_one(bgnz_md_id, deg)

#define POLY_ITEM_DEG_SET_WORD(bgnz_md_id, deg, n) bgn_z_set_word(bgnz_md_id, deg, n)

#define POLY_ITEM_DEG_CMP(bgnz_md_id, deg_a, deg_b) bgn_z_cmp( bgnz_md_id, deg_a, deg_b)

#define POLY_ITEM_DEG_CLONE(bgnz_md_id, deg_src, deg_des) bgn_z_clone( bgnz_md_id, deg_src, deg_des)

#define POLY_ITEM_DEG_ADD(bgnz_md_id, deg_a, deg_b, deg_des) bgn_z_add( bgnz_md_id, deg_a, deg_b, deg_des);

#define POLY_ITEM_DEG_SUB(bgnz_md_id, deg_a, deg_b, deg_des) bgn_z_sub( bgnz_md_id, deg_a, deg_b, deg_des);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
#define POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, deg) (( 0 == (*deg) ) ? EC_TRUE : EC_FALSE)

#define POLY_ITEM_DEG_IS_N(bgnz_md_id, deg, n) (( (n) == (*deg) ) ? EC_TRUE : EC_FALSE)

#define POLY_ITEM_DEG_IS_ODD(bgnz_md_id, deg) (( 1 & (*deg) ) ? EC_TRUE : EC_FALSE)

#define POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, deg) ((*deg) = 0)

#define POLY_ITEM_DEG_SET_ONE(bgnz_md_id, deg) ((*deg) = 1)

#define POLY_ITEM_DEG_SET_WORD(bgnz_md_id, deg, n) ((*deg) = n)

#define POLY_ITEM_DEG_CMP(bgnz_md_id, deg_a, deg_b) ((*deg_a) == (*deg_b)? 0 : ( (*deg_a) > (*deg_b)? 1: (-1) ))

#define POLY_ITEM_DEG_CLONE(bgnz_md_id, deg_src, deg_des) ((*deg_des) = (*deg_src))

#define POLY_ITEM_DEG_ADD(bgnz_md_id, deg_a, deg_b, deg_des) (((*deg_des) = (*deg_a) + (*deg_b)),((*deg_des) < (*deg_a)?1:0))

#define POLY_ITEM_DEG_SUB(bgnz_md_id, deg_a, deg_b, deg_des) ((*deg_des) = (*deg_a) - (*deg_b))
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

#define POLY_ITEM_BGN_COE_FLAG(item) ((item)->bgn_coe_flag)

#define POLY_ITEM_BGN_COE(item)  ((item)->coe.bgn_coe)

#define POLY_ITEM_POLY_COE(item)  ((item)->coe.poly_coe)

#define POLY_ITEM_IS_EMPTY(item)  (((EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) && NULL_PTR == POLY_ITEM_BGN_COE(item))\
                                   ||(EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item) && NULL_PTR == POLY_ITEM_POLY_COE(item)))? EC_TRUE: EC_FALSE)

#define POLY_ITEM_NEXT(item)  list_base_entry((item)->item_node.next, POLY_ITEM, item_node)

#define POLY_ITEM_PREV(item)  list_base_entry((item)->item_node.prev, POLY_ITEM, item_node)

#define POLY_ITEM_DEL(item)     list_base_del(POLY_ITEM_NODE(item))

/*insert a new item ahead the current item*/
#define POLY_ITEM_ADD_PREV(item_cur, item_new) list_base_add_tail(POLY_ITEM_NODE(item_new), POLY_ITEM_NODE(item_cur))

/*insert a new item behind the current item*/
#define POLY_ITEM_ADD_NEXT(item_cur, item_new) list_base_add(POLY_ITEM_NODE(item_new), POLY_ITEM_NODE(item_cur))

#define POLY_ITEM_INIT(item)    INIT_LIST_BASE_HEAD(POLY_ITEM_NODE(item))

#endif /*_POLY_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

