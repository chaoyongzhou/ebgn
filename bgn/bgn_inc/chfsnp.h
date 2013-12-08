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

#ifndef _CHFSNP_H
#define _CHFSNP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "type.h"
#include "log.h"

#include "cvector.h"
#include "croutine.h"
#include "cstring.h"

#include "cbloom.h"
#include "chashalgo.h"
#include "chfsnprb.h"

#define CHFSNP_KEY_MAX_SIZE             ( 16)  /*max len of file or dir seg name*/

#define CHFSNP_004K_MODEL   ((uint8_t) 0)
#define CHFSNP_064K_MODEL   ((uint8_t) 1)
#define CHFSNP_001M_MODEL   ((uint8_t) 2)
#define CHFSNP_002M_MODEL   ((uint8_t) 3)
#define CHFSNP_128M_MODEL   ((uint8_t) 4)
#define CHFSNP_256M_MODEL   ((uint8_t) 5)
#define CHFSNP_512M_MODEL   ((uint8_t) 6)
#define CHFSNP_001G_MODEL   ((uint8_t) 7)
#define CHFSNP_002G_MODEL   ((uint8_t) 8)
#define CHFSNP_004G_MODEL   ((uint8_t) 9)

#define CHFSNP_FILE_REPLICA_MAX_NUM     ((uint32_t) 1)  /*max num of supported replicas up to*/
//#define CHFSNP_BUCKET_MAX_NUM           ((uint32_t)32)
//#define CHFSNP_ERR_BUCKET               (CHFSNPRB_ERR_POS)

#define CHFSNP_ITEM_STAT_IS_NOT_USED    ((uint32_t) 0x1)  /*4 bits*/
#define CHFSNP_ITEM_STAT_IS_USED        ((uint32_t) 0x8)

typedef struct
{
    uint16_t    rsvd2;          /*tcid: not used yet*/
    uint16_t    disk_no;        /*local disk_no*/
    uint16_t    block_no;       /*block_no in above disk*/
    uint16_t    page_no;        /*page_no in above block*/    
}CHFSNP_INODE;

#define CHFSNP_INODE_DISK_NO(chfsnp_inode)           ((chfsnp_inode)->disk_no)
#define CHFSNP_INODE_BLOCK_NO(chfsnp_inode)          ((chfsnp_inode)->block_no)
#define CHFSNP_INODE_PAGE_NO(chfsnp_inode)           ((chfsnp_inode)->page_no)

typedef struct
{
    uint32_t      file_size;    /*data/value length < 64M = 2^26B*/
    uint32_t      file_replica_num;

    CHFSNP_INODE  inodes[ CHFSNP_FILE_REPLICA_MAX_NUM ];
}CHFSNP_FNODE;/*16B*/

#define CHFSNP_FNODE_FILESZ(chfsnp_fnode)        ((chfsnp_fnode)->file_size)
#define CHFSNP_FNODE_REPNUM(chfsnp_fnode)        ((chfsnp_fnode)->file_replica_num)
#define CHFSNP_FNODE_INODES(chfsnp_fnode)        ((chfsnp_fnode)->inodes)
#define CHFSNP_FNODE_INODE(chfsnp_fnode, idx)    (&((chfsnp_fnode)->inodes[ (idx) ]))

#define CHFSNP_FNODE_INODE_DISK_NO(chfsnp_fnode, idx)    CHFSNP_INODE_DISK_NO(CHFSNP_FNODE_INODE(chfsnp_fnode, idx))
#define CHFSNP_FNODE_INODE_BLOCK_NO(chfsnp_fnode, idx)   CHFSNP_INODE_BLOCK_NO(CHFSNP_FNODE_INODE(chfsnp_fnode, idx))
#define CHFSNP_FNODE_INODE_PAGE_NO(chfsnp_fnode, idx)    CHFSNP_INODE_PAGE_NO(CHFSNP_FNODE_INODE(chfsnp_fnode, idx))

typedef struct
{   
    CHFSNPRB_NODE   rb_node;/*16B*/
    
    /*ctime_t is 32 bits for 32bit OS, 64 bits for 64bit OS*/
    ctime_t       c_time; /*create time       */
    ctime_t       r_time; /*reference time    */
    ctime_t       e_time; /*expire time       */
    ctime_t       m_time; /*last modified time*/

    uint16_t      r_count;/*reader number or reference number*/
    uint16_t      rsvd2;
    uint32_t      rsvd3:20;
    uint32_t      stat :4;
    uint32_t      k_len:8;
        
    uint8_t       rsvd4;
    uint8_t       key[ CHFSNP_KEY_MAX_SIZE ];  /* file name or hash/digest value*/

    union
    {
        CHFSNP_FNODE fnode;
    }u;/*8B*/
} CHFSNP_ITEM;

