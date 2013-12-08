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
#include <malloc.h>
#include <errno.h>

#include <sys/stat.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "carray.h"
#include "cvector.h"

#include "cfile.h"

#include "cbc.h"

#include "cxml.h"
#include "cmisc.h"

#include "task.h"
#include "kbuff.h"

#include "cmpie.h"

#include "cdir.h"

#include "findex.inc"

#define CFILE_MD_CAPACITY()                  (cbc_md_capacity(MD_CFILE))

#define CFILE_MD_GET(cfile_md_id)     ((CFILE_MD *)cbc_md_get(MD_CFILE, (cfile_md_id)))

#define CFILE_MD_ID_CHECK_INVALID(cfile_md_id)  \
    ((CMPI_ANY_MODI != (cfile_md_id)) && ((NULL_PTR == CFILE_MD_GET(cfile_md_id)) || (0 == (CFILE_MD_GET(cfile_md_id)->usedcounter))))

#define CFILE_MD_LOCAL_TCID(cfile_md)  (MOD_MGR_LOCAL_MOD_TCID(CFILE_MD_MOD_MGR(cfile_md)))
#define CFILE_LOCAL_TCID(cfile_md_id)  (MOD_MGR_LOCAL_MOD_TCID(CFILE_MD_MOD_MGR(CFILE_MD_GET(cfile_md_id))))

#define CFILE_MD_LOCAL_TCID_STR(cfile_md)  (c_word_to_ipv4(CFILE_MD_LOCAL_TCID(cfile_md)))
#define CFILE_LOCAL_TCID_STR(cfile_md_id)  (c_word_to_ipv4(CFILE_LOCAL_TCID(cfile_md_id)))

static UINT32 cfile_seg_vec_read(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const UINT32 cfile_seg_pos_beg, const UINT32 cfile_seg_pos_end, CVECTOR *kbuff_vec, FILE *in_fp);
static UINT32 cfile_seg_vec_write(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const UINT32 cfile_seg_pos_beg, const UINT32 cfile_seg_pos_end, const CVECTOR *kbuff_vec, FILE *out_fp);

static UINT32 cfile_read_fwrite_group_on_node_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, FILE *in_fp, const UINT32 node_tcid);
static UINT32 cfile_fread_write_group_on_local_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, FILE *out_fp);

/*************************************************************************************************************************************\
example:test.log.xml
====================
<node name="/tmp/cfile_node/test.log.xml" tcid="2" size="35873124">
    <segments>
        <segment id="0" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000000.dat" cachesize="1048576"/>
        <segment id="1" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000001.dat" cachesize="1048576"/>
        <segment id="2" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000002.dat" cachesize="1048576"/>
        <segment id="3" size="1048576" tcid="2" name="/tmp/cfile_seg/test.log.xml.00000003.dat" cachesize="1048576"/>
        <segment id="4" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000004.dat" cachesize="1048576"/>
        <segment id="5" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000005.dat" cachesize="1048576"/>
        <segment id="6" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000006.dat" cachesize="1048576"/>
        <segment id="7" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000007.dat" cachesize="1048576"/>
        <segment id="8" size="1048576" tcid="2" name="/tmp/cfile_seg/test.log.xml.00000008.dat" cachesize="1048576"/>
        <segment id="9" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000009.dat" cachesize="1048576"/>
        <segment id="10" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000010.dat" cachesize="1048576"/>
        <segment id="11" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000011.dat" cachesize="1048576"/>
        <segment id="12" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000012.dat" cachesize="1048576"/>
        <segment id="13" size="1048576" tcid="2" name="/tmp/cfile_seg/test.log.xml.00000013.dat" cachesize="1048576"/>
        <segment id="14" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000014.dat" cachesize="1048576"/>
        <segment id="15" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000015.dat" cachesize="1048576"/>
        <segment id="16" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000016.dat" cachesize="1048576"/>
        <segment id="17" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000017.dat" cachesize="1048576"/>
        <segment id="18" size="1048576" tcid="2" name="/tmp/cfile_seg/test.log.xml.00000018.dat" cachesize="1048576"/>
        <segment id="19" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000019.dat" cachesize="1048576"/>
        <segment id="20" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000020.dat" cachesize="1048576"/>
        <segment id="21" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000021.dat" cachesize="1048576"/>
        <segment id="22" size="1048576" tcid="2" name="/tmp/cfile_seg/test.log.xml.00000022.dat" cachesize="1048576"/>
        <segment id="23" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000023.dat" cachesize="1048576"/>
        <segment id="24" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000024.dat" cachesize="1048576"/>
        <segment id="25" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000025.dat" cachesize="1048576"/>
        <segment id="26" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000026.dat" cachesize="1048576"/>
        <segment id="27" size="1048576" tcid="2" name="/tmp/cfile_seg/test.log.xml.00000027.dat" cachesize="1048576"/>
        <segment id="28" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000028.dat" cachesize="1048576"/>
        <segment id="29" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000029.dat" cachesize="1048576"/>
        <segment id="30" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000030.dat" cachesize="1048576"/>
        <segment id="31" size="1048576" tcid="1,2" name="/tmp/cfile_seg/test.log.xml.00000031.dat" cachesize="1048576"/>
        <segment id="32" size="1048576" tcid="2" name="/tmp/cfile_seg/test.log.xml.00000032.dat" cachesize="1048576"/>
        <segment id="33" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000033.dat" cachesize="1048576"/>
        <segment id="34" size="221540" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000034.dat" cachesize="221540"/>
    </segments>
</node>
\*************************************************************************************************************************************/

/**
*   for test only
*
*   to query the status of CFILE Module
*
**/
void cfile_print_module_status(const UINT32 cfile_md_id, LOG *log)
{
    CFILE_MD *cfile_md;
    UINT32 this_cfile_md_id;

    for( this_cfile_md_id = 0; this_cfile_md_id < CFILE_MD_CAPACITY(); this_cfile_md_id ++ )
    {
        cfile_md = CFILE_MD_GET(this_cfile_md_id);

        if ( NULL_PTR != cfile_md && 0 < cfile_md->usedcounter )
        {
            sys_log(log,"CFILE Module # %ld : %ld refered, refer CDIR Module : %ld\n",
                    this_cfile_md_id,
                    cfile_md->usedcounter,
                    cfile_md->cdir_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed CFILE module
*
*
**/
UINT32 cfile_free_module_static_mem(const UINT32 cfile_md_id)
{
    CFILE_MD  *cfile_md;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_free_module_static_mem: cfile module #0x%lx not started.\n",
                cfile_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);

    free_module_static_mem(MD_CFILE, cfile_md_id);

    return 0;
}

/**
*
* start CFILE module
*
**/
UINT32 cfile_start(const CVECTOR *node_tcid_vec, const CVECTOR *seg_tcid_vec)
{
    CFILE_MD *cfile_md;
    UINT32 cfile_md_id;
    UINT32 cdir_md_id;

    TASK_BRD *task_brd;

    cfile_md_id = cbc_md_new(MD_CFILE, sizeof(CFILE_MD));
    if(ERR_MODULE_ID == cfile_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one CFILE module */
    cfile_md = (CFILE_MD *)cbc_md_get(MD_CFILE, cfile_md_id);
    cfile_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();

    task_brd = task_brd_default_get();

    cdir_md_id = cdir_start(node_tcid_vec);
    if(ERR_MODULE_ID == cdir_md_id)
    {
        sys_log(LOGSTDOUT, "error:cfile_start: failed to start a CDIR module\n");
        return (ERR_MODULE_ID);
    }

    cfile_md->cdir_md_id = cdir_md_id;
    cdir_set_cfile_md_id(cdir_md_id, cfile_md_id);

    sys_log(LOGSTDOUT, "info:cfile_start: input node_tcid_vec is\n");
    cvector_print(LOGSTDOUT, node_tcid_vec, NULL_PTR);

    sys_log(LOGSTDOUT, "info:cfile_start: input seg_tcid_vec is\n");
    cvector_print(LOGSTDOUT, seg_tcid_vec, NULL_PTR);

    cvector_init(&(cfile_md->node_tcid_vec), cvector_size(node_tcid_vec), MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CFILE_0001);
    cvector_clone_with_prev_filter(node_tcid_vec, &(cfile_md->node_tcid_vec), task_brd, (CVECTOR_DATA_PREV_FILTER)task_brd_check_tcid_connected, NULL_PTR, NULL_PTR);

    cvector_init(&(cfile_md->seg_tcid_vec), cvector_size(seg_tcid_vec),  MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CFILE_0002);
    cvector_clone_with_prev_filter(seg_tcid_vec, &(cfile_md->seg_tcid_vec), task_brd, (CVECTOR_DATA_PREV_FILTER)task_brd_check_tcid_connected, NULL_PTR, NULL_PTR);

    sys_log(LOGSTDOUT, "info:cfile_start: output node_tcid_vec is\n");
    cvector_print(LOGSTDOUT, &(cfile_md->node_tcid_vec), NULL_PTR);

    sys_log(LOGSTDOUT, "info:cfile_start: output seg_tcid_vec is\n");
    cvector_print(LOGSTDOUT, &(cfile_md->seg_tcid_vec), NULL_PTR);

    /*default setting which will be override after cfile_set_mod_mgr calling*/
    cfile_md->mod_mgr = mod_mgr_new(cfile_md_id, LOAD_BALANCING_LOOP);

    cfile_md->usedcounter = 1;

    sys_log(LOGSTDOUT, "cfile_start: start CFILE module #%ld\n", cfile_md_id);
    //sys_log(LOGSTDOUT, "========================= cfile_start: CFILE table info:\n");
    //cfile_print_module_status(cfile_md_id, LOGSTDOUT);
    //cbc_print();

    return ( cfile_md_id );
}

/**
*
* end CFILE module
*
**/
void cfile_end(const UINT32 cfile_md_id)
{
    CFILE_MD *cfile_md;

    cfile_md = CFILE_MD_GET(cfile_md_id);
    if(NULL_PTR == cfile_md)
    {
        sys_log(LOGSTDOUT,"error:cfile_end: cfile_md_id = %ld not exist.\n", cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < cfile_md->usedcounter )
    {
        cfile_md->usedcounter --;
        return ;
    }

    if ( 0 == cfile_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:cfile_end: cfile_md_id = %ld is not started.\n", cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }

    /* if nobody else occupied the module,then free its resource */

    //task_brd_mod_mgr_rmv(cfile_md->task_brd, cfile_md->mod_mgr);
    mod_mgr_free(cfile_md->mod_mgr);
    cfile_md->mod_mgr  = NULL_PTR;

    cvector_clean(&(cfile_md->node_tcid_vec), NULL_PTR, LOC_CFILE_0003);
    cvector_clean(&(cfile_md->seg_tcid_vec), NULL_PTR, LOC_CFILE_0004);

    cdir_end(cfile_md->cdir_md_id);
    cfile_md->cdir_md_id = ERR_MODULE_ID;

    /* free module : */
    //cfile_free_module_static_mem(cfile_md_id);

    cfile_md->usedcounter = 0;

    sys_log(LOGSTDOUT, "cfile_end: stop CFILE module #%ld\n", cfile_md_id);
    cbc_md_free(MD_CFILE, cfile_md_id);

    breathing_static_mem();

    //sys_log(LOGSTDOUT, "========================= cfile_end: CFILE table info:\n");
    //cfile_print_module_status(cfile_md_id, LOGSTDOUT);
    //cbc_print();

    return ;
}

/**
*
* get CDIR module id of CFILE module
*
**/
EC_BOOL cfile_get_cdir_md_id(const UINT32 cfile_md_id, UINT32 *cdir_md_id)
{
    CFILE_MD *cfile_md;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_get_cdir_md_id: cfile module #0x%lx not started.\n",
                cfile_md_id);
        cfile_print_module_status(cfile_md_id, LOGSTDOUT);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);

    (*cdir_md_id) = cfile_md->cdir_md_id;
    return (EC_TRUE);
}

static UINT32 cfile_tcid_filter_out(const CVECTOR *tcid_vec, const UINT32 tcid)
{
    if(EC_TRUE == cvector_search_front(tcid_vec, (void *)tcid, NULL_PTR))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}


/**
*
* initialize mod mgr of CFILE module
*
**/
UINT32 cfile_set_mod_mgr(const UINT32 cfile_md_id, const MOD_MGR * src_mod_mgr)
{
    CFILE_MD *cfile_md;
    MOD_MGR  *des_mod_mgr;

    CVECTOR  *tcid_vec;
    UINT32    tcid_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_set_mod_mgr: cfile module #0x%lx not started.\n",
                cfile_md_id);
        cfile_print_module_status(cfile_md_id, LOGSTDOUT);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    des_mod_mgr = cfile_md->mod_mgr;

    sys_log(LOGSTDOUT, "cfile_set_mod_mgr: md_id %d, input src_mod_mgr %lx\n", cfile_md_id, src_mod_mgr);
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    cdir_set_mod_mgr(cfile_md->cdir_md_id, src_mod_mgr);

    tcid_num = cvector_size(&(cfile_md->node_tcid_vec)) + cvector_size(&(cfile_md->seg_tcid_vec));
    tcid_vec = cvector_new(tcid_num, MM_UINT32, LOC_CFILE_0005);

    cvector_clone_with_prev_filter(&(cfile_md->node_tcid_vec), tcid_vec, (void *)tcid_vec, (CVECTOR_DATA_PREV_FILTER)cfile_tcid_filter_out, NULL_PTR, NULL_PTR);
    cvector_clone_with_prev_filter(&(cfile_md->seg_tcid_vec), tcid_vec, (void *)tcid_vec, (CVECTOR_DATA_PREV_FILTER)cfile_tcid_filter_out, NULL_PTR, NULL_PTR);

    sys_log(LOGSTDOUT, "info:cfile_set_mod_mgr: output tcid_vec is\n");
    cvector_print(LOGSTDOUT, tcid_vec, NULL_PTR);

    /*figure out mod_nodes with tcid belong to set of node_tcid_vec and node_tcid_vec*/
    mod_mgr_limited_clone_with_tcid_filter(cfile_md_id, src_mod_mgr, tcid_vec, des_mod_mgr);

    cvector_free(tcid_vec, LOC_CFILE_0006);

    sys_log(LOGSTDOUT, "====================================cfile_set_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    sys_log(LOGSTDOUT, "====================================cfile_set_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);

    return (0);
}

/**
*
* get mod mgr of CFILE module
*
**/
MOD_MGR * cfile_get_mod_mgr(const UINT32 cfile_md_id)
{
    CFILE_MD *cfile_md;

    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        return (MOD_MGR *)0;
    }

    cfile_md = CFILE_MD_GET(cfile_md_id);
    return (cfile_md->mod_mgr);
}

/*---------------------------------- internal interface ----------------------------------*/
/**
*
* split ${home}/${dir} into dir slices and store them into dir_clist
* e.g.,
*   home = /tmp/cfile_segs
*   dir = ../cfile_node
* then return dir_clist = {tmp, cfile_node}
*
**/
UINT32 cfile_dir_split(const UINT32 cfile_md_id, CSTRING *home, CSTRING *dir, CLIST *dir_clist)
{
    char *beg;
    char *cur;
    char *end;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_dir_split: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    beg = (char *)cstring_get_str(dir);
    cur = beg + strspn(beg, "/");
    end = beg + strlen(beg);

    if(cur == beg && NULL_PTR != home && NULL_PTR != cstring_get_str(home))
    {
        if(0 != cfile_dir_split(cfile_md_id, NULL_PTR, home, dir_clist))
        {
            sys_log(LOGSTDOUT, "error:cfile_dir_split: invalid home\n");
            return ((UINT32)-1);
        }
    }

    while(cur < end)
    {
        char *dir_seg;
        dir_seg = cur;

        cur += strcspn(cur, "/");
        (*cur ++) = '\0';

        if(0 == strcmp(dir_seg, ".."))
        {
            if(NULL_PTR == clist_pop_back(dir_clist))
            {
                sys_log(LOGSTDOUT, "error:cfile_dir_split: invalid dir\n");
                return ((UINT32)-1);
            }
            continue;
        }

        if(0 == strcmp(dir_seg, "."))
        {
            continue;
        }

        if(0 == strcmp(dir_seg, ""))
        {
            continue;
        }
        clist_push_back(dir_clist, (void *)dir_seg);
    }

    return (0);
}

/**
*
* encapsulate the dir slices into a dir
* e.g.,
*   dir_clist = {tmp, cfile_node}
* then return dir = /tmp/cfile_node
*
**/
UINT32 cfile_dir_encap(const UINT32 cfile_md_id, const CLIST *dir_clist, CSTRING *dir)
{
    CLIST_DATA *clist_data;
    char *dir_seg;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_dir_encap: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cstring_reset(dir);
    CLIST_LOOP_NEXT(dir_clist, clist_data)
    {
        dir_seg = (char *)CLIST_DATA_DATA(clist_data);
        cstring_append_char(dir, '/');
        cstring_append_str(dir, (UINT8 *)dir_seg);
    }

    return (0);
}

/**
*
* figure out basename from file name
* e.g.,
*   file_name = /tmp/cfile_node/test.log.xml.segs/00000000.dat
* then return base_name = 00000000.dat
*
**/
UINT32 cfile_basename(const UINT32 cfile_md_id, const CSTRING *file_name, CSTRING *base_name)
{
    CSTRING *tmp_file_name;
    CLIST  *dir_clist;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_basename: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    tmp_file_name = cstring_new(cstring_get_str(file_name), LOC_CFILE_0007);
    dir_clist = clist_new(MM_IGNORE, LOC_CFILE_0008);

    if(0 != cfile_dir_split(cfile_md_id, NULL_PTR, tmp_file_name, dir_clist))
    {
        sys_log(LOGSTDOUT, "error:cfile_basename: invalid file name %s\n", (char *)cstring_get_str(file_name));

        clist_clean(dir_clist, NULL_PTR);
        clist_free(dir_clist, LOC_CFILE_0009);

        cstring_free(tmp_file_name);
        return ((UINT32)-1);
    }

    cstring_clean(base_name);
    cstring_init(base_name, (UINT8 *)clist_back(dir_clist));

    clist_clean(dir_clist, NULL_PTR);
    clist_free(dir_clist, LOC_CFILE_0010);

    cstring_free(tmp_file_name);

    return (0);
}

/**
*
* figure out dirname from file name
* e.g.,
*   file_name = /tmp/cfile_node/test.log.xml.segs/00000000.dat
* then return dir_name = /tmp/cfile_node/test.log.xml.segs
*
**/
UINT32 cfile_dirname(const UINT32 cfile_md_id, const CSTRING *file_name, CSTRING *dir_name)
{
    CSTRING *tmp_file_name;
    CLIST  *dir_clist;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_dirname: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    tmp_file_name = cstring_new(cstring_get_str(file_name), LOC_CFILE_0011);
    dir_clist = clist_new(MM_IGNORE, LOC_CFILE_0012);

    if(0 != cfile_dir_split(cfile_md_id, NULL_PTR, tmp_file_name, dir_clist))
    {
        sys_log(LOGSTDOUT, "error:cfile_dirname: invalid file name %s\n", (char *)cstring_get_str(file_name));

        clist_clean(dir_clist, NULL_PTR);
        clist_free(dir_clist, LOC_CFILE_0013);

        cstring_free(tmp_file_name);
        return ((UINT32)-1);
    }

    clist_pop_back(dir_clist);
    cfile_dir_encap(cfile_md_id, dir_clist, dir_name);
    cstring_append_char(dir_name, '/');

    clist_clean(dir_clist, NULL_PTR);
    clist_free(dir_clist, LOC_CFILE_0014);

    cstring_free(tmp_file_name);

    return (0);
}

/*---------------------------------- CFILE_SEG interface ----------------------------------*/
/**
*
* generate seg name by seg_id, and seg_dir_name where seg name format is
*   ${seg_dir_name} = ${node_xml_name}.segs
* e.g.,
*   seg_id = 1
*   seg_dir_name = /tmp/cfile_node.segs
* then return api_cfile_seg_name = /tmp/cfile_node.segs/00000001.dat
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
static UINT32 cfile_seg_name_gen(const UINT32 cfile_md_id, const UINT32 seg_id, const CSTRING *seg_dir_name, CSTRING *api_cfile_seg_name)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_name_gen: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cstring_format(api_cfile_seg_name, "%s/%08ld.dat",
                                        (char *)cstring_get_str(seg_dir_name),
                                        seg_id);
    //sys_log(LOGSTDOUT, "info:cfile_seg_name_gen: %s => %s\n", (char *)cstring_get_str(ui_cfile_seg_name), (char *)cstring_get_str(api_cfile_seg_name));
    return (0);
}

/**
*
* generate seg dir by node xml file name and the format is
*   ${seg_dir_name} = ${node_xml_name}.segs
* e.g.,
*   node_xml_name = /tmp/cfile_node/test.log.xml
* then return seg_dir_name = /tmp/cfile_node/test.log.xml.segs
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
static UINT32 cfile_seg_dir_name_gen(const UINT32 cfile_md_id, const CSTRING *node_name, CSTRING *seg_dir_name)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_dir_name_gen: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cstring_format(seg_dir_name, "%s.segs", (char *)cstring_get_str(node_name));

    return (0);
}

/**
*
* new a CFILE_SEG including
*   1, alloc memory for CFILE_SEG
*   2, empty TCID vector of CFILE_SEG
*   3, set codec(encoder, decoder,encoder size, intialization functions) of tcid with type = UINT32
*   4, set SEG ID with input
*   5, set SEG SIZE with input
*   6, generate seg name if both ui_cfile_seg_name and seg_dir_name are not null
*
* note: here not initialize kbuff,append_pos and open mode which are private data area
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
CFILE_SEG * cfile_seg_new(const UINT32 cfile_md_id, const UINT32 seg_id, const UINT32 seg_size, const CSTRING *seg_dir_name)
{
    CFILE_SEG *cfile_seg;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_new: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    alloc_static_mem(MD_CFILE, cfile_md_id, MM_CFILE_SEG, &cfile_seg, LOC_CFILE_0015);

    cfile_seg_init(cfile_md_id, cfile_seg);
    cvector_init(CFILE_SEG_TCID_VEC(cfile_seg), 0,  MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CFILE_0016);

    CFILE_SEG_ID(cfile_seg)   = seg_id;
    CFILE_SEG_SIZE(cfile_seg) = seg_size;

    /*file segment name format: [seg_dir_name]/[segment id].dat*/
    if(NULL_PTR != seg_dir_name)
    {
        cfile_seg_name_gen(cfile_md_id, seg_id, seg_dir_name, CFILE_SEG_NAME(cfile_seg));
    }
    return (cfile_seg);
}

/**
*
* push a seg tcid into CFILE_SEG if satisfy
*   1, seg tcid NOT in TCID vector of CFILE_SEG
*   2, seg tcid in CFILE MD seg TCID vector
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_tcid_push(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg, const UINT32 seg_tcid)
{
    CFILE_MD   *cfile_md;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_tcid_push: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);

    if(CVECTOR_ERR_POS == cvector_search_front(CFILE_SEG_TCID_VEC(cfile_seg), (void *)seg_tcid, NULL_PTR)
    && CVECTOR_ERR_POS != cvector_search_front(&(cfile_md->seg_tcid_vec), (void *)seg_tcid, NULL_PTR))
    {
        cvector_push(CFILE_SEG_TCID_VEC(cfile_seg), (void *)seg_tcid);
        return (0);
    }

    return ((UINT32)-1);
}

/**
*
* expand seg TCID vector to size CFILE_SEG_MAX_NUM_OF_TCID as possible as it can
*
* note:
*   1, the pushed tcid comes from CFILE_MD TCID vector
*   2, the result CFILE_SEG TCID vector size may not reach CFILE_SEG_MAX_NUM_OF_TCID if no enough suitable tcid(s)
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_tcid_expand(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    UINT32 seg_tcid_pos;
    UINT32 seg_tcid_num;
    UINT32 max_tcid_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_tcid_expand: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    max_tcid_num = cvector_size(&(cfile_md->seg_tcid_vec));
    seg_tcid_num = CFILE_SEG_TCID_NUM(cfile_seg);

    for(seg_tcid_pos = 0; seg_tcid_pos < max_tcid_num && seg_tcid_num < CFILE_SEG_MAX_NUM_OF_TCID; seg_tcid_pos ++)
    {
        UINT32 seg_tcid;

        seg_tcid = (UINT32)cvector_get(&(cfile_md->seg_tcid_vec), seg_tcid_pos);
        //TODO: check seg_tcid connectivity. skip it if not connected

        if(0 == cfile_seg_tcid_push(cfile_md_id, cfile_seg, seg_tcid))
        {
            seg_tcid_num ++;
        }
    }

    return (0);
}

/**
*
* initialize CFILE_SEG to empty or invalid status
*   1, initialize seg id to zero
*   2, initialize seg size to zero
*   3, initialize seg TCID vector to empty
*   4, initialize seg name to null string
*   5, initialize seg kbuff to empty
*   6, initialize seg append_pos to zero
*   7, initialize seg open mode to invalide mode
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_init(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_init: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    CFILE_SEG_ID(cfile_seg) = 0;
    CFILE_SEG_SIZE(cfile_seg) = 0;

    cvector_init(CFILE_SEG_TCID_VEC(cfile_seg), 0,  MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CFILE_0017);
    cstring_init(CFILE_SEG_NAME(cfile_seg), NULL_PTR);

    CFILE_SEG_OPEN_MODE(cfile_seg)  = CFILE_ERR_OPEN_MODE;
    return (0);
}

/**
*
* clean CFILE_SEG to empty or invalid status
*   1, clean seg id to zero
*   2, clean seg size to zero
*   3, clean seg TCID vector to empty
*   4, clean seg name to null string
*   5, clean seg kbuff to empty
*   6, clean seg append_pos to zero
*   7, clean seg open mode to invalide mode
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_clean(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_clean: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    CFILE_SEG_ID(cfile_seg) = 0;
    CFILE_SEG_SIZE(cfile_seg) = 0;

    cvector_clean(CFILE_SEG_TCID_VEC(cfile_seg), NULL_PTR, LOC_CFILE_0018);
    cstring_clean(CFILE_SEG_NAME(cfile_seg));

    CFILE_SEG_OPEN_MODE(cfile_seg)  = CFILE_ERR_OPEN_MODE;
    return (0);
}

/**
*
* free CFILE_SEG with cleanning
*   1, clean seg
*   2, free CFILE_SEG itself
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_free(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_free: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_seg_clean(cfile_md_id, cfile_seg);
    free_static_mem(MD_CFILE, cfile_md_id, MM_CFILE_SEG, cfile_seg, LOC_CFILE_0019);
    return (0);
}

/*---------------------------------- CFILE_SEG_VEC interface ----------------------------------*/
/**
*
* new a CFILE_SEG vector including
*   1, alloc memory for vector
*   2, initialize the vector with codec setting
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
CVECTOR *cfile_seg_vec_new(const UINT32 cfile_md_id)
{
    CVECTOR *cfile_seg_vec;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_new: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    alloc_static_mem(MD_CFILE, cfile_md_id, MM_CVECTOR, &cfile_seg_vec, LOC_CFILE_0020);

    cfile_seg_vec_init(cfile_md_id, cfile_seg_vec);
    return (cfile_seg_vec);
}

/**
*
* initialize CFILE_SEG vector to empty but initialize codec of CFILE_SEG
*   1, initialze cvector to empty
*   2, initialize cvector codec of CFILE_SEG
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_vec_init(const UINT32 cfile_md_id, CVECTOR *cfile_seg_vec)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_init: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cvector_init(cfile_seg_vec, 0,  MM_CFILE_SEG, CVECTOR_LOCK_ENABLE, LOC_CFILE_0021);

    return (0);
}

/**
*
* clean CFILE_SEG vector to empty
*   1, free all segs in the vector
*   2, clean vector to empty
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_vec_clean(const UINT32 cfile_md_id, CVECTOR *cfile_seg_vec)
{
    UINT32 cfile_seg_num;
    UINT32 cfile_seg_pos;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_clean: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_seg_num = cvector_size(cfile_seg_vec);
    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        if(NULL_PTR != cfile_seg)
        {
            cfile_seg_free(cfile_md_id, cfile_seg);
        }
    }

    cvector_clean(cfile_seg_vec, NULL_PTR, LOC_CFILE_0022);
    return (0);
}

/**
*
* free CFILE_SEG vector with cleanning
*   1, clean CFILE_SEG vector
*   2, free vector itself
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_vec_free(const UINT32 cfile_md_id, CVECTOR *cfile_seg_vec)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_free: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_seg_vec_clean(cfile_md_id, cfile_seg_vec);
    free_static_mem(MD_CFILE, cfile_md_id, MM_CVECTOR, cfile_seg_vec, LOC_CFILE_0023);
    return (0);
}

/*---------------------------------- CFILE_NODE interface ----------------------------------*/
/**
*
* generate node name by ui_cfile_node_name and node_dir_name where node name format is
*   ${api_cfile_node_name} = ${node_dir_name}/basename(${ui_cfile_node_name}).xml
* e.g.,
*   ui_cfile_node_name = /tmp/cfile_upload/test.log
*   node_dir_name = /tmp/cfile_node
* then return api_cfile_node_name = /tmp/cfile_node/test.log.xml
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
static UINT32 cfile_node_name_gen(const UINT32 cfile_md_id, const CSTRING *ui_cfile_node_name, CSTRING *api_cfile_node_name)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_name_gen: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(NULL_PTR == ui_cfile_node_name)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_name_gen: ui_cfile_node_name is null\n");
        return ((UINT32)-1);
    }

    if(NULL_PTR == cstring_get_str(ui_cfile_node_name))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_name_gen: ui_cfile_node_name str is null\n");
        return ((UINT32)-1);
    }

    cstring_format(api_cfile_node_name, "%s.xml", (char *)cstring_get_str(ui_cfile_node_name));
    //sys_log(LOGSTDOUT, "info:cfile_node_name_gen: %s => %s\n", (char *)cstring_get_str(ui_cfile_node_name), (char *)cstring_get_str(api_cfile_node_name));
    return (0);
}

/**
*
* new a CFILE_NODE including
*   1, alloc memory for CFILE_NODE
*   2, initialize CFILE_NODE to empty or invalid status
*   3, generate node name if both ui_cfile_node_name and node_dir_name are not null
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
CFILE_NODE * cfile_node_new(const UINT32 cfile_md_id, const UINT32 ui_cfile_size, const CSTRING *ui_cfile_node_name)
{
    CFILE_MD   *cfile_md;
    CFILE_NODE *cfile_node;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_new: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);

    alloc_static_mem(MD_CFILE, cfile_md_id, MM_CFILE_NODE, &cfile_node, LOC_CFILE_0024);
    cfile_node_init(cfile_md_id, cfile_node);

    if(NULL_PTR != ui_cfile_node_name)
    {
        cfile_node_name_gen(cfile_md_id, ui_cfile_node_name, CFILE_NODE_NAME(cfile_node));
    }

    CFILE_NODE_SIZE(cfile_node) = ui_cfile_size;

    return (cfile_node);
}

/**
*
* push a node tcid into CFILE_NODE if satisfy
*   1, node tcid NOT in TCID vector of CFILE_NODE
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_tcid_push(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, const UINT32 node_tcid)
{
    CFILE_MD *cfile_md;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_tcid_push: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);

    if(CVECTOR_ERR_POS == cvector_search_front(CFILE_NODE_TCID_VEC(cfile_node), (void *)node_tcid, NULL_PTR)
    && CVECTOR_ERR_POS != cvector_search_front(&(cfile_md->node_tcid_vec), (void *)node_tcid, NULL_PTR))
    {
        cvector_push(CFILE_NODE_TCID_VEC(cfile_node), (void *)node_tcid);/*trick!*/
        return (0);
    }
    return ((UINT32)-1);
}

