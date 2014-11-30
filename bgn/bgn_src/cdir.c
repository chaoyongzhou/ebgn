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

#include <dirent.h>
#include <sys/stat.h>

#include <errno.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"
#include "carray.h"
#include "cvector.h"
#include "cfile.h"
#include "cdir.h"

#include "cbc.h"

#include "cxml.h"
#include "cmisc.h"

#include "task.h"
#include "kbuff.h"

#include "findex.inc"

#define CDIR_MD_CAPACITY()                  (cbc_md_capacity(MD_CDIR))

#define CDIR_MD_GET(cdir_md_id)             ((CDIR_MD *)cbc_md_get(MD_CDIR, (cdir_md_id)))

#define CDIR_MD_ID_CHECK_INVALID(cdir_md_id)  \
    ((CMPI_ANY_MODI != (cdir_md_id)) && ((NULL_PTR == CDIR_MD_GET(cdir_md_id)) || (0 == (CDIR_MD_GET(cdir_md_id)->usedcounter))))

/**
*   for test only
*
*   to query the status of CDIR Module
*
**/
void cdir_print_module_status(const UINT32 cdir_md_id, LOG *log)
{
    CDIR_MD *cdir_md;
    UINT32 this_cdir_md_id;

    for( this_cdir_md_id = 0; this_cdir_md_id < CDIR_MD_CAPACITY(); this_cdir_md_id ++ )
    {
        cdir_md = CDIR_MD_GET(this_cdir_md_id);

        if ( NULL_PTR != cdir_md && 0 < cdir_md->usedcounter )
        {
            sys_log(log,"CDIR Module # %ld : %ld refered, refer father CFILE Module: %ld\n",
                    this_cdir_md_id,
                    cdir_md->usedcounter,
                    cdir_md->cfile_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed CDIR module
*
*
**/
UINT32 cdir_free_module_static_mem(const UINT32 cdir_md_id)
{
    CDIR_MD  *cdir_md;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_free_module_static_mem: cdir module #0x%lx not started.\n",
                cdir_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);

    free_module_static_mem(MD_CDIR, cdir_md_id);

    return 0;
}

/**
*
* start CDIR module
*
**/
UINT32 cdir_start(const CVECTOR *node_tcid_vec)
{
    CDIR_MD *cdir_md;
    UINT32 cdir_md_id;
    TASK_BRD *task_brd;

    cdir_md_id = cbc_md_new(MD_CDIR, sizeof(CDIR_MD));
    if(ERR_MODULE_ID == cdir_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one CDIR module */
    cdir_md = (CDIR_MD *)cbc_md_get(MD_CDIR, cdir_md_id);
    cdir_md->usedcounter   = 0;
    cdir_md->cfile_md_id   = ERR_MODULE_ID;

    /* create a new module node */
    init_static_mem();

    task_brd = task_brd_default_get();

    dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "info:cdir_start: input node_tcid_vec is\n");
    cvector_print(LOGSTDOUT, node_tcid_vec, NULL_PTR);

    cvector_init(&(cdir_md->node_tcid_vec), cvector_size(node_tcid_vec), MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CDIR_0001);
    cvector_clone_with_prev_filter(node_tcid_vec, &(cdir_md->node_tcid_vec), task_brd, (CVECTOR_DATA_PREV_FILTER)task_brd_check_tcid_connected, NULL_PTR, NULL_PTR);

    dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "info:cdir_start: output node_tcid_vec is\n");
    cvector_print(LOGSTDOUT, &(cdir_md->node_tcid_vec), NULL_PTR);

    /*default setting which will be override after cdir_set_mod_mgr calling*/
    cdir_md->mod_mgr = mod_mgr_new(cdir_md_id, LOAD_BALANCING_LOOP);

    cdir_md->usedcounter = 1;

    dbg_log(SEC_0111_CDIR, 5)(LOGSTDOUT, "cdir_start: start CDIR module #%ld\n", cdir_md_id);
    //dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "========================= cdir_start: CDIR table info:\n");
    //cdir_print_module_status(cdir_md_id, LOGSTDOUT);
    //cbc_print();

    return ( cdir_md_id );
}

/**
*
* end CDIR module
*
**/
void cdir_end(const UINT32 cdir_md_id)
{
    CDIR_MD *cdir_md;

    cdir_md = CDIR_MD_GET(cdir_md_id);
    if(NULL_PTR == cdir_md)
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT,"error:cdir_end: cdir_md_id = %ld not exist.\n", cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < cdir_md->usedcounter )
    {
        cdir_md->usedcounter --;
        return ;
    }

    if ( 0 == cdir_md->usedcounter )
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT,"error:cdir_end: cdir_md_id = %ld is not started.\n", cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }

    //task_brd_mod_mgr_rmv(cdir_md->task_brd, cdir_md->mod_mgr);
    mod_mgr_free(cdir_md->mod_mgr);
    cdir_md->mod_mgr       = NULL_PTR;

    cdir_md->cfile_md_id   = ERR_MODULE_ID;

    cvector_clean(&(cdir_md->node_tcid_vec), NULL_PTR, LOC_CDIR_0002);
    /* if nobody else occupied the module,then free its resource */

    /* free module : */
    //cdir_free_module_static_mem(cdir_md_id);

    cdir_md->usedcounter = 0;

    dbg_log(SEC_0111_CDIR, 5)(LOGSTDOUT, "cdir_end: stop CDIR module #%ld\n", cdir_md_id);
    cbc_md_free(MD_CDIR, cdir_md_id);

    breathing_static_mem();

    //dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "========================= cdir_end: CDIR table info:\n");
    //cdir_print_module_status(cdir_md_id, LOGSTDOUT);
    //cbc_print();

    return ;
}

/**
*
* get father CFILE module id of CDIR module
*
**/
EC_BOOL cdir_get_cfile_md_id(const UINT32 cdir_md_id, UINT32 *cfile_md_id)
{
    CDIR_MD *cdir_md;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_get_cfile_md_id: cdir module #0x%lx not started.\n",
                cdir_md_id);
        cdir_print_module_status(cdir_md_id, LOGSTDOUT);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);

    (*cfile_md_id) = cdir_md->cfile_md_id;
    return (EC_TRUE);
}

