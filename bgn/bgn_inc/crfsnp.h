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

#ifndef _CRFSNP_H
#define _CRFSNP_H

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
#include "crfsnprb.h"


#define CRFSNP_KEY_MAX_SIZE             ( 255)  /*max len of file or dir seg name*/
#define CRFSNP_PATH_MAX_LEN             (1024)  /*max len of file or dir path name*/

#define CRFSNP_004K_MODEL   ((uint8_t) 0)
#define CRFSNP_064K_MODEL   ((uint8_t) 1)
#define CRFSNP_001M_MODEL   ((uint8_t) 2)
#define CRFSNP_002M_MODEL   ((uint8_t) 3)
#define CRFSNP_128M_MODEL   ((uint8_t) 4)
#define CRFSNP_256M_MODEL   ((uint8_t) 5)
#define CRFSNP_512M_MODEL   ((uint8_t) 6)
#define CRFSNP_001G_MODEL   ((uint8_t) 7)
#define CRFSNP_002G_MODEL   ((uint8_t) 8)
#define CRFSNP_004G_MODEL   ((uint8_t) 9)

#define CRFSNP_FILE_REPLICA_MAX_NUM     ((uint32_t) 1)  /*max num of supported replicas up to*/
#define CRFSNP_DNODE_DIR_BUCKET_MAX_NUM ((uint32_t)53)  
#define CRFSNP_BNODE_SEG_BUCKET_MAX_NUM ((uint32_t)51)  

#define CRFSNP_ITEM_REF_MAX_NUM         ((UINT32) 0xF)  /*4 bits*/

#define CRFSNP_ITEM_FILE_IS_PIP         ((uint32_t) 0x01)  /*pipe file   */
#define CRFSNP_ITEM_FILE_IS_DIR         ((uint32_t) 0x02)  /*directory   */
#define CRFSNP_ITEM_FILE_IS_LNK         ((uint32_t) 0x04)  /*link file   */
#define CRFSNP_ITEM_FILE_IS_REG         ((uint32_t) 0x08)  /*regular file*/
#define CRFSNP_ITEM_FILE_IS_SCK         ((uint32_t) 0x10)  /*socket file */
#define CRFSNP_ITEM_FILE_IS_CHR         ((uint32_t) 0x20)  /*char device */
#define CRFSNP_ITEM_FILE_IS_BLK         ((uint32_t) 0x40)  /*block device*/
#define CRFSNP_ITEM_FILE_IS_ANY         ((uint32_t) 0x80)  /*any file    */
#define CRFSNP_ITEM_FILE_IS_ERR         ((uint32_t) 0x00)  /*4 bits      */
#define CRFSNP_ITEM_FILE_IS_BIG         (CRFSNP_ITEM_FILE_IS_DIR | CRFSNP_ITEM_FILE_IS_REG)

#define CRFSNP_ITEM_STAT_IS_NOT_USED    ((uint32_t) 0x1)  /*4 bits*/
#define CRFSNP_ITEM_STAT_IS_USED        ((uint32_t) 0x8)

/**********************************************************************************
*   bit# 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
*        t  t  t  t  u   g  s  r  w  x  r  w  x  r  w  x
*        ----------  -   -  -  -------  -------  -------
*           |        |   |  |     |        |        |_ other permission
*           |        |   |  |     |        |
*           |        |   |  |     |        |_ group permission
*           |        |   |  |     |
*           |        |   |  |     |_ owner/user permission
*           |        |   |  |
*           |        |   |  |_ sticky bit
*           |        |   |
*           |        |   |_ set_gid bit
*           |        |
*           |        |_ set_uid bit
*           |
*           |_ file type
*
**********************************************************************************/

#define CRFSNP_PERMISSION_UID_BIT_MASK  ((uint32_t) 04000)
#define CRFSNP_PERMISSION_GID_BIT_MASK  ((uint32_t) 02000)
#define CRFSNP_PERMISSION_STK_BIT_MASK  ((uint32_t) 01000)
#define CRFSNP_PERMISSION_USR_BIT_MASK  ((uint32_t) 00700)
#define CRFSNP_PERMISSION_GRP_BIT_MASK  ((uint32_t) 00070)
#define CRFSNP_PERMISSION_OTH_BIT_MASK  ((uint32_t) 00007)

#define CRFSNP_PERMISSION_UID_NBITS     ((uint32_t)  1)    /*num of bits*/
#define CRFSNP_PERMISSION_GID_NBITS     ((uint32_t)  1)    /*num of bits*/
#define CRFSNP_PERMISSION_STK_NBITS     ((uint32_t)  1)    /*num of bits*/
#define CRFSNP_PERMISSION_USR_NBITS     ((uint32_t)  3)    /*num of bits*/
#define CRFSNP_PERMISSION_GRP_NBITS     ((uint32_t)  3)    /*num of bits*/
#define CRFSNP_PERMISSION_OTH_NBITS     ((uint32_t)  3)    /*num of bits*/

