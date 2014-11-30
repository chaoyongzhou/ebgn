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
#include "cmutex.h"
#include "cstring.h"
#include "cmisc.h"

#include "cbloom.h"

#include "chashalgo.h"
#include "cdsk.h"
#include "cstack.h"
#include "cmd5.h"

#include "cpgrb.h"
#include "chfsnprb.h"
#include "chfsnp.h"

static CHFSNP_CFG g_chfsnp_cfg_tbl[] = {
    {"CHFSNP_004K_MODEL", CHFSNP_004K_CFG_FILE_SIZE,  CHFSNP_004K_CFG_ITEM_MAX_NUM },
    {"CHFSNP_064K_MODEL", CHFSNP_064K_CFG_FILE_SIZE,  CHFSNP_064K_CFG_ITEM_MAX_NUM },
    {"CHFSNP_001M_MODEL", CHFSNP_001M_CFG_FILE_SIZE,  CHFSNP_001M_CFG_ITEM_MAX_NUM },
    {"CHFSNP_002M_MODEL", CHFSNP_002M_CFG_FILE_SIZE,  CHFSNP_002M_CFG_ITEM_MAX_NUM },
    {"CHFSNP_128M_MODEL", CHFSNP_128M_CFG_FILE_SIZE,  CHFSNP_128M_CFG_ITEM_MAX_NUM },
    {"CHFSNP_256M_MODEL", CHFSNP_256M_CFG_FILE_SIZE,  CHFSNP_256M_CFG_ITEM_MAX_NUM },
    {"CHFSNP_512M_MODEL", CHFSNP_512M_CFG_FILE_SIZE,  CHFSNP_512M_CFG_ITEM_MAX_NUM },
    {"CHFSNP_001G_MODEL", CHFSNP_001G_CFG_FILE_SIZE,  CHFSNP_001G_CFG_ITEM_MAX_NUM },
    {"CHFSNP_002G_MODEL", CHFSNP_002G_CFG_FILE_SIZE,  CHFSNP_002G_CFG_ITEM_MAX_NUM },
#if (64 == WORDSIZE)
    {"CHFSNP_004G_MODEL", CHFSNP_004G_CFG_FILE_SIZE,  CHFSNP_004G_CFG_ITEM_MAX_NUM },
#endif/*(64 == WORDSIZE)*/
};

static uint8_t g_chfsnp_cfg_tbl_len = (uint8_t)(sizeof(g_chfsnp_cfg_tbl)/sizeof(g_chfsnp_cfg_tbl[0]));

EC_BOOL chfsnp_model_str(const uint8_t chfsnp_model, char **mod_str)
{
    CHFSNP_CFG *chfsnp_cfg;
    if(chfsnp_model >= g_chfsnp_cfg_tbl_len)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_model_str: invalid chfsnp mode %u\n", chfsnp_model);
        return (EC_FALSE);
    }
    chfsnp_cfg = &(g_chfsnp_cfg_tbl[ chfsnp_model ]);
    (*mod_str) = CHFSNP_CFG_MOD_STR(chfsnp_cfg);
    return (EC_TRUE);
}

uint32_t chfsnp_model_get(const char *mod_str)
{
    uint8_t chfsnp_model;

    for(chfsnp_model = 0; chfsnp_model < g_chfsnp_cfg_tbl_len; chfsnp_model ++)
    {
        CHFSNP_CFG *chfsnp_cfg;
        chfsnp_cfg = &(g_chfsnp_cfg_tbl[ chfsnp_model ]);

        if(0 == strcasecmp(CHFSNP_CFG_MOD_STR(chfsnp_cfg), mod_str))
        {
            return (chfsnp_model);
        }
    }
    return (CHFSNP_ERR_MODEL);
}

EC_BOOL chfsnp_model_file_size(const uint8_t chfsnp_model, UINT32 *file_size)
{
    CHFSNP_CFG *chfsnp_cfg;
    if(chfsnp_model >= g_chfsnp_cfg_tbl_len)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_model_file_size: invalid chfsnp mode %u\n", chfsnp_model);
        return (EC_FALSE);
    }
    chfsnp_cfg = &(g_chfsnp_cfg_tbl[ chfsnp_model ]);
    (*file_size) = CHFSNP_CFG_FILE_SIZE(chfsnp_cfg);
    return (EC_TRUE);
}

EC_BOOL chfsnp_model_item_max_num(const uint8_t chfsnp_model, uint32_t *item_max_num)
{
    CHFSNP_CFG *chfsnp_cfg;
    if(chfsnp_model >= g_chfsnp_cfg_tbl_len)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_model_item_max_num: invalid chfsnp mode %u\n", chfsnp_model);
        return (EC_FALSE);
    }
    chfsnp_cfg = &(g_chfsnp_cfg_tbl[ chfsnp_model ]);
    (*item_max_num) = CHFSNP_CFG_ITEM_MAX_NUM(chfsnp_cfg);
    return (EC_TRUE);
}

static char *chfsnp_fname_gen(const char *root_dir, const uint32_t np_id)
{
    char *fname;
    uint32_t len;

    if(NULL_PTR == root_dir)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_fname_gen: root_dir is null\n");
        return (NULL_PTR);
    }

    len = strlen(root_dir) + strlen("/hfsnp_0000.dat") + 1;

    fname = safe_malloc(len, LOC_CHFSNP_0001);
    if(NULL_PTR == fname)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_fname_gen: malloc %u bytes failed\n", len);
        return (NULL_PTR);
    }
    snprintf(fname, len, "%s/hfsnp_%04X.dat", root_dir, np_id);
    return (fname);
}

static uint32_t chfsnp_path_seg_len(const uint8_t *full_path, const uint32_t full_path_len, const uint8_t *path_seg_beg)
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

static EC_BOOL __chfsnp_path_md5(const uint32_t path_len, const uint8_t *path, uint8_t digest[ CMD5_DIGEST_LEN ])
{
	return cmd5_sum(path_len, path, digest);
}

EC_BOOL chfsnp_inode_init(CHFSNP_INODE *chfsnp_inode)
{
    CHFSNP_INODE_DISK_NO(chfsnp_inode)  = CPGRB_ERR_POS;
    CHFSNP_INODE_BLOCK_NO(chfsnp_inode) = CPGRB_ERR_POS;
    CHFSNP_INODE_PAGE_NO(chfsnp_inode)  = CPGRB_ERR_POS;
    return (EC_TRUE);
}

EC_BOOL chfsnp_inode_clean(CHFSNP_INODE *chfsnp_inode)
{
    CHFSNP_INODE_DISK_NO(chfsnp_inode)  = CPGRB_ERR_POS;
    CHFSNP_INODE_BLOCK_NO(chfsnp_inode) = CPGRB_ERR_POS;
    CHFSNP_INODE_PAGE_NO(chfsnp_inode)  = CPGRB_ERR_POS;
    return (EC_TRUE);
}

EC_BOOL chfsnp_inode_clone(const CHFSNP_INODE *chfsnp_inode_src, CHFSNP_INODE *chfsnp_inode_des)
{
    CHFSNP_INODE_DISK_NO(chfsnp_inode_des)  = CHFSNP_INODE_DISK_NO(chfsnp_inode_src);
    CHFSNP_INODE_BLOCK_NO(chfsnp_inode_des) = CHFSNP_INODE_BLOCK_NO(chfsnp_inode_src);
    CHFSNP_INODE_PAGE_NO(chfsnp_inode_des)  = CHFSNP_INODE_PAGE_NO(chfsnp_inode_src);

    return (EC_TRUE);
}

void chfsnp_inode_print(LOG *log, const CHFSNP_INODE *chfsnp_inode)
{
    sys_print(log, "(disk %u, block %u, page %u)\n",
                    CHFSNP_INODE_DISK_NO(chfsnp_inode), 
                    CHFSNP_INODE_BLOCK_NO(chfsnp_inode),
                    CHFSNP_INODE_PAGE_NO(chfsnp_inode)
                    );
    return;
}

void chfsnp_inode_log_no_lock(LOG *log, const CHFSNP_INODE *chfsnp_inode)
{
    sys_print_no_lock(log, "(disk %u, block %u, page %u)\n",
                    CHFSNP_INODE_DISK_NO(chfsnp_inode), 
                    CHFSNP_INODE_BLOCK_NO(chfsnp_inode),
                    CHFSNP_INODE_PAGE_NO(chfsnp_inode)
                    );
    return;
}

CHFSNP_FNODE *chfsnp_fnode_new()
{
    CHFSNP_FNODE *chfsnp_fnode;
    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CHFSNP_FNODE, &chfsnp_fnode, LOC_CHFSNP_0002);
    if(NULL_PTR != chfsnp_fnode)
    {
        chfsnp_fnode_init(chfsnp_fnode);
    }
    return (chfsnp_fnode);

}

