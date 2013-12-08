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

#ifndef _CFUSE_H
#define _CFUSE_H

#if (SWITCH_ON == CFUSE_SUPPORT_SWITCH)

#include <fuse.h>
#include <ulockmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "clist.h"

#define CFUSE_OP_CREAT      ((UINT32) 1)
#define CFUSE_OP_WRITE      ((UINT32) 2)
#define CFUSE_OP_READ       ((UINT32) 3)
#define CFUSE_OP_ERR        ((UINT32)-1)

#define CFUSE_COP_BUF_MAX_SIZE (64 * 1024 * 1024)/*64M*/

#define CFUSE_NODE_MAX_NUM  ((UINT32)1024)

typedef struct
{
    UINT32 op;

    mode_t mode;
    UINT32 flag;
    char  *path;

    void  *pvoid;

    UINT32 pos;
    UINT8  buf[CFUSE_COP_BUF_MAX_SIZE];

}CFUSE_COP;

#define CFUSE_COP_OP(cfuse_cop)           ((cfuse_cop)->op)
#define CFUSE_COP_MODE(cfuse_cop)         ((cfuse_cop)->mode)
#define CFUSE_COP_FLAG(cfuse_cop)         ((cfuse_cop)->flag)
#define CFUSE_COP_PATH(cfuse_cop)         ((cfuse_cop)->path)
#define CFUSE_COP_VOID(cfuse_cop)         ((cfuse_cop)->pvoid)
#define CFUSE_COP_POS(cfuse_cop)          ((cfuse_cop)->pos)
#define CFUSE_COP_BUF(cfuse_cop)          ((cfuse_cop)->buf)

#define CFUSE_COP_BIND(fi, cfuse_cop)              ((fi->fh) = (UINT32)(cfuse_cop))
#define CFUSE_COP_FETCH(fi)                        ((CFUSE_COP *)((fi)->fh))

#if 0
typedef struct stat   CFUSE_STAT;
typedef struct mode_t CFUSE_MODE;
#else
typedef struct
{
    UINT8    suid:1;
    UINT8    sgid:1;
    UINT8    sticky:1;
    struct
    {
        UINT8    read:1;
        UINT8    write:1;
        UINT8    exec:1;
    } owner, group, other;
} CFUSE_MODE;

typedef struct
{
    UINT64     ust_ino;        /* [ignore] inode number */
    UUID       ust_gfid;
    UINT64     ust_dev;        /* [ignore] backing device ID */
    UINT32     ust_type;       /* type of file */
    CFUSE_MODE ust_mode;       /* protection */
    UINT32     ust_nlink;      /* Link count */
    UINT32     ust_uid;        /* user ID of owner */
    UINT32     ust_gid;        /* group ID of owner */
    UINT64     ust_rdev;       /* device ID (if special file) */
    UINT64     ust_size;       /* file size in bytes */
    UINT32     ust_blksize;    /* [ignore] blocksize for filesystem I/O */
    UINT64     ust_blocks;     /* number of 512B blocks allocated */
    UINT32     ust_atime;      /* last access time */
    UINT32     ust_atime_nsec;
    UINT32     ust_mtime;      /* last modification time */
    UINT32     ust_mtime_nsec;
    UINT32     ust_ctime;      /* last status change time */
    UINT32     ust_ctime_nsec;
}CFUSE_STAT;

#define CFUSE_STAT_INO(cfuse_stat)       ((cfuse_stat)->ust_ino)
#define CFUSE_STAT_TYPE(cfuse_stat)      ((cfuse_stat)->ust_type)
#define CFUSE_STAT_MODE(cfuse_stat)      (&((cfuse_stat)->ust_mode))
#define CFUSE_STAT_SIZE(cfuse_stat)      ((cfuse_stat)->ust_size)

typedef struct
{
    CFUSE_STAT cfuse_stat;
    CSTRING   *dname_cstr;
}CFUSE_NODE;

#define CFUSE_NODE_STAT(cfuse_node)         (&((cfuse_node)->cfuse_stat))
#define CFUSE_NODE_DNAME(cfuse_node)        ((cfuse_node)->dname_cstr)

#define CFUSE_NODE_DNAME_STR(cfuse_node)    (cstring_get_str(CFUSE_NODE_DNAME(cfuse_node)))
#define CFUSE_NODE_TYPE(cfuse_node)         (CFUSE_NODE_STAT(cfuse_node)->ust_type)
#define CFUSE_NODE_IS_REG(cfuse_node)       (CDFSNP_ITEM_FILE_IS_REG == CFUSE_NODE_TYPE(cfuse_node))
#define CFUSE_NODE_IS_DIR(cfuse_node)       (CDFSNP_ITEM_FILE_IS_DIR == CFUSE_NODE_TYPE(cfuse_node))

