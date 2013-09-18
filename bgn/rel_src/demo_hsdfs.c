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
#include "lib_cbytes.h"

#include "lib_cdfs.h"
#include "lib_cdfsnp.h"
#include "lib_csolr.h"
#include "demo.h"


static void print_tcid(LOG *log, const UINT32 tcid)
{
    sys_log(log, "%s\n", uint32_to_ipv4(tcid));
    return;
}

static EC_BOOL init_g_cbytes(const UINT32 cdfs_md_id, const UINT32 max_num)
{
    UINT32 pos;
    UINT32 max_cfg_num;

    max_cfg_num = sizeof(g_file_cfg_tbl)/sizeof(g_file_cfg_tbl[0]);
    if(max_num > max_cfg_num)
    {
        sys_log(LOGSTDOUT, "error:init_g_cbytes: max_num %ld but max_cfg_num %ld\n", max_num, max_cfg_num);
        return (EC_FALSE);
    }

    if(max_num > g_cbytes_max_len)
    {
        sys_log(LOGSTDOUT, "error:init_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
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

        file_name = g_file_cfg_tbl[ pos ].file_name;
        file_size = g_file_cfg_tbl[ pos ].file_size;
        //cbytes = ;

        if(0 != access(file_name, F_OK))
        {
            sys_log(LOGSTDOUT, "error:init_g_cbytes: file %s not exist or inaccessable\n", file_name);
            return (EC_FALSE);
        }

        fd = c_open(file_name, O_RDONLY, 0666);
        if(-1 == fd)
        {
            sys_log(LOGSTDOUT, "error:init_g_cbytes: open file %s to read failed\n", file_name);
            return (EC_FALSE);
        }

        cbytes = cbytes_new(file_size);
        if((ssize_t)file_size != read(fd, cbytes_buf(cbytes), file_size))
        {
            sys_log(LOGSTDOUT, "error:init_g_cbytes: read file %s with size %ld failed\n", file_name, file_size);
            cbytes_free(cbytes, 0);
            close(fd);
            return (EC_FALSE);
        }

        g_cbytes[ pos ] = cbytes;

        close(fd);
    }

    return (EC_TRUE);
}

static EC_BOOL clean_g_cbytes(const UINT32 cdfs_md_id, const UINT32 max_num)
{
    UINT32 pos;
    if(max_num > g_cbytes_max_len)
    {
        sys_log(LOGSTDOUT, "error:clean_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
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

static CBYTES *fetch_g_cbytes(const UINT32 cdfs_md_id, const UINT32 max_num, const UINT32 pos)
{
    if(max_num > g_cbytes_max_len)
    {
        sys_log(LOGSTDOUT, "error:fetch_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
        return (NULL_PTR);
    }

    return g_cbytes[ pos ];
}

EC_BOOL test_case_82_cdfs_read(const char *home, const UINT32 read_from_tcid, const UINT32 cdfs_md_id, const UINT32 max_test_data_files, UINT32 *counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    CSTRING    *path[CDFS_TEST_READ_MAX_FILES];
    CBYTES     *cbytes[CDFS_TEST_READ_MAX_FILES];/*read from dn*/
    CBYTES     *cbytes_des[CDFS_TEST_READ_MAX_FILES];/*benchmark*/
    EC_BOOL     ret[CDFS_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;

    for(index = 0; index < CDFS_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]          = NULL_PTR;
        cbytes[ index ]     = NULL_PTR;
        cbytes_des[ index ] = NULL_PTR;
        ret[ index ]           = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(cdfs_md_id, LOAD_BALANCING_LOOP);
    mod_mgr_incl(read_from_tcid, CMPI_ANY_COMM, CMPI_CDFS_RANK, cdfs_md_id, mod_mgr);

    sys_log(LOGSTDOUT, "test_case_82_cdfs_read: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, cdfs_get_npp_mod_mgr(cdfs_md_id));

    sys_log(LOGSTDOUT, "test_case_82_cdfs_read: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, cdfs_get_dn_mod_mgr(cdfs_md_id));

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(index = 0; index < CDFS_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        cbytes[ index ]     = cbytes_new(0);
        cbytes_des[ index ] = fetch_g_cbytes(cdfs_md_id, max_test_data_files, ((*counter) % max_test_data_files));

        ret[ index ] = EC_FALSE;

        //task_tcid_inc(task_mgr, read_from_tcid, &(ret[ index ]), FI_cdfs_read, ERR_MODULE_ID, path[ index ], cbytes[ index ]);
        task_inc(task_mgr, &(ret[ index ]), FI_cdfs_read, ERR_MODULE_ID, path[ index ], cbytes[ index ]);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CDFS_TEST_READ_MAX_FILES; index ++)
    {
        if(NULL_PTR != cbytes[ index ])
        {
            if(EC_TRUE == cbytes_ncmp(cbytes[ index ], cbytes_des[ index ], 16))
            {
                sys_log(LOGSTDOUT, "[SUCC] path: %s, len = %ld ",
                                  (char *)cstring_get_str(path[ index ]),
                                  cbytes_len(cbytes[ index ]));
                sys_print(LOGSTDOUT, "text = %.*s\n",
                                  cbytes_len(cbytes[ index ]) > 16 ? 16 : cbytes_len(cbytes[ index ]), /*output up to 16 chars*/
                                  (char *)cbytes_buf(cbytes[ index ]));
            }
            else
            {
                continue_flag = EC_FALSE;

                sys_log(LOGCONSOLE, "[FAIL] path: %s, read len = %ld ",
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


EC_BOOL test_case_83_cdf_write(const char *home, const UINT32 write_from_tcid, const UINT32 cdfs_md_id, const UINT32 max_test_data_files, UINT32 *counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    EC_BOOL continue_flag;

    CSTRING    *path[CDFS_TEST_WRITE_MAX_FILES];
    EC_BOOL     ret[CDFS_TEST_WRITE_MAX_FILES];

    for(index = 0; index < CDFS_TEST_WRITE_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(cdfs_md_id, LOAD_BALANCING_LOOP);
    mod_mgr_incl(write_from_tcid, CMPI_ANY_COMM, CMPI_CDFS_RANK, cdfs_md_id, mod_mgr);

    sys_log(LOGSTDOUT, "test_case_83_cdf_write: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, cdfs_get_npp_mod_mgr(cdfs_md_id));

    sys_log(LOGSTDOUT, "test_case_83_cdf_write: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, cdfs_get_dn_mod_mgr(cdfs_md_id));

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(index = 0; index < CDFS_TEST_WRITE_MAX_FILES; index ++, (*counter) ++)
    {
        void *cbytes;

        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        ret[ index ] = EC_FALSE;
        cbytes = fetch_g_cbytes(cdfs_md_id, max_test_data_files, ((*counter) % max_test_data_files));
        if(NULL_PTR == cbytes)
        {
            sys_log(LOGSTDOUT, "error:test_case_83_cdf_write: cdfs buff is null where index = %ld, max_test_data_files = %ld\n", index, max_test_data_files);
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
            break;
        }

        task_tcid_inc(task_mgr, write_from_tcid, &(ret[ index ]), FI_cdfs_write, ERR_MODULE_ID, path[ index ], cbytes, CDFS_REPLICA_MAX_NUM);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CDFS_TEST_WRITE_MAX_FILES; index ++)
    {
        if(EC_FALSE == ret[ index ] && NULL_PTR != path[ index ])
        {
            continue_flag = EC_FALSE;
            sys_log(LOGCONSOLE, "test_case_83_cdf_write: [FAIL] %s\n", (char *)cstring_get_str(path[ index ]));
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

/*check replica basic info*/
EC_BOOL test_case_84_check_replica(const char *home, const UINT32 cdfs_md_id, const UINT32 replica_num, const void *tcid_vec, UINT32 *counter)
{
    void *cdfsnpp_mod_mgr;
    void *task_mgr;

    UINT32 index;

    CSTRING    *path[CDFS_TEST_READ_MAX_FILES];
    EC_BOOL     ret[CDFS_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;

    for(index = 0; index < CDFS_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    cdfsnpp_mod_mgr = cdfs_get_npp_mod_mgr(cdfs_md_id);
    sys_log(LOGSTDOUT, "test_case_84_check_replica: cdfsnpp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, cdfsnpp_mod_mgr);

    task_mgr = task_new(cdfsnpp_mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(index = 0; index < CDFS_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        ret[ index ] = EC_FALSE;

        task_inc(task_mgr, &(ret[ index ]), FI_cdfs_check_replicas, ERR_MODULE_ID, path[ index ], replica_num, tcid_vec);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CDFS_TEST_READ_MAX_FILES; index ++)
    {
        if(EC_TRUE == ret[ index ])
        {
            sys_log(LOGSTDOUT, "[SUCC] path: %s\n", (char *)cstring_get_str(path[ index ]));
        }
        else
        {
            continue_flag = EC_FALSE;
            sys_log(LOGCONSOLE, "[FAIL] path: %s\n", (char *)cstring_get_str(path[ index ]));
        }

        if(NULL_PTR != path[ index ])
        {
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
        }
    }

    return (continue_flag);
}

/*check replica files*/
EC_BOOL test_case_85_cdfs_check_file_content(const char *home, const UINT32 cdfs_md_id, const UINT32 max_test_data_files, UINT32 *counter)
{
    void *cdfsnpp_mod_mgr;
    void *task_mgr;

    UINT32 index;

    CSTRING    *path[CDFS_TEST_READ_MAX_FILES];
    CSTRING    *file_content_cstr[CDFS_TEST_READ_MAX_FILES];
    EC_BOOL     ret[CDFS_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;

    for(index = 0; index < CDFS_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        file_content_cstr[ index ] = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    cdfsnpp_mod_mgr = cdfs_get_npp_mod_mgr(cdfs_md_id);
    sys_log(LOGSTDOUT, "test_case_85_cdfs_check_file_content: cdfsnpp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, cdfsnpp_mod_mgr);

    task_mgr = task_new(cdfsnpp_mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(index = 0; index < CDFS_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        CBYTES *cbytes_des;
        cbytes_des = fetch_g_cbytes(cdfs_md_id, max_test_data_files, ((*counter) % max_test_data_files));

        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        file_content_cstr[ index ] = cstring_new(NULL_PTR, 0);
        cstring_append_chars(file_content_cstr[ index ], 16, cbytes_buf(cbytes_des));

        ret[ index ] = EC_FALSE;

        task_inc(task_mgr, &(ret[ index ]),
                        FI_cdfs_check_replica_files_content, ERR_MODULE_ID, path[ index ], cbytes_len(cbytes_des), file_content_cstr[ index ]);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CDFS_TEST_READ_MAX_FILES; index ++)
    {
        if(EC_TRUE == ret[ index ])
        {
            sys_log(LOGSTDOUT, "[SUCC] path: %s\n", (char *)cstring_get_str(path[ index ]));
        }
        else
        {
            continue_flag = EC_FALSE;
            sys_log(LOGCONSOLE, "[FAIL] path: %s\n", (char *)cstring_get_str(path[ index ]));
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

        if(NULL_PTR != file_content_cstr[ index ])
        {
            cstring_free(file_content_cstr[ index ]);
            file_content_cstr[ index ] = NULL_PTR;
        }
    }

    return (continue_flag);
}

EC_BOOL test_case_86_cdfs_writer(const UINT32 cdfs_md_id, const UINT32 write_from_tcid, const UINT32 max_test_data_files, const char *root_dir_in_db, const void *cdfsdn_tcid_vec)
{
    UINT32 outer_loop;
    UINT32 inner_loop;
    EC_BOOL continue_flag;

    if(EC_FALSE == init_g_cbytes(cdfs_md_id, max_test_data_files))
    {
        sys_log(LOGSTDOUT, "error:test_case_86_cdfs_writer:init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        clean_g_cbytes(cdfs_md_id, max_test_data_files);
        return (EC_FALSE);
    }

    continue_flag = EC_TRUE;

    for(outer_loop = 0; outer_loop < CDFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        sys_log(LOGCONSOLE, "[DEBUG] test_case_86_cdfs_writer: outer_loop = %ld\n", outer_loop);

        dir0 = (outer_loop % CDFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CDFS_MAX_FILE_NUM_PER_LOOP) % CDFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CDFS_MAX_FILE_NUM_PER_LOOP) / CDFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%04ld/%04ld/%04ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CDFS_MAX_FILE_NUM_PER_LOOP + CDFS_TEST_WRITE_MAX_FILES - 1) / CDFS_TEST_WRITE_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_83_cdf_write(home, write_from_tcid, cdfs_md_id, max_test_data_files, &counter);
        }
    }

    clean_g_cbytes(cdfs_md_id, max_test_data_files);

    sys_log(LOGCONSOLE, "[DEBUG] test_case_86_cdfs_writer: end\n");

    return (continue_flag);
}

EC_BOOL test_case_87_cdfs_reader(const UINT32 cdfs_md_id, const UINT32 read_from_tcid, const UINT32 max_test_data_files, const char *root_dir_in_db, const void *cdfsdn_tcid_vec)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    if(EC_FALSE == init_g_cbytes(cdfs_md_id, max_test_data_files))
    {
        sys_log(LOGSTDOUT, "error:test_case_87_cdfs_reader:init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        clean_g_cbytes(cdfs_md_id, max_test_data_files);
        return (EC_FALSE);
    }

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CDFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        sys_log(LOGCONSOLE, "[DEBUG] test_case_87_cdfs_reader: outer_loop = %ld\n", outer_loop);

        dir0 = (outer_loop % CDFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CDFS_MAX_FILE_NUM_PER_LOOP) % CDFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CDFS_MAX_FILE_NUM_PER_LOOP) / CDFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%04ld/%04ld/%04ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CDFS_MAX_FILE_NUM_PER_LOOP + CDFS_TEST_READ_MAX_FILES - 1) / CDFS_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_82_cdfs_read(home, read_from_tcid, cdfs_md_id, max_test_data_files, &counter);
        }
    }

    clean_g_cbytes(cdfs_md_id, max_test_data_files);

    sys_log(LOGCONSOLE, "[DEBUG] test_case_87_cdfs_reader: end\n");

    return (continue_flag);
}

EC_BOOL test_case_88_cdfs_replica_checker(const UINT32 cdfs_md_id, const UINT32 max_test_data_files, const char *root_dir_in_db, const void *cdfsdn_tcid_vec)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    if(EC_FALSE == init_g_cbytes(cdfs_md_id, max_test_data_files))
    {
        sys_log(LOGSTDOUT, "error:test_case_88_cdfs_replica_checker:init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        clean_g_cbytes(cdfs_md_id, max_test_data_files);
        return (EC_FALSE);
    }

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CDFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        sys_log(LOGCONSOLE, "[DEBUG] test_case_88_cdfs_replica_checker: outer_loop = %ld\n", outer_loop);

        dir0 = (outer_loop % CDFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CDFS_MAX_FILE_NUM_PER_LOOP) % CDFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CDFS_MAX_FILE_NUM_PER_LOOP) / CDFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%04ld/%04ld/%04ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CDFS_MAX_FILE_NUM_PER_LOOP + CDFS_TEST_READ_MAX_FILES - 1) / CDFS_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_84_check_replica(home, cdfs_md_id, CDFS_REPLICA_MAX_NUM, cdfsdn_tcid_vec, &counter);
        }
    }

    clean_g_cbytes(cdfs_md_id, max_test_data_files);

    sys_log(LOGCONSOLE, "[DEBUG] test_case_88_cdfs_replica_checker: end\n");

    return (EC_TRUE);
}

EC_BOOL test_case_88_cdfs_file_content_checker(const UINT32 cdfs_md_id, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    if(EC_FALSE == init_g_cbytes(cdfs_md_id, max_test_data_files))
    {
        sys_log(LOGSTDOUT, "error:test_case_88_cdfs_file_content_checker:init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        clean_g_cbytes(cdfs_md_id, max_test_data_files);
        return (EC_FALSE);
    }

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CDFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        sys_log(LOGCONSOLE, "[DEBUG] test_case_88_cdfs_file_content_checker: outer_loop = %ld\n", outer_loop);

        dir0 = (outer_loop % CDFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CDFS_MAX_FILE_NUM_PER_LOOP) % CDFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CDFS_MAX_FILE_NUM_PER_LOOP) / CDFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%04ld/%04ld/%04ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CDFS_MAX_FILE_NUM_PER_LOOP + CDFS_TEST_READ_MAX_FILES - 1) / CDFS_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_85_cdfs_check_file_content(home, cdfs_md_id, max_test_data_files, &counter);
        }
    }

    clean_g_cbytes(cdfs_md_id, max_test_data_files);

    sys_log(LOGCONSOLE, "[DEBUG] test_case_88_cdfs_file_content_checker: end\n");

    return (EC_TRUE);
}

EC_BOOL __test_cdfs_np_runner()
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

EC_BOOL __test_cdfs_dn_runner()
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

EC_BOOL __test_cdfs_write_supplier_1()
{
    UINT32 cdfs_md_id;

    void * cdfsdn_tcid_vec;

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    
    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));
    ASSERT(EC_TRUE == cdfs_add_dn_vec(cdfs_md_id));    

    cdfsdn_tcid_vec = cvector_new(0, MM_UINT32, 0);
    ASSERT(NULL_PTR != cdfsdn_tcid_vec);
    cdfs_collect_dn_tcid_vec(cdfs_md_id, cdfsdn_tcid_vec);

    test_case_86_cdfs_writer(cdfs_md_id, CMPI_LOCAL_TCID, g_cbytes_used_num, (char *)"/hansoul01", cdfsdn_tcid_vec);

    cvector_free(cdfsdn_tcid_vec, 0);

    cdfs_end(cdfs_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cdfs_write_supplier_2()
{
    UINT32 cdfs_md_id;

    void * cdfsdn_tcid_vec;

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    
    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));
    ASSERT(EC_TRUE == cdfs_add_dn_vec(cdfs_md_id));    

    cdfsdn_tcid_vec = cvector_new(0, MM_UINT32, 0);
    ASSERT(NULL_PTR != cdfsdn_tcid_vec);    
    cdfs_collect_dn_tcid_vec(cdfs_md_id, cdfsdn_tcid_vec);

    test_case_86_cdfs_writer(cdfs_md_id, CMPI_LOCAL_TCID, g_cbytes_used_num, (char *)"/hansoul02", cdfsdn_tcid_vec);

    cvector_free(cdfsdn_tcid_vec, 0);

    cdfs_end(cdfs_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cdfs_read_consumer_1()
{
    UINT32 cdfs_md_id;

    void * cdfsdn_tcid_vec;

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);

    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));
    ASSERT(EC_TRUE == cdfs_add_dn_vec(cdfs_md_id));

    cdfsdn_tcid_vec = cvector_new(0, MM_UINT32, 0);
    ASSERT(NULL_PTR != cdfsdn_tcid_vec);    
    cdfs_collect_dn_tcid_vec(cdfs_md_id, cdfsdn_tcid_vec);

    test_case_87_cdfs_reader(cdfs_md_id, CMPI_LOCAL_TCID, g_cbytes_used_num, (char *)"/hansoul01", cdfsdn_tcid_vec);

    cvector_free(cdfsdn_tcid_vec, 0);

    cdfs_end(cdfs_md_id);
    return (EC_TRUE);
}

EC_BOOL __test_cdfs_read_consumer_2()
{
    UINT32 cdfs_md_id;

    void * cdfsdn_tcid_vec;

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    
    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));
    ASSERT(EC_TRUE == cdfs_add_dn_vec(cdfs_md_id));    

    cdfsdn_tcid_vec = cvector_new(0, MM_UINT32, 0);
    ASSERT(NULL_PTR != cdfsdn_tcid_vec);
    cdfs_collect_dn_tcid_vec(cdfs_md_id, cdfsdn_tcid_vec);

    test_case_87_cdfs_reader(cdfs_md_id, CMPI_LOCAL_TCID, g_cbytes_used_num, (char *)"/hansoul02", cdfsdn_tcid_vec);

    cvector_free(cdfsdn_tcid_vec, 0);

    cdfs_end(cdfs_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cdfs_check_replica_consumer_1()
{
    UINT32 cdfs_md_id;

    void * cdfsdn_tcid_vec;

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    
    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));

    cdfsdn_tcid_vec = cvector_new(0, MM_UINT32, 0);
    ASSERT(NULL_PTR != cdfsdn_tcid_vec);
    cdfs_collect_dn_tcid_vec(cdfs_md_id, cdfsdn_tcid_vec);

    test_case_88_cdfs_replica_checker(cdfs_md_id, g_cbytes_used_num, (char *)"/hansoul01", cdfsdn_tcid_vec);

    cvector_free(cdfsdn_tcid_vec, 0);

    cdfs_end(cdfs_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cdfs_check_replica_consumer_2()
{
    UINT32 cdfs_md_id;

    void * cdfsdn_tcid_vec;

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    
    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));

    cdfsdn_tcid_vec = cvector_new(0, MM_UINT32, 0);
    ASSERT(NULL_PTR != cdfsdn_tcid_vec);
    cdfs_collect_dn_tcid_vec(cdfs_md_id, cdfsdn_tcid_vec);

    test_case_88_cdfs_replica_checker(cdfs_md_id, g_cbytes_used_num, (char *)"/hansoul02", cdfsdn_tcid_vec);

    cvector_free(cdfsdn_tcid_vec, 0);

    cdfs_end(cdfs_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cdfs_check_content_consumer_1()
{
    UINT32 cdfs_md_id;

    void * cdfsdn_tcid_vec;

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    
    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));

    cdfsdn_tcid_vec = cvector_new(0, MM_UINT32, 0);
    ASSERT(NULL_PTR != cdfsdn_tcid_vec);
    cdfs_collect_dn_tcid_vec(cdfs_md_id, cdfsdn_tcid_vec);

    test_case_88_cdfs_file_content_checker(cdfs_md_id, g_cbytes_used_num, (char *)"/hansoul01");

    cvector_free(cdfsdn_tcid_vec, 0);

    cdfs_end(cdfs_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cdfs_check_content_consumer_2()
{
    UINT32 cdfs_md_id;

    void * cdfsdn_tcid_vec;

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    
    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));

    cdfsdn_tcid_vec = cvector_new(0, MM_UINT32, 0);
    ASSERT(NULL_PTR != cdfsdn_tcid_vec);
    cdfs_collect_dn_tcid_vec(cdfs_md_id, cdfsdn_tcid_vec);

    test_case_88_cdfs_file_content_checker(cdfs_md_id, g_cbytes_used_num, (char *)"/hansoul02");

    cvector_free(cdfsdn_tcid_vec, 0);

    cdfs_end(cdfs_md_id);

    return (EC_TRUE);
}

EC_BOOL __test_cdfs_cfuse_consumer()
{
    UINT32 cdfs_md_id;

    char *argv[] = {(char *)"hsdfs",  (char *)"/mnt/hsdfs",  (char *)"-d",};
    int argc;

    cdfs_md_id = cdfs_start(CDFS_NP_MIN_NUM);
    ASSERT(ERR_MODULE_ID != cdfs_md_id);
    
    ASSERT(EC_TRUE == cdfs_add_dn_vec(cdfs_md_id));
    ASSERT(EC_TRUE == cdfs_add_npp_vec(cdfs_md_id));

    argc = sizeof(argv)/sizeof(argv[0]);

    task_brd_default_start_cfuse(&argc, argv);
    return (EC_TRUE);
}

int main_mxnp_nxdn(int argc, char **argv)
{
    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_mxnp_nxdn: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }

    /*define specific runner for each (tcid, rank)*/
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.1"), CMPI_CDFS_RANK, __test_cdfs_np_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.20.1"), CMPI_CDFS_RANK, __test_cdfs_np_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.2"), CMPI_CDFS_RANK, __test_cdfs_dn_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.3"), CMPI_CDFS_RANK, __test_cdfs_dn_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.4"), CMPI_CDFS_RANK, __test_cdfs_dn_runner);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.5"), CMPI_CDFS_RANK, __test_cdfs_write_supplier_1);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.6"), CMPI_CDFS_RANK, __test_cdfs_read_consumer_1);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.7"), CMPI_CDFS_RANK, __test_cdfs_write_supplier_2);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.8"), CMPI_CDFS_RANK, __test_cdfs_read_consumer_2);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.91"), CMPI_CDFS_RANK, __test_cdfs_check_replica_consumer_1);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.92"), CMPI_CDFS_RANK, __test_cdfs_check_replica_consumer_2);

    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.101"), CMPI_CDFS_RANK, __test_cdfs_check_content_consumer_1);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.102"), CMPI_CDFS_RANK, __test_cdfs_check_content_consumer_2);

    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.201"), CMPI_CDFS_RANK, __test_cdfs_cfuse_consumer);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.202"), CMPI_CDFS_RANK, __test_cdfs_cfuse_consumer);

    /*start the defined runner on current (tcid, rank)*/
    task_brd_default_start_runner();    

    return (0);
}

