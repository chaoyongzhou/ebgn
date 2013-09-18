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

#include "lib_typeconst.h"
#include "lib_type.h"
#include "lib_char2int.h"
#include "lib_task.h"
#include "lib_mod.h"
#include "lib_log.h"
#include "lib_debug.h"
#include "lib_rank.h"

#include "lib_cstring.h"
#include "lib_cvector.h"

#include "lib_super.h"
#include "lib_tbd.h"
#include "lib_crun.h"

#include "lib_cthread.h"

#include "lib_cmpic.inc"
#include "lib_findex.inc"

#include "lib_chashalgo.h"

#include "lib_cbgt.h"
#include "lib_cdfs.h"
#include "lib_cdfsnp.h"
#include "lib_cbytes.h"

#include "demo.h"

EC_BOOL __test_cbgt_insert_client_1_runner();
EC_BOOL __test_cbgt_insert_client_2_runner();

EC_BOOL __test_cbgt_fetch_client_1_runner();
EC_BOOL __test_cbgt_fetch_client_2_runner();

EC_BOOL __test_cbgt_delete_client_1_runner();
EC_BOOL __test_cbgt_delete_client_2_runner();

void test_case_cbgt_insert_one(const UINT32 cbgt_md_id, const char *table_name, const char *row, const char *colf, const char *colq, const char *val)
{
    CBYTES table_name_bytes;
    CBYTES row_bytes;
    CBYTES colf_bytes;
    CBYTES colq_bytes;
    CBYTES val_bytes;

    cbytes_mount(&table_name_bytes, strlen(table_name), (UINT8 *)table_name);
    cbytes_mount(&row_bytes       , strlen(row)       , (UINT8 *)row       );
    cbytes_mount(&colf_bytes      , strlen(colf)      , (UINT8 *)colf      );
    cbytes_mount(&colq_bytes      , strlen(colq)      , (UINT8 *)colq      );
    cbytes_mount(&val_bytes       , strlen(val)       , (UINT8 *)val       );

    ASSERT(EC_TRUE == cbgt_insert(cbgt_md_id, &table_name_bytes, &row_bytes, &colf_bytes, &colq_bytes, &val_bytes));
    sys_log(LOGSTDOUT, "[test_case_cbgt_insert_one] inserted (%s:%s:%s) in table %s ==> done\n",
                        row, colf, colq, table_name);
    return;
}

void test_case_cbgt_search_one(const UINT32 cbgt_md_id, const char *table_name, const char *row, const char *colf, const char *colq)
{
    CBYTES table_name_bytes;
    CBYTES row_bytes;
    CBYTES colf_bytes;
    CBYTES colq_bytes;
    CBYTES val_bytes;

    cbytes_mount(&table_name_bytes, strlen(table_name), (UINT8 *)table_name);
    cbytes_mount(&row_bytes       , strlen(row)       , (UINT8 *)row       );
    cbytes_mount(&colf_bytes      , strlen(colf)      , (UINT8 *)colf      );
    cbytes_mount(&colq_bytes      , strlen(colq)      , (UINT8 *)colq      );
    cbytes_init(&val_bytes);

    ASSERT(EC_TRUE == cbgt_search(cbgt_md_id, &table_name_bytes, &row_bytes, &colf_bytes, &colq_bytes, &val_bytes));
    sys_log(LOGSTDOUT, "[test_case_cbgt_search_one] searched (%s:%s:%s) in table %s ==> val %.*s\n",
                        row, colf, colq, table_name, cbytes_len(&val_bytes), cbytes_buf(&val_bytes));
    return;
}

void test_case_cbgt_fetch_one(const UINT32 cbgt_md_id, const char *table_name, const char *row, const char *colf, const char *colq)
{
    CBYTES table_name_bytes;
    CBYTES row_bytes;
    CBYTES colf_bytes;
    CBYTES colq_bytes;
    CBYTES val_bytes;

    cbytes_mount(&table_name_bytes, strlen(table_name), (UINT8 *)table_name);
    cbytes_mount(&row_bytes       , strlen(row)       , (UINT8 *)row       );
    cbytes_mount(&colf_bytes      , strlen(colf)      , (UINT8 *)colf      );
    cbytes_mount(&colq_bytes      , strlen(colq)      , (UINT8 *)colq      );

    cbytes_init(&val_bytes);

    ASSERT(EC_TRUE == cbgt_fetch(cbgt_md_id, &table_name_bytes, &row_bytes, &colf_bytes, &colq_bytes, &val_bytes));
    sys_log(LOGSTDOUT, "[test_case_cbgt_fetch_one] fetched (%s:%s:%s) in table %s ==> %.*s\n",
                        row, colf, colq, table_name, cbytes_len(&val_bytes), cbytes_buf(&val_bytes));
    cbytes_clean(&val_bytes, 0);
    return;
}