#define CRFSNP_PERMISSION_UID_ABITS     ((uint32_t) 11)    /*bit alignment*/
#define CRFSNP_PERMISSION_GID_ABITS     ((uint32_t) 10)    /*bit alignment*/
#define CRFSNP_PERMISSION_STK_ABITS     ((uint32_t)  9)    /*bit alignment*/
#define CRFSNP_PERMISSION_USR_ABITS     ((uint32_t)  6)    /*bit alignment*/
#define CRFSNP_PERMISSION_GRP_ABITS     ((uint32_t)  3)    /*bit alignment*/
#define CRFSNP_PERMISSION_OTH_ABITS     ((uint32_t)  0)    /*bit alignment*/


#define CRFSDN_DATA_NOT_IN_CACHE        ((uint16_t)0x0000)
#define CRFSDN_DATA_IS_IN_CACHE         ((uint16_t)0x0001)

typedef struct
{
    uint16_t    cache_flag;     /*data is cached or not*/
    uint16_t    disk_no;        /*local disk_no*/
    uint16_t    block_no;       /*block_no in above disk*/
    uint16_t    page_no;        /*page_no in above block*/    
}CRFSNP_INODE;

//#define CRFSNP_INODE_TCID(crfsnp_inode)              ((crfsnp_inode)->tcid)
#define CRFSNP_INODE_CACHE_FLAG(crfsnp_inode)        ((crfsnp_inode)->cache_flag)
#define CRFSNP_INODE_DISK_NO(crfsnp_inode)           ((crfsnp_inode)->disk_no)
#define CRFSNP_INODE_BLOCK_NO(crfsnp_inode)          ((crfsnp_inode)->block_no)
#define CRFSNP_INODE_PAGE_NO(crfsnp_inode)           ((crfsnp_inode)->page_no)

#define CRFSNP_FNODE_PAD_SIZE                        (200)

typedef struct
{
    uint32_t      file_size;    /*data/value length < 64M = 2^26B*/
    uint32_t      file_replica_num;

    CRFSNP_INODE  inodes[ CRFSNP_FILE_REPLICA_MAX_NUM ];
    uint8_t       pad[CRFSNP_FNODE_PAD_SIZE]; /*200B*/
}CRFSNP_FNODE;/*216B*/

#define CRFSNP_FNODE_FILESZ(crfsnp_fnode)        ((crfsnp_fnode)->file_size)
#define CRFSNP_FNODE_REPNUM(crfsnp_fnode)        ((crfsnp_fnode)->file_replica_num)
#define CRFSNP_FNODE_INODES(crfsnp_fnode)        ((crfsnp_fnode)->inodes)
#define CRFSNP_FNODE_INODE(crfsnp_fnode, idx)    (&((crfsnp_fnode)->inodes[ (idx) ]))

#define CRFSNP_FNODE_CACHE_FLAG(crfsnp_fnode, idx)       CRFSNP_INODE_CACHE_FLAG(CRFSNP_FNODE_INODE(crfsnp_fnode, idx))
#define CRFSNP_FNODE_INODE_DISK_NO(crfsnp_fnode, idx)    CRFSNP_INODE_DISK_NO(CRFSNP_FNODE_INODE(crfsnp_fnode, idx))
#define CRFSNP_FNODE_INODE_BLOCK_NO(crfsnp_fnode, idx)   CRFSNP_INODE_BLOCK_NO(CRFSNP_FNODE_INODE(crfsnp_fnode, idx))
#define CRFSNP_FNODE_INODE_PAGE_NO(crfsnp_fnode, idx)    CRFSNP_INODE_PAGE_NO(CRFSNP_FNODE_INODE(crfsnp_fnode, idx))

typedef struct
{
    uint32_t       file_num;   /*number of files under this directory*/
    //uint32_t       rsvd;
    uint32_t       dir_buckets[ CRFSNP_DNODE_DIR_BUCKET_MAX_NUM ];/*store the CRFSNP_ITEM no.*/
}CRFSNP_DNODE;/*216B*/

#define CRFSNP_DNODE_FILE_NUM(crfsnp_dnode)         ((crfsnp_dnode)->file_num)
#define CRFSNP_DNODE_DIR_BUCKETS(crfsnp_dnode)      ((crfsnp_dnode)->dir_buckets)
#define CRFSNP_DNODE_DIR_BUCKET(crfsnp_dnode, idx)  ((crfsnp_dnode)->dir_buckets[ (idx) ])
#define CRFSNP_DNODE_DIR_BUCKET_POS(second_hash)    ((second_hash) % CRFSNP_DNODE_DIR_BUCKET_MAX_NUM)

typedef struct
{
    uint64_t       file_size;
    uint32_t       seg_num;     /*number of segments of this big file*/
    uint32_t       seg_buckets[ CRFSNP_BNODE_SEG_BUCKET_MAX_NUM ];/*store the CRFSNP_ITEM no. all are fnodes*/
}CRFSNP_BNODE;/*216B*/

#define CRFSNP_BNODE_FILESZ(crfsnp_bnode)           ((crfsnp_bnode)->file_size)
#define CRFSNP_BNODE_SEG_NUM(crfsnp_bnode)          ((crfsnp_bnode)->seg_num)
#define CRFSNP_BNODE_SEG_BUCKETS(crfsnp_bnode)      ((crfsnp_bnode)->seg_buckets)
#define CRFSNP_BNODE_SEG_BUCKET(crfsnp_bnode, idx)  ((crfsnp_bnode)->seg_buckets[ (idx) ])
#define CRFSNP_BNODE_SEG_BUCKET_POS(second_hash)    ((second_hash) % CRFSNP_BNODE_SEG_BUCKET_MAX_NUM)

