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

#if (SWITCH_ON == CFUSE_SUPPORT_SWITCH)

#define FUSE_USE_VERSION 26

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
#include "cvector.h"
#include "cfuse.h"

#include "task.h"
#include "cdfs.h"

#include "clist.h"

#if 0
/usr/include/asm-generic/errno-base.h
=====================================

#define EPERM            1      /* Operation not permitted */
#define ENOENT           2      /* No such file or directory */
#define ESRCH            3      /* No such process */
#define EINTR            4      /* Interrupted system call */
#define EIO              5      /* I/O error */
#define ENXIO            6      /* No such device or address */
#define E2BIG            7      /* Argument list too long */
#define ENOEXEC          8      /* Exec format error */
#define EBADF            9      /* Bad file number */
#define ECHILD          10      /* No child processes */
#define EAGAIN          11      /* Try again */
#define ENOMEM          12      /* Out of memory */
#define EACCES          13      /* Permission denied */
#define EFAULT          14      /* Bad address */
#define ENOTBLK         15      /* Block device required */
#define EBUSY           16      /* Device or resource busy */
#define EEXIST          17      /* File exists */
#define EXDEV           18      /* Cross-device link */
#define ENODEV          19      /* No such device */
#define ENOTDIR         20      /* Not a directory */
#define EISDIR          21      /* Is a directory */
#define EINVAL          22      /* Invalid argument */
#define ENFILE          23      /* File table overflow */
#define EMFILE          24      /* Too many open files */
#define ENOTTY          25      /* Not a typewriter */
#define ETXTBSY         26      /* Text file busy */
#define EFBIG           27      /* File too large */
#define ENOSPC          28      /* No space left on device */
#define ESPIPE          29      /* Illegal seek */
#define EROFS           30      /* Read-only file system */
#define EMLINK          31      /* Too many links */
#define EPIPE           32      /* Broken pipe */
#define EDOM            33      /* Math argument out of domain of func */
#define ERANGE          34      /* Math result not representable */
#endif

#if 0
O_RDONLY = 0x0000
O_RDWR   = 0x0002
O_CREAT  = 0x0040
#endif

static CFUSE_MGR g_cfuse_mgr;

#define CFUSE_COP_ASSERT(cfuse_cop, func_name) do{\
    if(NULL_PTR == (cfuse_cop)) {\
        sys_log(LOGSTDOUT, "error:%s: cfuse_cop is null\n", (func_name));\
        return (-1);\
    }\
}while(0)

static UINT32 cfuse_get_cdfs_md_id()
{
    return (0);
}

static CFUSE_MGR * cfuse_mgr_get()
{
    return (&g_cfuse_mgr);
}

EC_BOOL cfuse_mode_init_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode)
{
    memset(cfuse_mode, 0, sizeof(CFUSE_MODE));
    return (EC_TRUE);
}

EC_BOOL cfuse_mode_clean_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode)
{
    memset(cfuse_mode, 0, sizeof(CFUSE_MODE));
    return (EC_TRUE);
}

EC_BOOL cfuse_mode_free_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode)
{
    cfuse_mode_clean_0(md_id, cfuse_mode);
    free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CFUSE_MODE, cfuse_mode, LOC_CFUSE_0001);
    return (EC_TRUE);
}

CFUSE_STAT *cfuse_stat_new()
{
    CFUSE_STAT *cfuse_stat;
    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CFUSE_STAT, &cfuse_stat, LOC_CFUSE_0002);
    if(NULL_PTR != cfuse_stat)
    {
        cfuse_stat_init(cfuse_stat);
    }
    return (cfuse_stat);
}

EC_BOOL cfuse_stat_init(CFUSE_STAT *cfuse_stat)
{
    memset(cfuse_stat, 0, sizeof(CFUSE_STAT));
    CFUSE_STAT_TYPE(cfuse_stat) = CDFSNP_ITEM_FILE_IS_ERR;
    return (EC_TRUE);
}

EC_BOOL cfuse_stat_clean(CFUSE_STAT *cfuse_stat)
{
    memset(cfuse_stat, 0, sizeof(CFUSE_STAT));
    CFUSE_STAT_TYPE(cfuse_stat) = CDFSNP_ITEM_FILE_IS_ERR;
    return (EC_TRUE);
}

EC_BOOL cfuse_stat_free(CFUSE_STAT *cfuse_stat)
{
    if(NULL_PTR != cfuse_stat)
    {
        cfuse_stat_clean(cfuse_stat);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CFUSE_STAT, cfuse_stat, LOC_CFUSE_0003);
    }
    return (EC_TRUE);
}

EC_BOOL cfuse_stat_init_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat)
{
    return cfuse_stat_init(cfuse_stat);
}

EC_BOOL cfuse_stat_clean_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat)
{
    return cfuse_stat_clean(cfuse_stat);
}

EC_BOOL cfuse_stat_free_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat)
{
    return cfuse_stat_free(cfuse_stat);
}

CFUSE_NODE *cfuse_node_new()
{
    CFUSE_NODE *cfuse_node;
    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CFUSE_NODE, &cfuse_node, LOC_CFUSE_0004);
    if(NULL_PTR != cfuse_node)
    {
        cfuse_node_init(cfuse_node);
    }
    return (cfuse_node);
}

EC_BOOL cfuse_node_init(CFUSE_NODE *cfuse_node)
{
    cfuse_stat_init(CFUSE_NODE_STAT(cfuse_node));
    CFUSE_NODE_DNAME(cfuse_node) = NULL_PTR;
    return (EC_TRUE);
}

EC_BOOL cfuse_node_clean(CFUSE_NODE *cfuse_node)
{
    cfuse_stat_clean(CFUSE_NODE_STAT(cfuse_node));

    if(NULL_PTR != CFUSE_NODE_DNAME(cfuse_node))
    {
        cstring_free(CFUSE_NODE_DNAME(cfuse_node));
        CFUSE_NODE_DNAME(cfuse_node) = NULL_PTR;
    }
    return (EC_TRUE);
}

EC_BOOL cfuse_node_free(CFUSE_NODE *cfuse_node)
{
    if(NULL_PTR != cfuse_node)
    {
        cfuse_node_clean(cfuse_node);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CFUSE_NODE, cfuse_node, LOC_CFUSE_0005);
    }
    return (EC_TRUE);
}

EC_BOOL cfuse_node_set(CFUSE_NODE *cfuse_node, const char *path, const UINT32 type)
{
    if(NULL_PTR != CFUSE_NODE_DNAME(cfuse_node))
    {
        cstring_free(CFUSE_NODE_DNAME(cfuse_node));
        CFUSE_NODE_DNAME(cfuse_node) = NULL_PTR;
    }
    CFUSE_NODE_DNAME(cfuse_node) = cstring_new((UINT8 *)path, LOC_CFUSE_0006);
    if(NULL_PTR == CFUSE_NODE_DNAME(cfuse_node))
    {
        sys_log(LOGSTDOUT, "error:cfuse_node_set: set failed where path = %s, type = %ld\n", path, type);
        return (EC_FALSE);
    }

    CFUSE_NODE_TYPE(cfuse_node) = type;
    return (EC_TRUE);
}

