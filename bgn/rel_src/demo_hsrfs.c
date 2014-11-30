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
#include "demo_hsrfs.h"


#define CRFS_TEST_TCID_STR ((char *)"10.10.10.1")
#define CRFS_TEST_RANK     ((UINT32)0)
#define CRFS_TEST_MODI     ((UINT32)0)

#define CRFS_ROOT_DIR   ((char *)".")

#define CRFS_TEST_HOME_1   ((char *)"/h1")
#define CRFS_TEST_HOME_2   ((char *)"/h2")

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

static EC_BOOL __test_crfs_init_g_cbytes(const UINT32 max_num)
{
    UINT32 pos;
    UINT32 max_cfg_num;

    max_cfg_num = sizeof(g_crfs_file_cfg_tbl)/sizeof(g_crfs_file_cfg_tbl[0]);
    if(max_num > max_cfg_num)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfs_init_g_cbytes: max_num %ld but max_cfg_num %ld\n", max_num, max_cfg_num);
        return (EC_FALSE);
    }

    if(max_num > g_cbytes_max_len)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfs_init_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
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

        file_name = g_crfs_file_cfg_tbl[ pos ].file_name;
        file_size = g_crfs_file_cfg_tbl[ pos ].file_size;

        if(0 != access(file_name, F_OK))
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfs_init_g_cbytes: file %s not exist or inaccessable\n", file_name);
            return (EC_FALSE);
        }

        fd = c_file_open(file_name, O_RDONLY, 0666);
        if(-1 == fd)
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfs_init_g_cbytes: open file %s to read failed\n", file_name);
            return (EC_FALSE);
        }

        cbytes = cbytes_new(file_size);
        if((ssize_t)file_size != read(fd, cbytes_buf(cbytes), file_size))
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfs_init_g_cbytes: read file %s with size %ld failed\n", file_name, file_size);
            cbytes_free(cbytes, 0);
            c_file_close(fd);
            return (EC_FALSE);
        }

        g_cbytes[ pos ] = cbytes;

        c_file_close(fd);
    }

    return (EC_TRUE);
}

static EC_BOOL __test_crfs_clean_g_cbytes(const UINT32 max_num)
{
    UINT32 pos;
    if(max_num > g_cbytes_max_len)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfs_clean_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
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

static CBYTES *__test_crfs_fetch_g_cbytes(const UINT32 max_num, const UINT32 pos)
{
    if(max_num > g_cbytes_max_len)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_crfs_fetch_g_cbytes: max_num %ld but g_cbytes_max_len %ld\n", max_num, g_cbytes_max_len);
        return (NULL_PTR);
    }

    return g_cbytes[ pos ];
}

EC_BOOL test_case_81_crfs_md5sum(const char *home, const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, const UINT32 max_test_data_files, UINT32 *counter, UINT32 *file_num_counter, UINT32 *byte_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    CSTRING     *path[CRFS_TEST_READ_MAX_FILES];
    CMD5_DIGEST *md5sum[CRFS_TEST_READ_MAX_FILES];/*read from dn*/
    CMD5_DIGEST *md5sum_des[CRFS_TEST_READ_MAX_FILES];/*benchmark*/
    EC_BOOL      ret[CRFS_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;


    for(index = 0; index < CRFS_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]          = NULL_PTR;
        md5sum[ index ]     = NULL_PTR;
        md5sum_des[ index ] = NULL_PTR;
        ret[ index ]           = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(crfs_tcid, CMPI_ANY_COMM, crfs_rank, crfs_modi, mod_mgr);
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_crfs_read: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_npp_mod_mgr(crfs_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_crfs_read: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_dn_mod_mgr(crfs_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(index = 0; index < CRFS_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        CBYTES *cbytes;
        
        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        md5sum[ index ]     = cmd5_digest_new();
        md5sum_des[ index ] = cmd5_digest_new();

        cbytes = __test_crfs_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));
        cmd5_sum((uint32_t)cbytes_len(cbytes), cbytes_buf(cbytes), CMD5_DIGEST_SUM(md5sum_des[ index ]));

        ret[ index ] = EC_FALSE;

        (*file_num_counter) ++;
        (*byte_num_counter) += cbytes_len(__test_crfs_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files)));

        task_inc(task_mgr, &(ret[ index ]), FI_crfs_file_md5sum, ERR_MODULE_ID, path[ index ], md5sum[ index ]);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CRFS_TEST_READ_MAX_FILES; index ++)
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

