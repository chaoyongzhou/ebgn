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

#include "bgnctrl.h"
#include "type.h"
#include "moduleconst.h"

#include "mm.h"

#include "log.h"

#include "bgnz.h"
#include "ebgnz.h"

#include "poly.h"

#include "debug.h"

#include "conv.h"
#include "vector.h"
#include "matrix.h"

#include "vectorr.h"
#include "matrixr.h"

#include "cvector.h"

#include "mod.h"
#include "task.h"

#include "print.h"

#define CONV_CHAR_IS_DIGIT(ch) (((ch) >='0' && (ch) <='9')? EC_TRUE : EC_FALSE)
#define CONV_CHAR_IS_HEX(ch) ((((ch) >='0' && (ch) <='9') || ((ch) >='a' && (ch) <='f')|| ((ch) >='A' && (ch) <='F'))? EC_TRUE : EC_FALSE)
#define CONV_CHAR_IS_BIN(ch) (((ch) >='0' && (ch) <='1')? EC_TRUE : EC_FALSE)

typedef UINT32 (*FUNC_CONV_CHAR_TO_NUM)(const UINT32 conv_md_id,const UINT8 ch, UINT32 *num);
typedef UINT32 (*FUNC_CONV_NUM_TO_CHAR)(const UINT32 conv_md_id,const UINT32 num, UINT8 *ch);


static EC_BOOL conv_check_decstr(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen);
static EC_BOOL conv_check_hexstr(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen);
static EC_BOOL conv_check_binstr(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen);

static UINT32 conv_decchar_to_num(const UINT32 conv_md_id,const UINT8 ch, UINT32 *num);
static UINT32 conv_num_to_decchar(const UINT32 conv_md_id,const UINT32 num, UINT8 *ch);
static UINT32 conv_hexchar_to_num(const UINT32 conv_md_id,const UINT8 ch, UINT32 *num);
static UINT32 conv_num_to_hexchar(const UINT32 conv_md_id,const UINT32 num, UINT8 *ch);
static UINT32 conv_binchar_to_num(const UINT32 conv_md_id,const UINT8 ch, UINT32 *num);
static UINT32 conv_num_to_binchar(const UINT32 conv_md_id,const UINT32 num, UINT8 *ch);

static UINT32 conv_str_to_bgn(const UINT32 conv_md_id,const UINT8 *srcstr,const UINT32 srcstrlen,
                                    const UINT32 radix,
                                    FUNC_CONV_CHAR_TO_NUM conv_char_to_num,
                                    BIGINT *des);

static UINT32 conv_bgn_to_str(const UINT32 conv_md_id,const BIGINT *src,
                                    const UINT32 radix,
                                    FUNC_CONV_NUM_TO_CHAR conv_num_to_char,
                                    UINT8 *desstr,const UINT32 desstrmaxlen, UINT32 *desstrlen);

static UINT32 conv_str_to_ebgn(const UINT32 conv_md_id,const UINT8 *srcstr,const UINT32 srcstrlen,
                                    const UINT32 radix,
                                    FUNC_CONV_CHAR_TO_NUM conv_char_to_num,
                                    EBGN *des);

static UINT32 conv_ebgn_to_str(const UINT32 conv_md_id,const EBGN *src,
                                    const UINT32 radix,
                                    FUNC_CONV_NUM_TO_CHAR conv_num_to_char,
                                    UINT8 *desstr,const UINT32 desstrmaxlen, UINT32 *desstrlen);

static UINT32 conv_dec_to_poly_0(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,POLY *des, UINT32 *decstrusedlen);
static UINT32 conv_hex_to_poly_0(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,POLY *des, UINT32 *hexstrusedlen);
static UINT32 conv_bin_to_poly_0(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,POLY *des, UINT32 *binstrusedlen);

static UINT32 conv_dec_to_poly_item_0(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,POLY_ITEM *des, UINT32 *decstrusedlen);
static UINT32 conv_hex_to_poly_item_0(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,POLY_ITEM *des, UINT32 *hexstrusedlen);
static UINT32 conv_bin_to_poly_item_0(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,POLY_ITEM *des, UINT32 *binstrusedlen);

static CONV_MD g_conv_md[ MAX_NUM_OF_CONV_MD ];
static EC_BOOL  g_conv_md_init_flag = EC_FALSE;

/**
*   for test only
*
*   to query the status of CONV Module
*
**/
void conv_print_module_status(const UINT32 conv_md_id, LOG *log)
{
    CONV_MD *conv_md;
    UINT32 index;

    if ( EC_FALSE == g_conv_md_init_flag )
    {
        sys_log(log,"no CONV Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_CONV_MD; index ++ )
    {
        conv_md = &(g_conv_md[ index ]);

        if ( 0 < conv_md->usedcounter )
        {
            sys_log(log,"CONV Module # %ld : %ld refered, refer BGNZ Module : %ld, refer EBGNZ Module :%ld, refer VECTORR Module : %ld, refer MATRIXR Module : %ld\n",
                    index,
                    conv_md->usedcounter,
                    conv_md->bgnz_md_id,
                    conv_md->ebgnz_md_id,
                    conv_md->vectorr_md_id,
                    conv_md->matrixr_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed CONV module
*
*
**/
UINT32 conv_free_module_static_mem(const UINT32 conv_md_id)
{
    CONV_MD    *conv_md;
    UINT32 bgnz_md_id;
    UINT32 ebgnz_md_id;
    UINT32 vectorr_md_id;
    UINT32 matrixr_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_free_module_static_mem: conv module #0x%lx not started.\n",
                conv_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    conv_md = &(g_conv_md[ conv_md_id ]);
    bgnz_md_id  = conv_md->bgnz_md_id;
    ebgnz_md_id  = conv_md->ebgnz_md_id;
    vectorr_md_id = conv_md->vectorr_md_id;
    matrixr_md_id = conv_md->matrixr_md_id;

    free_module_static_mem(MD_CONV, conv_md_id);

    bgn_z_free_module_static_mem(bgnz_md_id);
    ebgn_z_free_module_static_mem(ebgnz_md_id);
    vector_r_free_module_static_mem(vectorr_md_id);
    matrix_r_free_module_static_mem(matrixr_md_id);

    return 0;
}

/**
*
* start CONV module
*
**/
UINT32 conv_start()
{
    CONV_MD *conv_md;
    UINT32 conv_md_id;
    UINT32 bgnz_md_id;
    UINT32 ebgnz_md_id;
    UINT32 vectorr_md_id;
    UINT32 matrixr_md_id;
    UINT32 index;

    /* if this is the 1st time to start CONV module, then */
    /* initialize g_conv_md */
    if ( EC_FALSE ==  g_conv_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_CONV_MD; index ++ )
        {
            conv_md = &(g_conv_md[ index ]);

            conv_md->usedcounter   = 0;
            conv_md->bgnz_md_id    = ERR_MODULE_ID;
            conv_md->ebgnz_md_id   = ERR_MODULE_ID;
        }

        /*register all functions of CONV module to DBG module*/
        //dbg_register_func_addr_list(g_conv_func_addr_list, g_conv_func_addr_list_len);

        g_conv_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_CONV_MD; index ++ )
    {
        conv_md = &(g_conv_md[ index ]);

        if ( 0 == conv_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_CONV_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    conv_md_id = ERR_MODULE_ID;

    conv_md_id = index;
    conv_md = &(g_conv_md[ conv_md_id ]);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    /*start one BGNZ module and EBGNZ module*/
    bgnz_md_id  = bgn_z_start();
    ebgnz_md_id = ebgn_z_start();
    vectorr_md_id = vector_r_start();
    matrixr_md_id = matrix_r_start();

    /* set module : */
    conv_md->bgnz_md_id  = bgnz_md_id;
    conv_md->ebgnz_md_id = ebgnz_md_id;
    conv_md->vectorr_md_id = vectorr_md_id;
    conv_md->matrixr_md_id = matrixr_md_id;

    /* at the first time, set the counter to 1 */
    conv_md->usedcounter = 1;

    return ( conv_md_id );
}

/**
*
* end CONV module
*
**/
void conv_end(const UINT32 conv_md_id)
{
    CONV_MD *conv_md;
    UINT32 bgnz_md_id;
    UINT32 ebgnz_md_id;
    UINT32 vectorr_md_id;
    UINT32 matrixr_md_id;

    if ( MAX_NUM_OF_CONV_MD < conv_md_id )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_end: conv_md_id = %ld is overflow.\n",conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }

    conv_md = &(g_conv_md[ conv_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < conv_md->usedcounter )
    {
        conv_md->usedcounter --;
        return ;
    }

    if ( 0 == conv_md->usedcounter )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_end: conv_md_id = %ld is not started.\n",conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnz_md_id  = conv_md->bgnz_md_id;
    ebgnz_md_id = conv_md->ebgnz_md_id;
    vectorr_md_id = conv_md->vectorr_md_id;
    matrixr_md_id = conv_md->matrixr_md_id;

    /*close the submodule*/
    bgn_z_end(bgnz_md_id);
    ebgn_z_end(ebgnz_md_id);
    vector_r_end( vectorr_md_id);
    matrix_r_end( matrixr_md_id);

    /* free module : */
    conv_md->usedcounter = 0;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    breathing_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   check all characters in the decstr range [0-9]
*   if the check is passed, then return EC_TRUE
*   otherwise, return EC_FALSE and print the error info
*
**/
static EC_BOOL conv_check_decstr(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen)
{
    UINT32 index;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_check_decstr: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_check_decstr: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    for ( index = 0; index < decstrlen; index ++ )
    {
        if ( EC_FALSE == CONV_CHAR_IS_DIGIT( decstr[ index ] ) )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_check_decstr:\n");
            dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"char No.%ld '%c' is not a valid digit\n",index,decstr[ index ]);
            return EC_FALSE;
        }
    }

    return EC_TRUE;
}

/**
*
*   check all characters in the hexstr range [0-9a-fA-F]
*   if the check is passed, then return EC_TRUE
*   otherwise, return EC_FALSE and print the error info
*
**/
static EC_BOOL conv_check_hexstr(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen)
{
    UINT32 index;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_check_hexstr: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_check_hexstr: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    for ( index = 0; index < hexstrlen; index ++ )
    {
        if ( EC_FALSE == CONV_CHAR_IS_HEX( hexstr[ index ] ) )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_check_hexstr:\n");
            dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"char No.%ld '%c' is not a valid hex symbol\n",index,hexstr[ index ]);
            return EC_FALSE;
        }
    }

    return EC_TRUE;
}

/**
*
*   check all characters in the binstr range [0-1]
*   if the check is passed, then return EC_TRUE
*   otherwise, return EC_FALSE and print the error info
*
**/
static EC_BOOL conv_check_binstr(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen)
{
    UINT32 index;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_check_binstr: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_check_binstr: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    for ( index = 0; index < binstrlen; index ++ )
    {
        if ( EC_FALSE == CONV_CHAR_IS_BIN( binstr[ index ] ) )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_check_binstr:\n");
            dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"char No.%ld '%c' is not a valid binary symbol\n",index,binstr[ index ]);
            return EC_FALSE;
        }
    }

    return EC_TRUE;
}

/**
*
*   conv a decimal char which belongs to [0-9] to the relative digit,i.e.
*   '0' map to 0
*   '1' map to 1
*   ...
*
**/
static UINT32 conv_decchar_to_num(const UINT32 conv_md_id,const UINT8 ch, UINT32 *num)
{
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == num )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_decchar_to_num: num is null.\n");
        return ((UINT32)(-1));
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_decchar_to_num: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if (EC_FALSE == CONV_CHAR_IS_DIGIT(ch))
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_decchar_to_num: %c is invalid digit symbol.\n",ch);
        return ((UINT32)(-1));
    }

    *num = ch - '0';

    return 0;
}

/**
*
*   conv a decimal number[0-9] to the relative char,i.e.
*   0 map to '0'
*   1 map to '1'
*   ...
*
**/
static UINT32 conv_num_to_decchar(const UINT32 conv_md_id,const UINT32 num, UINT8 *ch)
{
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == ch )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_num_to_decchar: ch is null.\n");
        return ((UINT32)(-1));
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_num_to_decchar: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
    if ( 9 < num )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_num_to_decchar: num = %ld is invalid single digit.\n",num);
        return ((UINT32)(-1));
    }

    *ch = ((UINT8)num) + '0';

    return 0;
}

/**
*
*   conv a hex char which belongs to [0-9a-fA-F] to the relative digit,i.e.
*   '0' map to 0
*   '1' map to 1
*   ...
*   'a' or 'A' map to 10
*   'b' or 'B' map to 11
*
**/
static UINT32 conv_hexchar_to_num(const UINT32 conv_md_id,const UINT8 ch, UINT32 *num)
{
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == num )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hexchar_to_num: num is null.\n");
        return ((UINT32)(-1));
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hexchar_to_num: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if (EC_FALSE == CONV_CHAR_IS_HEX(ch))
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hexchar_to_num: %c is invalid hex symbol.\n",ch);
        return ((UINT32)(-1));
    }

    if ( ch >= '0' && ch <= '9' )
    {
        *num = ch - '0' + 0 ;
    }
    else if ( ch >= 'a' && ch <= 'f' )
    {
        *num = ch - 'a' + 10;
    }
    else if ( ch >= 'A' && ch <= 'F' )
    {
        *num = ch - 'A' + 10;
    }
    else
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hexchar_to_num: %c is invalid hex symbol.\n",ch);
        return ((UINT32)(-1));
    }

    return 0;
}

/**
*
*   conv a hex number[0-15] to the relative char,i.e.
*   0 map to '0'
*   1 map to '1'
*   ...
*   10 map to 'a'
*   ...
*   15 map to 'f'
*
**/
static UINT32 conv_num_to_hexchar(const UINT32 conv_md_id,const UINT32 num, UINT8 *ch)
{
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == ch )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_num_to_hexchar: ch is null.\n");
        return ((UINT32)(-1));
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_num_to_hexchar: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( 15 < num)
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_num_to_hexchar: num = %ld is invalid single digit.\n",num);
        return ((UINT32)(-1));
    }

    if ( 9 < num )
    {
        *ch = ((UINT8)num) + 'a' - 10;
    }
    else
    {
        *ch = ((UINT8)num) + '0' - 0;
    }
    return 0;
}

/**
*
*   conv a binary char which belongs to [0-1] to the relative digit,i.e.
*   '0' map to 0
*   '1' map to 1
*
**/
static UINT32 conv_binchar_to_num(const UINT32 conv_md_id,const UINT8 ch, UINT32 *num)
{
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == num )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_binchar_to_num: num is null.\n");
        return ((UINT32)(-1));
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_binchar_to_num: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if (EC_FALSE == CONV_CHAR_IS_BIN(ch))
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_binchar_to_num: %c is invalid binary symbol.\n",ch);
        return ((UINT32)(-1));
    }

    if ( ch == '0' )
    {
        *num = 0 ;
    }
    else if ( ch == '1' )
    {
        *num = 1;
    }
    else
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_binchar_to_num: %c is invalid binary symbol.\n",ch);
        return ((UINT32)(-1));
    }

    return 0;
}

/**
*
*   conv a binary number[0-1] to the relative char,i.e.
*   0 map to '0'
*   1 map to '1'
*
**/
static UINT32 conv_num_to_binchar(const UINT32 conv_md_id,const UINT32 num, UINT8 *ch)
{
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == ch )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_num_to_binchar: ch is null.\n");
        return ((UINT32)(-1));
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_num_to_binchar: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( 1 < num)
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_num_to_binchar: num = %ld is invalid single digit.\n",num);
        return ((UINT32)(-1));
    }

    if ( 0 == num )
    {
        *ch = '0';
    }
    else
    {
        *ch = '1';
    }
    return 0;
}


/**
*
*   convert a valid string to the relative BIGINT number
*   where the string is composed by valid chars and by the appointed radix.
*   User provides the mapping from valid char to a valid number
*
*   if the string contains invalid char, the convertion process will stop
*   and print error.
*
*   if the number corresponding to the string is overflow of the BIGINT limit,
*   then the convertion process will stop and print error.
*
**/
static UINT32 conv_str_to_bgn(const UINT32 conv_md_id,const UINT8 *srcstr,const UINT32 srcstrlen,
                                    const UINT32 radix,
                                    FUNC_CONV_CHAR_TO_NUM conv_char_to_num,
                                    BIGINT *des)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *a;
    BIGINT *c0;
    BIGINT *c1;
    BIGINT *RADIX;

    BIGINT *t;
    UINT32 digit;
    UINT32 carry;

    CONV_MD    *conv_md;
    UINT32  bgnz_md_id;


    UINT32 index;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == srcstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_str_to_bgn: srcstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_str_to_bgn: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_str_to_bgn: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    a   = &buf_1;
    c0  = &buf_2;
    c1  = &buf_3;
    RADIX = &buf_4;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &a, LOC_CONV_0001);
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &c0, LOC_CONV_0002);
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &c1, LOC_CONV_0003);
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &RADIX, LOC_CONV_0004);
#endif/* STATIC_MEMORY_SWITCH */

    conv_md = &(g_conv_md[ conv_md_id ]);
    bgnz_md_id  = conv_md->bgnz_md_id;

    /*set RADIX = radix */
    bgn_z_set_word(bgnz_md_id, RADIX, radix);

    /*set a = 0 */
    bgn_z_set_zero(bgnz_md_id, a);

    for ( index = 0; index < srcstrlen; index ++ )
    {
        /*let (c1,c0) = RADIX * a*/
        bgn_z_mul(bgnz_md_id, a, RADIX, c0, c1);

        /*if c1 > 0 which means the number is overflow,then report error and return*/
        if ( EC_FALSE == bgn_z_is_zero(bgnz_md_id, c1))
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_str_to_bgn: the number is overflow.\n");

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, a, LOC_CONV_0005);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, c0, LOC_CONV_0006);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, c1, LOC_CONV_0007);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, RADIX, LOC_CONV_0008);
#endif/* STATIC_MEMORY_SWITCH */

            return ((UINT32)(-1));
        }

        /*if c1 = 0, then let a = c0*/
        t = a;
        a = c0;
        c0 = t;

        /*get the current digit value*/
        if ( 0 != conv_char_to_num(conv_md_id,srcstr[ index ], &digit) )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_str_to_bgn: failed to convert char(%c) to num.\n", srcstr[ index ]);
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, a, LOC_CONV_0009);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, c0, LOC_CONV_0010);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, c1, LOC_CONV_0011);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, RADIX, LOC_CONV_0012);
#endif/* STATIC_MEMORY_SWITCH */

            return ((UINT32)(-1));
        }

        /*add the current digit value to a*/
        carry = bgn_z_sadd(bgnz_md_id, a, digit, a);
        if ( 0 < carry )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_str_to_bgn: the decimal number is overflow.\n");
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, a, LOC_CONV_0013);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, c0, LOC_CONV_0014);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, c1, LOC_CONV_0015);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, RADIX, LOC_CONV_0016);
#endif/* STATIC_MEMORY_SWITCH */

            return ((UINT32)(-1));
        }
    }

    bgn_z_clone(bgnz_md_id, a, des);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, a, LOC_CONV_0017);
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, c0, LOC_CONV_0018);
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, c1, LOC_CONV_0019);
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, RADIX, LOC_CONV_0020);
#endif/* STATIC_MEMORY_SWITCH */

    return ( 0 );
}

