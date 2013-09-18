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

#ifndef _CSCORE_H
#define _CSCORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "mod.inc"

#include "clist.h"
#include "cstring.h"
#include "cbytes.h"
#include "chashalgo.h"

#include "croutine.h"

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

#define CSCORE_COLQ_WORD_TEXT_IDX            ((UINT32) 0)
#define CSCORE_COLQ_DOC_ID_IDX               ((UINT32) 1)
#define CSCORE_COLQ_DOC_TYPE_IDX             ((UINT32) 2)
#define CSCORE_COLQ_DOC_CODE_IDX             ((UINT32) 3)
#define CSCORE_COLQ_END_IDX                  ((UINT32) 4)

typedef struct
{
    CROUTINE_MUTEX      cmutex;
    UINT32      beg_row_no;
    UINT32      end_row_no;
    UINT32      cur_row_no;
}CSCORE_ROW_NO_POOL;

#define CSCORE_ROW_NO_POOL_CMUTEX(cscore_row_no_pool)        (&((cscore_row_no_pool)->cmutex))
#define CSCORE_ROW_NO_POOL_BEG(cscore_row_no_pool)           ((cscore_row_no_pool)->beg_row_no)
#define CSCORE_ROW_NO_POOL_END(cscore_row_no_pool)           ((cscore_row_no_pool)->end_row_no)
#define CSCORE_ROW_NO_POOL_CUR(cscore_row_no_pool)           ((cscore_row_no_pool)->cur_row_no)

#define CSCORE_ROW_NO_POOL_INIT_CMUTEX(cscore_row_no_pool, location)     (croutine_mutex_init(CSCORE_ROW_NO_POOL_CMUTEX(cscore_row_no_pool), CMUTEX_PROCESS_PRIVATE, location))
#define CSCORE_ROW_NO_POOL_CLEAN_CMUTEX(cscore_row_no_pool, location)    (croutine_mutex_clean(CSCORE_ROW_NO_POOL_CMUTEX(cscore_row_no_pool), location))
#define CSCORE_ROW_NO_POOL_CMUTEX_LOCK(cscore_row_no_pool, location)     (croutine_mutex_lock(CSCORE_ROW_NO_POOL_CMUTEX(cscore_row_no_pool), location))
#define CSCORE_ROW_NO_POOL_CMUTEX_UNLOCK(cscore_row_no_pool, location)   (croutine_mutex_unlock(CSCORE_ROW_NO_POOL_CMUTEX(cscore_row_no_pool), location))


typedef struct
{
    /* used counter >= 0 */
    UINT32      usedcounter;

    MOD_MGR    *mod_mgr;

    UINT32      cbgt_md_id;
    UINT32      chash_algo_id;
    CHASH_ALGO  chash_algo;

    CSCORE_ROW_NO_POOL row_no_pool;

    CBYTES      table_name_bytes;
    CBYTES      table_colf_name_bytes;

    CBYTES      table_colq_word_text_bytes;
    CBYTES      table_colq_doc_id_bytes;
    CBYTES      table_colq_doc_type_bytes;
    CBYTES      table_colq_doc_code_bytes;
}CSCORE_MD;

#define CSCORE_MD_MOD_MGR(cscore_md)              ((cscore_md)->mod_mgr)
#define CSCORE_MD_CBGT_MD_ID(cscore_md)           ((cscore_md)->cbgt_md_id)
#define CSCORE_CHASH_ALGO_ID(cscore_md)           ((cscore_md)->chash_algo_id)
#define CSCORE_CHASH_ALGO_FUNC(cscore_md)         ((cscore_md)->chash_algo)
#define CSCORE_MD_ROW_NO_POOL(cscore_md)          (&((cscore_md)->row_no_pool))

#define CSCORE_MD_TABLE_NAME(cscore_md)           (&((cscore_md)->table_name_bytes))
#define CSCORE_MD_TABLE_COLF_NAME(cscore_md)      (&((cscore_md)->table_colf_name_bytes))

#define CSCORE_MD_TABLE_COLQ_WORD_TEXT(cscore_md) (&((cscore_md)->table_colq_word_text_bytes))
#define CSCORE_MD_TABLE_COLQ_DOC_ID(cscore_md)    (&((cscore_md)->table_colq_doc_id_bytes))
#define CSCORE_MD_TABLE_COLQ_DOC_TYPE(cscore_md)  (&((cscore_md)->table_colq_doc_type_bytes))
#define CSCORE_MD_TABLE_COLQ_DOC_CODE(cscore_md)  (&((cscore_md)->table_colq_doc_code_bytes))


