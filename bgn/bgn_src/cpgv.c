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
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "type.h"
#include "mm.h"
#include "log.h"

#include "cmisc.h"
#include "real.h"

#include "db_internal.h"

#include "cpgrb.h"
#include "cpgd.h"
#include "cpgv.h"

/*page-cache disk:1TB = 2^14 page-cache block*/

/************************************************************************************************
  comment:
  ========
   1. if one block can assign max pages with page model, then put the block into page model 
      RB tree of disk
   2. one block was in at most one RB tree
************************************************************************************************/

//#define CPGV_ASSERT(cond)   ASSERT(cond)
#define CPGV_ASSERT(cond)   do{}while(0)

#define DEBUG_COUNT_CPGV_HDR_PAD_SIZE() \
                                (sizeof(CPGV_HDR) \
                                 - sizeof(CPGRB_POOL) \
                                 - CPGB_MODEL_NUM *sizeof(uint16_t) \
                                 - 3 * sizeof(uint16_t) \
                                 - 1 * sizeof(uint32_t) \
                                 - 3 * sizeof(uint64_t))

#define ASSERT_CPGV_HDR_PAD_SIZE() \
    CPGV_ASSERT( CPGV_HDR_PAD_SIZE == DEBUG_COUNT_CPGV_HDR_PAD_SIZE())

static uint16_t __cpgv_page_model_first_block(const CPGV *cpgv, const uint16_t page_model)
{
    uint16_t node_pos;
    const CPGRB_NODE *node;
    
    node_pos = cpgrb_tree_first_node(CPGV_PAGE_DISK_CPGRB_POOL(cpgv), CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv, page_model));
    if(CPGRB_ERR_POS == node_pos)
    {
        sys_log(LOGSTDERR, "error:__cpgv_page_model_first_block: no free page in page model %u\n", page_model);
        return (CPGRB_ERR_POS);
    }

    node = CPGRB_POOL_NODE(CPGV_PAGE_DISK_CPGRB_POOL(cpgv), node_pos);
    return (CPGRB_NODE_DATA(node));
}

static uint16_t __cpgv_page_model_get(const CPGV *cpgv, const uint16_t assign_bitmap)
{
    uint16_t page_model;
    uint16_t e;

    for(page_model = 0, e = 1; CPGB_MODEL_NUM > page_model && (assign_bitmap & e); page_model ++, e <<= 1)
    {
      /*do nothing*/
    }
    return (page_model);
}

static uint8_t *__cpgv_new_disk_fname(const CPGV *cpgv, const uint16_t disk_no)
{
    char *cpgd_dname;
    char *cpgd_fname;
    char  disk_fname[ 32 ];

    if(NULL_PTR == CPGV_FNAME(cpgv))
    {
        sys_log(LOGSTDOUT, "error:__cpgv_new_disk_fname: cpgv fname is null\n");
        return (NULL_PTR);
    }
    
    cpgd_dname = c_dirname((const char *)CPGV_FNAME(cpgv));
    if(NULL_PTR == cpgd_dname)
    {
        sys_log(LOGSTDOUT, "error:__cpgv_new_disk_fname: dname of cpgv fname %s is null\n", (const char *)CPGV_FNAME(cpgv));
        return (NULL_PTR);
    }

    /*disk fname format: ${CPGV_DIR}/dsk${disk_no}.dat*/
    snprintf(disk_fname, sizeof(disk_fname)/sizeof(disk_fname[ 0 ]), "/vol/dsk%04X.dat", disk_no);

    cpgd_fname = c_str_cat(cpgd_dname, disk_fname);
    if(NULL_PTR == cpgd_fname)
    {
        sys_log(LOGSTDOUT, "error:__cpgv_new_disk_fname: str cat %s and %s failed\n", cpgd_dname, disk_fname);
        safe_free(cpgd_dname, LOC_CPGV_0001);
        return (NULL_PTR);
    }
    
    safe_free(cpgd_dname, LOC_CPGV_0002);
    return ((uint8_t *)cpgd_fname);
}

static EC_BOOL __cpgv_free_disk_fname(const CPGV *cpgv, uint8_t *cpgd_fname)
{
    if(NULL_PTR != cpgd_fname)
    {
        safe_free(cpgd_fname, LOC_CPGV_0003);
    }
    return (EC_TRUE);
}    

static void __cpgv_hdr_size_info_print()
{
    sys_log(LOGSTDOUT, "[DEBUG] __cpgv_hdr_size_info_print: sizeof(CPGV_HDR)   = %u\n", sizeof(CPGV_HDR));
    sys_log(LOGSTDOUT, "[DEBUG] __cpgv_hdr_size_info_print: sizeof(CPGRB_POOL) = %u\n", sizeof(CPGRB_POOL));
    sys_log(LOGSTDOUT, "[DEBUG] __cpgv_hdr_size_info_print: CPGB_MODEL_NUM     = %u\n", CPGB_MODEL_NUM);
    sys_log(LOGSTDOUT, "[DEBUG] __cpgv_hdr_size_info_print: sizeof(uint64_t)   = %u\n", sizeof(uint64_t));
    sys_log(LOGSTDOUT, "[DEBUG] __cpgv_hdr_size_info_print: CPGV_HDR_PAD_SIZE  = %u\n", CPGV_HDR_PAD_SIZE);
    sys_log(LOGSTDOUT, "[DEBUG] __cpgv_hdr_size_info_print: sizeof(CPGV_HDR) "
                                 "- sizeof(CPGRB_POOL) "
                                 "- CPGB_MODEL_NUM *sizeof(uint16_t) "
                                 "- 3 * sizeof(uint16_t) "
                                 "- 1 * sizeof(uint32_t) "
                                 "- 3 * sizeof(uint64_t) = %u\n",
                                 DEBUG_COUNT_CPGV_HDR_PAD_SIZE());   
    return;
}

CPGV_HDR *cpgv_hdr_new(CPGV *cpgv, const uint16_t disk_num)
{
    CPGV_HDR *cpgv_hdr;
    uint16_t  page_model;

    ASSERT_CPGV_HDR_PAD_SIZE();

    cpgv_hdr = (CPGV_HDR *)mmap(NULL_PTR, CPGV_FSIZE(cpgv), PROT_READ | PROT_WRITE, MAP_SHARED, CPGV_FD(cpgv), 0);
    if(MAP_FAILED == cpgv_hdr)
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_new: mmap file %s failed, errno = %d, errorstr = %s\n", 
                           (char *)CPGV_FNAME(cpgv), errno, strerror(errno));
        return (NULL_PTR);
    } 

    if(EC_FALSE == cpgrb_pool_init(CPGV_HDR_CPGRB_POOL(cpgv_hdr), disk_num))
    {
        sys_log(LOGSTDERR, "error:cpgv_hdr_new: init cpgrb pool failed where disk_num = %u\n", disk_num);
        munmap(cpgv_hdr, CPGV_FSIZE(cpgv));
        return (NULL_PTR);
    }

    for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++)
    {
        CPGV_HDR_DISK_CPGRB_ROOT_POS(cpgv_hdr, page_model) = CPGRB_ERR_POS;
    }

    CPGV_HDR_ASSIGN_BITMAP(cpgv_hdr) = 0;

    CPGV_HDR_PAGE_DISK_MAX_NUM(cpgv_hdr) = disk_num;

    /*statistics*/    
    CPGV_HDR_PAGE_4K_MAX_NUM(cpgv_hdr)       = ((uint64_t)disk_num) * CPGD_MAX_BLOCK_NUM * CPGD_BLOCK_PAGE_NUM;
    CPGV_HDR_PAGE_4K_USED_NUM(cpgv_hdr)      = 0;
    CPGV_HDR_PAGE_ACTUAL_USED_SIZE(cpgv_hdr) = 0;    
    
    return (cpgv_hdr);
}

