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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <sys/mman.h>

#include <sys/stat.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "carray.h"
#include "cvector.h"

#include "cbc.h"
#include "ctimer.h"
#include "cbtimer.h"
#include "cmisc.h"

#include "task.h"

#include "cmpie.h"

#include "cxmppc2s.h"
#include "cxmpp.h"

#include "findex.inc"

#define CXMPPC2S_MD_CAPACITY()                  (cbc_md_capacity(MD_CXMPPC2S))

#define CXMPPC2S_MD_GET(cxmppc2s_md_id)     ((CXMPPC2S_MD *)cbc_md_get(MD_CXMPPC2S, (cxmppc2s_md_id)))

#define CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id)  \
    ((CMPI_ANY_MODI != (cxmppc2s_md_id)) && ((NULL_PTR == CXMPPC2S_MD_GET(cxmppc2s_md_id)) || (0 == (CXMPPC2S_MD_GET(cxmppc2s_md_id)->usedcounter))))


/**
*   for test only
*
*   to query the status of CXMPPC2S Module
*
**/
void cxmppc2s_print_module_status(const UINT32 cxmppc2s_md_id, LOG *log)
{
    CXMPPC2S_MD *cxmppc2s_md;
    UINT32 this_cxmppc2s_md_id;

    for( this_cxmppc2s_md_id = 0; this_cxmppc2s_md_id < CXMPPC2S_MD_CAPACITY(); this_cxmppc2s_md_id ++ )
    {
        cxmppc2s_md = CXMPPC2S_MD_GET(this_cxmppc2s_md_id);

        if ( NULL_PTR != cxmppc2s_md && 0 < cxmppc2s_md->usedcounter )
        {
            sys_log(log,"CXMPPC2S Module # %u : %u refered\n",
                    this_cxmppc2s_md_id,
                    cxmppc2s_md->usedcounter);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed CXMPPC2S module
*
*
**/
UINT32 cxmppc2s_free_module_static_mem(const UINT32 cxmppc2s_md_id)
{
    CXMPPC2S_MD  *cxmppc2s_md;

#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_free_module_static_mem: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*CXMPPC2S_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    free_module_static_mem(MD_CXMPPC2S, cxmppc2s_md_id);

    return 0;
}

UINT32 cxmppc2s_find_module(const uint8_t *usrname)
{
    CXMPPC2S_MD *cxmppc2s_md;
    UINT32       cxmppc2s_md_id;

    for( cxmppc2s_md_id = 1; cxmppc2s_md_id < CXMPPC2S_MD_CAPACITY(); cxmppc2s_md_id ++ )
    {
        cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

        if ( NULL_PTR != cxmppc2s_md && 0 < cxmppc2s_md->usedcounter )
        {
            if(EC_TRUE == cstring_is_str_ignore_case(CXMPPC2S_MD_USRNAME(cxmppc2s_md), usrname))
            {
                return (cxmppc2s_md_id);
            }
        }
    }
    return (ERR_MODULE_ID);
}


UINT32 cxmppc2s_find_module_by_jid(const CSTRING *jid)
{
    UINT32       cxmppc2s_md_id;
    
    char        *to_save;
    char        *fields[1];
    UINT32       field_num;

    to_save = c_str_dup((const char *)cstring_get_str(jid));
    if(NULL_PTR == to_save)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_find_module_by_jid: cxmppc2s %ld, dup jid '%s' failed\n", 
                            cxmppc2s_md_id, 
                            (char *)cstring_get_str(jid));    
        return (ERR_MODULE_ID);
    }   

    field_num = c_str_split(to_save, (const char *)"@/", (char **)fields, sizeof(fields)/sizeof(fields[0]));        
    ASSERT(1 == field_num);

    cxmppc2s_md_id = cxmppc2s_find_module((uint8_t *)fields[ 0 ]);
    safe_free(to_save, LOC_CXMPPC2S_0001);
    
    return (cxmppc2s_md_id);
}

/**
*
* start CXMPPC2S module
*
**/
UINT32 cxmppc2s_start()
{
    CXMPPC2S_MD *cxmppc2s_md;
    UINT32   cxmppc2s_md_id;

    TASK_BRD *task_brd;
    task_brd = task_brd_default_get();
    
    cxmppc2s_md_id = cbc_md_new(MD_CXMPPC2S, sizeof(CXMPPC2S_MD));
    if(ERR_MODULE_ID == cxmppc2s_md_id)
    {
        return (ERR_MODULE_ID);
    }
    
    /* initilize new one CXMPPC2S module */
    cxmppc2s_md = (CXMPPC2S_MD *)cbc_md_get(MD_CXMPPC2S, cxmppc2s_md_id);
    cxmppc2s_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();   

    cstring_init(CXMPPC2S_MD_USRNAME(cxmppc2s_md), NULL_PTR);
    clist_init(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), MM_CXMPPC2S_CONN, LOC_CXMPPC2S_0001);
    CXMPPC2S_MD_INIT_LOCK(cxmppc2s_md, LOC_CXMPPC2S_0001);

    cxmppc2s_md->usedcounter = 1;
    csig_atexit_register((CSIG_ATEXIT_HANDLER)cxmppc2s_end, cxmppc2s_md_id);
#if 1
    if(SWITCH_ON == CXMPPC2S_SWITCH && CMPI_FWD_RANK == CMPI_LOCAL_RANK)
    {
        if(EC_TRUE == task_brd_default_check_csrv_enabled() && NULL_PTR == TASK_BRD_CSRV(task_brd))
        {
            task_brd_default_start_cxmpp_srv(cxmppc2s_md_id, task_brd_default_get_srv_ipaddr(), task_brd_default_get_csrv_port());
        }
    }    
#endif
    dbg_log(SEC_0031_CRFS, 5)(LOGSTDOUT, "cxmppc2s_start: start CXMPPC2S module #%u\n", cxmppc2s_md_id);

    return ( cxmppc2s_md_id );
}