EC_BOOL cfuse_node_cmp(const CFUSE_NODE *cfuse_node_1st, const CFUSE_NODE *cfuse_node_2nd)
{

    if(CFUSE_NODE_TYPE(cfuse_node_1st) != CFUSE_NODE_TYPE(cfuse_node_2nd)
    && CDFSNP_ITEM_FILE_IS_ANY != CFUSE_NODE_TYPE(cfuse_node_1st)
    && CDFSNP_ITEM_FILE_IS_ANY != CFUSE_NODE_TYPE(cfuse_node_2nd)
    )
    {
        return (EC_FALSE);
    }

    if(0 != strcmp((char *)CFUSE_NODE_DNAME_STR(cfuse_node_1st), (char *)CFUSE_NODE_DNAME_STR(cfuse_node_2nd)))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

void cfuse_node_print(LOG *log, const CFUSE_NODE *cfuse_node)
{
    sys_log(log, "type = %ld, dname = %s\n", CFUSE_NODE_TYPE(cfuse_node), (char *)CFUSE_NODE_DNAME_STR(cfuse_node));
}

EC_BOOL cfuse_mgr_int(CFUSE_MGR *cfuse_mgr, const UINT32 max_node_num)
{
    clist_init(CFUSE_MGR_NODES(cfuse_mgr), MM_IGNORE, LOC_CFUSE_0007);
    CFUSE_MGR_NODE_MAX_NUM(cfuse_mgr) = max_node_num;
    return (EC_TRUE);
}

EC_BOOL cfuse_mgr_clean(CFUSE_MGR *cfuse_mgr)
{
    clist_clean(CFUSE_MGR_NODES(cfuse_mgr), (CLIST_DATA_DATA_CLEANER)cfuse_node_free);
    CFUSE_MGR_NODE_MAX_NUM(cfuse_mgr) = 0;
    return (EC_TRUE);
}

CFUSE_NODE *cfuse_mgr_search(const CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node)
{
    CLIST_DATA *clist_data;
    clist_data = clist_search_front(CFUSE_MGR_NODES(cfuse_mgr), (void *)cfuse_node, (CLIST_DATA_DATA_CMP)cfuse_node_cmp);
    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }
    return (CFUSE_NODE *)CLIST_DATA_DATA(clist_data);
}

CFUSE_NODE * cfuse_mgr_search_no_lock(const CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node)
{
    CLIST_DATA *clist_data;
    clist_data = clist_search_front_no_lock(CFUSE_MGR_NODES(cfuse_mgr), (void *)cfuse_node, (CLIST_DATA_DATA_CMP)cfuse_node_cmp);
    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }
    return (CFUSE_NODE *)CLIST_DATA_DATA(clist_data);
}

EC_BOOL cfuse_mgr_add(CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node)
{
    CLIST_LOCK(CFUSE_MGR_NODES(cfuse_mgr), LOC_CFUSE_0008);
    if(EC_TRUE == clist_search_front_no_lock(CFUSE_MGR_NODES(cfuse_mgr), (void *)cfuse_node, (CLIST_DATA_DATA_CMP)cfuse_node_cmp))
    {
        CLIST_UNLOCK(CFUSE_MGR_NODES(cfuse_mgr), LOC_CFUSE_0009);
        return (EC_TRUE);
    }

    if(CFUSE_MGR_NODE_CUR_NUM(cfuse_mgr) >= CFUSE_MGR_NODE_MAX_NUM(cfuse_mgr))
    {
        CFUSE_NODE *cfuse_node_rmv;
        cfuse_node_rmv = (CFUSE_NODE *)clist_pop_front_no_lock(CFUSE_MGR_NODES(cfuse_mgr));
        cfuse_node_free(cfuse_node_rmv);
    }
    clist_push_back_no_lock(CFUSE_MGR_NODES(cfuse_mgr), (void *)cfuse_node);
    CLIST_UNLOCK(CFUSE_MGR_NODES(cfuse_mgr), LOC_CFUSE_0010);
    return (EC_TRUE);
}

EC_BOOL cfuse_mgr_add_no_search(CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node)
{
    CLIST_LOCK(CFUSE_MGR_NODES(cfuse_mgr), LOC_CFUSE_0011);
    if(CFUSE_MGR_NODE_CUR_NUM(cfuse_mgr) >= CFUSE_MGR_NODE_MAX_NUM(cfuse_mgr))
    {
        CFUSE_NODE *cfuse_node_rmv;
        cfuse_node_rmv = (CFUSE_NODE *)clist_pop_front_no_lock(CFUSE_MGR_NODES(cfuse_mgr));
        cfuse_node_free(cfuse_node_rmv);
    }
    clist_push_back_no_lock(CFUSE_MGR_NODES(cfuse_mgr), (void *)cfuse_node);
    CLIST_UNLOCK(CFUSE_MGR_NODES(cfuse_mgr), LOC_CFUSE_0012);
    return (EC_TRUE);
}

void cfuse_mgr_print(LOG *log, const CFUSE_MGR *cfuse_mgr)
{
    sys_log(log, "cfuse mgr %lx, max node num %ld, cur node_num %ld\n",
                 cfuse_mgr, CFUSE_MGR_NODE_MAX_NUM(cfuse_mgr), CFUSE_MGR_NODE_CUR_NUM(cfuse_mgr));
    clist_print(log, CFUSE_MGR_NODES(cfuse_mgr), (CLIST_DATA_DATA_PRINT)cfuse_node_print);
}

void cfuse_mode_print(LOG *log, CFUSE_MODE *cfuse_mode)
{
    static char r_str[] = {'-', 'r'};
    static char w_str[] = {'-', 'w'};
    static char x_str[] = {'-', 'x'};

    sys_log(log, "suid  : %ld\n", cfuse_mode->suid);
    sys_log(log, "sgid  : %ld\n", cfuse_mode->sgid);
    sys_log(log, "sticky: %ld\n", cfuse_mode->sticky);

    sys_log(log, "owner: %c%c%c\n", r_str[cfuse_mode->owner.read], w_str[cfuse_mode->owner.write], x_str[cfuse_mode->owner.exec]);
    sys_log(log, "group: %c%c%c\n", r_str[cfuse_mode->group.read], w_str[cfuse_mode->group.write], x_str[cfuse_mode->group.exec]);
    sys_log(log, "other: %c%c%c\n", r_str[cfuse_mode->other.read], w_str[cfuse_mode->other.write], x_str[cfuse_mode->other.exec]);
    return;
}

void cfuse_stat_print(LOG *log, CFUSE_STAT *cfuse_stat)
{
    sys_log(log, "ust_ino : major = %lx, minor = %lx\n", MAJOR(CFUSE_STAT_INO(cfuse_stat)), MINOR(CFUSE_STAT_INO(cfuse_stat)));
    sys_log(log, "ust_type: %ld\n", CFUSE_STAT_TYPE(cfuse_stat));
    sys_log(log, "ust_mode: \n");
    cfuse_mode_print(log, CFUSE_STAT_MODE(cfuse_stat));
    sys_log(log, "ust_nlink: %ld\n", cfuse_stat->ust_nlink);
    sys_log(log, "ust_uid  : %ld\n", cfuse_stat->ust_uid);
    sys_log(log, "ust_gid  : %ld\n", cfuse_stat->ust_gid);
    sys_log(log, "ust_size : %ld\n", cfuse_stat->ust_size);
}