EC_BOOL cpgv_hdr_free(CPGV *cpgv)
{   
    if(NULL_PTR != CPGV_HEADER(cpgv))
    {        
        if(0 != msync(CPGV_HEADER(cpgv), CPGV_FSIZE(cpgv), MS_SYNC))
        {
            sys_log(LOGSTDOUT, "warn:cpgv_hdr_free: sync cpgv_hdr of %s with size %u failed\n", 
                               CPGV_FNAME(cpgv), CPGV_FSIZE(cpgv));
        }
        
        if(0 != munmap(CPGV_HEADER(cpgv), CPGV_FSIZE(cpgv)))
        {
            sys_log(LOGSTDOUT, "warn:cpgv_hdr_free: munmap cpgv of %s with size %u failed\n", 
                               CPGV_FNAME(cpgv), CPGV_FSIZE(cpgv));
        }
        
        CPGV_HEADER(cpgv) = NULL_PTR;        
    }

    return (EC_TRUE);
}

CPGV_HDR *cpgv_hdr_open(CPGV *cpgv)
{
    CPGV_HDR *cpgv_hdr;

     __cpgv_hdr_size_info_print();
    ASSERT_CPGV_HDR_PAD_SIZE();

    sys_log(LOGSTDOUT, "[DEBUG] cpgv_hdr_open: fsize %u\n", CPGV_FSIZE(cpgv));

    cpgv_hdr = (CPGV_HDR *)mmap(NULL_PTR, CPGV_FSIZE(cpgv), PROT_READ | PROT_WRITE, MAP_SHARED, CPGV_FD(cpgv), 0);
    if(MAP_FAILED == cpgv_hdr)
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_open: mmap file %s with fd %d failed, errno = %d, errorstr = %s\n", 
                           (char *)CPGV_FNAME(cpgv), CPGV_FD(cpgv), errno, strerror(errno));
        return (NULL_PTR);
    } 
    
    return (cpgv_hdr);
}

EC_BOOL cpgv_hdr_close(CPGV *cpgv)
{   
    if(NULL_PTR != CPGV_HEADER(cpgv))
    {   
        if(0 != msync(CPGV_HEADER(cpgv), CPGV_FSIZE(cpgv), MS_SYNC))
        {
            sys_log(LOGSTDOUT, "warn:cpgv_hdr_close: sync cpgv_hdr of %s with size %u failed\n", 
                               CPGV_FNAME(cpgv), CPGV_FSIZE(cpgv));
        }
        
        if(0 != munmap(CPGV_HEADER(cpgv), CPGV_FSIZE(cpgv)))
        {
            sys_log(LOGSTDOUT, "warn:cpgv_hdr_close: munmap cpgv of %s with size %u failed\n", 
                               CPGV_FNAME(cpgv), CPGV_FSIZE(cpgv));
        }
       
        CPGV_HEADER(cpgv) = NULL_PTR;        
    }

    return (EC_TRUE);
}

EC_BOOL cpgv_hdr_sync(CPGV *cpgv)
{   
    if(NULL_PTR != CPGV_HEADER(cpgv))
    {   
        if(0 != msync(CPGV_HEADER(cpgv), CPGV_FSIZE(cpgv), MS_SYNC))
        {
            sys_log(LOGSTDOUT, "warn:cpgv_hdr_sync: sync cpgv_hdr of %s with size %u failed\n", 
                               CPGV_FNAME(cpgv), CPGV_FSIZE(cpgv));
        }        
    }

    return (EC_TRUE);
}

EC_BOOL cpgv_hdr_flush_size(const CPGV_HDR *cpgv_hdr, UINT32 *size)
{
    (*size) += sizeof(CPGV_HDR);
    return (EC_TRUE);
}