/**
*
* end CXMPPC2S module
*
**/
void cxmppc2s_end(const UINT32 cxmppc2s_md_id)
{
    CXMPPC2S_MD *cxmppc2s_md;

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);
    if(NULL_PTR == cxmppc2s_md)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_end: cxmppc2s_md_id = %u not exist.\n", cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
    
    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < cxmppc2s_md->usedcounter )
    {
        cxmppc2s_md->usedcounter --;
        return ;
    }

    if ( 0 == cxmppc2s_md->usedcounter )
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_end: cxmppc2s_md_id = %u is not started.\n", cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }

    /* free module : */
    //cxmppc2s_free_module_static_mem(cxmppc2s_md_id);

    cxmppc2s_md->usedcounter = 0;
    cstring_clean(CXMPPC2S_MD_USRNAME(cxmppc2s_md));
    clist_clean_with_modi(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), cxmppc2s_md_id, (CLIST_DATA_CLEAN)cxmppc2s_conn_free);
    CXMPPC2S_MD_CLEAN_LOCK(cxmppc2s_md, LOC_CXMPPC2S_0002);

    dbg_log(SEC_0146_CXMPPC2S, 5)(LOGSTDOUT, "cxmppc2s_end: stop CXMPPC2S module #%u\n", cxmppc2s_md_id);
    cbc_md_free(MD_CXMPPC2S, cxmppc2s_md_id);

    return ;
}


CXMPPC2S_CONN *cxmppc2s_conn_new(const UINT32 cxmppc2s_md_id)
{
    CXMPPC2S_CONN *cxmppc2s_conn;

    alloc_static_mem(MD_CXMPPC2S, cxmppc2s_md_id, MM_CXMPPC2S_CONN, &cxmppc2s_conn, LOC_CXMPPC2S_0002);
    if(NULL_PTR == cxmppc2s_conn)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_conn_new: cxmppc2s %ld, new cxmppc2s_conn failed\n", cxmppc2s_md_id);
        return (NULL_PTR);
    }

    cxmppc2s_conn_init(cxmppc2s_md_id, cxmppc2s_conn);
    return (cxmppc2s_conn);
}

