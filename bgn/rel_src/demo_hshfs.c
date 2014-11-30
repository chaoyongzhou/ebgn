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
#include <malloc.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "typeconst.h"
#include "type.h"
#include "cmisc.h"
#include "task.h"
#include "mod.h"
#include "log.h"
#include "debug.h"
#include "rank.h"

#include "cstring.h"
#include "cvector.h"

#include "super.h"
#include "tbd.h"
#include "crun.h"

#include "cthread.h"

#include "cmpic.inc"
#include "findex.inc"

#include "chashalgo.h"
#include "cbytes.h"

#include "chfs.h"
#include "chfsnp.h"
#include "demo_hshfs.h"


#define CHFS_TEST_TCID_STR ((char *)"10.10.10.1")
#define CHFS_TEST_RANK     ((UINT32)0)
#define CHFS_TEST_MODI     ((UINT32)0)

#define CHFS_NP_ROOT_DIR   ((char *)".")
#define CHFS_DN_ROOT_DIR   ((char *)".")

#define CHFS_TEST_HOME_1   ((char *)"/h1")
#define CHFS_TEST_HOME_2   ((char *)"/h2")

static CBYTES *g_cbytes[32];
static UINT32 g_cbytes_max_len = sizeof(g_cbytes)/sizeof(g_cbytes[0]);

static void print_tcid(LOG *log, const UINT32 tcid)
{
    sys_log(log, "%s\n", c_word_to_ipv4(tcid));
    return;
}

static EC_BOOL __test_chfs_init_g_cbytes(const UINT32 max_num)
{
    UINT32 pos;
    UINT32 max_cfg_num;

    max_cfg_num = sizeof(g_chfs_file_cfg_tbl)/sizeof(g_chfs_file_cfg_tbl[0]);
    if(max_num > max_cfg_num)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_chfs_init_g_cbytes: max_num %ld but max_cfg_num %ld\n", max_num, max_cfg_num);
        return (EC_FALSE);
    }

    if(max_num > g_cbytes_max_len)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_chfs_init_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
        return (EC_FALSE);
    }

    for(pos = 0; pos < g_cbytes_max_len; pos ++)
    {
        g_cbytes[ pos ] = NULL_PTR;
    }

    for(pos = 0; pos < max_num; pos ++)
    {
        char   *file_name;
        UINT32  file_size;
        CBYTES *cbytes;
        int fd;

        file_name = g_chfs_file_cfg_tbl[ pos ].file_name;
        file_size = g_chfs_file_cfg_tbl[ pos ].file_size;

        if(0 != access(file_name, F_OK))
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_chfs_init_g_cbytes: file %s not exist or inaccessable\n", file_name);
            return (EC_FALSE);
        }

        fd = c_file_open(file_name, O_RDONLY, 0666);
        if(-1 == fd)
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_chfs_init_g_cbytes: open file %s to read failed\n", file_name);
            return (EC_FALSE);
        }

        cbytes = cbytes_new(file_size);
        if((ssize_t)file_size != read(fd, cbytes_buf(cbytes), file_size))
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_chfs_init_g_cbytes: read file %s with size %ld failed\n", file_name, file_size);
            cbytes_free(cbytes, 0);
            c_file_close(fd);
            return (EC_FALSE);
        }

        g_cbytes[ pos ] = cbytes;

        c_file_close(fd);
    }

    return (EC_TRUE);
}

static EC_BOOL __test_chfs_clean_g_cbytes(const UINT32 max_num)
{
    UINT32 pos;
    if(max_num > g_cbytes_max_len)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_chfs_clean_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
        return (EC_FALSE);
    }

    for(pos = 0; pos < max_num; pos ++)
    {
        CBYTES   *cbytes;

        cbytes = g_cbytes[ pos ];
        if(NULL_PTR != cbytes)
        {
            cbytes_free(cbytes, 0);
            g_cbytes[ pos ] = NULL_PTR;
        }
    }
    return (EC_TRUE);
}

static CBYTES *__test_chfs_fetch_g_cbytes(const UINT32 max_num, const UINT32 pos)
{
    if(max_num > g_cbytes_max_len)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_chfs_fetch_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
        return (NULL_PTR);
    }

    return g_cbytes[ pos ];
}

