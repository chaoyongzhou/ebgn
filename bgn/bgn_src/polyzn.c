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

#include <stdio.h>
#include <stdlib.h>

#include "bgnctrl.h"
#include "type.h"
#include "moduleconst.h"
#include "mm.h"
#include "log.h"

#include "poly.h"

#include "bgnz.h"
#include "bgnzn.h"
#include "polyzn.h"

#include "debug.h"

#include "print.h"



static POLYZN_MD g_polyzn_md[ MAX_NUM_OF_POLYZN_MD ];
static EC_BOOL  g_polyzn_md_init_flag = EC_FALSE;

static UINT32 poly_zn_adc_n(const UINT32 polyzn_md_id,const BIGINT *n, POLY *poly_c);
static UINT32 poly_zn_double_poly(const UINT32 polyzn_md_id, POLY *poly_c);
static UINT32 poly_zn_item_self_neg(const UINT32 polyzn_md_id, POLY_ITEM *item_c);
static UINT32 poly_zn_poly_self_neg(const UINT32 polyzn_md_id, POLY *poly_c);
static UINT32 poly_zn_sbb_c_a(const UINT32 polyzn_md_id, const POLY *poly_a, POLY *poly_c);
static UINT32 poly_zn_sbb_a_c(const UINT32 polyzn_md_id, const POLY *poly_a, POLY *poly_c);

static UINT32 poly_zn_item_insert(const UINT32 polyzn_md_id, POLY_ITEM *item_a, POLY *poly_c);
static UINT32 poly_zn_item_mul(const UINT32 polyzn_md_id, const POLY_ITEM *item_a, const POLY_ITEM *item_b,POLY_ITEM *item_c);
static UINT32 poly_zn_item_mul_self(const UINT32 polyzn_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c);
static UINT32 poly_zn_item_squ_self(const UINT32 polyzn_md_id, POLY_ITEM *item_c);
static UINT32 poly_zn_mul_self_bgn(const UINT32 polyzn_md_id,const BIGINT *bgn_a, POLY *poly_c);
static UINT32 poly_zn_mul_bgn(const UINT32 polyzn_md_id,const POLY *poly_a,const BIGINT *bgn_b, POLY *poly_c);
static UINT32 poly_zn_mul_word(const UINT32 polyzn_md_id,const POLY *poly_a,const UINT32 word_b, POLY *poly_c);
static UINT32 poly_zn_mul_self(const UINT32 polyzn_md_id,const POLY *poly_a, POLY *poly_c);

static UINT32 poly_zn_dx_self(const UINT32 polyzn_md_id, POLY *poly_c );
static UINT32 poly_zn_Dx_self(const UINT32 polyzn_md_id,const UINT32 depth_of_x, POLY *poly_c );
/**
*   for test only
*
*   to query the status of POLYZN Module
*
**/
void poly_zn_print_module_status(const UINT32 polyzn_md_id, LOG *log)
{
    POLYZN_MD *polyzn_md;
    UINT32 index;

    if ( EC_FALSE == g_polyzn_md_init_flag )
    {
        sys_log(log,"no POLYZN Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_POLYZN_MD; index ++ )
    {
        polyzn_md = &(g_polyzn_md[ index ]);

        if ( 0 < polyzn_md->usedcounter )
        {
            sys_log(log,"POLYZN Module # %ld : %ld refered, refer BGNZ Module : %ld, refer BGNZN Module : %ld\n",
                    index,
                    polyzn_md->usedcounter,
                    polyzn_md->bgnz_md_id,
                    polyzn_md->bgnzn_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed POLYZN module
*
*
**/
UINT32 poly_zn_free_module_static_mem(const UINT32 polyzn_md_id)
{
    POLYZN_MD  *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_free_module_static_mem: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    free_module_static_mem(MD_POLYZN, polyzn_md_id);

    bgn_z_free_module_static_mem(bgnz_md_id);
    bgn_zn_free_module_static_mem(bgnzn_md_id);

    return 0;
}
/**
*
* start POLYZN module
*
**/
UINT32 poly_zn_start( const BIGINT *n )
{
    POLYZN_MD *polyzn_md;
    UINT32 polyzn_md_id;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

    BIGINT *bgnzn_n;

    UINT32 index;

    /* if this is the 1st time to start POLYZN module, then */
    /* initialize g_polyzn_md */
    if ( EC_FALSE ==  g_polyzn_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_POLYZN_MD; index ++ )
        {
            polyzn_md = &(g_polyzn_md[ index ]);

            polyzn_md->usedcounter   = 0;
            polyzn_md->bgnz_md_id = ERR_MODULE_ID;
            polyzn_md->bgnzn_md_id = ERR_MODULE_ID;
        }

        /*register all functions of POLYZN module to DBG module*/
        //dbg_register_func_addr_list(g_polyzn_func_addr_list, g_polyzn_func_addr_list_len);

        g_polyzn_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_POLYZN_MD; index ++ )
    {
        polyzn_md = &(g_polyzn_md[ index ]);

        if ( 0 == polyzn_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_POLYZN_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    bgnz_md_id = ERR_MODULE_ID;
    bgnzn_md_id = ERR_MODULE_ID;

    /* initilize new one POLYZN module */
    polyzn_md_id = index;
    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);

    init_static_mem();

    bgnz_md_id = bgn_z_start();
    bgnzn_md_id = bgn_zn_start( n );

    alloc_static_mem(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgnzn_n  , LOC_POLYZN_0001);
    bgn_z_clone(bgnz_md_id, n, bgnzn_n);

    polyzn_md->bgnz_md_id = bgnz_md_id;
    polyzn_md->bgnzn_md_id = bgnzn_md_id;
    polyzn_md->bgnzn_n = bgnzn_n;
    polyzn_md->usedcounter = 1;

    return ( polyzn_md_id );
}

/**
*
* end POLYZN module
*
**/
void poly_zn_end(const UINT32 polyzn_md_id)
{
    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

    BIGINT *bgnzn_n;

    if ( MAX_NUM_OF_POLYZN_MD < polyzn_md_id )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_end: polyzn_md_id = %ld is overflow.\n",polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < polyzn_md->usedcounter )
    {
        polyzn_md->usedcounter --;
        return ;
    }

    if ( 0 == polyzn_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_end: polyzn_md_id = %ld is not started.\n",polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;
    bgnzn_n = polyzn_md->bgnzn_n;

    free_static_mem(MD_POLYZN, polyzn_md_id, MM_BIGINT, bgnzn_n  , LOC_POLYZN_0002);

    bgn_z_end( bgnz_md_id);
    bgn_zn_end( bgnzn_md_id);

    /* free module : */
    //poly_zn_free_module_static_mem(polyzn_md_id);
    polyzn_md->bgnz_md_id = ERR_MODULE_ID;
    polyzn_md->bgnzn_md_id = ERR_MODULE_ID;
    polyzn_md->usedcounter = 0;

    breathing_static_mem();

    return ;
}

#if 1
#define poly_zn_alloc_bgn(  polyz_md_type, polyzn_md_id, mm_type, ppbgn  , __location__) alloc_static_mem( polyz_md_type, polyzn_md_id, mm_type, ppbgn , (__location__))
#define poly_zn_alloc_deg(  polyz_md_type, polyzn_md_id, mm_type, ppdeg  , __location__) alloc_static_mem( polyz_md_type, polyzn_md_id, mm_type, ppdeg , (__location__))
#define poly_zn_alloc_item( polyz_md_type, polyzn_md_id, mm_type, ppitem , __location__) alloc_static_mem( polyz_md_type, polyzn_md_id, mm_type, ppitem, (__location__))
#define poly_zn_alloc_poly( polyz_md_type, polyzn_md_id, mm_type, pppoly , __location__) alloc_static_mem( polyz_md_type, polyzn_md_id, mm_type, pppoly, (__location__))
#define poly_zn_free_bgn(   polyz_md_type, polyzn_md_id, mm_type, pbgn   , __location__) free_static_mem ( polyz_md_type, polyzn_md_id, mm_type, pbgn  , (__location__))
#define poly_zn_free_deg(   polyz_md_type, polyzn_md_id, mm_type, pdeg   , __location__) free_static_mem ( polyz_md_type, polyzn_md_id, mm_type, pdeg  , (__location__))
#define poly_zn_free_item(  polyz_md_type, polyzn_md_id, mm_type, pitem  , __location__) free_static_mem ( polyz_md_type, polyzn_md_id, mm_type, pitem , (__location__))
#define poly_zn_free_poly(  polyz_md_type, polyzn_md_id, mm_type, ppoly  , __location__) free_static_mem ( polyz_md_type, polyzn_md_id, mm_type, ppoly , (__location__))

#else

UINT32 poly_zn_alloc_bgn(const UINT32 polyz_md_type, const UINT32 polyzn_md_id,const MM_TYPE_UINT32 type, BIGINT **bgn)
{
    if ( MD_POLYZN != polyz_md_type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_alloc_bgn: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_BIGINT != type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_alloc_bgn: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(polyz_md_type, polyzn_md_id, type, bgn, LOC_POLYZN_0003);
    return (0);
}
UINT32 poly_zn_alloc_item(const UINT32 polyz_md_type, const UINT32 polyzn_md_id,const MM_TYPE_UINT32 type,POLY_ITEM **item)
{
    //BIGINT *deg_of_item;

    if ( MD_POLYZN != polyz_md_type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_alloc_item: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_POLY_ITEM != type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_alloc_item: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(polyz_md_type, polyzn_md_id, type, item, LOC_POLYZN_0004);

    //poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &deg_of_item, LOC_POLYZN_0005);
    //POLY_ITEM_DEG(*item) = deg_of_item;

    return (0);
}
UINT32 poly_zn_alloc_poly(const UINT32 polyz_md_type, const UINT32 polyzn_md_id,const MM_TYPE_UINT32 type,POLY **poly)
{
    if ( MD_POLYZN != polyz_md_type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_alloc_poly: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_POLY != type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_alloc_poly: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(polyz_md_type, polyzn_md_id, type, poly, LOC_POLYZN_0006);
    return (0);
}
UINT32 poly_zn_free_bgn(const UINT32 polyz_md_type, const UINT32 polyzn_md_id,const MM_TYPE_UINT32 type,BIGINT *bgn)
{
    if ( MD_POLYZN != polyz_md_type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_free_bgn: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_BIGINT != type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_free_bgn: invalid type = %ld\n",type);
        exit(0);
    }

    free_static_mem(polyz_md_type, polyzn_md_id, type, bgn, LOC_POLYZN_0007);
    return (0);
}
UINT32 poly_zn_free_item(const UINT32 polyz_md_type, const UINT32 polyzn_md_id,const MM_TYPE_UINT32 type,POLY_ITEM *item)
{
    if ( MD_POLYZN != polyz_md_type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_free_item: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_POLY_ITEM != type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_free_item: invalid type = %ld\n",type);
        exit(0);
    }

    free_static_mem(polyz_md_type, polyzn_md_id, type, item, LOC_POLYZN_0008);
    return (0);
}
UINT32 poly_zn_free_poly(const UINT32 polyz_md_type, const UINT32 polyzn_md_id,const MM_TYPE_UINT32 type,POLY *poly)
{
    if ( MD_POLYZN != polyz_md_type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_free_poly: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_POLY != type )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_free_poly: invalid type = %ld\n",type);
        exit(0);
    }

    free_static_mem(polyz_md_type, polyzn_md_id, type, poly, LOC_POLYZN_0009);
    return (0);
}

#endif

/**
*
*   destory the whole item,including its content(deg, coe) and itself.
*   so, when return from this function, item cannot be refered any more
*
**/
UINT32 poly_zn_item_destory(const UINT32 polyzn_md_id, POLY_ITEM *item)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_destory: item is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_destory: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
    {
        poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, POLY_ITEM_BGN_COE(item), LOC_POLYZN_0010);
        POLY_ITEM_BGN_COE(item) = NULL_PTR;
    }
    else
    {
        poly_zn_poly_destory(polyzn_md_id, POLY_ITEM_POLY_COE(item));
        POLY_ITEM_POLY_COE(item) = NULL_PTR;
    }

    poly_zn_free_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, item, LOC_POLYZN_0011);

    return ( 0 );
}

/**
*
*   destory the whole poly,i.e., all its items but not the poly itself.
*   so, when return from this function, poly can be refered again without any item.
*
**/
UINT32 poly_zn_poly_clean(const UINT32 polyzn_md_id, POLY *poly)
{
    POLY_ITEM *item;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_clean: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_poly_clean: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    item = POLY_FIRST_ITEM(poly);

    while ( item != POLY_NULL_ITEM(poly) )
    {
        POLY_ITEM_DEL(item);
        poly_zn_item_destory(polyzn_md_id, item);

        item = POLY_FIRST_ITEM(poly);
    }

    POLY_INIT(poly);

    return ( 0 );
}

/**
*
*   destory the whole poly,including its all items and itself.
*   so, when return from this function, poly cannot be refered any more
*
**/
UINT32 poly_zn_poly_destory(const UINT32 polyzn_md_id, POLY *poly)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_destory: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_poly_destory: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    poly_zn_poly_clean(polyzn_md_id,poly);

    poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly, LOC_POLYZN_0012);

    return ( 0 );
}

UINT32 poly_zn_poly_clone(const UINT32 polyzn_md_id,const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_clone: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_clone: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_poly_clone: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_zn_poly_clean(polyzn_md_id, poly_c);
    }
    //POLY_INIT(poly_c);

    item_a = POLY_FIRST_ITEM(poly_a);

    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0013);
        poly_zn_item_clone(polyzn_md_id, item_a, item_c);

        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_a = POLY_ITEM_NEXT(item_a);
    }

    return (0);
}

UINT32 poly_zn_item_clone(const UINT32 polyzn_md_id,const POLY_ITEM *item_a, POLY_ITEM *item_c)
{
    BIGINT  *bgn_coe_of_item_c;
    POLY    *poly_coe_of_item_c;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_clone: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_clone: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_clone: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    POLY_ITEM_INIT(item_c);

    /*clone degree of item*/
    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

    /*clone coefficient of item*/
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
    {
        /*clone bgn coe*/
        poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0014);
        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        bgn_zn_clone(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c));

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
    }
    else
    {
        /*clone poly coe*/
        poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0015);
        POLY_INIT(poly_coe_of_item_c);

        POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;

        poly_zn_poly_clone(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
    }

    return (0);
}