/**
*
* set father CFILE module id of CDIR module
*
**/
EC_BOOL cdir_set_cfile_md_id(const UINT32 cdir_md_id, const UINT32 cfile_md_id)
{
    CDIR_MD *cdir_md;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_set_cfile_md_id: cdir module #0x%lx not started.\n",
                cdir_md_id);
        cdir_print_module_status(cdir_md_id, LOGSTDOUT);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    cdir_md->cfile_md_id = cfile_md_id;
    return (EC_TRUE);
}

/**
*
* check tcid does NOT belong to tcid vector
*   if tcid not belong to tcid vector, return EC_TRUE; otherwise return EC_FALSE
*
**/
static UINT32 cdir_tcid_filter_out(const CVECTOR *tcid_vec, const UINT32 tcid)
{
    if(EC_TRUE == cvector_search_front(tcid_vec, (void *)tcid, NULL_PTR))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

/**
*
* initialize mod mgr of CDIR module
*
**/
UINT32 cdir_set_mod_mgr(const UINT32 cdir_md_id, const MOD_MGR * src_mod_mgr)
{
    CDIR_MD *cdir_md;
    MOD_MGR * des_mod_mgr;
    UINT32   node_tcid_num;
    CVECTOR *node_tcid_vec;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_set_mod_mgr: cdir module #0x%lx not started.\n",
                cdir_md_id);
        cdir_print_module_status(cdir_md_id, LOGSTDOUT);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    des_mod_mgr = cdir_md->mod_mgr;

    dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "info:cdir_set_mod_mgr: cdir_md_id: %ld, src_mod_mgr is\n", cdir_md_id, src_mod_mgr);
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    node_tcid_num = cvector_size(&(cdir_md->node_tcid_vec));
    node_tcid_vec = cvector_new(node_tcid_num, MM_UINT32, LOC_CDIR_0003);

    cvector_clone_with_prev_filter(&(cdir_md->node_tcid_vec), node_tcid_vec, (void *)node_tcid_vec, (CVECTOR_DATA_PREV_FILTER)cdir_tcid_filter_out, NULL_PTR, NULL_PTR);

    dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "info:cdir_set_mod_mgr: output tcid_vec is\n");
    cvector_print(LOGSTDOUT, node_tcid_vec, NULL_PTR);

    mod_mgr_limited_clone_with_tcid_filter(cdir_md_id, src_mod_mgr, node_tcid_vec, des_mod_mgr);

    cvector_free(node_tcid_vec, LOC_CDIR_0004);

    dbg_log(SEC_0111_CDIR, 5)(LOGSTDOUT, "====================================cdir_set_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    dbg_log(SEC_0111_CDIR, 5)(LOGSTDOUT, "====================================cdir_set_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);
    return (0);
}

/**
*
* get mod mgr of CDIR module
*
**/
MOD_MGR * cdir_get_mod_mgr(const UINT32 cdir_md_id)
{
    CDIR_MD *cdir_md;

    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        return (MOD_MGR *)0;
    }

    cdir_md = CDIR_MD_GET(cdir_md_id);
    return (cdir_md->mod_mgr);
}