void test_case_cbgt_delete_one(const UINT32 cbgt_md_id, const char *table_name, const char *row, const char *colf, const char *colq)
{
    CBYTES table_name_bytes;
    CBYTES row_bytes;
    CBYTES colf_bytes;
    CBYTES colq_bytes;

    cbytes_mount(&table_name_bytes, strlen(table_name), (UINT8 *)table_name);
    cbytes_mount(&row_bytes       , strlen(row)       , (UINT8 *)row       );
    cbytes_mount(&colf_bytes      , strlen(colf)      , (UINT8 *)colf      );
    cbytes_mount(&colq_bytes      , strlen(colq)      , (UINT8 *)colq      );

    ASSERT(EC_TRUE == cbgt_delete(cbgt_md_id, &table_name_bytes, &row_bytes, &colf_bytes, &colq_bytes));
    sys_log(LOGSTDOUT, "[test_case_cbgt_delete_one] deleted (%s:%s:%s) in table %s\n",
                        row, colf, colq, table_name);
    return;
}

void test_case_cbgt_create_table_on_root(const UINT32 cbgt_md_id, const char *user_table_name, const int colf_num, ...)
{
    CBYTES table_name_bytes;
    void *colf_vec;

    int pos;

    va_list ap;

    cbytes_mount(&table_name_bytes, strlen(user_table_name), (UINT8 *)user_table_name);

    colf_vec = cvector_new(0, MM_CBYTES, 0);

    va_start(ap, colf_num);
    for(pos = 0; pos < colf_num; pos ++)
    {
        CBYTES *colf_bytes;
        char *colf;

        colf = va_arg(ap, char *);

        colf_bytes = cbytes_new(0);
        ASSERT(NULL_PTR != colf_bytes);

        cbytes_mount(colf_bytes, strlen(colf), (UINT8 *)colf);
        cvector_push(colf_vec, colf_bytes);
    }
    va_end(ap);

    ASSERT(EC_TRUE == cbgt_create_table_on_root(cbgt_md_id, &table_name_bytes, colf_vec));

    cvector_loop_front(colf_vec, (CVECTOR_DATA_HANDLER)cbytes_umount);
    cvector_clean_with_location(colf_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, 0);
    cvector_free(colf_vec, 0);
    return;
}