EC_BOOL cpgv_hdr_flush(const CPGV_HDR *cpgv_hdr, int fd, UINT32 *offset)
{
    UINT32 osize;/*flush once size*/

    DEBUG(UINT32 offset_saved = *offset;);

    /*flush rbtree pool*/
    if(EC_FALSE == cpgrb_flush(CPGV_HDR_CPGRB_POOL(cpgv_hdr), fd, offset))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_flush: flush CPGV_HDR_CPGRB_POOL at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*flush CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS_TBL*/
    osize = CPGB_MODEL_NUM * sizeof(uint16_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)CPGV_HDR_DISK_CPGRB_ROOT_POS_TBL(cpgv_hdr)))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_flush: flush CPGV_HDR_DISK_CPGRB_ROOT_POS_TBL at offset %u of fd %d failed\n", 
                            (*offset), fd);
        return (EC_FALSE);
    }  

    /*skip rsvd1*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_pad(fd, offset, osize, FILE_PAD_CHAR))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_flush: pad %ld bytes at offset %u of fd %d failed\n", osize, (*offset), fd);
        return (EC_FALSE);
    }    

    /*flush CPGV_HDR_ASSIGN_BITMAP*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGV_HDR_ASSIGN_BITMAP(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_flush: flush CPGV_HDR_ASSIGN_BITMAP at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    
    
    /*flush CPGV_HDR_PAGE_DISK_MAX_NUM*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGV_HDR_PAGE_DISK_MAX_NUM(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_flush: flush CPGV_HDR_PAGE_DISK_MAX_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*flush CPGV_HDR_PAGE_4K_MAX_NUM*/
    osize = sizeof(uint64_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGV_HDR_PAGE_4K_MAX_NUM(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_flush: flush CPGV_HDR_PAGE_4K_MAX_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*flush CPGV_HDR_PAGE_4K_USED_NUM*/
    osize = sizeof(uint64_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGV_HDR_PAGE_4K_USED_NUM(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_flush: flush CPGV_HDR_PAGE_4K_USED_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*flush CPGV_HDR_PAGE_ACTUAL_USED_SIZE*/
    osize = sizeof(uint64_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGV_HDR_PAGE_ACTUAL_USED_SIZE(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_flush: flush CPGV_HDR_PAGE_ACTUAL_USED_SIZE at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*skip rsvd2*/
    if(EC_FALSE == c_file_pad(fd, offset, CPGV_HDR_PAD_SIZE, FILE_PAD_CHAR))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_flush: flush CPGV_HDR_PAD at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    DEBUG(CPGV_ASSERT(sizeof(CPGV_HDR) == (*offset) - offset_saved));
   
    return (EC_TRUE);
}

EC_BOOL cpgv_hdr_load(CPGV_HDR *cpgv_hdr, int fd, UINT32 *offset)
{
    UINT32 osize;/*load once size*/

    /*load rbtree pool*/
    if(EC_FALSE == cpgrb_load(CPGV_HDR_CPGRB_POOL(cpgv_hdr), fd, offset))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_load: load CPGV_HDR_CPGRB_POOL at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*load CPGV_HDR_DISK_CPGRB_ROOT_POS_TBL*/
    osize = CPGB_MODEL_NUM * sizeof(uint16_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)CPGV_HDR_DISK_CPGRB_ROOT_POS_TBL(cpgv_hdr)))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_load: load CPGV_HDR_DISK_CPGRB_ROOT_POS_TBL at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*skip rsvd1*/
    (*offset) += sizeof(uint16_t);

    /*load CPGV_HDR_ASSIGN_BITMAP*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGV_HDR_ASSIGN_BITMAP(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_load: load CPGV_HDR_ASSIGN_BITMAP at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*load CPGV_HDR_PAGE_DISK_MAX_NUM*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGV_HDR_PAGE_DISK_MAX_NUM(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_load: load CPGV_HDR_PAGE_DISK_MAX_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*load CPGV_HDR_PAGE_4K_MAX_NUM*/
    osize = sizeof(uint64_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGV_HDR_PAGE_4K_MAX_NUM(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_load: load CPGV_HDR_PAGE_4K_MAX_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*load CPGV_HDR_PAGE_4K_USED_NUM*/
    osize = sizeof(uint64_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGV_HDR_PAGE_4K_USED_NUM(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_load: load CPGV_HDR_PAGE_4K_USED_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*load CPGV_HDR_PAGE_ACTUAL_USED_SIZE*/
    osize = sizeof(uint64_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGV_HDR_PAGE_ACTUAL_USED_SIZE(cpgv_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgv_hdr_load: load CPGV_HDR_PAGE_ACTUAL_USED_SIZE at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*skip rsvd2*/
    (*offset) += CPGV_HDR_PAD_SIZE;

    return (EC_TRUE);
}

CPGV *cpgv_new(const uint8_t *cpgv_fname, const uint16_t disk_num)
{
    CPGV      *cpgv;
    uint16_t   disk_no;

    if(CPGV_MAX_DISK_NUM < disk_num)
    {
        sys_log(LOGSTDOUT, "error:cpgv_new: disk_num %u overflow\n", disk_num);
        return (NULL_PTR);
    }
    
    if(EC_TRUE == c_file_access((const char *)cpgv_fname, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cpgv_new: %s already exist\n", cpgv_fname);
        return (NULL_PTR);
    }

    alloc_static_mem(MD_TBD, 0, MM_CPGV, &cpgv, LOC_CPGV_0004);
    if(NULL_PTR == cpgv)
    {
        sys_log(LOGSTDOUT, "error:cpgv_new:malloc cpgv failed\n");
        return (NULL_PTR);
    }

    cpgv_init(cpgv);

    CPGV_FNAME(cpgv) = (uint8_t *)c_str_dup((char *)cpgv_fname);
    if(NULL_PTR == CPGV_FNAME(cpgv))
    {
        sys_log(LOGSTDOUT, "error:cpgv_new:str dup %s failed\n", cpgv_fname);
        cpgv_free(cpgv);
        return (NULL_PTR);
    }

    CPGV_FD(cpgv) = c_file_open((const char *)cpgv_fname, O_RDWR | O_SYNC | O_CREAT, 0666);
    if(ERR_FD == CPGV_FD(cpgv))
    {
        sys_log(LOGSTDOUT, "error:cpgv_new: create %s failed\n", cpgv_fname);
        cpgv_free(cpgv);
        return (NULL_PTR);
    }

    CPGV_FSIZE(cpgv) = sizeof(CPGV_HDR) + disk_num * sizeof(CPGD);
    sys_log(LOGSTDOUT, "[DEBUG] sizeof(CPGV_HDR) = %u, disk_num = %u, sizeof(CPGD) = %u, CPGV_FSIZE(cpgv) = %u\n", 
                        sizeof(CPGV_HDR), disk_num, sizeof(CPGD), CPGV_FSIZE(cpgv));
    if(EC_FALSE == c_file_truncate(CPGV_FD(cpgv), CPGV_FSIZE(cpgv)))
    {
        sys_log(LOGSTDOUT, "error:cpgv_new: truncate %s to %u bytes failed\n", cpgv_fname, CPGV_FSIZE(cpgv));
        cpgv_free(cpgv);
        return (NULL_PTR);
    }

    CPGV_HEADER(cpgv) = cpgv_hdr_new(cpgv, disk_num);
    if(NULL_PTR == CPGV_HEADER(cpgv))
    {
        sys_log(LOGSTDOUT, "error:cpgv_new: new cpgv header of file %s failed\n", cpgv_fname);
        cpgv_free(cpgv);
        return (NULL_PTR);
    }

    /*init blocks*/
    for(disk_no = 0; disk_no < disk_num; disk_no ++)
    {
        uint8_t *cpgd_fname;

        cpgd_fname = __cpgv_new_disk_fname(cpgv, disk_no);
        if(NULL_PTR == cpgd_fname)
        {
            sys_log(LOGSTDOUT, "error:cpgv_new: new disk %u fname failed\n", disk_no);
            cpgv_free(cpgv);
            return (NULL_PTR);
        }

        sys_log(LOGSTDOUT, "info:cpgv_new: try to create disk %s ...\n", cpgd_fname);
        sys_log(LOGSTDOUT, "[DEBUG] cpgv_new: ### set to CPGD_MAX_BLOCK_NUM %d for debug purpose \n", CPGD_MAX_BLOCK_NUM);
        CPGV_DISK_CPGD(cpgv, disk_no) = cpgd_new(cpgd_fname, CPGD_MAX_BLOCK_NUM);
        if(NULL_PTR == CPGV_DISK_CPGD(cpgv, disk_no))
        {
            sys_log(LOGSTDOUT, "error:cpgv_new: create disk %u failed\n", disk_no);
            __cpgv_free_disk_fname(cpgv, cpgd_fname);
            cpgv_free(cpgv);
            return (NULL_PTR);
        }
        sys_log(LOGSTDOUT, "info:cpgv_new: create disk %s done\n", cpgd_fname);
        
        __cpgv_free_disk_fname(cpgv, cpgd_fname);
        cpgv_add_disk(cpgv, disk_no, CPGD_BLOCK_PAGE_MODEL);
    }    

    return (cpgv);
}

EC_BOOL cpgv_free(CPGV *cpgv)
{
    if(NULL_PTR != cpgv)
    {
        UINT32 disk_num;
        UINT32 disk_no;
        
        /*clean blocks*/
        disk_num = CPGV_PAGE_DISK_MAX_NUM(cpgv);
        for(disk_no = 0; disk_no < disk_num; disk_no ++)
        {
            if(NULL_PTR != CPGV_DISK_CPGD(cpgv, disk_no))
            {
                cpgd_free(CPGV_DISK_CPGD(cpgv, disk_no));
                CPGV_DISK_CPGD(cpgv, disk_no) = NULL_PTR;
            }
        } 
    
        cpgv_hdr_free(cpgv);

        if(ERR_FD != CPGV_FD(cpgv))
        {
            c_file_close(CPGV_FD(cpgv));
            CPGV_FD(cpgv) = ERR_FD;
        }

        if(NULL_PTR != CPGV_FNAME(cpgv))
        {
            safe_free(CPGV_FNAME(cpgv), LOC_CPGV_0005);
            CPGV_FNAME(cpgv) = NULL_PTR;
        }

        free_static_mem(MD_TBD, 0, MM_CPGV, cpgv, LOC_CPGV_0006);
    }

    return (EC_TRUE);
}

CPGV *cpgv_open(const uint8_t *cpgv_fname)
{
    CPGV      *cpgv;
    
    uint16_t  disk_num;
    uint16_t  disk_no;

    UINT32    fsize;
    
    if(EC_FALSE == c_file_access((const char *)cpgv_fname, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cpgv_open: %s not exist\n", cpgv_fname);
        return (NULL_PTR);
    }

    alloc_static_mem(MD_TBD, 0, MM_CPGV, &cpgv, LOC_CPGV_0007);
    if(NULL_PTR == cpgv)
    {
        sys_log(LOGSTDOUT, "error:cpgv_open:malloc cpgv failed\n");
        return (NULL_PTR);
    }

    cpgv_init(cpgv);

    CPGV_FNAME(cpgv) = (uint8_t *)c_str_dup((const char *)cpgv_fname);
    if(NULL_PTR == CPGV_FNAME(cpgv))
    {
        sys_log(LOGSTDOUT, "error:cpgv_open:str dup %s failed\n", cpgv_fname);
        cpgv_close(cpgv);
        return (NULL_PTR);
    }

    CPGV_FD(cpgv) = c_file_open((const char *)cpgv_fname, O_RDWR | O_SYNC , 0666);
    if(ERR_FD == CPGV_FD(cpgv))
    {
        sys_log(LOGSTDOUT, "error:cpgv_open: open %s failed\n", cpgv_fname);
        cpgv_close(cpgv);
        return (NULL_PTR);
    }

    if(EC_FALSE == c_file_size(CPGV_FD(cpgv), &(fsize)))
    {
        sys_log(LOGSTDOUT, "error:cpgv_open: get size of %s failed\n", cpgv_fname);
        cpgv_close(cpgv);
        return (NULL_PTR);
    }
    CPGV_FSIZE(cpgv) = (uint32_t)fsize;

    CPGV_HEADER(cpgv) = cpgv_hdr_open(cpgv);
    if(NULL_PTR == CPGV_HEADER(cpgv))
    {
        sys_log(LOGSTDOUT, "error:cpgv_open: open cpgv header of file %s failed\n", cpgv_fname);
        cpgv_close(cpgv);
        return (NULL_PTR);
    }

    /*init blocks*/
    disk_num = CPGV_PAGE_DISK_MAX_NUM(cpgv);
    for(disk_no = 0; disk_no < disk_num; disk_no ++)
    {
        uint8_t *cpgd_fname;
        cpgd_fname = __cpgv_new_disk_fname(cpgv, disk_no);
        if(NULL_PTR == cpgd_fname)
        {
            sys_log(LOGSTDOUT, "error:cpgv_open: new disk %u fname failed\n", disk_no);
            cpgv_close(cpgv);
            return (NULL_PTR);
        }

        CPGV_DISK_CPGD(cpgv, disk_no) = cpgd_open(cpgd_fname);
        if(NULL_PTR == CPGV_DISK_CPGD(cpgv, disk_no))
        {
            sys_log(LOGSTDOUT, "error:cpgv_open: open disk %u failed\n", disk_no);
            __cpgv_free_disk_fname(cpgv, cpgd_fname);
            cpgv_close(cpgv);
            return (NULL_PTR);
        } 
        __cpgv_free_disk_fname(cpgv, cpgd_fname);
    }

    return (cpgv);
}

EC_BOOL cpgv_close(CPGV *cpgv)
{
    if(NULL_PTR != cpgv)
    {
        UINT32 disk_num;
        UINT32 disk_no;
        
        /*clean blocks*/
        disk_num = CPGV_PAGE_DISK_MAX_NUM(cpgv);
        for(disk_no = 0; disk_no < disk_num; disk_no ++)
        {
            if(NULL_PTR != CPGV_DISK_CPGD(cpgv, disk_no))
            {
                cpgd_close(CPGV_DISK_CPGD(cpgv, disk_no));
                CPGV_DISK_CPGD(cpgv, disk_no) = NULL_PTR;
            }
        } 
    
        cpgv_hdr_close(cpgv);

        if(ERR_FD != CPGV_FD(cpgv))
        {
            c_file_close(CPGV_FD(cpgv));
            CPGV_FD(cpgv) = ERR_FD;
        }

        if(NULL_PTR != CPGV_FNAME(cpgv))
        {
            safe_free(CPGV_FNAME(cpgv), LOC_CPGV_0008);
            CPGV_FNAME(cpgv) = NULL_PTR;
        }

        free_static_mem(MD_TBD, 0, MM_CPGV, cpgv, LOC_CPGV_0009);
    }
    return (EC_TRUE);
}

EC_BOOL cpgv_sync(CPGV *cpgv)
{
    if(NULL_PTR != cpgv)
    {
        UINT32 disk_num;
        UINT32 disk_no;
        
        /*clean blocks*/
        disk_num = CPGV_PAGE_DISK_MAX_NUM(cpgv);
        for(disk_no = 0; disk_no < disk_num; disk_no ++)
        {
            if(NULL_PTR != CPGV_DISK_CPGD(cpgv, disk_no))
            {
                cpgd_sync(CPGV_DISK_CPGD(cpgv, disk_no));
            }
        } 
    
        cpgv_hdr_sync(cpgv);
    }
    return (EC_TRUE);
}
 
/* one disk = 1TB */
EC_BOOL cpgv_init(CPGV *cpgv)
{
    uint16_t disk_no;

    CPGV_FD(cpgv)    = ERR_FD;
    CPGV_FNAME(cpgv) = NULL_PTR;
    CPGV_FSIZE(cpgv) = 0;
    CPGV_HEADER(cpgv)= NULL_PTR;

    for(disk_no = 0; disk_no < CPGV_MAX_DISK_NUM; disk_no ++)
    {
        CPGV_DISK_CPGD(cpgv, disk_no) = NULL_PTR;
    }
    return (EC_TRUE);
}

/*note: cpgv_clean is for not applying mmap*/
void cpgv_clean(CPGV *cpgv)
{
    uint16_t page_model;
    uint16_t disk_no;

    if(ERR_FD != CPGV_FD(cpgv))
    {
        c_file_close(CPGV_FD(cpgv));
        CPGV_FD(cpgv) = ERR_FD;
    }

    if(NULL_PTR != CPGV_FNAME(cpgv))
    {
        safe_free(CPGV_FNAME(cpgv), LOC_CPGV_0010);
        CPGV_FNAME(cpgv) = NULL_PTR;
    }

    if(NULL_PTR == CPGV_HEADER(cpgv))
    {
        return;
    }

    cpgrb_pool_clean(CPGV_PAGE_DISK_CPGRB_POOL(cpgv));

    for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++)
    {
        CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv, page_model) = CPGRB_ERR_POS;
    }

    for(disk_no = 0; disk_no < CPGV_PAGE_DISK_MAX_NUM(cpgv); disk_no ++)
    {
        if(NULL_PTR != CPGV_DISK_CPGD(cpgv, disk_no))
        {           
            safe_free(CPGV_DISK_CPGD(cpgv, disk_no), LOC_CPGV_0011);
            CPGV_DISK_CPGD(cpgv, disk_no) = NULL_PTR;
        }
    }  
    CPGV_PAGE_DISK_MAX_NUM(cpgv)           = 0;

    CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv)     = 0;    
    CPGV_PAGE_4K_MAX_NUM(cpgv)              = 0;
    CPGV_PAGE_4K_USED_NUM(cpgv)             = 0;
    CPGV_PAGE_ACTUAL_USED_SIZE(cpgv)        = 0;

    safe_free(CPGV_HEADER(cpgv), LOC_CPGV_0012);
    CPGV_HEADER(cpgv) = NULL_PTR;
    
    return;    
}

/*add one free block into pool*/
EC_BOOL cpgv_add_disk(CPGV *cpgv, const uint16_t disk_no, const uint16_t page_model)
{    
    if(CPGV_PAGE_DISK_MAX_NUM(cpgv) <= disk_no)
    {
        sys_log(LOGSTDOUT, "error:cpgv_add_disk: disk_no %u overflow where block max num is %u\n", disk_no, CPGV_PAGE_DISK_MAX_NUM(cpgv));
        return (EC_FALSE);
    }

    /*insert disk_no to rbtree*/
    if(CPGRB_ERR_POS == cpgrb_tree_insert_data(CPGV_PAGE_DISK_CPGRB_POOL(cpgv), &(CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv, page_model)), disk_no))
    {
        sys_log(LOGSTDERR, "error:cpgv_add_disk: add disk_no %u to rbtree of page model %u failed\n", disk_no, page_model);
        return (EC_FALSE);
    }

    /*set assignment bitmap*/
    /*set bits of page_model, page_model + 1, ... page_4k_model, the highest bit is for 2k-page which is not supported,clear it!*/
    CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv) |= (uint16_t)(~((1 << page_model) - 1)) & CPGB_MODEL_MASK_ALL;

    return (EC_TRUE);
}

/*del one free block from pool*/
EC_BOOL cpgv_del_disk(CPGV *cpgv, const uint16_t disk_no, const uint16_t page_model)
{
    /*del disk_no from rbtree*/
    if(CPGRB_ERR_POS == cpgrb_tree_delete_data(CPGV_PAGE_DISK_CPGRB_POOL(cpgv), &(CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv, page_model)), disk_no))
    {
        sys_log(LOGSTDERR, "error:cpgv_del_disk: del disk_no %u from rbtree of page model %u failed\n", disk_no, page_model);
        return (EC_FALSE);
    }

    /*clear assignment bitmap if necessary*/
    if(0 == (CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv) & (uint16_t)((1 << page_model) - 1)))/*upper page-model has no page*/
    {
        uint16_t page_model_t;
        
        page_model_t = page_model;            
        while(CPGB_MODEL_NUM > page_model_t
           && EC_TRUE == cpgrb_tree_is_empty(CPGV_PAGE_DISK_CPGRB_POOL(cpgv), CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv, page_model_t))/*this page-model is empty*/
        )
        {
            CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv) &= (uint16_t)~(1 << page_model_t);/*clear bit*/
            page_model_t ++;
        }
    }

    return (EC_TRUE);
}

/*page_model is IN & OUT parameter*/
static EC_BOOL __cpgv_assign_disk(CPGV *cpgv, uint16_t *page_model, uint16_t *disk_no)
{
    uint16_t disk_no_t;
    uint16_t page_model_t;
    uint16_t mask;

    page_model_t = *page_model;

    mask = (uint16_t)((1 << (page_model_t + 1)) - 1);
    if(0 == (CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv) & mask))
    {
        sys_log(LOGSTDERR, "error:__cpgv_assign_disk: page_model = %u where 0 == bitmap %x & mask %x indicates page is not available\n", 
                           page_model_t, CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv), mask);
        return (EC_FALSE);
    }
    
    while(CPGB_MODEL_NUM > page_model_t 
       && EC_TRUE == cpgrb_tree_is_empty(CPGV_PAGE_DISK_CPGRB_POOL(cpgv), CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv, page_model_t))
       )
    {
        page_model_t --;
    }

    if(CPGB_MODEL_NUM <= page_model_t)
    {
        sys_log(LOGSTDERR, "error:__cpgv_assign_disk: no free block available from page model %u\n", *page_model);
        return (EC_FALSE);
    }

    disk_no_t = __cpgv_page_model_first_block(cpgv, page_model_t);
    if(CPGRB_ERR_POS == disk_no_t)
    {
        sys_log(LOGSTDERR, "error:__cpgv_assign_disk: no free block in page model %u\n", page_model_t);
        return (EC_FALSE);
    }  

    (*page_model) = page_model_t;
    (*disk_no)   = disk_no_t;
    
    return (EC_TRUE);
}