/**
*
* split ${home}/${dir} into dir slices and store them into dir_clist
* e.g.,
*   home = /tmp/cfile_segs
*   dir = ../cfile_node
* then return dir_clist = {tmp, cfile_node}
*
**/
UINT32 cdir_dir_split(const UINT32 cdir_md_id, CSTRING *home, CSTRING *dir, CLIST *dir_clist)
{
    char *beg;
    char *cur;
    char *end;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_dir_split: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    beg = (char *)cstring_get_str(dir);
    cur = beg + strspn(beg, "/");
    end = beg + strlen(beg);

    if(cur == beg && NULL_PTR != home && NULL_PTR != cstring_get_str(home))
    {
        if(0 != cdir_dir_split(cdir_md_id, NULL_PTR, home, dir_clist))
        {
            dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_dir_split: invalid home\n");
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
                dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_dir_split: invalid dir\n");
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
UINT32 cdir_dir_encap(const UINT32 cdir_md_id, const CLIST *dir_clist, CSTRING *dir)
{
    CLIST_DATA *clist_data;
    char *dir_seg;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_dir_encap: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

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
UINT32 cdir_basename(const UINT32 cdir_md_id, const CSTRING *file_name, CSTRING *base_name)
{
    CSTRING *tmp_file_name;
    CLIST  *dir_clist;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_basename: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    tmp_file_name = cstring_new(cstring_get_str(file_name), LOC_CDIR_0005);
    dir_clist = clist_new(MM_IGNORE, LOC_CDIR_0006);

    if(0 != cdir_dir_split(cdir_md_id, NULL_PTR, tmp_file_name, dir_clist))
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_basename: invalid file name %s\n", (char *)cstring_get_str(file_name));

        clist_clean(dir_clist, NULL_PTR);
        clist_free(dir_clist, LOC_CDIR_0007);

        cstring_free(tmp_file_name);
        return ((UINT32)-1);
    }

    cstring_clean(base_name);
    cstring_init(base_name, (UINT8 *)clist_back(dir_clist));

    clist_clean(dir_clist, NULL_PTR);
    clist_free(dir_clist, LOC_CDIR_0008);

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
UINT32 cdir_dirname(const UINT32 cdir_md_id, const CSTRING *file_name, CSTRING *dir_name)
{
    CSTRING *tmp_file_name;
    CLIST  *dir_clist;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_dirname: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    tmp_file_name = cstring_new(cstring_get_str(file_name), LOC_CDIR_0009);
    dir_clist = clist_new(MM_IGNORE, LOC_CDIR_0010);

    if(0 != cdir_dir_split(cdir_md_id, NULL_PTR, tmp_file_name, dir_clist))
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_dirname: invalid file name %s\n", (char *)cstring_get_str(file_name));

        clist_clean(dir_clist, NULL_PTR);
        clist_free(dir_clist, LOC_CDIR_0011);

        cstring_free(tmp_file_name);
        return ((UINT32)-1);
    }

    clist_pop_back(dir_clist);
    cdir_dir_encap(cdir_md_id, dir_clist, dir_name);
    //cstring_append_char(dir_name, '/');

    clist_clean(dir_clist, NULL_PTR);
    clist_free(dir_clist, LOC_CDIR_0012);

    cstring_free(tmp_file_name);

    return (0);
}

void cdir_indent_print(LOG *log, const UINT32 level)
{
    UINT32 idx;

    for(idx = 0; idx < level; idx ++)
    {
        sys_print(log, "    ");
    }
    return;
}

/*---------------------------------- CDIR_SEG interface ----------------------------------*/
/**
*
* new a CDIR_SEG including
*   1. alloc memory for CDIR_SEG
*   2. set CDIR_SEG type to invalid value
*   3. clean CDIR_SEG name to null
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
CDIR_SEG * cdir_seg_new(const UINT32 cdir_md_id)
{
    CDIR_SEG *cdir_seg;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_seg_new: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    alloc_static_mem(MD_CDIR, cdir_md_id, MM_CDIR_SEG, &cdir_seg, LOC_CDIR_0013);
    cdir_seg_init(cdir_md_id, cdir_seg);

    return (cdir_seg);
}

/**
*
* initialize a CDIR_SEG including
*   1. set CDIR_SEG type to invalid value
*   2. clean CDIR_SEG name to null
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_seg_init(const UINT32 cdir_md_id, CDIR_SEG *cdir_seg)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_seg_init: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    CDIR_SEG_TYPE(cdir_seg) = CDIR_ERR_SEG;
    cstring_init(CDIR_SEG_NAME(cdir_seg), NULL_PTR);

    return (0);
}

/**
*
* clean a CDIR_SEG including
*   1. set CDIR_SEG type to invalid value
*   2. clean CDIR_SEG name to null
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_seg_clean(const UINT32 cdir_md_id, CDIR_SEG *cdir_seg)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_seg_clean: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    CDIR_SEG_TYPE(cdir_seg) = CDIR_ERR_SEG;
    cstring_clean(CDIR_SEG_NAME(cdir_seg));

    return (0);
}

/**
*
* free CDIR_SEG with cleanning
*   1. clean seg
*   2. free CDIR_SEG itself
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_seg_free(const UINT32 cdir_md_id, CDIR_SEG *cdir_seg)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_seg_free: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_seg_clean(cdir_md_id, cdir_seg);
    free_static_mem(MD_CDIR, cdir_md_id, MM_CDIR_SEG, cdir_seg, LOC_CDIR_0014);
    return (0);
}

/**
*
* set CDIR_SEG with type and name
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cdir_seg_set(const UINT32 cdir_md_id, const UINT32 seg_type, const CSTRING *seg_name, CDIR_SEG *cdir_seg)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_seg_set: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    CDIR_SEG_TYPE(cdir_seg) = seg_type;
    cstring_clone(seg_name, CDIR_SEG_NAME(cdir_seg));
    return (0);
}

/**
*
* remove CDIR_SEG including
*   1. if CDIR_SEG is node xml file, then remove all its data files at remote and remove node xml file itself
*   2. if CDIR_SEG is directory, then remove all its children node xml file as step 1, and all its children sub-directory as step 2.
*
* algorithem:
*   1. loop start
*   2. if CDIR_SEG is node xml file, then
*       2.1. remote all its data files at remote
*       2.2. remote node xml file itself
*   3. else CDIR_SEG is directory, then
*       3.1. if it has no any more child, then terminate
*       3.2. else, goto step 1.
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cdir_seg_rmv(const UINT32 cdir_md_id, const CDIR_SEG *cdir_seg)
{
    CDIR_MD *cdir_md;
    MOD_MGR *mod_mgr;

    UINT32 cfile_md_id;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_seg_rmv: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    mod_mgr = cdir_md->mod_mgr;

    cfile_md_id = cdir_md->cfile_md_id;

    if(CDIR_FILE_SEG == CDIR_SEG_TYPE(cdir_seg))
    {
        cfile_rmv_on_node_tcid(cfile_md_id, CDIR_SEG_NAME(cdir_seg), MOD_MGR_LOCAL_MOD_TCID(mod_mgr));

        return (0);
    }

    if(CDIR_DIR_SEG == CDIR_SEG_TYPE(cdir_seg))
    {
        CDIR_NODE *sub_cdir_node;

        sub_cdir_node = cdir_node_new(cdir_md_id);
        if(NULL_PTR == sub_cdir_node)
        {
            dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_seg_rmv: failed to open sub dir %s\n",(char *)CDIR_SEG_NAME_STR(cdir_seg));
            return ((UINT32)-1);
        }

        cdir_read_on_node_tcid(cdir_md_id, CDIR_SEG_NAME(cdir_seg), sub_cdir_node, MOD_MGR_LOCAL_MOD_TCID(mod_mgr));

        cdir_rmv_on_node_tcid(cdir_md_id, sub_cdir_node, MOD_MGR_LOCAL_MOD_TCID(mod_mgr));

        cdir_node_free(cdir_md_id, sub_cdir_node);

        return (0);
    }

    dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_seg_rmv: invalid seg type %ld when handle %s\n",
                        CDIR_SEG_TYPE(cdir_seg),
                        (char *)CDIR_SEG_NAME_STR(cdir_seg));
    return ((UINT32)-1);
}

/**
*
* check CDIR_SEG is directory or not
*   1. CDIR_SEG is directory, then return EC_TRUE; otherwise return EC_FALSE
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
EC_BOOL cdir_seg_is_dir(const UINT32 cdir_md_id, const CDIR_SEG *cdir_seg)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_seg_is_dir: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    if(CDIR_DIR_SEG == CDIR_SEG_TYPE(cdir_seg))
    {
        return (EC_TRUE);
    }

    if(CDIR_FILE_SEG == CDIR_SEG_TYPE(cdir_seg))
    {
        return (EC_FALSE);
    }

    dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_seg_is_dir: invalid seg type %ld\n", CDIR_SEG_TYPE(cdir_seg));
    return (EC_FALSE);
}

/**
*
* sync src CDIR_SEG on local tcid to des CDIR_SEG to remote node tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cdir_seg_copy_to_node_tcid(const UINT32 cdir_md_id, const CDIR_SEG *cdir_seg_src, const CDIR_SEG *cdir_seg_des, const UINT32 node_tcid)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_seg_sync: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT,"error:cdir_seg_copy_to_node_tcid: incompleted implementation!!!\n");

    //CDIR_SEG_TYPE(cdir_seg_des) = CDIR_SEG_TYPE(cdir_seg_src);
    //cstring_clone(CDIR_SEG_NAME(cdir_seg_src), CDIR_SEG_NAME(cdir_seg_des));
    return (0);
}

/**
*
* print single CDIR_SEG to LOG
* e.g., CDIR_SEG is
*   SEG_TYPE     = CDIR_FILE_SEG
*   SEG_NAME     = /tmp/cfile_node/test.log.xml
*
* then output
* [F] /tmp/cfile_node/test.log.xml
*
* e.g., CDIR_SEG is
*   SEG_TYPE     = CDIR_DIR_SEG
*   SEG_NAME     = /tmp/cfile_node/test.log.xml.segs
*
* then output
* [D] /tmp/cfile_node/test.log.xml.segs
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_seg_print(LOG *log, const CDIR_SEG *cdir_seg, const UINT32 level)
{
    if(CDIR_DIR_SEG == CDIR_SEG_TYPE(cdir_seg))
    {
        cdir_indent_print(log, level);
        sys_print(log, "[D] %s\n", (char *)CDIR_SEG_NAME_STR(cdir_seg));
        return (0);
    }

    if(CDIR_FILE_SEG == CDIR_SEG_TYPE(cdir_seg))
    {
        cdir_indent_print(log, level);
        sys_print(log, "[F] %s\n", (char *)CDIR_SEG_NAME_STR(cdir_seg));
        return (0);
    }

    sys_print(log, "error:cdir_seg_print: invalid type %ld of file %s\n", CDIR_SEG_TYPE(cdir_seg), (char *)CDIR_SEG_NAME_STR(cdir_seg));
    return ((UINT32)-1);
}

/**
*
* print single CDIR_SEG in xml to LOG
* e.g., CDIR_SEG is
*   SEG_TYPE     = CDIR_FILE_SEG
*   SEG_NAME     = /tmp/cfile_node/test.log.xml
*
* then output
*  <seg type="F" name="/tmp/cfile_node/test.log.xml"/>
*
* e.g., CDIR_SEG is
*   SEG_TYPE     = CDIR_DIR_SEG
*   SEG_NAME     = /tmp/cfile_node/test.log.xml.segs
*
* then output
*  <seg type="D" name="/tmp/cfile_node/test.log.xml.segs"/>
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_seg_xml_print(LOG *log, const CDIR_SEG *cdir_seg, const UINT32 level)
{
    if(CDIR_DIR_SEG == CDIR_SEG_TYPE(cdir_seg))
    {
        cdir_indent_print(log, level);
        sys_print(log, "<seg type=\"D\" name=\"%s\"/>\n", (char *)CDIR_SEG_NAME_STR(cdir_seg));
        return (0);
    }

    if(CDIR_FILE_SEG == CDIR_SEG_TYPE(cdir_seg))
    {
        cdir_indent_print(log, level);
        sys_print(log, "<seg type=\"F\" name=\"%s\"/>\n", (char *)CDIR_SEG_NAME_STR(cdir_seg));
        return (0);
    }

    sys_print(log, "error:cdir_seg_print: invalid type %ld of file %s\n", CDIR_SEG_TYPE(cdir_seg), (char *)CDIR_SEG_NAME_STR(cdir_seg));
    return ((UINT32)-1);
}

/*---------------------------------- CDIR_NODE interface ----------------------------------*/
/**
*
* new a CDIR_NODE including
*   1. alloc memory for CDIR_NODE
*   2. clean CDIR_NODE name to null
*   3. clean CDIR_NODE segs vector to empty
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
CDIR_NODE * cdir_node_new(const UINT32 cdir_md_id)
{
    CDIR_NODE *cdir_node;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_node_new: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    alloc_static_mem(MD_CDIR, cdir_md_id, MM_CDIR_NODE, &cdir_node, LOC_CDIR_0015);
    cdir_node_init(cdir_md_id, cdir_node);

    return (cdir_node);
}

/**
*
* initialize a CDIR_NODE including
*   1. clean CDIR_NODE name to null
*   2. clean CDIR_NODE segs vector to empty
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_node_init(const UINT32 cdir_md_id, CDIR_NODE *cdir_node)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_node_init: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cstring_init(CDIR_NODE_NAME(cdir_node), NULL_PTR);

    cvector_init(CDIR_NODE_SEGS(cdir_node), 0, MM_CDIR_SEG, CVECTOR_LOCK_ENABLE, LOC_CDIR_0016);

    return (0);
}

/**
*
* clean a CDIR_NODE including
*   1. clean CDIR_NODE name to null
*   2. free all CDIR_NODE segs in CDIR_NODE segs vector
*   3. clean CDIR_NODE segs vector to empty
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_node_clean(const UINT32 cdir_md_id, CDIR_NODE *cdir_node)
{
    UINT32 cdir_seg_num;
    UINT32 cdir_seg_pos;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_node_clean: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cstring_clean(CDIR_NODE_NAME(cdir_node));

    cdir_seg_num = cvector_size(CDIR_NODE_SEGS(cdir_node));
    for(cdir_seg_pos = 0; cdir_seg_pos < cdir_seg_num; cdir_seg_pos ++)
    {
        CDIR_SEG *cdir_seg;
        cdir_seg = (CDIR_SEG *)cvector_get(CDIR_NODE_SEGS(cdir_node), cdir_seg_pos);
        if(NULL_PTR != cdir_seg)
        {
            cdir_seg_free(cdir_md_id, cdir_seg);
        }
    }

    cvector_clean(CDIR_NODE_SEGS(cdir_node), NULL_PTR, LOC_CDIR_0017);

    return (0);
}

/**
*
* free a CDIR_NODE including
*   1. clean CDIR_NODE to empty
*   2. free CDIR_NODE itself
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_node_free(const UINT32 cdir_md_id, CDIR_NODE *cdir_node)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_node_free: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_node_clean(cdir_md_id, cdir_node);
    free_static_mem(MD_CDIR, cdir_md_id, MM_CDIR_NODE, cdir_node, LOC_CDIR_0018);
    return (0);
}

/**
*
* clone CDIR_NODE info
*   1. clone src CDIR_NODE name to des CDIR_NODE
*   2. clone whole CDIR_NODE seg vector to des CDIR_NODE seg vector
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [N] anywhere
*
**/
UINT32 cdir_node_clone(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node_src, CDIR_NODE *cdir_node_des)
{
    CDIR_MD *cdir_md;
    MOD_MGR *mod_mgr;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_node_clone: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    mod_mgr = cdir_md->mod_mgr;

    dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_node_clone: incompleted implementation!!!\n");
#if 0
    cdir_node_clean(cdir_md_id, cdir_node_des);
    cstring_clone(CDIR_NODE_NAME(cdir_node_src), CDIR_NODE_NAME(cdir_node_des));

    cdir_seg_num = cvector_size(CDIR_NODE_SEGS(cdir_node_src));
    for(cdir_seg_pos = 0; cdir_seg_pos < cdir_seg_num; cdir_seg_pos ++)
    {
        CDIR_SEG *cdir_seg_src;
        CDIR_SEG *cdir_seg_des;

        cdir_seg_src = (CDIR_SEG *)cvector_get(CDIR_NODE_SEGS(cdir_node_src), cdir_seg_pos);
        if(NULL_PTR == cdir_seg_src)
        {
            continue;
        }

        cdir_seg_des = cdir_seg_new(cdir_md_id);
        if(NULL_PTR == cdir_seg_des)
        {
            dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_node_clone: failed to new CDIR_SEG when cdir_seg_pos = %ld and src seg name %s\n",
                               cdir_seg_pos, (char *)CDIR_SEG_NAME_STR(cdir_seg_src));
            return ((UINT32)-1);
        }

        cdir_seg_clone(cdir_md_id, cdir_seg_src, cdir_seg_des);

        if(CDIR_DIR_SEG == CDIR_SEG_TYPE(cdir_seg_src))
        {
            CDIR_NODE *sub_cdir_node_src;
            CDIR_NODE *sub_cdir_node_des;

            sub_cdir_node_src = cdir_node_new(cdir_md_id);
            if(NULL_PTR == sub_cdir_node_src)
            {
                dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_node_clone: failed to new sub CDIR_SEG src when cdir_seg_pos = %ld and src seg name %s\n",
                                   cdir_seg_pos, (char *)CDIR_SEG_NAME_STR(cdir_seg_src));
                return ((UINT32)-1);
            }

            sub_cdir_node_des = cdir_node_new(cdir_md_id);
            if(NULL_PTR == sub_cdir_node_des)
            {
                dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_node_clone: failed to new sub CDIR_SEG des when cdir_seg_pos = %ld and src seg name %s\n",
                                   cdir_seg_pos, (char *)CDIR_SEG_NAME_STR(cdir_seg_src));
                cdir_node_free(cdir_md_id, sub_cdir_node_src);
                return ((UINT32)-1);
            }

            if(0 != cdir_read_on_node_tcid(cdir_md_id, CDIR_SEG_NAME(cdir_seg_src), sub_cdir_node_src, MOD_MGR_LOCAL_MOD_TCID(mod_mgr)))
            {
                dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_node_clone: failed to read sub CDIR_NODE src when cdir_seg_pos = %ld and src seg name %s\n",
                                   cdir_seg_pos, (char *)CDIR_SEG_NAME_STR(cdir_seg_src));
                cdir_node_free(cdir_md_id, sub_cdir_node_src);
                cdir_node_free(cdir_md_id, sub_cdir_node_des);
                return ((UINT32)-1);
            }

            cdir_node_clone(cdir_md_id, sub_cdir_node_src, sub_cdir_node_des);

            cdir_node_free(cdir_md_id, sub_cdir_node_src);
            cdir_node_free(cdir_md_id, sub_cdir_node_des);
        }
    }
