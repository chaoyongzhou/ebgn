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

#ifndef _EBGNZ_H
#define _EBGNZ_H

#include "type.h"
#include "list_base.h"

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    /* BGNZ Module ID refered by EBGNZ Module */
    UINT32 bgnz_md_id;

}EBGNZ_MD;

#define EBGN_ITEM_HEAD(ebgn)  (&((ebgn)->ebgn_item_head))

/*the lowest item in the ebgn*/
#define EBGN_FIRST_ITEM(ebgn) list_base_entry((ebgn)->ebgn_item_head.next, EBGN_ITEM, item_node)

/*the highest item in the ebgn*/
#define EBGN_LAST_ITEM(ebgn)  list_base_entry((ebgn)->ebgn_item_head.prev, EBGN_ITEM, item_node)

/*the null item in the ebgn which is not any real item*/
#define EBGN_NULL_ITEM(ebgn) list_base_entry(&((ebgn)->ebgn_item_head), EBGN_ITEM, item_node)

#define EBGN_IS_EMPTY(ebgn)  list_base_empty(EBGN_ITEM_HEAD(ebgn))

#define EBGN_ADD_ITEM_TAIL(ebgn,item_new) list_base_add_tail(EBGN_ITEM_NODE(item_new), EBGN_ITEM_HEAD(ebgn))

#define EBGN_ADD_ITEM_HEAD(ebgn,item_new) list_base_add(EBGN_ITEM_NODE(item_new), EBGN_ITEM_HEAD(ebgn))

#define EBGN_ITEM_NODE(item)    (&((item)->item_node))

#define EBGN_ITEM_BGN_IS_ZERO(bgnz_md_id, bgn) bgn_z_is_zero(bgnz_md_id, bgn)

#define EBGN_ITEM_BGN_IS_N(bgnz_md_id, bgn, n) bgn_z_is_n(bgnz_md_id, bgn, n)

#define EBGN_ITEM_BGN_IS_ODD(bgnz_md_id, bgn) bgn_z_is_odd(bgnz_md_id, bgn)

#define EBGN_ITEM_BGN_SET_ZERO(bgnz_md_id, bgn) bgn_z_set_zero(bgnz_md_id, bgn)

#define EBGN_ITEM_BGN_SET_ONE(bgnz_md_id, bgn) bgn_z_set_one(bgnz_md_id, bgn)

#define EBGN_ITEM_BGN_SET_WORD(bgnz_md_id, bgn, n) bgn_z_set_word(bgnz_md_id, bgn, n)

#define EBGN_ITEM_BGN_CMP(bgnz_md_id, bgn_a, bgn_b) bgn_z_cmp( bgnz_md_id, bgn_a, bgn_b)

#define EBGN_ITEM_BGN_CLONE(bgnz_md_id, bgn_src, bgn_des) bgn_z_clone( bgnz_md_id, bgn_src, bgn_des)

#define EBGN_ITEM_BGN_ADD(bgnz_md_id, bgn_a, bgn_b, bgn_des) bgn_z_add( bgnz_md_id, bgn_a, bgn_b, bgn_des);

#define EBGN_ITEM_BGN_SUB(bgnz_md_id, bgn_a, bgn_b, bgn_des) bgn_z_sub( bgnz_md_id, bgn_a, bgn_b, bgn_des);

#define EBGN_ITEM_BGN_SADD(bgnz_md_id, bgn_a, word_b, bgn_des) bgn_z_sadd( bgnz_md_id, bgn_a, word_b, bgn_des);

#define EBGN_ITEM_BGN_SSUB(bgnz_md_id, bgn_a, word_b, bgn_des) bgn_z_ssub( bgnz_md_id, bgn_a, word_b, bgn_des);

#define EBGN_ITEM_BGN(item)  ((item)->bgn)

#define EBGN_ITEM_NEXT(item)  list_base_entry((item)->item_node.next, EBGN_ITEM, item_node)

#define EBGN_ITEM_PREV(item)  list_base_entry((item)->item_node.prev, EBGN_ITEM, item_node)

#define EBGN_ITEM_DEL(item)     list_base_del(EBGN_ITEM_NODE(item))

/*insert a new item ahead the current item*/
#define EBGN_ITEM_ADD_PREV(item_cur, item_new) list_base_add_tail(EBGN_ITEM_NODE(item_new), EBGN_ITEM_NODE(item_cur))

/*insert a new item behind the current item*/
#define EBGN_ITEM_ADD_NEXT(item_cur, item_new) list_base_add(EBGN_ITEM_NODE(item_new), EBGN_ITEM_NODE(item_cur))

#define EBGN_ITEM_INIT(item)    (EBGN_ITEM_BGN(item) = NULL_PTR)

