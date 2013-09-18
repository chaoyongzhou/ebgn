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

#ifndef _CFILE_H
#define _CFILE_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "type.h"
#include "cstring.h"
#include "clist.h"
#include "cvector.h"

#include "mod.inc"
#include "kbuff.h"

#define CFILE_DEBUG_ONLY_SWITCH     (SWITCH_OFF)

#define CFILE_SEG_MAX_NUM_OF_TCID   ((UINT32) 3)

#define CFILE_SEG_START_TCID_POS    ((UINT32) 0)

#define CFILE_NODE_TCID_POS         ((UINT32) 0)

#define CFILE_R_OPEN_MODE           ((UINT32) 1)
#define CFILE_RB_OPEN_MODE          ((UINT32) 2)
#define CFILE_W_OPEN_MODE           ((UINT32) 3)
#define CFILE_WB_OPEN_MODE          ((UINT32) 4)
#define CFILE_A_OPEN_MODE           ((UINT32) 5)
#define CFILE_AB_OPEN_MODE          ((UINT32) 6)
#define CFILE_ERR_OPEN_MODE         ((UINT32)-1)

#define CFILE_MASK_UNDEF            ((UINT32)      0)
#define CFILE_MASK_EXIST            ((UINT32) 1 << 0)
#define CFILE_MASK_RABLE            ((UINT32) 1 << 1)
#define CFILE_MASK_WABLE            ((UINT32) 1 << 2)
#define CFILE_MASK_XABLE            ((UINT32) 1 << 3)

typedef struct
{
    /* used counter >= 0 */
    UINT32    usedcounter ;

    UINT32    cdir_md_id;

    CVECTOR   node_tcid_vec;/*CFILE_NODE TCID set, i.e., the namenode set*/
    CVECTOR   seg_tcid_vec; /*CFILE_SEG TCID set, i.e., the datanode set*/

    MOD_MGR  *mod_mgr;

}CFILE_MD;

#define CFILE_MD_MOD_MGR(cfile_md)     ((cfile_md)->mod_mgr)

typedef struct
{
    UINT32        seg_id;     /*segment identifier*/
    UINT32        seg_size;   /*segment size*/    
    CSTRING       seg_name;   /*segment file name*/

    CVECTOR       seg_tcid_vec;   /*volume name on which segment stores. Note: here mark volume as TCID, but it is not wise choice*/

    UINT32        open_mode;  /*private: segment file open mode*/
}CFILE_SEG;

typedef struct
{
    CSTRING             node_name;    
    UINT32              node_size;

    CVECTOR             node_tcid_vec;
    CVECTOR             seg_vec;
}CFILE_NODE;

#define CFILE_SEG_ID(cfile_seg)                 ((cfile_seg)->seg_id)
#define CFILE_SEG_SIZE(cfile_seg)               ((cfile_seg)->seg_size)
#define CFILE_SEG_NAME(cfile_seg)               (&((cfile_seg)->seg_name))
#define CFILE_SEG_NAME_STR(cfile_seg)           (cstring_get_str(CFILE_SEG_NAME(cfile_seg)))
#define CFILE_SEG_TCID_VEC(cfile_seg)           (&((cfile_seg)->seg_tcid_vec))
#define CFILE_SEG_TCID_NUM(cfile_seg)           (cvector_size(CFILE_SEG_TCID_VEC(cfile_seg)))
#define CFILE_SEG_TCID(cfile_seg, pos)          ((UINT32)cvector_get(CFILE_SEG_TCID_VEC(cfile_seg), pos))
#define CFILE_SEG_TCID_STR(cfile_seg, pos)      (uint32_to_ipv4(CFILE_SEG_TCID(cfile_seg, pos)))
#define CFILE_SEG_OPEN_MODE(cfile_seg)          ((cfile_seg)->open_mode)

#define CFILE_NODE_NAME(cfile_node)             (&((cfile_node)->node_name))
#define CFILE_NODE_NAME_STR(cfile_node)         (cstring_get_str(CFILE_NODE_NAME(cfile_node)))
#define CFILE_NODE_TCID_VEC(cfile_node)         (&((cfile_node)->node_tcid_vec))
#define CFILE_NODE_TCID_NUM(cfile_node)         (cvector_size(CFILE_NODE_TCID_VEC(cfile_node)))
#define CFILE_NODE_TCID(cfile_node, pos)        ((UINT32)cvector_get(CFILE_NODE_TCID_VEC(cfile_node), pos))
#define CFILE_NODE_TCID_STR(cfile_node, pos)    (uint32_to_ipv4(CFILE_NODE_TCID(cfile_node, pos)))
#define CFILE_NODE_SIZE(cfile_node)             ((cfile_node)->node_size)

