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

#ifndef _BGNF2N_H
#define _BGNF2N_H

#include "type.h"

/* BGNF2N MODULE Defintion: */
typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    /* BGNZ2 Module ID refered by BGNF2N Module */
    UINT32 bgnz2_md_id;

    /* n represent of degree of f_x where F_{2^n} = Z_2[x]/(f_x) */
    UINT32  bgnf2n_n   ;

    /*f_x represent the generator of F_{2^n} = Z_2[x]/(f_x) */
    BIGINT *bgnf2n_f   ;
}BGNF2N_MD;


/**
*   for test only
*
*   to query the status of BGNF2N Module
*
**/
void bgn_f2n_print_module_status(const UINT32 bgnf2n_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed BGNF2N module
*
*
**/
UINT32 bgn_f2n_free_module_static_mem(const UINT32 bgnf2n_md_id);

/**
*
* start BGNF2N module
*
**/
UINT32 bgn_f2n_start( const BIGINT *f_x );

/**
*
* end BGNF2N module
*
**/
void bgn_f2n_end(const UINT32 bgnf2n_md_id);

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int bgn_f2n_cmp(const UINT32 bgnf2n_md_id,const BIGINT * a,const BIGINT *b);

/**
*   clone src to des
*   return des where des = src
**/
void bgn_f2n_clone(const UINT32 bgnf2n_md_id,const BIGINT * src,BIGINT * des);

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_f2n_is_zero(const UINT32 bgnf2n_md_id,const BIGINT* src);

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_f2n_is_one(const UINT32 bgnf2n_md_id,const BIGINT* src);

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_f2n_is_odd(const UINT32 bgnf2n_md_id,const BIGINT *src);

/**
*
*   set a = 0
**/
void bgn_f2n_set_zero(const UINT32 bgnf2n_md_id,BIGINT * a);

/**
*
*   set a = 1
**/
void bgn_f2n_set_one(const UINT32 bgnf2n_md_id,BIGINT * a);

/**
*
*   set a = n
**/
void bgn_f2n_set_word(const UINT32 bgnf2n_md_id,BIGINT *a,const UINT32 n);

/**
*   if k = 0, then return s[ 0 ] = 0 and 0
*   if k > 0, then
*       let k = SUM( k_i * 2 ^ i, i = 0..n, k_i takes 0 or 1, and k_n = 1 )
*   return s =[ k_0,...,k_n ] and n
*   i.e,
*       s[ 0 ] = k_0,... s[ n ] = k_n
*
**/
UINT32 bgn_f2n_parse_bits(const UINT32 bgnf2n_md_id,const BIGINT *k,UINT32 *s);

/**
**   Let the NAF representative of k be
*       k = SUM ( s_i * 2 ^ i, where s_i belong to {1,0,-1} and i = 0..n )
*   Then return s = [ s_0,...,s_n ] and n
*   i.e,
*       s[ 0 ] = s_0,... s[ n ] = s_n
*
**/
int bgn_f2n_naf(const UINT32 bgnf2n_md_id,const BIGINT *k,int *s);

/**
*
*   return deg(a)
*
*   let a = SUM(a_i * x^i, i =0..m)
*   if a  = 0, then return ( 0 )
*   if a != 0, then return ( m )
**/
UINT32 bgn_f2n_deg(const UINT32 bgnf2n_md_id,const BIGINT *a);

/**
*
*   let a belong to F_{2^n} = Z_2[x]/f(x)
*   let a0 = a mod x ^{WORDSIZE}
*   return c = ( a - a0 ) /( x ^{WORDSIZE} ) ) over F_{2^n}  = ( a - a0 ) /( x ^{WORDSIZE} ) )
*
*   note:
*       maybe address of c = address of a
*
**/
void bgn_f2n_shr_onewordsize(const UINT32 bgnf2n_md_id,const BIGINT *a,BIGINT *c);

/**
*
*   let a belong to F_{2^n} = Z_2[x]/f(x)
*   let a0 = a mod x ^{nbits}
*   return c = ( a - a0 ) /( x ^{nbits} ) ) over F_{2^n} = ( a - a0 ) /( x ^{nbits} ) )
*
*   note:
*       maybe address of c = address of a
*
**/
void bgn_f2n_shr_lesswordsize(const UINT32 bgnf2n_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c);

/**
*   let a belong to F_{2^n} = Z_2[x]/f(x)
*   return c = ( a * x ^{WORDSIZE} ) over F_{2^n} = ( a * x ^{WORDSIZE} ) mod f(x)
*
*   note:
*       maybe address of c = address of a
**/
void bgn_f2n_shl_onewordsize(const UINT32 bgnf2n_md_id,const BIGINT * a, BIGINT * c);

/**
*   return c = ( a * x ^{nbits} )
*
*   maybe address of c = address of a
**/
void bgn_f2n_shl_lesswordsize(const UINT32 bgnf2n_md_id,const BIGINT * a, const UINT32 nbits, BIGINT * c);

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a + b over F_{2^n} = Z_2[x]/f(x)
*   i.e,
*   c = a + b mod f(x) over Z_2[x]
*
**/
void bgn_f2n_add(const UINT32 bgnf2n_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c);

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a - b over F_{2^n} = Z_2[x]/f(x)
*   i.e,
*   c = a - b mod f(x) over Z_2[x]
*
**/
void bgn_f2n_sub(const UINT32 bgnf2n_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c);

/**
*       return c = a + b * x^( offset )
*       Must
*           offset > 0
*       and
*           deg(a) < deg(f) < BIGINTSIZE
*       and
*           deg(b) + offset < deg(f) < BIGINTSIZE
*
**/
void bgn_f2n_add_offset(const UINT32 bgnf2n_md_id,const BIGINT *a,const BIGINT *b,const UINT32 offset,BIGINT *c);

/**
*   deg (a) < n
*
*   c = a ^ 2 over F_{2^n}, where F_{2^n} = Z_2[ x ]/ f(x)
*   i.e,
*   c = a ^ 2 mod f(x) over F2[x]
*
**/
void bgn_f2n_squ(const UINT32 bgnf2n_md_id,const BIGINT* a,BIGINT * c);


/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a * b over F_{2^n}, where F_{2^n} = Z_2[ x ]/ f(x)
*   i.e,
*   c = a * b mod f(x) over F2[x]
*
**/
void bgn_f2n_mul(const UINT32 bgnf2n_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c);

/**
*   a != 0 and deg (a) < n
*
*   c = a ^{-1} over F_{2^n} = Z_2[x]/f(x)
*   i.e,
*   c = a ^{-1} mod f(x) over Z_2[x]
*
*/
EC_BOOL bgn_f2n_inv(const UINT32 bgnf2n_md_id,const BIGINT* a,BIGINT * c);

#endif /* _BGNF2N_H  */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