#endif
    return (0);
}

/**
*
* print single CDIR_NODE to LOG
* e.g., CDIR_NODE is
* {
*   NODE_NAME    = /tmp/cfile_node
*   {
*     SEG_TYPE     = CDIR_FILE_SEG
*     SEG_NAME     = /tmp/cfile_node/test_a.log.xml
*   }
*   {
*     SEG_TYPE     = CDIR_DIR_SEG
*     SEG_NAME     = /tmp/cfile_node/test_b.log.xml.segs
*   }
* }
* then output
* dir: /tmp/cfile_node
*   [F] /tmp/cfile_node/test_a.log.xml
*   [D] /tmp/cfile_node/test_b.log.xml.segs
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_node_print(LOG *log, const CDIR_NODE *cdir_node, const UINT32 level)
{
    UINT32 cdir_seg_num;
    UINT32 cdir_seg_pos;

    cdir_indent_print(log, level);
    sys_print(log, "dir: %s\n", (char *)CDIR_NODE_NAME_STR(cdir_node));

    cdir_seg_num = cvector_size(CDIR_NODE_SEGS(cdir_node));
    for(cdir_seg_pos = 0; cdir_seg_pos < cdir_seg_num; cdir_seg_pos ++)
    {
        CDIR_SEG *cdir_seg;

        cdir_seg = (CDIR_SEG *)cvector_get(CDIR_NODE_SEGS(cdir_node), cdir_seg_pos);
        cdir_seg_print(log, cdir_seg, level + 1);
    }

    return (0);
}

/**
*
* print single CDIR_NODE in xml to LOG
* e.g., CDIR_NODE is
* {
*   NODE_NAME    = /tmp/cfile_node
*   {
*     SEG_TYPE     = CDIR_FILE_SEG
*     SEG_NAME     = /tmp/cfile_node/test_a.log.xml
*   }
*   {
*     SEG_TYPE     = CDIR_DIR_SEG
*     SEG_NAME     = /tmp/cfile_node/test_b.log.xml.segs
*   }
* }
* then output
* <dir name="/tmp/cfile_node">
*   <segments>
*       <seg type="F" name="/tmp/cfile_node/test_a.log.xml"/>
*       <seg type="D" name="/tmp/cfile_node/test_b.log.xml.segs"/>
*   </segments>
* </dir>
*
* this function can be called
*
* [Y] on local
* [N] at remote
* [N] anywhere
*
**/
UINT32 cdir_node_xml_print(LOG *log, const CDIR_NODE *cdir_node, const UINT32 level)
{
    UINT32 cdir_seg_num;
    UINT32 cdir_seg_pos;

    UINT32 cur_level;

    cur_level = level;
    cdir_indent_print(log, cur_level ++);
    sys_print(log, "<dir name=\"%s\">\n", (char *)CDIR_NODE_NAME_STR(cdir_node));

    cdir_indent_print(log, cur_level ++);
    sys_print(log, "<segments>\n");

    cdir_seg_num = cvector_size(CDIR_NODE_SEGS(cdir_node));
    for(cdir_seg_pos = 0; cdir_seg_pos < cdir_seg_num; cdir_seg_pos ++)
    {
        CDIR_SEG *cdir_seg;

        cdir_seg = (CDIR_SEG *)cvector_get(CDIR_NODE_SEGS(cdir_node), cdir_seg_pos);
        cdir_seg_xml_print(log, cdir_seg, cur_level);
    }

    cdir_indent_print(log, -- cur_level);
    sys_print(log, "</segments>\n");

    cdir_indent_print(log, -- cur_level);
    sys_print(log, "</dir>\n");
    return (0);
}