CHFSNP_FNODE *chfsnp_fnode_make(const CHFSNP_FNODE *chfsnp_fnode_src)
{
    CHFSNP_FNODE *chfsnp_fnode_des;
    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CHFSNP_FNODE, &chfsnp_fnode_des, LOC_CHFSNP_0003);
    if(NULL_PTR != chfsnp_fnode_des)
    {
        chfsnp_fnode_clone(chfsnp_fnode_src, chfsnp_fnode_des);
    }
    return (chfsnp_fnode_des);
}

EC_BOOL chfsnp_fnode_init(CHFSNP_FNODE *chfsnp_fnode)
{
    uint32_t pos;

    CHFSNP_FNODE_FILESZ(chfsnp_fnode) = 0;
    CHFSNP_FNODE_REPNUM(chfsnp_fnode) = 0;

    for(pos = 0; pos < CHFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        chfsnp_inode_init(CHFSNP_FNODE_INODE(chfsnp_fnode, pos));
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_fnode_clean(CHFSNP_FNODE *chfsnp_fnode)
{
    uint32_t pos;

    CHFSNP_FNODE_FILESZ(chfsnp_fnode) = 0;
    CHFSNP_FNODE_REPNUM(chfsnp_fnode) = 0;

    for(pos = 0; pos < CHFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        chfsnp_inode_clean(CHFSNP_FNODE_INODE(chfsnp_fnode, pos));
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_fnode_free(CHFSNP_FNODE *chfsnp_fnode)
{
    if(NULL_PTR != chfsnp_fnode)
    {
        chfsnp_fnode_clean(chfsnp_fnode);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CHFSNP_FNODE, chfsnp_fnode, LOC_CHFSNP_0004);
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_fnode_init_0(const UINT32 md_id, CHFSNP_FNODE *chfsnp_fnode)
{
    return chfsnp_fnode_init(chfsnp_fnode);
}

EC_BOOL chfsnp_fnode_clean_0(const UINT32 md_id, CHFSNP_FNODE *chfsnp_fnode)
{
    return chfsnp_fnode_clean(chfsnp_fnode);
}

EC_BOOL chfsnp_fnode_free_0(const UINT32 md_id, CHFSNP_FNODE *chfsnp_fnode)
{
    return chfsnp_fnode_free(chfsnp_fnode);
}

EC_BOOL chfsnp_fnode_clone(const CHFSNP_FNODE *chfsnp_fnode_src, CHFSNP_FNODE *chfsnp_fnode_des)
{
    uint32_t pos;

    CHFSNP_FNODE_FILESZ(chfsnp_fnode_des) = CHFSNP_FNODE_FILESZ(chfsnp_fnode_src);
    CHFSNP_FNODE_REPNUM(chfsnp_fnode_des) = CHFSNP_FNODE_REPNUM(chfsnp_fnode_src);

    for(pos = 0; pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode_src) && pos < CHFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        chfsnp_inode_clone(CHFSNP_FNODE_INODE(chfsnp_fnode_src, pos), CHFSNP_FNODE_INODE(chfsnp_fnode_des, pos));
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_fnode_check_inode_exist(const CHFSNP_INODE *inode, const CHFSNP_FNODE *chfsnp_fnode)
{
    uint32_t replica_pos;

    for(replica_pos = 0; replica_pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode); replica_pos ++)
    {
        if(
            CHFSNP_INODE_DISK_NO(inode)  == CHFSNP_FNODE_INODE_DISK_NO(chfsnp_fnode, replica_pos)
         && CHFSNP_INODE_BLOCK_NO(inode) == CHFSNP_FNODE_INODE_BLOCK_NO(chfsnp_fnode, replica_pos)
         && CHFSNP_INODE_PAGE_NO(inode)  == CHFSNP_FNODE_INODE_PAGE_NO(chfsnp_fnode, replica_pos)
        )
        {
            return (EC_TRUE);
        }
    }
    return (EC_FALSE);
}

EC_BOOL chfsnp_fnode_cmp(const CHFSNP_FNODE *chfsnp_fnode_1st, const CHFSNP_FNODE *chfsnp_fnode_2nd)
{
    uint32_t replica_pos;

    if(NULL_PTR == chfsnp_fnode_1st && NULL_PTR == chfsnp_fnode_2nd)
    {
        return (EC_TRUE);
    }

    if(NULL_PTR == chfsnp_fnode_1st || NULL_PTR == chfsnp_fnode_2nd)
    {
        return (EC_FALSE);
    }

    if(CHFSNP_FNODE_REPNUM(chfsnp_fnode_1st) != CHFSNP_FNODE_REPNUM(chfsnp_fnode_2nd))
    {
        return (EC_FALSE);
    }

    if(CHFSNP_FNODE_FILESZ(chfsnp_fnode_1st) != CHFSNP_FNODE_FILESZ(chfsnp_fnode_2nd))
    {
        return (EC_FALSE);
    }
    
    for(replica_pos = 0; replica_pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode_1st); replica_pos ++)
    {
        if(EC_FALSE == chfsnp_fnode_check_inode_exist(CHFSNP_FNODE_INODE(chfsnp_fnode_1st, replica_pos), chfsnp_fnode_2nd))
        {
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

EC_BOOL chfsnp_fnode_import(const CHFSNP_FNODE *chfsnp_fnode_src, CHFSNP_FNODE *chfsnp_fnode_des)
{
    uint32_t src_pos;
    uint32_t des_pos;

    for(src_pos = 0, des_pos = 0; src_pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode_src) && src_pos < CHFSNP_FILE_REPLICA_MAX_NUM; src_pos ++)
    {
        CHFSNP_INODE *chfsnp_inode_src;

        chfsnp_inode_src = (CHFSNP_INODE *)CHFSNP_FNODE_INODE(chfsnp_fnode_src, src_pos);
        if(CPGRB_ERR_POS != CHFSNP_INODE_DISK_NO(chfsnp_inode_src) 
        && CPGRB_ERR_POS != CHFSNP_INODE_BLOCK_NO(chfsnp_inode_src)
        && CPGRB_ERR_POS != CHFSNP_INODE_PAGE_NO(chfsnp_inode_src)
        )
        {
            CHFSNP_INODE *chfsnp_inode_des;

            chfsnp_inode_des = CHFSNP_FNODE_INODE(chfsnp_fnode_des, des_pos);
            if(chfsnp_inode_src != chfsnp_inode_des)
            {
                chfsnp_inode_clone(chfsnp_inode_src, chfsnp_inode_des);
            }

            des_pos ++;
        }
    }

    CHFSNP_FNODE_FILESZ(chfsnp_fnode_des) = CHFSNP_FNODE_FILESZ(chfsnp_fnode_src);
    CHFSNP_FNODE_REPNUM(chfsnp_fnode_des) = des_pos;
    return (EC_TRUE);
}

uint32_t chfsnp_fnode_count_replica(const CHFSNP_FNODE *chfsnp_fnode)
{
    uint32_t pos;
    uint32_t count;

    for(pos = 0, count = 0; pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode) && pos < CHFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        CHFSNP_INODE *chfsnp_inode;

        chfsnp_inode = (CHFSNP_INODE *)CHFSNP_FNODE_INODE(chfsnp_fnode, pos);
        if(CPGRB_ERR_POS != CHFSNP_INODE_DISK_NO(chfsnp_inode) 
        && CPGRB_ERR_POS != CHFSNP_INODE_BLOCK_NO(chfsnp_inode)
        && CPGRB_ERR_POS != CHFSNP_INODE_PAGE_NO(chfsnp_inode)
        )
        {
            count ++;
        }
    }
    return (count);
}

void chfsnp_fnode_print(LOG *log, const CHFSNP_FNODE *chfsnp_fnode)
{
    uint32_t pos;

    sys_log(log, "chfsnp_fnode %p: file size %u, replica num %u\n",
                    chfsnp_fnode,
                    CHFSNP_FNODE_FILESZ(chfsnp_fnode), 
                    CHFSNP_FNODE_REPNUM(chfsnp_fnode)
                    );

    for(pos = 0; pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode) && pos < CHFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        chfsnp_inode_print(log, CHFSNP_FNODE_INODE(chfsnp_fnode, pos));
    }
    return;
}

void chfsnp_fnode_log_no_lock(LOG *log, const CHFSNP_FNODE *chfsnp_fnode)
{
    uint32_t pos;

    sys_print_no_lock(log, "size %u, replica %u",
                    CHFSNP_FNODE_FILESZ(chfsnp_fnode),
                    CHFSNP_FNODE_REPNUM(chfsnp_fnode));

    for(pos = 0; pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode) && pos < CHFSNP_FILE_REPLICA_MAX_NUM; pos ++)
    {
        chfsnp_inode_log_no_lock(log, CHFSNP_FNODE_INODE(chfsnp_fnode, pos));
    }
    sys_print_no_lock(log, "\n");

    return;
}

CHFSNP_ITEM *chfsnp_item_new()
{
    CHFSNP_ITEM *chfsnp_item;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CHFSNP_ITEM, &chfsnp_item, LOC_CHFSNP_0005);
    if(NULL_PTR != chfsnp_item)
    {
        chfsnp_item_init(chfsnp_item);
    }
    return (chfsnp_item);
}

EC_BOOL chfsnp_item_init(CHFSNP_ITEM *chfsnp_item)
{
    CHFSNP_ITEM_R_COUNT(chfsnp_item) = 0;
    CHFSNP_ITEM_K_LEN(chfsnp_item)   = 0;
    BSET(CHFSNP_ITEM_KEY(chfsnp_item), '\0', CHFSNP_KEY_MAX_SIZE);

    
    chfsnp_fnode_init(CHFSNP_ITEM_FNODE(chfsnp_item));
    
    /*note:do nothing on rb_node*/

    return (EC_TRUE);
}

EC_BOOL chfsnp_item_clean(CHFSNP_ITEM *chfsnp_item)
{
    CHFSNP_ITEM_R_COUNT(chfsnp_item) = 0;
    CHFSNP_ITEM_K_LEN(chfsnp_item)   = 0;
    BSET(CHFSNP_ITEM_KEY(chfsnp_item), '\0', CHFSNP_KEY_MAX_SIZE);
    
    chfsnp_fnode_clean(CHFSNP_ITEM_FNODE(chfsnp_item));
    
    /*note:do nothing on rb_node*/

    return (EC_TRUE);
}

EC_BOOL chfsnp_item_clone(const CHFSNP_ITEM *chfsnp_item_src, CHFSNP_ITEM *chfsnp_item_des)
{
    if(NULL_PTR == chfsnp_item_src)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_item_clone: chfsnp_item_src is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == chfsnp_item_des)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_item_clone: chfsnp_item_des is null\n");
        return (EC_FALSE);
    }

    CHFSNP_ITEM_R_COUNT(chfsnp_item_des) = CHFSNP_ITEM_R_COUNT(chfsnp_item_src);
    CHFSNP_ITEM_K_LEN(chfsnp_item_des)   = CHFSNP_ITEM_K_LEN(chfsnp_item_src);

    BCOPY(CHFSNP_ITEM_KEY(chfsnp_item_src), CHFSNP_ITEM_KEY(chfsnp_item_des), CHFSNP_ITEM_K_LEN(chfsnp_item_src));
    chfsnp_fnode_clone(CHFSNP_ITEM_FNODE(chfsnp_item_src), CHFSNP_ITEM_FNODE(chfsnp_item_des));
    
    return (EC_TRUE);
}

