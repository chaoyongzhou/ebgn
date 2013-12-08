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
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cmpic.inc"
#include "croutine.h"
#include "cstring.h"
#include "cmisc.h"

#include "cbloom.h"

#include "chashalgo.h"
#include "cdsk.h"
#include "cstack.h"

#include "cpgrb.h"
#include "crfsnprb.h"
#include "crfsnp.h"


static CRFSNP_CFG g_crfsnp_cfg_tbl[] = {
    {"CRFSNP_004K_MODEL", CRFSNP_004K_CFG_FILE_SIZE,  CRFSNP_004K_CFG_ITEM_MAX_NUM },
    {"CRFSNP_064K_MODEL", CRFSNP_064K_CFG_FILE_SIZE,  CRFSNP_064K_CFG_ITEM_MAX_NUM },
    {"CRFSNP_001M_MODEL", CRFSNP_001M_CFG_FILE_SIZE,  CRFSNP_001M_CFG_ITEM_MAX_NUM },
    {"CRFSNP_002M_MODEL", CRFSNP_002M_CFG_FILE_SIZE,  CRFSNP_002M_CFG_ITEM_MAX_NUM },
    {"CRFSNP_128M_MODEL", CRFSNP_128M_CFG_FILE_SIZE,  CRFSNP_128M_CFG_ITEM_MAX_NUM },
    {"CRFSNP_256M_MODEL", CRFSNP_256M_CFG_FILE_SIZE,  CRFSNP_256M_CFG_ITEM_MAX_NUM },
    {"CRFSNP_512M_MODEL", CRFSNP_512M_CFG_FILE_SIZE,  CRFSNP_512M_CFG_ITEM_MAX_NUM },
    {"CRFSNP_001G_MODEL", CRFSNP_001G_CFG_FILE_SIZE,  CRFSNP_001G_CFG_ITEM_MAX_NUM },
    {"CRFSNP_002G_MODEL", CRFSNP_002G_CFG_FILE_SIZE,  CRFSNP_002G_CFG_ITEM_MAX_NUM },
#if (64 == WORDSIZE)
    {"CRFSNP_004G_MODEL", CRFSNP_004G_CFG_FILE_SIZE,  CRFSNP_004G_CFG_ITEM_MAX_NUM },
#endif/*(64 == WORDSIZE)*/
};

static uint8_t g_crfsnp_cfg_tbl_len = (uint8_t)(sizeof(g_crfsnp_cfg_tbl)/sizeof(g_crfsnp_cfg_tbl[0]));

EC_BOOL crfsnp_model_str(const uint8_t crfsnp_model, char **mod_str)
{
    CRFSNP_CFG *crfsnp_cfg;
    if(crfsnp_model >= g_crfsnp_cfg_tbl_len)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_model_str: invalid crfsnp mode %u\n", crfsnp_model);
        return (EC_FALSE);
    }
    crfsnp_cfg = &(g_crfsnp_cfg_tbl[ crfsnp_model ]);
    (*mod_str) = CRFSNP_CFG_MOD_STR(crfsnp_cfg);
    return (EC_TRUE);
}

uint32_t crfsnp_model_get(const char *mod_str)
{
    uint8_t crfsnp_model;

    for(crfsnp_model = 0; crfsnp_model < g_crfsnp_cfg_tbl_len; crfsnp_model ++)
    {
        CRFSNP_CFG *crfsnp_cfg;
        crfsnp_cfg = &(g_crfsnp_cfg_tbl[ crfsnp_model ]);

        if(0 == strcasecmp(CRFSNP_CFG_MOD_STR(crfsnp_cfg), mod_str))
        {
            return (crfsnp_model);
        }
    }
    return (CRFSNP_ERR_MODEL);
}

EC_BOOL crfsnp_model_file_size(const uint8_t crfsnp_model, uint32_t *file_size)
{
    CRFSNP_CFG *crfsnp_cfg;
    if(crfsnp_model >= g_crfsnp_cfg_tbl_len)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_model_file_size: invalid crfsnp mode %u\n", crfsnp_model);
        return (EC_FALSE);
    }
    crfsnp_cfg = &(g_crfsnp_cfg_tbl[ crfsnp_model ]);
    (*file_size) = CRFSNP_CFG_FILE_SIZE(crfsnp_cfg);
    return (EC_TRUE);
}

EC_BOOL crfsnp_model_item_max_num(const uint8_t crfsnp_model, uint32_t *item_max_num)
{
    CRFSNP_CFG *crfsnp_cfg;
    if(crfsnp_model >= g_crfsnp_cfg_tbl_len)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_model_item_max_num: invalid crfsnp mode %u\n", crfsnp_model);
        return (EC_FALSE);
    }
    crfsnp_cfg = &(g_crfsnp_cfg_tbl[ crfsnp_model ]);
    (*item_max_num) = CRFSNP_CFG_ITEM_MAX_NUM(crfsnp_cfg);
    return (EC_TRUE);
}

static char *crfsnp_fname_gen(const char *root_dir, const uint32_t np_id)
{
    char *fname;
    uint32_t len;

    if(NULL_PTR == root_dir)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_fname_gen: root_dir is null\n");
        return (NULL_PTR);
    }

    len = strlen(root_dir) + strlen("/rfsnp_0000.dat") + 1;

    fname = safe_malloc(len, LOC_CRFSNP_0001);
    if(NULL_PTR == fname)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_fname_gen: malloc %u bytes failed\n", len);
        return (NULL_PTR);
    }
    snprintf(fname, len, "%s/rfsnp_%04X.dat", root_dir, np_id);
    return (fname);
}

static uint32_t crfsnp_path_seg_len(const uint8_t *full_path, const uint32_t full_path_len, const uint8_t *path_seg_beg)
{
    uint8_t *ptr;

    if(path_seg_beg < full_path || path_seg_beg >= full_path + full_path_len)
    {
        return (0);
    }

    for(ptr = (uint8_t *)path_seg_beg; ptr < full_path + full_path_len && '/' != (*ptr); ptr ++)
    {
        /*do nothing*/
    }

    return (ptr - path_seg_beg);
}

EC_BOOL crfsnp_inode_init(CRFSNP_INODE *crfsnp_inode)
{
    CRFSNP_INODE_DISK_NO(crfsnp_inode)  = CPGRB_ERR_POS;
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode) = CPGRB_ERR_POS;
    CRFSNP_INODE_PAGE_NO(crfsnp_inode)  = CPGRB_ERR_POS;
    return (EC_TRUE);
}

EC_BOOL crfsnp_inode_clean(CRFSNP_INODE *crfsnp_inode)
{
    CRFSNP_INODE_DISK_NO(crfsnp_inode)  = CPGRB_ERR_POS;
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode) = CPGRB_ERR_POS;
    CRFSNP_INODE_PAGE_NO(crfsnp_inode)  = CPGRB_ERR_POS;
    return (EC_TRUE);
}

EC_BOOL crfsnp_inode_clone(const CRFSNP_INODE *crfsnp_inode_src, CRFSNP_INODE *crfsnp_inode_des)
{
    CRFSNP_INODE_DISK_NO(crfsnp_inode_des)  = CRFSNP_INODE_DISK_NO(crfsnp_inode_src);
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode_des) = CRFSNP_INODE_BLOCK_NO(crfsnp_inode_src);
    CRFSNP_INODE_PAGE_NO(crfsnp_inode_des)  = CRFSNP_INODE_PAGE_NO(crfsnp_inode_src);

    return (EC_TRUE);
}

void crfsnp_inode_print(LOG *log, const CRFSNP_INODE *crfsnp_inode)
{
    sys_print(log, "(disk %u, block %u, page %u)\n",
                    CRFSNP_INODE_DISK_NO(crfsnp_inode), 
                    CRFSNP_INODE_BLOCK_NO(crfsnp_inode),
                    CRFSNP_INODE_PAGE_NO(crfsnp_inode)
                    );
    return;
}

void crfsnp_inode_log_no_lock(LOG *log, const CRFSNP_INODE *crfsnp_inode)
{
    sys_print_no_lock(log, "(disk %u, block %u, page %u)\n",
                    CRFSNP_INODE_DISK_NO(crfsnp_inode), 
                    CRFSNP_INODE_BLOCK_NO(crfsnp_inode),
                    CRFSNP_INODE_PAGE_NO(crfsnp_inode)
                    );
    return;
}

CRFSNP_FNODE *crfsnp_fnode_new()
{
    CRFSNP_FNODE *crfsnp_fnode;
    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP_FNODE, &crfsnp_fnode, LOC_CRFSNP_0002);
    if(NULL_PTR != crfsnp_fnode)
    {
        crfsnp_fnode_init(crfsnp_fnode);
    }
    return (crfsnp_fnode);
}

CRFSNP_FNODE *crfsnp_fnode_make(const CRFSNP_FNODE *crfsnp_fnode_src)
{
    CRFSNP_FNODE *crfsnp_fnode_des;
    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP_FNODE, &crfsnp_fnode_des, LOC_CRFSNP_0003);
    if(NULL_PTR != crfsnp_fnode_des)
    {
        crfsnp_fnode_clone(crfsnp_fnode_src, crfsnp_fnode_des);
    }
    return (crfsnp_fnode_des);
}

