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

#ifndef _ECFP_H
#define _ECFP_H

#include "type.h"

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    /* BGNFP Module ID refered by ECFP Module */
    UINT32 bgnfp_md_id;

    /* p represent the prime of F_{p} */
    BIGINT *ecfp_p;

    /* coefficients of the elliptic curve(ec) */
    ECFP_CURVE *ecfp_curve;

    /* the order of the elliptic curve(ec) */
    BIGINT *ecfp_order;

    /* the base point of the elliptic curve(ec) */
    EC_CURVE_POINT *ecfp_base_point;
}ECFP_MD;

/**
*   for test only
*
*   to query the status of ECFP Module
*
**/
void ec_fp_print_module_status(const UINT32 ecfp_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed ECFP module
*
*
**/
UINT32 ec_fp_free_module_static_mem(const UINT32 ecfp_md_id);

/**
*
* start ECFP module
* note:
*   order or base_point is not necessary
*
**/
UINT32 ec_fp_start( const BIGINT *p, const ECFP_CURVE *curve, const BIGINT *order, const EC_CURVE_POINT *base_point );

/**
*
* end ECFP module
*
**/
void ec_fp_end(const UINT32 ecfp_md_id);

/**
*
*   if point curve_1  = curve_2, then return 0
*   if point curve_1 != curve_2, then return 1
*
**/
UINT32 ec_fp_curve_cmp(const UINT32 ecfp_md_id, const ECFP_CURVE * curve_1,const ECFP_CURVE * curve_2);

/**
*
*   if point P  = Q, then return 0
*   if point P != Q, then return 1
*
**/
UINT32 ec_fp_point_cmp(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P,const EC_CURVE_POINT * Q);

/**
*
*   copy point src to des
*   return des
*
**/
void ec_fp_point_clone(const UINT32 ecfp_md_id, const EC_CURVE_POINT * src,EC_CURVE_POINT * des);

/**
*
*   copy affine point src to des
*   return des
*
**/
void ec_fp_point_aff_clone(const UINT32 ecfp_md_id, const EC_CURVE_AFF_POINT * src,EC_CURVE_AFF_POINT * des);

/**
*
*   if the point P = (0,0),i.e, it's an infinite point
*   then return EC_TRUE;
*   otherwise, return EC_FALSE
*
**/
EC_BOOL ec_fp_point_is_infinit(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P);

/**
*
*   let P = [x:y:z]
*   if z = 0, then it means the P is an infinite affine point
*   then return EC_TRUE;
*   otherwise, return EC_FALSE
*
**/
EC_BOOL ec_fp_point_aff_is_infinit(const UINT32 ecfp_md_id, const EC_CURVE_AFF_POINT * P);

/**
*
*   set point P = (0,0) as the infinit point
*   return the point P
*
**/
void ec_fp_point_set_infinit(const UINT32 ecfp_md_id, EC_CURVE_POINT * P);

/**
*
*   set affine point P = [1:0:0] as the infinit affine point
*   return the affine point P
*
**/
void ec_fp_point_aff_set_infinit(const UINT32 ecfp_md_id, EC_CURVE_AFF_POINT * P);

/**
*
*   let P = (x1:y1) be a point over ec
*   let R = (x3:y3)
*   return R =  -P over ec
*
*   note:
*       x3 =  x1 over F_p
*       y3 = -y1 over F_p
*
**/
void ec_fp_point_neg(const UINT32 ecfp_md_id,const EC_CURVE_POINT *P,EC_CURVE_POINT *R);

/**
*
*   let P = [x1:y1:z1] be a point over ec
*   let R = [x3:y3:z3]
*   return R =  -P over ec
*
*   note:
*       x3 =  x1 over F_p
*       y3 = -y1 over F_p
*       z3 =  z1 over F_p
*
**/
void ec_fp_point_aff_neg(const UINT32 ecfp_md_id,const EC_CURVE_AFF_POINT *P,EC_CURVE_AFF_POINT *R);

/**
*   let P = (x1,y1) be a point over ec
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
void ec_fp_point_convert(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P,EC_CURVE_AFF_POINT * R);

/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let R = (x3,y3)
*   return R = P over ec
*   where
*       x3 = x1 / z1 ^2
*       y3 = y1 / z1 ^ 3
*
*   note :
*       P address is not equal to R address
**/
void ec_fp_point_aff_convert(const UINT32 ecfp_md_id, const EC_CURVE_AFF_POINT * P,EC_CURVE_POINT * R);

/**
*
*   check point is on the elliptic curve(ec) refered by Module ID ecfp_md_id
*   if the point is on the elliptic curve(ec), return EC_TRUE,
*   otherwise, return EC_FALSE
*
*   note:
*       the ec is defined by
*           y^2 + xy = x^3 + ax + b over F_p
*
**/
EC_BOOL ec_fp_point_is_on_curve(const UINT32 ecfp_md_id, const EC_CURVE_POINT * point);

/**
*
*   let P = (x1,y1) be a point over ec
*   let R = (x3,y3)
*   return R = 2 * P over ec
*
**/
void ec_fp_point_double(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P,EC_CURVE_POINT * R);

/**
*
*   let P = [x1:y1:z1] be an affine point over ec
*   let R = [x3:y3:z3]
*   return R = 2 * P over ec
*
**/
void ec_fp_point_aff_double(const UINT32 ecfp_md_id, const EC_CURVE_AFF_POINT * P,EC_CURVE_AFF_POINT * R);

/**
*
*   let P = [x1:y1:z1] be a point over ec
*   let Q = (x2,z2) be a point over ec
*   let R = [x3:y3:z3]
*   return R =  P + Q over ec
*
**/
void ec_fp_point_mix_add(const UINT32 ecfp_md_id,const EC_CURVE_AFF_POINT *P,const EC_CURVE_POINT *Q,EC_CURVE_AFF_POINT *R);

/**
*
*   let P = [x1:y1:z1] be a point over ec
*   let Q = (x2,z2) be a point over ec
*   let R = [x3:y3:z3]
*   return R =  P - Q over ec
*
**/
void ec_fp_point_mix_sub(const UINT32 ecfp_md_id,const EC_CURVE_AFF_POINT *P,const EC_CURVE_POINT *Q,EC_CURVE_AFF_POINT *R);

/**
*
*   let P = (x1,y1) be a point over ec
*   let Q = (x2,y1) be a point over ec
*   let R = (x3,y3)
*   return R =  P + Q over ec
*
**/
void ec_fp_point_add(const UINT32 ecfp_md_id,const EC_CURVE_POINT *P,const EC_CURVE_POINT *Q,EC_CURVE_POINT *R);

/**
*
*   let P = (x1,y1) be a point over ec
*   let Q = (x2,y1) be a point over ec
*   let R = (x3,y3)
*   return R =  P - Q over ec
*
**/
void ec_fp_point_sub(const UINT32 ecfp_md_id,const EC_CURVE_POINT *P,const EC_CURVE_POINT *Q,EC_CURVE_POINT *R);

/**
*
*   let P = (x1,y1) be a point over ec
*   let R = (x3,y3)
*   return R =  k * P over ec
*
**/
void ec_fp_point_mul_naf(const UINT32 ecfp_md_id,const EC_CURVE_POINT *P,const BIGINT *k,EC_CURVE_POINT *R);

/**
*
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
*   here, basepoint is fixed, so contruct a table exp_basepoint_tbl:
*       {r * basepoint: r = 2 ^ i, i = 0.. ecfp_n = nbits(ec order) }
*
**/
void ec_fp_point_mul_fix_base(const UINT32 ecfp_md_id, const BIGINT * k, EC_CURVE_POINT * R);

/**
*
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
**/
void ec_fp_point_mul(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P,const BIGINT * k, EC_CURVE_POINT * R);

/**
*
*   compute z = x*3 + ax + b over F_p
*   return z
*
**/
void ec_fp_compute_z(const UINT32 ecfp_md_id, const BIGINT * x, BIGINT *z);

/**
*
*   Let p be an odd prime and p != 1 mod 8.
*   Given x, then to compute one resolution y if following congruence has resolutions:
*       y^2 = x^3 + ax + b mod p
*   and return EC_TRUE; if it has no resolution, then return EC_FALSE
*
*
**/
EC_BOOL ec_fp_compute_y(const UINT32 ecfp_md_id, const BIGINT * x,BIGINT *y);

/**
*
*   to determine a point with x-coordinate x is on ec or not.
*   note, the ec over F_p is defined by
*       y^2 = x^3 + ax + b over F_p
*   if yes, return EC_TRUE;
*   if no, return EC_FALSE
*
**/
EC_BOOL ec_fp_x_is_on_curve(const UINT32 ecfp_md_id, const BIGINT * x,BIGINT *z);


/**
*
*   compute the discriminant of the elliptic curve
*       y^2 = x^3 + a*x + b over F_{p}
*
*   and the discriminant
*       Delta = -16 * (27 * b^2 + 4 * a^3) over F_{p}
*
*
**/
UINT32 ec_fp_disc_compute(const UINT32 ecfp_md_id, BIGINT *delta);

/**
*
*   compute the j-invariant of the elliptic curve
*       y^2 = x^3 + a*x + b over F_{p}
*
*   and the j-invariant
*       j = (12^3) * 4 * (a^3)/(27 * b^2 + 4 * a^3) over F_{p}
*         = -1728 * (4*a)^3 / Delta over F_{p}
*   where Delta is the discriminant of the elliptic curve.
*
**/
UINT32 ec_fp_j_invar_compute(const UINT32 ecfp_md_id, BIGINT *j_invar);

#endif /*_ECFP_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

