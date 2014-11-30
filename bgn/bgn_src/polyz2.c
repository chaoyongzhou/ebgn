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

#include "poly.h"

#include "bgnz.h"
#include "bgnz2.h"
#include "polyz2.h"

#include "debug.h"

#include "print.h"



static POLYZ2_MD g_polyz2_md[ MAX_NUM_OF_POLYZ2_MD ];
static EC_BOOL  g_polyz2_md_init_flag = EC_FALSE;

static UINT32 poly_z2_adc_n(const UINT32 polyz2_md_id,const BIGINT *n, POLY *poly_c);
static UINT32 poly_z2_item_insert(const UINT32 polyz2_md_id, POLY_ITEM *item_a, POLY *poly_c);
static UINT32 poly_z2_item_mul(const UINT32 polyz2_md_id, const POLY_ITEM *item_a, const POLY_ITEM *item_b,POLY_ITEM *item_c);
static UINT32 poly_z2_item_mul_self(const UINT32 polyz2_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c);
static UINT32 poly_z2_item_squ_self(const UINT32 polyz2_md_id, POLY_ITEM *item_c);
static UINT32 poly_z2_mul_bgn_self(const UINT32 polyz2_md_id,const BIGINT *bgn_b, POLY *poly_c);
static UINT32 poly_z2_mul_bgn(const UINT32 polyz2_md_id,const POLY *poly_a,const BIGINT *bgn_b, POLY *poly_c);
static UINT32 poly_z2_mul_self(const UINT32 polyz2_md_id,const POLY *poly_a, POLY *poly_c);
static UINT32 poly_z2_squ_self(const UINT32 polyz2_md_id,POLY *poly_c);
static UINT32 poly_z2_dx_self(const UINT32 polyz2_md_id, POLY *poly_c );
static UINT32 poly_z2_Dx_self(const UINT32 polyz2_md_id,const UINT32 depth_of_x, POLY *poly_c );