void test_case_cbgt_insert_group(const UINT32 cbgt_md_id, const char *table_name)
{
    UINT32 row_idx;
    UINT32 colf_idx;
    UINT32 colq_idx;

    CBYTES table_name_bytes;

    void  *bytes_vec;

    cbytes_mount(&table_name_bytes, strlen(table_name), (UINT8 *)table_name);

    ASSERT(bytes_vec = cvector_new(0, MM_CBYTES, 0));

    for(row_idx = 0; row_idx < CBGT_TEST_ROW_NUM; row_idx ++)
    {
        char   *row;
        CBYTES *row_bytes;

        ASSERT(row = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
        snprintf(row, CBGT_STR_MAX_LEN, "row-%08ld", row_idx);

        ASSERT(row_bytes = cbytes_new(0));
        cbytes_mount(row_bytes       , strlen(row)       , (UINT8 *)row       );

        cvector_push(bytes_vec, (void *)row_bytes);

        for(colf_idx = 0; colf_idx < CBGT_TEST_COLF_NUM; colf_idx ++)
        {
            char   *colf;
            CBYTES *colf_bytes;

            ASSERT(colf = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
            snprintf(colf, CBGT_STR_MAX_LEN, "colf-%ld", colf_idx);

            ASSERT(colf_bytes = cbytes_new(0));
            cbytes_mount(colf_bytes       , strlen(colf)       , (UINT8 *)colf       );

            cvector_push(bytes_vec, (void *)colf_bytes);

            for(colq_idx = 0; colq_idx < CBGT_TEST_COLQ_NUM; colq_idx ++)
            {
                char   *colq;
                CBYTES *colq_bytes;

                char   *val;
                CBYTES *val_bytes;

                ASSERT(colq = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
                snprintf(colq, CBGT_STR_MAX_LEN, "colq-%08ld-%08ld-%08ld", row_idx, colf_idx, colq_idx);

                ASSERT(val = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
                snprintf(val, CBGT_STR_MAX_LEN, "val-%08ld-%08ld-%08ld", row_idx, colf_idx, colq_idx);

                ASSERT(colq_bytes = cbytes_new(0));
                ASSERT(val_bytes = cbytes_new(0));

                cbytes_mount(colq_bytes      , strlen(colq)      , (UINT8 *)colq      );
                cbytes_mount(val_bytes       , strlen(val)       , (UINT8 *)val       );

                cvector_push(bytes_vec, (void *)colq_bytes);
                cvector_push(bytes_vec, (void *)val_bytes);

                //sys_log(LOGCONSOLE, "test_case_cbgt_insert_group: [TRY ] insert %s %s:%s:%s %s\n", table_name, row, colf, colq, val);
                ASSERT(EC_TRUE == cbgt_insert(cbgt_md_id, &table_name_bytes, row_bytes, colf_bytes, colq_bytes, val_bytes));
                sys_log(LOGCONSOLE, "test_case_cbgt_insert_group: [SUCC] insert %s %s:%s:%s %s\n", table_name, row, colf, colq, val);
            }
        }
    }

    cvector_clean_with_location(bytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, 0);
    cvector_free(bytes_vec, 0);

    sys_log(LOGCONSOLE, "test_case_cbgt_insert_group: end\n");

    return;
}

void test_case_cbgt_insert_group_p(const UINT32 cbgt_md_id, const char *table_name, const UINT32 row_tag)
{
    UINT32 row_idx;
    UINT32 colf_idx;
    UINT32 colq_idx;

    CBYTES table_name_bytes;

    void  *mod_mgr;

    EC_BOOL continue_flag;

    ASSERT(32 <= CBGT_KV_VAL_LEN);

    ASSERT(mod_mgr = mod_mgr_new(cbgt_md_id, LOAD_BALANCING_OBJ));
    mod_mgr_incl(CMPI_LOCAL_TCID, CMPI_LOCAL_COMM, CMPI_LOCAL_RANK, cbgt_md_id, mod_mgr);
    mod_mgr_print(LOGCONSOLE, mod_mgr);

    cbytes_mount(&table_name_bytes, strlen(table_name), (UINT8 *)table_name);

    continue_flag = EC_TRUE;

    for(row_idx = 0; row_idx < CBGT_TEST_ROW_NUM && EC_TRUE == continue_flag; row_idx ++)
    {
        char   *row;
        CBYTES *row_bytes;

        ASSERT(row = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
        BSET(row, 0, CBGT_STR_MAX_LEN);
        snprintf(row, CBGT_STR_MAX_LEN, "row-%08ld-%08ld", row_tag, row_idx);

        ASSERT(row_bytes = cbytes_new(0));
        cbytes_mount(row_bytes       , strlen(row)       , (UINT8 *)row       );

        //cvector_push(bytes_vec, (void *)row_bytes);

        for(colf_idx = 0; colf_idx < CBGT_TEST_COLF_NUM && EC_TRUE == continue_flag; colf_idx ++)
        {
            char   *colf;
            CBYTES *colf_bytes;

            void   *task_mgr;
            void   *bytes_vec;

            EC_BOOL     ret[CBGT_TEST_COLQ_NUM];

            ASSERT(bytes_vec = cvector_new(0, MM_CBYTES, 0));

            ASSERT(colf = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
            BSET(colf, 0, CBGT_STR_MAX_LEN);
            snprintf(colf, CBGT_STR_MAX_LEN, "colf-%ld", colf_idx);

            ASSERT(colf_bytes = cbytes_new(0));
            cbytes_mount(colf_bytes       , strlen(colf)       , (UINT8 *)colf       );

            //cvector_push(bytes_vec, (void *)colf_bytes);

            task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

            for(colq_idx = 0; colq_idx < CBGT_TEST_COLQ_NUM && EC_TRUE == continue_flag; colq_idx ++)
            {
                char   *colq;
                CBYTES *colq_bytes;

                char   *val;
                CBYTES *val_bytes;

                ASSERT(colq = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
                snprintf(colq, CBGT_STR_MAX_LEN, "colq-%08ld-%08ld-%08ld", row_idx, colf_idx, colq_idx);
#if 0
                ASSERT(val = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
                snprintf(val, CBGT_STR_MAX_LEN, "val-%08ld-%08ld-%08ld", row_idx, colf_idx, colq_idx);
#else
                ASSERT(val = (char *)safe_malloc(CBGT_KV_VAL_LEN, 0));
                snprintf(val, CBGT_KV_VAL_LEN, "val-%08ld-%08ld-%08ld", row_idx, colf_idx, colq_idx);
                BSET(val + strlen(val), '0', CBGT_KV_VAL_LEN - strlen(val));
                val[CBGT_KV_VAL_LEN - 1] = '\0';
#endif
                ASSERT(colq_bytes = cbytes_new(0));
                ASSERT(val_bytes = cbytes_new(0));

                cbytes_mount(colq_bytes      , strlen(colq)      , (UINT8 *)colq      );
                cbytes_mount(val_bytes       , strlen(val)       , (UINT8 *)val       );

                cvector_push(bytes_vec, (void *)colq_bytes);
                cvector_push(bytes_vec, (void *)val_bytes);

                ret[ colq_idx ] = EC_FALSE;
                task_inc(task_mgr, &(ret[ colq_idx ]),
                        FI_cbgt_insert, ERR_MODULE_ID, &table_name_bytes, row_bytes, colf_bytes, colq_bytes, val_bytes, EC_FALSE);
            }

            task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

            for(colq_idx = 0; colq_idx < CBGT_TEST_COLQ_NUM; colq_idx ++)
            {
                if(EC_TRUE == ret[ colq_idx ])
                {
#if 0
                    sys_log(LOGSTDNULL, "[DEBUG] test_case_cbgt_insert_group_p: [SUCC] insert %s %s:%s:colq-%08ld-%08ld-%08ld val-%08ld-%08ld-%08ld\n",
                                        table_name, row, colf,
                                        row_idx, colf_idx, colq_idx,
                                        row_idx, colf_idx, colq_idx);
#endif                                        
                }
                else
                {
                    continue_flag = EC_FALSE;
                    sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_insert_group_p: [FAIL] insert %s %s:%s:colq-%08ld-%08ld-%08ld val-%08ld-%08ld-%08ld\n",
                                        table_name, row, colf,
                                        row_idx, colf_idx, colq_idx,
                                        row_idx, colf_idx, colq_idx);
                }
            }

            cbytes_free(colf_bytes, 0);

            cvector_clean_with_location(bytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, 0);
            cvector_free(bytes_vec, 0);
        }

        if(EC_TRUE == continue_flag)
        {
            sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_insert_group_p: [SUCC] row tag %ld, row no. %ld\n", row_tag, row_idx);
        }
        else
        {
            sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_insert_group_p: [FAIL] row tag %ld, row no. %ld\n", row_tag, row_idx);
        }

        cbytes_free(row_bytes, 0);
    }

    sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_insert_group_p: row tag %ld end\n", row_tag);

    return;
}

void test_case_cbgt_fetch_group_p(const UINT32 cbgt_md_id, const char *table_name, const UINT32 row_tag)
{
    UINT32 row_idx;
    UINT32 colf_idx;
    UINT32 colq_idx;

    CBYTES table_name_bytes;

    void  *mod_mgr;

    EC_BOOL continue_flag;

    ASSERT(32 <= CBGT_KV_VAL_LEN);

    ASSERT(mod_mgr = mod_mgr_new(cbgt_md_id, LOAD_BALANCING_OBJ));
    mod_mgr_incl(CMPI_LOCAL_TCID, CMPI_LOCAL_COMM, CMPI_LOCAL_RANK, cbgt_md_id, mod_mgr);
    mod_mgr_print(LOGCONSOLE, mod_mgr);

    cbytes_mount(&table_name_bytes, strlen(table_name), (UINT8 *)table_name);

    continue_flag = EC_TRUE;

    for(row_idx = 0; row_idx < CBGT_TEST_ROW_NUM && EC_TRUE == continue_flag; row_idx ++)
    {
        char   *row;
        CBYTES *row_bytes;

        ASSERT(row = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
        snprintf(row, CBGT_STR_MAX_LEN, "row-%08ld-%08ld", row_tag, row_idx);

        ASSERT(row_bytes = cbytes_new(0));
        cbytes_mount(row_bytes       , strlen(row)       , (UINT8 *)row       );

        //cvector_push(bytes_vec, (void *)row_bytes);

        for(colf_idx = 0; colf_idx < CBGT_TEST_COLF_NUM && EC_TRUE == continue_flag; colf_idx ++)
        {
            char   *colf;
            CBYTES *colf_bytes;
            void   *task_mgr;
            void   *bytes_vec;

            EC_BOOL     ret[CBGT_TEST_COLQ_NUM];
            CBYTES      val_bytes[CBGT_TEST_COLQ_NUM];

            ASSERT(bytes_vec = cvector_new(0, MM_CBYTES, 0));

            ASSERT(colf = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
            snprintf(colf, CBGT_STR_MAX_LEN, "colf-%ld", colf_idx);

            ASSERT(colf_bytes = cbytes_new(0));
            cbytes_mount(colf_bytes       , strlen(colf)       , (UINT8 *)colf       );

            //cvector_push(bytes_vec, (void *)colf_bytes);

            task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

            for(colq_idx = 0; colq_idx < CBGT_TEST_COLQ_NUM && EC_TRUE == continue_flag; colq_idx ++)
            {
                char   *colq;
                CBYTES *colq_bytes;

                ASSERT(colq = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
                snprintf(colq, CBGT_STR_MAX_LEN, "colq-%08ld-%08ld-%08ld", row_idx, colf_idx, colq_idx);

                ASSERT(colq_bytes = cbytes_new(0));

                cbytes_mount(colq_bytes      , strlen(colq)      , (UINT8 *)colq      );

                cvector_push(bytes_vec, (void *)colq_bytes);

                ret[ colq_idx ] = EC_FALSE;
                cbytes_init(&(val_bytes[colq_idx]));

                task_inc(task_mgr, &(ret[ colq_idx ]),
                        FI_cbgt_fetch, ERR_MODULE_ID, &table_name_bytes, row_bytes, colf_bytes, colq_bytes, &(val_bytes[colq_idx]));
            }

            task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

            for(colq_idx = 0; colq_idx < CBGT_TEST_COLQ_NUM; colq_idx ++)
            {
                char   *val;
                CBYTES  __val_bytes;

#if 0
                ASSERT(val = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
                snprintf(val, CBGT_STR_MAX_LEN, "val-%08ld-%08ld-%08ld", row_idx, colf_idx, colq_idx);
#else
                ASSERT(val = (char *)safe_malloc(CBGT_KV_VAL_LEN, 0));
                snprintf(val, CBGT_KV_VAL_LEN, "val-%08ld-%08ld-%08ld", row_idx, colf_idx, colq_idx);
                memset(val + strlen(val), '0', CBGT_KV_VAL_LEN - strlen(val));
                val[CBGT_KV_VAL_LEN - 1] = '\0';
#endif

                cbytes_init(&__val_bytes);
                cbytes_mount(&__val_bytes, strlen(val), (UINT8 *)val);                

                if(EC_TRUE == ret[ colq_idx ] && EC_TRUE == cbytes_cmp(&__val_bytes, &(val_bytes[colq_idx])))
                {
#if 0                
                    sys_log(LOGSTDNULL, "[DEBUG] test_case_cbgt_fetch_group_p: [SUCC] fetch %s %s:%s:colq-%08ld-%08ld-%08ld => %.*s\n",
                                        table_name, row, colf,
                                        row_idx, colf_idx, colq_idx,
                                        cbytes_len(&(val_bytes[colq_idx])), (char *)cbytes_buf(&(val_bytes[colq_idx])));
#endif                                        
                }
                else
                {
                    //continue_flag = EC_FALSE;
                    sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_fetch_group_p: [FAIL] fetch %s %s:%s:colq-%08ld-%08ld-%08ld => %.*s\n",
                                        table_name, row, colf,
                                        row_idx, colf_idx, colq_idx,
                                        cbytes_len(&(val_bytes[colq_idx])), (char *)cbytes_buf(&(val_bytes[colq_idx])));
                }
                cbytes_clean(&__val_bytes, 0);
                cbytes_clean(&(val_bytes[colq_idx]), 0);
            }

            cbytes_free(colf_bytes, 0);

            cvector_clean_with_location(bytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, 0);
            cvector_free(bytes_vec, 0);
        }

        if(EC_TRUE == continue_flag)
        {
            sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_fetch_group_p: [SUCC] row tag %ld, row no. %ld\n", row_tag, row_idx);
        }
        else
        {
            sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_fetch_group_p: [FAIL] row tag %ld, row no. %ld\n", row_tag, row_idx);
        }

        cbytes_free(row_bytes, 0);
    }


    sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_fetch_group_p: row tag %ld end\n", row_tag);

    return;
}

void test_case_cbgt_delete_group_p(const UINT32 cbgt_md_id, const char *table_name, const UINT32 row_tag)
{
    UINT32 row_idx;
    UINT32 colf_idx;
    UINT32 colq_idx;

    CBYTES table_name_bytes;

    void  *mod_mgr;

    EC_BOOL continue_flag;

    ASSERT(mod_mgr = mod_mgr_new(cbgt_md_id, LOAD_BALANCING_OBJ));
    mod_mgr_incl(CMPI_LOCAL_TCID, CMPI_LOCAL_COMM, CMPI_LOCAL_RANK, cbgt_md_id, mod_mgr);
    mod_mgr_print(LOGCONSOLE, mod_mgr);

    cbytes_mount(&table_name_bytes, strlen(table_name), (UINT8 *)table_name);

    continue_flag = EC_TRUE;

    for(row_idx = 0; row_idx < CBGT_TEST_ROW_NUM && EC_TRUE == continue_flag; row_idx ++)
    {
        char   *row;
        CBYTES *row_bytes;

        ASSERT(row = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
        snprintf(row, CBGT_STR_MAX_LEN, "row-%08ld-%08ld", row_tag, row_idx);

        ASSERT(row_bytes = cbytes_new(0));
        cbytes_mount(row_bytes       , strlen(row)       , (UINT8 *)row       );

        //cvector_push(bytes_vec, (void *)row_bytes);

        for(colf_idx = 0; colf_idx < CBGT_TEST_COLF_NUM && EC_TRUE == continue_flag; colf_idx ++)
        {
            char   *colf;
            CBYTES *colf_bytes;
            void   *task_mgr;

            void   *bytes_vec;

            EC_BOOL     ret[CBGT_TEST_COLQ_NUM];

            ASSERT(bytes_vec = cvector_new(0, MM_CBYTES, 0));

            ASSERT(colf = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
            snprintf(colf, CBGT_STR_MAX_LEN, "colf-%ld", colf_idx);

            ASSERT(colf_bytes = cbytes_new(0));
            cbytes_mount(colf_bytes       , strlen(colf)       , (UINT8 *)colf       );

            task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

            for(colq_idx = 0; colq_idx < CBGT_TEST_COLQ_NUM && EC_TRUE == continue_flag; colq_idx ++)
            {
                char   *colq;
                CBYTES *colq_bytes;

                ASSERT(colq = (char *)safe_malloc(CBGT_STR_MAX_LEN, 0));
                snprintf(colq, CBGT_STR_MAX_LEN, "colq-%08ld-%08ld-%08ld", row_idx, colf_idx, colq_idx);

                ASSERT(colq_bytes = cbytes_new(0));

                cbytes_mount(colq_bytes      , strlen(colq)      , (UINT8 *)colq      );

                cvector_push(bytes_vec, (void *)colq_bytes);

                ret[ colq_idx ] = EC_FALSE;

                task_inc(task_mgr, &(ret[ colq_idx ]),
                        FI_cbgt_delete, ERR_MODULE_ID, &table_name_bytes, row_bytes, colf_bytes, colq_bytes);
            }

            task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

            for(colq_idx = 0; colq_idx < CBGT_TEST_COLQ_NUM; colq_idx ++)
            {
                if(EC_TRUE == ret[ colq_idx ])
                {
                    sys_log(LOGSTDNULL, "[DEBUG] test_case_cbgt_delete_group_p: [SUCC] delete %s %s:%s:colq-%08ld-%08ld-%08ld\n",
                                        table_name, row, colf,
                                        row_idx, colf_idx, colq_idx);
                }
                else
                {
                    //continue_flag = EC_FALSE;
                    sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_delete_group_p: [FAIL] delete %s %s:%s:colq-%08ld-%08ld-%08ld\n",
                                        table_name, row, colf,
                                        row_idx, colf_idx, colq_idx);
                }
            }

            cbytes_free(colf_bytes, 0);

            cvector_clean_with_location(bytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, 0);
            cvector_free(bytes_vec, 0);
        }

        cbytes_free(row_bytes, 0);

        if(EC_TRUE == continue_flag)
        {
            sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_delete_group_p: [SUCC] row tag %ld, row no. %ld\n", row_tag, row_idx);
        }
        else
        {
            sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_delete_group_p: [FAIL] row tag %ld, row no. %ld\n", row_tag, row_idx);
        }
    }

    sys_log(LOGCONSOLE, "[DEBUG] test_case_cbgt_delete_group_p: row tag %ld end\n", row_tag);

    return;
}

void test_case_cbgt_insert(const UINT32 cbgt_md_id)
{
    //test_case_cbgt_create_table_on_root(cbgt_md_id, "hansoul", 3, "colf-0", "colf-1", "colf-2");

    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-0", "val-0-0-0");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-1", "val-0-0-1");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-2", "val-0-0-2");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-3", "val-0-0-3");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-4", "val-0-0-4");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-5", "val-0-0-5");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-6", "val-0-0-6");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-7", "val-0-0-7");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-8", "val-0-0-8");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-9", "val-0-0-9");

    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-0", "val-1-1-0");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-1", "val-1-1-1");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-2", "val-1-1-2");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-3", "val-1-1-3");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-4", "val-1-1-4");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-5", "val-1-1-5");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-6", "val-1-1-6");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-7", "val-1-1-7");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-8", "val-1-1-8");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-9", "val-1-1-9");

    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-0", "val-2-1-0");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-1", "val-2-1-1");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-2", "val-2-1-2");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-3", "val-2-1-3");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-4", "val-2-1-4");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-5", "val-2-1-5");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-6", "val-2-1-6");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-7", "val-2-1-7");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-8", "val-2-1-8");
    test_case_cbgt_insert_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-9", "val-2-1-9");

    sys_log(LOGSTDOUT, "[DEBUG] test_case_cbgt_insert: ============================ user data inserted ====================\n");

    return;
}

