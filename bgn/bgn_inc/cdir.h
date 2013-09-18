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

#ifndef _CDIR_H
#define _CDIR_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "type.h"
#include "cstring.h"
#include "clist.h"
#include "cvector.h"
#include "cfile.h"

#define CDIR_DIR_SEG          ((UINT32) 1)
#define CDIR_FILE_SEG         ((UINT32) 2)
#define CDIR_ERR_SEG          ((UINT32)-1)

typedef struct
{
    /* used counter >= 0 */
    UINT32    usedcounter ;

    UINT32    cfile_md_id; /*refer to father CFILE module of this CDIR module*/

    CVECTOR   node_tcid_vec;/*CFILE_NODE TCID set, i.e., the namenode set*/
    MOD_MGR  *mod_mgr;

}CDIR_MD;

typedef struct
{
    UINT32    type;/*CDIR_DIR_SEG or CDIR_FILE_SEG*/
    CSTRING   name;/*dir name or file name*/
}CDIR_SEG;

typedef struct
{
    CSTRING   name;     /*dir name*/
    CVECTOR   segs;     /*vector of CDIR_SEG on node_tcid*/
}CDIR_NODE;

#define CDIR_SEG_TYPE(cdir_seg)              ((cdir_seg)->type)
#define CDIR_SEG_NAME(cdir_seg)              (&((cdir_seg)->name))
#define CDIR_SEG_NAME_STR(cdir_seg)          (cstring_get_str(CDIR_SEG_NAME(cdir_seg)))

#define CDIR_NODE_NAME(cdir_node)            (&((cdir_node)->name))
#define CDIR_NODE_NAME_STR(cdir_node)        (cstring_get_str(CDIR_NODE_NAME(cdir_node)))
#define CDIR_NODE_SEGS(cdir_node)            (&((cdir_node)->segs))


