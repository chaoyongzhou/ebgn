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

#ifndef _ECF2N_H
#define _ECF2N_H

#include "type.h"

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    /* BGNF2N Module ID refered by ECF2N Module */
    UINT32 bgnf2n_md_id;

    /* n represent of degree of f_x where F_{2^n} = Z_2[x]/(f_x) */
    UINT32  ecf2n_n;

    /* f represent the generator of F_{2^n} = Z_2[x]/(f_x) */
    BIGINT *ecf2n_f;

    /* coefficients of the elliptic curve(ec) */
    ECF2N_CURVE *ecf2n_curve;

    /* the order of the elliptic curve(ec) */
    BIGINT *ecf2n_order;

    /* the base point of the elliptic curve(ec) */
    EC_CURVE_POINT *ecf2n_base_point;
}ECF2N_MD;

/**
*   for test only
*
*   to query the status of ECF2N Module
*
**/
void ec_f2n_print_module_status(const UINT32 ecf2n_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed ECF2N module
*
*
**/
UINT32 ec_f2n_free_module_static_mem(const UINT32 ecf2n_md_id);

/**
*
* start ECF2N module
* ec defined as y ^ 2 + x * y = x ^ 3 + a * x ^ 2 + b
*
**/
UINT32 ec_f2n_start( const BIGINT *f_x, const ECF2N_CURVE *curve, const BIGINT *order, const EC_CURVE_POINT *base_point );

/**
*
* end ECF2N module
*
**/
void ec_f2n_end(const UINT32 ecf2n_md_id);

/**
*   define Point (0,0) as infinit point
*   if the point P is equal to (0,0), then return EC_TRUE
*   otherwise, return EC_FALSE
*
**/
EC_BOOL ec_f2n_point_is_infinit(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P);

/**
*
*   set point P = (0,0) as the infinit point
*   return the point P
*
**/
void ec_f2n_point_set_infinit(const UINT32 ecf2n_md_id, EC_CURVE_POINT * P);

/**
*
*   define affine Point [x:y:0] as infinit affine point
*   if the z of affine point P is zero, then return EC_TRUE
*   otherwise, return EC_FALSE
*
**/
UINT32 ec_f2n_point_aff_is_infinit(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P);

/**
*
*   set affine point P = [1:0:0] as the infinit affine point
*   return the affine point P
*
**/
void ec_f2n_point_aff_set_infinit(const UINT32 ecf2n_md_id, EC_CURVE_AFF_POINT * P);

/**
*
*   copy point src to des
*   return des
*
**/
void ec_f2n_point_clone(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * src,EC_CURVE_POINT * des);

/**
*
*   copy affine point src to des
*   return des
*
**/
void ec_f2n_point_aff_clone(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * src,EC_CURVE_AFF_POINT * des);

/**
*
*   if point curve_1  = curve_2, then return 0
*   if point curve_1 != curve_2, then return 1
*
**/
UINT32 ec_f2n_curve_cmp(const UINT32 ecf2n_md_id, const ECF2N_CURVE * curve_1,const ECF2N_CURVE * curve_2);

/**
*
*   if point P  = Q, then return 0
*   if point P != Q, then return 1
*
**/
UINT32 ec_f2n_point_cmp(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const EC_CURVE_POINT * Q);

/**
*
*   if affine point P  = Q, then return 0
*   if affine point P != Q, then return 1
*
**/
UINT32 ec_f2n_point_aff_cmp(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,const EC_CURVE_AFF_POINT * Q);

/**
*
*   check point is on the elliptic curve(ec) refered by Module ID md_id
*   if the point is on the elliptic curve(ec), return EC_TRUE,
*   otherwise, return EC_FALSE
*
**/
EC_BOOL ec_f2n_point_is_on_curve(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * point);

/**
*   let P = (x1,y1) be a point over ec
*   let negP = (x2,y2)= -P be a point over ec
*   return the negative point negP of P
*   the ec is refered by Module ID md_id
*
*   note:
*       x2 = x1
*       y2 = x1 + y1 over F_{2^n}
*
**/
void ec_f2n_point_neg(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,EC_CURVE_POINT * negP);

/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let Q = (x2,y2) be a point over ec
*   let R = [x3:y3:z3]
*   return R = P + Q over ec
*
*   note :
*       P address is not equal to R address
**/
void ec_f2n_point_mix_add(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,const EC_CURVE_POINT * Q,EC_CURVE_AFF_POINT * R);

/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let Q = (x2,y2) be a point over ec
*   let R = [x3:y3:z3]
*   return R = P - Q over ec
*
*   note :
*       P address is not equal to R address
**/
void ec_f2n_point_mix_sub(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,const EC_CURVE_POINT * Q,EC_CURVE_AFF_POINT * R);

/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let R = [x3:y3:z3]
*   return R = 2 * P over ec
*
*   note :
*       P address is not equal to R address
**/
void ec_f2n_point_aff_double(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,EC_CURVE_AFF_POINT * R);

/**
*   let P = (x1,y1) be an affine point over ec
*   let R = [x3:y3:z3]
*   return R = P over ec
*   where
*       x3 = x1
*       y3 = y1
*       z3 = 1
*
*   note :
*       P address is not equal to R address
**/
void ec_f2n_point_convert(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,EC_CURVE_AFF_POINT * R);

/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let R = (x3,y3)
*   return R = P over ec
*   where
*       x3 = x1 / (z1)
*       y3 = y1 / (z1 ^ 2)
*
*   note :
*       P address is not equal to R address
**/
void ec_f2n_point_aff_convert(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,EC_CURVE_POINT * R);

/**
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
**/
void ec_f2n_point_mul_naf(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const BIGINT * k,EC_CURVE_POINT * R);

/**
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
*   here, basepoint is fixed, so contruct a table exp_basepoint_tbl:
*       {r * basepoint: r = 2 ^ i, i = 0.. ec_f2n_n = deg(f(x)) }
**/
void ec_f2n_point_mul_fix_base(const UINT32 ecf2n_md_id,  const BIGINT * k,EC_CURVE_POINT * R);

/**
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
*   here, if the point P is basepoint, then use the basepoint-fixed algorithm,
*   otherwise, use the NAF algorithm
*
**/
void ec_f2n_point_mul(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const BIGINT * k,EC_CURVE_POINT * R);

/**
*   let P = (x1,y1) be a point over ec
*   let Q = (x2,y2) be a point over ec
*   let R = (x3,y3)
*   return R = P + Qover ec
*
**/
void ec_f2n_point_add(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const EC_CURVE_POINT * Q,EC_CURVE_POINT * R);

/**
*   let P = (x1,y1) be a point over ec
*   let R = (x3,y3)
*   return R = 2 * P over ec
*
**/
void ec_f2n_point_double(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,EC_CURVE_POINT * R);

/**
*   let P = (x1,y1) be a point over ec
*   let Q = (x2,y2) be a point over ec
*   let R = (x3,y3)
*   return R = P - Q over ec
*
**/
void ec_f2n_point_sub(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const EC_CURVE_POINT * Q,EC_CURVE_POINT * R);

#if ( SWITCH_ON == ECC_PM_CTRL_SWITCH )
void ec_f2n_point_mul_M_ary_Pre(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,EC_CURVE_POINT *buf,UINT32 bufsize);
void ec_f2n_point_aff_doubles(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,const UINT32 doubles,EC_CURVE_AFF_POINT * R);
void ec_f2n_point_mul_M_ary(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const BIGINT * k,EC_CURVE_POINT * R);
#endif /* ECC_PM_CTRL_SWITCH */

#endif /*_ECF2N_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