EC_BOOL test_case_82_chfs_read(const char *home, const UINT32 chfs_tcid, const UINT32 chfs_rank, const UINT32 chfs_modi, const UINT32 max_test_data_files, UINT32 *counter, UINT32 *file_num_counter, UINT32 *byte_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    CSTRING    *path[CHFS_TEST_READ_MAX_FILES];
    CBYTES     *cbytes[CHFS_TEST_READ_MAX_FILES];/*read from dn*/
    CBYTES     *cbytes_des[CHFS_TEST_READ_MAX_FILES];/*benchmark*/
    EC_BOOL     ret[CHFS_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;

    for(index = 0; index < CHFS_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]          = NULL_PTR;
        cbytes[ index ]     = NULL_PTR;
        cbytes_des[ index ] = NULL_PTR;
        ret[ index ]           = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(chfs_tcid, CMPI_ANY_COMM, chfs_rank, chfs_modi, mod_mgr);
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_chfs_read: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, chfs_get_npp_mod_mgr(chfs_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_chfs_read: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, chfs_get_dn_mod_mgr(chfs_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(index = 0; index < CHFS_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        cbytes[ index ]     = cbytes_new(0);
        cbytes_des[ index ] = __test_chfs_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));

        ret[ index ] = EC_FALSE;

        (*file_num_counter) ++;
        (*byte_num_counter) += cbytes_len(cbytes_des[ index ]);

        task_inc(task_mgr, &(ret[ index ]), FI_chfs_read, ERR_MODULE_ID, path[ index ], cbytes[ index ]);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CHFS_TEST_READ_MAX_FILES; index ++)
    {
        if(NULL_PTR != cbytes[ index ])
        {
            if(EC_TRUE == cbytes_ncmp(cbytes[ index ], cbytes_des[ index ], 16))
            {
                dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "[SUCC] path: %s, len = %ld ",
                                  (char *)cstring_get_str(path[ index ]),
                                  cbytes_len(cbytes[ index ]));
                sys_print(LOGSTDOUT, "text = %.*s\n",
                                  cbytes_len(cbytes[ index ]) > 16 ? 16 : cbytes_len(cbytes[ index ]), /*output up to 16 chars*/
                                  (char *)cbytes_buf(cbytes[ index ]));
            }
            else
            {
                continue_flag = EC_FALSE;

                dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[FAIL] path: %s, read len = %ld ",
                                  (char *)cstring_get_str(path[ index ]),
                                  cbytes_len(cbytes[ index ]));
                sys_print(LOGCONSOLE, "text = %.*s <--> ",
                                  cbytes_len(cbytes[ index ]) > 16 ? 16 : cbytes_len(cbytes[ index ]), /*output up to 16 chars*/
                                  (char *)cbytes_buf(cbytes[ index ]));

                sys_print(LOGCONSOLE, "expect len = %ld ",
                                    cbytes_len(cbytes_des[ index ]));
                sys_print(LOGCONSOLE, "text = %.*s\n",
                                    cbytes_len(cbytes_des[ index ]) > 16 ? 16 : cbytes_len(cbytes_des[ index ]),
                                    (char *)cbytes_buf(cbytes_des[ index ]));
            }
        }

        if(NULL_PTR != path[ index ])
        {
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
        }

        if(NULL_PTR != cbytes[ index ])
        {
            cbytes_free(cbytes[ index ], 0);
            cbytes[ index ] = NULL_PTR;
        }

        if(NULL_PTR != cbytes_des[ index ])
        {
            cbytes_des[ index ] = NULL_PTR;
        }
    }

    mod_mgr_free(mod_mgr);
    return (continue_flag);
}


