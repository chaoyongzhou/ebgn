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
#ifndef _LIB_ECFP_H
#define _LIB_ECFP_H

#include "lib_type.h"

/**
*   for test only
*
*   to query the status of ECFP Module
*
**/
void print_ec_fp_status(LOG *log);

/**
*
* start ECFP module
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
*       where  a = -3 over F_p
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
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
**/
void ec_fp_point_mul(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P,const BIGINT * k, EC_CURVE_POINT * R);

#endif /*_LIB_ECFP_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
