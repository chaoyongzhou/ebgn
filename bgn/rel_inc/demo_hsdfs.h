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

#ifndef _DEMO_HSDFS_H
#define _DEMO_HSDFS_H

#include "type.h"
#include "cbytes.h"

#define CDFS_TEST_SCENARIO_1K_TO_16K (1)
#define CDFS_TEST_SCENARIO_4K_TO_1M  (2)
#define CDFS_TEST_SCENARIO_4K_TO_4M  (4)

#if (32 == WORDSIZE)
/*scenario choice*/
#define CDFS_TEST_SCENARIO_CHOICE    (CDFS_TEST_SCENARIO_1K_TO_16K)

/*common definition*/
#define CDFS_NP_CACHED_MAX_NUM       ((UINT32)   8)
#define CDFS_NP_MIN_NUM              ((UINT32)   1)/*xxx*/
#define CDFS_REPLICA_MAX_NUM         ((UINT32)   1)/*xxx*/
#define CDFS_MAX_FILE_NUM_PER_LOOP   ((UINT32)  64)/*xxx num of files handled per loop(= CDFSNP_DIR_FILE_MAX_NUM)*/

#if (CDFS_TEST_SCENARIO_1K_TO_16K == CDFS_TEST_SCENARIO_CHOICE)
static UINT32 g_cdfs_cbytes_used_num = 16;

#define CDFS_TEST_WRITE_MAX_FILES  4
#define CDFS_TEST_READ_MAX_FILES   4
#define CDFS_TEST_LOOP_MAX_TIMES   2
#endif/*(CDFS_TEST_SCENARIO_1K_TO_16K == CDFS_TEST_SCENARIO_CHOICE)*/

#if (CDFS_TEST_SCENARIO_4K_TO_1M == CDFS_TEST_SCENARIO_CHOICE)
static UINT32 g_cdfs_cbytes_used_num = 9;

#define CDFS_TEST_WRITE_MAX_FILES   4/*xxx*/
#define CDFS_TEST_READ_MAX_FILES    4/*xxx*/
#define CDFS_TEST_LOOP_MAX_TIMES   16/*xxx*/
#endif/*(CDFS_TEST_SCENARIO_4K_TO_1M == CDFS_TEST_SCENARIO_CHOICE)*/

#if (CDFS_TEST_SCENARIO_4K_TO_4M == CDFS_TEST_SCENARIO_CHOICE)
static UINT32 g_cdfs_cbytes_used_num = 11;

#define CDFS_TEST_WRITE_MAX_FILES 128
#define CDFS_TEST_READ_MAX_FILES  128
#define CDFS_TEST_LOOP_MAX_TIMES  256
#endif/*(CDFS_TEST_SCENARIO_4K_TO_4M == CDFS_TEST_SCENARIO_CHOICE)*/
#endif/*(32 == WORDSIZE)*/


#if (64 == WORDSIZE)
/*scenario choice*/
#define CDFS_TEST_SCENARIO_CHOICE    (CDFS_TEST_SCENARIO_4K_TO_1M)

/*common definition*/
#define CDFS_NP_CACHED_MAX_NUM       ((UINT32)  16)/*num of hot np cached in memory*/
#define CDFS_NP_MIN_NUM              ((UINT32)   1)/*xxx*/
#define CDFS_REPLICA_MAX_NUM         ((UINT32)   1)/*xxx*/
#define CDFS_MAX_FILE_NUM_PER_LOOP   ((UINT32)1024)/*xxx num of files handled per loop(= CDFSNP_DIR_FILE_MAX_NUM)*/

#if (CDFS_TEST_SCENARIO_1K_TO_16K == CDFS_TEST_SCENARIO_CHOICE)
static UINT32 g_cdfs_cbytes_used_num = 16;

#define CDFS_TEST_WRITE_MAX_FILES 128
#define CDFS_TEST_READ_MAX_FILES  128
#define CDFS_TEST_LOOP_MAX_TIMES    1
#endif/*(CDFS_TEST_SCENARIO_1K_TO_16K == CDFS_TEST_SCENARIO_CHOICE)*/

#if (CDFS_TEST_SCENARIO_4K_TO_1M == CDFS_TEST_SCENARIO_CHOICE)
static UINT32 g_cdfs_cbytes_used_num = 9;

