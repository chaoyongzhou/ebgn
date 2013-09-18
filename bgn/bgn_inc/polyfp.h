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

#ifndef _POLYFP_H
#define _POLYFP_H

#include "type.h"
#include "mm.h"

#define MAX_NUM_OF_PRECOMP_FOR_MOD (2*MAX_PRIME)
#define MAX_PRIME  113

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    /* BGNZ Module ID refered by POLYFP Module */
    UINT32 bgnz_md_id;

    /* BGNFP Module ID refered by POLYFP Module */
    UINT32 bgnfp_md_id;

    BIGINT *bgnfp_p;
}POLYFP_MD;

/**
*   for test only
*
*   to query the status of POLYFP Module
*
**/
void poly_fp_print_module_status(const UINT32 polyfp_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed POLYFP module
*
*
**/
UINT32 poly_fp_free_module_static_mem(const UINT32 polyfp_md_id);
/**
*
* start POLYZ module
*
**/
UINT32 poly_fp_start( const BIGINT *p );

/**
*
* end POLYZ module
*
**/
void poly_fp_end(const UINT32 polyfp_md_id);

/**
*
*   alloc a BIGINT type node from POLYFP space
*
**/
UINT32 poly_fp_alloc_bgn(const UINT32 polyfp_md_id, BIGINT **ppbgn);

/**
*
*   alloc a DEGREE type node from POLYFP space
*
**/
UINT32 poly_fp_alloc_deg(const UINT32 polyfp_md_id, DEGREE **ppdeg);

/**
*
*   alloc a POLY_ITEM type node from POLYFP space
*
**/
UINT32 poly_fp_alloc_item(const UINT32 polyfp_md_id,POLY_ITEM **ppitem);

#if 1
/**
*
*   alloc a POLY type node from POLYFP space
*
**/
UINT32 poly_fp_alloc_poly(const UINT32 polyfp_md_id,POLY **pppoly);
#else
#define poly_fp_alloc_poly(polyfp_md_id,pppoly)  alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, pppoly)
#endif
/**
*
*   free the BIGINT type node from POLYFP space
*
**/
UINT32 poly_fp_free_bgn(const UINT32 polyfp_md_id,BIGINT *bgn);

/**
*
*   free the DEGREE type node from POLYFP space
*
**/
UINT32 poly_fp_free_deg(const UINT32 polyfp_md_id,DEGREE *deg);

/**
*
*   free the POLY_ITEM type node from POLYFP space
*
**/
UINT32 poly_fp_free_item(const UINT32 polyfp_md_id,POLY_ITEM *item);

/**
*
*   free the POLY type node from POLYFP space
*
**/
UINT32 poly_fp_free_poly(const UINT32 polyfp_md_id,POLY *poly);

/**
*
*   destory the whole item,including its content(deg, coe) and the item itself.
*   so, when return from this function, item cannot be refered any more
*
**/
UINT32 poly_fp_item_destory(const UINT32 polyfp_md_id, POLY_ITEM *item);

/**
*
*   clone item_a to item_c.
*   i.e, return item_c = item_a
*
**/
UINT32 poly_fp_item_clone(const UINT32 polyfp_md_id,const POLY_ITEM *item_a, POLY_ITEM *item_c);

/**
*
*   if item_a  = item_b then return EC_TRUE
*   if item_a != item_b then return EC_FALSE
*
**/
EC_BOOL poly_fp_item_cmp(const UINT32 polyfp_md_id,const POLY_ITEM *item_a, const POLY_ITEM *item_b);

/*
*
*   item_c = - item_a
*
*/
UINT32 poly_fp_item_neg(const UINT32 polyfp_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c);


/**
*
*   move item_a to item_c and set coe of item_a to null
*
**/
UINT32 poly_fp_item_move(const UINT32 polyfp_md_id, POLY_ITEM *item_a, POLY_ITEM *item_c);

/**
*
*   clean all items of poly
*   after return, poly is empty and can be refered again.
*
**/
UINT32 poly_fp_poly_clean(const UINT32 polyfp_md_id, POLY *poly);

/**
*
*   destory all items of poly and the poly itself
*   after return, poly can be refered again.
*
**/
UINT32 poly_fp_poly_destory(const UINT32 polyfp_md_id, POLY *poly);

/**
*
*   clone poly_a to poly_c.
*   i.e, return poly_c = poly_a
*
**/
UINT32 poly_fp_poly_clone(const UINT32 polyfp_md_id,const POLY *poly_a, POLY *poly_c);

/**
*
*   set poly = 0 over F_{p}[X]
*
**/
UINT32 poly_fp_set_zero(const UINT32 polyfp_md_id,POLY *poly);

/**
*
*   set poly = 1 over F_{p}[X]
*
**/
UINT32 poly_fp_set_one(const UINT32 polyfp_md_id,POLY *poly);

/**
*
*   set poly = n
*
**/
UINT32 poly_fp_set_word(const UINT32 polyfp_md_id,const UINT32 n, POLY *poly);

/**
*
*   set poly = n over F_{p}[X]
*
**/
UINT32 poly_fp_set_n(const UINT32 polyfp_md_id,const BIGINT *n, POLY *poly);

/**
*
*   set poly = x^n
*
**/
UINT32 poly_fp_set_xn(const UINT32 polyfp_md_id,const DEGREE *deg, POLY *poly);

/**
*
*   set poly = x^n
*
**/
UINT32 poly_fp_set_xn_word(const UINT32 polyfp_md_id,const UINT32 deg, POLY *poly);
/**
*
*   move poly_a to poly_c and set poly_a to be empty
*   after return, poly_a has no any items.
*
**/
UINT32 poly_fp_poly_move(const UINT32 polyfp_md_id, POLY *poly_a, POLY *poly_c);

/**
*
*   if poly_a  = poly_b then return EC_TRUE
*   if poly_a != poly_b then return EC_FALSE
*
**/
EC_BOOL poly_fp_poly_cmp(const UINT32 polyfp_md_id,const POLY *poly_a, const POLY *poly_b);

/**
*
*   poly_c = poly_a + n  over F_{p}[X]
*
**/
UINT32 poly_fp_add_n(const UINT32 polyfp_md_id,const POLY *poly_a, const BIGINT *n, POLY *poly_c);

/**
*
*   poly_c += poly_a over F_{p}[X]
*
**/
UINT32 poly_fp_adc_poly(const UINT32 polyfp_md_id,const POLY *poly_a, POLY *poly_c);

/**
*
*   poly_c = poly_a + poly_b over F_{p}[X]
*
**/
UINT32 poly_fp_add_poly(const UINT32 polyfp_md_id,const POLY *poly_a, const POLY *poly_b, POLY *poly_c);

/**
*
*   poly_c = poly_a - bgn_b over F_{p}[X]
*
**/
UINT32 poly_fp_sub_poly_bgn(const UINT32 polyfp_md_id,const POLY *poly_a, const BIGINT *bgn_b, POLY *poly_c);

/**
*
*   poly_c = bgn_a - poly_b over F_{p}[X]
*
**/
UINT32 poly_fp_sub_bgn_poly(const UINT32 polyfp_md_id,const BIGINT *bgn_a, const POLY *poly_b, POLY *poly_c);

/**
*
*   poly_c = - poly_a over F_{p}[X]
*
**/
UINT32 poly_fp_poly_neg(const UINT32 polyfp_md_id, const POLY *poly_a, POLY *poly_c);

/**
*
*   poly_c = poly_a - poly_b over F_{p}[X]
*
**/
UINT32 poly_fp_sub_poly(const UINT32 polyfp_md_id, const POLY *poly_a, const POLY *poly_b, POLY *poly_c);

/**
*
*   poly_c = poly_a * poly_b over F_{p}[X]
*
**/
UINT32 poly_fp_mul_poly(const UINT32 polyfp_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c);

/**
*
*   poly_c = poly_a * bgn_b over F_{p}[X]
*
**/
UINT32 poly_fp_mul_bgn(const UINT32 polyfp_md_id,const POLY *poly_a,const BIGINT *bgn_b, POLY *poly_c);

/**
*
*   poly_c = poly_a * bgn_b over F_{p}[X]
*
**/
UINT32 poly_fp_mul_word(const UINT32 polyfp_md_id,const POLY *poly_a,const UINT32 word_b, POLY *poly_c);
/**
*
* poly_c = poly_a ^ 2 over F_{p}[X]
*
*/
UINT32 poly_fp_squ_poly(const UINT32 polyfp_md_id,const POLY *poly_a,POLY *poly_c);

/**
*
*   poly_c = poly_a ^ e over F_{p}[X]
*
**/
UINT32 poly_fp_sexp(const UINT32 polyfp_md_id,const POLY *poly_a,const UINT32 e,POLY *poly_c );

/**
*
*   poly_c = poly_a ^ e over F_{p}[X]
*
**/
UINT32 poly_fp_exp(const UINT32 polyfp_md_id,const POLY *poly_a,const BIGINT *e,POLY *poly_c );

/**
*
*   determine whether the poly is monic polynomial or not.
*   if poly is monic polynomial, then return EC_TRUE;
*   if poly is not monic polynomial, then return EC_FALSE.
*
*   note:
*       1. poly = 0 is not monic polynomial.
*       2. poly must be one-var polynomial.
**/

EC_BOOL poly_fp_is_monic(const UINT32 polyfp_md_id, const POLY *poly );

/**
*
*   return poly_c which is the monic polynomial of poly_a
*   where poly_a,poly_c must be one-var polynomial.
*
*/
UINT32 poly_fp_to_monic(const UINT32 polyfp_md_id, const POLY *poly_a, POLY * poly_c);
/**
*
*   poly_c = poly_a mod poly_b over F_{p}[x]
*   where poly_a,poly_b,poly_c are all with one-var only.
*
**/
UINT32 poly_fp_mod_poly(const UINT32 polyfp_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c );

/**
*
*
*   poly_c = poly_a ^ e mod poly_b
*
*
**/
UINT32 poly_fp_exp_mod(const UINT32 polyfp_md_id, const POLY *poly_a, const BIGINT *e, const POLY *poly_b, POLY *poly_c);

/**
*
*   poly_c = gcd(poly_a , poly_b) over F_{p}[x]
*   where poly_a,poly_b,poly_c are all with one-var only.
*
*   Note:
*       return poly_c is monic polynomial.
*
**/
UINT32 poly_fp_gcd_poly(const UINT32 polyfp_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c);

/*
*   over F_{p}[X]
*   poly_c = d(poly_a)/dx where x is the first var.
*/
UINT32 poly_fp_dx(const UINT32 polyfp_md_id,const POLY *poly_a, POLY *poly_c );

/*
*   over F_{p}[X]
*   poly_c = d(poly_a)/dx where x is the depth_of_x-th var.
*/
UINT32 poly_fp_Dx(const UINT32 polyfp_md_id,const POLY *poly_a, const UINT32 depth_of_x, POLY *poly_c );

/**
*
* let poly = P(x0,x1,...) over F_{p}[X}
* evaluate P(x0=n,x1,...)
*
**/
UINT32 poly_fp_eval_x(const UINT32 polyfp_md_id,const POLY *poly_a, const BIGINT *bgn_n, POLY *poly_c );

/**
*
* let poly = P(x0,x1,...) over F_{p}[X}
* evaluate P(x0,x1,...,xi=n,...)
*
**/
UINT32 poly_fp_eval(const UINT32 polyfp_md_id,const POLY *poly_a, const UINT32 depth_of_x, const BIGINT *bgn_n, POLY *poly_c );

UINT32 poly_fp_poly_output(const UINT32 polyfp_md_id,const POLY *poly, const UINT32 depth,const char *info);

#endif /*_POLYFP_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

