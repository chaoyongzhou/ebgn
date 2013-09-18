/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#ifndef _LIB_MM_H
#define _LIB_MM_H

#include "lib_type.h"
#include "lib_log.h"

/*MD_TYPE*/
#define     MD_BGNZ      ((UINT32)  0)
#define     MD_BGNZ2     ((UINT32)  1)
#define     MD_BGNZN     ((UINT32)  2)
#define     MD_BGNF2N    ((UINT32)  3)
#define     MD_BGNFP     ((UINT32)  4)
#define     MD_ECF2N     ((UINT32)  5)
#define     MD_ECFP      ((UINT32)  6)
#define     MD_ECCF2N    ((UINT32)  7)
#define     MD_ECCFP     ((UINT32)  8)
#define     MD_CONV      ((UINT32)  9)
#define     MD_POLYZ     ((UINT32) 10)
#define     MD_POLYZ2    ((UINT32) 11)
#define     MD_POLYZN    ((UINT32) 12)
#define     MD_POLYFP    ((UINT32) 13)
#define     MD_POLYF2N   ((UINT32) 14)
#define     MD_SEAFP     ((UINT32) 15)
#define     MD_TASK      ((UINT32) 16)
#define     MD_MATRIXR   ((UINT32) 17)
#define     MD_EBGNZ     ((UINT32) 18)
#define     MD_EBGNZ2    ((UINT32) 19)
#define     MD_VECTORR   ((UINT32) 20)
#define     MD_PARSER    ((UINT32) 21)
#define     MD_EBGNR     ((UINT32) 22)
#define     MD_DMATRIXR  ((UINT32) 23)
#define     MD_VMATRIXR  ((UINT32) 24)
#define     MD_CFILE     ((UINT32) 25)
#define     MD_CDIR      ((UINT32) 26)
#define     MD_CMON      ((UINT32) 27)
#define     MD_TBD       ((UINT32) 28)/* user defined */
#define     MD_CRUN      ((UINT32) 29)/* user interfaces */
#define     MD_VMM       ((UINT32) 30) 
#define     MD_SUPER     ((UINT32) 31)
#define     MD_CTIMER    ((UINT32) 32)
#define     MD_CDFS      ((UINT32) 33)
#define     MD_CSOLR     ((UINT32) 34)
#define     MD_CBGT      ((UINT32) 35)
#define     MD_GANGLIA   ((UINT32) 36)
#define     MD_CSESSION  ((UINT32) 37)
#define     MD_END       ((UINT32) 38)

/* Memory Management */
#define                        MM_UINT32    ((UINT32)  0)
#define                        MM_UINT16    ((UINT32)  1)
#define                         MM_UINT8    ((UINT32)  2)
#define                        MM_BIGINT    ((UINT32)  3)
#define                           MM_NAF    ((UINT32)  4)
#define                   MM_CURVE_POINT    ((UINT32)  5)
#define            MM_CURVE_AFFINE_POINT    ((UINT32)  6)
#define                   MM_ECF2N_CURVE    ((UINT32)  7)
#define                    MM_ECFP_CURVE    ((UINT32)  8)
#define                      MM_KEY_PAIR    ((UINT32)  9)
#define                          MM_POLY    ((UINT32) 10)
#define                     MM_POLY_ITEM    ((UINT32) 11)
#define                        MM_DEGREE    ((UINT32) 12)
#define                   MM_STRCHAR128B    ((UINT32) 13)
#define                   MM_STRCHAR256B    ((UINT32) 14)
#define                   MM_STRCHAR512B    ((UINT32) 15)
#define                     MM_STRCHAR1K    ((UINT32) 16)
#define                     MM_STRCHAR2K    ((UINT32) 17)
#define                     MM_STRCHAR4K    ((UINT32) 18)
#define                    MM_STRCHAR10K    ((UINT32) 19)
#define                    MM_STRCHAR64K    ((UINT32) 20)
#define                     MM_STRCHAR1M    ((UINT32) 21)
#define                          MM_EBGN    ((UINT32) 22)
#define                     MM_EBGN_ITEM    ((UINT32) 23)
#define                          MM_REAL    ((UINT32) 24)
#define                  MM_MATRIX_BLOCK    ((UINT32) 25)
#define                 MM_MATRIX_HEADER    ((UINT32) 26)
#define                        MM_MATRIX    ((UINT32) 27)
#define                  MM_VECTOR_BLOCK    ((UINT32) 28)
#define                        MM_VECTOR    ((UINT32) 29)
#ifdef PARSER_MEM_MGR
#define                          MM_PROD    ((UINT32) 30)
#define                          MM_TREE    ((UINT32) 31)
#else
#define                     MM_PROD_RSVD    ((UINT32) 30) /*reserved*/
#define                     MM_TREE_RSVD    ((UINT32) 31) /*reserved*/
#endif/*PARSER_MEM_MGR*/
#define                     MM_TASK_NODE    ((UINT32) 32)
#define                      MM_TASK_MGR    ((UINT32) 33)
#define                  MM_TASK_CONTEXT    ((UINT32) 34)
#define                      MM_MOD_NODE    ((UINT32) 35)
#define                       MM_MOD_MGR    ((UINT32) 36)
#define                     MM_TASKC_MGR    ((UINT32) 37)

