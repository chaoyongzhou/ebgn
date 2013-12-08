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
#include <math.h>

#include "bgnctrl.h"
#include "type.h"
#include "moduleconst.h"
#include "mm.h"
#include "log.h"

#include "bgnz.h"
#include "ebgnz.h"
#include "ebgnr.h"

#include "conv.h"
#include "debug.h"

#include "print.h"

static EBGNR_MD g_ebgnr_md[ MAX_NUM_OF_EBGNR_MD ];
static EC_BOOL  g_ebgnr_md_init_flag = EC_FALSE;

/**
*   for test only
*
*   to query the status of EBGNR Module
*
**/
void print_ebgn_r_status()
{
    EBGNR_MD *ebgnr_md;
    UINT32 index;

    if ( EC_FALSE == g_ebgnr_md_init_flag )
    {
        sys_log(LOGSTDOUT,"no EBGNR Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_EBGNR_MD; index ++ )
    {
        ebgnr_md = &(g_ebgnr_md[ index ]);

        if ( 0 < ebgnr_md->usedcounter )
        {
            sys_log(LOGSTDOUT,"EBGNR Module # %ld : %ld refered, refer EBGNZ Module : %ld\n",
                    index,
                    ebgnr_md->usedcounter,
                    ebgnr_md->ebgnz_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed EBGNR module
*
*
**/
UINT32 ebgn_r_free_module_static_mem(const UINT32 ebgnr_md_id)
{
    EBGNR_MD  *ebgnr_md;
    UINT32  ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_free_module_static_mem: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    free_module_static_mem(MD_EBGNR, ebgnr_md_id);

    ebgn_z_free_module_static_mem(ebgnz_md_id);

    return 0;
}
/**
*
* start EBGNR module
*
**/
UINT32 ebgn_r_start( UINT32 dec_prec )
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnr_md_id;
    UINT32 ebgnz_md_id;
    UINT32 bgnz_md_id;

    UINT32 nbits;
    UINT32 bin_prec;

    UINT32 index;

    /* if this is the 1st time to start EBGNR module, then */
    /* initialize g_ebgnr_md */
    if ( EC_FALSE ==  g_ebgnr_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_EBGNR_MD; index ++ )
        {
            ebgnr_md = &(g_ebgnr_md[ index ]);

            ebgnr_md->usedcounter   = 0;
            ebgnr_md->dec_prec = 0;
            ebgnr_md->bin_prec = 0;
            ebgnr_md->one            = NULL_PTR;
            ebgnr_md->five_offset    = NULL_PTR;
            ebgnr_md->ebgnz_md_id = ERR_MODULE_ID;
            ebgnr_md->bgnz_md_id  = ERR_MODULE_ID;
        }

        /*register all functions of EBGNR module to DBG module*/
        //dbg_register_func_addr_list(g_ebgnr_func_addr_list, g_ebgnr_func_addr_list_len);

        g_ebgnr_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_EBGNR_MD; index ++ )
    {
        ebgnr_md = &(g_ebgnr_md[ index ]);

        if ( 0 == ebgnr_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_EBGNR_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    ebgnr_md_id = ERR_MODULE_ID;

    /* initilize new one EBGNR module */
    ebgnr_md_id = index;
    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);

    init_static_mem();

    ebgnz_md_id = ebgn_z_start();
    bgnz_md_id = bgn_z_start();

    ebgnr_md->ebgnz_md_id = ebgnz_md_id;
    ebgnr_md->bgnz_md_id  = bgnz_md_id;

    /*compute decimal prec*/
    ebgnr_md->dec_prec = dec_prec;

    /*ten_offset = 10^m where m is decimal precision*/
    ebgn_z_alloc_ebgn(ebgnz_md_id, &(ebgnr_md->ten_offset));
    ebgn_z_set_word(ebgnz_md_id, 10, ebgnr_md->ten_offset);
    ebgn_z_sexp(ebgnz_md_id, ebgnr_md->ten_offset, dec_prec, ebgnr_md->ten_offset);

    /*five_offset = 5^m where m is decimal precision*/
    ebgn_z_alloc_ebgn(ebgnz_md_id, &(ebgnr_md->five_offset));
    ebgn_z_shr(ebgnz_md_id, ebgnr_md->ten_offset, ebgnr_md->dec_prec, ebgnr_md->five_offset);

    /*compute binary prec: 2^(n-1) <= 10^m < 2^n*/
    ebgn_z_get_nbits(ebgnz_md_id, ebgnr_md->ten_offset, &nbits);

    /*now 2^(k + m -1 ) <= 10^m < 2^(k +m)*/
    bin_prec = ( ( nbits + WORDSIZE - 1 ) / WORDSIZE ) * WORDSIZE;
    //bin_prec = ( nbits + 1);
    //bin_prec = ( nbits );

    ebgnr_md->bin_prec = bin_prec;

    sys_log(LOGSTDOUT, "ebgn_r_start: binary precision = %d\n", ebgnr_md->bin_prec);

    /*set one in current space*/
    ebgn_z_alloc_ebgn(ebgnz_md_id, &(ebgnr_md->one));
    ebgn_z_set_one(ebgnz_md_id, ebgnr_md->one);
    ebgn_z_shl(ebgnz_md_id, ebgnr_md->one, bin_prec, ebgnr_md->one);

    ebgnr_md->usedcounter = 1;

    return ( ebgnr_md_id );
}

/**
*
* end EBGNR module
*
**/
void ebgn_r_end(const UINT32 ebgnr_md_id)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;
    UINT32 bgnz_md_id;

    if ( MAX_NUM_OF_EBGNR_MD < ebgnr_md_id )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_end: ebgnr_md_id = %ld is overflow.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < ebgnr_md->usedcounter )
    {
        ebgnr_md->usedcounter --;
        return ;
    }

    if ( 0 == ebgnr_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_end: ebgnr_md_id = %ld is not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }

    ebgnz_md_id = ebgnr_md->ebgnz_md_id;
    bgnz_md_id  = ebgnr_md->bgnz_md_id;

    /*free memory*/
    ebgn_z_destroy(ebgnz_md_id, ebgnr_md->one);
    ebgn_z_destroy(ebgnz_md_id, ebgnr_md->ten_offset);
    ebgn_z_destroy(ebgnz_md_id, ebgnr_md->five_offset);

    ebgn_z_end( ebgnz_md_id);
    bgn_z_end( bgnz_md_id);

    /* free module : */
    ebgnr_md->ebgnz_md_id = ERR_MODULE_ID;
    ebgnr_md->bgnz_md_id  = ERR_MODULE_ID;
    ebgnr_md->bin_prec = 0;
    ebgnr_md->dec_prec = 0;
    ebgnr_md->one = NULL_PTR;
    ebgnr_md->usedcounter = 0;

    breathing_static_mem();

    return ;
}

/**
*
*   alloc a EBGN type node from EBGNR space
*
**/
UINT32 ebgn_r_alloc_ebgn(const UINT32 ebgnr_md_id,EBGN **ppebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ppebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_alloc_ebgn: ppebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_alloc_ebgn: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, ppebgn);

    return ( 0 );
}

/**
*
*   free the EBGN type node from EBGNR space
*
**/
UINT32 ebgn_r_free_ebgn(const UINT32 ebgnr_md_id, EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_free_ebgn: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_free_ebgn: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_free_ebgn(ebgnz_md_id, ebgn);

    return ( 0 );
}

/**
*
*   destory the whole ebgn,i.e., all its items but not the ebgn itself.
*   so, when return from this function, ebgn can be refered again without any item.
*
**/
UINT32 ebgn_r_clean(const UINT32 ebgnr_md_id, EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_clean: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_clean: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_clean(ebgnz_md_id, ebgn);

    return ( 0 );
}

/**
*
*   destory the whole ebgn,including its all items and itself.
*   so, when return from this function, ebgn cannot be refered any more
*
**/
UINT32 ebgn_r_destroy(const UINT32 ebgnr_md_id, EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_destroy: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_destroy: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_destroy(ebgnz_md_id, ebgn);

    return ( 0 );
}

/**
*
*   ebgn_c = ebgn_a
*
**/
UINT32 ebgn_r_clone(const UINT32 ebgnr_md_id,const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_clone: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_clone: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_clone: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_c);

    return ( 0 );
}

/**
*
*   if ebgn = 0, return EC_TRUE
*   if ebgn != 0, return EC_FALSE
*
**/
EC_BOOL ebgn_r_is_zero(const UINT32 ebgnr_md_id, const EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_is_zero: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_is_zero: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_is_zero(ebgnz_md_id, ebgn);
}

/**
*
*   if ebgn = 1, return EC_TRUE
*   if ebgn != 1, return EC_FALSE
*
**/
EC_BOOL ebgn_r_is_one(const UINT32 ebgnr_md_id, const EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_is_one: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_is_one: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_is_one(ebgnz_md_id, ebgn);
}

/**
*
*   set ebgn = 0
*
**/
UINT32 ebgn_r_set_zero(const UINT32 ebgnr_md_id, EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_set_zero: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_set_zero: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_set_zero(ebgnz_md_id, ebgn);

    return (0);
}

/**
*
*   set ebgn = 1
*
**/
UINT32 ebgn_r_set_one(const UINT32 ebgnr_md_id,EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_set_one: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_set_one: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_clone(ebgnz_md_id, ebgnr_md->one, ebgn);

    return ( 0 );
}

/**
*
*   set ebgn = n
*
**/
UINT32 ebgn_r_set_word(const UINT32 ebgnr_md_id,const UINT32 n, EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_set_word: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_set_word: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_set_word(ebgnz_md_id, n, ebgn);
    ebgn_z_shl(ebgnz_md_id, ebgn, ebgnr_md->bin_prec, ebgn);

    return ( 0 );
}

/**
*
*   set ebgn = n
*
**/
UINT32 ebgn_r_set_n(const UINT32 ebgnr_md_id,const BIGINT *n, EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_set_n: n is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_set_n: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_set_n: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_set_n(ebgnz_md_id, n, ebgn);
    ebgn_z_shl(ebgnz_md_id, ebgn, ebgnr_md->bin_prec, ebgn);

    return ( 0 );
}

/**
*
*   set ebgn = x
*
**/
UINT32 ebgn_r_set_real(const UINT32 ebgnr_md_id, const REAL *x, EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

    EBGN *t0;
    EBGN *t1;
    UINT32 bit_20th;
    UINT32 nbits;

    REAL_FORMAT *x_fmt;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_set_real: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_set_real: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    bit_20th = (1 << 20);

    x_fmt = (REAL_FORMAT *)x;

    //sys_log(LOGSTDOUT, "x %f, m0: %lx, m1: %lx, p-1023: %d, s:%d\n", *x, x_fmt->m0, x_fmt->m1, bin_prec, x_fmt->s);

    ebgn_z_alloc_ebgn(ebgnz_md_id, &t0);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &t1);

    ebgn_z_set_word(ebgnz_md_id, x_fmt->m0, t0);
    ebgn_z_set_word(ebgnz_md_id, (x_fmt->m1 | bit_20th), t1);

    ebgn_z_shl(ebgnz_md_id, t1, WORDSIZE, t1);
    ebgn_z_add(ebgnz_md_id, t0, t1, t1);

    while( 0 == ebgn_z_get_bit(ebgnz_md_id, t1, 0))
    {
        ebgn_z_shr(ebgnz_md_id, t1, 1, t1);
    }

    ebgn_z_get_nbits(ebgnz_md_id, t1, &nbits);

    /*how the bin digitals after radix should be nbits - 1 - (p - 1023)*/

    if( nbits - 1 + 1023 < ebgnr_md->bin_prec + x_fmt->p )
    {
        ebgn_z_shl(ebgnz_md_id, t1, (ebgnr_md->bin_prec + x_fmt->p) - (nbits - 1 + 1023), t1);
    }
    else
    {
        ebgn_z_shr(ebgnz_md_id, t1, (nbits - 1 + 1023) - (ebgnr_md->bin_prec + x_fmt->p), t1);
    }
    ebgn_r_clone(ebgnr_md_id, t1, ebgn);

    if( 1 == x_fmt->s )
    {
        ebgn_r_neg(ebgnz_md_id, ebgn, ebgn);
    }

    ebgn_z_free_ebgn(ebgnz_md_id, t0);
    ebgn_z_free_ebgn(ebgnz_md_id, t1);

    return (0);
}