/**
*
*   convert a BIGINT number to the relative string which is composed by
*   valid chars with the appointed radix.
*   User provides the mapping function from a valid number to a valid char
*
*   if the digit is overflow the radix range, then the convertion process
*   will stop and print error.
*
*   if the return string buffer is not enough, then the convertion process
*   will stop and print error.
*
**/
static UINT32 conv_bgn_to_str(const UINT32 conv_md_id,const BIGINT *src,
                                    const UINT32 radix,
                                    FUNC_CONV_NUM_TO_CHAR conv_num_to_char,
                                    UINT8 *desstr,const UINT32 desstrmaxlen, UINT32 *desstrlen)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    int buf_5[MAX_NAF_ARRAY_LEN];
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *a;
    BIGINT *q;
    BIGINT *r;
    BIGINT *RADIX;

    UINT8 *tmp_desstr;

    BIGINT *t;
    UINT32 digit;

    CONV_MD    *conv_md;
    UINT32  bgnz_md_id;


    UINT32 index;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_str: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == desstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_str: desstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == desstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_str: desstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bgn_to_str: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    conv_md = &(g_conv_md[ conv_md_id ]);
    bgnz_md_id  = conv_md->bgnz_md_id;

    /*check the bgn's validation*/
    if ( INTMAX < src->len )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bgn_to_str: invalid BIGINT with len %ld.\n",
                src->len);
        //desstr[ 0 ] = 'X';
        *desstrlen = 0;
        return ((UINT32)(-1));
    }
    /*if src = 0, then*/
    if ( EC_TRUE == bgn_z_is_zero(bgnz_md_id, src))
    {
        desstr[ 0 ] = '0';
        *desstrlen = 1;
        return ( 0 );
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    a   = &buf_1;
    q   = &buf_2;
    r   = &buf_3;
    RADIX = &buf_4;
    tmp_desstr = buf_5;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &a, LOC_CONV_0021);
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &q, LOC_CONV_0022);
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &r, LOC_CONV_0023);
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &RADIX, LOC_CONV_0024);
    alloc_static_mem(MD_CONV, conv_md_id, MM_NAF, &tmp_desstr, LOC_CONV_0025);
#endif/* STATIC_MEMORY_SWITCH */

    /*set RADIX = radix */
    bgn_z_set_word(bgnz_md_id, RADIX, radix);

    /*let a = src*/
    bgn_z_clone(bgnz_md_id, src, a);

    /*at first,point to the first char of desstr,and the current length of desstr is zero*/
    index = 0;

    /*while a is non-zero, then do a = [a / RADIX]*/
    while( EC_FALSE == bgn_z_is_zero(bgnz_md_id, a))
    {
        /*let a = RADIX * q + r*/
        bgn_z_div(bgnz_md_id, a, RADIX, q, r);

        /*let digit = r*/
        if ( EC_TRUE == bgn_z_is_zero(bgnz_md_id, r) )
        {
            digit = 0;
        }
        else
        {
            digit = r->data[ 0 ];
        }

        /*push digit to the desstr*/
        /*if the decimal desstring is full, then warning and return error code*/
        if ( index >= desstrmaxlen )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_str: desstr is overflow.\n");
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, a, LOC_CONV_0026);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, q, LOC_CONV_0027);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, r, LOC_CONV_0028);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, RADIX, LOC_CONV_0029);
            free_static_mem(MD_CONV, conv_md_id, MM_NAF, tmp_desstr, LOC_CONV_0030);
#endif/* STATIC_MEMORY_SWITCH */
            return ((UINT32)(-1));
        }

        if ( 0 != conv_num_to_char(conv_md_id,digit, &(tmp_desstr[ index ]) ))
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_str: failed to convert num(%ld) to char.\n",digit);
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, a, LOC_CONV_0031);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, q, LOC_CONV_0032);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, r, LOC_CONV_0033);
            free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, RADIX, LOC_CONV_0034);
            free_static_mem(MD_CONV, conv_md_id, MM_NAF, tmp_desstr, LOC_CONV_0035);
#endif/* STATIC_MEMORY_SWITCH */
            return ((UINT32)(-1));
        }

        /*let a = q*/
        t = a;
        a = q;
        q = t;

        /*move to next char */
        index ++;
    }

    /*set the desstr return length*/
    *desstrlen = index;

    /*reverse the decimal desstring in tmp_desstr and take it back by desstr*/
    for (; index > 0; )
    {
        index --;
        desstr[ ((*desstrlen)- 1) - index ] = (UINT8)tmp_desstr[ index ];
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, a, LOC_CONV_0036);
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, q, LOC_CONV_0037);
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, r, LOC_CONV_0038);
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, RADIX, LOC_CONV_0039);
    free_static_mem(MD_CONV, conv_md_id, MM_NAF, tmp_desstr, LOC_CONV_0040);
#endif/* STATIC_MEMORY_SWITCH */

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*
*   convert a valid string to the relative EBGN number
*   where the string is composed by valid chars and by the appointed radix.
*   User provides the mapping from valid char to a valid number
*
*   if the string contains invalid char, the convertion process will stop
*   and print error.
*
**/
static UINT32 conv_str_to_ebgn(const UINT32 conv_md_id,const UINT8 *srcstr,const UINT32 srcstrlen,
                                    const UINT32 radix,
                                    FUNC_CONV_CHAR_TO_NUM conv_char_to_num,
                                    EBGN *des)
{
    CONV_MD    *conv_md;
    UINT32  ebgnz_md_id;

    EBGN *a;
    EBGN *c0;
    EBGN *c1;
    EBGN *RADIX;

    UINT32 digit;

    UINT32 sgn;
    UINT32 index;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == srcstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_str_to_ebgn: srcstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_str_to_ebgn: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_str_to_ebgn: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    conv_md = &(g_conv_md[ conv_md_id ]);
    ebgnz_md_id  = conv_md->ebgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &a);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &c0);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &c1);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &RADIX);

    /*set RADIX = radix */
    ebgn_z_set_word(ebgnz_md_id, radix, RADIX);

    /*determine the sgn of target des*/
    if( 0 < srcstrlen )
    {
    if( '-' == srcstr[ 0 ] )
    {
        sgn = EC_FALSE;
        index = 1;
    }
    else if( '+' == srcstr[ 0 ] )
    {
        sgn = EC_TRUE;
        index = 1;
    }
    else
    {
        sgn = EC_TRUE;
        index = 0;
    }
    }
    else
    {
    sgn = EC_TRUE;
    }

    for ( ; index < srcstrlen; index ++ )
    {
        /*let c0 = RADIX * a*/
        ebgn_z_mul(ebgnz_md_id, a, RADIX, c0);

        /*get the current digit value*/
        if ( 0 != conv_char_to_num(conv_md_id, srcstr[ index ], &digit) )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_str_to_ebgn: failed to convert char(%c) to num.\n", srcstr[ index ]);

            ebgn_z_destroy(ebgnz_md_id, a);
            ebgn_z_destroy(ebgnz_md_id, c0);
            ebgn_z_destroy(ebgnz_md_id, c1);
            ebgn_z_destroy(ebgnz_md_id, RADIX);

         return ((UINT32)-1);
        }

        /*set c1 = digit */
        ebgn_z_set_word(ebgnz_md_id, digit, c1);

        /*add the current digit value to a*/
        ebgn_z_add(ebgnz_md_id, c0, c1, a);
    }

    EBGN_SGN(a) = sgn;
    ebgn_z_move(ebgnz_md_id, a, des);

    ebgn_z_destroy(ebgnz_md_id, a);
    ebgn_z_destroy(ebgnz_md_id, c0);
    ebgn_z_destroy(ebgnz_md_id, c1);
    ebgn_z_destroy(ebgnz_md_id, RADIX);

    return ( 0 );
}

/**
*
*   convert a EBGN number to the relative string which is composed by
*   valid chars with the appointed radix.
*   User provides the mapping function from a valid number to a valid char
*
*   if the digit is overflow the radix range, then the convertion process
*   will stop and print error.
*
*   if the return string buffer is not enough, then the convertion process
*   will stop and print error.
*
**/
static UINT32 conv_ebgn_to_str(const UINT32 conv_md_id,const EBGN *src,
                                    const UINT32 radix,
                                    FUNC_CONV_NUM_TO_CHAR conv_num_to_char,
                                    UINT8 *desstr,const UINT32 desstrmaxlen, UINT32 *desstrlen)
{
    CONV_MD    *conv_md;
    UINT32  ebgnz_md_id;

    EBGN *a;
    EBGN *q;
    EBGN *r;
    EBGN *RADIX;

    EBGN *t;
    UINT8  ch[ WORDSIZE/BYTESIZE ]; /* redundant chars for word aligntment */
    UINT32 digit;

    UINT32 index;
    UINT32 left_idx;
    UINT32 right_idx;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_str: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == desstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_str: desstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == desstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_str: desstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ebgn_to_str: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    conv_md = &(g_conv_md[ conv_md_id ]);
    ebgnz_md_id  = conv_md->ebgnz_md_id;

    /*if src = 0, then*/
    if ( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, src))
    {
        desstr[ 0 ] = '0';
        *desstrlen = 1;
        return ( 0 );
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &a);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &q);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &r);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &RADIX);

    /*set RADIX = radix */
    ebgn_z_set_word(ebgnz_md_id, radix, RADIX);

    /*let a = abs(src)*/
    //print_ebgn(LOGSTDOUT, src, "conv_ebgn_to_str:src: ");
    ebgn_z_clone(ebgnz_md_id, src, a);
    //print_ebgn(LOGSTDOUT, src, "conv_ebgn_to_str:src: ");
    ebgn_z_abs(ebgnz_md_id, a, a);
    //print_ebgn(LOGSTDOUT, a, "conv_ebgn_to_str:a: ");

    /*at first,point to the first char of desstr,and the current length of desstr is zero*/
    index = 0;

    /*while a is non-zero, then do a = [a / RADIX]*/
    while( EC_FALSE == ebgn_z_is_zero(ebgnz_md_id, a))
    {
        /*let a = RADIX * q + r*/
        ebgn_z_div(ebgnz_md_id, a, RADIX, q, r);

    //fprintf(LOGSTDOUT,"index = %d\n", index);
        //print_ebgn(LOGSTDOUT, a, "conv_ebgn_to_str:a: ");
        //print_ebgn(LOGSTDOUT, q, "conv_ebgn_to_str:q: ");
        //print_ebgn(LOGSTDOUT, r, "conv_ebgn_to_str:r: ");
    //fprintf(LOGSTDOUT,"\n");

        /*let digit = r*/
        if ( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, r) )
        {
            digit = 0;
        }
        else
        {
            digit = (EBGN_ITEM_BGN(EBGN_FIRST_ITEM(r)))->data[ 0 ];
        }

        /*push digit to the desstr*/
        /*if the decimal desstring is full, then warning and return error code*/
    /*keep one char space for sgn '+' or '-'*/
        if ( index >= desstrmaxlen - 1 )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_str: desstr is overflow.\n");

            ebgn_z_destroy(ebgnz_md_id, a);
            ebgn_z_destroy(ebgnz_md_id, q);
            ebgn_z_destroy(ebgnz_md_id, r);
            ebgn_z_destroy(ebgnz_md_id, RADIX);

            return ((UINT32)(-1));
        }

        if ( 0 != conv_num_to_char(conv_md_id,digit, &(desstr[ index ]) ))
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_str: failed to convert num(%ld) to char.\n",digit);

            ebgn_z_destroy(ebgnz_md_id, a);
            ebgn_z_destroy(ebgnz_md_id, q);
            ebgn_z_destroy(ebgnz_md_id, r);
            ebgn_z_destroy(ebgnz_md_id, RADIX);

            return ((UINT32)(-1));
        }

        /*let a = q*/
        t = a;
        a = q;
        q = t;

        /*move to next char */
        index ++;
    }

    /*reverse the desstring in desstr and append sgn at the first char of string*/
    for ( left_idx = 0, right_idx = index; left_idx < right_idx; left_idx ++, right_idx -- )
    {
    ch[ 0 ] = desstr[ right_idx ];
        desstr[ right_idx ] = desstr[ left_idx ];
    desstr[ left_idx ]  = ch[ 0 ];
    }

    if( EC_TRUE == EBGN_SGN(src) )
    {
        desstr[ 0 ] = '+';
    }
    else
    {
        desstr[ 0 ] = '-';
    }

    /*set the desstr return length*/
    *desstrlen = index + 1;

    ebgn_z_destroy(ebgnz_md_id, a);
    ebgn_z_destroy(ebgnz_md_id, q);
    ebgn_z_destroy(ebgnz_md_id, r);
    ebgn_z_destroy(ebgnz_md_id, RADIX);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_bgn(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,BIGINT *des)
{
    UINT32 ret;
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_bgn: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_bgn: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_bgn: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_FALSE == conv_check_decstr(conv_md_id, decstr, decstrlen) )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_bgn: invalid decstr\n");
        return ((UINT32)(-1));
    }

    ret = conv_str_to_bgn(conv_md_id, decstr, decstrlen, 10, conv_decchar_to_num,des);
    return ret;
}


/**
*
*   convert a bigint to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the bigint and return 0.
*
**/
UINT32 conv_bgn_to_dec(const UINT32 conv_md_id,const BIGINT *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT32 ret;
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bgn_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_bgn_to_str(conv_md_id, src, 10, conv_num_to_decchar,decstr, decstrmaxlen, decstrlen);

    return ret;
}

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
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_bgn(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,BIGINT *des)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_bgn: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_bgn: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_bgn: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_FALSE == conv_check_hexstr(conv_md_id, hexstr, hexstrlen) )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_bgn: invalid hexstr\n");
        return ((UINT32)(-1));
    }

    ret = conv_str_to_bgn(conv_md_id, hexstr, hexstrlen, 16, conv_hexchar_to_num,des);

    return ret;
}

/**
*
*   convert a BIGINT number to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of the BIGINT number and return 0.
*
**/
UINT32 conv_bgn_to_hex(const UINT32 conv_md_id,const BIGINT *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bgn_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_bgn_to_str(conv_md_id, src, 16, conv_num_to_hexchar,hexstr, hexstrmaxlen, hexstrlen);

    return ret;
}

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
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_bgn(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,BIGINT *des)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_bgn: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_bgn: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_bgn: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_FALSE == conv_check_binstr(conv_md_id, binstr, binstrlen) )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_bgn: invalid binstr\n");
        return ((UINT32)(-1));
    }

    ret = conv_str_to_bgn(conv_md_id, binstr, binstrlen, 2, conv_binchar_to_num,des);

    return ret;
}

/**
*
*   convert a BIGINT number to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of the BIGINT number and return 0.
*
**/
UINT32 conv_bgn_to_bin(const UINT32 conv_md_id,const BIGINT *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bgn_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bgn_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_bgn_to_str(conv_md_id, src, 2, conv_num_to_binchar,binstr, binstrmaxlen, binstrlen);

    return ret;
}

/*--------------------------------------------EC_CURVE_POINT----------------------------------------------------*/
/**
*
*   convert a decimal string to a EC_CURVE_POINT.
*   if the decimal is overflow the limit of element of EC_CURVE_POINT,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the EC_CURVE_POINT value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of EC_CURVE_POINT,
*      i.e., its decimal string format is: x#y#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ec_curve_point(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,EC_CURVE_POINT *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ec_curve_point: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ec_curve_point: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_ec_curve_point: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* x */
    for( index = 0; index < decstrlen; index ++ )
    {
    if( '#' == decstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)decstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->x));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ec_curve_point: x conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* y */
    for( ; index < decstrlen; index ++ )
    {
    if( '#' == decstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->y));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ec_curve_point: y conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != decstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ec_curve_point: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}


/**
*
*   convert a EC_CURVE_POINT to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_POINT,
*      i.e., its decimal string format is: x#y#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of EC_CURVE_POINT and return 0.
*
**/
UINT32 conv_ec_curve_point_to_dec(const UINT32 conv_md_id,const EC_CURVE_POINT *src,
                  UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_point_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_point_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_point_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ec_curve_point_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *decstrlen = 0;

    /* x */
    curstr = decstr;
    curstrmaxlen = decstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->x), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_point_to_dec: x conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* y */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->y), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_point_to_dec: y conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *decstrlen = ( curstr + curstrlen - decstr);

    return ret;
}

/**
*
*   convert a hex string to EC_CURVE_POINT.
*   if the hex number is overflow the limit of element of EC_CURVE_POINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is EC_CURVE_POINT value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_POINT,
*      i.e., its decimal string format is: x#y#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ec_curve_point(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,EC_CURVE_POINT *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ec_curve_point: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ec_curve_point: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_ec_curve_point: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* x */
    for( index = 0; index < hexstrlen; index ++ )
    {
    if( '#' == hexstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)hexstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->x));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ec_curve_point: x conversion failed\n");

        return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* y */
    for( ; index < hexstrlen; index ++ )
    {
        if( '#' == hexstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->y));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ec_curve_point: y conversion failed\n");

        return ret;
    }

    /* redundant checking */
    if( index + 1 != hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ec_curve_point: invalid format string\n");

        return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert EC_CURVE_POINT to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_POINT,
*      i.e., its decimal string format is: x#y#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of EC_CURVE_POINT and return 0.
*
**/
UINT32 conv_ec_curve_point_to_hex(const UINT32 conv_md_id,const EC_CURVE_POINT *src,
                 UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_point_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_point_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_point_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ec_curve_point_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *hexstrlen = 0;

    /* x */
    curstr = hexstr;
    curstrmaxlen = hexstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->x), 16, conv_num_to_hexchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_point_to_hex: x conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* y */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->y), 16, conv_num_to_hexchar, curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_point_to_hex: y conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *hexstrlen = ( curstr + curstrlen - hexstr);

    return ret;
}

/**
*
*   convert a binary string to EC_CURVE_POINT
*   if the binary number is overflow the limit of EC_CURVE_POINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the EC_CURVE_POINT value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_POINT,
*      i.e., its decimal string format is: x#y#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ec_curve_point(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,EC_CURVE_POINT *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ec_curve_point: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ec_curve_point: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_ec_curve_point: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* x */
    for( index = 0; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)binstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->x));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ec_curve_point: x conversion failed\n");

        return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* y */
    for( ; index < binstrlen; index ++ )
    {
        if( '#' == binstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->y));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ec_curve_point: y conversion failed\n");

        return ret;
    }

    /* redundant checking */
    if( index + 1 != binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ec_curve_point: invalid format string\n");

        return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert EC_CURVE_POINT to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of EC_CURVE_POINT and return 0.
*
**/
UINT32 conv_ec_curve_point_to_bin(const UINT32 conv_md_id,const EC_CURVE_POINT *src,
                 UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_point_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_point_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_point_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ec_curve_point_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *binstrlen = 0;

    /* x */
    curstr = binstr;
    curstrmaxlen = binstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->x), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_point_to_bin: x conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* y */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->y), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_point_to_bin: y conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *binstrlen = ( curstr + curstrlen - binstr);

    return ret;
}

/*--------------------------------------------EC_CURVE_AFF_POINT----------------------------------------------------*/
/**
*
*   convert a decimal string to a EC_CURVE_AFF_POINT.
*   if the decimal is overflow the limit of element of EC_CURVE_AFF_POINT,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the EC_CURVE_AFF_POINT value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of EC_CURVE_AFF_POINT,
*      i.e., its decimal string format is: x#y#z#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ec_curve_aff_point(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,EC_CURVE_AFF_POINT *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ec_curve_aff_point: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ec_curve_aff_point: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_ec_curve_aff_point: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* x */
    for( index = 0; index < decstrlen; index ++ )
    {
    if( '#' == decstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)decstr;
    curstrlen = decstr + index - curstr;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->x));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ec_curve_aff_point: x conversion failed\n");

        return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* y */
    for( ; index < decstrlen; index ++ )
    {
        if( '#' == decstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = decstr + index - curstr;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->y));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ec_curve_aff_point: y conversion failed\n");

        return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* z */
    for( ; index < decstrlen; index ++ )
    {
        if( '#' == decstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = decstr + index - curstr;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->z));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ec_curve_aff_point: z conversion failed\n");

        return ret;
    }
    /* redundant checking */
    if( index + 1 != decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ec_curve_aff_point: invalid format string\n");

        return ((UINT32)(-1));
    }

    return ret;
}