EC_BOOL test_case_82_crfs_read(const char *home, const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, const UINT32 max_test_data_files, UINT32 *counter, UINT32 *file_num_counter, UINT32 *byte_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    CSTRING    *path[CRFS_TEST_READ_MAX_FILES];
    CBYTES     *cbytes[CRFS_TEST_READ_MAX_FILES];/*read from dn*/
    CBYTES     *cbytes_des[CRFS_TEST_READ_MAX_FILES];/*benchmark*/
    EC_BOOL     ret[CRFS_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;
    UINT32      expires_timestamp;

    for(index = 0; index < CRFS_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]          = NULL_PTR;
        cbytes[ index ]     = NULL_PTR;
        cbytes_des[ index ] = NULL_PTR;
        ret[ index ]           = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(crfs_tcid, CMPI_ANY_COMM, crfs_rank, crfs_modi, mod_mgr);
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_crfs_read: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_npp_mod_mgr(crfs_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_82_crfs_read: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_dn_mod_mgr(crfs_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    for(index = 0; index < CRFS_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        UINT32 need_expired_content;
        
        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        cbytes[ index ]     = cbytes_new(0);
        cbytes_des[ index ] = __test_crfs_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));

        ret[ index ] = EC_FALSE;

        (*file_num_counter) ++;
        (*byte_num_counter) += cbytes_len(cbytes_des[ index ]);

        need_expired_content = EC_TRUE;
        task_inc(task_mgr, &(ret[ index ]), FI_crfs_read, ERR_MODULE_ID, path[ index ], cbytes[ index ], &expires_timestamp, need_expired_content);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CRFS_TEST_READ_MAX_FILES; index ++)
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


EC_BOOL test_case_83_crfs_write(const char *home, const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, const UINT32 max_test_data_files, UINT32 *counter, UINT32 *file_num_counter, UINT32 *byte_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    EC_BOOL continue_flag;

    CSTRING    *path[CRFS_TEST_WRITE_MAX_FILES];
    EC_BOOL     ret[CRFS_TEST_WRITE_MAX_FILES];

    for(index = 0; index < CRFS_TEST_WRITE_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(crfs_tcid, CMPI_ANY_COMM, crfs_rank, crfs_modi, mod_mgr);

    /*---------  multiple process verfication BEG ---------*/
    //mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_QUE);
    //mod_mgr_incl(crfs_tcid, CMPI_ANY_COMM, 0, crfs_modi, mod_mgr);
    //mod_mgr_incl(crfs_tcid, CMPI_ANY_COMM, 1, crfs_modi, mod_mgr);
    /*---------  multiple process verfication END ---------*/
    
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_crfs_write: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_npp_mod_mgr(crfs_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_crfs_write: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_dn_mod_mgr(crfs_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(index = 0; index < CRFS_TEST_WRITE_MAX_FILES; index ++, (*counter) ++)
    {
        void *cbytes;

        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        ret[ index ] = EC_FALSE;
        cbytes = __test_crfs_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));
        if(NULL_PTR == cbytes)
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_83_crfs_write: crfs buff is null where index = %ld, max_test_data_files = %ld\n", index, max_test_data_files);
            cstring_free(path[ index ]);
            path[ index ] = NULL_PTR;
            break;
        }

        (*file_num_counter) ++;
        (*byte_num_counter) += cbytes_len(cbytes);

        task_inc(task_mgr, &(ret[ index ]), FI_crfs_write, ERR_MODULE_ID, path[ index ], cbytes, (UINT32)0);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CRFS_TEST_WRITE_MAX_FILES; index ++)
    {
        if(EC_FALSE == ret[ index ] && NULL_PTR != path[ index ])
        {
            continue_flag = EC_FALSE;
            dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "test_case_83_crfs_write: [FAIL] %s\n", (char *)cstring_get_str(path[ index ]));
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

EC_BOOL test_case_84_crfs_delete(const char *home, const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, UINT32 *counter, UINT32 *file_num_counter)
{
    void *mod_mgr;
    void *task_mgr;

    UINT32 index;

    EC_BOOL continue_flag;

    CSTRING    *path[CRFS_TEST_WRITE_MAX_FILES];
    EC_BOOL     ret[CRFS_TEST_WRITE_MAX_FILES];

    for(index = 0; index < CRFS_TEST_WRITE_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(crfs_tcid, CMPI_ANY_COMM, crfs_rank, crfs_modi, mod_mgr);

    /*---------  multiple process verfication BEG ---------*/
    //mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_QUE);
    //mod_mgr_incl(crfs_tcid, CMPI_ANY_COMM, 0, crfs_modi, mod_mgr);
    //mod_mgr_incl(crfs_tcid, CMPI_ANY_COMM, 1, crfs_modi, mod_mgr);
    /*---------  multiple process verfication END ---------*/
    
#if 0
    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_crfs_write: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_npp_mod_mgr(crfs_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_83_crfs_write: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_dn_mod_mgr(crfs_modi));
#endif
    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(index = 0; index < CRFS_TEST_WRITE_MAX_FILES; index ++, (*counter) ++)
    {
        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        ret[ index ] = EC_FALSE;

        (*file_num_counter) ++;
        task_inc(task_mgr, &(ret[ index ]), FI_crfs_delete, ERR_MODULE_ID, path[ index ], CRFSNP_ITEM_FILE_IS_REG);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CRFS_TEST_WRITE_MAX_FILES; index ++)
    {
        if(EC_FALSE == ret[ index ] && NULL_PTR != path[ index ])
        {
            //continue_flag = EC_FALSE;
            dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "test_case_84_crfs_delete: [FAIL] %s\n", (char *)cstring_get_str(path[ index ]));
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
EC_BOOL test_case_85_crfs_check_file_content(const char *home, const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, const UINT32 max_test_data_files, UINT32 *counter)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 index;

    CSTRING    *path[CRFS_TEST_READ_MAX_FILES];
    EC_BOOL     ret[CRFS_TEST_READ_MAX_FILES];

    EC_BOOL     continue_flag;

    for(index = 0; index < CRFS_TEST_READ_MAX_FILES; index ++)
    {
        path[ index ]      = NULL_PTR;
        ret[ index ]       = EC_FALSE;
    }

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
    mod_mgr_incl(crfs_tcid, CMPI_ANY_COMM, crfs_rank, crfs_modi, mod_mgr);

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_85_crfs_check_file_content: npp mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_npp_mod_mgr(crfs_modi));

    dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "test_case_85_crfs_check_file_content: dn mod mgr is\n");
    mod_mgr_print(LOGSTDOUT, crfs_get_dn_mod_mgr(crfs_modi));

    task_mgr = task_new(mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(index = 0; index < CRFS_TEST_READ_MAX_FILES; index ++, (*counter) ++)
    {
        CBYTES *cbytes_des;
        cbytes_des = __test_crfs_fetch_g_cbytes(max_test_data_files, ((*counter) % max_test_data_files));

        path[ index ] = cstring_new(NULL_PTR, 0);
        cstring_format(path[ index ], "%s/%ld.dat", home, (*counter));

        ret[ index ] = EC_FALSE;

        task_inc(task_mgr, &(ret[ index ]),
                        FI_crfs_check_file_is, ERR_MODULE_ID, path[ index ], cbytes_des);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

    continue_flag = EC_TRUE;

    for(index = 0; index < CRFS_TEST_READ_MAX_FILES; index ++)
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

EC_BOOL test_case_86_crfs_writer(const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32  outer_loop;
    UINT32  inner_loop;
    EC_BOOL continue_flag;

    UINT32  file_num_counter;
    UINT32  byte_num_counter;

    if(EC_FALSE == __test_crfs_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_86_crfs_writer:__test_crfs_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_crfs_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    file_num_counter = 0;
    byte_num_counter = 0;

    continue_flag = EC_TRUE;

    for(outer_loop = 0; outer_loop < CRFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_86_crfs_writer: outer_loop = %ld, file_num = %ld, byte_num = %ld\n", outer_loop, file_num_counter, byte_num_counter);

        dir0 = (outer_loop % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) / CRFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CRFS_MAX_FILE_NUM_PER_LOOP + CRFS_TEST_WRITE_MAX_FILES - 1) / CRFS_TEST_WRITE_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_83_crfs_write(home, crfs_tcid, crfs_rank, crfs_modi, max_test_data_files, &counter, &file_num_counter, &byte_num_counter);
        }
    }

    __test_crfs_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_86_crfs_writer: end\n");

    return (continue_flag);
}

EC_BOOL test_case_87_crfs_reader(const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    UINT32  file_num_counter;
    UINT32  byte_num_counter;

    if(EC_FALSE == __test_crfs_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_87_crfs_reader:__test_crfs_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_crfs_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    file_num_counter = 0;
    byte_num_counter = 0;    

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CRFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_87_crfs_reader: outer_loop = %ld, file_num = %ld, byte_num = %ld\n", outer_loop, file_num_counter, byte_num_counter);

        dir0 = (outer_loop % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) / CRFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CRFS_MAX_FILE_NUM_PER_LOOP + CRFS_TEST_READ_MAX_FILES - 1) / CRFS_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_82_crfs_read(home, crfs_tcid, crfs_rank, crfs_modi, max_test_data_files, &counter, &file_num_counter, &byte_num_counter);
        }
    }

    __test_crfs_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_87_crfs_reader: end\n");

    return (continue_flag);
}

EC_BOOL test_case_88_crfs_file_content_checker(const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    if(EC_FALSE == __test_crfs_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_88_crfs_file_content_checker:__test_crfs_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_crfs_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CRFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_88_crfs_file_content_checker: outer_loop = %ld\n", outer_loop);

        dir0 = (outer_loop % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) / CRFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CRFS_MAX_FILE_NUM_PER_LOOP + CRFS_TEST_READ_MAX_FILES - 1) / CRFS_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_85_crfs_check_file_content(home, crfs_tcid, crfs_rank, crfs_modi, max_test_data_files, &counter);
        }
    }

    __test_crfs_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_88_crfs_file_content_checker: end\n");

    return (EC_TRUE);
}

EC_BOOL test_case_89_crfs_delete(const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, const char *root_dir_in_db)
{
    UINT32  outer_loop;
    UINT32  inner_loop;
    EC_BOOL continue_flag;

    UINT32  file_num_counter;

    file_num_counter = 0;

    continue_flag = EC_TRUE;

    for(outer_loop = 0; outer_loop < CRFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_89_crfs_delete: outer_loop = %ld, file_num = %ld\n", outer_loop, file_num_counter);

        dir0 = (outer_loop % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) / CRFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CRFS_MAX_FILE_NUM_PER_LOOP + CRFS_TEST_WRITE_MAX_FILES - 1) / CRFS_TEST_WRITE_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_84_crfs_delete(home, crfs_tcid, crfs_rank, crfs_modi, &counter, &file_num_counter);
        }
    }

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_89_crfs_delete: end\n");

    return (continue_flag);
}