void cfuse_os_mode_print(LOG *log, mode_t os_mode)
{
    sys_log(log, "os_mode = %o:\n", os_mode);
    if(S_ISSOCK(os_mode))   sys_log(log, "=> socket                                          \n");
    if(S_ISLNK (os_mode))   sys_log(log, "=> symbolic link                                   \n");
    if(S_ISREG (os_mode))   sys_log(log, "=> regular file                                    \n");
    if(S_ISBLK (os_mode))   sys_log(log, "=> block device                                    \n");
    if(S_ISDIR (os_mode))   sys_log(log, "=> directory                                       \n");
    if(S_ISCHR (os_mode))   sys_log(log, "=> character device                                \n");
    if(S_ISFIFO(os_mode))   sys_log(log, "=> FIFO                                            \n");

    if(S_ISUID  & os_mode)   sys_log(log, "S_ISUID  = 0004000 , was set, set UID bit                                     \n");
    if(S_ISGID  & os_mode)   sys_log(log, "S_ISGID  = 0002000 , was set, set-group-ID bit (see below)                    \n");
    if(S_ISVTX  & os_mode)   sys_log(log, "S_ISVTX  = 0001000 , was set,  sticky bit (see below)                         \n");
    //if(S_IRWXU  & os_mode)   sys_log(log, "S_IRWXU  = 00700   , was set, mask for file owner permissions                 \n");
    if(S_IRUSR  & os_mode)   sys_log(log, "S_IRUSR  = 00400   , was set, owner has read permission                       \n");
    if(S_IWUSR  & os_mode)   sys_log(log, "S_IWUSR  = 00200   , was set, owner has write permission                      \n");
    if(S_IXUSR  & os_mode)   sys_log(log, "S_IXUSR  = 00100   , was set, owner has execute permission                    \n");
    //if(S_IRWXG  & os_mode)   sys_log(log, "S_IRWXG  = 00070   , was set, mask for group permissions                      \n");
    if(S_IRGRP  & os_mode)   sys_log(log, "S_IRGRP  = 00040   , was set, group has read permission                       \n");
    if(S_IWGRP  & os_mode)   sys_log(log, "S_IWGRP  = 00020   , was set, group has write permission                      \n");
    if(S_IXGRP  & os_mode)   sys_log(log, "S_IXGRP  = 00010   , was set, group has execute permission                    \n");
    //if(S_IRWXO  & os_mode)   sys_log(log, "S_IRWXO  = 00007   , was set, mask for permissions for others (not in group)  \n");
    if(S_IROTH  & os_mode)   sys_log(log, "S_IROTH  = 00004   , was set, others have read permission                     \n");
    if(S_IWOTH  & os_mode)   sys_log(log, "S_IWOTH  = 00002   , was set, others have write permission                    \n");
    if(S_IXOTH  & os_mode)   sys_log(log, "S_IXOTH  = 00001   , was set, others have execute permission                  \n");
}

#if 0
man lstat
======
          struct stat {
              dev_t     st_dev;     /* ID of device containing file */
              ino_t     st_ino;     /* inode number */
              mode_t    st_mode;    /* protection */
              nlink_t   st_nlink;   /* number of hard links */
              uid_t     st_uid;     /* user ID of owner */
              gid_t     st_gid;     /* group ID of owner */
              dev_t     st_rdev;    /* device ID (if special file) */
              off_t     st_size;    /* total size, in bytes */
              blksize_t st_blksize; /* blocksize for filesystem I/O */
              blkcnt_t  st_blocks;  /* number of blocks allocated */
              time_t    st_atime;   /* time of last access */
              time_t    st_mtime;   /* time of last modification */
              time_t    st_ctime;   /* time of last status change */
          };
#endif
void cfuse_os_stat_print(LOG *log, struct stat *os_stat)
{
    sys_log(log, "st_dev  : %lx-%lx\n", MAJOR(os_stat->st_dev), MINOR(os_stat->st_dev));
    sys_log(log, "st_rdev : %lx-%lx\n", MAJOR(os_stat->st_rdev), MINOR(os_stat->st_rdev));
    sys_log(log, "st_ino  : %lx-%lx\n", MAJOR(os_stat->st_ino), MINOR(os_stat->st_ino));
    //sys_log(log, "st_mode : 0x%lx\n", os_stat->st_mode);
    cfuse_os_mode_print(log, os_stat->st_mode);
    sys_log(log, "st_nlink: %lx-%lx\n", MAJOR(os_stat->st_nlink), MINOR(os_stat->st_nlink));
    sys_log(log, "st_uid  : %lx-%lx\n", MAJOR(os_stat->st_uid), MINOR(os_stat->st_uid));
    sys_log(log, "st_gid  : %lx-%lx\n", MAJOR(os_stat->st_gid), MINOR(os_stat->st_gid));
    sys_log(log, "st_size : %lx-%lx\n", MAJOR(os_stat->st_size), MINOR(os_stat->st_size));
}

CFUSE_COP *cfuse_cop_new()
{
    CFUSE_COP *cfuse_cop;
    alloc_static_mem(MD_CDFS, 0, MM_CFUSE_COP, &cfuse_cop, LOC_CFUSE_0013);
    if(NULL_PTR != cfuse_cop)
    {
        cfuse_cop_init(cfuse_cop);
    }
    return (cfuse_cop);
}

EC_BOOL cfuse_cop_init(CFUSE_COP *cfuse_cop)
{
    CFUSE_COP_OP(cfuse_cop)   = CFUSE_OP_ERR;
    CFUSE_COP_MODE(cfuse_cop) = 0;
    CFUSE_COP_FLAG(cfuse_cop) = 0;
    CFUSE_COP_PATH(cfuse_cop) = NULL_PTR;
    CFUSE_COP_VOID(cfuse_cop) = NULL_PTR;
    CFUSE_COP_POS(cfuse_cop)  = 0;
    return (EC_TRUE);
}

EC_BOOL cfuse_cop_clean(CFUSE_COP *cfuse_cop)
{
    CFUSE_COP_OP(cfuse_cop)   = CFUSE_OP_ERR;
    CFUSE_COP_MODE(cfuse_cop) = 0;
    CFUSE_COP_FLAG(cfuse_cop) = 0;
    CFUSE_COP_PATH(cfuse_cop) = NULL_PTR;
    CFUSE_COP_VOID(cfuse_cop) = NULL_PTR;
    CFUSE_COP_POS(cfuse_cop)  = 0;
    return (EC_TRUE);
}

EC_BOOL cfuse_cop_free(CFUSE_COP *cfuse_cop)
{
    if(NULL_PTR != cfuse_cop)
    {
        cfuse_cop_clean(cfuse_cop);
        free_static_mem(MD_CDFS, 0, MM_CFUSE_COP, cfuse_cop, LOC_CFUSE_0014);
    }
    return (EC_TRUE);
}

EC_BOOL cfuse_cop_bind(CFUSE_COP *cfuse_cop, struct fuse_file_info *fi)
{
    fi->fh = (UINT32)cfuse_cop;
    return (EC_TRUE);
}

EC_BOOL cfuse_cop_unbind(struct fuse_file_info *fi)
{
    CFUSE_COP *cfuse_cop;

    cfuse_cop = (CFUSE_COP *)fi->fh;
    if(NULL_PTR != cfuse_cop)
    {
        cfuse_cop_free(cfuse_cop);
        fi->fh = 0;
    }

    return (EC_TRUE);
}

CFUSE_COP * cfuse_cop_fetch(struct fuse_file_info *fi)
{
    CFUSE_COP *cfuse_cop;

    cfuse_cop = (CFUSE_COP *)(fi->fh);
    if(NULL_PTR == cfuse_cop)
    {
        sys_log(LOGSTDOUT, "error:cfuse_cop_fetch: fi->fh is null\n");
        return (NULL_PTR);
    }
    return (cfuse_cop);
}