EC_BOOL crfsnp_fnode_init(CRFSNP_FNODE *crfsnp_fnode)
{
    uint32_t pos;

    CRFSNP_FNODE_FILESZ(crfsnp_fnode) = 0;
    CRFSNP_FNODE_REPNUM(crfsnp_fnode) = 0;

    for(pos = 0; pos < CRFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        crfsnp_inode_init(CRFSNP_FNODE_INODE(crfsnp_fnode, pos));
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_fnode_clean(CRFSNP_FNODE *crfsnp_fnode)
{
    uint32_t pos;

    CRFSNP_FNODE_FILESZ(crfsnp_fnode) = 0;
    CRFSNP_FNODE_REPNUM(crfsnp_fnode) = 0;

    for(pos = 0; pos < CRFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        crfsnp_inode_clean(CRFSNP_FNODE_INODE(crfsnp_fnode, pos));
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_fnode_free(CRFSNP_FNODE *crfsnp_fnode)
{
    if(NULL_PTR != crfsnp_fnode)
    {
        crfsnp_fnode_clean(crfsnp_fnode);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP_FNODE, crfsnp_fnode, LOC_CRFSNP_0004);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_fnode_init_0(const UINT32 md_id, CRFSNP_FNODE *crfsnp_fnode)
{
    return crfsnp_fnode_init(crfsnp_fnode);
}

EC_BOOL crfsnp_fnode_clean_0(const UINT32 md_id, CRFSNP_FNODE *crfsnp_fnode)
{
    return crfsnp_fnode_clean(crfsnp_fnode);
}

EC_BOOL crfsnp_fnode_free_0(const UINT32 md_id, CRFSNP_FNODE *crfsnp_fnode)
{
    return crfsnp_fnode_free(crfsnp_fnode);
}

EC_BOOL crfsnp_fnode_clone(const CRFSNP_FNODE *crfsnp_fnode_src, CRFSNP_FNODE *crfsnp_fnode_des)
{
    uint32_t pos;

    CRFSNP_FNODE_FILESZ(crfsnp_fnode_des) = CRFSNP_FNODE_FILESZ(crfsnp_fnode_src);
    CRFSNP_FNODE_REPNUM(crfsnp_fnode_des) = CRFSNP_FNODE_REPNUM(crfsnp_fnode_src);

    for(pos = 0; pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode_src) && pos < CRFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        crfsnp_inode_clone(CRFSNP_FNODE_INODE(crfsnp_fnode_src, pos), CRFSNP_FNODE_INODE(crfsnp_fnode_des, pos));
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_fnode_check_inode_exist(const CRFSNP_INODE *inode, const CRFSNP_FNODE *crfsnp_fnode)
{
    uint32_t replica_pos;

    for(replica_pos = 0; replica_pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode); replica_pos ++)
    {
        if(
            CRFSNP_INODE_DISK_NO(inode)  == CRFSNP_FNODE_INODE_DISK_NO(crfsnp_fnode, replica_pos)
         && CRFSNP_INODE_BLOCK_NO(inode) == CRFSNP_FNODE_INODE_BLOCK_NO(crfsnp_fnode, replica_pos)
         && CRFSNP_INODE_PAGE_NO(inode)  == CRFSNP_FNODE_INODE_PAGE_NO(crfsnp_fnode, replica_pos)
        )
        {
            return (EC_TRUE);
        }
    }
    return (EC_FALSE);
}

EC_BOOL crfsnp_fnode_cmp(const CRFSNP_FNODE *crfsnp_fnode_1st, const CRFSNP_FNODE *crfsnp_fnode_2nd)
{
    uint32_t replica_pos;

    if(NULL_PTR == crfsnp_fnode_1st && NULL_PTR == crfsnp_fnode_2nd)
    {
        return (EC_TRUE);
    }

    if(NULL_PTR == crfsnp_fnode_1st || NULL_PTR == crfsnp_fnode_2nd)
    {
        return (EC_FALSE);
    }

    if(CRFSNP_FNODE_REPNUM(crfsnp_fnode_1st) != CRFSNP_FNODE_REPNUM(crfsnp_fnode_2nd))
    {
        return (EC_FALSE);
    }

    if(CRFSNP_FNODE_FILESZ(crfsnp_fnode_1st) != CRFSNP_FNODE_FILESZ(crfsnp_fnode_2nd))
    {
        return (EC_FALSE);
    }
    
    for(replica_pos = 0; replica_pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode_1st); replica_pos ++)
    {
        if(EC_FALSE == crfsnp_fnode_check_inode_exist(CRFSNP_FNODE_INODE(crfsnp_fnode_1st, replica_pos), crfsnp_fnode_2nd))
        {
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

EC_BOOL crfsnp_fnode_import(const CRFSNP_FNODE *crfsnp_fnode_src, CRFSNP_FNODE *crfsnp_fnode_des)
{
    uint32_t src_pos;
    uint32_t des_pos;

    for(src_pos = 0, des_pos = 0; src_pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode_src) && src_pos < CRFSNP_FILE_REPLICA_MAX_NUM; src_pos ++)
    {
        CRFSNP_INODE *crfsnp_inode_src;

        crfsnp_inode_src = (CRFSNP_INODE *)CRFSNP_FNODE_INODE(crfsnp_fnode_src, src_pos);
        if(CPGRB_ERR_POS != CRFSNP_INODE_DISK_NO(crfsnp_inode_src) 
        && CPGRB_ERR_POS != CRFSNP_INODE_BLOCK_NO(crfsnp_inode_src)
        && CPGRB_ERR_POS != CRFSNP_INODE_PAGE_NO(crfsnp_inode_src)
        )
        {
            CRFSNP_INODE *crfsnp_inode_des;

            crfsnp_inode_des = CRFSNP_FNODE_INODE(crfsnp_fnode_des, des_pos);
            if(crfsnp_inode_src != crfsnp_inode_des)
            {
                crfsnp_inode_clone(crfsnp_inode_src, crfsnp_inode_des);
            }

            des_pos ++;
        }
    }

    CRFSNP_FNODE_FILESZ(crfsnp_fnode_des) = CRFSNP_FNODE_FILESZ(crfsnp_fnode_src);
    CRFSNP_FNODE_REPNUM(crfsnp_fnode_des) = des_pos;
    return (EC_TRUE);
}

uint32_t crfsnp_fnode_count_replica(const CRFSNP_FNODE *crfsnp_fnode)
{
    uint32_t pos;
    uint32_t count;

    for(pos = 0, count = 0; pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode) && pos < CRFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        CRFSNP_INODE *crfsnp_inode;

        crfsnp_inode = (CRFSNP_INODE *)CRFSNP_FNODE_INODE(crfsnp_fnode, pos);
        if(CPGRB_ERR_POS != CRFSNP_INODE_DISK_NO(crfsnp_inode) 
        && CPGRB_ERR_POS != CRFSNP_INODE_BLOCK_NO(crfsnp_inode)
        && CPGRB_ERR_POS != CRFSNP_INODE_PAGE_NO(crfsnp_inode)
        )
        {
            count ++;
        }
    }
    return (count);
}

void crfsnp_fnode_print(LOG *log, const CRFSNP_FNODE *crfsnp_fnode)
{
    uint32_t pos;

    sys_log(log, "crfsnp_fnode %p: file size %u, replica num %u\n",
                    crfsnp_fnode,
                    CRFSNP_FNODE_FILESZ(crfsnp_fnode), 
                    CRFSNP_FNODE_REPNUM(crfsnp_fnode)
                    );

    for(pos = 0; pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode) && pos < CRFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        crfsnp_inode_print(log, CRFSNP_FNODE_INODE(crfsnp_fnode, pos));
    }
    return;
}

void crfsnp_fnode_log_no_lock(LOG *log, const CRFSNP_FNODE *crfsnp_fnode)
{
    uint32_t pos;

    sys_print_no_lock(log, "size %u, replica %u",
                    CRFSNP_FNODE_FILESZ(crfsnp_fnode),
                    CRFSNP_FNODE_REPNUM(crfsnp_fnode));

    for(pos = 0; pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode) && pos < CRFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        crfsnp_inode_log_no_lock(log, CRFSNP_FNODE_INODE(crfsnp_fnode, pos));
    }
    sys_print_no_lock(log, "\n");

    return;
}

CRFSNP_DNODE *crfsnp_dnode_new()
{
    CRFSNP_DNODE *crfsnp_dnode;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP_DNODE, &crfsnp_dnode, LOC_CRFSNP_0005);
    if(NULL_PTR != crfsnp_dnode)
    {
        crfsnp_dnode_init(crfsnp_dnode);
    }
    return (crfsnp_dnode);

}

EC_BOOL crfsnp_dnode_init(CRFSNP_DNODE *crfsnp_dnode)
{
    uint32_t pos;

    CRFSNP_DNODE_FILE_NUM(crfsnp_dnode) = 0;

    for(pos = 0; pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; pos ++)
    {
        CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, pos) = (uint32_t)(CRFSNPRB_ERR_POS);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_dnode_clean(CRFSNP_DNODE *crfsnp_dnode)
{
    uint32_t pos;

    CRFSNP_DNODE_FILE_NUM(crfsnp_dnode) = 0;

    for(pos = 0; pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; pos ++)
    {
        CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, pos) = (uint32_t)(CRFSNPRB_ERR_POS);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_dnode_free(CRFSNP_DNODE *crfsnp_dnode)
{
    if(NULL_PTR != crfsnp_dnode)
    {
        crfsnp_dnode_clean(crfsnp_dnode);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP_DNODE, crfsnp_dnode, LOC_CRFSNP_0006);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_dnode_clone(const CRFSNP_DNODE *crfsnp_dnode_src, CRFSNP_DNODE *crfsnp_dnode_des)
{
    uint32_t pos;

    CRFSNP_DNODE_FILE_NUM(crfsnp_dnode_des) = CRFSNP_DNODE_FILE_NUM(crfsnp_dnode_src);
    for(pos = 0; pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; pos ++)
    {
        CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode_des, pos) = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode_src, pos);
    }
    return (EC_TRUE);
}

CRFSNP_BNODE *crfsnp_bnode_new()
{
    CRFSNP_BNODE *crfsnp_bnode;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP_BNODE, &crfsnp_bnode, LOC_CRFSNP_0007);
    if(NULL_PTR != crfsnp_bnode)
    {
        crfsnp_bnode_init(crfsnp_bnode);
    }
    return (crfsnp_bnode);

}

EC_BOOL crfsnp_bnode_init(CRFSNP_BNODE *crfsnp_bnode)
{
    uint32_t pos;

    CRFSNP_BNODE_FILESZ(crfsnp_bnode)     = 0;
    CRFSNP_BNODE_SEG_NUM(crfsnp_bnode)    = 0;

    for(pos = 0; pos < CRFSNP_BNODE_SEG_BUCKET_MAX_NUM; pos ++)
    {
        CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, pos) = (uint32_t)(CRFSNPRB_ERR_POS);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_bnode_clean(CRFSNP_BNODE *crfsnp_bnode)
{
    uint32_t pos;

    CRFSNP_BNODE_FILESZ(crfsnp_bnode)     = 0;
    CRFSNP_BNODE_SEG_NUM(crfsnp_bnode)    = 0;

    for(pos = 0; pos < CRFSNP_BNODE_SEG_BUCKET_MAX_NUM; pos ++)
    {
        CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, pos) = (uint32_t)(CRFSNPRB_ERR_POS);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_bnode_free(CRFSNP_BNODE *crfsnp_bnode)
{
    if(NULL_PTR != crfsnp_bnode)
    {
        crfsnp_bnode_clean(crfsnp_bnode);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP_BNODE, crfsnp_bnode, LOC_CRFSNP_0008);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_bnode_clone(const CRFSNP_BNODE *crfsnp_bnode_src, CRFSNP_BNODE *crfsnp_bnode_des)
{
    uint32_t pos;

    CRFSNP_BNODE_FILESZ(crfsnp_bnode_des)     = CRFSNP_BNODE_FILESZ(crfsnp_bnode_src);
    CRFSNP_BNODE_SEG_NUM(crfsnp_bnode_des)    = CRFSNP_BNODE_SEG_NUM(crfsnp_bnode_src);

    for(pos = 0; pos < CRFSNP_BNODE_SEG_BUCKET_MAX_NUM; pos ++)
    {
        CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode_des, pos) = CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode_src, pos);
    }
    return (EC_TRUE);
}

void crfsnp_bnode_print(LOG *log, const CRFSNP_BNODE *crfsnp_bnode)
{
    sys_log(log, "crfsnp_bnode %p: file size %llu, seg num %u\n",
                    crfsnp_bnode,
                    CRFSNP_BNODE_FILESZ(crfsnp_bnode), 
                    CRFSNP_BNODE_SEG_NUM(crfsnp_bnode)
                    );
    return;
}

CRFSNP_ITEM *crfsnp_item_new()
{
    CRFSNP_ITEM *crfsnp_item;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP_ITEM, &crfsnp_item, LOC_CRFSNP_0009);
    if(NULL_PTR != crfsnp_item)
    {
        crfsnp_item_init(crfsnp_item);
    }
    return (crfsnp_item);
}

EC_BOOL crfsnp_item_init(CRFSNP_ITEM *crfsnp_item)
{
    CRFSNP_ITEM_DFLG(crfsnp_item)             = CRFSNP_ITEM_FILE_IS_ERR;
    CRFSNP_ITEM_STAT(crfsnp_item)             = CRFSNP_ITEM_STAT_IS_NOT_USED;
    CRFSNP_ITEM_KLEN(crfsnp_item)             = 0;

    BSET(CRFSNP_ITEM_KEY(crfsnp_item), CRFSNP_KEY_MAX_SIZE, '\0');

    crfsnp_dnode_init(CRFSNP_ITEM_DNODE(crfsnp_item));
    
    /*note:do nothing on rb_node*/

    return (EC_TRUE);
}

EC_BOOL crfsnp_item_clean(CRFSNP_ITEM *crfsnp_item)
{
    CRFSNP_ITEM_DFLG(crfsnp_item)             = CRFSNP_ITEM_FILE_IS_ERR;
    CRFSNP_ITEM_STAT(crfsnp_item)             = CRFSNP_ITEM_STAT_IS_NOT_USED;
    CRFSNP_ITEM_KLEN(crfsnp_item)             = 0;

    BSET(CRFSNP_ITEM_KEY(crfsnp_item), CRFSNP_KEY_MAX_SIZE, '\0');

    crfsnp_dnode_clean(CRFSNP_ITEM_DNODE(crfsnp_item));

    /*note:do nothing on rb_node*/

    return (EC_TRUE);
}

EC_BOOL crfsnp_item_clone(const CRFSNP_ITEM *crfsnp_item_src, CRFSNP_ITEM *crfsnp_item_des)
{
    if(NULL_PTR == crfsnp_item_src)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_item_clone: crfsnp_item_src is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == crfsnp_item_des)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_item_clone: crfsnp_item_des is null\n");
        return (EC_FALSE);
    }

    CRFSNP_ITEM_DFLG(crfsnp_item_des)        =  CRFSNP_ITEM_DFLG(crfsnp_item_src);
    CRFSNP_ITEM_STAT(crfsnp_item_des)        =  CRFSNP_ITEM_STAT(crfsnp_item_src);
    CRFSNP_ITEM_KLEN(crfsnp_item_des)        =  CRFSNP_ITEM_KLEN(crfsnp_item_src);

    BCOPY(CRFSNP_ITEM_KEY(crfsnp_item_src), CRFSNP_ITEM_KEY(crfsnp_item_des), CRFSNP_ITEM_KLEN(crfsnp_item_src));

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item_src))
    {
        crfsnp_fnode_clone(CRFSNP_ITEM_FNODE(crfsnp_item_src), CRFSNP_ITEM_FNODE(crfsnp_item_des));
    }
    else if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item_src))
    {
        crfsnp_dnode_clone(CRFSNP_ITEM_DNODE(crfsnp_item_src), CRFSNP_ITEM_DNODE(crfsnp_item_des));
    }
    else if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item_src))
    {
        crfsnp_bnode_clone(CRFSNP_ITEM_BNODE(crfsnp_item_src), CRFSNP_ITEM_BNODE(crfsnp_item_des));
    }    

    return (EC_TRUE);
}

EC_BOOL crfsnp_item_free(CRFSNP_ITEM *crfsnp_item)
{
    if(NULL_PTR != crfsnp_item)
    {
        crfsnp_item_clean(crfsnp_item);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP_ITEM, crfsnp_item, LOC_CRFSNP_0010);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_item_init_0(const UINT32 md_id, CRFSNP_ITEM *crfsnp_item)
{
    return crfsnp_item_init(crfsnp_item);
}

EC_BOOL crfsnp_item_clean_0(const UINT32 md_id, CRFSNP_ITEM *crfsnp_item)
{
    return crfsnp_item_clean(crfsnp_item);
}

EC_BOOL crfsnp_item_free_0(const UINT32 md_id, CRFSNP_ITEM *crfsnp_item)
{
    return crfsnp_item_free(crfsnp_item);
}

EC_BOOL crfsnp_item_set_key(CRFSNP_ITEM *crfsnp_item, const uint32_t klen, const uint8_t *key)
{
    BCOPY(key, CRFSNP_ITEM_KEY(crfsnp_item), klen);
    CRFSNP_ITEM_KLEN(crfsnp_item) = klen;

    return (EC_TRUE);
}

static const char *__crfsnp_item_dir_flag_str(const uint32_t dir_flag)
{
    switch(dir_flag)
    {
        case CRFSNP_ITEM_FILE_IS_DIR:
            return (const char *)"D";
        case CRFSNP_ITEM_FILE_IS_REG:
            return (const char *)"F";
        case CRFSNP_ITEM_FILE_IS_PIP:
            return (const char *)"P";
        case CRFSNP_ITEM_FILE_IS_LNK:
            return (const char *)"L";
        case CRFSNP_ITEM_FILE_IS_SCK:
            return (const char *)"S";
        case CRFSNP_ITEM_FILE_IS_CHR:
            return (const char *)"C";
        case CRFSNP_ITEM_FILE_IS_BLK:
        case CRFSNP_ITEM_FILE_IS_BIG:
            return (const char *)"B";
    }

    return (const char *)"UFO";
}

void crfsnp_item_print(LOG *log, const CRFSNP_ITEM *crfsnp_item)
{
    uint32_t pos;

    sys_print(log, "crfsnp_item %p: flag [%s], stat %u, klen %u\n",
                    crfsnp_item,
                    __crfsnp_item_dir_flag_str(CRFSNP_ITEM_DFLG(crfsnp_item)),
                    CRFSNP_ITEM_STAT(crfsnp_item),
                    CRFSNP_ITEM_KLEN(crfsnp_item)
                    );

    sys_log(log, "key: %.*s\n", CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_FNODE *crfsnp_fnode;

        crfsnp_fnode = (CRFSNP_FNODE *)CRFSNP_ITEM_FNODE(crfsnp_item);
        sys_log(log, "file size %u, replica num %u\n",
                        CRFSNP_FNODE_FILESZ(crfsnp_fnode),
                        CRFSNP_FNODE_REPNUM(crfsnp_fnode)
                        );
        for(pos = 0; pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode); pos ++)
        {
            CRFSNP_INODE *crfsnp_inode;

            crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, pos);
            crfsnp_inode_print(log, crfsnp_inode);
            //sys_print(log, "\n");
        }
    }

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_DNODE *crfsnp_dnode;

        crfsnp_dnode = (CRFSNP_DNODE *)CRFSNP_ITEM_DNODE(crfsnp_item);
        sys_log(log, "file num: %u\n", CRFSNP_DNODE_FILE_NUM(crfsnp_dnode));

#if 0
        sys_log(log, "dir bucket: ");
        for(pos = 0; pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; pos ++)
        {
            sys_print(log, "%u,", CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, pos));
        }
        sys_print(log, "\n");
#endif
    }    

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_BNODE *crfsnp_bnode;

        crfsnp_bnode = (CRFSNP_BNODE *)CRFSNP_ITEM_BNODE(crfsnp_item);
        sys_log(log, "file size: %llu, seg num: %u\n", CRFSNP_BNODE_FILESZ(crfsnp_bnode), CRFSNP_BNODE_SEG_NUM(crfsnp_bnode));