EC_BOOL cpgv_new_space(CPGV *cpgv, const uint32_t size, uint16_t *disk_no, uint16_t *block_no, uint16_t *page_4k_no)
{
    CPGD    *cpgd;
    
    uint16_t page_4k_num_need;
    uint16_t page_model;
    uint16_t page_model_t;
    uint16_t e;
    uint16_t t;
    uint16_t page_4k_no_t;/*the page No. in certain page model*/
  
    uint16_t disk_no_t;
    uint16_t block_no_t;
    
    uint16_t pgd_assign_bitmap_old;
    uint16_t pgd_assign_bitmap_new;    

    CPGV_ASSERT(0 < size);

    if(CPGB_CACHE_MAX_BYTE_SIZE < size)
    {
        sys_log(LOGSTDERR, "error:cpgv_new_space: the expected size %u overflow\n", size);
        return (EC_FALSE);
    }

    page_4k_num_need = (uint16_t)((size + CPGB_PAGE_4K_BYTE_SIZE - 1) >> CPGB_PAGE_4K_BIT_SIZE);
    sys_log(LOGSTDNULL, "[DEBUG] cpgv_new_space: size = %u ==> page_4k_num_need = %u\n", size, page_4k_num_need);

    /*find a page model which can accept the page_4k_num_need 4k-pages */
    /*and then split the left space into page model with smaller size  */ 

    CPGV_ASSERT(0 == (CPGB_PAGE_4K_ERR_HI_BITS_MASK & page_4k_num_need));
    
    /*check bits of page_4k_num_need and determine the page_model*/
    e = CPGB_PAGE_4K_HI_BIT_MASK;
    for(t = page_4k_num_need, page_model = 0; 0 == (t & e); t <<= 1, page_model ++)
    {
        /*do nothing*/
    }
    sys_log(LOGSTDNULL, "[DEBUG] cpgv_new_space: t = 0x%x, page_model = %u, e = 0x%x, t << 1 is 0x%x\n", t, page_model, e, (t << 1));

    if(CPGB_PAGE_4K_LO_BITS_MASK & t)
    {
        page_model --;/*upgrade page_model one level*/
    }

    sys_log(LOGSTDNULL, "[DEBUG] cpgv_new_space: page_4k_num_need = %u ==> page_model = %u (has %u 4k-pages )\n", 
                       page_4k_num_need, page_model, (uint16_t)(1 << (CPGB_MODEL_NUM - 1 - page_model)));

    if(EC_FALSE == __cpgv_assign_disk(cpgv, &page_model, &disk_no_t))
    {
        sys_log(LOGSTDERR, "error:cpgv_new_space: assign one disk from page model %u failed\n", page_model);
        return (EC_FALSE);
    }

    cpgd = CPGV_DISK_NODE(cpgv, disk_no_t);
    pgd_assign_bitmap_old = CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd);    
    
    if(EC_FALSE == cpgd_new_space(cpgd, size, &block_no_t, &page_4k_no_t))
    {
        sys_log(LOGSTDERR, "error:cpgv_new_space: free one page from page model %u block %u failed\n", page_model, disk_no_t);
        return (EC_FALSE);
    }

    pgd_assign_bitmap_new = CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd);

    sys_log(LOGSTDOUT, "[DEBUG] cpgv_new_space: disk_no_t %u: pgd bitmap %x => %x\n", disk_no_t, pgd_assign_bitmap_old, pgd_assign_bitmap_new);

    /*pgd_assign_bitmap changes may make pgv_assign_bitmap changes*/
    if(pgd_assign_bitmap_new != pgd_assign_bitmap_old)
    {        
        sys_log(LOGSTDOUT, "[DEBUG] cpgv_new_space: before delete disk_no_t %u: pgb bitmap %s, pgv assign bitmap %s\n", 
                            disk_no_t, 
                            c_uint16_to_bin_str(CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd)), 
                            c_uint16_to_bin_str(CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv)));

        cpgv_del_disk(cpgv, disk_no_t, page_model);
        
        sys_log(LOGSTDOUT, "[DEBUG] cpgv_new_space: after  delete disk_no_t %u: pgb bitmap %s, pgv assign bitmap %s\n", 
                            disk_no_t, 
                            c_uint16_to_bin_str(CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd)), 
                            c_uint16_to_bin_str(CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv)));

        sys_log(LOGSTDOUT, "[DEBUG] cpgv_new_space: disk_no_t %u: max pages %u, used pages %u\n", 
                            disk_no_t, CPGD_PAGE_4K_MAX_NUM(cpgd), CPGD_PAGE_4K_USED_NUM(cpgd));
                            
        if(EC_FALSE == cpgd_is_full(cpgd))
        {
            page_model_t = page_model;
            while(CPGB_MODEL_NUM > page_model_t 
               && 0 == (pgd_assign_bitmap_new & (uint16_t)(1 << page_model_t))
               )
            {
                 page_model_t ++;
            }
#if 0            
            if(CPGB_MODEL_NUM <= page_model_t)
            {
                cpgv_print(LOGSTDOUT, cpgv);
                cpgd_print(LOGSTDOUT, cpgd);
                CPGV_ASSERT(CPGB_MODEL_NUM > page_model_t);
            }
#endif            
            CPGV_ASSERT(CPGB_MODEL_NUM > page_model_t);
            
            sys_log(LOGSTDOUT, "[DEBUG] cpgv_new_space: page_model %u, page_model_t %u\n", page_model, page_model_t);
            cpgv_add_disk(cpgv, disk_no_t, page_model_t);
            sys_log(LOGSTDOUT, "[DEBUG] cpgv_new_space: disk_no_t %u: pgb bitmap %s, pgv assign bitmap %s\n", 
                                disk_no_t, 
                                c_uint16_to_bin_str(CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd)), 
                                c_uint16_to_bin_str(CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv)));
        }
        else
        {
            /*do nothing*/
        }
    }
    
    (*disk_no)    = disk_no_t;
    (*block_no)   = block_no_t;
    (*page_4k_no) = page_4k_no_t;

    CPGV_PAGE_4K_USED_NUM(cpgv)      += page_4k_num_need;
    CPGV_PAGE_ACTUAL_USED_SIZE(cpgv) += size;