static int cfuse_getattr(const char *path, struct stat *stbuf)
{
#if 1
    CFUSE_NODE *cfuse_node;
    CFUSE_NODE *cfuse_node_searched;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_getattr: input path = %s\n", path);

    cfuse_node = cfuse_node_new();
    cfuse_node_set(cfuse_node, path, CDFSNP_ITEM_FILE_IS_ANY);

    cfuse_node_searched = cfuse_mgr_search(cfuse_mgr_get(), cfuse_node);
    if(NULL_PTR != cfuse_node_searched)
    {
        sys_log(LOGSTDOUT, "[DEBUG] cfuse_getattr: path %s ===> searched %s and type %ld\n",
                            path, (char *)CFUSE_NODE_DNAME_STR(cfuse_node_searched), CFUSE_NODE_TYPE(cfuse_node_searched));
        CONV_CFUSE_STAT_TO_OS_STAT(CFUSE_NODE_STAT(cfuse_node_searched), stbuf);
        cfuse_node_free(cfuse_node);
        return (0);
    }

    if(EC_FALSE == cdfs_getattr_npp(cfuse_get_cdfs_md_id(), CFUSE_NODE_DNAME(cfuse_node), CFUSE_NODE_STAT(cfuse_node)))
    {
        cfuse_node_free(cfuse_node);
        return (-ENOENT);/*check RETURN VALUE of man lstat*/
    }

    CONV_CFUSE_STAT_TO_OS_STAT(CFUSE_NODE_STAT(cfuse_node), stbuf);
    cfuse_mgr_add_no_search(cfuse_mgr_get(), cfuse_node);

    return (0);
#endif
#if 0
    res = lstat(path, stbuf);
    if (res == -1)
        return -errno;
    sys_log(LOGSTDOUT, "cfuse_getattr: st_ino = %lu, st_mode = %o\n", stbuf->st_ino, stbuf->st_mode);

        //st.st_ino = d->entry->d_ino;
        //st.st_mode = d->entry->d_type << 12;

    return 0;
#endif
}

static int cfuse_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
#if 0
    int res;

    (void) path;

    res = fstat(fi->fh, stbuf);
    if (res == -1)
        return -errno;

    return 0;
#endif
#if 1
    CFUSE_COP *cfuse_cop;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_fgetattr: input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));

    cfuse_cop = cfuse_cop_fetch(fi);
    CFUSE_COP_ASSERT(cfuse_cop, "cfuse_fgetattr");

    //sys_log(LOGSTDOUT, "[DEBUG] cfuse_fgetattr: path %s, input stat:\n", path);
    //cfuse_os_stat_print(LOGSTDOUT, stbuf);

    stbuf->st_mode = CFUSE_COP_MODE(cfuse_cop);

    //sys_log(LOGSTDOUT, "[DEBUG] cfuse_fgetattr: path %s,  fi->fh => mode_t:\n", path);
    //cfuse_os_mode_print(LOGSTDOUT, stbuf->st_mode);

    return (0);
#endif
#if 0
    sys_log(LOGSTDOUT, "error:cfuse_fgetattr: not implemented, input path = %s\n", path);
    return -1;
#endif
}

static int cfuse_access(const char *path, int mask)
{
#if 0
    int res;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_access: input path = %s\n", path);

    res = access(path, mask);
    if (res == -1)
        return -errno;
    return 0;
#endif

#if 1
    CFUSE_NODE *cfuse_node;
    CFUSE_NODE *cfuse_node_searched;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_access: input path = %s\n", path);

    cfuse_node = cfuse_node_new();
    cfuse_node_set(cfuse_node, path, CDFSNP_ITEM_FILE_IS_ANY);

    cfuse_node_searched = cfuse_mgr_search(cfuse_mgr_get(), cfuse_node);
    if(NULL_PTR != cfuse_node_searched)
    {
        sys_log(LOGSTDOUT, "[DEBUG] cfuse_access: path %s ===> searched %s and type %ld\n",
                            path, (char *)CFUSE_NODE_DNAME_STR(cfuse_node_searched), CFUSE_NODE_TYPE(cfuse_node_searched));

        cfuse_node_free(cfuse_node);
        return (0);
    }

    if(EC_FALSE == cdfs_exists_npp(cfuse_get_cdfs_md_id(), CFUSE_NODE_DNAME(cfuse_node)))
    {
        sys_log(LOGSTDOUT, "error:cfuse_access: access failed where path = %s\n", path);
        cfuse_node_free(cfuse_node);
        return (-ENOENT);
    }

    cfuse_node_free(cfuse_node);
    sys_log(LOGSTDOUT, "[DEBUG] cfuse_access: exist path = %s\n", path);
    return (0);
#endif
}

static int cfuse_readlink(const char *path, char *buf, size_t size)
{
    sys_log(LOGSTDOUT, "error:cfuse_readlink: not implemented, input path = %s\n", path);
    return -1;
}

struct cfuse_dirp {
    DIR *dp;
    struct dirent *entry;
    off_t offset;
};

static int cfuse_opendir(const char *path, struct fuse_file_info *fi)
{
#if 0
    int res;
    struct cfuse_dirp *d = malloc(sizeof(struct cfuse_dirp));
    if (d == NULL)
        return -ENOMEM;

    d->dp = opendir(path);
    if (d->dp == NULL) {
        res = -errno;
        free(d);
        return res;
    }
    d->offset = 0;
    d->entry = NULL;

    fi->fh = (unsigned long) d;
    return 0;
#endif

#if 1
    CSTRING *file_path_cstr;
    CVECTOR *path_cstr_vec;

    CFUSE_COP *cfuse_cop;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_opendir: input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));

    file_path_cstr = cstring_new((UINT8 *)path, LOC_CFUSE_0015);
    if(NULL_PTR == file_path_cstr)
    {
        sys_log(LOGSTDOUT, "error:cfuse_opendir: new file_path_cstr failed\n");
        return (-ENOMEM);
    }

    path_cstr_vec = cvector_new(0, MM_CSTRING, LOC_CFUSE_0016);
    if(NULL_PTR == path_cstr_vec)
    {
        sys_log(LOGSTDOUT, "error:cfuse_opendir: new path_cstr_vec failed\n");
        cstring_free(file_path_cstr);
        return (-ENOMEM);
    }

    if(EC_FALSE == cdfs_qlist_seg_npp(cfuse_get_cdfs_md_id(), file_path_cstr, path_cstr_vec))
    {
        sys_log(LOGSTDOUT, "error:cfuse_opendir: access failed where path = %s\n", path);
        cstring_free(file_path_cstr);
        cvector_clean(path_cstr_vec, (CVECTOR_DATA_CLEANER)cstring_free, LOC_CFUSE_0017);
        cvector_free(path_cstr_vec, LOC_CFUSE_0018);
        return (-ENOENT);
    }

    cstring_free(file_path_cstr);

    cfuse_cop = cfuse_cop_new();
    if(NULL_PTR == cfuse_cop)
    {
        sys_log(LOGSTDOUT, "error:cfuse_opendir: new cfuse_cop failed\n");
        cvector_clean(path_cstr_vec, (CVECTOR_DATA_CLEANER)cstring_free, LOC_CFUSE_0019);
        cvector_free(path_cstr_vec, LOC_CFUSE_0020);
        return (-ENOMEM);
    }

    CFUSE_COP_VOID(cfuse_cop) = (void *)path_cstr_vec;
    CFUSE_COP_BIND(fi, cfuse_cop);

    return (0);
#endif
}