EC_BOOL test_case_83_chfs_write(const char *home, const UINT32 chfs_tcid, const UINT32 chfs_rank, const UINT32 chfs_modi, const UINT32 max_test_data_files, UINT32 *counter, UINT32 *file_num_counter, UINT32 *byte_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    EC_BOOL continue_flag;

    CSTRING    *path[CHFS_TEST_WRITE_MAX_FILES];
    EC_BOOL     ret[CHFS_TEST_WRITE_MAX_FILES];

    for(index = 0; index < CHFS_TEST_WRITE_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(chfs_tcid, CMPI_ANY_COMM, chfs_rank, chfs_modi, mod_mgr);
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_chfs_write: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, chfs_get_npp_mod_mgr(chfs_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_chfs_write: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, chfs_get_dn_mod_mgr(chfs_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(index = 0; index < CHFS_TEST_WRITE_MAX_FILES; index ++, (*counter) ++)
    {
        void *cbytes;

        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        ret[ index ] = EC_FALSE;
        cbytes = __test_chfs_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));
        if(NULL_PTR == cbytes)
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_83_chfs_write: chfs buff is null where index = %ld, max_test_data_files = %ld\n", index, max_test_data_files);
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
            break;
        }

        (*file_num_counter) ++;
        (*byte_num_counter) += cbytes_len(cbytes);

        task_inc(task_mgr, &(ret[ index ]), FI_chfs_write, ERR_MODULE_ID, path[ index ], cbytes);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CHFS_TEST_WRITE_MAX_FILES; index ++)
    {
        if(EC_FALSE == ret[ index ] && NULL_PTR != path[ index ])
        {
            continue_flag = EC_FALSE;
            dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "test_case_83_chfs_write: [FAIL] %s\n", (char *)cstring_get_str(path[ index ]));
        }
        if(NULL_PTR != path[ index ])
        {
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
        }
    }

    mod_mgr_free(mod_mgr);

    return (continue_flag);
}


/*check replica files*/
EC_BOOL test_case_85_chfs_check_file_content(const char *home, const UINT32 chfs_tcid, const UINT32 chfs_rank, const UINT32 chfs_modi, const UINT32 max_test_data_files, UINT32 *counter)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 index;

    CSTRING    *path[CHFS_TEST_READ_MAX_FILES];
    EC_BOOL     ret[CHFS_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;

    for(index = 0; index < CHFS_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(chfs_tcid, CMPI_ANY_COMM, chfs_rank, chfs_modi, mod_mgr);

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_85_chfs_check_file_content: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, chfs_get_npp_mod_mgr(chfs_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_85_chfs_check_file_content: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, chfs_get_dn_mod_mgr(chfs_modi));

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(index = 0; index < CHFS_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        CBYTES *cbytes_des;
        cbytes_des = __test_chfs_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));

        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        ret[ index ] = EC_FALSE;

        task_inc(task_mgr, &(ret[ index ]),
                        FI_chfs_check_file_is, ERR_MODULE_ID, path[ index ], cbytes_des);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CHFS_TEST_READ_MAX_FILES; index ++)
    {
        if(EC_TRUE == ret[ index ])
        {
            dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "[SUCC] path: %s\n", (char *)cstring_get_str(path[ index ]));
        }
        else
        {
            continue_flag = EC_FALSE;
            dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[FAIL] path: %s\n", (char *)cstring_get_str(path[ index ]));
        }

        if(NULL_PTR != path[ index ])
        {
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
        }

        if(NULL_PTR != path[ index ])
        {
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
        }
    }

    mod_mgr_free(mod_mgr);
    
    return (continue_flag);
}

EC_BOOL test_case_86_chfs_writer(const UINT32 chfs_tcid, const UINT32 chfs_rank, const UINT32 chfs_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32  outer_loop;
    UINT32  inner_loop;
    EC_BOOL continue_flag;

    UINT32  file_num_counter;
    UINT32  byte_num_counter;

    if(EC_FALSE == __test_chfs_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_86_chfs_writer:__test_chfs_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_chfs_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    file_num_counter = 0;
    byte_num_counter = 0;

    continue_flag = EC_TRUE;

    for(outer_loop = 0; outer_loop < CHFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_86_chfs_writer: outer_loop = %ld, file_num = %ld, byte_num = %ld\n", outer_loop, file_num_counter, byte_num_counter);

        dir0 = (outer_loop % CHFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CHFS_MAX_FILE_NUM_PER_LOOP) % CHFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CHFS_MAX_FILE_NUM_PER_LOOP) / CHFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CHFS_MAX_FILE_NUM_PER_LOOP + CHFS_TEST_WRITE_MAX_FILES - 1) / CHFS_TEST_WRITE_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_83_chfs_write(home, chfs_tcid, chfs_rank, chfs_modi, max_test_data_files, &counter, &file_num_counter, &byte_num_counter);
        }
    }

    __test_chfs_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_86_chfs_writer: end\n");

    return (continue_flag);
}