/**
*
* initialize CFILE_NODE to empty or invalid status
*   1, initialize node name to null string
*   2, initialize node size to zero
*   3, initialize node TCID vector to empty
*   4, initialize seg VECTOR to empty but set codec of CFILE_SEG
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_init(const UINT32 cfile_md_id, CFILE_NODE *cfile_node)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_init: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cstring_init(CFILE_NODE_NAME(cfile_node), NULL_PTR);
    cvector_init(CFILE_NODE_TCID_VEC(cfile_node), 0,  MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CFILE_0025);
    CFILE_NODE_SIZE(cfile_node) = 0;
    cfile_seg_vec_init(cfile_md_id, CFILE_SEG_VEC(cfile_node));

    return (0);
}

/**
*
* clean CFILE_NODE to empty or invalid status
*   1, clean node name to null string
*   2, clean node size to zero
*   3, clean node TCID vector to empty
*   4, clean seg vector to empty
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_clean(const UINT32 cfile_md_id, CFILE_NODE *cfile_node)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_clean: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cstring_clean(CFILE_NODE_NAME(cfile_node));
    CFILE_NODE_SIZE(cfile_node) = 0;
    cvector_clean(CFILE_NODE_TCID_VEC(cfile_node), NULL_PTR, LOC_CFILE_0026);
    cfile_seg_vec_clean(cfile_md_id, CFILE_SEG_VEC(cfile_node));
    return (0);
}

/**
*
* free CFILE_NODE with cleanning
*   1, clean node
*   2, free CFILE_NODE itself
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_free(const UINT32 cfile_md_id, CFILE_NODE *cfile_node)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_free: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_node_clean(cfile_md_id, cfile_node);
    free_static_mem(MD_CFILE, cfile_md_id, MM_CFILE_NODE, cfile_node, LOC_CFILE_0027);
    return (0);
}

/*---------------------------------- CFILE_NODE XML Parser interface ----------------------------------*/

#define CFILE_NODE_XML_SKIP_TEXT_NODE(cur) if(xmlNodeIsText(cur))  { continue; }

/**
*
* parse node xml file to XML DOC
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
XMLDOCPTR cfile_node_xml_new(const UINT32 cfile_md_id, const UINT8 *cfile_node_xml_doc_name)
{
    XMLDOCPTR cfile_node_xml_doc_ptr;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_xml_new: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_node_xml_doc_ptr = xmlParseFile((const char *)cfile_node_xml_doc_name);
    if(NULL_PTR == cfile_node_xml_doc_ptr)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_xml_new: failed to parse %s\n", (const char *)cfile_node_xml_doc_name);
        return ((XMLDOCPTR)0);
    }
    return (cfile_node_xml_doc_ptr);
}

/**
*
* get root node of XML DOC
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
XMLNODEPTR cfile_node_xml_get_root(const UINT32 cfile_md_id, XMLDOCPTR cfile_node_xml_doc_ptr)
{
    XMLNODEPTR cfile_xml_node_ptr;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_xml_get_root: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_xml_node_ptr = xmlDocGetRootElement(cfile_node_xml_doc_ptr);
    if(NULL_PTR == cfile_xml_node_ptr)
    {
        sys_log(LOGSTDOUT, "warn:cfile_node_xml_get_root: empty document\n");
        return ((XMLNODEPTR)0);
    }
    return (cfile_xml_node_ptr);
}

/**
*
* free XML DOC
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
void cfile_node_xml_free(const UINT32 cfile_md_id, XMLDOCPTR cfile_node_xml_doc_ptr)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_xml_free: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    xmlFreeDoc(cfile_node_xml_doc_ptr);
    return;
}

/**
*
* parse tcid list and split them into tcid vector
* e.g.
*   tcid list(i.e., attr_val) = "1,2,3"
* then
*   tcid vector = [1,2,3]
*
* note: input attr_val will be changed, hence not reuse it after returning
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_xml_parse_tcid(const UINT32 cfile_md_id, XMLCHAR *attr_val, CVECTOR *tcid_vec)
{
    char *safe_ptr;
    char *tcid_ptr;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_xml_parse_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

/**
       The strtok_r() function is thread-safe and stores its state in a user-supplied buffer instead of possibly using  a  static
       data area that may be overwritten by an unrelated call from another thread.
**/
    safe_ptr = (char *)attr_val;/*attr_val will be polished*/
    while((char *)0 != (tcid_ptr = strtok_r(NULL_PTR, ",", &safe_ptr)))
    {
        cvector_push(tcid_vec, (void *)c_ipv4_to_word(tcid_ptr));/*trick*/
    }

    return (0);
}

/**
*
* parse single segment line to CFILE_SEG
* e.g., segment line is
*   <segment id="0" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000000.dat" cachesize="1048576"/>
* then
*   CFILE_SEG_ID       = 0
*   CFILE_SEG_SIZE     = 1048576
*   CFILE_SEG_TCID_VEC = [2,1]
*   CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000000.dat"
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_xml_parse_seg(const UINT32 cfile_md_id, XMLNODEPTR node, CFILE_SEG *cfile_seg)
{
    XMLCHAR *attr_val;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_xml_parse_seg: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(xmlHasProp(node, (const XMLCHAR*)"id"))
    {
        attr_val = xmlGetProp(node, (const XMLCHAR*)"id");
        //sys_log(LOGSTDOUT,"info:cfile_node_xml_parse_seg: id=>%s\n", attr_val);
        CFILE_SEG_ID(cfile_seg) = c_xmlchar_to_word(attr_val);
        xmlFree(attr_val);
    }

    if(xmlHasProp(node, (const XMLCHAR*)"size"))
    {
        attr_val = xmlGetProp(node, (const XMLCHAR*)"size");
        //sys_log(LOGSTDOUT,"info:cfile_node_xml_parse_seg: size=>%s\n", attr_val);
        CFILE_SEG_SIZE(cfile_seg) = c_xmlchar_to_word(attr_val);
        xmlFree(attr_val);
    }

    if(xmlHasProp(node, (const XMLCHAR*)"tcid"))
    {
        attr_val = xmlGetProp(node, (const XMLCHAR*)"tcid");
        cfile_node_xml_parse_tcid(cfile_md_id, attr_val, CFILE_SEG_TCID_VEC(cfile_seg));
        xmlFree(attr_val);
    }

    if(xmlHasProp(node, (const XMLCHAR*)"name"))
    {
        attr_val = xmlGetProp(node, (const XMLCHAR*)"name");
        //sys_log(LOGSTDOUT,"info:cfile_node_xml_parse_seg: name=>%s\n", attr_val);
        cstring_init(CFILE_SEG_NAME(cfile_seg), (const UINT8 *)attr_val);
        xmlFree(attr_val);
    }
    return (0);
}

/**
*
* parse multiple segment lines to CFILE_SEG vector
* e.g,
*   <segment id="0" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000000.dat" cachesize="1048576"/>
*   <segment id="1" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000001.dat" cachesize="1048576"/>
* then
*   CFILE_SEG vector = [seg_1, seg_2]
* where
*   seg_1 = {
*       CFILE_SEG_ID       = 0
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000000.dat"
*   }
*
*   seg_2 = {
*       CFILE_SEG_ID       = 1
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000001.dat"
*   }
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_xml_parse_seg_vec(const UINT32 cfile_md_id, XMLNODEPTR node, CVECTOR *cfile_seg_vec)
{
    XMLNODEPTR cur;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_xml_parse_seg_vec: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    for(cur = node->xmlChildrenNode; NULL_PTR != cur; cur = cur->next)
    {
        CFILE_NODE_XML_SKIP_TEXT_NODE(cur);
        //sys_log(LOGSTDOUT, "info:cfile_node_xml_parse_seg_vec: cur name: %s\n", cur->name);

        if(0 == xmlStrcmp(cur->name, (const XMLCHAR *)"segment"))
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = cfile_seg_new(cfile_md_id, 0, 0, NULL_PTR);

            cfile_node_xml_parse_seg(cfile_md_id, cur, cfile_seg);

            cvector_push(cfile_seg_vec, (void *)cfile_seg);
            continue;
        }

        //sys_log(LOGSTDOUT, "error:cfile_node_xml_parse_seg_vec: unknow name: %s\n", cur->name);
    }

    return (0);
}

/**
*
* parse single node block to CFILE_NODE
* e.g., node block is
* <node name="/tmp/cfile_node/test.log.xml" tcid="2" size="2097152">
*     <segments>
*         <segment id="0" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000000.dat" cachesize="1048576"/>
*         <segment id="1" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000001.dat" cachesize="1048576"/>
*     </segments>
* </node>
* then
*   NODE_NAME     = /tmp/cfile_node/test.log.xml
*   NODE_TCID_VEC = [2]
*   NODE_SIZE     = 2097152
*   NODE_SEG_VEC  = [seg_1, seg_2]
* where
*   seg_1 = {
*       CFILE_SEG_ID       = 0
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000000.dat"
*   }
*
*   seg_2 = {
*       CFILE_SEG_ID       = 1
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000001.dat"
*   }
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_xml_parse_node(const UINT32 cfile_md_id, XMLNODEPTR node, CFILE_NODE *cfile_node)
{
    XMLCHAR *attr_val;
    XMLNODEPTR cur;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_xml_parse_node: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(xmlHasProp(node, (const XMLCHAR*)"name"))
    {
        attr_val = xmlGetProp(node, (const XMLCHAR*)"name");
        //sys_log(LOGSTDOUT,"info:cfile_node_xml_parse_node: name=>%s\n", attr_val);

        if(0 != strcmp((char *)CFILE_NODE_NAME_STR(cfile_node), (char *)attr_val))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_xml_parse_node: mismatched file name %s of node and xml name tag value %s\n",
                                (char *)CFILE_NODE_NAME_STR(cfile_node), (char *)attr_val);
            xmlFree(attr_val);
            return ((UINT32)-1);
        }
#if 0
        cstring_init(CFILE_NODE_NAME(cfile_node), (const UINT8 *)attr_val);
#endif
        xmlFree(attr_val);
    }

    if(xmlHasProp(node, (const XMLCHAR*)"tcid"))
    {
        attr_val = xmlGetProp(node, (const XMLCHAR*)"tcid");
        //sys_log(LOGSTDOUT,"info:cfile_node_xml_parse_node: tcid=>%s\n", attr_val);
#if 1
        cfile_node_xml_parse_tcid(cfile_md_id, attr_val, CFILE_NODE_TCID_VEC(cfile_node));
        xmlFree(attr_val);
#endif
#if 0
        if(CMPI_ANY_TCID == CFILE_NODE_TCID(cfile_node, 0))
        {
            cvector_set(CFILE_NODE_TCID_VEC(cfile_node), 0, (void *)c_xmlchar_to_word(attr_val));
            xmlFree(attr_val);
        }

        else if(CFILE_NODE_TCID(cfile_node, 0) != c_xmlchar_to_word(attr_val))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_xml_parse_node: mismatched tcid %s of node and xml tcid tag value %s\n",
                                CFILE_NODE_TCID_STR(cfile_node, 0), (char *)attr_val);
            xmlFree(attr_val);
            return ((UINT32)-1);
        }
        else
        {
            xmlFree(attr_val);
        }
#endif

    }

    if(xmlHasProp(node, (const XMLCHAR*)"size"))
    {
        attr_val = xmlGetProp(node, (const XMLCHAR*)"size");
        //sys_log(LOGSTDOUT,"info:cfile_node_xml_parse_node: size=>%s\n", attr_val);
        CFILE_NODE_SIZE(cfile_node) = c_xmlchar_to_word(attr_val);
        xmlFree(attr_val);
    }

    for(cur = node->xmlChildrenNode; NULL_PTR != cur; cur = cur->next)
    {
        CFILE_NODE_XML_SKIP_TEXT_NODE(cur);
        //sys_log(LOGSTDOUT, "info:cfile_node_xml_parse_node: cur name: %s\n", cur->name);

        if(0 == xmlStrcmp(cur->name, (const XMLCHAR *)"segments"))
        {
            cfile_node_xml_parse_seg_vec(cfile_md_id, cur, CFILE_SEG_VEC(cfile_node));
            break;
        }
    }
    return (0);
}

/*---------------------------------- CFILE_NODE XML Printer interface ----------------------------------*/
/**
*
* print space indent to LOG
* the space number = level * 4
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
void cfile_node_print_ident(LOG *log, const UINT32 level)
{
    UINT32 idx;

    for(idx = 0; idx < level; idx ++)
    {
        sys_print(log, "    ");
    }
    return;
}

/**
*
* print TCID vector seperating by comma to LOG
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_xml_print_tcid(LOG *log, const CVECTOR *tcid_vec, const UINT32 level)
{
    UINT32 pos;
    UINT32 num;

    cfile_node_print_ident(log, level);

    num = cvector_size(tcid_vec);
    pos = 0;

    if(0 < num)
    {
        sys_print(log, "%s", c_word_to_ipv4((UINT32)cvector_get(tcid_vec, pos)));
        pos ++;
    }

    for(; pos < num; pos ++)
    {
        sys_print(log, ",%s", c_word_to_ipv4((UINT32)cvector_get(tcid_vec, pos)));
    }

    return (0);
}

/**
*
* print CFILE_SEG to LOG
* e.g.,
*   CFILE_SEG_ID       = 0
*   CFILE_SEG_SIZE     = 1048576
*   CFILE_SEG_TCID_VEC = [2,1]
*   CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000000.dat"
* then output
*   <segment id="0" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000000.dat" cachesize="1048576"/>
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_xml_print_seg(LOG *log, const CFILE_SEG *cfile_seg, const UINT32 level)
{
    cfile_node_print_ident(log, level);

    sys_print(log, "<segment id=\"%ld\" size=\"%ld\" ",
                    CFILE_SEG_ID(cfile_seg),
                    CFILE_SEG_SIZE(cfile_seg));

    sys_print(log, "tcid=\"");
    cfile_node_xml_print_tcid(log, CFILE_SEG_TCID_VEC(cfile_seg), 0);/*set this level to zero*/
    sys_print(log, "\" ");

    sys_print(log, "name=\"%s\"/>\n",
                    NULL_PTR == CFILE_SEG_NAME_STR(cfile_seg)? "" : (char *)CFILE_SEG_NAME_STR(cfile_seg));

    return (0);
}

/**
*
* print CFILE_SEG vector to LOG
* e.g,
*   CFILE_SEG vector = [seg_1, seg_2]
* where
*   seg_1 = {
*       CFILE_SEG_ID       = 0
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000000.dat"
*   }
*
*   seg_2 = {
*       CFILE_SEG_ID       = 1
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000001.dat"
*   }
* then output
*   <segment id="0" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000000.dat" cachesize="1048576"/>
*   <segment id="1" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000001.dat" cachesize="1048576"/>
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_xml_print_seg_vec(LOG *log, const CVECTOR *cfile_seg_vec, const UINT32 level)
{
    UINT32 pos;

    cfile_node_print_ident(log, level);
    sys_print(log, "<segments>\n");

    for(pos = 0; pos < cvector_size(cfile_seg_vec); pos ++)
    {
        CFILE_SEG *cfile_seg;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, pos);
        cfile_node_xml_print_seg(log, cfile_seg, level + 1);
    }

    cfile_node_print_ident(log, level);
    sys_print(log, "</segments>\n");

    return (0);
}

/**
*
* print single CFILE_NODE to LOG
* e.g., CFILE_NODE is
*   NODE_NAME     = /tmp/cfile_node/test.log.xml
*   NODE_TCID_VEC = [2]
*   NODE_SIZE     = 2097152
*   NODE_SEG_VEC  = [seg_1, seg_2]
* where
*   seg_1 = {
*       CFILE_SEG_ID       = 0
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000000.dat"
*   }
*
*   seg_2 = {
*       CFILE_SEG_ID       = 1
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000001.dat"
*   }
* then output
* <node name="/tmp/cfile_node/test.log.xml" tcid="2" size="2097152">
*     <segments>
*         <segment id="0" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000000.dat" cachesize="1048576"/>
*         <segment id="1" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000001.dat" cachesize="1048576"/>
*     </segments>
* </node>
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_xml_print_node(LOG *log, const CFILE_NODE *cfile_node, const UINT32 level)
{
    cfile_node_print_ident(log, level);
    sys_print(log, "<node name=\"%s\" ", (char *)CFILE_NODE_NAME_STR(cfile_node));

    sys_print(log, "tcid=\"");
    cfile_node_xml_print_tcid(log, CFILE_NODE_TCID_VEC(cfile_node), 0);/*set this level to zero*/
    sys_print(log, "\" ");

    sys_print(log, "size=\"%ld\">\n", CFILE_NODE_SIZE(cfile_node));

    cfile_node_xml_print_seg_vec(log, CFILE_SEG_VEC(cfile_node), level + 1);

    cfile_node_print_ident(log, level);
    sys_print(log, "</node>\n");

    return (0);
}

