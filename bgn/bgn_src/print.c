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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "bgnctrl.h"

#include "type.h"

#include "mm.h"

#include "conv.h"

#include "ebgnz.h"
#include "poly.h"

#include "vector.h"
#include "matrix.h"

#include "vectorr.h"
#include "matrixr.h"

#include "print.h"
#include "cmisc.h"

#include "log.h"

static void print_poly_with_depth(LOG *log,const POLY* poly,POLY_ITEM_BUF *poly_item_buf);
static UINT32 print_poly_item_with_depth(LOG *log,const POLY_ITEM* item,POLY_ITEM_BUF *poly_item_buf);

static void print_poly_dec_with_depth(LOG *log,const POLY* poly,POLY_ITEM_BUF *poly_item_buf);
static UINT32 print_poly_item_dec_with_depth(LOG *log,const POLY_ITEM* item,POLY_ITEM_BUF *poly_item_buf);

#if 0 /*This part is for Windows OS*/
void print_cur_time(LOG *log, const UINT8 *info )
{
    struct   tm   cur_time;

#if 0
struct tm {
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };
#endif

    _getsystime(&cur_time);

#if 1
    sys_print(log, "time: %02d:%02d:%02d: %s",
        cur_time.tm_hour,
        cur_time.tm_min,
        cur_time.tm_sec,
        info);
#else
    sys_print(log,"time %s: %s\n", asctime( &cur_time ), info);
#endif
    return ;
}
#else/*This part is for Linux OS*/
void print_cur_time(LOG *log, const UINT8 *info )
{
   struct tm *cur_time;
   
   cur_time = c_localtime_r(NULL_PTR);

   sys_print(log, "time: %02d:%02d:%02d: %s",
                    cur_time->tm_hour,
                    cur_time->tm_min,
                    cur_time->tm_sec,
        info);
   return ;
}
#endif
void print_words(LOG *log,const UINT32* a, const UINT32 a_len )
{
    int i;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_words: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == a )
    {
        sys_print(LOGSTDOUT,"error:print_words: a is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    sys_print(log,"0x");

    for(i = a_len; i > 0; )
    {
        i --;
        sys_print(log,"%lx ",a[i]);
    }
    sys_print(log,"\n");
}

void print_bitarry(LOG *log,const UINT32 *bits,const UINT32 width,const UINT32 nbits)
{
    UINT32 index;
    UINT32 tmp;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_bitarry: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == bits )
    {
        sys_print(LOGSTDOUT,"error:print_bitarry: bits is NULL_PTR.\n");
        return ;
    }
    if ( WORDSIZE < width )
    {
        sys_print(LOGSTDOUT,"error:print_bitarry: width = %ld is too long.\n",width);
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    /* width = 32; */

    tmp = 0;
    for ( index = 0; index < nbits; index ++ )
    {
        sys_print(log,"%d",bits[ index ]);

        tmp = (tmp >> 1) | ( bits[ index ] << ( WORDSIZE - 1 ) );
        if ( index % width == ( width - 1 ) )
        {
            sys_print(log," --> %0x\n", (tmp >> (WORDSIZE - width)));

            tmp = 0; /*reset tmp*/
        }
    }

    if ( ( index - 1) % width != ( width - 1 ))
    {
        sys_print(log," --> %0x\n", (tmp >> (WORDSIZE - (index % width))));
    }

    sys_print(log,"\n");
}

void print_bits(LOG *log, const BIGINT *src)
{
    UINT32 tmp;
    UINT32 e;
    UINT32 word_idx;
    UINT32 bit_idx;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_bits: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_bits: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    if ( INTMAX < src->len )
    {
        sys_print(LOGSTDOUT,"error:print_bits: src len = %ld is invali.\n",src->len);
        return ;
    }

#if 0
    for ( word_idx = INTMAX; word_idx > src->len; )
    {
        word_idx --;
        sys_print(log,"00000000000000000000000000000000\n");
    }
#endif

    for (word_idx = src->len; word_idx > 0; )
    {
        word_idx --;

        tmp = src->data[ word_idx ];
        e = ( ((UINT32) 1) << ( WORDSIZE - 1 ) );
        for ( bit_idx = WORDSIZE; bit_idx > 0; bit_idx -- )
        {
            if ( tmp & e )
            {
                sys_print(log,"1");
            }
            else
            {
                sys_print(log,"0");
            }
            e >>= 1;
        }
        sys_print(log,"\n");
    }
}

void print_odd_bits(LOG *log, const BIGINT *src)
{
    UINT32 tmp;
    UINT32 e;
    UINT32 word_idx;
    UINT32 bit_idx;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_odd_bits: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_odd_bits: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    if ( INTMAX < src->len )
    {
        sys_print(LOGSTDOUT,"error:print_odd_bits: src len = %ld is invali.\n",src->len);
        return ;
    }

#if 0
    for ( word_idx = INTMAX; word_idx > src->len; )
    {
        word_idx --;
        sys_print(log,"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n");
    }
#endif
    for (word_idx = src->len; word_idx > 0; )
    {
        word_idx --;

        tmp = src->data[ word_idx ];
        e = ( ((UINT32) 1) << ( WORDSIZE - 1 ) );
        for ( bit_idx = WORDSIZE; bit_idx > 0; )
        {
            bit_idx -- ;
            /* if it's the odd-th bit,then print*/
            if ( 1 == (bit_idx & 1) )
            {
                if ( tmp & e )
                {
                    sys_print(log,"1");
                }
                else
                {
                    sys_print(log,"0");
                }
            }
            else
            {
                sys_print(log," ");
            }
            e >>= 1;
        }
        sys_print(log,"\n");
    }
}

void print_even_bits(LOG *log, const BIGINT *src)
{
    UINT32 tmp;
    UINT32 e;
    UINT32 word_idx;
    UINT32 bit_idx;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_even_bits: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_even_bits: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    if ( INTMAX < src->len )
    {
        sys_print(LOGSTDOUT,"error:print_even_bits: src len = %ld is invali.\n",src->len);
        return ;
    }

#if 0
    for ( word_idx = INTMAX; word_idx > src->len; )
    {
        word_idx --;
        sys_print(log," 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
    }
#endif
    for (word_idx = src->len; word_idx > 0; )
    {
        word_idx --;

        tmp = src->data[ word_idx ];
        e = ( ((UINT32) 1) << ( WORDSIZE - 1 ) );
        for ( bit_idx = WORDSIZE; bit_idx > 0; )
        {
            bit_idx -- ;
            /* if it's the odd-th bit,then print*/
            if ( 0 == ( bit_idx & 1 ) )
            {
                if ( tmp & e )
                {
                    sys_print(log,"1");
                }
                else
                {
                    sys_print(log,"0");
                }
            }
            else
            {
                sys_print(log," ");
            }
            e >>= 1;
        }
        sys_print(log,"\n");
    }
}
void print_str(LOG *log,const UINT8 *srcstr, const UINT32 srcstrlen)
{
    //UINT32 index;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_str: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == srcstr )
    {
        sys_print(LOGSTDOUT,"error:print_str: srcstr is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
#if 0
    for ( index = 0; index < srcstrlen; index ++ )
    {
        sys_print(log,"%c",srcstr[ index ]);
        fflush(log);
    }

    sys_print(log,"\n");
    fflush(log);
#else
    sys_print(log,"%.*s\n", srcstrlen, srcstr);
#endif
    return ;
}

void print_chars(LOG *log,const UINT8 *src_chars, const UINT32 src_charnum)
{
    UINT32 index;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_str: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src_chars )
    {
        sys_print(LOGSTDOUT,"error:print_str: srcstr is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    for ( index = 0; index < src_charnum; index ++ )
    {
        sys_print(log,"%c",src_chars[ index ]);
    }

    return ;
}
void print_msg(LOG *log,const UINT8 *msg, const UINT32 msglen)
{

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(log,"error:print_msg: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == msg )
    {
        sys_print(log,"error:print_msg: msg is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    print_str(log, msg, msglen);

    return ;
}

void print_bigint_test(LOG *log,const BIGINT* src )
{
    UINT32 index;

    if ( 0 == src->len)
    {
        for ( index = 6; index > 0; index -- )
        {
            sys_print(log,"%lx ",0);
        }
        sys_print(LOGSTDOUT,"\n");

        return ;
    }

    for ( index = 6; index > src->len; index -- )
    {
        sys_print(log,"%lx ",0);
    }
    for( ; index > 0; )
    {
        index --;
        sys_print(log,"%lx ",src->data[ index ]);
    }
    sys_print(LOGSTDOUT,"\n");

    return ;
}

void print_uint32(LOG *log,const UINT32 src )
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(log,"error:print_uint32: log is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

   sys_print(log,"%lx", src);

   return ;
}

void print_real(LOG *log,const REAL *src )
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(log,"error:print_real: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(log,"error:print_real: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

   sys_print(log,"%+032.15f", *src);

   return ;
}

void print_vectorr_block(LOG *log, const VECTOR_BLOCK *vector_block)
{
    UINT32 sub_idx;

    REAL *data_addr;

    for( sub_idx = 0; sub_idx < MATRIX_VECTOR_WIDTH; sub_idx ++)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);

        if( NULL_PTR != data_addr )
        {
                print_real(log, data_addr);
                sys_print(log, " ");
        }
        else
        {
                sys_print(log,"%+032.15f ", 0.0);
        }
    }
    sys_print(log, "\n");

    return ;
}

void print_vectorr(LOG *log,const VECTOR *vector )
{
    VECTOR_BLOCK *vector_block;
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(log,"error:print_vectorr: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == vector )
    {
        sys_print(log,"error:print_vectorr: vector is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    sys_print(log,"vector: 0x%lx, rotated_flag = %ld, num = %ld\n",
            vector,
            VECTOR_GET_ROTATED_FLAG(vector),
            VECTOR_GET_NUM(vector)
           );

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        print_vectorr_block(log, vector_block);
    }

   return ;
}

static void print_matrixr_row(LOG *log, const MATRIX *matrix, const UINT32 row_no)
{
    MATRIX_BLOCK  *matrix_block;

    UINT32 col_num;
    UINT32 col_no;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    REAL *data_addr;

    col_num = MATRIX_GET_COL_NUM(matrix);

    for(col_no = 0; col_no < col_num; col_no ++)
    {
        block_row_idx = (row_no / MATRIX_VECTOR_WIDTH);
        block_col_idx = (col_no / MATRIX_VECTOR_WIDTH);

        sub_row_idx = (row_no % MATRIX_VECTOR_WIDTH);
        sub_col_idx = (col_no % MATRIX_VECTOR_WIDTH);

        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);

        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block , sub_row_idx, sub_col_idx);

        if( NULL_PTR != data_addr )
        {
                print_real(log, data_addr);
                sys_print(log, " ");
        }
        else
        {
                sys_print(log,"%lf ", 0.0);
        }
    }

    sys_print(log, "\n");

    return ;
}

void print_matrixr(LOG *log, const MATRIX *matrix)
{
    UINT32 row_num;
    UINT32 row_no;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(log,"error:print_matrixr: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == matrix )
    {
        sys_print(log,"error:print_matrixr: matrix is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    sys_print(log,"matrix: 0x%lx, rotated_flag = %ld, row_num = %ld, col_num = %ld\n",
            matrix,
            MATRIX_GET_ROTATED_FLAG(matrix),
            MATRIX_GET_ROW_NUM(matrix),
            MATRIX_GET_COL_NUM(matrix));

    row_num = MATRIX_GET_ROW_NUM(matrix);

    for(row_no = 0; row_no < row_num; row_no ++)
    {
        print_matrixr_row(log, matrix, row_no);
    }

    return ;
}


void print_bigint(LOG *log,const BIGINT* src )
{
    UINT32 i;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_bigint: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_bigint: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    if ( INTMAX < src->len )
    {
        sys_print(LOGSTDOUT,"error:print_bigint: src len = %ld is invalid.\n",src->len);
        return ;
    }

    if ( 0 == src->len)
    {
        sys_print(log,"0x%lx   [len = %ld]\n",0,src->len);
        return ;
    }

    sys_print(log,"0x");

#if 0
    for ( i = INTMAX; i > src->len; )
    {
        i -- ;
        sys_print(log,"%lx ",0);
    }
#endif
    for( i = src->len; i > 0; )
    {
        i --;
        sys_print(log,"%lx ",src->data[i]);
    }
    sys_print(log,"  [len = %ld]\n",src->len);

    return ;
}

void print_dbigint(LOG *log,const BIGINT* a0,const BIGINT *a1 )
{
    print_bigint(log, a1);
    print_bigint(log, a0);
}
void print_bigint_bgn_format(LOG *log,const BIGINT* src )
{
    UINT32 i;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_bigint_bgn_format: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_bigint_bgn_format: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    if ( INTMAX < src->len )
    {
        sys_print(LOGSTDOUT,"{print_bigint_bgn_format:error src len = %ld}\n",src->len);
        return ;
    }

    sys_print(log,"{%ld,",src->len);

    for(i = 0; i < src->len; i ++ )
    {
        sys_print(log,"0x%lx,",src->data[i]);
    }
#if 0
    for(i = src->len; i < INTMAX; i ++ )
    {
        sys_print(log,"0x%lx,",0);
    }
#endif
    sys_print(log,"},\n");
}

void print_point(LOG *log,const EC_CURVE_POINT * point)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_point: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == point )
    {
        sys_print(LOGSTDOUT,"error:print_point: point is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"    x = ");
    print_bigint(log,&point->x);

    sys_print(log,"    y = ");
    print_bigint(log,&point->y);

    sys_print(log,"}    \n");
}

void print_point_bgn_format(LOG *log,const EC_CURVE_POINT * point)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_point_bgn_format: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == point )
    {
        sys_print(LOGSTDOUT,"error:print_point_bgn_format: point is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"/* x */ ");
    print_bigint_bgn_format(log,&point->x);

    sys_print(log,"/* y */ ");
    print_bigint_bgn_format(log,&point->y);

    sys_print(log,"},    \n");
}

void print_aff_point(LOG *log,const EC_CURVE_AFF_POINT * point)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_aff_point: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == point )
    {
        sys_print(LOGSTDOUT,"error:print_aff_point: point is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"    x = ");
    print_bigint(log,&point->x);

    sys_print(log,"    y = ");
    print_bigint(log,&point->y);

    sys_print(log,"    z = ");
    print_bigint(log,&point->z);

    sys_print(log,"}    \n");
}

void print_aff_point_bgn_format(LOG *log,const EC_CURVE_AFF_POINT * point)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_aff_point_bgn_format: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == point )
    {
        sys_print(LOGSTDOUT,"error:print_aff_point_bgn_format: point is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"/* x */ ");
    print_bigint_bgn_format(log,&point->x);

    sys_print(log,"/* y */ ");
    print_bigint_bgn_format(log,&point->y);

    sys_print(log,"/* z */ ");
    print_bigint_bgn_format(log,&point->z);

    sys_print(log,"},   \n");
}

void print_keypair(LOG *log,const ECC_KEYPAIR* keypair)
{
    BIGINT *prikey;
    EC_CURVE_POINT *pubkey;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_keypair: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == keypair )
    {
        sys_print(LOGSTDOUT,"error:print_keypair: keypair is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    prikey = ( BIGINT * )&(keypair->private_key);
    pubkey = ( EC_CURVE_POINT * )&(keypair->public_key);

    sys_print(log,"private key: \n");
    print_bigint(log,prikey);

    sys_print(log,"public key: \n");
    print_point(log,pubkey);

    return ;
}

void print_keypair_bgn_format(LOG *log,const ECC_KEYPAIR* keypair)
{
    BIGINT *prikey;
    EC_CURVE_POINT *pubkey;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_keypair: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == keypair )
    {
        sys_print(LOGSTDOUT,"error:print_keypair: keypair is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    prikey = ( BIGINT * )&(keypair->private_key);
    pubkey = ( EC_CURVE_POINT * )&(keypair->public_key);

    sys_print(log,"private key: \n");
    print_bigint_bgn_format(log,prikey);

    sys_print(log,"public key: \n");
    print_point_bgn_format(log,pubkey);

    return ;
}

void print_ecf2n_curve(LOG *log,const ECF2N_CURVE* ecf2n_curve)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ecf2n_curve: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == ecf2n_curve )
    {
        sys_print(LOGSTDOUT,"error:print_ecf2n_curve: ecf2n_curve is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"/* a */ ");
    print_bigint(log,&(ecf2n_curve->a));

    sys_print(log,"/* b */ ");
    print_bigint(log,&(ecf2n_curve->b));

    sys_print(log,"},   \n");
}

void print_ecf2n_curve_bgn_format(LOG *log,const ECF2N_CURVE* ecf2n_curve)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ecf2n_curve_bgn_format: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == ecf2n_curve )
    {
        sys_print(LOGSTDOUT,"error:print_ecf2n_curve_bgn_format: ecf2n_curve is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"/* a */ ");
    print_bigint_bgn_format(log,&(ecf2n_curve->a));

    sys_print(log,"/* b */ ");
    print_bigint_bgn_format(log,&(ecf2n_curve->b));

    sys_print(log,"},   \n");
}

void print_ecfp_curve(LOG *log,const ECFP_CURVE* ecfp_curve)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ecfp_curve: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == ecfp_curve )
    {
        sys_print(LOGSTDOUT,"error:print_ecfp_curve: ecfp_curve is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"/* a */ ");
    print_bigint(log,&(ecfp_curve->a));

    sys_print(log,"/* b */ ");
    print_bigint(log,&(ecfp_curve->b));

    sys_print(log,"},   \n");
}

void print_ecfp_curve_bgn_format(LOG *log,const ECFP_CURVE* ecfp_curve)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ecfp_curve_bgn_format: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == ecfp_curve )
    {
        sys_print(LOGSTDOUT,"error:print_ecfp_curve_bgn_format: ecfp_curve is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"/* a */ ");
    print_bigint_bgn_format(log,&(ecfp_curve->a));

    sys_print(log,"/* b */ ");
    print_bigint_bgn_format(log,&(ecfp_curve->b));

    sys_print(log,"},   \n");
}

void print_ecc_signature(LOG *log,const ECC_SIGNATURE* ecc_signature)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ecc_signature: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == ecc_signature )
    {
        sys_print(LOGSTDOUT,"error:print_ecc_signature: ecc_signature is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"/* r */ ");
    print_bigint(log,&(ecc_signature->r));

    sys_print(log,"/* s */ ");
    print_bigint(log,&(ecc_signature->s));

    sys_print(log,"},   \n");
}

static UINT32 print_poly_item_with_depth(LOG *log,const POLY_ITEM* item,POLY_ITEM_BUF *poly_item_buf)
{
    UINT32 index;

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
    {
        if ( PRINT_MAX_DEPTH_OF_POLY_ITEM <= poly_item_buf->depth )
        {
            sys_print(LOGSTDOUT,"error:print_poly_item_with_depth: poly_item_buf overflow.\n");
            return ( 0 );
        }

        poly_item_buf->node[ poly_item_buf->depth ].coe = POLY_ITEM_BGN_COE(item);
        poly_item_buf->node[ poly_item_buf->depth ].deg = POLY_ITEM_DEG(item);
        poly_item_buf->depth ++;
#if 0
        for ( index = 0; index < poly_item_buf->depth; index ++ )
        {
            sys_print(log," deg = ");
            print_deg(log, poly_item_buf->node[ index ].deg);

            if ( NULL_PTR == poly_item_buf->node[ index ].coe )
            {
                //sys_print(log," coe = 1");
            }
            else
            {
                sys_print(log," coe = ");
                print_bigint(log, poly_item_buf->node[ index ].coe);
            }
        }
        sys_print(log,"\n");
#endif

#if 1
        for ( index = 0; index < poly_item_buf->depth; index ++ )
        {

            if ( 0 == poly_item_buf->node[ index ].deg->len)
            {
                sys_print(log,"x^0 : ");
            }
            else
            {
                sys_print(log,"x^%d : ",poly_item_buf->node[ index ].deg->data[0]);
            }


            if ( NULL_PTR == poly_item_buf->node[ index ].coe )
            {
                //sys_print(log," coe = 1");
            }
            else
            {
                UINT32 _idx;
                UINT32 _len;
                const BIGINT *_coe;
                _coe = poly_item_buf->node[ index ].coe;
                _len = _coe->len;
                for ( _idx = INTMAX; _idx > _len; )
                {
                    _idx--;
                    sys_print(log,"%lx ",0);
                }
                for( _idx = _len; _idx > 0; )
                {
                    _idx --;
                    sys_print(log,"%lx ",_coe->data[_idx]);
                }
            }
        }
        sys_print(log,"\n");
#endif

        poly_item_buf->depth --;
        poly_item_buf->node[ poly_item_buf->depth ].coe = NULL_PTR;
        poly_item_buf->node[ poly_item_buf->depth ].deg = NULL_PTR;
    }
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item) )
    {
        if ( PRINT_MAX_DEPTH_OF_POLY_ITEM <= poly_item_buf->depth )
        {
            sys_print(LOGSTDOUT,"error:print_poly_item_with_depth: poly_item_buf overflow.\n");
            return ( 0 );
        }

        poly_item_buf->node[ poly_item_buf->depth ].coe = NULL_PTR;/*it represents actually one*/
        poly_item_buf->node[ poly_item_buf->depth ].deg = POLY_ITEM_DEG(item);
        poly_item_buf->depth ++;

        print_poly_with_depth(log, POLY_ITEM_POLY_COE(item), poly_item_buf);
        poly_item_buf->depth --;
        poly_item_buf->node[ poly_item_buf->depth ].coe = NULL_PTR;
        poly_item_buf->node[ poly_item_buf->depth ].deg = NULL_PTR;
    }
    else
    {
        sys_print(LOGSTDOUT,"error:print_poly_item_with_depth: bgn_coe_flag = %lx error.\n");
        return ((UINT32)(-1));
    }

    return ( 0 );
}
static void print_poly_with_depth(LOG *log,const POLY* poly,POLY_ITEM_BUF *poly_item_buf)
{
    POLY_ITEM *item;
    UINT32 ret;

    item = POLY_FIRST_ITEM(poly);

    while( item != POLY_NULL_ITEM(poly) )
    {
        ret = print_poly_item_with_depth(log, item, poly_item_buf);
        if ( 0 != ret )
        {
            sys_print(LOGSTDOUT,"...\n");
            break;
        }

        item = POLY_ITEM_NEXT(item);
    }
    return ;
}
void print_poly(LOG *log,const POLY* poly)
{
    POLY_ITEM_BUF buf_1;
    POLY_ITEM_BUF *poly_item_buf;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_poly: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == poly )
    {
        sys_print(LOGSTDOUT,"error:print_poly: poly is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    poly_item_buf = &buf_1;

    poly_item_buf->depth = 0;

    print_poly_with_depth(log, poly, poly_item_buf);
    return ;
}

void print_poly_item(LOG *log,const POLY_ITEM* item)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_poly_item: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == item )
    {
        sys_print(LOGSTDOUT,"error:print_poly_item: item is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    sys_print(LOGSTDOUT,"item deg = ");
    print_deg(log, POLY_ITEM_DEG(item));
    sys_print(LOGSTDOUT,"\n");

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
    {
        sys_print(LOGSTDOUT,"item coe = ");
        print_bigint(log, POLY_ITEM_BGN_COE(item));
        sys_print(LOGSTDOUT,"\n");
    }
    else
    {
        sys_print(LOGSTDOUT,"item coe = \n");
        print_poly(log, POLY_ITEM_POLY_COE(item));
    }

    return;

    return ;
}

void print_ecc_signature_bgn_format(LOG *log,const ECC_SIGNATURE* ecc_signature)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ecc_signature_bgn_format: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == ecc_signature )
    {
        sys_print(LOGSTDOUT,"error:print_ecc_signature_bgn_format: ecc_signature is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"{    \n");

    sys_print(log,"/* r */ ");
    print_bigint_bgn_format(log,&(ecc_signature->r));

    sys_print(log,"/* s */ ");
    print_bigint_bgn_format(log,&(ecc_signature->s));

    sys_print(log,"},   \n");
}


void print_maple(LOG *log,const char *varstr,const BIGINT *src)
{
    UINT32 index;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_maple: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == varstr )
    {
        sys_print(LOGSTDOUT,"error:print_maple: varstr is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_maple: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    sys_print(log,"%s",varstr);
    sys_print(log,":= convert(`");
#if 0
    for ( index = INTMAX; index > src->len; )
    {
        index -- ;
        sys_print(log,"%lx",0);
    }
#endif
    for( index = src->len ; index > 0; )
    {
        index --;
        sys_print(log,"%lx",src->data[ index ] );
    }
    sys_print(log,"`,decimal,hex);\n");
}

void print_point_maple(LOG *log,const char *varstr,const EC_CURVE_POINT * point)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_point: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == point )
    {
        sys_print(LOGSTDOUT,"error:print_point: point is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    sys_print(log,"%s:\n",varstr);
    sys_print(log,"{    \n");

    print_maple(log,"x",&point->x);

    print_maple(log,"y",&point->y);

    sys_print(log,"}    \n");
}

void print_aff_point_maple(LOG *log,const char *varstr,const EC_CURVE_AFF_POINT * point)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_point: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == point )
    {
        sys_print(LOGSTDOUT,"error:print_point: point is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    sys_print(log,"%s:\n",varstr);
    sys_print(log,"{    \n");

    print_maple(log,"x",&point->x);
    print_maple(log,"y",&point->y);
    print_maple(log,"z",&point->z);

    sys_print(log,"}    \n");
}

void print_uint32_dec(LOG *log,const UINT32 src )
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_uint32_dec: log is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

   sys_print(log,"%ld\n", src);

   return ;
}

void print_bigint_dec(LOG *log,const BIGINT * src)
{
    UINT8  decstr[ BIGINTSIZE ];
    UINT32 decstr_maxlen;
    UINT32 decstr_retlen;
    UINT32 conv_md_id;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_bigint_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_bigint_dec: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    decstr_maxlen = sizeof(decstr)/sizeof(decstr[0]);

    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_print(LOGSTDERR,"print_bigint_dec: no CONV module available\n");
        return ;
    }
    conv_bgn_to_dec(conv_md_id, src, decstr, decstr_maxlen, &decstr_retlen);
    conv_end(conv_md_id);

    print_str(log, decstr, decstr_retlen);;

    return ;
}

void print_bgn_dec_info(LOG *log,const BIGINT * src, const char *info)
{
    sys_print(log, "%s", info);
    print_bigint_dec(log, src);
    return;
}

void print_deg(LOG *log,const DEGREE * deg)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_deg: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == deg )
    {
        sys_print(LOGSTDOUT,"error:print_deg: deg is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
    print_bigint(log, deg);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
    sys_print(log,"%ld\n",(*deg));
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

    return ;
}
void print_deg_dec(LOG *log,const DEGREE * deg)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_degree_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == deg )
    {
        sys_print(LOGSTDOUT,"error:print_degree_dec: deg is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
    print_bigint_dec(log, deg);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
    sys_print(log,"%ld\n",(*deg));
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

    return ;
}

void print_deg_decchars(LOG *log,const DEGREE * deg)
{
    UINT8  decstr[ BIGINTSIZE ];
    UINT32 decstr_maxlen;
    UINT32 decstr_retlen;
    UINT32 conv_md_id;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_deg_decchars: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == deg )
    {
        sys_print(LOGSTDOUT,"error:print_deg_decchars: deg is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
    decstr_maxlen = sizeof(decstr)/sizeof(decstr[0]);

    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_print(LOGSTDERR,"print_deg_decchars: no CONV module available\n");
        return ;
    }
    conv_bgn_to_dec(conv_md_id, deg, decstr, decstr_maxlen, &decstr_retlen);
    conv_end(conv_md_id);

     print_chars(log, decstr, decstr_retlen);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
    sys_print(log,"%ld",(*deg));
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

    return ;
}

void print_bigint_decchars(LOG *log,const BIGINT * src)
{
    UINT8  decstr[ BIGINTSIZE ];
    UINT32 decstr_maxlen;
    UINT32 decstr_retlen;
    UINT32 conv_md_id;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_bigint_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_bigint_dec: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    decstr_maxlen = sizeof(decstr)/sizeof(decstr[0]);

    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_print(LOGSTDERR,"print_bigint_dec: no CONV module available\n");
        return ;
    }
    conv_bgn_to_dec(conv_md_id, src, decstr, decstr_maxlen, &decstr_retlen);
    conv_end(conv_md_id);

    print_chars(log, decstr, decstr_retlen);

    return ;
}
void print_point_dec(LOG *log,const EC_CURVE_POINT * point)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_point_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == point )
    {
        sys_print(LOGSTDOUT,"error:print_point_dec: point is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"      /*x = */  ");
    print_bigint_dec(log, &point->x);

    sys_print(log,"      /*y = */  ");
    print_bigint_dec(log, &point->y);

    return ;
}

void print_aff_point_dec(LOG *log,const EC_CURVE_AFF_POINT * point)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_aff_point_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == point )
    {
        sys_print(LOGSTDOUT,"error:print_aff_point_dec: point is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"      /*x = */  ");
    print_bigint_dec(log, &point->x);

    sys_print(log,"      /*y = */  ");
    print_bigint_dec(log, &point->y);

    sys_print(log,"      /*z = */  ");
    print_bigint_dec(log, &point->z);

    return ;
}

void print_keypair_dec(LOG *log,const ECC_KEYPAIR* keypair)
{
    BIGINT *prikey;
    EC_CURVE_POINT *pubkey;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_keypair_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == keypair )
    {
        sys_print(LOGSTDOUT,"error:print_keypair_dec: keypair is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    prikey = ( BIGINT * )&(keypair->private_key);
    pubkey = ( EC_CURVE_POINT * )&(keypair->public_key);

    sys_print(log,"      /*private key: */  \n");
    print_bigint_dec(log, prikey);

    sys_print(log,"      /*public key: */  \n");
    print_point_dec(log, pubkey);

    return ;
}

void print_ecf2n_curve_dec(LOG *log,const ECF2N_CURVE* ecf2n_curve)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ecf2n_curve_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == ecf2n_curve )
    {
        sys_print(LOGSTDOUT,"error:print_ecf2n_curve_dec: ecf2n_curve is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"      /* a */  ");
    print_bigint_dec(log, &(ecf2n_curve->a));

    sys_print(log,"      /* b */  ");
    print_bigint_dec(log, &(ecf2n_curve->b));

    return ;
}

void print_ecfp_curve_dec(LOG *log,const ECFP_CURVE* ecfp_curve)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ecfp_curve_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == ecfp_curve )
    {
        sys_print(LOGSTDOUT,"error:print_ecfp_curve_dec: ecfp_curve is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"      /* a */  ");
    print_bigint_dec(log, &(ecfp_curve->a));

    sys_print(log,"      /* b */  ");
    print_bigint_dec(log, &(ecfp_curve->b));

    return ;
}

void print_ecc_signature_dec(LOG *log,const ECC_SIGNATURE* ecc_signature)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ecc_signature_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == ecc_signature )
    {
        sys_print(LOGSTDOUT,"error:print_ecc_signature_dec: ecc_signature is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */
    sys_print(log,"      /* r */  ");
    print_bigint_dec(log, &(ecc_signature->r));

    sys_print(log,"      /* s */  ");
    print_bigint_dec(log, &(ecc_signature->s));

    return;
}

static UINT32 print_poly_item_dec_with_depth(LOG *log,const POLY_ITEM* item,POLY_ITEM_BUF *poly_item_buf)
{
    UINT32 index;

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
    {
        if ( PRINT_MAX_DEPTH_OF_POLY_ITEM <= poly_item_buf->depth )
        {
            sys_print(LOGSTDOUT,"error:print_poly_item_dec_with_depth: poly_item_buf overflow.\n");
            return ( 0 );
        }

        poly_item_buf->node[ poly_item_buf->depth ].coe = POLY_ITEM_BGN_COE(item);
        poly_item_buf->node[ poly_item_buf->depth ].deg = POLY_ITEM_DEG(item);
        poly_item_buf->depth ++;

        for ( index = 0; index < poly_item_buf->depth; index ++ )
        {
            sys_print(log," deg = ");
            print_deg_decchars(log, poly_item_buf->node[ index ].deg);

            if ( NULL_PTR == poly_item_buf->node[ index ].coe )
            {
                //sys_print(log," coe = 1");
            }
            else
            {
                sys_print(log," coe = ");
                print_bigint_decchars(log, poly_item_buf->node[ index ].coe);
            }
        }
        sys_print(log,"\n");

        poly_item_buf->depth --;
        poly_item_buf->node[ poly_item_buf->depth ].coe = NULL_PTR;
        poly_item_buf->node[ poly_item_buf->depth ].deg = NULL_PTR;
    }
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item) )
    {
        if ( PRINT_MAX_DEPTH_OF_POLY_ITEM <= poly_item_buf->depth )
        {
            sys_print(LOGSTDOUT,"error:print_poly_item_dec_with_depth: poly_item_buf overflow.\n");
            return ( 0 );
        }

        poly_item_buf->node[ poly_item_buf->depth ].coe = NULL_PTR;/*it represents actually one*/
        poly_item_buf->node[ poly_item_buf->depth ].deg = POLY_ITEM_DEG(item);
        poly_item_buf->depth ++;

        print_poly_dec_with_depth(log, POLY_ITEM_POLY_COE(item), poly_item_buf);
        poly_item_buf->depth --;
        poly_item_buf->node[ poly_item_buf->depth ].coe = NULL_PTR;
        poly_item_buf->node[ poly_item_buf->depth ].deg = NULL_PTR;
    }
    else
    {
        sys_print(LOGSTDOUT,"error:print_poly_item_dec_with_depth: bgn_coe_flag = %lx error.\n");
        return ((UINT32)(-1));
    }

    return ( 0 );
}
static void print_poly_dec_with_depth(LOG *log,const POLY* poly,POLY_ITEM_BUF *poly_item_buf)
{
    POLY_ITEM *item;
    UINT32 ret;

    item = POLY_FIRST_ITEM(poly);

    while( item != POLY_NULL_ITEM(poly) )
    {
        ret = print_poly_item_dec_with_depth(log, item, poly_item_buf);
        if ( 0 != ret )
        {
            sys_print(LOGSTDOUT,"...\n");
            break;
        }

        item = POLY_ITEM_NEXT(item);
    }
    return ;
}

void print_poly_dec(LOG *log,const POLY* poly)
{
    POLY_ITEM_BUF buf_1;
    POLY_ITEM_BUF *poly_item_buf;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_poly_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == poly )
    {
        sys_print(LOGSTDOUT,"error:print_poly_dec: poly is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    //sys_print(LOGSTDOUT,"error:print_poly_dec: incompleted interface.\n");

    poly_item_buf = &buf_1;

    poly_item_buf->depth = 0;

    print_poly_dec_with_depth(log, poly, poly_item_buf);

    return;
}

void print_poly_item_dec(LOG *log,const POLY_ITEM* item)
{
#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_poly_item_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == item )
    {
        sys_print(LOGSTDOUT,"error:print_poly_item_dec: item is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    sys_print(LOGSTDOUT,"item deg = ");
    print_deg_decchars(log, POLY_ITEM_DEG(item));
    sys_print(LOGSTDOUT,"\n");

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
    {
        sys_print(LOGSTDOUT,"item coe = ");
        print_bigint_decchars(log, POLY_ITEM_BGN_COE(item));
        sys_print(LOGSTDOUT,"\n");
    }
    else
    {
        sys_print(LOGSTDOUT,"item coe = \n");
        print_poly_dec(log, POLY_ITEM_POLY_COE(item));
    }

    return;
}

void print_ebgn_dec(LOG *log,const EBGN * src)
{
    UINT8  *decstr;
    UINT32 decstr_maxlen;
    UINT32 decstr_retlen;
    UINT32 conv_md_id;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ebgn_dec: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_ebgn_dec: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_print(LOGSTDERR,"print_ebgn_dec: no CONV module available\n");
        return ;
    }

    alloc_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, &decstr, LOC_PRINT_0001);
    decstr_maxlen = 4 * 1024;

    conv_ebgn_to_dec(conv_md_id, src, decstr, decstr_maxlen, &decstr_retlen);

    print_str(log, decstr, decstr_retlen);

    free_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, decstr, LOC_PRINT_0002);

    conv_end(conv_md_id);

    return ;
}

void print_ebgn_hex(LOG *log,const EBGN * src)
{
    UINT8  *hexstr;
    UINT32 hexstr_maxlen;
    UINT32 hexstr_retlen;
    UINT32 conv_md_id;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ebgn_hex: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_ebgn_hex: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_print(LOGSTDERR,"print_ebgn_hex: no CONV module available\n");
        return ;
    }
    alloc_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, &hexstr, LOC_PRINT_0003);
    hexstr_maxlen = 4 * 1024;

    conv_ebgn_to_hex(conv_md_id, src, hexstr, hexstr_maxlen, &hexstr_retlen);

    print_str(log, hexstr, hexstr_retlen);;

    free_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, hexstr, LOC_PRINT_0004);

    conv_end(conv_md_id);


    return ;
}

void print_ebgn_bin(LOG *log,const EBGN * src)
{
    UINT8  *binstr;
    UINT32 binstr_maxlen;
    UINT32 binstr_retlen;
    UINT32 conv_md_id;

#if ( SWITCH_ON == PRINT_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_print(LOGSTDOUT,"error:print_ebgn_bin: log is NULL_PTR.\n");
        return ;
    }
    if ( NULL_PTR == src )
    {
        sys_print(LOGSTDOUT,"error:print_ebgn_bin: src is NULL_PTR.\n");
        return ;
    }
#endif /* BIGINT_DEBUG_SWITCH */

    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_print(LOGSTDERR,"print_ebgn_bin: no CONV module available\n");
        return ;
    }
    alloc_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, &binstr, LOC_PRINT_0005);
    binstr_maxlen = 4 * 1024;

    conv_ebgn_to_bin(conv_md_id, src, binstr, binstr_maxlen, &binstr_retlen);

    print_str(log, binstr, binstr_retlen);;

    free_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, binstr, LOC_PRINT_0006);

    conv_end(conv_md_id);

    return ;
}

