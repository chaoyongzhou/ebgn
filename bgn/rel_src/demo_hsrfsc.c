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
#include "mm.h"
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

#include "crfs.h"
#include "crfsnp.h"
#include "ccurl.h"
#include "crfsc.h"
#include "demo_hsrfsc.h"


#define CRFSC_TEST_TCID_STR ((char *)"10.10.10.1")
#define CRFSC_TEST_RANK     ((UINT32)0)
#define CRFSC_TEST_MODI     ((UINT32)0)

#define CRFSC_ROOT_DIR   ((char *)".")

#define CRFSC_TEST_HOME_1   ((char *)"/h1")
#define CRFSC_TEST_HOME_2   ((char *)"/h2")

#define CCURL_TEST_RANK     ((UINT32)0)
#define CCURL_TEST_MODI     ((UINT32)0)
#define CCURL_TEST_STEP     ((UINT32)128)


static CBYTES   *g_cbytes[32];
static UINT32    g_cbytes_max_len = sizeof(g_cbytes)/sizeof(g_cbytes[0]);

static UINT32    g_curl_num  = ((UINT32)~0);  /*default*/
static UINT32    g_curl_step = CCURL_TEST_STEP; /*default*/
static CSTRING  *g_curl_fname_cstr   = NULL_PTR;/*url file name*/
static int       g_curl_list_fd   = ERR_FD;     /*url file descriptor*/
static UINT32    g_curl_list_fsize = 0;         /*url file size*/
static char     *g_curl_list_buff = NULL_PTR;   /*pointer to the url file mapped memory start*/
static char     *g_curl_list_pcur = NULL_PTR;   /*pointer to current available url str*/
static UINT32    g_curl_list_lcur = 0;          /*length of current available url str*/
static UINT32    g_curl_idx  = 0;               /*default*/

static UINT32    g_rfs_tcid = CMPI_ANY_TCID;

static void print_tcid(LOG *log, const UINT32 tcid)
{
    sys_log(log, "%s\n", c_word_to_ipv4(tcid));
    return;
}

static EC_BOOL __test_crfsc_init_g_cbytes(const UINT32 max_num)
{
    UINT32 pos;
    UINT32 max_cfg_num;

    max_cfg_num = sizeof(g_crfsc_file_cfg_tbl)/sizeof(g_crfsc_file_cfg_tbl[0]);
    if(max_num > max_cfg_num)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfsc_init_g_cbytes: max_num %ld but max_cfg_num %ld\n", max_num, max_cfg_num);
        return (EC_FALSE);
    }

    if(max_num > g_cbytes_max_len)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfsc_init_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
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

        file_name = g_crfsc_file_cfg_tbl[ pos ].file_name;
        file_size = g_crfsc_file_cfg_tbl[ pos ].file_size;

        if(0 != access(file_name, F_OK))
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfsc_init_g_cbytes: file %s not exist or inaccessable\n", file_name);
            return (EC_FALSE);
        }

        fd = c_file_open(file_name, O_RDONLY, 0666);
        if(-1 == fd)
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfsc_init_g_cbytes: open file %s to read failed\n", file_name);
            return (EC_FALSE);
        }

        cbytes = cbytes_new(file_size);
        if((ssize_t)file_size != read(fd, cbytes_buf(cbytes), file_size))
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfsc_init_g_cbytes: read file %s with size %ld failed\n", file_name, file_size);
            cbytes_free(cbytes, 0);
            c_file_close(fd);
            return (EC_FALSE);
        }

        g_cbytes[ pos ] = cbytes;

        c_file_close(fd);
    }

    return (EC_TRUE);
}

static EC_BOOL __test_crfsc_clean_g_cbytes(const UINT32 max_num)
{
    UINT32 pos;
    if(max_num > g_cbytes_max_len)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfsc_clean_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
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

static CBYTES *__test_crfsc_fetch_g_cbytes(const UINT32 max_num, const UINT32 pos)
{
    if(max_num > g_cbytes_max_len)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfsc_fetch_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
        return (NULL_PTR);
    }

    return g_cbytes[ pos ];
}