EC_BOOL chfsnp_item_free(CHFSNP_ITEM *chfsnp_item)
{
    if(NULL_PTR != chfsnp_item)
    {
        chfsnp_item_clean(chfsnp_item);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CHFSNP_ITEM, chfsnp_item, LOC_CHFSNP_0006);
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_item_init_0(const UINT32 md_id, CHFSNP_ITEM *chfsnp_item)
{
    return chfsnp_item_init(chfsnp_item);
}

EC_BOOL chfsnp_item_clean_0(const UINT32 md_id, CHFSNP_ITEM *chfsnp_item)
{
    return chfsnp_item_clean(chfsnp_item);
}

EC_BOOL chfsnp_item_free_0(const UINT32 md_id, CHFSNP_ITEM *chfsnp_item)
{
    return chfsnp_item_free(chfsnp_item);
}

EC_BOOL chfsnp_item_set_key(CHFSNP_ITEM *chfsnp_item, const uint32_t klen, const uint8_t *key)
{
    BCOPY(key, CHFSNP_ITEM_KEY(chfsnp_item), klen);
    CHFSNP_ITEM_K_LEN(chfsnp_item) = klen;

    return (EC_TRUE);
}

void chfsnp_item_print(LOG *log, const CHFSNP_ITEM *chfsnp_item)
{
    uint32_t pos;
    const CHFSNP_FNODE *chfsnp_fnode;
    const uint8_t *key;

    sys_print(log, "chfsnp_item %p: stat %u, r_count %u, k_len %u\n",
                    chfsnp_item,
                    CHFSNP_ITEM_STAT(chfsnp_item),
                    CHFSNP_ITEM_R_COUNT(chfsnp_item),
                    CHFSNP_ITEM_K_LEN(chfsnp_item)
                    );

    key = CHFSNP_ITEM_KEY(chfsnp_item);
    sys_log(log, "key: [len = %u] %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", 
                 CHFSNP_ITEM_K_LEN(chfsnp_item), 
                 key[ 0 ], key[ 1 ], key[ 2 ], key[ 3 ], 
                 key[ 4 ], key[ 5 ], key[ 6 ], key[ 7 ], 
                 key[ 8 ], key[ 9 ], key[ 10 ], key[ 11 ], 
                 key[ 12 ], key[ 13 ], key[ 14 ], key[ 15 ]);

    chfsnp_fnode = CHFSNP_ITEM_FNODE(chfsnp_item);
    sys_log(log, "file size %u, replica num %u\n",
                    CHFSNP_FNODE_FILESZ(chfsnp_fnode),
                    CHFSNP_FNODE_REPNUM(chfsnp_fnode)
                    );
    for(pos = 0; pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode); pos ++)
    {
        const CHFSNP_INODE *chfsnp_inode;

        chfsnp_inode = CHFSNP_FNODE_INODE(chfsnp_fnode, pos);
        chfsnp_inode_print(log, chfsnp_inode);
        //sys_print(log, "\n");
    }

    return;
}

EC_BOOL chfsnp_item_load(CHFSNP *chfsnp, uint32_t *offset, CHFSNP_ITEM *chfsnp_item)
{
    RWSIZE rsize;
    UINT32 offset_t;

    offset_t = (*offset);
    rsize = sizeof(CHFSNP_ITEM);
    if(EC_FALSE == c_file_load(CHFSNP_FD(chfsnp), &offset_t, rsize, (UINT8 *)chfsnp_item))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_item_load: load item from offset %u failed\n", *offset);
        return (EC_FALSE);
    }

    (*offset) = (uint32_t)offset_t;

    return (EC_TRUE);
}

EC_BOOL chfsnp_item_flush(CHFSNP *chfsnp, uint32_t *offset, const CHFSNP_ITEM *chfsnp_item)
{
    RWSIZE wsize;
    UINT32 offset_t;

    offset_t = (*offset);
    wsize = sizeof(CHFSNP_ITEM);
    if(EC_FALSE == c_file_flush(CHFSNP_FD(chfsnp), &offset_t, wsize, (UINT8 *)chfsnp_item))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_item_load: flush item to offset %u failed\n", *offset);
        return (EC_FALSE);
    }

    (*offset) = (uint32_t)offset_t;

    return (EC_TRUE);
}

EC_BOOL chfsnp_item_is(const CHFSNP_ITEM *chfsnp_item, const uint32_t klen, const uint8_t *key)
{
    if(klen !=  CHFSNP_ITEM_K_LEN(chfsnp_item))
    {
        return (EC_FALSE);
    }

    if(0 != strncmp((char *)key, (char *)CHFSNP_ITEM_KEY(chfsnp_item), klen))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

void chfsnp_bucket_print(LOG *log, const uint32_t *chfsnp_buckets, const uint32_t bucket_num)
{
    uint32_t pos;

    for(pos = 0; pos < bucket_num; pos ++)
    {
        sys_log(log, "bucket %u#: offset %u\n", pos, *(chfsnp_buckets + pos));
    }
    return;
}

static CHFSNP_HEADER *__chfsnp_header_open(const uint32_t np_id, const UINT32 fsize, int fd)
{
    CHFSNP_HEADER *chfsnp_header;

    chfsnp_header = (CHFSNP_HEADER *)mmap(NULL_PTR, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(MAP_FAILED == chfsnp_header)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:__chfsnp_header_open: mmap np %u with fd %d failed, errno = %d, errorstr = %s\n", 
                           np_id, fd, errno, strerror(errno));
        return (NULL_PTR);
    } 
    
    return (chfsnp_header);
}