/**
*
* print single CFILE_NODE with specific node tcid to LOG
* e.g.,node tcid = 3 and  CFILE_NODE is
*   NODE_NAME     = /tmp/cfile_node/test.log.xml
*   NODE_TCID_VEC = [2]
*   NODE_SIZE     = 2097152
*   NODE_SEG_VEC  = [seg_1, seg_2]
* where
*   seg_1 = {
*       CFILE_SEG_ID       = 0
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000000.dat"
*   }
*
*   seg_2 = {
*       CFILE_SEG_ID       = 1
*       CFILE_SEG_SIZE     = 1048576
*       CFILE_SEG_TCID_VEC = [2,1]
*       CFILE_SEG_NAME     = "/tmp/cfile_seg/test.log.xml.00000001.dat"
*   }
* then output
* <node name="/tmp/cfile_node/test.log.xml" tcid="3" size="2097152">
*     <segments>
*         <segment id="0" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000000.dat" cachesize="1048576"/>
*         <segment id="1" size="1048576" tcid="2,1" name="/tmp/cfile_seg/test.log.xml.00000001.dat" cachesize="1048576"/>
*     </segments>
* </node>
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_xml_print_node_with_node_tcid(LOG *log, const CFILE_NODE *cfile_node, const UINT32 node_tcid, const UINT32 level)
{
    cfile_node_print_ident(log, level);
    sys_print(log, "<node name=\"%s\" ", (char *)CFILE_NODE_NAME_STR(cfile_node));

    sys_print(log, "tcid=\"%ld\" ", node_tcid);

    sys_print(log, "size=\"%ld\">\n", CFILE_NODE_SIZE(cfile_node));

    cfile_node_xml_print_seg_vec(log, CFILE_SEG_VEC(cfile_node), level + 1);

    cfile_node_print_ident(log, level);
    sys_print(log, "</node>\n");

    return (0);
}

/*---------------------------------- CFILE_NODE file operation interface ----------------------------------*/
/**
*
* read node xml file and import its content to CFILE_NODE
* 1, node xml file must be existing
* 2, node xml file must be readable
* 3, parse node xml file to CFILE_NODE
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_read(const UINT32 cfile_md_id, CFILE_NODE *cfile_node)
{
    XMLDOCPTR  cfile_node_xml_doc;
    XMLNODEPTR cfile_node_xml_root;
    UINT8 * file_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_read: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    file_name_str = CFILE_NODE_NAME_STR(cfile_node);
    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_read: file name is null\n");
        return ((UINT32)-1);
    }

    /*not exist*/
    if(0 != access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_read: file %s not exist\n", (char *)file_name_str);
        return ((UINT32)-1);
    }

    /*not readable*/
    if(0 != access((char *)file_name_str, R_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_read: file %s not readable\n", (char *)file_name_str);
        return ((UINT32)-1);
    }

    cfile_node_xml_doc  = cfile_node_xml_new(cfile_md_id, file_name_str);
    if((XMLDOCPTR)0 == cfile_node_xml_doc)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_read: failed to open %s to read\n", (char *)file_name_str);
        return ((UINT32)-1);
    }
    cfile_node_xml_root = cfile_node_xml_get_root(cfile_md_id, cfile_node_xml_doc);

    if(0 != cfile_node_xml_parse_node(cfile_md_id, cfile_node_xml_root, cfile_node))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_read: failed to xml parse node of file %s\n", (char *)file_name_str);
        cxml_free(cfile_node_xml_doc);
        return ((UINT32)-1);
    }

    cxml_free(cfile_node_xml_doc);

    return (0);
}

/**
*
* write/export CFILE_NODE to node xml file
* 1, if target node xml file is existing but not writable, then report error and export nothing
* 2, if target node xml file is existing and writable, then export CFILE_NODE to it with overriding
* 3, if target node xml file is not existing, then create one and export CFILE_NODE to it
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_write(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
    LOG  *cfile_node_xml_log;
    UINT8 * file_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_write: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    file_name_str = CFILE_NODE_NAME_STR(cfile_node);
    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_write: file name is null\n");
        return ((UINT32)-1);
    }

    /*exist but not writable*/
    if(0 == access((char *)file_name_str, F_OK) && 0 != access((char *)file_name_str, W_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_write: file %s exist but not writable\n", (char *)file_name_str);
        return ((UINT32)-1);
    }

    cfile_node_xml_log = log_file_open((char *)file_name_str, "w",
                                    CMPI_LOCAL_TCID, CMPI_LOCAL_RANK,
                                    LOGD_FILE_RECORD_LIMIT_DISABLED, (UINT32)SWITCH_OFF,
                                    LOGD_SWITCH_OFF_DISABLE, LOGD_PID_INFO_ENABLE);
    if(NULL_PTR == cfile_node_xml_log)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_write: failed to open %s to write\n", (char *)file_name_str);
        return ((UINT32)-1);
    }

    cfile_node_xml_print_node(cfile_node_xml_log, cfile_node, 0);
    log_file_close(cfile_node_xml_log);/*close cfile_node_xml_fp*/
    return (0);
}

/**
*
* write/export CFILE_NODE to node xml file
* 1, if target node xml file is existing but not writable, then report error and export nothing
* 2, if target node xml file is existing and writable, then export CFILE_NODE to it with overriding
* 3, if target node xml file is not existing, then create one and export CFILE_NODE to it
*
* note: NODE TCID will be replaced with input node_tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_write_with_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 node_tcid)
{
    LOG  *cfile_node_xml_log;
    UINT8 * file_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_write_with_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    file_name_str = CFILE_NODE_NAME_STR(cfile_node);
    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_write_with_node_tcid: file name is null\n");
        return ((UINT32)-1);
    }

    /*exist but not writable*/
    if(0 == access((char *)file_name_str, F_OK) && 0 != access((char *)file_name_str, W_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_write_with_node_tcid: file %s exist but not writable\n", (char *)file_name_str);
        return ((UINT32)-1);
    }

    cfile_node_xml_log = log_file_open((char *)file_name_str, "w",
                                    CMPI_LOCAL_TCID, CMPI_LOCAL_RANK,
                                    LOGD_FILE_RECORD_LIMIT_DISABLED, (UINT32)SWITCH_OFF,
                                    LOGD_SWITCH_OFF_DISABLE, LOGD_PID_INFO_ENABLE);
    if(NULL_PTR == cfile_node_xml_log)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_write_with_node_tcid: failed to open %s to write\n", (char *)file_name_str);
        return ((UINT32)-1);
    }

    cfile_node_xml_print_node_with_node_tcid(cfile_node_xml_log, cfile_node, node_tcid, 0);
    log_file_close(cfile_node_xml_log);/*close cfile_node_xml_fp*/
    return (0);
}

/**
*
* create a node xml file if not existing
* 1, if node xml file exist, then give up creating without exporting
* 2, if node xml file not exist, then create one and export CFILE_NODE to it
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_fcreate(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_fcreate: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    /*exist*/
    if(0 == access((char *)CFILE_NODE_NAME_STR(cfile_node), F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fcreate: file %s already exist\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
        return ((UINT32)-1);
    }

    cfile_node_write(cfile_md_id, cfile_node);

    return (0);
}

/**
*
* open CFILE_NODE
*
* 1, open a node xml file and importing its content to CFILE_NODE
*
* note:
*  1, the node xml file name must have been set in CFILE_NODE
*  2, close CFILE_NODE is not mandatory after open CFILE_NODE
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_fopen(const UINT32 cfile_md_id, CFILE_NODE *cfile_node)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_fopen: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(0 != cfile_node_read(cfile_md_id, cfile_node))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fopen: failed to read node info from file %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
        return ((UINT32)-1);
    }

    sys_log(LOGSTDOUT, "cfile_node_fopen: after cfile_node_read, CFILE_NODE_TCID_VEC is:\n");
    cvector_print(LOGSTDOUT, CFILE_NODE_TCID_VEC(cfile_node), NULL_PTR);
    return (0);
}

/**
*
* close CFILE_NODE
*
* 1, close a CFILE_NODE with exporting its info to node xml file
*
* note:
*  1, the node xml file name must have been set in CFILE_NODE
*  2, close CFILE_NODE does not need open CFILE_NODE before
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_fclose(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_fclose: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_node_write(cfile_md_id, cfile_node);
    return (0);
}

/**
*
* check CFILE_NODE exist or not
*
* 1, if node xml file exist, return EC_TRUE, otherwise return EC_FALSE
*
* note:
*  1, the node xml file name must have been set in CFILE_NODE
*  2, check CFILE_NODE existing does not need open CFILE_NODE before
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_node_fexist(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
    UINT8 * file_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_fexist: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    file_name_str = CFILE_NODE_NAME_STR(cfile_node);
    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fexist: file name is null\n");
        return (EC_FALSE);
    }

    /*not exist*/
    if(0 != access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fexist: file %s not exist on tcid %s\n", (char *)file_name_str, CFILE_LOCAL_TCID_STR(cfile_md_id));
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "info:cfile_node_fexist: node %s exist on tcid %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node), CFILE_LOCAL_TCID_STR(cfile_md_id));
    return (EC_TRUE);
}

/**
*
* check CFILE_NODE readable or not
* 1, if node xml file not exist, return EC_FALSE
* 2, if node xml file is readable, return EC_TRUE, otherwise return EC_FALSE
*
* note:
*  1, the node xml file name must have been set in CFILE_NODE
*  2, check CFILE_NODE readable does not need open CFILE_NODE before
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_node_frable(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
    UINT8 * file_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_frable: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    file_name_str = CFILE_NODE_NAME_STR(cfile_node);
    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_frable: file name is null\n");
        return (EC_FALSE);
    }

    /*not exist*/
    if(0 != access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_frable: file %s not exist\n", (char *)file_name_str);
        return (EC_FALSE);
    }

    /*not readable*/
    if(0 != access((char *)file_name_str, R_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_frable: file %s not readable\n", (char *)file_name_str);
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "info:cfile_node_frable: node %s is readable on tcid %s\n",
                        (char *)CFILE_NODE_NAME_STR(cfile_node), CFILE_LOCAL_TCID_STR(cfile_md_id));
    return (EC_TRUE);
}

/**
*
* check CFILE_NODE writable or not
* 1, if node xml file not exist, return EC_FALSE
* 2, if node xml file is writable, return EC_TRUE, otherwise return EC_FALSE
*
* note:
*  1, the node xml file name must have been set in CFILE_NODE
*  2, check CFILE_NODE writable does not need open CFILE_NODE before
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_node_fwable(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
    UINT8 * file_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_fwable: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    file_name_str = CFILE_NODE_NAME_STR(cfile_node);
    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fwable: file name is null\n");
        return (EC_FALSE);
    }

    /*not exist*/
    if(0 != access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fwable: file %s not exist\n", (char *)file_name_str);
        return (EC_TRUE);/*warning: maybe here we should further check whethere dir is writable*/
    }

    /*exist but not writable*/
    if(0 != access((char *)file_name_str, W_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fwable: file %s exist but not writable\n", (char *)file_name_str);
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "info:cfile_node_fwable: node %s is readable on tcid %s\n",
                        (char *)CFILE_NODE_NAME_STR(cfile_node), CFILE_LOCAL_TCID_STR(cfile_md_id));
    return (EC_TRUE);
}

/**
*
* check CFILE_NODE executable or not
* 1, if node xml file not exist, return EC_FALSE
* 2, if node xml file is executable, return EC_TRUE, otherwise return EC_FALSE
*
* note:
*  1, the node xml file name must have been set in CFILE_NODE
*  2, check CFILE_NODE executable does not need open CFILE_NODE before
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_node_fxable(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
    UINT8 * file_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_fxable: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    file_name_str = CFILE_NODE_NAME_STR(cfile_node);
    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fxable: file name is null\n");
        return (EC_FALSE);
    }

    /*not exist*/
    if(0 != access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fxable: file %s not exist\n", (char *)file_name_str);
        return (EC_FALSE);
    }

    /*not executable*/
    if(0 != access((char *)file_name_str, X_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_fxable: file %s exist but not executable\n", (char *)file_name_str);
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "info:cfile_node_fwable: node %s is executable on tcid %s\n",
                        (char *)CFILE_NODE_NAME_STR(cfile_node), CFILE_LOCAL_TCID_STR(cfile_md_id));
    return (EC_TRUE);
}

/**
*
* remove CFILE_NODE
* 1, try to remove node xml file
*
* note:
*  1, the node xml file name must have been set in CFILE_NODE
*  2, the node xml file must exist
*  3, the node xml file is able to remove by the user
*  4, the node xml file will be removed without its segs removing, hence user have to remove its segs in other place
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_rmv(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
    UINT8 * file_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_rmv: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    file_name_str = CFILE_NODE_NAME_STR(cfile_node);
    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_node_rmv: file name is null\n");
        return ((UINT32)-1);
    }

    if(0 != remove((char *)file_name_str))
    {
        sys_log(LOGSTDOUT, "error:cfile_node_rmv: failed to rmv node %s with errno = %d, errstr = %s\n", file_name_str, errno, strerror(errno));
        return ((UINT32)-1);
    }
    return (0);
}

/**
*
* clone src CFILE_NODE to des CFILE_NODE
* 1, clone node name
* 2, clone node TCID vector
* 3, clone node size
*
* warning:
*  1, here not clone segs of node, which means two CFILE_NODE will own same segs
*     the issue is when user remove some one node with cfile_frmv_on_node_tcid or cfile_rmv_on_node_tcid, the other node
*     will be left with segs info but without seg data files
*  2, hence, call this function if and only if user knows his purpose clearly
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_node_clone(const UINT32 cfile_md_id, const CFILE_NODE *src_cfile_node, CFILE_NODE *des_cfile_node)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_rmv: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cstring_clone(CFILE_NODE_NAME(src_cfile_node), CFILE_NODE_NAME(des_cfile_node));

    cvector_clone(CFILE_NODE_TCID_VEC(src_cfile_node), CFILE_NODE_TCID_VEC(des_cfile_node), NULL_PTR, NULL_PTR);
    CFILE_NODE_SIZE(des_cfile_node) = CFILE_NODE_SIZE(src_cfile_node);

    //cfile_seg_vec_clone(cfile_md_id, CFILE_SEG_VEC(src_cfile_node), CFILE_SEG_VEC(des_cfile_node));

    return (0);
}

/**
*
* compare two CFILE_NODE
* 1, compare node TCID vector size and tcid list
* 2, compare node size
* 3, compare node segs without seg name comparing
*
* note:
*  1, here not compare node name
*  2, here not compare node seg names
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_node_cmp(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node_1st, const CFILE_NODE *cfile_node_2nd)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_cmp: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/
/*
    if(EC_FALSE == cstring_cmp(CFILE_NODE_NAME(cfile_node_1st), CFILE_NODE_NAME(cfile_node_2nd)))
    {
        return (EC_FALSE);
    }
*/
    /*warning: when tcid disorder in SEG_TCID_VEC, the compare result may be wrong, because these tcid should be set but not vector:)*/
    if(EC_FALSE == cvector_cmp(CFILE_NODE_TCID_VEC(cfile_node_1st), CFILE_NODE_TCID_VEC(cfile_node_2nd), NULL_PTR))
    {
        return (EC_FALSE);
    }

    if(CFILE_NODE_SIZE(cfile_node_1st) != CFILE_NODE_SIZE(cfile_node_2nd))
    {
        return (EC_FALSE);
    }

    if(EC_FALSE == cfile_seg_vec_cmp(cfile_md_id, CFILE_SEG_VEC(cfile_node_1st), CFILE_SEG_VEC(cfile_node_2nd)))
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/**
*
* check CFILE_NODE existing/readable/writable/executable or their bit 'and'('&') combination
* 1, if check passed, return EC_TRUE, otherwise return EC_FALSE
*
* note:
*   CFILE_MASK_EXIST: check existing
*   CFILE_MASK_RABLE: check readable
*   CFILE_MASK_WABLE: check writable
*   CFILE_MASK_XABLE: check executable
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_node_fcheck(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 mask)
{
    CFILE_MD *cfile_md;
    MOD_MGR  *mod_mgr;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_node_fcheck: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    if(mask & CFILE_MASK_EXIST)
    {
        if(EC_FALSE == cfile_node_fexist(cfile_md_id, cfile_node))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_fcheck: node of file %s does not exist\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
            return (EC_FALSE);
        }

        if(EC_FALSE == cfile_seg_vec_fexist(cfile_md_id, CFILE_SEG_VEC(cfile_node)))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_fcheck: some segment(s) of file %s does not exist\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
            return (EC_FALSE);
        }
    }

    if(mask & CFILE_MASK_RABLE)
    {
        if(EC_FALSE == cfile_node_frable(cfile_md_id, cfile_node))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_fcheck: node of file %s is not readable\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
            return (EC_FALSE);
        }

        if(EC_FALSE == cfile_seg_vec_frable(cfile_md_id, CFILE_SEG_VEC(cfile_node)))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_fcheck: some segment of file %s is not readable\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
            return (EC_FALSE);
        }
    }

    if(mask & CFILE_MASK_WABLE)
    {
        if(EC_FALSE == cfile_node_fwable(cfile_md_id, cfile_node))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_fcheck: node of file %s is not writable\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
            return (EC_FALSE);
        }

        if(EC_FALSE == cfile_seg_vec_fwable(cfile_md_id, CFILE_SEG_VEC(cfile_node)))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_fcheck: some segment of file %s is not writable\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
            return (EC_FALSE);
        }
    }

    if(mask & CFILE_MASK_XABLE)
    {
        if(EC_FALSE == cfile_node_fxable(cfile_md_id, cfile_node))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_fcheck: node of file %s is not executable\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
            return (EC_FALSE);
        }

        if(EC_FALSE == cfile_seg_vec_fxable(cfile_md_id, CFILE_SEG_VEC(cfile_node)))
        {
            sys_log(LOGSTDOUT, "error:cfile_node_fcheck: some segment of file %s is not executable\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
            return (EC_FALSE);
        }
    }

    sys_log(LOGSTDOUT, "info:cfile_node_fcheck: file %s check mask %lx successfully on tcid %s\n",
                        (char *)CFILE_NODE_NAME_STR(cfile_node), mask, c_word_to_ipv4(MOD_MGR_LOCAL_MOD_TCID(mod_mgr)));
    return (EC_TRUE);
}

/*---------------------------------- CFILE_SEG file operation interface ----------------------------------*/
/**
*
* set CFILE_SEG open MODE
*
* open MODE take one of values,
*
* CFILE_R_OPEN_MODE   : ascii read only
* CFILE_RB_OPEN_MODE  : binary read only
* CFILE_W_OPEN_MODE   : asicii write only
* CFILE_WB_OPEN_MODE  : binary write only
* CFILE_A_OPEN_MODE   : ascii append only  (NOT SUPPORT yet)
* CFILE_AB_OPEN_MODE  : binary append only (NOT SUPPORT yet)
* CFILE_ERR_OPEN_MODE : invalid mode
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_open_mode_set(const UINT32 cfile_md_id, const UINT32 open_mode, CFILE_SEG *cfile_seg)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_open_mode_set: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(NULL_PTR != cfile_seg)
    {
        CFILE_SEG_OPEN_MODE(cfile_seg) = open_mode;
    }

    return (0);
}

/**
*
* create a seg data file if not existing
* 1, if seg data file exist, then give up creating
* 2, if seg data file not exist, then create one empty data file
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_fcreate(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg)
{
    FILE *fp;

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 *file_name_str;
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 file_name_str[256];
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    CSTRING *dir_name;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fcreate: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/
#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    file_name_str = CFILE_SEG_NAME_STR(cfile_seg);
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    if(1)
    {
        CFILE_MD *cfile_md;
        MOD_MGR  *mod_mgr;

        cfile_md = CFILE_MD_GET(cfile_md_id);
        mod_mgr = cfile_md->mod_mgr;

        snprintf((char *)file_name_str,
                 sizeof(file_name_str)/sizeof(file_name_str[0]),
                 "%s.tcid_%s",
                 (char *)CFILE_SEG_NAME_STR(cfile_seg),
                 MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
     }
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fcreate: file name is null\n");
        return ((UINT32)-1);
    }

    /*exist*/
    if(0 == access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fcreate: file %s exist\n", (char *)file_name_str);
        return ((UINT32)-1);
    }

    /*create dir if not exist*/
    dir_name = cstring_new(NULL_PTR, LOC_CFILE_0028);

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    cfile_dirname(cfile_md_id, CFILE_SEG_NAME(cfile_seg), dir_name);
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    if(1)
    {
        CSTRING *file_name;

        file_name = cstring_new(file_name_str, LOC_CFILE_0029);
        cfile_dirname(cfile_md_id, file_name, dir_name);
        cstring_free(file_name);
    }
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    if(0 != access((char *)cstring_get_str(dir_name), F_OK))
    {
        sys_log(LOGSTDOUT, "info:cfile_seg_fcreate: create folder %s\n", (char *)cstring_get_str(dir_name));
        mkdir((char *)cstring_get_str(dir_name), 0700);/*default mode = 0600*/
    }
    cstring_free(dir_name);

    fp = fopen((char *)file_name_str, "wb");
    if(NULL_PTR == fp)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fcreate: failed to create file %s\n", (char *)file_name_str);
        return ((UINT32)-1);
    }
#if 0
    if(0 != truncate((char *)file_name_str, CFILE_SEG_SIZE(cfile_seg)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fcreate: failed to truncate file %s to %ld bytes\n", (char *)file_name_str, CFILE_SEG_SIZE(cfile_seg));
        fclose(fp);
        return ((UINT32)-1);
    }
#endif
    fclose(fp);
    return (0);
}

/**
*
* oepn a seg data file with specific open mode and return file handler
*
* open MODE take one of values,
*
* CFILE_R_OPEN_MODE   : ascii read only
* CFILE_RB_OPEN_MODE  : binary read only
* CFILE_W_OPEN_MODE   : asicii write only
* CFILE_WB_OPEN_MODE  : binary write only
* CFILE_A_OPEN_MODE   : ascii append only  (NOT SUPPORT yet)
* CFILE_AB_OPEN_MODE  : binary append only (NOT SUPPORT yet)
* CFILE_ERR_OPEN_MODE : invalid mode
*
* note:
*   warning: when call fopen to open a file in a thread, the FILE opinter may be same as that in another thread in the same process,
*   and the FILE pointer is only meaningful in the thread, so that user cannot save FILE pointer info to transfer
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
static FILE * cfile_seg_fopen(const UINT32 cfile_md_id, const UINT32 mode, const CFILE_SEG *cfile_seg)
{
    FILE *fp;

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 *file_name_str;
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 file_name_str[256];
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fopen: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    file_name_str = CFILE_SEG_NAME_STR(cfile_seg);
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    if(1)
    {
        CFILE_MD *cfile_md;
        MOD_MGR  *mod_mgr;

        cfile_md = CFILE_MD_GET(cfile_md_id);
        mod_mgr = cfile_md->mod_mgr;

        snprintf((char *)file_name_str,
                 sizeof(file_name_str)/sizeof(file_name_str[0]),
                 "%s.tcid_%s",
                 (char *)CFILE_SEG_NAME_STR(cfile_seg),
                 MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
     }
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    //sys_log(LOGSTDOUT, "info:cfile_seg_fopen: try to open file %s as mode %ld\n", (char *)CFILE_SEG_NAME_STR(cfile_seg), mode);

    switch(mode)
    {
        case CFILE_R_OPEN_MODE:
            fp = fopen((char *)file_name_str, "r");
            break;
        case CFILE_RB_OPEN_MODE:
            fp = fopen((char *)file_name_str, "rb");
            break;
        case CFILE_W_OPEN_MODE:
            fp = fopen((char *)file_name_str, "w");
            break;
        case CFILE_WB_OPEN_MODE:
            fp = fopen((char *)file_name_str, "wb");
            break;
        case CFILE_A_OPEN_MODE:
            fp = fopen((char *)file_name_str, "w+");
            break;
        case CFILE_AB_OPEN_MODE:
            fp = fopen((char *)file_name_str, "wb+");
            break;
        default:
            fp = NULL_PTR;
            sys_log(LOGSTDOUT, "error:cfile_seg_fopen: unknown open mode %ld of file %s\n", mode, (char *)file_name_str);
            return (NULL_PTR);
    }

    //sys_log(LOGSTDOUT, "info:cfile_seg_fopen: try to open file %s as mode %ld, fp %lx\n", (char *)CFILE_SEG_NAME_STR(cfile_seg), mode, fp);

    return (fp);
}

/**
*
* close a seg data file file handler
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
static UINT32 cfile_seg_fclose(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, FILE *fp)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fclose: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(NULL_PTR != fp)
    {
        fclose(fp);
    }
    return (0);
}

/**
*
* check seg data file exist or not
*
* 1, if seg data file exist, return EC_TRUE, otherwise return EC_FALSE
*
* note:
*  1, the seg data file name must have been set in CFILE_SEG
*  2, check CFILE_SEG existing does not need open CFILE_SEG before
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_fexist(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg)
{
#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 *file_name_str;
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 file_name_str[256];
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fexist: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    file_name_str = CFILE_SEG_NAME_STR(cfile_seg);
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    if(1)
    {
        CFILE_MD *cfile_md;
        MOD_MGR  *mod_mgr;

        cfile_md = CFILE_MD_GET(cfile_md_id);
        mod_mgr = cfile_md->mod_mgr;

        snprintf((char *)file_name_str,
                 sizeof(file_name_str)/sizeof(file_name_str[0]),
                 "%s.tcid_%s",
                 CFILE_SEG_NAME_STR(cfile_seg),
                 MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
     }
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fexist: seg name is null\n");
        return (EC_FALSE);
    }

    /*not exist*/
    if(0 != access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fexist: seg %s not exist on tcid %s\n", (char *)file_name_str, CFILE_LOCAL_TCID_STR(cfile_md_id));
        return (EC_FALSE);
    }
    sys_log(LOGSTDOUT, "info:cfile_seg_fexist: seg %s exist on tcid %s\n", (char *)CFILE_SEG_NAME_STR(cfile_seg), CFILE_LOCAL_TCID_STR(cfile_md_id));
    return (EC_TRUE);
}

