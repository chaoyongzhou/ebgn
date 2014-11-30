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

#include "chashalgo.h"
#include "cdsk.h"
#include "cstack.h"
#include "cmd5.h"

#include "cpgrb.h"
#include "cpgd.h"
#include "crfsnprb.h"
#include "crfsnp.inc"
#include "crfsnp.h"
#include "crfsmc.h"
#include "crfs.h"
#include "crfsmclist.h"

CRFSMC *crfsmc_new(const UINT32 crfs_md_id, const uint32_t np_id, const uint8_t np_model, const uint8_t hash_2nd_algo_id, const uint16_t block_num)
{
    CRFSMC *crfsmc;

    crfsmc = (CRFSMC *)safe_malloc(sizeof(CRFSMC), LOC_CRFSMC_0001);
    if(NULL_PTR == crfsmc)
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_new: new crfsmc failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == crfsmc_init(crfsmc, crfs_md_id, np_id, np_model, hash_2nd_algo_id, block_num))
    {
        safe_free(crfsmc, LOC_CRFSMC_0002);
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_new: init crfsmc failed\n");
        return (NULL_PTR);
    }

    return (crfsmc);
}

EC_BOOL crfsmc_init(CRFSMC *crfsmc, const UINT32 crfs_md_id, const uint32_t np_id, const uint8_t np_model, const uint8_t hash_2nd_algo_id, const uint16_t block_num)
{
    CRFSNP            *crfsnp;
    CPGD              *cpgd;  
    CRFSMCLIST        *mclist;
    uint32_t           mclist_max_num;
    UINT32             mcache_size;           

    crfsnp = crfsnp_mem_create(np_id, np_model, hash_2nd_algo_id);
    if(NULL_PTR == crfsnp)
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_init: create mem np %u with model %u, hash %u failed\n",
                           np_id, np_model, hash_2nd_algo_id);        
        return (EC_FALSE);
    }    

    cpgd = cpgd_mem_new(block_num);
    if(NULL_PTR == cpgd)
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_init: new mem pgd with %u blocks failed\n", block_num);
        crfsnp_mem_free(crfsnp);
        return (EC_FALSE);
    }

    mclist_max_num = CRFSNP_ITEMS_MAX_NUM(crfsnp);

    mclist = crfsmclist_new(mclist_max_num);
    if(NULL_PTR == mclist)
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_init: new crfsmclist with %u nodes failed\n", mclist_max_num);
        cpgd_mem_free(cpgd);
        crfsnp_mem_free(crfsnp);
        return (EC_FALSE);
    }

    mcache_size = ((UINT32)block_num) * CPGD_BLOCK_PAGE_NUM * CPGB_PAGE_4K_BYTE_SIZE;
    CRFSMC_MCACHE(crfsmc) = safe_malloc(mcache_size, LOC_CRFSMC_0003);
    if(NULL_PTR == CRFSMC_MCACHE(crfsmc))
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_init: new mcache with %ld bytes failed\n", mcache_size);
        crfsmclist_free(mclist);
        cpgd_mem_free(cpgd);
        crfsnp_mem_free(crfsnp);
        return (EC_FALSE);
    }    

    CRFSMC_CRFS_MD_ID(crfsmc) = crfs_md_id;
    CRFSMC_NP(crfsmc)         = crfsnp;
    CRFSMC_PGD(crfsmc)        = cpgd;
    CRFSMC_LIST(crfsmc)       = mclist;

    CRFSMC_INIT_LOCK(crfsmc, LOC_CRFSMC_0004);

    return (EC_TRUE);
}

