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
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "typeconst.h"
#include "type.h"
#include "cmisc.h"
#include "clist.h"
#include "task.h"
#include "mod.h"
#include "log.h"
#include "debug.h"
#include "rank.h"

#include "cmpic.inc"

#include "cstring.h"
#include "cvector.h"

#include "findex.inc"

#include "super.h"
#include "cbgt.h"
#include "cscore.h"
#include "demo_cscore.h"

extern void test_case_cbgt_create_table_on_root(const UINT32 cbgt_md_id, const char *user_table_name, const int colf_num, ...);

static void __test_case_cscore_csdoc_set_doc(const UINT32 cscore_md_id, void *csdoc, const UINT32 doc_id, const UINT32 doc_type, const UINT32 doc_code)
{
    cscore_csdoc_set_doc_id  (cscore_md_id, csdoc, doc_id);
    cscore_csdoc_set_doc_type(cscore_md_id, csdoc, doc_type);
    cscore_csdoc_set_doc_code(cscore_md_id, csdoc, doc_code);
    return;
}

static void __test_case_cscore_csdoc_words_add_words(const UINT32 cscore_md_id, void *csdoc_words, const UINT32 word_num, ...)
{
    UINT32 pos;
    va_list ap;

    va_start(ap, word_num);
    for(pos = 0; pos < word_num; pos ++)
    {
        UINT8 *word_name;
        void *csword;

        word_name = va_arg(ap, UINT8 *);

        csword = cscore_csword_make_by_str_content(cscore_md_id, word_name);
        ASSERT(NULL_PTR != csword);
        cscore_csdoc_words_push_word(cscore_md_id, csdoc_words, csword);        
    }
    va_end(ap);
    return;
}

static void __test_case_cscore_csword_docs_set_word(const UINT32 cscore_md_id, void *csword_docs, const UINT8 *word_name)
{
    void *csword;
    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)word_name);
    ASSERT(NULL_PTR != csword);
    cscore_csword_docs_set_word(cscore_md_id, csword_docs, csword);
    cscore_csword_free(cscore_md_id, csword);
    return;
}

static void __test_case_cscore_csword_list_add_word(const UINT32 cscore_md_id, void *csword_list, const UINT8 *word_name)
{
    void *csword;

    csword = cscore_csword_make_by_str_content(cscore_md_id, word_name);
    ASSERT(NULL_PTR != csword);
    clist_push_back(csword_list, (void *)csword);
    return;
}

void test_case_cscore_001(const UINT32 cscore_md_id)
{
    void *csdoc_words;
    void *csword_docs;
    void *csdoc;
    void *csword;
   
    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"hansoul");
    ASSERT(NULL_PTR != csword);

    csdoc_words = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words);
    cscore_csdoc_words_add_word(cscore_md_id, csdoc_words, csword);    

    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words);
    cscore_csdoc_set_doc_id  (cscore_md_id, csdoc, 1001);
    cscore_csdoc_set_doc_type(cscore_md_id, csdoc, 1002);
    cscore_csdoc_set_doc_code(cscore_md_id, csdoc, 1003);

    ASSERT(EC_TRUE == cscore_csdoc_words_import(cscore_md_id, csdoc_words));
    cscore_csdoc_words_free(cscore_md_id, csdoc_words);

    csword_docs = cscore_csword_docs_new(cscore_md_id);
    ASSERT(NULL_PTR != csword_docs);
    cscore_csword_docs_set_word(cscore_md_id, csword_docs, csword);

    ASSERT(EC_TRUE == cscore_csword_docs_export(cscore_md_id, CBGT_SELECT_FROM_ALL_TABLE, csword_docs));
    cscore_csword_docs_print(LOGSTDOUT, csword_docs);
    cscore_csword_docs_free(cscore_md_id, csword_docs);

    cscore_csword_free(cscore_md_id, csword);

    return;
}

