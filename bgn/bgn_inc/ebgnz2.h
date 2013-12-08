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

#ifndef _EBGNZ2_H
#define _EBGNZ2_H

#include "type.h"

/* EBGNZ2 MODULE Defintion: */
typedef struct
{
    UINT32 usedcounter ;/* used counter >= 0 */

    UINT32 ebgnz_md_id;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;
}EBGNZ2_MD;

/**
*   for test only
*
*   to query the status of EBGNZ2 Module
*
**/
void print_ebgn_z2_status();

/**
*
*   free all static memory occupied by the appointed EBGNZ2 module
*
*
**/
UINT32 ebgn_z2_free_module_static_mem(const UINT32 bgnz2_md_id);

/**
*
* start EBGNZ2 module
*
**/
UINT32 ebgn_z2_start();

/**
*
* end EBGNZ2 module
*
**/
void ebgn_z2_end(const UINT32 bgnz2_md_id);

/**
*
*   alloc a EBGN type node from EBGNZ2 space
*
**/
UINT32 ebgn_z2_alloc_ebgn(const UINT32 ebgnz2_md_id,EBGN **ppebgn);

/**
*
*   free the EBGN type node from EBGNZ2 space
*
**/
UINT32 ebgn_z2_free_ebgn(const UINT32 ebgnz2_md_id,EBGN *ebgn);

/**
*
*   destory the whole ebgn,i.e., all its items but not the ebgn itself.
*   so, when return from this function, ebgn can be refered again without any item.
*
**/
UINT32 ebgn_z2_clean(const UINT32 ebgnz2_md_id, EBGN *ebgn);

/**
*
*   destory the whole ebgn,including its all items and itself.
*   so, when return from this function, ebgn cannot be refered any more
*
**/
UINT32 ebgn_z2_destroy(const UINT32 ebgnz2_md_id, EBGN *ebgn);

/**
*   clone src to des
*   return des where des = src
**/
UINT32 ebgn_z2_clone(const UINT32 bgnz2_md_id,const EBGN * src,EBGN * des);

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int ebgn_z2_cmp(const UINT32 bgnz2_md_id,const EBGN * a,const EBGN *b);