/**
*
*   convert a EC_CURVE_AFF_POINT to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_AFF_POINT,
*      i.e., its decimal string format is: x#y#z#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of EC_CURVE_AFF_POINT and return 0.
*
**/
UINT32 conv_ec_curve_aff_point_to_dec(const UINT32 conv_md_id,const EC_CURVE_AFF_POINT *src,
                  UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_aff_point_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_aff_point_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_aff_point_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ec_curve_aff_point_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *decstrlen = 0;

    /* x */
    curstr = decstr;
    curstrmaxlen = decstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->x), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_aff_point_to_dec: x conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* y */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->y), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_aff_point_to_dec: y conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* z */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->z), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_aff_point_to_dec: z conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *decstrlen = ( curstr + curstrlen - decstr);

    return ret;
}

/**
*
*   convert a hex string to EC_CURVE_AFF_POINT.
*   if the hex number is overflow the limit of element of EC_CURVE_AFF_POINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is EC_CURVE_AFF_POINT value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_AFF_POINT,
*      i.e., its decimal string format is: x#y#z#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ec_curve_aff_point(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,EC_CURVE_AFF_POINT *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ec_curve_aff_point: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ec_curve_aff_point: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_ec_curve_aff_point: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* x */
    for( index = 0; index < hexstrlen; index ++ )
    {
        if( '#' == hexstr[ index ] )
        {
            break;
        }
    }
    curstr = (UINT8 *)hexstr;
    curstrlen = hexstr + index - curstr;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->x));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ec_curve_aff_point: x conversion failed\n");

        return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* y */
    for( ; index < hexstrlen; index ++ )
    {
        if( '#' == hexstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = hexstr + index - curstr;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->y));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ec_curve_aff_point: y conversion failed\n");

        return ret;
    }

    /* z */
    for( ; index < hexstrlen; index ++ )
    {
        if( '#' == hexstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = hexstr + index - curstr;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->z));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ec_curve_aff_point: z conversion failed\n");

        return ret;
    }

    /* redundant checking */
    if( index + 1 != hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ec_curve_aff_point: invalid format string\n");

        return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert EC_CURVE_AFF_POINT to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_AFF_POINT,
*      i.e., its decimal string format is: x#y#z#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of EC_CURVE_AFF_POINT and return 0.
*
**/
UINT32 conv_ec_curve_aff_point_to_hex(const UINT32 conv_md_id,const EC_CURVE_AFF_POINT *src,
                 UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_aff_point_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_aff_point_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_aff_point_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ec_curve_aff_point_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *hexstrlen = 0;

    /* x */
    curstr = hexstr;
    curstrmaxlen = hexstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->x), 16, conv_num_to_hexchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_aff_point_to_hex: x conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* y */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->y), 16, conv_num_to_hexchar, curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_aff_point_to_hex: y conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* z */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->z), 16, conv_num_to_hexchar, curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_aff_point_to_hex: z conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *hexstrlen = ( curstr + curstrlen - hexstr);

    return ret;
}

/**
*
*   convert a binary string to EC_CURVE_AFF_POINT
*   if the binary number is overflow the limit of EC_CURVE_AFF_POINT,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the EC_CURVE_AFF_POINT value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of EC_CURVE_AFF_POINT,
*      i.e., its decimal string format is: x#y#z#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ec_curve_aff_point(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,EC_CURVE_AFF_POINT *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ec_curve_aff_point: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ec_curve_aff_point: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_ec_curve_aff_point: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* x */
    for( index = 0; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)binstr;
    curstrlen = binstr + index - curstr;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->x));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ec_curve_aff_point: x conversion failed\n");

        return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* y */
    for( ; index < binstrlen; index ++ )
    {
        if( '#' == binstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = binstr + index - curstr;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->y));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ec_curve_aff_point: y conversion failed\n");

        return ret;
    }

    /* z */
    for( ; index < binstrlen; index ++ )
    {
        if( '#' == binstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = binstr + index - curstr;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->z));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ec_curve_aff_point: z conversion failed\n");

        return ret;
    }

    /* redundant checking */
    if( index + 1 != binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ec_curve_aff_point: invalid format string\n");

        return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert EC_CURVE_AFF_POINT to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of EC_CURVE_AFF_POINT and return 0.
*
**/
UINT32 conv_ec_curve_aff_point_to_bin(const UINT32 conv_md_id,const EC_CURVE_AFF_POINT *src,
                 UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_aff_point_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_aff_point_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ec_curve_aff_point_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ec_curve_aff_point_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *binstrlen = 0;

    /* x */
    curstr = binstr;
    curstrmaxlen = binstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->x), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_aff_point_to_bin: x conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* y */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->y), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_aff_point_to_bin: y conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* z */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->z), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ec_curve_aff_point_to_bin: z conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *binstrlen = ( curstr + curstrlen - binstr);

    return ret;
}

/*--------------------------------------------ECF2N_CURVE----------------------------------------------------*/
/**
*
*   convert a decimal string to a ECF2N_CURVE.
*   if the decimal is overflow the limit of element of ECF2N_CURVE,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the ECF2N_CURVE value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of ECF2N_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ecf2n_curve(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,ECF2N_CURVE *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ecf2n_curve: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ecf2n_curve: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_ecf2n_curve: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* a */
    for( index = 0; index < decstrlen; index ++ )
    {
        if( '#' == decstr[ index ] )
        {
            break;
        }
    }
    curstr = (UINT8 *)decstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->a));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecf2n_curve: a conversion failed\n");

        return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* b */
    for( ; index < decstrlen; index ++ )
    {
        if( '#' == decstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->b));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecf2n_curve: b conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecf2n_curve: invalid format string\n");

        return ((UINT32)(-1));
    }

    return ret;
}


/**
*
*   convert a ECF2N_CURVE to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECF2N_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of ECF2N_CURVE and return 0.
*
**/
UINT32 conv_ecf2n_curve_to_dec(const UINT32 conv_md_id,const ECF2N_CURVE *src,
                  UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecf2n_curve_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecf2n_curve_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecf2n_curve_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecf2n_curve_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *decstrlen = 0;

    /* a */
    curstr = decstr;
    curstrmaxlen = decstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->a), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecf2n_curve_to_dec: a conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* b */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->b), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecf2n_curve_to_dec: b conversion failed\n");

        return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *decstrlen = ( curstr + curstrlen - decstr);

    return ret;
}

/**
*
*   convert a hex string to ECF2N_CURVE.
*   if the hex number is overflow the limit of element of ECF2N_CURVE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is ECF2N_CURVE value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECF2N_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ecf2n_curve(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,ECF2N_CURVE *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ecf2n_curve: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ecf2n_curve: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_ecf2n_curve: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* a */
    for( index = 0; index < hexstrlen; index ++ )
    {
        if( '#' == hexstr[ index ] )
        {
            break;
        }
    }
    curstr = (UINT8 *)hexstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->a));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecf2n_curve: a conversion failed\n");

        return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* b */
    for( ; index < hexstrlen; index ++ )
    {
        if( '#' == hexstr[ index ] )
        {
            break;
        }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->b));
    if ( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecf2n_curve: b conversion failed\n");

        return ret;
    }

    /* redundant checking */
    if( index + 1 != hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecf2n_curve: invalid format string\n");

        return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert ECF2N_CURVE to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECF2N_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of ECF2N_CURVE and return 0.
*
**/
UINT32 conv_ecf2n_curve_to_hex(const UINT32 conv_md_id,const ECF2N_CURVE *src,
                 UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecf2n_curve_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecf2n_curve_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecf2n_curve_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecf2n_curve_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *hexstrlen = 0;

    /* a */
    curstr = hexstr;
    curstrmaxlen = hexstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->a), 16, conv_num_to_hexchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecf2n_curve_to_hex: a conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* b */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->b), 16, conv_num_to_hexchar, curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecf2n_curve_to_hex: b conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *hexstrlen = ( curstr + curstrlen - hexstr);

    return ret;
}

/**
*
*   convert a binary string to ECF2N_CURVE
*   if the binary number is overflow the limit of ECF2N_CURVE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the ECF2N_CURVE value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECF2N_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ecf2n_curve(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,ECF2N_CURVE *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ecf2n_curve: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ecf2n_curve: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_ecf2n_curve: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* a */
    for( index = 0; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)binstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->a));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecf2n_curve: a conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* b */
    for( ; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->b));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecf2n_curve: b conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != binstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecf2n_curve: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert ECF2N_CURVE to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of ECF2N_CURVE and return 0.
*
**/
UINT32 conv_ecf2n_curve_to_bin(const UINT32 conv_md_id,const ECF2N_CURVE *src,
                 UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecf2n_curve_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecf2n_curve_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecf2n_curve_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecf2n_curve_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *binstrlen = 0;

    /* a */
    curstr = binstr;
    curstrmaxlen = binstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->a), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecf2n_curve_to_bin: a conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* b */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->b), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecf2n_curve_to_bin: b conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *binstrlen = ( curstr + curstrlen - binstr);

    return ret;
}

/*--------------------------------------------ECFP_CURVE----------------------------------------------------*/
/**
*
*   convert a decimal string to a ECFP_CURVE.
*   if the decimal is overflow the limit of element of ECFP_CURVE,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the ECFP_CURVE value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of ECFP_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ecfp_curve(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,ECFP_CURVE *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ecfp_curve: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ecfp_curve: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_ecfp_curve: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* a */
    for( index = 0; index < decstrlen; index ++ )
    {
    if( '#' == decstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)decstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->a));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecfp_curve: a conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* b */
    for( ; index < decstrlen; index ++ )
    {
    if( '#' == decstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->b));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecfp_curve: b conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != decstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecfp_curve: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}


/**
*
*   convert a ECFP_CURVE to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECFP_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of ECFP_CURVE and return 0.
*
**/
UINT32 conv_ecfp_curve_to_dec(const UINT32 conv_md_id,const ECFP_CURVE *src,
                  UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecfp_curve_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecfp_curve_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecfp_curve_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecfp_curve_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *decstrlen = 0;

    /* a */
    curstr = decstr;
    curstrmaxlen = decstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->a), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecfp_curve_to_dec: a conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* b */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->b), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecfp_curve_to_dec: b conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *decstrlen = ( curstr + curstrlen - decstr);

    return ret;
}

/**
*
*   convert a hex string to ECFP_CURVE.
*   if the hex number is overflow the limit of element of ECFP_CURVE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is ECFP_CURVE value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECFP_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ecfp_curve(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,ECFP_CURVE *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ecfp_curve: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ecfp_curve: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_ecfp_curve: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* a */
    for( index = 0; index < hexstrlen; index ++ )
    {
    if( '#' == hexstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)hexstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->a));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecfp_curve: a conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* b */
    for( ; index < hexstrlen; index ++ )
    {
    if( '#' == hexstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->b));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecfp_curve: b conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != hexstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecfp_curve: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert ECFP_CURVE to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECFP_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of ECFP_CURVE and return 0.
*
**/
UINT32 conv_ecfp_curve_to_hex(const UINT32 conv_md_id,const ECFP_CURVE *src,
                 UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecfp_curve_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecfp_curve_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecfp_curve_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecfp_curve_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *hexstrlen = 0;

    /* a */
    curstr = hexstr;
    curstrmaxlen = hexstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->a), 16, conv_num_to_hexchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecfp_curve_to_hex: a conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* b */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->b), 16, conv_num_to_hexchar, curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecfp_curve_to_hex: b conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *hexstrlen = ( curstr + curstrlen - hexstr);

    return ret;
}

/**
*
*   convert a binary string to ECFP_CURVE
*   if the binary number is overflow the limit of ECFP_CURVE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the ECFP_CURVE value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECFP_CURVE,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ecfp_curve(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,ECFP_CURVE *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ecfp_curve: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ecfp_curve: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_ecfp_curve: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* a */
    for( index = 0; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)binstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->a));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecfp_curve: a conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* b */
    for( ; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->b));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecfp_curve: b conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != binstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecfp_curve: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert ECFP_CURVE to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of ECFP_CURVE and return 0.
*
**/
UINT32 conv_ecfp_curve_to_bin(const UINT32 conv_md_id,const ECFP_CURVE *src,
                 UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecfp_curve_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecfp_curve_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecfp_curve_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecfp_curve_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *binstrlen = 0;

    /* a */
    curstr = binstr;
    curstrmaxlen = binstrmaxlen - 1; /* remain a char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->a), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecfp_curve_to_bin: a conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* b */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* a char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->b), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecfp_curve_to_bin: b conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *binstrlen = ( curstr + curstrlen - binstr);

    return ret;
}

/*--------------------------------------------ECC_KEYPAIR----------------------------------------------------*/
/**
*
*   convert private_key decimal string to private_key ECC_KEYPAIR.
*   if the decimal is overflow the limit of element of ECC_KEYPAIR,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the ECC_KEYPAIR value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of ECC_KEYPAIR,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ecc_keypair(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,ECC_KEYPAIR *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ecc_keypair: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ecc_keypair: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_ecc_keypair: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* private_key */
    for( index = 0; index < decstrlen; index ++ )
    {
    if( '#' == decstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)decstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->private_key));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecc_keypair: private_key conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* public_key */
    for( ; index < decstrlen; index ++ )
    {
    if( '#' == decstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_dec_to_ec_curve_point(conv_md_id, curstr, curstrlen, &(des->public_key));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecc_keypair: public_key conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != decstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecc_keypair: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}


/**
*
*   convert private_key ECC_KEYPAIR to private_key decimal string.
*   the destinate decimal string should be given private_key buffer with private_key maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECC_KEYPAIR,
*      i.e., its decimal string format is: a#b#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of ECC_KEYPAIR and return 0.
*
**/
UINT32 conv_ecc_keypair_to_dec(const UINT32 conv_md_id,const ECC_KEYPAIR *src,
                  UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_keypair_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_keypair_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_keypair_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecc_keypair_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *decstrlen = 0;

    /* private_key */
    curstr = decstr;
    curstrmaxlen = decstrmaxlen - 1; /* remain private_key char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->private_key), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_keypair_to_dec: private_key conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* public_key */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* private_key char position for deliminator '#' was already remained before */
    ret = conv_ec_curve_point_to_dec(conv_md_id, &(src->public_key), curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_keypair_to_dec: public_key conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *decstrlen = ( curstr + curstrlen - decstr);

    return ret;
}

/**
*
*   convert private_key hex string to ECC_KEYPAIR.
*   if the hex number is overflow the limit of element of ECC_KEYPAIR,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is ECC_KEYPAIR value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECC_KEYPAIR,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ecc_keypair(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,ECC_KEYPAIR *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ecc_keypair: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ecc_keypair: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_ecc_keypair: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* private_key */
    for( index = 0; index < hexstrlen; index ++ )
    {
    if( '#' == hexstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)hexstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->private_key));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecc_keypair: private_key conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* public_key */
    for( ; index < hexstrlen; index ++ )
    {
    if( '#' == hexstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_hex_to_ec_curve_point(conv_md_id, curstr, curstrlen, &(des->public_key));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecc_keypair: public_key conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != hexstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecc_keypair: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert ECC_KEYPAIR to private_key hex string.
*   the destinate hex string should be given private_key buffer with private_key maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECC_KEYPAIR,
*      i.e., its decimal string format is: a#b#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of ECC_KEYPAIR and return 0.
*
**/
UINT32 conv_ecc_keypair_to_hex(const UINT32 conv_md_id,const ECC_KEYPAIR *src,
                 UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_keypair_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_keypair_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_keypair_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecc_keypair_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *hexstrlen = 0;

    /* private_key */
    curstr = hexstr;
    curstrmaxlen = hexstrmaxlen - 1; /* remain private_key char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->private_key), 16, conv_num_to_hexchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_keypair_to_hex: private_key conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* public_key */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* private_key char position for deliminator '#' was already remained before */
    ret = conv_ec_curve_point_to_hex(conv_md_id, &(src->public_key), curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_keypair_to_hex: public_key conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *hexstrlen = ( curstr + curstrlen - hexstr);

    return ret;
}

/**
*
*   convert private_key binary string to ECC_KEYPAIR
*   if the binary number is overflow the limit of ECC_KEYPAIR,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the ECC_KEYPAIR value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECC_KEYPAIR,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ecc_keypair(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,ECC_KEYPAIR *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ecc_keypair: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ecc_keypair: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_ecc_keypair: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* private_key */
    for( index = 0; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)binstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->private_key));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecc_keypair: private_key conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* public_key */
    for( ; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_bin_to_ec_curve_point(conv_md_id, curstr, curstrlen, &(des->public_key));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecc_keypair: public_key conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != binstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecc_keypair: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert ECC_KEYPAIR to private_key binary string.
*   the destinate binary string should be given private_key buffer with private_key maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of ECC_KEYPAIR and return 0.
*
**/
UINT32 conv_ecc_keypair_to_bin(const UINT32 conv_md_id,const ECC_KEYPAIR *src,
                 UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_keypair_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_keypair_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_keypair_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecc_keypair_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *binstrlen = 0;

    /* private_key */
    curstr = binstr;
    curstrmaxlen = binstrmaxlen - 1; /* remain private_key char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->private_key), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_keypair_to_bin: private_key conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* public_key */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* private_key char position for deliminator '#' was already remained before */
    ret = conv_ec_curve_point_to_bin(conv_md_id, &(src->public_key), curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_keypair_to_bin: public_key conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *binstrlen = ( curstr + curstrlen - binstr);

    return ret;
}

/*--------------------------------------------ECC_SIGNATURE----------------------------------------------------*/
/**
*
*   convert decimal string to ECC_SIGNATURE.
*   if the decimal is overflow the limit of element of ECC_SIGNATURE,i.e, decimal >= 2 ^{BIGINTSIZE},then
*   report error and return -1.
*   else des is the ECC_SIGNATURE value of the decimal string and return 0.
*
*   rule: decimal string is split by '#' for each element of ECC_SIGNATURE,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ecc_signature(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,ECC_SIGNATURE *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ecc_signature: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ecc_signature: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_ecc_signature: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* r */
    for( index = 0; index < decstrlen; index ++ )
    {
    if( '#' == decstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)decstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->r));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecc_signature: r conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* s */
    for( ; index < decstrlen; index ++ )
    {
    if( '#' == decstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 10, conv_decchar_to_num, &(des->s));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecc_signature: s conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != decstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ecc_signature: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}


/**
*
*   convert ECC_SIGNATURE to decimal string.
*   the destinate decimal string should be given  buffer with  maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECC_SIGNATURE,
*      i.e., its decimal string format is: a#b#
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of ECC_SIGNATURE and return 0.
*
**/
UINT32 conv_ecc_signature_to_dec(const UINT32 conv_md_id,const ECC_SIGNATURE *src,
                  UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_signature_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_signature_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_signature_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecc_signature_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *decstrlen = 0;

    /* r */
    curstr = decstr;
    curstrmaxlen = decstrmaxlen - 1; /* remain r char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->r), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_signature_to_dec: r conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* s */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* r char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->s), 10, conv_num_to_decchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_signature_to_dec: s conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *decstrlen = ( curstr + curstrlen - decstr);

    return ret;
}

/**
*
*   convert r hex string to ECC_SIGNATURE.
*   if the hex number is overflow the limit of element of ECC_SIGNATURE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is ECC_SIGNATURE value of the hex string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECC_SIGNATURE,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the hex string contains any exceptional character,i.e.,
*   not belong to [0-9a-fA-F], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ecc_signature(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,ECC_SIGNATURE *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ecc_signature: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ecc_signature: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_ecc_signature: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* r */
    for( index = 0; index < hexstrlen; index ++ )
    {
    if( '#' == hexstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)hexstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->r));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecc_signature: r conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* s */
    for( ; index < hexstrlen; index ++ )
    {
    if( '#' == hexstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 16, conv_hexchar_to_num, &(des->s));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecc_signature: s conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != hexstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ecc_signature: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert ECC_SIGNATURE to r hex string.
*   the destinate hex string should be given r buffer with r maximum lenght at first.
*
*   rule: decimal string is concated by '#' for each element of ECC_SIGNATURE,
*      i.e., its decimal string format is: a#b#
*
*   if the hex string buffer is not enough, then print error and return nonzero.
*   otherwise, return the hex expression of ECC_SIGNATURE and return 0.
*
**/
UINT32 conv_ecc_signature_to_hex(const UINT32 conv_md_id,const ECC_SIGNATURE *src,
                 UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_signature_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_signature_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_signature_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecc_signature_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *hexstrlen = 0;

    /* r */
    curstr = hexstr;
    curstrmaxlen = hexstrmaxlen - 1; /* remain r char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->r), 16, conv_num_to_hexchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_signature_to_hex: r conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* s */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* r char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->s), 16, conv_num_to_hexchar, curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_signature_to_hex: s conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *hexstrlen = ( curstr + curstrlen - hexstr);

    return ret;
}

/**
*
*   convert r binary string to ECC_SIGNATURE
*   if the binary number is overflow the limit of ECC_SIGNATURE,i.e, number >= 2 ^{BIGINTSIZE},then
*   report error and return nonzero.
*   else des is the ECC_SIGNATURE value of the binary string and return 0.
*
*   rule: decimal string is concated by '#' for each element of ECC_SIGNATURE,
*      i.e., its decimal string format is: a#b#
*
*   note:
*       if the binary string contains any exceptional character,i.e.,
*   not belong to [0-1], then return nonzero without warning information.
*   Indeed, the waring information is reported in conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ecc_signature(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,ECC_SIGNATURE *des)
{
    UINT8  *curstr;
    UINT32 curstrlen;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ecc_signature: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ecc_signature: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_ecc_signature: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* r */
    for( index = 0; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = (UINT8 *)binstr;
    curstrlen = index;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->r));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecc_signature: r conversion failed\n");

    return ret;
    }

    /* skip '#' */
    index ++;
    curstrlen ++;

    /* s */
    for( ; index < binstrlen; index ++ )
    {
    if( '#' == binstr[ index ] )
    {
        break;
    }
    }
    curstr = curstr + curstrlen;
    curstrlen = index - curstrlen;
    ret = conv_str_to_bgn(conv_md_id, curstr, curstrlen, 2, conv_binchar_to_num, &(des->s));
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecc_signature: s conversion failed\n");

    return ret;
    }

    /* redundant checking */
    if( index + 1 != binstrlen )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ecc_signature: invalid format string\n");

    return ((UINT32)(-1));
    }

    return ret;
}