/**
*
*   set poly = 0
*
**/
UINT32 poly_zn_set_zero(const UINT32 polyzn_md_id,POLY *poly)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_set_zero: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_set_zero: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    if ( EC_TRUE == POLY_IS_EMPTY(poly) )
    {
        return ( 0 );
    }

    return poly_zn_poly_clean(polyzn_md_id, poly);
}

/**
*
*   set poly = 1
*
**/
UINT32 poly_zn_set_one(const UINT32 polyzn_md_id,POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_set_one: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_set_one: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_zn_poly_clean(polyzn_md_id, poly);
    }

    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item, LOC_POLYZN_0016);
    poly_zn_alloc_bgn (MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYZN_0017);

    /*deg(item) = 0*/
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item));

    /*bgn_coe(item) = 1*/
    bgn_zn_set_one(bgnzn_md_id, bgn_coe_of_item);

    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly, item);

    return ( 0 );
}

/**
*
*   set poly = 1
*
**/
UINT32 poly_zn_set_n(const UINT32 polyzn_md_id,const BIGINT *n, POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_set_n: n is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_set_n: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_set_n: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_zn_poly_clean(polyzn_md_id, poly);
    }

    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item, LOC_POLYZN_0018);
    poly_zn_alloc_bgn (MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYZN_0019);

    /*deg(item) = 0*/
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item));

    /*bgn_coe(item) = n*/
    bgn_zn_clone(bgnzn_md_id, n, bgn_coe_of_item);

    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly, item);

    return ( 0 );
}


EC_BOOL poly_zn_item_cmp(const UINT32 polyzn_md_id,const POLY_ITEM *item_a, const POLY_ITEM *item_b)
{
    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_cmp: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == item_b )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_cmp: item_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_cmp: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    if ( 0 != POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_b)) )
    {
        return (EC_FALSE);
    }

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        if ( 0 != bgn_zn_cmp(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b)))
        {
            return (EC_FALSE);
        }
        return (EC_TRUE);
    }
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        return poly_zn_poly_cmp(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b));
    }

    return (EC_FALSE);
}
/**
*
*   if poly_a = poly_b then return EC_TRUE
*   else    return EC_FALSE
*
**/
EC_BOOL poly_zn_poly_cmp(const UINT32 polyzn_md_id,const POLY *poly_a, const POLY *poly_b)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_cmp: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_cmp: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_poly_cmp: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    item_a = POLY_FIRST_ITEM(poly_a);
    item_b = POLY_FIRST_ITEM(poly_b);

    while ( item_a != POLY_NULL_ITEM(poly_a) && item_b != POLY_NULL_ITEM(poly_b) )
    {
        if ( EC_FALSE == poly_zn_item_cmp(polyzn_md_id, item_a, item_b) )
        {
            return (EC_FALSE);
        }

        item_a = POLY_ITEM_NEXT(item_a);
        item_b = POLY_ITEM_NEXT(item_b);
    }

    if ( item_a != POLY_NULL_ITEM(poly_a) || item_b != POLY_NULL_ITEM(poly_b)  )
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/**
*
*Function: poly_c = poly_c + n
*
*poly_adc_const := proc(poly_c, n)
*
*item_c = first_item(poly_c);
*
*if deg(item_c) is zero then
*    if TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) + n;
*    end if
*    if FALSE == bgn_coe_flag(item_c) then
*
*        poly_adc_const(poly_coe(item_c), n);
*    end if
*end if
*
*if deg(item_c) is nonzero then
*    new tmp_item_c
*    deg(tmp_item_c) = 0;
*    bgn_coe(tmp_item_c) = n;
*    bgn_coe_flag = TRUE;
*    insert tmp_item_c to the head of poly_c;
*end if
*
**/
static UINT32 poly_zn_adc_n(const UINT32 polyzn_md_id,const BIGINT *n, POLY *poly_c)
{
    POLY_ITEM *item_c;
    BIGINT  *bgn_coe_of_item_c;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_adc_n: n is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_adc_n: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_adc_n: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    item_c = POLY_FIRST_ITEM(poly_c);

    /*
    *if deg(item_c) is zero then
    *    if TRUE == bgn_coe_flag(item_c) then
    *        bgn_coe(item_c) = bgn_coe(item_c) + n;
    *    end if
    *    if FALSE == bgn_coe_flag(item_c) then
    *
    *        poly_adc_const(poly_coe(item_c), n);
    *    end if
    *end if
    */
    if ( EC_TRUE == POLY_ITEM_DEG_IS_ZERO( bgnz_md_id, POLY_ITEM_DEG(item_c)))
    {
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            bgn_zn_add(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c), n, POLY_ITEM_BGN_COE(item_c));
        }
        else
        {
            poly_zn_adc_n(polyzn_md_id, n, POLY_ITEM_POLY_COE(item_c));
        }
    }
    /*
    *if deg(item_c) is nonzero then
    *    new tmp_item_c
    *    deg(tmp_item_c) = 0;
    *    bgn_coe(tmp_item_c) = n;
    *    bgn_coe_flag = TRUE;
    *    insert tmp_item_c to the head of poly_c;
    *end if
    */
    else
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0020);

        POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));

        poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0021);
        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        bgn_zn_clone(bgnzn_md_id, n, POLY_ITEM_BGN_COE(item_c));

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

        POLY_ADD_ITEM_HEAD(poly_c, item_c);
    }
    return (0);
}


/**
*
*Function: poly_c = poly_a + n
*
*poly_add_n := proc(poly_a, n, poly_c)
*
*if poly_a != poly_c then
*    clone poly_a to poly_c
*end if
*
*poly_adc_n(poly_c, n);
*
*end proc;
*
**/
UINT32 poly_zn_add_n(const UINT32 polyzn_md_id,const POLY *poly_a, const BIGINT *n, POLY *poly_c)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_add_n: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_add_n: n is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_add_n: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_add_n: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a != poly_c )
    {
        /*clone poly_a to poly_c*/
        poly_zn_poly_clone(polyzn_md_id, poly_a, poly_c);
    }

    poly_zn_adc_n(polyzn_md_id, n, poly_c);

    return (0);
}

/**
*
*Function: poly_c = poly_c + poly_c
*poly_double:= proc(poly_c)
*
*item_c = first_item(poly_c);
*while item_c != null_item(poly_c) do
*
*    if TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) + bgn_coe(item_c);
*
*        if bgn_coe(item_c) is zero then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    end if
*
*    if FALSE == bgn_coe_flag(item_c) then
*        poly_double(poly_coe(item_c), poly_coe(item_c));
*
*        if poly_coe(item_c) is empty then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*
*    end if
*
*    item_c = next_item(poly_c, item_c);
*
*end do;
*end proc;
*
**/
static UINT32 poly_zn_double_poly(const UINT32 polyzn_md_id, POLY *poly_c)
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;
    BIGINT    *bgn_coe_of_item_c;

    POLYZN_MD *polyzn_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_double_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_double_poly: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    item_c = POLY_FIRST_ITEM(poly_c);

    while( item_c != POLY_NULL_ITEM(poly_c) )
    {
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            bgn_coe_of_item_c = POLY_ITEM_BGN_COE(item_c);
            bgn_zn_add(bgnzn_md_id, bgn_coe_of_item_c, bgn_coe_of_item_c, bgn_coe_of_item_c);

            if ( 0 == bgn_zn_is_zero(bgnzn_md_id, bgn_coe_of_item_c) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_NEXT(item_c);

                POLY_ITEM_DEL(item_t);
                poly_zn_item_destory(polyzn_md_id, item_t);
            }
            else
            {
                item_c = POLY_ITEM_NEXT(item_c);
            }
        }
        else
        {
            poly_zn_double_poly(polyzn_md_id,POLY_ITEM_POLY_COE(item_c));
            if ( EC_TRUE == POLY_IS_EMPTY( POLY_ITEM_POLY_COE(item_c) ) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_NEXT(item_c);

                POLY_ITEM_DEL(item_t);
                poly_zn_item_destory(polyzn_md_id, item_t);
            }
            else
            {
                item_c = POLY_ITEM_NEXT(item_c);
            }
        }
    }

    return ( 0 );
}

/**
*
*poly_c = poly_c + poly_a
*poly_adc:= proc(poly_a, poly_c)
*
*if poly_a == poly_c then
* poly_double(poly_c)
*end if
*
* >>> from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_c = first_item(poly_c);
*while item_a != null_item(poly_a) && item_c != null_item(poly_c) do
*
*if deg(item_a) == deg(item_c) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_a) + bgn_coe(item_c);
*
* >>> to judge the carry flag of the addition operation seems better
*        if bgn_coe(item_c) is zero then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
*        poly_add_const(poly_coe(item_c), bgn_coe(item_a), poly_coe(item_c));
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
*        new tmp_poly_c
*        poly_add_const(poly_coe(item_a), bgn_coe(item_c), tmp_poly_c);
*
*        del bgn_coe(item_c)
*
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*        poly_add(poly_coe(item_a), poly_coe(item_c), poly_coe(item_c));
*
*        if poly_coe(item_c) is empty then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_c = next_item(poly_c, item_c);
*end if;
*
*if deg(item_a) < deg(item_c) then
*    new tmp_item_c
*
*    clone item_a to tmp_item_c(note: here clone item including the sub poly coe if exist)
*    prev_item(poly_c,item_c) = tmp_item_c;
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_c) then
*    item_c = next_item(poly_c, item_c);
*end if;
*
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new tmp_item_c
*
*    clone item_a to tmp_item_c(note: here clone item including the sub poly coe if exist)
*    insert tmp_item_cto the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*
*end proc;
*
**/
UINT32 poly_zn_adc_poly(const UINT32 polyzn_md_id,const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_c;
    POLY_ITEM   *item_t;
    POLY        *poly_coe_of_item_c;
    int cmp_result;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_adc_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_adc_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_adc_poly: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_c )
    {
        return poly_zn_double_poly(polyzn_md_id, poly_c);
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    item_a = POLY_FIRST_ITEM(poly_a);
    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_a != POLY_NULL_ITEM(poly_a) && item_c != POLY_NULL_ITEM(poly_c) )
    {
        cmp_result = POLY_ITEM_DEG_CMP( bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
            *        bgn_coe(item_c) = bgn_coe(item_a) + bgn_coe(item_c);
            *
            * >>> to judge the carry flag of the addition operation seems better
            *        if bgn_coe(item_c) is zero then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                bgn_zn_add(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));

                if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c)) )
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_zn_item_destory(polyzn_md_id, item_t);
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
            *        poly_add_const(poly_coe(item_c), bgn_coe(item_a), poly_coe(item_c));
            *
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_zn_add_n(polyzn_md_id,POLY_ITEM_POLY_COE(item_c), POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_c));
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
            *        new tmp_poly_c
            *        poly_add_const(poly_coe(item_a), bgn_coe(item_c), tmp_poly_c);
            *
            *        del bgn_coe(item_c)
            *
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0022);
                POLY_INIT(poly_coe_of_item_c);

                poly_zn_add_n(polyzn_md_id,POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_c), poly_coe_of_item_c);

                poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, POLY_ITEM_BGN_COE(item_c), LOC_POLYZN_0023);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;

                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
            *        poly_add(poly_coe(item_a), poly_coe(item_c), poly_coe(item_c));
            *
            *        if poly_coe(item_c) is empty then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *
            *    end if
            */
            else if( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_zn_add_poly(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c), POLY_ITEM_POLY_COE(item_c));

                if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_zn_item_destory(polyzn_md_id, item_t);
                }
            }
            else
            {
                ;
            }
            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_c = next_item(poly_c, item_c);
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_c = POLY_ITEM_NEXT(item_c);
        }

        /*
        *if deg(item_a) < deg(item_c) then
        *    new tmp_item_c
        *
        *    clone item_a to tmp_item_c(note: here clone item including the sub poly coe if exist)
        *    prev_item(poly_c,item_c) = tmp_item_c;
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( -1 == cmp_result )
        {
            poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_t, LOC_POLYZN_0024);
            poly_zn_item_clone(polyzn_md_id, item_a, item_t);

            POLY_ITEM_ADD_PREV(item_c, item_t);

            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_c) then
        *    item_c = next_item(poly_c, item_c);
        *end if;
        */
        if ( 1 == cmp_result )
        {
            item_c = POLY_ITEM_NEXT(item_c);
        }
    }

    /*
    *while item_a != null_item(poly_a) do
    *    new tmp_item_c
    *
    *    clone item_a to tmp_item_c(note: here clone item including the sub poly coe if exist)
    *    insert tmp_item_cto the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    */
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_t, LOC_POLYZN_0025);

        poly_zn_item_clone(polyzn_md_id, item_a, item_t);

        POLY_ADD_ITEM_TAIL(poly_c, item_t);

        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