#if 0
        sys_log(log, "seg bucket: ");
        for(pos = 0; pos < CRFSNP_BNODE_SEG_BUCKET_MAX_NUM; pos ++)
        {
            sys_print(log, "%u,", CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, pos));
        }
        sys_print(log, "\n");
#endif
    }

    return;
}

EC_BOOL crfsnp_item_load(CRFSNP *crfsnp, uint32_t *offset, CRFSNP_ITEM *crfsnp_item)
{
    RWSIZE rsize;
    UINT32 offset_t;

    offset_t = (*offset);
    rsize = sizeof(CRFSNP_ITEM);
    if(EC_FALSE == c_file_load(CRFSNP_FD(crfsnp), &offset_t, rsize, (UINT8 *)crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_item_load: load item from offset %u failed\n", *offset);
        return (EC_FALSE);
    }

    (*offset) = (uint32_t)offset_t;

    return (EC_TRUE);
}

EC_BOOL crfsnp_item_flush(CRFSNP *crfsnp, uint32_t *offset, const CRFSNP_ITEM *crfsnp_item)
{
    RWSIZE wsize;
    UINT32 offset_t;

    offset_t = (*offset);
    wsize = sizeof(CRFSNP_ITEM);
    if(EC_FALSE == c_file_flush(CRFSNP_FD(crfsnp), &offset_t, wsize, (UINT8 *)crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_item_load: flush item to offset %u failed\n", *offset);
        return (EC_FALSE);
    }

    (*offset) = (uint32_t)offset_t;

    return (EC_TRUE);
}

EC_BOOL crfsnp_item_is(const CRFSNP_ITEM *crfsnp_item, const uint32_t klen, const uint8_t *key)
{
    if(klen !=  CRFSNP_ITEM_KLEN(crfsnp_item))
    {
        return (EC_FALSE);
    }

    if(0 != strncmp((char *)key, (char *)CRFSNP_ITEM_KEY(crfsnp_item), klen))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

CRFSNP_ITEM *crfsnp_item_parent(const CRFSNP *crfsnp, const CRFSNP_ITEM *crfsnp_item)
{
    uint32_t parent_pos;

    parent_pos = CRFSNPRB_NODE_PARENT_POS(CRFSNP_ITEM_RB_NODE(crfsnp_item));
    if(CRFSNPRB_ERR_POS == parent_pos)
    {
        return (NULL_PTR);
    }

    return crfsnp_fetch(crfsnp, parent_pos); 
}

CRFSNP_ITEM *crfsnp_item_left(const CRFSNP *crfsnp, const CRFSNP_ITEM *crfsnp_item)
{
    uint32_t left_pos;

    left_pos = CRFSNPRB_NODE_LEFT_POS(CRFSNP_ITEM_RB_NODE(crfsnp_item));
    if(CRFSNPRB_ERR_POS == left_pos)
    {
        return (NULL_PTR);
    }

    return crfsnp_fetch(crfsnp, left_pos); 
}

CRFSNP_ITEM *crfsnp_item_right(const CRFSNP *crfsnp, const CRFSNP_ITEM *crfsnp_item)
{
    uint32_t right_offset;

    right_offset = CRFSNPRB_NODE_RIGHT_POS(CRFSNP_ITEM_RB_NODE(crfsnp_item));
    if(CRFSNPRB_ERR_POS == right_offset)
    {
        return (NULL_PTR);
    }

    return crfsnp_fetch(crfsnp, right_offset); 
}

void crfsnp_bucket_print(LOG *log, const uint32_t *crfsnp_buckets)
{
    uint32_t pos;

    for(pos = 0; pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; pos ++)
    {
        sys_log(log, "bucket %u#: offset %u\n", pos, *(crfsnp_buckets + pos));
    }
    return;
}

static CRFSNP_HEADER *__crfsnp_header_open(const uint32_t np_id, const uint32_t fsize, int fd)
{
    CRFSNP_HEADER *crfsnp_header;

    crfsnp_header = (CRFSNP_HEADER *)mmap(NULL_PTR, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(MAP_FAILED == crfsnp_header)
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_header_open: mmap np %u with fd %d failed, errno = %d, errorstr = %s\n", 
                           np_id, fd, errno, strerror(errno));
        return (NULL_PTR);
    } 
    
    return (crfsnp_header);
}

static CRFSNP_HEADER *__crfsnp_header_create(const uint32_t np_id, const uint32_t fsize, int fd, const uint8_t np_model)
{
    CRFSNP_HEADER *crfsnp_header;
    uint32_t node_max_num;
    uint32_t node_sizeof;

    crfsnp_header = (CRFSNP_HEADER *)mmap(NULL_PTR, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(MAP_FAILED == crfsnp_header)
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_header_open: mmap np %u with fd %d failed, errno = %d, errorstr = %s\n", 
                           np_id, fd, errno, strerror(errno));
        return (NULL_PTR);
    }     

    CRFSNP_HEADER_NP_ID(crfsnp_header)  = np_id;
    CRFSNP_HEADER_MODEL(crfsnp_header)  = np_model;

    crfsnp_model_item_max_num(np_model, &node_max_num);
    node_sizeof = sizeof(CRFSNP_ITEM);
    crfsnprb_pool_init(CRFSNP_HEADER_ITEMS_POOL(crfsnp_header), node_max_num, node_sizeof);
    
    return (crfsnp_header);
}

static CRFSNP_HEADER * __crfsnp_header_sync(CRFSNP_HEADER *crfsnp_header, const uint32_t np_id, const uint32_t fsize)
{   
    if(NULL_PTR != crfsnp_header)
    {   
        if(0 != msync(crfsnp_header, fsize, MS_SYNC))
        {
            sys_log(LOGSTDOUT, "warn:__crfsnp_header_sync: sync crfsnp_hdr of np %u with size %u failed\n", 
                               np_id, fsize);
        }
    }    
    return (crfsnp_header);
}

static CRFSNP_HEADER *__crfsnp_header_close(CRFSNP_HEADER *crfsnp_header, const uint32_t np_id, const uint32_t fsize)
{   
    if(NULL_PTR != crfsnp_header)
    {   
        if(0 != msync(crfsnp_header, fsize, MS_SYNC))
        {
            sys_log(LOGSTDOUT, "warn:__crfsnp_header_close: sync crfsnp_hdr of np %u with size %u failed\n", 
                               np_id, fsize);
        }
        
        if(0 != munmap(crfsnp_header, fsize))
        {
            sys_log(LOGSTDOUT, "warn:__crfsnp_header_close: munmap crfsnp of np %u with size %u failed\n", 
                               np_id, fsize);
        } 
    }
    
    /*crfsnp_header cannot be accessed again*/
    return (NULL_PTR);
}


EC_BOOL crfsnp_header_init(CRFSNP_HEADER *crfsnp_header, const uint32_t np_id, const uint8_t model, const uint8_t first_chash_algo_id, const uint8_t second_chash_algo_id)
{
    CRFSNP_HEADER_NP_ID(crfsnp_header)         = np_id;
    CRFSNP_HEADER_MODEL(crfsnp_header)         = model;
    
    CRFSNP_HEADER_1ST_CHASH_ALGO_ID(crfsnp_header)  = first_chash_algo_id;
    CRFSNP_HEADER_2ND_CHASH_ALGO_ID(crfsnp_header)  = second_chash_algo_id;

    /*do nothing on CRFSNPRB_POOL pool*/

    return (EC_TRUE);
}

EC_BOOL crfsnp_header_clean(CRFSNP_HEADER *crfsnp_header)
{
    CRFSNP_HEADER_NP_ID(crfsnp_header)              = CRFSNP_ERR_ID;
    CRFSNP_HEADER_MODEL(crfsnp_header)              = CRFSNP_ERR_MODEL;
    
    CRFSNP_HEADER_1ST_CHASH_ALGO_ID(crfsnp_header)  = CHASH_ERR_ALGO_ID;
    CRFSNP_HEADER_2ND_CHASH_ALGO_ID(crfsnp_header)  = CHASH_ERR_ALGO_ID;

    /*do nothing on CRFSNPRB_POOL pool*/

    return (EC_TRUE);
}

EC_BOOL crfsnp_header_create(CRFSNP_HEADER *crfsnp_header, const uint8_t crfsnp_model, const uint32_t np_id, const uint8_t model, const uint8_t first_chash_algo_id, const uint8_t second_chash_algo_id)
{
    return crfsnp_header_init(crfsnp_header,
                               np_id,
                               model,
                               first_chash_algo_id,
                               second_chash_algo_id);
}

CRFSNP *crfsnp_new()
{
    CRFSNP *crfsnp;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP, &crfsnp, LOC_CRFSNP_0011);
    if(NULL_PTR != crfsnp)
    {
        crfsnp_init(crfsnp);
    }
    return (crfsnp);
}

EC_BOOL crfsnp_init(CRFSNP *crfsnp)
{    
    CRFSNP_FD(crfsnp)         = ERR_FD;
    CRFSNP_FSIZE(crfsnp)      = 0;
    CRFSNP_FNAME(crfsnp)      = NULL_PTR;
    CRFSNP_STATE(crfsnp)      = CRFSNP_STATE_NOT_DIRTY;
    CRFSNP_READER_NUM(crfsnp) = 0;
    CRFSNP_HDR(crfsnp)        = NULL_PTR;

    CRFSNP_INIT_LOCK(crfsnp, LOC_CRFSNP_0012);

    CRFSNP_1ST_CHASH_ALGO(crfsnp) = NULL_PTR;
    CRFSNP_2ND_CHASH_ALGO(crfsnp) = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL crfsnp_clean(CRFSNP *crfsnp)
{
    if(NULL_PTR != CRFSNP_HDR(crfsnp))
    {
        __crfsnp_header_close(CRFSNP_HDR(crfsnp), CRFSNP_ID(crfsnp), CRFSNP_FSIZE(crfsnp));
        CRFSNP_HDR(crfsnp) = NULL_PTR;
    }
    
    if(ERR_FD != CRFSNP_FD(crfsnp))
    {
        c_file_close(CRFSNP_FD(crfsnp));
        CRFSNP_FD(crfsnp) = ERR_FD;
    }

    CRFSNP_FSIZE(crfsnp) = 0;

    if(NULL_PTR != CRFSNP_FNAME(crfsnp))
    {
        safe_free(CRFSNP_FNAME(crfsnp), LOC_CRFSNP_0013);
        CRFSNP_FNAME(crfsnp) = NULL_PTR;
    }

    CRFSNP_STATE(crfsnp)      = CRFSNP_STATE_NOT_DIRTY;
    CRFSNP_READER_NUM(crfsnp) = 0;

    CRFSNP_HDR(crfsnp) = NULL_PTR;

    CRFSNP_CLEAN_LOCK(crfsnp, LOC_CRFSNP_0014);

    CRFSNP_1ST_CHASH_ALGO(crfsnp) = NULL_PTR;
    CRFSNP_2ND_CHASH_ALGO(crfsnp) = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL crfsnp_free(CRFSNP *crfsnp)
{
    if(NULL_PTR != crfsnp)
    {
        crfsnp_clean(crfsnp);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSNP, crfsnp, LOC_CRFSNP_0015);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_is_full(const CRFSNP *crfsnp)
{
    CRFSNPRB_POOL *pool;

    pool = CRFSNP_ITEMS_POOL(crfsnp);
    return crfsnprb_pool_is_full(pool);
}

void crfsnp_header_print(LOG *log, const CRFSNP *crfsnp)
{
    const CRFSNP_HEADER *crfsnp_header;

    crfsnp_header = CRFSNP_HDR(crfsnp);

    sys_log(log, "np %u, model %u, item max num %u, item used num %u, 1st hash algo %u, 2nd hash algo %u\n",
                CRFSNP_HEADER_NP_ID(crfsnp_header),
                CRFSNP_HEADER_MODEL(crfsnp_header),
                CRFSNP_HEADER_ITEMS_MAX_NUM(crfsnp_header) ,
                CRFSNP_HEADER_ITEMS_USED_NUM(crfsnp_header) ,
                CRFSNP_HEADER_1ST_CHASH_ALGO_ID(crfsnp_header),
                CRFSNP_HEADER_2ND_CHASH_ALGO_ID(crfsnp_header)
        );

    crfsnprb_pool_print(log, CRFSNP_HEADER_ITEMS_POOL(crfsnp_header));
    return;
}

void crfsnp_print(LOG *log, const CRFSNP *crfsnp)
{
    sys_log(log, "crfsnp %p: np %u, fname %s, fsize %u, readers %u\n", 
                 crfsnp, 
                 CRFSNP_ID(crfsnp), 
                 CRFSNP_FNAME(crfsnp),
                 CRFSNP_FSIZE(crfsnp),
                 CRFSNP_READER_NUM(crfsnp)
                 );

    sys_log(log, "crfsnp %p: header: \n", crfsnp);
    crfsnp_header_print(log, crfsnp);
    return;
}

CRFSNP_ITEM *crfsnp_dnode_find(const CRFSNP *crfsnp, const CRFSNP_DNODE *crfsnp_dnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key)
{
    const CRFSNPRB_POOL *pool;
    uint32_t root_pos;
    uint32_t node_pos;

    pool     = CRFSNP_ITEMS_POOL(crfsnp);
    root_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, CRFSNP_DNODE_DIR_BUCKET_POS(first_hash));

    node_pos = crfsnprb_tree_search_data(pool, root_pos, second_hash, klen, key);
    if(CRFSNPRB_ERR_POS != node_pos)
    {
        const CRFSNPRB_NODE *node;
        const CRFSNP_ITEM   *item;
        
        node = CRFSNPRB_POOL_NODE(pool, node_pos);
        item = CRFSNP_RB_NODE_ITEM(node);

        return (CRFSNP_ITEM *)(item);
    }

    return (NULL_PTR);
}

uint32_t crfsnp_dnode_search(const CRFSNP *crfsnp, const CRFSNP_DNODE *crfsnp_dnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key)
{
    const CRFSNPRB_POOL *pool;
    uint32_t root_pos;

    pool     = CRFSNP_ITEMS_POOL(crfsnp);
    root_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, CRFSNP_DNODE_DIR_BUCKET_POS(first_hash));

    return crfsnprb_tree_search_data(pool, root_pos, second_hash, klen, key);
}

uint32_t crfsnp_dnode_insert(CRFSNP *crfsnp, const uint32_t parent_pos, const uint32_t path_seg_first_hash, const uint32_t path_seg_second_hash, const uint32_t path_seg_len, const uint8_t *path_seg, const uint32_t dir_flag)
{
    uint32_t insert_offset;
    uint32_t bucket_pos;
    uint32_t root_pos;

    CRFSNP_ITEM *crfsnp_item_parent;
    CRFSNP_ITEM *crfsnp_item_insert;

    CRFSNP_DNODE *crfsnp_dnode_parent;

    if(CRFSNP_ITEM_FILE_IS_REG != dir_flag 
    && CRFSNP_ITEM_FILE_IS_DIR != dir_flag 
    && CRFSNP_ITEM_FILE_IS_BIG != dir_flag)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_dnode_insert: invalid input dir flag %x\n", dir_flag);
        return (CRFSNPRB_ERR_POS);
    }

    if(EC_TRUE == crfsnp_is_full(crfsnp))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_dnode_insert: crfsnp is full\n");
        return (CRFSNPRB_ERR_POS);
    }

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);/*must be dnode*/
    if(NULL_PTR == crfsnp_item_parent)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_dnode_insert: fetch parent item failed where parent offset %u\n", parent_pos);
        return (CRFSNPRB_ERR_POS);
    }

    crfsnp_dnode_parent = CRFSNP_ITEM_DNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_DIR != CRFSNP_ITEM_DFLG(crfsnp_item_parent) 
    || CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item_parent))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_dnode_insert: invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DFLG(crfsnp_item_parent),
                            CRFSNP_ITEM_STAT(crfsnp_item_parent));
        return (CRFSNPRB_ERR_POS);
    }

    /*insert the item to parent and update parent*/
    bucket_pos  = CRFSNP_DNODE_DIR_BUCKET_POS(path_seg_first_hash);
    root_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode_parent, bucket_pos);

    if(EC_FALSE == crfsnprb_tree_insert_data(CRFSNP_ITEMS_POOL(crfsnp), &root_pos, path_seg_second_hash, path_seg_len, path_seg, &insert_offset))
    {
        sys_log(LOGSTDOUT, "warn:crfsnp_dnode_insert: found duplicate rb node with root %u at node %u\n", root_pos, insert_offset);
        return (insert_offset);
    }
    crfsnp_item_insert = crfsnp_fetch(crfsnp, insert_offset);

    /*fill in crfsnp_item_insert*/    
    crfsnp_item_set_key(crfsnp_item_insert, path_seg_len, path_seg);
    CRFSNP_ITEM_PARENT_POS(crfsnp_item_insert) = parent_pos;
    if(CRFSNP_ITEM_FILE_IS_REG == dir_flag)
    {
        crfsnp_fnode_init(CRFSNP_ITEM_FNODE(crfsnp_item_insert));
        CRFSNP_ITEM_DFLG(crfsnp_item_insert) = CRFSNP_ITEM_FILE_IS_REG;
    }
    else if(CRFSNP_ITEM_FILE_IS_DIR == dir_flag)
    {
        crfsnp_dnode_init(CRFSNP_ITEM_DNODE(crfsnp_item_insert));
        CRFSNP_ITEM_DFLG(crfsnp_item_insert) = CRFSNP_ITEM_FILE_IS_DIR;
    }
    else if(CRFSNP_ITEM_FILE_IS_BIG == dir_flag)
    {
        crfsnp_bnode_init(CRFSNP_ITEM_BNODE(crfsnp_item_insert));
        CRFSNP_ITEM_DFLG(crfsnp_item_insert) = CRFSNP_ITEM_FILE_IS_BIG;
    }    
    CRFSNP_ITEM_STAT(crfsnp_item_insert) = CRFSNP_ITEM_STAT_IS_USED;
    
    CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode_parent, bucket_pos) = root_pos;    
    CRFSNP_DNODE_FILE_NUM(crfsnp_dnode_parent) ++;
    return (insert_offset);
}