/**
*   for test only
*
*   to query the status of CDIR Module
*
**/
void cdir_print_module_status(const UINT32 cdir_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CDIR module
*
*
**/
UINT32 cdir_free_module_static_mem(const UINT32 cdir_md_id);

/**
*
* start CDIR module
*
**/
UINT32 cdir_start(const CVECTOR *node_tcid_vec);

/**
*
* end CDIR module
*
**/
void cdir_end(const UINT32 cdir_md_id);

/**
*
* get father CFILE module id of CDIR module
*
**/
EC_BOOL cdir_get_cfile_md_id(const UINT32 cdir_md_id, UINT32 *cfile_md_id);

/**
*
* set father CFILE module id of CDIR module
*
**/
EC_BOOL cdir_set_cfile_md_id(const UINT32 cdir_md_id, const UINT32 cfile_md_id);

/**
*
* initialize mod mgr of CDIR module
*
**/
UINT32 cdir_set_mod_mgr(const UINT32 cdir_md_id, const MOD_MGR * src_mod_mgr);

/**
*
* get mod mgr of CDIR module
*
**/
MOD_MGR * cdir_get_mod_mgr(const UINT32 cdir_md_id);

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
UINT32 cdir_dir_split(const UINT32 cdir_md_id, CSTRING *home, CSTRING *dir, CLIST *dir_clist);

/**
*
* encapsulate the dir slices into a dir
* e.g.,
*   dir_clist = {tmp, cfile_node}  
* then return dir = /tmp/cfile_node
*
**/
UINT32 cdir_dir_encap(const UINT32 cdir_md_id, const CLIST *dir_clist, CSTRING *dir);

/**
*
* figure out basename from file name
* e.g.,
*   file_name = /tmp/cfile_node/test.log.xml.segs/00000000.dat
* then return base_name = 00000000.dat
*
**/
UINT32 cdir_basename(const UINT32 cdir_md_id, const CSTRING *dir_name, CSTRING *base_name);

/**
*
* figure out dirname from file name
* e.g.,
*   file_name = /tmp/cfile_node/test.log.xml.segs/00000000.dat
* then return dir_name = /tmp/cfile_node/test.log.xml.segs
*
**/
UINT32 cdir_dirname(const UINT32 cdir_md_id, const CSTRING *file_name, CSTRING *dir_name);

/*---------------------------------- CDIR_SEG interface ----------------------------------*/
/**
*
* new a CDIR_SEG 
*
**/
CDIR_SEG * cdir_seg_new(const UINT32 cdir_md_id);

/**
*
* initialize a CDIR_SEG
*
**/
UINT32 cdir_seg_init(const UINT32 cdir_md_id, CDIR_SEG *cdir_seg);

/**
*
* clean a CDIR_SEG
*
**/
UINT32 cdir_seg_clean(const UINT32 cdir_md_id, CDIR_SEG *cdir_seg);

/**
*
* free CDIR_SEG 
*
**/
UINT32 cdir_seg_free(const UINT32 cdir_md_id, CDIR_SEG *cdir_seg);

/**
*
* set CDIR_SEG with type and name
*
**/
UINT32 cdir_seg_set(const UINT32 cdir_md_id, const UINT32 seg_type, const CSTRING *seg_name, CDIR_SEG *cdir_seg);

/**
*
* remove CDIR_SEG
*
**/
UINT32 cdir_seg_rmv(const UINT32 cdir_md_id, const CDIR_SEG *cdir_seg);

/**
*
* check CDIR_SEG is directory or not
*
**/
EC_BOOL cdir_seg_is_dir(const UINT32 cdir_md_id, const CDIR_SEG *cdir_seg);

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
**/
UINT32 cdir_seg_print(LOG *log, const CDIR_SEG *cdir_seg, const UINT32 level);

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
**/
UINT32 cdir_seg_xml_print(LOG *log, const CDIR_SEG *cdir_seg, const UINT32 level);

/*---------------------------------- CDIR_NODE interface ----------------------------------*/
/**
*
* new a CDIR_NODE
*
**/
CDIR_NODE * cdir_node_new(const UINT32 cdir_md_id);

/**
*
* initialize a CDIR_NODE
*
**/
UINT32 cdir_node_init(const UINT32 cdir_md_id, CDIR_NODE *cdir_node);

/**
*
* clean a CDIR_NODE
*
**/
UINT32 cdir_node_clean(const UINT32 cdir_md_id, CDIR_NODE *cdir_node);

/**
*
* free a CDIR_NODE
*
**/
UINT32 cdir_node_free(const UINT32 cdir_md_id, CDIR_NODE *cdir_node);

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
**/
UINT32 cdir_node_print(LOG *log, const CDIR_NODE *cdir_node, const UINT32 level);

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
**/
UINT32 cdir_node_xml_print(LOG *log, const CDIR_NODE *cdir_node, const UINT32 level);

/**
*
* check the path is directory or not on specific node tcid
*
**/
EC_BOOL cdir_is_dir_on_node_tcid(const UINT32 cdir_md_id, const CSTRING *path_name, const UINT32 node_tcid);

/**
*
* create a dir with specific mode on specific node tcid
*
**/
UINT32 cdir_create_on_node_tcid(const UINT32 cdir_md_id, const CSTRING *dir_name, const UINT32 mode, const UINT32 node_tcid);

/**
*
* read a dir on specific node tcid
*
**/
UINT32 cdir_read_on_node_tcid(const UINT32 cdir_md_id, const CSTRING *dir_name, CDIR_NODE *cdir_node, const UINT32 node_tcid);

/**
*
* clean a dir on specific node tcid

*
**/
UINT32 cdir_clean_on_node_tcid(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node, const UINT32 node_tcid);

/**
*
* remove a dir on specific node tcid
*
**/
UINT32 cdir_rmv_on_node_tcid(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node, const UINT32 node_tcid);

/**
*
* search/find out a dir on which node tcid
*
**/
EC_BOOL cdir_search_trans(const UINT32 cdir_md_id, const CSTRING *dir_name, UINT32 *node_tcid);

/**
*
* check the specific path name is directory or not. if yes, return tcid which own it
*
**/
EC_BOOL cdir_is_dir_trans(const UINT32 cdir_md_id, const CSTRING *path_name, UINT32 *node_tcid);

/**
*
* create a transparent dir
*
**/
UINT32 cdir_create_trans(const UINT32 cdir_md_id, const CSTRING *dir_name, const UINT32 mode);

/**
*
* read a transparent dir
*
**/
UINT32 cdir_read_trans(const UINT32 cdir_md_id, const CSTRING *dir_name, CDIR_NODE *cdir_node);

/**
*
* clean a transparent dir
*
**/
UINT32 cdir_clean_trans(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node);

/**
*
* remove a transparent dir
*
**/
UINT32 cdir_rmv_trans(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node);

/**
*
* get the current working directory name
*
**/
UINT32 cdir_cwd(const UINT32 cdir_md_id, const CDIR_NODE *cdir_node, CSTRING *cwd);

#endif /*_CDIR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

