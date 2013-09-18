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

#ifndef _PRINT_H
#define _PRINT_H

#include <stdio.h>

#include "type.h"

#define PRINT_MAX_DEPTH_OF_POLY_ITEM 3

typedef struct
{
    const BIGINT *coe;
    const DEGREE *deg;
}POLY_ITEM_NODE;

typedef struct
{
    POLY_ITEM_NODE node[PRINT_MAX_DEPTH_OF_POLY_ITEM];
    UINT32  depth;
}POLY_ITEM_BUF;

void print_cur_time(LOG *log, const UINT8 *info );

void print_words(LOG *log,const UINT32* a, const UINT32 a_len );

void print_bitarry(LOG *log,const UINT32 *bits,const UINT32 width,const UINT32 nbits);

void print_bits(LOG *log, const BIGINT *src);

void print_odd_bits(LOG *log, const BIGINT *src);

void print_even_bits(LOG *log, const BIGINT *src);

void print_str(LOG *log,const UINT8 *srcstr, const UINT32 srcstrlen);

void print_chars(LOG *log,const UINT8 *src_chars, const UINT32 src_charnum);

void print_msg(LOG *log,const UINT8 *msg, const UINT32 msglen);

void print_uint32(LOG *log,const UINT32 src );

void print_real(LOG *log,const REAL *src );

void print_vectorr_block(LOG *log, const VECTOR_BLOCK *vector_block);

void print_vectorr(LOG *log,const VECTOR *vector );
void print_matrixr(LOG *log, const MATRIX *matrix);

void print_bigint(LOG *log,const BIGINT* src );

void print_bgn_dec_info(LOG *log,const BIGINT * src, const char *info);

void print_deg(LOG *log,const DEGREE * deg);

void print_deg_dec(LOG *log,const DEGREE * deg);

void print_deg_decchars(LOG *log,const DEGREE * deg);

void print_point(LOG *log,const EC_CURVE_POINT * point);

void print_aff_point(LOG *log,const EC_CURVE_AFF_POINT * point);

void print_keypair(LOG *log,const ECC_KEYPAIR* keypair);

void print_bigint_bgn_format(LOG *log,const BIGINT* src );

void print_point_bgn_format(LOG *log,const EC_CURVE_POINT * point);

void print_aff_point_bgn_format(LOG *log,const EC_CURVE_AFF_POINT * point);

void print_keypair_bgn_format(LOG *log,const ECC_KEYPAIR* keypair);

void print_ecf2n_curve(LOG *log,const ECF2N_CURVE* ecf2n_curve);

void print_ecf2n_curve_bgn_format(LOG *log,const ECF2N_CURVE* ecf2n_curve);

void print_ecfp_curve(LOG *log,const ECFP_CURVE* ecfp_curve);

void print_ecfp_curve_bgn_format(LOG *log,const ECFP_CURVE* ecfp_curve);

void print_ecc_signature(LOG *log,const ECC_SIGNATURE* ecc_signature);

void print_ecc_signature_bgn_format(LOG *log,const ECC_SIGNATURE* ecc_signature);

void print_poly(LOG *log,const POLY* poly);

void print_poly_item(LOG *log,const POLY_ITEM* item);

void print_maple(LOG *log,const char *varstr,const BIGINT *src);

void print_point_maple(LOG *log,const char *varstr,const EC_CURVE_POINT * point);

void print_aff_point_maple(LOG *log,const char *varstr,const EC_CURVE_AFF_POINT * point);

void print_uint32_dec(LOG *log,const UINT32 src );

void print_bigint_dec(LOG *log,const BIGINT * src);

void print_bigint_decchars(LOG *log,const BIGINT * src);

void print_point_dec(LOG *log,const EC_CURVE_POINT * point);

void print_aff_point_dec(LOG *log,const EC_CURVE_AFF_POINT * point);

void print_keypair_dec(LOG *log,const ECC_KEYPAIR* keypair);

void print_ecf2n_curve_dec(LOG *log,const ECF2N_CURVE* ecf2n_curve);

void print_ecfp_curve_dec(LOG *log,const ECFP_CURVE* ecfp_curve);

void print_ecc_signature_dec(LOG *log,const ECC_SIGNATURE* ecc_signature);

void print_poly_dec(LOG *log,const POLY* poly);

void print_poly_item_dec(LOG *log,const POLY_ITEM* item);

void print_ebgn_dec(LOG *log,const EBGN * src);
void print_ebgn_hex(LOG *log,const EBGN * src);
void print_ebgn_bin(LOG *log,const EBGN * src);

void print_ebgn(LOG *log, const EBGN *ebgn);
void print_ebgn_info(LOG *log, const EBGN *ebgn, const char *info);

void print_ebgn_dec_info(LOG *log, const EBGN *ebgn, const char *info);
void print_ebgn_hex_info(LOG *log, const EBGN *ebgn, const char *info);
void print_ebgn_bin_info(LOG *log, const EBGN *ebgn, const char *info);

#endif /* _PRINT_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