typedef struct
{
    CLIST   cfuse_node_list;
    UINT32  cfuse_node_max_num;
}CFUSE_MGR;

#define CFUSE_MGR_NODES(cfuse_mgr)          (&((cfuse_mgr)->cfuse_node_list))
#define CFUSE_MGR_NODE_MAX_NUM(cfuse_mgr)   ((cfuse_mgr)->cfuse_node_max_num)
#define CFUSE_MGR_NODE_CUR_NUM(cfuse_mgr)   (clist_size(CFUSE_MGR_NODES(cfuse_mgr)))

#if 0
/* Linux, Solaris, Cygwin */
#define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atim.tv_nsec)
#define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctim.tv_nsec)
#define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtim.tv_nsec)
#define ST_ATIM_NSEC_SET(stbuf, val) ((stbuf)->st_atim.tv_nsec = (val))
#define ST_MTIM_NSEC_SET(stbuf, val) ((stbuf)->st_mtim.tv_nsec = (val))
#define ST_CTIM_NSEC_SET(stbuf, val) ((stbuf)->st_ctim.tv_nsec = (val))
#endif
#if 0
/* FreeBSD, NetBSD */
#define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atimespec.tv_nsec)
#define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctimespec.tv_nsec)
#define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtimespec.tv_nsec)
#define ST_ATIM_NSEC_SET(stbuf, val) ((stbuf)->st_atimespec.tv_nsec = (val))
#define ST_MTIM_NSEC_SET(stbuf, val) ((stbuf)->st_mtimespec.tv_nsec = (val))
#define ST_CTIM_NSEC_SET(stbuf, val) ((stbuf)->st_ctimespec.tv_nsec = (val))
#endif
#if 1
#define ST_ATIM_NSEC(stbuf) (0)
#define ST_CTIM_NSEC(stbuf) (0)
#define ST_MTIM_NSEC(stbuf) (0)
#define ST_ATIM_NSEC_SET(stbuf, val) do { } while (0);
#define ST_MTIM_NSEC_SET(stbuf, val) do { } while (0);
#define ST_CTIM_NSEC_SET(stbuf, val) do { } while (0);
#endif

#define ST_MAKEDEV(major, minor) \
    ((((UINT64) (major)) << 32) | (minor))

#define ST_INO(major, minor) \
    ((((UINT64) (major)) << 32) | (minor))

