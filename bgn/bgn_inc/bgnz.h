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

#ifndef _BGNZ_H
#define _BGNZ_H

#include "type.h"
#include "task.h"
#include "task.inc"

/* BGNZ MODULE Defintion: */
typedef struct
{
    UINT32 usedcounter;/* used counter >= 0 */

    MOD_MGR *mod_mgr;
}BGNZ_MD;


/**
*   for test only
*
*   to query the status of BGNZ Module
*
**/
void bgn_z_print_module_status(const UINT32 bgnz_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed BGNZ module
*
*
**/
UINT32 bgn_z_free_module_static_mem(const UINT32 bgnz_md_id);

/**
*
* start bgn_z module
*
**/
UINT32 bgn_z_start();

/**
*
* end bgn_z module
*
**/
void bgn_z_end(const UINT32 bgnz_md_id);

/**
*
* initialize mod mgr of BGNZ module
*
**/
UINT32 bgn_z_set_mod_mgr(const UINT32 bgnz_md_id, const MOD_MGR * src_task_mod_mgr);

/**
*
* get mod mgr of BGNZ module
*
**/
MOD_MGR * bgn_z_get_mod_mgr(const UINT32 bgnz_md_id);

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int bgn_z_cmp(const UINT32 bgnz_md_id,const BIGINT * a,const BIGINT *b);

/**
*   clone src to des
*   return des where des = src
**/
void bgn_z_clone(const UINT32 bgnz_md_id,const BIGINT * src,BIGINT * des);

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_z_is_zero(const UINT32 bgnz_md_id,const BIGINT* src);

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_z_is_one(const UINT32 bgnz_md_id,const BIGINT* src);

/**
*
*   if src = n, then return EC_TRUE
*   if src !=n, then return EC_FALSE
*
**/
EC_BOOL bgn_z_is_n(const UINT32 bgnz_md_id,const BIGINT* src, const UINT32 n);

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_z_is_odd(const UINT32 bgnz_md_id,const BIGINT *src);

/**
*
*   if src is short number,i.e, src < 2^32, then return EC_TRUE and src
*   if src is not short number,i.e, src >= 2^32, then return EC_FALSE only.
*
**/
EC_BOOL bgn_z_get_word(const UINT32 bgnz_md_id,const BIGINT *src, UINT32 *n);

/**
*   if a = 0, then return 0
*   if a > 0, then
*       let M = 2 ^ WORDSIZE
*       let a = SUM(a_i * M ^ i, i = 0..n, where a_n > 0 )
*   return ( n + 1 )
*
**/
UINT32 bgn_z_get_len(const UINT32 bgnz_md_id,const BIGINT *a);

/**
*
*   set a = n
**/
void bgn_z_set_word(const UINT32 bgnz_md_id,BIGINT *a, const UINT32 n);

/**
*
*   set a = 0
**/
void bgn_z_set_zero(const UINT32 bgnz_md_id,BIGINT * a);

/**
*
*   set a = 1
**/
void bgn_z_set_one(const UINT32 bgnz_md_id,BIGINT * a);

/**
*
*   set a =  2^ BIGINTSIZE - 1
**/
void bgn_z_set_max(const UINT32 bgnz_md_id,BIGINT * a);

/**
*   if k = 0, then return s[ 0 ] = 0 and 0
*   if k > 0, then
*       let k = SUM( k_i * 2 ^ i, i = 0..n, k_i takes 0 or 1, and k_n = 1 )
*   return s =[ k_0,...,k_n ] and n
*   i.e,
*       s[ 0 ] = k_0,... s[ n ] = k_n
*
**/
UINT32 bgn_z_parse_bits(const UINT32 bgnz_md_id,const BIGINT *k,UINT32 *s);

/**
**   Let the NAF representative of k be
*       k = SUM ( s_i * 2 ^ i, where s_i belong to {1,0,-1} and i = 0..n )
*   Then return s = [ s_0,...,s_n ] and n
*   i.e,
*       s[ 0 ] = s_0,... s[ n ] = s_n
*
**/
int bgn_z_naf(const UINT32 bgnz_md_id,const BIGINT *k,int *s);

/**
*   if a = 0, then nbits = 0
*   else
*       let a = SUM( a_i * 2 ^i, where i = 0..m where a_m > 0,i.e, a_m = 1 )
*   then    nbits = m + 1
*   return nbits
**/
UINT32 bgn_z_get_nbits(const UINT32 bgnz_md_id,const BIGINT *a);

/**
*
*   let a = SUM(a_i * 2^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   return a_{nthbit}
*
**/
UINT32 bgn_z_get_bit(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nthbit);

/**
*   let a = SUM(a_i * 2^i, where i = 0..BIGINTSIZE - 1,a_i = 0 or 1)
*   then set a_{nthbit} = 1
*   note:
*       this function is able to deal with a = 0.
**/
void  bgn_z_set_bit(const UINT32 bgnz_md_id,BIGINT *a, const UINT32 nthbit);

/**
*   let a = SUM(a_i * 2^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   then set a_{nthbit} = 0
*   note:
*       this function is able to deal with a = 0.
**/
void  bgn_z_clear_bit(const UINT32 bgnz_md_id,BIGINT *a, const UINT32 nthbit);

/**
*   return e = 2 ^ nth mod 2 ^ BIGINTSIZE
*            = ( 1 << nth ) mod 2 ^ BIGINTSIZE
*   where nth = 0,1,...
*
**/
void bgn_z_set_e(const UINT32 bgnz_md_id,BIGINT *e,const UINT32 nth);

/**
*   binary operation:
*       c = a AND b (i.e, a & b  )
**/
void bgn_z_bit_and(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c );

/**
*   binary operation:
*       c = a OR b (i.e, a | b )
**/
void bgn_z_bit_or(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c );

/**
*   binary operation:
*       c = a XOR b (i.e, a ^ b )
**/
void bgn_z_bit_xor(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c );

/**
*   binary operation:
*       c = NOT a (i.e, ~a )
**/
void bgn_z_bit_not(const UINT32 bgnz_md_id,const BIGINT *a, BIGINT *c );

/**
*   cut lower bit segment
*       c = a mod 2 ^ nbits
**/
void bgn_z_bit_cut(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits, BIGINT *c);

/**
*   return c = ( a >> WORDSIZE ) and the shifted out word is returned.
*   where a = ( c << WORDSIZE ) + shift_out
*   maybe address of c = address of a
**/
UINT32 bgn_z_shr_onewordsize(const UINT32 bgnz_md_id,const BIGINT *a,BIGINT *c);

/**
*   return c = ( a >> nbits ) and the shifted out word is returned.
*   where 0 <= nbits < WORDSIZE
*   where a = ( c << nbits ) + shift_out
*   maybe address of c = address of a
**/
UINT32 bgn_z_shr_lesswordsize(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c);

/**
*   return c = ( a >> nbits ) and no shifted out word is returned.
*   where 0 <= nbits
*   maybe address of c = address of a
**/
void bgn_z_shr_no_shift_out(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c);

/**
*   let a = (a1,a0) = a1 * 2 ^BIGINTSIZE + a0
*   let c = (c1,c0) = c1 * 2 ^BIGINTSIZE + c0
*   return c = ( a >> nbits ) and no shifted out word is returned.
*   where 0 <= nbits
*   maybe address of c = address of a
**/
void bgn_z_dshr_no_shift_out(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1);

/**
*   return c = ( a << WORDSIZE ) and the shifted out word is returned.
*   where ( a << WORDSIZE ) = shift_out * 2 ^( BIGINTSIZE ) + c
*   maybe address of c = address of a
**/
UINT32 bgn_z_shl_onewordsize(const UINT32 bgnz_md_id,const BIGINT *a, BIGINT *c);

/**
*   return c = ( a << nbits ) and the shifted out word is returned.
*   where ( a << bnits ) = shift_out * 2 ^( BIGINTSIZE ) + c
*   maybe address of c = address of a
**/
UINT32 bgn_z_shl_lesswordsize(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c);

/**
*   return c = ( a << nbits ) mod 2 ^ BIGINTSIZE
*          and no shifted out word is returned.
*   where 0 <= nbits
*   maybe address of c = address of a
**/
void bgn_z_shl_no_shift_out(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c);

/**
*   let a = (a1,a0) = a1 * 2 ^BIGINTSIZE + a0
*   let c = (c1,c0) = c1 * 2 ^BIGINTSIZE + c0
*
*   return c = ( a << nbits ) mod 2 ^ {2 *BIGINTSIZE}
*          and no shifted out word is returned.
*   where 0 <= nbits
*   maybe address of c = address of a
**/
void bgn_z_dshl_no_shift_out(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1);

/**
*       a + b = (carry, c) = carry * 2 ^ BIGINTSIZE + c
**/
UINT32 bgn_z_add(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c );

/**
*       a + b = ( carry, c ) = carry * 2 ^ BIGINTSIZE + c
*       where b < 2 ^ WORDSIZE
**/
UINT32 bgn_z_sadd(const UINT32 bgnz_md_id,const BIGINT *a,const UINT32 b, BIGINT *c );

/**
*       a + b + carry0 = (carry1, c) = carry1 * 2 ^ BIGINTSIZE + c
**/
UINT32 bgn_z_adc(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, const UINT32 carry, BIGINT *c );

/**
*       let a = (a1, a0) = a1 * 2^ BIGINTSIZE + a0
*           c = (c1, c0) = c1 * 2^ BIGINTSIZE + c0
*
*       return c such that a + b = ( carry, c  ) = carry * 2 ^ BIGINTSIZE + c
*
**/
UINT32 bgn_z_dadd(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b, BIGINT *c0, BIGINT *c1 );

/**
*       c = borrow * 2 ^ BIGINTSIZE + a - b
**/
UINT32 bgn_z_sub(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c );

/**
*       c = borrow * 2 ^ BIGINTSIZE + a - b
*       where b < 2 ^ WORDSIZE
**/
UINT32 bgn_z_ssub(const UINT32 bgnz_md_id,const BIGINT *a,const UINT32 b, BIGINT *c );

/**
*       ( -borrow1, c) =  a - b - borrow0
**/
UINT32 bgn_z_sbb(const UINT32 bgnz_md_id,const BIGINT *a, const BIGINT *b, const UINT32 borrow, BIGINT *c);

/**
*       let a = (a1, a0) = a1 * 2^ BIGINTSIZE + a0
*           c = (c1, c0) = c1 * 2^ BIGINTSIZE + c0
*
*       return c such that c = borrow * 2 ^ BIGINTSIZE + a - b
*
**/
UINT32 bgn_z_dsub(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b, BIGINT *c0, BIGINT *c1 );

/**
*
*   return c1 * 2^BIGINTSIZE+ c0 = a * b
*
**/
void bgn_z_mul(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c0,BIGINT *c1 );

/**
*
*   return c1 * 2^BIGINTSIZE+ c0 = a * b
*
**/
void bgn_z_smul(const UINT32 bgnz_md_id,const BIGINT *a,const UINT32 b, BIGINT *c0,BIGINT *c1 );

/**
*   return c1 * 2^BIGINTSIZE + c0 = a ^ 2
*
**/
void bgn_z_squ(const UINT32 bgnz_md_id,const BIGINT *a,BIGINT *c0,BIGINT *c1 );

/**
*       return (q, r) where a = b * q + r
*
**/
void bgn_z_div(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *q,BIGINT *r );

/**
*       let a = (a1, a0) = a1 * 2^ BIGINTSIZE + a0
*           q = (q1, q0) = q1 * 2^ BIGINTSIZE + q0
*
*       return (q, r) where a = b * q + r
*
**/
void bgn_z_ddiv(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b,BIGINT *q0,BIGINT *q1,BIGINT *r );

/**
*
*       return c = GCD(a,b)
*
**/
void bgn_z_gcd(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c);

/**
*
*   let a = q * 2^e where q is odd and e >=0
*
**/
void bgn_z_evens(const UINT32 bgnz_md_id,const BIGINT *a, UINT32 *e,BIGINT *q);

/**
*
*   return r = a mod b
*
**/
void bgn_z_mod(const UINT32 bgnz_md_id,const BIGINT *a, const BIGINT *b,BIGINT *r);

/**
*
*   Chinese Remainder Theorem
*
*   return x such that
*       x = x1 mod m1
*       x = x2 mod m2
*   where 0 <= x < m1 * m2
*
**/

void bgn_z_crt(const UINT32 bgnz_md_id,const BIGINT *x1, const BIGINT *m1,const BIGINT *x2, const BIGINT *m2,BIGINT *x, BIGINT *m);

/**
*   Squroot Ceil of a
*
*   return c such that c^2 <= a < (c+1)^2
*
**/
void bgn_z_sqrt_ceil(const UINT32 bgnz_md_id,const BIGINT *a, BIGINT *c);

#endif/* _BGNZ_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

