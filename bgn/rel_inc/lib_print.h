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
#ifndef _LIB_PRINT_H
#define _LIB_PRINT_H

#include <stdio.h>

#include "lib_type.h"


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
void print_bigint(LOG * log,const BIGINT* src );
void print_bgn_dec_info(LOG *log,const BIGINT * src, const char *info);

void print_deg(LOG *log,const DEGREE * deg);
void print_deg_dec(LOG *log,const DEGREE * deg);
void print_deg_decchars(LOG *log,const DEGREE * deg);
void print_point(LOG * log,const EC_CURVE_POINT * point);
void print_aff_point(LOG * log,const EC_CURVE_AFF_POINT * point);
void print_keypair(LOG * log,const ECC_KEYPAIR* keypair);


void print_ecf2n_curve(LOG *log,const ECF2N_CURVE* ecf2n_curve);
void print_ecfp_curve(LOG *log,const ECFP_CURVE* ecfp_curve);
void print_ecc_signature(LOG *log,const ECC_SIGNATURE* ecc_signature);
void print_poly(LOG *log,const POLY* poly);
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

void print_ebgn_dec(LOG *log,const EBGN * src);
void print_ebgn_hex(LOG *log,const EBGN * src);
void print_ebgn_bin(LOG *log,const EBGN * src);

void print_ebgn_dec_info(LOG *log, const EBGN *ebgn, const char *info);
void print_ebgn_hex_info(LOG *log, const EBGN *ebgn, const char *info);
void print_ebgn_bin_info(LOG *log, const EBGN *ebgn, const char *info);

#endif /* _LIB_PRINT_H */
#ifdef __cplusplus
}
#endif/*__cplusplus*/

