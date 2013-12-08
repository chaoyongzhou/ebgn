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
#include <stdarg.h>
#include <pthread.h>

#include "type.h"

#include "log.h"
#include "bgnctrl.h"

#include "ecfp.h"
#include "ecf2n.h"
#include "eccfp.h"
#include "eccf2n.h"
#include "bgnzn.h"
#include "bgnz2.h"
#include "bgnz.h"
#include "ebgnz.h"
#include "bgnfp.h"
#include "bgnf2n.h"

#include "conv.h"

#include "poly.h"

#include "polyzn.h"
#include "polyfp.h"
#include "polyf2n.h"
#include "polyz2.h"
#include "seafp.h"

#include "mm.h"

#include "print.h"

#include "debug.h"

#include "cthread.h"
#include "creg.h"
#include "coroutine.h"

static EC_BOOL g_dbg_func_addr_list_init_flag = EC_FALSE;

//static FUNC_ADDR_BLOCK g_dbg_func_addr_block;


#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
static MYSQL g_dbg_mysql_srv;
static MYSQL_ACCESS g_dbg_mysql_acc;

FUNC_ADDR_NODE g_dbg_entry_func_addr_node = {
/*func priority   */    PRIO_BEG,
/*func logic addr */    0,              /*filled with input parameter dbg_start*/
/*func beg addr   */    0,              /*filled with input parameter dbg_start*/
/*func end addr   */    0,              /*N/A*/
/*func addr offset*/    0,              /*N/A*/
/*func name       */    NULL_PTR,       /*filled with input parameter dbg_start*/
/*func ret type   */    e_dbg_type_end, /*ignore*/
/*func para num   */    0,              /*ignore*/
/*func para direct*/    {E_DIRECT_END},   /*ignore*/
/*func para type  */    {e_dbg_type_end}, /*ignore*/
};

#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
UINT32 dbg_start(const MYSQL_ACCESS *mysql_acc, const UINT32 entry_func_logic_addr, const UINT8 *entry_func_name)
{
    MYSQL *mysql_srv;
    //FUNC_ADDR_BLOCK *func_addr_block;
    FUNC_ADDR_NODE *entry_func_addr_node;
    UINT32 index;

    mysql_srv = &g_dbg_mysql_srv;
    entry_func_addr_node = &g_dbg_entry_func_addr_node;
    if ( EC_TRUE == g_dbg_func_addr_list_init_flag )
    {
        if ( entry_func_logic_addr != entry_func_addr_node->func_logic_addr)
         {
            sys_log(LOGSTDOUT,
                    "error:dbg_start: dbg has been started, hence unable to start again with different entry.\n");
            return (~((UINT32)0));
        }
        return 0;
    }

    /*clean up the func addr block*/
    //func_addr_block = &g_dbg_func_addr_block;
    //for( index = 0; index < MAX_NUM_OF_FUNC_FOR_DBG; index ++)
    //{
    //    func_addr_block->func_addr_node[ index ] = NULL_PTR;
    //}
    //func_addr_block->func_num = 0;

    memcpy( &(g_dbg_mysql_acc), mysql_acc, sizeof(MYSQL_ACCESS) );

    /*inser the entry func as the first func addr node in block*/
    /* the system will roll back to this entry func after exception occurs */
    entry_func_addr_node->func_logic_addr = entry_func_logic_addr;
    entry_func_addr_node->func_beg_addr = entry_func_logic_addr;
    entry_func_addr_node->func_end_addr = 0;
    entry_func_addr_node->func_addr_offset = 0;
    strcpy(entry_func_addr_node->func_name, entry_func_name);
    /*entry_func_addr_node->func_name = entry_func_name;*/
    //func_addr_block->func_addr_node[ func_addr_block->func_num ] = entry_func_addr_node;
    //func_addr_block->func_num ++;

    g_dbg_func_addr_list_init_flag = EC_TRUE;

    return  0;
}
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/


/**
*
*     query the type_conv_item of specific type
*
*
**/
TYPE_CONV_ITEM * dbg_query_type_conv_item_by_type(const UINT32 type)
{
    return creg_type_conv_vec_get(creg_type_conv_vec_fetch(), type);
}

/**
*
*     query the type_conv_item of specific mm type
*
*
**/
TYPE_CONV_ITEM * dbg_query_type_conv_item_by_mm(const UINT32 var_mm_type)
{
    CVECTOR *type_conv_vec;
    UINT32 var_dbg_type;

    type_conv_vec = creg_type_conv_vec_fetch();
    if(NULL_PTR == type_conv_vec)
    {
        sys_log(LOGSTDOUT, "error:dbg_query_type_conv_item_by_mm: fetch type conv vec failed\n");
        return (NULL_PTR);
    }
    
    for(var_dbg_type = 0; var_dbg_type < cvector_size(type_conv_vec); var_dbg_type ++)
    {
        TYPE_CONV_ITEM *type_conv_item;;

        type_conv_item = creg_type_conv_vec_get(type_conv_vec, var_dbg_type);
        if(NULL_PTR == type_conv_item)
        {
            continue;
        }
        if(var_mm_type == TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item))
        {
            return (type_conv_item);
        }
    }
    return (NULL_PTR);
}


/**
*
* dbg tiny caller forge a calling scenario who push paras into stack and push out after come back
*
*
**/
#if (32 == WORDSIZE)
UINT32 dbg_tiny_caller(const UINT32 func_para_num, const UINT32 func_addr, ...)
{
    UINT32 func_para_value[ MAX_NUM_OF_FUNC_PARAS ];
    UINT32 func_ret_value;
    UINT32 esp_offset;
    UINT32 index;

    va_list ap;

    if(0 == func_addr)
    {
        return (0);
    }

    va_start(ap, func_addr);
    for( index = 0; index < func_para_num; index ++ )
    {
        func_para_value[ index ] = va_arg(ap, UINT32);
    }
    va_end(ap);

    /*call the function and restore the stack after its return*/
    /*the return value should be returned by EAX register*/
    esp_offset = (func_para_num) * (WORDSIZE/BYTESIZE);

    //sys_print(LOGSTDOUT,"task_tiny_caller: before calling...\n");
    /*if one PUSH operation occurs in the loop and out of the asm scope, then corrupt!*/
    /*push the parameters of the function from right to left one by one*/
    /*for example, if function is defined as void add(int a, int b,int *c), then do*/
    /* push c */
    /* push b*/
    /* push a*/
    for( index = func_para_num; index > 0; )
    {
        index --;
        __asm__ __volatile__
        (
                "pushl %0;"
        :
        :"im"(func_para_value[ index ])
        :"memory"
        );
    }

    /* IMPORTANT: here all values are transferred to assembler via memory style */
    /* due to the observation of anti-assembler shows the registers are not organized very well, */
    /* and some time it is even far wrong! */
       __asm__ __volatile__
       (
            "call %1;"
            "movl %%eax, %0;"
            "addl %2, %%esp;"
            :"=m"(func_ret_value)
            :"im"(func_addr),"im"(esp_offset)
    :"memory"
       );

    //sys_print(LOGSTDOUT,"dbg_tiny_caller: after calling, func_ret_value = %lx\n", func_ret_value);

    return ( func_ret_value );
}
#endif/*(32 == WORDSIZE)*/