#define CHFSNP_ITEM_RB_NODE(chfsnp_item)          (&((chfsnp_item)->rb_node))
#define CHFSNP_ITEM_C_TIME(chfsnp_item)           ((chfsnp_item)->c_time)
#define CHFSNP_ITEM_R_TIME(chfsnp_item)           ((chfsnp_item)->r_time)
#define CHFSNP_ITEM_E_TIME(chfsnp_item)           ((chfsnp_item)->e_time)
#define CHFSNP_ITEM_M_TIME(chfsnp_item)           ((chfsnp_item)->m_time)
#define CHFSNP_ITEM_STAT(chfsnp_item)             ((chfsnp_item)->stat)
#define CHFSNP_ITEM_K_LEN(chfsnp_item)            ((chfsnp_item)->k_len)
#define CHFSNP_ITEM_KEY(chfsnp_item)              ((chfsnp_item)->key)
#define CHFSNP_ITEM_R_COUNT(chfsnp_item)          ((chfsnp_item)->r_count)
#define CHFSNP_ITEM_FNODE(chfsnp_item)            (&((chfsnp_item)->u.fnode))
#define CHFSNP_ITEM_F_SIZE(chfsnp_item)           (CHFSNP_FNODE_FILESZ(CHFSNP_ITEM_FNODE(chfsnp_item)))

/*get CHFSNP_ITEM from CHFSNPRB_NODE*/
#define CHFSNP_RB_NODE_ITEM(chfsnprb_node)        ((NULL_PTR == (chfsnprb_node)) ? NULL_PTR : \
    ((CHFSNP_ITEM *)((char *)(chfsnprb_node)-(unsigned long)(&((CHFSNP_ITEM *)0)->rb_node))))

/*item max num = file size / sizeof(CHFSNP_ITEM) where sizeof(CHFSNP_ITEM) = 128B = 2^7*/
#define CHFSNP_ITEM_BIT_SIZE             (7)
#define CHFSNP_004K_CFG_FILE_SIZE        ((uint32_t)(1 << 12))
#define CHFSNP_004K_CFG_ITEM_MAX_NUM     (CHFSNP_004K_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

#define CHFSNP_064K_CFG_FILE_SIZE        ((uint32_t)(1 << 16))
#define CHFSNP_064K_CFG_ITEM_MAX_NUM     (CHFSNP_064K_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

#define CHFSNP_001M_CFG_FILE_SIZE        ((uint32_t)(1 << 20))
#define CHFSNP_001M_CFG_ITEM_MAX_NUM     (CHFSNP_001M_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

#define CHFSNP_002M_CFG_FILE_SIZE        ((uint32_t)(1 << 21))
#define CHFSNP_002M_CFG_ITEM_MAX_NUM     (CHFSNP_002M_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

#define CHFSNP_128M_CFG_FILE_SIZE        ((uint32_t)(1 << 27))
#define CHFSNP_128M_CFG_ITEM_MAX_NUM     (CHFSNP_128M_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

#define CHFSNP_256M_CFG_FILE_SIZE        ((uint32_t)(1 << 28))
#define CHFSNP_256M_CFG_ITEM_MAX_NUM     (CHFSNP_256M_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

#define CHFSNP_512M_CFG_FILE_SIZE        ((uint32_t)(1 << 29))
#define CHFSNP_512M_CFG_ITEM_MAX_NUM     (CHFSNP_512M_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

#define CHFSNP_001G_CFG_FILE_SIZE        ((uint32_t)(1 << 30))
#define CHFSNP_001G_CFG_ITEM_MAX_NUM     (CHFSNP_001G_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

#define CHFSNP_002G_CFG_FILE_SIZE        ((uint32_t)(1 << 31))
#define CHFSNP_002G_CFG_ITEM_MAX_NUM     (CHFSNP_002G_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

#if (64 == WORDSIZE)
#define CHFSNP_004G_CFG_FILE_SIZE        ((uint32_t)(1 << 32))
#define CHFSNP_004G_CFG_ITEM_MAX_NUM     (CHFSNP_004G_CFG_FILE_SIZE >> CHFSNP_ITEM_BIT_SIZE)

/*due to offset is defined as 32bit integer, here cannot support more than 4G file*/
#endif/*(64 == WORDSIZE)*/