/**
*
* check data file readable or not
* 1, if seg data file not exist, return EC_FALSE
* 2, if seg data file is readable, return EC_TRUE, otherwise return EC_FALSE
*
* note:
*  1, the seg data file name must have been set in CFILE_SEG
*  2, check CFILE_SEG readable does not need open CFILE_SEG before
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_frable(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg)
{
#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 *file_name_str;
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 file_name_str[256];
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_frable: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    file_name_str = CFILE_SEG_NAME_STR(cfile_seg);
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    if(1)
    {
        CFILE_MD *cfile_md;
        MOD_MGR  *mod_mgr;

        cfile_md = CFILE_MD_GET(cfile_md_id);
        mod_mgr = cfile_md->mod_mgr;

        snprintf((char *)file_name_str,
                 sizeof(file_name_str)/sizeof(file_name_str[0]),
                 "%s.tcid_%s",
                 CFILE_SEG_NAME_STR(cfile_seg),
                 MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
     }
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_frable: seg name is null\n");
        return (EC_FALSE);
    }

    /*not exist*/
    if(0 != access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_frable: seg %s not exist on tcid %s\n", (char *)file_name_str, CFILE_LOCAL_TCID_STR(cfile_md_id));
        return (EC_FALSE);
    }

    /*not readable*/
    if(0 != access((char *)file_name_str, R_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_frable: seg %s not readable on tcid %s\n", (char *)file_name_str, CFILE_LOCAL_TCID_STR(cfile_md_id));
        return (EC_FALSE);
    }
    sys_log(LOGSTDOUT, "info:cfile_seg_frable: seg %s is readable on tcid %s\n", (char *)file_name_str, CFILE_LOCAL_TCID_STR(cfile_md_id));
    return (EC_TRUE);
}

/**
*
* check seg data file writable or not
* 1, if seg data file not exist, return EC_FALSE
* 2, if seg data file is writable, return EC_TRUE, otherwise return EC_FALSE
*
* note:
*  1, the seg data file name must have been set in CFILE_SEG
*  2, check CFILE_SEG writable does not need open CFILE_SEG before
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_fwable(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg)
{
#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 *file_name_str;
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 file_name_str[256];
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fwable: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    file_name_str = CFILE_SEG_NAME_STR(cfile_seg);
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    if(1)
    {
        CFILE_MD *cfile_md;
        MOD_MGR  *mod_mgr;

        cfile_md = CFILE_MD_GET(cfile_md_id);
        mod_mgr = cfile_md->mod_mgr;

        snprintf((char *)file_name_str,
                 sizeof(file_name_str)/sizeof(file_name_str[0]),
                 "%s.tcid_%s",
                 CFILE_SEG_NAME_STR(cfile_seg),
                 MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
     }
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fwable: file name is null\n");
        return ((UINT32)-1);
    }

    /*not exist*/
    if(0 != access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fwable: file %s not exist\n", (char *)file_name_str);
        return (EC_TRUE);/*warning: maybe here we should further check whethere dir is writable*/
    }

    /*exist but not writable*/
    if(0 != access((char *)file_name_str, W_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fwable: file %s exist but not writable\n", (char *)file_name_str);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

/**
*
* check seg data file executable or not
* 1, if seg data file not exist, return EC_FALSE
* 2, if seg data file is executable, return EC_TRUE, otherwise return EC_FALSE
*
* note:
*  1, the seg data file name must have been set in CFILE_SEG
*  2, check CFILE_SEG executable does not need open CFILE_SEG before
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_fxable(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg)
{
#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 *file_name_str;
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 file_name_str[256];
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fxable: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    file_name_str = CFILE_SEG_NAME_STR(cfile_seg);
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    if(1)
    {
        CFILE_MD *cfile_md;
        MOD_MGR  *mod_mgr;

        cfile_md = CFILE_MD_GET(cfile_md_id);
        mod_mgr = cfile_md->mod_mgr;

        snprintf((char *)file_name_str,
                 sizeof(file_name_str)/sizeof(file_name_str[0]),
                 "%s.tcid_%s",
                 CFILE_SEG_NAME_STR(cfile_seg),
                 MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
     }
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    if(NULL_PTR == file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fxable: file name is null\n");
        return ((UINT32)-1);
    }

    /*not exist*/
    if(0 != access((char *)file_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fxable: file %s not exist\n", (char *)file_name_str);
        return (EC_FALSE);
    }

    /*not executable*/
    if(0 != access((char *)file_name_str, X_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fxable: file %s exist but not executable\n", (char *)file_name_str);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/**
*
* read seg data file to KBUFF
* 1, open seg data file
* 2, read seg data file to KBUFF
* 3, close seg data file
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_fread(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, KBUFF *kbuff)
{
    FILE *fp;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fread: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    fp = cfile_seg_fopen(cfile_md_id, CFILE_SEG_OPEN_MODE(cfile_seg), cfile_seg);
    if(NULL_PTR == fp)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fread: file %s does not open to read\n", (char *)CFILE_SEG_NAME_STR(cfile_seg));
        return ((UINT32)-1);
    }

    if(EC_FALSE == kbuff_fread(kbuff, CFILE_SEG_SIZE(cfile_seg), fp))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fread: failed to read file %s\n", (char *)CFILE_SEG_NAME_STR(cfile_seg));
        return ((UINT32)-1);
    }

    /*check consistency*/
    if(CFILE_SEG_SIZE(cfile_seg) != KBUFF_CUR_LEN(kbuff))
    {
        sys_log(LOGSTDOUT, "warn:cfile_seg_fread: mismatched file segment size %ld and kbuff cur len %ld\n", CFILE_SEG_SIZE(cfile_seg), KBUFF_CUR_LEN(kbuff));
        return ((UINT32)-1);
    }

    cfile_seg_fclose(cfile_md_id, cfile_seg, fp);

    return (0);
}

/**
*
* write KBUFF to seg data file
* 1, open seg data file
* 2, write KBUFF to seg data file
* 3, close seg data file
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_fwrite(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, const KBUFF *kbuff)
{
    FILE *fp;
    UINT32 pos;

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 *file_name_str;
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 file_name_str[256];
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fwrite: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    file_name_str = CFILE_SEG_NAME_STR(cfile_seg);
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    if(1)
    {
        CFILE_MD *cfile_md;
        MOD_MGR  *mod_mgr;

        cfile_md = CFILE_MD_GET(cfile_md_id);
        mod_mgr = cfile_md->mod_mgr;

        snprintf((char *)file_name_str,
                 sizeof(file_name_str)/sizeof(file_name_str[0]),
                 "%s.tcid_%s",
                 CFILE_SEG_NAME_STR(cfile_seg),
                 MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
     }
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    if(NULL_PTR == (char *)file_name_str)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fwrite: file name is empty\n");
        return ((UINT32)-1);
    }

    fp = cfile_seg_fopen(cfile_md_id, CFILE_SEG_OPEN_MODE(cfile_seg), cfile_seg);
    if(NULL_PTR == fp)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fwrite: file %s does not open to write\n", (char *)file_name_str);
        return ((UINT32)-1);
    }

    pos = 0;
    if(EC_FALSE == kbuff_fwrite(kbuff, fp, &pos))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fwrite: failed to write file %s\n", (char *)file_name_str);
        return ((UINT32)-1);
    }

    if(pos < KBUFF_CUR_LEN(kbuff))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fwrite: only once write %ld bytes of total %ld bytes into file %s\n",
                            pos, KBUFF_CUR_LEN(kbuff),
                            (char *)file_name_str);
        /*call cfile_seg_fappend here ??*/
        return ((UINT32)-1);
    }

    cfile_seg_fclose(cfile_md_id, cfile_seg, fp);

    return (0);
}

/**
*
* append KBUFF from specific starting point to seg data file
* 1, open seg data file
* 2, write KBUFF from specific starting point to seg data file
* 3, close seg data file
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_fappend(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg, const KBUFF *kbuff, UINT32 *pos)
{
    FILE *fp;
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fappend: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    fp = cfile_seg_fopen(cfile_md_id, CFILE_WB_OPEN_MODE, cfile_seg);
    if(NULL_PTR == fp)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fappend: file %s does not open to append\n", (char *)CFILE_SEG_NAME_STR(cfile_seg));
        return ((UINT32)-1);
    }

    if(EC_FALSE == kbuff_fwrite(kbuff, fp, pos))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fappend: failed to write file %s\n", (char *)CFILE_SEG_NAME_STR(cfile_seg));
        return ((UINT32)-1);
    }

    /*update CFILE_SEG*/
    CFILE_SEG_SIZE(cfile_seg) = KBUFF_CUR_LEN(kbuff);

    cfile_seg_fclose(cfile_md_id, cfile_seg, fp);

    return (0);
}

/**
*
* remove seg data file if existing
* 1, if seg data file not exist, give up removing
* 2, try to remove seg data file
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_rmv(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg)
{
#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 *file_name_str;
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    UINT8 file_name_str[256];
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_rmv: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

#if (SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)
    file_name_str = CFILE_SEG_NAME_STR(cfile_seg);
#endif/*(SWITCH_OFF == CFILE_DEBUG_ONLY_SWITCH)*/

#if (SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)
    if(1)
    {
        CFILE_MD *cfile_md;
        MOD_MGR  *mod_mgr;

        cfile_md = CFILE_MD_GET(cfile_md_id);
        mod_mgr = cfile_md->mod_mgr;

        snprintf((char *)file_name_str,
                 sizeof(file_name_str)/sizeof(file_name_str[0]),
                 "%s.tcid_%s",
                 CFILE_SEG_NAME_STR(cfile_seg),
                 MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
     }
#endif/*(SWITCH_ON == CFILE_DEBUG_ONLY_SWITCH)*/

    if(EC_FALSE == cfile_seg_fexist(cfile_md_id, cfile_seg))
    {
        sys_log(LOGSTDOUT, "warn:cfile_seg_rmv: file %s does not exist\n", (char *)CFILE_SEG_NAME_STR(cfile_seg));
        return (0);
    }

    if(0 != remove((char *)file_name_str))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_rmv: failed to rmv seg %s with errno = %d, errstr = %s\n",
                            (char *)CFILE_SEG_NAME_STR(cfile_seg), errno, strerror(errno));
        return ((UINT32)-1);
    }

    return (0);
}

/**
*
* clone src CFILE_SEG to des CFILE_SEG
* 1, clone seg id
* 2, clone seg sieze
* 1, clone seg name
* 2, clone seg TCID vector
*
* warning:
*  1, here not clone seg data file
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_clone(const UINT32 cfile_md_id, const CFILE_SEG *src_cfile_seg, CFILE_SEG *des_cfile_seg)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_clone: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_seg_clean(cfile_md_id, des_cfile_seg);

    CFILE_SEG_ID(des_cfile_seg)   = CFILE_SEG_ID(src_cfile_seg);
    CFILE_SEG_SIZE(des_cfile_seg) = CFILE_SEG_SIZE(src_cfile_seg);

    cstring_clone(CFILE_SEG_NAME(src_cfile_seg), CFILE_SEG_NAME(des_cfile_seg));

    cvector_clone(CFILE_SEG_TCID_VEC(src_cfile_seg), CFILE_SEG_TCID_VEC(des_cfile_seg), NULL_PTR, NULL_PTR);

    return (0);
}

/**
*
* copy src seg data file to des seg data file
* 1, if src seg data file exist at local, then
*   1.1, read src seg data file to kbuff
*   1.2, write kbuff to des seg data file
* 2, otherwise, searching existing src seg data in CFILE_SEG pool
*   2.1, if not exist in the whole CFILE_SEG pool, then give up copy and return
*   2.2, otherwise, read src seg data file from remote to kbuff
*   2.3, write kbuff to des seg data file
*
* note:
*  1, src seg data may exist at local or remote
*  2, des seg data may be at local
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_copy(const UINT32 cfile_md_id, const CFILE_SEG *src_cfile_seg, const CFILE_SEG *des_cfile_seg)
{
    CFILE_MD *cfile_md;
    MOD_MGR  *mod_mgr;

    KBUFF    *kbuff;
    UINT32    seg_tcid_pos;
    UINT32    ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_copy: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    //CFILE_SEG_ID(des_cfile_seg)   = CFILE_SEG_ID(src_cfile_seg);
    //CFILE_SEG_SIZE(des_cfile_seg) = CFILE_SEG_SIZE(src_cfile_seg);

    //cvector_clone(CFILE_SEG_TCID_VEC(src_cfile_seg), CFILE_SEG_TCID_VEC(des_cfile_seg), NULL_PTR, NULL_PTR);

    /*when src_cfile_seg exist at local*/
    if(EC_TRUE == cfile_seg_fexist(cfile_md_id, src_cfile_seg))
    {
        kbuff = kbuff_new(CFILE_SEG_SIZE(src_cfile_seg));
        cfile_seg_fread(cfile_md_id, src_cfile_seg, kbuff);
        cfile_seg_fwrite(cfile_md_id, des_cfile_seg, kbuff);
        kbuff_free(kbuff);
        return (0);
    }

    seg_tcid_pos = CFILE_SEG_START_TCID_POS;
    if(EC_FALSE == cfile_seg_fexist_ppl(cfile_md_id, src_cfile_seg, &seg_tcid_pos))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_copy: no tcid has src seg\n");
        return ((UINT32)-1);
    }

    kbuff = kbuff_new(0);
    if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, CFILE_SEG_TCID(src_cfile_seg, seg_tcid_pos), &ret, FI_cfile_seg_fread, ERR_MODULE_ID, src_cfile_seg, kbuff))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_copy: something wrong when make task to read seg file %s on tcid %s\n",
                            (char *)CFILE_SEG_NAME_STR(src_cfile_seg), CFILE_SEG_TCID_STR(src_cfile_seg, seg_tcid_pos));
        kbuff_free(kbuff);
        return ((UINT32)-1);
    }

    if(0 != ret)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_copy: failed to read seg file %s on tcid %s\n",
                        (char *)CFILE_SEG_NAME_STR(src_cfile_seg), CFILE_SEG_TCID_STR(src_cfile_seg, seg_tcid_pos));
        kbuff_free(kbuff);
        return ((UINT32)-1);
    }

    cfile_seg_fwrite(cfile_md_id, des_cfile_seg, kbuff);
    kbuff_free(kbuff);
    return (0);
}

/**
*
* compare two CFILE_SEG
* 1, compare seg id
* 2, compare seg TCID vector size and tcid list
* 3, compare seg size
* 4, compare seg name
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_cmp(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg_1st, const CFILE_SEG *cfile_seg_2nd)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_cmp: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(CFILE_SEG_ID(cfile_seg_1st) != CFILE_SEG_ID(cfile_seg_2nd))
    {
        return (EC_FALSE);
    }

    /*warning: when tcid disorder in SEG_TCID_VEC, the compare result may be wrong, because these tcid should be set but not vector:)*/
    if(EC_FALSE == cvector_cmp(CFILE_SEG_TCID_VEC(cfile_seg_1st), CFILE_SEG_TCID_VEC(cfile_seg_1st), NULL_PTR))
    {
        return (EC_FALSE);
    }
    if(CFILE_SEG_TCID(cfile_seg_1st, 0) != CFILE_SEG_TCID(cfile_seg_2nd, 0))
    {
        return (EC_FALSE);
    }
    if(EC_FALSE == cstring_cmp(CFILE_SEG_NAME(cfile_seg_1st), CFILE_SEG_NAME(cfile_seg_2nd)))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

/**
*
* create a seg dir if not existing
* 1, if seg dir exist, then give up creating
* 2, if seg dir not exist, then create one seg dir
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_dir_create(const UINT32 cfile_md_id, const CSTRING *seg_dir_name)
{
    UINT8    *seg_dir_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_dir_create: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(NULL_PTR == seg_dir_name)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_dir_create: seg dir name is null\n");
        return ((UINT32)-1);
    }

    seg_dir_name_str = cstring_get_str(seg_dir_name);

    /*exist*/
    if(0 == access((char *)seg_dir_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "warn:cfile_seg_dir_create: seg dir %s exist\n", (char *)seg_dir_name_str);
        return (0);
    }

    /*seg dir default mode = 0700*/
    if(0 != mkdir((char *)seg_dir_name_str, 0700))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_dir_create: failed to make seg dir %s with errno = %d, errstr = %s\n",
                           (char *)seg_dir_name_str, errno, strerror(errno));
        return ((UINT32)-1);
    }

    return (0);
}

/**
*
* remove seg dir if existing
* 1, if seg dir not exist, give up removing
* 2, try to remove seg dir
*
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_dir_rmv(const UINT32 cfile_md_id, const CSTRING *seg_dir_name)
{
    UINT8    *seg_dir_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_dir_rmv: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(NULL_PTR == seg_dir_name)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_dir_rmv: seg dir name is null\n");
        return ((UINT32)-1);
    }

    seg_dir_name_str = cstring_get_str(seg_dir_name);

    if(0 != access((char *)seg_dir_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "warn:cfile_seg_dir_rmv: seg dir %s not exist\n", (char *)seg_dir_name_str);
        return (0);
    }

    if(0 != rmdir((char *)seg_dir_name_str))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_dir_rmv: failed to rmv seg dir %s with errno = %d, errstr = %s\n",
                            (char *)seg_dir_name_str, errno, strerror(errno));
        return ((UINT32)-1);
    }

    return (0);
}

/**
*
* check seg dir exist or not
*
* 1, if seg dir exist, return EC_TRUE, otherwise return EC_FALSE
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_dir_exist(const UINT32 cfile_md_id, const CSTRING *seg_dir_name)
{
    UINT8    *seg_dir_name_str;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_dir_exist: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(NULL_PTR == seg_dir_name)
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_dir_exist: seg dir name is null\n");
        return (EC_FALSE);
    }

    seg_dir_name_str = cstring_get_str(seg_dir_name);

    /*not exist*/
    if(0 != access((char *)seg_dir_name_str, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_dir_exist: seg dir %s not exist on tcid %s\n", (char *)seg_dir_name_str, CFILE_LOCAL_TCID_STR(cfile_md_id));
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "info:cfile_seg_dir_exist: seg %s exist on tcid %s\n", (char *)seg_dir_name_str, CFILE_LOCAL_TCID_STR(cfile_md_id));
    return (EC_TRUE);
}

/*---------------------------------- CFILE_SEG PIPELINE MODE file operation interface ----------------------------------*/
/**
* About PIPELINE MODE
* ===================
* Assume data table: c0,c1,.....cn, and a function F. when F operate one data ci, it will return TRUE or FALSE, we say F
* found ci is TRUE or FALSE. Some ci may refuse operations of F, if so, should skip this ci.
*
* Consider two scenarios:
* Scenario 1: (searching mode) when F found some ci is TRUE, stop moving forward and return ci and TRUE.
*
*BOOL F(DataTable c, DataSize num, DataPos *pos)
*{
*   ci = c[ (*pos) ];
*   if(TRUE == F(ci))
*   {
*       return TRUE;
*   }
*
*   while((*pos) + 1 < num)
*   {
*       (*pos) ++;
*
*       ci = c[ (*pos) ];
*
*       if(ci refuse operation of F)
*       {
*           continue;
*       }
*
*       return F(ci);
*   }
*
*   return FALSE;
*}
*
* main_caller do:
*   pos = 0;
*   F(c, nm, &pos);
*
*Scenario 2: (ignore mode) when F found some ci is FALSE, skip it and move forward to operate next
*
*BOOL F(DataTable c, DataSize num, DataPos *pos)
*{
*   ci = c[ (*pos) ];
*
*   ret = F(ci);
*   //if(FALSE == ret)
*   //{
*   //    return FALSE;
*   //}
*
*   while((*pos) + 1 < num)
*   {
*       (*pos) ++;
*
*       ci = c[ (*pos) ];
*
*       if(ci refuse operation of F)
*       {
*           continue;
*       }
*
*       ret = F(ci)
*       if(TRUE == ret)
*       {
*           break;
*       }
*   }
*
*   return ret;
*}
*
* main_caller do:
*   pos = 0;
*   F(c, nm, &pos);
*
**/

/**
*
* create seg data file in piple-line mode
*   1, if seg tcid is not matched to local tcid, then report error and return
*   2, otherwise, create a seg data file at local, and
*   3, if has no more seg tcid available,  then return
*   4, otherwise, figure out next seg tcid, and trigger a task to create the same seg data file on that seg tcid
*
* note:
*   if some seg tcid in CFILE_SEG TCID vector is not available, the seg data file creating on that seg tcid will be skipped
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_fcreate_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;

    //UINT32 mod_node_pos;
    UINT32 seg_tcid;

    UINT32 seg_tcid_num;
    UINT32 ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fcreate_ppl: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    seg_tcid_num = CFILE_SEG_TCID_NUM(cfile_seg);
    if((*seg_tcid_pos) >= seg_tcid_num)
    {
        sys_log(LOGSTDOUT, "warn:cfile_seg_fcreate_ppl: seg_tcid_pos = %ld reach max tcid num %ld\n", (*seg_tcid_pos), seg_tcid_num);
        return ((UINT32)-1);
    }

    seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));
    if(seg_tcid != MOD_NODE_TCID(MOD_MGR_LOCAL_MOD(mod_mgr)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fcreate_ppl: seg_tcid_pos %ld has tcid %s but not match to local mod_node tcid %s\n",
                         (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
        return ((UINT32)-1);
    }

    ret = cfile_seg_fcreate(cfile_md_id, cfile_seg);

    /*skip unavailable tcid. the right way should check tcid connectivity and skip unavailable ones*/
    //TODO: check tcid connectivity
    for(; (*seg_tcid_pos) + 1 < seg_tcid_num;)
    {
        (*seg_tcid_pos) ++;
        seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));

        if(0 == task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, seg_tcid, &ret, FI_cfile_seg_fcreate_ppl, ERR_MODULE_ID, cfile_seg, seg_tcid_pos))
        {
            /*PIPELINE MODE, after hand on the data to next datanode, the current task can return,*/
            /*because the next datanode will hand on the data to its next datanode*/
            break;
        }
        sys_log(LOGSTDOUT, "warn:cfile_seg_fcreate_ppl: seg_tcid_pos = %ld, seg_tcid = %s, seg_tcid_num = %ld\n",
                            (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), seg_tcid_num);
    }
    return (ret);
}