#if (64 == WORDSIZE)
UINT32 dbg_tiny_caller(const UINT32 func_para_num, const UINT32 func_addr, ...)
{
    /* ================WARNING: DO NOT INSERT ANY CODE HERE: beg ===========================================*/
    UINT32 func_para_value[ MAX_NUM_OF_FUNC_PARAS ];
    UINT32 func_ret_value;
    UINT32 index;

    va_list ap;

    if(0 == func_addr)
    {
        return (0);
    }

    //sys_log(LOGSTDOUT, "dbg_tiny_caller: func_addr = %lx, func_para_num = %d\n", func_addr, func_para_num);
    va_start(ap, func_addr);
    for( index = 0; index < func_para_num; index ++ )
    {
        func_para_value[ index ] = va_arg(ap, UINT32);
        //sys_log(LOGSTDOUT, "dbg_tiny_caller: func para no %d: %lx\n", index, func_para_value[ index ]);
    }
    va_end(ap);

#if (16 != MAX_NUM_OF_FUNC_PARAS)
#error "fatal error:debug.c: MAX_NUM_OF_FUNC_PARAS != 16"
#endif
    #define PARA_VALUE(func_para, x)    ((func_para)[ (x) ])

    #define PARA_LIST_0(func_para)    /*no parameter*/
    #define PARA_LIST_1(func_para)    PARA_VALUE(func_para, 0)
    #define PARA_LIST_2(func_para)    PARA_LIST_1(func_para) ,PARA_VALUE(func_para, 1)
    #define PARA_LIST_3(func_para)    PARA_LIST_2(func_para) ,PARA_VALUE(func_para, 2)
    #define PARA_LIST_4(func_para)    PARA_LIST_3(func_para) ,PARA_VALUE(func_para, 3)
    #define PARA_LIST_5(func_para)    PARA_LIST_4(func_para) ,PARA_VALUE(func_para, 4)
    #define PARA_LIST_6(func_para)    PARA_LIST_5(func_para) ,PARA_VALUE(func_para, 5)
    #define PARA_LIST_7(func_para)    PARA_LIST_6(func_para) ,PARA_VALUE(func_para, 6)
    #define PARA_LIST_8(func_para)    PARA_LIST_7(func_para) ,PARA_VALUE(func_para, 7)
    #define PARA_LIST_9(func_para)    PARA_LIST_8(func_para) ,PARA_VALUE(func_para, 8)
    #define PARA_LIST_10(func_para)   PARA_LIST_9(func_para) ,PARA_VALUE(func_para, 9)
    #define PARA_LIST_11(func_para)   PARA_LIST_10(func_para),PARA_VALUE(func_para, 10)
    #define PARA_LIST_12(func_para)   PARA_LIST_11(func_para),PARA_VALUE(func_para, 11)
    #define PARA_LIST_13(func_para)   PARA_LIST_12(func_para),PARA_VALUE(func_para, 12)
    #define PARA_LIST_14(func_para)   PARA_LIST_13(func_para),PARA_VALUE(func_para, 13)
    #define PARA_LIST_15(func_para)   PARA_LIST_14(func_para),PARA_VALUE(func_para, 14)
    #define PARA_LIST_16(func_para)   PARA_LIST_15(func_para),PARA_VALUE(func_para, 15)

    #define FUNC_CALL(x, func_addr, func_para) \
            ((FUNC_TYPE_##x) func_addr)(PARA_LIST_##x(func_para))

    switch(func_para_num)
    {
        case 0:
            func_ret_value = FUNC_CALL(0, func_addr, func_para_value);
            break;
        case 1:
            func_ret_value = FUNC_CALL(1, func_addr, func_para_value);
            break;
        case 2:
            func_ret_value = FUNC_CALL(2, func_addr, func_para_value);
            break;
        case 3:
            func_ret_value = FUNC_CALL(3, func_addr, func_para_value);
            break;
        case 4:
            func_ret_value = FUNC_CALL(4, func_addr, func_para_value);
            break;
        case 5:
            func_ret_value = FUNC_CALL(5, func_addr, func_para_value);
            break;
        case 6:
            func_ret_value = FUNC_CALL(6, func_addr, func_para_value);
            break;
        case 7:
            func_ret_value = FUNC_CALL(7, func_addr, func_para_value);
            break;
        case 8:
            func_ret_value = FUNC_CALL(8, func_addr, func_para_value);
            break;
        case 9:
            func_ret_value = FUNC_CALL(9, func_addr, func_para_value);
            break;
        case 10:
            func_ret_value = FUNC_CALL(10, func_addr, func_para_value);
            break;
        case 11:
            func_ret_value = FUNC_CALL(11, func_addr, func_para_value);
            break;
        case 12:
            func_ret_value = FUNC_CALL(12, func_addr, func_para_value);
            break;
        case 13:
            func_ret_value = FUNC_CALL(13, func_addr, func_para_value);
            break;
        case 14:
            func_ret_value = FUNC_CALL(14, func_addr, func_para_value);
            break;
        case 15:
            func_ret_value = FUNC_CALL(15, func_addr, func_para_value);
            break;
        case 16:
            func_ret_value = FUNC_CALL(16, func_addr, func_para_value);
            break;
        default:
            sys_log(LOGSTDOUT, "error:dbg_tiny_caller: func para num = %d overflow\n", func_para_num);
            return ((UINT32)(-1));
    }

    #undef PARA_VALUE

    #undef PARA_LIST_0
    #undef PARA_LIST_1
    #undef PARA_LIST_2
    #undef PARA_LIST_3
    #undef PARA_LIST_4
    #undef PARA_LIST_5
    #undef PARA_LIST_6
    #undef PARA_LIST_7
    #undef PARA_LIST_8
    #undef PARA_LIST_9
    #undef PARA_LIST_10
    #undef PARA_LIST_11
    #undef PARA_LIST_12
    #undef PARA_LIST_13
    #undef PARA_LIST_14
    #undef PARA_LIST_15
    #undef PARA_LIST_16

    #undef FUNC_CALL

    return ( func_ret_value );
}
#endif/*(64 == WORDSIZE)*/

EC_BOOL dbg_caller(const UINT32 func_addr, const UINT32 func_para_num, UINT32 *func_para_value, UINT32 *func_ret_val)
{
    UINT32 func_ret_value;
    
#if (16 != MAX_NUM_OF_FUNC_PARAS)
#error "fatal error:debug.c::dbg_caller MAX_NUM_OF_FUNC_PARAS != 16"
#endif
    #define PARA_VALUE(func_para, x)    ((func_para)[ (x) ])

    #define PARA_LIST_0(func_para)    /*no parameter*/
    #define PARA_LIST_1(func_para)    PARA_VALUE(func_para, 0)
    #define PARA_LIST_2(func_para)    PARA_LIST_1(func_para) ,PARA_VALUE(func_para, 1)
    #define PARA_LIST_3(func_para)    PARA_LIST_2(func_para) ,PARA_VALUE(func_para, 2)
    #define PARA_LIST_4(func_para)    PARA_LIST_3(func_para) ,PARA_VALUE(func_para, 3)
    #define PARA_LIST_5(func_para)    PARA_LIST_4(func_para) ,PARA_VALUE(func_para, 4)
    #define PARA_LIST_6(func_para)    PARA_LIST_5(func_para) ,PARA_VALUE(func_para, 5)
    #define PARA_LIST_7(func_para)    PARA_LIST_6(func_para) ,PARA_VALUE(func_para, 6)
    #define PARA_LIST_8(func_para)    PARA_LIST_7(func_para) ,PARA_VALUE(func_para, 7)
    #define PARA_LIST_9(func_para)    PARA_LIST_8(func_para) ,PARA_VALUE(func_para, 8)
    #define PARA_LIST_10(func_para)   PARA_LIST_9(func_para) ,PARA_VALUE(func_para, 9)
    #define PARA_LIST_11(func_para)   PARA_LIST_10(func_para),PARA_VALUE(func_para, 10)
    #define PARA_LIST_12(func_para)   PARA_LIST_11(func_para),PARA_VALUE(func_para, 11)
    #define PARA_LIST_13(func_para)   PARA_LIST_12(func_para),PARA_VALUE(func_para, 12)
    #define PARA_LIST_14(func_para)   PARA_LIST_13(func_para),PARA_VALUE(func_para, 13)
    #define PARA_LIST_15(func_para)   PARA_LIST_14(func_para),PARA_VALUE(func_para, 14)
    #define PARA_LIST_16(func_para)   PARA_LIST_15(func_para),PARA_VALUE(func_para, 15)

    #define FUNC_CALL(x, func_addr, func_para) \
            ((FUNC_TYPE_##x) func_addr)(PARA_LIST_##x(func_para))

    switch(func_para_num)
    {
        case 0:
            func_ret_value = FUNC_CALL(0, func_addr, func_para_value);
            break;
        case 1:
            func_ret_value = FUNC_CALL(1, func_addr, func_para_value);
            break;
        case 2:
            func_ret_value = FUNC_CALL(2, func_addr, func_para_value);
            break;
        case 3:
            func_ret_value = FUNC_CALL(3, func_addr, func_para_value);
            break;
        case 4:
            func_ret_value = FUNC_CALL(4, func_addr, func_para_value);
            break;
        case 5:
            func_ret_value = FUNC_CALL(5, func_addr, func_para_value);
            break;
        case 6:
            func_ret_value = FUNC_CALL(6, func_addr, func_para_value);
            break;
        case 7:
            func_ret_value = FUNC_CALL(7, func_addr, func_para_value);
            break;
        case 8:
            func_ret_value = FUNC_CALL(8, func_addr, func_para_value);
            break;
        case 9:
            func_ret_value = FUNC_CALL(9, func_addr, func_para_value);
            break;
        case 10:
            func_ret_value = FUNC_CALL(10, func_addr, func_para_value);
            break;
        case 11:
            func_ret_value = FUNC_CALL(11, func_addr, func_para_value);
            break;
        case 12:
            func_ret_value = FUNC_CALL(12, func_addr, func_para_value);
            break;
        case 13:
            func_ret_value = FUNC_CALL(13, func_addr, func_para_value);
            break;
        case 14:
            func_ret_value = FUNC_CALL(14, func_addr, func_para_value);
            break;
        case 15:
            func_ret_value = FUNC_CALL(15, func_addr, func_para_value);
            break;
        case 16:
            func_ret_value = FUNC_CALL(16, func_addr, func_para_value);
            break;
        default:
            sys_log(LOGSTDOUT, "error:dbg_caller: func para num = %d overflow\n", func_para_num);
            return (EC_FALSE);
    }

    if(NULL_PTR != func_ret_val)
    {
        (*func_ret_val) = func_ret_value;
    }
    return (EC_TRUE);

    #undef PARA_VALUE

    #undef PARA_LIST_0
    #undef PARA_LIST_1
    #undef PARA_LIST_2
    #undef PARA_LIST_3
    #undef PARA_LIST_4
    #undef PARA_LIST_5
    #undef PARA_LIST_6
    #undef PARA_LIST_7
    #undef PARA_LIST_8
    #undef PARA_LIST_9
    #undef PARA_LIST_10
    #undef PARA_LIST_11
    #undef PARA_LIST_12
    #undef PARA_LIST_13
    #undef PARA_LIST_14
    #undef PARA_LIST_15
    #undef PARA_LIST_16

    #undef FUNC_CALL    
}

FUNC_ADDR_MGR * dbg_fetch_func_addr_mgr_by_md_type(const UINT32 md_type)
{
    return creg_func_addr_vec_get(creg_func_addr_vec_fetch(), md_type);
}

UINT32 dbg_fetch_func_addr_node_by_index(UINT32 func_index, FUNC_ADDR_NODE **func_addr_node_ret)
{
    UINT32 func_addr_node_idx;
    UINT32 func_num;
    FUNC_ADDR_MGR *func_addr_mgr;
    FUNC_ADDR_NODE *func_addr_node;

    func_addr_mgr = dbg_fetch_func_addr_mgr_by_md_type(func_index >> (WORDSIZE/2));
    if(NULL_PTR == func_addr_mgr)
    {
        sys_log(LOGSTDOUT, "error:dbg_fetch_func_addr_node_by_index: func index %lx out of range\n", func_index);
        return ((UINT32)(-1));
    }

    func_num = *(func_addr_mgr->func_num);
    func_addr_node_idx = ((func_index << (WORDSIZE/2)) >> (WORDSIZE/2));

    if(func_addr_node_idx < func_num
    && func_index == func_addr_mgr->func_addr_node[ func_addr_node_idx ].func_index)
    {
        func_addr_node = &(func_addr_mgr->func_addr_node[ func_addr_node_idx ]);
        *func_addr_node_ret = func_addr_node;
        return (0);
    }

    for(func_addr_node_idx = 0; func_addr_node_idx < func_num; func_addr_node_idx ++)
    {
        func_addr_node = &(func_addr_mgr->func_addr_node[ func_addr_node_idx ]);
        if(func_index == func_addr_node->func_index)
        {
            *func_addr_node_ret = func_addr_node;
            return (0);
        }
    }

    return ((UINT32)(-2));
}

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
UINT32 dbg_register_func_addr_list(const FUNC_ADDR_NODE *func_addr_node_list, const UINT32 func_addr_node_list_len, FUNC_ADDR_BLOCK *func_addr_block)
{
    FUNC_ADDR_NODE *func_addr_node;
    UINT32 index;

    /* shortcut */
    return ( 0 );

    if ( EC_FALSE == g_dbg_func_addr_list_init_flag )
    {
        sys_log(LOGSTDOUT,
                "error:dbg_register_func_addr_list: dbg module had not being started yet\n");
        exit( 0 );
    }

    for( index = 0; index < func_addr_node_list_len; index ++ )
    {
        if ( func_addr_block->func_num >= MAX_NUM_OF_FUNC_FOR_DBG )
        {
            sys_log(LOGSTDOUT,
                    "error:dbg_register_func_addr_list: dbg func addr block is full and some funcs are not register.\n");
            break;
        }

        func_addr_node = &(((FUNC_ADDR_NODE *)func_addr_node_list)[ index ]);
        func_addr_node->func_beg_addr = 0;
        func_addr_node->func_end_addr = 0;
        func_addr_node->func_addr_offset = 0;

        func_addr_block->func_addr_node[ func_addr_block->func_num ] = func_addr_node;
        func_addr_block->func_num ++;
    }

    return 0;
}

EC_BOOL dbg_print_func_paras( const FUNC_ADDR_BLOCK *func_addr_block )
{
    UINT8 *func_paras_log_name = "../dump/func_paras.log";
    LOG *func_paras_log;

    FUNC_ADDR_NODE *func_addr_node;
    UINT32 node_idx;
    UINT32 para_idx;

    func_paras_log = fopen( func_paras_log_name, "w");
    if( NULL_PTR ==func_paras_log )
    {
        sys_log(LOGSTDERR,"dbg_print_func_paras: failed to open %s to write\n",
                func_paras_log_name);

        return ( EC_FALSE );
    }

    for( node_idx = 0; node_idx < func_addr_block->func_num; node_idx ++ )
    {
        func_addr_node = func_addr_block->func_addr_node[ node_idx ];

        /* module priority, func name, return type, num of paras */
        sys_log(func_paras_log,"%d %s %d %d",
            func_addr_node->module_priority,
            func_addr_node->func_name,
            func_addr_node->func_ret_type,
            func_addr_node->func_para_num);

        /* direction */
        for ( para_idx = 0; para_idx < func_addr_node->func_para_num; para_idx ++  )
        {
            sys_print(func_paras_log," %d",
                    func_addr_node->func_para_direction[ para_idx ] );
        }
        for( ; para_idx < MAX_NUM_OF_FUNC_PARAS; para_idx ++ )
        {
            sys_print(func_paras_log, " %d", 0);
        }

        /* para type */
        for ( para_idx = 0; para_idx < func_addr_node->func_para_num; para_idx ++  )
        {
            sys_print(func_paras_log," %d",
                    func_addr_node->func_para_type[ para_idx ] );
        }
        for( ; para_idx < MAX_NUM_OF_FUNC_PARAS; para_idx ++ )
        {
            sys_print(func_paras_log, " %d", 0);
        }

        sys_print(func_paras_log,"\n");
    }

    fclose( func_paras_log );
    return ( EC_TRUE );
}

EC_BOOL dbg_fetch_func_addr( const UINT8 * srv_host, const UINT8 * srv_port, FUNC_ADDR_BLOCK *func_addr_block )
{
    EC_BOOL ret;

    /************************** debug beg ***************************/
    dbg_print_func_paras( func_addr_block );
    /************************** debug end ***************************/

#if 0
    ret = client_do( srv_host, srv_port, func_addr_block );
    if( EC_FALSE == ret )
    {
        sys_log(LOGSTDERR,"dbg_fetch_func_addr: client_do failed\n");

    return ( EC_FALSE );
    }
#else
    ret = sqlf_do_all_by_name( NULL_PTR, func_addr_block );
    if( EC_FALSE == ret )
    {
        sys_log(LOGSTDERR,"dbg_fetch_func_addr: sqlf_do_all_by_name failed\n");

    return ( EC_FALSE );
    }
#endif

    return ( EC_TRUE );
}

EC_BOOL dbg_func_addr_consistency_checking(const FUNC_ADDR_NODE *func_addr_node)
{
    return ( ( func_addr_node->func_logic_addr == func_addr_node->func_beg_addr ) ? EC_TRUE : EC_FALSE );
}


/* if a function A call get_cur_func_reg_ebp, then return the reg_ebp of function A */
UINT32 dbg_get_cur_func_reg_ebp()
{
    UINT32 reg_ebp;
#if (32 == WORDSIZE)
    __asm__
    (
        "movl %%ebp, %%eax;"
        "movl %%eax, %0;"
    :"=m"(reg_ebp)
    :
    );
#endif/*(32 == WORDSIZE)*/
#if (64 == WORDSIZE)
    __asm__
    (
        "mov %%rbp, %%rax;"
        "mov %%rax, %0;"
    :"=m"(reg_ebp)
    :
    );
#endif/*(64 == WORDSIZE)*/
    return N_GET_RET_FUNC_EBP(reg_ebp);
}
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/


#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
UINT32 dbg_get_func_by_code_addr(const UINT32 code_addr, FUNC_ADDR_NODE *func_addr_node)
{
    MYSQL_ACCESS    *mysql_acc;

    mysql_acc= &g_dbg_mysql_acc;

    if( EC_FALSE == sqlf_do_one_by_code_addr( mysql_acc, code_addr, func_addr_node ) )
    {
        sys_log(LOGSTDERR,"dbg_get_func_by_code_addr: sqlf_do_one_by_code_addr failed where code_addr = %lx\n",
                code_addr);
        return ((UINT32)(-1));
    }

    return 0;
}
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/

EC_BOOL dbg_type_is_stru_type(UINT32 type)
{
    switch( type )
    {
        case   e_dbg_BIGINT_ptr:
        case   e_dbg_EBGN_ptr:
        case   e_dbg_EBGN_ITEM_ptr:
        case   e_dbg_EC_CURVE_POINT_ptr:
        case   e_dbg_EC_CURVE_AFF_POINT_ptr:
        case   e_dbg_ECF2N_CURVE_ptr:
        case   e_dbg_ECFP_CURVE_ptr:
        case   e_dbg_ECC_KEYPAIR_ptr:
        case   e_dbg_ECC_SIGNATURE_ptr:
        case   e_dbg_POLY_ptr:
        case   e_dbg_POLY_ITEM_ptr:
            break;

        default:
        return (EC_FALSE);
    }
    return (EC_TRUE);
}
UINT32 dbg_print_type(LOG *log,UINT32 func_para_type)
{
    switch ( func_para_type )
    {
       case  e_dbg_UINT32:
        {
            sys_print(log,"UINT32 ");
            break;
        }
        case  e_dbg_UINT16:
        {
            sys_print(log,"UINT16 ");
            break;
        }
        case  e_dbg_UINT8:
        {
            sys_print(log,"UINT8 ");
            break;
        }
        case  e_dbg_int:
        {

            sys_print(log,"int ");
            break;
        }
        case  e_dbg_FUNC_RAND_GEN:
        {
            sys_print(log,"FUNC_RAND_GEN ");
            break;
        }
        case  e_dbg_FUNC_HASH:
        {
            sys_print(log,"FUNC_HASH ");
            break;
        }
        case  e_dbg_EC_BOOL:
        {
            sys_print(log,"EC_BOOL ");
            break;
        }

        case  e_dbg_UINT32_ptr:
        {
            sys_print(log,"UINT32 * ");
            break;
        }
        case  e_dbg_UINT16_ptr:
        {
            sys_print(log,"UINT16 * ");
            break;
        }
        case  e_dbg_UINT8_ptr:
        {
            sys_print(log,"UINT8 * ");
            break;
        }
        case  e_dbg_int_ptr:
        {
            sys_print(log,"int * ");
            break;
        }
        case  e_dbg_BIGINT_ptr:
        {
            sys_print(log,"BIGINT * ");
            break;
        }
        case  e_dbg_EBGN_ptr:
        {
            sys_print(log,"EBGN * ");
            break;
        }
        case  e_dbg_EBGN_ITEM_ptr:
        {
            sys_print(log,"EBGN_ITEM * ");
            break;
        }
        case  e_dbg_EC_CURVE_POINT_ptr:
        {
            sys_print(log,"EC_CURVE_POINT * ");
            break;
        }
        case  e_dbg_EC_CURVE_AFF_POINT_ptr:
        {
            sys_print(log,"EC_CURVE_AFF_POINT * ");
            break;
        }
        case  e_dbg_ECF2N_CURVE_ptr:
        {

            sys_print(log,"ECF2N_CURVE * ");
            break;
        }
        case  e_dbg_ECFP_CURVE_ptr:
        {
            sys_print(log,"ECFP_CURVE * ");
            break;
        }
        case  e_dbg_ECC_KEYPAIR_ptr:
        {
            sys_print(log,"ECC_KEYPAIR * ");
            break;
        }
        case  e_dbg_ECC_SIGNATURE_ptr:
        {
            sys_print(log,"ECC_SIGNATURE * ");
            break;
        }
        case   e_dbg_POLY_ptr:
        {
            sys_print(LOGSTDOUT,"POLY * ");
            break;
        }
        case   e_dbg_POLY_ITEM_ptr:
        {
            sys_print(LOGSTDOUT,"POLY_ITEM * ");
            break;
        }
        case  e_dbg_FUNC_RAND_GEN_ptr:
        {
            sys_print(log,"FUNC_RAND_GEN_ptr * ");
            break;
        }
        case  e_dbg_FUNC_HASH_ptr:
        {
            sys_print(log,"FUNC_HASH * ");
            break;
        }

        default:
        {
            sys_log(log,"error:dbg_print_type: unknown type %ld\n",
                    func_para_type);
            return 0;
        }
    }

    return 0;
}

UINT32 dbg_print_func_para_hex(LOG *log,UINT32 func_para_type, UINT32 func_para_naked_value)
{

    /*1st switch deal with non-pointer type*/
    switch ( func_para_type )
    {
        case  e_dbg_UINT32:
        {
            sys_log(log,"%ld\n", (UINT32)func_para_naked_value);
            return 0;
        }
        case  e_dbg_UINT16:
        {
            sys_log(log,"%d\n",
                    (UINT16)func_para_naked_value);
            return 0;
        }
        case  e_dbg_UINT8:
        {
            sys_log(log,"%d\n",
                    (UINT8)func_para_naked_value);
            return 0;
        }
        case  e_dbg_int:
        {

            sys_log(log,"%d\n",
                    (int)func_para_naked_value);
            return 0;
        }

        case  e_dbg_EC_BOOL:
        {
            if ( EC_TRUE == (UINT32)func_para_naked_value)
            {
                sys_log(log,"EC_TRUE\n");
            }
            else if ( EC_FALSE == (UINT32)func_para_naked_value)
            {
                sys_log(log,"EC_FALSE\n");
            }
            else
            {
                sys_log(log,"error:dbg_print_func_para_hex: unknown bool %ld\n",
                        (UINT32)func_para_naked_value);
            }
            return 0;
        }
    }

    /* if the pointer address is 0, then print info and return success*/
    if ( 0 == func_para_naked_value)
    {
        sys_log(log,"/*NULL_PTR*/0\n");
        return 0;
    }

    /*2nd switch deal with pointer type*/
    switch ( func_para_type )
    {
        case  e_dbg_UINT32:
        case  e_dbg_UINT16:
        case  e_dbg_UINT8:
        case  e_dbg_int:
        case  e_dbg_EC_BOOL:
        {
           return 0;
        }

        case  e_dbg_UINT32_ptr:
        {
            sys_log(log,"%ld\n",
                (UINT32 *)func_para_naked_value);
            break;
        }
        case  e_dbg_UINT16_ptr:
        {
            sys_log(log,"%d\n",
                (UINT16 *)func_para_naked_value);
            break;
        }
        case  e_dbg_UINT8_ptr:
        {
            sys_log(log,"%d\n",
                 (UINT8 *)func_para_naked_value);
            break;
        }
        case  e_dbg_int_ptr:
        {
            sys_log(log,"%d",
                 (int *)func_para_naked_value);
            break;
        }
        case  e_dbg_BIGINT_ptr:
        {
            print_bigint(log,(BIGINT *)func_para_naked_value);
            break;
        }
        case  e_dbg_EBGN_ptr:
        {
            print_ebgn(log,(EBGN *)func_para_naked_value);
            break;
        }
        case  e_dbg_EBGN_ITEM_ptr:
        {
            print_bigint(log,((EBGN_ITEM *)func_para_naked_value)->bgn);
            break;
        }
        case  e_dbg_EC_CURVE_POINT_ptr:
        {
            print_point(log, (EC_CURVE_POINT *)func_para_naked_value);
            break;
        }
        case  e_dbg_EC_CURVE_AFF_POINT_ptr:
        {
            print_aff_point(log, (EC_CURVE_AFF_POINT *)func_para_naked_value);
            break;
        }
        case  e_dbg_ECF2N_CURVE_ptr:
        {

            print_ecf2n_curve(log, (ECF2N_CURVE *)func_para_naked_value);
            break;
        }
        case  e_dbg_ECFP_CURVE_ptr:
        {
            print_ecfp_curve(log, (ECFP_CURVE *)func_para_naked_value);
            break;
        }
        case  e_dbg_ECC_KEYPAIR_ptr:
        {
            print_keypair(log, (ECC_KEYPAIR *)func_para_naked_value);
            break;
        }
        case  e_dbg_ECC_SIGNATURE_ptr:
        {
            print_ecc_signature(log, (ECC_SIGNATURE *)func_para_naked_value);
            break;
        }
        case  e_dbg_POLY_ptr:
        {
            print_poly(log, (POLY *)func_para_naked_value);
            break;
        }
        case  e_dbg_POLY_ITEM_ptr:
        {
            print_poly_item(log, (POLY_ITEM *)func_para_naked_value);
            break;
        }
        case  e_dbg_FUNC_RAND_GEN_ptr:
        {
            sys_log(log,"Random Generator address = %lx\n",
                (UINT32)func_para_naked_value);
            break;
        }
        case  e_dbg_FUNC_HASH_ptr:
        {
            sys_log(log,"Hash Function address = %lx\n",
                    (UINT32)func_para_naked_value);
            break;
        }

        default:
        {
            sys_log(log,"error:dbg_print_func_para_hex: unknown type %ld\n",
                    func_para_type);
            return 0;
        }
    }

    return 0;
}

UINT32 dbg_print_func_para_dec(LOG *log, UINT32 func_para_type, UINT32 func_para_naked_value)
{
    /*1st switch deal with non-pointer type*/
    switch ( func_para_type )
    {
        case  e_dbg_UINT32:
        {
            sys_log(log,"%ld\n", (UINT32)func_para_naked_value);
            return 0;
        }
        case  e_dbg_UINT16:
        {
            sys_log(log,"%d\n",
                    (UINT16)func_para_naked_value);
            return 0;
        }
        case  e_dbg_UINT8:
        {
            sys_log(log,"%d\n",
                    (UINT8)func_para_naked_value);
            return 0;
        }
        case  e_dbg_int:
        {

            sys_log(log,"%d\n",
                    (int)func_para_naked_value);
            return 0;
        }
        case  e_dbg_void:
        {
            sys_log(log,"%ld\n", (UINT32)func_para_naked_value);
            return 0;
        }
        case  e_dbg_EC_BOOL:
        {
            if ( EC_TRUE == (UINT32)func_para_naked_value)
            {
                sys_log(log,"EC_TRUE\n");
            }
            else if ( EC_FALSE == (UINT32)func_para_naked_value)
            {
                sys_log(log,"EC_FALSE\n");
            }
            else
            {
                sys_log(log,"error:dbg_print_func_para_dec: unknown bool %d\n",
                        (UINT32)func_para_naked_value);
            }
            return 0;
        }
    }

    /* if the pointer address is 0, then print info and return success*/
    if ( 0 == func_para_naked_value)
    {
        sys_log(log,"/*NULL_PTR*/0\n");
        return 0;
    }

    /*2nd switch deal with pointer type*/
    switch ( func_para_type )
    {
        case  e_dbg_UINT32:
        case  e_dbg_UINT16:
        case  e_dbg_UINT8:
        case  e_dbg_int:
        case  e_dbg_EC_BOOL:
        {
           return 0;
        }

        case  e_dbg_UINT32_ptr:
        {
            sys_log(log,"%ld\n",
                (UINT32 *)func_para_naked_value);
            break;
        }
        case  e_dbg_UINT16_ptr:
        {
            sys_log(log,"%d\n",
                (UINT16 *)func_para_naked_value);
            break;
        }
        case  e_dbg_UINT8_ptr:
        {
            sys_log(log,"%d\n",
                 (UINT8 *)func_para_naked_value);
            break;
        }
        case  e_dbg_int_ptr:
        {
            sys_log(log,"%d",
                 (int *)func_para_naked_value);
            break;
        }
        case  e_dbg_void_ptr:
        {
            sys_log(log,"%ld",
                 (UINT32 *)func_para_naked_value);
            break;
        }
        case  e_dbg_BIGINT_ptr:
        {
            print_bigint_dec(log, (BIGINT *)func_para_naked_value);
            break;
        }
        case  e_dbg_EBGN_ptr:
        {
            print_ebgn_dec(log, (EBGN *)func_para_naked_value);
            break;
        }
        case  e_dbg_EBGN_ITEM_ptr:
        {
            print_bigint_dec(log, ((EBGN_ITEM *)func_para_naked_value)->bgn);
            break;
        }
        case  e_dbg_EC_CURVE_POINT_ptr:
        {
            print_point_dec(log, (EC_CURVE_POINT *)func_para_naked_value);
            break;
        }
        case  e_dbg_EC_CURVE_AFF_POINT_ptr:
        {
            print_aff_point_dec(log, (EC_CURVE_AFF_POINT *)func_para_naked_value);
            break;
        }
        case  e_dbg_ECF2N_CURVE_ptr:
        {

            print_ecf2n_curve_dec(log, (ECF2N_CURVE *)func_para_naked_value);
            break;
        }
        case  e_dbg_ECFP_CURVE_ptr:
        {
            print_ecfp_curve_dec(log, (ECFP_CURVE *)func_para_naked_value);
            break;
        }
        case  e_dbg_ECC_KEYPAIR_ptr:
        {
            print_keypair_dec(log, (ECC_KEYPAIR *)func_para_naked_value);
            break;
        }
        case  e_dbg_ECC_SIGNATURE_ptr:
        {
            print_ecc_signature_dec(log, (ECC_SIGNATURE *)func_para_naked_value);
            break;
        }
        case  e_dbg_POLY_ptr:
        {
            print_poly_dec(log, (POLY *)func_para_naked_value);
            break;
        }
        case  e_dbg_POLY_ITEM_ptr:
        {
            print_poly_item_dec(log, (POLY_ITEM *)func_para_naked_value);
            break;
        }
        case  e_dbg_FUNC_RAND_GEN_ptr:
        {
            sys_log(log,"Random Generator address = %lx\n",
                (UINT32)func_para_naked_value);
            break;
        }
        case  e_dbg_FUNC_HASH_ptr:
        {
            sys_log(log,"Hash Function address = %lx\n",
                    (UINT32)func_para_naked_value);
            break;
        }

        default:
        {
            sys_log(log,"error:dbg_print_func_para_dec: unknown type %ld\n",
                    func_para_type);
            return 0;
        }
    }

    return 0;
}

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
/**
*    Rev History:
*    1. Apr 13,2008: add cur_func_para_direction
*
**/
UINT32 dbg_x_ray(const UINT32 entry_addr, const UINT32 func_reg_ebp, UINT32 *top_func_reg_ebp, UINT32 *top_module_priority, UINT32 *top_module_id)

{
    FUNC_ADDR_NODE func_addr_node_tmp;
    FUNC_ADDR_NODE *func_addr_node;

    UINT32 func_addr_list_len;
    UINT32 cur_reg_ebp;
    UINT32 ret_code_addr;
    UINT32 cur_func_index;
    UINT32 cur_func_para_num;
    UINT32 cur_func_para_type;
    UINT32 cur_func_para_direction;
    UINT32 cur_func_para_stack_pos;
    UINT32 cur_func_para_indx;
    UINT32 cur_func_firs_para_type;

    UINT32 val_in_func_para_stack_pos;

    UINT32 cur_mod_id;
    UINT32 cur_mod_prio;

    UINT32 top_md_prio;
    UINT32 top_md_id;

    UINT32 ret;

    UINT32 counter = 0;

    func_addr_node = &func_addr_node_tmp;
    func_addr_node->func_beg_addr = 0; /* trick !!! */

    top_md_prio = PRIO_BEG;
    top_md_id   = ERR_MODULE_ID;
    ret = ((UINT32)(-1));

    /* take out the function stack and parameter list */
    cur_reg_ebp = func_reg_ebp;

    while( entry_addr != func_addr_node->func_beg_addr )
    {
        ret_code_addr  = N_GET_RET_CODE_ADDRESS(cur_reg_ebp);
        cur_reg_ebp    = N_GET_RET_FUNC_EBP(cur_reg_ebp);
        ret = dbg_get_func_by_code_addr(ret_code_addr, func_addr_node);

        if ( 0 != ret )
        {
            sys_log(LOGSTDOUT,"func: <unknown>\n");
            sys_log(LOGSTDOUT,"para: <unknown>\n");
            sys_log(LOGSTDOUT,"/*ret eip   */  %lx\n",N_GET_RET_CODE_ADDRESS(cur_reg_ebp));
            sys_log(LOGSTDOUT,"\n");

            continue;
        }

        cur_func_para_num = func_addr_node->func_para_num;

        cur_mod_prio = func_addr_node->module_priority;

        /*update the top_md_prio if current module priority higher*/
        if ( top_md_prio < cur_mod_prio )
        {
            top_md_prio = cur_mod_prio;

            if ( 0 < cur_func_para_num )
            {
                cur_func_firs_para_type = func_addr_node->func_para_type[ 0 ];

                if (IS_MODULE_ID_TYPE(cur_func_firs_para_type))
                {
                    /*when this function has more than one paramters, */
                    /*and the first parameter type is MODULE ID, then pick it up*/

                    cur_mod_id = (UINT32)N_GET_VAL_FROM_PARA_STACK_POS(N_GET_PARA_STACK_POS(cur_reg_ebp, 0));

                }
                else
                {
                    /*when this function has more than one paramters,     */
                    /*but the first parameter type is not MODULE ID, then */
                    /*this function should be the entry of the module,    */
                    cur_mod_id = ERR_MODULE_ID;
                }
            }
            else
            {
                /*when this function has no paramter, then        */
                /*this function should be the entry of the module,*/
                cur_mod_id = ERR_MODULE_ID;
            }
            top_md_id   = cur_mod_id;
        }

        sys_log(LOGSTDOUT,"/*func name */  %s\n", func_addr_node->func_name);
        for ( cur_func_para_indx = 0; cur_func_para_indx < cur_func_para_num; cur_func_para_indx ++ )
        {
            cur_func_para_type = func_addr_node->func_para_type[ cur_func_para_indx ];
            cur_func_para_direction = func_addr_node->func_para_direction[ cur_func_para_indx ];
            cur_func_para_stack_pos = N_GET_PARA_STACK_POS(cur_reg_ebp,cur_func_para_indx);
            val_in_func_para_stack_pos = N_GET_VAL_FROM_PARA_STACK_POS(cur_func_para_stack_pos);

            if ( E_DIRECT_IN == cur_func_para_direction || E_DIRECT_IO == cur_func_para_direction )
            {
                if ( e_dbg_BIGINT_ptr != cur_func_para_type && EC_TRUE == dbg_type_is_stru_type(cur_func_para_type))
                {
                    /* output the parameter with this type*/
                    sys_log(LOGSTDOUT,"/*para #%ld: */  \n",cur_func_para_indx + 1);
                    dbg_print_func_para_dec(LOGSTDOUT,cur_func_para_type, val_in_func_para_stack_pos);
                }
                else
                {
                    sys_log(LOGSTDOUT,"/*para #%ld: */  ",cur_func_para_indx + 1);
                    dbg_print_func_para_dec(LOGSTDOUT,cur_func_para_type, val_in_func_para_stack_pos);
                }
            }
            else
            {
                /* output the parameter with this type*/
                sys_log(LOGSTDOUT,"/*para #%ld: */  N/A\n",cur_func_para_indx + 1);
            }
        }
        sys_log(LOGSTDOUT,"/*ret eip   */  %lx\n",N_GET_RET_CODE_ADDRESS(cur_reg_ebp));
        sys_log(LOGSTDOUT,"\n");
    }

    /**
    *
    *   here cur_reg_ebp is the main's ebp
    *
    **/
    *top_func_reg_ebp    = cur_reg_ebp;
    *top_module_priority = top_md_prio;
    *top_module_id       = top_md_id;

    return 0;
}


UINT32 dbg_free_static_mem_from_top_module( UINT32 top_module_prio,UINT32 top_module_id )
{
    if ( ERR_MODULE_ID == top_module_id )
    {
        return 0;
    }

    switch( top_module_prio )
    {
        case PRIO_MD_BGNZ   :
            bgn_z_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_EBGNZ   :
            ebgn_z_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_BGNZ2  :
             bgn_z2_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_BGNZN  :
            bgn_zn_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_BGNF2N :
            bgn_f2n_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_BGNFP  :
            bgn_fp_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_ECF2N  :
            ec_f2n_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_ECFP   :
            ec_fp_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_ECCF2N :
            ecc_f2n_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_ECCFP  :
            ecc_fp_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_POLYZ:
            sys_log(LOGSTDOUT,"error:dbg_free_static_mem_from_top_module: incompleted top module priority = %ld, top module id = %ld\n",
                            top_module_prio,
                            top_module_id);
            break;
        case PRIO_MD_POLYZN:
            poly_zn_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_POLYZ2:
            poly_z2_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_POLYFP:
            poly_fp_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_POLYF2N:
            poly_f2n_free_module_static_mem(top_module_id);
            break;
        case PRIO_MD_SEAFP:
            sea_fp_free_module_static_mem(top_module_id);
            break;
        default:
            sys_log(LOGSTDOUT,"error:dbg_free_static_mem_from_top_module: unknown top module priority = %ld, top module id = %ld\n",
                            top_module_prio,
                            top_module_id);
    }
    return 0;
}

void dbg_roll_back_to_top_func(UINT32 top_func_reg_ebp)
{
    /*forge a ret scenario which will return to the caller of main and can never come back...*/
#if (32 == WORDSIZE)
    __asm__
    (
        /* free function stack including the function local variables */
        "movl %0, %%esp;"
        "popl %%ebp;"
        "ret;"
    :
    :"ir"(top_func_reg_ebp)
    );
#endif/*(32 == WORDSIZE)*/
#if (64 == WORDSIZE)
    __asm__
    (
        /* free function stack including the function local variables */
        "mov %0, %%rsp;"
        "pop %%rbp;"
        "ret;"
    :
    :"ir"(top_func_reg_ebp)
    );
#endif/*(64 == WORDSIZE)*/
    return ;
}

void dbg_exit( UINT32  mod_type,UINT32  mod_id)
{
    UINT32 cur_reg_ebp;
    UINT32 ret_code_addr;
    UINT32 func_index;
    UINT32 func_reg_ebp;

    UINT32 top_func_reg_ebp;
    UINT32 top_module_prio;
    UINT32 top_module_id;

#if ( SWITCH_ON == STATIC_MEM_STATS_INFO_PRINT_SWITCH )
    sys_log(LOGSTDOUT,"\n");
    sys_log(LOGSTDOUT,"dbg_exit: when enter dbg_exit:\n");
    print_static_mem_status(LOGSTDOUT);
#endif/*STATIC_MEM_STATS_INFO_PRINT_SWITCH*/

    if ( EC_FALSE == g_dbg_func_addr_list_init_flag )
    {
        sys_log(LOGSTDOUT,"error:dbg_exit: debug module was not started. pls call dbg_start in main function at firt.\n");
        exit( 0 );
    }

    //if( EC_FALSE == dbg_fetch_func_addr( g_dbg_func_addr_dbsrv_ip, g_dbg_func_addr_dbsrv_port, &g_dbg_func_addr_block ) )
    //{
        //sys_log(LOGSTDERR,"dbg_exit: dbg_fetch_func_addr failed\n");
    //}
#if (32 == WORDSIZE)
    __asm__
    (
        "movl %%ebp, %%eax;"
        "movl %%eax, %0;"
    :"=m"(cur_reg_ebp)
    );
#endif/*(32 == WORDSIZE)*/
#if (64 == WORDSIZE)
    __asm__
    (
        "mov %%rbp, %%rax;"
        "mov %%rax, %0;"
    :"=m"(cur_reg_ebp)
    );
#endif/*(64 == WORDSIZE)*/
    func_reg_ebp = N_GET_RET_FUNC_EBP(cur_reg_ebp);

    //ret_code_addr  = N_GET_RET_CODE_ADDRESS(cur_reg_ebp);
    //func_index     = dbg_get_func_by_code_addr(ret_code_addr);

    /* X-ray the whole function stack */
    dbg_x_ray(g_dbg_entry_func_addr_node.func_beg_addr, func_reg_ebp, &top_func_reg_ebp, &top_module_prio,&top_module_id);

#if 0
    sys_log(LOGSTDOUT,"dbg_exit: x-ray scan result: top module priority = 0x%lx, top module id = %ld\n",
                    top_module_prio,
                    top_module_id);
#endif
    /*free relative static memory*/
    dbg_free_static_mem_from_top_module( top_module_prio, top_module_id );

#if ( SWITCH_ON == STATIC_MEM_STATS_INFO_PRINT_SWITCH )
    sys_log(LOGSTDOUT,"\n");
    sys_log(LOGSTDOUT,"dbg_exit: before roll back to top func:\n");
    print_static_mem_status(LOGSTDOUT);
    print_static_mem_diag_info(LOGSTDOUT);
#endif/*STATIC_MEM_STATS_INFO_PRINT_SWITCH*/

    dbg_roll_back_to_top_func(top_func_reg_ebp);

}


/*called by autotest.c*/
EC_BOOL dbg_str_cmp(const UINT8 *str_1, const UINT8 * str_2)
{
    UINT32 index;

    if( NULL_PTR == str_1 && NULL_PTR == str_2 )
    {
        return (EC_TRUE);

    }
    if( NULL_PTR == str_1 && NULL_PTR != str_2 )
    {
        return (EC_FALSE);

    }
    if( NULL_PTR != str_1 && NULL_PTR == str_2 )
    {
        return (EC_FALSE);

    }

    index = 0;
    for(;;)
    {
        if ( str_1[ index ] != str_2[ index ] )
        {
            return (EC_FALSE);
        }

        if ( '\0' == str_1[ index ] )
        {
            break;
        }
        index ++;
    }

    return (EC_TRUE);
}

/*called by autotest.c*/
EC_BOOL dbg_value_cmp(LOG *log,UINT32 type, UINT32 db_naked_value,UINT32 ret_naked_value)
{
    TYPE_CONV_ITEM *type_conv_item;

    UINT8 *db_naked_str;
    UINT8 *ret_naked_str;

    UINT32 used_len;

    UINT32 conv_md_id;

    UINT32 ret;

    if( 0 == db_naked_value && 0 != ret_naked_value )
    {
        return ( EC_FALSE );
    }

    if( 0 != db_naked_value && 0 == ret_naked_value )
    {
        return ( EC_FALSE );
    }

    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_log(LOGSTDOUT,"dbg_value_cmp: conv start failed\n");
        return ( EC_FALSE );
    }

    type_conv_item = dbg_query_type_conv_item_by_type(type);
    if( NULL_PTR == type_conv_item )
    {
        sys_log(LOGSTDOUT,"dbg_value_cmp: type %ld is not defined\n", type);
        return ( EC_FALSE );
    }

    alloc_static_mem(MD_TBD, ERR_MODULE_ID, type, &db_naked_str, LOC_DEBUG_0001);
    alloc_static_mem(MD_TBD, ERR_MODULE_ID, type, &ret_naked_str, LOC_DEBUG_0002);

    ret = dbg_tiny_caller(5,
            type_conv_item->conv_type_to_dec_func,
            conv_md_id,
            db_naked_value,
            db_naked_str,
            type_conv_item->str_mm_size - 1, /* remain a string terminator character */
            &used_len);
    if( 0 != ret )
    {
        sys_log(LOGSTDERR,"error:dbg_value_cmp: conv type %d to str failed\n", type);
        free_static_mem(MD_TBD, ERR_MODULE_ID, type, db_naked_str, LOC_DEBUG_0003);
        free_static_mem(MD_TBD, ERR_MODULE_ID, type, ret_naked_str, LOC_DEBUG_0004);
        return ( EC_FALSE );
    }
    db_naked_str[ used_len ] = '\0'; /* terminal character*/

    ret = dbg_tiny_caller(5,
            type_conv_item->conv_type_to_dec_func,
            conv_md_id,
            ret_naked_value,
                ret_naked_str,
            type_conv_item->str_mm_size - 1, /* remain a string terminator character */
            &used_len);
    if( 0 != ret )
    {
        sys_log(LOGSTDERR,"error:dbg_value_cmp: conv type %d to str failed\n", type);
        free_static_mem(MD_TBD, ERR_MODULE_ID, type, db_naked_str, LOC_DEBUG_0005);
        free_static_mem(MD_TBD, ERR_MODULE_ID, type, ret_naked_str, LOC_DEBUG_0006);
        return ( EC_FALSE );
    }
    ret_naked_str[ used_len ] = '\0'; /* terminal character*/

    ret = strcmp(db_naked_str, ret_naked_str);

    free_static_mem(MD_TBD, ERR_MODULE_ID, type, db_naked_str, LOC_DEBUG_0007);
    free_static_mem(MD_TBD, ERR_MODULE_ID, type, ret_naked_str, LOC_DEBUG_0008);

    conv_end(conv_md_id);

    if( 0 == ret )
    {
    return ( EC_TRUE );
    }

    return (EC_FALSE);
}

EC_BOOL dbg_func_para_value_cmp(LOG *log,UINT32 para_type, UINT32 db_naked_para_value,UINT32 ret_naked_para_value)
{
    return dbg_value_cmp(log, para_type, db_naked_para_value, ret_naked_para_value);
}

EC_BOOL dbg_func_ret_value_cmp(LOG *log,UINT32 ret_type, UINT32 db_naked_ret_value,UINT32 ret_naked_ret_value)
{
    return dbg_value_cmp(log, ret_type, db_naked_ret_value, ret_naked_ret_value);
}

#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/

#if ( ASM_DISABLE_SWITCH == SWITCH_ON )
void dbg_exit( UINT32  module_type,UINT32  module_id)
{
    TASK_BRD *task_brd;
    
    task_brd = task_brd_default_get();
    
    if(CTHREAD_GET_TID() == CTHREAD_FETCH_TID(TASK_BRD_DO_ROUTINE_CTHREAD_ID(task_brd), CTHREAD_TID_OFFSET))
    {
        sys_log(LOGSTDOUT, "dbg_exit: cancel coroutine\n");
        coroutine_cancel();
    }
    else
    {
        CTHREAD_ID cthread_id;

        cthread_id = pthread_self();
        sys_log(LOGSTDOUT, "dbg_exit: cancel thread %u\n", cthread_id);

        /*kill thread self*/
        cthread_cancel(cthread_id);    
    }
}
#endif/*( ASM_DISABLE_SWITCH == SWITCH_ON )*/

/*get one char which is excluded comments from the file*/
UINT32 dbg_get_one_char_from_file(FILE *file, UINT8 *ch)
{
    /*when the char '/', then check whether the next char is '*' or not.*/
    /*if the next char is not '*', then store the next char to next_char*/
    /*generally, set next_char = 0xFF to distinguish above.*/
    static UINT8 next_char = 0xFF;

    /*read a char from file and store it to cur_char*/
    int cur_char = 0xEE;

    /*if the next_char is not 0xFF, */
    /*which means that one char is stored in next_char waiting to be read*/
    /*then return this char directly*/
    if ( 0xFF != next_char )
    {
        *ch = next_char;
        next_char = 0xFF;

        return ( 0 );
    }

    /*read a char from file*/
    cur_char = fgetc(file);

    /*if the char is '/' then*/
    if ( '/' == cur_char )
    {
        /*read next char from file and check whether it's '*' */
        cur_char = fgetc(file);

        /*if the char is NOT '*', then we do not enter a comment string*/
        /*so, return the current char directly*/
        if ( '*' != cur_char )
        {
            next_char = cur_char;
            *ch = '/';
            return ( 0 );
        }

        /*else the char is '*', then enter a comment string*/
        cur_char = fgetc(file);
        while ( EOF != cur_char )
        {
            if ( '*' == cur_char )
            {
                cur_char = fgetc(file);
                if ( EOF == cur_char )
                {
                    sys_log(LOGSTDOUT,
                            "dbg_get_one_char_from_file: find a incompleted comment string.\n");
                    exit( 0 );
                }

                if ('/' == cur_char )
                {
                    return dbg_get_one_char_from_file(file, ch);
                }
            }
            else
            {
                cur_char = fgetc(file);
            }
        }
    }

    /*if the char is NOT '/' then it */
    if ( 0xFF == cur_char || EOF == cur_char)
    {
        return ( DBG_EOF );
    }

    *ch = (UINT8)cur_char;

    return ( 0 );
}


/**
*
*   read a string from the file.
*   comment will be skipped and any space char will terminate the string
*
**/
UINT32 dbg_get_one_str_from_file(FILE *log, UINT8 *desstr, UINT32 maxlen,UINT32 *retlen)
{
    UINT32 index;
    UINT8 ch;
    UINT32 ret;

    /*skip the first spaces*/
    ret = 0;
    do
    {
        ret = dbg_get_one_char_from_file(log, &ch);
    }while( 0 == ret && EOF != (char)ch && EC_TRUE == DBG_ISSPACE(ch) );

    if ( DBG_EOF == ret)
    {
        return (DBG_EOF);
    }

    ret = 0;
    index = 0;
    while( 0 == ret && EOF != (char)ch && EC_FALSE == DBG_ISSPACE(ch) && index < maxlen)
    {
        desstr[ index ] = ch;
        index ++;

        ret = dbg_get_one_char_from_file(log, &ch);
    }
    if ( DBG_EOF == ret)
    {
        return (DBG_EOF);
    }

    if ( index >= maxlen )
    {
        sys_log(LOGSTDOUT,"error:dbg_get_one_str_from_file: buffer is not enough (index = %ld).\n",
                index);
        return ((UINT32)( -1 ));
    }

    if ( index < maxlen)
    {
        desstr[ index ] = '\0';
    }

    /*okay, the return length does not include the EOL(End Of Line) char*/
    *retlen = index;

    return 0;
}

/**
*
* init UINT32
*
**/
UINT32 dbg_init_uint32_ptr(const UINT32 md_id, UINT32 *num)
{
    (*num) = 0;
    return (0);
}

/**
*
* init UINT32
*
**/
UINT32 dbg_clean_uint32_ptr(const UINT32 md_id, UINT32 *num)
{
    (*num) = 0;
    return (0);
}

/**
*
* free UINT32
*
**/
UINT32 dbg_free_uint32_ptr(const UINT32 md_id, UINT32 *num)
{
    (*num) = 0;
    return (0);
}

/**
*
* init uint64_t
*
**/
uint64_t dbg_init_uint64_ptr(const UINT32 md_id, uint64_t *num)
{
    (*num) = 0;
    return (0);
}

/**
*
* init uint64_t
*
**/
uint64_t dbg_clean_uint64_ptr(const UINT32 md_id, uint64_t *num)
{
    (*num) = 0;
    return (0);
}

/**
*
* free uint64_t
*
**/
uint64_t dbg_free_uint64_ptr(const UINT32 md_id, uint64_t *num)
{
    (*num) = 0;
    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