/**
* umount one son from crfsnp_dnode,  where son is regular file item or dir item without any son
* crfsnp_dnode will be impacted on bucket and file num
**/
CRFSNP_ITEM * crfsnp_dnode_umount_son(const CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key)
{
    CRFSNPRB_POOL *pool;
    CRFSNP_ITEM   *item;
    uint32_t       root_pos;
    uint32_t       node_pos;
    uint32_t       bucket_pos;

    node_pos = crfsnp_dnode_search(crfsnp, crfsnp_dnode, first_hash, second_hash, klen, key);
    if(CRFSNPRB_ERR_POS == node_pos)
    {
        return (NULL_PTR);
    }

    item = crfsnp_fetch(crfsnp, node_pos);

    bucket_pos = CRFSNP_DNODE_DIR_BUCKET_POS(first_hash);
    root_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, bucket_pos);

    pool = CRFSNP_ITEMS_POOL(crfsnp);
    crfsnprb_tree_delete(pool, &root_pos, node_pos);
    
    CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, bucket_pos) = root_pos;
    CRFSNP_DNODE_FILE_NUM(crfsnp_dnode) --;
    
    return (item);
}

/*delete single item from dnode*/
static EC_BOOL __crfsnp_dnode_delete_item(const CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, CRFSNP_ITEM *crfsnp_item, CVECTOR *crfsnp_item_vec)
{
    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        if(NULL_PTR != crfsnp_item_vec)
        {
            cvector_push_no_lock(crfsnp_item_vec, crfsnp_item);
        }

        crfsnp_item_clean(crfsnp_item);
        CRFSNP_DNODE_FILE_NUM(crfsnp_dnode) --;
    }

    else if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        crfsnp_dnode_delete_dir_son(crfsnp, CRFSNP_ITEM_DNODE(crfsnp_item), crfsnp_item_vec);/*recursively*/
        crfsnp_item_clean(crfsnp_item);
        CRFSNP_DNODE_FILE_NUM(crfsnp_dnode) --;
    }
    
    return (EC_TRUE);
}

static EC_BOOL __crfsnp_dnode_delete_all_items(const CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, const uint32_t node_pos, CVECTOR *crfsnp_item_vec)
{
    CRFSNPRB_POOL *pool;
    CRFSNPRB_NODE *node;
    CRFSNP_ITEM   *item;

    pool = CRFSNP_ITEMS_POOL(crfsnp);
  
    node  = CRFSNPRB_POOL_NODE(pool, node_pos);    
    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_LEFT_POS(node))
    {
        __crfsnp_dnode_delete_all_items(crfsnp, crfsnp_dnode, CRFSNPRB_NODE_LEFT_POS(node), crfsnp_item_vec);
    }

    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_RIGHT_POS(node))
    {
        __crfsnp_dnode_delete_all_items(crfsnp, crfsnp_dnode, CRFSNPRB_NODE_RIGHT_POS(node), crfsnp_item_vec);
    }
    
    item = CRFSNP_RB_NODE_ITEM(node);
    __crfsnp_dnode_delete_item(crfsnp, crfsnp_dnode, item, crfsnp_item_vec);

    /*crfsnprb recycle the rbnode, do not use crfsnprb_tree_delete which will change the tree structer*/
    crfsnprb_node_free(pool, node_pos);
    
    return (EC_TRUE);
}

/*delete one dir son, not including crfsnp_dnode itself*/
EC_BOOL crfsnp_dnode_delete_dir_son(const CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, CVECTOR *crfsnp_item_vec)
{
    uint32_t bucket_pos;

    for(bucket_pos = 0; bucket_pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; bucket_pos ++)
    {
        uint32_t root_pos;

        root_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, bucket_pos);    
        if(CRFSNPRB_ERR_POS != root_pos)
        {
            __crfsnp_dnode_delete_all_items(crfsnp, crfsnp_dnode, root_pos, crfsnp_item_vec);
            CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, bucket_pos) = CRFSNPRB_ERR_POS;
        }
    }
    return (EC_TRUE);
}

CRFSNP_ITEM *crfsnp_bnode_find(const CRFSNP *crfsnp, const CRFSNP_BNODE *crfsnp_bnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key)
{
    const CRFSNPRB_POOL *pool;
    uint32_t root_pos;
    uint32_t node_pos;

    pool     = CRFSNP_ITEMS_POOL(crfsnp);
    root_pos = CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, CRFSNP_BNODE_SEG_BUCKET_POS(first_hash));

    node_pos = crfsnprb_tree_search_data(pool, root_pos, second_hash, klen, key);
    if(CRFSNPRB_ERR_POS != node_pos)
    {
        const CRFSNPRB_NODE *node;
        const CRFSNP_ITEM   *item;
        
        node = CRFSNPRB_POOL_NODE(pool, node_pos);
        item = CRFSNP_RB_NODE_ITEM(node);

        return (CRFSNP_ITEM *)(item);
    }

    return (NULL_PTR);
}

uint32_t crfsnp_bnode_search(const CRFSNP *crfsnp, const CRFSNP_BNODE *crfsnp_bnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key)
{
    const CRFSNPRB_POOL *pool;
    uint32_t root_pos;

    pool     = CRFSNP_ITEMS_POOL(crfsnp);
    root_pos = CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, CRFSNP_BNODE_SEG_BUCKET_POS(first_hash));

    return crfsnprb_tree_search_data(pool, root_pos, second_hash, klen, key);
}

uint32_t crfsnp_bnode_insert(CRFSNP *crfsnp, const uint32_t parent_pos, const uint32_t path_seg_first_hash, const uint32_t path_seg_second_hash, const uint32_t path_seg_len, const uint8_t *path_seg, const uint32_t dir_flag)
{
    uint32_t insert_offset;
    uint32_t bucket_pos;
    uint32_t root_pos;

    CRFSNP_ITEM *crfsnp_item_parent;
    CRFSNP_ITEM *crfsnp_item_insert;

    CRFSNP_BNODE *crfsnp_bnode_parent;

    if(CRFSNP_ITEM_FILE_IS_REG != dir_flag)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_bnode_insert: invalid input dir flag %x\n", dir_flag);
        return (CRFSNPRB_ERR_POS);
    }

    if(EC_TRUE == crfsnp_is_full(crfsnp))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_bnode_insert: crfsnp is full\n");
        return (CRFSNPRB_ERR_POS);
    }

    crfsnp_item_parent = crfsnp_fetch(crfsnp, parent_pos);/*must be bnode*/
    if(NULL_PTR == crfsnp_item_parent)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_bnode_insert: fetch parent item failed where parent offset %u\n", parent_pos);
        return (CRFSNPRB_ERR_POS);
    }

    crfsnp_bnode_parent = CRFSNP_ITEM_BNODE(crfsnp_item_parent);
    if(CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item_parent) 
    || CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item_parent))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_bnode_insert: parent owns invalid dir flag %u or stat %u\n",
                            CRFSNP_ITEM_DFLG(crfsnp_item_parent),
                            CRFSNP_ITEM_STAT(crfsnp_item_parent));
        return (CRFSNPRB_ERR_POS);
    }

    /*insert the item to parent and update parent*/
    bucket_pos  = CRFSNP_BNODE_SEG_BUCKET_POS(path_seg_first_hash);
    root_pos = CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode_parent, bucket_pos);

    if(EC_FALSE == crfsnprb_tree_insert_data(CRFSNP_ITEMS_POOL(crfsnp), &root_pos, path_seg_second_hash, path_seg_len, path_seg, &insert_offset))
    {
        sys_log(LOGSTDOUT, "warn:crfsnp_bnode_insert: found duplicate rb node with root %u at node %u\n", root_pos, insert_offset);
        return (insert_offset);
    }
    crfsnp_item_insert = crfsnp_fetch(crfsnp, insert_offset);

    /*fill in crfsnp_item_insert*/    
    crfsnp_item_set_key(crfsnp_item_insert, path_seg_len, path_seg);
    CRFSNP_ITEM_PARENT_POS(crfsnp_item_insert) = parent_pos;
    if(CRFSNP_ITEM_FILE_IS_REG == dir_flag)
    {
        crfsnp_fnode_init(CRFSNP_ITEM_FNODE(crfsnp_item_insert));
        CRFSNP_ITEM_DFLG(crfsnp_item_insert) = CRFSNP_ITEM_FILE_IS_REG;
    }
    CRFSNP_ITEM_STAT(crfsnp_item_insert) = CRFSNP_ITEM_STAT_IS_USED;
    
    CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode_parent, bucket_pos) = root_pos;    
    CRFSNP_BNODE_SEG_NUM(crfsnp_bnode_parent) ++;

    return (insert_offset);
}

/*delete single item from bnode*/
static EC_BOOL __crfsnp_bnode_delete_item(const CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, CRFSNP_ITEM *crfsnp_item, CVECTOR *crfsnp_item_vec)
{
    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        if(NULL_PTR != crfsnp_item_vec)
        {
            cvector_push_no_lock(crfsnp_item_vec, crfsnp_item);
        }

        crfsnp_item_clean(crfsnp_item);
        CRFSNP_BNODE_SEG_NUM(crfsnp_bnode) --;
        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "error:__crfsnp_bnode_delete_item:invalid dir flag %x\n", CRFSNP_ITEM_DFLG(crfsnp_item));    
    return (EC_FALSE);
}

static EC_BOOL __crfsnp_bnode_delete_all_items(const CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, const uint32_t node_pos, CVECTOR *crfsnp_item_vec)
{
    CRFSNPRB_POOL *pool;
    CRFSNPRB_NODE *node;
    CRFSNP_ITEM   *item;

    pool = CRFSNP_ITEMS_POOL(crfsnp);
  
    node  = CRFSNPRB_POOL_NODE(pool, node_pos);    
    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_LEFT_POS(node))
    {
        __crfsnp_bnode_delete_all_items(crfsnp, crfsnp_bnode, CRFSNPRB_NODE_LEFT_POS(node), crfsnp_item_vec);
    }

    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_RIGHT_POS(node))
    {
        __crfsnp_bnode_delete_all_items(crfsnp, crfsnp_bnode, CRFSNPRB_NODE_RIGHT_POS(node), crfsnp_item_vec);
    }
    
    item = CRFSNP_RB_NODE_ITEM(node);
    __crfsnp_bnode_delete_item(crfsnp, crfsnp_bnode, item, crfsnp_item_vec);

    /*crfsnprb recycle the rbnode, do not use crfsnprb_tree_delete which will change the tree structer*/
    crfsnprb_node_free(pool, node_pos);
    
    return (EC_TRUE);
}

/*delete one dir son, not including crfsnp_bnode itself*/
EC_BOOL crfsnp_bnode_delete_dir_son(const CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, CVECTOR *crfsnp_item_vec)
{
    uint32_t bucket_pos;

    for(bucket_pos = 0; bucket_pos < CRFSNP_BNODE_SEG_BUCKET_MAX_NUM; bucket_pos ++)
    {
        uint32_t root_pos;

        root_pos = CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, bucket_pos);    
        if(CRFSNPRB_ERR_POS != root_pos)
        {
            __crfsnp_bnode_delete_all_items(crfsnp, crfsnp_bnode, root_pos, crfsnp_item_vec);
            CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, bucket_pos) = CRFSNPRB_ERR_POS;
        }
    }
    return (EC_TRUE);
}