static CHFSNP_HEADER *__chfsnp_header_create(const uint32_t np_id, const UINT32 fsize, int fd, const uint8_t np_model, const uint32_t bucket_max_num)
{
    CHFSNP_HEADER *chfsnp_header;
    uint32_t *bucket;
    uint32_t node_max_num;
    uint32_t node_sizeof;
    uint32_t bucket_pos;
    UINT32   bucket_offset;
    UINT32   expect_fsize;    

    chfsnp_model_item_max_num(np_model, &node_max_num);
    node_sizeof = sizeof(CHFSNP_ITEM);

    bucket_offset = (UINT32)(sizeof(CHFSNP_HEADER) + node_max_num * sizeof(CHFSNP_ITEM));
    expect_fsize  = (UINT32)(bucket_offset + bucket_max_num * sizeof(uint32_t));

    dbg_log(SEC_0097_CHFSNP, 9)(LOGSTDOUT, "[DEBUG] __chfsnp_header_create: fsize %lu, expect_fsize %lu, where node_max_num %u, "
                       "node_sizeof %u, sizeof(CHFSNP_HEADER) %u, sizeof(CHFSNP_ITEM) %u\n", 
                        fsize, expect_fsize, node_max_num, node_sizeof, sizeof(CHFSNP_HEADER), sizeof(CHFSNP_ITEM));


    if(expect_fsize > fsize)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:__chfsnp_header_create: fsize %lu, but expect_fsize %lu, where node_max_num %u, "
                           "node_sizeof %u, sizeof(CHFSNP_HEADER) %u, sizeof(CHFSNP_ITEM) %u\n", 
                            fsize, expect_fsize, node_max_num, node_sizeof, sizeof(CHFSNP_HEADER), sizeof(CHFSNP_ITEM));
        return (NULL_PTR);
    }

    chfsnp_header = (CHFSNP_HEADER *)mmap(NULL_PTR, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(MAP_FAILED == chfsnp_header)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:__chfsnp_header_open: mmap np %u with fd %d failed, errno = %d, errorstr = %s\n", 
                           np_id, fd, errno, strerror(errno));
        return (NULL_PTR);
    }     

    CHFSNP_HEADER_NP_ID(chfsnp_header)  = np_id;
    CHFSNP_HEADER_MODEL(chfsnp_header)  = np_model;
    
    dbg_log(SEC_0097_CHFSNP, 9)(LOGSTDOUT, "[DEBUG] __chfsnp_header_create: fsize %lu, node_max_num %u, node_sizeof %u\n", fsize, node_max_num, node_sizeof);
    chfsnprb_pool_init(CHFSNP_HEADER_ITEMS_POOL(chfsnp_header), node_max_num, node_sizeof);

    CHFSNP_HEADER_BUCKET_OFFSET(chfsnp_header)  = bucket_offset;
    CHFSNP_HEADER_BUCKET_MAX_NUM(chfsnp_header) = bucket_max_num;

    bucket = (uint32_t *)(((uint8_t *)chfsnp_header) + bucket_offset);
    dbg_log(SEC_0097_CHFSNP, 9)(LOGSTDOUT, "[DEBUG] __chfsnp_header_create: bucket %p\n", bucket);
    for(bucket_pos = 0; bucket_pos < bucket_max_num; bucket_pos ++)
    {
        *(bucket + bucket_pos) = CHFSNPRB_ERR_POS;
    }
    
    return (chfsnp_header);
}

static CHFSNP_HEADER * __chfsnp_header_sync(CHFSNP_HEADER *chfsnp_header, const uint32_t np_id, const UINT32 fsize)
{   
    if(NULL_PTR != chfsnp_header)
    {   
        if(0 != msync(chfsnp_header, fsize, MS_SYNC))
        {
            dbg_log(SEC_0097_CHFSNP, 1)(LOGSTDOUT, "warn:__chfsnp_header_sync: sync chfsnp_hdr of np %u with size %u failed\n", 
                               np_id, fsize);
        }
    }    
    return (chfsnp_header);
}

static CHFSNP_HEADER *__chfsnp_header_close(CHFSNP_HEADER *chfsnp_header, const uint32_t np_id, const UINT32 fsize)
{   
    if(NULL_PTR != chfsnp_header)
    {   
        if(0 != msync(chfsnp_header, fsize, MS_SYNC))
        {
            dbg_log(SEC_0097_CHFSNP, 1)(LOGSTDOUT, "warn:__chfsnp_header_close: sync chfsnp_hdr of np %u with size %u failed\n", 
                               np_id, fsize);
        }
        
        if(0 != munmap(chfsnp_header, fsize))
        {
            dbg_log(SEC_0097_CHFSNP, 1)(LOGSTDOUT, "warn:__chfsnp_header_close: munmap chfsnp of np %u with size %u failed\n", 
                               np_id, fsize);
        } 
    }
    
    /*chfsnp_header cannot be accessed again*/
    return (NULL_PTR);
}


EC_BOOL chfsnp_header_init(CHFSNP_HEADER *chfsnp_header, const uint32_t np_id, const uint8_t model, const uint8_t first_chash_algo_id, const uint8_t second_chash_algo_id, const uint32_t bucket_max_num)
{
    CHFSNP_HEADER_NP_ID(chfsnp_header)         = np_id;
    CHFSNP_HEADER_MODEL(chfsnp_header)         = model;
    
    CHFSNP_HEADER_1ST_CHASH_ALGO_ID(chfsnp_header)  = first_chash_algo_id;
    CHFSNP_HEADER_2ND_CHASH_ALGO_ID(chfsnp_header)  = second_chash_algo_id;

    CHFSNP_HEADER_BUCKET_MAX_NUM(chfsnp_header)     = bucket_max_num;

    /*do nothing on CHFSNPRB_POOL pool*/

    return (EC_TRUE);
}

EC_BOOL chfsnp_header_clean(CHFSNP_HEADER *chfsnp_header)
{
    CHFSNP_HEADER_NP_ID(chfsnp_header)              = CHFSNP_ERR_ID;
    CHFSNP_HEADER_MODEL(chfsnp_header)              = CHFSNP_ERR_MODEL;
    
    CHFSNP_HEADER_1ST_CHASH_ALGO_ID(chfsnp_header)  = CHASH_ERR_ALGO_ID;
    CHFSNP_HEADER_2ND_CHASH_ALGO_ID(chfsnp_header)  = CHASH_ERR_ALGO_ID;

    CHFSNP_HEADER_BUCKET_MAX_NUM(chfsnp_header)     = 0;

    /*do nothing on CHFSNPRB_POOL pool*/

    return (EC_TRUE);
}

EC_BOOL chfsnp_header_create(CHFSNP_HEADER *chfsnp_header, const uint8_t chfsnp_model, const uint32_t np_id, const uint8_t model, const uint8_t first_chash_algo_id, const uint8_t second_chash_algo_id, const uint32_t bucket_max_num)
{
    return chfsnp_header_init(chfsnp_header,
                               np_id,
                               model,
                               first_chash_algo_id,
                               second_chash_algo_id,
                               bucket_max_num);
}

CHFSNP *chfsnp_new()
{
    CHFSNP *chfsnp;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CHFSNP, &chfsnp, LOC_CHFSNP_0007);
    if(NULL_PTR != chfsnp)
    {
        chfsnp_init(chfsnp);
    }
    return (chfsnp);
}