EC_BOOL test_case_81_crfsc_md5sum(const char *home, const UINT32 crfsc_tcid, const UINT32 crfsc_rank, const UINT32 crfsc_modi, const UINT32 max_test_data_files, UINT32 *counter, UINT32 *file_num_counter, UINT32 *byte_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    CSTRING     *path[CRFSC_TEST_READ_MAX_FILES];
    CMD5_DIGEST *md5sum[CRFSC_TEST_READ_MAX_FILES];/*read from dn*/
    CMD5_DIGEST *md5sum_des[CRFSC_TEST_READ_MAX_FILES];/*benchmark*/
    EC_BOOL      ret[CRFSC_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;


    for(index = 0; index < CRFSC_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]          = NULL_PTR;
        md5sum[ index ]     = NULL_PTR;
        md5sum_des[ index ] = NULL_PTR;
        ret[ index ]           = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(crfsc_tcid, CMPI_ANY_COMM, crfsc_rank, crfsc_modi, mod_mgr);
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_crfsc_read: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfsc_get_npp_mod_mgr(crfsc_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_crfsc_read: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfsc_get_dn_mod_mgr(crfsc_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(index = 0; index < CRFSC_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        CBYTES *cbytes;
        
        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        md5sum[ index ]     = cmd5_digest_new();
        md5sum_des[ index ] = cmd5_digest_new();

        cbytes = __test_crfsc_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));
        cmd5_sum((uint32_t)cbytes_len(cbytes), cbytes_buf(cbytes), CMD5_DIGEST_SUM(md5sum_des[ index ]));

        ret[ index ] = EC_FALSE;

        (*file_num_counter) ++;
        (*byte_num_counter) += cbytes_len(__test_crfsc_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files)));

        task_inc(task_mgr, &(ret[ index ]), FI_crfsc_file_md5sum, ERR_MODULE_ID, path[ index ], md5sum[ index ]);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CRFSC_TEST_READ_MAX_FILES; index ++)
    {
        if(NULL_PTR != md5sum[ index ])
        {
            if(EC_TRUE == cmd5_digest_cmp(md5sum[ index ], md5sum_des[ index ]))
            {
                dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "[SUCC] path: %s\n",
                                  (char *)cstring_get_str(path[ index ]));
            }
            else
            {
                continue_flag = EC_FALSE;

                dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[FAIL] path: %s, ",
                                  (char *)cstring_get_str(path[ index ]));
            }
        }

        if(NULL_PTR != path[ index ])
        {
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
        }

        if(NULL_PTR != md5sum[ index ])
        {
            cmd5_digest_free(md5sum[ index ]);
            md5sum[ index ] = NULL_PTR;
        }

        if(NULL_PTR != md5sum_des[ index ])
        {
            cmd5_digest_free(md5sum_des[ index ]);
            md5sum_des[ index ] = NULL_PTR;
        }
    }

    mod_mgr_free(mod_mgr);
    return (continue_flag);
}