EC_BOOL test_case_90_crfs_md5sum_checker(const UINT32 crfs_tcid, const UINT32 crfs_rank, const UINT32 crfs_modi, const UINT32 max_test_data_files, const char *root_dir_in_db)
{
    UINT32 outer_loop;
    UINT32 inner_loop;

    EC_BOOL continue_flag;

    UINT32  file_num_counter;
    UINT32  byte_num_counter;

    if(EC_FALSE == __test_crfs_init_g_cbytes(max_test_data_files))
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:test_case_90_crfs_md5sum_checker:__test_crfs_init_g_cbytes failed where max_test_data_files = %ld\n", max_test_data_files);

        __test_crfs_clean_g_cbytes(max_test_data_files);
        return (EC_FALSE);
    }

    file_num_counter = 0;
    byte_num_counter = 0;    

    continue_flag = EC_TRUE;
    for(outer_loop = 0; outer_loop < CRFS_TEST_LOOP_MAX_TIMES && EC_TRUE == continue_flag; outer_loop ++)
    {
        char home[64];
        UINT32 counter;

        UINT32 dir0;
        UINT32 dir1;
        UINT32 dir2;

        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_90_crfs_md5sum_checker: outer_loop = %ld, file_num = %ld, byte_num = %ld\n", outer_loop, file_num_counter, byte_num_counter);

        dir0 = (outer_loop % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir1 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) % CRFS_MAX_FILE_NUM_PER_LOOP);
        dir2 = ((outer_loop / CRFS_MAX_FILE_NUM_PER_LOOP) / CRFS_MAX_FILE_NUM_PER_LOOP);

        snprintf(home, sizeof(home), "%s/%ld/%ld/%ld", root_dir_in_db, dir2, dir1, dir0);

        counter = 0;
        for(inner_loop = 0; inner_loop < ((CRFS_MAX_FILE_NUM_PER_LOOP + CRFS_TEST_READ_MAX_FILES - 1) / CRFS_TEST_READ_MAX_FILES) && EC_TRUE == continue_flag; inner_loop ++)
        {
            continue_flag = test_case_81_crfs_md5sum(home, crfs_tcid, crfs_rank, crfs_modi, max_test_data_files, &counter, &file_num_counter, &byte_num_counter);
        }
    }

    __test_crfs_clean_g_cbytes(max_test_data_files);

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] test_case_90_crfs_md5sum_checker: end\n");

    return (continue_flag);
}

EC_BOOL __test_crfs_runner()
{
    UINT32 crfs_modi;

    CSTRING *crfs_root_dir;

    crfs_root_dir = cstring_new((UINT8 *)CRFS_ROOT_DIR, 0);
    ASSERT(NULL_PTR != crfs_root_dir);

    crfs_modi = crfs_start(crfs_root_dir);
    ASSERT(ERR_MODULE_ID != crfs_modi);

    cstring_free(crfs_root_dir);

    //crfs_end(crfs_modi);

    dbg_log(SEC_0137_DEMO, 9)(LOGSTDOUT, "[DEBUG] __test_crfs_runner: crfs_modi = %ld\n", crfs_modi);
    
    return (EC_TRUE);
}