EC_BOOL cxmppc2s_conn_init(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn)
{
    CXMPPC2S_CONN_MD_ID(cxmppc2s_conn) = cxmppc2s_md_id;
    
    cstring_init(CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn), NULL_PTR);
    cstring_init(CXMPPC2S_CONN_RESOURCE(cxmppc2s_conn), NULL_PTR);

    CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn) = NULL_PTR;
    return (EC_TRUE);
}

EC_BOOL cxmppc2s_conn_clean(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn)
{
    CXMPPC2S_CONN_MD_ID(cxmppc2s_conn) = ERR_MODULE_ID;
    
    cstring_clean(CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn));
    cstring_clean(CXMPPC2S_CONN_RESOURCE(cxmppc2s_conn));

    CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn) = NULL_PTR;

#if 0
    if(NULL_PTR != CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn))/*xxx*/
    {
        cxmpp_csocket_cnode_epoll_close(CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn));
        CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn) = NULL_PTR;
    }
#endif    
    return (EC_TRUE);
}

EC_BOOL cxmppc2s_conn_free(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn)
{
    if(NULL_PTR != cxmppc2s_conn)
    {
        cxmppc2s_conn_clean(cxmppc2s_md_id, cxmppc2s_conn);
        free_static_mem(MD_CXMPPC2S, cxmppc2s_md_id, MM_CXMPPC2S_CONN, cxmppc2s_conn, LOC_CXMPPC2S_0002);
    }
    return (EC_TRUE);
}

CSTRING *cxmppc2s_conn_make_jid(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    CSTRING         *jid;
    
    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    jid = cstring_make("%s@%s/%s", 
                       CXMPPC2S_MD_USRNAME_STR(cxmppc2s_md),
                       CXMPPC2S_CONN_DOMAIN_STR(cxmppc2s_conn),
                       CXMPPC2S_CONN_RESOURCE_STR(cxmppc2s_conn));
    if(NULL_PTR == jid)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_conn_make_jid: cxmppc2s %ld, make jid '%s@%s/%s' failed\n", 
                            cxmppc2s_md_id,
                            CXMPPC2S_MD_USRNAME_STR(cxmppc2s_md),
                            CXMPPC2S_CONN_DOMAIN_STR(cxmppc2s_conn),
                            CXMPPC2S_CONN_RESOURCE_STR(cxmppc2s_conn));
        return (NULL_PTR);
    }

    dbg_log(SEC_0146_CXMPPC2S, 9)(LOGSTDOUT, "[DEBUG] cxmppc2s_conn_make_jid: cxmppc2s %ld, make jid (%s, %s, %s) to '%s'\n", 
                        cxmppc2s_md_id,
                        CXMPPC2S_MD_USRNAME_STR(cxmppc2s_md),
                        CXMPPC2S_CONN_DOMAIN_STR(cxmppc2s_conn),
                        CXMPPC2S_CONN_RESOURCE_STR(cxmppc2s_conn),
                        (char *)cstring_get_str(jid));    

    return (jid);
}

CSTRING *cxmppc2s_conn_make_jid_no_resource(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    CSTRING         *jid_no_resource;
    
    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    jid_no_resource = cstring_make("%s@%s", 
                       CXMPPC2S_MD_USRNAME_STR(cxmppc2s_md),
                       CXMPPC2S_CONN_DOMAIN_STR(cxmppc2s_conn));
    if(NULL_PTR == jid_no_resource)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_conn_make_jid_no_resource_without_resource: cxmppc2s %ld, make jid_no_resource_no_resource '%s@%s' failed\n", 
                            cxmppc2s_md_id,
                            CXMPPC2S_MD_USRNAME_STR(cxmppc2s_md),
                            CXMPPC2S_CONN_DOMAIN_STR(cxmppc2s_conn));
        return (NULL_PTR);
    }

    return (jid_no_resource);
}