/**
*
*   if ebgn_a = ebgn_b then return 0
*   if ebgn_a > ebgn_b then return 1
*   if ebgn_a < ebgn_b then return -1
*
**/
int ebgn_r_cmp(const UINT32 ebgnr_md_id,const EBGN *ebgn_a, const EBGN *ebgn_b)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_cmp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_cmp: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_cmp: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_cmp(ebgnz_md_id, ebgn_a, ebgn_b);
}

/*
*
*   if abs(ebgn_a) = abs(ebgn_b) then return 0
*   if abs(ebgn_a) > abs(ebgn_b) then return 1
*   if abs(ebgn_a) < abs(ebgn_b) then return -1
*
**/
int ebgn_r_abs_cmp(const UINT32 ebgnr_md_id,const EBGN *ebgn_a, const EBGN *ebgn_b)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_abs_cmp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_abs_cmp: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_abs_cmp: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_abs_cmp(ebgnz_md_id, ebgn_a, ebgn_b);
}

/**
*
*    move ebgn_a to ebgn_c, ebgn_c = ebgn_a, and ebgn_a return null
*
**/
UINT32 ebgn_r_move(const UINT32 ebgnr_md_id, EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_move: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_move: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_move: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_move(ebgnz_md_id, ebgn_a, ebgn_c);
}