void test_case_cscore_002(const UINT32 cscore_md_id)
{
    void *csdoc_words;
    void *csword_docs;
    void *csdoc;
    void *csword;

    /*make csdoc_words ready*/
    csdoc_words = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words);

    /*set csdoc of csdoc_words*/
    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words);
    cscore_csdoc_set_doc_id  (cscore_md_id, csdoc, 1001);
    cscore_csdoc_set_doc_type(cscore_md_id, csdoc, 1002);
    cscore_csdoc_set_doc_code(cscore_md_id, csdoc, 1003);

    /*add cswords to csdoc_words*/
    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"pepole");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_add_word(cscore_md_id, csdoc_words, csword);

    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"repbulic");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_add_word(cscore_md_id, csdoc_words, csword);

    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"of");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_add_word(cscore_md_id, csdoc_words, csword);    

    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"china");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_add_word(cscore_md_id, csdoc_words, csword);      

    /*import csdoc_words into bigtable*/
    ASSERT(EC_TRUE == cscore_csdoc_words_import(cscore_md_id, csdoc_words));
    cscore_csdoc_words_free(cscore_md_id, csdoc_words);

    /*make csword_docs ready*/
    csword_docs = cscore_csword_docs_new(cscore_md_id);
    ASSERT(NULL_PTR != csword_docs);

    /*add cswords to csword_docs*/
    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"pepole");
    ASSERT(NULL_PTR != csword);
    cscore_csword_docs_set_word(cscore_md_id, csword_docs, csword);
    cscore_csword_free(cscore_md_id, csword);

    ASSERT(EC_TRUE == cscore_csword_docs_export(cscore_md_id, CBGT_SELECT_FROM_ALL_TABLE, csword_docs));
    cscore_csword_docs_print(LOGSTDOUT, csword_docs);
    cscore_csword_docs_free(cscore_md_id, csword_docs);

    return;
}

void test_case_cscore_003(const UINT32 cscore_md_id)
{
    void *csdoc_words_1;
    void *csdoc_words_2;
    void *csdoc_words_list;
    void *csword_docs;
    void *csdoc;
    void *csword;

    csdoc_words_list = clist_new(MM_CSDOC_WORDS, 0);
    ASSERT(NULL_PTR != csdoc_words_list);

    /*make csdoc_words_1 ready*/
    csdoc_words_1 = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words_1);

    /*{doc={1001,1002,1003}, words={pepople, republic, of, china}}*/
    /*set csdoc of csdoc_words_1*/
    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words_1);
    cscore_csdoc_set_doc_id  (cscore_md_id, csdoc, 1001);
    cscore_csdoc_set_doc_type(cscore_md_id, csdoc, 1002);
    cscore_csdoc_set_doc_code(cscore_md_id, csdoc, 1003);

    /*add cswords to csdoc_words_1*/
    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"pepole");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_push_word(cscore_md_id, csdoc_words_1, csword);

    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"republic");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_push_word(cscore_md_id, csdoc_words_1, csword);

    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"of");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_push_word(cscore_md_id, csdoc_words_1, csword);    

    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"china");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_push_word(cscore_md_id, csdoc_words_1, csword);   

    clist_push_back(csdoc_words_list, (void *)csdoc_words_1);

    /*make csdoc_words_2 ready*/
    csdoc_words_2 = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words_2);

    /*{doc={1101,1002,1003}, words={pepople, working, hard}}*/
    /*set csdoc of csdoc_words_2*/
    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words_2);
    cscore_csdoc_set_doc_id  (cscore_md_id, csdoc, 1101);
    cscore_csdoc_set_doc_type(cscore_md_id, csdoc, 1002);
    cscore_csdoc_set_doc_code(cscore_md_id, csdoc, 1003);

    /*add cswords to csdoc_words_2*/
    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"pepole");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_push_word(cscore_md_id, csdoc_words_2, csword);

    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"working");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_push_word(cscore_md_id, csdoc_words_2, csword);

    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"hard");
    ASSERT(NULL_PTR != csword);
    cscore_csdoc_words_push_word(cscore_md_id, csdoc_words_2, csword);    

    clist_push_back(csdoc_words_list, (void *)csdoc_words_2);

    /*import csdoc_words_1 into bigtable*/
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_003: import beg\n");
    ASSERT(EC_TRUE == cscore_csdoc_words_list_import(cscore_md_id, csdoc_words_list));
    clist_free_with_modi(csdoc_words_list, cscore_md_id);
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_003: import end\n");

    /*make csword_docs ready*/
    csword_docs = cscore_csword_docs_new(cscore_md_id);
    ASSERT(NULL_PTR != csword_docs);

    /*add cswords to csword_docs*/
    csword = cscore_csword_make_by_str_content(cscore_md_id, (UINT8 *)"pepole");
    ASSERT(NULL_PTR != csword);
    cscore_csword_docs_set_word(cscore_md_id, csword_docs, csword);
    cscore_csword_free(cscore_md_id, csword);

    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_003: export beg\n");
    ASSERT(EC_TRUE == cscore_csword_docs_export(cscore_md_id, CBGT_SELECT_FROM_ALL_TABLE, csword_docs));
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_003: export end\n");
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_003: export result:\n");
    cscore_csword_docs_print(LOGSTDOUT, csword_docs);
    cscore_csword_docs_free(cscore_md_id, csword_docs);

    return;
}