/**
*   for test only
*
*   to query the status of POLYZ2 Module
*
**/
void poly_z2_print_module_status(const UINT32 polyz2_md_id, LOG *log)
{
    POLYZ2_MD *polyz2_md;
    UINT32 index;

    if ( EC_FALSE == g_polyz2_md_init_flag )
    {
        sys_log(log,"no POLYZ2 Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_POLYZ2_MD; index ++ )
    {
        polyz2_md = &(g_polyz2_md[ index ]);

        if ( 0 < polyz2_md->usedcounter )
        {
            sys_log(log,"POLYZ2 Module # %ld : %ld refered, refer BGNZ Module : %ld, refer BGNZ2 Module : %ld\n",
                    index,
                    polyz2_md->usedcounter,
                    polyz2_md->bgnz_md_id,
                    polyz2_md->bgnz2_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed POLYZ2 module
*
*
**/
UINT32 poly_z2_free_module_static_mem(const UINT32 polyz2_md_id)
{
    POLYZ2_MD  *polyz2_md;
    UINT32  bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_free_module_static_mem: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    free_module_static_mem(MD_POLYZ2, polyz2_md_id);

    bgn_z_free_module_static_mem(bgnz_md_id);
    bgn_z2_free_module_static_mem(bgnz2_md_id);

    return 0;
}
/**
*
* start POLYZ2 module
*
**/
UINT32 poly_z2_start( )
{
    POLYZ2_MD *polyz2_md;
    UINT32 polyz2_md_id;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

    UINT32 index;

    /* if this is the 1st time to start POLYZ2 module, then */
    /* initialize g_polyz2_md */
    if ( EC_FALSE ==  g_polyz2_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_POLYZ2_MD; index ++ )
        {
            polyz2_md = &(g_polyz2_md[ index ]);

            polyz2_md->usedcounter   = 0;
            polyz2_md->bgnz_md_id = ERR_MODULE_ID;
            polyz2_md->bgnz2_md_id = ERR_MODULE_ID;
        }

        /*register all functions of POLYZ2 module to DBG module*/
        //dbg_register_func_addr_list(g_polyz2_func_addr_list, g_polyz2_func_addr_list_len);

        g_polyz2_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_POLYZ2_MD; index ++ )
    {
        polyz2_md = &(g_polyz2_md[ index ]);

        if ( 0 == polyz2_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_POLYZ2_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    bgnz_md_id = ERR_MODULE_ID;
    bgnz2_md_id = ERR_MODULE_ID;

    /* initilize new one POLYZ2 module */
    polyz2_md_id = index;
    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);

    init_static_mem();

    bgnz_md_id = bgn_z_start();
    bgnz2_md_id = bgn_z2_start( );

    polyz2_md->bgnz_md_id = bgnz_md_id;
    polyz2_md->bgnz2_md_id = bgnz2_md_id;
    polyz2_md->usedcounter = 1;

    return ( polyz2_md_id );
}

/**
*
* end POLYZ2 module
*
**/
void poly_z2_end(const UINT32 polyz2_md_id)
{
    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

    if ( MAX_NUM_OF_POLYZ2_MD < polyz2_md_id )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_end: polyz2_md_id = %ld is overflow.\n",polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < polyz2_md->usedcounter )
    {
        polyz2_md->usedcounter --;
        return ;
    }

    if ( 0 == polyz2_md->usedcounter )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_end: polyz2_md_id = %ld is not started.\n",polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    bgn_z_end( bgnz_md_id);
    bgn_z2_end( bgnz2_md_id);

    /* free module : */
    //poly_z2_free_module_static_mem(polyz2_md_id);
    polyz2_md->bgnz_md_id = ERR_MODULE_ID;
    polyz2_md->bgnz2_md_id = ERR_MODULE_ID;
    polyz2_md->usedcounter = 0;

    breathing_static_mem();

    return ;
}

#if 1
#define poly_z2_alloc_bgn(  polyz_md_type, polyz2_md_id, mm_type, ppbgn  , __location__) alloc_static_mem( polyz_md_type, polyz2_md_id, mm_type, ppbgn , (__location__))
#define poly_z2_alloc_deg(  polyz_md_type, polyz2_md_id, mm_type, ppdeg  , __location__) alloc_static_mem( polyz_md_type, polyz2_md_id, mm_type, ppdeg , (__location__))
#define poly_z2_alloc_item( polyz_md_type, polyz2_md_id, mm_type, ppitem , __location__) alloc_static_mem( polyz_md_type, polyz2_md_id, mm_type, ppitem, (__location__))
#define poly_z2_alloc_poly( polyz_md_type, polyz2_md_id, mm_type, pppoly , __location__) alloc_static_mem( polyz_md_type, polyz2_md_id, mm_type, pppoly, (__location__))
#define poly_z2_free_bgn(   polyz_md_type, polyz2_md_id, mm_type, pbgn   , __location__) free_static_mem ( polyz_md_type, polyz2_md_id, mm_type, pbgn  , (__location__))
#define poly_z2_free_deg(   polyz_md_type, polyz2_md_id, mm_type, pdeg   , __location__) free_static_mem ( polyz_md_type, polyz2_md_id, mm_type, pdeg  , (__location__))
#define poly_z2_free_item(  polyz_md_type, polyz2_md_id, mm_type, pitem  , __location__) free_static_mem ( polyz_md_type, polyz2_md_id, mm_type, pitem , (__location__))
#define poly_z2_free_poly(  polyz_md_type, polyz2_md_id, mm_type, ppoly  , __location__) free_static_mem ( polyz_md_type, polyz2_md_id, mm_type, ppoly , (__location__))

#else

UINT32 poly_z2_alloc_bgn(const UINT32 polyz_md_type, const UINT32 polyz2_md_id,const UINT32 type, BIGINT **bgn)
{
    if ( MD_POLYZ2 != polyz_md_type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_alloc_bgn: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_BIGINT != type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_alloc_bgn: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(polyz_md_type, polyz2_md_id, type, bgn, LOC_POLYZ2_0001);
    return (0);
}
UINT32 poly_z2_alloc_item(const UINT32 polyz_md_type, const UINT32 polyz2_md_id,const UINT32 type,POLY_ITEM **item)
{
    //BIGINT *deg_of_item;

    if ( MD_POLYZ2 != polyz_md_type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_alloc_item: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_POLY_ITEM != type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_alloc_item: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(polyz_md_type, polyz2_md_id, type, item, LOC_POLYZ2_0002);

    //poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &deg_of_item, LOC_POLYZ2_0003);
    //POLY_ITEM_DEG(*item) = deg_of_item;

    return (0);
}
UINT32 poly_z2_alloc_poly(const UINT32 polyz_md_type, const UINT32 polyz2_md_id,const UINT32 type,POLY **poly)
{
    if ( MD_POLYZ2 != polyz_md_type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_alloc_poly: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_POLY != type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_alloc_poly: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(polyz_md_type, polyz2_md_id, type, poly, LOC_POLYZ2_0004);
    return (0);
}
UINT32 poly_z2_free_bgn(const UINT32 polyz_md_type, const UINT32 polyz2_md_id,const UINT32 type,BIGINT *bgn)
{
    if ( MD_POLYZ2 != polyz_md_type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_free_bgn: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_BIGINT != type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_free_bgn: invalid type = %ld\n",type);
        exit(0);
    }

    free_static_mem(polyz_md_type, polyz2_md_id, type, bgn, LOC_POLYZ2_0005);
    return (0);
}
UINT32 poly_z2_free_item(const UINT32 polyz_md_type, const UINT32 polyz2_md_id,const UINT32 type,POLY_ITEM *item)
{
    if ( MD_POLYZ2 != polyz_md_type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_free_item: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_POLY_ITEM != type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_free_item: invalid type = %ld\n",type);
        exit(0);
    }

    free_static_mem(polyz_md_type, polyz2_md_id, type, item, LOC_POLYZ2_0006);
    return (0);
}
UINT32 poly_z2_free_poly(const UINT32 polyz_md_type, const UINT32 polyz2_md_id,const UINT32 type,POLY *poly)
{
    if ( MD_POLYZ2 != polyz_md_type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_free_poly: invalid polyz_md_type = %ld\n",polyz_md_type);
        exit(0);
    }
    if ( MM_POLY != type )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_free_poly: invalid type = %ld\n",type);
        exit(0);
    }

    free_static_mem(polyz_md_type, polyz2_md_id, type, poly, LOC_POLYZ2_0007);
    return (0);
}

#endif

/**
*
*   destory the whole item,including its content(deg, coe) and itself.
*   so, when return from this function, item cannot be refered any more
*
**/
UINT32 poly_z2_item_destory(const UINT32 polyz2_md_id, POLY_ITEM *item)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_destory: item is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_item_destory: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
    {
        poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, POLY_ITEM_BGN_COE(item), LOC_POLYZ2_0008);
        POLY_ITEM_BGN_COE(item) = NULL_PTR;
    }
    else
    {
        poly_z2_poly_destory(polyz2_md_id, POLY_ITEM_POLY_COE(item));
        POLY_ITEM_POLY_COE(item) = NULL_PTR;
    }

    poly_z2_free_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, item, LOC_POLYZ2_0009);

    return ( 0 );
}

/**
*
*   destory the whole poly,i.e., all its items but not the poly itself.
*   so, when return from this function, poly can be refered again without any item.
*
**/
UINT32 poly_z2_poly_clean(const UINT32 polyz2_md_id, POLY *poly)
{
    POLY_ITEM *item;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_clean: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_poly_clean: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    item = POLY_FIRST_ITEM(poly);

    while ( item != POLY_NULL_ITEM(poly) )
    {
        POLY_ITEM_DEL(item);
        poly_z2_item_destory(polyz2_md_id, item);

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
UINT32 poly_z2_poly_destory(const UINT32 polyz2_md_id, POLY *poly)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_destory: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_poly_destory: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    poly_z2_poly_clean(polyz2_md_id,poly);

    poly_z2_free_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, poly, LOC_POLYZ2_0010);

    return ( 0 );
}

UINT32 poly_z2_poly_clone(const UINT32 polyz2_md_id,const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_clone: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_clone: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_poly_clone: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_z2_poly_clean(polyz2_md_id,poly_c);
    }

    //POLY_INIT(poly_c);

    item_a = POLY_FIRST_ITEM(poly_a);

    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0011);
        poly_z2_item_clone(polyz2_md_id, item_a, item_c);

        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_a = POLY_ITEM_NEXT(item_a);
    }

    return (0);
}

UINT32 poly_z2_item_clone(const UINT32 polyz2_md_id,const POLY_ITEM *item_a, POLY_ITEM *item_c)
{
    BIGINT  *bgn_coe_of_item_c;
    POLY    *poly_coe_of_item_c;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_clone: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_clone: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_item_clone: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    POLY_ITEM_INIT(item_c);

    /*clone degree of item*/
    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

    /*clone coefficient of item*/
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
    {
        /*clone bgn coe*/
        poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZ2_0012);
        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        bgn_z2_clone(bgnz2_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c));

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
    }
    else
    {
        /*clone poly coe*/
        poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0013);
        POLY_INIT(poly_coe_of_item_c);

        POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;

        poly_z2_poly_clone(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
    }

    return (0);
}