/**
*
*
*  ebgn_b = -ebgn_a
*
*  note, maybe address of ebgn_a = address of ebgn_b
*
**/
UINT32 ebgn_r_neg(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_b)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_neg: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_neg: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_neg: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_neg(ebgnz_md_id, ebgn_a, ebgn_b);
}

/**
*
*
*  ebgn_b = abs(ebgn_a)
*
*  note, maybe address of ebgn_a = address of ebgn_b
*
**/
UINT32 ebgn_r_abs(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_b)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_abs: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_abs: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_abs: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_abs(ebgnz_md_id, ebgn_a, ebgn_b);
}

/**
*
*   ebgn ++
*
**/
UINT32 ebgn_r_inc(const UINT32 ebgnr_md_id, EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;
    EBGN *one;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_inc: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_inc: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;
    one = ebgnr_md->one;

    return ebgn_z_add(ebgnz_md_id, ebgn, one, ebgn);
}

/**
*
*
*   ebgn --
*
**/
UINT32 ebgn_r_dec(const UINT32 ebgnr_md_id, EBGN *ebgn)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;
    EBGN *one;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_dec: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_dec: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;
    one = ebgnr_md->one;

    return ebgn_z_sub(ebgnz_md_id, ebgn, one, ebgn);
}

/**
*
*
*   ebgn_c = ebgn_a + ebgn_b
*
**/
UINT32 ebgn_r_add(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_add: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_add: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_add: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_add: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_add(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
}

/**
*
*
*   ebgn_c = ebgn_a - ebgn_b
*
**/
UINT32 ebgn_r_sub(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sub: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sub: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sub: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_sub: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_sub(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
}

/**
*
*
*   ebgn_c = ebgn_a * bgn_b
*
**/
UINT32 ebgn_r_smul(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const BIGINT *bgn_b, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_smul: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == bgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_smul: bgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_smul: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_smul: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    return ebgn_z_smul(ebgnz_md_id, ebgn_a, bgn_b, ebgn_c);
}

/**
*
*
*   ebgn_c = ebgn_a * ebgn_b
*
**/
UINT32 ebgn_r_mul(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_mul: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_mul: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_mul: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_mul: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_mul(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
    ebgn_z_shr(ebgnz_md_id, ebgn_c, ebgnr_md->bin_prec, ebgn_c);

    return ( 0 );
}

/**
*
*
*   ebgn_c = ebgn_a ^ 2
*
**/
UINT32 ebgn_r_squ(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_squ: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_squ: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_squ: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_z_squ(ebgnz_md_id, ebgn_a, ebgn_c);
    ebgn_z_shr(ebgnz_md_id, ebgn_c, ebgnr_md->bin_prec, ebgn_c);

    return ( 0 );
}

/**
*
*
*  ebgn_a = ebgn_b * ebgn_q + ebgn_r
*
**/
UINT32 ebgn_r_div(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_q)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

    EBGN *ebgn_ta;
    EBGN *ebgn_tr;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_div: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_div: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_q )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_div: ebgn_q is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_r_div: ebgnr module #0x%lx not started.\n",
                ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_r_alloc_ebgn(ebgnz_md_id, &ebgn_ta);
    ebgn_r_alloc_ebgn(ebgnz_md_id, &ebgn_tr);

#if 0
    print_ebgn_dec_info(LOGSTDOUT, ebgn_a, "ebgn_r_div: ebgn_a : ");
    print_ebgn_dec_info(LOGSTDOUT, ebgn_b, "ebgn_r_div: ebgn_b : ");
#endif

    ebgn_z_shl(ebgnz_md_id, ebgn_a, ebgnr_md->bin_prec, ebgn_ta);
#if 0
    print_ebgn_dec_info(LOGSTDOUT, ebgn_ta, "ebgn_r_div: ebgn_ta : ");
#endif

    ebgn_z_div(ebgnz_md_id, ebgn_ta, ebgn_b, ebgn_q, ebgn_tr);
#if 0
    print_ebgn_dec_info(LOGSTDOUT, ebgn_q, "ebgn_r_div: ebgn_q : ");
    print_ebgn_dec_info(LOGSTDOUT, ebgn_tr, "ebgn_r_div: ebgn_tr : ");
#endif

    ebgn_r_destroy(ebgnz_md_id, ebgn_ta);
    ebgn_r_destroy(ebgnz_md_id, ebgn_tr);

    return ( 0 );
}

/**
*       c = ( a ^ e )
*
*   Note:
*       let e = 2 * e1 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e
*   Output: c such that c = ( a ^ e )
*   1.  set c = 1
*   2.  while ( 1 ) do
*   2.1     if e is odd, then do
*               c = c * a
*   2.2     end if
*   2.3     e  = ( e >> 1)
*   2.4     if e is zero, then break while, end if
*   2.5     a = a ^ 2
*   2.6     next while
*   3.  return c
*
**/
UINT32 ebgn_r_sexp(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const UINT32 e, EBGN *ebgn_c)
{
    EBGN *ebgn_ta;
    UINT32 te;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sexp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sexp: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sexp: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_TRUE == ebgn_r_is_zero( ebgnr_md_id, ebgn_a ) )
    {
        ebgn_r_set_zero( ebgnr_md_id, ebgn_c );
        return ( 0 );
    }

    if( 0 == e || EC_TRUE == ebgn_r_is_one( ebgnr_md_id, ebgn_a ) )
    {
        ebgn_r_set_one( ebgnr_md_id, ebgn_c );
        return ( 0 );
    }

    ebgn_r_alloc_ebgn(ebgnr_md_id, &ebgn_ta);

    te = e;
    ebgn_r_clone(ebgnr_md_id, ebgn_a, ebgn_ta);
    ebgn_r_set_one(ebgnr_md_id, ebgn_c);

    while ( 1 )
    {
        if ( te & 1 )
        {
            ebgn_r_mul(ebgnr_md_id, ebgn_c, ebgn_ta, ebgn_c);
        }
        te >>= 1;

        if ( 0 == te )
        {
            break;
        }

        ebgn_r_squ(ebgnr_md_id, ebgn_ta, ebgn_ta);
    }

    ebgn_r_destroy(ebgnr_md_id, ebgn_ta);

    return ( 0 );
}