EC_BOOL __test_crfs_write_supplier_1()
{
    UINT32 crfs_tcid;
    UINT32 crfs_rank;
    UINT32 crfs_modi;

    crfs_tcid = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    crfs_rank = CRFS_TEST_RANK;
    crfs_modi = CRFS_TEST_MODI;

    test_case_86_crfs_writer(crfs_tcid, crfs_rank, crfs_modi, g_crfs_cbytes_used_num, CRFS_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_crfs_write_supplier_2()
{
    UINT32 crfs_tcid;
    UINT32 crfs_rank;
    UINT32 crfs_modi;

    crfs_tcid = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    crfs_rank = CRFS_TEST_RANK;
    crfs_modi = CRFS_TEST_MODI;

    test_case_86_crfs_writer(crfs_tcid, crfs_rank, crfs_modi, g_crfs_cbytes_used_num, CRFS_TEST_HOME_2);

    return (EC_TRUE);
}

EC_BOOL __test_crfs_read_consumer_1()
{
    UINT32 crfs_tcid;
    UINT32 crfs_rank;
    UINT32 crfs_modi;

    crfs_tcid = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    crfs_rank = CRFS_TEST_RANK;
    crfs_modi = CRFS_TEST_MODI;

    test_case_87_crfs_reader(crfs_tcid, crfs_rank, crfs_modi, g_crfs_cbytes_used_num, CRFS_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_crfs_read_consumer_2()
{
    UINT32 crfs_tcid;
    UINT32 crfs_rank;
    UINT32 crfs_modi;

    crfs_tcid = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    crfs_rank = CRFS_TEST_RANK;
    crfs_modi = CRFS_TEST_MODI;

    test_case_87_crfs_reader(crfs_tcid, crfs_rank, crfs_modi, g_crfs_cbytes_used_num, CRFS_TEST_HOME_2);

    return (EC_TRUE);
}

EC_BOOL __test_crfs_delete_consumer_1()
{
    UINT32 crfs_tcid;
    UINT32 crfs_rank;
    UINT32 crfs_modi;

    crfs_tcid = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    crfs_rank = CRFS_TEST_RANK;
    crfs_modi = CRFS_TEST_MODI;

    test_case_89_crfs_delete(crfs_tcid, crfs_rank, crfs_modi, CRFS_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_crfs_delete_consumer_2()
{
    UINT32 crfs_tcid;
    UINT32 crfs_rank;
    UINT32 crfs_modi;

    crfs_tcid = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    crfs_rank = CRFS_TEST_RANK;
    crfs_modi = CRFS_TEST_MODI;

    test_case_89_crfs_delete(crfs_tcid, crfs_rank, crfs_modi, CRFS_TEST_HOME_2);

    return (EC_TRUE);
}

EC_BOOL __test_crfs_check_content_consumer_1()
{
    UINT32 crfs_tcid;
    UINT32 crfs_rank;
    UINT32 crfs_modi;

    crfs_tcid = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    crfs_rank = CRFS_TEST_RANK;
    crfs_modi = CRFS_TEST_MODI;

    test_case_88_crfs_file_content_checker(crfs_tcid, crfs_rank, crfs_modi, g_crfs_cbytes_used_num, CRFS_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_crfs_check_content_consumer_2()
{
    UINT32 crfs_tcid;
    UINT32 crfs_rank;
    UINT32 crfs_modi;

    crfs_tcid = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    crfs_rank = CRFS_TEST_RANK;
    crfs_modi = CRFS_TEST_MODI;

    test_case_88_crfs_file_content_checker(crfs_tcid, crfs_rank, crfs_modi, g_crfs_cbytes_used_num, CRFS_TEST_HOME_2);

    return (EC_TRUE);
}

EC_BOOL __test_crfs_check_content_md5sum_1()
{
    UINT32 crfs_tcid;
    UINT32 crfs_rank;
    UINT32 crfs_modi;

    crfs_tcid = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    crfs_rank = CRFS_TEST_RANK;
    crfs_modi = CRFS_TEST_MODI;

    test_case_90_crfs_md5sum_checker(crfs_tcid, crfs_rank, crfs_modi, g_crfs_cbytes_used_num, CRFS_TEST_HOME_1);

    return (EC_TRUE);
}

EC_BOOL __test_ccurl_fetch_url_cstr0(CSTRING *url_cstr)
{
    UINT32 url_num;
    static UINT32 url_idx = 0;
    static const char *url_tab[] = 
    {
        "http://www.autoimg.cn/album/2009/5/17/500_1a4da7c7-b036-4b88-b9f1-0c6957f1cddb.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_46ddd425-1ec0-4f8f-a0d1-742d676d83a5.gif",
        "http://www.autoimg.cn/album/2009/5/17/500_6aad40f7-2ca1-47fc-a849-5ce2ba3357ee.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_1bee4e6e-1166-4056-90dc-df76414d4ce7.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_e17b3791-580d-4736-b5f6-e3fcc3e3b575.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_eaf80ec3-c7a4-4c1f-a9f8-ccf406d27211.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_fa7deedc-4c9c-4742-94f0-5203b1a4cbda.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_fbebe4b0-b4ac-47d2-bfb4-c89596aec5f1.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_731a8d39-9665-413f-b6a9-c98dffb11efa.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_0b2aa587-63a6-4515-a0a2-686577db639b.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_8beb98c6-af0d-475d-9d53-406d63b2d62b.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_587dc0e7-10ab-4bd8-9621-613ca1ab158a.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_09d10fa5-5340-4dad-b649-bfb78fc65bab.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_2cee3e43-42d0-4b34-b2ab-1aa49f0335b0.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_698b8b8a-8b4c-46d6-9001-724cb03e27bc.gif",
        "http://www.autoimg.cn/album/2009/5/17/500_361170f7-c841-4273-bed7-21c65d17d1eb.gif",
        "http://www.autoimg.cn/album/2009/5/17/500_a361c8ac-1bfd-4e9f-8681-d32b2c230638.gif",
        "http://www.autoimg.cn/album/2009/5/17/500_2eb3b83b-2037-46d9-844b-25e399ea8b85.gif",
        "http://www.autoimg.cn/album/2009/5/17/500_4d2a075e-fbb0-4e50-bd82-1fb98f5d8861.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_05752550-a321-4002-9aa7-0bc286013f9a.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_ba85a1eb-d332-4550-9303-bc4487f23dd5.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_0ce0e865-7a16-4984-a972-4e7d040743ea.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_32058de5-0b23-4b50-9341-9847c682ee54.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_0084fa1e-45c0-45a5-9ccd-3c900933eff8.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_0e46035e-dc29-4ba3-9594-7bbb52341cf0.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_3385f535-d716-4b24-86ac-7eed9f5d212d.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_6d19a3fa-5e03-4150-a3ca-48244d3b6eb8.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_8f5f9e5b-382e-4936-8ead-eff454b19959.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_372ae7f1-09de-42d3-af76-32db5c2b583d.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_2bd06302-4012-4a2d-96c5-b3d795f9d458.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_e8668ce2-b8ba-41a3-bc6b-737230816e85.jpg",
        "http://www.autoimg.cn/album/2009/5/17/500_217aa0c6-283a-4867-a11b-3f95c150485c.jpg",    
    };

    const char *url_str;

    url_str  = url_tab[ url_idx ];
    cstring_init(url_cstr, (UINT8 *)url_str);

    url_num = sizeof(url_tab)/sizeof(url_tab[0]);

    url_idx = ((url_idx + 1) % url_num);
    return (EC_TRUE);
}

EC_BOOL __test_ccurl_next_url()
{
    g_curl_list_pcur += g_curl_list_lcur + 1;
    g_curl_list_lcur  = c_line_len(g_curl_list_pcur);

    return (EC_TRUE);
}

EC_BOOL __test_ccurl_seek_url_list_file()
{
    UINT32 idx;
    if(NULL_PTR == g_curl_list_buff)
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_ccurl_seek_url_list_file: g_curl_list_buff is null\n");
        return (EC_FALSE);
    }

    g_curl_list_pcur = g_curl_list_buff;
    g_curl_list_lcur = c_line_len(g_curl_list_pcur);
    for(idx = 0; idx < g_curl_idx; idx ++)
    {
        __test_ccurl_next_url();
    }

    return (EC_TRUE);
}

EC_BOOL __test_ccurl_open_url_list_file()
{
    if(ERR_FD == g_curl_list_fd)
    {
        char *fname;
        char *ftext;
        int   fd;
        UINT32 fsize;
        
        if(NULL_PTR == g_curl_fname_cstr)
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_ccurl_open_url_list_file: pls specific url list file name\n");
            return (EC_FALSE);
        }

        fname = (char *)cstring_get_str(g_curl_fname_cstr);

        fd = c_file_open(fname, O_RDONLY, 0666);
        if(ERR_FD == fd)
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_ccurl_open_url_list_file: open url list file %s failed\n", fname);
            return (EC_FALSE);
        }

        if(EC_FALSE == c_file_size(fd, &fsize))
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_ccurl_open_url_list_file: get size of url list file %s failed\n", fname);
            c_file_close(fd);
            return (EC_FALSE);
        }

        ftext = (char *)mmap(NULL_PTR, fsize, PROT_READ, MAP_SHARED, fd, 0);
        if(MAP_FAILED == ftext)
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_ccurl_open_url_list_file: mmap url list file %s with fd %d failed, errno = %d, errorstr = %s\n", 
                               fname, fd, errno, strerror(errno));
            return (EC_FALSE);
        }        

        g_curl_list_fd    = fd;
        g_curl_list_buff  = ftext;
        g_curl_list_fsize = fsize;        
    }

    if(EC_FALSE == __test_ccurl_seek_url_list_file())
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:__test_ccurl_open_url_list_file: seek url list file to idx %ld failed\n", g_curl_idx);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL __test_ccurl_close_url_list_file()
{
    dbg_log(SEC_0137_DEMO, 9)(LOGSTDOUT, "[DEBUG] __test_ccurl_close_url_list_file: url idx %ld, url %s\n", g_curl_idx, g_curl_list_pcur);
    
    if(ERR_FD != g_curl_list_fd)
    {
        close(g_curl_list_fd);
        g_curl_list_fd = ERR_FD;
    }

    if(NULL_PTR != g_curl_list_buff)
    {
        munmap(g_curl_list_buff, g_curl_list_fsize);
        g_curl_list_buff = NULL_PTR;
        g_curl_list_fsize = 0;
    }

    return (EC_TRUE);
}

EC_BOOL __test_ccurl_fetch_url_cstr(CSTRING *url_cstr)
{
    cstring_append_chars(url_cstr, g_curl_list_lcur/*cut tail new line end*/, (UINT8 *)g_curl_list_pcur);
    cstring_append_char(url_cstr, '\0');

    __test_ccurl_next_url();
    g_curl_idx ++;

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[DEBUG] __test_ccurl_fetch_url_cstr: [url] %s\n", (char *)cstring_get_str(url_cstr));

    return (EC_TRUE);
}

/*url_cstr = http:/path_cstr*/
EC_BOOL __test_ccurl_fetch_path_cstr(const CSTRING *url_cstr, CSTRING *path_cstr)
{
    cstring_append_str(path_cstr, cstring_get_str(url_cstr) + strlen("http://") - 1);
    return (EC_TRUE);
}

EC_BOOL __test_ccurl_move_cstr_to_cbytes(CSTRING *cstr, CBYTES *cbytes)
{
    cbytes_mount(cbytes, cstring_get_len(cstr), cstring_get_str(cstr));
    cstring_init(cstr, NULL_PTR);
    return (EC_TRUE);
}

EC_BOOL __test_case_9x_fetch_path_cstr(const CVECTOR *url_cstr_vec, CVECTOR *path_cstr_vec)
{
    UINT32 idx;
    UINT32 num;

    num = cvector_size(url_cstr_vec);
    for(idx = 0; idx < num; idx ++)
    {
        CSTRING *url_cstr;
        CSTRING *path_cstr;

        url_cstr = (CSTRING *)cvector_get(url_cstr_vec, idx);
        if(NULL_PTR == url_cstr)
        {
            continue;
        }

        dbg_log(SEC_0137_DEMO, 9)(LOGSTDNULL, "[DEBUG] url '%s'\n", 
                           (char *)cstring_get_str(url_cstr));
        path_cstr = cstring_new(cstring_get_str(url_cstr) + strlen("http://") - 1, LOC_DEMO_0002);
        ASSERT(NULL_PTR != path_cstr);
        cvector_push(path_cstr_vec, path_cstr);

        dbg_log(SEC_0137_DEMO, 9)(LOGSTDNULL, "[DEBUG] url '%s' => path '%s'\n", 
                            (char *)cstring_get_str(url_cstr),
                            (char *)cstring_get_str(path_cstr));
    }

    return (EC_TRUE);
}

EC_BOOL __test_case_9x_move_cstr_to_cbytes(const CVECTOR *body_cstr_vec, CVECTOR *body_cbytes_vec)
{
    UINT32 idx;
    UINT32 num;

    num = cvector_size(body_cstr_vec);
    for(idx = 0; idx < num; idx ++)
    {
        CSTRING *body_cstr;
        CBYTES  *body_cbytes;

        body_cstr = (CSTRING *)cvector_get(body_cstr_vec, idx);
        if(NULL_PTR == body_cstr)
        {
            continue;
        }

        body_cbytes = cbytes_new(0);
        ASSERT(NULL_PTR != body_cbytes);

        cbytes_mount(body_cbytes, cstring_get_len(body_cstr), cstring_get_str(body_cstr));
        cstring_init(body_cstr, NULL_PTR);

        cvector_push(body_cbytes_vec, body_cbytes);
    }

    return (EC_TRUE);
}

EC_BOOL test_case_90_ccurl_get(MOD_MGR  *src_mod_mgr, 
                                      const UINT32 ccurl_tcid, 
                                      const UINT32 ccurl_rank, 
                                      const UINT32 ccurl_modi, 
                                      const UINT32 ccurl_mod_num, 
                                      const CSTRING *proxy_ip_port_cstr,
                                      CVECTOR  *url_cstr_vec,
                                      CVECTOR  *body_cstr_vec)
{
    MOD_MGR  *curl_mod_mgr;
    TASK_MGR *task_mgr;
    UINT32    des_mod_node_num;
    UINT32    des_mod_node_idx;

    CVECTOR  *curl_ret_vec;
 
    curl_ret_vec = cvector_new(0, MM_UINT32, LOC_DEMO_0003);
    ASSERT(NULL_PTR != curl_ret_vec);
    
    task_act(src_mod_mgr, &curl_mod_mgr, TASK_DEFAULT_LIVE, ccurl_mod_num, LOAD_BALANCING_QUE, TASK_PRIO_NORMAL, FI_ccurl_start);
    dbg_log(SEC_0137_DEMO, 9)(LOGSTDOUT, "[DEBUG] test_case_90_ccurl_get: curl_mod_mgr is:\n");
    mod_mgr_print(LOGSTDOUT, curl_mod_mgr);

    /*get file content from glusterfs*/
    task_mgr = task_new(curl_mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    des_mod_node_num = MOD_MGR_REMOTE_NUM(curl_mod_mgr);
    for(des_mod_node_idx = 0; des_mod_node_idx < des_mod_node_num; des_mod_node_idx ++)
    {
        UINT32  *ret;
        CSTRING *url_cstr;        
        CSTRING *body_cstr;

        alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT32, &ret, LOC_DEMO_0004);
        cvector_push(curl_ret_vec, (void *)ret);
        (*ret) = EC_FALSE;/*init*/

        body_cstr = cstring_new(NULL_PTR, LOC_DEMO_0005);
        cvector_push(body_cstr_vec, (void *)body_cstr);

        url_cstr = cstring_new(NULL_PTR, LOC_DEMO_0006);
        cvector_push(url_cstr_vec, (void *)url_cstr);
        
        ASSERT(EC_TRUE == __test_ccurl_fetch_url_cstr(url_cstr));/*TODO*/
        
        task_inc(task_mgr, ret, FI_ccurl_get, ERR_MODULE_ID, url_cstr, proxy_ip_port_cstr, body_cstr);
    }    
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NEED_RESCHEDULE_FLAG, task_default_bool_checker);
#if 1
    for(des_mod_node_idx = 0; des_mod_node_idx < des_mod_node_num; des_mod_node_idx ++)
    {
        MOD_NODE  *mod_node;
        CSTRING   *url_cstr;
        UINT32    *ret;
        CSTRING   *body_cstr;

        mod_node  = MOD_MGR_REMOTE_MOD(curl_mod_mgr, des_mod_node_idx);
        ret       = (UINT32  *)cvector_get(curl_ret_vec, des_mod_node_idx);
        body_cstr = (CSTRING *)cvector_get(body_cstr_vec, des_mod_node_idx);
        url_cstr  = (CSTRING *)cvector_get(url_cstr_vec, des_mod_node_idx);

        if(EC_TRUE == (*ret))
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[rank_%s_%ld][CURL SUCC] [%8ld] url %s, len %8ld\n", 
                               MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node),des_mod_node_idx,
                               (char *)cstring_get_str(url_cstr),
                               cstring_get_len(body_cstr));
        }
        else
        {
            dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "[rank_%s_%ld][CURL FAIL] [%8ld] url %s\n", 
                               MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node),des_mod_node_idx,
                               (char *)cstring_get_str(url_cstr));
        }
        
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT32, ret, LOC_DEMO_0007);
        cvector_set(curl_ret_vec, des_mod_node_idx, NULL_PTR);        
    }
