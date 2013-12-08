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

#ifndef _BGNZN_H
#define _BGNZN_H

#include "type.h"

/* BGNZN MODULE Defintion: */
typedef struct
{
    UINT32 usedcounter    ;/* used counter >= 0 */

    UINT32 bgnz_md_id;

    BIGINT *bgnzn_n   ;
    BIGINT *bgnzn_res ; /* bgnzn_res = 2^{BIGINTSIZE} mod n. note here: maybe bgnzn_res > n*/
}BGNZN_MD;

/**
*   for test only
*
*   to query the status of BGNZN Module
*
**/
void bgn_zn_print_module_status(const UINT32 bgnzn_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed BGNZN module
*
*
**/
UINT32 bgn_zn_free_module_static_mem(const UINT32 bgnzn_md_id);

/**
*
* start BGNZN module
*
**/
UINT32 bgn_zn_start( const BIGINT *n );

/**
*
* end BGNZN module
*
**/
void bgn_zn_end(const UINT32 bgnzn_md_id);

/**
*   clone src to des
*   return des where des = src
**/
void bgn_zn_clone(const UINT32 bgnzn_md_id,const BIGINT * src,BIGINT * des);

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int bgn_zn_cmp(const UINT32 bgnzn_md_id,const BIGINT * a,const BIGINT *b);

/**
*
*   set a = 0
**/
void bgn_zn_set_zero(const UINT32 bgnzn_md_id,BIGINT * a);

/**
*
*   set a = 1
**/
void bgn_zn_set_one(const UINT32 bgnzn_md_id,BIGINT * a);

/**
*
*   set a = n
**/
void bgn_zn_set_word(const UINT32 bgnzn_md_id,BIGINT *a,const UINT32 n);

/**
*   return e = 2 ^ nth mod n
*            = ( 1 << nth ) mod n
*   where nth = 0,1,...,{BIGINTSIZE - 1}
*
**/
void bgn_zn_set_e(const UINT32 bgnzn_md_id,BIGINT *e,const UINT32 nth);

/**
*
*   set a =  2^ {BIGINTSIZE} - 1 mod n
**/
void bgn_zn_set_max(const UINT32 bgnzn_md_id,BIGINT * a);

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_zn_is_zero(const UINT32 bgnzn_md_id,const BIGINT* src);

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_zn_is_one(const UINT32 bgnzn_md_id,const BIGINT* src);

/**
*
*   if src = n, then return EC_TRUE
*   if src !=n, then return EC_FALSE
*
**/
EC_BOOL bgn_zn_is_n(const UINT32 bgnzn_md_id,const BIGINT* src, const UINT32 n);

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_zn_is_odd(const UINT32 bgnzn_md_id,const BIGINT *src);

/**
*   let a belong to [0, n - 1], then
*       c = a / 2^{nbits} mod n
*   where 0 <= nbits < BIGINTSIZE
*   return c
*
*   maybe address of c = address of a
**/
EC_BOOL bgn_zn_shr_lessbgnsize(const UINT32 bgnzn_md_id,const BIGINT *a,const UINT32 nbits,BIGINT *c);

/**
*   let a belong to [0, n - 1], then
*       c = ( a << WORDSIZE ) mod n
*   return c
*
**/
void bgn_zn_shl_onewordsize(const UINT32 bgnzn_md_id,const BIGINT * a, BIGINT * c);

/**
*   let a belong to [0, n - 1], then
*       c = ( a << nbits ) mod n
*   return c
*
**/
void bgn_zn_shl_lesswordsize(const UINT32 bgnzn_md_id,const BIGINT * a, const UINT32 nbits, BIGINT * c);

/**
**   Let the NAF representative of k be
*       k = SUM ( s_i * 2 ^ i, where s_i belong to {1,0,-1} and i = 0..n )
*   Then return s = [ s_0,...,s_n ] and n
*   i.e,
*       s[ 0 ] = s_0,... s[ n ] = s_n
*
**/
int bgn_zn_naf(const UINT32 bgnzn_md_id,const BIGINT *k,int *s);

/**
*       c = ( a + b ) mod n
*       where a < n and b < n
*
**/
void bgn_zn_add(const UINT32 bgnzn_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c );

/**
*       c = ( a - b ) mod n
*       where a < n and b < n
*
**/
void bgn_zn_sub(const UINT32 bgnzn_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c );

/**
*       c = ( -a ) mod n
*       where a < n
*
**/
void bgn_zn_neg(const UINT32 bgnzn_md_id,const BIGINT *a,BIGINT *c );

/**
*       c = ( a * b ) mod n
*       where a < n and b < n
*
**/
void bgn_zn_mul(const UINT32 bgnzn_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c );

/**
*       c = ( a * b ) mod n
*       where a < n and b < n
*
**/
void bgn_zn_smul(const UINT32 bgnzn_md_id,const BIGINT *a,const UINT32 b,BIGINT *c );
/**
*       c = ( a ^ 2 ) mod n
*       where a < n
*
**/
void bgn_zn_squ(const UINT32 bgnzn_md_id,const BIGINT *a,BIGINT *c );

/**
*       c = ( a ^ e ) mod n
*       where 0 < a < n and e < 2 ^ WORDSIZE
*
**/
void bgn_zn_sexp(const UINT32 bgnzn_md_id,const BIGINT *a,const UINT32 e,BIGINT *c );

/**
*       c = ( a ^ e ) mod n
*       where 0 < a < n and e < 2 ^ BIGINTSIZE
*
**/
void bgn_zn_exp(const UINT32 bgnzn_md_id,const BIGINT *a,const BIGINT *e,BIGINT *c );

/**
*
*   if a = 0 , then return EC_FALSE
*   if n > a > 0 and GCD(a,n) > 1, then return EC_FALSE
*   if n > a > 0 and GCD(a,n) = 1, then return EC_TRUE and
*       c = ( 1 / a ) mod n
*   where 0 < a < n
*
**/
EC_BOOL bgn_zn_inv(const UINT32 bgnzn_md_id,const BIGINT *a,BIGINT *c );

/**
*
*   c = a / b mod n
*   if GCD(b,n) > 1, then return FALSE
*   else return c = a *(b^(-1)) mod n
*
**/
EC_BOOL bgn_zn_div(const UINT32 bgnzn_md_id,const BIGINT *a, const BIGINT *b, BIGINT *c );

#endif /* _BGNZN_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