static inline struct cfuse_dirp *get_dirp(struct fuse_file_info *fi)
{
    return (struct cfuse_dirp *) (uintptr_t) fi->fh;
}

static int cfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
#if 0
    struct cfuse_dirp *d = get_dirp(fi);

    (void) path;
    if (offset != d->offset) {
        seekdir(d->dp, offset);
        d->entry = NULL;
        d->offset = offset;
    }
    while (1) {
        struct stat st;
        off_t nextoff;

        if (!d->entry) {
            d->entry = readdir(d->dp);
            if (!d->entry)
                break;
        }

        memset(&st, 0, sizeof(st));
        st.st_ino = d->entry->d_ino;
        st.st_mode = d->entry->d_type << 12;
        nextoff = telldir(d->dp);
        if (filler(buf, d->entry->d_name, &st, nextoff))
            break;

        d->entry = NULL;
        d->offset = nextoff;
    }

    return 0;
#endif
#if 1
    CFUSE_COP *cfuse_cop;
    CVECTOR *path_cstr_vec;
    UINT32 pos;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_readdir: input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));

    cfuse_cop = cfuse_cop_fetch(fi);
    CFUSE_COP_ASSERT(cfuse_cop, "cfuse_readdir");

    path_cstr_vec = (CVECTOR *)CFUSE_COP_VOID(cfuse_cop);
    if(NULL_PTR == path_cstr_vec)
    {
        sys_log(LOGSTDOUT, "error:cfuse_readdir: CFUSE_COP_PVOID point to null path_cstr_vec\n");
        return (-EINVAL);
    }

    for(pos = 0; pos < cvector_size(path_cstr_vec); pos ++)
    {
        CSTRING *path_cstr;

        path_cstr = (CSTRING *)cvector_get(path_cstr_vec, pos);
        if(NULL_PTR == path_cstr)
        {
            continue;
        }

        sys_log(LOGSTDOUT, "[DEBUG] cfuse_readdir: before filler: %ld # %s\n", pos, (char *)cstring_get_str(path_cstr));
    }


    for(pos = 0; pos < cvector_size(path_cstr_vec); pos ++)
    {
        CSTRING *path_cstr;
        struct stat st;

        /*NOTE: st info should obtain from hsdfs*/
        memset(&st, 0, sizeof(st));
        st.st_ino = 0;
        st.st_mode = (S_IFDIR | S_IWUSR | S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH);

        path_cstr = (CSTRING *)cvector_get(path_cstr_vec, pos);
        if(NULL_PTR == path_cstr)
        {
            continue;
        }

        sys_log(LOGSTDOUT, "[DEBUG] cfuse_readdir: %ld# path_cstr = %s\n", pos, (char *)cstring_get_str(path_cstr));
        if (filler(buf,(char *)cstring_get_str(path_cstr), &st, 0/*nextoff*/))
        {
            sys_log(LOGSTDOUT, "[DEBUG] cfuse_readdir: break at pos %ld while path_cstr_vec size = %ld\n", pos, cvector_size(path_cstr_vec));
            break;
        }
    }
    return (0);
#endif
}

static int cfuse_releasedir(const char *path, struct fuse_file_info *fi)
{
#if 0
    struct cfuse_dirp *d = get_dirp(fi);
    (void) path;
    closedir(d->dp);
    free(d);
    return 0;
#endif
#if 1
    CFUSE_COP *cfuse_cop;
    CVECTOR *path_cstr_vec;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_releasedir: input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));

    cfuse_cop = cfuse_cop_fetch(fi);
    CFUSE_COP_ASSERT(cfuse_cop, "cfuse_releasedir");

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_releasedir: input path = %s\n", path);
    path_cstr_vec = (CVECTOR *)CFUSE_COP_VOID(cfuse_cop);
    if(NULL_PTR == path_cstr_vec)
    {
        return (-EBADF);
    }

    cvector_clean(path_cstr_vec, (CVECTOR_DATA_CLEANER)cstring_free, LOC_CFUSE_0021);
    cvector_free(path_cstr_vec, LOC_CFUSE_0022);

    CFUSE_COP_VOID(cfuse_cop) = 0;
    cfuse_cop_unbind(fi);
    return (0);
#endif
}

static int cfuse_mknod(const char *path, mode_t mode, dev_t rdev)
{
#if 0
    int res;

    if (S_ISFIFO(mode))
        res = mkfifo(path, mode);
    else
        res = mknod(path, mode, rdev);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_mknod: not implemented, input path = %s\n", path);
    return (0);
}

static int cfuse_mkdir(const char *path, mode_t mode)
{
#if 0
    int res;

    res = mkdir(path, mode);
    if (res == -1)
        return -errno;

    return 0;
#endif
    CFUSE_NODE *cfuse_node;
    CFUSE_NODE *cfuse_node_searched;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_mkdir: input path = %s\n", path);

    cfuse_node = cfuse_node_new();
    cfuse_node_set(cfuse_node, path, CDFSNP_ITEM_FILE_IS_DIR);

    cfuse_node_searched = cfuse_mgr_search(cfuse_mgr_get(), cfuse_node);
    if(NULL_PTR != cfuse_node_searched)
    {
        sys_log(LOGSTDOUT, "[DEBUG] cfuse_mkdir: path %s ===> searched %s and type %ld\n",
                            path, (char *)CFUSE_NODE_DNAME_STR(cfuse_node_searched), CFUSE_NODE_TYPE(cfuse_node_searched));
        cfuse_node_free(cfuse_node);
        return (0);
    }

    if(EC_FALSE == cdfs_mkdir_npp(cfuse_get_cdfs_md_id(), CFUSE_NODE_DNAME(cfuse_node)))
    {
        sys_log(LOGSTDOUT, "error:cfuse_mkdir: access failed where path = %s\n", path);
        cfuse_node_free(cfuse_node);
        return (-ENOENT);
    }

    cfuse_mgr_add_no_search(cfuse_mgr_get(), cfuse_node);
    return (0);
}

static int cfuse_unlink(const char *path)
{
#if 0
    int res;

    res = unlink(path);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_unlink: not implemented, input path = %s\n", path);
    return (0);
}

static int cfuse_rmdir(const char *path)
{
#if 0
    int res;

    res = rmdir(path);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_rmdir: not implemented, input path = %s\n", path);
    return -1;
}

static int cfuse_symlink(const char *from, const char *to)
{
#if 0
    int res;

    res = symlink(from, to);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_symlink: not implemented, input from = %s, to = %s\n", from, to);
    return -1;
}

static int cfuse_rename(const char *from, const char *to)
{
#if 0
    int res;

    res = rename(from, to);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_rename: not implemented, input from = %s, to = %s\n", from, to);
    return -1;
}

static int cfuse_link(const char *from, const char *to)
{
#if 0
    int res;

    res = link(from, to);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_link: not implemented, input from = %s, to = %s\n", from, to);
    return -1;
}

static int cfuse_chmod(const char *path, mode_t mode)
{
#if 0
    int res;

    res = chmod(path, mode);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_chmod: not implemented, input path = %s\n", path);
    return -1;
}

static int cfuse_chown(const char *path, uid_t uid, gid_t gid)
{
#if 0
    int res;

    res = lchown(path, uid, gid);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_chown: not implemented, input path = %s\n", path);
    return -1;
}

static int cfuse_truncate(const char *path, off_t size)
{
#if 0
    int res;

    res = truncate(path, size);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_truncate: not implemented, input path = %s\n", path);
    return -1;
}