#endif
    mod_mgr_free(curl_mod_mgr);
    curl_mod_mgr = NULL_PTR;

    cvector_free(curl_ret_vec, LOC_DEMO_0008);
    
    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "test_case_90_ccurl_get: end\n");

    return (EC_TRUE);    
}

EC_BOOL test_case_91_crfs_write(MOD_MGR  *src_mod_mgr, 
                                      const UINT32 ccurl_tcid, 
                                      const UINT32 ccurl_rank, 
                                      const UINT32 ccurl_modi, 
                                      const UINT32 ccurl_mod_num, 
                                      const CSTRING *proxy_ip_port_cstr,
                                      CVECTOR  *path_cstr_vec,
                                      CVECTOR  *body_cbytes_vec)
{
    MOD_NODE  crfs_mod_node;
    TASK_MGR *task_mgr;
    UINT32    path_num;
    UINT32    path_idx;

    CVECTOR  *crfs_ret_vec;

    crfs_ret_vec = cvector_new(0, MM_UINT32, LOC_DEMO_0009);
    ASSERT(NULL_PTR != crfs_ret_vec);    

    MOD_NODE_TCID(&crfs_mod_node) = c_ipv4_to_word(CRFS_TEST_TCID_STR);
    MOD_NODE_COMM(&crfs_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&crfs_mod_node) = CMPI_CRFS_RANK;
    MOD_NODE_MODI(&crfs_mod_node) = 0;

    /*write file content to rfs*/
    task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);    
    path_num = cvector_size(path_cstr_vec);
    for(path_idx = 0; path_idx < path_num; path_idx ++)
    {   
        EC_BOOL *ret;
        CSTRING *path_cstr;        
        CBYTES  *body_cbytes;

        alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT32, &ret, LOC_DEMO_0010);
        cvector_push(crfs_ret_vec, (void *)ret);
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

        task_p2p_inc(task_mgr, CMPI_ANY_MODI, &crfs_mod_node, 
                    ret, FI_crfs_write, ERR_MODULE_ID, path_cstr, body_cbytes, (UINT32)0);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    /*check success*/
    for(path_idx = 0; path_idx < path_num; path_idx ++)
    {
        UINT32    *ret;
        CSTRING   *path_cstr;        
        CBYTES    *body_cbytes;

        ret         = (EC_BOOL *)cvector_get(crfs_ret_vec, path_idx);
        body_cbytes = (CBYTES  *)cvector_get(body_cbytes_vec, path_idx);
        path_cstr   = (CSTRING *)cvector_get(path_cstr_vec, path_idx);

        if(EC_TRUE == (*ret))
        {
            sys_log(LOGUSER05, "[rank_%s_%ld][RFS SUCC] [%8ld] path %s, len %8ld\n", 
                               MOD_NODE_TCID_STR(&crfs_mod_node),MOD_NODE_RANK(&crfs_mod_node),path_idx,
                               (char *)cstring_get_str(path_cstr),
                               cbytes_len(body_cbytes));
        }
        else
        {
            sys_log(LOGUSER05, "[rank_%s_%ld][RFS FAIL] [%8ld] path %s\n", 
                               MOD_NODE_TCID_STR(&crfs_mod_node),MOD_NODE_RANK(&crfs_mod_node),path_idx,
                               (char *)cstring_get_str(path_cstr));
        }

        cvector_set(crfs_ret_vec, path_idx, NULL_PTR);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT32, ret, LOC_DEMO_0011);
    }

    cvector_free(crfs_ret_vec, LOC_DEMO_0012);
    
    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "test_case_91_crfs_write: end\n");

    return (EC_TRUE);    
}