void test_case_cbgt_search(const UINT32 cbgt_md_id)
{
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-9");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-8");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-7");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-6");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-5");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-4");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-3");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-2");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-1");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-0");

    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-9");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-8");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-7");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-6");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-5");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-4");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-3");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-2");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-1");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-0");

    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-9");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-8");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-7");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-6");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-5");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-4");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-3");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-2");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-1");
    test_case_cbgt_search_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-0");

    sys_log(LOGSTDOUT, "[DEBUG] test_case_cbgt_search: ============================ user data searched ====================\n");

    return;
}

void test_case_cbgt_fetch(const UINT32 cbgt_md_id)
{
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-9");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-8");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-7");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-6");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-5");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-4");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-3");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-2");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-1");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-0");

    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-9");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-8");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-7");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-6");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-5");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-4");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-3");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-2");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-1");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-0");

    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-9");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-8");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-7");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-6");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-5");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-4");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-3");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-2");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-1");
    test_case_cbgt_fetch_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-0");

    sys_log(LOGSTDOUT, "[DEBUG] test_case_cbgt_fetch: ============================ user data fetched ====================\n");

    return;
}

void test_case_cbgt_delete(const UINT32 cbgt_md_id)
{
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-9");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-8");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-7");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-6");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-5");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-4");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-3");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-2");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-1");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-0", "colq-0-0");

    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-9");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-8");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-7");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-6");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-5");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-4");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-3");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-2");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-1");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-1", "colq-1-0");

    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-9");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-8");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-7");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-6");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-5");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-4");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-3");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-2");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-1");
    test_case_cbgt_delete_one(cbgt_md_id, "hansoul", "row-A", "colf-2", "colq-2-0");

    sys_log(LOGSTDOUT, "[DEBUG] test_case_cbgt_delete: ============================ user data deleted ====================\n");

    return;
}

