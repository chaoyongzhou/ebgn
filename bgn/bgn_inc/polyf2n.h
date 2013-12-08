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

#ifndef _POLYF2N_H
#define _POLYF2N_H

#include "type.h"
#include "mm.h"

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    /* BGNZ Module ID refered by POLYF2N Module */
    UINT32 bgnz_md_id;

    /* BGNF2N Module ID refered by POLYF2N Module */
    UINT32 bgnf2n_md_id;

    BIGINT *bgnf2n_f_x;
}POLYF2N_MD;

/**
*   for test only
*
*   to query the status of POLYF2N Module
*
**/
void poly_f2n_print_module_status(const UINT32 polyf2n_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed POLYF2N module
*
*
**/
UINT32 poly_f2n_free_module_static_mem(const UINT32 polyf2n_md_id);

/**
*
* start POLYF2N module
*
**/
UINT32 poly_f2n_start( const BIGINT *f_x );

/**
*
* end POLYF2N module
*
**/
void poly_f2n_end(const UINT32 polyf2n_md_id);

/**
*
*   destory the whole item,including its content(deg, coe) and itself.
*   so, when return from this function, item cannot be refered any more
*
**/
UINT32 poly_f2n_item_destory(const UINT32 polyf2n_md_id, POLY_ITEM *item);

/**
*
*   move item_a to item_c and set coe of item_a to null
*
**/
UINT32 poly_f2n_item_move(const UINT32 polyf2n_md_id, POLY_ITEM *item_a, POLY_ITEM *item_c);

/**
*
*   clone item_a to item_c.
*   i.e, return item_c = item_a
*
**/
UINT32 poly_f2n_item_clone(const UINT32 polyf2n_md_id,const POLY_ITEM *item_a, POLY_ITEM *item_c);

/**
*
*   if item_a  = item_b then return EC_TRUE
*   if item_a != item_b then return EC_FALSE
*
**/
EC_BOOL poly_f2n_item_cmp(const UINT32 polyf2n_md_id,const POLY_ITEM *item_a, const POLY_ITEM *item_b);

/**
*
*   destory the whole poly,i.e., all its items but not the poly itself.
*   so, when return from this function, poly can be refered again without any item.
*
**/
UINT32 poly_f2n_poly_clean(const UINT32 polyf2n_md_id, POLY *poly);

/**
*
*   destory the whole poly,including its all items and itself.
*   so, when return from this function, poly cannot be refered any more
*
**/
UINT32 poly_f2n_poly_destory(const UINT32 polyf2n_md_id, POLY *poly);

/**
*
*   clone poly_a to poly_c.
*   i.e, return poly_c = poly_a
*
**/
UINT32 poly_f2n_poly_clone(const UINT32 polyf2n_md_id,const POLY *poly_a, POLY *poly_c);

/**
*
*   set poly = 0 over F_{2^N}[X]
*
**/
UINT32 poly_f2n_set_zero(const UINT32 polyf2n_md_id,POLY *poly);

/**
*
*   set poly = 1 over F_{2^N}[X]
*
**/
UINT32 poly_f2n_set_one(const UINT32 polyf2n_md_id,POLY *poly);

/**
*
*   set poly = n over F_{2^N}[X]
*
**/
UINT32 poly_f2n_set_n(const UINT32 polyf2n_md_id,const BIGINT *n, POLY *poly);

/**
*
*   if poly_a  = poly_b then return EC_TRUE
*   if poly_a != poly_b then return EC_FALSE
*
**/
EC_BOOL poly_f2n_poly_cmp(const UINT32 polyf2n_md_id,const POLY *poly_a, const POLY *poly_b);

/**
*
*   poly_c = poly_a + n  over F_{2^N}[X]
*
**/
UINT32 poly_f2n_add_n(const UINT32 polyf2n_md_id,const POLY *poly_a, const BIGINT *n, POLY *poly_c);

/**
*
*   poly_c = poly_c + poly_a  over F_{2^N}[X]
*
*/
UINT32 poly_f2n_adc_poly(const UINT32 polyf2n_md_id,const POLY *poly_a, POLY *poly_c);

/**
*
*   poly_c = poly_a + poly_b over F_{2^N}[X]
*
**/
UINT32 poly_f2n_add_poly(const UINT32 polyf2n_md_id,const POLY *poly_a, const POLY *poly_b, POLY *poly_c);

/**
*
*   move poly_a to poly_c and set poly_a to be empty
*   after return, poly_a has no any items.
*
**/
UINT32 poly_f2n_poly_move(const UINT32 polyf2n_md_id, POLY *poly_a, POLY *poly_c);

/**
* poly_c = poly_a * poly_b over F_{2^N}[X]
*
*/
UINT32 poly_f2n_mul_poly(const UINT32 polyf2n_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c);

/**
*
* poly_c = poly_a ^ 2 over F_{2^N}[X]
*
*/
UINT32 poly_f2n_squ_poly(const UINT32 polyf2n_md_id,const POLY *poly_a,POLY *poly_c);

/**
*
*   poly_c = poly_a ^ e over F_{2^N}[X]
*
**/
UINT32 poly_f2n_sexp(const UINT32 polyf2n_md_id,const POLY *poly_a,const UINT32 e,POLY *poly_c );

/**
*
*   poly_c = poly_a ^ e over F_{2^N}[X]
*
**/
UINT32 poly_f2n_exp(const UINT32 polyf2n_md_id,const POLY *poly_a,const BIGINT *e,POLY *poly_c );

/**
*    over F_{2^N}[X]
*   poly_c = d(poly_a)/dx where x is the first var.
*
*/
UINT32 poly_f2n_dx(const UINT32 polyf2n_md_id,const POLY *poly_a, POLY *poly_c );

/**
*    over F_{2^N}[X]
*   poly_c = d(poly_a)/dx where x is the depth_of_x-th var.
*
**/
UINT32 poly_f2n_Dx(const UINT32 polyf2n_md_id,const POLY *poly_a, const UINT32 depth_of_x, POLY *poly_c );

UINT32 poly_f2n_poly_output(const UINT32 polyf2n_md_id,const POLY *poly, const UINT32 depth,const char *info);

#endif /*_POLYF2N_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