/**
*
*   set poly = 0
*
**/
UINT32 poly_z2_set_zero(const UINT32 polyz2_md_id,POLY *poly)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_set_zero: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_set_zero: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    if ( EC_TRUE == POLY_IS_EMPTY(poly) )
    {
        return ( 0 );
    }

    return poly_z2_poly_clean(polyz2_md_id, poly);
}

/**
*
*   set poly = 1
*
**/
UINT32 poly_z2_set_one(const UINT32 polyz2_md_id,POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_set_one: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_set_one: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_z2_poly_clean(polyz2_md_id, poly);
    }

    poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item, LOC_POLYZ2_0014);
    poly_z2_alloc_bgn (MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYZ2_0015);

    /*deg(item) = 0*/
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item));

    /*bgn_coe(item) = 1*/
    bgn_z2_set_one(bgnz2_md_id, bgn_coe_of_item);

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
UINT32 poly_z2_set_n(const UINT32 polyz2_md_id,const BIGINT *n, POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_set_n: n is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_set_n: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_set_n: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_z2_poly_clean(polyz2_md_id, poly);
    }

    poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item, LOC_POLYZ2_0016);
    poly_z2_alloc_bgn (MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYZ2_0017);

    /*deg(item) = 0*/
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item));

    /*bgn_coe(item) = n*/
    bgn_z2_clone(bgnz2_md_id, n, bgn_coe_of_item);

    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly, item);

    return ( 0 );
}


EC_BOOL poly_z2_item_cmp(const UINT32 polyz2_md_id,const POLY_ITEM *item_a, const POLY_ITEM *item_b)
{
    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_cmp: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == item_b )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_cmp: item_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_item_cmp: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    if ( 0 != POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_b)) )
    {
        return (EC_FALSE);
    }

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        if ( 0 != bgn_z2_cmp(bgnz2_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b)))
        {
            return (EC_FALSE);
        }
        return (EC_TRUE);
    }
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        return poly_z2_poly_cmp(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b));
    }

    return (EC_FALSE);
}
/**
*
*   if poly_a = poly_b then return EC_TRUE
*   else    return EC_FALSE
*
**/
EC_BOOL poly_z2_poly_cmp(const UINT32 polyz2_md_id,const POLY *poly_a, const POLY *poly_b)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_cmp: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_cmp: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_poly_cmp: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    item_a = POLY_FIRST_ITEM(poly_a);
    item_b = POLY_FIRST_ITEM(poly_b);

    while ( item_a != POLY_NULL_ITEM(poly_a) && item_b != POLY_NULL_ITEM(poly_b) )
    {
        if ( EC_FALSE == poly_z2_item_cmp(polyz2_md_id, item_a, item_b) )
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
static UINT32 poly_z2_adc_n(const UINT32 polyz2_md_id,const BIGINT *n, POLY *poly_c)
{
    POLY_ITEM *item_c;
    BIGINT  *bgn_coe_of_item_c;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_adc_n: n is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_adc_n: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_adc_n: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

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
            bgn_z2_add(bgnz2_md_id, POLY_ITEM_BGN_COE(item_c), n, POLY_ITEM_BGN_COE(item_c));
        }
        else
        {
            poly_z2_adc_n(polyz2_md_id, n, POLY_ITEM_POLY_COE(item_c));
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
        poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0018);

        POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));

        poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZ2_0019);
        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        bgn_z2_clone(bgnz2_md_id, n, POLY_ITEM_BGN_COE(item_c));

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
UINT32 poly_z2_add_n(const UINT32 polyz2_md_id,const POLY *poly_a, const BIGINT *n, POLY *poly_c)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_add_n: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == n )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_add_n: n is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_add_n: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_add_n: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a != poly_c )
    {
        /*clone poly_a to poly_c*/
        poly_z2_poly_clone(polyz2_md_id, poly_a, poly_c);
    }

    poly_z2_adc_n(polyz2_md_id, n, poly_c);

    return (0);
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
UINT32 poly_z2_adc_poly(const UINT32 polyz2_md_id,const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_c;
    POLY_ITEM   *item_t;
    POLY        *poly_coe_of_item_c;
    int cmp_result;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_adc_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_adc_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_adc_poly: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_c )
    {
        return poly_z2_poly_clean(polyz2_md_id, poly_c);
    }

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

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
                bgn_z2_add(bgnz2_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));

                if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, POLY_ITEM_BGN_COE(item_c)) )
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_z2_item_destory(polyz2_md_id, item_t);
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
                poly_z2_add_n(polyz2_md_id,POLY_ITEM_POLY_COE(item_c), POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_c));
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
                poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0020);
                POLY_INIT(poly_coe_of_item_c);

                poly_z2_add_n(polyz2_md_id,POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_c), poly_coe_of_item_c);

                poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, POLY_ITEM_BGN_COE(item_c), LOC_POLYZ2_0021);

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
                poly_z2_add_poly(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c), POLY_ITEM_POLY_COE(item_c));

                if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_z2_item_destory(polyz2_md_id, item_t);
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
            poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_t, LOC_POLYZ2_0022);
            poly_z2_item_clone(polyz2_md_id, item_a, item_t);

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
        poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_t, LOC_POLYZ2_0023);

        poly_z2_item_clone(polyz2_md_id, item_a, item_t);

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
UINT32 poly_z2_add_poly(const UINT32 polyz2_md_id,const POLY *poly_a, const POLY *poly_b, POLY *poly_c)
{
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_b;
    POLY_ITEM   *item_c;
    POLY        *poly_coe_of_item_c;
    BIGINT      *bgn_coe_of_item_c;
    int cmp_result;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_add_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_add_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_add_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_add_poly: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_c == poly_a )
    {
        return poly_z2_adc_poly(polyz2_md_id, poly_b, poly_c);
    }

    if ( poly_c == poly_b )
    {
        return poly_z2_adc_poly(polyz2_md_id, poly_a, poly_c);
    }

    /*clean all items of the poly_c*/
    poly_z2_poly_clean(polyz2_md_id, poly_c);

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

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
                poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZ2_0024);

                bgn_z2_add(bgnz2_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b), bgn_coe_of_item_c);

                if ( EC_FALSE == bgn_z2_is_zero(bgnz2_md_id, bgn_coe_of_item_c) )
                {
                    poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0025);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
                else
                {
                    poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZ2_0026);
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
                    poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0027);
                    POLY_INIT(poly_coe_of_item_c);

                    poly_z2_add_n(polyz2_md_id, POLY_ITEM_POLY_COE(item_b), POLY_ITEM_BGN_COE(item_a), poly_coe_of_item_c);

                    poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0028);

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
                    poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0029);
                    POLY_INIT(poly_coe_of_item_c);

                    poly_z2_add_n(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_b), poly_coe_of_item_c);

                    poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0030);

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
                    poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0031);
                    POLY_INIT(poly_coe_of_item_c);

                    poly_z2_add_poly(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);
                    if ( EC_FALSE == POLY_IS_EMPTY(poly_coe_of_item_c) )
                    {
                        poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0032);

                        POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                        POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                        POLY_ADD_ITEM_TAIL(poly_c, item_c);
                    }
                    else
                    {
                        poly_z2_free_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZ2_0033);
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
            poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0034);

            poly_z2_item_clone(polyz2_md_id, item_a, item_c);

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

            poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0035);

            poly_z2_item_clone(polyz2_md_id, item_b, item_c);

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
        poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0036);

        poly_z2_item_clone(polyz2_md_id, item_a, item_c);
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
        poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0037);

        poly_z2_item_clone(polyz2_md_id, item_b, item_c);
        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_b = POLY_ITEM_NEXT(item_b);
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