void test_case_cscore_004(const UINT32 cscore_md_id)
{
    void *csdoc_words_1;
    void *csdoc_words_2;
    void *csdoc_words_3;
    void *csdoc_words_list;
    void *csword_docs;
    void *csdoc;

    csdoc_words_list = clist_new(MM_CSDOC_WORDS, 0);
    ASSERT(NULL_PTR != csdoc_words_list);

    /*make csdoc_words_1 = {doc={1001,1002,1003}, words={pepople, republic, of, china}}*/
    csdoc_words_1 = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words_1);
    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words_1);
    __test_case_cscore_csdoc_set_doc(cscore_md_id, csdoc, 1001, 1002, 1003);
    __test_case_cscore_csdoc_words_add_words(cscore_md_id, csdoc_words_1, 4, (UINT8 *)"pepole", (UINT8 *)"republic", (UINT8 *)"of", (UINT8 *)"china");
    clist_push_back(csdoc_words_list, (void *)csdoc_words_1);

    /*make csdoc_words_2 = {doc={1101,1002,1003}, words={pepople, working, hard}}*/
    csdoc_words_2 = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words_2);    
    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words_2);
    __test_case_cscore_csdoc_set_doc(cscore_md_id, csdoc, 1101, 1002, 1003);
    __test_case_cscore_csdoc_words_add_words(cscore_md_id, csdoc_words_2, 3, (UINT8 *)"pepole", (UINT8 *)"working", (UINT8 *)"hard");
    clist_push_back(csdoc_words_list, (void *)csdoc_words_2);

    /*make csdoc_words_3 = {doc={1101,1002,1003}, words={pepople, living, happ}}*/
    csdoc_words_3 = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words_3);    
    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words_3);
    __test_case_cscore_csdoc_set_doc(cscore_md_id, csdoc, 1011, 1002, 1003);
    __test_case_cscore_csdoc_words_add_words(cscore_md_id, csdoc_words_3, 3, (UINT8 *)"pepole", (UINT8 *)"living", (UINT8 *)"happ");
    clist_push_back(csdoc_words_list, (void *)csdoc_words_3);

    /*import csdoc_words_1 into bigtable*/
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_004: import beg\n");
    ASSERT(EC_TRUE == cscore_csdoc_words_list_import(cscore_md_id, csdoc_words_list));
    clist_free_with_modi(csdoc_words_list, cscore_md_id);
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_004: import end\n");

    /*make csword_docs ready*/
    csword_docs = cscore_csword_docs_new(cscore_md_id);
    ASSERT(NULL_PTR != csword_docs);

    /*add cswords to csword_docs*/
    __test_case_cscore_csword_docs_set_word(cscore_md_id, csword_docs, (UINT8 *)"pepole");

    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_004: export beg\n");
    ASSERT(EC_TRUE == cscore_csword_docs_export(cscore_md_id, CBGT_SELECT_FROM_ALL_TABLE, csword_docs));
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_004: export end\n");
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_004: export result:\n");
    cscore_csword_docs_print(LOGSTDOUT, csword_docs);
    cscore_csword_docs_free(cscore_md_id, csword_docs);

    return;
}