#define CSDOC_ERROR_TYPE     ((UINT32)   0)
#define CSDOC_ASCII_TYPE     ((UINT32)   1)
#define CSDOC_BINARY_TYPE    ((UINT32)   2)
#define CSDOC_PDF_TYPE       ((UINT32)   3)
#define CSDOC_JPG_TYPE       ((UINT32)   4)
#define CSDOC_PNG_TYPE       ((UINT32)   5)
#define CSDOC_BITMAP_TYPE    ((UINT32)   6)
#define CSDOC_MSWORD_TYPE    ((UINT32)   7)
#define CSDOC_MSPPT_TYPE     ((UINT32)   8)
#define CSDOC_HTML_TYPE      ((UINT32)   9)

#define CSDOC_ERROR_CODE     ((UINT32)   0)
#define CSDOC_US_UTF8_CODE   ((UINT32)   1)
#define CSDOC_ZH_BGK_CODE    ((UINT32)   2)
#define CSDOC_ZH_BIG5_CODE   ((UINT32)   3)
#define CSDOC_ZH_FANTI_CODE  ((UINT32)   4)

#define CSDOC_ERROR_ID       ((UINT32)  -1)

typedef struct
{
    CSTRING    *doc_name;          /*optional*/
    UINT32      doc_id;            /*mandatory*/
    UINT32      doc_type:16;       /*doc type, CSDOC_XXX_TYPE*/
    UINT32      doc_code:16;       /*language code, CSDOC_XXX_CODE*/
    UINT32      doc_rate:32;       /*doc referred rate*/
    CBYTES     *doc_content;       /*optional*/
}CSDOC;

#define CSDOC_NAME(csdoc)           ((csdoc)->doc_name)
#define CSDOC_ID(csdoc)             ((csdoc)->doc_id)
#define CSDOC_TYPE(csdoc)           ((csdoc)->doc_type)
#define CSDOC_CODE(csdoc)           ((csdoc)->doc_code)
#define CSDOC_RATE(csdoc)           ((csdoc)->doc_rate)
#define CSDOC_CONTENT(csdoc)        ((csdoc)->doc_content)

typedef struct
{
    CBYTES      word_content;          /*always utf-8*/
}CSWORD;

#define CSWORD_CONTENT(csword)         (&((csword)->word_content))

typedef struct
{
    CSDOC        csdoc;
    CLIST        csword_list;          /*item type is CSWORD*/
}CSDOC_WORDS;

#define CSDOC_WORDS_DOC(csdoc_words)       (&((csdoc_words)->csdoc))
#define CSDOC_WORDS_LIST(csdoc_words)      (&((csdoc_words)->csword_list))

typedef struct
{
    CSWORD      csword;
    CLIST       csdoc_list;            /*item type is CSDOC*/
}CSWORD_DOCS;

#define CSWORD_DOCS_WORD(csword_docs)      (&((csword_docs)->csword))
#define CSWORD_DOCS_LIST(csword_docs)      (&((csword_docs)->csdoc_list))

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

UINT32  cscore_reserve_row_no(const UINT32 cscore_md_id);

EC_BOOL cscore_release_row_no(const UINT32 cscore_md_id, const UINT32 row_no);

CSDOC * cscore_csdoc_new(const UINT32 cscore_md_id);

EC_BOOL cscore_csdoc_init(const UINT32 cscore_md_id, CSDOC *csdoc);

EC_BOOL cscore_csdoc_clean(const UINT32 cscore_md_id, CSDOC *csdoc);

EC_BOOL cscore_csdoc_free(const UINT32 cscore_md_id, CSDOC *csdoc);

EC_BOOL cscore_csdoc_clone(const UINT32 cscore_md_id, const CSDOC *csdoc_src, CSDOC *csdoc_des);;

EC_BOOL cscore_csdoc_rate_lt(const UINT32 cscore_md_id, const CSDOC *csdoc_1st, const CSDOC *csdoc_2nd);

EC_BOOL cscore_csdoc_rate_le(const UINT32 cscore_md_id, const CSDOC *csdoc_1st, const CSDOC *csdoc_2nd);

EC_BOOL cscore_csdoc_rate_gt(const UINT32 cscore_md_id, const CSDOC *csdoc_1st, const CSDOC *csdoc_2nd);

EC_BOOL cscore_csdoc_rate_ge(const UINT32 cscore_md_id, const CSDOC *csdoc_1st, const CSDOC *csdoc_2nd);

EC_BOOL cscore_csdoc_set_name(const UINT32 cscore_md_id, CSDOC *csdoc, const CSTRING *doc_name);