/**
*
*poly_add:= proc(poly_a, poly_b, poly_c)
*
*if poly_c == poly_a then
*poly_adc(poly_b,poly_c)
*end if
*
*if poly_c == poly_b then
*poly_adc(poly_a,poly_c)
*end if
*
*
*>>> from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_b = first_item(poly_b);
*while item_a != null_item(poly_a) && item_b != null_item(poly_b) do
*
*if deg(item_a) == deg(item_b) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
*        bgn_coe_c = bgn_coe(item_a) + bgn_coe(item_b);
*
*        if bgn_coe_c is nonzero then
*            new item_c
*            deg(item_c) = deg(item_a);
*            bgn_coe(item_c) = bgn_coe_c;
*            bgn_coe_flag(item_c) = TRUE;
*            insert item_c to tail of poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b)
*        new tmp_poly_c
*        poly_add_const(poly_coe(item_b), bgn_coe(item_a), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_b);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b)
*        new tmp_poly_c
*        poly_add_const(poly_coe(item_a), bgn_coe(item_b), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
*        new tmp_poly_c
*        poly_add(poly_coe(item_a), poly_coe(item_b), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_b = next_item(poly_b, item_b);
*end if;
*
*if deg(item_a) < deg(item_b) then
*    new item_c
*    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_b) then
*    new item_c
*    clone item_b to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_b = next_item(poly_b, item_b);
*end if;
*
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new item_c
*
*    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*while item_b != null_item(poly_b) do
*    new item_c
*
*    clone item_b to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_b = next_item(poly_b, item_b);
*end do
*
*
*end proc;
*
**/
UINT32 poly_zn_add_poly(const UINT32 polyzn_md_id,const POLY *poly_a, const POLY *poly_b, POLY *poly_c)
{
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_b;
    POLY_ITEM   *item_c;
    POLY        *poly_coe_of_item_c;
    BIGINT      *bgn_coe_of_item_c;
    int cmp_result;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_add_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_add_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_add_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_add_poly: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_c == poly_a )
    {
        return poly_zn_adc_poly(polyzn_md_id, poly_b, poly_c);
    }

    if ( poly_c == poly_b )
    {
        return poly_zn_adc_poly(polyzn_md_id, poly_a, poly_c);
    }

    /*clean all items of the poly_c*/
    poly_zn_poly_clean(polyzn_md_id, poly_c);

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    item_a = POLY_FIRST_ITEM(poly_a);
    item_b = POLY_FIRST_ITEM(poly_b);

    while ( item_a != POLY_NULL_ITEM(poly_a) && item_b != POLY_NULL_ITEM(poly_b) )
    {
        cmp_result = POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_b));

        /*if deg(item_a) == deg(item_b) then*/
        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
            *        bgn_coe_c = bgn_coe(item_a) + bgn_coe(item_b);
            *
            *        if bgn_coe_c is nonzero then
            *            new item_c
            *            deg(item_c) = deg(item_a);
            *            bgn_coe(item_c) = bgn_coe_c;
            *            bgn_coe_flag(item_c) = TRUE;
            *            insert item_c to tail of poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b))
            {
                poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0026);

                bgn_zn_add(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b), bgn_coe_of_item_c);

                if ( EC_FALSE == bgn_zn_is_zero(bgnzn_md_id, bgn_coe_of_item_c) )
                {
                    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0027);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
                else
                {
                    poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZN_0028);
                    bgn_coe_of_item_c = NULL_PTR;
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b)
            *        new tmp_poly_c
            *        poly_add_const(poly_coe(item_b), bgn_coe(item_a), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_b);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b))
            {
                    poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0029);
                    POLY_INIT(poly_coe_of_item_c);

                    poly_zn_add_n(polyzn_md_id, POLY_ITEM_POLY_COE(item_b), POLY_ITEM_BGN_COE(item_a), poly_coe_of_item_c);

                    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0030);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_b), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b)
            *        new tmp_poly_c
            *        poly_add_const(poly_coe(item_a), bgn_coe(item_b), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b))
            {
                    poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0031);
                    POLY_INIT(poly_coe_of_item_c);

                    poly_zn_add_n(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_b), poly_coe_of_item_c);

                    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0032);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
            *        new tmp_poly_c
            *        poly_add(poly_coe(item_a), poly_coe(item_b), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b))
            {
                    poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0033);
                    POLY_INIT(poly_coe_of_item_c);

                    poly_zn_add_poly(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);
                    if ( EC_FALSE == POLY_IS_EMPTY(poly_coe_of_item_c) )
                    {
                        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0034);

                        POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                        POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                        POLY_ADD_ITEM_TAIL(poly_c, item_c);
                    }
                    else
                    {
                        poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZN_0035);
                        poly_coe_of_item_c = NULL_PTR;
                    }
            }
            else
            {
                ;
            }
            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_b = next_item(poly_b, item_b);
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_b = POLY_ITEM_NEXT(item_b);
        }
        /*
        *if deg(item_a) < deg(item_b) then
        *    new item_c
        *    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
        *    insert item_c to the tail of poly_c
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( -1 == cmp_result )
        {
            poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0036);

            poly_zn_item_clone(polyzn_md_id, item_a, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);

            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_b) then
        *    new item_c
        *    clone item_b to item_c (note: here clone item including the sub poly coe if exist)
        *    insert item_c to the tail of poly_c
        *    item_b = next_item(poly_b, item_b);
        *end if;
        */
        if ( 1 == cmp_result )
        {

            poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0037);

            poly_zn_item_clone(polyzn_md_id, item_b, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);

            item_b = POLY_ITEM_NEXT(item_b);
        }
    }
    /*
    *while item_a != null_item(poly_a) do
    *    new item_c
    *
    *    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
    *    insert item_c to the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    */
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0038);

        poly_zn_item_clone(polyzn_md_id, item_a, item_c);
        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_a = POLY_ITEM_NEXT(item_a);
    }
    /*
    *while item_b != null_item(poly_b) do
    *    new item_c
    *
    *    clone item_b to item_c (note: here clone item including the sub poly coe if exist)
    *    insert item_c to the tail of poly_c
    *    item_b = next_item(poly_b, item_b);
    *end do
    */
    while( item_b != POLY_NULL_ITEM(poly_b) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0039);

        poly_zn_item_clone(polyzn_md_id, item_b, item_c);
        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_b = POLY_ITEM_NEXT(item_b);
    }

    return ( 0 );
}


/**
*
*poly_c = poly_a - bgn_b
*
*poly_sbb_const_2 :=proc(poly_a, bgn_b,poly_c)
*
*if bgn_b is zero then
* return;
*end if
*
*TODO: poly_c = poly_c - bgn_b
*
*item_c = first_item(poly_c);
*
*if item_c = null_item(poly_c) or deg(item_c) is nonzero then
*    new item_c
*
*    deg(item_c) = 0;
*    bgn_coe(item_c) = -bgn_b;
*    bgn_coe_flag(item_c) = TRUE;
*    insert item_c to the head of poly_c
*endif
*
*if deg(item_c) is zero  then
*
*    if TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) - bgn_b;
*        if 0 == deg(item_c) then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    endif
*
*    if FALSE == bgn_coe_flag(item_c) then
*        poly_sbb_const_2(poly_coe(item_c), bgn_b,poly_coe(item_c));
*    endif
*end if
*
*end proc
*
**/
UINT32 poly_zn_sub_poly_bgn(const UINT32 polyzn_md_id,const POLY *poly_a, const BIGINT *bgn_b, POLY *poly_c)
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;
    BIGINT *bgn_coe_of_item_c;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sub_poly_bgn: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == bgn_b )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sub_poly_bgn: bgn_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sub_poly_bgn: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_sub_poly_bgn: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a != poly_c then
    *   poly_clone(poly_a, poly_c);
    *end if
    */
    if ( poly_a != poly_c )
    {
        poly_zn_poly_clone(polyzn_md_id, poly_a, poly_c);
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    /*
    *if bgn_b is zero then
    *   return;
    *end if
    */
    if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, bgn_b))
    {
        return ( 0 );
    }

    /*TODO: poly_c = poly_c - bgn_b*/
    /*
    *item_c = first_item(poly_c);
    *
    *if item_c = null_item(poly_c) or deg(item_c) is nonzero then
    *    new item_c
    *
    *    deg(item_c) = 0;
    *    bgn_coe(item_c) = -bgn_b;
    *    bgn_coe_flag(item_c) = TRUE;
    *    insert item_c to the head of poly_c
    *endif
    */
    item_c = POLY_FIRST_ITEM(poly_c);
    if ( item_c == POLY_NULL_ITEM(poly_c) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0040);

        POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));

        poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0041);

        bgn_zn_neg(bgnzn_md_id, bgn_b, bgn_coe_of_item_c);

        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

        POLY_ADD_ITEM_HEAD(poly_c, item_c);

        return ( 0 );
    }

    if ( EC_FALSE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c)))
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0042);

        POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));

        poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0043);

        bgn_zn_neg(bgnzn_md_id, bgn_b, bgn_coe_of_item_c);

        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

        POLY_ADD_ITEM_HEAD(poly_c, item_c);

        return ( 0 );
    }
    /* now deg(item_c) is zero */
    /*
    *
    *    if TRUE == bgn_coe_flag(item_c) then
    *        bgn_coe(item_c) = bgn_coe(item_c) - bgn_b;
    *        if 0 == bgn_coe(item_c) then
    *            item_c = prev_item(poly_c, item_c)
    *            remove next_item(poly_c,item_c) from poly_c
    *        end if
    *    endif
    *
    *    if FALSE == bgn_coe_flag(item_c) then
    *        poly_sbb_const_2(poly_coe(item_c), bgn_b,poly_coe(item_c));
    *    endif
    *
    */
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        bgn_zn_sub(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c), bgn_b, POLY_ITEM_BGN_COE(item_c));

        if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c)))
        {
            item_t = item_c;
            item_c = POLY_ITEM_PREV(item_c);

            POLY_ITEM_DEL(item_t);
            poly_zn_item_destory(polyzn_md_id, item_t);
        }
        return ( 0 );
    }

    return poly_zn_sub_poly_bgn(polyzn_md_id, POLY_ITEM_POLY_COE(item_c), bgn_b, POLY_ITEM_POLY_COE(item_c));
}


/**
*poly_c = bgn_a - poly_b
*
*poly_sbb_const_1 :=proc(bgn_a, poly_b,poly_c)
*
*if poly_b == poly_c then
*   poly_neg_0(poly_c);
*   return poly_adc_const(bgn_a,poly_c);
*end if
*
*poly_neg(poly_b,poly_c);
*poly_adc_const(bgn_a,poly_c);
*
*end proc
*
**/
UINT32 poly_zn_sub_bgn_poly(const UINT32 polyzn_md_id,const BIGINT *bgn_a, const POLY *poly_b, POLY *poly_c)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == bgn_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sub_bgn_poly: bgn_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sub_bgn_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sub_bgn_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_sub_bgn_poly: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    poly_zn_poly_neg(polyzn_md_id, poly_b, poly_c);
    return poly_zn_adc_n(polyzn_md_id, bgn_a, poly_c);
}

/**
*item = - item
*
*poly_item_neg_0 := proc(item)
*
*if TRUE == bgn_coe_flag(item) then
*   bgn_coe(item) = -bgn_coe(item);
*end if
*
*if FALSE == bgn_coe_flag(item) then
*   poly_neg_0(poly_coe(item));
*end if
*
*end proc
**/
static UINT32 poly_zn_item_self_neg(const UINT32 polyzn_md_id, POLY_ITEM *item_c)
{
    POLYZN_MD *polyzn_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_self_neg: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_self_neg: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        bgn_zn_neg(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));
        return ( 0 );
    }
    else
    {
        return poly_zn_poly_self_neg(polyzn_md_id, POLY_ITEM_POLY_COE(item_c));
    }
    return ( 0 );
}

/*
*item_c = - item_a
*
*poly_item_neg := proc(item_a, item_c)
*
*if item_a == item_c then
*   poly_item_neg_0(item_c);
*   return;
*end if
*
*new deg(item_c)
*deg(item_c) = deg(item_a);
*
*if TRUE == bgn_coe_flag(item_a) then
*   new bgn_coe(item_c)
*   bgn_coe(item_c) = bgn_coe_of_item_c;
*   bgn_coe_flag(item_c) = TRUE
*end if
*
*if FALSE == bgn_coe_flag(item_a) then
*   new poly_coe(item_c)
*   poly_neg(poly_coe(item_a),poly_coe(item_c));
*
*   bgn_coe_flag(item_c) = FALSE
*end if
*
*end proc
**/
UINT32 poly_zn_item_neg(const UINT32 polyzn_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c)
{
    BIGINT *bgn_coe_of_item_c;
    POLY   *poly_coe_of_item_c;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_neg: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_neg: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_neg: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( item_a == item_c )
    {

        return poly_zn_item_self_neg(polyzn_md_id, item_c);
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    /*
    *new deg(item_c)
    *deg(item_c) = deg(item_a);
    */
    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

    /*
    *if TRUE == bgn_coe_flag(item_a) then
    *   new bgn_coe(item_c)
    *   bgn_coe(item_c) = bgn_coe_of_item_c;
    *   bgn_coe_flag(item_c) = TRUE
    *end if
    */
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a))
    {
        poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0044);
        bgn_zn_neg(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), bgn_coe_of_item_c);

        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) then
    *   new poly_coe(item_c)
    *   poly_neg(poly_coe(item_a),poly_coe(item_c));
    *
    *   bgn_coe_flag(item_c) = FALSE
    *end if
    */
    else
    {
        poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0045);
        POLY_INIT(poly_coe_of_item_c);

        poly_zn_poly_neg(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), poly_coe_of_item_c);
        POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
    }

    return ( 0 );
}

/**
*poly_c = - poly_c
*
*poly_neg_0 := proc(poly_c)
*
*item_c = first_item(poly_c);
*
*while item_c != null_item(poly_c) do
*   poly_item_neg_0(item_c);
*   item_c = next_item(poly_c, item_c);
*end do
*
*end proc
**/
static UINT32 poly_zn_poly_self_neg(const UINT32 polyzn_md_id, POLY *poly_c)
{
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_self_neg: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_poly_self_neg: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    item_c = POLY_FIRST_ITEM(poly_c);
    while( item_c != POLY_NULL_ITEM(poly_c) )
    {
        poly_zn_item_self_neg(polyzn_md_id, item_c);

        item_c = POLY_ITEM_NEXT(item_c);
    }
    return ( 0 );
}