#define                     MM_STRCHAR2M    ((UINT32) 38)
#define                     MM_STRCHAR4M    ((UINT32) 39)
#define                     MM_STRCHAR8M    ((UINT32) 40)
#define                    MM_STRCHAR16M    ((UINT32) 41)
#define                    MM_STRCHAR32M    ((UINT32) 42)
#define                    MM_STRCHAR64M    ((UINT32) 43)
#define                   MM_STRCHAR128M    ((UINT32) 44)

#define                    MM_CLIST_DATA    ((UINT32) 45)
#define                   MM_CSTACK_DATA    ((UINT32) 46)
#define                     MM_CSET_DATA    ((UINT32) 47)
#define                   MM_CQUEUE_DATA    ((UINT32) 48)
#define                       MM_CSTRING    ((UINT32) 49)
#define                 MM_FUNC_ADDR_MGR    ((UINT32) 50)
/*---------------- buff mem definition beg: ----------------*/
#define                    BUFF_MEM_DEF_BEG MM_UINT8_064B /*the minimum size buff*/
#define                    BUFF_MEM_DEF_END MM_UINT8_512M /*the maximum size buff*/

#define                    MM_UINT8_064B    ((UINT32) 51)
#define                    MM_UINT8_128B    ((UINT32) 52)
#define                    MM_UINT8_256B    ((UINT32) 53)
#define                    MM_UINT8_512B    ((UINT32) 54)
#define                    MM_UINT8_001K    ((UINT32) 55)
#define                    MM_UINT8_002K    ((UINT32) 56)
#define                    MM_UINT8_004K    ((UINT32) 57)
#define                    MM_UINT8_008K    ((UINT32) 58)
#define                    MM_UINT8_016K    ((UINT32) 59)
#define                    MM_UINT8_032K    ((UINT32) 60)
#define                    MM_UINT8_064K    ((UINT32) 61)
#define                    MM_UINT8_128K    ((UINT32) 62)
#define                    MM_UINT8_256K    ((UINT32) 63)
#define                    MM_UINT8_512K    ((UINT32) 64)
#define                    MM_UINT8_001M    ((UINT32) 65)
#define                    MM_UINT8_002M    ((UINT32) 66)
#define                    MM_UINT8_004M    ((UINT32) 67)
#define                    MM_UINT8_008M    ((UINT32) 68)
#define                    MM_UINT8_016M    ((UINT32) 69)
#define                    MM_UINT8_032M    ((UINT32) 70)
#define                    MM_UINT8_064M    ((UINT32) 71)
#define                    MM_UINT8_128M    ((UINT32) 72)
#define                    MM_UINT8_256M    ((UINT32) 73)
#define                    MM_UINT8_512M    ((UINT32) 74)
/*---------------- buff mem definition end: ----------------*/