typedef struct
{
    char    *mode_str;
    uint32_t file_size;
    uint32_t item_max_num;
}CHFSNP_CFG;

#define CHFSNP_CFG_MOD_STR(chfsnp_cfg)                ((chfsnp_cfg)->mode_str)
#define CHFSNP_CFG_FILE_SIZE(chfsnp_cfg)              ((chfsnp_cfg)->file_size)
#define CHFSNP_CFG_ITEM_MAX_NUM(chfsnp_cfg)           ((chfsnp_cfg)->item_max_num)

#define CHFSNP_ERR_MODEL             ((uint32_t)0xF)  /*4 bits*/

#define CHFSNP_O_RDONLY              ((uint32_t)O_RDONLY)
#define CHFSNP_O_WRONLY              ((uint32_t)O_WRONLY)
#define CHFSNP_O_RDWR                ((uint32_t)O_RDWR  )
#define CHFSNP_O_CREATE              ((uint32_t)O_CREAT )

/*bitmap*/
#define CHFSNP_STATE_NOT_DIRTY       ((uint8_t)0x00)
#define CHFSNP_STATE_DIRTY           ((uint8_t)0x01)


#define CHFSNP_ERR_ID                     ((uint32_t)0xFFFFFFFF)


/*each np own one header*/
typedef struct
{
    uint32_t       np_id;               /*chfsnp id              */    
    uint8_t        model;               /*chfsnp model           */    
    uint8_t        rsvd1;                    
    uint8_t        chash_algo_1st_id ;  /*first hash algo func id : used to compute bucket pos in dnode   */
    uint8_t        chash_algo_2nd_id;   /*second hash algo func id: used to compute chfsnprb_node hash data*/   

    uint32_t       bucket_max_num;
    uint32_t       bucket_offset;       /*bucket start position, offset from CHFSNP_HEADER*/
    CHFSNPRB_POOL  pool;                /*pool of CHFSNP_ITEM, CHFSNP_ITEM head must be CHFSNPRB_NODE*/    
} CHFSNP_HEADER;

#define CHFSNP_HEADER_NP_ID(chfsnp_header)              ((chfsnp_header)->np_id)
#define CHFSNP_HEADER_MODEL(chfsnp_header)              ((chfsnp_header)->model)

#define CHFSNP_HEADER_1ST_CHASH_ALGO_ID(chfsnp_header)  ((chfsnp_header)->chash_algo_1st_id)
#define CHFSNP_HEADER_2ND_CHASH_ALGO_ID(chfsnp_header)  ((chfsnp_header)->chash_algo_2nd_id)

#define CHFSNP_HEADER_ITEMS_POOL(chfsnp_header)         (&((chfsnp_header)->pool))
#define CHFSNP_HEADER_ITEMS_MAX_NUM(chfsnp_header)      (CHFSNPRB_POOL_NODE_MAX_NUM(CHFSNP_HEADER_ITEMS_POOL(chfsnp_header)))
#define CHFSNP_HEADER_ITEMS_USED_NUM(chfsnp_header)     (CHFSNPRB_POOL_NODE_USED_NUM(CHFSNP_HEADER_ITEMS_POOL(chfsnp_header)))

#define CHFSNP_HEADER_BUCKET_MAX_NUM(chfsnp_header)     ((chfsnp_header)->bucket_max_num)
#define CHFSNP_HEADER_BUCKET_OFFSET(chfsnp_header)      ((chfsnp_header)->bucket_offset)
//#define CHFSNP_HEADER_BUCKET_ADDRESS(chfsnp_header)     ((uint32_t *)(((uint8_t *)(chfsnp_header)) + CHFSNP_HEADER_BUCKET_OFFSET(chfsnp_header)))
//#define CHFSNP_HEADER_BUCKET(chfsnp_header, bucket_pos) (CHFSNP_HEADER_BUCKET_ADDRESS(chfsnp_header)[bucket_pos])

typedef struct
{
    int              fd;         /* hfs namespace fd  */
    uint32_t         fsize;
    
    uint8_t         *fname;
    
    uint16_t         state;
    uint16_t         rsvd;
    uint32_t         reader_num; /* current reader num*/    
    
    CROUTINE_RWLOCK  crwlock;       /* bucket crwlock*/
    CHFSNP_HEADER   *header;        /* hashdb header */
    uint32_t        *bucket_addr;

    CHASH_ALGO       chash_algo_1st;       /* hash algo for hash bucket              : used to compute bucket pos in dnode   */
    CHASH_ALGO       chash_algo_2nd;       /* hash algo for rbtree in the hash bucket: used to compute chfsnprb_node hash data*/   
} CHFSNP;