/**
*poly_c = - poly_a
*
*poly_neg := proc(poly_a, poly_c)
*
*if poly_a ==  poly_c then
*   poly_neg_0(poly_c);
*   return ;
*end if
*
*item_a = first_item(poly_a)
*
*while item_a != null_item(poly_a) do
*   new item_c
*   poly_item_neg(item_a, item_c);
*
*   insert item_c to the tail of poly_c
*   item_a = next_item(poly_a, item_a);
*
*end do
*
*end proc
**/
UINT32 poly_zn_poly_neg(const UINT32 polyzn_md_id, const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_neg: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_neg: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_poly_neg: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_c )
    {
        return poly_zn_poly_self_neg(polyzn_md_id, poly_c);
    }
    /*
    *item_a = first_item(poly_a)
    *
    *while item_a != null_item(poly_a) do
    *   new item_c
    *   poly_item_neg(item_a, item_c);
    *
    *   insert item_c to the tail of poly_c
    *   item_a = next_item(poly_a, item_a);
    *
    *end do
    */

    item_a = POLY_FIRST_ITEM(poly_a);
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0046);

        poly_zn_item_neg(polyzn_md_id, item_a, item_c);

        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_a = POLY_ITEM_NEXT(item_a);
    }
    return ( 0 );
}

/**
*poly_c = poly_c - poly_a
*
*poly_sbb_1:= proc(poly_a, poly_c)
*
*if poly_a == poly_c then
*   poly_destory(poly_c)
*end if
*
*>>> from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_c = first_item(poly_c);
*
*while item_a != null_item(poly_a) && item_c != null_item(poly_c) do
*
*if deg(item_a) == deg(item_c) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) - bgn_coe(item_a);
*
*        if bgn_coe(item_c) is zero then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
*        poly_sbb_const_2(poly_coe(item_c),bgn_coe(item_a), poly_coe(item_c));
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
*        new tmp_poly_c
*        poly_sbb_const_1(bgn_coe(item_c), poly_coe(item_a), tmp_poly_c);
*
*        del bgn_coe(item_c)
*
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*        poly_sub(poly_coe(item_c), poly_coe(item_a), poly_coe(item_c));
*
*        if poly_coe(item_c) is empty then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_c = next_item(poly_c, item_c);
*end if;
*
*if deg(item_a) < deg(item_c) then
*    new tmp_item_c
*
*    poly_item_neg(item_a,item_c)
*    prev_item(poly_c,item_c) = tmp_item_c;
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_c) then
*    item_c = next_item(poly_c, item_c);
*end if;
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new tmp_item_c
*
*    poly_item_neg(item_a,item_c)
*    insert tmp_item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*end proc;
**/
static UINT32 poly_zn_sbb_c_a(const UINT32 polyzn_md_id, const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;
    BIGINT    *bgn_coe_of_item_c;
    POLY      *poly_coe_of_item_c;

    UINT32 cmp_result;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sbb_c_a: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sbb_c_a: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_sbb_c_a: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *   poly_destory(poly_c)
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_zn_poly_clean(polyzn_md_id, poly_c);
    }


    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    /* from lower degree item to higher degree item */

    /*
    *item_a = first_item(poly_a);
    *item_c = first_item(poly_c);
    */
    item_a = POLY_FIRST_ITEM(poly_a);
    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_a != POLY_NULL_ITEM(poly_a) && item_c != POLY_NULL_ITEM(poly_c) )
    {
        cmp_result = POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
            *        bgn_coe(item_c) = bgn_coe(item_c) - bgn_coe(item_a);
            *
            *        if bgn_coe(item_c) is zero then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
                bgn_zn_sub(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c));

                if (EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_zn_item_destory(polyzn_md_id, item_t);
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
            *        poly_sbb_const_2(poly_coe(item_c),bgn_coe(item_a), poly_coe(item_c));
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
                poly_zn_sbb_c_a(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
            *        new tmp_poly_c
            *        poly_sbb_const_1(bgn_coe(item_c), poly_coe(item_a), tmp_poly_c);
            *
            *        del bgn_coe(item_c)
            *
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
                poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0047);
                POLY_INIT(poly_coe_of_item_c);

                poly_zn_sub_bgn_poly(polyzn_md_id, POLY_ITEM_BGN_COE(item_c), POLY_ITEM_POLY_COE(item_a), poly_coe_of_item_c);

                bgn_coe_of_item_c = POLY_ITEM_BGN_COE(item_c);
                poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZN_0048);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
            *        poly_sub(poly_coe(item_c), poly_coe(item_a), poly_coe(item_c));
            *
            *        if poly_coe(item_c) is empty then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *
            *    end if
            *
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
                poly_zn_sub_poly(polyzn_md_id, POLY_ITEM_POLY_COE(item_c), POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));
                if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_zn_item_destory(polyzn_md_id, item_t);
                }
            }
            else
            {
                ;
            }

            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_c = next_item(poly_c, item_c);
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_c = POLY_ITEM_NEXT(item_c);
        }
        /*
        *if deg(item_a) < deg(item_c) then
        *    new tmp_item_c
        *
        *    poly_item_neg(item_a,item_c)
        *    prev_item(poly_c,item_c) = tmp_item_c;
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( ((UINT32)-1) == cmp_result )
        {
            poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_t, LOC_POLYZN_0049);

            poly_zn_item_neg(polyzn_md_id, item_a, item_t);
            POLY_ITEM_ADD_PREV(item_c, item_t);

            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_c) then
        *    item_c = next_item(poly_c, item_c);
        *end if;
        *
        *end do;
        */
        if ( 1 == cmp_result )
        {
            item_c = POLY_ITEM_NEXT(item_c);
        }
    }
    /*
    *while item_a != null_item(poly_a) do
    *    new tmp_item_c
    *
    *    poly_item_neg(item_a,item_c)
    *    insert tmp_item_c to the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    *
    **/
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_t, LOC_POLYZN_0050);

        poly_zn_item_neg(polyzn_md_id, item_a, item_t);
        POLY_ADD_ITEM_TAIL(poly_c, item_t);

        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

/**
*poly_c = poly_a - poly_c
*
*poly_sbb_2 :=proc(poly_a,poly_c)
*
*if poly_a == poly_c then
*   poly_destory(poly_c)
*end if
*
* from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_c = first_item(poly_c);
*
*while item_a != null_item(poly_a) && item_c != null_item(poly_c) do
*
*if deg(item_a) == deg(item_c) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_a) - bgn_coe(item_c);
*
*        if bgn_coe(item_c) is zero then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
*        poly_sbb_const_1(bgn_coe(item_a), poly_coe(item_c),poly_coe(item_c));
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
*        new tmp_poly_c
*        poly_sbb_const_2(poly_coe(item_a), bgn_coe(item_c), tmp_poly_c);
*
*        del bgn_coe(item_c)
*
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*        poly_sub(poly_coe(item_a), poly_coe(item_c), poly_coe(item_c));
*
*        if poly_coe(item_c) is empty then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_c = next_item(poly_c, item_c);
*end if;
*
*if deg(item_a) < deg(item_c) then
*    new tmp_item_c
*
*    poly_item_clone(item_a,tmp_item_c)
*    prev_item(poly_c,item_c) = tmp_item_c;
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_c) then
*    item_c = next_item(poly_c, item_c);
*end if;
*
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new tmp_item_c
*
*    poly_item_clone(item_a,tmp_item_c)
*    insert tmp_item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*while item_c != null_item(poly_c) do
*
*    poly_item_neg(item_c,item_c);
*
*    item_c = next_item(poly_c, item_c);
*end do
*end proc
*
**/
static UINT32 poly_zn_sbb_a_c(const UINT32 polyzn_md_id, const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;
    BIGINT    *bgn_coe_of_item_c;
    POLY      *poly_coe_of_item_c;
    UINT32 cmp_result;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sbb_a_c: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sbb_a_c: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_sbb_a_c: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *   poly_destory(poly_c)
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_zn_poly_clean(polyzn_md_id, poly_c);
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    /* from lower degree item to higher degree item */
    /*
    *item_a = first_item(poly_a);
    *item_c = first_item(poly_c);
    */
    item_a = POLY_FIRST_ITEM(poly_a);
    item_c = POLY_FIRST_ITEM(poly_c);

    /*while item_a != null_item(poly_a) && item_c != null_item(poly_c) do*/
    while( item_a != POLY_NULL_ITEM(poly_a) && item_c != POLY_NULL_ITEM(poly_c) )
    {
        cmp_result = POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));
        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
            *        bgn_coe(item_c) = bgn_coe(item_a) - bgn_coe(item_c);
            *
            *        if bgn_coe(item_c) is zero then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                bgn_zn_sub(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));

                if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_zn_item_destory(polyzn_md_id, item_t);
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
            *        poly_sbb_const_1(bgn_coe(item_a), poly_coe(item_c),poly_coe(item_c));
            *
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_zn_sub_bgn_poly(polyzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_c), POLY_ITEM_POLY_COE(item_c));
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
            *        new tmp_poly_c
            *        poly_sbb_const_2(poly_coe(item_a), bgn_coe(item_c), tmp_poly_c);
            *
            *        del bgn_coe(item_c)
            *
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0051);
                POLY_INIT(poly_coe_of_item_c);

                poly_zn_sub_poly_bgn(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_c), poly_coe_of_item_c);

                bgn_coe_of_item_c = POLY_ITEM_BGN_COE(item_c);
                poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZN_0052);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
            *        poly_sub(poly_coe(item_a), poly_coe(item_c), poly_coe(item_c));
            *
            *        if poly_coe(item_c) is empty then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_zn_sub_poly(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c), POLY_ITEM_POLY_COE(item_c));

                if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_zn_item_destory(polyzn_md_id, item_t);
                }
            }
            else
            {
                ;
            }
            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_c = next_item(poly_c, item_c);
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_c = POLY_ITEM_NEXT(item_c);
        }
        /*
        *if deg(item_a) < deg(item_c) then
        *    new tmp_item_c
        *
        *    poly_item_clone(item_a,tmp_item_c)
        *    prev_item(poly_c,item_c) = tmp_item_c;
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( ((UINT32)-1) == cmp_result )
        {
            poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_t, LOC_POLYZN_0053);

            poly_zn_item_clone(polyzn_md_id, item_a, item_t);
            POLY_ITEM_ADD_PREV(item_c, item_t);

            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_c) then
        *    item_c = next_item(poly_c, item_c);
        *end if;
        */
        if ( 1 == cmp_result )
        {
            item_c = POLY_ITEM_NEXT(item_c);
        }
    }
    /*
    *while item_a != null_item(poly_a) do
    *    new tmp_item_c
    *
    *    poly_item_clone(item_a,tmp_item_c)
    *    insert tmp_item_c to the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    */
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_t, LOC_POLYZN_0054);
        poly_zn_item_clone(polyzn_md_id, item_a, item_t);

        POLY_ADD_ITEM_TAIL(poly_c, item_t);

        item_a = POLY_ITEM_NEXT(item_a);
    }
    /*
    *while item_c != null_item(poly_c) do
    *
    *    poly_item_neg(item_c,item_c);
    *
    *    item_c = next_item(poly_c, item_c);
    *end do
    */
    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        poly_zn_item_neg(polyzn_md_id, item_c, item_c);

        item_c = POLY_ITEM_NEXT(item_c);
    }
    return ( 0 );
}