EC_BOOL test_case_82_crfsc_read(const char *home, const UINT32 crfsc_tcid, const UINT32 crfsc_rank, const UINT32 crfsc_modi, const UINT32 max_test_data_files, UINT32 *counter, UINT32 *file_num_counter, UINT32 *byte_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    CSTRING    *path[CRFSC_TEST_READ_MAX_FILES];
    CBYTES     *cbytes[CRFSC_TEST_READ_MAX_FILES];/*read from dn*/
    CBYTES     *cbytes_des[CRFSC_TEST_READ_MAX_FILES];/*benchmark*/
    EC_BOOL     ret[CRFSC_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;
    UINT32      expires_timestamp;

    for(index = 0; index < CRFSC_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]          = NULL_PTR;
        cbytes[ index ]     = NULL_PTR;
        cbytes_des[ index ] = NULL_PTR;
        ret[ index ]           = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(crfsc_tcid, CMPI_ANY_COMM, crfsc_rank, crfsc_modi, mod_mgr);
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_crfsc_read: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfsc_get_npp_mod_mgr(crfsc_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_crfsc_read: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfsc_get_dn_mod_mgr(crfsc_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(index = 0; index < CRFSC_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        UINT32 need_expired_content;
        
        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        cbytes[ index ]     = cbytes_new(0);
        cbytes_des[ index ] = __test_crfsc_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));

        ret[ index ] = EC_FALSE;

        (*file_num_counter) ++;
        (*byte_num_counter) += cbytes_len(cbytes_des[ index ]);

        need_expired_content = EC_TRUE;
        task_inc(task_mgr, &(ret[ index ]), FI_crfsc_read, ERR_MODULE_ID, path[ index ], cbytes[ index ], &expires_timestamp, need_expired_content);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CRFSC_TEST_READ_MAX_FILES; index ++)
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


EC_BOOL test_case_83_crfsc_write(const char *home, const UINT32 crfsc_tcid, const UINT32 crfsc_rank, const UINT32 crfsc_modi, const UINT32 max_test_data_files, UINT32 *counter, UINT32 *file_num_counter, UINT32 *byte_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    EC_BOOL continue_flag;

    CSTRING    *path[CRFSC_TEST_WRITE_MAX_FILES];
    EC_BOOL     ret[CRFSC_TEST_WRITE_MAX_FILES];

    for(index = 0; index < CRFSC_TEST_WRITE_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(crfsc_tcid, CMPI_ANY_COMM, crfsc_rank, crfsc_modi, mod_mgr);

    /*---------  multiple process verfication BEG ---------*/
    //mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_QUE);
    //mod_mgr_incl(crfsc_tcid, CMPI_ANY_COMM, 0, crfsc_modi, mod_mgr);
    //mod_mgr_incl(crfsc_tcid, CMPI_ANY_COMM, 1, crfsc_modi, mod_mgr);
    /*---------  multiple process verfication END ---------*/
    
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_crfsc_write: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfsc_get_npp_mod_mgr(crfsc_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_crfsc_write: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfsc_get_dn_mod_mgr(crfsc_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(index = 0; index < CRFSC_TEST_WRITE_MAX_FILES; index ++, (*counter) ++)
    {
        void *cbytes;

        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        ret[ index ] = EC_FALSE;
        cbytes = __test_crfsc_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));
        if(NULL_PTR == cbytes)
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_83_crfsc_write: crfs buff is null where index = %ld, max_test_data_files = %ld\n", index, max_test_data_files);
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
            break;
        }

        (*file_num_counter) ++;
        (*byte_num_counter) += cbytes_len(cbytes);

        task_inc(task_mgr, &(ret[ index ]), FI_crfsc_write, ERR_MODULE_ID, path[ index ], cbytes, (UINT32)0);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CRFSC_TEST_WRITE_MAX_FILES; index ++)
    {
        if(EC_FALSE == ret[ index ] && NULL_PTR != path[ index ])
        {
            continue_flag = EC_FALSE;
            dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "test_case_83_crfsc_write: [FAIL] %s\n", (char *)cstring_get_str(path[ index ]));
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

EC_BOOL test_case_84_crfsc_delete(const char *home, const UINT32 crfsc_tcid, const UINT32 crfsc_rank, const UINT32 crfsc_modi, UINT32 *counter, UINT32 *file_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    EC_BOOL continue_flag;

    CSTRING    *path[CRFSC_TEST_WRITE_MAX_FILES];
    EC_BOOL     ret[CRFSC_TEST_WRITE_MAX_FILES];

    for(index = 0; index < CRFSC_TEST_WRITE_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(crfsc_tcid, CMPI_ANY_COMM, crfsc_rank, crfsc_modi, mod_mgr);

    /*---------  multiple process verfication BEG ---------*/
    //mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_QUE);
    //mod_mgr_incl(crfsc_tcid, CMPI_ANY_COMM, 0, crfsc_modi, mod_mgr);
    //mod_mgr_incl(crfsc_tcid, CMPI_ANY_COMM, 1, crfsc_modi, mod_mgr);
    /*---------  multiple process verfication END ---------*/
    
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_crfsc_write: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfsc_get_npp_mod_mgr(crfsc_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_crfsc_write: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfsc_get_dn_mod_mgr(crfsc_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(index = 0; index < CRFSC_TEST_WRITE_MAX_FILES; index ++, (*counter) ++)
    {
        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        ret[ index ] = EC_FALSE;

        (*file_num_counter) ++;
        task_inc(task_mgr, &(ret[ index ]), FI_crfsc_delete, ERR_MODULE_ID, path[ index ], CRFSNP_ITEM_FILE_IS_REG);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CRFSC_TEST_WRITE_MAX_FILES; index ++)
    {
        if(EC_FALSE == ret[ index ] && NULL_PTR != path[ index ])
        {
            //continue_flag = EC_FALSE;
            dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "test_case_84_crfsc_delete: [FAIL] %s\n", (char *)cstring_get_str(path[ index ]));
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


EC_BOOL test_case_86_crfsc_writer(const UINT32 crfsc_tcid, const UINT32 crfsc_rank, const UINT32 crfsc_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32  outer_loop;
    UINT32  inner_loop;
    EC_BOOL continue_flag;

    UINT32  file_num_counter;
    UINT32  byte_num_counter;

    if(EC_FALSE == __test_crfsc_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_86_crfsc_writer:__test_crfsc_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_crfsc_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    file_num_counter = 0;
    byte_num_counter = 0;

    continue_flag = EC_TRUE;

    for(outer_loop = 0; outer_loop < CRFSC_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    //for(outer_loop = 1815; outer_loop < CRFSC_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_86_crfsc_writer: outer_loop = %ld, file_num = %ld, byte_num = %ld\n", outer_loop, file_num_counter, byte_num_counter);

        dir0 = (outer_loop % CRFSC_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CRFSC_MAX_FILE_NUM_PER_LOOP) % CRFSC_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CRFSC_MAX_FILE_NUM_PER_LOOP) / CRFSC_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CRFSC_MAX_FILE_NUM_PER_LOOP + CRFSC_TEST_WRITE_MAX_FILES - 1) / CRFSC_TEST_WRITE_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_83_crfsc_write(home, crfsc_tcid, crfsc_rank, crfsc_modi, max_test_data_files, &counter, &file_num_counter, &byte_num_counter);
        }
    }

    __test_crfsc_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_86_crfsc_writer: end\n");

    return (continue_flag);
}

EC_BOOL test_case_87_crfsc_reader(const UINT32 crfsc_tcid, const UINT32 crfsc_rank, const UINT32 crfsc_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    UINT32  file_num_counter;
    UINT32  byte_num_counter;

    if(EC_FALSE == __test_crfsc_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_87_crfsc_reader:__test_crfsc_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_crfsc_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    file_num_counter = 0;
    byte_num_counter = 0;    

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CRFSC_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_87_crfsc_reader: outer_loop = %ld, file_num = %ld, byte_num = %ld\n", outer_loop, file_num_counter, byte_num_counter);

        dir0 = (outer_loop % CRFSC_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CRFSC_MAX_FILE_NUM_PER_LOOP) % CRFSC_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CRFSC_MAX_FILE_NUM_PER_LOOP) / CRFSC_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CRFSC_MAX_FILE_NUM_PER_LOOP + CRFSC_TEST_READ_MAX_FILES - 1) / CRFSC_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_82_crfsc_read(home, crfsc_tcid, crfsc_rank, crfsc_modi, max_test_data_files, &counter, &file_num_counter, &byte_num_counter);
        }
    }

    __test_crfsc_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_87_crfsc_reader: end\n");

    return (continue_flag);
}

EC_BOOL test_case_89_crfsc_delete(const UINT32 crfsc_tcid, const UINT32 crfsc_rank, const UINT32 crfsc_modi, const char *root_dir_in_db)
{
    UINT32  outer_loop;
    UINT32  inner_loop;
    EC_BOOL continue_flag;

    UINT32  file_num_counter;

    file_num_counter = 0;

    continue_flag = EC_TRUE;

    for(outer_loop = 0; outer_loop < CRFSC_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_89_crfsc_delete: outer_loop = %ld, file_num = %ld\n", outer_loop, file_num_counter);

        dir0 = (outer_loop % CRFSC_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CRFSC_MAX_FILE_NUM_PER_LOOP) % CRFSC_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CRFSC_MAX_FILE_NUM_PER_LOOP) / CRFSC_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CRFSC_MAX_FILE_NUM_PER_LOOP + CRFSC_TEST_WRITE_MAX_FILES - 1) / CRFSC_TEST_WRITE_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_84_crfsc_delete(home, crfsc_tcid, crfsc_rank, crfsc_modi, &counter, &file_num_counter);
        }
    }

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_89_crfsc_delete: end\n");

    return (continue_flag);
}

EC_BOOL test_case_90_crfsc_md5sum_checker(const UINT32 crfsc_tcid, const UINT32 crfsc_rank, const UINT32 crfsc_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    UINT32  file_num_counter;
    UINT32  byte_num_counter;

    if(EC_FALSE == __test_crfsc_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_90_crfsc_md5sum_checker:__test_crfsc_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_crfsc_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    file_num_counter = 0;
    byte_num_counter = 0;    

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CRFSC_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_90_crfsc_md5sum_checker: outer_loop = %ld, file_num = %ld, byte_num = %ld\n", outer_loop, file_num_counter, byte_num_counter);

        dir0 = (outer_loop % CRFSC_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CRFSC_MAX_FILE_NUM_PER_LOOP) % CRFSC_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CRFSC_MAX_FILE_NUM_PER_LOOP) / CRFSC_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CRFSC_MAX_FILE_NUM_PER_LOOP + CRFSC_TEST_READ_MAX_FILES - 1) / CRFSC_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_81_crfsc_md5sum(home, crfsc_tcid, crfsc_rank, crfsc_modi, max_test_data_files, &counter, &file_num_counter, &byte_num_counter);
        }
    }

    __test_crfsc_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_90_crfsc_md5sum_checker: end\n");

    return (continue_flag);
}