#define EBGN_SGN(ebgn)    	((ebgn)->sgn)

#define EBGN_SET_SGN(ebgn, sign) ((ebgn)->sgn = (sign))

#define EBGN_IS_POS(ebgn) 	(0 < EBGN_LEN(ebgn) && EC_TRUE == (ebgn)->sgn)     /* ebgn > 0 */

#define EBGN_IS_NEG(ebgn) 	(0 < EBGN_LEN(ebgn) && EC_FALSE == (ebgn)->sgn)    /* ebgn < 0 */

#define EBGN_LEN(ebgn)    ((ebgn)->len)

#define EBGN_GET_LEN(ebgn)	(EBGN_LEN(ebgn))

#define EBGN_SET_LEN(ebgn, length)	(EBGN_LEN(ebgn) = length)

#define EBGN_INC_LEN(ebgn)	(EBGN_LEN(ebgn) ++)

#define EBGN_DEC_LEN(ebgn)	(EBGN_LEN(ebgn) --)

#define EBGN_INIT(ebgn) do{ \
	 INIT_LIST_BASE_HEAD(EBGN_ITEM_HEAD(ebgn));\
	 EBGN_LEN(ebgn) = 0;\
	 EBGN_SGN(ebgn) = EC_TRUE;\
} while(0)