/**
*
* check seg data file existing or not in piple-line mode
*   1, if seg tcid is not matched to local tcid, then report error and return
*   2, otherwise, check seg data file at local
*       2.1, if seg data file exist at local, then stop pipe-line and return EC_TRUE
*       2.2, otherwise,
*           2.2.1, if has no more seg tcid available, then EC_FALSE
*           2.2.2, figure out next seg tcid, and trigger a task to check the seg data file existing on that seg tcid
*
* note:
*   if some seg tcid in CFILE_SEG TCID vector is not available, the seg data file will be regarded existing
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_fexist_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;

    UINT32 seg_tcid;

    UINT32 seg_tcid_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fexist_ppl: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    seg_tcid_num = CFILE_SEG_TCID_NUM(cfile_seg);
    if((*seg_tcid_pos) >= seg_tcid_num)
    {
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "info:cfile_seg_fexist_ppl: seg_tcid_pos = %ld, seg_tcid = %s, local tcid = %s, file = %s\n",
                        (*seg_tcid_pos), CFILE_SEG_TCID_STR(cfile_seg, (*seg_tcid_pos)),
                        MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)), (char *)CFILE_SEG_NAME_STR(cfile_seg));

    seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));
    if(seg_tcid != MOD_NODE_TCID(MOD_MGR_LOCAL_MOD(mod_mgr)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fexist_ppl: seg_tcid_pos %ld has tcid %s but not match to local mod_node tcid %s\n",
                         (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
        return (EC_FALSE);
    }

    if(EC_TRUE == cfile_seg_fexist(cfile_md_id, cfile_seg))
    {
        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "warn:cfile_seg_fexist_ppl: tcid %s has not found seg file %s\n",
                        c_word_to_ipv4(seg_tcid), (char *)CFILE_SEG_NAME_STR(cfile_seg));

    /*skip unavailable tcid. the right way should check tcid connectivity and skip unavailable ones*/
    //TODO: check tcid connectivity

    for(; (*seg_tcid_pos) + 1 < seg_tcid_num;)
    {
        EC_BOOL ret;

        (*seg_tcid_pos) ++;
        seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));

        if(0 == task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, seg_tcid, &ret, FI_cfile_seg_fexist_ppl, ERR_MODULE_ID, cfile_seg, seg_tcid_pos))
        {
            return (ret);
        }
        sys_log(LOGSTDOUT, "warn:cfile_seg_fexist_ppl: seg_tcid_pos = %ld, seg_tcid = %s, seg_tcid_num = %ld\n",
                            (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), seg_tcid_num);
    }

    return (EC_FALSE);
}

/**
*
* check seg data file readable or not in piple-line mode
*   1, if seg tcid is not matched to local tcid, then report error and return
*   2, otherwise, check seg data file readable at local
*       2.1, if seg data file is reable at local, then stop pipe-line and return EC_TRUE
*       2.2, otherwise,
*           2.2.1, if has no more seg tcid available, then EC_FALSE
*           2.2.2, figure out next seg tcid, and trigger a task to check the seg data file readable or not on that seg tcid
*
* note:
*   if some seg tcid in CFILE_SEG TCID vector is not available, the seg data file will be regarded NOT readable
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_frable_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;

    UINT32 seg_tcid;

    UINT32 seg_tcid_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_frable_ppl: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    seg_tcid_num = CFILE_SEG_TCID_NUM(cfile_seg);
    if((*seg_tcid_pos) >= seg_tcid_num)
    {
        return (EC_FALSE);
    }

    seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));
    if(seg_tcid != MOD_NODE_TCID(MOD_MGR_LOCAL_MOD(mod_mgr)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_frable_ppl: seg_tcid_pos %ld has tcid %s but not match to local mod_node tcid %s\n",
                         (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
        return (EC_FALSE);
    }

    if(EC_TRUE == cfile_seg_frable(cfile_md_id, cfile_seg))
    {
        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "warn:cfile_seg_frable_ppl: tcid %s has not found seg file %s\n", c_word_to_ipv4(seg_tcid), (char *)CFILE_SEG_NAME_STR(cfile_seg));

    /*skip unavailable tcid. the right way should check tcid connectivity and skip unavailable ones*/
    //TODO: check tcid connectivity

    for(; (*seg_tcid_pos) + 1 < seg_tcid_num;)
    {
        EC_BOOL ret;

        (*seg_tcid_pos) ++;
        seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));

        if(0 == task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, seg_tcid, &ret, FI_cfile_seg_frable_ppl, ERR_MODULE_ID, cfile_seg, seg_tcid_pos))
        {
            return (ret);
        }
        sys_log(LOGSTDOUT, "warn:cfile_seg_frable_ppl: seg_tcid_pos = %ld, seg_tcid = %s, seg_tcid_num = %ld\n",
                            (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), seg_tcid_num);
    }

    return (EC_FALSE);
}

/**
*
* check seg data file writable or not in piple-line mode
*   1, if seg tcid is not matched to local tcid, then report error and return
*   2, otherwise, check seg data file writable at local
*       2.1, if seg data file is writable at local, then stop pipe-line and return EC_TRUE
*       2.2, otherwise,
*           2.2.1, if has no more seg tcid available, then EC_FALSE
*           2.2.2, figure out next seg tcid, and trigger a task to check the seg data file writable or not on that seg tcid
*
* note:
*   if some seg tcid in CFILE_SEG TCID vector is not available, the seg data file will be regarded NOT writable
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_fwable_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;

    UINT32 seg_tcid;

    UINT32 seg_tcid_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fwable_ppl: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    seg_tcid_num = CFILE_SEG_TCID_NUM(cfile_seg);
    if((*seg_tcid_pos) >= seg_tcid_num)
    {
        return (EC_FALSE);
    }

    seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));
    if(seg_tcid != MOD_NODE_TCID(MOD_MGR_LOCAL_MOD(mod_mgr)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fwable_ppl: seg_tcid_pos %ld has tcid %s but not match to local mod_node tcid %s\n",
                         (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
        return (EC_FALSE);
    }

    if(EC_TRUE == cfile_seg_fwable(cfile_md_id, cfile_seg))
    {
        return (EC_TRUE);
    }

    /*skip unavailable tcid. the right way should check tcid connectivity and skip unavailable ones*/
    //TODO: check tcid connectivity
    for(; (*seg_tcid_pos) + 1 < seg_tcid_num;)
    {
        EC_BOOL   ret;

        (*seg_tcid_pos) ++;
        seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));

        if(0 == task_tcid_mono(mod_mgr,TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, seg_tcid, &ret, FI_cfile_seg_fwable_ppl, ERR_MODULE_ID, cfile_seg, seg_tcid_pos))
        {
            return (ret);
        }
        sys_log(LOGSTDOUT, "warn:cfile_seg_fwable_ppl: seg_tcid_pos = %ld, seg_tcid = %s, seg_tcid_num = %ld\n",
                            (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), seg_tcid_num);
    }

    return (EC_FALSE);
}

/**
*
* check seg data file executable or not in piple-line mode
*   1, if seg tcid is not matched to local tcid, then report error and return
*   2, otherwise, check seg data file executable at local
*       2.1, if seg data file is executable at local, then stop pipe-line and return EC_TRUE
*       2.2, otherwise,
*           2.2.1, if has no more seg tcid available, then EC_FALSE
*           2.2.2, figure out next seg tcid, and trigger a task to check the seg data file executable or not on that seg tcid
*
* note:
*   if some seg tcid in CFILE_SEG TCID vector is not available, the seg data file will be regarded NOT executable
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cfile_seg_fxable_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;

    UINT32 seg_tcid;

    UINT32 seg_tcid_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fxable_ppl: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    seg_tcid_num = CFILE_SEG_TCID_NUM(cfile_seg);
    if((*seg_tcid_pos) >= seg_tcid_num)
    {
        return (EC_FALSE);
    }

    seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));
    if(seg_tcid != MOD_NODE_TCID(MOD_MGR_LOCAL_MOD(mod_mgr)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fxable_ppl: seg_tcid_pos %ld has tcid %s but not match to local mod_node tcid %s\n",
                         (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
        return (EC_FALSE);
    }

    if(EC_TRUE == cfile_seg_fxable(cfile_md_id, cfile_seg))
    {
        return (EC_TRUE);
    }

    /*skip unavailable tcid. the right way should check tcid connectivity and skip unavailable ones*/
    //TODO: check tcid connectivity
    for(; (*seg_tcid_pos) + 1 < seg_tcid_num;)
    {
        EC_BOOL   ret;

        (*seg_tcid_pos) ++;
        seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));

        if(0 == task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, seg_tcid, &ret, FI_cfile_seg_fxable_ppl, ERR_MODULE_ID, cfile_seg, seg_tcid_pos))
        {
            return (ret);
        }
        sys_log(LOGSTDOUT, "warn:cfile_seg_fxable_ppl: seg_tcid_pos = %ld, seg_tcid = %s, seg_tcid_num = %ld\n",
                            (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), seg_tcid_num);
    }

    return (EC_FALSE);
}

/**
*
* read seg data file in piple-line mode
*   1, if seg tcid is not matched to local tcid, then report error and return
*   2, otherwise, read seg data file at local
*       2.1, if read successfully, then stop pipe-line and return success
*       2.2, otherwise,
*           2.2.1, if has no more seg tcid available, then failure
*           2.2.2, figure out next seg tcid, and trigger a task to read the seg data file on that seg tcid
*
* note:
*   if some seg tcid in CFILE_SEG TCID vector is not available, the seg data file will be regarded NOT readable and be skipped
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_fread_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, KBUFF *kbuff, UINT32 *seg_tcid_pos)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;

    UINT32 seg_tcid;

    UINT32 seg_tcid_num;

    UINT32 kbuff_cur_len_saved;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fread_ppl: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    seg_tcid_num = CFILE_SEG_TCID_NUM(cfile_seg);
    if((*seg_tcid_pos) >= seg_tcid_num)
    {
        return ((UINT32)-1);
    }

    seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));
    if(seg_tcid != MOD_NODE_TCID(MOD_MGR_LOCAL_MOD(mod_mgr)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fread_ppl: seg_tcid_pos %ld has tcid %s but not match to local mod_node tcid %s\n",
                         (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
        return ((UINT32)-1);
    }

    kbuff_cur_len_saved = KBUFF_CUR_LEN(kbuff);/*save*/

    if(0 == cfile_seg_fread(cfile_md_id, cfile_seg, kbuff))
    {
        return (0);
    }

    KBUFF_CUR_LEN(kbuff) = kbuff_cur_len_saved; /*restore*/

    /*skip unavailable tcid. the right way should check tcid connectivity and skip unavailable ones*/
    //TODO: check tcid connectivity
    for(; (*seg_tcid_pos) + 1 < seg_tcid_num;)
    {
        UINT32 ret;

        (*seg_tcid_pos) ++;
        seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));

        if(0 == task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, seg_tcid, &ret, FI_cfile_seg_fread_ppl, ERR_MODULE_ID, cfile_seg, kbuff, seg_tcid_pos))
        {
            return (ret);
        }
        sys_log(LOGSTDOUT, "warn:cfile_seg_fread_ppl: seg_tcid_pos = %ld, seg_tcid = %s, seg_tcid_num = %ld\n",
                            (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), seg_tcid_num);
    }

    return ((UINT32)-1);
}

/**
*
* write seg data file in piple-line mode
*   1, if seg tcid is not matched to local tcid, then report error and return
*   2, otherwise, write a seg data file at local, and
*   3, if has no more seg tcid available,  then return
*   4, otherwise, figure out next seg tcid, and trigger a task to write the same seg data file on that seg tcid
*
* note:
*   if some seg tcid in CFILE_SEG TCID vector is not available, the seg data file will be regarded NOT writable and be skipped
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_fwrite_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, const KBUFF *kbuff, UINT32 *seg_tcid_pos)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;

    UINT32 seg_tcid;

    UINT32 seg_tcid_num;
    UINT32 ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_fwrite_ppl: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    seg_tcid_num = CFILE_SEG_TCID_NUM(cfile_seg);
    if((*seg_tcid_pos) >= seg_tcid_num)
    {
        return ((UINT32)-1);
    }

    seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));
    if(seg_tcid != MOD_NODE_TCID(MOD_MGR_LOCAL_MOD(mod_mgr)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_fwrite_ppl: seg_tcid_pos %ld has tcid %s but not match to local mod_node tcid %s\n",
                         (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
        return ((UINT32)-1);
    }

    ret = cfile_seg_fwrite(cfile_md_id, cfile_seg, kbuff);

    /*skip unavailable tcid. the right way should check tcid connectivity and skip unavailable ones*/
    //TODO: check tcid connectivity
    for(; (*seg_tcid_pos) + 1 < seg_tcid_num;)
    {
        (*seg_tcid_pos) ++;
        seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));

        if(0 == task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, seg_tcid, &ret, FI_cfile_seg_fwrite_ppl, ERR_MODULE_ID, cfile_seg, kbuff, seg_tcid_pos))
        {
            break;
        }
        sys_log(LOGSTDOUT, "warn:cfile_seg_fwrite_ppl: seg_tcid_pos = %ld, seg_tcid = %s, seg_tcid_num = %ld\n", (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), seg_tcid_num);
    }
    return (ret);
}

/**
*
* remove seg data file in piple-line mode
*   1, if seg tcid is not matched to local tcid, then report error and return
*   2, otherwise, remove a seg data file at local, and
*   3, if has no more seg tcid available,  then return
*   4, otherwise, figure out next seg tcid, and trigger a task to remove the same seg data file on that seg tcid
*
* note:
*   if some seg tcid in CFILE_SEG TCID vector is not available, the seg data file will be regarded NOT removable and be skipped
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_rmv_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;

    UINT32 seg_tcid;

    UINT32 seg_tcid_num;
    UINT32 ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_rmv_ppl: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    seg_tcid_num = CFILE_SEG_TCID_NUM(cfile_seg);
    if((*seg_tcid_pos) >= seg_tcid_num)
    {
        return ((UINT32)-1);
    }

    seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));
    if(seg_tcid != MOD_NODE_TCID(MOD_MGR_LOCAL_MOD(mod_mgr)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_rmv_ppl: seg_tcid_pos %ld has tcid %s but not match to local mod_node tcid %s\n",
                         (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
        return ((UINT32)-1);
    }

    ret = cfile_seg_rmv(cfile_md_id, cfile_seg);

    /*skip unavailable tcid. the right way should check tcid connectivity and skip unavailable ones*/
    //TODO: check tcid connectivity
    for(; (*seg_tcid_pos) + 1 < seg_tcid_num;)
    {
        (*seg_tcid_pos) ++;
        seg_tcid = CFILE_SEG_TCID(cfile_seg, (*seg_tcid_pos));

        if(0 == task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, seg_tcid, &ret, FI_cfile_seg_rmv_ppl, ERR_MODULE_ID, cfile_seg, seg_tcid_pos))
        {
            break;
        }
        sys_log(LOGSTDOUT, "warn:cfile_seg_rmv_ppl: seg_tcid_pos = %ld, seg_tcid = %s, seg_tcid_num = %ld\n",
                        (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), seg_tcid_num);
    }

    return (ret);
}

/**
*
* copy seg data file (and its backups) in piple-line mode
*   1, if seg tcid is not matched to local tcid, then report error and return
*   2, otherwise, copy a seg data file at local, and
*   3, if has no more seg tcid available,  then return
*   4, otherwise, figure out next seg tcid, and trigger a task to copy the backups on that seg tcid
*
* note:
*   1. seg_tcid_pos is for tcid in des_cfile_seg but not src_file_seg
*   2. if some seg tcid in CFILE_SEG TCID vector is not available, the seg data file will be regarded NOT copyable and be skipped
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_copy_ppl(const UINT32 cfile_md_id, const CFILE_SEG *src_cfile_seg, const CFILE_SEG *des_cfile_seg, UINT32 *seg_tcid_pos)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;

    UINT32 seg_tcid;

    UINT32 seg_tcid_num;
    UINT32 ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_copy_ppl: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    seg_tcid_num = CFILE_SEG_TCID_NUM(des_cfile_seg);
    if((*seg_tcid_pos) >= seg_tcid_num)
    {
        return ((UINT32)-1);
    }

    seg_tcid = CFILE_SEG_TCID(des_cfile_seg, (*seg_tcid_pos));
    if(seg_tcid != MOD_NODE_TCID(MOD_MGR_LOCAL_MOD(mod_mgr)))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_copy_ppl: seg_tcid_pos %ld of des_cfile_seg has tcid %s but not match to local mod_node tcid %s\n",
                         (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), MOD_NODE_TCID_STR(MOD_MGR_LOCAL_MOD(mod_mgr)));
        return ((UINT32)-1);
    }

    ret = cfile_seg_copy(cfile_md_id, src_cfile_seg, des_cfile_seg);

    /*skip unavailable tcid. the right way should check tcid connectivity and skip unavailable ones*/
    //TODO: check tcid connectivity
    for(; (*seg_tcid_pos) + 1 < seg_tcid_num;)
    {
        (*seg_tcid_pos) ++;
        seg_tcid = CFILE_SEG_TCID(des_cfile_seg, (*seg_tcid_pos));

        if(0 == task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, seg_tcid, &ret, FI_cfile_seg_copy_ppl, ERR_MODULE_ID, src_cfile_seg, des_cfile_seg, seg_tcid_pos))
        {
            break;
        }
        sys_log(LOGSTDOUT, "warn:cfile_seg_copy_ppl: seg_tcid_pos = %ld of des_cfile_seg, seg_tcid = %s, seg_tcid_num = %ld\n",
                            (*seg_tcid_pos), c_word_to_ipv4(seg_tcid), seg_tcid_num);
    }

    return (ret);
}

/*---------------------------------- CFILE_SEG_VEC file operation interface ----------------------------------*/
/**
*
* read seg data file to its KBUFF of all segs in vector from file handler
*
* note:
* 1, KBUFF of CFILE_SEG should be empty or with max size
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
static UINT32 cfile_seg_vec_read(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const UINT32 cfile_seg_pos_beg, const UINT32 cfile_seg_pos_end, CVECTOR *kbuff_vec, FILE *in_fp)
{
    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_read: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_seg_num = cvector_size(cfile_seg_vec);
    for(cfile_seg_pos = cfile_seg_pos_beg; cfile_seg_pos < cfile_seg_num && cfile_seg_pos < cfile_seg_pos_end; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        KBUFF     *kbuff;
        UINT32     kbuff_cur_len_save;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        kbuff     = (KBUFF *)cvector_get(kbuff_vec, cfile_seg_pos - cfile_seg_pos_beg);

        kbuff_cur_len_save = KBUFF_CUR_LEN(kbuff);

        if(EC_FALSE == kbuff_fread(kbuff, CFILE_SEG_SIZE(cfile_seg), in_fp))
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_read: failed to read to segment #%ld\n", cfile_seg_pos);
            return ((UINT32)-1);
        }

        if(kbuff_cur_len_save == KBUFF_CUR_LEN(kbuff))
        {
            break;/*come to file end*/
        }
    }

    return (0);
}

/**
*
* write all seg KBUFF to file handler
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
static UINT32 cfile_seg_vec_write(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const UINT32 cfile_seg_pos_beg, const UINT32 cfile_seg_pos_end, const CVECTOR *kbuff_vec, FILE *out_fp)
{
    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_write: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_seg_num = cvector_size(cfile_seg_vec);
    for(cfile_seg_pos = cfile_seg_pos_beg; cfile_seg_pos < cfile_seg_num && cfile_seg_pos < cfile_seg_pos_end; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        KBUFF     *kbuff;
        UINT32     pos;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        kbuff     = (KBUFF *)cvector_get(kbuff_vec, cfile_seg_pos - cfile_seg_pos_beg);

        if(0 == KBUFF_CUR_LEN(kbuff))
        {
            break;/*no more data to write*/
        }

        pos = 0;
        if(EC_FALSE == kbuff_fwrite(kbuff, out_fp, &pos) || pos != KBUFF_CUR_LEN(kbuff))
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_write: failed to write to segment #%ld where kbuff cur len = %ld, pos = %ld\n",
                                cfile_seg_pos, KBUFF_CUR_LEN(kbuff), pos);
            return ((UINT32)-1);
        }
    }

    return (0);
}

/**
*
* create all seg data files of seg vector in pipe-line mode
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_seg_vec_fcreate(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;
    UINT32     ret;

    CARRAY *  ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_fcreate: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    cfile_seg_num = cvector_size(cfile_seg_vec);
    ret_list = carray_new(cfile_seg_num, (void *)EC_ERROR, LOC_CFILE_0030);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        void      *ret_addr;
        UINT32     seg_tcid_pos;

        cfile_seg    = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        seg_tcid_pos = CFILE_SEG_START_TCID_POS;
        ret_addr     = (void *)carray_get_addr(ret_list, cfile_seg_pos);

        if(0 != task_tcid_inc(task_mgr, CFILE_SEG_TCID(cfile_seg, seg_tcid_pos), ret_addr, FI_cfile_seg_fcreate_ppl, ERR_MODULE_ID, cfile_seg, &seg_tcid_pos))
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fcreate: cfile_seg # %ld has no tcid or mod_node\n", cfile_seg_pos);
            task_mgr_free(task_mgr);
            return ((UINT32)-1);
        }

        sys_log(LOGSTDOUT, "info:cfile_seg_vec_fcreate: cfile_seg # %ld, seg_tcid_pos = %ld\n", cfile_seg_pos, seg_tcid_pos);
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    ret = 0;

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        UINT32 seg_ret;

        seg_ret = (EC_BOOL)carray_get(ret_list, cfile_seg_pos);
        if(0 != seg_ret)
        {
            CFILE_SEG *cfile_seg;
            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);

            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fcreate: file segment #%ld with file %s does not create\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));
            ret = ((UINT32)-1);
        }
    }

    carray_free(ret_list, LOC_CFILE_0031);
    return (ret);
}

/**
*
* check all seg data files existing of seg vector in pipe-line mode
* if any one seg data file or its backup does not exist, then return EC_FALSE, otherwise, return EC_TRUE
*
* note:
*   if some seg tcid does not available, its checking will be ignored, which means it will be regarded existing.
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_seg_vec_fexist(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

    CARRAY *  ret_list;
    CARRAY *  seg_tcid_pos_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_fexist: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    cfile_seg_num     = cvector_size(cfile_seg_vec);
    ret_list          = carray_new(cfile_seg_num, (void *)EC_FALSE, LOC_CFILE_0032);
    seg_tcid_pos_list = carray_new(cfile_seg_num, (void *)CFILE_SEG_START_TCID_POS, LOC_CFILE_0033);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        void      *ret_addr;
        void      *seg_tcid_pos_addr;
        UINT32     seg_tcid_pos;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        seg_tcid_pos = CFILE_SEG_START_TCID_POS;

        ret_addr = carray_get_addr(ret_list, cfile_seg_pos);
        seg_tcid_pos_addr = carray_get_addr(seg_tcid_pos_list, cfile_seg_pos);

        task_tcid_inc(task_mgr, CFILE_SEG_TCID(cfile_seg, seg_tcid_pos), ret_addr, FI_cfile_seg_fexist_ppl, ERR_MODULE_ID, cfile_seg, seg_tcid_pos_addr);
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    carray_free(seg_tcid_pos_list, LOC_CFILE_0034);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        EC_BOOL seg_ret;

        seg_ret = (EC_BOOL)carray_get(ret_list, cfile_seg_pos);
        if(EC_FALSE == seg_ret)
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fexist: file segment #%ld with file %s does NOT exist\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));

            carray_free(ret_list, LOC_CFILE_0035);
            return (EC_FALSE);
        }
        else/*debug only*/
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "info:cfile_seg_vec_fexist: file segment #%ld with file %s exist on tcid %s\n",
                                cfile_seg_pos,
                                (char *)CFILE_SEG_NAME_STR(cfile_seg),
                                CFILE_SEG_TCID_STR(cfile_seg, (UINT32)(seg_tcid_pos_list->data[ cfile_seg_pos ])));
        }
    }

    carray_free(ret_list, LOC_CFILE_0036);

    sys_log(LOGSTDOUT, "info:cfile_seg_vec_fexist: segs exist on tcid %s\n", MOD_MGR_LOCAL_MOD_TCID_STR(mod_mgr));
    return (EC_TRUE);
}

/**
*
* check all seg data files readable of seg vector in pipe-line mode
* if any one seg data file or its backup is not readable, then return EC_FALSE, otherwise, return EC_TRUE
*
* note:
*   if some seg tcid does not available, its checking will be ignored, which means it will be regarded readable.
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_seg_vec_frable(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

    CARRAY *   ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_frable: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    cfile_seg_num = cvector_size(cfile_seg_vec);
    ret_list = carray_new(cfile_seg_num, (void *)EC_FALSE, LOC_CFILE_0037);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        void      *ret_addr;
        UINT32     seg_tcid_pos;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        ret_addr  = (void *)carray_get_addr(ret_list, cfile_seg_pos);
        seg_tcid_pos = CFILE_SEG_START_TCID_POS;


        task_tcid_inc(task_mgr, CFILE_SEG_TCID(cfile_seg, seg_tcid_pos), ret_addr, FI_cfile_seg_frable_ppl, ERR_MODULE_ID, cfile_seg, &seg_tcid_pos);
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        EC_BOOL seg_ret;

        seg_ret = (EC_BOOL)(ret_list->data[ cfile_seg_pos ]);
        if(EC_TRUE == seg_ret)
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_frable: file segment #%ld with file %s does not exist\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));

            carray_free(ret_list, LOC_CFILE_0038);
            return (EC_FALSE);
        }
        else/*debug only*/
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "info:cfile_seg_vec_frable: file segment #%ld with file %s exist\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));
        }
    }

    carray_free(ret_list, LOC_CFILE_0039);
    return (EC_TRUE);
}