EC_BOOL chfsnp_init(CHFSNP *chfsnp)
{    
    CHFSNP_FD(chfsnp)         = ERR_FD;
    CHFSNP_FSIZE(chfsnp)      = 0;
    CHFSNP_FNAME(chfsnp)      = NULL_PTR;
    CHFSNP_STATE(chfsnp)      = CHFSNP_STATE_NOT_DIRTY;
    CHFSNP_READER_NUM(chfsnp) = 0;
    CHFSNP_HDR(chfsnp)        = NULL_PTR;
    CHFSNP_BUCKET_ADDR(chfsnp)= NULL_PTR;

    CHFSNP_INIT_LOCK(chfsnp, LOC_CHFSNP_0008);

    CHFSNP_1ST_CHASH_ALGO(chfsnp) = NULL_PTR;
    CHFSNP_2ND_CHASH_ALGO(chfsnp) = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL chfsnp_clean(CHFSNP *chfsnp)
{
    if(NULL_PTR != CHFSNP_HDR(chfsnp))
    {
        __chfsnp_header_close(CHFSNP_HDR(chfsnp), CHFSNP_ID(chfsnp), CHFSNP_FSIZE(chfsnp));
        CHFSNP_HDR(chfsnp) = NULL_PTR;
    }
    
    if(ERR_FD != CHFSNP_FD(chfsnp))
    {
        c_file_close(CHFSNP_FD(chfsnp));
        CHFSNP_FD(chfsnp) = ERR_FD;
    }

    CHFSNP_FSIZE(chfsnp) = 0;

    if(NULL_PTR != CHFSNP_FNAME(chfsnp))
    {
        safe_free(CHFSNP_FNAME(chfsnp), LOC_CHFSNP_0009);
        CHFSNP_FNAME(chfsnp) = NULL_PTR;
    }

    CHFSNP_STATE(chfsnp)      = CHFSNP_STATE_NOT_DIRTY;
    CHFSNP_READER_NUM(chfsnp) = 0;

    CHFSNP_HDR(chfsnp) = NULL_PTR;
    CHFSNP_BUCKET_ADDR(chfsnp)= NULL_PTR;

    CHFSNP_CLEAN_LOCK(chfsnp, LOC_CHFSNP_0010);

    CHFSNP_1ST_CHASH_ALGO(chfsnp) = NULL_PTR;
    CHFSNP_2ND_CHASH_ALGO(chfsnp) = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL chfsnp_free(CHFSNP *chfsnp)
{
    if(NULL_PTR != chfsnp)
    {
        chfsnp_clean(chfsnp);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CHFSNP, chfsnp, LOC_CHFSNP_0011);
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_is_full(const CHFSNP *chfsnp)
{
    CHFSNPRB_POOL *pool;

    pool = CHFSNP_ITEMS_POOL(chfsnp);
    return chfsnprb_pool_is_full(pool);
}

EC_BOOL chfsnp_is_empty(const CHFSNP *chfsnp)
{
    CHFSNPRB_POOL *pool;

    pool = CHFSNP_ITEMS_POOL(chfsnp);
    return chfsnprb_pool_is_empty(pool);
}

void chfsnp_header_print(LOG *log, const CHFSNP *chfsnp)
{
    const CHFSNP_HEADER *chfsnp_header;

    chfsnp_header = CHFSNP_HDR(chfsnp);

    sys_log(log, "np %u, model %u, item max num %u, item used num %u, bucket max num %u, bucket offset %u, 1st hash algo %u, 2nd hash algo %u\n",
                CHFSNP_HEADER_NP_ID(chfsnp_header),
                CHFSNP_HEADER_MODEL(chfsnp_header),
                CHFSNP_HEADER_ITEMS_MAX_NUM(chfsnp_header) ,
                CHFSNP_HEADER_ITEMS_USED_NUM(chfsnp_header) ,
                CHFSNP_HEADER_BUCKET_MAX_NUM(chfsnp_header),
                CHFSNP_HEADER_BUCKET_OFFSET(chfsnp_header),
                CHFSNP_HEADER_1ST_CHASH_ALGO_ID(chfsnp_header),
                CHFSNP_HEADER_2ND_CHASH_ALGO_ID(chfsnp_header)
        );

    chfsnprb_pool_print(log, CHFSNP_HEADER_ITEMS_POOL(chfsnp_header));

    chfsnp_show_all_buckets(log, chfsnp);
    return;
}

void chfsnp_print(LOG *log, const CHFSNP *chfsnp)
{
    sys_log(log, "chfsnp %p: np %u, fname %s, fsize %u, readers %u\n", 
                 chfsnp, 
                 CHFSNP_ID(chfsnp), 
                 CHFSNP_FNAME(chfsnp),
                 CHFSNP_FSIZE(chfsnp),
                 CHFSNP_READER_NUM(chfsnp)
                 );

    sys_log(log, "chfsnp %p: header: \n", chfsnp);
    chfsnp_header_print(log, chfsnp);
    return;
}

static const CHFSNP_ITEM *__chfsnp_bucket_find(const CHFSNP *chfsnp, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key)
{
    const CHFSNPRB_POOL *pool;
    uint32_t bucket_pos;
    uint32_t root_pos;
    uint32_t node_pos;

    pool       = CHFSNP_ITEMS_POOL(chfsnp);
    bucket_pos = CHFSNP_BUCKET_POS(chfsnp, first_hash);
    root_pos   = CHFSNP_BUCKET(chfsnp, bucket_pos);

    node_pos = chfsnprb_tree_search_data(pool, root_pos, second_hash, klen, key);
    if(CHFSNPRB_ERR_POS != node_pos)
    {
        const CHFSNPRB_NODE *node;
        const CHFSNP_ITEM   *item;
        
        node = CHFSNPRB_POOL_NODE(pool, node_pos);
        item = CHFSNP_RB_NODE_ITEM(node);

        return (item);
    }

    return (NULL_PTR);
}

static uint32_t __chfsnp_bucket_search(const CHFSNP *chfsnp, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key)
{
    const CHFSNPRB_POOL *pool;
    uint32_t bucket_pos;
    uint32_t root_pos;

    pool       = CHFSNP_ITEMS_POOL(chfsnp);
    bucket_pos = CHFSNP_BUCKET_POS(chfsnp, first_hash);
    root_pos   = CHFSNP_BUCKET(chfsnp, bucket_pos);

    return chfsnprb_tree_search_data(pool, root_pos, second_hash, klen, key);
}

uint32_t __chfsnp_bucket_insert(CHFSNP *chfsnp, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key)
{
    uint32_t insert_offset;
    uint32_t bucket_pos;
    uint32_t root_pos;

    if(EC_TRUE == chfsnp_is_full(chfsnp))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:__chfsnp_bucket_insert: chfsnp is full\n");
        return (CHFSNPRB_ERR_POS);
    }

    /*insert the item*/
    bucket_pos = CHFSNP_BUCKET_POS(chfsnp, first_hash);
    root_pos = CHFSNP_BUCKET(chfsnp, bucket_pos);

    if(EC_FALSE == chfsnprb_tree_insert_data(CHFSNP_ITEMS_POOL(chfsnp), &root_pos, second_hash, klen, key, &insert_offset))
    {
        dbg_log(SEC_0097_CHFSNP, 1)(LOGSTDOUT, "warn:__chfsnp_bucket_insert: found duplicate rb node with root %u at node %u\n", root_pos, insert_offset);
        return (insert_offset);
    }
    
    CHFSNP_BUCKET(chfsnp, bucket_pos) = root_pos;    
    return (insert_offset);
}

uint32_t __chfsnp_bucket_delete(CHFSNP *chfsnp, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key)
{
    uint32_t delete_pos;
    uint32_t bucket_pos;
    uint32_t root_pos;

    if(EC_TRUE == chfsnp_is_empty(chfsnp))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:__chfsnp_bucket_delete: chfsnp is empty\n");
        return (CHFSNPRB_ERR_POS);
    }

    /*delete the item*/
    bucket_pos = CHFSNP_BUCKET_POS(chfsnp, first_hash);
    root_pos = CHFSNP_BUCKET(chfsnp, bucket_pos);

    if(EC_FALSE == chfsnprb_tree_delete_data(CHFSNP_ITEMS_POOL(chfsnp), &root_pos, second_hash, klen, key, &delete_pos))
    {
        dbg_log(SEC_0097_CHFSNP, 1)(LOGSTDOUT, "warn:__chfsnp_bucket_delete: delete rb node from root %u failed\n", root_pos);
        return (CHFSNPRB_ERR_POS);
    }
    CHFSNP_BUCKET(chfsnp, bucket_pos) = root_pos;
    
    return (delete_pos);
}

/*delete single item from dnode*/
static EC_BOOL __chfsnp_bucket_delete_item(const CHFSNP *chfsnp, CHFSNP_ITEM *chfsnp_item, CVECTOR *chfsnp_fnode_vec)
{
    if(NULL_PTR != chfsnp_fnode_vec)
    {
        cvector_push_no_lock(chfsnp_fnode_vec, chfsnp_fnode_make(CHFSNP_ITEM_FNODE(chfsnp_item)));
    }
    chfsnp_item_clean(chfsnp_item);
    return (EC_TRUE);
}

static EC_BOOL __chfsnp_bucket_delete_all_items(const CHFSNP *chfsnp, const uint32_t node_pos, CVECTOR *chfsnp_fnode_vec)
{
    CHFSNPRB_POOL *pool;
    CHFSNPRB_NODE *node;
    CHFSNP_ITEM   *item;

    pool = CHFSNP_ITEMS_POOL(chfsnp);
  
    node  = CHFSNPRB_POOL_NODE(pool, node_pos);    
    if(CHFSNPRB_ERR_POS != CHFSNPRB_NODE_LEFT_POS(node))
    {
        __chfsnp_bucket_delete_all_items(chfsnp, CHFSNPRB_NODE_LEFT_POS(node), chfsnp_fnode_vec);
    }

    if(CHFSNPRB_ERR_POS != CHFSNPRB_NODE_RIGHT_POS(node))
    {
        __chfsnp_bucket_delete_all_items(chfsnp, CHFSNPRB_NODE_RIGHT_POS(node), chfsnp_fnode_vec);
    }
    
    item = CHFSNP_RB_NODE_ITEM(node);
    __chfsnp_bucket_delete_item(chfsnp, item, chfsnp_fnode_vec);

    /*chfsnprb recycle the rbnode, do not use chfsnprb_tree_delete which will change the tree structer*/
    chfsnprb_node_free(pool, node_pos);
    
    return (EC_TRUE);
}

EC_BOOL __chfsnp_delete_one_bucket(const CHFSNP *chfsnp, const uint32_t bucket_pos, CVECTOR *chfsnp_fnode_vec)
{
    uint32_t root_pos;

    root_pos = CHFSNP_BUCKET(chfsnp, bucket_pos);    
    if(CHFSNPRB_ERR_POS != root_pos)
    {
        __chfsnp_bucket_delete_all_items(chfsnp, root_pos, chfsnp_fnode_vec);
        CHFSNP_BUCKET(chfsnp, bucket_pos) = CHFSNPRB_ERR_POS;
    }
    return (EC_TRUE);
}

EC_BOOL __chfsnp_delete_all_buckets(const CHFSNP *chfsnp, CVECTOR *chfsnp_fnode_vec)
{
    uint32_t bucket_num;
    uint32_t bucket_pos;

    bucket_num = CHFSNP_BUCKET_MAX_NUM(chfsnp);
    for(bucket_pos = 0; bucket_pos < bucket_num; bucket_pos ++)
    {
        __chfsnp_delete_one_bucket(chfsnp, bucket_pos, chfsnp_fnode_vec);
    }
    return (EC_TRUE);
}

static uint32_t __chfsnp_search_no_lock(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path)
{  
    uint32_t path_1st_hash;
    uint32_t path_2nd_hash;
    uint32_t node_pos;

    path_1st_hash = CHFSNP_1ST_CHASH_ALGO_COMPUTE(chfsnp, path_len, path);
    path_2nd_hash = CHFSNP_2ND_CHASH_ALGO_COMPUTE(chfsnp, path_len, path);
    node_pos      = __chfsnp_bucket_search(chfsnp, 
                                           path_1st_hash, path_2nd_hash, 
                                           path_len, path);
    return (node_pos);
}

uint32_t chfsnp_search_no_lock(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path)
{  
    uint8_t  digest[ CMD5_DIGEST_LEN ];

    __chfsnp_path_md5(path_len, path, digest);
    return __chfsnp_search_no_lock(chfsnp, CMD5_DIGEST_LEN, digest);
}

uint32_t chfsnp_search(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path)
{
    uint8_t  digest[ CMD5_DIGEST_LEN ];
    uint32_t node_pos;
#if 0
    if(CHFSNP_KEY_MAX_SIZE < path_len)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_search: path_len %u > CHFSNP_KEY_MAX_SIZE %u, overflow\n", path_len, CHFSNP_KEY_MAX_SIZE);
        return (CHFSNPRB_ERR_POS);
    }
#endif
    __chfsnp_path_md5(path_len, path, digest);

    CHFSNP_RDLOCK(chfsnp, LOC_CHFSNP_0012);
    node_pos = __chfsnp_search_no_lock(chfsnp, CMD5_DIGEST_LEN, digest);
    CHFSNP_UNLOCK(chfsnp, LOC_CHFSNP_0013);

    return (node_pos);
}