/*
*poly_sub:= proc(poly_a, poly_b, poly_c)
*
*if poly_c == poly_a then
*     c = c - b
*poly_sbb_1(poly_b,poly_c)
*end if
*
*if poly_c == poly_b then
*     c = a - c
*poly_sbb_2(poly_a,poly_c)
*end if
*
*
*>> from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_b = first_item(poly_b);
*while item_a != null_item(poly_a) && item_b != null_item(poly_b) do
*
*if deg(item_a) == deg(item_b) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
*        bgn_coe_c = bgn_coe(item_a) - bgn_coe(item_b);
*
*        if bgn_coe_c is nonzero then
*            new item_c
*            deg(item_c) = deg(item_a);
*            bgn_coe(item_c) = bgn_coe_c;
*            bgn_coe_flag(item_c) = TRUE;
*            insert item_c to tail of poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a)  and FALSE == bgn_coe_flag(item_b) then
*        new tmp_poly_c
*        >>>tmp_poly_c = bgn_coe(item_a) - poly_coe(item_b)
*        poly_sbb_const_1(bgn_coe(item_a), poly_coe(item_b),tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    if FALSE == bgn_coe_flag(item_a)  and TRUE == bgn_coe_flag(item_b)
*        new tmp_poly_c
*        >>> tmp_poly_c = poly_coe(item_a) bgn_coe(item_b)
*        poly_sbb_const_2(poly_coe(item_a), bgn_coe(item_b), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
*        new tmp_poly_c
*        poly_sub(poly_coe(item_a), poly_coe(item_b), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_b = next_item(poly_b, item_b);
*end if;
*
*if deg(item_a) < deg(item_b) then
*    new item_c
*    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_b) then
*    new item_c
*    poly_item_neg(item_b, item_c)
*
*    insert item_c to the tail of poly_c
*    item_b = next_item(poly_b, item_b);
*end if;
*
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new item_c
*
*    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*while item_b != null_item(poly_b) do
*    new item_c
*
*    poly_item_neg(item_b, item_c)
*
*    insert item_c to the tail of poly_c
*    item_b = next_item(poly_b, item_b);
*end do
*
*end proc;
*/
UINT32 poly_zn_sub_poly(const UINT32 polyzn_md_id, const POLY *poly_a, const POLY *poly_b, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_b;
    POLY_ITEM *item_c;

    BIGINT *bgn_coe_of_item_c;
    POLY   *poly_coe_of_item_c;

    UINT32 cmp_result;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sub_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sub_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sub_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_sub_poly: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_c == poly_a then
    *     c = c - b
    *poly_sbb_1(poly_b,poly_c)
    *end if
    */
    if ( poly_c == poly_a )
    {
        return poly_zn_sbb_c_a(polyzn_md_id, poly_b, poly_c);
    }

    /*
    *if poly_c == poly_b then
    *     c = a - c
    *poly_sbb_2(poly_a,poly_c)
    *end if
    */
    if ( poly_c == poly_b )
    {
        return poly_zn_sbb_a_c(polyzn_md_id, poly_a, poly_c);
    }

    /*clean all items of the poly_c*/
    poly_zn_poly_clean(polyzn_md_id, poly_c);

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    /* from lower degree item to higher degree item */

    /*
    *item_a = first_item(poly_a);
    *item_b = first_item(poly_b);
    */
    item_a = POLY_FIRST_ITEM(poly_a);
    item_b = POLY_FIRST_ITEM(poly_b);

    /*while item_a != null_item(poly_a) && item_b != null_item(poly_b) do */
    while( item_a != POLY_NULL_ITEM(poly_a) && item_b != POLY_NULL_ITEM(poly_b))
    {
        cmp_result = POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_b));

        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
            *        bgn_coe_c = bgn_coe(item_a) - bgn_coe(item_b);
            *
            *        if bgn_coe_c is nonzero then
            *            new item_c
            *            deg(item_c) = deg(item_a);
            *            bgn_coe(item_c) = bgn_coe_c;
            *            bgn_coe_flag(item_c) = TRUE;
            *            insert item_c to tail of poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
            {
                if ( 0 != bgn_zn_cmp(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b)))
                {
                    poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0055);
                    bgn_zn_sub(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b), bgn_coe_of_item_c);

                    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0056);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a)  and FALSE == bgn_coe_flag(item_b) then
            *        new tmp_poly_c
            *        >>>tmp_poly_c = bgn_coe(item_a) - poly_coe(item_b)
            *        poly_sbb_const_1(bgn_coe(item_a), poly_coe(item_b),tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
            {
                poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0057);
                POLY_INIT(poly_coe_of_item_c);

                poly_zn_sub_bgn_poly(polyzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);

                poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0058);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a)  and TRUE == bgn_coe_flag(item_b)
            *        new tmp_poly_c
            *        >>> tmp_poly_c = poly_coe(item_a) bgn_coe(item_b)
            *        poly_sbb_const_2(poly_coe(item_a), bgn_coe(item_b), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
            {
                poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0059);
                POLY_INIT(poly_coe_of_item_c);

                poly_zn_sub_poly_bgn(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_b), poly_coe_of_item_c);

                poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0060);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
            *        new tmp_poly_c
            *        poly_sub(poly_coe(item_a), poly_coe(item_b), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
            {
                poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0061);
                POLY_INIT(poly_coe_of_item_c);

                poly_zn_sub_poly(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);
                if ( EC_FALSE == POLY_IS_EMPTY(poly_coe_of_item_c) )
                {
                    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0062);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
                else
                {
                    poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZN_0063);
                    poly_coe_of_item_c = NULL_PTR;
                }
            }
            else
            {
                ;
            }
            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_b = next_item(poly_b, item_b);
            *
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_b = POLY_ITEM_NEXT(item_b);
        }

        /*
        *if deg(item_a) < deg(item_b) then
        *    new item_c
        *    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
        *    insert item_c to the tail of poly_c
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( ((UINT32)-1) == cmp_result )
        {
            poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0064);

            poly_zn_item_clone(polyzn_md_id, item_a, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);
            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_b) then
        *    new item_c
        *    poly_item_neg(item_b, item_c)
        *
        *    insert item_c to the tail of poly_c
        *    item_b = next_item(poly_b, item_b);
        *end if;
        */
        if ( 1 == cmp_result )
        {
            poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0065);

            poly_zn_item_neg(polyzn_md_id, item_b, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);
            item_b = POLY_ITEM_NEXT(item_b);
        }
    }
    /*
    *while item_a != null_item(poly_a) do
    *    new item_c
    *
    *    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
    *    insert item_c to the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    */
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0066);

        poly_zn_item_clone(polyzn_md_id, item_a, item_c);

        POLY_ADD_ITEM_TAIL(poly_c, item_c);
        item_a = POLY_ITEM_NEXT(item_a);
    }
    /*
    *while item_b != null_item(poly_b) do
    *    new item_c
    *
    *    poly_item_neg(item_b, item_c)
    *
    *    insert item_c to the tail of poly_c
    *    item_b = next_item(poly_b, item_b);
    *end do
    *
    */
    while ( item_b != POLY_NULL_ITEM(poly_b) )
    {
        poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0067);

        poly_zn_item_neg(polyzn_md_id, item_b, item_c);

        POLY_ADD_ITEM_TAIL(poly_c, item_c);
        item_b = POLY_ITEM_NEXT(item_b);
    }

    return ( 0 );
}

UINT32 poly_zn_poly_output(const UINT32 polyzn_md_id,const POLY *poly, const UINT32 depth,const char *info)
{
    POLY_ITEM *item;
    UINT8  space_str[32];
    UINT32 index;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_output: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_poly_output: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( NULL_PTR != info )
    {
        sys_log(LOGSTDOUT,"%s: ",info);
    }

    if ( depth >= 32 )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_output: depth = %ld overflow\n");
        return ((UINT32)(-1));
    }

    for ( index = 0; index < depth; index ++ )
    {
        space_str[ index ] = ' ';
    }
    space_str[ index ] = '\0';

    item = POLY_FIRST_ITEM(poly);

    while( item != POLY_NULL_ITEM(poly) )
    {
        sys_log(LOGSTDOUT,"%s deg: ", space_str);
        print_deg_dec(LOGSTDOUT, POLY_ITEM_DEG(item));

        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
        {
            sys_log(LOGSTDOUT,"%s coe: ", space_str);
            print_bigint_dec(LOGSTDOUT, POLY_ITEM_BGN_COE(item));
        }
        else
        {
            sys_log(LOGSTDOUT,"%s coe: \n", space_str);
            poly_zn_poly_output(polyzn_md_id, POLY_ITEM_POLY_COE(item), depth + 1, NULL_PTR);
        }

        item = POLY_ITEM_NEXT(item);
    }

    return ( 0 );
}

/**
*move item_a to item_c
*
*poly_item_move:=proc(item_a, item_c)
*
*deg(item_c) = deg(item_a);
*
*if TRUE == bgn_coe_flag(item_a) then
*    bgn_coe(item_c) = bgn_coe(item_a);
*    bgn_coe_flag(item_c) = TRUE;
*
*    bgn_coe(item_a) = NULL_PTR;
*end if
*
*if FALSE == bgn_coe_flag(item_a) then
*    poly_coe(item_c) = poly_coe(item_a);
*    bgn_coe_flag(item_c) = FALSE;
*
*    poly_coe(item_a) = NULL_PTR;
*end if
*
*end proc
**/

UINT32 poly_zn_item_move(const UINT32 polyzn_md_id, POLY_ITEM *item_a, POLY_ITEM *item_c)
{
    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_move: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_move: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_move: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
    {
        POLY_ITEM_BGN_COE(item_c) = POLY_ITEM_BGN_COE(item_a);
        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
        POLY_ITEM_BGN_COE(item_a) = NULL_PTR;
    }
    else
    {
        POLY_ITEM_POLY_COE(item_c) = POLY_ITEM_POLY_COE(item_a);
        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        POLY_ITEM_POLY_COE(item_a) = NULL_PTR;
    }

    return ( 0 );
}

/**
*move poly_a to poly_c
*
*poly_move:=proc(poly_a, poly_c)
*
*init poly_c
*
*item_a = first_item(poly_a);
*
*while item_a != null_item(poly_a) do
*
*    remove item_a from poly_a;
*    insert item_a to the tail of poly_c;
*
*    item_a = first_item(poly_a);
*end do
*
*end proc
**/
UINT32 poly_zn_poly_move(const UINT32 polyzn_md_id, POLY *poly_a, POLY *poly_c)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_move: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_poly_move: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_poly_move: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a != poly_c )
    {
        poly_zn_poly_clean(polyzn_md_id, poly_c);
        //POLY_INIT(poly_c);
        if ( EC_FALSE == POLY_IS_EMPTY(poly_a) )
        {
            POLY_ITEM_HEAD(poly_c)->next = POLY_ITEM_HEAD(poly_a)->next;
            POLY_ITEM_HEAD(poly_c)->next->prev = POLY_ITEM_HEAD(poly_c);

            POLY_ITEM_HEAD(poly_c)->prev = POLY_ITEM_HEAD(poly_a)->prev;
            POLY_ITEM_HEAD(poly_c)->prev->next = POLY_ITEM_HEAD(poly_c);
#if 0
            poly_c->poly_item_head.next = poly_a->poly_item_head.next;
            poly_c->poly_item_head.next->prev = &(poly_c->poly_item_head);

            poly_c->poly_item_head.prev = poly_a->poly_item_head.prev;
            poly_c->poly_item_head.prev->next = &(poly_c->poly_item_head);
#endif
            POLY_INIT(poly_a);
        }
    }
    return ( 0 );
}

/**
*poly_c = poly_c + item_a
*poly_item_insert := proc(item_a, poly_c)
*
*item_c = first_item(poly_c)
*
*while item_c != null_item(poly_c) do
*
*    if deg(item_c) >= deg(item_a) then
*        break;
*    end if
*
*    item_c = next_item(item_c);
*end do
*
*if item_c == null_item(poly_c) then
*    insert item_a to the tail of poly_c
*    return;
*end if
*
*if deg(item_c) > deg(item_a) then
*    insert iteam_a to the prev of item_c;
*    return;
*end if
*
*>>> now deg(item_c) == deg(item_a)
*
*if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*
*    bgn_coe(item_c) = bgn_coe(item_c) + bgn_coe(item_a);
*
*    if bgn_coe(item_c) is zero then
*        remove item_c from poly_c;
*        destory item_c;
*    end if
*
*end if
*
*
*if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*    poly_coe(item_c) = poly_coe(item_c) + bgn_coe(item_a);
*
*    if poly_coe(item_c) is empty then
*        remove item_c from poly_c;
*        destory item_c;
*    end if
*end if
*
*
*if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*    new poly_coe_of_item_c;
*    init poly_coe_of_item_c;
*
*    poly_coe_of_item_c = poly_coe(item_a) + bgn_coe(item_c);
*
*    if poly_coe_of_item_c is empty then
*
*        destory poly_coe_of_item_c
*        remove item_c from poly_c
*        destory item_c
*    end if
*
*    if poly_coe_of_item_c is not empty then
*
*        free bgn_coe(item_c);
*        poly_coe(item_c) = poly_coe_of_item_c;
*
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*
*end if
*
*if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*
*    poly_coe(item_c) = poly_coe(item_c) + poly_coe(item_a);
*
*    if poly_coe(item_c) is empty then
*        remove item_c from poly_c
*        destory item_c;
*    end if
*end if
*
*end proc
*
*
*Note:
*
*   After insertion, the item_a should not be referred any more!
*
**/
static UINT32 poly_zn_item_insert(const UINT32 polyzn_md_id, POLY_ITEM *item_a, POLY *poly_c)
{
    POLY_ITEM *item_c;
    POLY      *poly_coe_of_item_c;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_insert: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_insert: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_insert: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    /**
    *item_c = first_item(poly_c)
    *
    *while item_c != null_item(poly_c) do
    *
    *    if deg(item_c) >= deg(item_a) then
    *        break;
    *    end if
    *
    *    item_c = next_item(item_c);
    *end do
    */

    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        if ( 0 <= POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_a)) )
        {
            break;
        }
        item_c = POLY_ITEM_NEXT(item_c);
    }

    /*
    *if item_c == null_item(poly_c) then
    *    insert item_a to the tail of poly_c
    *    return;
    *end if
    */
    if ( item_c == POLY_NULL_ITEM(poly_c) )
    {
        POLY_ADD_ITEM_TAIL(poly_c, item_a);
        return ( 0 );
    }

    /*
    *if deg(item_c) > deg(item_a) then
    *    insert iteam_a to the prev of item_c;
    *    return;
    *end if
    */
    if ( 0 < POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_a)) )
    {
        POLY_ITEM_ADD_PREV(item_c, item_a);
        return ( 0 );
    }

    /* now deg(item_c) == deg(item_a) */

    /*
    *if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
    *
    *    bgn_coe(item_c) = bgn_coe(item_c) + bgn_coe(item_a);
    *
    *    if bgn_coe(item_c) is zero then
    *        remove item_c from poly_c;
    *        destory item_c;
    *    end if
    *
    *end if
    */
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        bgn_zn_add(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));

        if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c)))
        {
            POLY_ITEM_DEL(item_c);
            poly_zn_item_destory(polyzn_md_id, item_c);
        }
    }

    /*
    *if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
    *    poly_coe(item_c) = poly_coe(item_c) + bgn_coe(item_a);
    *
    *    if poly_coe(item_c) is empty then
    *        remove item_c from poly_c;
    *        destory item_c;
    *    end if
    *end if
    */
    else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        poly_zn_adc_n(polyzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_c));

        if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
        {
            POLY_ITEM_DEL(item_c);
            poly_zn_item_destory(polyzn_md_id, item_c);
        }
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
    *    new poly_coe_of_item_c;
    *    init poly_coe_of_item_c;
    *
    *    poly_coe_of_item_c = poly_coe(item_a) + bgn_coe(item_c);
    *
    *    if poly_coe_of_item_c is empty then
    *
    *        destory poly_coe_of_item_c
    *        remove item_c from poly_c
    *        destory item_c
    *    end if
    *
    *    if poly_coe_of_item_c is not empty then
    *
    *        free bgn_coe(item_c);
    *        poly_coe(item_c) = poly_coe_of_item_c;
    *
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *
    *end if
    */
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0068);
        POLY_INIT(poly_coe_of_item_c);

        poly_zn_add_n(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_c), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZN_0069);

            POLY_ITEM_DEL(item_c);
            poly_zn_item_destory(polyzn_md_id, item_c);
        }
        else
        {
            poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, POLY_ITEM_BGN_COE(item_c), LOC_POLYZN_0070);

            POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
    *
    *    poly_coe(item_c) = poly_coe(item_c) + poly_coe(item_a);
    *
    *    if poly_coe(item_c) is empty then
    *        remove item_c from poly_c
    *        destory item_c;
    *    end if
    *end if
    */
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        poly_zn_adc_poly(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));

        if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)) )
        {
            POLY_ITEM_DEL(item_c);

            poly_zn_item_destory(polyzn_md_id, item_c);
        }
    }
    else
    {
        ;
    }

    poly_zn_item_destory(polyzn_md_id, item_a);

    return ( 0 );
}

