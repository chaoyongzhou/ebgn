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

#ifndef _LIB_CSCORE_H
#define _LIB_CSCORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_type.h"
#include "lib_mm.h"
#include "lib_log.h"

#include "lib_cstring.h"
#include "lib_cbytes.h"
#include "lib_chashalgo.h"

#define CSCORE_MD_DEFAULT_CHASH_ALGO_ID      (CHASH_CRC_ALGO_ID)

#define CSCORE_MD_DEFAULT_BEG_ROW_NO         ((UINT32) 1)
#define CSCORE_MD_DEFAULT_MAX_ROW_NUM        ((UINT32) 999)
#define CSCORE_MD_DEFAULT_ERR_ROW_NO         ((UINT32) 0)

#define CSCORE_MD_DEFAULT_TABLE_NAME         ((char *)"usutf8words")
#define CSCORE_MD_DEFAULT_TABLE_NAME_LEN     (strlen(CSCORE_MD_DEFAULT_TABLE_NAME))

#define CSCORE_MD_DEFAULT_COLF_NAME         ((char *)"docwords")
#define CSCORE_MD_DEFAULT_COLF_NAME_LEN     (strlen(CSCORE_MD_DEFAULT_COLF_NAME))

#define CSCORE_MD_DEFAULT_COLQ_WORD_TEXT     ((char *)"wordtext")
#define CSCORE_MD_DEFAULT_COLQ_DOC_ID        ((char *)"docid")
#define CSCORE_MD_DEFAULT_COLQ_DOC_TYPE      ((char *)"doctype")
#define CSCORE_MD_DEFAULT_COLQ_DOC_CODE      ((char *)"doccode")

#define CSCORE_MD_DEFAULT_COLQ_WORD_TEXT_LEN (strlen(CSCORE_MD_DEFAULT_COLQ_WORD_TEXT))
#define CSCORE_MD_DEFAULT_COLQ_DOC_ID_LEN    (strlen(CSCORE_MD_DEFAULT_COLQ_DOC_ID   ))
#define CSCORE_MD_DEFAULT_COLQ_DOC_TYPE_LEN  (strlen(CSCORE_MD_DEFAULT_COLQ_DOC_TYPE ))
#define CSCORE_MD_DEFAULT_COLQ_DOC_CODE_LEN  (strlen(CSCORE_MD_DEFAULT_COLQ_DOC_CODE ))

/**
*   for test only
*
*   to query the status of CSCORE Module
*
**/
void cscore_print_module_status(const UINT32 cscore_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CSCORE module
*
*
**/
UINT32 cscore_free_module_static_mem(const UINT32 cscore_md_id);

/**
*
* start CSCORE module
*
**/
UINT32 cscore_start();

/**
*
* end CSCORE module
*
**/
void cscore_end(const UINT32 cscore_md_id);

void * cscore_csdoc_new(const UINT32 cscore_md_id);

EC_BOOL cscore_csdoc_init(const UINT32 cscore_md_id, void *csdoc);

EC_BOOL cscore_csdoc_clean(const UINT32 cscore_md_id, void *csdoc);

EC_BOOL cscore_csdoc_free(const UINT32 cscore_md_id, void *csdoc);

EC_BOOL cscore_csdoc_clone(const UINT32 cscore_md_id, const void *csdoc_src, void *csdoc_des);

EC_BOOL cscore_csdoc_set_name(const UINT32 cscore_md_id, void *csdoc, const CSTRING *doc_name);

EC_BOOL cscore_csdoc_set_doc_id(const UINT32 cscore_md_id, void *csdoc, const UINT32 doc_id);

EC_BOOL cscore_csdoc_set_doc_type(const UINT32 cscore_md_id, void *csdoc, const UINT32 doc_type);

EC_BOOL cscore_csdoc_set_doc_code(const UINT32 cscore_md_id, void *csdoc, const UINT32 doc_code);

EC_BOOL cscore_csdoc_set_doc_content(const UINT32 cscore_md_id, void *csdoc, const CBYTES *doc_content);

void   cscore_csdoc_print(LOG *log, const void *csdoc);

void * cscore_csword_new(const UINT32 cscore_md_id, const UINT32 location);

EC_BOOL cscore_csword_init(const UINT32 cscore_md_id, void *csword);

EC_BOOL cscore_csword_clean(const UINT32 cscore_md_id, void *csword);

EC_BOOL cscore_csword_free(const UINT32 cscore_md_id, void *csword);

EC_BOOL cscore_csword_make_by_cbytes_content(const UINT32 cscore_md_id, void *csword, const CBYTES *word_content);

void *  cscore_csword_make_by_str_content(const UINT32 cscore_md_id, const UINT8 *word_content_str);

void *  cscore_csword_make_by_cstr_content(const UINT32 cscore_md_id, const CSTRING *word_content_cstr);

EC_BOOL cscore_csword_clone(const UINT32 cscore_md_id, const void *csword_src, void *csword_des);

void   cscore_csword_print(LOG *log, const void *csword);

void * cscore_csdoc_words_new(const UINT32 cscore_md_id);

EC_BOOL cscore_csdoc_words_init(const UINT32 cscore_md_id, void *csdoc_words);

EC_BOOL cscore_csdoc_words_clean(const UINT32 cscore_md_id, void *csdoc_words);

EC_BOOL cscore_csdoc_words_free(const UINT32 cscore_md_id, void *csdoc_words);

void   cscore_csdoc_words_print(LOG *log, const void *csdoc_words);

const void *cscore_csdoc_words_get_doc(const UINT32 cscore_md_id, const void *csdoc_words);

const void *cscore_csdoc_words_get_word_list(const UINT32 cscore_md_id, const void *csdoc_words);

EC_BOOL cscore_csdoc_words_push_word(const UINT32 cscore_md_id, void *csdoc_words, const void *csword);

EC_BOOL cscore_csdoc_words_add_word(const UINT32 cscore_md_id, void *csdoc_words, const void *csword);

void * cscore_csword_docs_new(const UINT32 cscore_md_id);

EC_BOOL cscore_csword_docs_init(const UINT32 cscore_md_id, void *csword_docs);

EC_BOOL cscore_csword_docs_clean(const UINT32 cscore_md_id, void *csword_docs);

EC_BOOL cscore_csword_docs_free(const UINT32 cscore_md_id, void *csword_docs);

EC_BOOL cscore_csword_docs_set_word(const UINT32 cscore_md_id, void *csword_docs, const void *csword);

void   cscore_csword_docs_print(LOG *log, const void *csword_docs);

/*import doc and its word tokens into bigtable*/
EC_BOOL cscore_csdoc_words_import(const UINT32 cscore_md_id, const void *csdoc_words);

EC_BOOL cscore_csdoc_words_list_import(const UINT32 cscore_md_id, const void *csdoc_words_clist);

/*export word and its docs from bigtable*/
/*note: csword_docs is IO parameter*/
EC_BOOL cscore_csword_docs_export(const UINT32 cscore_md_id, const UINT32 cached_mode, void *csword_docs);

EC_BOOL cscore_csword_docs_list_export(const UINT32 cscore_md_id, const UINT32 cached_mode, void *csword_docs_list);

EC_BOOL cscore_csword_docs_list_export_docs(const UINT32 cscore_md_id, const UINT32 cached_mode, const void *csword_list, void *csdoc_list);

EC_BOOL cscore_csword_docs_list_merge(const UINT32 cscore_md_id, const void *csword_docs_list, void *csdoc_list);


#endif/*_LIB_CSCORE_H*/
#ifdef __cplusplus
}
#endif/*__cplusplus*/