uint32_t chfsnp_insert_no_lock(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path)
{
    CHFSNP_ITEM *chfsnp_item;
    uint32_t path_1st_hash;
    uint32_t path_2nd_hash;
    uint32_t node_pos;

    dbg_log(SEC_0097_CHFSNP, 9)(LOGSTDNULL, "[DEBUG] chfsnp_insert_no_lock: path_len %u\n", path_len);
    if(CHFSNP_KEY_MAX_SIZE < path_len)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_insert_no_lock: path_len %u overflow\n", path_len);
        return (CHFSNPRB_ERR_POS);
    }

    path_1st_hash = CHFSNP_1ST_CHASH_ALGO_COMPUTE(chfsnp, path_len, path);
    path_2nd_hash = CHFSNP_2ND_CHASH_ALGO_COMPUTE(chfsnp, path_len, path);
    node_pos      = __chfsnp_bucket_insert(chfsnp, 
                                           path_1st_hash, path_2nd_hash, 
                                           path_len, path);
    if(CHFSNPRB_ERR_POS == node_pos)
    {
        return (CHFSNPRB_ERR_POS);
    }
    
    chfsnp_item = chfsnp_fetch(chfsnp, node_pos);

    /*fill in chfsnp_item_insert*/    
    chfsnp_item_set_key(chfsnp_item, path_len, path);
    chfsnp_fnode_init(CHFSNP_ITEM_FNODE(chfsnp_item));
    CHFSNP_ITEM_STAT(chfsnp_item) = CHFSNP_ITEM_STAT_IS_USED;

    return (node_pos);
}

uint32_t chfsnp_insert(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path)
{
    uint8_t  digest[ CMD5_DIGEST_LEN ];
    uint32_t node_pos;
#if 0
    if(CHFSNP_KEY_MAX_SIZE < path_len)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_insert: path_len %u > CHFSNP_KEY_MAX_SIZE %u, overflow\n", path_len, CHFSNP_KEY_MAX_SIZE);
        return (CHFSNPRB_ERR_POS);
    }
#endif
    __chfsnp_path_md5(path_len, path, digest);

    CHFSNP_WRLOCK(chfsnp, LOC_CHFSNP_0014);
    node_pos = chfsnp_insert_no_lock(chfsnp, CMD5_DIGEST_LEN, digest);
    CHFSNP_UNLOCK(chfsnp, LOC_CHFSNP_0015);

    return (node_pos);
}

CHFSNP_ITEM *chfsnp_fetch(const CHFSNP *chfsnp, const uint32_t node_pos)
{
    if(CHFSNPRB_ERR_POS != node_pos)
    {
        const CHFSNPRB_POOL *pool;
        const CHFSNPRB_NODE *node;

        pool = CHFSNP_ITEMS_POOL(chfsnp);
        node = CHFSNPRB_POOL_NODE(pool, node_pos);
        if(NULL_PTR != node)
        {
            return (CHFSNP_ITEM *)CHFSNP_RB_NODE_ITEM(node);
        }
    }
    dbg_log(SEC_0097_CHFSNP, 9)(LOGSTDOUT, "[DEBUG] chfsnp_fetch: np %u, fetch node %u failed\n", CHFSNP_ID(chfsnp), node_pos);
    return (NULL_PTR);
}