EC_BOOL cxmppc2s_set_usrname(const UINT32 cxmppc2s_md_id, const CSTRING *usrname)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_set_usrname: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    cstring_clone(usrname, CXMPPC2S_MD_USRNAME(cxmppc2s_md));

    return (EC_TRUE);
}

/*internal interface*/
CSTRING *cxmppc2s_get_usrname(const UINT32 cxmppc2s_md_id)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_get_usrname: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    return CXMPPC2S_MD_USRNAME(cxmppc2s_md);
}

CXMPPC2S_CONN *cxmppc2s_add_conn(const UINT32 cxmppc2s_md_id, const CSTRING *domain, const CSTRING *resource, const CSOCKET_CNODE *csocket_cnode)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    CXMPPC2S_CONN   *cxmppc2s_conn;
    
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_add_conn: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    cxmppc2s_conn = cxmppc2s_conn_new(cxmppc2s_md_id);
    if(NULL_PTR == cxmppc2s_conn)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_add_conn: cxmppc2s %ld, new cxmppc2s_conn failed\n", cxmppc2s_md_id);
        return (NULL_PTR);
    } 

    cstring_clone(domain  , CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn));
    cstring_clone(resource, CXMPPC2S_CONN_RESOURCE(cxmppc2s_conn));
    CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn) = (CSOCKET_CNODE *)csocket_cnode;

    clist_push_back(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), (void *)cxmppc2s_conn);

    return (cxmppc2s_conn);
}

CXMPPC2S_CONN *cxmppc2s_del_conn(const UINT32 cxmppc2s_md_id, const CSTRING *domain, const CSTRING *resource)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    CLIST_DATA      *clist_data;
    
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_del_conn: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    CLIST_LOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);
    CLIST_LOOP_NEXT(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), clist_data)
    {
        CXMPPC2S_CONN   *cxmppc2s_conn;

        cxmppc2s_conn = (CXMPPC2S_CONN *)CLIST_DATA_DATA(clist_data);
        if(EC_TRUE == cstring_cmp(CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn), domain)
        && EC_TRUE == cstring_cmp(CXMPPC2S_CONN_RESOURCE(cxmppc2s_conn), resource))
        {
            /*matched*/
            clist_rmv_no_lock(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), clist_data);
            //cxmppc2s_conn_free(cxmppc2s_md_id, cxmppc2s_conn);

            CLIST_UNLOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);
            return (cxmppc2s_conn);
        }
    }
    CLIST_UNLOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);

    dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_del_conn: cxmppc2s %ld, not found conn %s@%s/%s\n", 
                        cxmppc2s_md_id, 
                        CXMPPC2S_MD_USRNAME_STR(cxmppc2s_md),
                        (char *)cstring_get_str(domain),
                        (char *)cstring_get_str(resource));
    return (NULL_PTR);
}

CXMPPC2S_CONN *cxmppc2s_rm_conn(const UINT32 cxmppc2s_md_id, CXMPPC2S_CONN *cxmppc2s_conn)
{   
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_rm_conn: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    return cxmppc2s_del_conn(cxmppc2s_md_id, CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn), CXMPPC2S_CONN_RESOURCE(cxmppc2s_conn));
}

CXMPPC2S_CONN *cxmppc2s_find_conn(const UINT32 cxmppc2s_md_id, const CSTRING *domain, const CSTRING *resource)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    CLIST_DATA      *clist_data;
    
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_find_conn: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    CLIST_LOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);
    CLIST_LOOP_NEXT(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), clist_data)
    {
        CXMPPC2S_CONN   *cxmppc2s_conn;

        cxmppc2s_conn = (CXMPPC2S_CONN *)CLIST_DATA_DATA(clist_data);
        if(EC_TRUE == cstring_cmp(CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn), domain)
        && EC_TRUE == cstring_cmp(CXMPPC2S_CONN_RESOURCE(cxmppc2s_conn), resource))
        {
            /*matched*/
            CLIST_UNLOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);
            return (cxmppc2s_conn);
        }
    }
    CLIST_UNLOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);

    dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_find_conn: cxmppc2s %ld, not found conn %s@%s/%s\n", 
                        cxmppc2s_md_id, 
                        CXMPPC2S_MD_USRNAME_STR(cxmppc2s_md),
                        (char *)cstring_get_str(domain),
                        (char *)cstring_get_str(resource));
    return (NULL_PTR);
}