uint32_t crfsnp_search_no_lock(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag)
{
    uint32_t node_pos;
    uint32_t path_seg_len;
    uint8_t *path_seg_beg;
    uint8_t *path_seg_end;

    path_seg_beg = (uint8_t *)path;
    path_seg_len = 0;
    path_seg_end = (uint8_t *)(path_seg_beg + path_seg_len + 1);/*path always start with '/'*/

    node_pos = 0;/*the first item is root directory*/
    sys_log(LOGSTDNULL, "[DEBUG] crfsnp_search_no_lock: np %u, item pos %u\n", CRFSNP_ID(crfsnp), node_pos);
    while(CRFSNPRB_ERR_POS != node_pos)
    {
        CRFSNP_ITEM *crfsnp_item;

        sys_log(LOGSTDNULL, "[DEBUG] crfsnp_search_no_lock: np %u, node_pos %u, item pos %u\n",
                            CRFSNP_ID(crfsnp), node_pos, (node_pos / sizeof(CRFSNP_ITEM)));

        crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
        if(CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item))
        {
            sys_log(LOGSTDOUT, "error:crfsnp_search_no_lock: np %u, item at node_pos %u was not used\n", CRFSNP_ID(crfsnp), node_pos);
            return (CRFSNPRB_ERR_POS);
        }

        if(EC_FALSE == crfsnp_item_is(crfsnp_item, path_seg_len, path_seg_beg))
        {
            sys_log(LOGSTDOUT, "error:crfsnp_search_no_lock: np %u, check failed where path seg: ", CRFSNP_ID(crfsnp));
            sys_print(LOGSTDOUT, "%.*s\n", path_seg_len, path_seg_beg);
            return (CRFSNPRB_ERR_POS);
        }

        /*when matched and reached the last path seg*/
        if(path_len <= (uint32_t)(path_seg_end - path))
        {
            sys_log(LOGSTDNULL, "[DEBUG] [target dflag %u] crfsnp_search_no_lock: np %u, "
                                "matched and reached end where path_len %u, len from path to path_seg_end is %u, node_pos %u\n",
                                dflag, CRFSNP_ID(crfsnp), path_len, path_seg_end - path, node_pos);

            if(CRFSNP_ITEM_FILE_IS_ANY == dflag || dflag == CRFSNP_ITEM_DFLG(crfsnp_item))
            {
                return (node_pos);
            }

            /*big file can be searched by REG file dflag*/
            if(CRFSNP_ITEM_FILE_IS_REG == dflag && CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
            {
                return (node_pos);
            }

            return (CRFSNPRB_ERR_POS);
        }

        if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))/*no more to search*/
        {
            return (CRFSNPRB_ERR_POS);
        }

        if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))/*no more to search*/
        {
            return (CRFSNPRB_ERR_POS);
        }        

        if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))/*search sons*/
        {
            uint32_t path_seg_1st_hash;
            uint32_t path_seg_2nd_hash;

            path_seg_beg = (uint8_t *)path_seg_end;
            path_seg_len = crfsnp_path_seg_len(path, path_len, path_seg_beg);
            path_seg_end = path_seg_beg + path_seg_len + 1;

            path_seg_1st_hash = CRFSNP_1ST_CHASH_ALGO_COMPUTE(crfsnp, path_seg_len, path_seg_beg);
            path_seg_2nd_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, path_seg_len, path_seg_beg);
            node_pos          = crfsnp_dnode_search(crfsnp, CRFSNP_ITEM_DNODE(crfsnp_item), 
                                                       path_seg_1st_hash, path_seg_2nd_hash, 
                                                       path_seg_len, path_seg_beg);
            if(CRFSNPRB_ERR_POS == node_pos)/*Oops!*/
            {
                return (CRFSNPRB_ERR_POS);
            }
        }
        else
        {
            sys_log(LOGSTDOUT, "error:crfsnp_search_no_lock_item: np %u, invalid item dir flag %u at node_pos %u\n",
                                CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item), node_pos);
            break;
        }
    }

    return (CRFSNPRB_ERR_POS);
}

uint32_t crfsnp_search(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag)
{
    uint32_t node_pos;

    CRFSNP_RDLOCK(crfsnp, LOC_CRFSNP_0016);
    node_pos = crfsnp_search_no_lock(crfsnp, path_len, path, dflag);
    CRFSNP_UNLOCK(crfsnp, LOC_CRFSNP_0017);

    return (node_pos);
}

uint32_t crfsnp_insert_no_lock(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag)
{
    uint32_t node_pos;
    uint32_t path_seg_len;
    uint8_t *path_seg_beg;
    uint8_t *path_seg_end;

    if('/' != (*path))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_insert_no_lock: np %u, invalid path ", CRFSNP_ID(crfsnp));
        sys_print(LOGSTDOUT, "%.*s\n", path_len, path);
        return (CRFSNPRB_ERR_POS);
    }

    path_seg_end = (uint8_t *)(path + 1);/*path always start with '/'*/

    sys_log(LOGSTDNULL, "[DEBUG] crfsnp_insert_no_lock: np header is\n");
    crfsnp_header_print(LOGSTDNULL, crfsnp);

    node_pos = 0;/*the first item is root directory*/
    sys_log(LOGSTDNULL, "[DEBUG] crfsnp_insert_no_lock: np %u, node_pos %u\n", CRFSNP_ID(crfsnp), node_pos);
    while(CRFSNPRB_ERR_POS != node_pos)
    {
        CRFSNP_ITEM *crfsnp_item;

        sys_log(LOGSTDNULL, "[DEBUG] crfsnp_insert_no_lock: np %u, node_pos %u\n", CRFSNP_ID(crfsnp), node_pos);

        crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
        sys_log(LOGSTDNULL, "[DEBUG] crfsnp_insert_no_lock: np %u, node_pos %u,  dir flag %u\n",
                            CRFSNP_ID(crfsnp), node_pos, CRFSNP_ITEM_DFLG(crfsnp_item));
        if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            sys_log(LOGSTDOUT, "error:crfsnp_insert_no_lock: np %u, find regular file at node_pos %u has same key: ",
                                CRFSNP_ID(crfsnp), node_pos);
            sys_print(LOGSTDOUT, "%.*s\n", CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));

            return (CRFSNPRB_ERR_POS);
        }
#if 0
        if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            sys_log(LOGSTDOUT, "error:crfsnp_insert_no_lock: np %u, find big file at node_pos %u has same key: ",
                                CRFSNP_ID(crfsnp), node_pos);
            sys_print(LOGSTDOUT, "%.*s\n", CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));

            return (CRFSNPRB_ERR_POS);
        }       
#endif        

        else if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            uint32_t path_seg_1st_hash;
            uint32_t path_seg_2nd_hash;
            uint32_t parent_node_pos;

            path_seg_beg = (uint8_t *)path_seg_end;
            path_seg_len = crfsnp_path_seg_len(path, path_len, path_seg_beg);
            path_seg_end = path_seg_beg + path_seg_len + 1;

            sys_log(LOGSTDNULL, "[DEBUG] crfsnp_insert_no_lock: path_seg_len %u\n", path_seg_len);
            if(CRFSNP_KEY_MAX_SIZE < path_seg_len)
            {
                sys_log(LOGSTDOUT, "error:crfsnp_insert_no_lock: path_seg_len %u overflow\n", path_seg_len);
                return (CRFSNPRB_ERR_POS);
            }

            path_seg_1st_hash = CRFSNP_1ST_CHASH_ALGO_COMPUTE(crfsnp, path_seg_len, path_seg_beg);
            path_seg_2nd_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, path_seg_len, path_seg_beg);
            parent_node_pos   = crfsnp_dnode_search(crfsnp, CRFSNP_ITEM_DNODE(crfsnp_item), 
                                                       path_seg_1st_hash, path_seg_2nd_hash, 
                                                       path_seg_len, path_seg_beg);
            sys_log(LOGSTDNULL, "[DEBUG] crfsnp_insert_no_lock: np %u, searched node_pos %u, path_seg_len %u, path_seg_beg: %s\n",
                                CRFSNP_ID(crfsnp),
                                node_pos,
                                path_seg_len, path_seg_beg
                                );
            if(CRFSNPRB_ERR_POS != parent_node_pos)
            {
                node_pos = parent_node_pos;
                continue;
            }

            if(path_len > (uint32_t)(path_seg_end - path))/*create dnode item under parent crfsnp_item*/
            {
                node_pos = crfsnp_dnode_insert(crfsnp,
                                            node_pos,
                                            path_seg_1st_hash,
                                            path_seg_2nd_hash,                                            
                                            path_seg_len,
                                            path_seg_beg,
                                            CRFSNP_ITEM_FILE_IS_DIR
                                            );
                continue;
            }
            else/*create fnode item under parent crfsnp_item*/
            {
                node_pos = crfsnp_dnode_insert(crfsnp,
                                            node_pos,
                                            path_seg_1st_hash,
                                            path_seg_2nd_hash,                                            
                                            path_seg_len,
                                            path_seg_beg,
                                            /*CRFSNP_ITEM_FILE_IS_REG*/dflag
                                            );
                return (node_pos);
            }
        }
        
        else if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            uint32_t path_seg_1st_hash;
            uint32_t path_seg_2nd_hash;
            uint32_t parent_node_pos;

            path_seg_beg = (uint8_t *)path_seg_end;
            path_seg_len = crfsnp_path_seg_len(path, path_len, path_seg_beg);
            path_seg_end = path_seg_beg + path_seg_len + 1;

            sys_log(LOGSTDNULL, "[DEBUG] crfsnp_insert_no_lock: path_seg_len %u\n", path_seg_len);
            if(CRFSNP_KEY_MAX_SIZE < path_seg_len)
            {
                sys_log(LOGSTDOUT, "error:crfsnp_insert_no_lock: path_seg_len %u overflow\n", path_seg_len);
                return (CRFSNPRB_ERR_POS);
            }

            path_seg_1st_hash = CRFSNP_1ST_CHASH_ALGO_COMPUTE(crfsnp, path_seg_len, path_seg_beg);
            path_seg_2nd_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, path_seg_len, path_seg_beg);
            parent_node_pos   = crfsnp_bnode_search(crfsnp, CRFSNP_ITEM_BNODE(crfsnp_item), 
                                                       path_seg_1st_hash, path_seg_2nd_hash, 
                                                       path_seg_len, path_seg_beg);
            sys_log(LOGSTDNULL, "[DEBUG] crfsnp_insert_no_lock: np %u, searched node_pos %u, path_seg_len %u, path_seg_beg: %s\n",
                                CRFSNP_ID(crfsnp),
                                node_pos,
                                path_seg_len, path_seg_beg
                                );
            if(CRFSNPRB_ERR_POS != parent_node_pos)
            {
                node_pos = parent_node_pos;
                continue;
            }

            if(path_len > (uint32_t)(path_seg_end - path))/*create dnode item under parent crfsnp_item*/
            {
                sys_log(LOGSTDOUT, "error:crfsnp_insert_no_lock: np %u, invalid item big flag %u at node_pos %u\n",
                                    CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item), node_pos);
                return (CRFSNPRB_ERR_POS);                                

            }
            else/*create fnode item under parent crfsnp_item*/
            {
                node_pos = crfsnp_bnode_insert(crfsnp,
                                            node_pos,
                                            path_seg_1st_hash,
                                            path_seg_2nd_hash,                                            
                                            path_seg_len,
                                            path_seg_beg,
                                            CRFSNP_ITEM_FILE_IS_REG
                                            );
                return (node_pos);
            }
        }        
        else
        {
            sys_log(LOGSTDOUT, "error:crfsnp_insert_no_lock: np %u, invalid item dir flag %u at node_pos %u\n",
                                CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item), node_pos);
            break;
        }
    }

    return (CRFSNPRB_ERR_POS);
}

uint32_t crfsnp_insert(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag)
{
    uint32_t node_pos;

    CRFSNP_WRLOCK(crfsnp, LOC_CRFSNP_0018);
    node_pos = crfsnp_insert_no_lock(crfsnp, path_len, path, dflag);
    CRFSNP_UNLOCK(crfsnp, LOC_CRFSNP_0019);

    return (node_pos);
}