#if 0
    /*for debug only*/
    if(EC_FALSE == cpgv_check(cpgv))
    {
        cpgv_free_space(cpgv, disk_no_t, block_no_t, page_4k_no_t, size);
         CPGV_ASSERT(EC_TRUE == cpgv_check(cpgv));
    }
#endif    

    CPGV_ASSERT(EC_TRUE == cpgv_check(cpgv));
    sys_log(LOGSTDOUT, "[DEBUG] cpgv_new_space: pgv check passed\n");
    
    sys_log(LOGSTDNULL, "[DEBUG] cpgv_new_space: pgv_page_4k_used_num %u due to increment %u\n", 
                        CPGV_PAGE_4K_USED_NUM(cpgv), page_4k_num_need);
    sys_log(LOGSTDNULL, "[DEBUG] cpgv_new_space: pgv_actual_used_size %u due to increment %u\n", 
                        CPGV_PAGE_ACTUAL_USED_SIZE(cpgv), size);

    return (EC_TRUE);
}

EC_BOOL cpgv_free_space(CPGV *cpgv, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_4k_no, const uint32_t size)
{
    CPGD    *cpgd;
    
    uint16_t page_4k_num_used;    
    
    uint16_t pgd_assign_bitmap_old;
    uint16_t pgd_assign_bitmap_new;   

    CPGV_ASSERT(0 < size);
    
    if(CPGB_CACHE_MAX_BYTE_SIZE < size)
    {
        sys_log(LOGSTDERR, "error:cpgv_free_space: invalid size %u due to overflow\n", size);
        return (EC_FALSE);
    }

    cpgd = CPGV_DISK_NODE(cpgv, disk_no);
    pgd_assign_bitmap_old = CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd);
    
    if(EC_FALSE == cpgd_free_space(cpgd, block_no, page_4k_no, size))
    {
        sys_log(LOGSTDOUT, "error:cpgv_free_space: disk_no %u free space of block_no %u, page_4k_no %u, size %u failed\n", 
                           disk_no, block_no, page_4k_no, size);
        return (EC_FALSE);
    }

    pgd_assign_bitmap_new = CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd);

    if(pgd_assign_bitmap_new != pgd_assign_bitmap_old)
    {
        uint16_t page_model_old;
        uint16_t page_model_new;     
        
        page_model_old = __cpgv_page_model_get(cpgv, pgd_assign_bitmap_old);
        page_model_new = __cpgv_page_model_get(cpgv, pgd_assign_bitmap_new);

        if(CPGB_MODEL_NUM > page_model_old)
        {
            cpgv_del_disk(cpgv, disk_no, page_model_old);
        }
        cpgv_add_disk(cpgv, disk_no, page_model_new);
    }

    page_4k_num_used = (uint16_t)((size + CPGB_PAGE_4K_BYTE_SIZE - 1) >> CPGB_PAGE_4K_BIT_SIZE);
 
    CPGV_PAGE_4K_USED_NUM(cpgv)      -= page_4k_num_used;
    CPGV_PAGE_ACTUAL_USED_SIZE(cpgv) -= size;
    
    sys_log(LOGSTDNULL, "[DEBUG] cpgv_free_space: pgv_page_4k_used_num %u due to decrement %u\n", 
                        CPGV_PAGE_4K_USED_NUM(cpgv), page_4k_num_used);
    sys_log(LOGSTDNULL, "[DEBUG] cpgv_free_space: pgv_actual_used_size %u due to decrement %u\n", 
                        CPGV_PAGE_ACTUAL_USED_SIZE(cpgv), size);

    return (EC_TRUE);    
}