/**
*
*       move ebgn_a to ebgn_c, ebgn_c = ebgn_a, and ebgn_a return null
*
**/
UINT32 ebgn_z2_move(const UINT32 ebgnz2_md_id, EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL ebgn_z2_is_zero(const UINT32 bgnz2_md_id,const EBGN * src);

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL ebgn_z2_is_one(const UINT32 bgnz2_md_id,const EBGN * src);

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL ebgn_z2_is_odd(const UINT32 bgnz2_md_id,const EBGN *src);

/**
*
*   set a = 0
**/
UINT32 ebgn_z2_set_zero(const UINT32 bgnz2_md_id,EBGN * a);

/**
*
*   set a = 1
**/
UINT32 ebgn_z2_set_one(const UINT32 bgnz2_md_id,EBGN * a);

/**
*
*   set ebgn = n
**/
UINT32 ebgn_z2_set_word(const UINT32 ebgnz2_md_id,const UINT32 n, EBGN *ebgn);

/**
*
*   set a = n
**/
UINT32 ebgn_z2_set_n(const UINT32 ebgnz2_md_id,const BIGINT *n, EBGN *a);

/**
*
*   return deg(a)
*
*   let a = SUM(a_i * x^i, i =0..m) over Z_2[x]
*   if a  = 0, then return ( 0 )
*   if a != 0, then return ( m )
**/
UINT32 ebgn_z2_deg(const UINT32 bgnz2_md_id,const EBGN *a);

/**
*   return e = x ^ nth over Z_2[x]
*   where nth = 0,1,...
*
**/
UINT32 ebgn_z2_set_e(const UINT32 bgnz2_md_id,EBGN *e,const UINT32 nth);

/**
*
*   let a = SUM(a_i * x^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   return a_{nthbit}
*
**/
UINT32 ebgn_z2_get_bit(const UINT32 bgnz2_md_id,const EBGN *a, const UINT32 nthbit);

/**
*   let a = SUM(a_i * x^i, where i = 0..EBGNZSIZE - 1,a_i = 0 or 1)
*   then set a_{nthbit} = 1
*   note:
*       this function is able to deal with a = 0.
**/
UINT32 ebgn_z2_set_bit(const UINT32 bgnz2_md_id,EBGN *a, const UINT32 nthbit);

/**
*   let a = SUM(a_i * x^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   then set a_{nthbit} = 0
*   note:
*       this function is able to deal with a = 0.
**/
UINT32 ebgn_z2_clear_bit(const UINT32 bgnz2_md_id,EBGN *a, const UINT32 nthbit);

/**
*   let a0 = a mod x ^{nbits} over Z_2[x]
*
*   return c = ( a  - a0 ) / ( x ^ {nbits} ) over Z_2[x]
*   and the shifted out a0 is NOT returned.
*   where 0 <= nbits
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 ebgn_z2_shr(const UINT32 bgnz2_md_id,const EBGN *a, const UINT32 nbits,EBGN *c);

/**
*
*   return c = ( a  * x ^ {nbits} ) mod x ^{EBGNZSIZE} over Z_2[x]
*   and the shifted out part is NOT returned.
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 ebgn_z2_shl(const UINT32 bgnz2_md_id,const EBGN *a, const UINT32 nbits,EBGN *c);

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a + b over Z_2[x]
*
**/
UINT32 ebgn_z2_add(const UINT32 bgnz2_md_id,const EBGN * a,const EBGN * b,EBGN * c);

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a - b over Z_2[x]
*
**/
UINT32 ebgn_z2_sub(const UINT32 bgnz2_md_id,const EBGN * a,const EBGN * b,EBGN * c);

/**
*       return c = a + b * x^( offset ) over Z_2[x]
*                = a ^ ( b << ofset) over Z
*       where
*           offset > 0
*       and
*           deg(a) < EBGNZSIZE
*       and
*           deg(b) + offset < EBGNZSIZE
*
**/
UINT32 ebgn_z2_add_offset(const UINT32 bgnz2_md_id,const EBGN *a,const EBGN *b,const UINT32 offset,EBGN *c);

/**
*   deg (a) < EBGNZSIZE
*
*   c = a ^ 2 over Z_2[x]
*
**/
UINT32 ebgn_z2_squ(const UINT32 bgnz2_md_id,const EBGN * a,EBGN * c);

/**
*   precompute a table for ebgn_z2_mul
*
*   deg (a) < n and a is nonzero over Z_2[x]
*
*   let B = { b over Z_2[x] : deg(b) < WINWIDTH_OF_EBGNZ2_PREMUL }
*   construct a table
*       { table[i]: i = 0..MAX_SIZE_OF_BGNZ_2N_PREMUL_TABLE - 1 }
*    =  { a * b over Z_2[x], where b run through the set B }
*   and assume
*       table[b] = a * b over Z_2[x]
*
**/
void ebgn_z2_premul(const UINT32 bgnz2_md_id,const EBGN * a);

/**
*   deg (a) < EBGNZSIZE
*   deg (b) < EBGNZSIZE
*
*   let c = (c1,c0) = c1 * x ^ {EBGNZSIZE} + c0
*
*   return c = a * b over Z_2[x]
*
**/
UINT32 ebgn_z2_mul(const UINT32 bgnz2_md_id,const EBGN * a,const EBGN * b,EBGN * c);

/**
*   deg (a) < EBGNZSIZE and a is nonzero
*   deg (b) < EBGNZSIZE and b is nonzero
*
*   return (q,r) where
*       a = b * q + r over Z_2[x] with deg( r ) < deg(b)
*
**/
UINT32 ebgn_z2_div(const UINT32 bgnz2_md_id,const EBGN * a,const EBGN * b,EBGN * q,EBGN *r);

#endif /* _EBGNZ2_H  */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