/**
*
*   convert ECC_SIGNATURE to r binary string.
*   the destinate binary string should be given r buffer with r maximum lenght at first.
*
*   if the binary string buffer is not enough, then print error and return nonzero.
*   otherwise, return the binary expression of ECC_SIGNATURE and return 0.
*
**/
UINT32 conv_ecc_signature_to_bin(const UINT32 conv_md_id,const ECC_SIGNATURE *src,
                 UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT8  *curstr;
    UINT32 curstrmaxlen;
    UINT32 curstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_signature_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_signature_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ecc_signature_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ecc_signature_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    *binstrlen = 0;

    /* r */
    curstr = binstr;
    curstrmaxlen = binstrmaxlen - 1; /* remain r char position for deliminator '#' */
    ret = conv_bgn_to_str(conv_md_id, &(src->r), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_signature_to_bin: r conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* s */
    curstr = curstr +  curstrlen;
    curstrmaxlen = curstrmaxlen - curstrlen; /* r char position for deliminator '#' was already remained before */
    ret = conv_bgn_to_str(conv_md_id, &(src->s), 2, conv_num_to_binchar,curstr, curstrmaxlen, &curstrlen);
    if ( 0 != ret )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_ecc_signature_to_bin: s conversion failed\n");

    return ret;
    }
    curstr[ curstrlen ++ ] = '#';

    /* count the total used buf */
   *binstrlen = ( curstr + curstrlen - binstr);

    return ret;
}

/*--------------------------------------------POLY----------------------------------------------------*/
/**
*
*   convert a dec string to a POLY.
*
*   poly = (item) [ (item) ]
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
static UINT32 conv_dec_to_poly_0(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,POLY *des, UINT32 *decstrusedlen)
{
    POLY_ITEM *item;

    UINT32 pos;
    UINT32 nused;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_poly_0: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_poly_0: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_poly_0: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* initialize des poly */
    POLY_INIT( des );

    pos = 0;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_dec_to_poly_0     : beg: %.*s\n", decstrlen - pos, decstr + pos);

    /* format is () which measn poly = 0 */
    if( 2 == decstrlen )
    {
        if ( NULL_PTR != decstrusedlen )
        {
            *decstrusedlen = 2;
        }
        return ( 0 );
    }

    /* do items with format ( deg(coe) ) */
    while( '(' == decstr[ pos ] )
    {
        /* skip '(' of item */
        pos ++;

        alloc_static_mem(MD_CONV, conv_md_id, MM_POLY_ITEM, &item, LOC_CONV_0041);

        /* item = deg ( bgn | poly ) */
        if( 0 != conv_dec_to_poly_item_0(conv_md_id, decstr + pos, decstrlen - pos, item,  &nused) )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,
                "conv_dec_to_poly_0: failed to conv decstr %.*s to poly_item\n", decstrlen - pos, decstr + pos);
            return ((UINT32)(-1));
        }

        POLY_ADD_ITEM_TAIL(des, item);

        pos   += nused ;

        /* skip ')' of item*/
        pos ++;
    }

    if( NULL_PTR != decstrusedlen )
    {
        *decstrusedlen = pos;
    }

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_dec_to_poly_0     : end: %.*s\n", decstrlen - pos, decstr + pos);
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_dec_to_poly_0     : eat: %.*s\n", pos, decstr);

    return ( 0 );
}

/**
*
*   convert a dec string to a POLY.
*
*   poly = (item) [ (item) ]
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_dec_to_poly(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,POLY *des)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_poly: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_poly: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_poly: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_dec_to_poly_0(conv_md_id, decstr, decstrlen, des, 0);
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_dec_to_poly: conv_dec_to_poly_0 failed where string is %.*s\n", decstrlen, decstr);
        return ((UINT32)(-1));
    }
    return ( 0 );
}

/**
*
*   convert a POLY to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   poly = ( item )[ ( item ) ]
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the POLY and return 0.
*
**/
UINT32 conv_poly_to_dec(const UINT32 conv_md_id,const POLY *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    const POLY_ITEM *item;

    UINT32    cur_decstrlen;
    UINT32    ret_decstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_poly_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret_decstrlen      = 0;

    if( EC_TRUE == POLY_IS_EMPTY(src) )
    {
        decstr[ ret_decstrlen ] = '(';
        ret_decstrlen ++;

        decstr[ ret_decstrlen ] = ')';
        ret_decstrlen ++;

        *decstrlen = ret_decstrlen;

        return ( 0 );
    }

    item = POLY_FIRST_ITEM(src);
    while( item != POLY_NULL_ITEM(src) )
    {
        decstr[ ret_decstrlen ] = '(';
        ret_decstrlen ++;

        ret = conv_poly_item_to_dec(conv_md_id, item, decstr + ret_decstrlen, decstrmaxlen - ret_decstrlen, &cur_decstrlen);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_to_dec: conv_poly_item_to_dec failed\n");
            return ((UINT32)(-1));
        }
        ret_decstrlen += cur_decstrlen;

        decstr[ ret_decstrlen ] = ')';
        ret_decstrlen ++;

        item = POLY_ITEM_NEXT(item);
    }

    *decstrlen = ret_decstrlen;

    return ( 0 );
}

/**
*
*   convert a hex string to a POLY.
*
*   poly = (item) [ (item) ]
*
*   note:
*       if the hex string contains any exceptional character,i.e., not belong to [0-9a-fA-F], then
*   return -1 without warning information.
*
**/
static UINT32 conv_hex_to_poly_0(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,POLY *des, UINT32 *hexstrusedlen)
{
    POLY_ITEM *item;

    UINT32 pos;
    UINT32 nused;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_poly_0: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_poly_0: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_poly_0: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* initialize des poly */
    POLY_INIT( des );

    pos = 0;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_hex_to_poly_0     : beg: %.*s\n", hexstrlen - pos, hexstr + pos);

    /* format is () which measn poly = 0 */
    if( 2 == hexstrlen )
    {
        if ( NULL_PTR != hexstrusedlen )
        {
            *hexstrusedlen = 2;
        }
        return ( 0 );
    }

    /* do items with format ( deg(coe) ) */
    while( '(' == hexstr[ pos ] )
    {
        /* skip '(' of item */
        pos ++;

        alloc_static_mem(MD_CONV, conv_md_id, MM_POLY_ITEM, &item, LOC_CONV_0042);

        /* item = deg ( bgn | poly ) */
        if( 0 != conv_hex_to_poly_item_0(conv_md_id, hexstr + pos, hexstrlen - pos, item,  &nused) )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,
                "conv_hex_to_poly_0: failed to conv hexstr %.*s to poly_item\n", hexstrlen - pos, hexstr + pos);
            return ((UINT32)(-1));
        }

        POLY_ADD_ITEM_TAIL(des, item);

        pos   += nused ;

        /* skip ')' of item*/
        pos ++;
    }

    if( NULL_PTR != hexstrusedlen )
    {
        *hexstrusedlen = pos;
    }

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_hex_to_poly_0     : end: %.*s\n", hexstrlen - pos, hexstr + pos);
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_hex_to_poly_0     : eat: %.*s\n", pos, hexstr);

    return ( 0 );
}

/**
*
*   convert a hex string to a POLY.
*
*   poly = (item) [ (item) ]
*
*   note:
*       if the hex string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_hex_to_poly(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,POLY *des)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_poly: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_poly: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_poly: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_hex_to_poly_0(conv_md_id, hexstr, hexstrlen, des, 0);
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_hex_to_poly: conv_hex_to_poly_0 failed where string is %.*s\n", hexstrlen, hexstr);
        return ((UINT32)(-1));
    }
    return ( 0 );
}

/**
*
*   convert a POLY to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   poly = (item) [ (item) ]
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the POLY and return 0.
*
**/
UINT32 conv_poly_to_hex(const UINT32 conv_md_id,const POLY *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    const POLY_ITEM *item;

    UINT32    cur_hexstrlen;
    UINT32    ret_hexstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_poly_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret_hexstrlen      = 0;

    if( EC_TRUE == POLY_IS_EMPTY(src) )
    {
        hexstr[ ret_hexstrlen ] = '(';
        ret_hexstrlen ++;

        hexstr[ ret_hexstrlen ] = ')';
        ret_hexstrlen ++;

        *hexstrlen = ret_hexstrlen;

        return ( 0 );
    }

    item = POLY_FIRST_ITEM(src);
    while( item != POLY_NULL_ITEM(src) )
    {
        hexstr[ ret_hexstrlen ] = '(';
        ret_hexstrlen ++;

        ret = conv_poly_item_to_hex(conv_md_id, item, hexstr + ret_hexstrlen, hexstrmaxlen - ret_hexstrlen, &cur_hexstrlen);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_to_hex: conv_poly_item_to_hex failed\n");
            return ((UINT32)(-1));
        }
        ret_hexstrlen += cur_hexstrlen;

        hexstr[ ret_hexstrlen ] = ')';
        ret_hexstrlen ++;

        item = POLY_ITEM_NEXT(item);
    }

    *hexstrlen = ret_hexstrlen;

    return ( 0 );
}

/**
*
*   convert a bin string to a POLY.
*
*   poly = (item) [ (item) ]
*
*   note:
*       if the bin string contains any exceptional character,i.e., not belong to [0-9a-fA-F], then
*   return -1 without warning information.
*
**/
static UINT32 conv_bin_to_poly_0(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,POLY *des, UINT32 *binstrusedlen)
{
    POLY_ITEM *item;

    UINT32 pos;
    UINT32 nused;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_poly_0: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_poly_0: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_poly_0: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* initialize des poly */
    POLY_INIT( des );

    pos = 0;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_bin_to_poly_0     : beg: %.*s\n", binstrlen - pos, binstr + pos);

    /* format is () which measn poly = 0 */
    if( 2 == binstrlen )
    {
        if ( NULL_PTR != binstrusedlen )
        {
            *binstrusedlen = 2;
        }
        return ( 0 );
    }

    /* do items with format ( deg(coe) ) */
    while( '(' == binstr[ pos ] )
    {
        /* skip '(' of item */
        pos ++;

        alloc_static_mem(MD_CONV, conv_md_id, MM_POLY_ITEM, &item, LOC_CONV_0043);

        /* item = deg ( bgn | poly ) */
        if( 0 != conv_bin_to_poly_item_0(conv_md_id, binstr + pos, binstrlen - pos, item,  &nused) )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,
                "conv_bin_to_poly_0: failed to conv binstr %.*s to poly_item\n", binstrlen - pos, binstr + pos);
            return ((UINT32)(-1));
        }

        POLY_ADD_ITEM_TAIL(des, item);

        pos   += nused ;

        /* skip ')' of item*/
        pos ++;
    }

    if( NULL_PTR != binstrusedlen )
    {
        *binstrusedlen = pos;
    }

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_bin_to_poly_0     : end: %.*s\n", binstrlen - pos, binstr + pos);
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_bin_to_poly_0     : eat: %.*s\n", pos, binstr);

    return ( 0 );
}

/**
*
*   convert a bin string to a POLY.
*
*   poly = (item) [ (item) ]
*
*   note:
*       if the bin string contains any exceptional character,i.e., not belong to [0-1], then
*   return -1 without warning information.
*
**/
UINT32 conv_bin_to_poly(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,POLY *des)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_poly: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_poly: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_poly: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_bin_to_poly_0(conv_md_id, binstr, binstrlen, des, 0);
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_bin_to_poly: conv_bin_to_poly_0 failed where string is %.*s\n", binstrlen, binstr);
        return ((UINT32)(-1));
    }
    return ( 0 );
}

/**
*
*   convert a POLY to a bin string.
*   the destinate bin string should be given a buffer with a maximum lenght at first.
*
*   poly = (item) [ (item) ]
*
*   if the bin string buffer is not enough, then warn and return -1.
*   otherwise, return the bin expression of the POLY and return 0.
*
**/
UINT32 conv_poly_to_bin(const UINT32 conv_md_id,const POLY *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    const POLY_ITEM *item;

    UINT32    cur_binstrlen;
    UINT32    ret_binstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_poly_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret_binstrlen      = 0;

    if( EC_TRUE == POLY_IS_EMPTY(src) )
    {
        binstr[ ret_binstrlen ] = '(';
        ret_binstrlen ++;

        binstr[ ret_binstrlen ] = ')';
        ret_binstrlen ++;

        *binstrlen = ret_binstrlen;

        return ( 0 );
    }

    item = POLY_FIRST_ITEM(src);
    while( item != POLY_NULL_ITEM(src) )
    {
        binstr[ ret_binstrlen ] = '(';
        ret_binstrlen ++;

        ret = conv_poly_item_to_bin(conv_md_id, item, binstr + ret_binstrlen, binstrmaxlen - ret_binstrlen, &cur_binstrlen);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_to_bin: conv_poly_item_to_bin failed\n");
            return ((UINT32)(-1));
        }
        ret_binstrlen += cur_binstrlen;

        binstr[ ret_binstrlen ] = ')';
        ret_binstrlen ++;

        item = POLY_ITEM_NEXT(item);
    }

    *binstrlen = ret_binstrlen;

    return ( 0 );
}

/*--------------------------------------------POLY_ITEM----------------------------------------------------*/
/**
*
*   convert a decimal string to a POLY_ITEM.
*
*   item = deg ( bgn | poly )
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
static UINT32 conv_dec_to_poly_item_0(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,POLY_ITEM *des, UINT32 *decstrusedlen)
{
    BIGINT *bgn_coe_of_item;
    POLY   *poly_coe_of_item;

    UINT32 decstrlen_of_deg;

    UINT32 nused;

    UINT32 pos;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_poly_item_0: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_poly_item_0: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_poly_item_0: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    pos = 0;

    dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_dec_to_poly_item_0: beg: %.*s\n", decstrlen - pos, decstr + pos);

    /* do degree */
    for( index = 0; index < decstrlen; index ++ )
    {
        /* until encounter coe part */
        if( '(' == decstr[ index ] )
        {
            break;
        }
    }

    decstrlen_of_deg = index;

    ret = conv_dec_to_deg(conv_md_id, decstr + pos, decstrlen_of_deg, POLY_ITEM_DEG(des) );
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_dec_to_poly_item_0: failed to conv %.*s to degree\n", decstrlen_of_deg, decstr + pos);
        return ((UINT32)(-1));
    }

    pos   += decstrlen_of_deg;

    if( '(' != decstr[ pos ] )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_dec_to_poly_item_0: error:coe not beg with '(' %.*s\n", decstrlen - pos, decstr + pos);
        return ((UINT32)(-1));
    }

    /* skip '(' of coe */
    pos ++;

    /* do coe */
    /* coe is poly */
    if( '(' == decstr[ pos ] )
    {
        alloc_static_mem(MD_CONV, conv_md_id, MM_POLY, &poly_coe_of_item, LOC_CONV_0044);
        ret = conv_dec_to_poly_0(conv_md_id, decstr + pos, decstrlen - pos, poly_coe_of_item, &nused);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_dec_to_poly_item_0: failed to conv %.*s to poly coe\n", decstrlen - pos, decstr + pos);
            return ((UINT32)(-1));
        }

        POLY_ITEM_BGN_COE_FLAG(des) = EC_FALSE;
        POLY_ITEM_POLY_COE(des) = poly_coe_of_item;
    }
    /* coe is bgn */
    else
    {
        for( index = pos; index < decstrlen; index ++ )
        {
            /* coe is bgn */
            if( ')'    == decstr[ index ] )
            {
                break;
            }
        }

        if( index >= decstrlen )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_dec_to_poly_item_0: error string %.*s\n", decstrlen - pos, decstr + pos);
            return ((UINT32)(-1));
        }

        alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_CONV_0045);
        ret = conv_dec_to_bgn(conv_md_id, decstr + pos, index - pos, bgn_coe_of_item);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_dec_to_poly_item_0: failed to conv %.*s to bgn coe\n", index - pos, decstr + pos);
            return ((UINT32)(-1));
        }

        POLY_ITEM_BGN_COE_FLAG(des) = EC_TRUE;
        POLY_ITEM_BGN_COE(des) = bgn_coe_of_item;

        nused = index - pos;
    }

    pos += nused;

    /* skip ')' of coe */
    pos ++;

    if( NULL_PTR != decstrusedlen )
    {
        *decstrusedlen = pos;
    }

    dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_dec_to_poly_item_0: end: %.*s\n", decstrlen - pos, decstr + pos);
    dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_dec_to_poly_item_0: eat: %.*s\n", pos, decstr);

    return ( 0 );
}