EC_BOOL cpgv_is_full(const CPGV *cpgv)
{
    if(CPGV_PAGE_4K_USED_NUM(cpgv) == CPGV_PAGE_4K_MAX_NUM(cpgv))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cpgv_is_empty(const CPGV *cpgv)
{
    if(0 == CPGV_PAGE_4K_USED_NUM(cpgv) && 0 < CPGV_PAGE_4K_MAX_NUM(cpgv))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cpgv_flush_size(const CPGV *cpgv, UINT32 *size)
{
    uint16_t disk_no;

    cpgv_hdr_flush_size(CPGV_HEADER(cpgv), size);    

    for(disk_no = 0; disk_no < CPGV_PAGE_DISK_MAX_NUM(cpgv); disk_no ++)
    {
        cpgd_flush_size(CPGV_DISK_NODE(cpgv, disk_no), size);
    }
    return (EC_TRUE);
}

EC_BOOL cpgv_flush(const CPGV *cpgv, int fd, UINT32 *offset)
{
    uint16_t disk_no;

    DEBUG(UINT32 offset_saved = *offset;);

    /*flush CPGV_HEADER*/
    if(EC_FALSE == cpgv_hdr_flush(CPGV_HEADER(cpgv), fd, offset))
    {
        sys_log(LOGSTDOUT, "error:cpgv_flush: flush CPGV_HEADER at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    DEBUG(CPGV_ASSERT(sizeof(CPGV_HDR) == (*offset) - offset_saved));

    /*flush CPGV_DISK_NODE table*/
    for(disk_no = 0; disk_no < CPGV_PAGE_DISK_MAX_NUM(cpgv); disk_no ++)
    {
        if(EC_FALSE == cpgd_flush(CPGV_DISK_NODE(cpgv, disk_no), fd, offset))
        {
            sys_log(LOGSTDOUT, "error:cpgv_flush: flush CPGV_DISK_NODE of disk_no %u at offset %u of fd %d failed\n", 
                                disk_no, (*offset), fd);
            return (EC_FALSE);
        }
    }
    
    return (EC_TRUE);
}

EC_BOOL cpgv_load(CPGV *cpgv, int fd, UINT32 *offset)
{
    uint16_t disk_no;

    if(NULL_PTR == CPGV_HEADER(cpgv))
    {
        CPGV_HEADER(cpgv) = safe_malloc(sizeof(CPGV_HDR), LOC_CPGV_0013);
        if(NULL_PTR == CPGV_HEADER(cpgv))
        {
            sys_log(LOGSTDOUT, "error:cpgv_load: malloc CPGV_HDR failed\n");
            return (EC_FALSE);
        }
    }

    /*load rbtree pool*/
    if(EC_FALSE == cpgv_hdr_load(CPGV_HEADER(cpgv), fd, offset))
    {
        sys_log(LOGSTDOUT, "error:cpgv_load: load CPGV_HEADER at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*load CPGV_DISK_NODE table*/
    for(disk_no = 0; disk_no < CPGV_PAGE_DISK_MAX_NUM(cpgv); disk_no ++)
    {
        if(NULL_PTR == CPGV_DISK_NODE(cpgv, disk_no))
        {
            CPGV_DISK_CPGD(cpgv, disk_no) = safe_malloc(sizeof(CPGD), LOC_CPGV_0014);
            if(NULL_PTR == CPGV_DISK_CPGD(cpgv, disk_no))
            {
                sys_log(LOGSTDOUT, "error:cpgv_load: malloc block %u failed\n", disk_no);
                return (EC_FALSE);
            }
        }
        if(EC_FALSE == cpgd_load(CPGV_DISK_CPGD(cpgv, disk_no), fd, offset))
        {
            sys_log(LOGSTDOUT, "error:cpgv_load: load CPGV_DISK_NODE of disk_no %u at offset %u of fd %d failed\n", 
                                disk_no, (*offset), fd);
            return (EC_FALSE);
        }
    }    
    return (EC_TRUE);
}

EC_BOOL cpgv_check(const CPGV *cpgv)
{
    uint16_t  pgv_assign_bitmap;
    uint16_t  pgd_assign_bitmap;/*all pgd's bitmap*/
    uint16_t  disk_no;
    uint16_t  disk_num;

    uint64_t  pgv_actual_used_size;
    uint64_t  pgd_actual_used_size;/*all pgd's used size*/

    uint64_t  pgv_page_4k_max_num;
    uint64_t  pgd_page_4k_max_num;/*all pgd's 4k-page max num*/

    uint64_t  pgv_page_4k_used_num;
    uint64_t  pgd_page_4k_used_num;/*all pgd's 4k-page used num*/

    pgv_assign_bitmap    = CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv);
    pgv_actual_used_size = CPGV_PAGE_ACTUAL_USED_SIZE(cpgv);
    pgv_page_4k_max_num  = CPGV_PAGE_4K_MAX_NUM(cpgv);
    pgv_page_4k_used_num = CPGV_PAGE_4K_USED_NUM(cpgv);
    disk_num = CPGV_PAGE_DISK_MAX_NUM(cpgv);

    pgd_assign_bitmap    = 0;
    pgd_actual_used_size = 0;
    pgd_page_4k_max_num  = 0;
    pgd_page_4k_used_num = 0;
    
    for(disk_no = 0; disk_no < disk_num; disk_no ++)
    {
        pgd_assign_bitmap    |= CPGD_PAGE_MODEL_ASSIGN_BITMAP(CPGV_DISK_NODE(cpgv, disk_no));
        pgd_actual_used_size += CPGD_PAGE_ACTUAL_USED_SIZE(CPGV_DISK_NODE(cpgv, disk_no));
        pgd_page_4k_max_num  += CPGD_PAGE_4K_MAX_NUM(CPGV_DISK_NODE(cpgv, disk_no));
        pgd_page_4k_used_num += CPGD_PAGE_4K_USED_NUM(CPGV_DISK_NODE(cpgv, disk_no));
    }

    if(pgv_assign_bitmap != pgd_assign_bitmap)
    {
        sys_log(LOGSTDOUT, "error:cpgv_check: inconsistent bitmap: pgv_assign_bitmap = %s, pgd_assign_bitmap = %s\n",
                           c_uint16_to_bin_str(pgv_assign_bitmap), c_uint16_to_bin_str(pgd_assign_bitmap));
        return (EC_FALSE);
    }

    if(pgv_actual_used_size != pgd_actual_used_size)
    {
        sys_log(LOGSTDOUT, "error:cpgv_check: inconsistent actual used size: pgv_actual_used_size = %llu, pgd_actual_used_size = %llu\n",
                            pgv_actual_used_size, pgd_actual_used_size);
        return (EC_FALSE);
    }

    if(pgv_page_4k_max_num != pgd_page_4k_max_num)
    {
        sys_log(LOGSTDOUT, "error:cpgv_check: inconsistent 4k-page max num: pgv_page_4k_max_num = %llu, pgd_page_4k_max_num = %llu\n",
                            pgv_page_4k_max_num, pgd_page_4k_max_num);
        return (EC_FALSE);
    }

    if(pgv_page_4k_used_num != pgd_page_4k_used_num)
    {
        sys_log(LOGSTDOUT, "error:cpgv_check: inconsistent 4k-page used num: pgv_page_4k_used_num = %llu, pgd_page_4k_used_num = %llu\n",
                            pgv_page_4k_used_num, pgd_page_4k_used_num);
        return (EC_FALSE);
    }    
    
    /*check block table*/
    for(disk_no = 0; disk_no < CPGV_PAGE_DISK_MAX_NUM(cpgv); disk_no ++)
    {
        if(EC_FALSE == cpgd_check(CPGV_DISK_NODE(cpgv, disk_no)))
        {
            sys_log(LOGSTDOUT, "error:cpgv_check: check CPGV_DISK_NODE of disk_no %u failed\n", disk_no);
            return (EC_FALSE);
        }
    }
    sys_log(LOGSTDOUT, "cpgv_check: cpgv %p check passed\n", cpgv);
    return (EC_TRUE);
}

void cpgv_print(LOG *log, const CPGV *cpgv)
{
    uint16_t  page_model;
    
    REAL      used_size;
    REAL      occupied_size;
    REAL      ratio;

    CPGV_ASSERT(NULL_PTR != cpgv);

    //cpgrb_pool_print(log, CPGV_PAGE_DISK_CPGRB_POOL(cpgv));
    
    for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++)
    {
        //cpgrb_tree_print(log, CPGV_PAGE_DISK_CPGRB_POOL(cpgv), CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv, page_model));
        //sys_log(log, "----------------------------------------------------------\n");
        sys_log(log, "cpgv_print: page_model %u, block root_pos %u\n", page_model, CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv, page_model));
    }

    used_size     = (0.0 + CPGV_PAGE_ACTUAL_USED_SIZE(cpgv));
    occupied_size = (0.0 + (((uint64_t)CPGV_PAGE_4K_USED_NUM(cpgv)) << CPGB_PAGE_4K_BIT_SIZE));
    ratio         = (EC_TRUE == REAL_ISZERO(ERR_MODULE_ID, occupied_size) ? 0.0 : (used_size / occupied_size));
    
    sys_log(log, "cpgv_print: cpgv %p, disk num %u, 4k-page max num %llu, 4k-page used num %llu, used size %llu, ratio %.2f\n", 
                 cpgv, 
                 CPGV_PAGE_DISK_MAX_NUM(cpgv), 
                 CPGV_PAGE_4K_MAX_NUM(cpgv),
                 CPGV_PAGE_4K_USED_NUM(cpgv), 
                 CPGV_PAGE_ACTUAL_USED_SIZE(cpgv),
                 ratio
                 );

    sys_log(log, "cpgv_print: cpgv %p, assign bitmap %s \n", 
                 cpgv, 
                 c_uint16_to_bin_str(CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv))
                 );
    for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++)
    {
        if(CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv) & (1 << page_model))
        {
            sys_log(log, "cpgv_print: cpgv %p, model %u has page to assign\n", cpgv, page_model);
        }
        else
        {
            sys_log(log, "cpgv_print: cpgv %p, model %u no  page to assign\n", cpgv, page_model);
        }
    }

    if(1)
    {
        uint16_t  disk_no;
        for(disk_no = 0; disk_no < CPGV_PAGE_DISK_MAX_NUM(cpgv); disk_no ++)
        {
            sys_log(log, "cpgv_print: disk %u is\n", disk_no);
            cpgd_print(log, CPGV_DISK_NODE(cpgv, disk_no));
        }
    }

    return;    
}