/**
*       c = a ^ e
*
*   Note:
*       let e = e1 * 2 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e
*   Output: c such that c = ( a ^ e )
*   0.  let e = SUM(e_i * 2 ^ i, i = 0..m)
*   1.  set c = 1
*   2.  set i = 0
*   3.  while ( 1 ) do
*   3.1     if e_i is odd, then do
*               c = c * a
*   3.2     end if
*   3.3     i = i + 1
*   3.4     if i = m + 1, then break while, end if
*   3.5     a = a ^ 2
*   3.6     next while
*   4.  return c
*
**/
UINT32 ebgn_r_exp(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const BIGINT *bgn_e, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;
    UINT32 bgnz_md_id;

    EBGN *ebgn_ta;

    UINT32 nbits_of_e;
    UINT32 e_bit;
    UINT32 index;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_exp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == bgn_e )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_exp: bgn_e is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_exp: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_exp: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_TRUE == ebgn_r_is_zero( ebgnr_md_id, ebgn_a ) )
    {
        ebgn_r_set_zero( ebgnr_md_id, ebgn_c );
        return ( 0 );
    }

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;
    bgnz_md_id = ebgnr_md->bgnz_md_id;

    if ( EC_TRUE == ebgn_r_is_one(ebgnr_md_id, ebgn_a)
      || EC_TRUE == bgn_z_is_zero(bgnz_md_id, bgn_e))
    {
        ebgn_r_set_one( ebgnr_md_id, ebgn_c );
        return ( 0 );
    }

    ebgn_r_alloc_ebgn(ebgnr_md_id, &ebgn_ta);
    ebgn_r_clone(ebgnr_md_id, ebgn_a, ebgn_ta);

    nbits_of_e = bgn_z_get_nbits( bgnz_md_id, bgn_e);

    ebgn_r_set_one( ebgnr_md_id, ebgn_c );
    index = 0;

    while ( 1 )
    {
        e_bit = bgn_z_get_bit(bgnz_md_id, bgn_e, index);
        if ( 1 == e_bit )
        {
            ebgn_r_mul(ebgnr_md_id, ebgn_c, ebgn_ta, ebgn_c);
        }

        index ++;

        /*modify index == te_nbits to index >= te_nbits */
        /*in order to deal with e = 0*/
        if ( index >= nbits_of_e )
        {
            break;
        }
        ebgn_r_squ(ebgnr_md_id, ebgn_ta, ebgn_ta);
    }

    ebgn_r_destroy(ebgnr_md_id, ebgn_ta);

    return ( 0 );
}