/**
*
*   convert a decimal string to a POLY_ITEM.
*
*   item = deg ( bgn | poly )
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_dec_to_poly_item(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,POLY_ITEM *des)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_poly_item: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_poly_item: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_poly_item: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_dec_to_poly_item_0(conv_md_id, decstr, decstrlen, des, 0);
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_dec_to_poly_item: failed to conv string to poly_item: %.*s\n", decstrlen, decstr);
        return ((UINT32)(-1));
    }

    return ( 0 );
}

/**
*
*   convert a POLY_ITEM to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   item = deg ( bgn | poly )
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the POLY_ITEM and return 0.
*
**/
UINT32 conv_poly_item_to_dec(const UINT32 conv_md_id,const POLY_ITEM *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT32    cur_decstrlen;
    UINT32    ret_decstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_item_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_item_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_item_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_poly_item_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret_decstrlen = 0;

    ret = conv_deg_to_dec(conv_md_id, POLY_ITEM_DEG(src),
                  decstr + ret_decstrlen,
                  decstrmaxlen - ret_decstrlen,
                  &cur_decstrlen);
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_item_to_dec: failed to conv degree to decstr\n");
        return ((UINT32)(-1));
    }

    ret_decstrlen = ret_decstrlen + cur_decstrlen;

    decstr[ ret_decstrlen ] = '(';
    ret_decstrlen ++;

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(src) )
    {
        ret = conv_bgn_to_dec(conv_md_id, POLY_ITEM_BGN_COE(src),
                      decstr + ret_decstrlen,
                      decstrmaxlen - ret_decstrlen,
                      &cur_decstrlen);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_item_to_dec: failed to conv bgn coe to decstr\n");
            return ((UINT32)(-1));
        }

        ret_decstrlen = ret_decstrlen + cur_decstrlen;
    }
    else
    {
        ret = conv_poly_to_dec(conv_md_id, POLY_ITEM_POLY_COE(src),
                      decstr + ret_decstrlen,
                      decstrmaxlen - ret_decstrlen,
                      &cur_decstrlen);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_item_to_dec: failed to conv poly coe to decstr\n");
            return ((UINT32)(-1));
        }

        ret_decstrlen = ret_decstrlen + cur_decstrlen;
    }

    decstr[ ret_decstrlen ] = ')';
    ret_decstrlen ++;

    *decstrlen = ret_decstrlen;

    return ( 0 );
}

/**
*
*   convert a hex string to a POLY_ITEM.
*
*   item = deg ( bgn | poly )
*
*   note:
*       if the hex string contains any exceptional character,i.e., not belong to [0-9a-fA-F], then
*   return -1 without warning information.
*
**/
static UINT32 conv_hex_to_poly_item_0(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,POLY_ITEM *des, UINT32 *hexstrusedlen)
{
    BIGINT *bgn_coe_of_item;
    POLY   *poly_coe_of_item;

    UINT32 hexstrlen_of_deg;

    UINT32 nused;

    UINT32 pos;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_poly_item_0: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_poly_item_0: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_poly_item_0: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    pos = 0;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_hex_to_poly_item_0: beg: %.*s\n", hexstrlen - pos, hexstr + pos);

    /* do degree */
    for( index = 0; index < hexstrlen; index ++ )
    {
        /* until encounter coe part */
        if( '(' == hexstr[ index ] )
        {
            break;
        }
    }

    hexstrlen_of_deg = index;

    ret = conv_hex_to_deg(conv_md_id, hexstr + pos, hexstrlen_of_deg, POLY_ITEM_DEG(des) );
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_hex_to_poly_item_0: failed to conv %.*s to degree\n", hexstrlen_of_deg, hexstr + pos);
        return ((UINT32)(-1));
    }

    pos   += hexstrlen_of_deg;

    if( '(' != hexstr[ pos ] )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_hex_to_poly_item_0: error:coe not beg with '(' %.*s\n", hexstrlen - pos, hexstr + pos);
        return ((UINT32)(-1));
    }

    /* skip '(' of coe */
    pos ++;

    /* do coe */
    /* coe is poly */
    if( '(' == hexstr[ pos ] )
    {
        alloc_static_mem(MD_CONV, conv_md_id, MM_POLY, &poly_coe_of_item, LOC_CONV_0046);
        ret = conv_hex_to_poly_0(conv_md_id, hexstr + pos, hexstrlen - pos, poly_coe_of_item, &nused);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_hex_to_poly_item_0: failed to conv %.*s to poly coe\n", hexstrlen - pos, hexstr + pos);
            return ((UINT32)(-1));
        }

        POLY_ITEM_BGN_COE_FLAG(des) = EC_FALSE;
        POLY_ITEM_POLY_COE(des) = poly_coe_of_item;
    }
    /* coe is bgn */
    else
    {
        for( index = pos; index < hexstrlen; index ++ )
        {
            /* coe is bgn */
            if( ')'    == hexstr[ index ] )
            {
                break;
            }
        }

        if( index >= hexstrlen )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_hex_to_poly_item_0: error string %.*s\n", hexstrlen - pos, hexstr + pos);
            return ((UINT32)(-1));
        }

        alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_CONV_0047);
        ret = conv_hex_to_bgn(conv_md_id, hexstr + pos, index - pos, bgn_coe_of_item);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_hex_to_poly_item_0: failed to conv %.*s to bgn coe\n", index - pos, hexstr + pos);
            return ((UINT32)(-1));
        }

        POLY_ITEM_BGN_COE_FLAG(des) = EC_TRUE;
        POLY_ITEM_BGN_COE(des) = bgn_coe_of_item;

        nused = index - pos;
    }

    pos += nused;

    /* skip ')' of coe */
    pos ++;

    if( NULL_PTR != hexstrusedlen )
    {
        *hexstrusedlen = pos;
    }

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_hex_to_poly_item_0: end: %.*s\n", hexstrlen - pos, hexstr + pos);
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_hex_to_poly_item_0: eat: %.*s\n", pos, hexstr);

    return ( 0 );
}

/**
*
*   convert a hex string to a POLY_ITEM.
*
*   item = deg ( bgn | poly )
*
*   note:
*       if the hex string contains any exceptional character,i.e., not belong to [0-9a-fA-F], then
*   return -1 without warning information.
*
**/
UINT32 conv_hex_to_poly_item(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,POLY_ITEM *des)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_poly_item: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_poly_item: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_poly_item: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_hex_to_poly_item_0(conv_md_id, hexstr, hexstrlen, des, 0);
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_hex_to_poly_item: failed to conv string to poly_item: %.*s\n", hexstrlen, hexstr);
        return ((UINT32)(-1));
    }

    return ( 0 );
}

/**
*
*   convert a POLY_ITEM to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   item = deg ( bgn | poly )
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the POLY_ITEM and return 0.
*
**/
UINT32 conv_poly_item_to_hex(const UINT32 conv_md_id,const POLY_ITEM *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT32    cur_hexstrlen;
    UINT32    ret_hexstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_item_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_item_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_item_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_poly_item_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret_hexstrlen = 0;

    ret = conv_deg_to_hex(conv_md_id, POLY_ITEM_DEG(src),
                  hexstr + ret_hexstrlen,
                  hexstrmaxlen - ret_hexstrlen,
                  &cur_hexstrlen);
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_item_to_hex: failed to conv degree to hexstr\n");
        return ((UINT32)(-1));
    }

    ret_hexstrlen = ret_hexstrlen + cur_hexstrlen;

    hexstr[ ret_hexstrlen ] = '(';
    ret_hexstrlen ++;

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(src) )
    {
        ret = conv_bgn_to_hex(conv_md_id, POLY_ITEM_BGN_COE(src),
                      hexstr + ret_hexstrlen,
                      hexstrmaxlen - ret_hexstrlen,
                      &cur_hexstrlen);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_item_to_hex: failed to conv bgn coe to hexstr\n");
            return ((UINT32)(-1));
        }

        ret_hexstrlen = ret_hexstrlen + cur_hexstrlen;
    }
    else
    {
        ret = conv_poly_to_hex(conv_md_id, POLY_ITEM_POLY_COE(src),
                      hexstr + ret_hexstrlen,
                      hexstrmaxlen - ret_hexstrlen,
                      &cur_hexstrlen);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_item_to_hex: failed to conv poly coe to hexstr\n");
            return ((UINT32)(-1));
        }

        ret_hexstrlen = ret_hexstrlen + cur_hexstrlen;
    }

    hexstr[ ret_hexstrlen ] = ')';
    ret_hexstrlen ++;

    *hexstrlen = ret_hexstrlen;

    return ( 0 );
}

/**
*
*   convert a bin string to a POLY_ITEM.
*
*   item = deg ( bgn | poly )
*
*   note:
*       if the bin string contains any exceptional character,i.e., not belong to [0-1], then
*   return -1 without warning information.
*
**/
static UINT32 conv_bin_to_poly_item_0(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,POLY_ITEM *des, UINT32 *binstrusedlen)
{
    BIGINT *bgn_coe_of_item;
    POLY   *poly_coe_of_item;

    UINT32 binstrlen_of_deg;

    UINT32 nused;

    UINT32 pos;
    UINT32 index;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_poly_item_0: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_poly_item_0: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_poly_item_0: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    pos = 0;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_bin_to_poly_item_0: beg: %.*s\n", binstrlen - pos, binstr + pos);

    /* do degree */
    for( index = 0; index < binstrlen; index ++ )
    {
        /* until encounter coe part */
        if( '(' == binstr[ index ] )
        {
            break;
        }
    }

    binstrlen_of_deg = index;

    ret = conv_bin_to_deg(conv_md_id, binstr + pos, binstrlen_of_deg, POLY_ITEM_DEG(des) );
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_bin_to_poly_item_0: failed to conv %.*s to degree\n", binstrlen_of_deg, binstr + pos);
        return ((UINT32)(-1));
    }

    pos   += binstrlen_of_deg;

    if( '(' != binstr[ pos ] )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_bin_to_poly_item_0: error:coe not beg with '(' %.*s\n", binstrlen - pos, binstr + pos);
        return ((UINT32)(-1));
    }

    /* skip '(' of coe */
    pos ++;

    /* do coe */
    /* coe is poly */
    if( '(' == binstr[ pos ] )
    {
        alloc_static_mem(MD_CONV, conv_md_id, MM_POLY, &poly_coe_of_item, LOC_CONV_0048);
        ret = conv_bin_to_poly_0(conv_md_id, binstr + pos, binstrlen - pos, poly_coe_of_item, &nused);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_bin_to_poly_item_0: failed to conv %.*s to poly coe\n", binstrlen - pos, binstr + pos);
            return ((UINT32)(-1));
        }

        POLY_ITEM_BGN_COE_FLAG(des) = EC_FALSE;
        POLY_ITEM_POLY_COE(des) = poly_coe_of_item;
    }
    /* coe is bgn */
    else
    {
        for( index = pos; index < binstrlen; index ++ )
        {
            /* coe is bgn */
            if( ')'    == binstr[ index ] )
            {
                break;
            }
        }

        if( index >= binstrlen )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_bin_to_poly_item_0: error string %.*s\n", binstrlen - pos, binstr + pos);
            return ((UINT32)(-1));
        }

        alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_CONV_0049);
        ret = conv_bin_to_bgn(conv_md_id, binstr + pos, index - pos, bgn_coe_of_item);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_bin_to_poly_item_0: failed to conv %.*s to bgn coe\n", index - pos, binstr + pos);
            return ((UINT32)(-1));
        }

        POLY_ITEM_BGN_COE_FLAG(des) = EC_TRUE;
        POLY_ITEM_BGN_COE(des) = bgn_coe_of_item;

        nused = index - pos;
    }

    pos += nused;

    /* skip ')' of coe */
    pos ++;

    if( NULL_PTR != binstrusedlen )
    {
        *binstrusedlen = pos;
    }

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_bin_to_poly_item_0: end: %.*s\n", binstrlen - pos, binstr + pos);
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT,"conv_bin_to_poly_item_0: eat: %.*s\n", pos, binstr);

    return ( 0 );
}
/**
*
*   convert a bin string to a POLY_ITEM.
*
*   item = deg ( bgn | poly )
*
*   note:
*       if the bin string contains any exceptional character,i.e., not belong to [0-1], then
*   return -1 without warning information.
*
**/
UINT32 conv_bin_to_poly_item(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,POLY_ITEM *des)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_poly_item: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_poly_item: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_poly_item: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_bin_to_poly_item_0(conv_md_id, binstr, binstrlen, des, 0);
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_bin_to_poly_item: failed to conv string to poly_item: %.*s\n", binstrlen, binstr);
        return ((UINT32)(-1));
    }

    return ( 0 );
}

/**
*
*   convert a POLY_ITEM to a bin string.
*   the destinate bin string should be given a buffer with a maximum lenght at first.
*
*   item = deg ( bgn | poly )
*
*   if the bin string buffer is not enough, then warn and return -1.
*   otherwise, return the bin expression of the POLY_ITEM and return 0.
*
**/
UINT32 conv_poly_item_to_bin(const UINT32 conv_md_id,const POLY_ITEM *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT32    cur_binstrlen;
    UINT32    ret_binstrlen;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_item_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_item_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_poly_item_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_poly_item_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret_binstrlen = 0;

    ret = conv_deg_to_bin(conv_md_id, POLY_ITEM_DEG(src),
                  binstr + ret_binstrlen,
                  binstrmaxlen - ret_binstrlen,
                  &cur_binstrlen);
    if( 0 != ret )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_item_to_bin: failed to conv degree to binstr\n");
        return ((UINT32)(-1));
    }

    ret_binstrlen = ret_binstrlen + cur_binstrlen;

    binstr[ ret_binstrlen ] = '(';
    ret_binstrlen ++;

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(src) )
    {
        ret = conv_bgn_to_bin(conv_md_id, POLY_ITEM_BGN_COE(src),
                      binstr + ret_binstrlen,
                      binstrmaxlen - ret_binstrlen,
                      &cur_binstrlen);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_item_to_bin: failed to conv bgn coe to binstr\n");
            return ((UINT32)(-1));
        }

        ret_binstrlen = ret_binstrlen + cur_binstrlen;
    }
    else
    {
        ret = conv_poly_to_bin(conv_md_id, POLY_ITEM_POLY_COE(src),
                      binstr + ret_binstrlen,
                      binstrmaxlen - ret_binstrlen,
                      &cur_binstrlen);
        if( 0 != ret )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"conv_poly_item_to_bin: failed to conv poly coe to binstr\n");
            return ((UINT32)(-1));
        }

        ret_binstrlen = ret_binstrlen + cur_binstrlen;
    }

    binstr[ ret_binstrlen ] = ')';
    ret_binstrlen ++;

    *binstrlen = ret_binstrlen;

    return ( 0 );
}

/*--------------------------------------------UINT32----------------------------------------------------*/
/**
*
*   convert a decimal string to a UINT32.
*   if the decimal is overflow the limit of UINT32,i.e, decimal >= 2 ^{UINT32SIZE},then
*   report error and return -1.
*   else des is the UINT32 value of the decimal string and return 0.
*
*   note:
*       if the decimal string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information.
*
**/
UINT32 conv_dec_to_uint32(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,UINT32 *des)
{
    BIGINT *bgn;

    CONV_MD    *conv_md;
    UINT32  bgnz_md_id;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_uint32: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_uint32: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_uint32: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &bgn, LOC_CONV_0050);
#endif/* STATIC_MEMORY_SWITCH */

    conv_md = &(g_conv_md[ conv_md_id ]);
    bgnz_md_id  = conv_md->bgnz_md_id;

    ret = conv_dec_to_bgn(conv_md_id, decstr, decstrlen, bgn);
    if( 0 == ret )
    {
    if( EC_FALSE == bgn_z_get_word(bgnz_md_id, bgn, des ) )
    {
        ret = ((UINT32)(-2));
    }
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, bgn, LOC_CONV_0051);
#endif/* STATIC_MEMORY_SWITCH */

    return ret;
}

/**
*
*   convert a UINT32 to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the UINT32 and return 0.
*
**/
UINT32 conv_uint32_to_dec(const UINT32 conv_md_id,const UINT32 src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    BIGINT *bgn;

    CONV_MD    *conv_md;
    UINT32  bgnz_md_id;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint32_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint32_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_uint32_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &bgn, LOC_CONV_0052);
#endif/* STATIC_MEMORY_SWITCH */

    conv_md = &(g_conv_md[ conv_md_id ]);
    bgnz_md_id  = conv_md->bgnz_md_id;

    bgn_z_set_word(bgnz_md_id, bgn, src);

    print_bigint(LOGSTDOUT, bgn);

    ret = conv_bgn_to_dec(conv_md_id, bgn, decstr, decstrmaxlen, decstrlen);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, bgn, LOC_CONV_0053);
#endif/* STATIC_MEMORY_SWITCH */

    return ret;
}

UINT32 conv_uint32_to_dec_0(const UINT32 conv_md_id,const UINT32 *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    return conv_uint32_to_dec(conv_md_id, *src, decstr, decstrmaxlen, decstrlen);
}

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
UINT32 conv_hex_to_uint32(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,UINT32 *des)
{
    UINT32 data;
    UINT32 e;
    UINT32 pos;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_uint32: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_uint32: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_uint32: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if((WORDSIZE / 4 ) > hexstrlen)
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_uint32: buffer is not enough with length %d.\n", hexstrlen);
        dbg_exit(MD_CONV, conv_md_id);
    }

    data = 0;
    for(pos = 0; pos < (WORDSIZE / 4 ); pos ++)
    {
        conv_hexchar_to_num(conv_md_id, *(hexstr + pos), &e);
        data = ((data << 4) | e);
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_uint32: %0lx <--- %.*s\n", data, WORDSIZE/4, hexstr);
    *des = data;

    return 0;
}

/**
*
*   convert a UINT32 to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the UINT32 and return 0.
*
**/
UINT32 conv_uint32_to_hex(const UINT32 conv_md_id,const UINT32 src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT32 data;
    UINT32 e;
    UINT32 pos;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint32_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint32_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_uint32_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if((WORDSIZE / 4 ) > hexstrmaxlen)
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint32_to_hex: buffer is not enough with max length %d.\n", hexstrmaxlen);
        dbg_exit(MD_CONV, conv_md_id);
    }

    e = 0xF;/*4 bits per symbol*/
    data = src;

    for(pos = (WORDSIZE / 4 ); pos -- > 0; )
    {
        conv_num_to_hexchar(conv_md_id, data & e, hexstr + pos);
        data >>= 4;
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_uint32_to_hex: %0lx ---> %.*s\n", src, WORDSIZE/4, hexstr);
    *hexstrlen = (WORDSIZE / 4 );

    return (0);
}

UINT32 conv_uint32_to_hex_0(const UINT32 conv_md_id,const UINT32 *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    return conv_uint32_to_hex(conv_md_id, *src, hexstr, hexstrmaxlen, hexstrlen);
}

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
UINT32 conv_bin_to_uint32(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,UINT32 *des)
{
    BIGINT *bgn;

    CONV_MD    *conv_md;
    UINT32  bgnz_md_id;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_uint32: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_uint32: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_uint32: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &bgn, LOC_CONV_0054);
#endif/* STATIC_MEMORY_SWITCH */

    conv_md = &(g_conv_md[ conv_md_id ]);
    bgnz_md_id  = conv_md->bgnz_md_id;

    ret = conv_bin_to_bgn(conv_md_id, binstr, binstrlen, bgn);
    if( 0 == ret )
    {
        if( EC_FALSE == bgn_z_get_word(bgnz_md_id, bgn, des ) )
        {
            ret = ((UINT32)(-2));
        }
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, bgn, LOC_CONV_0055);
#endif/* STATIC_MEMORY_SWITCH */

    return ret;
}

/**
*
*   convert a UINT32 to a bin string.
*   the destinate bin string should be given a buffer with a maximum lenght at first.
*
*   if the bin string buffer is not enough, then warn and return -1.
*   otherwise, return the bin expression of the UINT32 and return 0.
*
**/
UINT32 conv_uint32_to_bin(const UINT32 conv_md_id,const UINT32 src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    BIGINT *bgn;

    CONV_MD    *conv_md;
    UINT32  bgnz_md_id;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint32_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint32_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_uint32_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_CONV, conv_md_id, MM_BIGINT, &bgn, LOC_CONV_0056);
