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

#ifndef _LIB_EBGNZ_H
#define _LIB_EBGNZ_H

#include "lib_type.h"

#define EBGN_SGN(ebgn)    	((ebgn)->sgn)

/* ------------------------------------------ function prototype definition ------------------------------------*/
/**
*   for test only
*
*   to query the status of EBGNZ Module
*
**/
void print_ebgn_z_status(LOG *log);

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
*
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
UINT32 ebgn_z_shr(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const UINT32 nbits, EBGN *ebgn_c);

/**
*
*
*  ebgn_c = (ebgn_a << nbits)
*
*  note,
*       1. the sgn will not be changed
*
**/
UINT32 ebgn_z_shl(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, const UINT32 nbits, EBGN *ebgn_c);

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
*   let ebgn_a = ebgn_q * 2^e where ebgn_q is odd and e >=0
*
**/
UINT32 ebgn_z_evens(const UINT32 ebgnz_md_id, const EBGN *ebgn_a, UINT32 *e, EBGN *ebgn_q);

#endif /*_LIB_EBGNZ_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