CRFSNP_ITEM *crfsnp_fetch(const CRFSNP *crfsnp, const uint32_t node_pos)
{
    if(CRFSNPRB_ERR_POS != node_pos)
    {
        const CRFSNPRB_POOL *pool;
        const CRFSNPRB_NODE *node;

        pool = CRFSNP_ITEMS_POOL(crfsnp);
        node = CRFSNPRB_POOL_NODE(pool, node_pos);
        if(NULL_PTR != node)
        {
            return (CRFSNP_ITEM *)CRFSNP_RB_NODE_ITEM(node);
        }
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfsnp_fetch: np %u, fetch node %u failed\n", CRFSNP_ID(crfsnp), node_pos);
    return (NULL_PTR);
}

EC_BOOL crfsnp_inode_update(CRFSNP *crfsnp, CRFSNP_INODE *crfsnp_inode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)
{
    if(src_disk_no  == CRFSNP_INODE_DISK_NO(crfsnp_inode) 
    && src_block_no == CRFSNP_INODE_BLOCK_NO(crfsnp_inode)
    && src_page_no  == CRFSNP_INODE_PAGE_NO(crfsnp_inode))
    {
        CRFSNP_INODE_DISK_NO(crfsnp_inode)  = des_disk_no;
        CRFSNP_INODE_BLOCK_NO(crfsnp_inode) = des_block_no;
        CRFSNP_INODE_PAGE_NO(crfsnp_inode)  = des_page_no;
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_fnode_update(CRFSNP *crfsnp, CRFSNP_FNODE *crfsnp_fnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)

{
    uint32_t replica;

    for(replica = 0; replica < CRFSNP_FNODE_REPNUM(crfsnp_fnode); replica ++)
    {
        crfsnp_inode_update(crfsnp, CRFSNP_FNODE_INODE(crfsnp_fnode, replica), 
                            src_disk_no, src_block_no, src_page_no, 
                            des_disk_no, des_block_no, des_page_no);
    }
    return (EC_TRUE);
}

static EC_BOOL __crfsnp_bucket_update(CRFSNP * crfsnp, CRFSNPRB_POOL *pool, const uint32_t node_pos, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)
{
    CRFSNPRB_NODE *node;
    CRFSNP_ITEM   *item;

    if(CRFSNPRB_ERR_POS == node_pos)
    {
        return (EC_TRUE);
    }
    
    node  = CRFSNPRB_POOL_NODE(pool, node_pos);    
    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_LEFT_POS(node))
    {
        __crfsnp_bucket_update(crfsnp, pool, CRFSNPRB_NODE_LEFT_POS(node), 
                               src_disk_no, src_block_no, src_page_no, 
                               des_disk_no, des_block_no, des_page_no);
    }

    item = CRFSNP_RB_NODE_ITEM(node);

    crfsnp_item_update(crfsnp, item, 
                       src_disk_no, src_block_no, src_page_no, 
                       des_disk_no, des_block_no, des_page_no);


    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_RIGHT_POS(node))
    {
        __crfsnp_bucket_update(crfsnp, pool, CRFSNPRB_NODE_RIGHT_POS(node), 
                               src_disk_no, src_block_no, src_page_no, 
                               des_disk_no, des_block_no, des_page_no);
    }    
    
    return (EC_TRUE);
}

EC_BOOL crfsnp_bucket_update(CRFSNP *crfsnp, const uint32_t node_pos, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)
{
    CRFSNPRB_POOL *pool;
    pool = CRFSNP_ITEMS_POOL(crfsnp);

    return __crfsnp_bucket_update(crfsnp, pool, node_pos,
                                   src_disk_no, src_block_no, src_page_no, 
                                   des_disk_no, des_block_no, des_page_no);    
}

EC_BOOL crfsnp_dnode_update(CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)

{
    uint32_t bucket_pos;
    for(bucket_pos = 0; bucket_pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; bucket_pos ++)
    {
        if(EC_FALSE == crfsnp_bucket_update(crfsnp, CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, bucket_pos), 
                                       src_disk_no, src_block_no, src_page_no, 
                                       des_disk_no, des_block_no, des_page_no))
        {
            sys_log(LOGSTDOUT, "error:crfsnp_dnode_update: update bucket %u failed\n", CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, bucket_pos));
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_bnode_update(CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)

{
    uint32_t bucket_pos;
    for(bucket_pos = 0; bucket_pos < CRFSNP_BNODE_SEG_BUCKET_MAX_NUM; bucket_pos ++)
    {
        if(EC_FALSE == crfsnp_bucket_update(crfsnp, CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, bucket_pos), 
                                       src_disk_no, src_block_no, src_page_no, 
                                       des_disk_no, des_block_no, des_page_no))
        {
            sys_log(LOGSTDOUT, "error:crfsnp_bnode_update: update bucket %u failed\n", CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, bucket_pos));
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_item_update(CRFSNP *crfsnp, CRFSNP_ITEM *crfsnp_item, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)
{
    if(CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_item_update: item was not used\n");
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        return crfsnp_fnode_update(crfsnp, CRFSNP_ITEM_FNODE(crfsnp_item), 
                                   src_disk_no, src_block_no, src_page_no, 
                                   des_disk_no, des_block_no, des_page_no);    
        
    }

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        return crfsnp_dnode_update(crfsnp, CRFSNP_ITEM_DNODE(crfsnp_item), 
                                   src_disk_no, src_block_no, src_page_no, 
                                   des_disk_no, des_block_no, des_page_no);    

    }

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        return crfsnp_bnode_update(crfsnp, CRFSNP_ITEM_BNODE(crfsnp_item), 
                                   src_disk_no, src_block_no, src_page_no, 
                                   des_disk_no, des_block_no, des_page_no);    

    }    

    sys_log(LOGSTDOUT, "error:crfsnp_item_update: invalid item dflag %u\n", CRFSNP_ITEM_DFLG(crfsnp_item));
    return (EC_FALSE);
}

EC_BOOL crfsnp_update_no_lock(CRFSNP *crfsnp, 
                               const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                               const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)

{
    uint32_t offset;
    CRFSNP_ITEM *crfsnp_item;

    offset = 0;/*the first item is root directory*/
    crfsnp_item = crfsnp_fetch(crfsnp, offset);
    return crfsnp_item_update(crfsnp, crfsnp_item, 
                              src_disk_no, src_block_no, src_page_no, 
                              des_disk_no, des_block_no, des_page_no);    /*recursively*/
}


CRFSNP_ITEM *crfsnp_set(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag)
{
    return crfsnp_fetch(crfsnp, crfsnp_insert(crfsnp, path_len, path, dflag));
}

CRFSNP_ITEM *crfsnp_get(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag)
{
    if(path_len > 0 && '/' == *(path + path_len - 1))
    {
        if(CRFSNP_ITEM_FILE_IS_DIR != dflag && CRFSNP_ITEM_FILE_IS_ANY != dflag)
        {
            return (NULL_PTR);
        }

        return crfsnp_fetch(crfsnp, crfsnp_search(crfsnp, path_len - 1, path, CRFSNP_ITEM_FILE_IS_DIR));
    }
    return crfsnp_fetch(crfsnp, crfsnp_search(crfsnp, path_len, path, dflag));
}

EC_BOOL crfsnp_delete(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag, CVECTOR *crfsnp_item_vec)
{
    CRFSNP_ITEM *crfsnp_item;

    if('/' != (*path))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_delete: np %u, invalid path %.*s\n", CRFSNP_ID(crfsnp), path_len, (char *)path);
        return (EC_FALSE);
    }

    if(path_len > 0 && '/' == *(path + path_len - 1))
    {
        if(CRFSNP_ITEM_FILE_IS_DIR != dflag && CRFSNP_ITEM_FILE_IS_ANY != dflag)
        {
            return (EC_FALSE);
        }

        CRFSNP_WRLOCK(crfsnp, LOC_CRFSNP_0020);
        crfsnp_item = crfsnp_fetch(crfsnp, crfsnp_search_no_lock(crfsnp, path_len - 1, path, CRFSNP_ITEM_FILE_IS_DIR));
    }
    else
    {
        CRFSNP_WRLOCK(crfsnp, LOC_CRFSNP_0021);
        crfsnp_item = crfsnp_fetch(crfsnp, crfsnp_search_no_lock(crfsnp, path_len, path, dflag));
    }

    if(NULL_PTR == crfsnp_item)
    {
        CRFSNP_UNLOCK(crfsnp, LOC_CRFSNP_0022);
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_ITEM *crfsnp_item_parent;
        CRFSNP_ITEM *crfsnp_item_son;
        uint32_t first_hash;
        uint32_t second_hash;

        crfsnp_item_parent = crfsnp_fetch(crfsnp, CRFSNP_ITEM_PARENT_POS(crfsnp_item));
        first_hash  = CRFSNP_1ST_CHASH_ALGO_COMPUTE(crfsnp, CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        second_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        crfsnp_item_son = crfsnp_dnode_umount_son(crfsnp, CRFSNP_ITEM_DNODE(crfsnp_item_parent), 
                                                  first_hash, second_hash, 
                                                  CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        ASSERT(crfsnp_item_son == crfsnp_item);
        if(NULL_PTR != crfsnp_item_vec)
        {
            cvector_push_no_lock(crfsnp_item_vec, crfsnp_item_son);
        }
        crfsnp_item_clean(crfsnp_item_son);
    }

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_ITEM *crfsnp_item_parent;
        CRFSNP_ITEM *crfsnp_item_son;
        uint32_t first_hash;
        uint32_t second_hash;

        crfsnp_item_parent = crfsnp_fetch(crfsnp, CRFSNP_ITEM_PARENT_POS(crfsnp_item));
        first_hash  = CRFSNP_1ST_CHASH_ALGO_COMPUTE(crfsnp, CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        second_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        crfsnp_item_son = crfsnp_dnode_umount_son(crfsnp, CRFSNP_ITEM_DNODE(crfsnp_item_parent), 
                                                  first_hash, second_hash, 
                                                  CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        ASSERT(crfsnp_item_son == crfsnp_item);
        crfsnp_dnode_delete_dir_son(crfsnp, CRFSNP_ITEM_DNODE(crfsnp_item_son), crfsnp_item_vec);
        crfsnp_item_clean(crfsnp_item_son);
    }

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_ITEM *crfsnp_item_parent;
        CRFSNP_ITEM *crfsnp_item_son;
        uint32_t first_hash;
        uint32_t second_hash;

        crfsnp_item_parent = crfsnp_fetch(crfsnp, CRFSNP_ITEM_PARENT_POS(crfsnp_item));
        //ASSERT(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item_parent ));
        first_hash  = CRFSNP_1ST_CHASH_ALGO_COMPUTE(crfsnp, CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        second_hash = CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        //sys_log(LOGSTDOUT, "[DEBUG] crfsnp_delete: bigfile item key: %.*s\n", CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        crfsnp_item_son = crfsnp_dnode_umount_son(crfsnp, CRFSNP_ITEM_DNODE(crfsnp_item_parent), 
                                                  first_hash, second_hash, 
                                                  CRFSNP_ITEM_KLEN(crfsnp_item), CRFSNP_ITEM_KEY(crfsnp_item));
        ASSERT(crfsnp_item_son == crfsnp_item);

        crfsnp_bnode_delete_dir_son(crfsnp, CRFSNP_ITEM_BNODE(crfsnp_item_son), crfsnp_item_vec);
        crfsnp_item_clean(crfsnp_item_son);
    }

    CRFSNP_UNLOCK(crfsnp, LOC_CRFSNP_0023);

    return (EC_TRUE);
}

EC_BOOL crfsnp_path_name(const CRFSNP *crfsnp, const uint32_t node_pos, const uint32_t path_max_len, uint32_t *path_len, uint8_t *path)
{
    CSTACK   *cstack;
    uint32_t  cur_node_pos;
    uint32_t  cur_path_len;

    cstack = cstack_new(MM_IGNORE, LOC_CRFSNP_0024);

    cur_node_pos = node_pos;
    while(CRFSNPRB_ERR_POS != cur_node_pos)
    {
        CRFSNP_ITEM *crfsnp_item;

        cstack_push(cstack, (void *)cur_node_pos);

        crfsnp_item = crfsnp_fetch(crfsnp, cur_node_pos);
        cur_node_pos = CRFSNP_ITEM_PARENT_POS(crfsnp_item);
    }

    cur_path_len = 0;
    path[ 0 ] = '\0';

    while(EC_FALSE == cstack_is_empty(cstack) && cur_path_len < path_max_len)
    {
        CRFSNP_ITEM *crfsnp_item;

        cur_node_pos = (uint32_t)cstack_pop(cstack);
        crfsnp_item = crfsnp_fetch(crfsnp, cur_node_pos);

        if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            cur_path_len += snprintf((char *)path + cur_path_len, path_max_len - cur_path_len, "%.*s/",
                                    CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        }
        else if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            cur_path_len += snprintf((char *)path + cur_path_len, path_max_len - cur_path_len, "%.*s",
                                    CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        }
        else if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            cur_path_len += snprintf((char *)path + cur_path_len, path_max_len - cur_path_len, "%.*s",
                                    CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        }        
        else
        {
            sys_log(LOGSTDOUT, "error:crfsnp_path_name: np %u, invalid dir flag %u at offset\n",
                                CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item), cur_node_pos);
        }
    }

    (*path_len) = cur_path_len;
    path[ cur_path_len ] = '\0';

    cstack_clean(cstack, NULL_PTR);/*cleanup for safe reason*/
    cstack_free(cstack, LOC_CRFSNP_0025);
    return (EC_TRUE);
}

EC_BOOL crfsnp_path_name_cstr(const CRFSNP *crfsnp, const uint32_t node_pos, CSTRING *path_cstr)
{
    CSTACK *cstack;
    uint32_t  cur_node_pos;

    cstack = cstack_new(MM_IGNORE, LOC_CRFSNP_0026);

    cur_node_pos = node_pos;
    while(CRFSNPRB_ERR_POS != cur_node_pos)
    {
        CRFSNP_ITEM *crfsnp_item;

        cstack_push(cstack, (void *)cur_node_pos);

        crfsnp_item = crfsnp_fetch(crfsnp, cur_node_pos);
        cur_node_pos  = CRFSNP_ITEM_PARENT_POS(crfsnp_item);
    }

    while(EC_FALSE == cstack_is_empty(cstack))
    {
        CRFSNP_ITEM *crfsnp_item;

        cur_node_pos = (uint32_t)cstack_pop(cstack);
        crfsnp_item = crfsnp_fetch(crfsnp, cur_node_pos);

        if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            cstring_format(path_cstr, "%.*s/", CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        }
        else if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            cstring_format(path_cstr, "%.*s", CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        }
        else if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            cstring_format(path_cstr, "%.*s", CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        }        
        else
        {
            sys_log(LOGSTDOUT, "error:crfsnp_path_name_cstr: np %u, invalid dir flag %u at offset\n",
                                CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item), cur_node_pos);
        }
    }

    cstack_clean(cstack, NULL_PTR);/*cleanup for safe reason*/
    cstack_free(cstack, LOC_CRFSNP_0027);
    return (EC_TRUE);
}

EC_BOOL crfsnp_seg_name(const CRFSNP *crfsnp, const uint32_t offset, const uint32_t seg_name_max_len, uint32_t *seg_name_len, uint8_t *seg_name)
{
    CRFSNP_ITEM *crfsnp_item;

    crfsnp_item = crfsnp_fetch(crfsnp, offset);

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        (*seg_name_len) = snprintf((char *)seg_name, seg_name_max_len, "%.*s/",
                                CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        return (EC_TRUE);
    }
    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        (*seg_name_len) = snprintf((char *)seg_name, seg_name_max_len, "%.*s",
                                CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        return (EC_TRUE);
    }
    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        (*seg_name_len) = snprintf((char *)seg_name, seg_name_max_len, "%.*s",
                                CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        return (EC_TRUE);
    }
    sys_log(LOGSTDOUT, "error:crfsnp_seg_name: np %u, invalid dir flag %u at offset\n",
                        CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item), offset);
    return (EC_FALSE);
}

EC_BOOL crfsnp_seg_name_cstr(const CRFSNP *crfsnp, const uint32_t offset, CSTRING *seg_cstr)
{
    CRFSNP_ITEM *crfsnp_item;

    crfsnp_item = crfsnp_fetch(crfsnp, offset);
    if(NULL_PTR == crfsnp_item)
    {
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        cstring_format(seg_cstr, "%.*s/", CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        return (EC_TRUE);
    }
    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        cstring_format(seg_cstr, "%.*s", CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        return (EC_TRUE);
    }
    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        cstring_format(seg_cstr, "%.*s", CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        return (EC_TRUE);
    }    

    sys_log(LOGSTDOUT, "error:crfsnp_seg_name_cstr: np %u, invalid dir flag %u at offset\n",
                        CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item), offset);
    return (EC_FALSE);
}