#endif/* STATIC_MEMORY_SWITCH */

    conv_md = &(g_conv_md[ conv_md_id ]);
    bgnz_md_id  = conv_md->bgnz_md_id;

    bgn_z_set_word(bgnz_md_id, bgn, src);

    ret = conv_bgn_to_bin(conv_md_id, bgn, binstr, binstrmaxlen, binstrlen);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, bgn, LOC_CONV_0057);
#endif/* STATIC_MEMORY_SWITCH */

    return ret;
}

UINT32 conv_uint32_to_bin_0(const UINT32 conv_md_id,const UINT32 *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    return conv_uint32_to_bin(conv_md_id, *src, binstr, binstrmaxlen, binstrlen);
}

/*--------------------------------------------REAL----------------------------------------------------*/
UINT32 conv_real_to_dec(const UINT32 conv_md_id,const REAL * src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT8 buf[128];
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_real_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_real_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_real_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if( DOUBLE_IN_CHAR > decstrmaxlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_real_to_dec: decstr buf is not enough: decstrmaxlen = %d\n", decstrmaxlen);
        return ((UINT32)(-1));
    }
#if (32 == DOUBLE_IN_CHAR)
    sprintf((char *)buf, "%+032.14f", *src);
#endif/*DOUBLE_IN_CHAR*/
#if (64 == DOUBLE_IN_CHAR)
    sprintf((char *)buf, "%+064.30f", *src);
#endif/*DOUBLE_IN_CHAR*/

    sprintf((char *)decstr, "%.*s", DOUBLE_IN_CHAR, buf);/*fix 32 char width including sign, dot, and numbers*/
    *decstrlen = DOUBLE_IN_CHAR;

    return (0);
}

UINT32 conv_dec_to_real(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,REAL *des)
{
    UINT8 buf[128];
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_real: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_real: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_real: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if( DOUBLE_IN_CHAR > decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_real: decstrlen %d is out of range\n", decstrlen);
        return ((UINT32)(-1));
    }

    sprintf((char *)buf, "%.*s", DOUBLE_IN_CHAR, decstr);

    *des = atof((char *)buf);
    return (0);
}

UINT32 conv_real_to_hex(const UINT32 conv_md_id,const REAL * src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT32 *data;
    UINT32 this_len;
    UINT32 sum_len;
    UINT32 idx;

    data = (UINT32 *)src;
    sum_len = 0;

    for(idx = 0; idx < (DOUBLESIZE)/(WORDSIZE); idx ++)
    {
        conv_uint32_to_hex_0(conv_md_id, data + idx, hexstr + sum_len, hexstrmaxlen - sum_len, &this_len);
        sum_len += this_len;
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_real_to_hex: real = %f, str = %.*s\n", *src, sum_len, hexstr);
    *hexstrlen = sum_len;
    return (0);

    /*trick here!*/
    //return conv_real_to_dec(conv_md_id, src, hexstr, hexstrmaxlen, hexstrlen);
}

UINT32 conv_hex_to_real(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,REAL *des)
{
    UINT32 *data;
    UINT32 sum_len;
    UINT32 idx;

    data = (UINT32 *)des;
    sum_len = 0;

    for(idx = 0; idx < (DOUBLESIZE / WORDSIZE); idx ++)
    {
        conv_hex_to_uint32(conv_md_id, hexstr + sum_len, hexstrlen - sum_len, data + idx);
        sum_len += (WORDSIZE / 4 );
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_real: real = %f, str = %.*s\n", *des, sum_len, hexstr);

    /*trick here!*/
    //return conv_dec_to_real(conv_md_id, hexstr, hexstrlen, des);
    return (0);
}

UINT32 conv_real_to_bin(const UINT32 conv_md_id,const REAL * src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    /*trick here!*/
    return conv_real_to_dec(conv_md_id, src, binstr, binstrmaxlen, binstrlen);
}
UINT32 conv_bin_to_real(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,REAL *des)
{
    /*trick here!*/
    return conv_dec_to_real(conv_md_id, binstr, binstrlen, des);
}

/*--------------------------------------------VECTORR----------------------------------------------------*/
UINT32 conv_vectorr_block_to_dec(const UINT32 conv_md_id,const VECTOR_BLOCK * vector_block, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT32 sub_idx;

    REAL *data_addr;
    REAL zero;

    UINT8 *cur_decstr;

    UINT32 len;
    UINT32 left_decstrmaxlen;

    /*initialize*/
    *decstrlen = 0;
    cur_decstr = decstr;
    left_decstrmaxlen = decstrmaxlen;

    REAL_SETZERO(0, zero);

#if (32 == WORDSIZE)
    sprintf((char *)cur_decstr, "%08lx", VECTOR_BLOCK_GET_NUM(vector_block));
#endif/*WORDSIZE*/
#if (64 == WORDSIZE)
    sprintf((char *)cur_decstr, "%016lx", VECTOR_BLOCK_GET_NUM(vector_block));
#endif/*WORDSIZE*/

    len = (WORDSIZE / 4);/*16 = 2^4*/
    cur_decstr        += len;
    left_decstrmaxlen -= len;
    *decstrlen        += len;

    for( sub_idx = 0; sub_idx < VECTOR_BLOCK_GET_NUM(vector_block); sub_idx ++)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
        if(NULL_PTR == data_addr)
        {
            data_addr = &zero;
        }

        conv_real_to_dec(conv_md_id, data_addr, cur_decstr, left_decstrmaxlen, &len);
        cur_decstr        += len;
        left_decstrmaxlen -= len;
        *decstrlen        += len;
    }

    return (0);
}

UINT32 conv_dec_to_vectorr_block(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,VECTOR_BLOCK *vector_block)
{
    UINT32 sub_idx;

    UINT32 num;
    UINT32 len;

    UINT8 *cur_decstr;
    REAL *data_addr;

    UINT32 left_decstrlen;


#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_vectorr_block: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == vector_block )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_vectorr_block: vector_block is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_vectorr_block: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
    /*initialize*/
    cur_decstr = (UINT8 *)decstr;
    left_decstrlen = decstrlen;

    if( (WORDSIZE / 4) > left_decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_dec_to_vectorr_block: left_decstrlen %d is too small\n", left_decstrlen);
        return ((UINT32)(-1));
    }

#if (32 == WORDSIZE)
    sscanf((char *)cur_decstr, "%08lx",  &num);
#endif/*WORDSIZE*/
#if (64 == WORDSIZE)
    sscanf((char *)cur_decstr, "%016lx",  &num);
#endif/*WORDSIZE*/

    sscanf((char *)cur_decstr, "%08lx",  &num);
    len = (WORDSIZE / 4);
    cur_decstr    += len;
    left_decstrlen -= len;

    VECTOR_BLOCK_SET_NUM(vector_block, num);

    for( sub_idx = 0; sub_idx < num; sub_idx ++ )
    {
        len = DOUBLE_IN_CHAR;

        if( len > left_decstrlen )
        {
            dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_dec_to_vectorr_block: corrupted vector with not enough data\n");
            return ((UINT32)(-1));
        }

        alloc_static_mem(MD_CONV, conv_md_id, MM_REAL, &data_addr, LOC_CONV_0058);
        conv_dec_to_real(conv_md_id, cur_decstr, left_decstrlen, data_addr);
        VECTOR_BLOCK_SET_DATA_ADDR(vector_block, sub_idx, data_addr);

        cur_decstr     += len;
        left_decstrlen -= len;
    }

    return (0);
}

UINT32 conv_vectorr_block_to_hex(const UINT32 conv_md_id,const VECTOR_BLOCK * vector_block, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT32 sub_idx;

    REAL *data_addr;
    REAL zero;

    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrmaxlen;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == vector_block )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_vectorr_block_to_hex: vector_block is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_vectorr_block_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_vectorr_block_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_vectorr_block_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = hexstr;
    left_hexstrmaxlen = hexstrmaxlen;

    REAL_SETZERO(0, zero);

    conv_uint32_to_hex(conv_md_id, VECTOR_BLOCK_GET_NUM(vector_block), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    for( sub_idx = 0; sub_idx < VECTOR_BLOCK_GET_NUM(vector_block); sub_idx ++)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
        if(NULL_PTR == data_addr)
        {
            data_addr = &zero;
        }

        conv_real_to_hex(conv_md_id, data_addr, cur_hexstr, left_hexstrmaxlen, &len);
        cur_hexstr        += len;
        left_hexstrmaxlen -= len;
    }
    *hexstrlen = hexstrmaxlen - left_hexstrmaxlen;

    return (0);
}

UINT32 conv_hex_to_vectorr_block(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,VECTOR_BLOCK *vector_block)
{
    UINT32 sub_idx;

    UINT32 num;
    UINT32 len;

    UINT8 *cur_hexstr;
    REAL *data_addr;

    UINT32 left_hexstrlen;


#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_vectorr_block: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == vector_block )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_vectorr_block: vector_block is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_vectorr_block: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
    /*initialize*/
    cur_hexstr = (UINT8 *)hexstr;
    left_hexstrlen = hexstrlen;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &num);
    len = (WORDSIZE / 4);
    cur_hexstr    += len;
    left_hexstrlen -= len;

    VECTOR_BLOCK_SET_NUM(vector_block, num);

    for( sub_idx = 0; sub_idx < num; sub_idx ++ )
    {
        alloc_static_mem(MD_CONV, conv_md_id, MM_REAL, &data_addr, LOC_CONV_0059);
        conv_hex_to_real(conv_md_id, cur_hexstr, left_hexstrlen, data_addr);
        VECTOR_BLOCK_SET_DATA_ADDR(vector_block, sub_idx, data_addr);

        len = (DOUBLESIZE / 4);
        cur_hexstr     += len;
        left_hexstrlen -= len;
    }

    return (0);
}

UINT32 conv_vectorr_block_to_bin(const UINT32 conv_md_id,const VECTOR_BLOCK * vector_block, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    return conv_vectorr_block_to_dec(conv_md_id, vector_block, binstr, binstrmaxlen, binstrlen);
}

UINT32 conv_bin_to_vectorr_block(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,VECTOR_BLOCK *vector_block)
{
    return conv_dec_to_vectorr_block(conv_md_id, binstr, binstrlen, vector_block);
}

/*--------------------------------------------VECTORR----------------------------------------------------*/
UINT32 conv_vectorr_to_dec(const UINT32 conv_md_id,const VECTOR * vector, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    VECTOR_BLOCK *vector_block;
    UINT8 *cur_decstr;

    UINT32 len;
    UINT32 left_decstrmaxlen;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == vector )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_vectorr_to_dec: vector is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_vectorr_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_vectorr_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_vectorr_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    *decstrlen = 0;
    cur_decstr = decstr;
    left_decstrmaxlen = decstrmaxlen;

    if( (2 * (WORDSIZE / 4)) > left_decstrmaxlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_vectorr_to_dec: left_decstrmaxlen %d is too small\n", left_decstrmaxlen);
        return ((UINT32)(-1));
    }

    /*rotated flag and num of vector*/
#if (32 == WORDSIZE)
    sprintf((char *)cur_decstr,"%08lx%08lx", VECTOR_GET_ROTATED_FLAG(vector), VECTOR_GET_NUM(vector));
#endif/*WORDSIZE*/
#if (64 == WORDSIZE)
    sprintf((char *)cur_decstr,"%016lx%016lx", VECTOR_GET_ROTATED_FLAG(vector), VECTOR_GET_NUM(vector));
#endif/*WORDSIZE*/

    len = (2 * (WORDSIZE / 4));
    cur_decstr        += len;
    left_decstrmaxlen -= len;
    *decstrlen        += len;

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        conv_vectorr_block_to_dec(conv_md_id, vector_block, cur_decstr, left_decstrmaxlen, &len);
        cur_decstr        += len;
        left_decstrmaxlen -= len;
        *decstrlen        += len;
    }

   return (0);
}

UINT32 conv_dec_to_vectorr(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,VECTOR *vector)
{
    CONV_MD    *conv_md;
    VECTOR_BLOCK *vector_block;
    UINT8 *cur_decstr;

    UINT32 num;
    UINT32 rotated_flag;

    UINT32 len;
    UINT32 left_decstrlen;

    UINT32 vectorr_md_id;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_vectorr: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == vector )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_vectorr: vector is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_vectorr: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
    conv_md = &(g_conv_md[ conv_md_id ]);
    vectorr_md_id = conv_md->vectorr_md_id;

    /*initialize*/
    cur_decstr     = (UINT8 *)decstr;
    left_decstrlen = decstrlen;

    if( (2 * (WORDSIZE / 4)) > left_decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_dec_to_vectorr: left_decstrlen %d is too small\n", left_decstrlen);
        return ((UINT32)(-1));
    }

#if (32 == WORDSIZE)
    sscanf((char *)cur_decstr, "%08lx%08lx", &rotated_flag, &num);
#endif/*WORDSIZE*/
#if (64 == WORDSIZE)
    sscanf((char *)cur_decstr, "%016lx%016lx", &rotated_flag, &num);
#endif/*WORDSIZE*/

    len = (2 * (WORDSIZE / 4));
    cur_decstr     += len;
    left_decstrlen -= len;

    vector_r_clean_vector(vectorr_md_id, vector);
    vector_r_new_vector_skeleton(vectorr_md_id, num, vector);

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        conv_dec_to_vectorr_block(conv_md_id, cur_decstr, left_decstrlen, vector_block);
        len = (WORDSIZE / 4)  /*num*/
            + DOUBLE_IN_CHAR * VECTOR_BLOCK_GET_NUM(vector_block);
        cur_decstr     += len;
        left_decstrlen -= len;
    }

    return (0);
}

UINT32 conv_vectorr_to_hex(const UINT32 conv_md_id,const VECTOR * vector, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    VECTOR_BLOCK *vector_block;
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrmaxlen;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == vector )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_vectorr_to_hex: vector is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_vectorr_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_vectorr_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_vectorr_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = hexstr;
    left_hexstrmaxlen = hexstrmaxlen;

    conv_uint32_to_hex(conv_md_id, VECTOR_GET_ROTATED_FLAG(vector), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_uint32_to_hex(conv_md_id, VECTOR_GET_NUM(vector), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        conv_vectorr_block_to_hex(conv_md_id, vector_block, cur_hexstr, left_hexstrmaxlen, &len);
        cur_hexstr        += len;
        left_hexstrmaxlen -= len;
    }
    *hexstrlen = hexstrmaxlen - left_hexstrmaxlen;

   return (0);
}
UINT32 conv_hex_to_vectorr(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,VECTOR *vector)
{
    CONV_MD    *conv_md;
    VECTOR_BLOCK *vector_block;
    UINT8 *cur_hexstr;

    UINT32 num;
    UINT32 rotated_flag;

    UINT32 len;
    UINT32 left_hexstrlen;

    UINT32 vectorr_md_id;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_vectorr: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == vector )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_vectorr: vector is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_vectorr: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
    conv_md = &(g_conv_md[ conv_md_id ]);
    vectorr_md_id = conv_md->vectorr_md_id;

    /*initialize*/
    cur_hexstr     = (UINT8 *)hexstr;
    left_hexstrlen = hexstrlen;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &rotated_flag);
    len = (WORDSIZE / 4 );
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &num);
    len = (WORDSIZE / 4 );
    cur_hexstr     += len;
    left_hexstrlen -= len;

    vector_r_clean_vector(vectorr_md_id, vector);
    vector_r_new_vector_skeleton(vectorr_md_id, num, vector);

    if(1 == rotated_flag)
    {
        vector_r_rotate(vectorr_md_id, vector);
    }

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        conv_hex_to_vectorr_block(conv_md_id, cur_hexstr, left_hexstrlen, vector_block);
        len = (WORDSIZE / 4)  /*num*/
            + (DOUBLESIZE / 4) * VECTOR_BLOCK_GET_NUM(vector_block);
        cur_hexstr     += len;
        left_hexstrlen -= len;
    }

    return (0);
}

UINT32 conv_vectorr_to_bin(const UINT32 conv_md_id,const VECTOR * vector, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    /*trick!!*/
    return conv_vectorr_to_dec(conv_md_id, vector, binstr, binstrmaxlen, binstrlen);
}

UINT32 conv_bin_to_vectorr(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,VECTOR *vector)
{
    /*trick!!*/
    return conv_dec_to_vectorr(conv_md_id, binstr, binstrlen, vector);
}

/*--------------------------------------------MATRIXR BLOCK----------------------------------------------*/
UINT32 conv_matrixr_block_to_dec(const UINT32 conv_md_id,const MATRIX_BLOCK * matrix_block, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT8 *cur_decstr;
    REAL *data_addr;
    REAL zero;

    UINT32 len;
    UINT32 left_decstrmaxlen;

    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

    UINT32 row_idx;
    UINT32 col_idx;


#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == matrix_block )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_block_to_dec: matrix_block is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_block_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_block_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_matrixr_block_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    *decstrlen = 0;
    cur_decstr = decstr;
    left_decstrmaxlen = decstrmaxlen;

    if( (3 * (WORDSIZE / 4)) > left_decstrmaxlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_matrixr_block_to_dec: left_decstrmaxlen %d is too small\n", left_decstrmaxlen);
        return ((UINT32)(-1));
    }

    REAL_SETZERO(0, zero);

    rotated_flag = MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block);
    row_num = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    col_num = MATRIX_BLOCK_GET_COL_NUM(matrix_block);

#if (32 == WORDSIZE)
    sprintf((char *)cur_decstr,"%08lx%08lx%08lx", rotated_flag, row_num, col_num);
#endif/*WORDSIZE*/
#if (64 == WORDSIZE)
    sprintf((char *)cur_decstr,"%016lx%016lx%016lx", rotated_flag, row_num, col_num);
#endif/*WORDSIZE*/

    len = (3 * (WORDSIZE / 4));
    cur_decstr        += len;
    left_decstrmaxlen -= len;
    *decstrlen        += len;

    if( row_num * col_num > left_decstrmaxlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_matrixr_block_to_dec: left_decstrmaxlen %d is too small\n", left_decstrmaxlen);
        return ((UINT32)(-1));
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "beg-------------------------------------------------------------------------------------\n");
    for(row_idx = 0; row_idx < row_num; row_idx ++)
    {
        for(col_idx = 0; col_idx < col_num; col_idx ++)
        {
            //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "row_idx %d col_idx %d------------------------------------------------------------\n", row_idx, col_idx);

            data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, row_idx, col_idx);
            if(NULL_PTR == data_addr)
            {
                data_addr = &zero;
            }
            conv_real_to_dec(conv_md_id, data_addr, cur_decstr, left_decstrmaxlen, &len);
            cur_decstr        += len;
            left_decstrmaxlen -= len;
            *decstrlen        += len;
        }
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "end-------------------------------------------------------------------------------------\n");

    return (0);
}

UINT32 conv_dec_to_matrixr_block(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,MATRIX_BLOCK *matrix_block)
{
    REAL *data_addr;
    UINT8 *cur_decstr;

    UINT32 len;
    UINT32 left_decstrlen;

    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

    UINT32 row_idx;
    UINT32 col_idx;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_matrixr_block: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == matrix_block )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_matrixr_block: matrix_block is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_matrixr_block: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_decstr = (UINT8 *)decstr;
    left_decstrlen = decstrlen;

    if( (3 * (WORDSIZE / 4)) > left_decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_dec_to_matrixr_block: left_decstrlen %d is too small\n", left_decstrlen);
        return ((UINT32)(-1));
    }
#if (32 == WORDSIZE)
    sscanf((char *)cur_decstr, "%08lx%08lx%08lx", &rotated_flag, &row_num, &col_num);