/**
*
* check all seg data files writable of seg vector in pipe-line mode
* if any one seg data file or its backup is not writable, then return EC_FALSE, otherwise, return EC_TRUE
*
* note:
*   if some seg tcid does not available, its checking will be ignored, which means it will be regarded writable.
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_seg_vec_fwable(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

    CARRAY *  ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_fwable: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    cfile_seg_num = cvector_size(cfile_seg_vec);
    ret_list = carray_new(cfile_seg_num, (void *)EC_FALSE, LOC_CFILE_0040);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        void      *ret_addr;
        UINT32     seg_tcid_pos;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        ret_addr  = (void *)carray_get_addr(ret_list, cfile_seg_pos);
        seg_tcid_pos = CFILE_SEG_START_TCID_POS;

        task_tcid_inc(task_mgr, CFILE_SEG_TCID(cfile_seg, seg_tcid_pos), ret_addr, FI_cfile_seg_fwable_ppl, ERR_MODULE_ID, cfile_seg, &seg_tcid_pos);
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        EC_BOOL seg_ret;

        seg_ret = (EC_BOOL)carray_get(ret_list, cfile_seg_pos);
        if(EC_TRUE == seg_ret)
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fwable: file segment #%ld with file %s does not exist\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));

            carray_free(ret_list, LOC_CFILE_0041);
            return (EC_FALSE);
        }
        else/*debug only*/
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "info:cfile_seg_vec_fwable: file segment #%ld with file %s exist\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));
        }
    }

    carray_free(ret_list, LOC_CFILE_0042);
    return (EC_TRUE);
}

/**
*
* check all seg data files executable of seg vector in pipe-line mode
* if any one seg data file or its backup is not executable, then return EC_FALSE, otherwise, return EC_TRUE
*
* note:
*   if some seg tcid does not available, its checking will be ignored, which means it will be regarded executable.
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_seg_vec_fxable(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

    CARRAY *  ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_fxable: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    cfile_seg_num = cvector_size(cfile_seg_vec);
    ret_list = carray_new(cfile_seg_num, (void *)EC_FALSE, LOC_CFILE_0043);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        void      *ret_addr;
        UINT32     seg_tcid_pos;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        ret_addr  = (void *)carray_get_addr(ret_list, cfile_seg_pos);
        seg_tcid_pos = CFILE_SEG_START_TCID_POS;

        task_tcid_inc(task_mgr, CFILE_SEG_TCID(cfile_seg, seg_tcid_pos), ret_addr, FI_cfile_seg_fxable_ppl, ERR_MODULE_ID, cfile_seg, &seg_tcid_pos);
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        EC_BOOL seg_ret;

        seg_ret = (EC_BOOL)carray_get(ret_list, cfile_seg_pos);
        if(EC_TRUE == seg_ret)
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fxable: file segment #%ld with file %s does not exist\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));

            carray_free(ret_list, LOC_CFILE_0044);
            return (EC_FALSE);
        }
        else/*debug only*/
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "info:cfile_seg_vec_fxable: file segment #%ld with file %s exist\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));
        }
    }

    carray_free(ret_list, LOC_CFILE_0045);
    return (EC_TRUE);
}

/**
*
* read all seg data files of seg vector in pipe-line mode
*  1. for each CFILE_SEG, if any one seg data file read successfully, the reading will be regarded successful
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_seg_vec_fread(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const UINT32 cfile_seg_pos_beg, const UINT32 cfile_seg_pos_end, CVECTOR *kbuff_vec)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

    CARRAY *  ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_fread: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    cfile_seg_num = cvector_size(cfile_seg_vec);
    ret_list = carray_new(cfile_seg_num, (void *)EC_FALSE, LOC_CFILE_0046);

    for(cfile_seg_pos = cfile_seg_pos_beg; cfile_seg_pos < cfile_seg_pos_end && cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        KBUFF     *kbuff;
        void      *ret_addr;
        UINT32     seg_tcid_pos;

        cfile_seg    = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        kbuff        = (KBUFF *)cvector_get(kbuff_vec, cfile_seg_pos - cfile_seg_pos_beg);
        seg_tcid_pos = CFILE_SEG_START_TCID_POS;

        ret_addr = (void *)carray_get_addr(ret_list, cfile_seg_pos);

        if(0 != task_tcid_inc(task_mgr, CFILE_SEG_TCID(cfile_seg, seg_tcid_pos), ret_addr, FI_cfile_seg_fread_ppl, ERR_MODULE_ID, cfile_seg, kbuff, &seg_tcid_pos))
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fread: cfile_seg # %ld has no tcid or mod_node\n", cfile_seg_pos);
            task_mgr_free(task_mgr);
            carray_free(ret_list, LOC_CFILE_0047);
            return ((UINT32)-1);
        }
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    for(cfile_seg_pos = cfile_seg_pos_beg; cfile_seg_pos < cfile_seg_pos_end && cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        UINT32 seg_ret;

        seg_ret = (UINT32)carray_get(ret_list, cfile_seg_pos);
        if(0 != seg_ret)
        {
            CFILE_SEG *cfile_seg;

            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fread: file segment #%ld with name %s failed to read\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));

            carray_free(ret_list, LOC_CFILE_0048);
            return ((UINT32)-1);
        }
    }

    carray_free(ret_list, LOC_CFILE_0049);
    return (0);
}

/**
*
* write all seg data files of seg vector in pipe-line mode
*  1. for each CFILE_SEG, if some seg data files write failed but others successfully , the writing will be regarded successful
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_seg_vec_fwrite(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const UINT32 cfile_seg_pos_beg, const UINT32 cfile_seg_pos_end, const CVECTOR *kbuff_vec)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

    UINT32     ret;
    CARRAY *  ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_fwrite: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    cfile_seg_num = cvector_size(cfile_seg_vec);
    ret_list = carray_new(cfile_seg_num, (void *)EC_ERROR, LOC_CFILE_0050);

    for(cfile_seg_pos = cfile_seg_pos_beg; cfile_seg_pos < cfile_seg_pos_end && cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        KBUFF     *kbuff;
        void      *ret_addr;
        UINT32     seg_tcid_pos;

        cfile_seg    = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        kbuff        = (KBUFF *)cvector_get(kbuff_vec, cfile_seg_pos - cfile_seg_pos_beg);
        seg_tcid_pos = CFILE_SEG_START_TCID_POS;

        if(0 == KBUFF_CUR_LEN(kbuff))
        {
            sys_log(LOGSTDOUT, "info:cfile_seg_vec_fwrite: cfile_seg # %ld has empty kbuff\n", cfile_seg_pos);
            carray_set(ret_list, cfile_seg_pos, (void *)0);
            continue;
        }

        ret_addr = (void *)carray_get_addr(ret_list, cfile_seg_pos);

        if(0 != task_tcid_inc(task_mgr, CFILE_SEG_TCID(cfile_seg, seg_tcid_pos), ret_addr, FI_cfile_seg_fwrite_ppl, ERR_MODULE_ID, cfile_seg, kbuff, &seg_tcid_pos))
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fwrite: cfile_seg # %ld has no tcid or mod_node\n", cfile_seg_pos);
            task_mgr_free(task_mgr);
            carray_free(ret_list, LOC_CFILE_0051);
            return ((UINT32)-1);
        }
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    ret = 0;

    for(cfile_seg_pos = cfile_seg_pos_beg; cfile_seg_pos < cfile_seg_pos_end && cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        UINT32 seg_ret;

        seg_ret = (UINT32)carray_get(ret_list, cfile_seg_pos);
        if(0 != seg_ret)
        {
            CFILE_SEG *cfile_seg;
            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fwrite: file segment #%ld with name %s failed to write\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));
            ret = ((UINT32)-1);
        }
    }

    carray_free(ret_list, LOC_CFILE_0052);

    return (ret);
}

/**
*
* INCOMPLETE INTERFACE!
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_seg_vec_fappend(const UINT32 cfile_md_id, CVECTOR *cfile_seg_vec)
{
    sys_log(LOGSTDOUT, "error:cfile_seg_vec_fappend: incomplete implementation!\n");
    return ((UINT32)-1);
#if 0
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

    UINT32     ret;
    CARRAY *  ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_fappend: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, &task_mgr);

    cfile_seg_num = cvector_size(cfile_seg_vec);
    ret_list = carray_new(cfile_seg_num, (void *)EC_ERROR, LOC_CFILE_0053);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        void      *ret_addr;
        UINT32 seg_tcid_pos;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        seg_tcid_pos = CFILE_SEG_START_TCID_POS;

        if(CFILE_SEG_APPEND_POS(cfile_seg) >= KBUFF_CUR_LEN(CFILE_SEG_KBUFF(cfile_seg)))
        {
            carray_set(ret_list, cfile_seg_pos, (void *)0);
            continue;
        }

        ret_addr = (void *)carray_get_addr(ret_list, cfile_seg_pos);


#if 0
        if(0 != task_tcid_inc(task_mgr, CFILE_SEG_TCID(cfile_seg, seg_tcid_pos), &(ret_list->data[ cfile_seg_pos ]), FI_cfile_seg_fappend_ppl, ERR_MODULE_ID, cfile_seg, CFILE_SEG_KBUFF(cfile_seg), &(CFILE_SEG_APPEND_POS(cfile_seg)), &seg_tcid_pos))
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fappend: cfile_seg # %ld has no tcid or mod_node\n", cfile_seg_pos);
            task_mgr_free(task_mgr);
            carray_free(ret_list, LOC_CFILE_0054);
            return ((UINT32)-1);
        }
#endif
    }

    task_wait(task_mgr);

    ret = 0;

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        UINT32 seg_ret;

        seg_ret = (UINT32)carray_get(ret_list, cfile_seg_pos);
        if(0 != seg_ret)
        {
            CFILE_SEG *cfile_seg;
            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_fappend: file segment #%ld with name %s failed to append\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));
            ret = ((UINT32)-1);
        }
    }

    carray_free(ret_list, LOC_CFILE_0055);

    return (ret);
#endif
}

/**
*
* remove all seg data files of seg vector in pipe-line mode
*  1. for each CFILE_SEG, if some seg data files remove failed but others successfully, the removing will be regarded successful
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_seg_vec_rmv(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

    UINT32     ret;
    CARRAY    *ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_rmv: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    ret = 0;

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    cfile_seg_num = cvector_size(cfile_seg_vec);
    ret_list = carray_new(cfile_seg_num, (void *)EC_ERROR, LOC_CFILE_0056);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;
        void      *ret_addr;
        UINT32  seg_tcid_pos;

        cfile_seg    = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        ret_addr     = (void *)carray_get_addr(ret_list, cfile_seg_pos);
        seg_tcid_pos = CFILE_SEG_START_TCID_POS;

        if(0 != task_tcid_inc(task_mgr, CFILE_SEG_TCID(cfile_seg, seg_tcid_pos), ret_addr, FI_cfile_seg_rmv_ppl, ERR_MODULE_ID, cfile_seg, &seg_tcid_pos))
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_rmv: cfile_seg # %ld has no tcid or mod_node\n", cfile_seg_pos);
            task_mgr_free(task_mgr);
            carray_free(ret_list, LOC_CFILE_0057);
            return ((UINT32)-1);
        }
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        UINT32 seg_ret;

        seg_ret = (UINT32)carray_get(ret_list, cfile_seg_pos);
        if(0 != seg_ret)
        {
            CFILE_SEG *cfile_seg;
            cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_rmv: file segment #%ld failed to remove file %s\n", cfile_seg_pos, (char *)CFILE_SEG_NAME_STR(cfile_seg));
            ret = ((UINT32)-1);
        }
    }

    carray_free(ret_list, LOC_CFILE_0058);

    return (ret);
}

static UINT32 cfile_seg_vec_tcid_collect(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, CVECTOR *seg_tcid_vec)
{
    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_tcid_collect: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_seg_num = cvector_size(cfile_seg_vec);
    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        cvector_clone_with_prev_filter(CFILE_SEG_TCID_VEC(cfile_seg), seg_tcid_vec, seg_tcid_vec, (CVECTOR_DATA_PREV_FILTER)cfile_tcid_filter_out, NULL_PTR, NULL_PTR);
    }

    return (0);
}

UINT32 cfile_seg_vec_dir_rmv(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const CSTRING *node_name)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    CVECTOR   *seg_tcid_vec;

    CSTRING   *seg_dir_name;

    UINT32     seg_tcid_pos;
    UINT32     seg_tcid_num;

    UINT32     ret;
    CARRAY    *ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_dir_rmv: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    ret = 0;

    seg_tcid_vec = cvector_new(0, MM_UINT32, LOC_CFILE_0059);
    cfile_seg_vec_tcid_collect(cfile_md_id, cfile_seg_vec, seg_tcid_vec);

    seg_dir_name = cstring_new(NULL_PTR, LOC_CFILE_0060);
    cfile_seg_dir_name_gen(cfile_md_id, node_name, seg_dir_name);

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    seg_tcid_num = cvector_size(seg_tcid_vec);
    ret_list = carray_new(seg_tcid_num, (void *)EC_ERROR, LOC_CFILE_0061);

    for(seg_tcid_pos = 0; seg_tcid_pos < seg_tcid_num; seg_tcid_pos ++)
    {
        void    *ret_addr;
        UINT32   seg_tcid;

        ret_addr = (void *)carray_get_addr(ret_list, seg_tcid_pos);
        seg_tcid = (UINT32)cvector_get(seg_tcid_vec, seg_tcid_pos);

        if(0 != task_tcid_inc(task_mgr, seg_tcid, ret_addr, FI_cfile_seg_dir_rmv, ERR_MODULE_ID, seg_dir_name))
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_dir_rmv: has no tcid %s or mod_node\n", c_word_to_ipv4(seg_tcid));
            task_mgr_free(task_mgr);
            cstring_free(seg_dir_name);
            cvector_free(seg_tcid_vec, LOC_CFILE_0062);
            carray_free(ret_list, LOC_CFILE_0063);
            return ((UINT32)-1);
        }
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    for(seg_tcid_pos = 0; seg_tcid_pos < seg_tcid_num; seg_tcid_pos ++)
    {
        UINT32 seg_ret;

        seg_ret = (UINT32)carray_get(ret_list, seg_tcid_pos);
        if(0 != seg_ret)
        {
            UINT32   seg_tcid;
            seg_tcid = (UINT32)cvector_get(seg_tcid_vec, seg_tcid_pos);
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_dir_rmv: failed to remove seg dir %s on tcid %s\n",
                                (char *)cstring_get_str(seg_dir_name), c_word_to_ipv4(seg_tcid));
            ret = ((UINT32)-1);
        }
    }

    cstring_free(seg_dir_name);
    cvector_free(seg_tcid_vec, LOC_CFILE_0064);
    carray_free(ret_list, LOC_CFILE_0065);

    return (ret);
}

/**
*
* clone all CFILE_SEG in src seg vector to des seg vector
*
* note:
*   no seg data file is cloned
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cfile_seg_vec_clone(const UINT32 cfile_md_id, const CVECTOR *src_cfile_seg_vec, CVECTOR *des_cfile_seg_vec)
{
    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;
    //CARRAY    *ret_list;
    //UINT32     ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_clone: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_seg_vec_clean(cfile_md_id, des_cfile_seg_vec);

    cfile_seg_num = cvector_size(src_cfile_seg_vec);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *src_cfile_seg;
        CFILE_SEG *des_cfile_seg;

        src_cfile_seg = (CFILE_SEG *)cvector_get(src_cfile_seg_vec, cfile_seg_pos);

        des_cfile_seg = cfile_seg_new(cfile_md_id, 0, 0, NULL_PTR);
        if(NULL_PTR == des_cfile_seg)
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_clone: failed to alloc #%ld des seg\n", cfile_seg_pos);
            cfile_seg_vec_clean(cfile_md_id, des_cfile_seg_vec);
            return ((UINT32)-1);
        }

        cfile_seg_clone(cfile_md_id, src_cfile_seg, des_cfile_seg);
        cvector_push(des_cfile_seg_vec, (void *)des_cfile_seg);
    }

    return (0);
}

/**
*
* copy all seg data files of src seg vector to des seg vector in pipe-line mode
*
* note:
*  1. CFILE_SEG info will not be changed in both src seg vector and des vector
*  2. src seg data file name will change to des seg data file name
*  3. after copy, each seg vector owns its seg data files seperatly, so remove one will not impact on the other
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_seg_vec_copy(const UINT32 cfile_md_id, const CVECTOR *src_cfile_seg_vec, const CVECTOR *des_cfile_seg_vec)
{
    CFILE_MD  *cfile_md;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

    UINT32     ret;
    CARRAY    *ret_list;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_copy: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    ret = 0;

    cfile_seg_num = cvector_size(des_cfile_seg_vec);
    if(cfile_seg_num != cvector_size(src_cfile_seg_vec))
    {
        sys_log(LOGSTDOUT, "error:cfile_seg_vec_copy: mismatched seg num where des seg num = %ld and src seg num = %ld\n",
                            cfile_seg_num, cvector_size(src_cfile_seg_vec));
        return ((UINT32)-1);
    }

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    ret_list = carray_new(cfile_seg_num, (void *)EC_ERROR, LOC_CFILE_0066);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *src_cfile_seg;
        CFILE_SEG *des_cfile_seg;
        void      *ret_addr;
        UINT32     seg_tcid_pos;

        src_cfile_seg = (CFILE_SEG *)cvector_get(src_cfile_seg_vec, cfile_seg_pos);
        des_cfile_seg = (CFILE_SEG *)cvector_get(des_cfile_seg_vec, cfile_seg_pos);
        ret_addr      = (void *)carray_get_addr(ret_list, cfile_seg_pos);
        seg_tcid_pos  = CFILE_SEG_START_TCID_POS;

        if(0 != task_tcid_inc(task_mgr, CFILE_SEG_TCID(des_cfile_seg, seg_tcid_pos), ret_addr, FI_cfile_seg_copy_ppl, ERR_MODULE_ID, src_cfile_seg, des_cfile_seg, &seg_tcid_pos))
        {
            sys_log(LOGSTDOUT, "error:cfile_seg_vec_copy: des cfile_seg # %ld has no tcid or mod_node\n", cfile_seg_pos);
            task_mgr_free(task_mgr);
            carray_free(ret_list, LOC_CFILE_0067);
            return ((UINT32)-1);
        }
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        UINT32 seg_ret;

        seg_ret = (UINT32)carray_get(ret_list, cfile_seg_pos);
        if(0 != seg_ret)
        {
            CFILE_SEG *src_cfile_seg;
            CFILE_SEG *des_cfile_seg;

            src_cfile_seg = (CFILE_SEG *)cvector_get(src_cfile_seg_vec, cfile_seg_pos);
            des_cfile_seg = (CFILE_SEG *)cvector_get(des_cfile_seg_vec, cfile_seg_pos);

            sys_log(LOGSTDOUT, "error:cfile_seg_vec_copy: seg #%ld failed to copy from file %s to file %s\n",
                                cfile_seg_pos,
                                (char *)CFILE_SEG_NAME_STR(src_cfile_seg),
                                (char *)CFILE_SEG_NAME_STR(des_cfile_seg));
            ret = ((UINT32)-1);
        }
    }

    carray_free(ret_list, LOC_CFILE_0068);

    return (ret);
}

/**
*
*   compare each CFILE_SEG in two seg vectors
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_seg_vec_cmp(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec_1st, const CVECTOR *cfile_seg_vec_2nd)
{
    UINT32     cfile_seg_pos;
    UINT32     cfile_seg_num;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_vec_cmp: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_seg_num = cvector_size(cfile_seg_vec_1st);
    if(cfile_seg_num != cvector_size(cfile_seg_vec_2nd))
    {
        return (EC_FALSE);
    }

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg_1st;
        CFILE_SEG *cfile_seg_2nd;

        cfile_seg_1st = (CFILE_SEG *)cvector_get(cfile_seg_vec_1st, cfile_seg_pos);
        cfile_seg_2nd = (CFILE_SEG *)cvector_get(cfile_seg_vec_2nd, cfile_seg_pos);

        if(EC_FALSE == cfile_seg_cmp(cfile_md_id, cfile_seg_1st, cfile_seg_2nd))
        {
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

/*-------------------------------------------- external interface --------------------------------------------*/
/**
*
* query num of seg data files of the node
*
* note:
* 1. node must be open before calling
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_seg_num(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_seg_num: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    return cvector_size(CFILE_SEG_VEC(cfile_node));
}

/**
*
* alloc kbuff_num KBUFF and return them in vector
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
CVECTOR * cfile_kbuff_vec_new(const UINT32 cfile_md_id, const UINT32 kbuff_num)
{
    CVECTOR *kbuff_vec;
    UINT32   kbuff_pos;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_kbuff_vec_new: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    kbuff_vec = cvector_new(kbuff_num, MM_KBUFF, LOC_CFILE_0069);
    for(kbuff_pos = 0; kbuff_pos < kbuff_num; kbuff_pos ++)
    {
        KBUFF *kbuff;

        kbuff = kbuff_new(0);
        if(NULL_PTR == kbuff)
        {
            sys_log(LOGSTDOUT, "error:cfile_kbuff_vec_new: failed to alloc kbuff #%ld\n", kbuff_pos);
            cvector_clean(kbuff_vec, (CVECTOR_DATA_CLEANER)kbuff_free, LOC_CFILE_0070);
            cvector_free(kbuff_vec, LOC_CFILE_0071);
            return (NULL_PTR);
        }
        cvector_push(kbuff_vec, (void *)kbuff);
    }
    return (kbuff_vec);
}

/**
*
* clean all KBUFF in vector
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_kbuff_vec_clean(const UINT32 cfile_md_id, CVECTOR *kbuff_vec)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_kbuff_vec_clean: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cvector_clean(kbuff_vec, (CVECTOR_DATA_CLEANER)kbuff_free, LOC_CFILE_0072);
    return (0);
}

/**
*
* reset all KBUFF in vector
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_kbuff_vec_reset(const UINT32 cfile_md_id, CVECTOR *kbuff_vec)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_kbuff_vec_reset: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cvector_loop_front(kbuff_vec, (CVECTOR_DATA_HANDLER)kbuff_reset);
    return (0);
}

/**
*
* free all KBUFF in vector and free vector itself
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cfile_kbuff_vec_free(const UINT32 cfile_md_id, CVECTOR *kbuff_vec)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_kbuff_vec_free: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cvector_clean(kbuff_vec, (CVECTOR_DATA_CLEANER)kbuff_free, LOC_CFILE_0073);
    cvector_free(kbuff_vec, LOC_CFILE_0074);
    return (0);
}

/**
*
* create a node xml file on specific tcid and its seg data files in CFILE_SEG pool
*
* note:
* 1. node size must be initialized before calling
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fcreate_on_node_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, const UINT32 node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    CSTRING    *seg_dir_name;

    UINT32      file_seg_num;
    UINT32      file_seg_idx;
    UINT32      file_seg_size;

    UINT32      mod_node_pos;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fcreate_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr = cfile_md->mod_mgr;

    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        UINT32 ret;
        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cfile_fcreate_on_node_tcid, ERR_MODULE_ID, cfile_node, node_tcid))
        {
            sys_log(LOGSTDOUT, "error:cfile_fcreate_on_node_tcid: something wrong when make task to create node file %s on tcid %s\n",
                                (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(node_tcid));
            return ((UINT32)-1);
        }
        return (ret);
    }

    cfile_node_tcid_push(cfile_md_id, cfile_node, node_tcid);

    seg_dir_name = cstring_new(NULL_PTR, LOC_CFILE_0075);
    cfile_seg_dir_name_gen(cfile_md_id, CFILE_NODE_NAME(cfile_node), seg_dir_name);

    mod_node_pos = 0;

    /*initialize CFILE_SEG_VEC*/
    file_seg_num = (CFILE_NODE_SIZE(cfile_node)+ FILE_SEG_MAX_SIZE - 1) / FILE_SEG_MAX_SIZE;

    for(file_seg_idx = 0; file_seg_idx < file_seg_num; file_seg_idx ++)
    {
        CFILE_SEG *cfile_seg;

        if(file_seg_idx + 1 == file_seg_num)
        {
            file_seg_size = CFILE_NODE_SIZE(cfile_node) % FILE_SEG_MAX_SIZE;
        }
        else
        {
            file_seg_size = FILE_SEG_MAX_SIZE;
        }

        cfile_seg = cfile_seg_new(cfile_md_id, file_seg_idx, file_seg_size, seg_dir_name);
        cfile_seg_tcid_expand(cfile_md_id, cfile_seg);/*expand*/

        CFILE_SEG_OPEN_MODE(cfile_seg) = CFILE_W_OPEN_MODE;

        cvector_push(CFILE_SEG_VEC(cfile_node), (void *)cfile_seg);
    }

    //sys_log(LOGSTDOUT, "info:cfile_fcreate_on_node_tcid: after set segs:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, cfile_node, 0);

    cfile_seg_vec_fcreate(cfile_md_id, CFILE_SEG_VEC(cfile_node));

    cfile_node_fclose(cfile_md_id, cfile_node);

    cstring_free(seg_dir_name);

    return (0);
}