/**
*   Squroot of a
*
*   return c such that c^2 = a
*
*  Algorithm ( Newton - Iteration ):
*  input: n
*  output: x such that x^2 = n
*  1. x0 = 1
*  2. x1 = (1/2)*(x0 + n/x0)
*  3. if abs(x1 - x0) = 0, return x0;
*  4. else x0 <---> x1
*  5. goto 2
*
**/
UINT32 ebgn_r_sqrt(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;
    UINT32 bgnz_md_id;

    EBGN *tmp;
    EBGN *delta;
    EBGN *t0;
    EBGN *t1;
    EBGN *t;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sqrt: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sqrt: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sqrt: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if( EC_FALSE == EBGN_SGN(ebgn_a) )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_sqrt: ebgn_a < 0\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_TRUE == ebgn_r_is_zero(ebgnr_md_id, ebgn_a) )
    {
        ebgn_r_set_zero(ebgnr_md_id, ebgn_c);
        return ( 0 );
    }

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;
    bgnz_md_id = ebgnr_md->bgnz_md_id;

    ebgn_r_alloc_ebgn(ebgnr_md_id, &tmp);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &delta);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t0);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t1);

    ebgn_r_set_one(ebgnr_md_id, t0);

    for(;;)
    {
        ebgn_r_div(ebgnr_md_id, ebgn_a, t0, tmp);

        /*compute delta*/
        ebgn_r_sub(ebgnr_md_id, tmp, t0, delta);
        ebgn_z_shr(ebgnz_md_id, delta, 1, delta);

        if( EC_TRUE == ebgn_r_is_zero(ebgnr_md_id, delta) )
        {
            break;
        }

        ebgn_r_add(ebgnr_md_id, tmp, t0, t1);
        ebgn_z_shr(ebgnz_md_id, t1, 1, t1);

        t = t0;
        t0 = t1;
        t1 = t;
    }

    ebgn_r_move(ebgnr_md_id, t0, ebgn_c);

    ebgn_r_destroy(ebgnr_md_id, tmp);
    ebgn_r_destroy(ebgnr_md_id, delta);
    ebgn_r_destroy(ebgnr_md_id, t0);
    ebgn_r_destroy(ebgnr_md_id, t1);

    return ( 0 );
}

/**
*
*
*   ebgn_c = 1 / ebgn_a
*
* where ebgn_a != 0
*
**/
UINT32 ebgn_r_inv(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_inv: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_inv: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_inv: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    if( EC_TRUE == ebgn_r_is_zero(ebgnr_md_id, ebgn_a) )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_inv: ebgn_a = 0\n");
        return ((UINT32)(-1));
    }

    ebgn_r_div(ebgnr_md_id, ebgnr_md->one, ebgn_a, ebgn_c);

    return (0);
}

/**
*
*
*   ebgn_c = ln(ebgn_a)
*
*  Algorithm ( Newton - Iteration ):
*  input: n
*  output: x such that e^x = n
*  1. x0 = 1
*  2. x1 = x0 + n/(e^x0) -1
*  3. if abs(x1 - x0) = 0, return x0;
*  4. else x0 <---> x1
*  5. goto 2
*
**/
UINT32 ebgn_r_ln(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

    EBGN *t_x0;
    EBGN *t_x1;
    EBGN *t;
    EBGN *t_tmp;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_ln: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_ln: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_ln: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_x0);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_x1);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_tmp);

    ebgn_r_set_zero(ebgnr_md_id, t_x0);

    for(;;)
    {
        ebgn_r_eexp(ebgnr_md_id, t_x0, t_tmp);
        ebgn_r_div(ebgnr_md_id, ebgn_a, t_tmp, t_tmp);
        ebgn_r_dec(ebgnr_md_id, t_tmp);

        if( EC_TRUE == ebgn_r_is_zero(ebgnr_md_id, t_tmp) )
        {
            break;
        }

        ebgn_r_add(ebgnr_md_id, t_x0, t_tmp, t_x1);
        t = t_x0;
        t_x0 = t_x1;
        t_x1 = t;
    }

    ebgn_r_move(ebgnr_md_id, t_x0, ebgn_c);

    ebgn_r_destroy(ebgnr_md_id, t_x0);
    ebgn_r_destroy(ebgnr_md_id, t_x1);
    ebgn_r_destroy(ebgnr_md_id, t_tmp);

    return (0);
}