static EC_BOOL __crfsnp_list_path_vec(const CRFSNP *crfsnp, const uint32_t node_pos, const uint8_t *prev_path_str, CVECTOR *path_cstr_vec)
{
    const CRFSNPRB_POOL *pool;
    const CRFSNPRB_NODE *node;
    CSTRING *full_path_cstr;    

    if(CRFSNPRB_ERR_POS == node_pos)
    {
        return (EC_TRUE);
    }

    pool = CRFSNP_ITEMS_POOL(crfsnp);
    
    node  = CRFSNPRB_POOL_NODE(pool, node_pos);    
    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_LEFT_POS(node))
    {
        __crfsnp_list_path_vec(crfsnp, CRFSNPRB_NODE_LEFT_POS(node), prev_path_str, path_cstr_vec);
    }    

    full_path_cstr = cstring_new(prev_path_str, LOC_CRFSNP_0028);
    if(NULL_PTR == full_path_cstr)
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_list_path_vec: np %u, new cstring from %s failed\n", 
                            CRFSNP_ID(crfsnp), prev_path_str);
        return (EC_FALSE);
    }

    crfsnp_seg_name_cstr(crfsnp, node_pos, full_path_cstr);

    sys_log(LOGSTDOUT, "[DEBUG] crfsnp_list_path_vec: np %u, node_pos son %u, %s\n",
                        CRFSNP_ID(crfsnp), node_pos, (char *)cstring_get_str(full_path_cstr));

    if(CVECTOR_ERR_POS == cvector_search_front(path_cstr_vec, (void *)full_path_cstr, (CVECTOR_DATA_CMP)cstring_cmp))
    {
        cvector_push(path_cstr_vec, (void *)full_path_cstr);
    }
    else
    {
        cstring_free(full_path_cstr);
    }    

    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_RIGHT_POS(node))
    {
        __crfsnp_list_path_vec(crfsnp, CRFSNPRB_NODE_RIGHT_POS(node), prev_path_str, path_cstr_vec);
    }    

    return (EC_TRUE);
}

EC_BOOL crfsnp_list_path_vec(const CRFSNP *crfsnp, const uint32_t node_pos, CVECTOR *path_cstr_vec)
{
    CRFSNP_ITEM *crfsnp_item;
    CSTRING *path_cstr;

    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    if(NULL_PTR == crfsnp_item)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_list_path_vec: np %u, item is null at node_pos %u\n", CRFSNP_ID(crfsnp), node_pos);
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_REG != CRFSNP_ITEM_DFLG(crfsnp_item) 
    && CRFSNP_ITEM_FILE_IS_DIR != CRFSNP_ITEM_DFLG(crfsnp_item)
    && CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_list_path_vec: np %u, invalid dir flag %u at node_pos\n",
                            CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item), node_pos);
        return (EC_FALSE);
    }

    path_cstr = cstring_new(NULL_PTR, LOC_CRFSNP_0029);
    if(NULL_PTR == path_cstr)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_list_path_vec: np %u, new path cstr failed\n", CRFSNP_ID(crfsnp));
        return (EC_FALSE);
    }

    crfsnp_path_name_cstr(crfsnp, node_pos, path_cstr);

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        if(CVECTOR_ERR_POS == cvector_search_front(path_cstr_vec, (void *)path_cstr, (CVECTOR_DATA_CMP)cstring_cmp))
        {
            cvector_push(path_cstr_vec, (void *)path_cstr);
        }
        else
        {
            cstring_free(path_cstr);
        }

        return (EC_TRUE);
    }

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_DNODE *crfsnp_dnode;
        uint32_t crfsnp_bucket_pos;

        crfsnp_dnode = (CRFSNP_DNODE *)CRFSNP_ITEM_DNODE(crfsnp_item);
        for(crfsnp_bucket_pos = 0; crfsnp_bucket_pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; crfsnp_bucket_pos ++)
        {
            uint32_t son_node_pos;

            son_node_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, crfsnp_bucket_pos);
            sys_log(LOGSTDNULL, "[DEBUG] crfsnp_list_path_vec: np %u, crfsnp_bucket_posn = %u, node_pos son %u\n",
                                CRFSNP_ID(crfsnp), crfsnp_bucket_pos, son_node_pos);
            __crfsnp_list_path_vec(crfsnp, son_node_pos, cstring_get_str(path_cstr), path_cstr_vec);
        }

        cstring_free(path_cstr);
        return (EC_TRUE);
    }

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        if(CVECTOR_ERR_POS == cvector_search_front(path_cstr_vec, (void *)path_cstr, (CVECTOR_DATA_CMP)cstring_cmp))
        {
            cvector_push(path_cstr_vec, (void *)path_cstr);
        }
        else
        {
            cstring_free(path_cstr);
        }

        return (EC_TRUE);
    }    
    
    /*never reach here*/
    return (EC_FALSE);
}

EC_BOOL crfsnp_list_seg_vec(const CRFSNP *crfsnp, const uint32_t node_pos, CVECTOR *seg_cstr_vec)
{
    CRFSNP_ITEM *crfsnp_item;

    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    if(NULL_PTR == crfsnp_item)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_list_seg_vec: np %u, item is null at node_pos %u\n",
                            CRFSNP_ID(crfsnp), node_pos);
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_REG != CRFSNP_ITEM_DFLG(crfsnp_item) 
    && CRFSNP_ITEM_FILE_IS_DIR != CRFSNP_ITEM_DFLG(crfsnp_item)
    && CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_list_seg_vec: np %u, invalid dir flag %u at node_pos\n",
                            CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item), node_pos);
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CSTRING *seg_name_cstr;

        seg_name_cstr = cstring_new(NULL_PTR, LOC_CRFSNP_0030);
        if(NULL_PTR == seg_name_cstr)
        {
            sys_log(LOGSTDOUT, "error:crfsnp_list_seg_vec: np %u, new seg str failed\n", CRFSNP_ID(crfsnp));
            return (EC_FALSE);
        }

        crfsnp_seg_name_cstr(crfsnp, node_pos, seg_name_cstr);

        if(CVECTOR_ERR_POS == cvector_search_front(seg_cstr_vec, (void *)seg_name_cstr, (CVECTOR_DATA_CMP)cstring_cmp))
        {
            cvector_push(seg_cstr_vec, (void *)seg_name_cstr);
        }
        else
        {
            cstring_free(seg_name_cstr);
        }
        return (EC_TRUE);
    }

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_DNODE *crfsnp_dnode;
        uint32_t crfsnp_bucket_pos;

        crfsnp_dnode = (CRFSNP_DNODE *)CRFSNP_ITEM_DNODE(crfsnp_item);
        for(crfsnp_bucket_pos = 0; crfsnp_bucket_pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; crfsnp_bucket_pos ++)
        {
            uint32_t son_node_pos;

            son_node_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, crfsnp_bucket_pos);

            crfsnp_list_path_vec(crfsnp, son_node_pos, seg_cstr_vec);
        }

        return (EC_TRUE);
    }

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CSTRING *seg_name_cstr;

        seg_name_cstr = cstring_new(NULL_PTR, LOC_CRFSNP_0031);
        if(NULL_PTR == seg_name_cstr)
        {
            sys_log(LOGSTDOUT, "error:crfsnp_list_seg_vec: np %u, new seg str failed\n", CRFSNP_ID(crfsnp));
            return (EC_FALSE);
        }

        crfsnp_seg_name_cstr(crfsnp, node_pos, seg_name_cstr);

        if(CVECTOR_ERR_POS == cvector_search_front(seg_cstr_vec, (void *)seg_name_cstr, (CVECTOR_DATA_CMP)cstring_cmp))
        {
            cvector_push(seg_cstr_vec, (void *)seg_name_cstr);
        }
        else
        {
            cstring_free(seg_name_cstr);
        }
        return (EC_TRUE);
    }    

    /*never reach here*/
    return (EC_FALSE);
}

EC_BOOL crfsnp_file_num(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, uint32_t *file_num)
{
    CRFSNP_ITEM *crfsnp_item;

    crfsnp_item = crfsnp_get(crfsnp, path_len, path, CRFSNP_ITEM_FILE_IS_ANY);
    if(NULL_PTR == crfsnp_item)
    {
        (*file_num) = 0;
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        (*file_num) = 1;
        return (EC_TRUE);
    }

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_DNODE *crfsnp_dnode;
        crfsnp_dnode = CRFSNP_ITEM_DNODE(crfsnp_item);

        (*file_num) = CRFSNP_DNODE_FILE_NUM(crfsnp_dnode);
        return (EC_TRUE);
    }

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        (*file_num) = 1;
        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "error:crfsnp_file_num: np %u, invalid dflg %x\n", CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item));
    return (EC_FALSE);
}

EC_BOOL crfsnp_file_size(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, uint64_t *file_size)
{
    CRFSNP_ITEM *crfsnp_item;

    crfsnp_item = crfsnp_get(crfsnp, path_len, path, CRFSNP_ITEM_FILE_IS_ANY);
    if(NULL_PTR == crfsnp_item)
    {
        (*file_size) = 0;
        return (EC_FALSE);
    }

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_FNODE *crfsnp_fnode;
        crfsnp_fnode = CRFSNP_ITEM_FNODE(crfsnp_item);

        (*file_size) = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
        return (EC_TRUE);
    }

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        CRFSNP_BNODE *crfsnp_bnode;
        crfsnp_bnode = CRFSNP_ITEM_BNODE(crfsnp_item);

        (*file_size) = CRFSNP_BNODE_FILESZ(crfsnp_bnode);
        return (EC_TRUE);
    }    

    sys_log(LOGSTDOUT, "error:crfsnp_file_size: np %u, invalid dflg %x\n", CRFSNP_ID(crfsnp), CRFSNP_ITEM_DFLG(crfsnp_item));
    return (EC_FALSE);
}

EC_BOOL crfsnp_mkdirs(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path)
{
    if(CRFSNPRB_ERR_POS == crfsnp_insert(crfsnp, path_len, path, CRFSNP_ITEM_FILE_IS_DIR))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_mkdirs: mkdirs %.*s failed\n", path_len, (char *)path);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

CRFSNP *crfsnp_open(const char *np_root_dir, const uint32_t np_id)
{
    UINT32 fsize;
    char *np_fname;
    CRFSNP *crfsnp;
    CRFSNP_HEADER *crfsnp_header;
    int fd;

    np_fname = crfsnp_fname_gen(np_root_dir, np_id);
    if(NULL_PTR == np_fname)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_open: generate np fname from np_root_dir %s failed\n", np_root_dir);
        return (NULL_PTR);
    }

    if(EC_FALSE == c_file_access(np_fname, F_OK))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_open: np %s not exist, try to create it\n", np_fname);
        safe_free(np_fname, LOC_CRFSNP_0032);
        return (NULL_PTR);
    }

    fd = c_file_open(np_fname, O_RDWR, 0666);
    if(ERR_FD == fd)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_open: open crfsnp file %s failed\n", np_fname);
        safe_free(np_fname, LOC_CRFSNP_0033);
        return (NULL_PTR);
    }

    if(EC_FALSE == c_file_size(fd, &fsize))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_open: get size of %s failed\n", np_fname);
        safe_free(np_fname, LOC_CRFSNP_0034);
        c_file_close(fd);
        return (NULL_PTR);
    }    

    crfsnp_header = __crfsnp_header_open(np_id, (uint32_t)fsize, fd);
    if(NULL_PTR == crfsnp_header)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_open: open crfsnp file %s failed\n", np_fname);
        safe_free(np_fname, LOC_CRFSNP_0035);
        c_file_close(fd);
        return (NULL_PTR);
    }    

    crfsnp = crfsnp_new();
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_open: new crfsnp %u failed\n", np_id);
        safe_free(np_fname, LOC_CRFSNP_0036);
        c_file_close(fd);
        __crfsnp_header_close(crfsnp_header, np_id, fsize);
        return (NULL_PTR);
    }

    CRFSNP_HDR(crfsnp) = crfsnp_header;

    CRFSNP_1ST_CHASH_ALGO(crfsnp) = chash_algo_fetch(CRFSNP_HEADER_1ST_CHASH_ALGO_ID(crfsnp_header));
    CRFSNP_2ND_CHASH_ALGO(crfsnp) = chash_algo_fetch(CRFSNP_HEADER_2ND_CHASH_ALGO_ID(crfsnp_header));    

    CRFSNP_FD(crfsnp)    = fd;
    CRFSNP_FSIZE(crfsnp) = (uint32_t)fsize;
    CRFSNP_FNAME(crfsnp) = (uint8_t *)np_fname;

    ASSERT(np_id == CRFSNP_HEADER_NP_ID(crfsnp_header));

    return (crfsnp);
}