#define CDFS_TEST_WRITE_MAX_FILES  128/*xxx*/
#define CDFS_TEST_READ_MAX_FILES   128/*xxx*/
#define CDFS_TEST_LOOP_MAX_TIMES   512/*xxx*/
#endif/*(CDFS_TEST_SCENARIO_4K_TO_1M == CDFS_TEST_SCENARIO_CHOICE)*/

#if (CDFS_TEST_SCENARIO_4K_TO_4M == CDFS_TEST_SCENARIO_CHOICE)
static UINT32 g_cdfs_cbytes_used_num = 11;

#define CDFS_TEST_WRITE_MAX_FILES 128
#define CDFS_TEST_READ_MAX_FILES  128
#define CDFS_TEST_LOOP_MAX_TIMES  256
#endif/*(CDFS_TEST_SCENARIO_4K_TO_4M == CDFS_TEST_SCENARIO_CHOICE)*/
#endif/*(64 == WORDSIZE)*/

typedef struct
{
    char  *file_name;
    UINT32 file_size;
}DEMO_CDFS_FILE_CFG;

static   DEMO_CDFS_FILE_CFG g_cdfs_file_cfg_tbl[] = {
#if (CDFS_TEST_SCENARIO_1K_TO_16K == CDFS_TEST_SCENARIO_CHOICE)
        {(char *)"../../data_files/1K.dat",     1 * 1024},
        {(char *)"../../data_files/2K.dat",     2 * 1024},
        {(char *)"../../data_files/3K.dat",     3 * 1024},
        {(char *)"../../data_files/4K.dat",     4 * 1024},
        {(char *)"../../data_files/5K.dat",     5 * 1024},
        {(char *)"../../data_files/6K.dat",     6 * 1024},
        {(char *)"../../data_files/7K.dat",     7 * 1024},
        {(char *)"../../data_files/8K.dat",     8 * 1024},
        {(char *)"../../data_files/9K.dat",     9 * 1024},
        {(char *)"../../data_files/10K.dat",   10 * 1024},
        {(char *)"../../data_files/11K.dat",   11 * 1024},
        {(char *)"../../data_files/12K.dat",   12 * 1024},
        {(char *)"../../data_files/13K.dat",   13 * 1024},
        {(char *)"../../data_files/14K.dat",   14 * 1024},
        {(char *)"../../data_files/15K.dat",   15 * 1024},
        {(char *)"../../data_files/16K.dat",   16 * 1024},
#endif/*(CDFS_TEST_SCENARIO_1K_TO_16K == CDFS_TEST_SCENARIO_CHOICE)*/

#if (CDFS_TEST_SCENARIO_4K_TO_1M == CDFS_TEST_SCENARIO_CHOICE)
        {(char *)"../../data_files/4K.dat",     4 * 1024},
        {(char *)"../../data_files/8K.dat",     8 * 1024},
        {(char *)"../../data_files/16K.dat",   16 * 1024},
        {(char *)"../../data_files/32K.dat",   32 * 1024},
        {(char *)"../../data_files/64K.dat",   64 * 1024},
        {(char *)"../../data_files/128K.dat", 128 * 1024},
        {(char *)"../../data_files/256K.dat", 256 * 1024},
        {(char *)"../../data_files/512K.dat", 512 * 1024},
        {(char *)"../../data_files/1M.dat",     1 * 1024 * 1024},
#endif/*(CDFS_TEST_SCENARIO_4K_TO_1M == CDFS_TEST_SCENARIO_CHOICE)*/

#if (CDFS_TEST_SCENARIO_4K_TO_4M == CDFS_TEST_SCENARIO_CHOICE)
        {(char *)"../../data_files/4K.dat",     4 * 1024},
        {(char *)"../../data_files/8K.dat",     8 * 1024},
        {(char *)"../../data_files/16K.dat",   16 * 1024},
        {(char *)"../../data_files/32K.dat",   32 * 1024},
        {(char *)"../../data_files/64K.dat",   64 * 1024},
        {(char *)"../../data_files/128K.dat", 128 * 1024},
        {(char *)"../../data_files/256K.dat", 256 * 1024},
        {(char *)"../../data_files/512K.dat", 512 * 1024},
        {(char *)"../../data_files/1M.dat",     1 * 1024 * 1024},
        {(char *)"../../data_files/2M.dat",     2 * 1024 * 1024},
        {(char *)"../../data_files/4M.dat",     4 * 1024 * 1024},
#endif/*(CDFS_TEST_SCENARIO_4K_TO_4M == CDFS_TEST_SCENARIO_CHOICE)*/
};


#endif /*_DEMO_HSDFS_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

