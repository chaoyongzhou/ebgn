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

#ifndef _SEAFP_H
#define _SEAFP_H

#include "type.h"
#include "mm.h"

#define SEAFP_SMALL_PRIME_NUM  (24)
#define SEAFP_MAX_SMALL_PRIME  (101) /*must be larger than the max small prime used*/

#define SEAFP_MAX_ARRAY_SIZE   (2*1024*1024)

#define SEAFP_NUM_OF_FACTOR    (11)
#define SEAFP_MAX_FACTOR       (SEAFP_NUM_OF_FACTOR + 1)

#define SEAFP_MAX_LEN_OF_ZTABLE (22)

#define SEAFP_MAX_INDEX        (SEAFP_MAX_SMALL_PRIME + 1 )

#define SEAFP_FAI_EACH_TBL_SIZE (SEAFP_SMALL_PRIME_NUM)

#define SEAFP_KUSAI_TBL_SIZE (SEAFP_MAX_SMALL_PRIME)
#define SEAFP_BGN_CLUST_SIZE (SEAFP_MAX_SMALL_PRIME)

#define SEAFP_ATKIN_TBL_SIZE (SEAFP_SMALL_PRIME_NUM)
typedef struct
{
    /*note: it's better to change "POLY" to "POLY *" to facilitate reference*/
    POLY *fai   [SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dxfai [SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dyfai [SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dxxfai[SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dyyfai[SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dxyfai[SEAFP_FAI_EACH_TBL_SIZE];

    /*fai_j(x) = fai(x, y=j)*/
    POLY *fai_j   [SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dxfai_j [SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dyfai_j [SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dxxfai_j[SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dyyfai_j[SEAFP_FAI_EACH_TBL_SIZE];
    POLY *dxyfai_j[SEAFP_FAI_EACH_TBL_SIZE];
}SEAFP_FAI_TBL;

typedef struct
{
    POLY *kusai[SEAFP_KUSAI_TBL_SIZE];
}SEAFP_KUSAI_TBL;

/*define BGN Cluster Structer*/
typedef struct
{
    BIGINT *bgn[SEAFP_BGN_CLUST_SIZE];
}SEAFP_BGN_TBL;

typedef struct{
    BIGINT  *prime;
    UINT32  tmodprime[ 2 * SEAFP_MAX_LEN_OF_ZTABLE ];
    double  rate;
}SEAFP_ATKIN_ITEM;

typedef struct
{
    SEAFP_ATKIN_ITEM Atkin[SEAFP_ATKIN_TBL_SIZE];
    UINT32      pos;
}SEAFP_ATKIN_TBL;

typedef struct
{
    BIGINT         *r1;            /*where 0 <=r1 < [m1/2]*/
                                   /*if Atkin_r1_sign = TRUE , then r1 =  (t1-t3)/(m2*m3) mod m1*/
                                   /*if Atkin_r1_sign = FALSE, then r1 = -(t1-t3)/(m2*m3) mod m1*/
    EC_CURVE_POINT *Atkin_Q;       /*Q = r1*Q1 + Q0, where 0 <=r1 < [m1/2]*/
    UINT32          x_sort_key;    /*key word for heap sorting*/
    UINT32          y_sort_key;    /*key word for heap sorting*/
}SEAFP_ATKIN_NODE;

typedef struct
{
    UINT32               Atkin_Set_Node_Index[SEAFP_MAX_ARRAY_SIZE];
    SEAFP_ATKIN_NODE     Atkin_Set_Node[SEAFP_MAX_ARRAY_SIZE];
    UINT32               Atkin_Set_size;
}SEAFP_ATKIN_SET;

/*SEAFP_ORDER_CASE_ENUM*/
#define               SEAFP_ORDER_CASE_1    ((UINT32)  0)  /*here order = (p + 1 - t3) - (r1*m2*m3) -(r2*m1*m3)*/
                                                           /*so that, trace t = t3 + (r1*m2*m3) + (r2*m1*m3)*/
#define               SEAFP_ORDER_CASE_2    ((UINT32)  1)   /*here order = (p + 1 - t3) - (r1*m2*m3) -(r2*m1*m3) + m1*m2*m3*/
                                                            /*so that, trace t = t3 + (r1*m2*m3) + (r2*m1*m3) - m1*m2*m3*/
#define               SEAFP_ORDER_CASE_0    ((UINT32) -1)

typedef UINT32 SEAFP_ORDER_CASE_ENUM_UINT32;

typedef struct
{
    UINT32  oddsmallprime[ SEAFP_SMALL_PRIME_NUM ];
    UINT32  pmodsmallprime[ SEAFP_SMALL_PRIME_NUM ];
    UINT32  sqrtof4pmodsmallprime[ SEAFP_SMALL_PRIME_NUM ];

    UINT32 oddsmallprime_num;   /*number of odd small primes in the table*/
}SEAFP_PRIME_TBL;

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    /* BGNZ Module ID refered by SEAFP Module */
    UINT32 bgnz_md_id;

    /* BGNFP Module ID refered by SEAFP Module */
    UINT32 bgnfp_md_id;

    /* ECFP Module ID refered by SEAFP Module */
    UINT32 ecfp_md_id;

    /* POLYFP Module ID refered by SEAFP Module */
    UINT32 polyfp_md_id;

    /* p represent the prime of F_{p} */
    BIGINT *ecfp_p;

    /*Hasse Theorem: |#E - (p+1)| <= 2*sqrt(p), where ecfp_hasse is defined as 2*sqrt(p)*/
    BIGINT *ecfp_hasse;

    /* coefficients of the elliptic curve(ec) */
    ECFP_CURVE *ecfp_curve;

    /* j-invariant of the elliptic curve(ec)*/
    BIGINT *ecfp_j_invar;

    /*global buffers*/
    SEAFP_PRIME_TBL     seafp_oddsmallprime_tbl;
    SEAFP_FAI_TBL       seafp_fai_tbl;  /*fai(x,y),dxfai(x,y),dyfai(x,y),dxxfai(x,y),dyyfai(x,y),dxyfai(x,y)*/
                                        /*fai(x,y=j),dxfai(x,y=j),dyfai(x,y=j),dxxfai(x,y=j),dyyfai(x,y=j),dxyfai(x,y=j)*/
    SEAFP_KUSAI_TBL     seafp_kusai_tbl;/*KUSAI(x)*/
    SEAFP_BGN_TBL       seafp_prod_tbl; /* k! mod p*/
    SEAFP_ATKIN_TBL     seafp_Atkin_tbl;
    SEAFP_ATKIN_SET     seafp_Atkin_set;
}SEAFP_MD;


/**
*   for test only
*
*   to query the status of SEAFP Module
*
**/
void sea_fp_print_module_status(const UINT32 seafp_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed SEAFP module
*
*
**/
UINT32 sea_fp_free_module_static_mem(const UINT32 seafp_md_id);

/**
*
* start SEAFP module
*
**/
UINT32 sea_fp_start( const BIGINT *p, const ECFP_CURVE *curve);

/**
*
* end SEAFP module
*
**/
void sea_fp_end(const UINT32 seafp_md_id);

/**
*
*   return poly = x^p - x over F_{p}[x]
*               = x^p + (p - 1);*x over F_{p}[x]
*
**/
UINT32 sea_fp_set_xpx_to_poly(const UINT32 seafp_md_id,POLY *poly);

/**
*
*   return poly = x^3 + a*x + b  over F_{p}[x]
*
**/
UINT32 sea_fp_set_ec_xpart_to_poly(const UINT32 seafp_md_id,POLY *poly);


/**
*
*   determine the order of ec is even or not by computing
*      gcd(x^p - x, x^3 + ax + b);
*   if the degree of gcd is not zero, then the order is even and return TRUE;
*   else the degree of gcd is zero, then the order is odd and return FALSE
*
**/
EC_BOOL sea_fp_ec_order_is_even(const UINT32 seafp_md_id );


/**
*
*   build the FAI tables but not FAI_J table by computing
*       DXFAI = dx(FAI); tables
*       DYFAI = dy(FAI); tables
*       DXXFAI = dx(DXFAI); tables
*       DYYFAI = dy(DYFAI); tables
*       DXYFAI = dx(DYFAI); = dy(DXFAI); tables
*
*  note:
*       at present, read the FAI table from dat files
*
**/
UINT32 sea_fp_poly_fai_tbl_build(const UINT32 seafp_md_id);


/*this interface is for test only*/
UINT32 sea_fp_poly_destory(const UINT32 seafp_md_id, POLY *poly);


/*this interface is for test only*/
UINT32 sea_fp_poly_clean(const UINT32 seafp_md_id, POLY *poly);

/**
*
*   here deg(poly(x);); = 2. then find a root such tat poly(x = root); = 0 mod p.
*   let poly = x^2 + a*x + b, then
*       x^2 + a*x +b =0 <==> (x +(a/2););^2 = ((a/2);^2 -b); mod p
*
*
**/
UINT32 sea_fp_poly_squroot(const UINT32 seafp_md_id, const POLY *poly, BIGINT *root);


UINT32 sea_fp_iso_ec(const UINT32 seafp_md_id,
                         const BIGINT *F,
                         const UINT32 oddsmallprime_pos,
                         ECFP_CURVE *iso_ecfp_curve,
                         BIGINT *isop1);



UINT32 sea_fp_iso_h(const UINT32 seafp_md_id,
                         const ECFP_CURVE *iso_ecfp_curve,
                         const BIGINT *isop1,
                         const UINT32 lpos,
                         POLY *iso_h );

UINT32 sea_fp_Elkies_kusais(const UINT32 seafp_md_id,
                         const POLY *iso_h,
                         const UINT32 m);


UINT32 sea_fp_Elkies(const UINT32 seafp_md_id,
                         const UINT32  *seafp_pmodsmallprime_tbl,
                         const POLY *poly_gcd_result,
                         const UINT32 oddsmallprime_pos,
                         INT32 *tmod);


UINT32 sea_fp_Elkies_CRT(const UINT32 seafp_md_id, const UINT32 x2, const UINT32 m2,BIGINT *x1,BIGINT *m1);


/**
*
*   here,
*       poly_xpx_mod_fai_j  = (x^p - x); mod fai[oddsmallprime_pos](x,y=j);
*
*
*
**/
UINT32 sea_fp_Atkin_r(const UINT32 seafp_md_id, const POLY *poly_xpx_mod_fai_j, const UINT32 oddsmallprime_pos, INT32 *r);


UINT32 sea_fp_Atkin_r_pos(const UINT32 seafp_md_id, const UINT32 oddsmallprime_pos, const INT32 Atkin_r, UINT32 *Atkin_r_pos );


UINT32 sea_fp_Atkin_item_clone(const UINT32 seafp_md_id,
                                    const SEAFP_ATKIN_ITEM *Atkin_item_src,
                                    SEAFP_ATKIN_ITEM *Atkin_item_des );

UINT32 sea_fp_Atkin_s_insert(const UINT32 seafp_md_id,
                                    const SEAFP_ATKIN_ITEM *Atkin_item_new );


UINT32 sea_fp_Atkin_s_pos_modify(const UINT32 seafp_md_id, const BIGINT *Elkies_m );


UINT32 sea_fp_Atkin_s_breakpoint_get(const UINT32 seafp_md_id, UINT32 *breakpoint );


UINT32 sea_fp_Atkin_s_modify(const UINT32 seafp_md_id);


UINT32 sea_fp_Atkin_s_breakpoint_modify(const UINT32 seafp_md_id, BIGINT *Atkin_m1, BIGINT *Atkin_m2, UINT32 *breakpoint );


UINT32 sea_fp_hash_key_gen(const UINT32 seafp_md_id,const BIGINT *data,UINT32 *key);

UINT32 sea_fp_Atkin_node_clone(const UINT32 seafp_md_id, const SEAFP_ATKIN_NODE *Atkin_node_src, SEAFP_ATKIN_NODE *Atkin_node_des);

UINT32 sea_fp_Atkin_node_exchange(const UINT32 seafp_md_id, SEAFP_ATKIN_NODE *Atkin_node_1, SEAFP_ATKIN_NODE *Atkin_node_2);

UINT32 sea_fp_heap_sift(const UINT32 seafp_md_id, const UINT32 beg_pos, const UINT32 end_pos);

UINT32 sea_fp_heap_sort(const UINT32 seafp_md_id);

EC_BOOL sea_fp_match_R(const UINT32 seafp_md_id,
                            const BIGINT *m1,
                            const BIGINT *m2,
                            const BIGINT *m3,
                            const BIGINT *t3,
                            const BIGINT *r2,
                            const EC_CURVE_POINT *H1,
                            BIGINT *ecfp_order);


UINT32 sea_fp_baby_step(const UINT32 seafp_md_id,
                            const BIGINT *m1,
                            const BIGINT *m2,
                            const BIGINT *m3,
                            const BIGINT *t3,
                            const UINT32  depth,
                            const EC_CURVE_POINT *test_point
                            );


EC_BOOL sea_fp_giant_step(const UINT32 seafp_md_id,
                            const BIGINT *m1,
                            const BIGINT *m2,
                            const BIGINT *m3,
                            const BIGINT *t3,
                            const UINT32  from,
                            const UINT32  depth,
                            const EC_CURVE_POINT *test_point,
                            BIGINT *r1,
                            BIGINT *r2,
                            UINT32 *order_case
                            );


UINT32 sea_fp_ec_point_gen(const UINT32 seafp_md_id, EC_CURVE_POINT *ecfp_point);


UINT32 sea_fp_main(const UINT32 seafp_md_id);



#endif /*_SEAFP_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