#define CRFSNP_BNODE_FILESZ_UPDATE_IF_NEED(crfsnp_bnode, cur_len) do{\
    if((cur_len) > CRFSNP_BNODE_FILESZ(crfsnp_bnode))\
    {\
        sys_log(LOGSTDOUT, "[DEBUG] CRFSNP_BNODE_FILESZ_UPDATE: update bnode %p fsize: %llu ---> %llu\n", \
                            (crfsnp_bnode), CRFSNP_BNODE_FILESZ(crfsnp_bnode), (cur_len));\
        CRFSNP_BNODE_FILESZ(crfsnp_bnode) = (cur_len);\
    }\
}while(0)

typedef struct
{   
    CRFSNPRB_NODE   rb_node;/*16B*/
    
    /*bitmap: 32bits*/
    uint32_t      item_stat:4;  /* item status: not used, used */
    uint32_t      key_len  :8;  /* key lenght, range [0..CRFSNP_KEY_MAX_SIZE] */
    uint32_t      dir_flag :8;  /* directory or regular file */
    uint32_t      set_uid  :1;  /* set uid bit*/
    uint32_t      set_gid  :1;  /* set gid bit*/
    uint32_t      sticky   :1;  /* sticky bit*/
    uint32_t      owner_per:3;  /* owner permission*/
    uint32_t      group_per:3;  /* group permission*/
    uint32_t      other_per:3;  /* other permission*/

    /*32 bits*/
    uint16_t      gid;
    uint16_t      uid;

    /*64 bits*/
    ctime_t       time; /*file created or modified time. 32 bits for 32bit OS, 64 bits for 64bit OS*/
#if (32 == WORDSIZE)    
    uint32_t      rsvd1;
#endif/*(32 == WORDSIZE)*/    
    
    /*64 bits*/
    uint32_t      parent_pos;/*parent directory*/
    uint32_t      rsvd2;
    
    uint8_t       rsvd3;
    uint8_t       key[ CRFSNP_KEY_MAX_SIZE ];  /* dir name or file name */

    union
    {
        CRFSNP_FNODE fnode;/*16B + 200B = 216B*/
        CRFSNP_DNODE dnode;/*216B*/
        CRFSNP_BNODE bnode;/*216B*/
    }u;/*216B*/   
} CRFSNP_ITEM;/*512B*/

#define CRFSNP_ITEM_RB_NODE(crfsnp_item)          (&((crfsnp_item)->rb_node))
#define CRFSNP_ITEM_DFLG(crfsnp_item)             ((crfsnp_item)->dir_flag)
#define CRFSNP_ITEM_STAT(crfsnp_item)             ((crfsnp_item)->item_stat)
#define CRFSNP_ITEM_KLEN(crfsnp_item)             ((crfsnp_item)->key_len)
#define CRFSNP_ITEM_KEY(crfsnp_item)              ((crfsnp_item)->key)
#define CRFSNP_ITEM_PARENT_POS(crfsnp_item)       ((crfsnp_item)->parent_pos)
#define CRFSNP_ITEM_FNODE(crfsnp_item)            (&((crfsnp_item)->u.fnode))
#define CRFSNP_ITEM_DNODE(crfsnp_item)            (&((crfsnp_item)->u.dnode))
#define CRFSNP_ITEM_BNODE(crfsnp_item)            (&((crfsnp_item)->u.bnode))

/*get CRFSNP_ITEM from CRFSNPRB_NODE*/
#define CRFSNP_RB_NODE_ITEM(crfsnprb_node)        ((NULL_PTR == (crfsnprb_node)) ? NULL_PTR : \
    ((CRFSNP_ITEM *)((char *)(crfsnprb_node)-(unsigned long)(&((CRFSNP_ITEM *)0)->rb_node))))


#define CRFSNP_ITEM_IS_REG(crfsnp_item)            (CRFSNP_ITEM_FILE_IS_REG == (CRFSNP_ITEM_FILE_IS_REG & CRFSNP_ITEM_DFLG(crfsnp_item)))    
#define CRFSNP_ITEM_IS_DIR(crfsnp_item)            (CRFSNP_ITEM_FILE_IS_DIR == (CRFSNP_ITEM_FILE_IS_DIR & CRFSNP_ITEM_DFLG(crfsnp_item)))    
#define CRFSNP_ITEM_IS_BIG(crfsnp_item)            (CRFSNP_ITEM_FILE_IS_BIG == (CRFSNP_ITEM_FILE_IS_BIG & CRFSNP_ITEM_DFLG(crfsnp_item)))    

#define CRFSNP_ITEM_IS_NOT_REG(crfsnp_item)        (CRFSNP_ITEM_FILE_IS_REG != (CRFSNP_ITEM_FILE_IS_REG & CRFSNP_ITEM_DFLG(crfsnp_item)))    
#define CRFSNP_ITEM_IS_NOT_DIR(crfsnp_item)        (CRFSNP_ITEM_FILE_IS_DIR != (CRFSNP_ITEM_FILE_IS_DIR & CRFSNP_ITEM_DFLG(crfsnp_item)))    
#define CRFSNP_ITEM_IS_NOT_BIG(crfsnp_item)        (CRFSNP_ITEM_FILE_IS_BIG != (CRFSNP_ITEM_FILE_IS_BIG & CRFSNP_ITEM_DFLG(crfsnp_item)))  