EC_BOOL cscore_csdoc_set_doc_id(const UINT32 cscore_md_id, CSDOC *csdoc, const UINT32 doc_id);

EC_BOOL cscore_csdoc_set_doc_type(const UINT32 cscore_md_id, CSDOC *csdoc, const UINT32 doc_type);

EC_BOOL cscore_csdoc_set_doc_code(const UINT32 cscore_md_id, CSDOC *csdoc, const UINT32 doc_code);

EC_BOOL cscore_csdoc_set_doc_content(const UINT32 cscore_md_id, CSDOC *csdoc, const CBYTES *doc_content);

void    cscore_csdoc_print(LOG *log, const CSDOC *csdoc);

CSWORD *cscore_csword_new(const UINT32 cscore_md_id, const UINT32 location);

EC_BOOL cscore_csword_init(const UINT32 cscore_md_id, CSWORD *csword);

EC_BOOL cscore_csword_clean(const UINT32 cscore_md_id, CSWORD *csword);

EC_BOOL cscore_csword_free(const UINT32 cscore_md_id, CSWORD *csword);

EC_BOOL cscore_csword_make_by_cbytes_content(const UINT32 cscore_md_id, CSWORD *csword, const CBYTES *word_content);

CSWORD *cscore_csword_make_by_str_content(const UINT32 cscore_md_id, const UINT8 *word_content_str);

CSWORD *cscore_csword_make_by_cstr_content(const UINT32 cscore_md_id, const CSTRING *word_content_cstr);

EC_BOOL cscore_csword_clone(const UINT32 cscore_md_id, const CSWORD *csword_src, CSWORD *csword_des);

void    cscore_csword_print(LOG *log, const CSWORD *csword);

CSDOC_WORDS *cscore_csdoc_words_new(const UINT32 cscore_md_id);

EC_BOOL cscore_csdoc_words_init(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words);

EC_BOOL cscore_csdoc_words_clean(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words);

EC_BOOL cscore_csdoc_words_free(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words);

void    cscore_csdoc_words_print(LOG *log, const CSDOC_WORDS *csdoc_words);

const CSDOC *cscore_csdoc_words_get_doc(const UINT32 cscore_md_id, const CSDOC_WORDS *csdoc_words);

const CLIST *cscore_csdoc_words_get_word_list(const UINT32 cscore_md_id, const CSDOC_WORDS *csdoc_words);

EC_BOOL cscore_csdoc_words_push_word(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words, const CSWORD *csword);

EC_BOOL cscore_csdoc_words_add_word(const UINT32 cscore_md_id, CSDOC_WORDS *csdoc_words, const CSWORD *csword);

CSWORD_DOCS *cscore_csword_docs_new(const UINT32 cscore_md_id);

EC_BOOL cscore_csword_docs_init(const UINT32 cscore_md_id, CSWORD_DOCS *csword_docs);

EC_BOOL cscore_csword_docs_clean(const UINT32 cscore_md_id, CSWORD_DOCS *csword_docs);

EC_BOOL cscore_csword_docs_free(const UINT32 cscore_md_id, CSWORD_DOCS *csword_docs);

EC_BOOL cscore_csword_docs_set_word(const UINT32 cscore_md_id, CSWORD_DOCS *csword_docs, const CSWORD *csword);

void    cscore_csword_docs_print(LOG *log, const CSWORD_DOCS *csword_docs);


/*import doc and its word tokens into bigtable*/
EC_BOOL cscore_csdoc_words_import(const UINT32 cscore_md_id, const CSDOC_WORDS *csdoc_words);

EC_BOOL cscore_csdoc_words_list_import(const UINT32 cscore_md_id, const CLIST *csdoc_words_clist);

/*export word and its docs from bigtable*/
/*note: csword_docs is IO parameter*/
EC_BOOL cscore_csword_docs_export(const UINT32 cscore_md_id, const UINT32 cached_mode, CSWORD_DOCS *csword_docs);

EC_BOOL cscore_csword_docs_list_export(const UINT32 cscore_md_id, const UINT32 cached_mode, CLIST *csword_docs_list);

EC_BOOL cscore_csword_docs_list_export_docs(const UINT32 cscore_md_id, const UINT32 cached_mode, const CLIST *csword_list, CLIST *csdoc_list);

EC_BOOL cscore_csword_docs_list_merge(const UINT32 cscore_md_id, const CLIST *csword_docs_list, CLIST *csdoc_list);


#endif/*_CSCORE_H*/
#ifdef __cplusplus
}
#endif/*__cplusplus*/

