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
#ifndef _LIB_BGNFP_H
#define _LIB_BGNFP_H

#include "lib_type.h"

/**
*   for test only
*
*   to query the status of BGNFP Module
*
**/
void print_bgn_fp_status(LOG *log);

/**
*
* start BGNFP module
*
**/
UINT32 bgn_fp_start( const BIGINT *p );

/**
*
* end BGNFP module
*
**/
void bgn_fp_end(const UINT32 bgnfp_md_id);

/**
*   clone src to des
*   return des where des = src
**/
void bgn_fp_clone(const UINT32 bgnfp_md_id,const BIGINT * src,BIGINT * des);

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int bgn_fp_cmp(const UINT32 bgnfp_md_id,const BIGINT * a,const BIGINT *b);

/**
*
*   set a = 0
**/
void bgn_fp_set_zero(const UINT32 bgnfp_md_id,BIGINT * a);

/**
*
*   set a = 1
**/
void bgn_fp_set_one(const UINT32 bgnfp_md_id,BIGINT * a);

/**
*
*   set a = n
**/
void bgn_fp_set_word(const UINT32 bgnfp_md_id,BIGINT *a,const UINT32 n);

/**
*   return e = 2 ^ nth mod p
*            = ( 1 << nth ) mod p
*   where nth = 0,1,...,{BIGINTSIZE - 1}
*
**/
void bgn_fp_set_e(const UINT32 bgnfp_md_id,BIGINT *e,const UINT32 nth);

/**
*
*   set a =  2^ {BIGINTSIZE} - 1 mod p
**/
void bgn_fp_set_max(const UINT32 bgnfp_md_id,BIGINT * a);

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_fp_is_zero(const UINT32 bgnfp_md_id,const BIGINT* src);

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_fp_is_one(const UINT32 bgnfp_md_id,const BIGINT* src);

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_fp_is_odd(const UINT32 bgnfp_md_id,const BIGINT *src);

/**
*   let a belong to [0, n - 1], then
*       c = a / 2^{nbits} mod p
*   where 0 <= nbits < BIGINTSIZE
*   return c
*
*   maybe address of c = address of a
**/
EC_BOOL bgn_fp_shr_lessbgnsize(const UINT32 bgnfp_md_id,const BIGINT *a,const UINT32 nbits,BIGINT *c);

/**
*   let a belong to [0, p - 1], then
*       c = ( a << WORDSIZE ) mod p
*   return c
*
**/
void bgn_fp_shl_onewordsize(const UINT32 bgnfp_md_id,const BIGINT * a, BIGINT * c);

/**
*   let a belong to [0, p - 1], then
*       c = ( a << nbits ) mod p
*   return c
*
**/
void bgn_fp_shl_lesswordsize(const UINT32 bgnfp_md_id,const BIGINT * a, const UINT32 nbits, BIGINT * c);

/**
**   Let the NAF representative of k be
*       k = SUM ( s_i * 2 ^ i, where s_i belong to {1,0,-1} and i = 0..n )
*   Then return s = [ s_0,...,s_n ] and n
*   i.e,
*       s[ 0 ] = s_0,... s[ n ] = s_n
*
**/
int bgn_fp_naf(const UINT32 bgnfp_md_id,const BIGINT *k,int *s);

/**
*       c = ( a + b ) mod p
*       where a < p and b < p
*
**/
void bgn_fp_add(const UINT32 bgnfp_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c );

/**
*       c = ( a - b ) mod p
*       where a < p and b < p
*
**/
void bgn_fp_sub(const UINT32 bgnfp_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c );

/**
*       c = ( -a ) mod p
*       where a < p
*
**/
void bgn_fp_neg(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c );

/**
*       c = ( a * b ) mod p
*       where a < p and b < p
*
**/
void bgn_fp_mul(const UINT32 bgnfp_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c );

/**
*       c = ( a ^ 2 ) mod p
*       where a < p
*
**/
void bgn_fp_squ(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c );

/**
*       c = ( a ^ e ) mod p
*       where 0 < a < p and e < 2 ^ WORDSIZE
*
**/
void bgn_fp_sexp(const UINT32 bgnfp_md_id,const BIGINT *a,const UINT32 e,BIGINT *c );

/**
*       c = ( a ^ e ) mod p
*       where 0 < a < p and e < 2 ^ BIGINTSIZE
*
**/
void bgn_fp_exp(const UINT32 bgnfp_md_id,const BIGINT *a,const BIGINT *e,BIGINT *c );

/**
*
*   if a = 0 , then return EC_FALSE
*   if p > a > 0 and GCD(a,p) > 1, then return EC_FALSE
*   if p > a > 0 and GCD(a,p) = 1, then return EC_TRUE and
*       c = ( 1 / a ) mod p
*   where 0 < a < p
*
**/
EC_BOOL bgn_fp_inv(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c );

/**
*
*   assume p is odd prime and a belong to [ 0.. p -1 ]
*
*   return the legendre symbol (a/p)
*
**/
int bgn_fp_legendre(const UINT32 bgnfp_md_id,const BIGINT *a);

/**
*
*   if exist x such that
*           x^2 = a mod p
*   then
*	return EC_TRUE and c = x or -x over F_{p}
*
*   otherwise,
*	return EC_FALSE
*
**/
EC_BOOL bgn_fp_squroot(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c);
#endif /* _LIB_LIB_BGNFP_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/