#endif/*WORDSIZE*/
#if (64 == WORDSIZE)
    sscanf((char *)cur_decstr, "%016lx%016lx%016lx", &rotated_flag, &row_num, &col_num);
#endif/*WORDSIZE*/
    len = (3 * (WORDSIZE / 4));
    cur_decstr     += len;
    left_decstrlen -= len;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_dec_to_matrixr_block: rotated_flag %d, row_num %d, col_num %d\n", rotated_flag, row_num, col_num);

    MATRIX_BLOCK_SET_ROTATED_FLAG(matrix_block, rotated_flag);
    MATRIX_BLOCK_SET_ROW_NUM(matrix_block, row_num);
    MATRIX_BLOCK_SET_COL_NUM(matrix_block, col_num);

    if( row_num * col_num * DOUBLE_IN_CHAR > left_decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_dec_to_matrixr_block: left_decstrlen %d is too small\n", left_decstrlen);
        return ((UINT32)(-1));
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "beg#####################################################################################\n");

    for(row_idx = 0; row_idx < row_num; row_idx ++)
    {
        for(col_idx = 0; col_idx < col_num; col_idx ++)
        {
            len = DOUBLE_IN_CHAR;

            alloc_static_mem(MD_CONV, conv_md_id, MM_REAL, &data_addr, LOC_CONV_0060);
            conv_dec_to_real(conv_md_id, cur_decstr, left_decstrlen, data_addr);
            MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, row_idx, col_idx, data_addr);
            //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "row_idx %d col_idx %d, addr %08lx, val %+032.14f\n", row_idx, col_idx, data_addr, *data_addr);
            cur_decstr     += len;
            left_decstrlen -= len;
        }
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "end#####################################################################################\n");
    return (0);
}

UINT32 conv_matrixr_block_to_hex(const UINT32 conv_md_id,const MATRIX_BLOCK * matrix_block, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8 *cur_hexstr;
    REAL *data_addr;
    REAL zero;

    UINT32 len;
    UINT32 left_hexstrmaxlen;

    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

    UINT32 row_idx;
    UINT32 col_idx;


#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == matrix_block )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_block_to_hex: matrix_block is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_block_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_block_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_matrixr_block_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = hexstr;
    left_hexstrmaxlen = hexstrmaxlen;

    REAL_SETZERO(0, zero);

    rotated_flag = MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block);
    row_num = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    col_num = MATRIX_BLOCK_GET_COL_NUM(matrix_block);

    conv_uint32_to_hex(conv_md_id, rotated_flag, cur_hexstr, left_hexstrmaxlen, &len);
    len = (WORDSIZE / 4);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_uint32_to_hex(conv_md_id, row_num, cur_hexstr, left_hexstrmaxlen, &len);
    len = (WORDSIZE / 4);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_uint32_to_hex(conv_md_id, col_num, cur_hexstr, left_hexstrmaxlen, &len);
    len = (WORDSIZE / 4);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    if( row_num * col_num > left_hexstrmaxlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_matrixr_block_to_hex: left_hexstrmaxlen %d is too small\n", left_hexstrmaxlen);
        return ((UINT32)(-1));
    }

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(row_idx, row_num, col_idx, col_num)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, row_idx, col_idx);
        if(NULL_PTR == data_addr)
        {
            data_addr = &zero;
        }
        conv_real_to_hex(conv_md_id, data_addr, cur_hexstr, left_hexstrmaxlen, &len);
        cur_hexstr        += len;
        left_hexstrmaxlen -= len;
    }

    *hexstrlen = hexstrmaxlen - left_hexstrmaxlen;

    return (0);
}
UINT32 conv_hex_to_matrixr_block(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,MATRIX_BLOCK *matrix_block)
{
    REAL *data_addr;
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrlen;

    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

    UINT32 row_idx;
    UINT32 col_idx;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_matrixr_block: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == matrix_block )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_matrixr_block: matrix_block is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_matrixr_block: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = (UINT8 *)hexstr;
    left_hexstrlen = hexstrlen;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &rotated_flag);
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &row_num);
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &col_num);
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    MATRIX_BLOCK_SET_ROTATED_FLAG(matrix_block, rotated_flag);
    MATRIX_BLOCK_SET_ROW_NUM(matrix_block, row_num);
    MATRIX_BLOCK_SET_COL_NUM(matrix_block, col_num);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(row_idx, row_num, col_idx, col_num)
    {
        alloc_static_mem(MD_CONV, conv_md_id, MM_REAL, &data_addr, LOC_CONV_0061);
        conv_hex_to_real(conv_md_id, cur_hexstr, left_hexstrlen, data_addr);
        MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, row_idx, col_idx, data_addr);

        len = (DOUBLESIZE / 4);
        cur_hexstr     += len;
        left_hexstrlen -= len;
    }

    return (0);
}

UINT32 conv_matrixr_block_to_bin(const UINT32 conv_md_id,const MATRIX_BLOCK * matrix_block, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    return conv_matrixr_block_to_dec(conv_md_id, matrix_block, binstr, binstrmaxlen, binstrlen);
}
UINT32 conv_bin_to_matrixr_block(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,MATRIX_BLOCK *matrix_block)
{
    return conv_dec_to_matrixr_block(conv_md_id, binstr, binstrlen, matrix_block);
}

/*--------------------------------------------MATRIXR----------------------------------------------------*/
UINT32 conv_matrixr_to_dec(const UINT32 conv_md_id,const MATRIX * matrix, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    MATRIX_BLOCK  *matrix_row_block;
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT8 *cur_decstr;

    UINT32 len;
    UINT32 left_decstrmaxlen;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == matrix )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_to_dec: matrix is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_matrixr_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    *decstrlen = 0;
    cur_decstr = decstr;
    left_decstrmaxlen = decstrmaxlen;

    if( (3 * (WORDSIZE / 4)) > left_decstrmaxlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_matrixr_to_dec: left_decstrmaxlen %d is too small\n", left_decstrmaxlen);
        return ((UINT32)(-1));
    }

    /*rotated flag and row num and col num of matrix*/
#if (32 == WORDSIZE)
    sprintf((char *)cur_decstr,"%08lx%08lx%08lx",
                    MATRIX_GET_ROTATED_FLAG(matrix),
                    MATRIX_GET_ROW_NUM(matrix),
                    MATRIX_GET_COL_NUM(matrix));
#endif/*WORDSIZE*/
#if (64 == WORDSIZE)
    sprintf((char *)cur_decstr,"%016lx%016lx%016lx",
                    MATRIX_GET_ROTATED_FLAG(matrix),
                    MATRIX_GET_ROW_NUM(matrix),
                    MATRIX_GET_COL_NUM(matrix));
#endif/*WORDSIZE*/
    len = (3 * (WORDSIZE / 4));
    cur_decstr        += len;
    left_decstrmaxlen -= len;
    *decstrlen        += len;


    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_row_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        conv_matrixr_block_to_dec(conv_md_id,matrix_row_block, cur_decstr, left_decstrmaxlen, &len);
        cur_decstr        += len;
        left_decstrmaxlen -= len;
        *decstrlen        += len;
    }

   return (0);
}

UINT32 conv_dec_to_matrixr(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,MATRIX *matrix)
{
    CONV_MD    *conv_md;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_row_block;


    UINT8 *cur_decstr;

    UINT32 len;
    UINT32 left_decstrlen;

    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

    UINT32 matrixr_md_id;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_matrixr: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == matrix )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_matrixr: matrix is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_matrixr: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
    conv_md = &(g_conv_md[ conv_md_id ]);
    matrixr_md_id = conv_md->matrixr_md_id;

    if( (3 * (WORDSIZE / 4)) > decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_dec_to_matrixr: decstrlen %d is too small\n", decstrlen);
        return ((UINT32)(-1));
    }

    cur_decstr = (UINT8 *)decstr;
    left_decstrlen = decstrlen;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_dec_to_matrixr: matrix %lx, decstr: %.*s\n", matrix, left_decstrlen, cur_decstr);
#if (32 == WORDSIZE)
    sscanf((char *)cur_decstr, "%08lx%08lx%08lx", &rotated_flag, &row_num, &col_num);
#endif/*WORDSIZE*/
#if (64 == WORDSIZE)
    sscanf((char *)cur_decstr, "%016lx%016lx%016lx", &rotated_flag, &row_num, &col_num);
#endif/*WORDSIZE*/
    len = (3 * (WORDSIZE / 4));
    cur_decstr     += len;
    left_decstrlen -= len;
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_dec_to_matrixr: rotated_flag %d, row_num %d, col_num %d\n", rotated_flag, row_num, col_num);

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_dec_to_matrixr: matrixr_md_id %d\n", matrixr_md_id);
    if(row_num != MATRIX_GET_ROW_NUM(matrix)
    || col_num != MATRIX_GET_COL_NUM(matrix))
    {
        matrix_r_clean_matrix(matrixr_md_id, matrix);
        matrix_r_new_matrix_skeleton(matrixr_md_id, row_num, col_num, matrix);
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_dec_to_matrixr: matrix_r_new_matrix_skeleton back\n");


    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_row_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_dec_to_matrixr: beg: handle matrix_block %lx in matrix %lx\n", matrix_row_block, matrix);
        conv_dec_to_matrixr_block(conv_md_id, cur_decstr, left_decstrlen, matrix_row_block);
        //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_dec_to_matrixr: end: handle matrix_block %lx in matrix %lx\n", matrix_row_block, matrix);

        len = (3 * (WORDSIZE / 4)) /*rotate flag, row num, col num of matrix block*/
            + DOUBLE_IN_CHAR * MATRIX_BLOCK_GET_ROW_NUM(matrix_row_block) * MATRIX_BLOCK_GET_COL_NUM(matrix_row_block);

        cur_decstr     += len;
        left_decstrlen -= len;
    }

    return (0);
}

UINT32 conv_matrixr_to_hex(const UINT32 conv_md_id,const MATRIX * matrix, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_row_block;
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrmaxlen;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == matrix )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_to_hex: matrix is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_matrixr_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_matrixr_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = hexstr;
    left_hexstrmaxlen = hexstrmaxlen;

    /*rotated flag and row num and col num of matrix*/
    conv_uint32_to_hex(conv_md_id, MATRIX_GET_ROTATED_FLAG(matrix), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_uint32_to_hex(conv_md_id, MATRIX_GET_ROW_NUM(matrix), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_uint32_to_hex(conv_md_id, MATRIX_GET_COL_NUM(matrix), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_row_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        conv_matrixr_block_to_hex(conv_md_id,matrix_row_block, cur_hexstr, left_hexstrmaxlen, &len);
        cur_hexstr        += len;
        left_hexstrmaxlen -= len;
    }

    *hexstrlen = hexstrmaxlen - left_hexstrmaxlen;

   return (0);
}

UINT32 conv_hex_to_matrixr(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,MATRIX *matrix)
{
    CONV_MD    *conv_md;
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_row_block;


    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrlen;

    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

    UINT32 matrixr_md_id;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_matrixr: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == matrix )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_matrixr: matrix is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_matrixr: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
    conv_md = &(g_conv_md[ conv_md_id ]);
    matrixr_md_id = conv_md->matrixr_md_id;

    cur_hexstr = (UINT8 *)hexstr;
    left_hexstrlen = hexstrlen;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &rotated_flag);
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &row_num);
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &col_num);
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_matrixr: matrixr_md_id %d\n", matrixr_md_id);
    if(row_num != MATRIX_GET_ROW_NUM(matrix)
    || col_num != MATRIX_GET_COL_NUM(matrix))
    {
        matrix_r_clean_matrix(matrixr_md_id, matrix);
        matrix_r_new_matrix_skeleton(matrixr_md_id, row_num, col_num, matrix);
    }
    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_matrixr: matrix_r_new_matrix_skeleton back\n");

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_row_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_matrixr: beg: handle matrix_block %lx in matrix %lx\n", matrix_row_block, matrix);
        conv_hex_to_matrixr_block(conv_md_id, cur_hexstr, left_hexstrlen, matrix_row_block);
        //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_matrixr: end: handle matrix_block %lx in matrix %lx\n", matrix_row_block, matrix);

        len = (3 * (WORDSIZE / 4)) /*rotate flag, row num, col num of matrix block*/
            + (DOUBLESIZE / 4) * MATRIX_BLOCK_GET_ROW_NUM(matrix_row_block) * MATRIX_BLOCK_GET_COL_NUM(matrix_row_block);

        cur_hexstr     += len;
        left_hexstrlen -= len;
    }

    return (0);
}

UINT32 conv_matrixr_to_bin(const UINT32 conv_md_id,const MATRIX * matrix, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
     /*trick*/
    return conv_matrixr_to_dec(conv_md_id, matrix, binstr, binstrmaxlen, binstrlen);
}
UINT32 conv_bin_to_matrixr(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,MATRIX *matrix)
{
     /*trick*/
    return conv_dec_to_matrixr(conv_md_id, binstr, binstrlen, matrix);
}

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
UINT32 conv_dec_to_uint16(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,UINT16 *des)
{
    UINT32 tmp;
    UINT32 max;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_uint16: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_uint16: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_uint16: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    max = ((~((UINT32)0)) >> (WORDSIZE/2));

    ret = conv_dec_to_uint32(conv_md_id, decstr, decstrlen, &tmp);
    if( 0 == ret )
    {
    if( tmp > max )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_uint16: %ld overflow of UINT16\n", tmp);
        ret = ((UINT32)(-3));
    }
    else
    {
        *des = tmp;
    }
    }

    return ret;
}

/**
*
*   convert a UINT16 to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the UINT16 and return 0.
*
**/
UINT32 conv_uint16_to_dec(const UINT32 conv_md_id, const UINT16 src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint16_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint16_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_uint16_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_uint32_to_dec(conv_md_id, src, decstr, decstrmaxlen, decstrlen);

    return ret;
}

UINT32 conv_uint16_to_dec_0(const UINT32 conv_md_id, const UINT16 *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    return conv_uint16_to_dec(conv_md_id, *src, decstr, decstrmaxlen, decstrlen);
}


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
UINT32 conv_hex_to_uint16(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,UINT16 *des)
{
    UINT32 tmp;
    UINT32 max;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_uint16: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_uint16: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_uint16: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    max = ((~((UINT32)0)) >> (WORDSIZE/2));

    ret = conv_hex_to_uint32(conv_md_id, hexstr, hexstrlen, &tmp);
    if( 0 == ret )
    {
    if( tmp > max )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_uint16: %ld overflow of UINT16\n", tmp);
        ret = ((UINT32)(-3));
    }
    else
    {
        *des = tmp;
    }
    }

    return ret;
}

/**
*
*   convert a UINT16 to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the UINT16 and return 0.
*
**/
UINT32 conv_uint16_to_hex(const UINT32 conv_md_id,const UINT16 src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT32 tmp;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint16_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint16_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_uint16_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    tmp = src;
    ret = conv_uint32_to_hex(conv_md_id, tmp, hexstr, hexstrmaxlen, hexstrlen);

    return ret;
}

UINT32 conv_uint16_to_hex_0(const UINT32 conv_md_id,const UINT16 *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    return conv_uint16_to_hex(conv_md_id, *src, hexstr, hexstrmaxlen, hexstrlen);
}

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
UINT32 conv_bin_to_uint16(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,UINT16 *des)
{
    UINT32 tmp;
    UINT32 max;

    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_uint16: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_uint16: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_uint16: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    max = ((~((UINT32)0)) >> (WORDSIZE/2));

    ret = conv_bin_to_uint32(conv_md_id, binstr, binstrlen, &tmp);
    if( 0 == ret )
    {
    if( tmp > max )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_uint16: %ld overflow of UINT16\n", tmp);
        ret = ((UINT32)(-3));
    }
    else
    {
        *des = tmp;
    }
    }

    return ret;
}

/**
*
*   convert a UINT16 to a bin string.
*   the destinate bin string should be given a buffer with a maximum lenght at first.
*
*   if the bin string buffer is not enough, then warn and return -1.
*   otherwise, return the bin expression of the UINT16 and return 0.
*
**/
UINT32 conv_uint16_to_bin(const UINT32 conv_md_id,const UINT16 src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT32 tmp;
    UINT32 ret;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint16_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_uint16_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_uint16_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    tmp = src;

    ret = conv_uint32_to_bin(conv_md_id, tmp, binstr, binstrmaxlen, binstrlen);

    return ret;
}

UINT32 conv_uint16_to_bin_0(const UINT32 conv_md_id,const UINT16 *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    return conv_uint16_to_bin(conv_md_id,*src, binstr,binstrmaxlen, binstrlen);
}

/*--------------------------------------------EBGN----------------------------------------------------*/
/**
*
*   convert a decimal string to a ebgn.
*   return des is the ebgn value of the decimal string and return 0.
*
*   note:
*    1. the first char may be the sgn of integer. '+' means positive integer, '-' means negative integer.
*       if the first char belong to [0-9] which means it is positive integer or zero.
*       2. if the decimal part string contains any exceptional character,i.e., not belong to [0-9], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_decstr function.
*
**/
UINT32 conv_dec_to_ebgn(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen,EBGN *des)
{
    UINT32 ret;
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ebgn: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_dec_to_ebgn: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_dec_to_ebgn: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( 0 < decstrlen && EC_FALSE == conv_check_decstr(conv_md_id, decstr + 1, decstrlen - 1) )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_dec_to_ebgn: invalid decstr\n");
        return ((UINT32)(-1));
    }

    /* initialize des ebgn */
    EBGN_INIT( des );

    ret = conv_str_to_ebgn(conv_md_id, decstr, decstrlen, 10, conv_decchar_to_num, des);
    return ret;
}

/**
*
*   convert a ebgn to a decimal string.
*   the destinate decimal string should be given a buffer with a maximum lenght at first.
*
*   if the decimal string buffer is not enough, then warn and return -1.
*   otherwise, return the decimal expression of the ebgn and return 0.
*
**/
UINT32 conv_ebgn_to_dec(const UINT32 conv_md_id,const EBGN *src, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    UINT32 ret;
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_dec: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_dec: decstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == decstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_dec: decstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ebgn_to_dec: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_ebgn_to_str(conv_md_id, src, 10, conv_num_to_decchar, decstr, decstrmaxlen, decstrlen);

    return ret;
}


/**
*
*   convert a hex string to a ebgn.
*   return des is the ebgn value of the hex string and return 0.
*
*   note:
*    1. the first char may be the sgn of integer. '+' means positive integer, '-' means negative integer.
*       if the first char belong to [0-9] which means it is positive integer or zero.
*       2. if the hex part string contains any exceptional character,i.e., not belong to [0-9a-f], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_hexstr function.
*
**/
UINT32 conv_hex_to_ebgn(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen,EBGN *des)
{
    UINT32 ret;
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ebgn: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_ebgn: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_ebgn: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( 0 < hexstrlen && EC_FALSE == conv_check_hexstr(conv_md_id, hexstr + 1, hexstrlen - 1) )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_hex_to_ebgn: invalid hexstr\n");
        return ((UINT32)(-1));
    }

    /* initialize des ebgn */
    EBGN_INIT( des );

    ret = conv_str_to_ebgn(conv_md_id, hexstr, hexstrlen, 16, conv_hexchar_to_num, des);
    return ret;
}

/**
*
*   convert a ebgn to a hex string.
*   the destinate hex string should be given a buffer with a maximum lenght at first.
*
*   if the hex string buffer is not enough, then warn and return -1.
*   otherwise, return the hex expression of the ebgn and return 0.
*
**/
UINT32 conv_ebgn_to_hex(const UINT32 conv_md_id,const EBGN *src, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT32 ret;
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_hex: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ebgn_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_ebgn_to_str(conv_md_id, src, 16, conv_num_to_hexchar, hexstr, hexstrmaxlen, hexstrlen);

    return ret;
}

