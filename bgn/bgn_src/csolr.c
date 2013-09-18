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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>

#include <sys/stat.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "carray.h"
#include "cvector.h"

#include "cbc.h"

#include "char2int.h"

#include "task.h"

#include "csocket.h"

#include "cmpie.h"

#include "csolr.h"

#include "findex.inc"

#define CSOLR_MD_CAPACITY()                  (cbc_md_capacity(MD_CSOLR))

#define CSOLR_MD_GET(csolr_md_id)     ((CSOLR_MD *)cbc_md_get(MD_CSOLR, (csolr_md_id)))

#define CSOLR_MD_ID_CHECK_INVALID(csolr_md_id)  \
    ((CMPI_ANY_MODI != (csolr_md_id)) && ((NULL_PTR == CSOLR_MD_GET(csolr_md_id)) || (0 == (CSOLR_MD_GET(csolr_md_id)->usedcounter))))

#if 1
#define PRINT_BUFF(info, buff, len) do{\
    UINT32 __pos__;\
    sys_log(LOGSTDOUT, "%s: ", info);\
    for(__pos__ = 0; __pos__ < len; __pos__ ++)\
    {\
        sys_print(LOGSTDOUT, "%x,", ((UINT8 *)buff)[ __pos__ ]);\
    }\
    sys_print(LOGSTDOUT, "\n");\
}while(0)
#else
#define PRINT_BUFF(info, buff, len) do{}while(0)
#endif