UINT32 poly_z2_item_move(const UINT32 polyz2_md_id, POLY_ITEM *item_a, POLY_ITEM *item_c)
{
    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_move: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_move: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_item_move: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

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
UINT32 poly_z2_poly_move(const UINT32 polyz2_md_id, POLY *poly_a, POLY *poly_c)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_move: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_move: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_poly_move: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a != poly_c )
    {
        poly_z2_poly_clean(polyz2_md_id, poly_c);
        //POLY_INIT(poly_c);
        if ( EC_FALSE == POLY_IS_EMPTY(poly_a) )
        {
            POLY_ITEM_HEAD(poly_c)->next = POLY_ITEM_HEAD(poly_a)->next;
            POLY_ITEM_HEAD(poly_c)->next->prev = POLY_ITEM_HEAD(poly_c);

            POLY_ITEM_HEAD(poly_c)->prev = POLY_ITEM_HEAD(poly_a)->prev;
            POLY_ITEM_HEAD(poly_c)->prev->next = POLY_ITEM_HEAD(poly_c);
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
static UINT32 poly_z2_item_insert(const UINT32 polyz2_md_id, POLY_ITEM *item_a, POLY *poly_c)
{
    POLY_ITEM *item_c;
    POLY      *poly_coe_of_item_c;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_insert: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_insert: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_item_insert: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

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
        bgn_z2_add(bgnz2_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));

        if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, POLY_ITEM_BGN_COE(item_c)))
        {
            POLY_ITEM_DEL(item_c);
            poly_z2_item_destory(polyz2_md_id, item_c);
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
        poly_z2_adc_n(polyz2_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_c));

        if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
        {
            POLY_ITEM_DEL(item_c);
            poly_z2_item_destory(polyz2_md_id, item_c);
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
        poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0038);
        POLY_INIT(poly_coe_of_item_c);

        poly_z2_add_n(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_c), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            poly_z2_free_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZ2_0039);

            POLY_ITEM_DEL(item_c);
            poly_z2_item_destory(polyz2_md_id, item_c);
        }
        else
        {
            poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, POLY_ITEM_BGN_COE(item_c), LOC_POLYZ2_0040);

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
        poly_z2_adc_poly(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));

        if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)) )
        {
            POLY_ITEM_DEL(item_c);

            poly_z2_item_destory(polyz2_md_id, item_c);
        }
    }
    else
    {
        ;
    }

    poly_z2_item_destory(polyz2_md_id, item_a);

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
static UINT32 poly_z2_item_mul(const UINT32 polyz2_md_id, const POLY_ITEM *item_a, const POLY_ITEM *item_b,POLY_ITEM *item_c)
{
    BIGINT *bgn_coe_of_item_c;
    BIGINT *bgn_t_c1;
    POLY   *poly_coe_of_item_c;
    UINT32 carry;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_mul: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == item_b )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_mul: item_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_mul: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_item_mul: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
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
        poly_z2_item_mul_self( polyz2_md_id, item_b, item_c);
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
        poly_z2_item_mul_self( polyz2_md_id, item_a, item_c);
        return ( 0 );
    }

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

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
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_mul: degree overflow.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
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
        poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZ2_0041);
        poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_t_c1, LOC_POLYZ2_0042);

        bgn_z2_mul(bgnz2_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b), bgn_coe_of_item_c,bgn_t_c1);
        if ( EC_FALSE == bgn_z2_is_zero(bgnz2_md_id, bgn_t_c1) )
        {
            dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_mul: bgn_z2_mul overflow with nonzero bgn_t_c1.\n");
            dbg_exit(MD_POLYZ2, polyz2_md_id);
        }
        poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_t_c1, LOC_POLYZ2_0043);

        if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, bgn_coe_of_item_c))
        {
            poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZ2_0044);

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
        poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0045);
        POLY_INIT(poly_coe_of_item_c);

        poly_z2_mul_bgn(polyz2_md_id, POLY_ITEM_POLY_COE(item_b), POLY_ITEM_BGN_COE(item_a), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            poly_z2_free_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZ2_0046);

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
        poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0047);
        POLY_INIT(poly_coe_of_item_c);

        poly_z2_mul_bgn(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_b), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            poly_z2_free_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZ2_0048);

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
        poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0049);
        POLY_INIT(poly_coe_of_item_c);

        poly_z2_mul_poly(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            poly_z2_free_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZ2_0050);

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
static UINT32 poly_z2_item_mul_self(const UINT32 polyz2_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c)
{
    POLY_ITEM *item_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_mul_self: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_mul_self: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_item_mul_self: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_b, LOC_POLYZ2_0051);

    /*mov item_c to item_b*/
    poly_z2_item_move(polyz2_md_id, item_c, item_b);

    poly_z2_item_mul(polyz2_md_id, item_a, item_b, item_c);

    poly_z2_item_destory(polyz2_md_id, item_b);

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
static UINT32 poly_z2_item_squ_self(const UINT32 polyz2_md_id, POLY_ITEM *item_c)
{
    POLY_ITEM *item_a;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_item_squ_self: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_item_squ_self: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_a, LOC_POLYZ2_0052);

    /*mov item_c to item_a*/
    poly_z2_item_move(polyz2_md_id, item_c, item_a);

    poly_z2_item_mul(polyz2_md_id, item_a, item_a, item_c);

    poly_z2_item_destory(polyz2_md_id, item_a);

    return ( 0 );
}
/**
*
*   poly_c = poly_c * bgn_b
*   where bgn_b belong to F_{2^n}
*
**/
static UINT32 poly_z2_mul_bgn_self(const UINT32 polyz2_md_id,const BIGINT *bgn_b, POLY *poly_c)
{
    BIGINT *bgn_t_c0;
    BIGINT *bgn_t_c1;
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == bgn_b )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_bgn_self: bgn_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_bgn_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_mul_bgn_self: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, bgn_b) )
    {
        poly_z2_poly_clean(polyz2_md_id, poly_c);

        return ( 0 );
    }

    /*alloc BIGINT bgn_t_c0,bgn_t_c1 for bgn_z2_mul usage. Must free them before return from this function*/
    poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_t_c0, LOC_POLYZ2_0053);
    poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_t_c1, LOC_POLYZ2_0054);

    item_c = POLY_FIRST_ITEM(poly_c);
    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            bgn_z2_mul(bgnz2_md_id, POLY_ITEM_BGN_COE(item_c), bgn_b, bgn_t_c0,bgn_t_c1);
            if ( EC_FALSE == bgn_z2_is_zero(bgnz2_md_id, bgn_t_c1) )
            {
                dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_bgn_self: bgn_z2_mul overflow with nonzero bgn_t_c1.\n");
                dbg_exit(MD_POLYZ2, polyz2_md_id);
            }
            bgn_z2_clone(bgnz2_md_id, bgn_t_c0, POLY_ITEM_BGN_COE(item_c));

            if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, POLY_ITEM_BGN_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);
                poly_z2_item_destory(polyz2_md_id, item_t);
            }
        }
        else
        {
            poly_z2_mul_bgn(polyz2_md_id, POLY_ITEM_POLY_COE(item_c), bgn_b, POLY_ITEM_POLY_COE(item_c));

            if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);
                poly_z2_item_destory(polyz2_md_id, item_t);
            }
        }

        item_c = POLY_ITEM_NEXT(item_c);
    }

    /*free BIGINT bgn_t_c0,bgn_t_c1 which is used by bgn_z2_mul*/
    poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_t_c0, LOC_POLYZ2_0055);
    poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_t_c1, LOC_POLYZ2_0056);

    return ( 0 );
}