EC_BOOL __test_cbgt_np_runner()
{
    UINT32 cdfs_md_id;

    CSTRING *cdfsnp_db_root_dir;

    cdfsnp_db_root_dir = task_brd_default_get_hsdfs_np_root_dir();
    ASSERT(NULL_PTR != cdfsnp_db_root_dir);

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    cdfs_open_npp(cdfs_md_id, cdfsnp_db_root_dir, CDFS_NP_CACHED_MAX_NUM);
    //ASSERT(EC_TRUE == cdfs_open_npp(cdfs_md_id, cdfsnp_db_root_dir, CDFS_NP_CACHED_MAX_NUM));

    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));
    ASSERT(EC_TRUE == cdfs_add_dn_vec(cdfs_md_id));

    cdfs_reg_npp_vec(cdfs_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cbgt_dn_runner()
{
    UINT32 cdfs_md_id;

    CSTRING *cdfsdn_db_root_dir;

    cdfsdn_db_root_dir = task_brd_default_get_hsdfs_dn_root_dir();
    ASSERT(NULL_PTR != cdfsdn_db_root_dir);

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    cdfs_open_dn(cdfs_md_id, cdfsdn_db_root_dir);
    //ASSERT(EC_TRUE == cdfs_open_dn(cdfs_md_id, root_dir));

    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));
    ASSERT(EC_TRUE == cdfs_add_dn_vec(cdfs_md_id));

    cdfs_reg_dn_vec(cdfs_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cbgt_root_runner()
{
    UINT32 cbgt_md_id;

    CSTRING *cbgt_db_root_dir;
    CBYTES   root_table_name;
    extern UINT32 g_cbtree_key_cmp_counter;
    extern UINT32 g_do_slave_usleep_counter;

    cbgt_db_root_dir = task_brd_default_get_hsbgt_root_table_dir();
    ASSERT(NULL_PTR != cbgt_db_root_dir);
    cbytes_mount(&root_table_name, strlen("root"), (UINT8 *)"root");

    cbgt_md_id = cbgt_start(CBGT_TYPE_ROOT_SERVER, CBGT_ROOT_TABLE_ID, &root_table_name, NULL_PTR, cbgt_db_root_dir, CBGT_O_RDWR | CBGT_O_CREAT);

    ASSERT(ERR_MODULE_ID != cbgt_md_id);    
#if 1
    sys_log(LOGSTDOUT, "[DEBUG] __test_cbgt_root_runner: ============================ root server started ====================\n");
    test_case_cbgt_create_table_on_root(cbgt_md_id, "hansoul", 3, "colf-0", "colf-1", "colf-2");

    g_cbtree_key_cmp_counter = 0;
    g_do_slave_usleep_counter = 0;
    __test_cbgt_insert_client_1_runner();    
    //sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_root_runner: after insert, g_cbtree_key_cmp_counter = %ld\n", g_cbtree_key_cmp_counter);
    //sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_root_runner: after insert, g_do_slave_usleep_counter = %ld\n", g_do_slave_usleep_counter);
#endif
    //g_cbtree_key_cmp_counter = 0;
    //g_do_slave_usleep_counter = 0;
    //__test_cbgt_delete_client_1_runner();    
    //sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_root_runner: after delete, g_cbtree_key_cmp_counter = %ld\n", g_cbtree_key_cmp_counter);
    //sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_root_runner: after insert, g_do_slave_usleep_counter = %ld\n", g_do_slave_usleep_counter);

    g_cbtree_key_cmp_counter = 0;
    g_do_slave_usleep_counter = 0;
    __test_cbgt_fetch_client_1_runner();
    sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_root_runner: after fetch, g_cbtree_key_cmp_counter = %ld\n", g_cbtree_key_cmp_counter);
    sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_root_runner: after insert, g_do_slave_usleep_counter = %ld\n", g_do_slave_usleep_counter);
    return (EC_TRUE);
}

EC_BOOL __test_cbgt_insert_client_1_runner()
{
    UINT32 cbgt_md_id;

    CSTRING *cbgt_db_root_dir;

    cbgt_db_root_dir = task_brd_default_get_hsbgt_root_table_dir();
    ASSERT(NULL_PTR != cbgt_db_root_dir);

    cbgt_md_id = cbgt_start(CBGT_TYPE_USER_CLIENT, CBGT_ERR_TABLE_ID, NULL_PTR, NULL_PTR, cbgt_db_root_dir, CBGT_O_UNDEF);

    ASSERT(ERR_MODULE_ID != cbgt_md_id);

    sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_insert_client_1_runner: ============================ user client started ====================\n");
    //sys_log_switch_off();
    test_case_cbgt_insert_group_p(cbgt_md_id, "hansoul", 10101021);    
    cbgt_end(cbgt_md_id);
    //sys_log_switch_on();
    sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_insert_client_1_runner: ============================ user client stopped ====================\n");

    return (EC_TRUE);
}

EC_BOOL __test_cbgt_insert_client_2_runner()
{
    UINT32 cbgt_md_id;

    CSTRING *cbgt_db_root_dir;

    cbgt_db_root_dir = task_brd_default_get_hsbgt_root_table_dir();
    ASSERT(NULL_PTR != cbgt_db_root_dir);

    cbgt_md_id = cbgt_start(CBGT_TYPE_USER_CLIENT, CBGT_ERR_TABLE_ID, NULL_PTR, NULL_PTR, cbgt_db_root_dir, CBGT_O_UNDEF);

    ASSERT(ERR_MODULE_ID != cbgt_md_id);

    sys_log(LOGSTDOUT, "[DEBUG] __test_cbgt_insert_client_2_runner: ============================ user client started ====================\n");

    test_case_cbgt_insert_group_p(cbgt_md_id, "hansoul", 10101022);
    cbgt_end(cbgt_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cbgt_fetch_client_1_runner()
{
    UINT32 cbgt_md_id;

    CSTRING *cbgt_db_root_dir;

    cbgt_db_root_dir = task_brd_default_get_hsbgt_root_table_dir();
    ASSERT(NULL_PTR != cbgt_db_root_dir);

    cbgt_md_id = cbgt_start(CBGT_TYPE_USER_CLIENT, CBGT_ERR_TABLE_ID, NULL_PTR, NULL_PTR, cbgt_db_root_dir, CBGT_O_UNDEF);

    ASSERT(ERR_MODULE_ID != cbgt_md_id);

    sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_fetch_client_1_runner: ============================ user client started ====================\n");
    //sys_log_switch_off();
    test_case_cbgt_fetch_group_p(cbgt_md_id, "hansoul", 10101021);    
    cbgt_end(cbgt_md_id);
    //sys_log_switch_on();
    sys_log(LOGCONSOLE, "[DEBUG] __test_cbgt_fetch_client_1_runner: ============================ user client stopped ====================\n");
    
    return (EC_TRUE);
}

EC_BOOL __test_cbgt_fetch_client_2_runner()
{
    UINT32 cbgt_md_id;

    CSTRING *cbgt_db_root_dir;

    cbgt_db_root_dir = task_brd_default_get_hsbgt_root_table_dir();
    ASSERT(NULL_PTR != cbgt_db_root_dir);

    cbgt_md_id = cbgt_start(CBGT_TYPE_USER_CLIENT, CBGT_ERR_TABLE_ID, NULL_PTR, NULL_PTR, cbgt_db_root_dir, CBGT_O_UNDEF);

    ASSERT(ERR_MODULE_ID != cbgt_md_id);

    sys_log(LOGSTDOUT, "[DEBUG] __test_cbgt_fetch_client_2_runner: ============================ user client started ====================\n");

    test_case_cbgt_fetch_group_p(cbgt_md_id, "hansoul", 10101022);
    cbgt_end(cbgt_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cbgt_delete_client_1_runner()
{
    UINT32 cbgt_md_id;

    CSTRING *cbgt_db_root_dir;

    cbgt_db_root_dir = task_brd_default_get_hsbgt_root_table_dir();
    ASSERT(NULL_PTR != cbgt_db_root_dir);
    
    cbgt_md_id = cbgt_start(CBGT_TYPE_USER_CLIENT, CBGT_ERR_TABLE_ID, NULL_PTR, NULL_PTR, cbgt_db_root_dir, CBGT_O_UNDEF);
    
    ASSERT(ERR_MODULE_ID != cbgt_md_id);

    sys_log(LOGSTDOUT, "[DEBUG] __test_cbgt_delete_client_1_runner: ============================ user client started ====================\n");

    test_case_cbgt_delete_group_p(cbgt_md_id, "hansoul", 10101021);
    cbgt_end(cbgt_md_id);
    
    return (EC_TRUE);
}

EC_BOOL __test_cbgt_delete_client_2_runner()
{
    UINT32 cbgt_md_id;

    CSTRING *cbgt_db_root_dir;

    cbgt_db_root_dir = task_brd_default_get_hsbgt_root_table_dir();
    ASSERT(NULL_PTR != cbgt_db_root_dir);

    cbgt_md_id = cbgt_start(CBGT_TYPE_USER_CLIENT, CBGT_ERR_TABLE_ID, NULL_PTR, NULL_PTR, cbgt_db_root_dir, CBGT_O_UNDEF);

    ASSERT(ERR_MODULE_ID != cbgt_md_id);

    sys_log(LOGSTDOUT, "[DEBUG] __test_cbgt_delete_client_2_runner: ============================ user client started ====================\n");

    test_case_cbgt_delete_group_p(cbgt_md_id, "hansoul", 10101022);
    cbgt_end(cbgt_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cbgt_suite_01()
{
    /*define specific runner for each (tcid, rank)*/
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.1"), CMPI_CDFS_RANK, __test_cbgt_np_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.20.1"), CMPI_CDFS_RANK, __test_cbgt_np_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.2"), CMPI_CDFS_RANK, __test_cbgt_dn_runner);
    
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.3"), 0, __test_cbgt_root_runner);
    
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.4"), 0, __test_cbgt_insert_client_1_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.4"), 1, __test_cbgt_insert_client_2_runner);
 
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.5"), 1, __test_cbgt_fetch_client_1_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.5"), 2, __test_cbgt_fetch_client_2_runner);

    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.6"), 1, __test_cbgt_delete_client_1_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.6"), 2, __test_cbgt_delete_client_2_runner);

    /*start the defined runner on current (tcid, rank)*/
    task_brd_default_start_runner();

    return (EC_TRUE);
}

