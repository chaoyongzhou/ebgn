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

#ifndef _TYPECONST_H
#define _TYPECONST_H

#define DOUBLE_IN_CHAR  32

//#define BIGINTSIZE    (192+32)
#define BIGINTSIZE    (192)

#if ( 0 != (BIGINTSIZE % WORDSIZE) )
#error "fatal error:typeconst.h: BIGINTSIZE % WORDSIZE != 0"
#endif/*( 0 != (BIGINTSIZE % WORDSIZE) )*/

#define INTMAX      ( BIGINTSIZE  / WORDSIZE )

//#define MAX_NUM_OF_POLY_VAR       3

//#define MATRIX_VECTOR_WIDTH     16 /* row num and col num of matrix block or vector block*/

#define NBITS_TO_NWORDS(nbits)         (((nbits) + WORDSIZE - 1) / WORDSIZE)
#define NBITS_TO_NBYTES(nbits)         (((nbits) + BYTESIZE - 1) / BYTESIZE) 
#define NBYTES_TO_NWORDS(nbytes)       (((nbytes) + (WORDSIZE / BYTESIZE) - 1) / (WORDSIZE / BYTESIZE))
#define NWORDS_TO_NBYTES(nwords)       ((nwords) * (WORDSIZE / BYTESIZE))

#endif /*_TYPECONST_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