EC_BOOL test_case_87_chfs_reader(const UINT32 chfs_tcid, const UINT32 chfs_rank, const UINT32 chfs_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    UINT32  file_num_counter;
    UINT32  byte_num_counter;

    if(EC_FALSE == __test_chfs_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_87_chfs_reader:__test_chfs_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_chfs_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    file_num_counter = 0;
    byte_num_counter = 0;    

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CHFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_87_chfs_reader: outer_loop = %ld, file_num = %ld, byte_num = %ld\n", outer_loop, file_num_counter, byte_num_counter);

        dir0 = (outer_loop % CHFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CHFS_MAX_FILE_NUM_PER_LOOP) % CHFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CHFS_MAX_FILE_NUM_PER_LOOP) / CHFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CHFS_MAX_FILE_NUM_PER_LOOP + CHFS_TEST_READ_MAX_FILES - 1) / CHFS_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_82_chfs_read(home, chfs_tcid, chfs_rank, chfs_modi, max_test_data_files, &counter, &file_num_counter, &byte_num_counter);
        }
    }

    __test_chfs_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_87_chfs_reader: end\n");

    return (continue_flag);
}

EC_BOOL test_case_88_chfs_file_content_checker(const UINT32 chfs_tcid, const UINT32 chfs_rank, const UINT32 chfs_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    if(EC_FALSE == __test_chfs_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_88_chfs_file_content_checker:__test_chfs_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_chfs_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CHFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_88_chfs_file_content_checker: outer_loop = %ld\n", outer_loop);

        dir0 = (outer_loop % CHFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CHFS_MAX_FILE_NUM_PER_LOOP) % CHFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CHFS_MAX_FILE_NUM_PER_LOOP) / CHFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CHFS_MAX_FILE_NUM_PER_LOOP + CHFS_TEST_READ_MAX_FILES - 1) / CHFS_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_85_chfs_check_file_content(home, chfs_tcid, chfs_rank, chfs_modi, max_test_data_files, &counter);
        }
    }

    __test_chfs_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_88_chfs_file_content_checker: end\n");

    return (EC_TRUE);
}

EC_BOOL __test_chfs_runner()
{
    UINT32 chfs_modi;

    CSTRING *chfsnp_root_dir;
    CSTRING *chfsdn_root_dir;

    chfsnp_root_dir = cstring_new((UINT8 *)CHFS_NP_ROOT_DIR, 0);
    ASSERT(NULL_PTR != chfsnp_root_dir);

    chfsdn_root_dir = cstring_new((UINT8 *)CHFS_DN_ROOT_DIR, 0);
    ASSERT(NULL_PTR != chfsdn_root_dir);    

    chfs_modi = chfs_start(chfsnp_root_dir, chfsdn_root_dir);
    ASSERT(ERR_MODULE_ID != chfs_modi);

    cstring_free(chfsnp_root_dir);
    cstring_free(chfsdn_root_dir);

    //chfs_end(chfs_modi);
    
    return (EC_TRUE);
}

EC_BOOL __test_chfs_np_runner()
{
    UINT32 chfs_modi;

    CSTRING *chfsnp_root_dir;

    chfsnp_root_dir = cstring_new((UINT8 *)CHFS_NP_ROOT_DIR, 0);
    ASSERT(NULL_PTR != chfsnp_root_dir);

    chfs_modi = chfs_start(chfsnp_root_dir, NULL_PTR);
    ASSERT(ERR_MODULE_ID != chfs_modi);

    cstring_free(chfsnp_root_dir);

    chfs_end(chfs_modi);
    
    return (EC_TRUE);
}

EC_BOOL __test_chfs_dn_runner()
{
    UINT32 chfs_modi;

    CSTRING *chfsdn_root_dir;

    chfsdn_root_dir = cstring_new((UINT8 *)CHFS_DN_ROOT_DIR, 0);
    ASSERT(NULL_PTR != chfsdn_root_dir);    

    chfs_modi = chfs_start(NULL_PTR, chfsdn_root_dir);
    ASSERT(ERR_MODULE_ID != chfs_modi);

    cstring_free(chfsdn_root_dir);

    chfs_end(chfs_modi);
    
    return (EC_TRUE);
}