EC_BOOL cxmppc2s_is_me(const UINT32 cxmppc2s_md_id, const CSTRING *to, CXMPPC2S_CONN **cxmppc2s_conn)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    char            *to_save;
    char            *fields[4];
    UINT32           field_num;
        
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_is_me: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    (*cxmppc2s_conn) = NULL_PTR;
    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    to_save = c_str_dup((const char *)cstring_get_str(to));
    if(NULL_PTR == to_save)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_is_me: cxmppc2s %ld, dup '%s' failed\n", 
                            cxmppc2s_md_id, 
                            (char *)cstring_get_str(to));    
        return (EC_FALSE);
    }

    field_num = c_str_split(to_save, (const char *)"@/", (char **)fields, sizeof(fields)/sizeof(fields[0]));    
    if(3 == field_num) /*user@domain/resource*/
    {
        CLIST_DATA *clist_data;

        if(EC_FALSE == cstring_is_str_ignore_case(CXMPPC2S_MD_USRNAME(cxmppc2s_md), (const uint8_t *)fields[0]))
        {
            safe_free(to_save, LOC_CXMPPC2S_0001);
            return (EC_FALSE);
        }         

        CLIST_LOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);
        CLIST_LOOP_NEXT(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), clist_data)
        {
            CXMPPC2S_CONN   *cxmppc2s_conn_t;

            cxmppc2s_conn_t = (CXMPPC2S_CONN *)CLIST_DATA_DATA(clist_data);
            if(EC_TRUE == cstring_is_str_ignore_case(CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn_t), (const uint8_t *)fields[1])
            && EC_TRUE == cstring_is_str_ignore_case(CXMPPC2S_CONN_RESOURCE(cxmppc2s_conn_t), (const uint8_t *)fields[2]))
            {
                /*matched*/
                (*cxmppc2s_conn) = cxmppc2s_conn_t;
                CLIST_UNLOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);

                safe_free(to_save, LOC_CXMPPC2S_0001);
                return (EC_TRUE);
            }
        }
        CLIST_UNLOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);        

        safe_free(to_save, LOC_CXMPPC2S_0001);
        return (EC_FALSE);
    }

    /*TODO: should to do more!*/
    if(2 == field_num) /*user@domain*/
    {
        CLIST_DATA *clist_data;

        if(EC_FALSE == cstring_is_str_ignore_case(CXMPPC2S_MD_USRNAME(cxmppc2s_md), (const uint8_t *)fields[0]))
        {
            safe_free(to_save, LOC_CXMPPC2S_0001);
            return (EC_FALSE);
        } 
    
        CLIST_LOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);
        CLIST_LOOP_NEXT(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), clist_data)
        {
            CXMPPC2S_CONN   *cxmppc2s_conn_t;

            cxmppc2s_conn_t = (CXMPPC2S_CONN *)CLIST_DATA_DATA(clist_data);
            if(EC_TRUE == cstring_is_str_ignore_case(CXMPPC2S_CONN_DOMAIN(cxmppc2s_conn_t), (const uint8_t *)fields[1]))
            {
                /*matched*/
                (*cxmppc2s_conn) = cxmppc2s_conn_t;
                CLIST_UNLOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);

                safe_free(to_save, LOC_CXMPPC2S_0001);
                return (EC_TRUE);
            }
        }
        CLIST_UNLOCK(CXMPPC2S_MD_CONN_LIST(cxmppc2s_md), LOC_CXMPPC2S_0002);        

        safe_free(to_save, LOC_CXMPPC2S_0001);
        return (EC_FALSE);
    }

    dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_is_me: cxmppc2s %ld, split '%s' into invalid %ld fields\n", 
                        cxmppc2s_md_id, 
                        to_save, 
                        field_num);  
    safe_free(to_save, LOC_CXMPPC2S_0001);
    return (EC_FALSE);
}