/**
*
*   poly_c = poly_a * bgn_b
*   where bgn_b belong to F_{2^n}
*
**/
static UINT32 poly_z2_mul_bgn(const UINT32 polyz2_md_id,const POLY *poly_a,const BIGINT *bgn_b, POLY *poly_c)
{
    BIGINT *bgn_t;

    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    BIGINT    *bgn_coe_of_item_c;
    POLY      *poly_coe_of_item_c;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_bgn: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == bgn_b )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_bgn: bgn_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_bgn: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_mul_bgn: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_c )
    {
        return poly_z2_mul_bgn_self(polyz2_md_id, bgn_b, poly_c);
    }

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    poly_z2_poly_clean(polyz2_md_id, poly_c);

    if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, bgn_b) )
    {
        return ( 0 );
    }

    /*alloc BIGINT bgn_t for bgn_z2_mul usage. Must free bgn_t before return from this function*/
    poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_t, LOC_POLYZ2_0057);

    item_a = POLY_FIRST_ITEM(poly_a);
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
        {
            poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZ2_0058);

            bgn_z2_mul(bgnz2_md_id, POLY_ITEM_BGN_COE(item_a), bgn_b, bgn_coe_of_item_c, bgn_t);
            if ( EC_FALSE == bgn_z2_is_zero(bgnz2_md_id, bgn_t) )
            {
                dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_bgn: bgn_z2_mul overflow with nonzero bgn_t.\n");
                dbg_exit(MD_POLYZ2, polyz2_md_id);
            }

            if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, bgn_coe_of_item_c) )
            {
                poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZ2_0059);
            }
            else
            {
                poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0060);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                poly_z2_item_insert(polyz2_md_id, item_c, poly_c);
            }
        }
        else
        {
            poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0061);

            poly_z2_mul_bgn(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), bgn_b, poly_coe_of_item_c);

            if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
            {
                poly_z2_free_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZ2_0062);
            }
            else
            {
                poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0063);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                poly_z2_item_insert(polyz2_md_id, item_c, poly_c);
            }
        }

        item_a = POLY_ITEM_NEXT(item_a);
    }


    /*free BIGINT bgn_t which is used by bgn_z2_mul*/
    poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_t, LOC_POLYZ2_0064);

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
static UINT32 poly_z2_mul_self(const UINT32 polyz2_md_id,const POLY *poly_a, POLY *poly_c)
{
    POLY *poly_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_self: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_mul_self: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_c )
    {
        return poly_z2_squ_self(polyz2_md_id, poly_c);
    }
    poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_b, LOC_POLYZ2_0065);
    POLY_INIT(poly_b);

    /*move poly_c to poly_b*/
    poly_z2_poly_move(polyz2_md_id, poly_c, poly_b);

    poly_z2_mul_poly(polyz2_md_id, poly_a, poly_b, poly_c);

    poly_z2_poly_destory(polyz2_md_id, poly_b);

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
UINT32 poly_z2_mul_poly(const UINT32 polyz2_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_b;
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_mul_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_mul_poly: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_b )
    {
        return poly_z2_squ_poly(polyz2_md_id, poly_a, poly_c);
    }
    /**
    *
    *if poly_a == poly_c then
    *    poly_mul_self(poly_b,poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_z2_mul_self(polyz2_md_id, poly_b, poly_c);
    }
    /*
    *if poly_b == poly_c then
    *    poly_mul_self(poly_a,poly_c);
    *    return;
    *end if
    */
    if ( poly_b == poly_c )
    {
        return poly_z2_mul_self(polyz2_md_id, poly_a, poly_c);
    }

    /*clean all items of the poly_c*/
    poly_z2_poly_clean(polyz2_md_id, poly_c);

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
            poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0066);

            poly_z2_item_mul(polyz2_md_id, item_a, item_b, item_c);
