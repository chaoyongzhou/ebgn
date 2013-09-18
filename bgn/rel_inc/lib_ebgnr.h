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

#ifndef _LIB_EBGNR_H
#define _LIB_EBGNR_H

#include "lib_type.h"

//#define EBGN_POS(ebgn)          ((ebgn)->pos)

//#define EBGN_SET_POS(ebgn, posx) ((ebgn)->pos = (posx))

/* ------------------------------------------ function prototype definition ------------------------------------*/
/**
*   for test only
*
*   to query the status of EBGNR Module
*
**/
void print_ebgn_r_status(LOG *log);

/**
*
*   free all static memory occupied by the appointed EBGNR module
*
*
**/
UINT32 ebgn_r_free_module_static_mem(const UINT32 ebgnr_md_id);
/**
*
* start EBGNR module
*
**/
UINT32 ebgn_r_start( UINT32 decimal_precision );

/**
*
* end EBGNR module
*
**/
void ebgn_r_end(const UINT32 ebgnr_md_id);

/**
*
*   alloc a EBGN type node from EBGNR space
*
**/
UINT32 ebgn_r_alloc_ebgn(const UINT32 ebgnr_md_id,EBGN **ppebgn);

/**
*
*   free the EBGN type node from EBGNR space
*
**/
UINT32 ebgn_r_free_ebgn(const UINT32 ebgnr_md_id,EBGN *ebgn);

/**
*
*   destroy the whole ebgn,i.e., all its items but not the ebgn itself.
*   so, when return from this function, ebgn can be refered again without any item.
*
**/
UINT32 ebgn_r_clean(const UINT32 ebgnr_md_id, EBGN *ebgn);

/**
*
*   destory the whole ebgn,including its all items and itself.
*   so, when return from this function, ebgn cannot be refered any more
*
**/
UINT32 ebgn_r_destroy(const UINT32 ebgnr_md_id, EBGN *ebgn);

/**
*
*   ebgn_c = ebgn_a
*
**/
UINT32 ebgn_r_clone(const UINT32 ebgnr_md_id,const EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*   if ebgn = 0, return EC_TRUE
*   if ebgn != 0, return EC_FALSE
*
**/
EC_BOOL ebgn_r_is_zero(const UINT32 ebgnr_md_id, const EBGN *ebgn);

/**
*
*   if ebgn = 1, return EC_TRUE
*   if ebgn != 1, return EC_FALSE
*
**/
EC_BOOL ebgn_r_is_one(const UINT32 ebgnr_md_id, const EBGN *ebgn);

/**
*
*   set ebgn = 0
*
**/
UINT32 ebgn_r_set_zero(const UINT32 ebgnr_md_id,EBGN *ebgn);

/**
*
*   set ebgn = 1
*
**/
UINT32 ebgn_r_set_one(const UINT32 ebgnr_md_id,EBGN *ebgn);

/**
*
*   set ebgn = n
*
**/
UINT32 ebgn_r_set_word(const UINT32 ebgnr_md_id,const UINT32 n, EBGN *ebgn);

/**
*
*   set ebgn = n
*
**/
UINT32 ebgn_r_set_n(const UINT32 ebgnr_md_id,const BIGINT *n, EBGN *ebgn);

/**
*
*   set ebgn = x
*
**/
UINT32 ebgn_r_set_real(const UINT32 ebgnr_md_id, const REAL *x, EBGN *ebgn);

/**
*
*   if ebgn_a = ebgn_b then return 0
*   if ebgn_a > ebgn_b then return 1
*   if ebgn_a < ebgn_b then return -1
*
**/
int ebgn_r_cmp(const UINT32 ebgnr_md_id,const EBGN *ebgn_a, const EBGN *ebgn_b);

/*
*
*   if abs(ebgn_a) = abs(ebgn_b) then return 0
*   if abs(ebgn_a) > abs(ebgn_b) then return 1
*   if abs(ebgn_a) < abs(ebgn_b) then return -1
*
**/
int ebgn_r_abs_cmp(const UINT32 ebgnr_md_id,const EBGN *ebgn_a, const EBGN *ebgn_b);

/**
*
*       move ebgn_a to ebgn_c, ebgn_c = ebgn_a, and ebgn_a return null
*
**/
UINT32 ebgn_r_move(const UINT32 ebgnr_md_id, EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*
*  ebgn_b = -ebgn_a
*
*  note, maybe address of ebgn_a = address of ebgn_b
*
**/
UINT32 ebgn_r_neg(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_b);

/**
*
*
*  ebgn_b = abs(ebgn_a)
*
*  note, maybe address of ebgn_a = address of ebgn_b
*
**/
UINT32 ebgn_r_abs(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_b);

/**
*
*   ebgn ++
*
**/
UINT32 ebgn_r_inc(const UINT32 ebgnr_md_id, EBGN *ebgn);

/**
*
*
*   ebgn --
*
**/
UINT32 ebgn_r_dec(const UINT32 ebgnr_md_id, EBGN *ebgn);

/**
*
*
*   ebgn_c = ebgn_a + ebgn_b
*
**/
UINT32 ebgn_r_add(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = ebgn_a - ebgn_b
*
**/
UINT32 ebgn_r_sub(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);


/**
*
*
*   ebgn_c = ebgn_a * bgn_b
*
**/
UINT32 ebgn_r_smul(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const BIGINT *bgn_b, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = ebgn_a * ebgn_b
*
**/
UINT32 ebgn_r_mul(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = ebgn_a ^ 2
*
**/
UINT32 ebgn_r_squ(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*
*  ebgn_a = ebgn_b * ebgn_q
*
**/
UINT32 ebgn_r_div(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_q);

/**
*       c = ( a ^ e )
*
**/
UINT32 ebgn_r_sexp(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const UINT32 e, EBGN *ebgn_c);

/**
*       c = ( a ^ e )
*
**/
UINT32 ebgn_r_exp(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const BIGINT *bgn_e, EBGN *ebgn_c);

/**
*       c = ( a ^ e )
*
**/
UINT32 ebgn_r_dexp(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_e, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = e^x = 1 + SUM( x ^ i) / i!, i = 1,2,...)
*
* where x = ebgn_a
*
**/
UINT32 ebgn_r_eexp(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);

/**
*   Squroot of a
*
*   return c such that c^2 = a
*
**/
UINT32 ebgn_r_sqrt(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = 1 / ebgn_a
*
* where ebgn_a != 0
*
**/
UINT32 ebgn_r_inv(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = ln(x) = SUM( (-1)^i *((x - 1) ^ i) / i, i = 1,2,...)
*
* where x = ebgn_a
*
**/
UINT32 ebgn_r_ln(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);

/**
*
*
*   ebgn_c = pi = 4 * SUM((-1)^i/(2*i+1), i = 0,1,2,...)
*
*
**/
UINT32 ebgn_r_pi(const UINT32 ebgnr_md_id, EBGN *ebgn_c);

UINT32 ebgn_r_print_dec_info(LOG *log, const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const char *info);
UINT32 ebgn_r_print_bin_info(LOG *log, const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const char *info);
UINT32 ebgn_r_print_hex_info(LOG *log, const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const char *info);

#endif /*_LIB_EBGNR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