void test_case_cscore_005(const UINT32 cscore_md_id)
{
    void *csdoc_words_1;
    void *csdoc_words_2;
    void *csdoc_words_3;
    void *csdoc_words_list;
    void *csword_list;
    void *csdoc_list;
    void *csdoc;

    csdoc_words_list = clist_new(MM_CSDOC_WORDS, 0);
    ASSERT(NULL_PTR != csdoc_words_list);

    /*make csdoc_words_1 = {doc={1001,1002,1003}, words={pepople, republic, of, china}}*/
    csdoc_words_1 = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words_1);
    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words_1);
    __test_case_cscore_csdoc_set_doc(cscore_md_id, csdoc, 1001, 1002, 1003);
    __test_case_cscore_csdoc_words_add_words(cscore_md_id, csdoc_words_1, 4, (UINT8 *)"pepole", (UINT8 *)"republic", (UINT8 *)"of", (UINT8 *)"china");
    clist_push_back(csdoc_words_list, (void *)csdoc_words_1);

    /*make csdoc_words_2 = {doc={1101,1002,1003}, words={pepople, working, hard}}*/
    csdoc_words_2 = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words_2);    
    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words_2);
    __test_case_cscore_csdoc_set_doc(cscore_md_id, csdoc, 1101, 1002, 1003);
    __test_case_cscore_csdoc_words_add_words(cscore_md_id, csdoc_words_2, 3, (UINT8 *)"pepole", (UINT8 *)"working", (UINT8 *)"hard");
    clist_push_back(csdoc_words_list, (void *)csdoc_words_2);

    /*make csdoc_words_3 = {doc={1101,1002,1003}, words={pepople, living, happ}}*/
    csdoc_words_3 = cscore_csdoc_words_new(cscore_md_id);
    ASSERT(NULL_PTR != csdoc_words_3);    
    csdoc = (void *)cscore_csdoc_words_get_doc(cscore_md_id, csdoc_words_3);
    __test_case_cscore_csdoc_set_doc(cscore_md_id, csdoc, 1011, 1002, 1003);
    __test_case_cscore_csdoc_words_add_words(cscore_md_id, csdoc_words_3, 3, (UINT8 *)"pepole", (UINT8 *)"living", (UINT8 *)"happy");
    clist_push_back(csdoc_words_list, (void *)csdoc_words_3);

    /*import csdoc_words_1 into bigtable*/
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_005: import beg\n");
    ASSERT(EC_TRUE == cscore_csdoc_words_list_import(cscore_md_id, csdoc_words_list));
    clist_free_with_modi(csdoc_words_list, cscore_md_id);
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_005: import end\n");

    /*make csword_list ready*/
    csword_list = clist_new(MM_CSWORD, 0);
    ASSERT(NULL_PTR != csword_list);
    __test_case_cscore_csword_list_add_word(cscore_md_id, csword_list, (UINT8 *)"pepole");
    __test_case_cscore_csword_list_add_word(cscore_md_id, csword_list, (UINT8 *)"china");

    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_005: csword_list is:\n");
    clist_print(LOGSTDOUT, csword_list, cscore_csword_print);

    /*make csdoc_list ready*/
    csdoc_list = clist_new(MM_CSDOC, 0);
    ASSERT(NULL_PTR != csdoc_list);    

    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_005: export beg\n");
    ASSERT(EC_TRUE == cscore_csword_docs_list_export_docs(cscore_md_id, CBGT_SELECT_FROM_ALL_TABLE, csword_list, csdoc_list));
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_005: export end\n");
    sys_log(LOGSTDOUT, "[DEBUG] test_case_cscore_005: export result:\n");
    clist_print(LOGSTDOUT, csdoc_list, cscore_csdoc_print);

    clist_clean_with_modi(csword_list, cscore_md_id, cscore_csword_free);
    clist_clean_with_modi(csdoc_list, cscore_md_id, cscore_csdoc_free);

    clist_free(csword_list, 0);
    clist_free(csdoc_list, 0);    

    return;
}