/**
*
* open a node xml file with specific open mode on specific tcid
*
* open MODE take one of values,
*
* CFILE_R_OPEN_MODE   : ascii read only
* CFILE_RB_OPEN_MODE  : binary read only
* CFILE_W_OPEN_MODE   : asicii write only
* CFILE_WB_OPEN_MODE  : binary write only
* CFILE_A_OPEN_MODE   : ascii append only  (NOT SUPPORT yet)
* CFILE_AB_OPEN_MODE  : binary append only (NOT SUPPORT yet)
* CFILE_ERR_OPEN_MODE : invalid mode
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fopen_on_node_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, const UINT32 mode, const UINT32 node_tcid)
{
    CFILE_MD *cfile_md;
    MOD_MGR  *mod_mgr;

    CVECTOR  *cfile_seg_vec;
    UINT32    cfile_seg_num;
    UINT32    cfile_seg_pos;
    UINT32    ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fopen_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    if(node_tcid == MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        if(0 != cfile_node_fopen(cfile_md_id, cfile_node))
        {
            sys_log(LOGSTDOUT, "error:cfile_fopen_on_node_tcid: failed to fopen node of file %s on tcid %s\n",
                                (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(node_tcid));
            return ((UINT32)-1);
        }
    }

    else
    {
        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cfile_node_fopen, ERR_MODULE_ID, cfile_node))
        {
            sys_log(LOGSTDOUT, "error:cfile_fopen_on_node_tcid: something wrong when make task to open node file %s on tcid %s\n",
                                (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(node_tcid));
            return ((UINT32)-1);
        }
    }

    //sys_log(LOGSTDOUT, "info:cfile_fopen_on_node_tcid: cfile_node is\n");
    //cfile_node_xml_print_node(LOGSTDOUT, cfile_node, 0);

    cfile_seg_vec = CFILE_SEG_VEC(cfile_node);
    cfile_seg_num = cvector_size(cfile_seg_vec);
    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG *cfile_seg;

        cfile_seg = (CFILE_SEG *)cvector_get(cfile_seg_vec, cfile_seg_pos);
        CFILE_SEG_OPEN_MODE(cfile_seg) = mode;
    }

    return (0);
}

/**
*
* close a node xml file on specific tcid
*
* note:
*   1. the CFILE_NODE info will override the old node xml file if existing
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fclose_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 node_tcid)
{
    CFILE_MD *cfile_md;
    MOD_MGR  *mod_mgr;

    UINT32 ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fclose_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    if(node_tcid == MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        //cfile_seg_vec_fclose(cfile_md_id, CFILE_SEG_VEC(cfile_node));
        ret = cfile_node_fclose(cfile_md_id, cfile_node);
    }
    else
    {
        task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cfile_node_fclose, ERR_MODULE_ID, cfile_node);
    }
    return (ret);
}

/**
*
* check CFILE_NODE existing/readable/writable/executable or their bit 'and'('&') combination
*   1. if node xml file check failed, then return EC_FALSE
*   2. otherwise, if each seg data file check passed on one tcid at least, then return EC_TRUE, otherwise return EC_FALSE
*
* note:
*   CFILE_MASK_EXIST: check existing
*   CFILE_MASK_RABLE: check readable
*   CFILE_MASK_WABLE: check writable
*   CFILE_MASK_XABLE: check executable
*
* note:
*   1. if some seg data file check failed on any tcid of the CFILE_SEG pool, then the CFILE_NODE checking is regarded failed
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_fcheck_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 mask, const UINT32 node_tcid)
{
    CFILE_MD *cfile_md;
    MOD_MGR  *mod_mgr;
    EC_BOOL   ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fcheck_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    if(node_tcid == MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        ret = cfile_node_fcheck(cfile_md_id, cfile_node, mask);
    }
    else
    {
        task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cfile_node_fcheck, ERR_MODULE_ID, cfile_node, mask);
    }
    return (ret);
}

/**
*
* search node xml file and return one tcid which own it if possible
*
* warn:
*   1. this function implementation has issue because of only one node tcid is supported now, so HOW TO SEARCH is undefined
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_fsearch_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, UINT32 *node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;
    TASK_MGR   *task_mgr;

    CVECTOR    *tcid_list;
    CARRAY     *ret_list;

    UINT32 mod_node_num;
    UINT32 mod_node_idx;

    UINT32 tcid_num;
    UINT32 tcid_pos;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fsearch_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    tcid_list = cvector_new(0, MM_UINT32, LOC_CFILE_0076);

    mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
    {
        UINT32 tcid;

        tcid = MOD_NODE_TCID(MOD_MGR_REMOTE_MOD(mod_mgr, mod_node_idx));
        if(CVECTOR_ERR_POS == cvector_search_front(tcid_list, (void *)tcid, NULL_PTR))
        {
            cvector_push(tcid_list, (void *)tcid);
        }
    }

    tcid_num = cvector_size(tcid_list);
    ret_list = carray_new(tcid_num, (void *)EC_FALSE, LOC_CFILE_0077);

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(tcid_pos = 0; tcid_pos < tcid_num; tcid_pos ++)
    {
        UINT32 tcid;
        void  *ret_addr;

        tcid     = (UINT32)cvector_get(tcid_list, tcid_pos);
        ret_addr = (void *)carray_get_addr(ret_list, tcid_pos);

        task_tcid_inc(task_mgr, tcid, ret_addr, FI_cfile_node_fexist, ERR_MODULE_ID, cfile_node);
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    tcid_pos = carray_search_back(ret_list, (void *)EC_TRUE, NULL_PTR);
    if(tcid_pos < tcid_num)
    {
        (*node_tcid) = (UINT32)cvector_get(tcid_list, tcid_pos);

        sys_log(LOGSTDOUT, "info:cfile_fsearch_on_node_tcid: file %s found on tcid %s\n",
                        (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(*node_tcid));

        carray_free(ret_list, LOC_CFILE_0078);
        cvector_free(tcid_list, LOC_CFILE_0079);
        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "info:cfile_fsearch_on_node_tcid: file %s NOT found\n", (char *)CFILE_NODE_NAME_STR(cfile_node));

    carray_free(ret_list, LOC_CFILE_0080);
    cvector_free(tcid_list, LOC_CFILE_0081);
    return (EC_FALSE);
}

/**
*
* read node on specific tcid
*   1. read node xml file on specific tcid
*   2. read all seg data files into each CFILE_SEG KBUFF
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fread_on_node_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, CVECTOR *kbuff_vec, const UINT32 node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;
    UINT32      ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fread_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cfile_fread_on_node_tcid, ERR_MODULE_ID, cfile_node, node_tcid))
        {
            sys_log(LOGSTDOUT, "error:cfile_fread_on_node_tcid: something wrong when make task with tcid %s and file %s for cfile fread\n",
                                c_word_to_ipv4(node_tcid), (char *)CFILE_NODE_NAME_STR(cfile_node));
            return ((UINT32)-1);
        }
        return (ret);
    }

    if(0 != cfile_node_read(cfile_md_id, cfile_node))
    {
        sys_log(LOGSTDOUT, "error:cfile_fread_on_node_tcid: failed to read node info of file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(node_tcid));
        return ((UINT32)-1);
    }

    return cfile_seg_vec_fread(cfile_md_id, CFILE_SEG_VEC(cfile_node), 0, cvector_size(CFILE_SEG_VEC(cfile_node)), kbuff_vec);
}

/**
*
* write node on specific tcid
*   1. write node xml file with & on specific tcid
*   2. write all seg data files in pipe-line mode
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fwrite_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const CVECTOR *kbuff_vec, const UINT32 node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;
    UINT32      ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fwrite_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cfile_node_write, ERR_MODULE_ID, cfile_node, node_tcid))
        {
            sys_log(LOGSTDOUT, "error:cfile_fwrite_on_node_tcid: something wrong when make task with tcid %s and file %s for node write\n",
                                c_word_to_ipv4(node_tcid), (char *)CFILE_NODE_NAME_STR(cfile_node));
            return ((UINT32)-1);
        }
    }
    else
    {
        ret = cfile_node_write_with_node_tcid(cfile_md_id, cfile_node, node_tcid);
    }

    if(0 != ret)
    {
        sys_log(LOGSTDOUT, "error:cfile_fwrite_on_node_tcid: failed to write node info with file %s and tcid %s to tcid %s\n",
                           (char *)CFILE_NODE_NAME_STR(cfile_node), CFILE_NODE_TCID_STR(cfile_node, CFILE_NODE_TCID_POS), c_word_to_ipv4(node_tcid));
        return ((UINT32)-1);
    }

    return cfile_seg_vec_fwrite(cfile_md_id, CFILE_SEG_VEC(cfile_node), 0, cvector_size(CFILE_SEG_VEC(cfile_node)), kbuff_vec);
}

/**
*
* remove node on specific tcid
*   1. remove all seg data files and their folders
*   2. remove node xml file on specific tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_frmv_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    UINT32      ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_frmv_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    cfile_seg_vec_rmv(cfile_md_id, CFILE_SEG_VEC(cfile_node));
    cfile_seg_vec_dir_rmv(cfile_md_id, CFILE_SEG_VEC(cfile_node), CFILE_NODE_NAME(cfile_node));

    if(node_tcid == MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        ret = cfile_node_rmv(cfile_md_id, cfile_node);
    }
    else
    {
        task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cfile_node_rmv, ERR_MODULE_ID, cfile_node);
    }

    return (ret);
}

/**
*
* clone src CFILE_NODE to des CFILE_NODE with & on specific tcid
*
* warn: inproper implementation !!
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fclone_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *src_cfile_node, CFILE_NODE *des_cfile_node, const UINT32 node_tcid)
{
#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fclone_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_node_tcid_push(cfile_md_id, des_cfile_node, node_tcid);
    cfile_seg_vec_clone(cfile_md_id, CFILE_SEG_VEC(src_cfile_node), CFILE_SEG_VEC(des_cfile_node));

    return (0);
}

/**
*
* copy node on specific tcid
*   1. copy all seg data files in pipe-line mode
*   2. copy node xml file with & on specific tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fcopy_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *src_cfile_node, const CFILE_NODE *des_cfile_node, const UINT32 node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    UINT32      ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fcopy_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    if(0 != cfile_seg_vec_copy(cfile_md_id, CFILE_SEG_VEC(src_cfile_node), CFILE_SEG_VEC(des_cfile_node)))
    {
        sys_log(LOGSTDOUT, "error:cfile_fcopy_on_node_tcid: failed to copy segs\n");
        return ((UINT32)-1);
    }

    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cfile_node_write_with_node_tcid, ERR_MODULE_ID, des_cfile_node, node_tcid))
        {
            sys_log(LOGSTDOUT, "error:cfile_fcopy_on_node_tcid: something wrong when make task with tcid %s and file %s for node write\n",
                                c_word_to_ipv4(node_tcid), (char *)CFILE_NODE_NAME_STR(des_cfile_node));
            return ((UINT32)-1);
        }
    }
    else
    {
        ret = cfile_node_write_with_node_tcid(cfile_md_id, des_cfile_node, node_tcid);
    }

    if(0 != ret)
    {
        sys_log(LOGSTDOUT, "error:cfile_fcopy_on_node_tcid: failed to write node info with file %s and tcid %s to tcid %s\n",
                           (char *)CFILE_NODE_NAME_STR(des_cfile_node), CFILE_NODE_TCID_STR(des_cfile_node, CFILE_NODE_TCID_POS),
                           c_word_to_ipv4(node_tcid));
        return ((UINT32)-1);
    }

    return (0);
}

/**
*
* upload a general file to specific tcid
*   1. create node xml file on specific tcid and its segs  ---- can ignore???
*   2. read the general file to each CFILE_SEG KBUFF of CFILE_NODE
*   3. write node xml file with & on specific tcid, and write all CFILE_SEG KBUFF to each seg data file in pipe-line mode
*
* note:
*   1. the general file should exist in local tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_upload(const UINT32 cfile_md_id, const CSTRING *in_file_name, const CSTRING *node_dir_name, const UINT32 node_tcid)
{
    CFILE_MD   *cfile_md;
    CFILE_NODE *cfile_node;

    CSTRING    *ui_cfile_node_name;
    CSTRING    *in_file_basename;

    UINT32      file_size;
    FILE *      in_fp;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_upload: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);

    in_fp = fopen((char *)cstring_get_str(in_file_name), "rb");/*open in file to read*/
    if(NULL_PTR == in_fp)
    {
        sys_log(LOGSTDOUT, "error:cfile_upload: failed to open file %s to read\n", (char *)cstring_get_str(in_file_name));
        return ((UINT32)-1);
    }

    /*get file size*/
    fseek(in_fp, 0, SEEK_END);
    file_size = ftell(in_fp);
    fseek(in_fp, 0, SEEK_SET);

    //sys_log(LOGSTDOUT, "info:cfile_upload: file %s has size %ld\n", (char *)cstring_get_str(in_file_name), file_size);
    in_file_basename = cstring_new(NULL_PTR, LOC_CFILE_0082);
    cfile_basename(cfile_md_id, in_file_name, in_file_basename);

    ui_cfile_node_name = cstring_new(NULL_PTR, LOC_CFILE_0083);
    cstring_format(ui_cfile_node_name, "%s/%s", (char *)cstring_get_str(node_dir_name), (char *)cstring_get_str(in_file_basename));

    cstring_free(in_file_basename);

    cfile_node = cfile_node_new(cfile_md_id, file_size, ui_cfile_node_name);
    if(NULL_PTR == cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_upload: failed to new cfile node of file %s\n", (char *)cstring_get_str(ui_cfile_node_name));
        cstring_free(ui_cfile_node_name);
        return ((UINT32)-1);
    }
#if 1
    /*create segment files at remote*/
    if(0 != cfile_fcreate_on_node_tcid(cfile_md_id, cfile_node, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_upload: failed to create file %s on tcid %s\n",
                            (char *)cstring_get_str(ui_cfile_node_name), c_word_to_ipv4(node_tcid));
        cstring_free(ui_cfile_node_name);
        cfile_node_free(cfile_md_id, cfile_node);
        return ((UINT32)-1);
    }
#endif
    //sys_log(LOGSTDOUT, "info:cfile_upload: after cfile_fcreate_on_node_tcid, CFILE_NODE_TCID_VEC is:\n");
    //cvector_print(LOGSTDOUT, CFILE_NODE_TCID_VEC(cfile_node), NULL_PTR);


#if 0
    cfile_kbuff_set(cfile_md_id, cfile_node);
    cfile_seg_vec_read(cfile_md_id, CFILE_SEG_VEC(cfile_node), in_fp, 0, cvector_size(CFILE_SEG_VEC(cfile_node)));/*read data from local file to kbuff set*/
    fclose(in_fp);/*close in file*/

    //sys_log(LOGSTDOUT, "info:cfile_upload: after cfile_seg_vec_read:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, cfile_node, 0);

    cfile_fwrite_on_node_tcid(cfile_md_id, cfile_node, node_tcid);/*write data from kbuff to remote segment file*/
#endif

#if 1
    cfile_read_fwrite_group_on_node_tcid(cfile_md_id, cfile_node, in_fp, node_tcid);
#endif

    cstring_free(ui_cfile_node_name);
    cfile_node_free(cfile_md_id, cfile_node);
    return (0);
}

/**
*
* download a file from specific tcid
*   1. search the node xml file on  which tcid  --- oh shit! try to download from specific tcid but why search it!
*   2. open node on specific tcid
*   3. read seg data files to each CFILE_SEG KBUFF of the node
*   3. write all CFILE_SEG KBUFF to the general file
*
* note:
*   1. the general file should exist in local tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_download(const UINT32 cfile_md_id, const CSTRING *in_file_name, const CSTRING *out_file_name, const UINT32 node_tcid)
{
    CFILE_MD   *cfile_md;
    CFILE_NODE *in_cfile_node;

    FILE *      out_fp;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_download: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);

    in_cfile_node = cfile_node_new(cfile_md_id, 0, in_file_name);
    if(NULL_PTR == in_cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_download: failed to new node of in file %s\n", (char *)cstring_get_str(in_file_name));
        return ((UINT32)-1);
    }
#if 0
    if(EC_FALSE == cfile_fsearch_on_node_tcid(cfile_md_id, in_cfile_node, &in_node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_download: not found node of file %s\n", (char *)CFILE_NODE_NAME_STR(in_cfile_node));
        cfile_node_free(cfile_md_id, in_cfile_node);
        return ((UINT32)-1);
    }
#endif
    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, in_cfile_node, CFILE_RB_OPEN_MODE, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_download: failed to fopen node of file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(in_cfile_node), c_word_to_ipv4(node_tcid));
        cfile_node_free(cfile_md_id, in_cfile_node);
        return ((UINT32)-1);
    }

    //sys_log(LOGSTDOUT, "info:cfile_download: after cfile_fopen_on_node_tcid:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, cfile_node, 0);

    out_fp = fopen((char *)cstring_get_str(out_file_name), "wb");/*open in file to write*/
    if(NULL_PTR == out_fp)
    {
        sys_log(LOGSTDOUT, "error:cfile_download: failed to open file %s to write locally\n", (char *)cstring_get_str(out_file_name));
        cfile_node_free(cfile_md_id, in_cfile_node);
        return ((UINT32)-1);
    }
#if 0
    cfile_seg_vec_fread(cfile_md_id, CFILE_SEG_VEC(in_cfile_node), 0, cvector_size(CFILE_SEG_VEC(in_cfile_node)));

    //sys_log(LOGSTDOUT, "info:cfile_download: after cfile_fread_on_node_tcid:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, cfile_node, 0);

    cfile_seg_vec_write(cfile_md_id, CFILE_SEG_VEC(in_cfile_node), out_fp, 0, cvector_size(CFILE_SEG_VEC(in_cfile_node)));
#endif
#if 1
    cfile_fread_write_group_on_local_tcid(cfile_md_id, in_cfile_node, out_fp);
#endif

    fclose(out_fp);

    cfile_node_free(cfile_md_id, in_cfile_node);

    return (0);
}

/**
*
* remove a file on specific tcid
*   1. open node on specific tcid
*   2. remove all seg data files in pipe-line mode and remove the node xml file on specific tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_rmv_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *file_name, const UINT32 node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    CFILE_NODE *cfile_node;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_rmv_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    cfile_node = cfile_node_new(cfile_md_id, 0, file_name);
    if(NULL_PTR == cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_rmv_on_node_tcid: failed to new node of file %s\n", (char *)cstring_get_str(file_name));
        return ((UINT32)-1);
    }

    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, cfile_node, CFILE_ERR_OPEN_MODE, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_rmv_on_node_tcid: failed to open file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(node_tcid));
        cfile_node_free(cfile_md_id, cfile_node);
        return ((UINT32)-1);
    }

    if(0 != cfile_frmv_on_node_tcid(cfile_md_id, cfile_node, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_rmv_on_node_tcid: failed to frmv file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(node_tcid));
        cfile_node_free(cfile_md_id, cfile_node);
        return ((UINT32)-1);
    }

    cfile_node_free(cfile_md_id, cfile_node);

    return (0);
}

/**
*
* clone a file to specific tcid
*
* warn: inproper implementation !!
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_clone_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *src_file_name, const CSTRING *des_file_name, const UINT32 node_tcid)
{
    sys_log(LOGSTDOUT, "error:cfile_clone_on_node_tcid: incompleted implementation!\n");
#if 0
    CFILE_NODE *src_cfile_node;
    CFILE_NODE *des_cfile_node;

    UINT32      src_node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_clone_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    src_cfile_node = cfile_node_new(cfile_md_id, 0, src_file_name, src_node_dir_name);
    if(NULL_PTR == src_cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_clone_on_node_tcid: failed to new src node of file %s\n", (char *)cstring_get_str(src_file_name));
        return ((UINT32)-1);
    }

    if(EC_FALSE == cfile_fsearch_on_node_tcid(cfile_md_id, src_cfile_node, &src_node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_clone_on_node_tcid: not found node of file %s\n", (char *)CFILE_NODE_NAME_STR(src_cfile_node));
        cfile_node_free(cfile_md_id, src_cfile_node);
        return ((UINT32)-1);
    }

    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, src_cfile_node, CFILE_RB_OPEN_MODE, src_node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_clone_on_node_tcid: failed to open node of file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(src_cfile_node), c_word_to_ipv4(node_tcid));
        cfile_node_free(cfile_md_id, src_cfile_node);
        return ((UINT32)-1);
    }

    //sys_log(LOGSTDOUT, "info:cfile_clone_on_node_tcid: after cfile_fopen_on_node_tcid, src_cfile_node:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, src_cfile_node, 0);

    des_cfile_node = cfile_node_new(cfile_md_id, CFILE_NODE_SIZE(src_cfile_node), des_file_name, des_node_dir_name);
    if(NULL_PTR == des_cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_clone_on_node_tcid: failed to new des node of file %s\n", (char *)cstring_get_str(des_file_name));
        cfile_node_free(cfile_md_id, src_cfile_node);
        return ((UINT32)-1);
    }

    cfile_fclone_on_node_tcid(cfile_md_id, src_cfile_node, des_cfile_node, node_tcid);

    //sys_log(LOGSTDOUT, "info:cfile_clone_on_node_tcid: after cfile_fclone_on_node_tcid, des_cfile_node:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, des_cfile_node, 0);

    //cfile_fclose_on_node_tcid(cfile_md_id, src_cfile_node, node_tcid);
    cfile_fclose_on_node_tcid(cfile_md_id, des_cfile_node, node_tcid);

    cfile_node_free(cfile_md_id, src_cfile_node);
    cfile_node_free(cfile_md_id, des_cfile_node);
#endif
    return (0);
}

/**
*
* copy src file to des file to specific tcid
*   1. search src file on which tcid
*   2. read src node xml file
*   3. create des node xml and its segs on specific tcid
*   4. copy src node xml file and its seg data files to des node xml file and its segs
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_copy_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *src_file_name, const CSTRING *des_file_name, const UINT32 node_tcid)
{
    CFILE_NODE *src_cfile_node;
    CFILE_NODE *des_cfile_node;

    UINT32      src_node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_copy_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    src_cfile_node = cfile_node_new(cfile_md_id, 0, src_file_name);
    if(NULL_PTR == src_cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_copy_on_node_tcid: failed to new src node of file %s\n", (char *)cstring_get_str(src_file_name));
        return ((UINT32)-1);
    }

    if(EC_FALSE == cfile_fsearch_on_node_tcid(cfile_md_id, src_cfile_node, &src_node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_copy_on_node_tcid: not found node of file %s\n", (char *)CFILE_NODE_NAME_STR(src_cfile_node));
        cfile_node_free(cfile_md_id, src_cfile_node);
        return ((UINT32)-1);
    }

    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, src_cfile_node, CFILE_RB_OPEN_MODE, src_node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_copy_on_node_tcid: failed to open node of file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(src_cfile_node), c_word_to_ipv4(src_node_tcid));
        cfile_node_free(cfile_md_id, src_cfile_node);
        return ((UINT32)-1);
    }

    //sys_log(LOGSTDOUT, "info:cfile_clone_on_node_tcid: after cfile_fopen_on_node_tcid, src_cfile_node:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, src_cfile_node, 0);

    des_cfile_node = cfile_node_new(cfile_md_id, CFILE_NODE_SIZE(src_cfile_node), des_file_name);
    if(NULL_PTR == des_cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_copy_on_node_tcid: failed to new des node of file %s\n", (char *)cstring_get_str(des_file_name));
        cfile_node_free(cfile_md_id, src_cfile_node);
        return ((UINT32)-1);
    }

    cfile_fcreate_on_node_tcid(cfile_md_id, des_cfile_node, node_tcid);

    cfile_fcopy_on_node_tcid(cfile_md_id, src_cfile_node, des_cfile_node, node_tcid);

    //sys_log(LOGSTDOUT, "info:cfile_clone_on_node_tcid: after cfile_fclone_on_node_tcid, des_cfile_node:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, des_cfile_node, 0);

    //cfile_fclose_on_node_tcid(cfile_md_id, src_cfile_node, node_tcid);
    //cfile_fclose_on_node_tcid(cfile_md_id, des_cfile_node, node_tcid);

    cfile_node_free(cfile_md_id, src_cfile_node);
    cfile_node_free(cfile_md_id, des_cfile_node);

    return (0);
}

/**
*
* search file and return the tcid owning it
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_search_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *file_name, UINT32 *node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    CFILE_NODE *cfile_node;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_search_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    cfile_node = cfile_node_new(cfile_md_id, 0, file_name);
    if(NULL_PTR == cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_search_on_node_tcid: failed to new node of file %s\n", (char *)cstring_get_str(file_name));
        return (EC_FALSE);
    }

    if(EC_FALSE == cfile_fsearch_on_node_tcid(cfile_md_id, cfile_node, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_search_on_node_tcid: failed to search node of file %s\n", (char *)cstring_get_str(file_name));
        cfile_node_free(cfile_md_id, cfile_node);
        return (EC_FALSE);
    }

    cfile_node_free(cfile_md_id, cfile_node);
    return (EC_TRUE);
}

/**
*
* compare two files on specific tcid
*   1. open the 1st file on specific tcid
*   2. open the 2nd file on specific tcid
*   3. compare the node info only
*
* note:
*   1. not compare the node name, otherwise, they are always different:-)
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_cmp_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *file_name_1st, const CSTRING *file_name_2nd, const UINT32 node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    CFILE_NODE *cfile_node_1st;
    CFILE_NODE *cfile_node_2nd;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_cmp_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    /*warning: trick! we not desire to return any changes on cfile_node_1st and cfile_node_2nd*/

    cfile_node_1st = cfile_node_new(cfile_md_id, 0, file_name_1st);
    if(NULL_PTR == cfile_node_1st)
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_on_node_tcid: failed to new 1st node of file %s\n", (char *)cstring_get_str(file_name_1st));
        return (EC_FALSE);
    }

    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, cfile_node_1st, CFILE_RB_OPEN_MODE, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_on_node_tcid: failed to open 1st node of file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(cfile_node_1st), c_word_to_ipv4(node_tcid));
        cfile_node_free(cfile_md_id, cfile_node_1st);
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "info:cfile_cmp_on_node_tcid: cfile_node_1st:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, cfile_node_1st, 0);

    cfile_node_2nd = cfile_node_new(cfile_md_id, 0, file_name_2nd);
    if(NULL_PTR == cfile_node_2nd)
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_on_node_tcid: failed to new 2nd node of file %s\n", (char *)cstring_get_str(file_name_2nd));
        cfile_node_free(cfile_md_id, cfile_node_1st);
        return (EC_FALSE);
    }

    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, cfile_node_2nd, CFILE_RB_OPEN_MODE, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_on_node_tcid: failed to open 2nd node of file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(cfile_node_2nd), c_word_to_ipv4(node_tcid));
        cfile_node_free(cfile_md_id, cfile_node_1st);
        cfile_node_free(cfile_md_id, cfile_node_2nd);
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "info:cfile_cmp_on_node_tcid: cfile_node_2nd:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, cfile_node_2nd, 0);

    if(EC_FALSE == cfile_node_cmp(cfile_md_id, cfile_node_1st, cfile_node_2nd))
    {
        cfile_node_free(cfile_md_id, cfile_node_1st);
        cfile_node_free(cfile_md_id, cfile_node_2nd);
        return (EC_FALSE);
    }

    cfile_node_free(cfile_md_id, cfile_node_1st);
    cfile_node_free(cfile_md_id, cfile_node_2nd);

    return (EC_TRUE);
}