EC_BOOL crfsnp_close(CRFSNP *crfsnp)
{
    if(NULL_PTR != crfsnp)
    {
        if(NULL_PTR != CRFSNP_HDR(crfsnp))
        {
            __crfsnp_header_close(CRFSNP_HDR(crfsnp), CRFSNP_ID(crfsnp), CRFSNP_FSIZE(crfsnp));
            CRFSNP_HDR(crfsnp) = NULL_PTR;
        }
        crfsnp_free(crfsnp);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_sync(CRFSNP *crfsnp)
{
    if(NULL_PTR != crfsnp && NULL_PTR != CRFSNP_HDR(crfsnp))
    {
        __crfsnp_header_sync(CRFSNP_HDR(crfsnp), CRFSNP_ID(crfsnp), CRFSNP_FSIZE(crfsnp));
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_create_root_item(CRFSNP *crfsnp)
{
    CRFSNP_ITEM *crfsnp_item;
    uint32_t     second_hash;
    uint32_t     root_pos;
    uint32_t     insert_pos;
    uint32_t     klen;
    uint8_t      key[ 1 ];
    
    root_pos = CRFSNPRB_ERR_POS;
    second_hash = 0;
    klen = 0;
    key[0] = '\0';
    
    if(EC_FALSE == crfsnprb_tree_insert_data(CRFSNP_ITEMS_POOL(crfsnp), &root_pos, second_hash, klen, (uint8_t *)key, &insert_pos))
    {
        sys_log(LOGSTDOUT, "warn:crfsnp_create_root_item: insert create item failed\n");
        return (EC_FALSE);
    }    

    if(0 != insert_pos)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create_root_item: insert root item at pos %u is not zero!\n", insert_pos);
        return (EC_FALSE);
    }

    if(0 != root_pos)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create_root_item: root_pos %u is not zero!\n", root_pos);
        return (EC_FALSE);
    }
    
    crfsnp_item = crfsnp_fetch(crfsnp, insert_pos);

    CRFSNP_ITEM_DFLG(crfsnp_item)             = CRFSNP_ITEM_FILE_IS_DIR;
    CRFSNP_ITEM_STAT(crfsnp_item)             = CRFSNP_ITEM_STAT_IS_USED;
    CRFSNP_ITEM_KLEN(crfsnp_item)             = klen;
    CRFSNP_ITEM_PARENT_POS(crfsnp_item)       = CRFSNPRB_ERR_POS;

    /******************************************************************************************************/
    /*when enable this branch, qlist can query root dir "/"; otherwise, qlist query it will return nothing*/
    /*if enable this branch, qlist "/" will run-through all np which is time-cost operation!!!            */
    /******************************************************************************************************/

    //CRFSNP_ITEM_KEY(crfsnp_item)[ 0 ] = '/';/*deprecated*/
    CRFSNP_ITEM_KEY(crfsnp_item)[ 0 ] = key[ 0 ];

    crfsnp_dnode_init(CRFSNP_ITEM_DNODE(crfsnp_item));

    return (EC_TRUE);
}

CRFSNP *crfsnp_create(const char *np_root_dir, const uint32_t np_id, const uint8_t np_model, const uint8_t hash_1st_algo_id, const uint8_t hash_2nd_algo_id)
{
    CRFSNP  *crfsnp;
    CRFSNP_HEADER * crfsnp_header;
    char    *np_fname;
    int      fd;
    uint32_t fsize;
    uint32_t item_max_num;

    if(EC_FALSE == crfsnp_model_file_size(np_model, &fsize))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create: invalid np_model %u\n", np_model);
        return (NULL_PTR);
    }

    if(EC_FALSE == crfsnp_model_item_max_num(np_model, &item_max_num))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create: invalid np_model %u\n", np_model);
        return (NULL_PTR);
    }    

    np_fname = crfsnp_fname_gen(np_root_dir, np_id);
    if(NULL_PTR == np_fname)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create: generate np_fname of np %u, root_dir %s failed\n", np_id, np_root_dir);
        return (NULL_PTR);
    }
    
    if(EC_TRUE == c_file_access(np_fname, F_OK))/*exist*/
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create: np %u exist already\n", np_id);
        safe_free(np_fname, LOC_CRFSNP_0037);
        return (NULL_PTR);
    }

    fd = c_file_open(np_fname, O_RDWR | O_CREAT, 0666);
    if(ERR_FD == fd)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create: cannot create np %s\n", np_fname);
        safe_free(np_fname, LOC_CRFSNP_0038);
        return (NULL_PTR);
    }

    if(EC_FALSE == c_file_truncate(fd, fsize))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create: truncate np %s to size %u failed\n", np_fname, fsize);
        safe_free(np_fname, LOC_CRFSNP_0039);
        c_file_close(fd);
        return (NULL_PTR);
    }

    crfsnp_header = __crfsnp_header_create(np_id, fsize, fd, np_model);
    if(NULL_PTR == crfsnp_header)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create: open crfsnp file %s failed\n", np_fname);
        safe_free(np_fname, LOC_CRFSNP_0040);
        c_file_close(fd);
        return (NULL_PTR);
    }
    CRFSNP_HEADER_1ST_CHASH_ALGO_ID(crfsnp_header) = hash_1st_algo_id;
    CRFSNP_HEADER_2ND_CHASH_ALGO_ID(crfsnp_header) = hash_2nd_algo_id;

    crfsnp = crfsnp_new();
    if(NULL_PTR == crfsnp)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_create: new crfsnp %u failed\n", np_id);
        safe_free(np_fname, LOC_CRFSNP_0041);
        c_file_close(fd);
        __crfsnp_header_close(crfsnp_header, np_id, fsize);
        return (NULL_PTR);
    }
    CRFSNP_HDR(crfsnp) = crfsnp_header;

    CRFSNP_1ST_CHASH_ALGO(crfsnp) = chash_algo_fetch(CRFSNP_HEADER_1ST_CHASH_ALGO_ID(crfsnp_header));
    CRFSNP_2ND_CHASH_ALGO(crfsnp) = chash_algo_fetch(CRFSNP_HEADER_2ND_CHASH_ALGO_ID(crfsnp_header));    

    CRFSNP_FD(crfsnp)    = fd;
    CRFSNP_FSIZE(crfsnp) = fsize;
    CRFSNP_FNAME(crfsnp) = (uint8_t *)np_fname;

    ASSERT(np_id == CRFSNP_HEADER_NP_ID(crfsnp_header));    

    /*create root item*/
    crfsnp_create_root_item(crfsnp);

    sys_log(LOGSTDOUT, "[DEBUG] crfsnp_create: create np %u done\n", np_id);

    return (crfsnp);
}

EC_BOOL crfsnp_show_item_full_path(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos)
{
    uint8_t  path[1024];
    uint32_t path_len;
    uint32_t path_max_len;
    CSTACK  *cstack;

    CRFSNP_ITEM  *crfsnp_item;
    uint32_t      cur_node_pos;

    if(CRFSNPRB_ERR_POS == node_pos)
    {
        return (EC_FALSE);
    }

    cstack = cstack_new(MM_IGNORE, LOC_CRFSNP_0042);
    cur_node_pos = node_pos;

    while(CRFSNPRB_ERR_POS != cur_node_pos)
    {
        cstack_push(cstack, (void *)cur_node_pos);
        crfsnp_item = crfsnp_fetch(crfsnp, cur_node_pos);
        //sys_log(LOGSTDOUT, "[DEBUG] crfsnp_show_item_full_path: node_pos %u -> parent %u\n", cur_node_pos, CRFSNP_ITEM_PARENT_POS(crfsnp_item));
        //sys_log(LOGSTDOUT, "[DEBUG] crfsnp_show_item_full_path: node_pos %x -> parent %x\n", cur_node_pos, CRFSNP_ITEM_PARENT_POS(crfsnp_item));
        cur_node_pos  = CRFSNP_ITEM_PARENT_POS(crfsnp_item);
    }

    path[ 0 ] = '\0';
    path_len = 0;
    path_max_len = sizeof(path)/sizeof(path[0]);

    while(EC_FALSE == cstack_is_empty(cstack))
    {
        cur_node_pos = (uint32_t)cstack_pop(cstack);
        crfsnp_item  = crfsnp_fetch(crfsnp, cur_node_pos);
        
        if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            path_len += snprintf((char *)path + path_len, path_max_len - path_len, "%.*s/", CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        }
        else if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            path_len += snprintf((char *)path + path_len, path_max_len - path_len, "%.*s", CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        }
        else if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
        {
            path_len += snprintf((char *)path + path_len, path_max_len - path_len, "%.*s", CRFSNP_ITEM_KLEN(crfsnp_item), (char *)CRFSNP_ITEM_KEY(crfsnp_item));
        }        
        else
        {
            sys_log(log, "error:crfsnp_show_item_full_path: invalid dir flag %u at node_pos\n", CRFSNP_ITEM_DFLG(crfsnp_item), cur_node_pos);
        }
        if(path_len >= path_max_len)
        {
            sys_log(log, "error:crfsnp_show_item_full_path: path overflow\n");
        }
        //sys_print(log, "%s [klen %u, node_pos %u]\n", (char *)path, CRFSNP_ITEM_KLEN(crfsnp_item), node_pos);
    }

    cstack_free(cstack, LOC_CRFSNP_0043);

    if(path_len >= path_max_len)
    {
        path[path_max_len - 1] = '\0';
        path[path_max_len - 2] = '.';
        path[path_max_len - 3] = '.';
        path[path_max_len - 4] = '.';
    }
    else
    {
        path[path_len] = '\0';
    }

    crfsnp_item = crfsnp_fetch(crfsnp, cur_node_pos);
    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(log, "dir : %s\n", path);
    }
    else if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(log, "file: %s\n", path);
    }
    else if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(log, "bfile: %s\n", path);
    }    
    else
    {
        sys_log(log, "err: %s\n", path);
    }

    return (EC_TRUE);
}

static EC_BOOL __crfsnp_show_item(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos)
{
    const CRFSNPRB_POOL *pool;
    const CRFSNP_ITEM   *crfsnp_item;
    const CRFSNPRB_NODE *node;

    if(CRFSNPRB_ERR_POS == node_pos)
    {
        return (EC_TRUE);
    }

    pool = CRFSNP_ITEMS_POOL(crfsnp);

    node  = CRFSNPRB_POOL_NODE(pool, node_pos);    

    /*itself*/
    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    if(CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:__crfsnp_show_item: item not used\n");
        return (EC_FALSE);
    }
    
    if(CRFSNP_ITEM_FILE_IS_DIR != CRFSNP_ITEM_DFLG(crfsnp_item) 
    && CRFSNP_ITEM_FILE_IS_REG != CRFSNP_ITEM_DFLG(crfsnp_item)
    && CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(log, "error:__crfsnp_show_item: invalid dir flag %u\n", CRFSNP_ITEM_DFLG(crfsnp_item));
        return (EC_FALSE);
    }
    
    crfsnp_show_item_full_path(log, crfsnp, node_pos);

    /*do not show subdirectories*/
    return (EC_TRUE);
}

EC_BOOL crfsnp_show_item(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos)
{
    const CRFSNPRB_POOL *pool;
    const CRFSNP_ITEM   *crfsnp_item;
    const CRFSNPRB_NODE *node;

    if(CRFSNPRB_ERR_POS == node_pos)
    {
        return (EC_TRUE);
    }

    pool = CRFSNP_ITEMS_POOL(crfsnp);

    node  = CRFSNPRB_POOL_NODE(pool, node_pos);    

    /*itself*/
    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    if(CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_show_item: item not used\n");
        return (EC_FALSE);
    }
    
    if(CRFSNP_ITEM_FILE_IS_DIR != CRFSNP_ITEM_DFLG(crfsnp_item) 
    && CRFSNP_ITEM_FILE_IS_REG != CRFSNP_ITEM_DFLG(crfsnp_item)
    && CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(log, "error:crfsnp_show_item: invalid dir flag %u\n", CRFSNP_ITEM_DFLG(crfsnp_item));
        return (EC_FALSE);
    }
    
    crfsnp_show_item_full_path(log, crfsnp, node_pos);
    
    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        crfsnp_show_dir(log, crfsnp, crfsnp_item);
    }
    return (EC_TRUE);
}

EC_BOOL crfsnp_show_dir(LOG *log, const CRFSNP *crfsnp, const CRFSNP_ITEM  *crfsnp_item)
{
    CRFSNP_DNODE *crfsnp_dnode;
    uint32_t crfsnp_bucket_pos;

    crfsnp_dnode = (CRFSNP_DNODE *)CRFSNP_ITEM_DNODE(crfsnp_item);
    for(crfsnp_bucket_pos = 0; crfsnp_bucket_pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; crfsnp_bucket_pos ++)
    {
        uint32_t root_pos;

        root_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, crfsnp_bucket_pos);
        crfsnp_show_item_full_path(log, crfsnp, root_pos);
    }

    return (EC_TRUE);
}


EC_BOOL crfsnp_show_dir_depth(LOG *log, const CRFSNP *crfsnp, const CRFSNP_ITEM  *crfsnp_item)
{
    CRFSNP_DNODE *crfsnp_dnode;
    uint32_t crfsnp_bucket_pos;

    crfsnp_dnode = (CRFSNP_DNODE *)CRFSNP_ITEM_DNODE(crfsnp_item);
    for(crfsnp_bucket_pos = 0; crfsnp_bucket_pos < CRFSNP_DNODE_DIR_BUCKET_MAX_NUM; crfsnp_bucket_pos ++)
    {
        uint32_t root_pos;

        root_pos = CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, crfsnp_bucket_pos);
        crfsnp_show_item_depth(log, crfsnp, root_pos);
    }

    return (EC_TRUE);
}

EC_BOOL crfsnp_show_item_depth(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos)
{
    const CRFSNPRB_POOL *pool;
    const CRFSNP_ITEM   *crfsnp_item;
    const CRFSNPRB_NODE *node;

    if(CRFSNPRB_ERR_POS == node_pos)
    {
        return (EC_TRUE);
    }

    pool = CRFSNP_ITEMS_POOL(crfsnp);

    node  = CRFSNPRB_POOL_NODE(pool, node_pos);    

    /*left subtree*/
    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_LEFT_POS(node))
    {
        crfsnp_show_item_depth(log, crfsnp, CRFSNPRB_NODE_LEFT_POS(node));
    }

    /*itself*/
    crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
    if(CRFSNP_ITEM_STAT_IS_NOT_USED == CRFSNP_ITEM_STAT(crfsnp_item))
    {
        sys_log(LOGSTDOUT, "error:crfsnp_show_item_depth: item not used\n");
        return (EC_FALSE);
    }
    
    if(CRFSNP_ITEM_FILE_IS_DIR != CRFSNP_ITEM_DFLG(crfsnp_item) 
    && CRFSNP_ITEM_FILE_IS_REG != CRFSNP_ITEM_DFLG(crfsnp_item)
    && CRFSNP_ITEM_FILE_IS_BIG != CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        sys_log(log, "error:crfsnp_show_item_depth: invalid dir flag %u\n", CRFSNP_ITEM_DFLG(crfsnp_item));
        return (EC_FALSE);
    }
    
    crfsnp_show_item_full_path(log, crfsnp, node_pos);
    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DFLG(crfsnp_item))
    {
        crfsnp_show_dir_depth(log, crfsnp, crfsnp_item);
    }

    /*right subtree*/
    if(CRFSNPRB_ERR_POS != CRFSNPRB_NODE_RIGHT_POS(node))
    {
        crfsnp_show_item_depth(log, crfsnp, CRFSNPRB_NODE_RIGHT_POS(node));
    }

    return (EC_TRUE);
}

EC_BOOL crfsnp_show_path_depth(LOG *log, CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path)
{
    uint32_t node_pos;

    node_pos = crfsnp_search(crfsnp, path_len, path, CRFSNP_ITEM_FILE_IS_ANY);
    if(CRFSNPRB_ERR_POS == node_pos)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_show_path_depth: not found path %.*s\n", path_len, (char *)path);
        return (EC_FALSE);
    }

    return crfsnp_show_item_depth(log, crfsnp, node_pos);
}

EC_BOOL crfsnp_show_path(LOG *log, CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path)
{
    uint32_t node_pos;

    node_pos = crfsnp_search(crfsnp, path_len, path, CRFSNP_ITEM_FILE_IS_ANY);
    if(CRFSNPRB_ERR_POS == node_pos)
    {
        sys_log(LOGSTDOUT, "error:crfsnp_show_path: not found path %.*s\n", path_len, (char *)path);
        return (EC_FALSE);
    }

    return crfsnp_show_item(log, crfsnp, node_pos);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