/**
*
*
*   ebgn_c = e^x = 1 + SUM( (x ^ i) / i!, i = 1,2,...)
*
* where x = ebgn_a
*
**/
UINT32 ebgn_r_eexp(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

    EBGN *t_idx;
    EBGN *t_tmp;
    EBGN *t_sum;
    EBGN *t_upp;
    EBGN *t_low;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_eexp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_eexp: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_eexp: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_idx);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_upp);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_low);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_sum);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_tmp);

    ebgn_r_set_one(ebgnr_md_id, t_sum);
    ebgn_r_set_one(ebgnr_md_id, t_upp);
    ebgn_r_set_one(ebgnr_md_id, t_low);

    ebgn_r_set_zero(ebgnr_md_id, t_idx);

    for(;;)
    {
        ebgn_r_inc(ebgnr_md_id, t_idx);
        ebgn_r_mul(ebgnr_md_id, ebgn_a, t_upp, t_upp); /*x^i*/
        ebgn_r_mul(ebgnr_md_id, t_idx , t_low, t_low); /*i!*/
        ebgn_r_div(ebgnr_md_id, t_upp, t_low, t_tmp);

        if( EC_TRUE == ebgn_r_is_zero(ebgnr_md_id, t_tmp) )
        {
            break;
        }

        ebgn_r_add(ebgnr_md_id, t_sum, t_tmp, t_sum);
    }

    ebgn_r_move(ebgnr_md_id, t_sum, ebgn_c);

    ebgn_r_destroy(ebgnr_md_id, t_idx);
    ebgn_r_destroy(ebgnr_md_id, t_upp);
    ebgn_r_destroy(ebgnr_md_id, t_low);
    ebgn_r_destroy(ebgnr_md_id, t_sum);
    ebgn_r_destroy(ebgnr_md_id, t_tmp);

    return (0);
}

/**
*
*
*   ebgn_c = x ^ y = e^(y*ln(x))
*
* where x = ebgn_a, y = ebgn_b
*
**/
UINT32 ebgn_r_dexp(const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

    EBGN *t_lnx;
    EBGN *t_tmp;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_dexp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_dexp: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_dexp: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_dexp: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_lnx);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_tmp);

    ebgn_r_ln(ebgnr_md_id, ebgn_a, t_lnx);
    ebgn_r_mul(ebgnr_md_id, ebgn_b, t_lnx, t_tmp);
    ebgn_r_eexp(ebgnr_md_id, t_tmp, ebgn_c);

    ebgn_r_destroy(ebgnr_md_id, t_lnx);
    ebgn_r_destroy(ebgnr_md_id, t_tmp);

    return (0);
}

/**
*
*
*   ebgn_c = pi
* where pi serial is
*     4 * SUM((-1)^i/(2*i+1), i = 0,1,2,...)
* which is slow
* or
*     2 * SUM(1 + (i!/(2*i+1)!!), i = 0,1,2,...)
* which is fast
*
*
**/
UINT32 ebgn_r_pi(const UINT32 ebgnr_md_id, EBGN *ebgn_c)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

    EBGN *t_idx;
    EBGN *t_odd;
    EBGN *t_upp;
    EBGN *t_low;
    EBGN *t_tmp;
    EBGN *t_sum;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_pi: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_pi: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_idx);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_odd);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_upp);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_low);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_tmp);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_sum);

    ebgn_r_set_one(ebgnr_md_id, t_sum);

    ebgn_r_set_zero(ebgnr_md_id, t_idx);
    ebgn_r_set_one(ebgnr_md_id, t_odd);

    ebgn_r_set_one(ebgnr_md_id, t_upp);
    ebgn_r_set_one(ebgnr_md_id, t_low);

    for(;;)
    {
        ebgn_r_inc(ebgnr_md_id, t_idx);

        ebgn_r_inc(ebgnr_md_id, t_odd);
        ebgn_r_inc(ebgnr_md_id, t_odd);

        ebgn_r_mul(ebgnr_md_id, t_upp, t_idx, t_upp);
        ebgn_r_mul(ebgnr_md_id, t_low, t_odd, t_low);

        ebgn_r_div(ebgnr_md_id, t_upp, t_low, t_tmp);
#if 0
        ebgn_r_print_dec_info(LOGSTDOUT, ebgnr_md_id, t_odd, "ebgn_r_pi: t_odd = ");
        ebgn_r_print_bin_info(LOGSTDOUT, ebgnr_md_id, t_tmp, "ebgn_r_pi: t_tmp = ");
        ebgn_r_print_dec_info(LOGSTDOUT, ebgnr_md_id, t_sum, "ebgn_r_pi: t_sum = ");
#endif

        if( EC_TRUE == ebgn_r_is_zero(ebgnr_md_id, t_tmp) )
        {
            break;
        }

        ebgn_r_add(ebgnr_md_id, t_sum, t_tmp, t_sum);
    }
    ebgn_z_shl(ebgnz_md_id, t_sum, 1, ebgn_c);

    ebgn_r_destroy(ebgnr_md_id, t_idx);
    ebgn_r_destroy(ebgnr_md_id, t_odd);
    ebgn_r_destroy(ebgnr_md_id, t_upp);
    ebgn_r_destroy(ebgnr_md_id, t_low);
    ebgn_r_destroy(ebgnr_md_id, t_tmp);
    ebgn_r_destroy(ebgnr_md_id, t_sum);

    return (0);
}