static int cfuse_ftruncate(const char *path, off_t size, struct fuse_file_info *fi)
{
#if 0
    int res;

    (void) path;

    res = ftruncate(fi->fh, size);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_ftruncate: not implemented, input path = %s\n", path);
    return -1;
}

static int cfuse_utimens(const char *path, const struct timespec ts[2])
{
#if 0
    int res;
    struct timeval tv[2];

    tv[0].tv_sec = ts[0].tv_sec;
    tv[0].tv_usec = ts[0].tv_nsec / 1000;
    tv[1].tv_sec = ts[1].tv_sec;
    tv[1].tv_usec = ts[1].tv_nsec / 1000;

    res = utimes(path, tv);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_utimens: not implemented, input path = %s\n", path);
    return -1;
}

static int cfuse_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
#if 0
    int fd;

    fd = c_file_open(path, fi->flags, mode);
    if (fd == -1)
        return -errno;

    fi->fh = fd;
    return 0;
#endif
#if 1
    CFUSE_COP * cfuse_cop;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_create: input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));
    //sys_log(LOGSTDOUT, "[DEBUG] cfuse_create: question: mode coming from where? --\n");
    //cfuse_os_mode_print(LOGSTDOUT, mode);

    if(S_ISDIR (mode))
    {
        sys_log(LOGSTDOUT, "[DEBUG] error: cfuse_create: path %s is dir (mode = %o)\n", path, mode);
        return (-EISDIR);
    }

    if(!S_ISREG (mode))
    {
        sys_log(LOGSTDOUT, "[DEBUG] error: cfuse_create: path %s not exist (mode = %o)\n", path, mode);
        return (-ENOENT);
    }
    sys_log(LOGSTDOUT, "[DEBUG] cfuse_create: path %s SUCC\n", path);

    cfuse_cop = cfuse_cop_new();
    if(NULL_PTR == cfuse_cop)
    {
        sys_log(LOGSTDOUT, "error:cfuse_create: new cfuse_cop failed\n");
        return (-ENOMEM);
    }

    CFUSE_COP_MODE(cfuse_cop) = mode;
    CFUSE_COP_FLAG(cfuse_cop) = fi->flags;

    cfuse_cop_bind(cfuse_cop, fi);

    return (0);
#endif
}

static int cfuse_open(const char *path, struct fuse_file_info *fi)
{
#if 0
    int fd;

    fd = c_file_open(path, fi->flags);
    if (fd == -1)
        return -errno;

    fi->fh = fd;
    return 0;
#endif
#if 1
    CFUSE_COP * cfuse_cop;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_open: empty stub, input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));

    cfuse_cop = cfuse_cop_fetch(fi);
    if(NULL_PTR == cfuse_cop)
    {
        cfuse_cop = cfuse_cop_new();
        if(NULL_PTR == cfuse_cop)
        {
            sys_log(LOGSTDOUT, "error:cfuse_open: new cfuse_cop failed\n");
            return (-ENOMEM);
        }
    }

    /**
    * unsigned int (1 bit)    direct_iod    Can be filled in by open, to use direct I/O on this file. (Introduced in version 2.4)
    **/
    fi->direct_io = 1;/*no page cache*/

    CFUSE_COP_FLAG(cfuse_cop) = (fi->flags);
    CFUSE_COP_PATH(cfuse_cop) = (char *)path;

    cfuse_cop_bind(cfuse_cop, fi);
    return (0);
#endif
}

/*TODO: consider EAGAIN*/
static int cfuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
#if 0
    int res;

    (void) path;
    res = pread(fi->fh, buf, size, offset);
    if (res == -1)
        res = -errno;

    return res;
#endif
#if 1
    CFUSE_COP *cfuse_cop;
    CSTRING *file_path_cstr;
    CBYTES *cbytes;
    char *src_buff;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_read: input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));

    cfuse_cop = cfuse_cop_fetch(fi);
    CFUSE_COP_ASSERT(cfuse_cop, "cfuse_read");

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_read: input path = %s, buf size = %ld, offset = %ld\n", path, size, offset);
    file_path_cstr = cstring_new((UINT8 *)path, LOC_CFUSE_0023);
    if(NULL_PTR == file_path_cstr)
    {
        sys_log(LOGSTDOUT, "error:cfuse_read: new file_path_cstr failed\n");
        return (-ENOMEM);
    }

    cbytes = cbytes_new(0);
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:cfuse_read: new cbytes failed\n");
        cstring_free(file_path_cstr);
        return (-ENOMEM);
    }

    if(EC_FALSE == cdfs_read(cfuse_get_cdfs_md_id(), file_path_cstr, cbytes))
    {
        sys_log(LOGSTDOUT, "error:cfuse_read: access failed where path = %s\n", path);
        cstring_free(file_path_cstr);
        cbytes_free(cbytes, LOC_CFUSE_0024);

        return (-EFAULT);
    }

    //sys_log(LOGSTDOUT, "[DEBUG] cfuse_read: cbytes is ");
    //cbytes_print(cfuse_get_cdfs_md_id(),LOGSTDOUT, cbytes);

    cstring_free(file_path_cstr);

    src_buff = (char *)(CBYTES_BUF(cbytes) + offset);
/*
    for(pos = 0; pos < size && pos + offset < CBYTES_LEN(cbytes); pos ++)
    {
        *(buf ++) = *(src_buff ++);
    }
*/
    BCOPY(src_buff, buf, DMIN(size, CBYTES_LEN(cbytes) - offset));

    cbytes_free(cbytes, LOC_CFUSE_0025);

    CFUSE_COP_OP(cfuse_cop) = (CFUSE_OP_READ);

    return (DMIN(size, CBYTES_LEN(cbytes) - offset));
#endif
}

/*fuck! when offset = 0, path = 0x1000 = 4096, i.e., one file data block size*/
static int cfuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
#if 0
    int res;

    (void) path;
    res = pwrite(fi->fh, buf, size, offset);
    if (res == -1)
        res = -errno;

    return res;
#endif
#if 1
    UINT8 *src_buf;
    UINT8 *des_buf;
    CFUSE_COP *cfuse_cop;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_write: input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));

    cfuse_cop = cfuse_cop_fetch(fi);
    CFUSE_COP_ASSERT(cfuse_cop, "cfuse_write");

    if(offset != CFUSE_COP_POS(cfuse_cop))/*debug*/
    {
        sys_log(LOGSTDOUT, "error:cfuse_write:cfuse_cop pos = %ld, offset = %ld, size = %ld, path = %lx\n",
                            CFUSE_COP_POS(cfuse_cop), offset, size, path);
        return (-EFAULT);
    }

    if(offset + size >= CFUSE_COP_BUF_MAX_SIZE)
    {
        sys_log(LOGSTDOUT, "error:cfuse_write:insufficient buf: cfuse_cop pos = %ld, offset = %ld, size = %ld, CFUSE_COP_BUF_MAX_SIZE = %ld, path = %lx\n",
                            CFUSE_COP_POS(cfuse_cop), offset, size, CFUSE_COP_BUF_MAX_SIZE, path);
        return (-EFAULT);
    }

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_write:try to accept data: cfuse_cop pos = %ld, offset = %ld, size = %ld, path = %lx\n",
                        CFUSE_COP_POS(cfuse_cop), offset, size, path);

    src_buf = (UINT8 *)buf;
    des_buf = CFUSE_COP_BUF(cfuse_cop) + offset;