EC_BOOL __test_cbgt_suite_02()
{
    /*define specific runner for each (tcid, rank)*/
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.1"), CMPI_CDFS_RANK, __test_cbgt_np_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.20.1"), CMPI_CDFS_RANK, __test_cbgt_np_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.2"), CMPI_CDFS_RANK, __test_cbgt_dn_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.3"), CMPI_CDFS_RANK, __test_cbgt_dn_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.4"), CMPI_CDFS_RANK, __test_cbgt_dn_runner);
    
    task_brd_default_add_runner(ipv4_to_uint32("10.10.30.5"), 0, __test_cbgt_root_runner);
    //task_brd_default_add_runner(ipv4_to_uint32("10.10.30.3"), 0, __test_cbgt_insert_client_1_runner); 
    
    //task_brd_default_add_runner(ipv4_to_uint32("10.10.30.4"), 0, __test_cbgt_insert_client_1_runner); 
    //task_brd_default_add_runner(ipv4_to_uint32("10.10.30.5"), 0, __test_cbgt_insert_client_2_runner);
    //task_brd_default_add_runner(ipv4_to_uint32("10.10.30.6"), 0, __test_cbgt_fetch_client_1_runner);
    //task_brd_default_add_runner(ipv4_to_uint32("10.10.30.7"), 0, __test_cbgt_delete_client_1_runner);

    /*start the defined runner on current (tcid, rank)*/
    task_brd_default_start_runner();

    return (EC_TRUE);
}

int main_cbgt(int argc, char **argv)
{
    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_cbgt: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }

    //__test_cbgt_suite_01();
    __test_cbgt_suite_02();
    
    return (0);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