EC_BOOL cxmppc2s_get_mod_node(const UINT32 cxmppc2s_md_id, const CSTRING *to, MOD_NODE *mod_node)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    UINT32           cxmppc2s_md_id_t;
    
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_get_mod_node: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    /*TODO: query cached database*/
    cxmppc2s_md_id_t = cxmppc2s_find_module_by_jid(to);
    if(ERR_MODULE_ID == cxmppc2s_md_id_t)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_get_mod_node: cxmppc2s %ld, no module for '%s'\n", 
                    cxmppc2s_md_id, (char *)cstring_get_str(to));  
        return (EC_FALSE);
    }

    /*debug, for test only!*/
    MOD_NODE_TCID(mod_node) = CMPI_LOCAL_TCID;
    MOD_NODE_COMM(mod_node) = CMPI_LOCAL_COMM;
    MOD_NODE_RANK(mod_node) = CMPI_LOCAL_RANK;
    MOD_NODE_MODI(mod_node) = cxmppc2s_md_id_t;

    return (EC_TRUE);
}

EC_BOOL cxmppc2s_presence_subscribe(const UINT32 cxmppc2s_md_id, const CSTRING *to, const CBUFFER *send_subscribe_buffer)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    MOD_NODE         recv_mod_node;
    CXMPPC2S_CONN   *cxmppc2s_conn;
    EC_BOOL          ret;
    
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_presence_subscribe: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    /*if 'to' is local*/
    cxmppc2s_conn = NULL_PTR;
    if(EC_TRUE == cxmppc2s_is_me(cxmppc2s_md_id, to, &cxmppc2s_conn))
    {
        CSOCKET_CNODE *csocket_cnode;
        CXMPP_NODE    *cxmpp_node;
        
        csocket_cnode = CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn);
        cxmpp_node    = CSOCKET_CNODE_CXMPP_NODE(csocket_cnode);

        cbuffer_append(CXMPP_NODE_SEND_CBUFFER(cxmpp_node), CBUFFER_DATA(send_subscribe_buffer), CBUFFER_USED(send_subscribe_buffer));

        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)cxmpp_send_on_csocket_cnode_thread,(void *)csocket_cnode);          
        return (EC_TRUE);
    }

    /*get to mod_node*/
    if(EC_FALSE == cxmppc2s_get_mod_node(cxmppc2s_md_id, to, &recv_mod_node))
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_presence_subscribe: cxmppc2s %ld, get mod_node of '%s' failed\n", 
                            cxmppc2s_md_id, 
                            (char *)cstring_get_str(to));    
        
        return (EC_FALSE);
    }    

    //ASSERT(EC_FALSE == mod_node_is_local(&recv_mod_node));

    ret = EC_FALSE;
    task_p2p(cxmppc2s_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP,
             &recv_mod_node, 
             &ret, FI_cxmppc2s_presence_subscribe, ERR_MODULE_ID, to, send_subscribe_buffer);

    if(EC_FALSE == ret)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_presence_subscribe: cxmppc2s %ld, send '%s' subscribe to (tcid %s, comm %ld, rank %ld, modi %ld) failed\n", 
                            cxmppc2s_md_id, 
                            (char *)cstring_get_str(to),
                            MOD_NODE_TCID_STR(&recv_mod_node),
                            MOD_NODE_COMM(&recv_mod_node),
                            MOD_NODE_RANK(&recv_mod_node),
                            MOD_NODE_MODI(&recv_mod_node)
                            );    
        
        return (EC_FALSE);
    }

    dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "[DEBUG] cxmppc2s_presence_subscribe: cxmppc2s %ld, send '%s' subscribe to (tcid %s, comm %ld, rank %ld, modi %ld) done\n", 
                        cxmppc2s_md_id, 
                        (char *)cstring_get_str(to),
                        MOD_NODE_TCID_STR(&recv_mod_node),
                        MOD_NODE_COMM(&recv_mod_node),
                        MOD_NODE_RANK(&recv_mod_node),
                        MOD_NODE_MODI(&recv_mod_node)
                        );     

    return (EC_TRUE);
}