EC_BOOL test_case_92_curl_to_crfs(MOD_MGR  *src_mod_mgr, 
                                      const UINT32 ccurl_tcid, 
                                      const UINT32 ccurl_rank, 
                                      const UINT32 ccurl_modi, 
                                      const UINT32 ccurl_mod_num, 
                                      const CSTRING *proxy_ip_port_cstr)
{
    CVECTOR * url_cstr_vec;
    CVECTOR * body_cstr_vec;

    CVECTOR  *path_cstr_vec;
    CVECTOR  *body_cbytes_vec;

    url_cstr_vec = cvector_new(0, MM_CSTRING, LOC_DEMO_0013);
    ASSERT(NULL_PTR != url_cstr_vec);
    
    body_cstr_vec = cvector_new(0, MM_CSTRING, LOC_DEMO_0014);
    ASSERT(NULL_PTR != body_cstr_vec);    
        
    test_case_90_ccurl_get(src_mod_mgr, 
                           ccurl_tcid, ccurl_rank, ccurl_modi, 
                           ccurl_mod_num, proxy_ip_port_cstr, 
                           url_cstr_vec, body_cstr_vec);


    path_cstr_vec = cvector_new(0, MM_CSTRING, LOC_DEMO_0015);
    ASSERT(NULL_PTR != path_cstr_vec);
    
    body_cbytes_vec = cvector_new(0, MM_CBYTES, LOC_DEMO_0016);
    ASSERT(NULL_PTR != body_cbytes_vec);    

    __test_case_9x_fetch_path_cstr(url_cstr_vec, path_cstr_vec);
    __test_case_9x_move_cstr_to_cbytes(body_cstr_vec, body_cbytes_vec);

    cvector_clean(url_cstr_vec, (CVECTOR_DATA_CLEANER)cstring_free, LOC_DEMO_0017);
    cvector_clean(body_cstr_vec, (CVECTOR_DATA_CLEANER)cstring_free, LOC_DEMO_0018);

    cvector_free(url_cstr_vec, LOC_DEMO_0019);
    cvector_free(body_cstr_vec, LOC_DEMO_0020);    

    url_cstr_vec  = NULL_PTR;
    body_cstr_vec = NULL_PTR;

    test_case_91_crfs_write(src_mod_mgr, 
                            ccurl_tcid, ccurl_rank, ccurl_modi, 
                            ccurl_mod_num, proxy_ip_port_cstr, 
                            path_cstr_vec, body_cbytes_vec);

    cvector_clean(path_cstr_vec, (CVECTOR_DATA_CLEANER)cstring_free, LOC_DEMO_0021);
    cvector_clean_with_location(body_cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_DEMO_0022);

    cvector_free(path_cstr_vec, LOC_DEMO_0023);
    cvector_free(body_cbytes_vec, LOC_DEMO_0024);

    path_cstr_vec = NULL_PTR;
    body_cbytes_vec = NULL_PTR;
   
    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "test_case_92_curl_to_crfs: end\n");

    return (EC_TRUE);    
}