/**
*item_c = item_a * item_b
*
*poly_item_mul :=proc(item_a, item_b, item_c)
*
*if item_a == item_c then
*    poly_item_mul_self(item_b, item_c);
*    return ;
*end if
*
*
*if item_b == item_c then
*    poly_item_mul_self(item_a, item_c);
*    return ;
*end if
*
*(carry,deg(item_c)) = deg(item_a) + deg(item_b);
*
*if carry > 0 then
*    error("degree overflow");
*    exit( 0 );
*end if
*
*if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
*    new bgn_coe_of_item_c
*    bgn_coe_of_item_c = bgn_coe(item_a) * bgn_coe(item_b);
*
*    if bgn_coe_of_item_c is zero then
*        free bgn_coe_of_item_c;
*        bgn_coe(item_c) = NULL_PTR;
*        bgn_coe_flag(item_c) = TRUE;
*    end if
*
*    if bgn_coe_of_item_c is nonzero then
*        bgn_coe(item_c) = bgn_coe_of_item_c
*        bgn_coe_flag(item_c) = TRUE;
*    end if
*
*endif
*
*if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
*
*    new poly_coe_of_item_c;
*
*    poly_mul_bgn(poly_coe(item_b), bgn_coe(item_a), poly_coe_of_item_c);
*
*    if poly_coe_of_item_c is empty then
*        free poly_coe_of_item_c;
*
*        poly_coe(item_c) = NULL_PTR;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*
*    if poly_coe_of_item_c is not empty then
*        poly_coe(item_c) = poly_coe_of_item_c;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*endif
*
*if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
*    new poly_coe_of_item_c;
*
*    poly_mul_bgn(poly_coe(item_a), bgn_coe(item_b), poly_coe_of_item_c);
*
*    if poly_coe_of_item_c is empty then
*        free poly_coe_of_item_c;
*        poly_coe(item_c) = NULL_PTR;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*
*    if poly_coe_of_item_c is not empty then
*        poly_coe(item_c) = poly_coe_of_item_c;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*endif
*
*
*if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
*    new poly_coe_of_item_c;
*
*    poly_mul(poly_coe(item_a), poly_coe(item_b), poly_coe_of_item_c);
*
*    if poly_coe_of_item_c is empty then
*        free poly_coe_of_item_c;
*        poly_coe(item_c) = NULL_PTR;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*
*    if poly_coe_of_item_c is not empty then
*        poly_coe(item_c) = poly_coe_of_item_c;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*endif
*
**end proc
**/
static UINT32 poly_zn_item_mul(const UINT32 polyzn_md_id, const POLY_ITEM *item_a, const POLY_ITEM *item_b,POLY_ITEM *item_c)
{
    BIGINT *bgn_coe_of_item_c;
    POLY   *poly_coe_of_item_c;
    UINT32 carry;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_mul: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == item_b )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_mul: item_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_mul: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_mul: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /**
    *if item_a == item_c then
    *    poly_item_mul_self(item_b, item_c);
    *    return ;
    *end if
    */
    if ( item_a == item_c )
    {
        poly_zn_item_mul_self( polyzn_md_id, item_b, item_c);
        return ( 0 );
    }

    /*
    *if item_b == item_c then
    *    poly_item_mul_self(item_a, item_c);
    *    return ;
    *end if
    */
    if ( item_b == item_c )
    {
        poly_zn_item_mul_self( polyzn_md_id, item_a, item_c);
        return ( 0 );
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    /*
    *(carry,deg(item_c)) = deg(item_a) + deg(item_b);
    *
    *if carry > 0 then
    *    error("degree overflow");
    *    exit( 0 );
    *end if
    */
    carry = POLY_ITEM_DEG_ADD(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_b), POLY_ITEM_DEG(item_c));
    if ( 0 < carry )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_mul: degree overflow.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }

    /*
    *if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
    *    new bgn_coe_of_item_c
    *    bgn_coe_of_item_c = bgn_coe(item_a) * bgn_coe(item_b);
    *
    *    if bgn_coe_of_item_c is zero then
    *        free bgn_coe_of_item_c;
    *        bgn_coe(item_c) = NULL_PTR;
    *        bgn_coe_flag(item_c) = TRUE;
    *    end if
    *
    *    if bgn_coe_of_item_c is nonzero then
    *        bgn_coe(item_c) = bgn_coe_of_item_c
    *        bgn_coe_flag(item_c) = TRUE;
    *    end if
    *
    *endif
    */
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0071);

        bgn_zn_mul(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b), bgn_coe_of_item_c);

        if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, bgn_coe_of_item_c))
        {
            poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZN_0072);

            POLY_ITEM_BGN_COE(item_c) = NULL_PTR;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
        }
        else
        {
            POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
        }

        return ( 0 );
    }

    /*
    *if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
    *
    *    new poly_coe_of_item_c;
    *
    *    poly_mul_bgn(poly_coe(item_b), bgn_coe(item_a), poly_coe_of_item_c);
    *
    *    if poly_coe_of_item_c is empty then
    *        free poly_coe_of_item_c;
    *
    *        poly_coe(item_c) = NULL_PTR;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *
    *    if poly_coe_of_item_c is not empty then
    *        poly_coe(item_c) = poly_coe_of_item_c;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *endif
    */
    else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0073);
        POLY_INIT(poly_coe_of_item_c);

        poly_zn_mul_bgn(polyzn_md_id, POLY_ITEM_POLY_COE(item_b), POLY_ITEM_BGN_COE(item_a), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZN_0074);

            POLY_ITEM_POLY_COE(item_c) = NULL_PTR;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }
        else
        {
            POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }

        return ( 0 );
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
    *    new poly_coe_of_item_c;
    *
    *    poly_mul_bgn(poly_coe(item_a), bgn_coe(item_b), poly_coe_of_item_c);
    *
    *    if poly_coe_of_item_c is empty then
    *        free poly_coe_of_item_c;
    *        poly_coe(item_c) = NULL_PTR;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *
    *    if poly_coe_of_item_c is not empty then
    *        poly_coe(item_c) = poly_coe_of_item_c;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *endif
    */
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0075);
        POLY_INIT(poly_coe_of_item_c);

        poly_zn_mul_bgn(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_b), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZN_0076);

            POLY_ITEM_POLY_COE(item_c) = NULL_PTR;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }
        else
        {
            POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }

        return ( 0 );
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
    *    new poly_coe_of_item_c;
    *
    *    poly_mul(poly_coe(item_a), poly_coe(item_b), poly_coe_of_item_c);
    *
    *    if poly_coe_of_item_c is empty then
    *        free poly_coe_of_item_c;
    *        poly_coe(item_c) = NULL_PTR;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *
    *    if poly_coe_of_item_c is not empty then
    *        poly_coe(item_c) = poly_coe_of_item_c;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *endif
    */
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0077);
        POLY_INIT(poly_coe_of_item_c);

        poly_zn_mul_poly(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZN_0078);

            POLY_ITEM_POLY_COE(item_c) = NULL_PTR;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }
        else
        {
            POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }

        return ( 0 );
    }
    else
    {
        ;
    }

    return ( 0 );
}
/**
*item_c = item_a * item_c
*
*poly_item_mul_self :=proc(item_a, item_c)
*
*new item_b;
*
*mov item_c to item_b;
*
*poly_item_mul(item_a,item_b,item_c);
*
*destory item_b;
*
*end proc
**/
static UINT32 poly_zn_item_mul_self(const UINT32 polyzn_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c)
{
    POLY_ITEM *item_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_mul_self: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_mul_self: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_mul_self: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_b, LOC_POLYZN_0079);

    /*mov item_c to item_b*/
    poly_zn_item_move(polyzn_md_id, item_c, item_b);

    poly_zn_item_mul(polyzn_md_id, item_a, item_b, item_c);

    poly_zn_item_destory(polyzn_md_id, item_b);

    return ( 0 );
}
/**
*item_c = item_c ^ 2
*
*poly_item_squ_self :=proc(item_c)
*
*new item_a;
*
*mov item_c to item_a;
*
*poly_item_mul(item_a,item_a, item_c);
*
*destory item_a;
*
*end proc
**/
static UINT32 poly_zn_item_squ_self(const UINT32 polyzn_md_id, POLY_ITEM *item_c)
{
    POLY_ITEM *item_a;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_item_squ_self: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_item_squ_self: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_a, LOC_POLYZN_0080);

    /*mov item_c to item_a*/
    poly_zn_item_move(polyzn_md_id, item_c, item_a);

    poly_zn_item_mul(polyzn_md_id, item_a, item_a, item_c);

    poly_zn_item_destory(polyzn_md_id, item_a);

    return ( 0 );
}

/**
*poly_c = poly_c * bgn_a
*
*poly_mul_self_bgn := proc(bgn_a, poly_c)
*
*item_c = first_item(poly_c);
*
*while item_c != null_item(poly_c) do
*    if TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) * bgn_a;
*
*        if bgn_coe(item_c) is zero then
*            item_t = item_c;
*            item_c = prev_item(item_c);
*            remove item_t from poly_c;
*            destory item_t
*        end if
*    end if
*    if FALSE == bgn_coe_flag(item_c) then
*        poly_mul_self_bgn(bgn_a, poly_coe(item_c));
*    end if
*
*    item_c = next_item(item_c);
*end do
*
*end proc
**/
static UINT32 poly_zn_mul_self_bgn(const UINT32 polyzn_md_id,const BIGINT *bgn_a, POLY *poly_c)
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;


#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == bgn_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_self_bgn: bgn_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_self_bgn: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_mul_self_bgn: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            bgn_zn_mul(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c), bgn_a, POLY_ITEM_BGN_COE(item_c));

            if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);
                poly_zn_item_destory(polyzn_md_id, item_t);
            }
        }
        else
        {
            poly_zn_mul_self_bgn(polyzn_md_id, bgn_a, POLY_ITEM_POLY_COE(item_c));
        }

        item_c = POLY_ITEM_NEXT(item_c);
    }

    return ( 0 );
}

/**
* poly_c = poly_a * bgn_b
*
*poly_mul_bgn := proc(poly_a, bgn_b, poly_c)
*
*if poly_a == poly_c then
*    poly_mul_self_bgn (bgn_b, poly_c);
*    return ;
*end if
*
*item_a = first_item(poly_a);
*
*while item_a != null_item(poly_a) do
*
*    if TRUE == bgn_coe_flag(item_a) then
*        new bgn_coe_of_item_c ;
*        bgn_coe_of_item_c = bgn_coe(item_a) * bgn_b;
*
*        if bgn_coe_of_item_c is zero then
*            free bgn_coe_of_item_c     ;
*        end if
*
*        if bgn_coe_of_item_c is nonzero then
*            new item_c;
*
*            deg(item_c) = deg(item_a);
*            bgn_coe(item_c) = bgn_coe_of_item_c ;
*            bgn_coe_flag(item_c) = TRUE;
*
*            add item_c to the tail of poly_c
*        end if
*    end if
*    if FALSE == bgn_coe_flag(item_a) then
*        new poly_coe_of_item_c;
*
*        init poly_coe_of_item_c;
*
*        poly_mul_bgn(poly_coe(item_a),bgn_b, poly_coe_of_item_c);
*
*        if poly_coe_of_item_c is empty then
*            free poly_coe_of_item_c
*        end if
*
*        if poly_coe_of_item_c is not empty then
*            new item_c
*
*            deg(item_c) = deg(item_a);
*            poly_coe(item_c) = poly_coe_of_item_c;
*            bgn_coe_flag(item_c) = FALSE;
*
*            add item_c to the tail of poly_c
*        end if
*    end if
*
*    item_a = next_item(item_a);
*end do
*
*end proc
**/
static UINT32 poly_zn_mul_bgn(const UINT32 polyzn_md_id,const POLY *poly_a,const BIGINT *bgn_b, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    BIGINT    *bgn_coe_of_item_c;
    POLY      *poly_coe_of_item_c;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_bgn: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == bgn_b )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_bgn: bgn_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_bgn: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_mul_bgn: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    /**
    *
    *if poly_a == poly_c then
    *    poly_mul_self_bgn (bgn_b, poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        poly_zn_mul_self_bgn(polyzn_md_id, bgn_b, poly_c);
        return ( 0 );
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    item_a = POLY_FIRST_ITEM(poly_a);

    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*
        *    if TRUE == bgn_coe_flag(item_a) then
        *        new bgn_coe_of_item_c ;
        *        bgn_coe_of_item_c = bgn_coe(item_a) * bgn_b;
        *
        *        if bgn_coe_of_item_c is zero then
        *            free bgn_coe_of_item_c     ;
        *        end if
        *
        *        if bgn_coe_of_item_c is nonzero then
        *            new item_c;
        *
        *            deg(item_c) = deg(item_a);
        *            bgn_coe(item_c) = bgn_coe_of_item_c ;
        *            bgn_coe_flag(item_c) = TRUE;
        *
        *            add item_c to the tail of poly_c
        *        end if
        *    end if
        */
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
        {
            poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0081);

            bgn_zn_mul(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), bgn_b, bgn_coe_of_item_c);

            if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, bgn_coe_of_item_c) )
            {
                poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZN_0082);
            }
            else
            {
                poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0083);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
        }
        /*
        *    if FALSE == bgn_coe_flag(item_a) then
        *        new poly_coe_of_item_c;
        *
        *        init poly_coe_of_item_c;
        *
        *        poly_mul_bgn(poly_coe(item_a),bgn_b, poly_coe_of_item_c);
        *
        *        if poly_coe_of_item_c is empty then
        *            free poly_coe_of_item_c
        *        end if
        *
        *        if poly_coe_of_item_c is not empty then
        *            new item_c
        *
        *            deg(item_c) = deg(item_a);
        *            poly_coe(item_c) = poly_coe_of_item_c;
        *            bgn_coe_flag(item_c) = FALSE;
        *
        *            add item_c to the tail of poly_c
        *        end if
        *    end if
        */
        else
        {
            poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0084);
            POLY_INIT(poly_coe_of_item_c);

            poly_zn_mul_bgn(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), bgn_b, poly_coe_of_item_c);

            if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
            {
                poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZN_0085);
            }
            else
            {
                poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0086);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
        }
        /*
        *    item_a = next_item(item_a);
        */
        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