/*---------------------------------- external interface ----------------------------------*/
/**
*
* check the path is directory or not on specific node tcid
*   1. if node tcid is local tcid, then do
*       1.1. the path is a directory, then return EC_TRUE; otherwise, return EC_FALSE
*   2. if node tcid is remote tcid, then make a task and do step 1. on the specific node
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cdir_is_dir_on_node_tcid(const UINT32 cdir_md_id, const CSTRING *path_name, const UINT32 node_tcid)
{
    CDIR_MD *cdir_md;
    MOD_MGR *mod_mgr;

    struct stat sb;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_is_dir_on_node_tcid: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    mod_mgr = cdir_md->mod_mgr;
    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        EC_BOOL ret;
        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cdir_is_dir_on_node_tcid, ERR_MODULE_ID, path_name, node_tcid))
        {
            dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_is_dir_on_node_tcid: something wrong when make task to check dir %s on tcid %s\n",
                            (char *)cstring_get_str(path_name), c_word_to_ipv4(node_tcid));
            return (EC_FALSE);
        }
        return (ret);
    }
    if(stat((char *)cstring_get_str(path_name), &sb) >= 0 && S_ISDIR(sb.st_mode))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

/**
*
* create a dir with specific mode on specific node tcid
*   1. if node tcid is local tcid, then do
*       1.1. if the dir NOT exist and make dir with the specific mode successfully, then return failure(-1)
*       1.2. otherwise, return success(0)
*   2. if node tcid is remote tcid, then make a task and do step 1. on the specific node
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cdir_create_on_node_tcid(const UINT32 cdir_md_id, const CSTRING *dir_name, const UINT32 mode, const UINT32 node_tcid)
{
    CDIR_MD *cdir_md;
    MOD_MGR *mod_mgr;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_create_on_node_tcid: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    mod_mgr = cdir_md->mod_mgr;

    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        UINT32 ret;
        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cdir_create_on_node_tcid, ERR_MODULE_ID, dir_name, mode, node_tcid))
        {
            dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_create_on_node_tcid: something wrong when make task to create dir %s on tcid %s\n",
                                (char *)cstring_get_str(dir_name), c_word_to_ipv4(node_tcid));
            return ((UINT32)-1);
        }
        return (ret);
    }
    /*TODO: here need to check dir_name to make sure it is under home directory => to prevent malicious crack*/
    if(0 != mkdir((char *)cstring_get_str(dir_name), mode))
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_create_on_node_tcid: failed to create dir %s with mode %o\n", (char *)cstring_get_str(dir_name), mode);
        return ((UINT32)-1);
    }

    return (0);
}