#define CFILE_SEG_VEC(cfile_node)               (&((cfile_node)->seg_vec))


typedef xmlDocPtr    XMLDOCPTR;
typedef xmlNodePtr   XMLNODEPTR;
typedef xmlChar      XMLCHAR;

/**
*   for test only
*
*   to query the status of CFILE Module
*
**/
void cfile_print_module_status(const UINT32 cfile_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CFILE module
*
*
**/
UINT32 cfile_free_module_static_mem(const UINT32 cfile_md_id);

/**
*
* start CFILE module
*
**/
UINT32 cfile_start(const CVECTOR *node_tcid_vec, const CVECTOR *seg_tcid_vec);

/**
*
* end CFILE module
*
**/
void cfile_end(const UINT32 cfile_md_id);

/**
*
* get CDIR module id of CFILE module
*
**/
EC_BOOL cfile_get_cdir_md_id(const UINT32 cfile_md_id, UINT32 *cdir_md_id);

/**
*
* initialize mod mgr of CFILE module
*
**/
UINT32 cfile_set_mod_mgr(const UINT32 cfile_md_id, const MOD_MGR * src_mod_mgr);

/**
*
* get mod mgr of CFILE module
*
**/
MOD_MGR * cfile_get_mod_mgr(const UINT32 cfile_md_id);

/*---------------------------------- internal interface ----------------------------------*/
UINT32 cfile_dir_split(const UINT32 cfile_md_id, CSTRING *home, CSTRING *dir, CLIST *dir_clist);

UINT32 cfile_dir_encap(const UINT32 cfile_md_id, const CLIST *dir_clist, CSTRING *dir);

UINT32 cfile_basename(const UINT32 cfile_md_id, const CSTRING *file_name, CSTRING *base_name);

UINT32 cfile_dirname(const UINT32 cfile_md_id, const CSTRING *file_name, CSTRING *dir_name);

/*---------------------------------- CFILE_SEG interface ----------------------------------*/
CFILE_SEG * cfile_seg_new(const UINT32 cfile_md_id, const UINT32 seg_id, const UINT32 seg_size, const CSTRING *seg_dir_name);

UINT32 cfile_seg_tcid_push(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg, const UINT32 seg_tcid);

UINT32 cfile_seg_init(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg);

UINT32 cfile_seg_clean(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg);

UINT32 cfile_seg_free(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg);

UINT32 cfile_seg_dir_create(const UINT32 cfile_md_id, const CSTRING *seg_dir_name);

UINT32 cfile_seg_dir_rmv(const UINT32 cfile_md_id, const CSTRING *seg_dir_name);

EC_BOOL cfile_seg_dir_exist(const UINT32 cfile_md_id, const CSTRING *seg_dir_name);


/*---------------------------------- CFILE_SEG PIPELINE MODE file operation interface ----------------------------------*/
UINT32 cfile_seg_fcreate_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos);

EC_BOOL cfile_seg_fexist_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos);

EC_BOOL cfile_seg_frable_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos);

EC_BOOL cfile_seg_fwable_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos);

EC_BOOL cfile_seg_fxable_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos);

UINT32 cfile_seg_fread_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, KBUFF *kbuff, UINT32 *seg_tcid_pos);

UINT32 cfile_seg_fwrite_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, const KBUFF *kbuff, UINT32 *seg_tcid_pos);

UINT32 cfile_seg_rmv_ppl(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, UINT32 *seg_tcid_pos);

UINT32 cfile_seg_copy_ppl(const UINT32 cfile_md_id, const CFILE_SEG *src_cfile_seg, const CFILE_SEG *des_cfile_seg, UINT32 *seg_tcid_pos);

/*---------------------------------- CFILE_SEG_VEC interface ----------------------------------*/
CVECTOR *cfile_seg_vec_new(const UINT32 cfile_md_id);

UINT32 cfile_seg_vec_init(const UINT32 cfile_md_id, CVECTOR *cfile_seg_vec);

UINT32 cfile_seg_vec_clean(const UINT32 cfile_md_id, CVECTOR *cfile_seg_vec);

UINT32 cfile_seg_vec_free(const UINT32 cfile_md_id, CVECTOR *cfile_seg_vec);