/**
*
*   for partitial derivation usage
*
**/
static UINT32 poly_zn_mul_word(const UINT32 polyzn_md_id,const POLY *poly_a,const UINT32 word_b, POLY *poly_c)
{
    BIGINT *bgn_b;

    POLYZN_MD *polyzn_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_word: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_word: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_mul_word: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnzn_md_id = polyzn_md->bgnzn_md_id;
    poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_b, LOC_POLYZN_0087);

    bgn_zn_set_word(bgnzn_md_id, bgn_b, word_b);
    poly_zn_mul_bgn(polyzn_md_id, poly_a, bgn_b, poly_c);

    poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, bgn_b, LOC_POLYZN_0088);

    return ( 0 );
}

/**
* poly_c = poly_c * poly_a
*
*poly_mul_self := proc(poly_a, poly_c)
*
*new poly_b;
*
*init poly_b;
*
*move poly_c to poly_b;
*
*poly_mul(poly_a, poly_b,poly_c);
*
*destory poly_b;
*
*end proc
**/
static UINT32 poly_zn_mul_self(const UINT32 polyzn_md_id,const POLY *poly_a, POLY *poly_c)
{
    POLY *poly_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_self: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_mul_self: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_c )
    {
        return poly_zn_squ_poly(polyzn_md_id, poly_a, poly_c);
    }
    poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_b, LOC_POLYZN_0089);
    POLY_INIT(poly_b);

    /*move poly_c to poly_b*/
    poly_zn_poly_move(polyzn_md_id, poly_c, poly_b);

    poly_zn_mul_poly(polyzn_md_id, poly_a, poly_b, poly_c);

    poly_zn_poly_destory(polyzn_md_id, poly_b);

    return ( 0 );
}

/**
* poly_c = poly_a * poly_b
*
*poly_mul := proc(poly_a, poly_b, poly_c)
*
*
*if poly_a == poly_c then
*    poly_mul_self(poly_b,poly_c);
*    return ;
*end if
*
*if poly_b == poly_c then
*    poly_mul_self(poly_a,poly_c);
*    return;
*end if
*
*item_a = first_item(poly_a);
*
*while item_a != null_item(poly_a) do
*
*    item_b = first_item(poly_b);
*    while item_b != null_item(poly_b) do
*        new item_c
*
*        poly_item_mul(item_a,item_b,item_c);
*
*        if item_c is empty then
*            free item_c;
*        end if
*
*        if item_c is not empty then
*            poly_item_insert(item_c,poly_c);
*        end if
*       item_b = next_item(item_b);
*    end do
*
*    item_a = next_item(item_a);
*
*end do
*
*end proc
**/
UINT32 poly_zn_mul_poly(const UINT32 polyzn_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_b;
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_mul_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_mul_poly: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    /**
    *
    *if poly_a == poly_c then
    *    poly_mul_self(poly_b,poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        poly_zn_mul_self(polyzn_md_id, poly_b, poly_c);
        return ( 0 );
    }
    /*
    *if poly_b == poly_c then
    *    poly_mul_self(poly_a,poly_c);
    *    return;
    *end if
    */
    if ( poly_b == poly_c )
    {
        poly_zn_mul_self(polyzn_md_id, poly_a, poly_c);
        return ( 0 );
    }

    /*clean all items of the poly_c*/
    poly_zn_poly_clean(polyzn_md_id, poly_c);

    /*
    *item_a = first_item(poly_a);
    */
    item_a = POLY_FIRST_ITEM(poly_a);
    /*
    *while item_a != null_item(poly_a) do
    *
    *    item_b = first_item(poly_b);
    *    while item_b != null_item(poly_b) do
    *        new item_c
    *
    *        poly_item_mul(item_a,item_b,item_c);
    *
    *        if item_c is empty then
    *            free item_c;
    *        end if
    *
    *        if item_c is not empty then
    *            poly_item_insert(item_c,poly_c);
    *        end if
    *       item_b = next_item(item_b);
    *    end do
    *
    *    item_a = next_item(item_a);
    *
    *end do
    *
    **/
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        item_b = POLY_FIRST_ITEM(poly_b);

        while ( item_b != POLY_NULL_ITEM(poly_b) )
        {
            poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0090);

            poly_zn_item_mul(polyzn_md_id, item_a, item_b, item_c);

            if ( EC_TRUE == POLY_ITEM_IS_EMPTY(item_c) )
            {
                poly_zn_free_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, item_c, LOC_POLYZN_0091);
            }
            else
            {
                poly_zn_item_insert(polyzn_md_id, item_c, poly_c);
            }

            item_b = POLY_ITEM_NEXT(item_b);
        }
        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

/**
*
* poly_c = poly_a ^ 2
*
*/
UINT32 poly_zn_squ_poly(const UINT32 polyzn_md_id,const POLY *poly_a,POLY *poly_c)
{
    POLY *poly_ta;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_squ_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_squ_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_squ_poly: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    if ( poly_a == poly_c )
    {
        poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_ta, LOC_POLYZN_0092);
        POLY_INIT(poly_ta);

        poly_zn_poly_move(polyzn_md_id, poly_c, poly_ta);

        poly_zn_mul_poly(polyzn_md_id, poly_ta, poly_ta, poly_c);

        poly_zn_poly_destory(polyzn_md_id, poly_ta);
    }
    else
    {
        poly_zn_mul_poly(polyzn_md_id, poly_a, poly_a, poly_c);
    }

    return ( 0 );
}

/**
* here a,c are polynomials and e is integer
*
*       c = ( a ^ e ) mod n
*       where 0 < a < n and e < 2 ^ WORDSIZE
*
*   Note:
*       let e = 2 * e1 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e, where a is polynomail and e < 2 * WORDSIZE
*   Output: c such that c = ( a ^ e ) mod n
*   1.  set c = 1
*   2.  while ( 1 ) do
*   2.1     if e is odd, then do
*               c = c * a mod n
*   2.2     end if
*   2.3     e  = ( e >> 1)
*   2.4     if e is zero, then break while, end if
*   2.5     a = a ^ 2 mod n
*   2.6     next while
*   3.  return c
*
**/
UINT32 poly_zn_sexp(const UINT32 polyzn_md_id,const POLY *poly_a,const UINT32 e,POLY *poly_c )
{
    POLY *poly_ta;
    UINT32 te;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sexp: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_sexp: c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_sexp: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    sys_log(LOGSTDOUT,"poly_zn_sexp: poly_a :\n");
    print_poly_dec(LOGSTDOUT, poly_a);
    sys_log(LOGSTDOUT,"poly_zn_sexp: e = %ld\n", e);


    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        /*before set zero, poly_zn_set_zero will clean up the items of poly_c*/
        poly_zn_set_zero(polyzn_md_id, poly_c);

        return ( 0 );
    }

    poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_ta, LOC_POLYZN_0093);
    POLY_INIT(poly_ta);

    /*the branch is to decrease sharply the clone action of large number of items*/
    if ( poly_a == poly_c )
    {
        poly_zn_poly_move(polyzn_md_id, poly_c, poly_ta);
    }
    else
    {
        poly_zn_poly_clone(polyzn_md_id, poly_a, poly_ta);
    }

    sys_log(LOGSTDOUT,"poly_zn_sexp: 1\n");

    /*before set one, poly_zn_set_one will clean up the items of poly_c*/
    poly_zn_set_one(polyzn_md_id, poly_c);

    sys_log(LOGSTDOUT,"poly_zn_sexp: 2\n");

    te = e;
    while ( 1 )
    {
        if ( te & 1 )
        {
            poly_zn_mul_self(polyzn_md_id, poly_ta, poly_c);
        }
        te >>= 1;
    	sys_log(LOGSTDOUT,"poly_zn_sexp: 3\n");

        if ( 0 == te )
        {
            break;
        }

        poly_zn_squ_poly(polyzn_md_id, poly_ta, poly_ta);
    	sys_log(LOGSTDOUT,"poly_zn_sexp: 4\n");
    }

    sys_log(LOGSTDOUT,"poly_zn_sexp: poly_c :\n");
    print_poly_dec(LOGSTDOUT, poly_c);

    poly_zn_poly_destory(polyzn_md_id, poly_ta);

    return ( 0 );
}


/**
*
* here a,b,c are polynomials and e is integer
*
*       c = ( a ^ e ) mod n
*       where a is polynomial and e < 2 ^ BIGINTSIZE
*
*   Note:
*       let e = e1 * 2 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e,where a is polynomial
*   Output: c such that c = ( a ^ e ) mod n
*   0.  let e = SUM(e_i * 2 ^ i, i = 0..m)
*   1.  set c = 1
*   2.  set i = 0
*   3.  while ( 1 ) do
*   3.1     if e_i is odd, then do
*               c = c * a mod n
*   3.2     end if
*   3.3     i = i + 1
*   3.4     if i = m + 1, then break while, end if
*   3.5     a = a ^ 2 mod n
*   3.6     next while
*   4.  return c
*
**/
UINT32 poly_zn_exp(const UINT32 polyzn_md_id,const POLY *poly_a,const BIGINT *e,POLY *poly_c )
{
    POLY   *poly_ta;

    UINT32 te_nbits;
    UINT32 e_bit;
    UINT32 index;

    POLYZN_MD *polyzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_exp: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == e )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_exp: e is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_exp: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_exp: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;

    if ( EC_TRUE == POLY_IS_EMPTY( poly_a ) )
    {
        poly_zn_poly_clean(polyzn_md_id, poly_c);
        return ( 0 );
    }

    poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY,&poly_ta, LOC_POLYZN_0094);
    POLY_INIT(poly_ta);

    poly_zn_poly_clone(polyzn_md_id, poly_a, poly_ta);

    te_nbits = bgn_z_get_nbits( bgnz_md_id, e );

    poly_zn_set_one(polyzn_md_id, poly_c);

    index = 0;

    while ( 1 )
    {
        e_bit = bgn_z_get_bit(bgnz_md_id, e, index);
        if ( 1 == e_bit )
        {
            poly_zn_mul_self(polyzn_md_id, poly_ta, poly_c);
        }

        index ++;
        if ( index == te_nbits )
        {
            break;
        }
        poly_zn_squ_poly(polyzn_md_id, poly_ta, poly_ta);
    }

    poly_zn_poly_destory(polyzn_md_id, poly_ta);

    return ( 0 );
}

/*Partial derivatives */
/**
*
*   poly_c = d(poly_c)/dx where x is the first var.
*
*poly_zn_dx_self := proc(poly_c)
*
*item_c = first_item(item_c);
*while item_c != null_item(item_c) do
*    if deg(item_c) is zero then
*        item_t = item_c;
*        item_c = prev_item(item_c);
*        remove item_t from poly_c;
*        destory item_t;
*    else
*        if TRUE == bgn_coe_flag(item_c) then
*            bgn_mul(deg(item_c), bgn_coe(item_c), bgn_coe(item_c));
*
*            if bgn_coe(item_c) is zero then
*                item_t = item_c;
*                item_c = prev_item(item_c);
*                remove item_t from poly_c;
*                destory item_t;
*            else
*                deg(item_c) = deg(item_c) - 1;
*            end if
*        else
*            poly_mul_bgn(deg(item_c), poly_coe(item_c), poly_coe(item_c));
*
*            if poly_coe(item_c) is empty then
*                item_t = item_c;
*                item_c = prev_item(item_c);
*                remove item_t from poly_c;
*                destory item_t;
*            else
*                deg(item_c) = deg(item_c) - 1;
*            end if
*        end if
*    end if
*    item_c = next_item(item_c);
*end do
*
*end proc
*
**/
static UINT32 poly_zn_dx_self(const UINT32 polyzn_md_id, POLY *poly_c )
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    DEGREE *deg_one;

    POLYZN_MD  *polyzn_md;
    UINT32  bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_dx_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_dx_self: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    poly_zn_alloc_deg(MD_POLYZN, polyzn_md_id, MM_DEGREE, &deg_one, LOC_POLYZN_0095);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, deg_one);

    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        /*
        *    if deg(item_c) is zero then
        *        item_t = item_c;
        *        item_c = prev_item(item_c);
        *        remove item_t from poly_c;
        *        destory item_t;
        *   end if
        */
        if ( EC_TRUE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c)) )
        {
            item_t = item_c;
            item_c = POLY_ITEM_PREV(item_c);

            POLY_ITEM_DEL(item_t);

            poly_zn_item_destory(polyzn_md_id, item_t);
        }
        else
        {
            /*
            *        if TRUE == bgn_coe_flag(item_c) then
            *            bgn_mul(deg(item_c), bgn_coe(item_c), bgn_coe(item_c));
            *
            *            if bgn_coe(item_c) is zero then
            *                item_t = item_c;
            *                item_c = prev_item(item_c);
            *                remove item_t from poly_c;
            *                destory item_t;
            *            else
            *                deg(item_c) = deg(item_c) - 1;
            *            end if
            *        end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
                bgn_zn_mul(bgnzn_md_id, POLY_ITEM_DEG(item_c), POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
                bgn_zn_smul(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c), (*POLY_ITEM_DEG(item_c)), POLY_ITEM_BGN_COE(item_c));
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/
                if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, POLY_ITEM_BGN_COE(item_c)) )
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);

                    poly_zn_item_destory(polyzn_md_id, item_t);
                }
                else
                {
                    POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_c), deg_one, POLY_ITEM_DEG(item_c));
                }
            }
            /*
            *        if FALSE == bgn_coe_flag(item_c) then
            *            poly_mul_bgn(deg(item_c), poly_coe(item_c), poly_coe(item_c));
            *
            *            if poly_coe(item_c) is empty then
            *                item_t = item_c;
            *                item_c = prev_item(item_c);
            *                remove item_t from poly_c;
            *                destory item_t;
            *            else
            *                deg(item_c) = deg(item_c) - 1;
            *            end if
            *        end if
            */
            else
            {
#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
                poly_zn_mul_bgn(polyzn_md_id, POLY_ITEM_POLY_COE(item_c), POLY_ITEM_DEG(item_c), POLY_ITEM_POLY_COE(item_c));
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/
#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
                poly_zn_mul_word(polyzn_md_id, POLY_ITEM_POLY_COE(item_c), (*POLY_ITEM_DEG(item_c)), POLY_ITEM_POLY_COE(item_c));
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/
                if ( EC_TRUE == POLY_IS_EMPTY( POLY_ITEM_POLY_COE(item_c) ) )
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);

                    poly_zn_item_destory(polyzn_md_id, item_t);
                }
                else
                {
                    POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_c), deg_one, POLY_ITEM_DEG(item_c));
                }
            }
        }
        /*
        *    item_c = next_item(item_c);
        */
        item_c = POLY_ITEM_NEXT(item_c);
    }

    poly_zn_free_deg(MD_POLYZN, polyzn_md_id, MM_DEGREE, deg_one, LOC_POLYZN_0096);

    return ( 0 );
}