#define CHFSNP_FD(chfsnp)                     ((chfsnp)->fd)
#define CHFSNP_FSIZE(chfsnp)                  ((chfsnp)->fsize)
#define CHFSNP_FNAME(chfsnp)                  ((chfsnp)->fname)
#define CHFSNP_READER_NUM(chfsnp)             ((chfsnp)->reader_num)
#define CHFSNP_STATE(chfsnp)                  ((chfsnp)->state)
#define CHFSNP_CRWLOCK(chfsnp)                (&((chfsnp)->crwlock))
#define CHFSNP_HDR(chfsnp)                    ((chfsnp)->header)
#define CHFSNP_BUCKET_ADDR(chfsnp)            ((chfsnp)->bucket_addr)
#define CHFSNP_BUCKET_MAX_NUM(chfsnp)         (CHFSNP_HEADER_BUCKET_MAX_NUM(CHFSNP_HDR(chfsnp)))

#define CHFSNP_1ST_CHASH_ALGO(chfsnp)         ((chfsnp)->chash_algo_1st)
#define CHFSNP_2ND_CHASH_ALGO(chfsnp)         ((chfsnp)->chash_algo_2nd)

#define CHFSNP_INIT_LOCK(chfsnp, location)    (croutine_rwlock_init(CHFSNP_CRWLOCK(chfsnp), CMUTEX_PROCESS_PRIVATE, location))
#define CHFSNP_CLEAN_LOCK(chfsnp, location)   (croutine_rwlock_clean(CHFSNP_CRWLOCK(chfsnp), location))
#if 0
#define CHFSNP_RDLOCK(chfsnp, location)       (croutine_rwlock_rdlock(CHFSNP_CRWLOCK(chfsnp), location))
#define CHFSNP_WRLOCK(chfsnp, location)       (croutine_rwlock_wrlock(CHFSNP_CRWLOCK(chfsnp), location))
#define CHFSNP_UNLOCK(chfsnp, location)       (croutine_rwlock_unlock(CHFSNP_CRWLOCK(chfsnp), location))
#endif

#if 1/*note: lock/unlock happen in chfs.c*/
#define CHFSNP_RDLOCK(chfsnp, location)       do{}while(0)
#define CHFSNP_WRLOCK(chfsnp, location)       do{}while(0)
#define CHFSNP_UNLOCK(chfsnp, location)       do{}while(0)

#endif


#define CHFSNP_ID(chfsnp)                     (CHFSNP_HEADER_NP_ID(CHFSNP_HDR(chfsnp)))
#define CHFSNP_MODEL(chfsnp)                  (CHFSNP_HEADER_MODEL(CHFSNP_HDR(chfsnp)))
#define CHFSNP_FIRST_CHASH_ALGO_ID(chfsnp)    (CHFSNP_HEADER_1ST_CHASH_ALGO_ID(CHFSNP_HDR(chfsnp)) )
#define CHFSNP_SECOND_CHASH_ALGO_ID(chfsnp)   (CHFSNP_HEADER_2ND_CHASH_ALGO_ID(CHFSNP_HDR(chfsnp)))
#define CHFSNP_ITEMS_POOL(chfsnp)             (CHFSNP_HEADER_ITEMS_POOL(CHFSNP_HDR(chfsnp)))
#define CHFSNP_ITEMS_MAX_NUM(chfsnp)          (CHFSNPRB_POOL_NODE_MAX_NUM(CHFSNP_ITEMS_POOL(chfsnp)))
#define CHFSNP_ITEMS_USED_NUM(chfsnp)         (CHFSNPRB_POOL_NODE_USED_NUM(CHFSNP_ITEMS_POOL(chfsnp)))

#define CHFSNP_1ST_CHASH_ALGO_COMPUTE(chfsnp, klen, key)  (CHFSNP_1ST_CHASH_ALGO(chfsnp)(klen, key))
#define CHFSNP_2ND_CHASH_ALGO_COMPUTE(chfsnp, klen, key)  (CHFSNP_2ND_CHASH_ALGO(chfsnp)(klen, key))

#define CHFSNP_IS_DIRTY(chfsnp)           (CHFSNP_STATE_DIRTY == CHFSNP_STATE(chfsnp)
#define CHFSNP_IS_NOT_DIRTY(chfsnp)      (CHFSNP_STATE_NOT_DIRTY == CHFSNP_STATE(chfsnp)

