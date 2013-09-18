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

#ifndef _CONV_H
#define _CONV_H

#include "type.h"

#include "task.h"

typedef struct
{
    UINT32 usedcounter;/* used counter >= 0 */

    UINT32 bgnz_md_id;
    UINT32 ebgnz_md_id;

    UINT32 vectorr_md_id;
    UINT32 matrixr_md_id;
}CONV_MD;

/**
*   for test only
*
*   to query the status of CONV Module
*
**/
void conv_print_module_status(const UINT32 conv_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CONV module
*
*
**/
UINT32 conv_free_module_static_mem(const UINT32 conv_md_id);

/**
*
* start CONV module
*
**/
UINT32 conv_start();

/**
*
* end CONV module
*
**/
void conv_end(const UINT32 conv_md_id);

/*--------------------------------------------BIGINT----------------------------------------------------*/

/**
*
*   convert a decimal string to a bigint.
*   if the decimal is overflow the limit of bigint,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the bigint value of the decimal string and return 0.
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_dec_to_bgn(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,BIGINT *des);

/**
*
*   convert a bigint to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the bigint and return 0.
*
**/
UINT32 conv_bgn_to_dec(const UINT32 conv_md_id,const BIGINT *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert a hex string to a BIGINT number.
*   if the hex number is overflow the limit of BIGINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the BIGINT value of the hex string and return 0.
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*
**/
UINT32 conv_hex_to_bgn(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,BIGINT *des);

/**
*
*   convert a BIGINT number to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of the BIGINT number and return 0.
*
**/
UINT32 conv_bgn_to_hex(const UINT32 conv_md_id,const BIGINT *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a binary string to a BIGINT number.
*   if the binary number is overflow the limit of BIGINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the BIGINT value of the binary string and return 0.
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*
**/
UINT32 conv_bin_to_bgn(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,BIGINT *des);

/**
*
*   convert a BIGINT number to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of the BIGINT number and return 0.
*
**/
UINT32 conv_bgn_to_bin(const UINT32 conv_md_id,const BIGINT *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------DEGREE----------------------------------------------------*/
#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )

#define conv_dec_to_deg conv_dec_to_bgn
#define conv_hex_to_deg conv_hex_to_bgn
#define conv_bin_to_deg conv_bin_to_bgn

#define conv_deg_to_dec conv_bgn_to_dec
#define conv_deg_to_hex conv_bgn_to_hex
#define conv_deg_to_bin conv_bgn_to_bin

#endif /*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )

#define conv_dec_to_deg conv_dec_to_uint32
#define conv_hex_to_deg conv_hex_to_uint32
#define conv_bin_to_deg conv_bin_to_uint32

#define conv_deg_to_dec conv_uint32_to_dec
#define conv_deg_to_hex conv_uint32_to_hex
#define conv_deg_to_bin conv_uint32_to_bin

#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

/*--------------------------------------------EC_CURVE_POINT----------------------------------------------------*/
/**
*
*   convert a decimal string to a EC_CURVE_POINT.
*   if the decimal is overflow the limit of element of EC_CURVE_POINT,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the EC_CURVE_POINT value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of EC_CURVE_POINT,
*         i.e., its decimal string format is: x#y#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ec_curve_point(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,EC_CURVE_POINT *des);

/**
*
*   convert a EC_CURVE_POINT to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_POINT,
*         i.e., its decimal string format is: x#y#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of EC_CURVE_POINT and return 0.
*
**/
UINT32 conv_ec_curve_point_to_dec(const UINT32 conv_md_id,const EC_CURVE_POINT*src,UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert a hex string to EC_CURVE_POINT.
*   if the hex number is overflow the limit of element of EC_CURVE_POINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is EC_CURVE_POINT value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_POINT,
*         i.e., its decimal string format is: x#y#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ec_curve_point(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,EC_CURVE_POINT *des);

/**
*
*   convert EC_CURVE_POINT to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_POINT,
*         i.e., its decimal string format is: x#y#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of EC_CURVE_POINT and return 0.
*
**/
UINT32 conv_ec_curve_point_to_hex(const UINT32 conv_md_id,const EC_CURVE_POINT *src,UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a binary string to EC_CURVE_POINT
*   if the binary number is overflow the limit of EC_CURVE_POINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the EC_CURVE_POINT value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_POINT,
*         i.e., its decimal string format is: x#y#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ec_curve_point(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,EC_CURVE_POINT *des);

/**
*
*   convert EC_CURVE_POINT to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of EC_CURVE_POINT and return 0.
*
**/
UINT32 conv_ec_curve_point_to_bin(const UINT32 conv_md_id,const EC_CURVE_POINT *src,UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------EC_CURVE_AFF_POINT----------------------------------------------------*/
/**
*
*   convert a decimal string to a EC_CURVE_AFF_POINT.
*   if the decimal is overflow the limit of element of EC_CURVE_AFF_POINT,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the EC_CURVE_AFF_POINT value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of EC_CURVE_AFF_POINT,
*	  i.e., its decimal string format is: x#y#z#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ec_curve_aff_point(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,EC_CURVE_AFF_POINT *des);

/**
*
*   convert a EC_CURVE_AFF_POINT to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_AFF_POINT,
*	  i.e., its decimal string format is: x#y#z#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of EC_CURVE_AFF_POINT and return 0.
*
**/
UINT32 conv_ec_curve_aff_point_to_dec(const UINT32 conv_md_id,const EC_CURVE_AFF_POINT *src,UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert a hex string to EC_CURVE_AFF_POINT.
*   if the hex number is overflow the limit of element of EC_CURVE_AFF_POINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is EC_CURVE_AFF_POINT value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_AFF_POINT,
*	  i.e., its decimal string format is: x#y#z#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ec_curve_aff_point(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,EC_CURVE_AFF_POINT *des);

/**
*
*   convert EC_CURVE_AFF_POINT to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_AFF_POINT,
*	  i.e., its decimal string format is: x#y#z#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of EC_CURVE_AFF_POINT and return 0.
*
**/
UINT32 conv_ec_curve_aff_point_to_hex(const UINT32 conv_md_id,const EC_CURVE_AFF_POINT *src,UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a binary string to EC_CURVE_AFF_POINT
*   if the binary number is overflow the limit of EC_CURVE_AFF_POINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the EC_CURVE_AFF_POINT value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_AFF_POINT,
*	  i.e., its decimal string format is: x#y#z#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ec_curve_aff_point(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,EC_CURVE_AFF_POINT *des);

/**
*
*   convert EC_CURVE_AFF_POINT to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of EC_CURVE_AFF_POINT and return 0.
*
**/
UINT32 conv_ec_curve_aff_point_to_bin(const UINT32 conv_md_id,const EC_CURVE_AFF_POINT *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------ECF2N_CURVE----------------------------------------------------*/
/**
*
*   convert a decimal string to a ECF2N_CURVE.
*   if the decimal is overflow the limit of element of ECF2N_CURVE,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the ECF2N_CURVE value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of ECF2N_CURVE,
*         i.e., its decimal string format is: a#b#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ecf2n_curve(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,ECF2N_CURVE *des);

/**
*
*   convert a ECF2N_CURVE to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECF2N_CURVE,
*         i.e., its decimal string format is: a#b#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of ECF2N_CURVE and return 0.
*
**/
UINT32 conv_ecf2n_curve_to_dec(const UINT32 conv_md_id,const ECF2N_CURVE *src,UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert a hex string to ECF2N_CURVE.
*   if the hex number is overflow the limit of element of ECF2N_CURVE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is ECF2N_CURVE value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECF2N_CURVE,
*         i.e., its decimal string format is: a#b#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ecf2n_curve(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,ECF2N_CURVE *des);

/**
*
*   convert ECF2N_CURVE to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECF2N_CURVE,
*         i.e., its decimal string format is: a#b#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of ECF2N_CURVE and return 0.
*
**/
UINT32 conv_ecf2n_curve_to_hex(const UINT32 conv_md_id,const ECF2N_CURVE *src,UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a binary string to ECF2N_CURVE
*   if the binary number is overflow the limit of ECF2N_CURVE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the ECF2N_CURVE value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECF2N_CURVE,
*         i.e., its decimal string format is: a#b#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ecf2n_curve(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,ECF2N_CURVE *des);

/**
*
*   convert ECF2N_CURVE to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of ECF2N_CURVE and return 0.
*
**/
UINT32 conv_ecf2n_curve_to_bin(const UINT32 conv_md_id,const ECF2N_CURVE *src,UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------ECFP_CURVE----------------------------------------------------*/
/**
*
*   convert a decimal string to a ECFP_CURVE.
*   if the decimal is overflow the limit of element of ECFP_CURVE,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the ECFP_CURVE value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of ECFP_CURVE,
*	  i.e., its decimal string format is: a#b#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ecfp_curve(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,ECFP_CURVE *des);

/**
*
*   convert a ECFP_CURVE to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECFP_CURVE,
*	  i.e., its decimal string format is: a#b#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of ECFP_CURVE and return 0.
*
**/
UINT32 conv_ecfp_curve_to_dec(const UINT32 conv_md_id,const ECFP_CURVE *src,UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert a hex string to ECFP_CURVE.
*   if the hex number is overflow the limit of element of ECFP_CURVE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is ECFP_CURVE value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECFP_CURVE,
*	  i.e., its decimal string format is: a#b#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ecfp_curve(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,ECFP_CURVE *des);

/**
*
*   convert ECFP_CURVE to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECFP_CURVE,
*	  i.e., its decimal string format is: a#b#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of ECFP_CURVE and return 0.
*
**/
UINT32 conv_ecfp_curve_to_hex(const UINT32 conv_md_id,const ECFP_CURVE *src,UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a binary string to ECFP_CURVE
*   if the binary number is overflow the limit of ECFP_CURVE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the ECFP_CURVE value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECFP_CURVE,
*	  i.e., its decimal string format is: a#b#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ecfp_curve(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,ECFP_CURVE *des);

/**
*
*   convert ECFP_CURVE to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of ECFP_CURVE and return 0.
*
**/
UINT32 conv_ecfp_curve_to_bin(const UINT32 conv_md_id,const ECFP_CURVE *src,UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);


/*--------------------------------------------ECC_KEYPAIR----------------------------------------------------*/
/**
*
*   convert private_key decimal string to private_key ECC_KEYPAIR.
*   if the decimal is overflow the limit of element of ECC_KEYPAIR,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the ECC_KEYPAIR value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of ECC_KEYPAIR,
*	  i.e., its decimal string format is: a#b#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ecc_keypair(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,ECC_KEYPAIR *des);

/**
*
*   convert private_key ECC_KEYPAIR to private_key decimal string.
*   the destinate decimal string should be given private_key buffer with private_key maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECC_KEYPAIR,
*	  i.e., its decimal string format is: a#b#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of ECC_KEYPAIR and return 0.
*
**/
UINT32 conv_ecc_keypair_to_dec(const UINT32 conv_md_id,const ECC_KEYPAIR *src,UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert private_key hex string to ECC_KEYPAIR.
*   if the hex number is overflow the limit of element of ECC_KEYPAIR,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is ECC_KEYPAIR value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECC_KEYPAIR,
*	  i.e., its decimal string format is: a#b#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ecc_keypair(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,ECC_KEYPAIR *des);

/**
*
*   convert ECC_KEYPAIR to private_key hex string.
*   the destinate hex string should be given private_key buffer with private_key maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECC_KEYPAIR,
*	  i.e., its decimal string format is: a#b#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of ECC_KEYPAIR and return 0.
*
**/
UINT32 conv_ecc_keypair_to_hex(const UINT32 conv_md_id,const ECC_KEYPAIR *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert private_key binary string to ECC_KEYPAIR
*   if the binary number is overflow the limit of ECC_KEYPAIR,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the ECC_KEYPAIR value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECC_KEYPAIR,
*	  i.e., its decimal string format is: a#b#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ecc_keypair(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,ECC_KEYPAIR *des);

/**
*
*   convert ECC_KEYPAIR to private_key binary string.
*   the destinate binary string should be given private_key buffer with private_key maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of ECC_KEYPAIR and return 0.
*
**/
UINT32 conv_ecc_keypair_to_bin(const UINT32 conv_md_id,const ECC_KEYPAIR *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------ECC_SIGNATURE----------------------------------------------------*/
/**
*
*   convert decimal string to ECC_SIGNATURE.
*   if the decimal is overflow the limit of element of ECC_SIGNATURE,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the ECC_SIGNATURE value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of ECC_SIGNATURE,
*	  i.e., its decimal string format is: a#b#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ecc_signature(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,ECC_SIGNATURE *des);

/**
*
*   convert ECC_SIGNATURE to decimal string.
*   the destinate decimal string should be given  buffer with  maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECC_SIGNATURE,
*	  i.e., its decimal string format is: a#b#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of ECC_SIGNATURE and return 0.
*
**/
UINT32 conv_ecc_signature_to_dec(const UINT32 conv_md_id,const ECC_SIGNATURE *src,UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert r hex string to ECC_SIGNATURE.
*   if the hex number is overflow the limit of element of ECC_SIGNATURE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is ECC_SIGNATURE value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECC_SIGNATURE,
*	  i.e., its decimal string format is: a#b#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ecc_signature(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,ECC_SIGNATURE *des);

/**
*
*   convert ECC_SIGNATURE to r hex string.
*   the destinate hex string should be given r buffer with r maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECC_SIGNATURE,
*	  i.e., its decimal string format is: a#b#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of ECC_SIGNATURE and return 0.
*
**/
UINT32 conv_ecc_signature_to_hex(const UINT32 conv_md_id,const ECC_SIGNATURE *src,UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert r binary string to ECC_SIGNATURE
*   if the binary number is overflow the limit of ECC_SIGNATURE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the ECC_SIGNATURE value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECC_SIGNATURE,
*	  i.e., its decimal string format is: a#b#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ecc_signature(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,ECC_SIGNATURE *des);

/**
*
*   convert ECC_SIGNATURE to r binary string.
*   the destinate binary string should be given r buffer with r maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of ECC_SIGNATURE and return 0.
*
**/
UINT32 conv_ecc_signature_to_bin(const UINT32 conv_md_id,const ECC_SIGNATURE *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------POLY----------------------------------------------------*/
/**
*
*   convert a decimal string to a POLY.
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_dec_to_poly(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,POLY *des);

/**
*
*   convert a POLY to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the POLY and return 0.
*
**/
UINT32 conv_poly_to_dec(const UINT32 conv_md_id,const POLY *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert a hex string to a POLY.
*
*   note:
*       if the hex string contains any exceptional character,i.e., not belong to [0-9a-fA-F], then
*   return -1 without warning information.
*
**/
UINT32 conv_hex_to_poly(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,POLY *des);

/**
*
*   convert a POLY to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the POLY and return 0.
*
**/
UINT32 conv_poly_to_hex(const UINT32 conv_md_id,const POLY *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a bin string to a POLY.
*
*   note:
*       if the bin string contains any exceptional character,i.e., not belong to [0-1], then
*   return -1 without warning information.
*
**/
UINT32 conv_bin_to_poly(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,POLY *des);

/**
*
*   convert a POLY to a bin string.
*   the destinate bin string should be given a buffer with a maximum lenght at first.
*
*   if the bin string buffer is not enough, then warn and return -1.
*   otherwise, return the bin expression of the POLY and return 0.
*
**/
UINT32 conv_poly_to_bin(const UINT32 conv_md_id,const POLY *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------POLY_ITEM----------------------------------------------------*/
/**
*
*   convert a decimal string to a POLY_ITEM.
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_dec_to_poly_item(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,POLY_ITEM *des);

/**
*
*   convert a POLY_ITEM to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the POLY_ITEM and return 0.
*
**/
UINT32 conv_poly_item_to_dec(const UINT32 conv_md_id,const POLY_ITEM *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert a hex string to a POLY_ITEM.
*
*   note:
*       if the hex string contains any exceptional character,i.e., not belong to [0-9a-fA-F], then
*   return -1 without warning information.
*
**/
UINT32 conv_hex_to_poly_item(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,POLY_ITEM *des);

/**
*
*   convert a POLY_ITEM to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the POLY_ITEM and return 0.
*
**/
UINT32 conv_poly_item_to_hex(const UINT32 conv_md_id,const POLY_ITEM *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a bin string to a POLY_ITEM.
*
*   note:
*       if the bin string contains any exceptional character,i.e., not belong to [0-1], then
*   return -1 without warning information.
*
**/
UINT32 conv_bin_to_poly_item(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,POLY_ITEM *des);

/**
*
*   convert a POLY_ITEM to a bin string.
*   the destinate bin string should be given a buffer with a maximum lenght at first.
*
*   if the bin string buffer is not enough, then warn and return -1.
*   otherwise, return the bin expression of the POLY_ITEM and return 0.
*
**/
UINT32 conv_poly_item_to_bin(const UINT32 conv_md_id,const POLY_ITEM *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------UINT32----------------------------------------------------*/

/**
*
*   convert a decimal string to a uint32.
*   if the decimal is overflow the limit of UINT32,i.e, decimal >= 2 ^{UINT32SIZE},then
*   report error and return -1.
*   else des is the bigint value of the decimal string and return 0.
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_dec_to_uint32(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,UINT32 *des);

/**
*
*   convert a uint32 to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the bigint and return 0.
*
**/
UINT32 conv_uint32_to_dec(const UINT32 conv_md_id,const UINT32 src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);
UINT32 conv_uint32_to_dec_0(const UINT32 conv_md_id,const UINT32 *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert a hex string to a UINT32.
*   if the hex is overflow the limit of UINT32,i.e, hex >= 2 ^{UINT32SIZE},then
*   report error and return -1.
*   else des is the UINT32 value of the hex string and return 0.
*
*   note:
*       if the hex string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_hex_to_uint32(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,UINT32 *des);

/**
*
*   convert a UINT32 to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the UINT32 and return 0.
*
**/
UINT32 conv_uint32_to_hex(const UINT32 conv_md_id,const UINT32 src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);
UINT32 conv_uint32_to_hex_0(const UINT32 conv_md_id,const UINT32 *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a bin string to a UINT32.
*   if the bin is overflow the limit of UINT32,i.e, bin >= 2 ^{UINT32SIZE},then
*   report error and return -1.
*   else des is the UINT32 value of the bin string and return 0.
*
*   note:
*       if the bin string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_bin_to_uint32(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,UINT32 *des);

/**
*
*   convert a UINT32 to a bin string.
*   the destinate bin string should be given a buffer with a maximum lenght at first.
*
*   if the bin string buffer is not enough, then warn and return -1.
*   otherwise, return the bin expression of the UINT32 and return 0.
*
**/
UINT32 conv_uint32_to_bin(const UINT32 conv_md_id,const UINT32 src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);
UINT32 conv_uint32_to_bin_0(const UINT32 conv_md_id,const UINT32 *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------REAL----------------------------------------------------*/
UINT32 conv_real_to_dec(const UINT32 conv_md_id,const REAL * src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);
UINT32 conv_dec_to_real(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,REAL *des);

UINT32 conv_real_to_hex(const UINT32 conv_md_id,const REAL * src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);
UINT32 conv_hex_to_real(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,REAL *des);

UINT32 conv_real_to_bin(const UINT32 conv_md_id,const REAL * src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);
UINT32 conv_bin_to_real(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,REAL *des);

/*--------------------------------------------VECTORR BLOCK ----------------------------------------------------*/
UINT32 conv_vectorr_block_to_dec(const UINT32 conv_md_id,const VECTOR_BLOCK * vector_block, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

UINT32 conv_dec_to_vectorr_block(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,VECTOR_BLOCK *vector_block);

UINT32 conv_vectorr_block_to_hex(const UINT32 conv_md_id,const VECTOR_BLOCK * vector_block, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

UINT32 conv_hex_to_vectorr_block(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,VECTOR_BLOCK *vector_block);

UINT32 conv_vectorr_block_to_bin(const UINT32 conv_md_id,const VECTOR_BLOCK * vector_block, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

UINT32 conv_bin_to_vectorr_block(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,VECTOR_BLOCK *vector_block);

/*--------------------------------------------VECTORR----------------------------------------------------*/

UINT32 conv_vectorr_to_dec(const UINT32 conv_md_id,const VECTOR * vector, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);
UINT32 conv_dec_to_vectorr(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,VECTOR *vector);

UINT32 conv_vectorr_to_hex(const UINT32 conv_md_id,const VECTOR * vector, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);
UINT32 conv_hex_to_vectorr(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,VECTOR *vector);

UINT32 conv_vectorr_to_bin(const UINT32 conv_md_id,const VECTOR * vector, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);
UINT32 conv_bin_to_vectorr(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,VECTOR *vector);

/*--------------------------------------------MATRIXR BLOCK----------------------------------------------------*/
UINT32 conv_matrixr_block_to_dec(const UINT32 conv_md_id,const MATRIX_BLOCK * matrix_block, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

UINT32 conv_dec_to_matrixr_block(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,MATRIX_BLOCK *matrix_block);

UINT32 conv_matrixr_block_to_hex(const UINT32 conv_md_id,const MATRIX_BLOCK * matrix_block, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

UINT32 conv_hex_to_matrixr_block(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,MATRIX_BLOCK *matrix_block);


UINT32 conv_matrixr_block_to_bin(const UINT32 conv_md_id,const MATRIX_BLOCK * matrix_block, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

UINT32 conv_bin_to_matrixr_block(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,MATRIX_BLOCK *matrix_block);

/*--------------------------------------------MATRIXR----------------------------------------------------*/

UINT32 conv_matrixr_to_dec(const UINT32 conv_md_id,const MATRIX * matrix, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);
UINT32 conv_dec_to_matrixr(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,MATRIX *matrix);

UINT32 conv_matrixr_to_hex(const UINT32 conv_md_id,const MATRIX * matrix, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);
UINT32 conv_hex_to_matrixr(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,MATRIX *matrix);

UINT32 conv_matrixr_to_bin(const UINT32 conv_md_id,const MATRIX * matrix, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);
UINT32 conv_bin_to_matrixr(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,MATRIX *matrix);


/*--------------------------------------------UINT16----------------------------------------------------*/
/**
*
*   convert a decimal string to a UINT16.
*   if the decimal is overflow the limit of UINT16,i.e, decimal >= 2 ^{UINT16SIZE},then
*   report error and return -1.
*   else des is the UINT16 value of the decimal string and return 0.
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_dec_to_uint16(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,UINT16 *des);

/**
*
*   convert a UINT16 to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the UINT16 and return 0.
*
**/
UINT32 conv_uint16_to_dec(const UINT32 conv_md_id,const UINT16 src, UINT8 *decstr,const UINT32 decstrmaxlen,UINT32 *decstrlen);
UINT32 conv_uint16_to_dec_0(const UINT32 conv_md_id,const UINT16 *src, UINT8 *decstr,const UINT32 decstrmaxlen,UINT32 *decstrlen);

/**
*
*   convert a hex string to a UINT16.
*   if the hex is overflow the limit of UINT16,i.e, hex >= 2 ^{UINT16SIZE},then
*   report error and return -1.
*   else des is the UINT16 value of the hex string and return 0.
*
*   note:
*       if the hex string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_hex_to_uint16(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,UINT16 *des);

/**
*
*   convert a UINT16 to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the UINT16 and return 0.
*
**/
UINT32 conv_uint16_to_hex(const UINT32 conv_md_id,const UINT16 src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);
UINT32 conv_uint16_to_hex_0(const UINT32 conv_md_id,const UINT16 *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a bin string to a UINT16.
*   if the bin is overflow the limit of UINT16,i.e, bin >= 2 ^{UINT16SIZE},then
*   report error and return -1.
*   else des is the UINT32 value of the bin string and return 0.
*
*   note:
*       if the bin string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_bin_to_uint16(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,UINT16 *des);

/**
*
*   convert a UINT16 to a bin string.
*   the destinate bin string should be given a buffer with a maximum lenght at first.
*
*   if the bin string buffer is not enough, then warn and return -1.
*   otherwise, return the bin expression of the UINT16 and return 0.
*
**/
UINT32 conv_uint16_to_bin(const UINT32 conv_md_id,const UINT16 src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);
UINT32 conv_uint16_to_bin_0(const UINT32 conv_md_id,const UINT16 *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------EBGN----------------------------------------------------*/
/**
*
*   convert a decimal string to a ebgn.
*   return des is the ebgn value of the decimal string and return 0.
*
*   note:
*       1. the first char may be the sgn of integer. '+' means positive integer, '-' means negative integer.
*          if the first char belong to [0-9] which means it is positive integer or zero.
*       2. if the decimal part string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ebgn(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,EBGN *des);

/**
*
*   convert a ebgn to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the ebgn and return 0.
*
**/
UINT32 conv_ebgn_to_dec(const UINT32 conv_md_id,const EBGN *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);

/**
*
*   convert a hex string to a ebgn.
*   return des is the ebgn value of the hex string and return 0.
*
*   note:
*	1. the first char may be the sgn of integer. '+' means positive integer, '-' means negative integer.
*	   if the first char belong to [0-9] which means it is positive integer or zero.
*       2. if the hex part string contains any exceptional character,i.e., not belong to [0-9a-f], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ebgn(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,EBGN *des);

/**
*
*   convert a ebgn to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the ebgn and return 0.
*
**/
UINT32 conv_ebgn_to_hex(const UINT32 conv_md_id,const EBGN *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);

/**
*
*   convert a binary string to a ebgn.
*   return des is the ebgn value of the binary string and return 0.
*
*   note:
*	1. the first char may be the sgn of integer. '+' means positive integer, '-' means negative integer.
*	   if the first char belong to [0-9] which means it is positive integer or zero.
*       2. if the binary part string contains any exceptional character,i.e., not belong to [0-1], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ebgn(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,EBGN *des);

/**
*
*   convert a ebgn to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then warn and return -1.
*   otherwise, return the binary expression of the ebgn and return 0.
*
**/
UINT32 conv_ebgn_to_bin(const UINT32 conv_md_id,const EBGN *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);

/*--------------------------------------------MOD NODE----------------------------------------------*/
UINT32 conv_mod_node_to_dec(const UINT32 conv_md_id,const MOD_NODE * mod_node, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);
UINT32 conv_dec_to_mod_node(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen, MOD_NODE *mod_node);
UINT32 conv_mod_node_to_hex(const UINT32 conv_md_id,const MOD_NODE * mod_node, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);
UINT32 conv_hex_to_mod_node(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen, MOD_NODE *mod_node);
UINT32 conv_mod_node_to_bin(const UINT32 conv_md_id,const MOD_NODE * mod_node, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);
UINT32 conv_bin_to_mod_node(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen, MOD_NODE *mod_node);

/*--------------------------------------------MOD MGR----------------------------------------------*/
UINT32 conv_mod_mgr_to_dec(const UINT32 conv_md_id,const MOD_MGR * mod_mgr, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen);
UINT32 conv_dec_to_mod_mgr(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen, MOD_MGR *mod_mgr);
UINT32 conv_mod_mgr_to_hex(const UINT32 conv_md_id,const MOD_MGR * mod_mgr, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen);
UINT32 conv_hex_to_mod_mgr(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen, MOD_MGR *mod_mgr);
UINT32 conv_mod_mgr_to_bin(const UINT32 conv_md_id,const MOD_MGR * mod_mgr, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen);
UINT32 conv_bin_to_mod_mgr(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen, MOD_MGR *mod_mgr);

/*-------------------------------------------- CONV FREE ----------------------------------------------------*/
/**
*
*   destory the whole ebgn,i.e., all its items but not the ebgn itself.
*   so, when return from this function, ebgn can be refered again without any item.
*
**/
UINT32 conv_ebgn_clean(const UINT32 conv_md_id, EBGN *ebgn);

/**
*
*   the whole ebgn,including its all items and itself.
*   so, when return from this function, ebgn cannot be refered any more
*
**/
UINT32 conv_ebgn_destroy(const UINT32 conv_md_id, EBGN *ebgn);

#endif/*_CONV_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