/* ------------------------------------------ output interface ------------------------------------------ */
static UINT32 ebgn_r_str_rsh(const UINT32 count, const UINT32 len, const UINT32 maxlen, UINT8 *str)
{
    UINT32 idx;

    UINT32 from;
    UINT32 to;
    UINT32 cur_len;

    if( 0 == count )
    {
        return (len);
    }

    if( len + count < maxlen )
    {
        from = len + count;
        to = count;
        cur_len = len + count;
    }
    else
    {
        from = maxlen - 1;
        to = count;
        cur_len = maxlen;
    }

    for( idx = from; idx >= to; idx -- )
    {
        str[ idx ] = str[ idx - count ];
    }

    for( ++ idx; idx -- > 0; )
    {
       str[ idx ] = '0';
    }
    return (cur_len);
}

UINT32 ebgn_r_print_dec_info(LOG *log, const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const char *info)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

    EBGN *t_a0;
    EBGN *t_a1;
    EBGN *t_a2;

    UINT8  *decstr;
    UINT32 decstr_maxlen;
    UINT32 decstr_retlen;
    UINT32 decstr_sumlen;
    UINT32 decstr_offset;

    UINT32 conv_md_id;


#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_print_dec_info: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_print_dec_info: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_a0);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_a1);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_a2);

#if 0
    print_ebgn_bin_info(log, ebgn_a, "ebgn_r_print_dec_info: ebgn_a: ");
#endif

    /*a = a1 + a0 * (2^(-n)) = a1 + a2 * (10 ^(-m))*/
    ebgn_z_cut(ebgnz_md_id, ebgn_a, ebgnr_md->bin_prec, t_a0); /*t_a0 = ebgn_a / (2^n)*/
    ebgn_z_shr(ebgnz_md_id, ebgn_a, ebgnr_md->bin_prec, t_a1); /*t_a1 = ebgn_a % (2^n)*/

    /*convert to decimal: t_a2 = (a0 * 10^m)/(2^n) = (a0 * 5^m)/(2^(n-m))*/
    ebgn_z_mul(ebgnz_md_id, t_a0, ebgnr_md->five_offset, t_a2);
    ebgn_z_shr(ebgnz_md_id, t_a2, ebgnr_md->bin_prec - ebgnr_md->dec_prec, t_a2);

    if( info != NULL_PTR )
    {
        sys_print(log,"%s ", info);
    }

#if 0
    print_ebgn_bin_info(log, t_a1, "ebgn_r_print_dec_info: t_a1: ");
    print_ebgn_bin_info(log, t_a0, "ebgn_r_print_dec_info: t_a0: ");
    print_ebgn_bin_info(log, t_a2, "ebgn_r_print_dec_info: t_a2: ");
#else
    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_print(LOGSTDERR,"ebgn_r_print_dec_info: no CONV module available\n");
        return ((UINT32)-1);
    }

    alloc_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, &decstr, LOC_EBGNR_0001);
    decstr_maxlen = 4 * 1024;
    decstr_sumlen = 0;

    conv_ebgn_to_dec(conv_md_id, t_a1, decstr, decstr_maxlen, &decstr_retlen);
    decstr_sumlen += decstr_retlen;

    decstr[ decstr_sumlen ++ ] = '.';

    conv_ebgn_to_dec(conv_md_id, t_a2, decstr + decstr_sumlen, decstr_maxlen - decstr_sumlen, &decstr_retlen);
    if( decstr_retlen < ebgnr_md->dec_prec && (!(1 == decstr_retlen && *(decstr + decstr_sumlen) == '0')))
    {
        decstr_offset = ebgnr_md->dec_prec - decstr_retlen + 1;
        decstr_retlen = ebgn_r_str_rsh(decstr_offset, decstr_retlen, decstr_maxlen - decstr_sumlen, decstr + decstr_sumlen);
    }
    decstr_sumlen += decstr_retlen;

    print_str(log, decstr, decstr_sumlen);

    free_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, decstr, LOC_EBGNR_0002);

    conv_end(conv_md_id);
    //print_ebgn_dec_info(log, t_a1, "ebgn_r_print_dec_info: t_a1: ");
    //print_ebgn_dec_info(log, t_a0, "ebgn_r_print_dec_info: t_a0: ");
    //print_ebgn_dec_info(log, t_a2, "ebgn_r_print_dec_info: t_a2: ");