/*
    for(pos = 0; pos < size; pos ++)
    {
        *(des_buf ++) = *(src_buf ++);
    }
*/
    BCOPY(src_buf, des_buf, size);

    CFUSE_COP_POS(cfuse_cop) += size;

    CFUSE_COP_OP(cfuse_cop) = (CFUSE_OP_WRITE);
    return (size);

#endif
#if 0
    CSTRING *file_path_cstr;
    CBYTES *cbytes;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_write: input path = %s, buf size = %ld, offset = %ld\n", path, size, offset);
    file_path_cstr = cstring_new((UINT8 *)path, LOC_CFUSE_0026);
    cbytes = cbytes_new(0);
    cbytes_mount(cbytes, size, (UINT8 *)buf);

    if(EC_FALSE == cdfs_write(cfuse_get_cdfs_md_id(), file_path_cstr, cbytes, /*CDFS_REPLICA_MAX_NUM*/3))
    {
        sys_log(LOGSTDOUT, "error:cfuse_write: access failed where path = %s\n", path);
        cstring_free(file_path_cstr);

        cbytes_umount(cbytes);
        cbytes_free(cbytes);
        return -1;
    }

    cstring_free(file_path_cstr);

    cbytes_umount(cbytes);
    cbytes_free(cbytes);
    return (size);
#endif
}

static int cfuse_statfs(const char *path, struct statvfs *stbuf)
{
#if 0
    int res;

    res = statvfs(path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
#endif
    sys_log(LOGSTDOUT, "error:cfuse_statfs: not implemented, input path = %s\n", path);
    return -1;
}

static int cfuse_flush(const char *path, struct fuse_file_info *fi)
{
#if 0
    int res;

    (void) path;
    /* This is called from every close on an open file, so call the
       close on the underlying filesystem.    But since flush may be
       called multiple times for an open file, this must not really
       close the file.  This is important if used on a network
       filesystem like NFS which flush the data/metadata on close() */
    res = c_file_close(dup(fi->fh));
    if (res == -1)
        return -errno;

    return 0;
#endif
#if 0
    sys_log(LOGSTDOUT, "[DEBUG] cfuse_flush: empty stub, input path = %s\n", path);
    fi->fh = 0;
    return 0;
#endif

#if 1
    CSTRING *file_path_cstr;
    CBYTES *cbytes;
    CFUSE_COP *cfuse_cop;

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_flush: input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));

    cfuse_cop = cfuse_cop_fetch(fi);
    CFUSE_COP_ASSERT(cfuse_cop, "cfuse_flush");

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_flush: input path = %s, cfuse_cop->pos = %ld\n", path, CFUSE_COP_POS(cfuse_cop));


    if(CFUSE_OP_READ == CFUSE_COP_OP(cfuse_cop))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cfuse_flush: give up flush path = %s due to READ operation\n", path);
        return (0);
    }
#if 1
    if(0 == CFUSE_COP_POS(cfuse_cop))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cfuse_flush: give up flush path = %s due to CFUSE_COP_POS is zero\n", path);
        return (0);
    }
#endif
    file_path_cstr = cstring_new((UINT8 *)path, LOC_CFUSE_0027);
    if(NULL_PTR == file_path_cstr)
    {
        sys_log(LOGSTDOUT, "error:cfuse_flush: new file_path_cstr failed\n");
        cfuse_cop_unbind(fi);
        return (-ENOMEM);
    }

    cbytes = cbytes_new(0);
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:cfuse_flush: new cbytes failed\n");
        cstring_free(file_path_cstr);

        cfuse_cop_unbind(fi);
        return (-ENOMEM);
    }

    cbytes_mount(cbytes, CFUSE_COP_POS(cfuse_cop), CFUSE_COP_BUF(cfuse_cop));

    if(EC_FALSE == cdfs_write(cfuse_get_cdfs_md_id(), file_path_cstr, cbytes, /*CDFS_REPLICA_MAX_NUM*/3))
    {
        sys_log(LOGSTDOUT, "error:cfuse_wcfuse_flushrite: access failed where path = %s\n", path);
        cstring_free(file_path_cstr);

        cbytes_umount(cbytes);
        cbytes_free(cbytes, LOC_CFUSE_0028);

        cfuse_cop_unbind(fi);

        return (-EIO);
    }

    cstring_free(file_path_cstr);

    cbytes_umount(cbytes);
    cbytes_free(cbytes, LOC_CFUSE_0029);

    return (0);
#endif
}

static int cfuse_release(const char *path, struct fuse_file_info *fi)
{
#if 0
    (void) path;
    close(fi->fh);

    return 0;
#endif
#if 1
    sys_log(LOGSTDOUT, "[DEBUG] cfuse_release: input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));

    sys_log(LOGSTDOUT, "[DEBUG] cfuse_release: empty stub, input path = %s\n", path);

    cfuse_cop_unbind(fi);
    return 0;
#endif
}

static int cfuse_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
#if 0
    int res;
    (void) path;

#ifndef HAVE_FDATASYNC
    (void) isdatasync;
#else
    if (isdatasync)
        res = fdatasync(fi->fh);
    else
#endif
        res = fsync(fi->fh);
    if (res == -1)
        return -errno;

    return 0;
#endif
#if 1
    sys_log(LOGSTDOUT, "[DEBUG] cfuse_fsync: empty stub, input path = %s\n", path);
    return 0;
#endif
}


static int cfuse_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *lock)
{
#if 0
    (void) path;

    return ulockmgr_op(fi->fh, cmd, lock, &fi->lock_owner,
               sizeof(fi->lock_owner));
#endif
#if 1
    sys_log(LOGSTDOUT, "[DEBUG] cfuse_lock: empty stub, input path = %s, fi = %lx, fi->flags = %x, fi->fh = %lx\n", path, fi, fi->flags, (UINT32)(fi->fh));
    return 0;
#endif
}

#if 0
static struct fuse_operations cfuse_oper = {
    /*getattr    */ cfuse_getattr,
    /*readlink   */ cfuse_readlink,
    /*getdir     */ NULL_PTR,
    /*mknod      */ cfuse_mknod,
    /*mkdir      */ cfuse_mkdir,
    /*unlink     */ cfuse_unlink,
    /*rmdir      */ cfuse_rmdir,
    /*symlink    */ cfuse_symlink,
    /*rename     */ cfuse_rename,
    /*link       */ cfuse_link,
    /*chmod      */ cfuse_chmod,
    /*chown      */ cfuse_chown,
    /*truncate   */ cfuse_truncate,
    /*utime      */ NULL_PTR,
    /*open       */ cfuse_open,
    /*read       */ cfuse_read,
    /*write      */ cfuse_write,
    /*statfs     */ cfuse_statfs,
    /*flush      */ cfuse_flush,
    /*release    */ cfuse_release,
    /*fsync      */ cfuse_fsync,
    /*setxattr   */ NULL_PTR,
    /*getxattr   */ NULL_PTR,
    /*listxattr  */ NULL_PTR,
    /*removexattr*/ NULL_PTR,
    /*opendir    */ cfuse_opendir,
    /*readdir    */ cfuse_readdir,
    /*releasedir */ cfuse_releasedir,
    /*fsyncdir   */ NULL_PTR,
    /*init       */ NULL_PTR,
    /*destroy    */ NULL_PTR,
    /*access     */ cfuse_access,
    /*create     */ cfuse_create,
    /*ftruncate  */ cfuse_ftruncate,
    /*fgetattr   */ cfuse_fgetattr,
    /*lock       */ cfuse_lock,
    /*utimens    */ cfuse_utimens,
    /*bmap       */ NULL_PTR,
#if 0
    .getattr    = cfuse_getattr,
    .fgetattr    = cfuse_fgetattr,
    .access        = cfuse_access,
    .readlink    = cfuse_readlink,
    .opendir    = cfuse_opendir,
    .readdir    = cfuse_readdir,
    .releasedir    = cfuse_releasedir,
    .mknod        = cfuse_mknod,
    .mkdir        = cfuse_mkdir,
    .symlink    = cfuse_symlink,
    .unlink        = cfuse_unlink,
    .rmdir        = cfuse_rmdir,
    .rename        = cfuse_rename,
    .link        = cfuse_link,
    .chmod        = cfuse_chmod,
    .chown        = cfuse_chown,
    .truncate    = cfuse_truncate,
    .ftruncate    = cfuse_ftruncate,
    .utimens    = cfuse_utimens,
    .create        = cfuse_create,
    .open        = cfuse_open,
    .read        = cfuse_read,
    .write        = cfuse_write,
    .statfs        = cfuse_statfs,
    .flush        = cfuse_flush,
    .release    = cfuse_release,
    .fsync        = cfuse_fsync,