/**
*
* read & fwrite node in group mode on specific tcid
*   1. read all seg data files from local file into each CFILE_SEG KBUFF in group mode
*   2. write all seg data files into remote data files
*   3. write node xml file into specific tcid
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
static UINT32 cfile_read_fwrite_group_on_node_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, FILE *in_fp, const UINT32 node_tcid)
{
    CFILE_MD *cfile_md;
    MOD_MGR  *mod_mgr;

    CVECTOR    *kbuff_vec;

    UINT32    cfile_seg_num;
    UINT32    cfile_seg_pos;

    UINT32    ret;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_read_fwrite_group_on_node_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cfile_node_write, ERR_MODULE_ID, cfile_node, node_tcid))
        {
            sys_log(LOGSTDOUT, "error:cfile_read_fwrite_group_on_node_tcid: something wrong when make task with tcid %s and file %s for node write\n",
                               c_word_to_ipv4(node_tcid), (char *)CFILE_NODE_NAME_STR(cfile_node));
            return ((UINT32)-1);
        }
    }
    else
    {
        ret = cfile_node_write_with_node_tcid(cfile_md_id, cfile_node, node_tcid);
    }

    if(0 != ret)
    {
        sys_log(LOGSTDOUT, "error:cfile_read_fwrite_group_on_node_tcid: failed to write node info with file %s and tcid %s to tcid %s\n",
                           (char *)CFILE_NODE_NAME_STR(cfile_node), CFILE_NODE_TCID_STR(cfile_node, CFILE_NODE_TCID_POS), c_word_to_ipv4(node_tcid));
        return ((UINT32)-1);
    }

    kbuff_vec = cfile_kbuff_vec_new(cfile_md_id, FILE_SEG_GROUP_SIZE);
    if(NULL_PTR == kbuff_vec)
    {
        sys_log(LOGSTDOUT, "error:cfile_read_fwrite_group_on_node_tcid: failed to alloc %ld KBUFF\n", FILE_SEG_GROUP_SIZE);
        return ((UINT32)-1);
    }

    cfile_seg_num = cvector_size(CFILE_SEG_VEC(cfile_node));
    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos += FILE_SEG_GROUP_SIZE)
    {
        if(0 != cfile_seg_vec_read(cfile_md_id, CFILE_SEG_VEC(cfile_node), cfile_seg_pos, cfile_seg_pos + FILE_SEG_GROUP_SIZE, kbuff_vec, in_fp))
        {
            sys_log(LOGSTDOUT, "error:cfile_read_fwrite_group_on_node_tcid: failed to read from seg #%ld\n", cfile_seg_pos);
            cfile_kbuff_vec_free(cfile_md_id, kbuff_vec);
            return ((UINT32)-1);
        }

        if(0 != cfile_seg_vec_fwrite(cfile_md_id, CFILE_SEG_VEC(cfile_node), cfile_seg_pos, cfile_seg_pos + FILE_SEG_GROUP_SIZE, kbuff_vec))
        {
            sys_log(LOGSTDOUT, "error:cfile_read_fwrite_group_on_node_tcid: failed to write from seg #%ld\n", cfile_seg_pos);
            cfile_kbuff_vec_free(cfile_md_id, kbuff_vec);
            return ((UINT32)-1);
        }

        cfile_kbuff_vec_reset(cfile_md_id, kbuff_vec);
    }

    cfile_kbuff_vec_free(cfile_md_id, kbuff_vec);

    return (0);
}

/**
*
* fread & write node in group mode
*   1. read all seg data files from remote data files into each CFILE_SEG KBUFF in group mode
*   2. write all seg data files into local file
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
static UINT32 cfile_fread_write_group_on_local_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, FILE *out_fp)
{
    CVECTOR  *kbuff_vec;

    UINT32    cfile_seg_num;
    UINT32    cfile_seg_pos;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fread_write_group_on_local_tcid: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    kbuff_vec = cfile_kbuff_vec_new(cfile_md_id, FILE_SEG_GROUP_SIZE);
    if(NULL_PTR == kbuff_vec)
    {
        sys_log(LOGSTDOUT, "error:cfile_fread_write_group_on_local_tcid: failed to alloc %ld KBUFF\n", FILE_SEG_GROUP_SIZE);
        return ((UINT32)-1);
    }

    cfile_seg_num = cvector_size(CFILE_SEG_VEC(cfile_node));
    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos += FILE_SEG_GROUP_SIZE)
    {
        if(0 != cfile_seg_vec_fread(cfile_md_id, CFILE_SEG_VEC(cfile_node), cfile_seg_pos, cfile_seg_pos + FILE_SEG_GROUP_SIZE, kbuff_vec))
        {
            sys_log(LOGSTDOUT, "error:cfile_fread_write_group_on_local_tcid: failed to read from remote into seg #%ld\n", cfile_seg_pos);
            cfile_kbuff_vec_free(cfile_md_id, kbuff_vec);
            return ((UINT32)-1);
        }


        if(0 != cfile_seg_vec_write(cfile_md_id, CFILE_SEG_VEC(cfile_node), cfile_seg_pos, cfile_seg_pos + FILE_SEG_GROUP_SIZE, kbuff_vec, out_fp))
        {
            sys_log(LOGSTDOUT, "error:cfile_fread_write_group_on_local_tcid: failed to write from seg #%ld to local file\n", cfile_seg_pos);
            cfile_kbuff_vec_free(cfile_md_id, kbuff_vec);
            return ((UINT32)-1);
        }

        cfile_kbuff_vec_reset(cfile_md_id, kbuff_vec);
    }

    cfile_kbuff_vec_free(cfile_md_id, kbuff_vec);

    return (0);
}

/*-------------------------------------------- external interface --------------------------------------------*/
/**
*
* create a transparent node xml file and its seg data files in CFILE_SEG pool
*   1. search a mod node with min load
*   2. create node xml by the mod node and its seg data files
* note:
* 1. node size must be initialized before calling
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fcreate_trans(const UINT32 cfile_md_id, CFILE_NODE *cfile_node)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    MOD_NODE   *mod_node;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fcreate_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    mod_node = mod_mgr_find_min_load_with_tcid_vec_filter(mod_mgr, &(cfile_md->node_tcid_vec));
    if(NULL_PTR == mod_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_fcreate_trans: failed to filter mod_node with node tcid vec\n");
        cvector_print(LOGSTDOUT, &(cfile_md->node_tcid_vec), NULL_PTR);
        return ((UINT32)-1);
    }

    return cfile_fcreate_on_node_tcid(cfile_md_id, cfile_node, MOD_NODE_TCID(mod_node));
}

/**
*
* open a transparent node xml file with specific open mode
*   1. search node xml file on which tcid
*   2. open node xml file on that tcid
*
* open MODE take one of values,
*
* CFILE_R_OPEN_MODE   : ascii read only
* CFILE_RB_OPEN_MODE  : binary read only
* CFILE_W_OPEN_MODE   : asicii write only
* CFILE_WB_OPEN_MODE  : binary write only
* CFILE_A_OPEN_MODE   : ascii append only  (NOT SUPPORT yet)
* CFILE_AB_OPEN_MODE  : binary append only (NOT SUPPORT yet)
* CFILE_ERR_OPEN_MODE : invalid mode
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fopen_trans(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, const UINT32 mode)
{
    UINT32      node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fopen_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node, &node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_fopen_trans: failed to search node of file %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
        return ((UINT32)-1);
    }

    return cfile_fopen_on_node_tcid(cfile_md_id, cfile_node, mode, node_tcid);
}

/**
*
* close a transparent node xml file
*   1. search node xml file on which tcid
*   2. close node xml file on that tcid
*
* note:
*   1. the CFILE_NODE info will override the old node xml file if existing
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fclose_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
    UINT32    node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fclose_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node, &node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_fclose_trans: failed to search node of file %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
        return ((UINT32)-1);
    }

    return cfile_fclose_on_node_tcid(cfile_md_id, cfile_node, node_tcid);
}

/**
*
* check transparent CFILE_NODE check CFILE_NODE existing/readable/writable/executable or their bit 'and'('&') combination
*   1. search node xml file on which tcid
*   2. on that tcid, if node xml file check failed, return EC_FALSE
*   3. otherwise, if each seg data file check passed on one tcid at least, then return EC_TRUE, otherwise return EC_FALSE
*
* note:
*   1. if some seg data file check failed on any tcid of the CFILE_SEG pool, then the CFILE_NODE checking is regarded failed
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_fcheck_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 mode)
{
    UINT32    node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fexist_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node, &node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_fexist_trans: failed to search node of file %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
        return (EC_FALSE);
    }

    return cfile_fcheck_on_node_tcid(cfile_md_id, cfile_node, mode, node_tcid);
}

/**
*
* search tranparent node xml file and return one tcid which own it if possible
*   1. run through all node tcid vec and search the node xml file existing or not
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_fsearch_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, UINT32 *node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;
    TASK_MGR   *task_mgr;

    CVECTOR    *tcid_list;
    CARRAY     *ret_list;

    UINT32      tcid_num;
    UINT32      tcid_pos;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fsearch_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    tcid_list = &(cfile_md->node_tcid_vec);

    tcid_num = cvector_size(tcid_list);
    ret_list = carray_new(tcid_num, (void *)EC_FALSE, LOC_CFILE_0084);

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(tcid_pos = 0; tcid_pos < tcid_num; tcid_pos ++)
    {
        UINT32 tcid;
        void  *ret_addr;

        tcid     = (UINT32)cvector_get(tcid_list, tcid_pos);
        ret_addr = (void *)carray_get_addr(ret_list, tcid_pos);

        task_tcid_inc(task_mgr, tcid, ret_addr, FI_cfile_node_fexist, ERR_MODULE_ID, cfile_node);
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    tcid_pos = carray_search_back(ret_list, (void *)EC_TRUE, NULL_PTR);
    if(tcid_pos < tcid_num)
    {
        (*node_tcid) = (UINT32)cvector_get(tcid_list, tcid_pos);

        sys_log(LOGSTDOUT, "info:cfile_fsearch_trans: file %s found on tcid %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(*node_tcid));

        carray_free(ret_list, LOC_CFILE_0085);
        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "info:cfile_fsearch_trans: file %s NOT found\n", (char *)CFILE_NODE_NAME_STR(cfile_node));

    carray_free(ret_list, LOC_CFILE_0086);
    return (EC_FALSE);
}

/**
*
* read transparent node
*   1. search node xml file on which tcid
*   2. read node xml file on that tcid
*   3. read all seg data files into each CFILE_SEG KBUFF
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fread_trans(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, CVECTOR *kbuff_vec)
{
    UINT32    node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fread_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node, &node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_fread_trans: failed to search node of file %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
        return ((UINT32)-1);
    }

    return cfile_fread_on_node_tcid(cfile_md_id, cfile_node, kbuff_vec, node_tcid);
}

/**
*
* write transparent node
*   1. search node xml file on which tcid
*   2. write node xml file with & on that tcid
*   3. write all seg data files in pipe-line mode
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fwrite_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const CVECTOR *kbuff_vec)
{
    UINT32    node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_fwrite_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node, &node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_fwrite_trans: failed to search node of file %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
        return ((UINT32)-1);
    }

    return cfile_fwrite_on_node_tcid(cfile_md_id, cfile_node, kbuff_vec, node_tcid);
}

/**
*
* remove transparent node
*   1. search node xml file on which tcid
*   2. remove all seg data files in pipe-line mode
*   3. remove node xml file on that tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_frmv_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node)
{
    UINT32    node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_frmv_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node, &node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_frmv_trans: failed to search node of file %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
        return ((UINT32)-1);
    }

    return cfile_frmv_on_node_tcid(cfile_md_id, cfile_node, node_tcid);
}

/**
*
* copy transparent node
*   1. search node xml file on which tcid
*   2. copy all seg data files in pipe-line mode
*   3. copy node xml file with & on that tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_fcopy_trans(const UINT32 cfile_md_id, const CFILE_NODE *src_cfile_node, const CFILE_NODE *des_cfile_node)
{
    UINT32    node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_frmv_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, src_cfile_node, &node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_frmv_trans: failed to search src node of file %s\n", (char *)CFILE_NODE_NAME_STR(src_cfile_node));
        return ((UINT32)-1);
    }

    return cfile_fcopy_on_node_tcid(cfile_md_id, src_cfile_node, des_cfile_node, node_tcid);
}

/**
*
* remove a transparent file
*   1. search node xml file on which tcid
*   2. open node on that tcid
*   3. remove all seg data files in pipe-line mode and remove the node xml file on that tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_rmv_trans(const UINT32 cfile_md_id, const CSTRING *file_name)
{
    CFILE_NODE *cfile_node;
    UINT32      node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_rmv_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_node = cfile_node_new(cfile_md_id, 0, file_name);
    if(NULL_PTR == cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_rmv_trans: failed to new node of file %s\n", (char *)cstring_get_str(file_name));
        return ((UINT32)-1);
    }

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node, &node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_rmv_trans: failed to search src node of file %s\n", (char *)CFILE_NODE_NAME_STR(cfile_node));
        cfile_node_free(cfile_md_id, cfile_node);
        return ((UINT32)-1);
    }

    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, cfile_node, CFILE_ERR_OPEN_MODE, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_rmv_trans: failed to open file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(node_tcid));
        cfile_node_free(cfile_md_id, cfile_node);
        return ((UINT32)-1);
    }

    if(0 != cfile_frmv_on_node_tcid(cfile_md_id, cfile_node, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_rmv_trans: failed to frmv file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(cfile_node), c_word_to_ipv4(node_tcid));
        cfile_node_free(cfile_md_id, cfile_node);
        return ((UINT32)-1);
    }

    cfile_node_free(cfile_md_id, cfile_node);

    return (0);
}

/**
*
* copy src transparent file to des file
*   1. search src file on which tcid
*   2. read src node xml file
*   3. create des node xml and its segs on that tcid
*   4. copy src node xml file and its seg data files to des node xml file and its segs
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cfile_copy_trans(const UINT32 cfile_md_id, const CSTRING *src_file_name, const CSTRING *des_file_name, const UINT32 des_node_tcid)
{
    CFILE_NODE *src_cfile_node;
    CFILE_NODE *des_cfile_node;

    UINT32      src_node_tcid;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_copy_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    src_cfile_node = cfile_node_new(cfile_md_id, 0, src_file_name);
    if(NULL_PTR == src_cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_copy_trans: failed to new src node of file %s\n", (char *)cstring_get_str(src_file_name));
        return ((UINT32)-1);
    }

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, src_cfile_node, &src_node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_copy_trans: not found node of file %s\n", (char *)CFILE_NODE_NAME_STR(src_cfile_node));
        cfile_node_free(cfile_md_id, src_cfile_node);
        return ((UINT32)-1);
    }

    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, src_cfile_node, CFILE_RB_OPEN_MODE, src_node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_copy_trans: failed to open node of file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(src_cfile_node), c_word_to_ipv4(src_node_tcid));
        cfile_node_free(cfile_md_id, src_cfile_node);
        return ((UINT32)-1);
    }

    des_cfile_node = cfile_node_new(cfile_md_id, CFILE_NODE_SIZE(src_cfile_node), des_file_name);
    if(NULL_PTR == des_cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_copy_trans: failed to new des node of file %s\n", (char *)cstring_get_str(des_file_name));
        cfile_node_free(cfile_md_id, src_cfile_node);
        return ((UINT32)-1);
    }

    cfile_fcreate_on_node_tcid(cfile_md_id, des_cfile_node, des_node_tcid);/**/

    cfile_fcopy_on_node_tcid(cfile_md_id, src_cfile_node, des_cfile_node, des_node_tcid);

    cfile_node_free(cfile_md_id, src_cfile_node);
    cfile_node_free(cfile_md_id, des_cfile_node);

    return (0);
}

/**
*
* search transparent file and return the tcid owning it
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_search_trans(const UINT32 cfile_md_id, const CSTRING *file_name, UINT32 *node_tcid)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    CFILE_NODE *cfile_node;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_search_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    cfile_node = cfile_node_new(cfile_md_id, 0, file_name);
    if(NULL_PTR == cfile_node)
    {
        sys_log(LOGSTDOUT, "error:cfile_search_trans: failed to new node of file %s\n", (char *)cstring_get_str(file_name));
        return (EC_FALSE);
    }

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node, node_tcid))
    {
        sys_log(LOGSTDOUT, "error:cfile_search_trans: failed to search node of file %s\n", (char *)cstring_get_str(file_name));
        cfile_node_free(cfile_md_id, cfile_node);
        return (EC_FALSE);
    }

    cfile_node_free(cfile_md_id, cfile_node);
    return (EC_TRUE);
}

/**
*
* compare two transparent files
*   1. open the 1st file
*   2. open the 2nd file
*   3. compare the node info only
*
* note:
*   1. not compare the node name, otherwise, they are always different:-)
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cfile_cmp_trans(const UINT32 cfile_md_id, const CSTRING *file_name_1st, const CSTRING *file_name_2nd)
{
    CFILE_MD   *cfile_md;
    MOD_MGR    *mod_mgr;

    CFILE_NODE *cfile_node_1st;
    CFILE_NODE *cfile_node_2nd;

    UINT32      node_tcid_1st;
    UINT32      node_tcid_2nd;

#if ( SWITCH_ON == CFILE_DEBUG_SWITCH )
    if ( CFILE_MD_ID_CHECK_INVALID(cfile_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cfile_cmp_trans: cfile module #0x%lx not started.\n",
                cfile_md_id);
        dbg_exit(MD_CFILE, cfile_md_id);
    }
#endif/*CFILE_DEBUG_SWITCH*/

    cfile_md = CFILE_MD_GET(cfile_md_id);
    mod_mgr  = cfile_md->mod_mgr;

    /*warning: trick! we not desire to return any changes on cfile_node_1st and cfile_node_2nd*/

    cfile_node_1st = cfile_node_new(cfile_md_id, 0, file_name_1st);
    if(NULL_PTR == cfile_node_1st)
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_trans: failed to new 1st node of file %s\n", (char *)cstring_get_str(file_name_1st));
        return (EC_FALSE);
    }

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node_1st, &node_tcid_1st))
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_trans: failed to search 1st node of file %s\n", (char *)cstring_get_str(file_name_1st));
        cfile_node_free(cfile_md_id, cfile_node_1st);
        return (EC_FALSE);
    }

    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, cfile_node_1st, CFILE_RB_OPEN_MODE, node_tcid_1st))
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_trans: failed to open 1st node of file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(cfile_node_1st), c_word_to_ipv4(node_tcid_1st));
        cfile_node_free(cfile_md_id, cfile_node_1st);
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "info:cfile_cmp_on_node_tcid: cfile_node_1st:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, cfile_node_1st, 0);

    cfile_node_2nd = cfile_node_new(cfile_md_id, 0, file_name_2nd);
    if(NULL_PTR == cfile_node_2nd)
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_trans: failed to new 2nd node of file %s\n", (char *)cstring_get_str(file_name_2nd));
        cfile_node_free(cfile_md_id, cfile_node_1st);
        return (EC_FALSE);
    }

    if(EC_FALSE == cfile_fsearch_trans(cfile_md_id, cfile_node_2nd, &node_tcid_2nd))
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_trans: failed to search 2nd node of file %s\n", (char *)cstring_get_str(file_name_2nd));
        cfile_node_free(cfile_md_id, cfile_node_1st);
        cfile_node_free(cfile_md_id, cfile_node_2nd);
        return (EC_FALSE);
    }

    if(0 != cfile_fopen_on_node_tcid(cfile_md_id, cfile_node_2nd, CFILE_RB_OPEN_MODE, node_tcid_2nd))
    {
        sys_log(LOGSTDOUT, "error:cfile_cmp_trans: failed to open 2nd node of file %s on tcid %s\n",
                            (char *)CFILE_NODE_NAME_STR(cfile_node_2nd), c_word_to_ipv4(node_tcid_2nd));
        cfile_node_free(cfile_md_id, cfile_node_1st);
        cfile_node_free(cfile_md_id, cfile_node_2nd);
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "info:cfile_cmp_on_node_tcid: cfile_node_2nd:\n");
    //cfile_node_xml_print_node(LOGSTDOUT, cfile_node_2nd, 0);

    if(EC_FALSE == cfile_node_cmp(cfile_md_id, cfile_node_1st, cfile_node_2nd))
    {
        cfile_node_free(cfile_md_id, cfile_node_1st);
        cfile_node_free(cfile_md_id, cfile_node_2nd);
        return (EC_FALSE);
    }

    cfile_node_free(cfile_md_id, cfile_node_1st);
    cfile_node_free(cfile_md_id, cfile_node_2nd);

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