/**
*   for test only
*
*   to query the status of CSOLR Module
*
**/
void csolr_print_module_status(const UINT32 csolr_md_id, LOG *log)
{
    CSOLR_MD *csolr_md;
    UINT32 this_csolr_md_id;

    for( this_csolr_md_id = 0; this_csolr_md_id < CSOLR_MD_CAPACITY(); this_csolr_md_id ++ )
    {
        csolr_md = CSOLR_MD_GET(this_csolr_md_id);

        if ( NULL_PTR != csolr_md && 0 < csolr_md->usedcounter )
        {
            sys_log(log,"CSOLR Module # %ld : %ld refered\n",
                    this_csolr_md_id,
                    csolr_md->usedcounter);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed CSOLR module
*
*
**/
UINT32 csolr_free_module_static_mem(const UINT32 csolr_md_id)
{
    CSOLR_MD  *csolr_md;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_free_module_static_mem: csolr module #0x%lx not started.\n",
                csolr_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);

    free_module_static_mem(MD_CSOLR, csolr_md_id);

    return 0;
}

/**
*
* start CSOLR module
*
**/
UINT32 csolr_start()
{
    CSOLR_MD *csolr_md;
    UINT32 csolr_md_id;

    TASK_BRD *task_brd;

    csolr_md_id = cbc_md_new(MD_CSOLR, sizeof(CSOLR_MD));
    if(ERR_MODULE_ID == csolr_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one CSOLR module */
    csolr_md = (CSOLR_MD *)cbc_md_get(MD_CSOLR, csolr_md_id);
    csolr_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();

    task_brd = task_brd_default_get();

    CSOLR_MD_MOD_MGR(csolr_md)  = mod_mgr_new(csolr_md_id, /*LOAD_BALANCING_LOOP*//*LOAD_BALANCING_MOD*/LOAD_BALANCING_QUE);

    csolr_md->usedcounter = 1;

    CSOLR_MD_WAITING_RSP(csolr_md) = NULL_PTR;

    CSOLR_MD_RSP_CCOND_INIT(csolr_md, LOC_CSOLR_0001);

    sys_log(LOGSTDOUT, "csolr_start: start CSOLR module #%ld\n", csolr_md_id);
    //sys_log(LOGSTDOUT, "========================= csolr_start: CSOLR table info:\n");
    //csolr_print_module_status(csolr_md_id, LOGSTDOUT);
    //cbc_print();

    return ( csolr_md_id );
}

/**
*
* end CSOLR module
*
**/
void csolr_end(const UINT32 csolr_md_id)
{
    CSOLR_MD *csolr_md;

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    if(NULL_PTR == csolr_md)
    {
        sys_log(LOGSTDOUT,"error:csolr_end: csolr_md_id = %ld not exist.\n", csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < csolr_md->usedcounter )
    {
        csolr_md->usedcounter --;
        return ;
    }

    if ( 0 == csolr_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:csolr_end: csolr_md_id = %ld is not started.\n", csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }

    CSOLR_MD_WAITING_RSP(csolr_md) = NULL_PTR;
    CSOLR_MD_RSP_CCOND_CLEAN(csolr_md, LOC_CSOLR_0002);

    /* if nobody else occupied the module,then free its resource */
    if(NULL_PTR != CSOLR_MD_MOD_MGR(csolr_md))
    {
        mod_mgr_free(CSOLR_MD_MOD_MGR(csolr_md));
        CSOLR_MD_MOD_MGR(csolr_md)  = NULL_PTR;
    }

    /* free module : */
    //csolr_free_module_static_mem(csolr_md_id);

    csolr_md->usedcounter = 0;

    sys_log(LOGSTDOUT, "csolr_end: stop CSOLR module #%ld\n", csolr_md_id);
    cbc_md_free(MD_CSOLR, csolr_md_id);

    breathing_static_mem();

    //sys_log(LOGSTDOUT, "========================= csolr_end: CSOLR table info:\n");
    //csolr_print_module_status(csolr_md_id, LOGSTDOUT);
    //cbc_print();

    return ;
}

/**
*
* initialize mod mgr of CSOLR module
*
**/
UINT32 csolr_set_mod_mgr(const UINT32 csolr_md_id, const MOD_MGR * src_mod_mgr)
{
    CSOLR_MD *csolr_md;
    MOD_MGR  *des_mod_mgr;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_set_mod_mgr: csolr module #0x%lx not started.\n",
                csolr_md_id);
        csolr_print_module_status(csolr_md_id, LOGSTDOUT);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    des_mod_mgr = CSOLR_MD_MOD_MGR(csolr_md);

    sys_log(LOGSTDOUT, "csolr_set_mod_mgr: md_id %d, input src_mod_mgr %lx\n", csolr_md_id, src_mod_mgr);
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    /*figure out mod_nodes with tcid belong to set of csolrnp_tcid_vec and csolrnp_tcid_vec*/
    mod_mgr_limited_clone(csolr_md_id, src_mod_mgr, des_mod_mgr);

    sys_log(LOGSTDOUT, "====================================csolr_set_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    sys_log(LOGSTDOUT, "====================================csolr_set_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);

    return (0);
}

/**
*
* get mod mgr of CSOLR module
*
**/
MOD_MGR * csolr_get_mod_mgr(const UINT32 csolr_md_id)
{
    CSOLR_MD *csolr_md;

    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        return (MOD_MGR *)0;
    }

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    return (csolr_md->mod_mgr);
}

EC_BOOL csolr_add_mod(const UINT32 csolr_md_id, const UINT32 csolr_tcid)
{
    CSOLR_MD   *csolr_md;

    TASK_BRD *task_brd;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_add_mod: csolr module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);

    task_brd = task_brd_default_get();

    mod_mgr_incl(csolr_tcid, CMPI_ANY_COMM, 1, 0, CSOLR_MD_MOD_MGR(csolr_md));

    return (EC_TRUE);
}

EC_BOOL csolr_add_csocket_cnode(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode)
{
    CSOLR_MD *csolr_md;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_add_csocket_cnode: csolr module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    CSOLR_MD_CSOCKET_CNODE(csolr_md) = csocket_cnode;
    return (EC_TRUE);
}

EC_BOOL csolr_del_csocket_cnode(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode)
{
    CSOLR_MD *csolr_md;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_del_csocket_cnode: csolr module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    if(csocket_cnode != CSOLR_MD_CSOCKET_CNODE(csolr_md))
    {
        sys_log(LOGSTDOUT, "error:csolr_del_csocket_cnode: mismatched csocket_cnode\n");
        return (EC_FALSE);
    }
    CSOLR_MD_CSOCKET_CNODE(csolr_md) = csocket_cnode;
    return (EC_TRUE);
}

EC_BOOL csolr_send_req(const UINT32 csolr_md_id, const CSTRING *solr_req)
{
    CSOLR_MD *csolr_md;
    CSOCKET_CNODE *csocket_cnode;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_send_req: csolr module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    csocket_cnode =  CSOLR_MD_CSOCKET_CNODE(csolr_md);
    if(NULL_PTR == csocket_cnode)
    {
        sys_log(LOGSTDOUT, "error:csolr_send_req: no solr connected\n");
        return (EC_FALSE);
    }

    /*send solr_req to solr via csocket_cnode and wait for solr_rsp*/
    if(EC_FALSE == csocket_send_cstring(CSOCKET_CNODE_SOCKFD(csocket_cnode), solr_req))
    {
        sys_log(LOGSTDOUT, "error:csolr_send_req: send solr_req on sockfd %d failed\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL csolr_recv_req(const UINT32 csolr_md_id, CSTRING *solr_req)
{
    CSOLR_MD *csolr_md;
    CSOCKET_CNODE *csocket_cnode;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_recv_req: csolr module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    csocket_cnode =  CSOLR_MD_CSOCKET_CNODE(csolr_md);
    if(NULL_PTR == csocket_cnode)
    {
        sys_log(LOGSTDOUT, "error:csolr_recv_req: no solr connected\n");
        return (EC_FALSE);
    }

    /*send solr_req to solr via csocket_cnode and wait for solr_rsp*/
    if(EC_FALSE == csocket_recv_cstring(CSOCKET_CNODE_SOCKFD(csocket_cnode), solr_req))
    {
        sys_log(LOGSTDOUT, "error:csolr_recv_req: recv solr_req on sockfd %d failed\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL csolr_send_rsp(const UINT32 csolr_md_id, const CSTRING *solr_rsp)
{
    CSOLR_MD *csolr_md;
    CSOCKET_CNODE *csocket_cnode;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_send_rsp module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    csocket_cnode =  CSOLR_MD_CSOCKET_CNODE(csolr_md);
    if(NULL_PTR == csocket_cnode)
    {
        sys_log(LOGSTDOUT, "error:csolr_send_rsp: no solr connected\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_send_cstring(CSOCKET_CNODE_SOCKFD(csocket_cnode), solr_rsp))
    {
        sys_log(LOGSTDOUT, "error:csolr_send_rsp: send solr_rsp on sockfd %d failed\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL csolr_recv_rsp(const UINT32 csolr_md_id, CSTRING *solr_rsp)
{
    CSOLR_MD *csolr_md;
    CSOCKET_CNODE *csocket_cnode;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_recv_rsp module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    csocket_cnode =  CSOLR_MD_CSOCKET_CNODE(csolr_md);
    if(NULL_PTR == csocket_cnode)
    {
        sys_log(LOGSTDOUT, "error:csolr_recv_rsp: no solr connected\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_recv_cstring(CSOCKET_CNODE_SOCKFD(csocket_cnode), solr_rsp))
    {
        sys_log(LOGSTDOUT, "error:csolr_recv_rsp: recv solr_rsp on sockfd %d failed\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL csolr_do_req(const UINT32 csolr_md_id, const CSTRING *solr_req, CSTRING *solr_rsp)
{
    CSOLR_MD *csolr_md;
#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_do_req: csolr module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);

    PRINT_BUFF("[DEBUG] csolr_do_req: solr_req: ", cstring_get_str(solr_req), cstring_get_len(solr_req));

    /*send solr_req to solr via csocket_cnode and wait for solr_rsp*/
    if(EC_FALSE == csolr_send_req(csolr_md_id, solr_req))
    {
        sys_log(LOGSTDOUT, "error:csolr_do_req: send solr_req failed\n");
        return (EC_FALSE);
    }

    CSOLR_MD_WAITING_RSP(csolr_md) = solr_rsp;
    CSOLR_MD_RSP_CCOND_RESERVE(csolr_md, LOC_CSOLR_0003);
    CSOLR_MD_RSP_CCOND_WAIT(csolr_md, LOC_CSOLR_0004);
    CSOLR_MD_WAITING_RSP(csolr_md) = NULL_PTR;/*clean*/
/*
    if(EC_FALSE == csolr_recv_rsp(csolr_md_id, solr_rsp))
    {
        sys_log(LOGSTDOUT, "error:csolr_do_req: recv solr_rsp\n");
        return (EC_FALSE);
    }
*/
    sys_log(LOGSTDOUT, "[DEBUG] csolr_do_req: solr_rsp: %.*s\n", cstring_get_len(solr_rsp), (char *)cstring_get_str(solr_rsp));

    return (EC_TRUE);
}

EC_BOOL csolr_srv_fwd_req(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode)
{
    CSOLR_MD *csolr_md;

    EC_BOOL  result;
    CSTRING *solr_req;
    CSTRING *solr_rsp;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_srv_fwd_req: csolr module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);

    /*recv solr_req_len and solr_req*/
    solr_req = cstring_new(NULL_PTR, LOC_CSOLR_0005);
    if(NULL_PTR == solr_req)
    {
        sys_log(LOGSTDOUT, "error:csolr_srv_fwd_req: new solr_req failed\n");
        return (EC_FALSE);
    }

    solr_rsp = cstring_new(NULL_PTR, LOC_CSOLR_0006);
    if(NULL_PTR == solr_rsp)
    {
        sys_log(LOGSTDOUT, "error:csolr_srv_fwd_req: new solr_rsp failed\n");
        cstring_free(solr_req);
        return (EC_FALSE);
    }

    if(EC_FALSE == csolr_recv_req(csolr_md_id, solr_req))
    {
        sys_log(LOGSTDOUT, "error:csolr_srv_fwd_req: recv solr_req failed\n");
        cstring_free(solr_req);
        cstring_free(solr_rsp);
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "[DEBUG] csolr_srv_fwd_req: recv solr_req: %.*s\n", cstring_get_len(solr_req), (char *)cstring_get_str(solr_req));
    PRINT_BUFF("[DEBUG] csolr_srv_fwd_req: recv solr_req", cstring_get_str(solr_req), cstring_get_len(solr_req));

    if(1)
    {
        MOD_MGR  *mod_mgr;
        TASK_MGR *task_mgr;

        mod_mgr = mod_mgr_new(csolr_md_id, LOAD_BALANCING_QUE);
        mod_mgr_limited_clone(csolr_md_id, CSOLR_MD_MOD_MGR(csolr_md), mod_mgr);
        mod_mgr_excl(CMPI_LOCAL_TCID, CMPI_ANY_COMM, CMPI_ANY_RANK, csolr_md_id, mod_mgr);

        result = EC_FALSE;

        task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        task_inc(task_mgr, &result, FI_csolr_do_req, ERR_MODULE_ID, solr_req, solr_rsp);
        task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

        mod_mgr_free(mod_mgr);
    }

    /*return solr rsp*/
    if(EC_FALSE == result)
    {
        sys_log(LOGSTDOUT, "error:csolr_srv_fwd_req: csolr_do_req failed\n");
        cstring_free(solr_req);
        cstring_free(solr_rsp);
        return (EC_FALSE);
    }

    PRINT_BUFF("[DEBUG] csolr_srv_fwd_req: send solr_rsp: ", cstring_get_str(solr_rsp), cstring_get_len(solr_rsp));

    if(EC_FALSE == csolr_send_rsp(csolr_md_id, solr_rsp))
    {
        sys_log(LOGSTDOUT, "error:csolr_srv_fwd_req: send solr_rsp failed\n");
        cstring_free(solr_req);
        cstring_free(solr_rsp);
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] csolr_srv_fwd_req: send solr_rsp: done\n");

    cstring_free(solr_req);
    cstring_free(solr_rsp);
    return (EC_TRUE);
}

EC_BOOL csolr_srv_hit_rsp(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode)
{
    CSOLR_MD *csolr_md;

    CSTRING *solr_rsp;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_srv_hit_rsp: csolr module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    csolr_md = CSOLR_MD_GET(csolr_md_id);
    solr_rsp = CSOLR_MD_WAITING_RSP(csolr_md);
    if(NULL_PTR == solr_rsp)
    {
        sys_log(LOGSTDOUT, "error:csolr_srv_hit_rsp: no one waiting for solr_rsp\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == csolr_recv_rsp(csolr_md_id, solr_rsp))
    {
        sys_log(LOGSTDOUT, "error:csolr_srv_hit_rsp: recv solr_req failed\n");
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] csolr_srv_hit_rsp: recv solr_rsp: %.*s\n", cstring_get_len(solr_rsp), (char *)cstring_get_str(solr_rsp));
    CSOLR_MD_RSP_CCOND_RELEASE(csolr_md, LOC_CSOLR_0007);

    return (EC_TRUE);
}

/**
*
*   handle incomed data once from external server
*
**/
EC_BOOL csolr_srv_handle_once(const UINT32 csolr_md_id, CSOCKET_CNODE *csocket_cnode)
{
    UINT8 op;

#if ( SWITCH_ON == CSOLR_DEBUG_SWITCH )
    if ( CSOLR_MD_ID_CHECK_INVALID(csolr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:csolr_srv_handle_once: csolr module #0x%lx not started.\n",
                csolr_md_id);
        dbg_exit(MD_CSOLR, csolr_md_id);
    }
#endif/*CSOLR_DEBUG_SWITCH*/

    if(EC_FALSE == csocket_recv_uint8(CSOCKET_CNODE_SOCKFD(csocket_cnode), &op))
    {
        sys_log(LOGSTDOUT, "error:csolr_srv_handle_once: recv op on sockfd %d failed\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] csolr_srv_handle_once: op = %d\n", op);

    switch(op)
    {
        case CSOLR_OP_REQ:
            if(EC_FALSE == csolr_srv_fwd_req(csolr_md_id, csocket_cnode))
            {
                sys_log(LOGSTDOUT, "error:csolr_srv_handle_once: write once failed\n");
                return (EC_FALSE);
            }
            break;

        case CSOLR_OP_RSP:
            if(EC_FALSE == csolr_srv_hit_rsp(csolr_md_id, csocket_cnode))
            {
                sys_log(LOGSTDOUT, "error:csolr_srv_handle_once: read once failed\n");
                return (EC_FALSE);
            }
            break;
        default:
            sys_log(LOGSTDOUT, "error:csolr_srv_handle_once: unknow op %d\n", op);
            return (EC_FALSE);
    }

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