/*---------------------------------- CFILE_NODE interface ----------------------------------*/
CFILE_NODE * cfile_node_new(const UINT32 cfile_md_id, const UINT32 ui_cfile_size, const CSTRING *ui_cfile_node_name);

UINT32 cfile_node_tcid_push(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, const UINT32 node_tcid);

UINT32 cfile_node_init(const UINT32 cfile_md_id, CFILE_NODE *cfile_node);

UINT32 cfile_node_clean(const UINT32 cfile_md_id, CFILE_NODE *cfile_node);

UINT32 cfile_node_free(const UINT32 cfile_md_id, CFILE_NODE *cfile_node);


/*---------------------------------- CFILE_NODE XML Printer interface ----------------------------------*/
UINT32 cfile_node_xml_print_node(LOG *log, const CFILE_NODE *cfile_node, const UINT32 level);

UINT32 cfile_node_xml_print_node_with_node_tcid(LOG *log, const CFILE_NODE *cfile_node, const UINT32 node_tcid, const UINT32 level);

/*---------------------------------- CFILE_NODE file operation interface ----------------------------------*/
UINT32 cfile_node_read(const UINT32 cfile_md_id, CFILE_NODE *cfile_node);

UINT32 cfile_node_write(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

UINT32 cfile_node_write_with_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 node_tcid);