#define CRFSNP_ITEM_IS_INVALID(crfsnp_item)        ((CRFSNP_ITEM_FILE_IS_REG | CRFSNP_ITEM_FILE_IS_DIR) & CRFSNP_ITEM_DFLG(crfsnp_item))  
#define CRFSNP_ITEM_IS_NOT_INVALID(crfsnp_item)    (0 == ((CRFSNP_ITEM_FILE_IS_REG | CRFSNP_ITEM_FILE_IS_DIR) & CRFSNP_ITEM_DFLG(crfsnp_item)) )

/*item max num = file size / sizeof(CRFSNP_ITEM) where sizeof(CRFSNP_ITEM) = 128B = 2^7*/
#define CRFSNP_ITEM_BIT_SIZE             (9)
#define CRFSNP_004K_CFG_FILE_SIZE        ((uint32_t)(1 << 12))
#define CRFSNP_004K_CFG_ITEM_MAX_NUM     (CRFSNP_004K_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

#define CRFSNP_064K_CFG_FILE_SIZE        ((uint32_t)(1 << 16))
#define CRFSNP_064K_CFG_ITEM_MAX_NUM     (CRFSNP_064K_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

#define CRFSNP_001M_CFG_FILE_SIZE        ((uint32_t)(1 << 20))
#define CRFSNP_001M_CFG_ITEM_MAX_NUM     (CRFSNP_001M_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

#define CRFSNP_002M_CFG_FILE_SIZE        ((uint32_t)(1 << 21))
#define CRFSNP_002M_CFG_ITEM_MAX_NUM     (CRFSNP_002M_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

#define CRFSNP_128M_CFG_FILE_SIZE        ((uint32_t)(1 << 27))
#define CRFSNP_128M_CFG_ITEM_MAX_NUM     (CRFSNP_128M_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

#define CRFSNP_256M_CFG_FILE_SIZE        ((uint32_t)(1 << 28))
#define CRFSNP_256M_CFG_ITEM_MAX_NUM     (CRFSNP_256M_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

#define CRFSNP_512M_CFG_FILE_SIZE        ((uint32_t)(1 << 29))
#define CRFSNP_512M_CFG_ITEM_MAX_NUM     (CRFSNP_512M_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

#define CRFSNP_001G_CFG_FILE_SIZE        ((uint32_t)(1 << 30))
#define CRFSNP_001G_CFG_ITEM_MAX_NUM     (CRFSNP_001G_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

#define CRFSNP_002G_CFG_FILE_SIZE        ((uint32_t)(1 << 31))
#define CRFSNP_002G_CFG_ITEM_MAX_NUM     (CRFSNP_002G_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

#if (64 == WORDSIZE)
#define CRFSNP_004G_CFG_FILE_SIZE        ((uint32_t)(1 << 32))
#define CRFSNP_004G_CFG_ITEM_MAX_NUM     (CRFSNP_004G_CFG_FILE_SIZE >> CRFSNP_ITEM_BIT_SIZE)

/*due to offset is defined as 32bit integer, here cannot support more than 4G file*/
#endif/*(64 == WORDSIZE)*/

typedef struct
{
    char    *mode_str;
    uint32_t file_size;
    uint32_t item_max_num;
}CRFSNP_CFG;

#define CRFSNP_CFG_MOD_STR(crfsnp_cfg)                ((crfsnp_cfg)->mode_str)
#define CRFSNP_CFG_FILE_SIZE(crfsnp_cfg)              ((crfsnp_cfg)->file_size)
#define CRFSNP_CFG_ITEM_MAX_NUM(crfsnp_cfg)           ((crfsnp_cfg)->item_max_num)

#define CRFSNP_ERR_MODEL             ((uint32_t)0xF)  /*4 bits*/

#define CRFSNP_O_RDONLY              ((uint32_t)O_RDONLY)
#define CRFSNP_O_WRONLY              ((uint32_t)O_WRONLY)
#define CRFSNP_O_RDWR                ((uint32_t)O_RDWR  )
#define CRFSNP_O_CREATE              ((uint32_t)O_CREAT )

/*bitmap*/
#define CRFSNP_STATE_NOT_DIRTY       ((uint8_t)0x00)
#define CRFSNP_STATE_DIRTY           ((uint8_t)0x01)

#define CRFSNP_PATH_LAYOUT_DIR0_NBITS    ( 8)
#define CRFSNP_PATH_LAYOUT_DIR1_NBITS    ( 8)
#define CRFSNP_PATH_LAYOUT_DIR2_NBITS    ( 8)
#define CRFSNP_PATH_LAYOUT_DIR3_NBITS    ( 8)

#define CRFSNP_PATH_LAYOUT_DIR0_ABITS    (24) /*bit alignment*/
#define CRFSNP_PATH_LAYOUT_DIR1_ABITS    (16) /*bit alignment*/
#define CRFSNP_PATH_LAYOUT_DIR2_ABITS    ( 8) /*bit alignment*/
#define CRFSNP_PATH_LAYOUT_DIR3_ABITS    ( 0) /*bit alignment*/