EC_BOOL crfsmc_clean(CRFSMC *crfsmc)
{
    ASSERT(NULL_PTR != crfsmc);

    if(NULL_PTR != CRFSMC_NP(crfsmc))
    {
        crfsnp_mem_free(CRFSMC_NP(crfsmc));
        CRFSMC_NP(crfsmc) = NULL_PTR;
    }

    if(NULL_PTR != CRFSMC_PGD(crfsmc))
    {
        cpgd_mem_free(CRFSMC_PGD(crfsmc));
        CRFSMC_PGD(crfsmc) = NULL_PTR;
    }

    if(NULL_PTR != CRFSMC_LIST(crfsmc))
    {
        crfsmclist_free(CRFSMC_LIST(crfsmc));
        CRFSMC_LIST(crfsmc) = NULL_PTR;
    }    

    if(NULL_PTR != CRFSMC_MCACHE(crfsmc))
    {
        safe_free(CRFSMC_MCACHE(crfsmc), LOC_CRFSMC_0005);
        CRFSMC_MCACHE(crfsmc) = NULL_PTR;
    }    

    CRFSMC_CLEAN_LOCK(crfsmc, LOC_CRFSMC_0006);

    return (EC_TRUE);
}

EC_BOOL crfsmc_free(CRFSMC *crfsmc)
{
    if(NULL_PTR != crfsmc)
    {
        crfsmc_clean(crfsmc);
        safe_free(crfsmc, LOC_CRFSMC_0007);
    }

    return (EC_TRUE);
}

CRFSNP_FNODE *crfsmc_reserve_np_no_lock(CRFSMC *crfsmc, const CSTRING *file_path, uint32_t *node_pos)
{
    CRFSNP *crfsnp;
    CRFSNP_ITEM *crfsnp_item;
    uint32_t node_pos_t;

    crfsnp = CRFSMC_NP(crfsmc);

    dbg_log(SEC_0140_CRFSMC, 9)(LOGSTDOUT, "[DEBUG] crfsmc_reserve_np_no_lock: set mem np beg\n");

    node_pos_t = crfsnp_insert(crfsnp, cstring_get_len(file_path), cstring_get_str(file_path), CRFSNP_ITEM_FILE_IS_REG);
    if(CRFSNPRB_ERR_POS == node_pos_t)
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_reserve_np_no_lock: insert file %s failed\n",
                            (char *)cstring_get_str(file_path));
        return (NULL_PTR);
    }

    dbg_log(SEC_0140_CRFSMC, 9)(LOGSTDOUT, "[DEBUG] crfsmc_reserve_np_no_lock: insert file %s to node_pos %u done\n",
                        (char *)cstring_get_str(file_path), node_pos_t);    

    crfsnp_item = crfsnp_fetch(crfsnp, node_pos_t);
    
    dbg_log(SEC_0140_CRFSMC, 9)(LOGSTDOUT, "[DEBUG] crfsmc_reserve_np_no_lock: set mem np end\n");

    if(CRFSNP_ITEM_FILE_IS_REG != CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_reserve_np_no_lock: file path %s is not regular file\n", 
                            (char *)cstring_get_str(file_path));
        return (NULL_PTR);
    }

    CRFSNP_ITEM_CREATE_TIME(crfsnp_item) = 0/*task_brd_default_get_time()*/;
    CRFSNP_ITEM_EXPIRE_NSEC(crfsnp_item) = 0;

    if(do_log(SEC_0140_CRFSMC, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] crfsmc_reserve_np_no_lock: reserved crfsnp_item %p is\n", crfsnp_item);
        crfsnp_item_print(LOGSTDOUT, crfsnp_item);
    }

    (*node_pos) = node_pos_t;

    /*not import yet*/    
    return CRFSNP_ITEM_FNODE(crfsnp_item);
}

EC_BOOL crfsmc_release_np_no_lock(CRFSMC *crfsmc, const CSTRING *file_path)
{
    CRFSNP *crfsnp;

    crfsnp = CRFSMC_NP(crfsmc);

    if(EC_FALSE == crfsnp_delete(crfsnp, cstring_get_len(file_path), cstring_get_str(file_path), CRFSNP_ITEM_FILE_IS_REG))
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_release_np_no_lock: delete file %s failed\n",
                            (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfsmc_reserve_dn_no_lock(CRFSMC *crfsmc, const uint32_t size, uint16_t *block_no, uint16_t *page_4k_no)
{
    CPGD *cpgd;

    cpgd = CRFSMC_PGD(crfsmc);
    
    if(EC_FALSE == cpgd_new_space(cpgd, size, block_no, page_4k_no))
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDERR, "error:crfsmc_reserve_dn_no_lock: reserve size %u failed\n", size);
        return (EC_FALSE);
    } 

    return (EC_TRUE);
}

