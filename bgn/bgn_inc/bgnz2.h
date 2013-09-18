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

#ifndef _BGNZ2_H
#define _BGNZ2_H

#include "type.h"

/* BGNZ2 MODULE Defintion: */
typedef struct
{
    UINT32 usedcounter ;/* used counter >= 0 */

    UINT32 bgnz_md_id;
}BGNZ2_MD;


/**
*   for test only
*
*   to query the status of BGNZ2 Module
*
**/
void bgn_z2_print_module_status(const UINT32 bgnz2_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed BGNZ2 module
*
*
**/
UINT32 bgn_z2_free_module_static_mem(const UINT32 bgnz2_md_id);

/**
*
* start BGNZ2 module
*
**/
UINT32 bgn_z2_start();

/**
*
* end BGNZ2 module
*
**/
void bgn_z2_end(const UINT32 bgnz2_md_id);

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int bgn_z2_cmp(const UINT32 bgnz2_md_id,const BIGINT * a,const BIGINT *b);

/**
*   clone src to des
*   return des where des = src
**/
void bgn_z2_clone(const UINT32 bgnz2_md_id,const BIGINT * src,BIGINT * des);

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_z2_is_zero(const UINT32 bgnz2_md_id,const BIGINT* src);

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_z2_is_one(const UINT32 bgnz2_md_id,const BIGINT* src);

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_z2_is_odd(const UINT32 bgnz2_md_id,const BIGINT *src);

/**
*
*   set a = 0
**/
void bgn_z2_set_zero(const UINT32 bgnz2_md_id,BIGINT * a);

/**
*
*   set a = 1
**/
void bgn_z2_set_one(const UINT32 bgnz2_md_id,BIGINT * a);

/**
*
*   set a = n
**/
void bgn_z2_set_word(const UINT32 bgnz2_md_id,BIGINT *a,const UINT32 n);

/**
*   if k = 0, then return s[ 0 ] = 0 and 0
*   if k > 0, then
*       let k = SUM( k_i * 2 ^ i, i = 0..n, k_i takes 0 or 1, and k_n = 1 )
*   return s =[ k_0,...,k_n ] and n
*   i.e,
*       s[ 0 ] = k_0,... s[ n ] = k_n
*
**/
UINT32 bgn_z2_parse_bits(const UINT32 bgnz2_md_id,const BIGINT *k,UINT32 *s);

/**
**   Let the NAF representative of k be
*       k = SUM ( s_i * 2 ^ i, where s_i belong to {1,0,-1} and i = 0..n )
*   Then return s = [ s_0,...,s_n ] and n
*   i.e,
*       s[ 0 ] = s_0,... s[ n ] = s_n
*
**/
int bgn_z2_naf(const UINT32 bgnz2_md_id,const BIGINT *k,int *s);

/**
*
*   return deg(a)
*
*   let a = SUM(a_i * x^i, i =0..m) over Z_2[x]
*   if a  = 0, then return ( 0 )
*   if a != 0, then return ( m )
**/
UINT32 bgn_z2_deg(const UINT32 bgnz2_md_id,const BIGINT *a);

/**
*   return e = x ^ nth over Z_2[x]
*   where nth = 0,1,...
*
**/
void bgn_z2_set_e(const UINT32 bgnz2_md_id,BIGINT *e,const UINT32 nth);

/**
*
*   let a = SUM(a_i * x^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   return a_{nthbit}
*
**/
UINT32 bgn_z2_get_bit(const UINT32 bgnz2_md_id,const BIGINT *a, const UINT32 nthbit);

/**
*   let a = SUM(a_i * x^i, where i = 0..BIGINTSIZE - 1,a_i = 0 or 1)
*   then set a_{nthbit} = 1
*   note:
*       this function is able to deal with a = 0.
**/
void  bgn_z2_set_bit(const UINT32 bgnz2_md_id,BIGINT *a, const UINT32 nthbit);

/**
*   let a = SUM(a_i * x^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   then set a_{nthbit} = 0
*   note:
*       this function is able to deal with a = 0.
**/
void  bgn_z2_clear_bit(const UINT32 bgnz2_md_id,BIGINT *a, const UINT32 nthbit);

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*   let a0 = a mod x ^{WORDSIZE}
*
*   return c = ( a - a0 ) / (x ^ {BIGINTSIZE}) and a0 is returned.
*   hence a = ( c *  x ^ {BIGINTSIZE} ) + a0
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_dshr_onewordsize(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,BIGINT *c0,BIGINT *c1);

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*   let a0 = a mod x ^{nbits}
*
*   return c = ( a - a0 ) / (x ^ {nbits}) and a0 is returned.
*   hence a = ( c *  x ^ {nbits} ) + a0
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_dshr_lesswordsize(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1);

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*   let a0 = a mod x ^{nbits}
*
*   return c = ( a - a0 ) / (x ^ {nbits}) and a0 is NOT returned.
*   where 0 <= nbits
*
*   Note:
*       1. address of c not equal to address of a
*
**/
void bgn_z2_dshr_no_shift_out(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1);

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*
*   return c =  a  * x ^ {WORDSIZE} mod x ^{2 * BIGINTSIZE}
*   and the shifted out part is returned.
*   hence a  * x ^ {WORDSIZE} = shift_out * x ^( 2 * BIGINTSIZE ) + c
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_dshl_onewordsize(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,BIGINT *c0,BIGINT *c1);

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*
*   return c = ( a * x ^ {bnits} ) mod x ^( 2 * BIGINTSIZE )
*   and the shifted out part is returned.
*   where nbits < WORDSIZE
*
*   hence ( a * x ^ {bnits} ) = shift_out * x ^( 2 * BIGINTSIZE ) + c
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_dshl_lesswordsize(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1);

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*
*   return c = ( a * x ^ {bnits} ) mod x ^( 2 * BIGINTSIZE )
*   and the shifted out part is NOT returned.
*   where 0 <= nbits
*
*   Note:
*       1. address of c not equal to address of a
*
**/
void bgn_z2_dshl_no_shift_out(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1);

/**
*   let a0 = a mod x ^{WORDSIZE} over Z_2[x]
*
*   return c = ( a  - a0 ) / ( x ^ {WORDSIZE} ) over Z_2[x]
*   and the shifted out a0 is returned.
*
*   hence a = c * x ^{WORDSIZE} + a0
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_sshr_onewordsize(const UINT32 bgnz2_md_id,const BIGINT *a,BIGINT *c);

/**
*   let a0 = a mod x ^{nbits} over Z_2[x]
*
*   return c = ( a  - a0 ) / ( x ^ {nbits} ) over Z_2[x]
*   and the shifted out a0 is returned.
*
*   hence a = c * x ^{nbits} + a0
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_sshr_lesswordsize(const UINT32 bgnz2_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c);

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
void bgn_z2_sshr_no_shift_out(const UINT32 bgnz2_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c);

/**
*
*   return c = ( a  * x ^ {WORDSIZE} ) mod x ^{BIGINTSIZE} over Z_2[x]
*   and the shifted out part is returned.
*
*   hence ( a  * x ^ {WORDSIZE} ) = shift_out * x ^( BIGINTSIZE ) + c
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_sshl_onewordsize(const UINT32 bgnz2_md_id,const BIGINT * a, BIGINT * c);

/**
*
*   return c = ( a  * x ^ {nbits} ) mod x ^{BIGINTSIZE} over Z_2[x]
*   and the shifted out part is returned.
*
*   hence ( a  * x ^ {nbits} ) = shift_out * x ^( BIGINTSIZE ) + c
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_sshl_lesswordsize(const UINT32 bgnz2_md_id,const BIGINT * a, const UINT32 nbits, BIGINT * c);

/**
*
*   return c = ( a  * x ^ {nbits} ) mod x ^{BIGINTSIZE} over Z_2[x]
*   and the shifted out part is NOT returned.
*
*   Note:
*       1. address of c not equal to address of a
*
**/
void bgn_z2_sshl_no_shift_out(const UINT32 bgnz2_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c);

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a + b over Z_2[x]
*
**/
void bgn_z2_add(const UINT32 bgnz2_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c);

/**
*       c = a + b over Z_2[x]
*       where b < 2 ^ WORDSIZE
**/
void bgn_z2_sadd(const UINT32 bgnz2_md_id,const BIGINT *a,const UINT32 b, BIGINT *c );

/**
*       let a = (a1, a0) = a1 * x^ {BIGINTSIZE} + a0
*           c = (c1, c0) = c1 * x^ {BIGINTSIZE} + c0
*
*       return c = ( a + b ) over Z_2[x]
*
**/
void bgn_z2_dadd(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b, BIGINT *c0, BIGINT *c1 );

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a - b over Z_2[x]
*
**/
void bgn_z2_sub(const UINT32 bgnz2_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c);

/**
*       c = ( a - b ) over Z_2[x]
*       where b < 2 ^ WORDSIZE
**/
void bgn_z2_ssub(const UINT32 bgnz2_md_id,const BIGINT *a,const UINT32 b, BIGINT *c );

/**
*       let a = (a1, a0) = a1 * x^ {BIGINTSIZE} + a0
*           c = (c1, c0) = c1 * x^ {BIGINTSIZE} + c0
*
*       return c = ( a - b ) over Z_2[x]
*
**/
void bgn_z2_dsub(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b, BIGINT *c0, BIGINT *c1 );

/**
*       return c = a + b * x^( offset ) over Z_2[x]
*                = a ^ ( b << ofset) over Z
*       where
*           offset > 0
*       and
*           deg(a) < BIGINTSIZE
*       and
*           deg(b) + offset < BIGINTSIZE
*
**/
void bgn_z2_add_offset(const UINT32 bgnz2_md_id,const BIGINT *a,const BIGINT *b,const UINT32 offset,BIGINT *c);

/**
*   deg (a) < BIGINTSIZE
*
*   c = a ^ 2 over Z_2[x]
*
**/
void bgn_z2_squ(const UINT32 bgnz2_md_id,const BIGINT* a,BIGINT * c0,BIGINT *c1);

/**
*   precompute a table for bgn_z2_mul
*
*   deg (a) < n and a is nonzero over Z_2[x]
*
*   let B = { b over Z_2[x] : deg(b) < WINWIDTH_OF_BGNZ2_PREMUL }
*   construct a table
*       { table[i]: i = 0..MAX_SIZE_OF_BGNZ_2N_PREMUL_TABLE - 1 }
*    =  { a * b over Z_2[x], where b run through the set B }
*   and assume
*       table[b] = a * b over Z_2[x]
*
**/
void bgn_z2_premul(const UINT32 bgnz2_md_id,const BIGINT * a);

/**
*   deg (a) < BIGINTSIZE
*   deg (b) < BIGINTSIZE
*
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0
*
*   return c = a * b over Z_2[x]
*
**/
void bgn_z2_mul(const UINT32 bgnz2_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c0,BIGINT *c1);

/**
*   deg (a) < BIGINTSIZE and a is nonzero
*   deg (b) < BIGINTSIZE and b is nonzero
*
*   return (q,r) where
*       a = b * q + r over Z_2[x] with deg( r ) < deg(b)
*
**/
void bgn_z2_div(const UINT32 bgnz2_md_id,const BIGINT* a,const BIGINT * b,BIGINT * q,BIGINT *r);

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x] and a be nonzero
*   let q = (q1,q0) = q1 * x ^ {BIGINTSIZE} + q0 over Z_2[x] and q be nonzero
*
*   a = b * q + r over Z_2 with deg( r ) < deg(b)
*
**/
void bgn_z2_ddiv_1(const UINT32 bgnz2_md_id,const BIGINT* a0,const BIGINT* a1,const BIGINT * b,BIGINT * q0,BIGINT * q1,BIGINT *r);

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x] and a be nonzero
*   let q = (q1,q0) = q1 * x ^ {BIGINTSIZE} + q0 over Z_2[x] and q be nonzero
*
*   a = b * q + r over Z_2[x] with deg( r ) < deg(b)
*
**/
void bgn_z2_ddiv_2(const UINT32 bgnz2_md_id,const BIGINT* a0,const BIGINT* a1,const BIGINT * b,BIGINT * q0,BIGINT * q1,BIGINT *r);

/**
*   let a = (a1,a0) = a1 * 2 ^ BIGINTSIZE + a0 and a be nonzero
*   let q = (q1,q0) = q1 * 2 ^ BIGINTSIZE + q0 and q be nonzero
*
*   a = b * q + r over Z_2 with deg( r ) < deg(b)
**/
void bgn_z2_ddiv(const UINT32 bgnz2_md_id,const BIGINT* a0,const BIGINT* a1,const BIGINT * b,BIGINT * q0,BIGINT * q1,BIGINT *r);

#endif /* _BGNZ2_H  */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