EC_BOOL chfsnp_inode_update(CHFSNP *chfsnp, CHFSNP_INODE *chfsnp_inode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)
{
    if(src_disk_no  == CHFSNP_INODE_DISK_NO(chfsnp_inode) 
    && src_block_no == CHFSNP_INODE_BLOCK_NO(chfsnp_inode)
    && src_page_no  == CHFSNP_INODE_PAGE_NO(chfsnp_inode))
    {
        CHFSNP_INODE_DISK_NO(chfsnp_inode)  = des_disk_no;
        CHFSNP_INODE_BLOCK_NO(chfsnp_inode) = des_block_no;
        CHFSNP_INODE_PAGE_NO(chfsnp_inode)  = des_page_no;
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_fnode_update(CHFSNP *chfsnp, CHFSNP_FNODE *chfsnp_fnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)

{
    uint32_t replica;

    for(replica = 0; replica < CHFSNP_FNODE_REPNUM(chfsnp_fnode); replica ++)
    {
        chfsnp_inode_update(chfsnp, CHFSNP_FNODE_INODE(chfsnp_fnode, replica), 
                            src_disk_no, src_block_no, src_page_no, 
                            des_disk_no, des_block_no, des_page_no);
    }
    return (EC_TRUE);
}

static EC_BOOL __chfsnp_bucket_update(CHFSNP * chfsnp, CHFSNPRB_POOL *pool, const uint32_t node_pos, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)
{
    CHFSNPRB_NODE *node;
    CHFSNP_ITEM   *item;

    if(CHFSNPRB_ERR_POS == node_pos)
    {
        return (EC_TRUE);
    }
    
    node  = CHFSNPRB_POOL_NODE(pool, node_pos);    
    if(CHFSNPRB_ERR_POS != CHFSNPRB_NODE_LEFT_POS(node))
    {
        __chfsnp_bucket_update(chfsnp, pool, CHFSNPRB_NODE_LEFT_POS(node), 
                               src_disk_no, src_block_no, src_page_no, 
                               des_disk_no, des_block_no, des_page_no);
    }

    item = CHFSNP_RB_NODE_ITEM(node);

    chfsnp_item_update(chfsnp, item, 
                       src_disk_no, src_block_no, src_page_no, 
                       des_disk_no, des_block_no, des_page_no);


    if(CHFSNPRB_ERR_POS != CHFSNPRB_NODE_RIGHT_POS(node))
    {
        __chfsnp_bucket_update(chfsnp, pool, CHFSNPRB_NODE_RIGHT_POS(node), 
                               src_disk_no, src_block_no, src_page_no, 
                               des_disk_no, des_block_no, des_page_no);
    }    
    
    return (EC_TRUE);
}

EC_BOOL __chfsnp_update_one_bucket(CHFSNP *chfsnp, const uint32_t bucket_pos, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)
{
    CHFSNPRB_POOL *pool;
    pool = CHFSNP_ITEMS_POOL(chfsnp);

    return __chfsnp_bucket_update(chfsnp, pool, CHFSNP_BUCKET(chfsnp, bucket_pos),
                                   src_disk_no, src_block_no, src_page_no, 
                                   des_disk_no, des_block_no, des_page_no);    
}

EC_BOOL chfsnp_update_all_buckets(CHFSNP *chfsnp, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)

{
    uint32_t bucket_num;
    uint32_t bucket_pos;

    bucket_num = CHFSNP_BUCKET_MAX_NUM(chfsnp);    
    for(bucket_pos = 0; bucket_pos < bucket_num; bucket_pos ++)
    {
        if(EC_FALSE == __chfsnp_update_one_bucket(chfsnp, bucket_pos, 
                                       src_disk_no, src_block_no, src_page_no, 
                                       des_disk_no, des_block_no, des_page_no))
        {
            dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_update_all_buckets: update bucket %u failed\n", bucket_pos);
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_item_update(CHFSNP *chfsnp, CHFSNP_ITEM *chfsnp_item, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)
{
    if(CHFSNP_ITEM_STAT_IS_NOT_USED == CHFSNP_ITEM_STAT(chfsnp_item))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_item_update: item was not used\n");
        return (EC_FALSE);
    }

    return chfsnp_fnode_update(chfsnp, CHFSNP_ITEM_FNODE(chfsnp_item), 
                               src_disk_no, src_block_no, src_page_no, 
                               des_disk_no, des_block_no, des_page_no);       
}

EC_BOOL chfsnp_update_no_lock(CHFSNP *chfsnp, 
                               const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                               const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no)

{
    uint32_t offset;
    CHFSNP_ITEM *chfsnp_item;

    offset = 0;/*the first item is root directory*/
    chfsnp_item = chfsnp_fetch(chfsnp, offset);
    return chfsnp_item_update(chfsnp, chfsnp_item, 
                              src_disk_no, src_block_no, src_page_no, 
                              des_disk_no, des_block_no, des_page_no);    /*recursively*/
}


CHFSNP_ITEM *chfsnp_set(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path)
{
    return chfsnp_fetch(chfsnp, chfsnp_insert(chfsnp, path_len, path));
}

CHFSNP_ITEM *chfsnp_get(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path)
{
    return chfsnp_fetch(chfsnp, chfsnp_search(chfsnp, path_len, path));
}

EC_BOOL chfsnp_delete(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path, CVECTOR *chfsnp_fnode_vec)
{
    uint8_t  digest[ CMD5_DIGEST_LEN ];
    
    CHFSNP_ITEM *chfsnp_item;
    uint32_t first_hash;
    uint32_t second_hash;
    uint32_t delete_pos;

    if(CHFSNP_KEY_MAX_SIZE < path_len)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_delete: path_len %u > CHFSNP_KEY_MAX_SIZE %u, overflow\n", path_len, CHFSNP_KEY_MAX_SIZE);
        return (EC_FALSE);
    }

    __chfsnp_path_md5(path_len, path, digest);    

    first_hash  = CHFSNP_1ST_CHASH_ALGO_COMPUTE(chfsnp, CMD5_DIGEST_LEN, digest);
    second_hash = CHFSNP_2ND_CHASH_ALGO_COMPUTE(chfsnp, CMD5_DIGEST_LEN, digest);

    CHFSNP_WRLOCK(chfsnp, LOC_CHFSNP_0016);
    
    delete_pos = __chfsnp_bucket_delete(chfsnp, 
                                          first_hash, second_hash, 
                                          CMD5_DIGEST_LEN, digest);
    if(CHFSNPRB_ERR_POS == delete_pos)
    {
        CHFSNP_UNLOCK(chfsnp, LOC_CHFSNP_0017);
        return (EC_FALSE);
    }

    chfsnp_item = chfsnp_fetch(chfsnp, delete_pos);

    if(NULL_PTR != chfsnp_fnode_vec)
    {
        cvector_push_no_lock(chfsnp_fnode_vec, chfsnp_fnode_make(CHFSNP_ITEM_FNODE(chfsnp_item)));
    }
    chfsnp_item_clean(chfsnp_item);    
 
    CHFSNP_UNLOCK(chfsnp, LOC_CHFSNP_0018);

    return (EC_TRUE);
}

uint32_t chfsnp_count_file_num(const CHFSNP *chfsnp)
{
    return CHFSNP_ITEMS_USED_NUM(chfsnp);
}

EC_BOOL chfsnp_file_size(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path, uint32_t *file_size)
{
    CHFSNP_ITEM *chfsnp_item;

    chfsnp_item = chfsnp_get(chfsnp, path_len, path);
    if(NULL_PTR == chfsnp_item)
    {
        (*file_size) = 0;
        return (EC_FALSE);
    }

    (*file_size) = CHFSNP_ITEM_F_SIZE(chfsnp_item);

    return (EC_TRUE);
}

static EC_BOOL __chfsnp_count_bucket_file_size(CHFSNP * chfsnp, CHFSNPRB_POOL *pool, const uint32_t node_pos, uint64_t *file_size)
{
    CHFSNPRB_NODE *node;
    CHFSNP_ITEM   *item;

    if(CHFSNPRB_ERR_POS == node_pos)
    {
        return (EC_TRUE);
    }
    
    node  = CHFSNPRB_POOL_NODE(pool, node_pos);    
    if(CHFSNPRB_ERR_POS != CHFSNPRB_NODE_LEFT_POS(node))
    {
        __chfsnp_count_bucket_file_size(chfsnp, pool, CHFSNPRB_NODE_LEFT_POS(node), file_size);
    }

    item = CHFSNP_RB_NODE_ITEM(node);
    (*file_size) += CHFSNP_ITEM_F_SIZE(item);

    if(CHFSNPRB_ERR_POS != CHFSNPRB_NODE_RIGHT_POS(node))
    {
        __chfsnp_count_bucket_file_size(chfsnp, pool, CHFSNPRB_NODE_RIGHT_POS(node), file_size);
    }    
    
    return (EC_TRUE);
}

EC_BOOL __chfsnp_count_one_bucket_file_size(CHFSNP *chfsnp, const uint32_t bucket_pos, uint64_t *file_size)
{
    CHFSNPRB_POOL *pool;
    pool = CHFSNP_ITEMS_POOL(chfsnp);

    return __chfsnp_count_bucket_file_size(chfsnp, pool, bucket_pos, file_size);
}

EC_BOOL chfsnp_count_file_size(CHFSNP *chfsnp, uint64_t *file_size)
{
    uint32_t bucket_num;
    uint32_t bucket_pos;

    bucket_num = CHFSNP_BUCKET_MAX_NUM(chfsnp);
    for(bucket_pos = 0; bucket_pos < bucket_num; bucket_pos ++)
    {
        __chfsnp_count_one_bucket_file_size(chfsnp, bucket_pos, file_size);
    }
    return (EC_TRUE);
}

CHFSNP *chfsnp_open(const char *np_root_dir, const uint32_t np_id)
{
    UINT32 fsize;
    char *np_fname;
    CHFSNP *chfsnp;
    CHFSNP_HEADER *chfsnp_header;
    int fd;

    np_fname = chfsnp_fname_gen(np_root_dir, np_id);
    if(NULL_PTR == np_fname)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_open: generate np fname from np_root_dir %s failed\n", np_root_dir);
        return (NULL_PTR);
    }

    if(EC_FALSE == c_file_access(np_fname, F_OK))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_open: np %s not exist, try to create it\n", np_fname);
        safe_free(np_fname, LOC_CHFSNP_0019);
        return (NULL_PTR);
    }

    fd = c_file_open(np_fname, O_RDWR, 0666);
    if(ERR_FD == fd)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_open: open chfsnp file %s failed\n", np_fname);
        safe_free(np_fname, LOC_CHFSNP_0020);
        return (NULL_PTR);
    }

    if(EC_FALSE == c_file_size(fd, &fsize))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_open: get size of %s failed\n", np_fname);
        safe_free(np_fname, LOC_CHFSNP_0021);
        c_file_close(fd);
        return (NULL_PTR);
    }    

    chfsnp_header = __chfsnp_header_open(np_id, fsize, fd);
    if(NULL_PTR == chfsnp_header)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_open: open chfsnp file %s failed\n", np_fname);
        safe_free(np_fname, LOC_CHFSNP_0022);
        c_file_close(fd);
        return (NULL_PTR);
    }    

    chfsnp = chfsnp_new();
    if(NULL_PTR == chfsnp)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_open: new chfsnp %u failed\n", np_id);
        safe_free(np_fname, LOC_CHFSNP_0023);
        c_file_close(fd);
        __chfsnp_header_close(chfsnp_header, np_id, fsize);
        return (NULL_PTR);
    }

    CHFSNP_HDR(chfsnp) = chfsnp_header;
    CHFSNP_BUCKET_ADDR(chfsnp)= (uint32_t *)(((uint8_t *)chfsnp_header) + CHFSNP_HEADER_BUCKET_OFFSET(chfsnp_header));

    CHFSNP_1ST_CHASH_ALGO(chfsnp) = chash_algo_fetch(CHFSNP_HEADER_1ST_CHASH_ALGO_ID(chfsnp_header));
    CHFSNP_2ND_CHASH_ALGO(chfsnp) = chash_algo_fetch(CHFSNP_HEADER_2ND_CHASH_ALGO_ID(chfsnp_header));    

    CHFSNP_FD(chfsnp)    = fd;
    CHFSNP_FSIZE(chfsnp) = fsize;
    CHFSNP_FNAME(chfsnp) = (uint8_t *)np_fname;

    ASSERT(np_id == CHFSNP_HEADER_NP_ID(chfsnp_header));

    return (chfsnp);
}