#if 0
            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"----------------------------------------------------------------\n");
            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"item_a:\n");
            print_item_dec(LOGSTDOUT, item_a);
            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"\n");

            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"item_b:\n");
            print_item_dec(LOGSTDOUT, item_b);
            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"\n");

            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"item_c:\n");
            print_item_dec(LOGSTDOUT, item_c);
            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"\n");
            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"----------------------------------------------------------------\n\n");
#endif
            if ( EC_TRUE == POLY_ITEM_IS_EMPTY(item_c) )
            {
                poly_z2_free_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, item_c, LOC_POLYZ2_0067);
            }
            else
            {
                poly_z2_item_insert(polyz2_md_id, item_c, poly_c);
            }

            item_b = POLY_ITEM_NEXT(item_b);
        }
        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

/**
*
* poly_c = poly_c ^ 2
*
*   let poly_c = SUM( A_i * x ^ i)
*   then
*       pol_c = SUM( (A_i^2) * x ^{2*i} );
*
*/
static UINT32 poly_z2_squ_self(const UINT32 polyz2_md_id,POLY *poly_c)
{
    BIGINT *bgn_t_c0;
    BIGINT *bgn_t_c1;
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_squ_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_squ_self: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_t_c0, LOC_POLYZ2_0068);
    poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_t_c1, LOC_POLYZ2_0069);

    item_c = POLY_FIRST_ITEM(poly_c);
    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        /*
        *   if TRUE == bgn_coe_flag(item_c) then
        *       bgn_z2_squ(bgn_coe(item_c),bgn_coe(item_c));
        *       if bgn_coe(item_c) is zero then
        *           item_t = item_c;
        *           item_c = prev_item(item_c);
        *           remove item_t from poly_c;
        *           destory item_t;
        *       else
        *           deg(item_c) = 2 * deg(item_c);
        *       end if
        *   end if
        */
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            bgn_z2_squ(bgnz2_md_id, POLY_ITEM_BGN_COE(item_c), bgn_t_c0, bgn_t_c1);
            if ( EC_FALSE == bgn_z2_is_zero(bgnz2_md_id, bgn_t_c1) )
            {
                dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_squ_self: bgn_z2_squ overflow with nonzero bgn_t_c1.\n");
                dbg_exit(MD_POLYZ2, polyz2_md_id);
            }

            bgn_z2_clone(bgnz2_md_id, bgn_t_c0, POLY_ITEM_BGN_COE(item_c));

            if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, POLY_ITEM_BGN_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);
                poly_z2_item_destory(polyz2_md_id, item_t);
            }
            else
            {
                POLY_ITEM_DEG_ADD(bgnz_md_id, POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_c));
            }
        }
        /*
        *   if FALSE == bgn_coe_flag(item_c) then
        *       poly_z2_squ_self(poly_coe(item_c),poly_coe(item_c));
        *       if poly_coe(item_c) is empty then
        *           item_t = item_c;
        *           item_c = prev_item(item_c);
        *           remove item_t from poly_c;
        *           destory item_t;
        *       else
        *           deg(item_c) = 2 * deg(item_c);
        *       end if
        *   end if
        */
        else
        {
            poly_z2_squ_self(polyz2_md_id, POLY_ITEM_POLY_COE(item_c));
            if ( EC_TRUE == POLY_IS_EMPTY( POLY_ITEM_POLY_COE(item_c) ))
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);
                poly_z2_item_destory(polyz2_md_id, item_t);
            }
            else
            {
                POLY_ITEM_DEG_ADD(bgnz_md_id, POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_c));
            }
        }

        item_c = POLY_ITEM_NEXT(item_c);
    }

    poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_t_c0, LOC_POLYZ2_0070);
    poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_t_c1, LOC_POLYZ2_0071);

    return ( 0 );
}
/**
*
* poly_c = poly_a ^ 2
*
*   let poly_a = SUM( A_i * x ^ i)
*   then
*       pol_c = SUM( (A_i ^ 2) * x ^{2*i} );
*
*/
UINT32 poly_z2_squ_poly(const UINT32 polyz2_md_id,const POLY *poly_a,POLY *poly_c)
{
    BIGINT *bgn_t_c1;

    POLY_ITEM *item_a;
    POLY_ITEM *item_c;

    BIGINT     *bgn_coe_of_item_c;
    POLY       *poly_coe_of_item_c;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_squ_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_squ_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_squ_poly: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    if ( poly_a == poly_c )
    {
        return poly_z2_squ_self(polyz2_md_id, poly_c);
    }

    poly_z2_poly_clean(polyz2_md_id, poly_c);

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_t_c1, LOC_POLYZ2_0072);

    item_a = POLY_FIRST_ITEM(poly_a);
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*
        *   if TRUE == bgn_coe_flag(item_a) then
        *       new bgn_coe_of_item_c
        *       bgn_z2_squ(bgn_coe(item_a),bgn_coe_of_item_c);
        *       if bgn_coe_of_item_c is zero then
        *           free bgn_coe_of_item_c
        *       else
        *           new item_c
        *           deg(item_c) = 2 * deg(item_a);
        *           bgn_coe(item_c) = bgn_coe_of_item_c;
        *           bgn_coe_flag(item_c) = TRUE;
        *           insert item_c to the tail of poly_c
        *       end if
        *   end if
        */
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
        {
            poly_z2_alloc_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYZ2_0073);

            bgn_z2_squ(bgnz2_md_id, POLY_ITEM_BGN_COE(item_a), bgn_coe_of_item_c, bgn_t_c1);
            if ( EC_FALSE == bgn_z2_is_zero(bgnz2_md_id, bgn_t_c1) )
            {
                dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_squ_poly: bgn_z2_squ overflow with nonzero bgn_t_c1.\n");
                dbg_exit(MD_POLYZ2, polyz2_md_id);
            }

            if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, bgn_coe_of_item_c) )
            {
                poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYZ2_0074);
            }
            else
            {
                poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0075);

                POLY_ITEM_DEG_ADD(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
        }
        /*
        *   if TRUE == bgn_coe_flag(item_a) then
        *       new poly_coe_of_item_c
        *       init poly_coe_of_item_c
        *       poly_z2_squ_poly(poly_coe(item_a),poly_coe_of_item_c);
        *       if poly_coe_of_item_c is empty then
        *           free poly_coe_of_item_c
        *       else
        *           new item_c
        *           deg(item_c) = 2 * deg(item_a);
        *           poly_coe(item_c) = poly_coe_of_item_c;
        *           bgn_coe_flag(item_c) = FALSE;
        *           insert item_c to the tail of poly_c
        *       end if
        *   end if
        */
        else
        {
            poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYZ2_0076);
            POLY_INIT(poly_coe_of_item_c);

            poly_z2_squ_poly(bgnz2_md_id, POLY_ITEM_POLY_COE(item_a), poly_coe_of_item_c);
            if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
            {
                poly_z2_free_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYZ2_0077);
            }
            else
            {
                poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0078);

                POLY_ITEM_DEG_ADD(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
        }

        item_a = POLY_ITEM_NEXT(item_a);
    }

    poly_z2_free_bgn(MD_POLYZ2, polyz2_md_id, MM_BIGINT, bgn_t_c1, LOC_POLYZ2_0079);

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
UINT32 poly_z2_sexp(const UINT32 polyz2_md_id,const POLY *poly_a,const UINT32 e,POLY *poly_c )
{
    POLY *poly_ta;
    UINT32 te;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_sexp: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_sexp: c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_sexp: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        /*before set zero, poly_z2_set_zero will clean up the items of poly_c*/
        poly_z2_set_zero(polyz2_md_id, poly_c);

        return ( 0 );
    }

    poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_ta, LOC_POLYZ2_0080);
    POLY_INIT(poly_ta);

    /*the branch is to decrease sharply the clone action of large number of items*/
    if ( poly_a == poly_c )
    {
        poly_z2_poly_move(polyz2_md_id, poly_c, poly_ta);
    }
    else
    {
        poly_z2_poly_clone(polyz2_md_id, poly_a, poly_ta);
    }

    /*before set one, poly_z2_set_one will clean up the items of poly_c*/
    poly_z2_set_one(polyz2_md_id, poly_c);

    te = e;
    while ( 1 )
    {
        if ( te & 1 )
        {
            poly_z2_mul_self(polyz2_md_id, poly_ta, poly_c);
        }
        te >>= 1;

        if ( 0 == te )
        {
            break;
        }

        poly_z2_squ_poly(polyz2_md_id, poly_ta, poly_ta);
    }

    poly_z2_poly_destory(polyz2_md_id, poly_ta);

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
UINT32 poly_z2_exp(const UINT32 polyz2_md_id,const POLY *poly_a,const BIGINT *e,POLY *poly_c )
{
    POLY   *poly_ta;

    UINT32 te_nbits;
    UINT32 e_bit;
    UINT32 index;

    POLYZ2_MD *polyz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_exp: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == e )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_exp: e is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_exp: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_exp: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;

    if ( EC_TRUE == POLY_IS_EMPTY( poly_a ) )
    {
        poly_z2_poly_clean(polyz2_md_id, poly_c);
        return ( 0 );
    }

    poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY,&poly_ta, LOC_POLYZ2_0081);
    POLY_INIT(poly_ta);

    poly_z2_poly_clone(polyz2_md_id, poly_a, poly_ta);

    te_nbits = bgn_z_get_nbits( bgnz_md_id, e );

    poly_z2_set_one(polyz2_md_id, poly_c);

    index = 0;

    while ( 1 )
    {
        e_bit = bgn_z_get_bit(bgnz_md_id, e, index);
        if ( 1 == e_bit )
        {
            poly_z2_mul_self(polyz2_md_id, poly_ta, poly_c);
        }

        index ++;
        if ( index == te_nbits )
        {
            break;
        }
        poly_z2_squ_poly(polyz2_md_id, poly_ta, poly_ta);
    }

    poly_z2_poly_destory(polyz2_md_id, poly_ta);

    return ( 0 );
}