int main_cdfs(int argc, char **argv)
{
    main_mxnp_nxdn(argc, argv);
    return (0);
}

int main_csolr(int argc, char **argv)
{
    UINT32 this_tcid;
    UINT32 this_comm;
    UINT32 this_rank;

    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main: validity checking failed\n");
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

        //test_case_61(mod_mgr_def);
        //test_case_62(mod_mgr_def);
        //test_case_63(mod_mgr_def);
        //test_case_64(mod_mgr_def);
        //test_case_66(mod_mgr_def);
        //test_case_67(mod_mgr_def);

        mod_mgr_free(mod_mgr_def);

        do_slave_wait_default();
    }
#if 0
    /*fwd rank entrance*/
    else if (CMPI_FWD_RANK == this_rank)
    {
        sys_log(LOGSTDOUT,"======================================================================\n");
        sys_log(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", uint32_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        sys_log(LOGSTDOUT,"======================================================================\n");

        do_slave_wait_default();
    }
#endif
    /*user define the master process*/
    else if (ipv4_to_uint32("10.10.10.1") == this_tcid && 1 == this_rank)
    {
        UINT32 csolr_md_id;

        csolr_md_id = csolr_start();
        task_brd_default_start_csolr_srv(csolr_md_id, 58111);

        csolr_add_mod(csolr_md_id, ipv4_to_uint32("10.10.10.1"));
        csolr_add_mod(csolr_md_id, ipv4_to_uint32("10.10.10.2"));

        do_slave_wait_default();
    }
    else if (ipv4_to_uint32("10.10.10.2") == this_tcid && 1 == this_rank)
    {
        UINT32 csolr_md_id;

        csolr_md_id = csolr_start();
        task_brd_default_start_csolr_srv(csolr_md_id, 58112);

        csolr_add_mod(csolr_md_id, ipv4_to_uint32("10.10.10.1"));
        csolr_add_mod(csolr_md_id, ipv4_to_uint32("10.10.10.2"));

        do_slave_wait_default();
    }
#if 1
    /*fwd rank entrance*/
    else if (CMPI_FWD_RANK == this_rank)
    {
        sys_log(LOGSTDOUT,"======================================================================\n");
        sys_log(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", uint32_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        sys_log(LOGSTDOUT,"======================================================================\n");

        do_slave_wait_default();
    }
#endif

    /*work process*/
    else
    {
        do_slave_wait_default();
    }

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