#define CRFSNP_PATH_LAYOUT_DIR0_MASK     (((UINT32)(UINT32_ONE << CRFSNP_PATH_LAYOUT_DIR0_NBITS)) - 1)
#define CRFSNP_PATH_LAYOUT_DIR1_MASK     (((UINT32)(UINT32_ONE << CRFSNP_PATH_LAYOUT_DIR1_NBITS)) - 1)
#define CRFSNP_PATH_LAYOUT_DIR2_MASK     (((UINT32)(UINT32_ONE << CRFSNP_PATH_LAYOUT_DIR2_NBITS)) - 1)
#define CRFSNP_PATH_LAYOUT_DIR3_MASK     (((UINT32)(UINT32_ONE << CRFSNP_PATH_LAYOUT_DIR3_NBITS)) - 1)

#define CRFSNP_PATH_LAYOUT_DIR0_NO(path_id)     (((path_id) >> CRFSNP_PATH_LAYOUT_DIR0_ABITS) & CRFSNP_PATH_LAYOUT_DIR0_MASK)
#define CRFSNP_PATH_LAYOUT_DIR1_NO(path_id)     (((path_id) >> CRFSNP_PATH_LAYOUT_DIR1_ABITS) & CRFSNP_PATH_LAYOUT_DIR1_MASK)
#define CRFSNP_PATH_LAYOUT_DIR2_NO(path_id)     (((path_id) >> CRFSNP_PATH_LAYOUT_DIR2_ABITS) & CRFSNP_PATH_LAYOUT_DIR2_MASK)
#define CRFSNP_PATH_LAYOUT_DIR3_NO(path_id)     (((path_id) >> CRFSNP_PATH_LAYOUT_DIR3_ABITS) & CRFSNP_PATH_LAYOUT_DIR3_MASK)

#define CRFSNP_ERR_ID                     ((uint32_t)0xFFFFFFFF)


/*each np own one header*/
typedef struct
{
    uint32_t       np_id;               /*crfsnp id              */    
    uint8_t        model;               /*crfsnp model           */    
    uint8_t        rsvd;                    
    uint8_t        chash_algo_1st_id ;  /*first hash algo func id : used to compute bucket pos in dnode   */
    uint8_t        chash_algo_2nd_id;   /*second hash algo func id: used to compute crfsnprb_node hash data*/   
    
    CRFSNPRB_POOL  pool;                /*pool of CRFSNP_ITEM, CRFSNP_ITEM head must be CRFSNPRB_NODE*/
} CRFSNP_HEADER;

#define CRFSNP_HEADER_NP_ID(crfsnp_header)         ((crfsnp_header)->np_id)
#define CRFSNP_HEADER_MODEL(crfsnp_header)         ((crfsnp_header)->model)

#define CRFSNP_HEADER_1ST_CHASH_ALGO_ID(crfsnp_header)  ((crfsnp_header)->chash_algo_1st_id)
#define CRFSNP_HEADER_2ND_CHASH_ALGO_ID(crfsnp_header)  ((crfsnp_header)->chash_algo_2nd_id)

#define CRFSNP_HEADER_ITEMS_POOL(crfsnp_header)         (&((crfsnp_header)->pool))
#define CRFSNP_HEADER_ITEMS_MAX_NUM(crfsnp_header)      (CRFSNPRB_POOL_NODE_MAX_NUM(CRFSNP_HEADER_ITEMS_POOL(crfsnp_header)))
#define CRFSNP_HEADER_ITEMS_USED_NUM(crfsnp_header)     (CRFSNPRB_POOL_NODE_USED_NUM(CRFSNP_HEADER_ITEMS_POOL(crfsnp_header)))

typedef struct
{
    int              fd;         /* rfs namespace fd  */
    uint32_t         fsize;
    
    uint8_t         *fname;
    
    uint16_t         state;
    uint16_t         rsvd;
    uint32_t         reader_num; /* current reader num*/    
    
    CROUTINE_RWLOCK  crwlock;        /* bucket crwlock*/
    CRFSNP_HEADER   *header;        /* hashdb header */    

    CHASH_ALGO       chash_algo_1st;       /* hash algo for hash bucket              : used to compute bucket pos in dnode   */
    CHASH_ALGO       chash_algo_2nd;       /* hash algo for rbtree in the hash bucket: used to compute crfsnprb_node hash data*/   
} CRFSNP;

#define CRFSNP_FD(crfsnp)                     ((crfsnp)->fd)
#define CRFSNP_FSIZE(crfsnp)                  ((crfsnp)->fsize)
#define CRFSNP_FNAME(crfsnp)                  ((crfsnp)->fname)
#define CRFSNP_READER_NUM(crfsnp)             ((crfsnp)->reader_num)
#define CRFSNP_STATE(crfsnp)                  ((crfsnp)->state)
#define CRFSNP_CRWLOCK(crfsnp)                (&((crfsnp)->crwlock))
#define CRFSNP_HDR(crfsnp)                    ((crfsnp)->header)

#define CRFSNP_1ST_CHASH_ALGO(crfsnp)         ((crfsnp)->chash_algo_1st)
#define CRFSNP_2ND_CHASH_ALGO(crfsnp)         ((crfsnp)->chash_algo_2nd)