EC_BOOL crfsmc_release_dn_no_lock(CRFSMC *crfsmc, const uint32_t size, const uint16_t block_no, const uint16_t page_4k_no)
{
    CPGD *cpgd;

    cpgd = CRFSMC_PGD(crfsmc);
    
    if(EC_FALSE == cpgd_free_space(cpgd, block_no, page_4k_no, size))
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_release_dn_no_lock: release space of block_no %u, page_4k_no %u, size %u failed\n", 
                           block_no, page_4k_no, size);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

static EC_BOOL __crfsmc_release_dn_no_lock(CRFSMC *crfsmc, const CRFSNP_FNODE *crfsnp_fnode)
{
    const CRFSNP_INODE *crfsnp_inode;

    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);

    /*note: disk_no was ignored*/
    return crfsmc_release_dn_no_lock(crfsmc, 
                                     CRFSNP_FNODE_FILESZ(crfsnp_fnode),
                                     CRFSNP_INODE_BLOCK_NO(crfsnp_inode),
                                     CRFSNP_INODE_PAGE_NO(crfsnp_inode));
}


EC_BOOL crfsmc_import_dn_no_lock(CRFSMC *crfsmc, const CBYTES *cbytes, const CRFSNP_FNODE *crfsnp_fnode)
{
    const CRFSNP_INODE *crfsnp_inode;
    
    uint32_t size;
    uint16_t block_no;
    uint16_t page_no; 

    UINT32   offset;

    size = (uint32_t)CBYTES_LEN(cbytes);

    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;        
    
    offset  = ((UINT32)(block_no)) * (CPGB_064MB_PAGE_4K_NUM << CPGB_PAGE_4K_BIT_SIZE) 
            + (((UINT32)(page_no)) << (CPGB_PAGE_4K_BIT_SIZE));
    BCOPY(CBYTES_BUF(cbytes), CRFSMC_MCACHE(crfsmc) + offset, size);

    return (EC_TRUE);
}

/*for debug only*/
static REAL __crfsmc_room_ratio(CRFSMC *crfsmc)
{
    CPGD *cpgd;
    double ratio;

    cpgd = CRFSMC_PGD(crfsmc);
    ratio = (CPGD_PAGE_4K_USED_NUM(cpgd) + 0.0) / (CPGD_PAGE_4K_MAX_NUM(cpgd)  + 0.0);
    return (ratio);
}

EC_BOOL crfsmc_room_is_ok_no_lock(CRFSMC *crfsmc, const REAL level)
{
    CRFSNP   *crfsnp;
    CPGD     *cpgd;
    
    uint64_t  used_size;
    uint64_t  max_size;
    uint64_t  del_size;
    double    ratio;

    cpgd   = CRFSMC_PGD(crfsmc);
    crfsnp = CRFSMC_NP(crfsmc);

    used_size = (((uint64_t)CPGD_PAGE_4K_USED_NUM(cpgd)) << CPGB_PAGE_4K_BIT_SIZE);
    max_size  = (((uint64_t)CPGD_PAGE_4K_MAX_NUM(cpgd)) << CPGB_PAGE_4K_BIT_SIZE);
    del_size  = CRFSNP_DEL_SIZE(crfsnp);
    
    if(used_size < del_size)
    {   
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_room_is_ok_no_lock: invalid used size %ld < del size %ld\n", 
                           used_size, del_size);
        return (EC_FALSE);
    }

    ratio = (used_size + 0.0 - del_size) / (max_size  + 0.0);

    if(ratio < level)
    {
        return (EC_TRUE); /*ok*/
    }
    return (EC_FALSE);/*NOT ok*/
}