/**
*
* read a dir on specific node tcid
*   1. if node tcid is local tcid, then do
*       1.1. if the dir exist, then read all its children files and dirs to CDIR_NODE
*   2. if node tcid is remote tcid, then make a task and do step 1. on the specific node
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cdir_read_on_node_tcid(const UINT32 cdir_md_id, const CSTRING *dir_name, CDIR_NODE *cdir_node, const UINT32 node_tcid)
{
    CDIR_MD *cdir_md;
    MOD_MGR *mod_mgr;

    CSTRING *path_name;
    UINT32   from;
    UINT32   to;

    DIR *dir;
    struct dirent *file;

    char *postfix = ".xml";/*only scan file nodes*/
    UINT32 postfix_len;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_read_on_node_tcid: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    mod_mgr = cdir_md->mod_mgr;

    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        UINT32 ret;
        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cdir_read_on_node_tcid, ERR_MODULE_ID, dir_name, cdir_node, node_tcid))
        {
            dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_read_on_node_tcid: something wrong when make task to open dir %s on tcid %s\n",
                                (char *)cstring_get_str(dir_name), c_word_to_ipv4(node_tcid));
            return ((UINT32)-1);
        }
        return (ret);
    }

    postfix_len = strlen(postfix);

    path_name = cstring_new(cstring_get_str(dir_name), LOC_CDIR_0019);
    if(NULL_PTR == path_name)
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_read_on_node_tcid: failed to new a CSTRING\n");
        return ((UINT32)-1);
    }

    //dbg_log(SEC_0111_CDIR, 5)(LOGSTDOUT, "[before] path_name: \n");
    //cstring_print(LOGSTDOUT, path_name);

    cstring_append_char(path_name, '/');
    from = cstring_get_len(path_name);

    //dbg_log(SEC_0111_CDIR, 5)(LOGSTDOUT, "[after] from = %ld, path_name: \n", from);
    //cstring_print(LOGSTDOUT, path_name);

    dir = opendir((char *)cstring_get_str(dir_name));
    if(NULL_PTR == dir)
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_read_on_node_tcid: failed to open dir %s\n", (char *)cstring_get_str(dir_name));
        cstring_free(path_name);
        return ((UINT32)-1);
    }

    cstring_clone(dir_name, CDIR_NODE_NAME(cdir_node));

    for(;;)
    {
        file = readdir(dir);
        if(NULL_PTR == file)
        {
            break;
        }
        //dbg_log(SEC_0111_CDIR, 5)(LOGSTDOUT, "cdir_open: check [%s] ... ", file->d_name);
        /*skip hidden files*/
        if(0 == strncmp(file->d_name, ".", 1))
        {
            //sys_print(LOGSTDOUT, "skip hidden file\n");
            continue;
        }

        to = cstring_get_len(path_name);
        cstring_erase_tail_str(path_name, to - from);

        cstring_append_str(path_name, (UINT8 *)(file->d_name));

        /*when file is directory*/
        if(EC_TRUE == cdir_is_dir_on_node_tcid(cdir_md_id, path_name, MOD_MGR_LOCAL_MOD_TCID(mod_mgr)))
        {
            CDIR_SEG *cdir_seg;

            /*file is directory*/
            cdir_seg = cdir_seg_new(cdir_md_id);

            CDIR_SEG_TYPE(cdir_seg) = CDIR_DIR_SEG;
            /*cstring_init(CDIR_SEG_NAME(cdir_seg), (UINT8 *)(file->d_name));*/
            cstring_clone(path_name, CDIR_SEG_NAME(cdir_seg));

            cvector_push(CDIR_NODE_SEGS(cdir_node), (void *)cdir_seg);

            //sys_print(LOGSTDOUT, "[D]\n");
            continue;
        }

        /*when file is not directory*/
        /*check file postfix*/
        if(strlen(file->d_name) >= strlen(postfix)
        && 0 == strncmp(file->d_name + strlen(file->d_name) - strlen(postfix), postfix, strlen(postfix) ))
        {
            CDIR_SEG *cdir_seg;

            /*matched*/
            cdir_seg = cdir_seg_new(cdir_md_id);

            CDIR_SEG_TYPE(cdir_seg) = CDIR_FILE_SEG;
            /*cstring_init(CDIR_SEG_NAME(cdir_seg), (UINT8 *)(file->d_name));*/
            cstring_clone(path_name, CDIR_SEG_NAME(cdir_seg));

            cstring_erase_tail_str(CDIR_SEG_NAME(cdir_seg), postfix_len); /*discard file postfix(.xml)*/

            cvector_push(CDIR_NODE_SEGS(cdir_node), (void *)cdir_seg);

            //sys_print(LOGSTDOUT, "[F]\n");
            continue;
        }

        //sys_print(LOGSTDOUT, "[INVALID]\n");

        /*else skip files without the specified postfix*/
    }

    closedir(dir);

    cstring_free(path_name);

    return (0);
}