/*Partial derivatives */
/**
*
*   poly_c = d(poly_c)/dx where x is the first var.
*
*poly_z2_dx_self := proc(poly_c)
*
*item_c = first_item(item_c);
*while item_c != null_item(item_c) do
*    if deg(item_c) is even then
*        item_t = item_c;
*        item_c = prev_item(item_c);
*        remove item_t from poly_c;
*        destory item_t;
*    else
*       deg(item_c) = deg(item_c) - 1;
*    end if
*    item_c = next_item(item_c);
*end do
*
*end proc
*
**/
static UINT32 poly_z2_dx_self(const UINT32 polyz2_md_id, POLY *poly_c )
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    DEGREE *deg_one;

    POLYZ2_MD  *polyz2_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_dx_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_dx_self: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;

    poly_z2_alloc_deg(MD_POLYZ2, polyz2_md_id, MM_DEGREE, &deg_one, LOC_POLYZ2_0082);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, deg_one);

    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        /*
        *   if deg(item_c) is even then
        *        item_t = item_c;
        *        item_c = prev_item(item_c);
        *        remove item_t from poly_c;
        *        destory item_t;
        *    else
        *       deg(item_c) = deg(item_c) - 1;
        *    end if
        */
        if ( EC_FALSE == POLY_ITEM_DEG_IS_ODD(bgnz_md_id, POLY_ITEM_DEG(item_c)) )
        {
            item_t = item_c;
            item_c = POLY_ITEM_PREV(item_c);

            POLY_ITEM_DEL(item_t);
            poly_z2_item_destory(polyz2_md_id, item_t);
        }
        else
        {
            POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_c), deg_one, POLY_ITEM_DEG(item_c));
        }
        /*
        *    item_c = next_item(item_c);
        */
        item_c = POLY_ITEM_NEXT(item_c);
    }

    poly_z2_free_deg(MD_POLYZ2, polyz2_md_id, MM_DEGREE, deg_one, LOC_POLYZ2_0083);

    return ( 0 );
}