EC_BOOL crfsmc_write_no_lock(CRFSMC *crfsmc, const CSTRING *file_path, const CBYTES *cbytes, const uint8_t *md5sum)
{
    CRFSNP_FNODE *crfsnp_fnode;
    CRFSNP_INODE *crfsnp_inode;

    uint32_t node_pos;
    
    uint32_t size;
    uint16_t block_no;
    uint16_t page_4k_no;    

    UINT32   offset;

    crfsnp_fnode = crfsmc_reserve_np_no_lock(crfsmc, file_path, &node_pos);
    if(NULL_PTR == crfsnp_fnode)
    {   
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_write_no_lock: file %s reserve np failed\n", 
                           (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    size = (uint32_t)cbytes_len(cbytes);

    if(EC_FALSE == crfsmc_reserve_dn_no_lock(crfsmc, size, &block_no, &page_4k_no))
    {   
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_write_no_lock: file %s reserve dn with size %u failed\n", 
                           (char *)cstring_get_str(file_path), size);
        crfsmc_release_np_no_lock(crfsmc, file_path);
        return (EC_FALSE);
    }

    /*add to memcache*/
    crfsmclist_node_add_head(CRFSMC_LIST(crfsmc), node_pos);
    dbg_log(SEC_0140_CRFSMC, 9)(LOGSTDOUT, "[DEBUG] crfsmc_write_no_lock: file %s write to mem cache at %u done\n", 
                       (char *)cstring_get_str(file_path), node_pos);    

    crfsnp_fnode_init(crfsnp_fnode);
    CRFSNP_FNODE_FILESZ(crfsnp_fnode) = size;
    CRFSNP_FNODE_REPNUM(crfsnp_fnode) = 1;   

    if(SWITCH_ON == CRFS_MD5_SWITCH)
    {
        BCOPY(md5sum, CRFSNP_FNODE_MD5SUM(crfsnp_fnode), CMD5_DIGEST_LEN);
    }
    
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    CRFSNP_INODE_CACHE_FLAG(crfsnp_inode) = CRFSDN_DATA_NOT_IN_CACHE;
    CRFSNP_INODE_DISK_NO(crfsnp_inode)    = CRFSMC_DISK_NO;
    CRFSNP_INODE_BLOCK_NO(crfsnp_inode)   = block_no;
    CRFSNP_INODE_PAGE_NO(crfsnp_inode)    = page_4k_no;

    /*import data to memcache*/
    offset  = ((UINT32)(block_no)) * (CPGB_064MB_PAGE_4K_NUM << CPGB_PAGE_4K_BIT_SIZE) 
            + (((UINT32)(page_4k_no)) << (CPGB_PAGE_4K_BIT_SIZE));

    dbg_log(SEC_0140_CRFSMC, 9)(LOGSTDOUT, "[DEBUG] crfsmc_write_no_lock: size %u, block %u, page %u, offset %ld\n", 
                       size, block_no, page_4k_no, offset);            
    BCOPY(CBYTES_BUF(cbytes), CRFSMC_MCACHE(crfsmc) + offset, size);  

    return (EC_TRUE);
}

EC_BOOL crfsmc_read_np_no_lock(CRFSMC *crfsmc, const CSTRING *file_path, CRFSNP_FNODE *crfsnp_fnode)
{
    CRFSNP *crfsnp;

    uint32_t node_pos;

    crfsnp = CRFSMC_NP(crfsmc);
    
    node_pos = crfsnp_search_no_lock(crfsnp, (uint32_t)cstring_get_len(file_path), cstring_get_str(file_path), CRFSNP_ITEM_FILE_IS_REG);
    if(CRFSNPRB_ERR_POS != node_pos)
    {
        CRFSNP_ITEM *crfsnp_item;
        CRFSMCLIST * crfsmclist;

        crfsnp_item = crfsnp_fetch(crfsnp, node_pos);
        crfsnp_fnode_import(CRFSNP_ITEM_FNODE(crfsnp_item), crfsnp_fnode);

        /*update LRU list*/
        crfsmclist = CRFSMC_LIST(crfsmc);
        
        if(EC_TRUE == crfsmclist_node_is_used(crfsmclist, node_pos))
        {
            if(node_pos != crfsmclist_head(crfsmclist))
            {
                crfsmclist_node_del(crfsmclist, node_pos);
                
                crfsmclist_node_add_head(crfsmclist, node_pos);
            }
        }
        else
        {
            /*do nothing*/
        }
        
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL crfsmc_read_dn_no_lock(CRFSMC *crfsmc, const CRFSNP_FNODE *crfsnp_fnode, CBYTES *cbytes)
{
    const CRFSNP_INODE *crfsnp_inode;

    uint32_t file_size;
    uint16_t block_no;
    uint16_t page_no;

    UINT32   offset;

    file_size    = CRFSNP_FNODE_FILESZ(crfsnp_fnode);
    crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, 0);
    block_no = CRFSNP_INODE_BLOCK_NO(crfsnp_inode);
    page_no  = CRFSNP_INODE_PAGE_NO(crfsnp_inode) ;


    /*export data from memcache*/
    offset  = ((UINT32)(block_no)) * (CPGB_064MB_PAGE_4K_NUM << CPGB_PAGE_4K_BIT_SIZE) 
            + (((UINT32)(page_no)) << (CPGB_PAGE_4K_BIT_SIZE));

    if(CBYTES_LEN(cbytes) < file_size)
    {
        if(NULL_PTR != CBYTES_BUF(cbytes))
        {
            SAFE_FREE(CBYTES_BUF(cbytes), LOC_CRFSMC_0008);
        }
        CBYTES_BUF(cbytes) = (UINT8 *)SAFE_MALLOC(file_size, LOC_CRFSMC_0009);
        ASSERT(NULL_PTR != CBYTES_BUF(cbytes));
        CBYTES_LEN(cbytes) = 0;
    }

    BCOPY(CRFSMC_MCACHE(crfsmc) + offset, CBYTES_BUF(cbytes), file_size);
    CBYTES_LEN(cbytes) = file_size;
    
    return (EC_TRUE);    
}

EC_BOOL crfsmc_read_no_lock(CRFSMC *crfsmc, const CSTRING *file_path, CBYTES *cbytes)
{
    CRFSNP_FNODE crfsnp_fnode;

    crfsnp_fnode_init(&crfsnp_fnode);

    if(EC_FALSE == crfsmc_read_np_no_lock(crfsmc, file_path, &crfsnp_fnode))
    {
        dbg_log(SEC_0140_CRFSMC, 5)(LOGSTDOUT, "warn:crfsmc_read_no_lock: read file %s from np failed\n", 
                           (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsmc_read_dn_no_lock(crfsmc, &crfsnp_fnode, cbytes))
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_read_dn_no_lock: read file %s from dn failed\n", 
                           (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
   
    return (EC_TRUE);    
}

EC_BOOL crfsmc_retire_no_lock(CRFSMC *crfsmc)
{
    CRFSNP     *crfsnp;
    CRFSMCLIST *crfsmclist;
    uint32_t node_pos;

    /*retire the oldest from LRU list*/
    crfsnp     = CRFSMC_NP(crfsmc);
    crfsmclist = CRFSMC_LIST(crfsmc);

    node_pos = crfsmclist_pop_tail(crfsmclist);

    if(CRFSMCLIST_ERR_POS != node_pos)
    {
        crfsnp_umount_item_deep(crfsnp, node_pos);
    }

    return (EC_TRUE);
}

EC_BOOL crfsmc_recycle_no_lock(CRFSMC *crfsmc)
{
    CRFSNP     *crfsnp;
    CRFSNP_RECYCLE_DN crfsnp_recycle_dn;

    crfsnp     = CRFSMC_NP(crfsmc);

    CRFSNP_RECYCLE_DN_ARG1(&crfsnp_recycle_dn)   = (UINT32)crfsmc;
    CRFSNP_RECYCLE_DN_FUNC(&crfsnp_recycle_dn)   = (CRFSNP_RECYCLE_DN_FUNC)__crfsmc_release_dn_no_lock;
    //CRFSNP_RECYCLE_DN_WRLOCK(&crfsnp_recycle_dn) = NULL_PTR;
    //CRFSNP_RECYCLE_DN_UNLOCK(&crfsnp_recycle_dn) = NULL_PTR;
    
    if(EC_FALSE == crfsnp_recycle(crfsnp, &crfsnp_recycle_dn))
    {
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_recycle_no_lock: recycle failed\n");
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL crfsmc_write(CRFSMC *crfsmc, const CSTRING *file_path, const CBYTES *cbytes, const uint8_t *md5sum)
{
    CRFSMC_WRLOCK(crfsmc, LOC_CRFSMC_0010);
    if(EC_FALSE == crfsmc_write_no_lock(crfsmc, file_path, cbytes, md5sum))
    {
        CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0011);
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_write: write %s with %ld bytes failed\n", 
                           (char *)cstring_get_str(file_path), cbytes_len(cbytes));
        return (EC_FALSE);
    }
    CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0012);
    
    dbg_log(SEC_0140_CRFSMC, 9)(LOGSTDOUT, "[DEBUG] crfsmc_write: write %s with %ld bytes done\n", 
                       (char *)cstring_get_str(file_path), cbytes_len(cbytes));    

    return (EC_TRUE);
}

EC_BOOL crfsmc_read(CRFSMC *crfsmc, const CSTRING *file_path, CBYTES *cbytes)
{
    CRFSMC_RDLOCK(crfsmc, LOC_CRFSMC_0013);
    if(EC_FALSE == crfsmc_read_no_lock(crfsmc, file_path, cbytes))
    {
        CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0014);
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_read: read %s failed\n", 
                           (char *)cstring_get_str(file_path));
        return (EC_FALSE);
    }
    CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0015);

    dbg_log(SEC_0140_CRFSMC, 9)(LOGSTDOUT, "[DEBUG] crfsmc_read: read %s with %ld bytes done\n", 
                       (char *)cstring_get_str(file_path), cbytes_len(cbytes));

    return (EC_TRUE);
}

EC_BOOL crfsmc_retire(CRFSMC *crfsmc)
{
    CRFSMC_WRLOCK(crfsmc, LOC_CRFSMC_0016);
    if(EC_FALSE == crfsmc_retire_no_lock(crfsmc))
    {
        CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0017);
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_retire: retire failed\n");
        return (EC_FALSE);
    }
    CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0018);

    return (EC_TRUE);
}