#define CRFSNP_INIT_LOCK(crfsnp, location)    (croutine_rwlock_init(CRFSNP_CRWLOCK(crfsnp), CMUTEX_PROCESS_PRIVATE, location))
#define CRFSNP_CLEAN_LOCK(crfsnp, location)   (croutine_rwlock_clean(CRFSNP_CRWLOCK(crfsnp), location))
#if 0
#define CRFSNP_RDLOCK(crfsnp, location)       (croutine_rwlock_rdlock(CRFSNP_CRWLOCK(crfsnp), location))
#define CRFSNP_WRLOCK(crfsnp, location)       (croutine_rwlock_wrlock(CRFSNP_CRWLOCK(crfsnp), location))
#define CRFSNP_UNLOCK(crfsnp, location)       (croutine_rwlock_unlock(CRFSNP_CRWLOCK(crfsnp), location))
#endif

#if 1/*note: lock/unlock happen in crfs.c*/
#define CRFSNP_RDLOCK(crfsnp, location)       do{}while(0)
#define CRFSNP_WRLOCK(crfsnp, location)       do{}while(0)
#define CRFSNP_UNLOCK(crfsnp, location)       do{}while(0)

#endif


#define CRFSNP_ID(crfsnp)                     (CRFSNP_HEADER_NP_ID(CRFSNP_HDR(crfsnp)))
#define CRFSNP_MODEL(crfsnp)                  (CRFSNP_HEADER_MODEL(CRFSNP_HDR(crfsnp)))
#define CRFSNP_FIRST_CHASH_ALGO_ID(crfsnp)    (CRFSNP_HEADER_1ST_CHASH_ALGO_ID(CRFSNP_HDR(crfsnp)) )
#define CRFSNP_SECOND_CHASH_ALGO_ID(crfsnp)   (CRFSNP_HEADER_2ND_CHASH_ALGO_ID(CRFSNP_HDR(crfsnp)))
#define CRFSNP_ITEMS_POOL(crfsnp)             (CRFSNP_HEADER_ITEMS_POOL(CRFSNP_HDR(crfsnp)))
#define CRFSNP_ITEMS_MAX_NUM(crfsnp)          (CRFSNPRB_POOL_NODE_MAX_NUM(CRFSNP_ITEMS_POOL(crfsnp)))
#define CRFSNP_ITEMS_USED_NUM(crfsnp)         (CRFSNPRB_POOL_NODE_USED_NUM(CRFSNP_ITEMS_POOL(crfsnp)))

#define CRFSNP_1ST_CHASH_ALGO_COMPUTE(crfsnp, klen, key)  (CRFSNP_1ST_CHASH_ALGO(crfsnp)(klen, key))
#define CRFSNP_2ND_CHASH_ALGO_COMPUTE(crfsnp, klen, key)  (CRFSNP_2ND_CHASH_ALGO(crfsnp)(klen, key))

#define CRFSNP_IS_DIRTY(crfsnp)           (CRFSNP_STATE_DIRTY == CRFSNP_STATE(crfsnp)
#define CRFSNP_IS_NOT_DIRTY(crfsnp)      (CRFSNP_STATE_NOT_DIRTY == CRFSNP_STATE(crfsnp)

#define CRFSNP_SET_DIRTY(crfsnp)         (CRFSNP_STATE(crfsnp) = CRFSNP_STATE_DIRTY)
#define CRFSNP_SET_NOT_DIRTY(crfsnp)     (CRFSNP_STATE(crfsnp) = CRFSNP_STATE_NOT_DIRTY)

#define CRFSNP_HAS_READER(crfsnp)        (0 < CRFSNP_READER_NUM(crfsnp))
#define CRFSNP_NO_READER(crfsnp)         (0 == CRFSNP_READER_NUM(crfsnp))
#define CRFSNP_SET_NO_READER(crfsnp)     (CRFSNP_READER_NUM(crfsnp) = 0)

#define CRFSNP_INC_READER(crfsnp)        (CRFSNP_READER_NUM(crfsnp) ++)
#define CRFSNP_DEC_READER(crfsnp)        (CRFSNP_READER_NUM(crfsnp) --)

EC_BOOL crfsnp_model_str(const uint8_t crfsnp_model, char **mod_str);

uint32_t crfsnp_model_get(const char *mod_str);

EC_BOOL crfsnp_model_file_size(const uint8_t crfsnp_model, uint32_t *file_size);

EC_BOOL crfsnp_model_item_max_num(const uint8_t crfsnp_model, uint32_t *item_max_num);

EC_BOOL crfsnp_inode_init(CRFSNP_INODE *crfsnp_inode);

EC_BOOL crfsnp_inode_clean(CRFSNP_INODE *crfsnp_inode);

EC_BOOL crfsnp_inode_clone(const CRFSNP_INODE *crfsnp_inode_src, CRFSNP_INODE *crfsnp_inode_des);

void crfsnp_inode_print(LOG *log, const CRFSNP_INODE *crfsnp_inode);

void crfsnp_inode_log_no_lock(LOG *log, const CRFSNP_INODE *crfsnp_inode);

CRFSNP_FNODE *crfsnp_fnode_new();