void print_ebgn(LOG *log, const EBGN *ebgn)
{
    EBGN_ITEM *item;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn: log is NULL_PTR.\n");
        return;
    }
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn: ebgn is NULL_PTR.\n");
        return ;
    }
#endif/*EBGN_DEBUG_SWITCH*/

    sys_print(log, "[ high -> low ] length = %ld\n", EBGN_LEN(ebgn));
    if( EC_FALSE == EBGN_SGN(ebgn) )
    {
        sys_print(log, "- ");
    }
    else
    {
        sys_print(log, "+ ");
    }

    item = EBGN_LAST_ITEM(ebgn);

    while ( item != EBGN_NULL_ITEM(ebgn) )
    {
        print_bigint(log, EBGN_ITEM_BGN(item));
        sys_print(LOGSTDOUT," ");

        item = EBGN_ITEM_PREV(item);
    }

    sys_print(LOGSTDOUT,"\n");
}

void print_ebgn_info(LOG *log, const EBGN *ebgn, const char *info)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn_info: log is NULL_PTR.\n");
        return;
    }
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn_info: ebgn is NULL_PTR.\n");
        return ;
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( NULL_PTR != info )
    {
        sys_print(log,"%s\n", info);
    }

    print_ebgn(log, ebgn);
}
void print_ebgn_dec_info(LOG *log, const EBGN *ebgn, const char *info)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn_dec_info: log is NULL_PTR.\n");
        return;
    }
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn_dec_info: ebgn is NULL_PTR.\n");
        return ;
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( NULL_PTR != info )
    {
        sys_print(log,"%s ", info);
    }

    print_ebgn_dec(log, ebgn);
}

void print_ebgn_hex_info(LOG *log, const EBGN *ebgn, const char *info)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn_hex_info: log is NULL_PTR.\n");
        return;
    }
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn_hex_info: ebgn is NULL_PTR.\n");
        return ;
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( NULL_PTR != info )
    {
        sys_print(log,"%s ", info);
    }

    print_ebgn_hex(log, ebgn);
}

void print_ebgn_bin_info(LOG *log, const EBGN *ebgn, const char *info)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn_bin_info: log is NULL_PTR.\n");
        return;
    }
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:print_ebgn_bin_info: ebgn is NULL_PTR.\n");
        return ;
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( NULL_PTR != info )
    {
        sys_print(log,"%s ", info);
    }

    print_ebgn_bin(log, ebgn);
}
#ifdef __cplusplus
}
#endif/*__cplusplus*/