/*----------------------- extensive -----------------------*/
#define                MM_REAL_BLOCK_2_2    ((UINT32) 75)
#define                MM_REAL_BLOCK_4_4    ((UINT32) 76)
#define                MM_REAL_BLOCK_8_8    ((UINT32) 77)
#define              MM_REAL_BLOCK_16_16    ((UINT32) 78)
#define              MM_REAL_BLOCK_32_32    ((UINT32) 79)
#define              MM_REAL_BLOCK_64_64    ((UINT32) 80)
#define            MM_REAL_BLOCK_128_128    ((UINT32) 81)
#define            MM_REAL_BLOCK_256_256    ((UINT32) 82)
#define            MM_REAL_BLOCK_512_512    ((UINT32) 83)
#define          MM_REAL_BLOCK_1024_1024    ((UINT32) 84)

#define                         MM_CLIST    ((UINT32) 85)
#define                        MM_CSTACK    ((UINT32) 86)
#define                          MM_CSET    ((UINT32) 87)
#define                        MM_CQUEUE    ((UINT32) 88)
#define                       MM_CVECTOR    ((UINT32) 89)

#define                     MM_TASKS_CFG    ((UINT32) 90)
#define                     MM_TASKR_CFG    ((UINT32) 91)
#define                      MM_TASK_CFG    ((UINT32) 92)