/**
*
*   poly_c = d(poly_a)/dx where x is the first var.
*
*poly_z2_dx := proc(poly_a, poly_c)
*
*if poly_a == poly_c then
*    poly_z2_dx_self(poly_c);
*    return ;
*end if
*
*if poly_c is not empty then
*    clean up the items of poly_c;
*end if
*
*item_a = first_item(item_a);
*while item_a != null_item(item_a) do
*    if deg(item_a) is odd then
*       new item_c;
*       clone item_a to item_c;
*       deg(item_c) = deg(item_c) - 1;
*       insert item_c to poly_c;
*    end if
*    item_a = next_item(item_a);
*end do
*
*end proc
*
*end proc
*
**/
UINT32 poly_z2_dx(const UINT32 polyz2_md_id,const POLY *poly_a, POLY *poly_c )
{
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_c;

    DEGREE      *deg_one;

    POLYZ2_MD  *polyz2_md;
    UINT32  bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_dx: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_dx: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_dx: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *    poly_z2_dx_self(poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_z2_dx_self(polyz2_md_id, poly_c);
    }

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;
    bgnz2_md_id = polyz2_md->bgnz2_md_id;

    /*
    *if poly_c is not empty then
    *    clean up the items of poly_c;
    *end if
    */
    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_z2_poly_clean(polyz2_md_id, poly_c);
    }

    poly_z2_alloc_deg(MD_POLYZ2, polyz2_md_id, MM_DEGREE, &deg_one, LOC_POLYZ2_0084);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, deg_one);

    item_a = POLY_FIRST_ITEM(poly_a);
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*
        *    if deg(item_a) is odd then
        *       new item_c;
        *       clone item_a to item_c;
        *       deg(item_c) = deg(item_c) - 1;
        *       insert item_c to poly_c;
        *    end if
        */
        if ( EC_TRUE == POLY_ITEM_DEG_IS_ODD(bgnz_md_id,POLY_ITEM_DEG(item_a)) )
        {
            poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0085);

            poly_z2_item_clone(polyz2_md_id, item_a, item_c);
            POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_a), deg_one, POLY_ITEM_DEG(item_c));

            POLY_ADD_ITEM_TAIL(poly_c, item_c);
        }

        /*
        *    item_a = next_item(item_a);
        */
        item_a = POLY_ITEM_NEXT(item_a);
    }

    poly_z2_free_deg(MD_POLYZ2, polyz2_md_id, MM_DEGREE, deg_one, LOC_POLYZ2_0086);

    return ( 0 );
}

/**
*
*   poly_c = d(poly_c)/dx where x is the depth_of_x-th var.
*
*poly_z2_Dx_self := proc(depth_of_x, poly_c)
*
*if 0 == depth_of_x then
*    poly_z2_dx_self(poly_c);
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
*            poly_z2_Dx_self(depth_of_x - 1, poly_coe(item_c));
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
static UINT32 poly_z2_Dx_self(const UINT32 polyz2_md_id,const UINT32 depth_of_x, POLY *poly_c )
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    POLYZ2_MD  *polyz2_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_Dx_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_Dx_self: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if 0 == depth_of_x then
    *    poly_z2_dx_self(poly_c);
    *    return ;
    *end if
    */
    if ( 0 == depth_of_x )
    {
        return poly_z2_dx_self(polyz2_md_id, poly_c);
    }

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;

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
            poly_z2_item_destory(polyz2_md_id, item_t);
        }
        /*
        *        else
        *            poly_z2_Dx_self(depth_of_x - 1, poly_coe(item_c));
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
            poly_z2_Dx_self(polyz2_md_id, depth_of_x - 1, POLY_ITEM_POLY_COE(item_c));
            if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);
                poly_z2_item_destory(polyz2_md_id, item_t);
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
*poly_z2_Dx := proc(poly_a, depth_of_x, poly_c)
*
*if poly_a == poly_c then
*    poly_z2_Dx_self(depth_of_x, poly_c);
*    return ;
*end if
*
*if 0 == depth_of_x then
*    poly_z2_Dx(poly_a, poly_c);
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
*            poly_z2_Dx(poly_coe(item_a), depth_of_x - 1, poly_t);
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
UINT32 poly_z2_Dx(const UINT32 polyz2_md_id,const POLY *poly_a, const UINT32 depth_of_x, POLY *poly_c )
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    POLY      *poly_t;

    POLYZ2_MD  *polyz2_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_Dx: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_Dx: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_Dx: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *    poly_z2_Dx_self(depth_of_x, poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_z2_Dx_self(polyz2_md_id, depth_of_x, poly_c);
    }

    /*
    *if 0 == depth_of_x then
    *    poly_z2_dx(poly_a, poly_c);
    *    return ;
    *end if
    */
    if ( 0 == depth_of_x )
    {
        return poly_z2_dx( polyz2_md_id, poly_a, poly_c);
    }

    polyz2_md = &(g_polyz2_md[ polyz2_md_id ]);
    bgnz_md_id = polyz2_md->bgnz_md_id;

    /*
    *if poly_c is not empty then
    *    clean up the items of poly_c;
    *end if
    */
    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_z2_poly_clean(polyz2_md_id, poly_c);
    }

    item_a = POLY_FIRST_ITEM(poly_a);
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*
        *        if FALSE == bgn_coe_flag(item_a) then
        *            new  poly_t
        *            init poly_t
        *
        *            poly_z2_Dx(poly_coe(item_a), depth_of_x - 1, poly_t);
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
            poly_z2_alloc_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, &poly_t, LOC_POLYZ2_0087);
            POLY_INIT(poly_t);

            poly_z2_Dx(polyz2_md_id, POLY_ITEM_POLY_COE(item_a), depth_of_x - 1, poly_t);

            if ( EC_TRUE == POLY_IS_EMPTY(poly_t) )
            {
                poly_z2_free_poly(MD_POLYZ2, polyz2_md_id, MM_POLY, poly_t, LOC_POLYZ2_0088);
            }
            else
            {
                poly_z2_alloc_item(MD_POLYZ2, polyz2_md_id, MM_POLY_ITEM, &item_c, LOC_POLYZ2_0089);

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

UINT32 poly_z2_poly_output(const UINT32 polyz2_md_id,const POLY *poly, const UINT32 depth,const char *info)
{
    POLY_ITEM *item;
    UINT8  space_str[32];
    UINT32 index;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_output: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYZ2_MD <= polyz2_md_id || 0 == g_polyz2_md[ polyz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_z2_poly_output: polyz2 module #0x%lx not started.\n",
                polyz2_md_id);
        dbg_exit(MD_POLYZ2, polyz2_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( NULL_PTR != info )
    {
        dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"%s: ",info);
    }

    if ( depth >= 32 )
    {
        dbg_log(SEC_0022_POLYZ2, 0)(LOGSTDOUT,"error:poly_z2_poly_output: depth = %ld overflow\n");
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
        dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"%s deg: ", space_str);
        print_deg_dec(LOGSTDOUT, POLY_ITEM_DEG(item));

        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
        {
            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"%s coe: ", space_str);
            print_bigint_dec(LOGSTDOUT, POLY_ITEM_BGN_COE(item));
        }
        else
        {
            dbg_log(SEC_0022_POLYZ2, 5)(LOGSTDOUT,"%s coe: \n", space_str);
            poly_z2_poly_output(polyz2_md_id, POLY_ITEM_POLY_COE(item), depth + 1, NULL_PTR);
        }

        item = POLY_ITEM_NEXT(item);
    }

    return ( 0 );
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/