EC_BOOL __test_ccurl_open_user_log()
{
    char fname[64];

    snprintf(fname, sizeof(fname), "user_%s_%ld.log", c_word_to_ipv4(CMPI_LOCAL_TCID), CMPI_LOCAL_RANK);
    user_log_open(LOGUSER05, fname, "w+");

    return (EC_TRUE);
}

EC_BOOL __test_ccurl_close_user_log()
{
    user_log_close(LOGUSER05);

    return (EC_TRUE);
}

EC_BOOL __test_ccurl_get()
{
    UINT32 ccurl_tcid;
    UINT32 ccurl_rank;
    UINT32 ccurl_modi;
    UINT32 ccurl_mod_num;
    UINT32 ccurl_mod_step;
    UINT32 ccurl_mod_idx;
    
    MOD_MGR  *src_mod_mgr;
    CSTRING   proxy_ip_port_cstr;

    __test_ccurl_open_user_log();

    ASSERT(EC_TRUE == __test_ccurl_open_url_list_file());
    
    cstring_init(&proxy_ip_port_cstr, (UINT8 *)"211.155.81.164:80");

    ccurl_tcid = CMPI_LOCAL_TCID;
    ccurl_rank = CCURL_TEST_RANK;   
    ccurl_modi = ccurl_start();

    src_mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_QUE);
    mod_mgr_incl(ccurl_tcid, CMPI_ANY_COMM, ccurl_rank, ccurl_modi, src_mod_mgr);    
    dbg_log(SEC_0137_DEMO, 9)(LOGSTDOUT, "[DEBUG] __test_ccurl_get: src_mod_mgr is:\n");
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);
    
    ccurl_mod_num  = g_curl_num;
    ccurl_mod_step = g_curl_step;

    for(ccurl_mod_idx = 0; ccurl_mod_idx < ccurl_mod_num; ccurl_mod_idx += ccurl_mod_step)
    {
        ccurl_mod_step = DMIN(g_curl_step, ccurl_mod_num - ccurl_mod_idx);
        test_case_92_curl_to_crfs(src_mod_mgr, ccurl_tcid, ccurl_rank, ccurl_modi, ccurl_mod_step, &proxy_ip_port_cstr);
        dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "__test_ccurl_get: [%8ld, %8ld) done\n", ccurl_mod_idx, ccurl_mod_idx + ccurl_mod_step);
    }

    mod_mgr_free(src_mod_mgr);
    
    ccurl_end(ccurl_modi);

    cstring_clean(&proxy_ip_port_cstr);

    __test_ccurl_close_url_list_file();
    cstring_free(g_curl_fname_cstr);

    __test_ccurl_close_user_log();

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "__test_ccurl_get: end\n");

    return (EC_TRUE);    
}