EC_BOOL chfsnp_close(CHFSNP *chfsnp)
{
    if(NULL_PTR != chfsnp)
    {
        if(NULL_PTR != CHFSNP_HDR(chfsnp))
        {
            __chfsnp_header_close(CHFSNP_HDR(chfsnp), CHFSNP_ID(chfsnp), CHFSNP_FSIZE(chfsnp));
            CHFSNP_HDR(chfsnp) = NULL_PTR;
        }
        chfsnp_free(chfsnp);
    }
    return (EC_TRUE);
}

EC_BOOL chfsnp_sync(CHFSNP *chfsnp)
{
    if(NULL_PTR != chfsnp && NULL_PTR != CHFSNP_HDR(chfsnp))
    {
        __chfsnp_header_sync(CHFSNP_HDR(chfsnp), CHFSNP_ID(chfsnp), CHFSNP_FSIZE(chfsnp));
    }
    return (EC_TRUE);
}

CHFSNP *chfsnp_create(const char *np_root_dir, const uint32_t np_id, const uint8_t np_model, const uint8_t hash_1st_algo_id, const uint8_t hash_2nd_algo_id, const uint32_t bucket_max_num)
{
    CHFSNP  *chfsnp;
    CHFSNP_HEADER * chfsnp_header;
    char    *np_fname;
    int      fd;
    UINT32   fsize;
    uint32_t item_max_num;

    if(EC_FALSE == chfsnp_model_file_size(np_model, &fsize))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_create: invalid np_model %u\n", np_model);
        return (NULL_PTR);
    }

    if(EC_FALSE == chfsnp_model_item_max_num(np_model, &item_max_num))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_create: invalid np_model %u\n", np_model);
        return (NULL_PTR);
    }    

    np_fname = chfsnp_fname_gen(np_root_dir, np_id);
    if(NULL_PTR == np_fname)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_create: generate np_fname of np %u, root_dir %s failed\n", np_id, np_root_dir);
        return (NULL_PTR);
    }
    
    if(EC_TRUE == c_file_access(np_fname, F_OK))/*exist*/
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_create: np %u exist already\n", np_id);
        safe_free(np_fname, LOC_CHFSNP_0024);
        return (NULL_PTR);
    }

    fd = c_file_open(np_fname, O_RDWR | O_CREAT, 0666);
    if(ERR_FD == fd)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_create: cannot create np %s\n", np_fname);
        safe_free(np_fname, LOC_CHFSNP_0025);
        return (NULL_PTR);
    }

    dbg_log(SEC_0097_CHFSNP, 9)(LOGSTDOUT, "[DEBUG] chfsnp_create: try to truncate %s to size %lu\n", np_fname, fsize);

    if(EC_FALSE == c_file_truncate(fd, fsize))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_create: truncate np %s to size %lu failed\n", np_fname, fsize);
        safe_free(np_fname, LOC_CHFSNP_0026);
        c_file_close(fd);
        return (NULL_PTR);
    }

    chfsnp_header = __chfsnp_header_create(np_id, fsize, fd, np_model, bucket_max_num);
    if(NULL_PTR == chfsnp_header)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_create: open chfsnp file %s failed\n", np_fname);
        safe_free(np_fname, LOC_CHFSNP_0027);
        c_file_close(fd);
        return (NULL_PTR);
    }
    CHFSNP_HEADER_1ST_CHASH_ALGO_ID(chfsnp_header) = hash_1st_algo_id;
    CHFSNP_HEADER_2ND_CHASH_ALGO_ID(chfsnp_header) = hash_2nd_algo_id;    

    chfsnp = chfsnp_new();
    if(NULL_PTR == chfsnp)
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_create: new chfsnp %u failed\n", np_id);
        safe_free(np_fname, LOC_CHFSNP_0028);
        c_file_close(fd);
        __chfsnp_header_close(chfsnp_header, np_id, fsize);
        return (NULL_PTR);
    }
    CHFSNP_HDR(chfsnp) = chfsnp_header;
    CHFSNP_BUCKET_ADDR(chfsnp) = (uint32_t *)(((uint8_t *)chfsnp_header) + CHFSNP_HEADER_BUCKET_OFFSET(chfsnp_header));
    dbg_log(SEC_0097_CHFSNP, 9)(LOGSTDOUT, "[DEBUG] chfsnp_create: chfsnp_header = %p, offset = %u, bucket addr %p\n", 
                        chfsnp_header, CHFSNP_HEADER_BUCKET_OFFSET(chfsnp_header), CHFSNP_BUCKET_ADDR(chfsnp));

    CHFSNP_1ST_CHASH_ALGO(chfsnp) = chash_algo_fetch(CHFSNP_HEADER_1ST_CHASH_ALGO_ID(chfsnp_header));
    CHFSNP_2ND_CHASH_ALGO(chfsnp) = chash_algo_fetch(CHFSNP_HEADER_2ND_CHASH_ALGO_ID(chfsnp_header));    

    CHFSNP_FD(chfsnp)    = fd;
    CHFSNP_FSIZE(chfsnp) = fsize;
    CHFSNP_FNAME(chfsnp) = (uint8_t *)np_fname;

    ASSERT(np_id == CHFSNP_HEADER_NP_ID(chfsnp_header));    

    /*create root item*/
    //chfsnp_create_root_item(chfsnp);

    dbg_log(SEC_0097_CHFSNP, 9)(LOGSTDOUT, "[DEBUG] chfsnp_create: create np %u done\n", np_id);

    return (chfsnp);
}

EC_BOOL chfsnp_show_item(LOG *log, const CHFSNP_ITEM *chfsnp_item)
{
    if(CHFSNP_ITEM_STAT_IS_NOT_USED == CHFSNP_ITEM_STAT(chfsnp_item))
    {
        dbg_log(SEC_0097_CHFSNP, 0)(LOGSTDOUT, "error:chfsnp_show_item: item %p not used\n", chfsnp_item);
        return (EC_FALSE);
    }

    chfsnp_item_print(log, chfsnp_item);
    return (EC_TRUE);
}

static EC_BOOL __chfsnp_show_one_bucket(LOG *log, const CHFSNP * chfsnp, const CHFSNPRB_POOL *pool, const uint32_t node_pos)
{
    CHFSNPRB_NODE *node;
    CHFSNP_ITEM   *item;

    if(CHFSNPRB_ERR_POS == node_pos)
    {
        return (EC_TRUE);
    }
    
    node  = CHFSNPRB_POOL_NODE(pool, node_pos);    
    if(CHFSNPRB_ERR_POS != CHFSNPRB_NODE_LEFT_POS(node))
    {
        __chfsnp_show_one_bucket(log, chfsnp, pool, CHFSNPRB_NODE_LEFT_POS(node));
    }

    item = CHFSNP_RB_NODE_ITEM(node);
    chfsnp_show_item(log, item);

    if(CHFSNPRB_ERR_POS != CHFSNPRB_NODE_RIGHT_POS(node))
    {
        __chfsnp_show_one_bucket(log, chfsnp, pool, CHFSNPRB_NODE_RIGHT_POS(node));
    }    
    
    return (EC_TRUE);
}


EC_BOOL chfsnp_show_one_bucket(LOG *log, const CHFSNP *chfsnp, const uint32_t bucket_pos)
{
    const CHFSNPRB_POOL *pool;
    pool = CHFSNP_ITEMS_POOL(chfsnp);
    
    return __chfsnp_show_one_bucket(log, chfsnp, pool, CHFSNP_BUCKET(chfsnp, bucket_pos));
}


EC_BOOL chfsnp_show_all_buckets(LOG *log, const CHFSNP *chfsnp)
{
    uint32_t bucket_num;
    uint32_t bucket_pos;

    bucket_num = CHFSNP_BUCKET_MAX_NUM(chfsnp);

    for(bucket_pos = 0; bucket_pos < bucket_num; bucket_pos ++)
    {
        chfsnp_show_one_bucket(log, chfsnp, bucket_pos);
    }

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