#define CHFSNP_SET_DIRTY(chfsnp)         (CHFSNP_STATE(chfsnp) = CHFSNP_STATE_DIRTY)
#define CHFSNP_SET_NOT_DIRTY(chfsnp)     (CHFSNP_STATE(chfsnp) = CHFSNP_STATE_NOT_DIRTY)

#define CHFSNP_HAS_READER(chfsnp)        (0 < CHFSNP_READER_NUM(chfsnp))
#define CHFSNP_NO_READER(chfsnp)         (0 == CHFSNP_READER_NUM(chfsnp))
#define CHFSNP_SET_NO_READER(chfsnp)     (CHFSNP_READER_NUM(chfsnp) = 0)

#define CHFSNP_INC_READER(chfsnp)        (CHFSNP_READER_NUM(chfsnp) ++)
#define CHFSNP_DEC_READER(chfsnp)        (CHFSNP_READER_NUM(chfsnp) --)

#define CHFSNP_BUCKET_POS(chfsnp, first_hash)      ( (first_hash) % CHFSNP_BUCKET_MAX_NUM(chfsnp))
#define CHFSNP_BUCKET(chfsnp, bucket_pos)          (CHFSNP_BUCKET_ADDR(chfsnp)[(bucket_pos)])

EC_BOOL chfsnp_model_str(const uint8_t chfsnp_model, char **mod_str);

uint32_t chfsnp_model_get(const char *mod_str);

EC_BOOL chfsnp_model_file_size(const uint8_t chfsnp_model, uint32_t *file_size);

EC_BOOL chfsnp_model_item_max_num(const uint8_t chfsnp_model, uint32_t *item_max_num);

EC_BOOL chfsnp_inode_init(CHFSNP_INODE *chfsnp_inode);

EC_BOOL chfsnp_inode_clean(CHFSNP_INODE *chfsnp_inode);

EC_BOOL chfsnp_inode_clone(const CHFSNP_INODE *chfsnp_inode_src, CHFSNP_INODE *chfsnp_inode_des);

void chfsnp_inode_print(LOG *log, const CHFSNP_INODE *chfsnp_inode);

void chfsnp_inode_log_no_lock(LOG *log, const CHFSNP_INODE *chfsnp_inode);

CHFSNP_FNODE *chfsnp_fnode_new();

CHFSNP_FNODE *chfsnp_fnode_make(const CHFSNP_FNODE *chfsnp_fnode_src);