EC_BOOL crfsmc_recycle(CRFSMC *crfsmc)
{
    CRFSMC_WRLOCK(crfsmc, LOC_CRFSMC_0019);
    if(EC_FALSE == crfsmc_recycle_no_lock(crfsmc))
    {
        CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0020);
        dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_recycle: recycle failed\n");
        return (EC_FALSE);
    }
    CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0021);

    return (EC_TRUE);
}

EC_BOOL crfsmc_ensure_room_safe_level(CRFSMC *crfsmc)
{
    uint32_t retire_times;

    retire_times = 0;
    
    CRFSMC_WRLOCK(crfsmc, LOC_CRFSMC_0022);    
    while(EC_FALSE == crfsmc_room_is_ok_no_lock(crfsmc, CRFSMC_ROOM_SAFE_LEVEL))
    {
        if(EC_FALSE == crfsmc_retire_no_lock(crfsmc))
        {
            crfsmc_recycle_no_lock(crfsmc);
            CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0023);
            
            dbg_log(SEC_0140_CRFSMC, 0)(LOGSTDOUT, "error:crfsmc_ensure_room_safe_level: retire failed\n");
            return (EC_FALSE);
        }

        retire_times ++;
    }

    if(0 < retire_times)
    {
        dbg_log(SEC_0140_CRFSMC, 9)(LOGSTDOUT, "[DEBUG] crfsmc_ensure_room_safe_level: retire times %u\n", retire_times);
        crfsmc_recycle_no_lock(crfsmc);
    }
    
    CRFSMC_UNLOCK(crfsmc, LOC_CRFSMC_0024);

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

