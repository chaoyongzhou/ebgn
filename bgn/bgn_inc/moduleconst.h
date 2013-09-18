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

#ifndef _MODULECONST_H
#define _MODULECONST_H

#include "typeconst.h"

/* window width for BGNZ2 mul precomputation */
#define WINWIDTH_OF_BGNZ2_PREMUL 4

/* window width for BGNZ2 mul precomputation */
#define WINWIDTH_OF_EBGNZ2_PREMUL WINWIDTH_OF_BGNZ2_PREMUL

#if ( WINWIDTH_OF_BGNZ2_PREMUL >= WORDSIZE )
#error "invalid setting: WINWIDTH_OF_BGNZ2_PREMUL >= WORDSIZE"
#endif /* WINWIDTH_OF_BGNZ2_PREMUL >= WORDSIZE */

#if ( 0 != ( WORDSIZE % WINWIDTH_OF_BGNZ2_PREMUL ) )
#error "invalid setting: 0 != ( WINWIDTH_OF_BGNZ2_PREMUL % WORDSIZE )"
#endif /* 0 != ( WINWIDTH_OF_BGNZ2_PREMUL % WORDSIZE ) */

/* this constant defintion is for bgn_z2_ddiv algorithm */
/* let a = b * q + r and M = b * q2 + r2, where M = x ^ {BIGINTSIZE}    */
/* to fast the bgn_z2_ddiv_1, q2 should be as large as possible         */
/* and r2 should be as small as possible.                               */
/* but when b is close to M, e.g, deg(b) = BIGINTSIZE - 1, then         */
/* q2 is very small but r2 is very large which will cause so many loops.*/
/* to avoid such scenario, or to fast bgn_z2_ddiv_1, we restrict deg(r2)*/
/* note that deg(r2) < deg(b), so in fact, we restrict deg(b) < constant*/
#define MAX_DEG_OF_B_FOR_DDIV_1 32

/* table size of BGNZ2 mul precomputation */
#define MAX_SIZE_OF_BGNZ2_PREMUL_TABLE ( 1 << WINWIDTH_OF_BGNZ2_PREMUL )

/* table size of EBGNZ2 mul precomputation */
#define MAX_SIZE_OF_EBGNZ2_PREMUL_TABLE ( 1 << WINWIDTH_OF_EBGNZ2_PREMUL )

#define NUM_OF_ECCF2N_LOSS_BITS 4

#define MAX_LOOP_LEN_OF_SIGNATURE 99

/* MAX_NAF_ARRAY_SIZE: how long per NAF-array */
#define MAX_NAF_ARRAY_LEN              (BIGINTSIZE + 32)

/* bigintf2n.h include: */
/* max number of BIGINTF2N MODULEs */
#define MAX_NUM_OF_BGNF2N_MD 8

/*bgnz.h include: */
#define MAX_NUM_OF_BGNZ_MD  128

/*ebgnz.h include: */
#define MAX_NUM_OF_EBGNZ_MD  128

/*ebgnz2.h include: */
#define MAX_NUM_OF_EBGNZ2_MD  128

/*bgnzn.h include: */
#define MAX_NUM_OF_BGNZN_MD 8

/* bgnz2.h include: */
#define MAX_NUM_OF_BGNZ2_MD 32

/* ecf2n.h include: */
#define MAX_NUM_OF_ECF2N_MD 4

/* eccf2n.h include: */
#define MAX_NUM_OF_ECCF2N_MD 1

/* bgnfp.h include: */
#define MAX_NUM_OF_BGNFP_MD 8

/* ecfp.h include: */
#define MAX_NUM_OF_ECFP_MD 4

/* eccfp.h include: */
#define MAX_NUM_OF_ECCFP_MD 1

/*convert.h include:*/
#define MAX_NUM_OF_CONV_MD 32

/*cmpie.h include:*/
#define MAX_NUM_OF_ENCODE_MD 32

/*polyz.h include:*/
//#define MAX_NUM_OF_POLYZ_MD 1

/*polyz2.h include:*/
#define MAX_NUM_OF_POLYZ2_MD 32

/*polyzn.h include:*/
#define MAX_NUM_OF_POLYZN_MD 2

/*polyfp.h include:*/
#define MAX_NUM_OF_POLYFP_MD 1

/*polyf2n.h include:*/
#define MAX_NUM_OF_POLYF2N_MD 1

/*seafp.h include:*/
#define MAX_NUM_OF_SEAFP_MD 1

/*task.h include:*/
#define MAX_NUM_OF_TASK_MD 1024

/*matrixr.h include:*/
#define MAX_NUM_OF_MATRIXR_MD 1024

/*vectorr.h include:*/
#define MAX_NUM_OF_VECTORR_MD 1024

/*ebgnr.h include: */
#define MAX_NUM_OF_EBGNR_MD  64

#endif/* _MODULECONST_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