UINT32 cfile_node_fcreate(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

UINT32 cfile_node_fopen(const UINT32 cfile_md_id, CFILE_NODE *cfile_node);

UINT32 cfile_node_fclose(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

EC_BOOL cfile_node_fexist(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

EC_BOOL cfile_node_frable(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

EC_BOOL cfile_node_fwable(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

EC_BOOL cfile_node_fxable(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

UINT32 cfile_node_rmv(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

UINT32 cfile_node_clone(const UINT32 cfile_md_id, const CFILE_NODE *src_cfile_node, CFILE_NODE *des_cfile_node);

EC_BOOL cfile_node_cmp(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node_1st, const CFILE_NODE *cfile_node_2nd);

EC_BOOL cfile_node_fcheck(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 mask);
/*---------------------------------- CFILE_SEG file operation interface ----------------------------------*/
UINT32 cfile_seg_kbuff_set(const UINT32 cfile_md_id, const UINT32 size, CFILE_SEG *cfile_seg);

UINT32 cfile_seg_kbuff_clean(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg);

UINT32 cfile_seg_kbuff_reset(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg);
UINT32 cfile_seg_open_mode_set(const UINT32 cfile_md_id, const UINT32 open_mode, CFILE_SEG *cfile_seg);
UINT32 cfile_seg_fcreate(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg);

EC_BOOL cfile_seg_fexist(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg);

EC_BOOL cfile_seg_frable(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg);

EC_BOOL cfile_seg_fwable(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg);

EC_BOOL cfile_seg_fxable(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg);

UINT32 cfile_seg_fread(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, KBUFF *kbuff);

UINT32 cfile_seg_fwrite(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg, const KBUFF *kbuff);

UINT32 cfile_seg_fappend(const UINT32 cfile_md_id, CFILE_SEG *cfile_seg, const KBUFF *kbuff, UINT32 *pos);

UINT32 cfile_seg_rmv(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg);

UINT32 cfile_seg_clone(const UINT32 cfile_md_id, const CFILE_SEG *src_cfile_seg, CFILE_SEG *des_cfile_seg);

UINT32 cfile_seg_copy(const UINT32 cfile_md_id, const CFILE_SEG *src_cfile_seg, const CFILE_SEG *des_cfile_seg);

EC_BOOL cfile_seg_cmp(const UINT32 cfile_md_id, const CFILE_SEG *cfile_seg_1st, const CFILE_SEG *cfile_seg_2nd);

/*---------------------------------- CFILE_SEG_VEC file operation interface ----------------------------------*/
/**
*
*   CVECTOR create all file segments at remote
*
**/
UINT32 cfile_seg_vec_fcreate(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec);

/**
*
*   CFILE_SEG_VEC check all file segments existing
*
**/
EC_BOOL cfile_seg_vec_fexist(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec);

/**
*
*   CFILE_SEG_VEC check all file segments readable
*
**/
EC_BOOL cfile_seg_vec_frable(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec);

/*
*
*   CFILE_SEG_VEC check all file segments writable
*
**/
EC_BOOL cfile_seg_vec_fwable(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec);

/*
*
*   CFILE_SEG_VEC check all file segments executable
*
**/
EC_BOOL cfile_seg_vec_fxable(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec);

/**
*
*   CFILE_SEG_VEC read all file segments, update all private fp and update all private KBUFF
*
**/
UINT32 cfile_seg_vec_fread(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const UINT32 cfile_seg_pos_beg, const UINT32 cfile_seg_pos_end, CVECTOR *kbuff_vec);

/**
*
*   CFILE_SEG_VEC write all file segments, update all private fp and update all private KBUFF
*
**/
UINT32 cfile_seg_vec_fwrite(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const UINT32 cfile_seg_pos_beg, const UINT32 cfile_seg_pos_end, const CVECTOR *kbuff_vec);

/**
*
*   CFILE_SEG_VEC append all file segments, update all private fp and update all private append_pos
*
**/
UINT32 cfile_seg_vec_fappend(const UINT32 cfile_md_id, CVECTOR *cfile_seg_vec);

/**
*
*   CFILE_SEG_VEC remove all file segments
*
**/
UINT32 cfile_seg_vec_rmv(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec);

/**
*
*   CFILE_SEG_VEC remove all file segments and their folder
*
**/
UINT32 cfile_seg_vec_dir_rmv(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec, const CSTRING *node_name);

/**
*
*   CFILE_SEG_VEC clone all file segments
*
**/
UINT32 cfile_seg_vec_clone(const UINT32 cfile_md_id, const CVECTOR *src_cfile_seg_vec, CVECTOR *des_cfile_seg_vec);

/**
*
*   CFILE_SEG_VEC copy all data segments
*
**/
UINT32 cfile_seg_vec_copy(const UINT32 cfile_md_id, const CVECTOR *src_cfile_seg_vec, const CVECTOR *des_cfile_seg_vec);

EC_BOOL cfile_seg_vec_cmp(const UINT32 cfile_md_id, const CVECTOR *cfile_seg_vec_1st, const CVECTOR *cfile_seg_vec_2nd);

/*---------------------------------- CFILE Module file operation interface ----------------------------------*/

/**
*
* query num of seg data files of the node
*
**/
UINT32 cfile_seg_num(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

/**
*
* alloc kbuff_num KBUFF and return them in vector
*
*
**/
CVECTOR * cfile_kbuff_vec_new(const UINT32 cfile_md_id, const UINT32 kbuff_num);

/**
*
* clean all KBUFF in vector
*
**/
UINT32 cfile_kbuff_vec_clean(const UINT32 cfile_md_id, CVECTOR *kbuff_vec);

/**
*
* reset all KBUFF in vector
*
**/
UINT32 cfile_kbuff_vec_reset(const UINT32 cfile_md_id, CVECTOR *kbuff_vec);

/**
*
* free all KBUFF in vector and free vector itself
*
**/
UINT32 cfile_kbuff_vec_free(const UINT32 cfile_md_id, CVECTOR *kbuff_vec);

/*-------------------------------------------- external interface --------------------------------------------*/
/**
*
*   CFILE Module create file with CFILE_NODE at local and all file segments at remote
*
**/
UINT32 cfile_fcreate_on_node_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, const UINT32 node_tcid);

/**
*
*   CFILE Module open file with CFILE_NODE at local and set mode to each CFILE_SEG
*
**/
UINT32 cfile_fopen_on_node_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, const UINT32 mode, const UINT32 node_tcid);

/**
*
*   CFILE Module close file with CFILE_NODE at local and CFILE_SEG_VEC at remote
*
**/
UINT32 cfile_fclose_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 node_tcid);

/**
*
*   CFILE Module check file existing/readable/writable/executable or their bit 'and'('&') combination
*
**/
EC_BOOL cfile_fcheck_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 mask, const UINT32 node_tcid);

/**
*
*   CFILE Module check file existing at local
*
**/
EC_BOOL cfile_fsearch_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, UINT32 *node_tcid);

/**
*
*   CFILE Module read file with CFILE_SEG_VEC at remote
*   note: cfile_node should initialize KBUFF in each CFILE_SEG
*
**/
UINT32 cfile_fread_on_node_tcid(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, CVECTOR *kbuff_vec, const UINT32 node_tcid);

/**
*
*   CFILE Module write file with CFILE_SEG_VEC at remote
*   note: cfile_node should initialize KBUFF in each CFILE_SEG
*
**/
UINT32 cfile_fwrite_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const CVECTOR *kbuff_vec, const UINT32 node_tcid);

/**
*
*   CFILE Module remove file with CFILE_SEG_VEC at remote
*
**/
UINT32 cfile_frmv_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 node_tcid);

/**
*
*   CFILE Module clone file with CFILE_SEG_VEC at remote
*
**/
UINT32 cfile_fclone_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *src_cfile_node, CFILE_NODE *des_cfile_node, const UINT32 node_tcid);