CRFSNP_FNODE *crfsnp_fnode_make(const CRFSNP_FNODE *crfsnp_fnode_src);

EC_BOOL crfsnp_fnode_init(CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_clean(CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_free(CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_init_0(const UINT32 md_id, CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_clean_0(const UINT32 md_id, CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_free_0(const UINT32 md_id, CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_clone(const CRFSNP_FNODE *crfsnp_fnode_src, CRFSNP_FNODE *crfsnp_fnode_des);

EC_BOOL crfsnp_fnode_check_inode_exist(const CRFSNP_INODE *inode, const CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_cmp(const CRFSNP_FNODE *crfsnp_fnode_1st, const CRFSNP_FNODE *crfsnp_fnode_2nd);

EC_BOOL crfsnp_fnode_import(const CRFSNP_FNODE *crfsnp_fnode_src, CRFSNP_FNODE *crfsnp_fnode_des);

uint32_t crfsnp_fnode_count_replica(const CRFSNP_FNODE *crfsnp_fnode);

void crfsnp_fnode_print(LOG *log, const CRFSNP_FNODE *crfsnp_fnode);

void crfsnp_fnode_log_no_lock(LOG *log, const CRFSNP_FNODE *crfsnp_fnode);

CRFSNP_DNODE *crfsnp_dnode_new();

EC_BOOL crfsnp_dnode_init(CRFSNP_DNODE *crfsnp_dnode);

EC_BOOL crfsnp_dnode_clean(CRFSNP_DNODE *crfsnp_dnode);

EC_BOOL crfsnp_dnode_free(CRFSNP_DNODE *crfsnp_dnode);

EC_BOOL crfsnp_dnode_clone(const CRFSNP_DNODE *crfsnp_dnode_src, CRFSNP_DNODE *crfsnp_dnode_des);

CRFSNP_BNODE *crfsnp_bnode_new();

EC_BOOL crfsnp_bnode_init(CRFSNP_BNODE *crfsnp_bnode);

EC_BOOL crfsnp_bnode_clean(CRFSNP_BNODE *crfsnp_bnode);

EC_BOOL crfsnp_bnode_free(CRFSNP_BNODE *crfsnp_bnode);

EC_BOOL crfsnp_bnode_clone(const CRFSNP_BNODE *crfsnp_bnode_src, CRFSNP_BNODE *crfsnp_bnode_des);

void crfsnp_bnode_print(LOG *log, const CRFSNP_BNODE *crfsnp_bnode);

CRFSNP_ITEM *crfsnp_item_new();

EC_BOOL crfsnp_item_init(CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_clean(CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_clone(const CRFSNP_ITEM *crfsnp_item_src, CRFSNP_ITEM *crfsnp_item_des);

EC_BOOL crfsnp_item_free(CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_init_0(const UINT32 md_id, CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_clean_0(const UINT32 md_id, CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_free_0(const UINT32 md_id, CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_set_key(CRFSNP_ITEM *crfsnp_item, const uint32_t klen, const uint8_t *key);

void crfsnp_item_print(LOG *log, const CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_load(CRFSNP *crfsnp, uint32_t *offset, CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_flush(CRFSNP *crfsnp, uint32_t *offset, const CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_is(const CRFSNP_ITEM *crfsnp_item, const uint32_t klen, const uint8_t *key);

CRFSNP_ITEM *crfsnp_item_parent(const CRFSNP *crfsnp, const CRFSNP_ITEM *crfsnp_item);

CRFSNP_ITEM *crfsnp_item_left(const CRFSNP *crfsnp, const CRFSNP_ITEM *crfsnp_item);

CRFSNP_ITEM *crfsnp_item_right(const CRFSNP *crfsnp, const CRFSNP_ITEM *crfsnp_item);

void crfsnp_bucket_print(LOG *log, const uint32_t *crfsnp_buckets);

EC_BOOL crfsnp_header_init(CRFSNP_HEADER *crfsnp_header, const uint32_t np_id, const uint8_t model, const uint8_t first_chash_algo_id, const uint8_t second_chash_algo_id);

EC_BOOL crfsnp_header_clean(CRFSNP_HEADER *crfsnp_header);

CRFSNP_HEADER *crfsnp_header_open(const uint32_t np_id, const uint32_t fsize, int fd);

CRFSNP_HEADER *crfsnp_header_create(const uint32_t np_id, const uint32_t fsize, int fd, const uint8_t np_model);

CRFSNP_HEADER *crfsnp_header_sync(CRFSNP_HEADER *crfsnp_header, const uint32_t np_id, const uint32_t fsize, int fd);

CRFSNP_HEADER *crfsnp_header_close(CRFSNP_HEADER *crfsnp_header, const uint32_t np_id, const uint32_t fsize, int fd);

CRFSNP *crfsnp_new();

EC_BOOL crfsnp_init(CRFSNP *crfsnp);

EC_BOOL crfsnp_clean(CRFSNP *crfsnp);

EC_BOOL crfsnp_free(CRFSNP *crfsnp);

EC_BOOL crfsnp_is_full(const CRFSNP *crfsnp);

void crfsnp_header_print(LOG *log, const CRFSNP *crfsnp);

void crfsnp_print(LOG *log, const CRFSNP *crfsnp);

CRFSNP_ITEM *crfsnp_dnode_find(const CRFSNP *crfsnp, const CRFSNP_DNODE *crfsnp_dnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

uint32_t crfsnp_dnode_search(const CRFSNP *crfsnp, const CRFSNP_DNODE *crfsnp_dnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

uint32_t crfsnp_dnode_insert(CRFSNP *crfsnp, const uint32_t parent_pos, const uint32_t path_seg_first_hash, const uint32_t path_seg_second_hash, const uint32_t path_seg_len, const uint8_t *path_seg, const uint32_t dir_flag);

/**
* umount one son from crfsnp_dnode,  where son is regular file item or dir item without any son
* crfsnp_dnode will be impacted on bucket and file num
**/
CRFSNP_ITEM * crfsnp_dnode_umount_son(const CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);
EC_BOOL crfsnp_dnode_delete_dir_son(const CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, CVECTOR *crfsnp_item_vec);

CRFSNP_ITEM *crfsnp_bnode_find(const CRFSNP *crfsnp, const CRFSNP_BNODE *crfsnp_bnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

uint32_t crfsnp_bnode_search(const CRFSNP *crfsnp, const CRFSNP_BNODE *crfsnp_bnode, const uint32_t first_hash, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

uint32_t crfsnp_bnode_insert(CRFSNP *crfsnp, const uint32_t parent_pos, const uint32_t path_seg_first_hash, const uint32_t path_seg_second_hash, const uint32_t path_seg_len, const uint8_t *path_seg, const uint32_t dir_flag);

/*delete one dir son, not including crfsnp_bnode itself*/
EC_BOOL crfsnp_bnode_delete_dir_son(const CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, CVECTOR *crfsnp_item_vec);

uint32_t crfsnp_search_no_lock(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

uint32_t crfsnp_search(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

uint32_t crfsnp_insert_no_lock(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

uint32_t crfsnp_insert(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

CRFSNP_ITEM *crfsnp_fetch(const CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_inode_update(CRFSNP *crfsnp, CRFSNP_INODE *crfsnp_inode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_fnode_update(CRFSNP *crfsnp, CRFSNP_FNODE *crfsnp_fnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_bucket_update(CRFSNP *crfsnp, const uint32_t node_pos, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_dnode_update(CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_bnode_update(CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_item_update(CRFSNP *crfsnp, CRFSNP_ITEM *crfsnp_item, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_update_no_lock(CRFSNP *crfsnp, 
                               const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                               const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

CRFSNP_ITEM *crfsnp_set(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

CRFSNP_ITEM *crfsnp_get(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

EC_BOOL crfsnp_delete(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag, CVECTOR *crfsnp_item_vec);

EC_BOOL crfsnp_path_name(const CRFSNP *crfsnp, const uint32_t node_pos, const uint32_t path_max_len, uint32_t *path_len, uint8_t *path);

EC_BOOL crfsnp_path_name_cstr(const CRFSNP *crfsnp, const uint32_t node_pos, CSTRING *path_cstr);

EC_BOOL crfsnp_seg_name(const CRFSNP *crfsnp, const uint32_t offset, const uint32_t seg_name_max_len, uint32_t *seg_name_len, uint8_t *seg_name);

EC_BOOL crfsnp_seg_name_cstr(const CRFSNP *crfsnp, const uint32_t offset, CSTRING *seg_cstr);

EC_BOOL crfsnp_list_path_vec(const CRFSNP *crfsnp, const uint32_t node_pos, CVECTOR *path_cstr_vec);

EC_BOOL crfsnp_list_seg_vec(const CRFSNP *crfsnp, const uint32_t node_pos, CVECTOR *seg_cstr_vec);

EC_BOOL crfsnp_file_num(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, uint32_t *file_num);

EC_BOOL crfsnp_file_size(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, uint64_t *file_size);

EC_BOOL crfsnp_mkdirs(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path);

CRFSNP *crfsnp_open(const char *np_root_dir, const uint32_t np_id);

EC_BOOL crfsnp_close(CRFSNP *crfsnp);

EC_BOOL crfsnp_sync(CRFSNP *crfsnp);

EC_BOOL crfsnp_create_root_item(CRFSNP *crfsnp);

CRFSNP *crfsnp_create(const char *np_root_dir, const uint32_t np_id, const uint8_t np_model, const uint8_t hash_1st_algo_id, const uint8_t hash_2nd_algo_id);

EC_BOOL crfsnp_show_item_full_path(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_show_item(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_show_dir(LOG *log, const CRFSNP *crfsnp, const CRFSNP_ITEM  *crfsnp_item);

EC_BOOL crfsnp_show_path(LOG *log, CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path);

EC_BOOL crfsnp_show_dir_depth(LOG *log, const CRFSNP *crfsnp, const CRFSNP_ITEM  *crfsnp_item);

EC_BOOL crfsnp_show_item_depth(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_show_path_depth(LOG *log, CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path);

EC_BOOL crfsnp_get_first_fname_of_dir(const CRFSNP *crfsnp, const CRFSNP_ITEM  *crfsnp_item, uint8_t **fname, uint32_t *dflag);

EC_BOOL crfsnp_get_first_fname_of_path(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, uint8_t **fname, uint32_t *dflag);

#endif/* _CRFSNP_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