EC_BOOL __test_chfs_write_supplier_1()
{
    UINT32 chfs_tcid;
    UINT32 chfs_rank;
    UINT32 chfs_modi;

    chfs_tcid = c_ipv4_to_word(CHFS_TEST_TCID_STR);
    chfs_rank = CHFS_TEST_RANK;
    chfs_modi = CHFS_TEST_MODI;

    test_case_86_chfs_writer(chfs_tcid, chfs_rank, chfs_modi, g_chfs_cbytes_used_num, CHFS_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_chfs_write_supplier_2()
{
    UINT32 chfs_tcid;
    UINT32 chfs_rank;
    UINT32 chfs_modi;

    chfs_tcid = c_ipv4_to_word(CHFS_TEST_TCID_STR);
    chfs_rank = CHFS_TEST_RANK;
    chfs_modi = CHFS_TEST_MODI;

    test_case_86_chfs_writer(chfs_tcid, chfs_rank, chfs_modi, g_chfs_cbytes_used_num, CHFS_TEST_HOME_2);

    return (EC_TRUE);
}

EC_BOOL __test_chfs_read_consumer_1()
{
    UINT32 chfs_tcid;
    UINT32 chfs_rank;
    UINT32 chfs_modi;

    chfs_tcid = c_ipv4_to_word(CHFS_TEST_TCID_STR);
    chfs_rank = CHFS_TEST_RANK;
    chfs_modi = CHFS_TEST_MODI;

    test_case_87_chfs_reader(chfs_tcid, chfs_rank, chfs_modi, g_chfs_cbytes_used_num, CHFS_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_chfs_read_consumer_2()
{
    UINT32 chfs_tcid;
    UINT32 chfs_rank;
    UINT32 chfs_modi;

    chfs_tcid = c_ipv4_to_word(CHFS_TEST_TCID_STR);
    chfs_rank = CHFS_TEST_RANK;
    chfs_modi = CHFS_TEST_MODI;

    test_case_87_chfs_reader(chfs_tcid, chfs_rank, chfs_modi, g_chfs_cbytes_used_num, CHFS_TEST_HOME_2);

    return (EC_TRUE);
}

EC_BOOL __test_chfs_check_content_consumer_1()
{
    UINT32 chfs_tcid;
    UINT32 chfs_rank;
    UINT32 chfs_modi;

    chfs_tcid = c_ipv4_to_word(CHFS_TEST_TCID_STR);
    chfs_rank = CHFS_TEST_RANK;
    chfs_modi = CHFS_TEST_MODI;

    test_case_88_chfs_file_content_checker(chfs_tcid, chfs_rank, chfs_modi, g_chfs_cbytes_used_num, CHFS_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_chfs_check_content_consumer_2()
{
    UINT32 chfs_tcid;
    UINT32 chfs_rank;
    UINT32 chfs_modi;

    chfs_tcid = c_ipv4_to_word(CHFS_TEST_TCID_STR);
    chfs_rank = CHFS_TEST_RANK;
    chfs_modi = CHFS_TEST_MODI;

    test_case_88_chfs_file_content_checker(chfs_tcid, chfs_rank, chfs_modi, g_chfs_cbytes_used_num, CHFS_TEST_HOME_2);

    return (EC_TRUE);
}

int main_chfs(int argc, char **argv)
{
    UINT32 chfs_rank;
    UINT32 tester_rank;
    
    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:main_chfs: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }

    chfs_rank = CHFS_TEST_RANK;
    tester_rank = 0;

    /*define specific runner for each (tcid, rank)*/
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.1"), chfs_rank  , __test_chfs_runner);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.2"), tester_rank, __test_chfs_write_supplier_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.3"), tester_rank, __test_chfs_read_consumer_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.4"), tester_rank, __test_chfs_write_supplier_2);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.5"), tester_rank, __test_chfs_read_consumer_2);

    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.101"), tester_rank, __test_chfs_check_content_consumer_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.102"), tester_rank, __test_chfs_check_content_consumer_2);

    /*start the defined runner on current (tcid, rank)*/
    task_brd_default_start_runner();    

    return (0);
}

int main_chfs_x(int argc, char **argv)
{
    main_chfs(argc, argv);
    return (0);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