/**
*
*   CFILE Module copy file with CFILE_SEG_VEC at remote
*
**/
UINT32 cfile_fcopy_on_node_tcid(const UINT32 cfile_md_id, const CFILE_NODE *src_cfile_node, const CFILE_NODE *des_cfile_node, const UINT32 node_tcid);

/**
*
*   CFILE Module upload file to remote
*
**/
UINT32 cfile_upload(const UINT32 cfile_md_id, const CSTRING *in_file_name, const CSTRING *node_dir_name, const UINT32 node_tcid);

/**
*
*   CFILE Module download file from remote
*
**/
UINT32 cfile_download(const UINT32 cfile_md_id, const CSTRING *in_file_name, const CSTRING *out_file_name, const UINT32 node_tcid);

/**
*
*   CFILE Module remove file at local and remote
*
**/
UINT32 cfile_rmv_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *file_name, const UINT32 node_tcid);

/**
*
*   CFILE Module clone file at local and remote
*
**/
UINT32 cfile_clone_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *src_file_name, const CSTRING *des_file_name, const UINT32 node_tcid);

/**
*
*   CFILE Module copy file at local and remote
*
**/
UINT32 cfile_copy_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *src_file_name, const CSTRING *des_file_name, const UINT32 node_tcid);

/**
*
*   CFILE Module search file at local and remote
*
**/
EC_BOOL cfile_search_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *file_name, UINT32 *node_tcid);

EC_BOOL cfile_cmp_on_node_tcid(const UINT32 cfile_md_id, const CSTRING *file_name_1st, const CSTRING *file_name_2nd, const UINT32 node_tcid);

/**
*
* create a transparent node xml file and its seg data files in CFILE_SEG pool
*
**/
UINT32 cfile_fcreate_trans(const UINT32 cfile_md_id, CFILE_NODE *cfile_node);

/**
*
* open a transparent node xml file with specific open mode
*
**/
UINT32 cfile_fopen_trans(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, const UINT32 mode);

/**
*
* close a transparent node xml file
*
**/
UINT32 cfile_fclose_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

/**
*
* check transparent CFILE_NODE check CFILE_NODE existing/readable/writable/executable or their bit 'and'('&') combination
*
**/
EC_BOOL cfile_fcheck_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const UINT32 mode);

/**
*
* search tranparent node xml file and return one tcid which own it if possible
*
**/
EC_BOOL cfile_fsearch_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, UINT32 *node_tcid);

/**
*
* read transparent node
*
**/
UINT32 cfile_fread_trans(const UINT32 cfile_md_id, CFILE_NODE *cfile_node, CVECTOR *kbuff_vec);

/**
*
* write transparent node
*
**/
UINT32 cfile_fwrite_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node, const CVECTOR *kbuff_vec);

/**
*
* remove transparent node
*
**/
UINT32 cfile_frmv_trans(const UINT32 cfile_md_id, const CFILE_NODE *cfile_node);

/**
*
* copy transparent node
*
**/
UINT32 cfile_fcopy_trans(const UINT32 cfile_md_id, const CFILE_NODE *src_cfile_node, const CFILE_NODE *des_cfile_node);

/**
*
* remove a transparent file
*
**/
UINT32 cfile_rmv_trans(const UINT32 cfile_md_id, const CSTRING *file_name);

/**
*
* copy src transparent file to des file
*
**/
UINT32 cfile_copy_trans(const UINT32 cfile_md_id, const CSTRING *src_file_name, const CSTRING *des_file_name, const UINT32 des_node_tcid);

/**
*
* search transparent file and return the tcid owning it
*
**/
EC_BOOL cfile_search_trans(const UINT32 cfile_md_id, const CSTRING *file_name, UINT32 *node_tcid);

/**
*
* compare two transparent files
*
**/
EC_BOOL cfile_cmp_trans(const UINT32 cfile_md_id, const CSTRING *file_name_1st, const CSTRING *file_name_2nd);

#endif /*_CFILE_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