EC_BOOL chfsnp_fnode_init(CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfsnp_fnode_clean(CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfsnp_fnode_free(CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfsnp_fnode_init_0(const UINT32 md_id, CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfsnp_fnode_clean_0(const UINT32 md_id, CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfsnp_fnode_free_0(const UINT32 md_id, CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfsnp_fnode_clone(const CHFSNP_FNODE *chfsnp_fnode_src, CHFSNP_FNODE *chfsnp_fnode_des);

EC_BOOL chfsnp_fnode_check_inode_exist(const CHFSNP_INODE *inode, const CHFSNP_FNODE *chfsnp_fnode);

EC_BOOL chfsnp_fnode_cmp(const CHFSNP_FNODE *chfsnp_fnode_1st, const CHFSNP_FNODE *chfsnp_fnode_2nd);

EC_BOOL chfsnp_fnode_import(const CHFSNP_FNODE *chfsnp_fnode_src, CHFSNP_FNODE *chfsnp_fnode_des);

uint32_t chfsnp_fnode_count_replica(const CHFSNP_FNODE *chfsnp_fnode);

void chfsnp_fnode_print(LOG *log, const CHFSNP_FNODE *chfsnp_fnode);

void chfsnp_fnode_log_no_lock(LOG *log, const CHFSNP_FNODE *chfsnp_fnode);

CHFSNP_ITEM *chfsnp_item_new();

EC_BOOL chfsnp_item_init(CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_item_clean(CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_item_clone(const CHFSNP_ITEM *chfsnp_item_src, CHFSNP_ITEM *chfsnp_item_des);

EC_BOOL chfsnp_item_free(CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_item_init_0(const UINT32 md_id, CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_item_clean_0(const UINT32 md_id, CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_item_free_0(const UINT32 md_id, CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_item_set_key(CHFSNP_ITEM *chfsnp_item, const uint32_t klen, const uint8_t *key);

void chfsnp_item_print(LOG *log, const CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_item_load(CHFSNP *chfsnp, uint32_t *offset, CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_item_flush(CHFSNP *chfsnp, uint32_t *offset, const CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_item_is(const CHFSNP_ITEM *chfsnp_item, const uint32_t klen, const uint8_t *key);

void chfsnp_bucket_print(LOG *log, const uint32_t *chfsnp_buckets, const uint32_t bucket_num);

EC_BOOL chfsnp_header_init(CHFSNP_HEADER *chfsnp_header, const uint32_t np_id, const uint8_t model, const uint8_t first_chash_algo_id, const uint8_t second_chash_algo_id, const uint32_t bucket_max_num);

EC_BOOL chfsnp_header_clean(CHFSNP_HEADER *chfsnp_header);

EC_BOOL chfsnp_header_create(CHFSNP_HEADER *chfsnp_header, const uint8_t chfsnp_model, const uint32_t np_id, const uint8_t model, const uint8_t first_chash_algo_id, const uint8_t second_chash_algo_id, const uint32_t bucket_max_num);

CHFSNP *chfsnp_new();

EC_BOOL chfsnp_init(CHFSNP *chfsnp);

EC_BOOL chfsnp_clean(CHFSNP *chfsnp);

EC_BOOL chfsnp_free(CHFSNP *chfsnp);

EC_BOOL chfsnp_is_full(const CHFSNP *chfsnp);

void chfsnp_header_print(LOG *log, const CHFSNP *chfsnp);

void chfsnp_print(LOG *log, const CHFSNP *chfsnp);

uint32_t __chfsnp_bucket_insert(CHFSNP *chfsnp, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

uint32_t __chfsnp_bucket_delete(CHFSNP *chfsnp, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

EC_BOOL __chfsnp_delete_one_bucket(const CHFSNP *chfsnp, const uint32_t bucket_pos, CVECTOR *chfsnp_fnode_vec);

EC_BOOL __chfsnp_delete_all_buckets(const CHFSNP *chfsnp, CVECTOR *chfsnp_fnode_vec);

uint32_t chfsnp_search_no_lock(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path);

uint32_t chfsnp_search(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path);

uint32_t chfsnp_insert_no_lock(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path);

uint32_t chfsnp_insert(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path);

CHFSNP_ITEM *chfsnp_fetch(const CHFSNP *chfsnp, const uint32_t node_pos);

EC_BOOL chfsnp_inode_update(CHFSNP *chfsnp, CHFSNP_INODE *chfsnp_inode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL chfsnp_fnode_update(CHFSNP *chfsnp, CHFSNP_FNODE *chfsnp_fnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL __chfsnp_update_one_bucket(CHFSNP *chfsnp, const uint32_t bucket_pos, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL chfsnp_update_all_buckets(CHFSNP *chfsnp, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL chfsnp_item_update(CHFSNP *chfsnp, CHFSNP_ITEM *chfsnp_item, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL chfsnp_update_no_lock(CHFSNP *chfsnp, 
                               const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                               const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

CHFSNP_ITEM *chfsnp_set(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path);

CHFSNP_ITEM *chfsnp_get(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path);

EC_BOOL chfsnp_delete(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path, CVECTOR *chfsnp_fnode_vec);

uint32_t chfsnp_count_file_num(const CHFSNP *chfsnp);

EC_BOOL chfsnp_file_size(CHFSNP *chfsnp, const uint32_t path_len, const uint8_t *path, uint32_t *file_size);

EC_BOOL __chfsnp_count_one_bucket_file_size(CHFSNP *chfsnp, const uint32_t bucket_pos, uint64_t *file_size);

EC_BOOL chfsnp_count_file_size(CHFSNP *chfsnp, uint64_t *file_size);

CHFSNP *chfsnp_open(const char *np_root_dir, const uint32_t np_id);

EC_BOOL chfsnp_close(CHFSNP *chfsnp);

EC_BOOL chfsnp_sync(CHFSNP *chfsnp);

CHFSNP *chfsnp_create(const char *np_root_dir, const uint32_t np_id, const uint8_t np_model, const uint8_t hash_1st_algo_id, const uint8_t hash_2nd_algo_id, const uint32_t bucket_max_num);

EC_BOOL chfsnp_show_item(LOG *log, const CHFSNP_ITEM *chfsnp_item);

EC_BOOL chfsnp_show_one_bucket(LOG *log, const CHFSNP *chfsnp, const uint32_t bucket_pos);

EC_BOOL chfsnp_show_all_buckets(LOG *log, const CHFSNP *chfsnp);

#endif/* _CHFSNP_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

