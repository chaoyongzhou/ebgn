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

#ifndef _POLYZN_H
#define _POLYZN_H

#include "type.h"
#include "mm.h"

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    /* BGNZ Module ID refered by POLYZN Module */
    UINT32 bgnz_md_id;

    /* BGNZN Module ID refered by POLYZN Module */
    UINT32 bgnzn_md_id;

    BIGINT *bgnzn_n;
}POLYZN_MD;

/**
*   for test only
*
*   to query the status of POLYZN Module
*
**/
void poly_zn_print_module_status(const UINT32 polyzn_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed POLYZN module
*
*
**/
UINT32 poly_zn_free_module_static_mem(const UINT32 polyzn_md_id);
/**
*
* start POLYZN module
*
**/
UINT32 poly_zn_start( const BIGINT *n );

/**
*
* end POLYZN module
*
**/
void poly_zn_end(const UINT32 polyzn_md_id);

/**
*
*   destory the whole item,including its content(deg, coe) and the item itself.
*   so, when return from this function, item cannot be refered any more
*
**/
UINT32 poly_zn_item_destory(const UINT32 polyzn_md_id, POLY_ITEM *item);

/**
*
*   clone item_a to item_c.
*   i.e, return item_c = item_a
*
**/
UINT32 poly_zn_item_clone(const UINT32 polyzn_md_id,const POLY_ITEM *item_a, POLY_ITEM *item_c);

/**
*
*   if item_a  = item_b then return EC_TRUE
*   if item_a != item_b then return EC_FALSE
*
**/
EC_BOOL poly_zn_item_cmp(const UINT32 polyzn_md_id,const POLY_ITEM *item_a, const POLY_ITEM *item_b);

/*
*
*   item_c = - item_a
*
*/
UINT32 poly_zn_item_neg(const UINT32 polyzn_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c);


/**
*
*   move item_a to item_c and set coe of item_a to null
*
**/
UINT32 poly_zn_item_move(const UINT32 polyzn_md_id, POLY_ITEM *item_a, POLY_ITEM *item_c);

/**
*
*   clean all items of poly
*   after return, poly is empty and can be refered again.
*
**/
UINT32 poly_zn_poly_clean(const UINT32 polyzn_md_id, POLY *poly);

/**
*
*   destory all items of poly and the poly itself
*   after return, poly can be refered again.
*
**/
UINT32 poly_zn_poly_destory(const UINT32 polyzn_md_id, POLY *poly);

/**
*
*   clone poly_a to poly_c.
*   i.e, return poly_c = poly_a
*
**/
UINT32 poly_zn_poly_clone(const UINT32 polyzn_md_id,const POLY *poly_a, POLY *poly_c);

/**
*
*   set poly = 0 over Z_{N}[X]
*
**/
UINT32 poly_zn_set_zero(const UINT32 polyzn_md_id,POLY *poly);

/**
*
*   set poly = 1 over Z_{N}[X]
*
**/
UINT32 poly_zn_set_one(const UINT32 polyzn_md_id,POLY *poly);

/**
*
*   set poly = n over Z_{N}[X]
*
**/
UINT32 poly_zn_set_n(const UINT32 polyzn_md_id,const BIGINT *n, POLY *poly);

/**
*
*   move poly_a to poly_c and set poly_a to be empty
*   after return, poly_a has no any items.
*
**/
UINT32 poly_zn_poly_move(const UINT32 polyzn_md_id, POLY *poly_a, POLY *poly_c);

/**
* over Z_{N}[X]
*   if poly_a  = poly_b then return EC_TRUE
*   if poly_a != poly_b then return EC_FALSE
*
**/
EC_BOOL poly_zn_poly_cmp(const UINT32 polyzn_md_id,const POLY *poly_a, const POLY *poly_b);

/**
*
*   poly_c = poly_a + n  over Z_{N}[X]
*
**/
UINT32 poly_zn_add_n(const UINT32 polyzn_md_id,const POLY *poly_a, const BIGINT *n, POLY *poly_c);

/**
*
*   poly_c += poly_a over Z_{N}[X]
*
**/
UINT32 poly_zn_adc_poly(const UINT32 polyzn_md_id,const POLY *poly_a, POLY *poly_c);

/**
*
*   poly_c = poly_a + poly_b over Z_{N}[X]
*
**/
UINT32 poly_zn_add_poly(const UINT32 polyzn_md_id,const POLY *poly_a, const POLY *poly_b, POLY *poly_c);

/**
*
*   poly_c = poly_a - bgn_b over Z_{N}[X]
*
**/
UINT32 poly_zn_sub_poly_bgn(const UINT32 polyzn_md_id,const POLY *poly_a, const BIGINT *bgn_b, POLY *poly_c);

/**
*
*   poly_c = bgn_a - poly_b over Z_{N}[X]
*
**/
UINT32 poly_zn_sub_bgn_poly(const UINT32 polyzn_md_id,const BIGINT *bgn_a, const POLY *poly_b, POLY *poly_c);

/**
*
*   poly_c = - poly_a over Z_{N}[X]
*
**/
UINT32 poly_zn_poly_neg(const UINT32 polyzn_md_id, const POLY *poly_a, POLY *poly_c);

/**
*
*   poly_c = poly_a - poly_b over Z_{N}[X]
*
**/
UINT32 poly_zn_sub_poly(const UINT32 polyzn_md_id, const POLY *poly_a, const POLY *poly_b, POLY *poly_c);

/**
*
*   poly_c = poly_a * poly_b over Z_{N}[X]
*
**/
UINT32 poly_zn_mul_poly(const UINT32 polyzn_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c);


/**
*
* poly_c = poly_a ^ 2 over Z_{N}[X]
*
*/
UINT32 poly_zn_squ_poly(const UINT32 polyzn_md_id,const POLY *poly_a,POLY *poly_c);

/**
*
*   poly_c = poly_a ^ e over Z_{N}[X]
*
**/
UINT32 poly_zn_sexp(const UINT32 polyzn_md_id,const POLY *poly_a,const UINT32 e,POLY *poly_c );

/**
*
*   poly_c = poly_a ^ e over Z_{N}[X]
*
**/
UINT32 poly_zn_exp(const UINT32 polyzn_md_id,const POLY *poly_a,const BIGINT *e,POLY *poly_c );

/*
*   over Z_{N}[X]
*   poly_c = d(poly_a)/dx where x is the first var.
*/
UINT32 poly_zn_dx(const UINT32 polyzn_md_id,const POLY *poly_a, POLY *poly_c );

/*
*   over Z_{N}[X]
*   poly_c = d(poly_a)/dx where x is the depth_of_x-th var.
*/
UINT32 poly_zn_Dx(const UINT32 polyzn_md_id,const POLY *poly_a, const UINT32 depth_of_x, POLY *poly_c );

UINT32 poly_zn_poly_output(const UINT32 polyzn_md_id,const POLY *poly, const UINT32 depth,const char *info);

#endif /*_POLYZN_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