#endif

    ebgn_r_destroy(ebgnr_md_id, t_a0);
    ebgn_r_destroy(ebgnr_md_id, t_a1);
    ebgn_r_destroy(ebgnr_md_id, t_a2);

    return (0);
}

UINT32 ebgn_r_print_bin_info(LOG *log, const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const char *info)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

    EBGN *t_a0;
    EBGN *t_a1;

    UINT8  *binstr;
    UINT32 binstr_maxlen;
    UINT32 binstr_retlen;
    UINT32 binstr_sumlen;
    UINT32 conv_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_print_bin_info: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_print_bin_info: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_a0);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_a1);

#if 0
    print_ebgn_bin_info(log, ebgn_a, "ebgn_r_print_bin_info: ebgn_a: ");
#endif

    /*a = a1 + a0 * (2^(-n))*/
    ebgn_z_cut(ebgnz_md_id, ebgn_a, ebgnr_md->bin_prec, t_a0); /*t_a0 = ebgn_a / (2^n)*/
    ebgn_z_shr(ebgnz_md_id, ebgn_a, ebgnr_md->bin_prec, t_a1); /*t_a1 = ebgn_a % (2^n)*/

    if( info != NULL_PTR )
    {
        sys_print(log,"%s ", info);
    }

    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_print(LOGSTDERR,"ebgn_r_print_bin_info: no CONV module available\n");
        return ((UINT32)-1);
    }

    alloc_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, &binstr, LOC_EBGNR_0003);
    binstr_maxlen = 4 * 1024;
    binstr_sumlen = 0;

    conv_ebgn_to_bin(conv_md_id, t_a1, binstr, binstr_maxlen, &binstr_retlen);
    binstr_sumlen += binstr_retlen;

    binstr[ binstr_sumlen ++ ] = '.';

    conv_ebgn_to_bin(conv_md_id, t_a0, binstr + binstr_sumlen, binstr_maxlen - binstr_sumlen, &binstr_retlen);
    binstr_sumlen += binstr_retlen;

    print_str(log, binstr, binstr_sumlen);

    free_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, binstr, LOC_EBGNR_0004);

    conv_end(conv_md_id);

    ebgn_r_destroy(ebgnr_md_id, t_a0);
    ebgn_r_destroy(ebgnr_md_id, t_a1);

    return (0);
}

UINT32 ebgn_r_print_hex_info(LOG *log, const UINT32 ebgnr_md_id, const EBGN *ebgn_a, const char *info)
{
    EBGNR_MD *ebgnr_md;
    UINT32 ebgnz_md_id;

    EBGN *t_a0;
    EBGN *t_a1;

    UINT8  *hexstr;
    UINT32 hexstr_maxlen;
    UINT32 hexstr_retlen;
    UINT32 hexstr_sumlen;
    UINT32 conv_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_print_hex_info: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNR_MD <= ebgnr_md_id || 0 == g_ebgnr_md[ ebgnr_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_r_print_hex_info: ebgnr module #0x%lx not started.\n",ebgnr_md_id);
        dbg_exit(MD_EBGNR, ebgnr_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnr_md = &(g_ebgnr_md[ ebgnr_md_id ]);
    ebgnz_md_id = ebgnr_md->ebgnz_md_id;

    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_a0);
    ebgn_r_alloc_ebgn(ebgnr_md_id, &t_a1);

#if 0
    print_ebgn_bin_info(log, ebgn_a, "ebgn_r_print_hex_info: ebgn_a: ");
#endif

    /*a = a1 + a0 * (2^(-n))*/
    ebgn_z_cut(ebgnz_md_id, ebgn_a, ebgnr_md->bin_prec, t_a0); /*t_a0 = ebgn_a / (2^n)*/
    ebgn_z_shr(ebgnz_md_id, ebgn_a, ebgnr_md->bin_prec, t_a1); /*t_a1 = ebgn_a % (2^n)*/

    if( info != NULL_PTR )
    {
        sys_print(log,"%s ", info);
    }

    conv_md_id = conv_start();
    if( ERR_MODULE_ID == conv_md_id )
    {
        sys_print(LOGSTDERR,"ebgn_r_print_hex_info: no CONV module available\n");
        return ((UINT32)-1);
    }

    alloc_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, &hexstr, LOC_EBGNR_0005);
    hexstr_maxlen = 4 * 1024;
    hexstr_sumlen = 0;

    conv_ebgn_to_hex(conv_md_id, t_a1, hexstr, hexstr_maxlen, &hexstr_retlen);
    hexstr_sumlen += hexstr_retlen;

    hexstr[ hexstr_sumlen ++ ] = '.';

    conv_ebgn_to_hex(conv_md_id, t_a0, hexstr + hexstr_sumlen, hexstr_maxlen - hexstr_sumlen, &hexstr_retlen);
    hexstr_sumlen += hexstr_retlen;

    print_str(log, hexstr, hexstr_sumlen);

    free_static_mem(MD_CONV, conv_md_id, MM_STRCHAR4K, hexstr, LOC_EBGNR_0006);

    conv_end(conv_md_id);

    ebgn_r_destroy(ebgnr_md_id, t_a0);
    ebgn_r_destroy(ebgnr_md_id, t_a1);

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