/**
*
* clean a dir on specific node tcid
*   1. if node tcid is local tcid, then do
*       1.1. clean up all its children files(node xml files) and dirs
*   2. if node tcid is remote tcid, then make a task and do step 1. on the specific node
*
* note:
*   CDIR_NODE info must be read in to
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cdir_clean_on_node_tcid(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node, const UINT32 node_tcid)
{
    CDIR_MD *cdir_md;
    MOD_MGR *mod_mgr;

    UINT32 cdir_seg_num;
    UINT32 cdir_seg_pos;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_clean_on_node_tcid: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    mod_mgr = cdir_md->mod_mgr;

    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        UINT32 ret;

        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cdir_clean_on_node_tcid, ERR_MODULE_ID, cdir_node, node_tcid))
        {
            dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_clean_on_node_tcid: something wrong when make task to clean dir %s on tcid %s\n",
                                (char *)CDIR_NODE_NAME(cdir_node), c_word_to_ipv4(node_tcid));
            return ((UINT32)-1);
        }
        return (ret);
    }

    cdir_seg_num = cvector_size(CDIR_NODE_SEGS(cdir_node));
    for(cdir_seg_pos = 0; cdir_seg_pos < cdir_seg_num; cdir_seg_pos ++)
    {
        CDIR_SEG *cdir_seg;

        cdir_seg = (CDIR_SEG *)cvector_get(CDIR_NODE_SEGS(cdir_node), cdir_seg_pos);
        if(NULL_PTR == cdir_seg)
        {
            continue;
        }

        cdir_seg_rmv(cdir_md_id, cdir_seg);
    }

    return (0);
}

/**
*
* remove a dir on specific node tcid
*   1. if node tcid is local tcid, then do
*       1.1. clean up all its children files(node xml files) and dirs
*       1.2. remove the dir itself
*   2. if node tcid is remote tcid, then make a task and do step 1. on the specific node
*
* note:
*   1. CDIR_NODE info must be read in to
*   2. CDIR_NODE info would not be cleaned up, caller should take responsibility of that
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cdir_rmv_on_node_tcid(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node, const UINT32 node_tcid)
{
    CDIR_MD *cdir_md;
    MOD_MGR *mod_mgr;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_rmv_on_node_tcid: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    mod_mgr = cdir_md->mod_mgr;

    if(node_tcid != MOD_MGR_LOCAL_MOD_TCID(mod_mgr))
    {
        UINT32 ret;

        if(0 != task_tcid_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, node_tcid, &ret, FI_cdir_rmv_on_node_tcid, ERR_MODULE_ID, cdir_node, node_tcid))
        {
            dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_rmv_on_node_tcid: something wrong when make task to rmv dir %s on tcid %s\n",
                                (char *)CDIR_NODE_NAME(cdir_node), c_word_to_ipv4(node_tcid));
            return ((UINT32)-1);
        }
        return (ret);
    }

    if(NULL_PTR == CDIR_NODE_NAME_STR(cdir_node))
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_rmv_on_node_tcid: node name is null\n");
        return ((UINT32)-1);
    }

    cdir_clean_on_node_tcid(cdir_md_id, cdir_node, node_tcid);

    if(0 != rmdir((char *)CDIR_NODE_NAME_STR(cdir_node)))
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_rmv_on_node_tcid: failed to rmv dir %s with errno = %d, errstr = %s\n", (char *)CDIR_NODE_NAME_STR(cdir_node), errno, strerror(errno));
        return ((UINT32)-1);
    }

    //cdir_node_clean(cdir_md_id, cdir_node);
    return (0);
}

/**
*
* search/find out a dir on which node tcid
*   1. ask the whole namespace(CDIR NODE TCID vector) to query which tcid has the specific directory
*
* note:
*   1. only check directory name, hence this function is NOT suitable when dir_name is a node xml file name
*   2. only return the FIRST tcid which has the specific directory
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cdir_search_trans(const UINT32 cdir_md_id, const CSTRING *dir_name, UINT32 *node_tcid)
{
    CDIR_MD    *cdir_md;
    MOD_MGR    *mod_mgr;
    TASK_MGR   *task_mgr;

    CVECTOR    *node_tcid_list;

    CARRAY     *ret_list;

    UINT32      node_tcid_num;
    UINT32      node_tcid_pos;
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_search_trans: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/
    cdir_md = CDIR_MD_GET(cdir_md_id);
    mod_mgr = cdir_md->mod_mgr;

    node_tcid_list = &(cdir_md->node_tcid_vec);

    node_tcid_num = cvector_size(node_tcid_list);
    ret_list = carray_new(node_tcid_num, (void *)EC_FALSE, LOC_CDIR_0020);

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(node_tcid_pos = 0; node_tcid_pos < node_tcid_num; node_tcid_pos ++)
    {
        UINT32 this_node_tcid;
        void  *ret_addr;

        this_node_tcid = (UINT32)cvector_get(node_tcid_list, node_tcid_pos);
        ret_addr = (void *)carray_get_addr(ret_list, node_tcid_pos);
        task_tcid_inc(task_mgr, this_node_tcid, ret_addr, FI_cdir_is_dir_on_node_tcid, ERR_MODULE_ID, dir_name, this_node_tcid);
    }

    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    node_tcid_pos = carray_search_back(ret_list, (void *)EC_TRUE, NULL_PTR);
    if(node_tcid_pos < node_tcid_num)
    {
        (*node_tcid) = (UINT32)cvector_get(node_tcid_list, node_tcid_pos);
        dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "info:cdir_search_trans: dir %s found on node_tcid %s\n",
                            (char *)cstring_get_str(dir_name), c_word_to_ipv4(*node_tcid));
        carray_free(ret_list, LOC_CDIR_0021);
        return (EC_TRUE);
    }

    dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "info:cdir_search_trans: dir %s NOT found\n", (char *)cstring_get_str(dir_name));
    carray_free(ret_list, LOC_CDIR_0022);
    return (EC_FALSE);
}

/**
*
* check the specific path name is directory or not. if yes, return tcid which own it
*   1. ask the whole namespace(CDIR NODE TCID vector) to query which tcid has the specific directory
*   2. if some tcid own it, return EC_TRUE and the node tcid; otherwise, return EC_FALSE
*
* note:
*   1. only return the FIRST tcid which own the specific directory
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
EC_BOOL cdir_is_dir_trans(const UINT32 cdir_md_id, const CSTRING *path_name, UINT32 *node_tcid)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_is_dir_trans: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    return cdir_search_trans(cdir_md_id, path_name, node_tcid);
}

/**
*
* create a transparent dir
*   1. figure out the mod node which is the lightest load mod node
*   2. create a dir on tcid pointed by the mod node
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cdir_create_trans(const UINT32 cdir_md_id, const CSTRING *dir_name, const UINT32 mode)
{
    CDIR_MD *cdir_md;
    MOD_MGR *mod_mgr;

    MOD_NODE *mod_node;
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_create_trans: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cdir_md = CDIR_MD_GET(cdir_md_id);
    mod_mgr = cdir_md->mod_mgr;

    //dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "info:cdir_create_trans: cdir_md_id: %ld, src_mod_mgr is\n", cdir_md_id, mod_mgr);
    //mod_mgr_print(LOGSTDOUT, mod_mgr);

    //dbg_log(SEC_0111_CDIR, 3)(LOGSTDOUT, "info:cdir_create_trans: node_tcid_vec is:\n");
    //cvector_print(LOGSTDOUT, &(cdir_md->node_tcid_vec), NULL_PTR);

    mod_node = mod_mgr_find_min_load_with_tcid_vec_filter(mod_mgr, &(cdir_md->node_tcid_vec));
    if(NULL_PTR == mod_node)
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_create_trans: failed to filter mod_node with node tcid vec\n");
        cvector_print(LOGSTDOUT, &(cdir_md->node_tcid_vec), NULL_PTR);
        return ((UINT32)-1);
    }

    return cdir_create_on_node_tcid(cdir_md_id, dir_name, mode, MOD_NODE_TCID(mod_node));
}

/**
*
* read a transparent dir
*   1. search the dir on which tcid
*   2. read the CDIR_NODE on that tcid of the dir
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cdir_read_trans(const UINT32 cdir_md_id, const CSTRING *dir_name, CDIR_NODE *cdir_node)
{
    UINT32      node_tcid;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_read_trans: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    if(EC_FALSE == cdir_search_trans(cdir_md_id, dir_name, &node_tcid))
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_read_trans: failed to search dir %s\n", (char *)cstring_get_str(dir_name));
        return ((UINT32)-1);
    }

    return cdir_read_on_node_tcid(cdir_md_id, dir_name, cdir_node, node_tcid);
}

/**
*
* clean a transparent dir
*   1. search the dir on which tcid
*   2. clean the CDIR_NODE on that tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cdir_clean_trans(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node)
{
    UINT32      node_tcid;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_clean_trans: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    if(EC_FALSE == cdir_search_trans(cdir_md_id, CDIR_NODE_NAME(cdir_node), &node_tcid))
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_clean_trans: failed to search dir %s\n", (char *)CDIR_NODE_NAME(cdir_node));
        return ((UINT32)-1);
    }

    return cdir_clean_on_node_tcid(cdir_md_id, cdir_node, node_tcid);
}

/**
*
* remove a transparent dir
*   1. search the dir on which tcid
*   2. remove the CDIR_NODE on that tcid
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cdir_rmv_trans(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node)
{
    UINT32      node_tcid;

#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_rmv_trans: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    if(EC_FALSE == cdir_search_trans(cdir_md_id, CDIR_NODE_NAME(cdir_node), &node_tcid))
    {
        dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_rmv_trans: failed to search dir %s\n", (char *)CDIR_NODE_NAME(cdir_node));
        return ((UINT32)-1);
    }

    return cdir_rmv_on_node_tcid(cdir_md_id, cdir_node, node_tcid);
}

/**
*
* get the current working directory name
*
* this function can be called
*
* [Y] on local
* [Y] at remote
* [Y] anywhere
*
**/
UINT32 cdir_cwd(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node, CSTRING *cwd)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_cwd: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    cstring_clone(CDIR_NODE_NAME(cdir_node), cwd);

    return (0);
}

UINT32 cdir_sync(const UINT32 cdir_md_id, const CSTRING *src_dir_name, const UINT32 src_tcid, const CSTRING *des_dir_name, const UINT32 des_tcid)
{
#if ( SWITCH_ON == CDIR_DEBUG_SWITCH )
    if ( CDIR_MD_ID_CHECK_INVALID(cdir_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cdir_sync: cdir module #0x%lx not started.\n",
                cdir_md_id);
        dbg_exit(MD_CDIR, cdir_md_id);
    }
#endif/*CDIR_DEBUG_SWITCH*/

    dbg_log(SEC_0111_CDIR, 0)(LOGSTDOUT, "error:cdir_sync: sorry incomplete implementation...\n");
    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