#define CONV_OS_MODE_TO_CFUSE_TYPE(os_st_mode, cfuse_st_type) do{\
         if (S_ISREG (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_REG;\
    else if (S_ISDIR (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_DIR;\
    else if (S_ISLNK (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_LNK;\
    else if (S_ISBLK (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_BLK;\
    else if (S_ISCHR (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_CHR;\
    else if (S_ISFIFO (os_st_mode)) (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_PIP;\
    else if (S_ISSOCK (os_st_mode)) (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_SCK;\
    else (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_ERR;\
}while(0)

#define CONV_CFUSE_TYPE_TO_OS_MODE(cfuse_st_type, os_st_mode) do{\
         if(CDFSNP_ITEM_FILE_IS_REG == (cfuse_st_type)) (os_st_mode) = S_IFREG;\
    else if(CDFSNP_ITEM_FILE_IS_DIR == (cfuse_st_type)) (os_st_mode) = S_IFDIR;\
    else if(CDFSNP_ITEM_FILE_IS_LNK == (cfuse_st_type)) (os_st_mode) = S_IFLNK;\
    else if(CDFSNP_ITEM_FILE_IS_BLK == (cfuse_st_type)) (os_st_mode) = S_IFBLK;\
    else if(CDFSNP_ITEM_FILE_IS_CHR == (cfuse_st_type)) (os_st_mode) = S_IFCHR;\
    else if(CDFSNP_ITEM_FILE_IS_PIP == (cfuse_st_type)) (os_st_mode) = S_IFIFO;\
    else if(CDFSNP_ITEM_FILE_IS_SCK == (cfuse_st_type)) (os_st_mode) = S_IFSOCK;\
    else if(CDFSNP_ITEM_FILE_IS_ANY == (cfuse_st_type)) (os_st_mode) = (S_IFREG | S_IFDIR | S_IFLNK | S_IFBLK | S_IFCHR | S_IFIFO | S_IFSOCK);\
    else (os_st_mode) = 0;\
    sys_log(LOGSTDOUT, "cfuse type %o ===> os mode %o\n", (cfuse_st_type), (os_st_mode));\
}while(0)

#define CONV_OS_MODE_TO_CFUSE_MODE(os_st_mode, cfuse_st_type, cfuse_st_mode) do{\
    CONV_OS_MODE_TO_CFUSE_TYPE(os_st_mode, cfuse_st_type);\
    if ((os_st_mode) & S_ISUID)  (cfuse_st_mode)->suid        = 1;\
    if ((os_st_mode) & S_ISGID)  (cfuse_st_mode)->sgid        = 1;\
    if ((os_st_mode) & S_ISVTX)  (cfuse_st_mode)->sticky      = 1;\
    if ((os_st_mode) & S_IRUSR)  (cfuse_st_mode)->owner.read  = 1;\
    if ((os_st_mode) & S_IWUSR)  (cfuse_st_mode)->owner.write = 1;\
    if ((os_st_mode) & S_IXUSR)  (cfuse_st_mode)->owner.exec  = 1;\
    if ((os_st_mode) & S_IRGRP)  (cfuse_st_mode)->group.read  = 1;\
    if ((os_st_mode) & S_IWGRP)  (cfuse_st_mode)->group.write = 1;\
    if ((os_st_mode) & S_IXGRP)  (cfuse_st_mode)->group.exec  = 1;\
    if ((os_st_mode) & S_IROTH)  (cfuse_st_mode)->other.read  = 1;\
    if ((os_st_mode) & S_IWOTH)  (cfuse_st_mode)->other.write = 1;\
    if ((os_st_mode) & S_IXOTH)  (cfuse_st_mode)->other.exec  = 1;\
}while(0);

#define CONV_CFUSE_MODE_TO_OS_MODE(cfuse_st_type, cfuse_st_mode, os_st_mode) do{\
    CONV_CFUSE_TYPE_TO_OS_MODE(cfuse_st_type, os_st_mode);\
    if (1 == (cfuse_st_mode)->suid        )   ((os_st_mode) |= S_ISUID) ;\
    if (1 == (cfuse_st_mode)->sgid        )   ((os_st_mode) |= S_ISGID) ;\
    if (1 == (cfuse_st_mode)->sticky      )   ((os_st_mode) |= S_ISVTX) ;\
    if (1 == (cfuse_st_mode)->owner.read  )   ((os_st_mode) |= S_IRUSR) ;\
    if (1 == (cfuse_st_mode)->owner.write )   ((os_st_mode) |= S_IWUSR) ;\
    if (1 == (cfuse_st_mode)->owner.exec  )   ((os_st_mode) |= S_IXUSR) ;\
    if (1 == (cfuse_st_mode)->group.read  )   ((os_st_mode) |= S_IRGRP) ;\
    if (1 == (cfuse_st_mode)->group.write )   ((os_st_mode) |= S_IWGRP) ;\
    if (1 == (cfuse_st_mode)->group.exec  )   ((os_st_mode) |= S_IXGRP) ;\
    if (1 == (cfuse_st_mode)->other.read  )   ((os_st_mode) |= S_IROTH) ;\
    if (1 == (cfuse_st_mode)->other.write )   ((os_st_mode) |= S_IWOTH) ;\
    if (1 == (cfuse_st_mode)->other.exec  )   ((os_st_mode) |= S_IXOTH) ;\
}while(0);


#define CONV_OS_STAT_TO_CFUSE_STAT(os_stat, cfuse_stat) do{\
    (cfuse_stat)->ust_dev        = (os_stat)->st_dev;\
    /*(cfuse_stat)->ust_ino = ST_INO(MAJOR((os_stat)->st_ino), MINOR((os_stat)->st_ino));*/\
    (cfuse_stat)->ust_ino = (os_stat)->st_ino;\
\
    (cfuse_stat)->ust_type       = (os_stat)->st_type;\
    CONV_OS_MODE_TO_CFUSE_MODE((os_stat)->st_mode, (cfuse_stat)->ust_type, &((cfuse_stat)->ust_mode));\
\
    (cfuse_stat)->ust_nlink      = (os_stat)->st_nlink;\
    (cfuse_stat)->ust_uid        = (os_stat)->st_uid;\
    (cfuse_stat)->ust_gid        = (os_stat)->st_gid;\
\
    (cfuse_stat)->ust_rdev       = ST_MAKEDEV (MAJOR ((os_stat)->st_rdev), MINOR ((os_stat)->st_rdev));\
\
    (cfuse_stat)->ust_size       = (os_stat)->st_size;\
    (cfuse_stat)->ust_blksize    = (os_stat)->st_blksize;\
    (cfuse_stat)->ust_blocks     = (os_stat)->st_blocks;\
\
    (cfuse_stat)->ust_atime      = (os_stat)->st_atime;\
    (cfuse_stat)->ust_atime_nsec = ST_ATIM_NSEC ((os_stat));\
\
    (cfuse_stat)->ust_mtime      = (os_stat)->st_mtime;\
    (cfuse_stat)->ust_mtime_nsec = ST_MTIM_NSEC ((os_stat));\
\
    (cfuse_stat)->ust_ctime      = (os_stat)->st_ctime;\
    (cfuse_stat)->ust_ctime_nsec = ST_CTIM_NSEC ((os_stat));\
}while(0)


#define CONV_CFUSE_STAT_TO_OS_STAT(cfuse_stat, os_stat) do{\
    (os_stat)->st_dev        = (cfuse_stat)->ust_dev;\
    /*(os_stat)->st_ino = ST_INO(MAJOR((cfuse_stat)->ust_ino), MINOR((cfuse_stat)->ust_ino));*/\
    (os_stat)->st_ino = (cfuse_stat)->ust_ino;\
\
    CONV_CFUSE_MODE_TO_OS_MODE((cfuse_stat)->ust_type, &((cfuse_stat)->ust_mode), (os_stat)->st_mode);\
\
    (os_stat)->st_nlink      = (cfuse_stat)->ust_nlink;\
    (os_stat)->st_uid        = (cfuse_stat)->ust_uid;\
    (os_stat)->st_gid        = (cfuse_stat)->ust_gid;\
\
    (os_stat)->st_rdev       = makedev (MAJOR ((cfuse_stat)->ust_rdev), MINOR ((cfuse_stat)->ust_rdev));\
\
    (os_stat)->st_size       = (cfuse_stat)->ust_size;\
    (os_stat)->st_blksize    = (cfuse_stat)->ust_blksize;\
    (os_stat)->st_blocks     = (cfuse_stat)->ust_blocks;\
\
    (os_stat)->st_atime      = (cfuse_stat)->ust_atime;\
    ST_ATIM_NSEC_SET ((os_stat), (cfuse_stat)->ust_atime_nsec);\
\
    (os_stat)->st_mtime      = (cfuse_stat)->ust_mtime;\
    ST_MTIM_NSEC_SET ((os_stat), (cfuse_stat)->ust_mtime_nsec);\
\
    (os_stat)->st_ctime      = (cfuse_stat)->ust_ctime;\
    ST_CTIM_NSEC_SET ((os_stat), (cfuse_stat)->ust_ctime_nsec);\
}while(0)

#endif

EC_BOOL cfuse_mode_init_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode);

EC_BOOL cfuse_mode_clean_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode);

EC_BOOL cfuse_mode_free_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode);

CFUSE_STAT *cfuse_stat_new();

EC_BOOL cfuse_stat_init(CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_clean(CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_free(CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_init_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_clean_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_free_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat);

CFUSE_NODE *cfuse_node_new();

EC_BOOL cfuse_node_init(CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_node_clean(CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_node_free(CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_mgr_int(CFUSE_MGR *cfuse_mgr, const UINT32 max_node_num);

EC_BOOL cfuse_mgr_clean(CFUSE_MGR *cfuse_mgr);

CFUSE_NODE *cfuse_mgr_search(const CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node);

CFUSE_NODE *cfuse_mgr_search_no_lock(const CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_mgr_add(CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_mgr_add_no_search(CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node);

void cfuse_mgr_print(LOG *log, const CFUSE_MGR *cfuse_mgr);

CFUSE_COP *cfuse_cop_new();

EC_BOOL cfuse_cop_init(CFUSE_COP *cfuse_cop);

EC_BOOL cfuse_cop_clean(CFUSE_COP *cfuse_cop);

EC_BOOL cfuse_cop_free(CFUSE_COP *cfuse_cop);

EC_BOOL cfuse_cop_bind(CFUSE_COP *cfuse_cop, struct fuse_file_info *fi);

EC_BOOL cfuse_cop_unbind(struct fuse_file_info *fi);

CFUSE_COP * cfuse_cop_fetch(struct fuse_file_info *fi);

void cfuse_entry(int *argc, char **argv);

#endif /*(SWITCH_ON == CFUSE_SUPPORT_SWITCH)*/

#if (SWITCH_OFF == CFUSE_SUPPORT_SWITCH)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "clist.h"

#define CFUSE_OP_CREAT      ((UINT32) 1)
#define CFUSE_OP_WRITE      ((UINT32) 2)
#define CFUSE_OP_READ       ((UINT32) 3)
#define CFUSE_OP_ERR        ((UINT32)-1)

#define CFUSE_COP_BUF_MAX_SIZE (64 * 1024 * 1024)/*64M*/

#define CFUSE_NODE_MAX_NUM  ((UINT32)1024)

typedef struct
{
    UINT32 op;

    mode_t mode;
    UINT32 flag;
    char  *path;

    void  *pvoid;

    UINT32 pos;
    UINT8  buf[CFUSE_COP_BUF_MAX_SIZE];

}CFUSE_COP;

#define CFUSE_COP_OP(cfuse_cop)           ((cfuse_cop)->op)
#define CFUSE_COP_MODE(cfuse_cop)         ((cfuse_cop)->mode)
#define CFUSE_COP_FLAG(cfuse_cop)         ((cfuse_cop)->flag)
#define CFUSE_COP_PATH(cfuse_cop)         ((cfuse_cop)->path)
#define CFUSE_COP_VOID(cfuse_cop)         ((cfuse_cop)->pvoid)
#define CFUSE_COP_POS(cfuse_cop)          ((cfuse_cop)->pos)
#define CFUSE_COP_BUF(cfuse_cop)          ((cfuse_cop)->buf)

#define CFUSE_COP_BIND(fi, cfuse_cop)              ((fi->fh) = (UINT32)(cfuse_cop))
#define CFUSE_COP_FETCH(fi)                        ((CFUSE_COP *)((fi)->fh))

#if 0
typedef struct stat   CFUSE_STAT;
typedef struct mode_t CFUSE_MODE;
#else
typedef struct
{
    UINT8    suid:1;
    UINT8    sgid:1;
    UINT8    sticky:1;
    struct
    {
        UINT8    read:1;
        UINT8    write:1;
        UINT8    exec:1;
    } owner, group, other;
} CFUSE_MODE;

typedef struct
{
    UINT64     ust_ino;        /* [ignore] inode number */
    UUID       ust_gfid;
    UINT64     ust_dev;        /* [ignore] backing device ID */
    UINT32     ust_type;       /* type of file */
    CFUSE_MODE ust_mode;       /* protection */
    UINT32     ust_nlink;      /* Link count */
    UINT32     ust_uid;        /* user ID of owner */
    UINT32     ust_gid;        /* group ID of owner */
    UINT64     ust_rdev;       /* device ID (if special file) */
    UINT64     ust_size;       /* file size in bytes */
    UINT32     ust_blksize;    /* [ignore] blocksize for filesystem I/O */
    UINT64     ust_blocks;     /* number of 512B blocks allocated */
    UINT32     ust_atime;      /* last access time */
    UINT32     ust_atime_nsec;
    UINT32     ust_mtime;      /* last modification time */
    UINT32     ust_mtime_nsec;
    UINT32     ust_ctime;      /* last status change time */
    UINT32     ust_ctime_nsec;
}CFUSE_STAT;

#define CFUSE_STAT_INO(cfuse_stat)       ((cfuse_stat)->ust_ino)
#define CFUSE_STAT_TYPE(cfuse_stat)      ((cfuse_stat)->ust_type)
#define CFUSE_STAT_MODE(cfuse_stat)      (&((cfuse_stat)->ust_mode))
#define CFUSE_STAT_SIZE(cfuse_stat)      ((cfuse_stat)->ust_size)

typedef struct
{
    CFUSE_STAT cfuse_stat;
    CSTRING   *dname_cstr;
}CFUSE_NODE;

#define CFUSE_NODE_STAT(cfuse_node)         (&((cfuse_node)->cfuse_stat))
#define CFUSE_NODE_DNAME(cfuse_node)        ((cfuse_node)->dname_cstr)

#define CFUSE_NODE_DNAME_STR(cfuse_node)    (cstring_get_str(CFUSE_NODE_DNAME(cfuse_node)))
#define CFUSE_NODE_TYPE(cfuse_node)         (CFUSE_NODE_STAT(cfuse_node)->ust_type)
#define CFUSE_NODE_IS_REG(cfuse_node)       (CDFSNP_ITEM_FILE_IS_REG == CFUSE_NODE_TYPE(cfuse_node))
#define CFUSE_NODE_IS_DIR(cfuse_node)       (CDFSNP_ITEM_FILE_IS_DIR == CFUSE_NODE_TYPE(cfuse_node))

typedef struct
{
    CLIST   cfuse_node_list;
    UINT32  cfuse_node_max_num;
}CFUSE_MGR;

#define CFUSE_MGR_NODES(cfuse_mgr)          (&((cfuse_mgr)->cfuse_node_list))
#define CFUSE_MGR_NODE_MAX_NUM(cfuse_mgr)   ((cfuse_mgr)->cfuse_node_max_num)
#define CFUSE_MGR_NODE_CUR_NUM(cfuse_mgr)   (clist_size(CFUSE_MGR_NODES(cfuse_mgr)))

#if 0
/* Linux, Solaris, Cygwin */
#define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atim.tv_nsec)
#define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctim.tv_nsec)
#define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtim.tv_nsec)
#define ST_ATIM_NSEC_SET(stbuf, val) ((stbuf)->st_atim.tv_nsec = (val))
#define ST_MTIM_NSEC_SET(stbuf, val) ((stbuf)->st_mtim.tv_nsec = (val))
#define ST_CTIM_NSEC_SET(stbuf, val) ((stbuf)->st_ctim.tv_nsec = (val))
#endif
#if 0
/* FreeBSD, NetBSD */
#define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atimespec.tv_nsec)
#define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctimespec.tv_nsec)
#define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtimespec.tv_nsec)
#define ST_ATIM_NSEC_SET(stbuf, val) ((stbuf)->st_atimespec.tv_nsec = (val))
#define ST_MTIM_NSEC_SET(stbuf, val) ((stbuf)->st_mtimespec.tv_nsec = (val))
#define ST_CTIM_NSEC_SET(stbuf, val) ((stbuf)->st_ctimespec.tv_nsec = (val))
#endif
#if 1
#define ST_ATIM_NSEC(stbuf) (0)
#define ST_CTIM_NSEC(stbuf) (0)
#define ST_MTIM_NSEC(stbuf) (0)
#define ST_ATIM_NSEC_SET(stbuf, val) do { } while (0);
#define ST_MTIM_NSEC_SET(stbuf, val) do { } while (0);
#define ST_CTIM_NSEC_SET(stbuf, val) do { } while (0);
#endif

#define ST_MAKEDEV(major, minor) \
    ((((UINT64) (major)) << 32) | (minor))

#define ST_INO(major, minor) \
    ((((UINT64) (major)) << 32) | (minor))

#define CONV_OS_MODE_TO_CFUSE_TYPE(os_st_mode, cfuse_st_type) do{\
         if (S_ISREG (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_REG;\
    else if (S_ISDIR (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_DIR;\
    else if (S_ISLNK (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_LNK;\
    else if (S_ISBLK (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_BLK;\
    else if (S_ISCHR (os_st_mode))  (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_CHR;\
    else if (S_ISFIFO (os_st_mode)) (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_PIP;\
    else if (S_ISSOCK (os_st_mode)) (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_SCK;\
    else (cfuse_st_type) = CDFSNP_ITEM_FILE_IS_ERR;\
}while(0)

#define CONV_CFUSE_TYPE_TO_OS_MODE(cfuse_st_type, os_st_mode) do{\
         if(CDFSNP_ITEM_FILE_IS_REG == (cfuse_st_type)) (os_st_mode) = S_IFREG;\
    else if(CDFSNP_ITEM_FILE_IS_DIR == (cfuse_st_type)) (os_st_mode) = S_IFDIR;\
    else if(CDFSNP_ITEM_FILE_IS_LNK == (cfuse_st_type)) (os_st_mode) = S_IFLNK;\
    else if(CDFSNP_ITEM_FILE_IS_BLK == (cfuse_st_type)) (os_st_mode) = S_IFBLK;\
    else if(CDFSNP_ITEM_FILE_IS_CHR == (cfuse_st_type)) (os_st_mode) = S_IFCHR;\
    else if(CDFSNP_ITEM_FILE_IS_PIP == (cfuse_st_type)) (os_st_mode) = S_IFIFO;\
    else if(CDFSNP_ITEM_FILE_IS_SCK == (cfuse_st_type)) (os_st_mode) = S_IFSOCK;\
    else if(CDFSNP_ITEM_FILE_IS_ANY == (cfuse_st_type)) (os_st_mode) = (S_IFREG | S_IFDIR | S_IFLNK | S_IFBLK | S_IFCHR | S_IFIFO | S_IFSOCK);\
    else (os_st_mode) = 0;\
    sys_log(LOGSTDOUT, "cfuse type %o ===> os mode %o\n", (cfuse_st_type), (os_st_mode));\
}while(0)

#define CONV_OS_MODE_TO_CFUSE_MODE(os_st_mode, cfuse_st_type, cfuse_st_mode) do{\
    CONV_OS_MODE_TO_CFUSE_TYPE(os_st_mode, cfuse_st_type);\
    if ((os_st_mode) & S_ISUID)  (cfuse_st_mode)->suid        = 1;\
    if ((os_st_mode) & S_ISGID)  (cfuse_st_mode)->sgid        = 1;\
    if ((os_st_mode) & S_ISVTX)  (cfuse_st_mode)->sticky      = 1;\
    if ((os_st_mode) & S_IRUSR)  (cfuse_st_mode)->owner.read  = 1;\
    if ((os_st_mode) & S_IWUSR)  (cfuse_st_mode)->owner.write = 1;\
    if ((os_st_mode) & S_IXUSR)  (cfuse_st_mode)->owner.exec  = 1;\
    if ((os_st_mode) & S_IRGRP)  (cfuse_st_mode)->group.read  = 1;\
    if ((os_st_mode) & S_IWGRP)  (cfuse_st_mode)->group.write = 1;\
    if ((os_st_mode) & S_IXGRP)  (cfuse_st_mode)->group.exec  = 1;\
    if ((os_st_mode) & S_IROTH)  (cfuse_st_mode)->other.read  = 1;\
    if ((os_st_mode) & S_IWOTH)  (cfuse_st_mode)->other.write = 1;\
    if ((os_st_mode) & S_IXOTH)  (cfuse_st_mode)->other.exec  = 1;\
}while(0);

#define CONV_CFUSE_MODE_TO_OS_MODE(cfuse_st_type, cfuse_st_mode, os_st_mode) do{\
    CONV_CFUSE_TYPE_TO_OS_MODE(cfuse_st_type, os_st_mode);\
    if (1 == (cfuse_st_mode)->suid        )   ((os_st_mode) |= S_ISUID) ;\
    if (1 == (cfuse_st_mode)->sgid        )   ((os_st_mode) |= S_ISGID) ;\
    if (1 == (cfuse_st_mode)->sticky      )   ((os_st_mode) |= S_ISVTX) ;\
    if (1 == (cfuse_st_mode)->owner.read  )   ((os_st_mode) |= S_IRUSR) ;\
    if (1 == (cfuse_st_mode)->owner.write )   ((os_st_mode) |= S_IWUSR) ;\
    if (1 == (cfuse_st_mode)->owner.exec  )   ((os_st_mode) |= S_IXUSR) ;\
    if (1 == (cfuse_st_mode)->group.read  )   ((os_st_mode) |= S_IRGRP) ;\
    if (1 == (cfuse_st_mode)->group.write )   ((os_st_mode) |= S_IWGRP) ;\
    if (1 == (cfuse_st_mode)->group.exec  )   ((os_st_mode) |= S_IXGRP) ;\
    if (1 == (cfuse_st_mode)->other.read  )   ((os_st_mode) |= S_IROTH) ;\
    if (1 == (cfuse_st_mode)->other.write )   ((os_st_mode) |= S_IWOTH) ;\
    if (1 == (cfuse_st_mode)->other.exec  )   ((os_st_mode) |= S_IXOTH) ;\
}while(0);


#define CONV_OS_STAT_TO_CFUSE_STAT(os_stat, cfuse_stat) do{\
    (cfuse_stat)->ust_dev        = (os_stat)->st_dev;\
    /*(cfuse_stat)->ust_ino = ST_INO(MAJOR((os_stat)->st_ino), MINOR((os_stat)->st_ino));*/\
    (cfuse_stat)->ust_ino = (os_stat)->st_ino;\
\
    (cfuse_stat)->ust_type       = (os_stat)->st_type;\
    CONV_OS_MODE_TO_CFUSE_MODE((os_stat)->st_mode, (cfuse_stat)->ust_type, &((cfuse_stat)->ust_mode));\
\
    (cfuse_stat)->ust_nlink      = (os_stat)->st_nlink;\
    (cfuse_stat)->ust_uid        = (os_stat)->st_uid;\
    (cfuse_stat)->ust_gid        = (os_stat)->st_gid;\
\
    (cfuse_stat)->ust_rdev       = ST_MAKEDEV (MAJOR ((os_stat)->st_rdev), MINOR ((os_stat)->st_rdev));\
\
    (cfuse_stat)->ust_size       = (os_stat)->st_size;\
    (cfuse_stat)->ust_blksize    = (os_stat)->st_blksize;\
    (cfuse_stat)->ust_blocks     = (os_stat)->st_blocks;\
\
    (cfuse_stat)->ust_atime      = (os_stat)->st_atime;\
    (cfuse_stat)->ust_atime_nsec = ST_ATIM_NSEC ((os_stat));\
\
    (cfuse_stat)->ust_mtime      = (os_stat)->st_mtime;\
    (cfuse_stat)->ust_mtime_nsec = ST_MTIM_NSEC ((os_stat));\
\
    (cfuse_stat)->ust_ctime      = (os_stat)->st_ctime;\
    (cfuse_stat)->ust_ctime_nsec = ST_CTIM_NSEC ((os_stat));\
}while(0)


#define CONV_CFUSE_STAT_TO_OS_STAT(cfuse_stat, os_stat) do{\
    (os_stat)->st_dev        = (cfuse_stat)->ust_dev;\
    /*(os_stat)->st_ino = ST_INO(MAJOR((cfuse_stat)->ust_ino), MINOR((cfuse_stat)->ust_ino));*/\
    (os_stat)->st_ino = (cfuse_stat)->ust_ino;\
\
    CONV_CFUSE_MODE_TO_OS_MODE((cfuse_stat)->ust_type, &((cfuse_stat)->ust_mode), (os_stat)->st_mode);\
\
    (os_stat)->st_nlink      = (cfuse_stat)->ust_nlink;\
    (os_stat)->st_uid        = (cfuse_stat)->ust_uid;\
    (os_stat)->st_gid        = (cfuse_stat)->ust_gid;\
\
    (os_stat)->st_rdev       = makedev (MAJOR ((cfuse_stat)->ust_rdev), MINOR ((cfuse_stat)->ust_rdev));\
\
    (os_stat)->st_size       = (cfuse_stat)->ust_size;\
    (os_stat)->st_blksize    = (cfuse_stat)->ust_blksize;\
    (os_stat)->st_blocks     = (cfuse_stat)->ust_blocks;\
\
    (os_stat)->st_atime      = (cfuse_stat)->ust_atime;\
    ST_ATIM_NSEC_SET ((os_stat), (cfuse_stat)->ust_atime_nsec);\
\
    (os_stat)->st_mtime      = (cfuse_stat)->ust_mtime;\
    ST_MTIM_NSEC_SET ((os_stat), (cfuse_stat)->ust_mtime_nsec);\
\
    (os_stat)->st_ctime      = (cfuse_stat)->ust_ctime;\
    ST_CTIM_NSEC_SET ((os_stat), (cfuse_stat)->ust_ctime_nsec);\
}while(0)

#endif

EC_BOOL cfuse_mode_init_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode);

EC_BOOL cfuse_mode_clean_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode);

EC_BOOL cfuse_mode_free_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode);

CFUSE_STAT *cfuse_stat_new();

EC_BOOL cfuse_stat_init(CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_clean(CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_free(CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_init_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_clean_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat);

EC_BOOL cfuse_stat_free_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat);

CFUSE_NODE *cfuse_node_new();

EC_BOOL cfuse_node_init(CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_node_clean(CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_node_free(CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_mgr_int(CFUSE_MGR *cfuse_mgr, const UINT32 max_node_num);

EC_BOOL cfuse_mgr_clean(CFUSE_MGR *cfuse_mgr);

CFUSE_NODE *cfuse_mgr_search(const CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node);

CFUSE_NODE *cfuse_mgr_search_no_lock(const CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_mgr_add(CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node);

EC_BOOL cfuse_mgr_add_no_search(CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node);

void cfuse_mgr_print(LOG *log, const CFUSE_MGR *cfuse_mgr);

CFUSE_COP *cfuse_cop_new();

EC_BOOL cfuse_cop_init(CFUSE_COP *cfuse_cop);

EC_BOOL cfuse_cop_clean(CFUSE_COP *cfuse_cop);

EC_BOOL cfuse_cop_free(CFUSE_COP *cfuse_cop);

EC_BOOL cfuse_cop_bind(CFUSE_COP *cfuse_cop, void *fi);

EC_BOOL cfuse_cop_unbind(void *fi);

CFUSE_COP * cfuse_cop_fetch(void *fi);

void cfuse_entry(int *argc, char **argv);
#endif/*(SWITCH_OFF == CFUSE_SUPPORT_SWITCH)*/

#endif /*_CFUSE_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