/* ------------------------------------------ function prototype definition ------------------------------------*/
/**
*   for test only
*
*   to query the status of EBGNZ Module
*
**/
void ebgn_z_print_module_status(const UINT32  ebgnz_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed EBGNZ module
*
*
**/
UINT32 ebgn_z_free_module_static_mem(const UINT32 ebgnz_md_id);
/**
*
* start EBGNZ module
*
**/
UINT32 ebgn_z_start( );

/**
*
* end EBGNZ module
*
**/
void ebgn_z_end(const UINT32 ebgnz_md_id);

/**
*
*   alloc a BIGINT type node from EBGNZ space
*
**/
UINT32 ebgn_z_alloc_bgn(const UINT32 ebgnz_md_id, BIGINT **ppbgn);

/**
*
*   alloc a EBGN_ITEM type node from EBGNZ space
*
**/
UINT32 ebgn_z_alloc_item(const UINT32 ebgnz_md_id,EBGN_ITEM **ppitem);

/**
*
*   alloc a EBGN type node from EBGNZ space
*
**/
UINT32 ebgn_z_alloc_ebgn(const UINT32 ebgnz_md_id,EBGN **ppebgn);

/**
*
*   free the BIGINT type node from EBGNZ space
*
**/
UINT32 ebgn_z_free_bgn(const UINT32 ebgnz_md_id,BIGINT *bgn);

/**
*
*   free the EBGN_ITEM type node from EBGNZ space
*
**/
UINT32 ebgn_z_free_item(const UINT32 ebgnz_md_id,EBGN_ITEM *item);

/**
*
*   free the EBGN type node from EBGNZ space
*
**/
UINT32 ebgn_z_free_ebgn(const UINT32 ebgnz_md_id,EBGN *ebgn);

/**
*
*   destroy the whole ebgn,i.e., all its items but not the ebgn itself.
*   so, when return from this function, ebgn can be refered again without any item.
*
**/
UINT32 ebgn_z_clean(const UINT32 ebgnz_md_id, EBGN *ebgn);

/**
*
*   destory the whole ebgn,including its all items and itself.
*   so, when return from this function, ebgn cannot be refered any more
*
**/
UINT32 ebgn_z_destroy(const UINT32 ebgnz_md_id, EBGN *ebgn);

/**
*
*   ebgn_c = ebgn_a
*
**/
UINT32 ebgn_z_clone(const UINT32 ebgnz_md_id,const EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*   if ebgn = 0, return EC_TRUE
*   if ebgn != 0, return EC_FALSE
*
**/
EC_BOOL ebgn_z_is_zero(const UINT32 ebgnz_md_id, const EBGN *ebgn);

/**
*
*   if ebgn = 1, return EC_TRUE
*   if ebgn != 1, return EC_FALSE
*
**/
EC_BOOL ebgn_z_is_one(const UINT32 ebgnz_md_id, const EBGN *ebgn);

/**
*
*   if ebgn is odd, then return EC_TRUE
*   if ebgn is even, then return EC_FALSE
*
**/
EC_BOOL ebgn_z_is_odd(const UINT32 ebgnz_md_id, const EBGN *ebgn);

/**
*
*   set ebgn = 0
*
**/
UINT32 ebgn_z_set_zero(const UINT32 ebgnz_md_id,EBGN *ebgn);

/**
*
*   set ebgn = 1
*
**/
UINT32 ebgn_z_set_one(const UINT32 ebgnz_md_id,EBGN *ebgn);

/**
*
*   set ebgn = n
*
**/
UINT32 ebgn_z_set_word(const UINT32 ebgnz_md_id,const UINT32 n, EBGN *ebgn);

/**
*
*   set ebgn = n
*
**/
UINT32 ebgn_z_set_n(const UINT32 ebgnz_md_id,const BIGINT *n, EBGN *ebgn);

/**
*
*   set ebgn = 2 ^ e
*
**/
UINT32 ebgn_z_set_e(const UINT32 ebgnz_md_id,const UINT32 e, EBGN *ebgn);

/**
*
*   if ebgn_a = ebgn_b then return 0
*   if ebgn_a > ebgn_b then return 1
*   if ebgn_a < ebgn_b then return -1
*
**/
int ebgn_z_cmp(const UINT32 ebgnz_md_id,const EBGN *ebgn_a, const EBGN *ebgn_b);

/*
*
*   if abs(ebgn_a) = abs(ebgn_b) then return 0
*   if abs(ebgn_a) > abs(ebgn_b) then return 1
*   if abs(ebgn_a) < abs(ebgn_b) then return -1
*
**/
int ebgn_z_abs_cmp(const UINT32 ebgnz_md_id,const EBGN *ebgn_a, const EBGN *ebgn_b);

/**
*
*       move ebgn_a to ebgn_c, ebgn_c = ebgn_a, and ebgn_a return null
*
**/
UINT32 ebgn_z_move(const UINT32 ebgnz_md_id, EBGN *ebgn_a, EBGN *ebgn_c);

/**
*   if a = 0, then nbits = 0
*   else
*       let a = [+/-]SUM( a_i * 2 ^i, where i = 0..m where a_m > 0,i.e, a_m = 1 )
*   then    nbits = m + 1
*   return nbits
**/
UINT32 ebgn_z_get_nbits(const UINT32 ebgnz_md_id,const EBGN *ebgn_a, UINT32 *nbits);

/**
*  binary operation:
*
*  ebgn_c = abs(ebgn_a) & abs(ebgn_b)
*
*  note,
*       1. the sgn of ebgn_a or ebgn_b will be ignored
*       2. the sgn of ebgn_c will be TRUE
*       3. no shift out will be return or kept
*
**/
UINT32 ebgn_z_bit_and(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);

/**
*  binary operation:
*
*  ebgn_c = abs(ebgn_a) | abs(ebgn_b)
*
*  note,
*       1. the sgn of ebgn_a or ebgn_b will be ignored
*       2. the sgn of ebgn_c will be TRUE
*       3. no shift out will be return or kept
*
**/
UINT32 ebgn_z_bit_or(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);

/**
*  binary operation:
*
*  ebgn_c = abs(ebgn_a) ^ abs(ebgn_b)
*
*  note,
*       1. the sgn of ebgn_a or ebgn_b will be ignored
*       2. the sgn of ebgn_c will be TRUE
*       3. no shift out will be return or kept
*
**/
UINT32 ebgn_z_bit_xor(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);

/**
*  binary operation:
*
*  ebgn_c = NOT abs(ebgn_a) ( i.e, ebgn_c = ~ abs(ebgn_a) )
*
*  note,
*       1. the sgn of ebgn_a will be ignored
*       2. the sgn of ebgn_c will be TRUE
*       3. no shift out will be return or kept
*
**/
UINT32 ebgn_z_bit_not(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*   let ebgn_a = [+/-]SUM(a_i * 2^i, where i = 0..N, N is enough large, a_i = 0 or 1)
*   return a_{nthbit}
*
**/
UINT32 ebgn_z_get_bit(const UINT32 ebgnz_md_id,const EBGN *ebgn_a, const UINT32 nthbit);

/**
*   let ebgn_a = [+/-]SUM(a_i * 2^i, where a_i = 0 or 1)
*   then set a_{nthbit} = 1
*   note:
*       this function is able to deal with a = 0.
**/
UINT32 ebgn_z_set_bit(const UINT32 ebgnz_md_id, EBGN *ebgn_a, const UINT32 nthbit);

/**
*   let ebgn_a = [+/-]SUM(a_i * 2^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   then set a_{nthbit} = 0
*   note:
*       this function is able to deal with ebgn_a = 0.
**/
UINT32 ebgn_z_clear_bit(const UINT32 ebgnz_md_id, EBGN *ebgn_a, const UINT32 nthbit);

/**
*
*
*  ebgn_c = (ebgn_a >> nbits)
*
*  note,
*       1. the sgn will not be changed
*       2. no shift out will be return or kept
*
**/
UINT32 ebgn_z_shr(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, UINT32 nbits, EBGN *ebgn_c);

/**
*
*
*  ebgn_c = (ebgn_a << nbits)
*
*  note,
*       1. the sgn will not be changed
*
**/
UINT32 ebgn_z_shl(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, UINT32 nbits, EBGN *ebgn_c);

/**
*
*
*  ebgn_b = -ebgn_a
*
*  note, maybe address of ebgn_a = address of ebgn_b
*
**/
UINT32 ebgn_z_neg(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_b);

/**
*
*
*  ebgn_b = abs(ebgn_a)
*
*  note, maybe address of ebgn_a = address of ebgn_b
*
**/
UINT32 ebgn_z_abs(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_b);

/**
*
*   ebgn ++
*
**/
UINT32 ebgn_z_inc(const UINT32 ebgnz_md_id, EBGN *ebgn);

/**
*
*
*   ebgn --
*
**/
UINT32 ebgn_z_dec(const UINT32 ebgnz_md_id, EBGN *ebgn);

/**
*
*
*   ebgn_c = ebgn_a + ebgn_b
*
**/
UINT32 ebgn_z_add(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = ebgn_a - ebgn_b
*
**/
UINT32 ebgn_z_sub(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);


/**
*
*
*   ebgn_c = ebgn_a * bgn_b
*
**/
UINT32 ebgn_z_smul(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const BIGINT *bgn_b, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = ebgn_a * ebgn_b
*
**/
UINT32 ebgn_z_mul(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = ebgn_a ^ 2
*
**/
UINT32 ebgn_z_squ(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*
*  ebgn_c = ebgn_a mod 2 ^ nbits
*
**/
UINT32 ebgn_z_cut(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const UINT32 nbits, EBGN *ebgn_c);

/**
*
*
*  ebgn_a = ebgn_b * ebgn_q + ebgn_r
*
**/
UINT32 ebgn_z_div(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_q, EBGN *ebgn_r);

/**
*
*
*  ebgn_r = abs(ebgn_a) mod abs(ebgn_b)
*
**/
UINT32 ebgn_z_mod(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_r);

/**
*
*   let ebgn_a = ebgn_q * 2^e where ebgn_q is odd and e >=0
*   return e
*
**/
UINT32 ebgn_z_get_nevens(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, UINT32 *e);

/**
*
*   let ebgn_a = ebgn_q * 2^e where ebgn_q is odd and e >=0
*
**/
UINT32 ebgn_z_evens(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, UINT32 *e, EBGN *ebgn_q);

/**
*       c = ( a ^ e )
*
**/
UINT32 ebgn_z_sexp(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const UINT32 e, EBGN *ebgn_c);

/**
*       c = ( a ^ e )
*
**/
UINT32 ebgn_z_exp(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const BIGINT *bgn_e, EBGN *ebgn_c);

/**
*
*   return ebgn_c = GCD( abs(ebgn_a), abs(ebgn_b) )
*
*   use plain Euclidean algorithm for GCD
*
**/
UINT32 ebgn_z_gcdeucl(const UINT32 ebgnz_md_id,const EBGN *ebgn_a,const EBGN *ebgn_b,EBGN *ebgn_c);

/**
*
*   return ebgn_d = GCD( abs(ebgn_a), abs(ebgn_b) ) and ebgn_u, ebgn_v such that
*       ebgn_a * ebgn_u + ebgn_b * ebgn_v = ebgn_d
*       ( a * u + b * v = d )
*
*   uses binary GCD extented algorithm
*
**/
UINT32 ebgn_z_gcdext(const UINT32 ebgnz_md_id,const EBGN *ebgn_a,const EBGN *ebgn_b,EBGN *ebgn_d, EBGN *ebgn_u, EBGN *ebgn_v);

/**
*
*   return ebgn_c = GCD( abs(ebgn_a), abs(ebgn_b) )
*
*   uses binary GCD algorithm
*
**/
UINT32 ebgn_z_gcd(const UINT32 ebgnz_md_id,const EBGN *ebgn_a,const EBGN *ebgn_b,EBGN *ebgn_c);

/**
*
*   Chinese Remainder Theorem.
*
*   return x such that
*       x = x1 mod m1
*       x = x2 mod m2
*   and 0 <= x < m where m = m1*m2
*
**/
UINT32 ebgn_z_crt(const UINT32 ebgnz_md_id,const EBGN *ebgn_x1, const EBGN *ebgn_m1,const EBGN *ebgn_x2, const EBGN *ebgn_m2,EBGN *ebgn_x, EBGN *ebgn_m);

/**
*   Squroot Ceil of a
*
*   return c such that c^2 <= a < (c+1)^2
*
**/
UINT32 ebgn_z_sqrt_ceil(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);

#endif /*_EBGNZ_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