EC_BOOL cxmppc2s_presence_subscribed(const UINT32 cxmppc2s_md_id, const CSTRING *to, const CBUFFER *send_subscribed_buffer)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    MOD_NODE         recv_mod_node;
    CXMPPC2S_CONN   *cxmppc2s_conn;
    EC_BOOL          ret;
    
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_presence_subscribed: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    dbg_log(SEC_0146_CXMPPC2S, 9)(LOGSTDOUT, "[DEBUG] cxmppc2s_presence_subscribed: cxmppc2s %ld, usrname '%s', to is '%s'\n", 
                        cxmppc2s_md_id, 
                        CXMPPC2S_MD_USRNAME_STR(cxmppc2s_md),
                        (char *)cstring_get_str(to));   

    /*if 'to' is local*/
    cxmppc2s_conn = NULL_PTR;
    if(EC_TRUE == cxmppc2s_is_me(cxmppc2s_md_id, to, &cxmppc2s_conn))
    {
        CSOCKET_CNODE *csocket_cnode;
        CXMPP_NODE    *cxmpp_node;
        
        csocket_cnode = CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn);
        cxmpp_node    = CSOCKET_CNODE_CXMPP_NODE(csocket_cnode);

        cbuffer_append(CXMPP_NODE_SEND_CBUFFER(cxmpp_node), CBUFFER_DATA(send_subscribed_buffer), CBUFFER_USED(send_subscribed_buffer));

        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)cxmpp_send_on_csocket_cnode_thread,(void *)csocket_cnode);          
        return (EC_TRUE);
    }

    /*get to mod_node*/
    if(EC_FALSE == cxmppc2s_get_mod_node(cxmppc2s_md_id, to, &recv_mod_node))
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_presence_subscribed: cxmppc2s %ld, get mod_node of '%s' failed\n", 
                            cxmppc2s_md_id, 
                            (char *)cstring_get_str(to));    
        
        return (EC_FALSE);
    }    

    //ASSERT(EC_FALSE == mod_node_is_local(&recv_mod_node));

    ret = EC_FALSE;
    task_p2p(cxmppc2s_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP,
             &recv_mod_node, 
             &ret, FI_cxmppc2s_presence_subscribed, ERR_MODULE_ID, to, send_subscribed_buffer);

    if(EC_FALSE == ret)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_presence_subscribed: cxmppc2s %ld, send '%s' subscribed to (tcid %s, comm %ld, rank %ld, modi %ld) failed\n", 
                            cxmppc2s_md_id, 
                            (char *)cstring_get_str(to),
                            MOD_NODE_TCID_STR(&recv_mod_node),
                            MOD_NODE_COMM(&recv_mod_node),
                            MOD_NODE_RANK(&recv_mod_node),
                            MOD_NODE_MODI(&recv_mod_node)
                            );    
        
        return (EC_FALSE);
    }

    dbg_log(SEC_0146_CXMPPC2S, 9)(LOGSTDOUT, "[DEBUG] cxmppc2s_presence_subscribed: cxmppc2s %ld, send '%s' subscribed to (tcid %s, comm %ld, rank %ld, modi %ld) done\n", 
                        cxmppc2s_md_id, 
                        (char *)cstring_get_str(to),
                        MOD_NODE_TCID_STR(&recv_mod_node),
                        MOD_NODE_COMM(&recv_mod_node),
                        MOD_NODE_RANK(&recv_mod_node),
                        MOD_NODE_MODI(&recv_mod_node)
                        );      

    return (EC_TRUE);
}