EC_BOOL __test_crfsc_runner()
{
    UINT32 crfsc_modi;

    CSTRING *crfsc_root_dir;

    crfsc_root_dir = cstring_new((UINT8 *)CRFSC_ROOT_DIR, 0);
    ASSERT(NULL_PTR != crfsc_root_dir);

    crfsc_modi = crfsc_start(crfsc_root_dir);
    ASSERT(ERR_MODULE_ID != crfsc_modi);

    cstring_free(crfsc_root_dir);

    //crfsc_end(crfsc_modi);

    dbg_log(SEC_0137_DEMO, 9)(LOGSTDOUT, "[DEBUG] __test_crfsc_runner: crfsc_modi = %ld\n", crfsc_modi);
    
    return (EC_TRUE);
}

EC_BOOL __test_crfsc_write_supplier_1()
{
    UINT32 crfsc_tcid;
    UINT32 crfsc_rank;
    UINT32 crfsc_modi;

    crfsc_tcid = c_ipv4_to_word(CRFSC_TEST_TCID_STR);
    crfsc_rank = CRFSC_TEST_RANK;
    crfsc_modi = CRFSC_TEST_MODI;

    test_case_86_crfsc_writer(crfsc_tcid, crfsc_rank, crfsc_modi, g_crfsc_cbytes_used_num, CRFSC_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_crfsc_write_supplier_2()
{
    UINT32 crfsc_tcid;
    UINT32 crfsc_rank;
    UINT32 crfsc_modi;

    crfsc_tcid = c_ipv4_to_word(CRFSC_TEST_TCID_STR);
    crfsc_rank = CRFSC_TEST_RANK;
    crfsc_modi = CRFSC_TEST_MODI;

    test_case_86_crfsc_writer(crfsc_tcid, crfsc_rank, crfsc_modi, g_crfsc_cbytes_used_num, CRFSC_TEST_HOME_2);

    return (EC_TRUE);
}

EC_BOOL __test_crfsc_read_consumer_1()
{
    UINT32 crfsc_tcid;
    UINT32 crfsc_rank;
    UINT32 crfsc_modi;

    crfsc_tcid = c_ipv4_to_word(CRFSC_TEST_TCID_STR);
    crfsc_rank = CRFSC_TEST_RANK;
    crfsc_modi = CRFSC_TEST_MODI;

    test_case_87_crfsc_reader(crfsc_tcid, crfsc_rank, crfsc_modi, g_crfsc_cbytes_used_num, CRFSC_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_crfsc_read_consumer_2()
{
    UINT32 crfsc_tcid;
    UINT32 crfsc_rank;
    UINT32 crfsc_modi;

    crfsc_tcid = c_ipv4_to_word(CRFSC_TEST_TCID_STR);
    crfsc_rank = CRFSC_TEST_RANK;
    crfsc_modi = CRFSC_TEST_MODI;

    test_case_87_crfsc_reader(crfsc_tcid, crfsc_rank, crfsc_modi, g_crfsc_cbytes_used_num, CRFSC_TEST_HOME_2);

    return (EC_TRUE);
}

EC_BOOL __test_crfsc_delete_consumer_1()
{
    UINT32 crfsc_tcid;
    UINT32 crfsc_rank;
    UINT32 crfsc_modi;

    crfsc_tcid = c_ipv4_to_word(CRFSC_TEST_TCID_STR);
    crfsc_rank = CRFSC_TEST_RANK;
    crfsc_modi = CRFSC_TEST_MODI;

    test_case_89_crfsc_delete(crfsc_tcid, crfsc_rank, crfsc_modi, CRFSC_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_crfsc_delete_consumer_2()
{
    UINT32 crfsc_tcid;
    UINT32 crfsc_rank;
    UINT32 crfsc_modi;

    crfsc_tcid = c_ipv4_to_word(CRFSC_TEST_TCID_STR);
    crfsc_rank = CRFSC_TEST_RANK;
    crfsc_modi = CRFSC_TEST_MODI;

    test_case_89_crfsc_delete(crfsc_tcid, crfsc_rank, crfsc_modi, CRFSC_TEST_HOME_2);

    return (EC_TRUE);
}

EC_BOOL __test_crfsc_check_content_md5sum_1()
{
    UINT32 crfsc_tcid;
    UINT32 crfsc_rank;
    UINT32 crfsc_modi;

    crfsc_tcid = c_ipv4_to_word(CRFSC_TEST_TCID_STR);
    crfsc_rank = CRFSC_TEST_RANK;
    crfsc_modi = CRFSC_TEST_MODI;

    test_case_90_crfsc_md5sum_checker(crfsc_tcid, crfsc_rank, crfsc_modi, g_crfsc_cbytes_used_num, CRFSC_TEST_HOME_1);

    return (EC_TRUE);
}


EC_BOOL test_case_91_crfsc_write(MOD_MGR  *src_mod_mgr, 
                                      const UINT32 ccurl_tcid, 
                                      const UINT32 ccurl_rank, 
                                      const UINT32 ccurl_modi, 
                                      const UINT32 ccurl_mod_num, 
                                      const CSTRING *proxy_ip_port_cstr,
                                      CVECTOR  *path_cstr_vec,
                                      CVECTOR  *body_cbytes_vec)
{
    MOD_NODE  crfsc_mod_node;
    TASK_MGR *task_mgr;
    UINT32    path_num;
    UINT32    path_idx;

    CVECTOR  *crfsc_ret_vec;

    crfsc_ret_vec = cvector_new(0, MM_UINT32, LOC_DEMO_0027);
    ASSERT(NULL_PTR != crfsc_ret_vec);    

    MOD_NODE_TCID(&crfsc_mod_node) = c_ipv4_to_word(CRFSC_TEST_TCID_STR);
    MOD_NODE_COMM(&crfsc_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&crfsc_mod_node) = CMPI_CRFS_RANK;
    MOD_NODE_MODI(&crfsc_mod_node) = 0;

    /*write file content to rfs*/
    task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);    
    path_num = cvector_size(path_cstr_vec);
    for(path_idx = 0; path_idx < path_num; path_idx ++)
    {   
        EC_BOOL *ret;
        CSTRING *path_cstr;        
        CBYTES  *body_cbytes;

        alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT32, &ret, LOC_DEMO_0028);
        cvector_push(crfsc_ret_vec, (void *)ret);
        (*ret) = EC_FALSE;

        path_cstr = (CSTRING *)cvector_get(path_cstr_vec, path_idx);
        if(NULL_PTR == path_cstr)
        {
            continue;
        }        

        body_cbytes =(CBYTES *)cvector_get(body_cbytes_vec, path_idx);
        if(NULL_PTR == body_cbytes)
        {
            continue;
        }

        task_p2p_inc(task_mgr, CMPI_ANY_MODI, &crfsc_mod_node, 
                    ret, FI_crfsc_write, ERR_MODULE_ID, path_cstr, body_cbytes, (UINT32)0);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    /*check success*/
    for(path_idx = 0; path_idx < path_num; path_idx ++)
    {
        UINT32    *ret;
        CSTRING   *path_cstr;        
        CBYTES    *body_cbytes;

        ret         = (EC_BOOL *)cvector_get(crfsc_ret_vec, path_idx);
        body_cbytes = (CBYTES  *)cvector_get(body_cbytes_vec, path_idx);
        path_cstr   = (CSTRING *)cvector_get(path_cstr_vec, path_idx);

        if(EC_TRUE == (*ret))
        {
            sys_log(LOGUSER05, "[rank_%s_%ld][RFS SUCC] [%8ld] path %s, len %8ld\n", 
                               MOD_NODE_TCID_STR(&crfsc_mod_node),MOD_NODE_RANK(&crfsc_mod_node),path_idx,
                               (char *)cstring_get_str(path_cstr),
                               cbytes_len(body_cbytes));
        }
        else
        {
            sys_log(LOGUSER05, "[rank_%s_%ld][RFS FAIL] [%8ld] path %s\n", 
                               MOD_NODE_TCID_STR(&crfsc_mod_node),MOD_NODE_RANK(&crfsc_mod_node),path_idx,
                               (char *)cstring_get_str(path_cstr));
        }

        cvector_set(crfsc_ret_vec, path_idx, NULL_PTR);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT32, ret, LOC_DEMO_0029);
    }

    cvector_free(crfsc_ret_vec, LOC_DEMO_0030);
    
    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "test_case_91_crfsc_write: end\n");

    return (EC_TRUE);    
}


int main_crfsc(int argc, char **argv)
{
    UINT32 crfsc_rank;
    UINT32 tester_rank;
    
    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:main_crfsc: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }

    dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "[DEBUG] main_crfsc: validity checking done\n");

    crfsc_rank = CRFSC_TEST_RANK;
    tester_rank = 0;

    /*define specific runner for each (tcid, rank)*/
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.1"), CMPI_ANY_RANK  , __test_crfsc_runner);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.20.1"), CMPI_ANY_RANK  , __test_crfsc_runner);

    task_brd_default_add_runner(c_ipv4_to_word("10.10.30.1"), CMPI_ANY_RANK  , __test_crfsc_runner);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.40.1"), CMPI_ANY_RANK  , __test_crfsc_runner);    
    
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.2"), tester_rank, __test_crfsc_write_supplier_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.3"), tester_rank, __test_crfsc_read_consumer_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.4"), tester_rank, __test_crfsc_delete_consumer_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.5"), tester_rank, __test_crfsc_check_content_md5sum_1);
    
    task_brd_default_add_runner(c_ipv4_to_word("10.10.20.2"), tester_rank, __test_crfsc_write_supplier_2);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.20.3"), tester_rank, __test_crfsc_read_consumer_2);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.20.5"), tester_rank, __test_crfsc_delete_consumer_2);

    //task_brd_default_add_runner(c_ipv4_to_word("10.10.10.101"), tester_rank, __test_crfsc_check_content_consumer_1);
    //task_brd_default_add_runner(c_ipv4_to_word("10.10.10.102"), tester_rank, __test_crfsc_check_content_consumer_2);
    
    /*start the defined runner on current (tcid, rank)*/
    task_brd_default_start_runner();   

    return (0);
}

int main_crfscc_x(int argc, char **argv)
{
    main_crfsc(argc, argv);
    return (0);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