#define                    MM_TASKS_NODE    ((UINT32) 93)
#define                  MM_CROUTER_NODE    ((UINT32) 94)
#define                    MM_TASKC_NODE    ((UINT32) 95)
#define              MM_CROUTER_NODE_VEC    ((UINT32) 96)
#define                   MM_CROUTER_CFG    ((UINT32) 97)
#define                  MM_CTHREAD_TASK    ((UINT32) 98)
#define                      MM_VMM_NODE    ((UINT32) 99)
#define                           MM_LOG    ((UINT32)100)
#define                     MM_COMM_NODE    ((UINT32)101)
#define                     MM_CFILE_SEG    ((UINT32)102)
#define                    MM_CFILE_NODE    ((UINT32)103)
#define                         MM_KBUFF    ((UINT32)104)
#define                      MM_CDIR_SEG    ((UINT32)105)
#define                     MM_CDIR_NODE    ((UINT32)106)
#define                      MM_CMON_OBJ    ((UINT32)107)
#define                  MM_CMON_OBJ_VEC    ((UINT32)108)
#define                MM_CMON_OBJ_MERGE    ((UINT32)109)
#define                 MM_CSOCKET_CNODE    ((UINT32)110)
#define                   MM_CTIMER_NODE    ((UINT32)111)
#define                 MM_CSYS_CPU_STAT    ((UINT32)112)
#define             MM_CSYS_CPU_AVG_STAT    ((UINT32)113)
#define                 MM_CSYS_MEM_STAT    ((UINT32)114)
#define                MM_CPROC_MEM_STAT    ((UINT32)115)
#define                MM_CPROC_CPU_STAT    ((UINT32)116)
#define             MM_CPROC_THREAD_STAT    ((UINT32)117)
#define             MM_CRANK_THREAD_STAT    ((UINT32)118)
#define             MM_CPROC_MODULE_STAT    ((UINT32)119)
#define                 MM_CSYS_ETH_STAT    ((UINT32)120)
#define                 MM_CSYS_DSK_STAT    ((UINT32)121)
#define            MM_MM_MAN_OCCUPY_NODE    ((UINT32)122)
#define              MM_MM_MAN_LOAD_NODE    ((UINT32)123)
#define                  MM_CTHREAD_NODE    ((UINT32)124)
#define                  MM_CTHREAD_POOL    ((UINT32)125)
#define                MM_TASK_RANK_NODE    ((UINT32)126)
#define                       MM_CMD_SEG    ((UINT32)127)
#define                      MM_CMD_PARA    ((UINT32)128)
#define                      MM_CMD_HELP    ((UINT32)129)
#define                      MM_CMD_ELEM    ((UINT32)130)
#define              MM_TASK_REPORT_NODE    ((UINT32)131)
#define                    MM_CHASH_NODE    ((UINT32)132)
#define                     MM_CHASH_VEC    ((UINT32)133)
#define                  MM_CHASHDB_ITEM    ((UINT32)134)
#define                       MM_CHASHDB    ((UINT32)135)
#define                MM_CHASHDB_BUCKET    ((UINT32)136)
#define                        MM_CDFSNP    ((UINT32)137)
#define                   MM_CDFSNP_ITEM    ((UINT32)138)
#define                  MM_CDFSNP_INODE    ((UINT32)139)
#define                  MM_CDFSNP_FNODE    ((UINT32)140)
#define                  MM_CDFSNP_DNODE    ((UINT32)141)
#define                  MM_CDFSDN_CACHE    ((UINT32)142)
#define                  MM_CDFSDN_BLOCK    ((UINT32)143)
#define                        MM_CDFSDN    ((UINT32)144)
#define                 MM_CDFSDN_RECORD    ((UINT32)145)
#define             MM_CDFSDN_RECORD_MGR    ((UINT32)146)
#define                    MM_CLOAD_STAT    ((UINT32)147) 
#define                    MM_CLOAD_NODE    ((UINT32)148)
#define                    MM_CDFSNP_MGR    ((UINT32)149)
#define                   MM_CDFSDN_STAT    ((UINT32)150)
#define                MM_TYPE_CONV_ITEM    ((UINT32)151)
#define                          MM_CSRV    ((UINT32)152)
#define                    MM_CFUSE_MODE    ((UINT32)153)
#define                    MM_CFUSE_STAT    ((UINT32)154)
#define                    MM_CFUSE_NODE    ((UINT32)155)
#define                     MM_CFUSE_COP    ((UINT32)156)
#define                       MM_CBGT_KV    ((UINT32)157)
#define                        MM_CBYTES    ((UINT32)158)
#define                      MM_CBGT_REG    ((UINT32)159)
#define                       MM_CBITMAP    ((UINT32)160)
#define                  MM_CBTIMER_NODE    ((UINT32)161)
#define              MM_CLUSTER_NODE_CFG    ((UINT32)162)
#define                   MM_CLUSTER_CFG    ((UINT32)163)
#define                      MM_CPARACFG    ((UINT32)164)
#define                     MM_MCAST_CFG    ((UINT32)165)
#define                MM_BCAST_DHCP_CFG    ((UINT32)166)
#define                     MM_MACIP_CFG    ((UINT32)167)
#define                   MM_GANGLIA_CFG    ((UINT32)168)
#define                       MM_SYS_CFG    ((UINT32)169)
#define                   MM_SUPER_FNODE    ((UINT32)170)
#define                     MM_CMAP_NODE    ((UINT32)171)
#define                          MM_CMAP    ((UINT32)172)
#define                 MM_CSESSION_NODE    ((UINT32)173)
#define                 MM_CSESSION_ITEM    ((UINT32)174)
#define                        MM_CTIMET    ((UINT32)175)
#define                         MM_CSDOC    ((UINT32)176)
#define                        MM_CSWORD    ((UINT32)177)
#define                   MM_CSDOC_WORDS    ((UINT32)178)
#define                   MM_CSWORD_DOCS    ((UINT32)179)
#define                    MM_CBTREE_KEY    ((UINT32)180)
#define                   MM_CBTREE_NODE    ((UINT32)181)
#define                        MM_CBTREE    ((UINT32)182)
#define                      MM_CBGT_GDB    ((UINT32)183)
#define                MM_COROUTINE_TASK    ((UINT32)184)
#define                MM_COROUTINE_NODE    ((UINT32)185)
#define                MM_COROUTINE_POOL    ((UINT32)186)

#define                           MM_END    ((UINT32)256)
#define                        MM_IGNORE    ((UINT32)0xFFFF)

UINT32 init_static_mem();

UINT32 free_module_static_mem(UINT32 module_type, UINT32 module_id);

UINT32 breathing_static_mem();

void   print_static_mem_status(LOG *log);

UINT32 print_static_mem_diag_info(LOG *log);

void * safe_malloc(const UINT32 size, const UINT32 location);

void   safe_free(void *pvoid, const UINT32 location);

#endif/*_LIB_MM_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/