/* ---- debug ---- */
EC_BOOL cpgv_debug_cmp(const CPGV *cpgv_1st, const CPGV *cpgv_2nd)
{
    uint16_t page_model;
    uint16_t disk_no;

    /*cpgrb pool*/
    if(EC_FALSE == cpgrb_debug_cmp(CPGV_PAGE_DISK_CPGRB_POOL(cpgv_1st), CPGV_PAGE_DISK_CPGRB_POOL(cpgv_2nd)))
    {
        sys_log(LOGSTDOUT, "error:cpgv_debug_cmp: inconsistent cpgrb pool\n");
        return (EC_FALSE);
    }

    /*root pos*/
    for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++ )
    {
        uint16_t root_pos_1st;
        uint16_t root_pos_2nd;

        root_pos_1st = CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv_1st, page_model);
        root_pos_2nd = CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv_2nd, page_model);

        if(root_pos_1st != root_pos_2nd)
        {
            sys_log(LOGSTDERR, "error:cpgv_debug_cmp: inconsistent root_pos: %u != %u at page_model %u\n", 
                                root_pos_1st, root_pos_2nd, page_model);
            return (EC_FALSE);
        }     
    }

    /*assign bitmap*/
    if(CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv_1st) != CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv_1st))
    {
        sys_log(LOGSTDERR, "error:cpgv_debug_cmp: inconsistent CPGV_PAGE_MODEL_ASSIGN_BITMAP: %u != %u\n", 
                            CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv_1st), CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv_2nd));
        return (EC_FALSE);
    }

    /*block max num*/
    if(CPGV_PAGE_DISK_MAX_NUM(cpgv_1st) != CPGV_PAGE_DISK_MAX_NUM(cpgv_1st))
    {
        sys_log(LOGSTDERR, "error:cpgv_debug_cmp: inconsistent CPGV_PAGE_DISK_MAX_NUM: %u != %u\n", 
                            CPGV_PAGE_DISK_MAX_NUM(cpgv_1st), CPGV_PAGE_DISK_MAX_NUM(cpgv_2nd));
        return (EC_FALSE);
    }    
    
    /*4k-page max num*/
    if(CPGV_PAGE_4K_MAX_NUM(cpgv_1st) != CPGV_PAGE_4K_MAX_NUM(cpgv_1st))
    {
        sys_log(LOGSTDERR, "error:cpgv_debug_cmp: inconsistent CPGV_PAGE_4K_MAX_NUM: %u != %u\n", 
                            CPGV_PAGE_4K_MAX_NUM(cpgv_1st), CPGV_PAGE_DISK_MAX_NUM(cpgv_2nd));
        return (EC_FALSE);
    }

    /*4k-page used num*/
    if(CPGV_PAGE_4K_USED_NUM(cpgv_1st) != CPGV_PAGE_4K_USED_NUM(cpgv_1st))
    {
        sys_log(LOGSTDERR, "error:cpgv_debug_cmp: inconsistent CPGV_PAGE_4K_USED_NUM: %u != %u\n", 
                            CPGV_PAGE_4K_USED_NUM(cpgv_1st), CPGV_PAGE_4K_USED_NUM(cpgv_2nd));
        return (EC_FALSE);
    }

    /*4k-page actual used bytes num*/
    if(CPGV_PAGE_ACTUAL_USED_SIZE(cpgv_1st) != CPGV_PAGE_ACTUAL_USED_SIZE(cpgv_1st))
    {
        sys_log(LOGSTDERR, "error:cpgv_debug_cmp: inconsistent CPGV_PAGE_ACTUAL_USED_SIZE: %u != %u\n", 
                            CPGV_PAGE_ACTUAL_USED_SIZE(cpgv_1st), CPGV_PAGE_ACTUAL_USED_SIZE(cpgv_2nd));
        return (EC_FALSE);
    }    

    /*block cpgd*/
    for(disk_no = 0; disk_no < CPGV_PAGE_DISK_MAX_NUM(cpgv_1st); disk_no ++)
    {
        if(EC_FALSE == cpgd_debug_cmp(CPGV_DISK_NODE(cpgv_1st, disk_no), CPGV_DISK_NODE(cpgv_2nd, disk_no)))
        {
            sys_log(LOGSTDOUT, "error:cpgv_debug_cmp: inconsistent CPGV_DISK_NODE at disk_no %u\n", disk_no);
            return (EC_FALSE);
        }
    }
    
    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