/**
*
*   poly_c = d(poly_a)/dx where x is the first var.
*
*poly_zn_dx := proc(poly_a, poly_c)
*
*if poly_a == poly_c then
*    poly_zn_dx_self(poly_c);
*    return ;
*end if
*
*if poly_c is not empty then
*    clean up the items of poly_c;
*end if
*
*item_a = first_item(poly_a);
*while item_a != null_item(poly_a) do
*    if deg(item_a) is nonzero then
*        if TRUE == bgn_coe_flag(item_a) then
*            new bgn_coe_of_item_c;
*            bgn_mul(deg(item_a), bgn_coe(item_a), bgn_coe_of_item_c);
*
*            if bgn_coe_of_item_c is zero then
*                free bgn_coe_of_item_c;
*            else
*                new item_c;
*                deg(item_c) = deg(item_a) - 1;
*                bgn_coe(item_c) = bgn_coe_of_item_c;
*                bgn_coe_flag(item_c) = TRUE;
*            end if
*        else
*            new poly_coe_of_item_c;
*
*            poly_mul_bgn(deg(item_a), poly_coe(item_a), poly_coe_of_item_c);
*
*            if poly_coe_of_item_c is empty then
*                free poly_coe_of_item_c;
*            else
*                new item_c;
*                deg(item_c) = deg(item_a) - 1;
*                poly_coe(item_c) = poly_coe_of_item_c;
*                bgn_coe_flag(item_c) = FALSE;
*            end if
*        end if
*        insert item_c to the tail of poly_c;
*    end if
*
*    item_a = next_item(item_a);
*end do
*
*end proc
*
**/
UINT32 poly_zn_dx(const UINT32 polyzn_md_id,const POLY *poly_a, POLY *poly_c )
{
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_c;

    POLY        *poly_coe_of_item_c;
    BIGINT      *bgn_coe_of_item_c;

    DEGREE      *deg_one;

    POLYZN_MD  *polyzn_md;
    UINT32  bgnz_md_id;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_dx: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_dx: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_dx: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *    poly_zn_Dx_self(poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_zn_dx_self(polyzn_md_id, poly_c);
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;
    bgnzn_md_id = polyzn_md->bgnzn_md_id;

    /*
    *if poly_c is not empty then
    *    clean up the items of poly_c;
    *end if
    */
    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_zn_poly_clean(polyzn_md_id, poly_c);
    }

    poly_zn_alloc_deg(MD_POLYZN, polyzn_md_id, MM_DEGREE, &deg_one, LOC_POLYZN_0097);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, deg_one);

    item_a = POLY_FIRST_ITEM(poly_a);
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        if ( EC_FALSE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_a)) )
        {
            /*
            *        if TRUE == bgn_coe_flag(item_a) then
            *            new bgn_coe_of_item_c;
            *            bgn_mul(deg(item_a), bgn_coe(item_a), bgn_coe_of_item_c);
            *
            *            if bgn_coe_of_item_c is zero then
            *                free bgn_coe_of_item_c;
            *            else
            *                new item_c;
            *                deg(item_c) = deg(item_a) - 1;
            *                bgn_coe(item_c) = bgn_coe_of_item_c;
            *                bgn_coe_flag(item_c) = TRUE;
            *                insert item_c to the tail of poly_c;
            *            end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
            {
                poly_zn_alloc_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZN_0098);

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
                bgn_zn_mul(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_DEG(item_a), bgn_coe_of_item_c);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
                bgn_zn_smul(bgnzn_md_id, POLY_ITEM_BGN_COE(item_a), (*POLY_ITEM_DEG(item_a)), bgn_coe_of_item_c);
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/


                if ( EC_TRUE == bgn_zn_is_zero(bgnzn_md_id, bgn_coe_of_item_c) )
                {
                    poly_zn_free_bgn(MD_POLYZN, polyzn_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZN_0099);
                }
                else
                {
                    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0100);

                    POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_a), deg_one, POLY_ITEM_DEG(item_c));
                    POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
            }
            /*
            *        else
            *            new poly_coe_of_item_c;
            *            init poly_coe_of_item_c;
            *
            *            poly_mul_bgn(deg(item_a), poly_coe(item_a), poly_coe_of_item_c);
            *
            *            if poly_coe_of_item_c is empty then
            *                free poly_coe_of_item_c;
            *            else
            *                new item_c;
            *                deg(item_c) = deg(item_a) - 1;
            *                poly_coe(item_c) = poly_coe_of_item_c;
            *                bgn_coe_flag(item_c) = FALSE;
            *                insert item_c to the tail of poly_c;
            *            end if
            *        end if
            */
            else
            {
                poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZN_0101);
                POLY_INIT(poly_coe_of_item_c);

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
                poly_zn_mul_bgn(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_DEG(item_a), poly_coe_of_item_c);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
                poly_zn_mul_word(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), (*POLY_ITEM_DEG(item_a)), poly_coe_of_item_c);
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

                if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
                {
                    poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZN_0102);
                }
                else
                {
                    poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0103);

                    POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_a), deg_one, POLY_ITEM_DEG(item_c));
                    POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
            }
        }
        /*
        *    item_a = next_item(item_a);
        */
        item_a = POLY_ITEM_NEXT(item_a);
    }

    poly_zn_free_deg(MD_POLYZN, polyzn_md_id, MM_DEGREE, deg_one, LOC_POLYZN_0104);

    return ( 0 );
}

/**
*
*   poly_c = d(poly_c)/dx where x is the depth_of_x-th var.
*
*poly_zn_Dx_self := proc(depth_of_x, poly_c)
*
*if 0 == depth_of_x then
*    poly_zn_dx_self(poly_c);
*    return ;
*end if
*
*item_c = first_item(item_c);
*while item_c != null_item(item_c) do
*
*        if TRUE == bgn_coe_flag(item_c) then
*            item_t = item_c;
*            item_c = prev_item(item_c);
*            remove item_c from poly_c;
*            destory item_t;
*        else
*            poly_zn_Dx_self(depth_of_x - 1, poly_coe(item_c));
*
*            if poly_coe(item_c) is empty then
*                item_t = item_c;
*                item_c = prev_item(item_c);
*                remove item_t from poly_c;
*                destory item_t;
*            end if
*        end if
*        item_c = next_itm(item_c);
*end do
*
*end proc
*
**/
static UINT32 poly_zn_Dx_self(const UINT32 polyzn_md_id,const UINT32 depth_of_x, POLY *poly_c )
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    POLYZN_MD  *polyzn_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_Dx_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_Dx_self: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if 0 == depth_of_x then
    *    poly_zn_dx_self(poly_c);
    *    return ;
    *end if
    */
    if ( 0 == depth_of_x )
    {
        return poly_zn_dx_self(polyzn_md_id, poly_c);
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;

    item_c = POLY_FIRST_ITEM(poly_c);
    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        /*
        *        if TRUE == bgn_coe_flag(item_c) then
        *            item_t = item_c;
        *            item_c = prev_item(item_c);
        *            remove item_c from poly_c;
        *            destory item_t;
        */
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            item_t = item_c;
            item_c = POLY_ITEM_PREV(item_c);

            POLY_ITEM_DEL(item_t);
            poly_zn_item_destory(polyzn_md_id, item_t);
        }
        /*
        *        else
        *            poly_zn_Dx_self(depth_of_x - 1, poly_coe(item_c));
        *
        *            if poly_coe(item_c) is empty then
        *                item_t = item_c;
        *                item_c = prev_item(item_c);
        *                remove item_t from poly_c;
        *                destory item_t;
        *            end if
        *        end if
        */
        else
        {
            poly_zn_Dx_self(polyzn_md_id, depth_of_x - 1, POLY_ITEM_POLY_COE(item_c));
            if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);
                poly_zn_item_destory(polyzn_md_id, item_t);
            }
        }

        /*
        *    item_c = next_itm(item_c);
        */
        item_c = POLY_ITEM_NEXT(item_c);
    }

    return ( 0 );
}

/**
*
*   poly_c = d(poly_a)/dx where x is the depth_of_x-th var.
*
*poly_zn_Dx := proc(poly_a, depth_of_x, poly_c)
*
*if poly_a == poly_c then
*    poly_zn_Dx_self(depth_of_x, poly_c);
*    return ;
*end if
*
*if 0 == depth_of_x then
*    poly_zn_Dx(poly_a, poly_c);
*    return ;
*end if
*
*if poly_c is not empty then
*    clean up the items of poly_c;
*end if
*
*item_a = first_item(poly_a);
*while item_a != null_item(poly_a) do
*        if FALSE == bgn_coe_flag(item_a) then
*            new  poly_t
*            init poly_t
*
*            poly_zn_Dx(poly_coe(item_a), depth_of_x - 1, poly_t);
*
*            if poly_t is empty then
*                free poly_t
*            else
*                new item_c;
*                deg(item_c) = deg(item_a);
*
*                poly_coe(item_c) = poly_t;
*                bgn_coe_flag(item_c) = FALSE;
*                insert item_c to the tail of poly_c;
*            end if
*        end if
*
*        item_a = next_itm(item_a);
*end do
*end proc
**/
UINT32 poly_zn_Dx(const UINT32 polyzn_md_id,const POLY *poly_a, const UINT32 depth_of_x, POLY *poly_c )
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    POLY      *poly_t;

    POLYZN_MD  *polyzn_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_Dx: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_zn_Dx: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZN_MD <= polyzn_md_id || 0 == g_polyzn_md[ polyzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_zn_Dx: polyzn module #0x%lx not started.\n",
                polyzn_md_id);
        dbg_exit(MD_POLYZN, polyzn_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/


    /*
    *if poly_a == poly_c then
    *    poly_zn_Dx_self(depth_of_x, poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_zn_Dx_self(polyzn_md_id, depth_of_x, poly_c);
    }

    /*
    *if 0 == depth_of_x then
    *    poly_zn_dx(poly_a, poly_c);
    *    return ;
    *end if
    */
    if ( 0 == depth_of_x )
    {
        return poly_zn_dx( polyzn_md_id, poly_a, poly_c);
    }

    polyzn_md = &(g_polyzn_md[ polyzn_md_id ]);
    bgnz_md_id = polyzn_md->bgnz_md_id;

    /*
    *if poly_c is not empty then
    *    clean up the items of poly_c;
    *end if
    */
    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_zn_poly_clean(polyzn_md_id, poly_c);
    }

    item_a = POLY_FIRST_ITEM(poly_a);
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*
        *        if FALSE == bgn_coe_flag(item_a) then
        *            new  poly_t
        *            init poly_t
        *
        *            poly_zn_Dx(poly_coe(item_a), depth_of_x - 1, poly_t);
        *
        *            if poly_t is empty then
        *                free poly_t
        *            else
        *                new item_c;
        *                deg(item_c) = deg(item_a);
        *
        *                poly_coe(item_c) = poly_t;
        *                bgn_coe_flag(item_c) = FALSE;
        *                insert item_c to the tail of poly_c;
        *            end if
        *        end if
        */
        if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) )
        {
            poly_zn_alloc_poly(MD_POLYZN, polyzn_md_id, MM_POLY, &poly_t, LOC_POLYZN_0105);
            POLY_INIT(poly_t);

            poly_zn_Dx(polyzn_md_id, POLY_ITEM_POLY_COE(item_a), depth_of_x - 1, poly_t);

            if ( EC_TRUE == POLY_IS_EMPTY(poly_t) )
            {
                poly_zn_free_poly(MD_POLYZN, polyzn_md_id, MM_POLY, poly_t, LOC_POLYZN_0106);
            }
            else
            {
                poly_zn_alloc_item(MD_POLYZN, polyzn_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZN_0107);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_t;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
        }
        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