    .lock        = cfuse_lock,
#endif
};
#endif

static struct fuse_operations g_cfuse_oper;
void cfuse_init_operator(struct fuse_operations *cfuse_oper)
{
    cfuse_oper->getattr     = cfuse_getattr;
    cfuse_oper->readlink    = cfuse_readlink;
    cfuse_oper->getdir      = NULL_PTR;
    cfuse_oper->mknod       = cfuse_mknod;
    cfuse_oper->mkdir       = cfuse_mkdir;
    cfuse_oper->unlink      = cfuse_unlink;
    cfuse_oper->rmdir       = cfuse_rmdir;
    cfuse_oper->symlink     = cfuse_symlink;
    cfuse_oper->rename      = cfuse_rename;
    cfuse_oper->link        = cfuse_link;
    cfuse_oper->chmod       = cfuse_chmod;
    cfuse_oper->chown       = cfuse_chown;
    cfuse_oper->truncate    = cfuse_truncate;
    cfuse_oper->utime       = NULL_PTR;
    cfuse_oper->open        = cfuse_open;
    cfuse_oper->read        = cfuse_read;
    cfuse_oper->write       = cfuse_write;
    cfuse_oper->statfs      = cfuse_statfs;
    cfuse_oper->flush       = cfuse_flush;
    cfuse_oper->release     = cfuse_release;
    cfuse_oper->fsync       = cfuse_fsync;
    cfuse_oper->setxattr    = NULL_PTR;
    cfuse_oper->getxattr    = NULL_PTR;
    cfuse_oper->listxattr   = NULL_PTR;
    cfuse_oper->removexattr = NULL_PTR;
    cfuse_oper->opendir     = cfuse_opendir;
    cfuse_oper->readdir     = cfuse_readdir;
    cfuse_oper->releasedir  = cfuse_releasedir;
    cfuse_oper->fsyncdir    = NULL_PTR;
    cfuse_oper->init        = NULL_PTR;
    cfuse_oper->destroy     = NULL_PTR;
    cfuse_oper->access      = cfuse_access;
    cfuse_oper->create      = cfuse_create;
    cfuse_oper->ftruncate   = cfuse_ftruncate;
    cfuse_oper->fgetattr    = cfuse_fgetattr;
    cfuse_oper->lock        = cfuse_lock;
    cfuse_oper->utimens     = cfuse_utimens;
    cfuse_oper->bmap        = NULL_PTR;
}

void cfuse_entry(int *argc, char **argv)
{
#if 0
    char *argv[] = {
        "hsdfs",
        "/mnt/hsdfs",
        "-d",
    };
#endif
    umask(0);

    cfuse_mgr_int(cfuse_mgr_get(), CFUSE_NODE_MAX_NUM);

    cfuse_init_operator(&g_cfuse_oper);
    fuse_main((*argc), argv, &g_cfuse_oper, NULL);

    cfuse_mgr_clean(cfuse_mgr_get());
    return;
}
#endif/*(SWITCH_ON == CFUSE_SUPPORT_SWITCH)*/

#if (SWITCH_OFF == CFUSE_SUPPORT_SWITCH)
#define FUSE_USE_VERSION 26

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
#include "cvector.h"
#include "cfuse.h"

#include "task.h"
#include "cdfs.h"

#include "clist.h"


#if 0
O_RDONLY = 0x0000
O_RDWR   = 0x0002
O_CREAT  = 0x0040
#endif

static CFUSE_MGR g_cfuse_mgr;

#define CFUSE_COP_ASSERT(cfuse_cop, func_name) do{\
    if(NULL_PTR == (cfuse_cop)) {\
        sys_log(LOGSTDOUT, "error:%s: cfuse_cop is null\n", (func_name));\
        return (-1);\
    }\
}while(0)

static UINT32 cfuse_get_cdfs_md_id()
{
    return (0);
}

static CFUSE_MGR * cfuse_mgr_get()
{
    return (&g_cfuse_mgr);
}

EC_BOOL cfuse_mode_init_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_mode_clean_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_mode_free_0(const UINT32 md_id, CFUSE_MODE *cfuse_mode)
{
    return (EC_FALSE);
}

CFUSE_STAT *cfuse_stat_new()
{
    return (NULL_PTR);
}

EC_BOOL cfuse_stat_init(CFUSE_STAT *cfuse_stat)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_stat_clean(CFUSE_STAT *cfuse_stat)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_stat_free(CFUSE_STAT *cfuse_stat)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_stat_init_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_stat_clean_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_stat_free_0(const UINT32 md_id, CFUSE_STAT *cfuse_stat)
{
    return (EC_FALSE);
}

CFUSE_NODE *cfuse_node_new()
{
    return (NULL_PTR);
}

EC_BOOL cfuse_node_init(CFUSE_NODE *cfuse_node)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_node_clean(CFUSE_NODE *cfuse_node)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_node_free(CFUSE_NODE *cfuse_node)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_mgr_int(CFUSE_MGR *cfuse_mgr, const UINT32 max_node_num)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_mgr_clean(CFUSE_MGR *cfuse_mgr)
{
    return (EC_FALSE);
}

CFUSE_NODE *cfuse_mgr_search(const CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node)
{
    return (NULL_PTR);
}

CFUSE_NODE *cfuse_mgr_search_no_lock(const CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node)
{
    return (NULL_PTR);
}

EC_BOOL cfuse_mgr_add(CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_mgr_add_no_search(CFUSE_MGR *cfuse_mgr, const CFUSE_NODE *cfuse_node)
{
    return (EC_FALSE);
}

void cfuse_mgr_print(LOG *log, const CFUSE_MGR *cfuse_mgr)
{
    return ;
}

CFUSE_COP *cfuse_cop_new()
{
    return (NULL_PTR);
}

EC_BOOL cfuse_cop_init(CFUSE_COP *cfuse_cop)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_cop_clean(CFUSE_COP *cfuse_cop)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_cop_free(CFUSE_COP *cfuse_cop)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_cop_bind(CFUSE_COP *cfuse_cop, void *fi)
{
    return (EC_FALSE);
}

EC_BOOL cfuse_cop_unbind(void *fi)
{
    return (EC_FALSE);
}

CFUSE_COP * cfuse_cop_fetch(void *fi)
{
    return (NULL_PTR);
}

void cfuse_entry(int *argc, char **argv)
{
    return ;
}

#endif/*(SWITCH_OFF == CFUSE_SUPPORT_SWITCH)*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