EC_BOOL cxmppc2s_send_message(const UINT32 cxmppc2s_md_id, const CSTRING *to, const CBUFFER *send_message_buffer)
{
    CXMPPC2S_MD     *cxmppc2s_md;
    MOD_NODE         recv_mod_node;
    CXMPPC2S_CONN   *cxmppc2s_conn;
    EC_BOOL          ret;
    
#if ( SWITCH_ON == CXMPPC2S_DEBUG_SWITCH )
    if ( CXMPPC2S_MD_ID_CHECK_INVALID(cxmppc2s_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cxmppc2s_send_message: cxmppc2s module #0x%lx not started.\n",
                cxmppc2s_md_id);
        dbg_exit(MD_CXMPPC2S, cxmppc2s_md_id);
    }
#endif/*CRFSC_DEBUG_SWITCH*/

    cxmppc2s_md = CXMPPC2S_MD_GET(cxmppc2s_md_id);

    /*if 'to' is local*/
    cxmppc2s_conn = NULL_PTR;
    if(EC_TRUE == cxmppc2s_is_me(cxmppc2s_md_id, to, &cxmppc2s_conn))
    {
        CSOCKET_CNODE *csocket_cnode;
        CXMPP_NODE    *cxmpp_node;
        
        csocket_cnode = CXMPPC2S_CONN_CSOCKET_CNODE(cxmppc2s_conn);
        cxmpp_node    = CSOCKET_CNODE_CXMPP_NODE(csocket_cnode);

        cbuffer_append(CXMPP_NODE_SEND_CBUFFER(cxmpp_node), CBUFFER_DATA(send_message_buffer), CBUFFER_USED(send_message_buffer));

        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)cxmpp_send_on_csocket_cnode_thread,(void *)csocket_cnode);          
        return (EC_TRUE);
    }

    /*get to mod_node*/
    if(EC_FALSE == cxmppc2s_get_mod_node(cxmppc2s_md_id, to, &recv_mod_node))
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_send_message: cxmppc2s %ld, get mod_node of '%s' failed\n", 
                            cxmppc2s_md_id, 
                            (char *)cstring_get_str(to));    
        
        return (EC_FALSE);
    }

    //ASSERT(EC_FALSE == mod_node_is_local(&recv_mod_node));

    ret = EC_FALSE;
    task_p2p(cxmppc2s_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP,
             &recv_mod_node, 
             &ret, FI_cxmppc2s_send_message, ERR_MODULE_ID, to, send_message_buffer);

    if(EC_FALSE == ret)
    {
        dbg_log(SEC_0146_CXMPPC2S, 0)(LOGSTDOUT, "error:cxmppc2s_send_message: cxmppc2s %ld, send '%s' message to (tcid %s, comm %ld, rank %ld, modi %ld) failed\n", 
                            cxmppc2s_md_id, 
                            (char *)cstring_get_str(to),
                            MOD_NODE_TCID_STR(&recv_mod_node),
                            MOD_NODE_COMM(&recv_mod_node),
                            MOD_NODE_RANK(&recv_mod_node),
                            MOD_NODE_MODI(&recv_mod_node)
                            );    
        
        return (EC_FALSE);
    }

    dbg_log(SEC_0146_CXMPPC2S, 9)(LOGSTDOUT, "[DEBUG] cxmppc2s_send_message: cxmppc2s %ld, send '%s' message to (tcid %s, comm %ld, rank %ld, modi %ld) done\n", 
                        cxmppc2s_md_id, 
                        (char *)cstring_get_str(to),
                        MOD_NODE_TCID_STR(&recv_mod_node),
                        MOD_NODE_COMM(&recv_mod_node),
                        MOD_NODE_RANK(&recv_mod_node),
                        MOD_NODE_MODI(&recv_mod_node)
                        );       

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
    