EC_BOOL __test_rfs_forward()
{
    CSTRING   proxy_ip_port_cstr;

    __test_ccurl_open_user_log();

    ASSERT(EC_TRUE == __test_ccurl_open_url_list_file());
    
    cstring_init(&proxy_ip_port_cstr, (UINT8 *)"211.155.81.164:80");

    crfs_forward(CMPI_ANY_MODI, g_curl_fname_cstr, g_rfs_tcid, &proxy_ip_port_cstr);

    cstring_clean(&proxy_ip_port_cstr);

    __test_ccurl_close_url_list_file();
    cstring_free(g_curl_fname_cstr);

    __test_ccurl_close_user_log();

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "__test_ccurl_get: end\n");

    return (EC_TRUE);    
}

EC_BOOL __test_rfs_cmp_finger()
{
    CSTRING   proxy_ip_port_cstr;

    __test_ccurl_open_user_log();

    ASSERT(EC_TRUE == __test_ccurl_open_url_list_file());
    
    cstring_init(&proxy_ip_port_cstr, (UINT8 *)"211.155.81.164:80");

    crfs_cmp_finger(CMPI_ANY_MODI, g_curl_fname_cstr, g_rfs_tcid, &proxy_ip_port_cstr);

    cstring_clean(&proxy_ip_port_cstr);

    __test_ccurl_close_url_list_file();
    cstring_free(g_curl_fname_cstr);

    __test_ccurl_close_user_log();

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "__test_ccurl_get: end\n");

    return (EC_TRUE);    
}

EC_BOOL __test_rfs_cleanup()
{
    __test_ccurl_open_user_log();

    ASSERT(EC_TRUE == __test_ccurl_open_url_list_file());
    
    crfs_cleanup(CMPI_ANY_MODI, g_curl_fname_cstr, g_rfs_tcid);

    __test_ccurl_close_url_list_file();
    cstring_free(g_curl_fname_cstr);

    __test_ccurl_close_user_log();

    dbg_log(SEC_0137_DEMO, 0)(LOGCONSOLE, "__test_ccurl_get: end\n");

    return (EC_TRUE);    
}

/*parse args*/
EC_BOOL __test_ccurl_parse_args(int argc, char **argv)
{
    int idx;

    for(idx = 0; idx < argc; idx ++)
    {
        //fprintf(stdout, "[DEBUG] __test_ccurl_parse_args: [%d] %s\n", idx, argv[ idx ]);
        
        if(0 == strcasecmp(argv[idx], "-ncurl") && idx + 1 < argc)
        {
            g_curl_num = atol(argv[idx + 1]);
            continue;
        } 

        if(0 == strcasecmp(argv[idx], "-nstep") && idx + 1 < argc)
        {
            g_curl_step = atol(argv[idx + 1]);
            continue;
        }   

        if(0 == strcasecmp(argv[idx], "-urlidx") && idx + 1 < argc)
        {
            g_curl_idx = atol(argv[idx + 1]);
            continue;
        }   
        
        if(0 == strcasecmp(argv[idx], "-urlfile") && idx + 1 < argc)
        {  
            g_curl_fname_cstr = cstring_new(argv[idx + 1], LOC_NONE_BASE);
            ASSERT(NULL_PTR != g_curl_fname_cstr);
            continue;
        }  

        if(0 == strcasecmp(argv[idx], "-rfs") && idx + 1 < argc)
        {  
            g_rfs_tcid = c_ipv4_to_word(argv[idx + 1]);
            continue;
        }         
    }
 
     return (EC_FALSE);
}

int main_crfs(int argc, char **argv)
{
    UINT32 crfs_rank;
    UINT32 tester_rank;
    
    task_brd_default_init(argc, argv);
    //c_sleep(3, LOC_DEMO_0025);
    //dbg_log(SEC_0137_DEMO, 9)(LOGSTDOUT, "[DEBUG] main_crfs: sleep to wait tcp enter established ... shit!\n");
    if(EC_FALSE == task_brd_default_check_validity())
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:main_crfs: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }
    
    __test_ccurl_parse_args(argc, argv);

    crfs_rank = CRFS_TEST_RANK;
    tester_rank = 0;

    /*define specific runner for each (tcid, rank)*/
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.1"), CMPI_ANY_RANK  , __test_crfs_runner);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.20.1"), CMPI_ANY_RANK  , __test_crfs_runner);
    
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.2"), tester_rank, __test_crfs_write_supplier_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.3"), tester_rank, __test_crfs_read_consumer_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.4"), tester_rank, __test_crfs_delete_consumer_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.5"), tester_rank, __test_crfs_check_content_md5sum_1);
    
    task_brd_default_add_runner(c_ipv4_to_word("10.10.20.2"), tester_rank, __test_crfs_write_supplier_2);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.20.3"), tester_rank, __test_crfs_read_consumer_2);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.20.5"), tester_rank, __test_crfs_delete_consumer_2);

    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.101"), tester_rank, __test_crfs_check_content_consumer_1);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.102"), tester_rank, __test_crfs_check_content_consumer_2);

    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.201"), tester_rank, __test_ccurl_get);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.202"), tester_rank, __test_ccurl_get);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.203"), tester_rank, __test_ccurl_get);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.204"), tester_rank, __test_ccurl_get);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.205"), tester_rank, __test_ccurl_get);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.206"), tester_rank, __test_ccurl_get);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.207"), tester_rank, __test_ccurl_get);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.208"), tester_rank, __test_ccurl_get);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.209"), tester_rank, __test_ccurl_get);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.210"), tester_rank, __test_ccurl_get);

#if 0
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.31"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.32"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.33"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.34"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.35"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.36"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.37"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.38"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.39"), tester_rank, __test_rfs_forward);

    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.41"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.42"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.43"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.44"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.45"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.46"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.47"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.48"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.49"), tester_rank, __test_rfs_forward);
    
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.51"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.52"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.53"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.54"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.55"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.56"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.57"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.58"), tester_rank, __test_rfs_forward);
    task_brd_default_add_runner(c_ipv4_to_word("10.10.10.59"), tester_rank, __test_rfs_forward);
#endif

    //task_brd_range_add_runner(c_ipv4_to_word("10.10.10.31"), c_ipv4_to_word("10.10.10.100"), tester_rank, __test_rfs_forward);
    //task_brd_range_add_runner(c_ipv4_to_word("10.10.10.31"), c_ipv4_to_word("10.10.10.100"), tester_rank, __test_rfs_cmp_finger);
    //task_brd_range_add_runner(c_ipv4_to_word("10.10.10.31"), c_ipv4_to_word("10.10.10.100"), tester_rank, __test_rfs_cleanup);
    
    /*start the defined runner on current (tcid, rank)*/
    task_brd_default_start_runner();   

    return (0);
}

int main_crfs_x(int argc, char **argv)
{
    main_crfs(argc, argv);
    return (0);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