int main_cscore0(int argc, char **argv)
{
    UINT32 this_tcid;
    UINT32 this_comm;
    UINT32 this_rank;

    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_cscore: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }

    this_tcid = task_brd_default_get_tcid();
    this_comm = task_brd_default_get_comm();
    this_rank = task_brd_default_get_rank();

    if (EC_TRUE == task_brd_check_is_dbg_tcid(this_tcid) && CMPI_DBG_RANK == this_rank)
    {
        do_cmd_default();
    }
    else if (EC_TRUE == task_brd_check_is_monitor_tcid(this_tcid) && CMPI_MON_RANK == this_rank)
    {
        void * mod_mgr_def;

        mod_mgr_def = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
        mod_mgr_default_init(mod_mgr_def, CMPI_ANY_TCID, CMPI_ANY_RANK);

        //mod_mgr_excl(this_tcid, CMPI_ANY_COMM, this_rank, CMPI_ANY_MODI, mod_mgr_def);

        sys_log(LOGSTDOUT, "======================================================================\n");
        sys_log(LOGSTDOUT, "                       mod_mgr_default_init finished                  \n");
        sys_log(LOGSTDOUT, "======================================================================\n");
        mod_mgr_print(LOGSTDOUT, mod_mgr_def);

        mod_mgr_free(mod_mgr_def);

        do_slave_wait_default();
    }

    else if (c_ipv4_to_word("10.10.10.1") == this_tcid && 1 == this_rank)
    {
        UINT32 cbgt_md_id;

        CSTRING *cbgt_db_root_dir;
        CBYTES   root_table_name;

        cbgt_db_root_dir = task_brd_default_get_hsbgt_root_table_dir();
        ASSERT(NULL_PTR != cbgt_db_root_dir);
        cbytes_mount(&root_table_name, strlen("root"), (UINT8 *)"root");

        cbgt_md_id = cbgt_start(CBGT_TYPE_ROOT_SERVER, CBGT_ROOT_TABLE_ID, &root_table_name, NULL_PTR, cbgt_db_root_dir, CBGT_O_RDWR | CBGT_O_CREAT);
        test_case_cbgt_create_table_on_root(cbgt_md_id,
                                            CSCORE_MD_DEFAULT_TABLE_NAME,
                                            1,
                                            CSCORE_MD_DEFAULT_COLF_NAME
                                            );

        ASSERT(ERR_MODULE_ID != cbgt_md_id);

        sys_log(LOGSTDOUT, "[DEBUG] main_cbgt: ============================ root server started ====================\n");

        do_slave_wait_default();
    }

    /*fwd rank entrance*/
    else if (c_ipv4_to_word("10.10.10.2") == this_tcid && 1 == this_rank)
    {
        UINT32 cscore_md_id;
        sys_log(LOGSTDOUT,"======================================================================\n");
        sys_log(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", c_word_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        sys_log(LOGSTDOUT,"======================================================================\n");

        cscore_md_id = cscore_start();
        ASSERT(ERR_MODULE_ID != cscore_md_id );
        sys_log(LOGSTDOUT, "[DEBUG] main_cscore: cscore_md_id = %ld\n", cscore_md_id);

        //test_case_cscore_001(cscore_md_id);
        //test_case_cscore_002(cscore_md_id);
        //test_case_cscore_003(cscore_md_id);
        //test_case_cscore_004(cscore_md_id);
        test_case_cscore_005(cscore_md_id);

        cscore_end(cscore_md_id);

        do_slave_wait_default();
    }

    /*work process*/
    else
    {
        do_slave_wait_default();
    }

    return (0);
}

EC_BOOL __test_cscore_cbgt_root_server_runner()
{
    UINT32 cbgt_md_id;

    CSTRING *cbgt_db_root_dir;
    CBYTES   root_table_name;

    cbgt_db_root_dir = task_brd_default_get_hsbgt_root_table_dir();
    ASSERT(NULL_PTR != cbgt_db_root_dir);
    
    cbytes_mount(&root_table_name, strlen("root"), (UINT8 *)"root");

    cbgt_md_id = cbgt_start(CBGT_TYPE_ROOT_SERVER, CBGT_ROOT_TABLE_ID, &root_table_name, NULL_PTR, cbgt_db_root_dir, CBGT_O_RDWR | CBGT_O_CREAT);
    test_case_cbgt_create_table_on_root(cbgt_md_id,
                                        CSCORE_MD_DEFAULT_TABLE_NAME,
                                        1,
                                        CSCORE_MD_DEFAULT_COLF_NAME
                                        );

    ASSERT(ERR_MODULE_ID != cbgt_md_id);

    sys_log(LOGSTDOUT, "[DEBUG] __test_cscore_cbgt_root_server_runner: ============================ root server started ====================\n");

    return (EC_TRUE);
}

EC_BOOL __test_cscore_cases_runner()
{    
    UINT32 cscore_md_id;

    cscore_md_id = cscore_start();
    ASSERT(ERR_MODULE_ID != cscore_md_id );
    sys_log(LOGSTDOUT, "[DEBUG] __test_cscore_cases_runner: cscore_md_id = %ld\n", cscore_md_id);

    //test_case_cscore_001(cscore_md_id);
    //test_case_cscore_002(cscore_md_id);
    //test_case_cscore_003(cscore_md_id);
    //test_case_cscore_004(cscore_md_id);
    test_case_cscore_005(cscore_md_id);

    cscore_end(cscore_md_id);

    return (EC_TRUE);
}

int main_cscore(int argc, char **argv)
{
    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_cscore: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }

    /*define specific runner for each (tcid, rank)*/
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.1"), 1, __test_cscore_cbgt_root_server_runner);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.2"), 1, __test_cscore_cases_runner);

    /*start the defined runner on current (tcid, rank)*/
    task_brd_default_start_runner();

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