/**
*
*   convert a binary string to a ebgn.
*   return des is the ebgn value of the binary string and return 0.
*
*   note:
*    1. the first char may be the sgn of integer. '+' means positive integer, '-' means negative integer.
*       if the first char belong to [0-9] which means it is positive integer or zero.
*       2. if the binary part string contains any exceptional character,i.e., not belong to [0-1], then
*   return -1 without warning information. Indeed, the waring information is reported in
*   conv_check_binstr function.
*
**/
UINT32 conv_bin_to_ebgn(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen,EBGN *des)
{
    UINT32 ret;
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ebgn: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_bin_to_ebgn: des is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_bin_to_ebgn: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( 0 < binstrlen && EC_FALSE == conv_check_binstr(conv_md_id, binstr + 1, binstrlen - 1) )
    {
    dbg_log(SEC_0110_CONV, 0)(LOGSTDERR,"error:conv_bin_to_ebgn: invalid binstr\n");
        return ((UINT32)(-1));
    }

    /* initialize des ebgn */
    EBGN_INIT( des );

    ret = conv_str_to_ebgn(conv_md_id, binstr, binstrlen, 2, conv_binchar_to_num, des);
    return ret;
}

/**
*
*   convert a ebgn to a binary string.
*   the destinate binary string should be given a buffer with a maximum lenght at first.
*
*   if the binary string buffer is not enough, then warn and return -1.
*   otherwise, return the binary expression of the ebgn and return 0.
*
**/
UINT32 conv_ebgn_to_bin(const UINT32 conv_md_id,const EBGN *src, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    UINT32 ret;
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_bin: src is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_bin: binstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == binstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_to_bin: binstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }

#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ebgn_to_bin: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ret = conv_ebgn_to_str(conv_md_id, src, 2, conv_num_to_binchar, binstr, binstrmaxlen, binstrlen);

    return ret;
}

/*--------------------------------------------MOD MGR----------------------------------------------*/
UINT32 conv_mod_node_to_dec(const UINT32 conv_md_id,const MOD_NODE * mod_node, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    return conv_mod_node_to_hex(conv_md_id, mod_node, decstr, decstrmaxlen, decstrlen);
}

UINT32 conv_dec_to_mod_node(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen, MOD_NODE *mod_node)
{
    return conv_hex_to_mod_node(conv_md_id, decstr, decstrlen, mod_node);
}

#if 0
UINT32 conv_mod_node_to_hex(const UINT32 conv_md_id,const MOD_NODE * mod_node, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrmaxlen;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == mod_node )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_node_to_hex: mod_node is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_node_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_node_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_mod_node_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    cur_hexstr = hexstr;
    left_hexstrmaxlen = hexstrmaxlen;

    conv_uint32_to_hex(conv_md_id, mod_node->comm, cur_hexstr, left_hexstrmaxlen, &len);
    len = (WORDSIZE / 4);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_uint32_to_hex(conv_md_id, mod_node->rank, cur_hexstr, left_hexstrmaxlen, &len);
    len = (WORDSIZE / 4);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_uint32_to_hex(conv_md_id, mod_node->modi, cur_hexstr, left_hexstrmaxlen, &len);
    len = (WORDSIZE / 4);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    *hexstrlen = hexstrmaxlen - left_hexstrmaxlen;

    return (0);
}
UINT32 conv_hex_to_mod_node(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen, MOD_NODE *mod_node)
{
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrlen;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_mod_node: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == mod_node )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_mod_node: mod_node is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_mod_node: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = hexstr;
    left_hexstrlen = hexstrlen;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &(mod_node->comm));
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &(mod_node->rank));
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &(mod_node->modi));
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    return (0);
}

#endif

#if 1/*compression transmission mode*/
UINT32 conv_mod_node_to_hex(const UINT32 conv_md_id,const MOD_NODE * mod_node, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrmaxlen;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == mod_node )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_node_to_hex: mod_node is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_node_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_node_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_mod_node_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    cur_hexstr = hexstr;
    left_hexstrmaxlen = hexstrmaxlen;

    conv_uint32_to_hex(conv_md_id, mod_node->rank, cur_hexstr, left_hexstrmaxlen, &len);
    len = (WORDSIZE / 4);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_uint32_to_hex(conv_md_id, mod_node->modi, cur_hexstr, left_hexstrmaxlen, &len);
    len = (WORDSIZE / 4);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    *hexstrlen = hexstrmaxlen - left_hexstrmaxlen;

    return (0);
}

UINT32 conv_hex_to_mod_node(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen, MOD_NODE *mod_node)
{
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrlen;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_mod_node: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == mod_node )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_mod_node: mod_node is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_mod_node: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = (UINT8 *)hexstr;
    left_hexstrlen = hexstrlen;

    mod_node->comm = CMPI_COMM_WORLD;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &(mod_node->rank));
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &(mod_node->modi));
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    return (0);
}
#endif

UINT32 conv_mod_node_to_bin(const UINT32 conv_md_id,const MOD_NODE * mod_node, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    return conv_mod_node_to_hex(conv_md_id, mod_node, binstr, binstrmaxlen, binstrlen);
}

UINT32 conv_bin_to_mod_node(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen, MOD_NODE *mod_node)
{
    return conv_bin_to_mod_node(conv_md_id, binstr, binstrlen, mod_node);
}

/*--------------------------------------------MOD MGR----------------------------------------------*/
UINT32 conv_mod_mgr_to_dec(const UINT32 conv_md_id,const MOD_MGR * mod_mgr, UINT8 *decstr,const UINT32 decstrmaxlen, UINT32 *decstrlen)
{
    return conv_mod_mgr_to_hex(conv_md_id, mod_mgr, decstr, decstrmaxlen, decstrlen);
}

UINT32 conv_dec_to_mod_mgr(const UINT32 conv_md_id,const UINT8 *decstr,const UINT32 decstrlen, MOD_MGR *mod_mgr)
{
    return conv_hex_to_mod_mgr(conv_md_id, decstr, decstrlen, mod_mgr);
}
#if 0
UINT32 conv_mod_mgr_to_hex(const UINT32 conv_md_id,const MOD_MGR * mod_mgr, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrmaxlen;

    UINT32 mod_idx;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == mod_mgr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_mgr_to_hex: mod_mgr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_mgr_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_mgr_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_mod_mgr_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = hexstr;
    left_hexstrmaxlen = hexstrmaxlen;

    conv_uint32_to_hex(conv_md_id, MOD_MGR_LOAD_BALANCING(mod_mgr), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_mod_mgr_to_hex: load balancing: %d -> %.*s\n", MOD_MGR_LOAD_BALANCING(mod_mgr), len, cur_hexstr - len);

    conv_mod_node_to_hex(conv_md_id, MOD_MGR_LOCAL_MOD(mod_mgr), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;
#if 0
    dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_mod_mgr_to_hex: local mod: comm %ld, rank %ld, modi %ld -> %.*s\n",
                    MOD_NODE_COMM(MOD_MGR_LOCAL_MOD(mod_mgr)),
                    MOD_NODE_RANK(MOD_MGR_LOCAL_MOD(mod_mgr)),
                    MOD_NODE_MODI(MOD_MGR_LOCAL_MOD(mod_mgr)),
                    len, cur_hexstr - len);
#endif
    conv_uint32_to_hex(conv_md_id, MOD_MGR_REMOTE_NUM(mod_mgr), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_mod_mgr_to_hex: remote mod num: %d -> %.*s\n", MOD_MGR_REMOTE_NUM(mod_mgr), len, cur_hexstr - len);

    for(mod_idx = 0; mod_idx < MOD_MGR_REMOTE_NUM(mod_mgr); mod_idx ++)
    {
        conv_mod_node_to_hex(conv_md_id, MOD_MGR_REMOTE_MOD(mod_mgr, mod_idx), cur_hexstr, left_hexstrmaxlen, &len);
        cur_hexstr        += len;
        left_hexstrmaxlen -= len;

#if 0
        dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_mod_mgr_to_hex: remote mod: No %ld: comm %ld, rank %ld, modi %ld -> %.*s\n",
                    mod_idx,
                    MOD_NODE_COMM(MOD_MGR_REMOTE_MOD(mod_mgr, mod_idx)),
                    MOD_NODE_RANK(MOD_MGR_REMOTE_MOD(mod_mgr, mod_idx)),
                    MOD_NODE_MODI(MOD_MGR_REMOTE_MOD(mod_mgr, mod_idx)),
                    len, cur_hexstr - len);
#endif
    }

    *hexstrlen = hexstrmaxlen - left_hexstrmaxlen;

   return (0);
}

UINT32 conv_hex_to_mod_mgr(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen, MOD_MGR *mod_mgr)
{
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrlen;

    UINT32 mm_size;
    UINT32 mod_idx;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_mod_mgr: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == mod_mgr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_mod_mgr: mod_mgr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_mod_mgr: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = hexstr;
    left_hexstrlen = hexstrlen;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_mod_mgr: mod_mgr %lx:\n", mod_mgr);

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &(MOD_MGR_LOAD_BALANCING(mod_mgr)));
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_mod_mgr: load balancing: %.*s -> %d\n", len, cur_hexstr - len, MOD_MGR_LOAD_BALANCING(mod_mgr));

    conv_hex_to_mod_node(conv_md_id, cur_hexstr, left_hexstrlen, MOD_MGR_LOCAL_MOD(mod_mgr));
    len = (WORDSIZE / 4) * 3;
    cur_hexstr     += len;
    left_hexstrlen -= len;
#if 0
    dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_mod_mgr: local mod: %.*s -> comm %ld, rank %ld, modi %ld\n",
                    len, cur_hexstr - len,
                    MOD_NODE_COMM(MOD_MGR_LOCAL_MOD(mod_mgr)),
                    MOD_NODE_RANK(MOD_MGR_LOCAL_MOD(mod_mgr)),
                    MOD_NODE_MODI(MOD_MGR_LOCAL_MOD(mod_mgr)));
#endif
    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &(MOD_MGR_REMOTE_NUM(mod_mgr)));
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    //dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_mod_mgr: remote mod num: %.*s -> %d\n", len, cur_hexstr - len, MOD_MGR_REMOTE_NUM(mod_mgr));

    mm_size = MOD_MGR_REMOTE_NUM(mod_mgr) * sizeof(MOD_NODE);
 #if 1
    if( fetch_static_mem_typesize(MM_STRCHAR2M) >= mm_size )
    {
        alloc_static_mem(MD_TASK, 0, MM_STRCHAR2M, &(MOD_MGR_REMOTE(mod_mgr)), LOC_CONV_0062);
        MOD_MGR_MM_TYPE(mod_mgr) = MM_STRCHAR2M;
        MOD_MGR_REMOTE_MAX(mod_mgr) = fetch_static_mem_typesize(MM_STRCHAR2M) / sizeof(MOD_NODE);
    }
    else
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT, "error:conv_hex_to_mod_mgr: not support so large task mod table\n");
        return ((UINT32)(-1));
    }
#else
    MOD_MGR_REMOTE(mod_mgr) = malloc(mm_size);
    if(NULL_PTR == MOD_MGR_REMOTE(mod_mgr))
    {
         dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_mod_mgr: failed to alloc memory for MOD_MGR_REMOTE(%lx).\n", mod_mgr);
         return ((UINT32)(-1));
    }
    MOD_MGR_REMOTE_MAX(mod_mgr) = MOD_MGR_REMOTE_NUM(mod_mgr);
#endif

    MOD_MGR_REMOTE_POS(mod_mgr) = 0;

    for(mod_idx = 0; mod_idx < MOD_MGR_REMOTE_NUM(mod_mgr); mod_idx ++)
    {
        conv_hex_to_mod_node(conv_md_id, cur_hexstr, left_hexstrlen, MOD_MGR_REMOTE_MOD(mod_mgr, mod_idx));

        len = (WORDSIZE / 4) * 3;
        cur_hexstr     += len;
        left_hexstrlen -= len;

#if 0
        dbg_log(SEC_0110_CONV, 5)(LOGSTDOUT, "conv_hex_to_mod_mgr: remote mod: No %ld: %.*s -> comm %ld, rank %ld, modi %ld\n",
                    mod_idx, len, cur_hexstr - len,
                    MOD_NODE_COMM(MOD_MGR_REMOTE_MOD(mod_mgr, mod_idx)),
                    MOD_NODE_RANK(MOD_MGR_REMOTE_MOD(mod_mgr, mod_idx)),
                    MOD_NODE_MODI(MOD_MGR_REMOTE_MOD(mod_mgr, mod_idx)));
#endif
    }

    return (0);
}
#endif

#if 1/*compress transimission mode*/
UINT32 conv_mod_mgr_to_hex(const UINT32 conv_md_id,const MOD_MGR * mod_mgr, UINT8 *hexstr,const UINT32 hexstrmaxlen, UINT32 *hexstrlen)
{
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrmaxlen;

    UINT32 pos;
    MOD_NODE *mod_node;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == mod_mgr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_mgr_to_hex: mod_mgr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_mgr_to_hex: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == hexstrlen )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_mod_mgr_to_hex: hexstrlen is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_mod_mgr_to_hex: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = hexstr;
    left_hexstrmaxlen = hexstrmaxlen;

    conv_uint32_to_hex(conv_md_id, MOD_MGR_LDB_CHOICE(mod_mgr), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_mod_node_to_hex(conv_md_id, MOD_MGR_LOCAL_MOD(mod_mgr), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    conv_uint32_to_hex(conv_md_id, MOD_MGR_REMOTE_NUM(mod_mgr), cur_hexstr, left_hexstrmaxlen, &len);
    cur_hexstr        += len;
    left_hexstrmaxlen -= len;

    for(pos = 0; pos < cvector_size(MOD_MGR_REMOTE_LIST(mod_mgr)); pos ++)
    {
        mod_node = (MOD_NODE *)cvector_get(MOD_MGR_REMOTE_LIST(mod_mgr), pos);
        conv_mod_node_to_hex(conv_md_id, mod_node, cur_hexstr, left_hexstrmaxlen, &len);
        cur_hexstr        += len;
        left_hexstrmaxlen -= len;
    }

    *hexstrlen = hexstrmaxlen - left_hexstrmaxlen;

   return (0);
}

UINT32 conv_hex_to_mod_mgr(const UINT32 conv_md_id,const UINT8 *hexstr,const UINT32 hexstrlen, MOD_MGR *mod_mgr)
{
    UINT8 *cur_hexstr;

    UINT32 len;
    UINT32 left_hexstrlen;

    CVECTOR *remote_mod_node_list;
    MOD_NODE *remote_mod_node;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    MOD_MGR_LDB *mod_mgr_ldb;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == hexstr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_mod_mgr: hexstr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
    if ( NULL_PTR == mod_mgr )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_hex_to_mod_mgr: mod_mgr is null.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif /* CONV_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_hex_to_mod_mgr: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*initialize*/
    cur_hexstr = (UINT8 *)hexstr;
    left_hexstrlen = hexstrlen;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &(MOD_MGR_LDB_CHOICE(mod_mgr)));
    mod_mgr_ldb = mod_mgr_ldb_strategy(MOD_MGR_LDB_CHOICE(mod_mgr)); /*initialize the function pointer*/
    MOD_MGR_LDB_FUNCPTR(mod_mgr) = mod_mgr_ldb->get;
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_mod_node(conv_md_id, cur_hexstr, left_hexstrlen, MOD_MGR_LOCAL_MOD(mod_mgr));
    len = (WORDSIZE / 4) * 2;
    cur_hexstr     += len;
    left_hexstrlen -= len;

    conv_hex_to_uint32(conv_md_id, cur_hexstr, left_hexstrlen, &remote_mod_node_num);
    len = (WORDSIZE / 4);
    cur_hexstr     += len;
    left_hexstrlen -= len;

    remote_mod_node_list = MOD_MGR_REMOTE_LIST(mod_mgr);
    cvector_init(remote_mod_node_list, remote_mod_node_num, MM_MOD_NODE, CVECTOR_LOCK_ENABLE, LOC_CONV_0063);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        mod_node_alloc(&remote_mod_node);
        conv_hex_to_mod_node(conv_md_id, cur_hexstr, left_hexstrlen, remote_mod_node);
        cvector_push(remote_mod_node_list, remote_mod_node);

        len = (WORDSIZE / 4) * 2;
        cur_hexstr     += len;
        left_hexstrlen -= len;
    }

    return (0);
}
#endif

UINT32 conv_mod_mgr_to_bin(const UINT32 conv_md_id,const MOD_MGR * mod_mgr, UINT8 *binstr,const UINT32 binstrmaxlen, UINT32 *binstrlen)
{
    return conv_mod_mgr_to_hex(conv_md_id, mod_mgr, binstr, binstrmaxlen, binstrlen);
}

UINT32 conv_bin_to_mod_mgr(const UINT32 conv_md_id,const UINT8 *binstr,const UINT32 binstrlen, MOD_MGR *mod_mgr)
{
    return conv_hex_to_mod_mgr(conv_md_id, binstr, binstrlen, mod_mgr);
}


/*-------------------------------------------- CONV FREE ----------------------------------------------------*/
/**
*
*   destory the whole item,including its content bgn and itself.
*   so, when return from this function, item cannot be refered any more
*
**/
static UINT32 conv_ebgn_item_destory(const UINT32 conv_md_id, EBGN_ITEM *item)
{
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == item )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_item_destory: item is NULL_PTR.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*CONV_DEBUG_SWITCH*/
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ebgn_item_destory: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*CONV_DEBUG_SWITCH*/

    free_static_mem(MD_CONV, conv_md_id, MM_BIGINT, EBGN_ITEM_BGN(item), LOC_CONV_0064);
    EBGN_ITEM_BGN(item) = NULL_PTR;

    free_static_mem(MD_CONV, conv_md_id, MM_EBGN_ITEM, item, LOC_CONV_0065);

    return ( 0 );
}

/**
*
*   destory the whole ebgn,i.e., all its items but not the ebgn itself.
*   so, when return from this function, ebgn can be refered again without any item.
*
**/
UINT32 conv_ebgn_clean(const UINT32 conv_md_id, EBGN *ebgn)
{
    EBGN_ITEM *item;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_clean: ebgn is NULL_PTR.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*CONV_DEBUG_SWITCH*/
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ebgn_clean: conv module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*CONV_DEBUG_SWITCH*/

    item = EBGN_FIRST_ITEM(ebgn);

    while ( item != EBGN_NULL_ITEM(ebgn) )
    {
        EBGN_ITEM_DEL(item);
        conv_ebgn_item_destory(conv_md_id, item);

        item = EBGN_FIRST_ITEM(ebgn);
    }

    EBGN_INIT(ebgn);

    return ( 0 );
}

/**
*
*   the whole ebgn,including its all items and itself.
*   so, when return from this function, ebgn cannot be refered any more
*
**/
UINT32 conv_ebgn_destroy(const UINT32 conv_md_id, EBGN *ebgn)
{
    CONV_MD *conv_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0110_CONV, 0)(LOGSTDOUT,"error:conv_ebgn_destroy: ebgn is NULL_PTR.\n");
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*CONV_DEBUG_SWITCH*/
#if ( SWITCH_ON == CONV_DEBUG_SWITCH )
    if ( MAX_NUM_OF_CONV_MD <= conv_md_id || 0 == g_conv_md[ conv_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:conv_ebgn_destroy: ebgnz module #0x%lx not started.\n",
                conv_md_id);
        dbg_exit(MD_CONV, conv_md_id);
    }
#endif/*CONV_DEBUG_SWITCH*/

    conv_md = &(g_conv_md[ conv_md_id ]);
    ebgnz_md_id  = conv_md->ebgnz_md_id;

    ebgn_z_clean(ebgnz_md_id, ebgn);

    free_static_mem(MD_CONV, conv_md_id, MM_EBGN, ebgn, LOC_CONV_0066);

    return ( 0 );
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

