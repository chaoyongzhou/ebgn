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

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "mm.h"
#include "moduleconst.h"
#include "debug.h"
#include "log.h"
#include "task.h"
#include "api_cmd.h"
#include "mod.inc"
#include "super.h"

#include "clist.h"
#include "cstack.h"
#include "cset.h"
#include "cqueue.h"
#include "cvector.h"
#include "cstring.h"

#include "tcnode.h"
#include "cmutex.h"
#include "cthread.h"
#include "vmm.h"
#include "vmatrixr.h"
#include "cfile.h"
#include "kbuff.h"
#include "cdir.h"
#include "cmon.h"
#include "csocket.h"
#include "ctimer.h"
#include "csys.h"
#include "taskcfg.inc"
#include "crouter.inc"
#include "chashnode.h"
#include "chashvec.h"
#include "chashdb.h"
#include "cdfsnp.h"
#include "cdfsdn.h"
#include "cdfsnpmgr.h"
#include "cdfs.h"
#include "cbloom.h"
#include "cload.h"
#include "csrv.h"
#include "cfuse.h"
#include "cproc.h"
#include "cbgt.h"
#include "csession.h"
#include "cbytes.h"
#include "cbitmap.h"
#include "cbtimer.h"
#include "cscore.h"
#include "cxml.h"
#include "cbtree.h"
#include "coroutine.h"
#include "cparacfg.inc"

/* memory manager will manage all node blocks and node block manage its nodes.*/
/* each memory manager manage only one type of node block */
/* and all nodes in a certain node block has the same type */
MM_MAN g_mem_manager[ MM_END ];
EC_BOOL g_mem_init_flag = EC_FALSE;

static MM_LOC g_mm_loc_tbl[] = {
#include "loc_tbl.inc"
};


#if 0
#define MAN_INIT_LOCK(pMan, __location__)      cmutex_init((CMUTEX *)&((pMan)->cmutex), CMUTEX_PROCESS_PRIVATE, __location__)
#define MAN_CLEAN_LOCK(pMan, __location__)     cmutex_clean((CMUTEX *)&((pMan)->cmutex), __location__)
#define MAN_LOCK(pMan, __location__)           do{cmutex_lock((CMUTEX *)&((pMan)->cmutex), __location__); sys_log(LOGSTDOUT, "%s:%d: Man %lx is locked\n", MM_LOC_FILE_NAME(__location__), MM_LOC_LINE_NO(__location__), (pMan));}while(0)
#define MAN_UNLOCK(pMan, __location__)         do{cmutex_unlock((CMUTEX *)&((pMan)->cmutex), __location__);sys_log(LOGSTDOUT, "%s:%d: Man %lx is unlocked\n", MM_LOC_FILE_NAME(__location__), MM_LOC_LINE_NO(__location__), (pMan));}while(0)
#endif

#if 1
#define MAN_INIT_LOCK(pMan, __location__)      cmutex_init((CMUTEX *)&((pMan)->cmutex), CMUTEX_PROCESS_PRIVATE, __location__)
#define MAN_CLEAN_LOCK(pMan, __location__)     cmutex_clean((CMUTEX *)&((pMan)->cmutex), __location__)
#define MAN_LOCK(pMan, __location__)           cmutex_lock((CMUTEX *)&((pMan)->cmutex), __location__)
#define MAN_UNLOCK(pMan, __location__)         cmutex_unlock((CMUTEX *)&((pMan)->cmutex), __location__)
#endif


/**
*
*   Initilize the memory managers including managed node type, node size and node number per node block.
*   This function does not allocate any dynamic memory. But before allocate any memory, the memory manager
*   has to be initilized at first.
*
**/
#define MM_MGR_INIT(mm_type, block_num, type_size, __location__) do{\
    pMan = &(g_mem_manager[ (mm_type) ]);\
    pMan->type = (mm_type);\
    pMan->name = (UINT8 *)"UNDEF";\
    pMan->nodenumsum = 0;\
    pMan->nodeblocknum = 0;\
    pMan->nodenumperblock = (block_num);\
    pMan->typesize = (type_size);\
    pMan->maxusedsum = 0;\
    pMan->curusedsum = 0;\
    MAN_LINKNODEBLOCK_HEAD_INIT(pMan);\
    MAN_FREENODEBLOCK_HEAD_INIT(pMan);\
    MAN_INIT_LOCK(pMan, (__location__));/*init lock*/\
}while(0)

#define MM_MGR_DEF(mm_type, mm_name, block_num, type_size, __location__) do{\
    pMan = &(g_mem_manager[ (mm_type) ]);\
    pMan->type = (mm_type);\
    pMan->name = (UINT8 *)(mm_name);\
    pMan->nodenumsum = 0;\
    pMan->nodeblocknum = 0;\
    pMan->nodenumperblock = (block_num);\
    pMan->typesize = (type_size);\
    pMan->maxusedsum = 0;\
    pMan->curusedsum = 0;\
    MAN_LINKNODEBLOCK_HEAD_INIT(pMan);\
    MAN_FREENODEBLOCK_HEAD_INIT(pMan);\
    MAN_INIT_LOCK(pMan, (__location__));/*init lock*/\
}while(0)

static UINT32 init_mem_manager()
{
    MM_MAN *pMan;

    UINT32 idx;
    for(idx = 0; idx < MM_END; idx ++)
    {
        MM_MGR_INIT(idx, 0, 0, LOC_MM_0001);/*not init lock here*/
    }

    MM_MGR_DEF(MM_UINT32                       ,"MM_UINT32                       ", 32      , sizeof(UINT32)                     , LOC_MM_0002);
    MM_MGR_DEF(MM_UINT16                       ,"MM_UINT16                       ", 32      , sizeof(UINT16)                     , LOC_MM_0003);
    MM_MGR_DEF(MM_UINT8                        ,"MM_UINT8                        ", 32      , sizeof(UINT8)                      , LOC_MM_0004);

    MM_MGR_DEF(MM_BIGINT                       ,"MM_BIGINT                       ", 1024    , sizeof(BIGINT)                     , LOC_MM_0005);
    MM_MGR_DEF(MM_NAF                          ,"MM_NAF                          ", 2       , (MAX_NAF_ARRAY_LEN * sizeof (int )), LOC_MM_0006);

    MM_MGR_DEF(MM_CURVE_POINT                  ,"MM_CURVE_POINT                  ", 1024    , sizeof(EC_CURVE_POINT)              , LOC_MM_0007);
    MM_MGR_DEF(MM_CURVE_AFFINE_POINT           ,"MM_CURVE_AFFINE_POINT           ", 16      , sizeof(EC_CURVE_AFF_POINT)          , LOC_MM_0008);
    MM_MGR_DEF(MM_ECF2N_CURVE                  ,"MM_ECF2N_CURVE                  ", 8       , sizeof(ECF2N_CURVE)                 , LOC_MM_0009);
    MM_MGR_DEF(MM_ECFP_CURVE                   ,"MM_ECFP_CURVE                   ", 8       , sizeof(ECFP_CURVE)                  , LOC_MM_0010);
    MM_MGR_DEF(MM_KEY_PAIR                     ,"MM_KEY_PAIR                     ", 128     , sizeof(ECC_KEYPAIR)                 , LOC_MM_0011);
    MM_MGR_DEF(MM_EBGN                         ,"MM_EBGN                         ", 128     , sizeof(EBGN)                        , LOC_MM_0012);
    MM_MGR_DEF(MM_EBGN_ITEM                    ,"MM_EBGN_ITEM                    ", 128     , sizeof(EBGN_ITEM)                   , LOC_MM_0013);
    MM_MGR_DEF(MM_POLY                         ,"MM_POLY                         ", 512     , sizeof(POLY)                        , LOC_MM_0014);
    MM_MGR_DEF(MM_POLY_ITEM                    ,"MM_POLY_ITEM                    ", 1024    , sizeof(POLY_ITEM)                   , LOC_MM_0015);
    MM_MGR_DEF(MM_DEGREE                       ,"MM_DEGREE                       ", 8       , sizeof(DEGREE)                      , LOC_MM_0016);

    MM_MGR_DEF(MM_STRCHAR128B                  ,"MM_STRCHAR128B                  ", 8       , 128                                 , LOC_MM_0017);
    MM_MGR_DEF(MM_STRCHAR256B                  ,"MM_STRCHAR256B                  ", 64      , 256                                 , LOC_MM_0018);
    MM_MGR_DEF(MM_STRCHAR512B                  ,"MM_STRCHAR512B                  ", 8       , 512                                 , LOC_MM_0019);
    MM_MGR_DEF(MM_STRCHAR1K                    ,"MM_STRCHAR1K                    ", 8       , 1024                                , LOC_MM_0020);
    MM_MGR_DEF(MM_STRCHAR2K                    ,"MM_STRCHAR2K                    ", 8       , 2048                                , LOC_MM_0021);
    MM_MGR_DEF(MM_STRCHAR4K                    ,"MM_STRCHAR4K                    ", 4       , 4096                                , LOC_MM_0022);
    MM_MGR_DEF(MM_STRCHAR10K                   ,"MM_STRCHAR10K                   ", 128     , 10240                               , LOC_MM_0023);
    MM_MGR_DEF(MM_STRCHAR64K                   ,"MM_STRCHAR64K                   ", 128     , 1024 * 64                           , LOC_MM_0024);
    MM_MGR_DEF(MM_STRCHAR1M                    ,"MM_STRCHAR1M                    ", 4       , 1024 * 1024 * 1                     , LOC_MM_0025);
    MM_MGR_DEF(MM_STRCHAR2M                    ,"MM_STRCHAR2M                    ", 1       , 1024 * 1024 * 2                     , LOC_MM_0026);
    MM_MGR_DEF(MM_STRCHAR4M                    ,"MM_STRCHAR4M                    ", 1       , 1024 * 1024 * 4                     , LOC_MM_0027);
    MM_MGR_DEF(MM_STRCHAR8M                    ,"MM_STRCHAR8M                    ", 1       , 1024 * 1024 * 8                     , LOC_MM_0028);
    MM_MGR_DEF(MM_STRCHAR16M                   ,"MM_STRCHAR16M                   ", 1       , 1024 * 1024 * 16                    , LOC_MM_0029);
    MM_MGR_DEF(MM_STRCHAR32M                   ,"MM_STRCHAR32M                   ", 1       , 1024 * 1024 * 32                    , LOC_MM_0030);
    MM_MGR_DEF(MM_STRCHAR64M                   ,"MM_STRCHAR64M                   ", 1       , 1024 * 1024 * 64                    , LOC_MM_0031);
    MM_MGR_DEF(MM_STRCHAR128M                  ,"MM_STRCHAR128M                  ", 1       , 1024 * 1024 * 128                   , LOC_MM_0032);

    MM_MGR_DEF(MM_REAL                         ,"MM_REAL                         ", 1024    , sizeof(REAL)                        , LOC_MM_0033);
    MM_MGR_DEF(MM_MATRIX_BLOCK                 ,"MM_MATRIX_BLOCK                 ", 8 * 8   , sizeof(MATRIX_BLOCK)                , LOC_MM_0034);
    MM_MGR_DEF(MM_MATRIX_HEADER                ,"MM_MATRIX_HEADER                ", 8 * 2   , sizeof(void *)                      , LOC_MM_0035);/*MATRIX_HEADER is removed, so this line is not meaningful*/
    MM_MGR_DEF(MM_MATRIX                       ,"MM_MATRIX                       ", 4       , sizeof(MATRIX)                      , LOC_MM_0036);
    MM_MGR_DEF(MM_VECTOR_BLOCK                 ,"MM_VECTOR_BLOCK                 ", 8       , sizeof(VECTOR_BLOCK)                , LOC_MM_0037);
    MM_MGR_DEF(MM_VECTOR                       ,"MM_VECTOR                       ", 32      , sizeof(VECTOR)                      , LOC_MM_0038);

#ifdef PARSER_MEM_MGR
    MM_MGR_DEF(MM_PROD                         ,"MM_PROD                         ", 1024    , sizeof(struct prod_token_item)      , LOC_MM_0039);
    MM_MGR_DEF(MM_TREE                         ,"MM_TREE                         ", 1024    , sizeof(union tree_node)             , LOC_MM_0040);
#else
    MM_MGR_DEF(MM_PROD_RSVD                    ,"MM_PROD_RSVD                    ", 1024    , 1                                   , LOC_MM_0041);
    MM_MGR_DEF(MM_TREE_RSVD                    ,"MM_TREE_RSVD                    ", 1024    , 1                                   , LOC_MM_0042);
#endif/*PARSER_MEM_MGR*/

    MM_MGR_DEF(MM_TASK_NODE                    ,"MM_TASK_NODE                    ", 1024    , sizeof(TASK_NODE)                   , LOC_MM_0043);
    MM_MGR_DEF(MM_TASK_MGR                     ,"MM_TASK_MGR                     ", 32      , sizeof(TASK_MGR)                    , LOC_MM_0044);
    MM_MGR_DEF(MM_TASK_CONTEXT                 ,"MM_TASK_CONTEXT                 ", 32      , sizeof(TASK_CONTEXT)                , LOC_MM_0045);

    MM_MGR_DEF(MM_MOD_NODE                     ,"MM_MOD_NODE                     ", 1024    , sizeof(MOD_NODE)                    , LOC_MM_0046);
    MM_MGR_DEF(MM_MOD_MGR                      ,"MM_MOD_MGR                      ", 32      , sizeof(MOD_MGR)                     , LOC_MM_0047);

    MM_MGR_DEF(MM_TASKC_MGR                    ,"MM_TASKC_MGR                    ", 32      , sizeof(TASKC_MGR)                   , LOC_MM_0048);

    MM_MGR_DEF(MM_CLIST_DATA                   ,"MM_CLIST_DATA                   ", 1024    , sizeof(CLIST_DATA)                  , LOC_MM_0049);
    MM_MGR_DEF(MM_CSTACK_DATA                  ,"MM_CSTACK_DATA                  ", 1024    , sizeof(CSTACK_DATA)                 , LOC_MM_0050);
    MM_MGR_DEF(MM_CSET_DATA                    ,"MM_CSET_DATA                    ", 1024    , sizeof(CSET_DATA)                   , LOC_MM_0051);
    MM_MGR_DEF(MM_CQUEUE_DATA                  ,"MM_CQUEUE_DATA                  ", 1024    , sizeof(CQUEUE_DATA)                 , LOC_MM_0052);
    MM_MGR_DEF(MM_CSTRING                      ,"MM_CSTRING                      ", 32      , sizeof(CSTRING)                     , LOC_MM_0053);

    MM_MGR_DEF(MM_FUNC_ADDR_MGR                ,"MM_FUNC_ADDR_MGR                ", 16      , sizeof(FUNC_ADDR_MGR)               , LOC_MM_0054);

    MM_MGR_DEF(MM_UINT8_064B                   ,"MM_UINT8_064B                   ", 64      , 64                                  , LOC_MM_0055);
    MM_MGR_DEF(MM_UINT8_128B                   ,"MM_UINT8_128B                   ", 32      , 128                                 , LOC_MM_0056);
    MM_MGR_DEF(MM_UINT8_256B                   ,"MM_UINT8_256B                   ", 16      , 256                                 , LOC_MM_0057);
    MM_MGR_DEF(MM_UINT8_512B                   ,"MM_UINT8_512B                   ", 8       , 512                                 , LOC_MM_0058);

    MM_MGR_DEF(MM_UINT8_001K                   ,"MM_UINT8_001K                   ", 64      , 1 * 1024                            , LOC_MM_0059);
    MM_MGR_DEF(MM_UINT8_002K                   ,"MM_UINT8_002K                   ", 256     , 2 * 1024                            , LOC_MM_0060);
    MM_MGR_DEF(MM_UINT8_004K                   ,"MM_UINT8_004K                   ", 16      , 4 * 1024                            , LOC_MM_0061);
    MM_MGR_DEF(MM_UINT8_008K                   ,"MM_UINT8_008K                   ", 8       , 5 * 1024                            , LOC_MM_0062);
    MM_MGR_DEF(MM_UINT8_016K                   ,"MM_UINT8_016K                   ", 4       , 16 * 1024                           , LOC_MM_0063);
    MM_MGR_DEF(MM_UINT8_032K                   ,"MM_UINT8_032K                   ", 2       , 32 * 1024                           , LOC_MM_0064);
    MM_MGR_DEF(MM_UINT8_064K                   ,"MM_UINT8_064K                   ", 8       , 64 * 1024                           , LOC_MM_0065);
    MM_MGR_DEF(MM_UINT8_128K                   ,"MM_UINT8_128K                   ", 4       , 128 * 1024                          , LOC_MM_0066);
    MM_MGR_DEF(MM_UINT8_256K                   ,"MM_UINT8_256K                   ", 2       , 256 * 1024                          , LOC_MM_0067);
    MM_MGR_DEF(MM_UINT8_512K                   ,"MM_UINT8_512K                   ", 1       , 512 * 1024                          , LOC_MM_0068);

    MM_MGR_DEF(MM_UINT8_001M                   ,"MM_UINT8_001M                   ", 64      , 1 * 1024 * 1024                     , LOC_MM_0069);
    MM_MGR_DEF(MM_UINT8_002M                   ,"MM_UINT8_002M                   ", 32      , 2 * 1024 * 1024                     , LOC_MM_0070);
    MM_MGR_DEF(MM_UINT8_004M                   ,"MM_UINT8_004M                   ", 16      , 4 * 1024 * 1024                     , LOC_MM_0071);
    MM_MGR_DEF(MM_UINT8_008M                   ,"MM_UINT8_008M                   ", 8       , 5 * 1024 * 1024                     , LOC_MM_0072);
    MM_MGR_DEF(MM_UINT8_016M                   ,"MM_UINT8_016M                   ", 4       , 16 * 1024 * 1024                    , LOC_MM_0073);
    MM_MGR_DEF(MM_UINT8_032M                   ,"MM_UINT8_032M                   ", 2       , 32 * 1024 * 1024                    , LOC_MM_0074);
    MM_MGR_DEF(MM_UINT8_064M                   ,"MM_UINT8_064M                   ", 2       , 64 * 1024 * 1024                    , LOC_MM_0075);
    MM_MGR_DEF(MM_UINT8_128M                   ,"MM_UINT8_128M                   ", 2       , 128 * 1024 * 1024                   , LOC_MM_0076);
    MM_MGR_DEF(MM_UINT8_256M                   ,"MM_UINT8_256M                   ", 1       , 256 * 1024 * 1024                   , LOC_MM_0077);
    MM_MGR_DEF(MM_UINT8_512M                   ,"MM_UINT8_512M                   ", 1       , 512 * 1024 * 1024                   , LOC_MM_0078);

    MM_MGR_DEF(MM_REAL_BLOCK_2_2               ,"MM_REAL_BLOCK_2_2               ", 128     , 2 *    2 * sizeof(REAL)             , LOC_MM_0079);
    MM_MGR_DEF(MM_REAL_BLOCK_4_4               ,"MM_REAL_BLOCK_4_4               ", 128     , 4 *    4 * sizeof(REAL)             , LOC_MM_0080);
    MM_MGR_DEF(MM_REAL_BLOCK_8_8               ,"MM_REAL_BLOCK_8_8               ", 128     , 8 *    8 * sizeof(REAL)             , LOC_MM_0081);
    MM_MGR_DEF(MM_REAL_BLOCK_16_16             ,"MM_REAL_BLOCK_16_16             ", 128     , 16 *   16 * sizeof(REAL)            , LOC_MM_0082);
    MM_MGR_DEF(MM_REAL_BLOCK_32_32             ,"MM_REAL_BLOCK_32_32             ", 64      , 32 *   32 * sizeof(REAL)            , LOC_MM_0083);
    MM_MGR_DEF(MM_REAL_BLOCK_64_64             ,"MM_REAL_BLOCK_64_64             ", 32      , 64 *   64 * sizeof(REAL)            , LOC_MM_0084);
    MM_MGR_DEF(MM_REAL_BLOCK_128_128           ,"MM_REAL_BLOCK_128_128           ", 16      , 128 *  128 * sizeof(REAL)           , LOC_MM_0085);
    MM_MGR_DEF(MM_REAL_BLOCK_256_256           ,"MM_REAL_BLOCK_256_256           ", 8       , 256 *  256 * sizeof(REAL)           , LOC_MM_0086);
    MM_MGR_DEF(MM_REAL_BLOCK_512_512           ,"MM_REAL_BLOCK_512_512           ", 4       , 512 *  512 * sizeof(REAL)           , LOC_MM_0087);
    MM_MGR_DEF(MM_REAL_BLOCK_1024_1024         ,"MM_REAL_BLOCK_1024_1024         ", 4       , 1024 * 1024 * sizeof(REAL)          , LOC_MM_0088);

    MM_MGR_DEF(MM_CLIST                        ,"MM_CLIST                        ", 32      , sizeof(CLIST)                       , LOC_MM_0089);
    MM_MGR_DEF(MM_CSTACK                       ,"MM_CSTACK                       ", 32      , sizeof(CSTACK)                      , LOC_MM_0090);
    MM_MGR_DEF(MM_CSET                         ,"MM_CSET                         ", 32      , sizeof(CSET)                        , LOC_MM_0091);
    MM_MGR_DEF(MM_CQUEUE                       ,"MM_CQUEUE                       ", 32      , sizeof(CQUEUE)                      , LOC_MM_0092);
    MM_MGR_DEF(MM_CVECTOR                      ,"MM_CVECTOR                      ", 32      , sizeof(CVECTOR)                     , LOC_MM_0093);

    MM_MGR_DEF(MM_CTHREAD_TASK                 ,"MM_CTHREAD_TASK                 ", 32      , sizeof(CTHREAD_TASK)                , LOC_MM_0094);

    MM_MGR_DEF(MM_TASKS_CFG                    ,"MM_TASKS_CFG                    ", 32      , sizeof(TASKS_CFG)                   , LOC_MM_0095);
    MM_MGR_DEF(MM_TASKR_CFG                    ,"MM_TASKR_CFG                    ", 32      , sizeof(TASKR_CFG)                   , LOC_MM_0096);
    MM_MGR_DEF(MM_TASK_CFG                     ,"MM_TASK_CFG                     ", 1       , sizeof(TASK_CFG)                    , LOC_MM_0097);

    MM_MGR_DEF(MM_TASKS_NODE                   ,"MM_TASKS_NODE                   ", 32      , sizeof(TASKS_NODE)                  , LOC_MM_0098);
    MM_MGR_DEF(MM_TASKC_NODE                   ,"MM_TASKC_NODE                   ", 32      , sizeof(TASKC_NODE)                  , LOC_MM_0099);
    MM_MGR_DEF(MM_CROUTER_NODE                 ,"MM_CROUTER_NODE                 ", 128     , sizeof(CROUTER_NODE)                , LOC_MM_0100);
    MM_MGR_DEF(MM_CROUTER_NODE_VEC             ,"MM_CROUTER_NODE_VEC             ", 128     , sizeof(CROUTER_NODE_VEC)            , LOC_MM_0101);
    MM_MGR_DEF(MM_CROUTER_CFG                  ,"MM_CROUTER_CFG                  ", 128     , sizeof(CROUTER_CFG)                 , LOC_MM_0102);

    MM_MGR_DEF(MM_VMM_NODE                     ,"MM_VMM_NODE                     ", 32      , sizeof(VMM_NODE)                    , LOC_MM_0103);
    MM_MGR_DEF(MM_LOG                          ,"MM_LOG                          ", 4       , sizeof(LOG)                         , LOC_MM_0104);

    MM_MGR_DEF(MM_COMM_NODE                    ,"MM_COMM_NODE                    ", 128     , sizeof(MM_COMM)                     , LOC_MM_0105);
    MM_MGR_DEF(MM_CFILE_SEG                    ,"MM_CFILE_SEG                    ", 128     , sizeof(CFILE_SEG)                   , LOC_MM_0106);
    MM_MGR_DEF(MM_CFILE_NODE                   ,"MM_CFILE_NODE                   ", 128     , sizeof(CFILE_NODE)                  , LOC_MM_0107);
    MM_MGR_DEF(MM_KBUFF                        ,"MM_KBUFF                        ", 128     , sizeof(KBUFF)                       , LOC_MM_0108);
    MM_MGR_DEF(MM_CDIR_SEG                     ,"MM_CDIR_SEG                     ", 128     , sizeof(CDIR_SEG)                    , LOC_MM_0109);
    MM_MGR_DEF(MM_CDIR_NODE                    ,"MM_CDIR_NODE                    ", 128     , sizeof(CDIR_NODE)                   , LOC_MM_0110);

    MM_MGR_DEF(MM_CMON_OBJ                     ,"MM_CMON_OBJ                     ", 128     , sizeof(CMON_OBJ)                    , LOC_MM_0111);
    MM_MGR_DEF(MM_CMON_OBJ_VEC                 ,"MM_CMON_OBJ_VEC                 ", 128     , sizeof(CMON_OBJ_VEC)                , LOC_MM_0112);
    MM_MGR_DEF(MM_CMON_OBJ_MERGE               ,"MM_CMON_OBJ_MERGE               ", 128     , sizeof(CMON_OBJ_MERGE)              , LOC_MM_0113);

    MM_MGR_DEF(MM_CSOCKET_CNODE                ,"MM_CSOCKET_CNODE                ", 128     , sizeof(CSOCKET_CNODE)               , LOC_MM_0114);
    MM_MGR_DEF(MM_CTIMER_NODE                  ,"MM_CTIMER_NODE                  ", 128     , sizeof(CTIMER_NODE)                 , LOC_MM_0115);

    MM_MGR_DEF(MM_CSYS_CPU_STAT                ,"MM_CSYS_CPU_STAT                ", 128     , sizeof(CSYS_CPU_STAT)               , LOC_MM_0116);
    MM_MGR_DEF(MM_CSYS_CPU_AVG_STAT            ,"MM_CSYS_CPU_AVG_STAT            ", 128     , sizeof(CSYS_CPU_AVG_STAT)           , LOC_MM_0117);
    MM_MGR_DEF(MM_CSYS_MEM_STAT                ,"MM_CSYS_MEM_STAT                ", 4       , sizeof(CSYS_MEM_STAT)               , LOC_MM_0118);
    MM_MGR_DEF(MM_CPROC_MEM_STAT               ,"MM_CPROC_MEM_STAT               ", 4       , sizeof(CPROC_MEM_STAT)              , LOC_MM_0119);
    MM_MGR_DEF(MM_CPROC_CPU_STAT               ,"MM_CPROC_CPU_STAT               ", 4       , sizeof(CPROC_CPU_STAT)              , LOC_MM_0120);
    MM_MGR_DEF(MM_CPROC_THREAD_STAT            ,"MM_CPROC_THREAD_STAT            ", 4       , sizeof(CPROC_THREAD_STAT)           , LOC_MM_0121);
    MM_MGR_DEF(MM_CRANK_THREAD_STAT            ,"MM_CRANK_THREAD_STAT            ", 4       , sizeof(CRANK_THREAD_STAT)           , LOC_MM_0122);
    MM_MGR_DEF(MM_CPROC_MODULE_STAT            ,"MM_CPROC_MODULE_STAT            ", 4       , sizeof(CPROC_MODULE_STAT)           , LOC_MM_0123);
    MM_MGR_DEF(MM_CSYS_ETH_STAT                ,"MM_CSYS_ETH_STAT                ", 4       , sizeof(CSYS_ETH_STAT)               , LOC_MM_0124);
    MM_MGR_DEF(MM_CSYS_DSK_STAT                ,"MM_CSYS_DSK_STAT                ", 4       , sizeof(CSYS_DSK_STAT)               , LOC_MM_0125);
    MM_MGR_DEF(MM_MM_MAN_OCCUPY_NODE           ,"MM_MM_MAN_OCCUPY_NODE           ", 128     , sizeof(MM_MAN_OCCUPY_NODE)          , LOC_MM_0126);
    MM_MGR_DEF(MM_MM_MAN_LOAD_NODE             ,"MM_MM_MAN_LOAD_NODE             ", 128     , sizeof(MM_MAN_LOAD_NODE)            , LOC_MM_0127);
    MM_MGR_DEF(MM_CTHREAD_NODE                 ,"MM_CTHREAD_NODE                 ", 16      , sizeof(CTHREAD_NODE)                , LOC_MM_0128);
    MM_MGR_DEF(MM_CTHREAD_POOL                 ,"MM_CTHREAD_POOL                 ", 1       , sizeof(CTHREAD_POOL)                , LOC_MM_0129);
    MM_MGR_DEF(MM_TASK_RANK_NODE               ,"MM_TASK_RANK_NODE               ", 1       , sizeof(TASK_RANK_NODE)              , LOC_MM_0130);
    MM_MGR_DEF(MM_CMD_SEG                      ,"MM_CMD_SEG                      ", 128     , sizeof(CMD_SEG)                     , LOC_MM_0131);
    MM_MGR_DEF(MM_CMD_PARA                     ,"MM_CMD_PARA                     ", 8       , sizeof(CMD_PARA)                    , LOC_MM_0132);
    MM_MGR_DEF(MM_CMD_HELP                     ,"MM_CMD_HELP                     ", 32      , sizeof(CMD_HELP)                    , LOC_MM_0133);
    MM_MGR_DEF(MM_CMD_ELEM                     ,"MM_CMD_ELEM                     ", 32      , sizeof(CMD_ELEM)                    , LOC_MM_0134);
    MM_MGR_DEF(MM_TASK_REPORT_NODE             ,"MM_TASK_REPORT_NODE             ", 128     , sizeof(TASK_REPORT_NODE)            , LOC_MM_0135);

    MM_MGR_DEF(MM_CHASH_NODE                   ,"MM_CHASH_NODE                   ", 32      , sizeof(CHASH_NODE)                  , LOC_MM_0136);
    MM_MGR_DEF(MM_CHASH_VEC                    ,"MM_CHASH_VEC                    ", 4       , sizeof(CHASH_VEC)                   , LOC_MM_0137);

    MM_MGR_DEF(MM_CHASHDB_ITEM                 ,"MM_CHASHDB_ITEM                 ", 4       , sizeof(CHASHDB_ITEM)                , LOC_MM_0138);
    MM_MGR_DEF(MM_CHASHDB                      ,"MM_CHASHDB                      ", 4       , sizeof(CHASHDB)                     , LOC_MM_0139);
    MM_MGR_DEF(MM_CHASHDB_BUCKET               ,"MM_CHASHDB_BUCKET               ", 4       , sizeof(CHASHDB_BUCKET)              , LOC_MM_0140);

    MM_MGR_DEF(MM_CDFSNP                       ,"MM_CDFSNP                       ", 4       , sizeof(CDFSNP)                      , LOC_MM_0141);
    MM_MGR_DEF(MM_CDFSNP_ITEM                  ,"MM_CDFSNP_ITEM                  ", 4       , sizeof(CDFSNP_ITEM)                 , LOC_MM_0142);
    MM_MGR_DEF(MM_CDFSNP_INODE                 ,"MM_CDFSNP_INODE                 ", 4       , sizeof(CDFSNP_INODE)                , LOC_MM_0143);
    MM_MGR_DEF(MM_CDFSNP_FNODE                 ,"MM_CDFSNP_FNODE                 ", 4       , sizeof(CDFSNP_FNODE)                , LOC_MM_0144);
    MM_MGR_DEF(MM_CDFSNP_DNODE                 ,"MM_CDFSNP_DNODE                 ", 4       , sizeof(CDFSNP_DNODE)                , LOC_MM_0145);

    MM_MGR_DEF(MM_CDFSDN_CACHE                 ,"MM_CDFSDN_CACHE                 ", 4       , sizeof(CDFSDN_CACHE)                , LOC_MM_0146);
    MM_MGR_DEF(MM_CDFSDN_BLOCK                 ,"MM_CDFSDN_BLOCK                 ", 4       , sizeof(CDFSDN_BLOCK)                , LOC_MM_0147);
    MM_MGR_DEF(MM_CDFSDN                       ,"MM_CDFSDN                       ", 4       , sizeof(CDFSDN)                      , LOC_MM_0148);
    MM_MGR_DEF(MM_CDFSDN_RECORD                ,"MM_CDFSDN_RECORD                ", 4       , sizeof(CDFSDN_RECORD)               , LOC_MM_0149);
    MM_MGR_DEF(MM_CDFSDN_RECORD_MGR            ,"MM_CDFSDN_RECORD_MGR            ", 4       , sizeof(CDFSDN_RECORD_MGR)           , LOC_MM_0150);

    MM_MGR_DEF(MM_CLOAD_STAT                   ,"MM_CLOAD_STAT                   ", 8       , sizeof(CLOAD_STAT)                  , LOC_MM_0151);
    MM_MGR_DEF(MM_CLOAD_NODE                   ,"MM_CLOAD_NODE                   ", 8       , sizeof(CLOAD_NODE)                  , LOC_MM_0152);

    MM_MGR_DEF(MM_CDFSNP_MGR                   ,"MM_CDFSNP_MGR                   ", 8       , sizeof(CDFSNP_MGR)                  , LOC_MM_0153);
    MM_MGR_DEF(MM_CDFSDN_STAT                  ,"MM_CDFSDN_STAT                  ", 8       , sizeof(CDFSDN_STAT)                 , LOC_MM_0154);

    MM_MGR_DEF(MM_TYPE_CONV_ITEM               ,"MM_TYPE_CONV_ITEM               ",64       , sizeof(TYPE_CONV_ITEM)              , LOC_MM_0155);
    MM_MGR_DEF(MM_CSRV                         ,"MM_CSRV                         ",64       , sizeof(CSRV)                        , LOC_MM_0156);
    MM_MGR_DEF(MM_CFUSE_MODE                   ,"MM_CFUSE_MODE                   ",64       , sizeof(CFUSE_MODE)                  , LOC_MM_0157);
    MM_MGR_DEF(MM_CFUSE_STAT                   ,"MM_CFUSE_STAT                   ",64       , sizeof(CFUSE_STAT)                  , LOC_MM_0158);
    MM_MGR_DEF(MM_CFUSE_NODE                   ,"MM_CFUSE_NODE                   ",64       , sizeof(CFUSE_NODE)                  , LOC_MM_0159);
    MM_MGR_DEF(MM_CFUSE_COP                    ,"MM_CFUSE_COP                    ",4        , sizeof(CFUSE_COP)                   , LOC_MM_0160);

    MM_MGR_DEF(MM_CBYTES                       ,"MM_CBYTES                       ",4        , sizeof(CBYTES)                      , LOC_MM_0161);
    MM_MGR_DEF(MM_CBGT_REG                     ,"MM_CBGT_REG                     ",4        , sizeof(CBGT_REG)                    , LOC_MM_0162);
    MM_MGR_DEF(MM_CBITMAP                      ,"MM_CBITMAP                      ",1        , sizeof(CBITMAP)                     , LOC_MM_0163);

    MM_MGR_DEF(MM_CBTIMER_NODE                 ,"MM_CBTIMER_NODE                 ",4        , sizeof(CBTIMER_NODE)                , LOC_MM_0164);
    MM_MGR_DEF(MM_CLUSTER_NODE_CFG             ,"MM_CLUSTER_NODE_CFG             ",4        , sizeof(CLUSTER_NODE_CFG)            , LOC_MM_0165);
    MM_MGR_DEF(MM_CLUSTER_CFG                  ,"MM_CLUSTER_CFG                  ",4        , sizeof(CLUSTER_CFG)                 , LOC_MM_0166);
    MM_MGR_DEF(MM_CPARACFG                     ,"MM_CPARACFG                     ",1        , sizeof(CPARACFG)                    , LOC_MM_0167);
    MM_MGR_DEF(MM_MCAST_CFG                    ,"MM_MCAST_CFG                    ",1        , sizeof(MCAST_CFG)                   , LOC_MM_0168);
    MM_MGR_DEF(MM_BCAST_DHCP_CFG               ,"MM_BCAST_DHCP_CFG               ",1        , sizeof(BCAST_DHCP_CFG)              , LOC_MM_0169);
    MM_MGR_DEF(MM_MACIP_CFG                    ,"MM_MACIP_CFG                    ",4        , sizeof(MACIP_CFG)                   , LOC_MM_0170);
    MM_MGR_DEF(MM_GANGLIA_CFG                  ,"MM_GANGLIA_CFG                  ",1        , sizeof(GANGLIA_CFG)                 , LOC_MM_0171);
    MM_MGR_DEF(MM_SYS_CFG                      ,"MM_SYS_CFG                      ",1        , sizeof(SYS_CFG)                     , LOC_MM_0172);

    MM_MGR_DEF(MM_SUPER_FNODE                  ,"MM_SUPER_FNODE                  ",1        , sizeof(SUPER_FNODE)                 , LOC_MM_0173);

    MM_MGR_DEF(MM_CMAP_NODE                    ,"MM_CMAP_NODE                    ",4        , sizeof(CMAP_NODE)                   , LOC_MM_0174);
    MM_MGR_DEF(MM_CMAP                         ,"MM_CMAP                         ",4        , sizeof(CMAP)                        , LOC_MM_0175);
    MM_MGR_DEF(MM_CSESSION_NODE                ,"MM_CSESSION_NODE                ",4        , sizeof(CSESSION_NODE)               , LOC_MM_0176);
    MM_MGR_DEF(MM_CSESSION_ITEM                ,"MM_CSESSION_ITEM                ",4        , sizeof(CSESSION_ITEM)               , LOC_MM_0177);
    MM_MGR_DEF(MM_CTIMET                       ,"MM_CTIMET                       ",4        , sizeof(CTIMET)                      , LOC_MM_0178);

    MM_MGR_DEF(MM_CSDOC                        ,"MM_CSDOC                        ",64       , sizeof(CSDOC)                       , LOC_MM_0179);
    MM_MGR_DEF(MM_CSWORD                       ,"MM_CSWORD                       ",64       , sizeof(CSWORD)                      , LOC_MM_0180);
    MM_MGR_DEF(MM_CSDOC_WORDS                  ,"MM_CSDOC_WORDS                  ",64       , sizeof(CSDOC_WORDS)                 , LOC_MM_0181);
    MM_MGR_DEF(MM_CSWORD_DOCS                  ,"MM_CSWORD_DOCS                  ",64       , sizeof(CSWORD_DOCS)                 , LOC_MM_0182);

    MM_MGR_DEF(MM_CBTREE_KEY                   ,"MM_CBTREE_KEY                   ",256      , sizeof(CBTREE_KEY)                  , LOC_MM_0183);
    MM_MGR_DEF(MM_CBTREE_NODE                  ,"MM_CBTREE_NODE                  ",256      , sizeof(CBTREE_NODE)                 , LOC_MM_0184);
    MM_MGR_DEF(MM_CBTREE                       ,"MM_CBTREE                       ",4        , sizeof(CBTREE)                      , LOC_MM_0185);

    MM_MGR_DEF(MM_CBGT_GDB                     ,"MM_CBGT_GDB                     ",4        , sizeof(CBGT_GDB)                    , LOC_MM_0186);

    MM_MGR_DEF(MM_COROUTINE_TASK               ,"MM_COROUTINE_TASK               ",4        , sizeof(COROUTINE_TASK)              , LOC_MM_0187);
    MM_MGR_DEF(MM_COROUTINE_NODE               ,"MM_COROUTINE_NODE               ",4        , sizeof(COROUTINE_NODE)              , LOC_MM_0188);
    MM_MGR_DEF(MM_COROUTINE_POOL               ,"MM_COROUTINE_POOL               ",4        , sizeof(COROUTINE_POOL)              , LOC_MM_0189);

    return ( 0 );
}
#undef MM_MGR_DEF

/**
*
*   initilize the static memory. this interface is for outer calling.
*   this interface will initilize the global memory managers and the
*   global memory manager initilized flag to EC_TRUE.
*
*
**/
UINT32 init_static_mem()
{
    if ( EC_TRUE == g_mem_init_flag)
    {
        return ( 0 );
    }

    g_mem_init_flag = EC_TRUE;

    init_mem_manager();

    return ( 0 );
}

UINT32 init_mm_man(const UINT32 mm_type)
{
    MM_MAN *pMan;

    if ( EC_TRUE == g_mem_init_flag)
    {
        sys_log(LOGSTDOUT,"error:init_mm_man: mm was already initialized.\n");
        return ((UINT32)-1);
    }

    if ( MM_END <= mm_type )
    {
        sys_log(LOGSTDOUT,"error:init_mm_man: type %ld is invalid\n", mm_type);
        return ((UINT32)-1);
    }

    pMan = &(g_mem_manager[ (mm_type) ]);

    if(0 != pMan->typesize)
    {
        sys_log(LOGSTDOUT, "error:init_mm_man: type %ld was already registered\n", mm_type);
        return ((UINT32)-1);
    }

    pMan->type = (mm_type);
    pMan->name = (UINT8 *)"UNDEF";
    pMan->nodenumsum = 0;
    pMan->nodeblocknum = 0;
    pMan->nodenumperblock = 0;
    pMan->typesize = (0);
    pMan->maxusedsum = 0;
    pMan->curusedsum = 0;
    MAN_LINKNODEBLOCK_HEAD_INIT(pMan);
    MAN_FREENODEBLOCK_HEAD_INIT(pMan);

    return (0);
}

UINT32 reg_mm_man(const UINT32 mm_type, const char *mm_name, const UINT32 block_num, const UINT32 type_size, const UINT32 location)
{
    MM_MAN *pMan;

    if ( EC_FALSE == g_mem_init_flag)
    {
        sys_log(LOGSTDOUT,"error:reg_mm_man: mm is not initialized yet.\n");
        return ((UINT32)-1);
    }

    if ( MM_END <= mm_type )
    {
        sys_log(LOGSTDOUT,"error:reg_mm_man: type %ld is invalid where location %ld.\n", mm_type, location);
        return ((UINT32)-1);
    }

    pMan = &(g_mem_manager[ mm_type ]);
    if(0 != pMan->typesize)
    {
        sys_log(LOGSTDOUT, "error:reg_mm_man: type %ld was already registered where location %ld\n", mm_type, location);
        return ((UINT32)-1);
    }

    pMan->type = (mm_type);
    pMan->name = (UINT8 *)(mm_name);
    pMan->nodenumsum = 0;
    pMan->nodeblocknum = 0;
    pMan->nodenumperblock = (block_num);
    pMan->typesize = (type_size);
    pMan->maxusedsum = 0;
    pMan->curusedsum = 0;
    MAN_LINKNODEBLOCK_HEAD_INIT(pMan);
    MAN_FREENODEBLOCK_HEAD_INIT(pMan);
    MAN_INIT_LOCK(pMan, location);/*init lock*/

    return ( 0 );
}

/**
*
*   get file name of location
*
**/
const char *get_file_name_of_location(const UINT32 location)
{
    if(LOC_NONE_BASE < location && location < LOC_NONE_END)
    {
        MM_LOC *mm_loc;

        mm_loc = &(g_mm_loc_tbl[ location ]);
        if(location != mm_loc->location)
        {
            sys_log(LOGSTDOUT, "error:get_file_name_of_location: mistached location %ld and mm location %ld\n", location, mm_loc->location);
            return ("mistached");
        }
        return (mm_loc->filename);
    }

    sys_log(LOGSTDOUT, "error:get_file_name_of_location: invalid location %ld\n", location);
    return ("overflow");
}

/**
*
*   get line no of location
*
**/
UINT32 get_line_no_of_location(const UINT32 location)
{
    if(LOC_NONE_BASE < location && location < LOC_NONE_END)
    {
        MM_LOC *mm_loc;

        mm_loc = &(g_mm_loc_tbl[ location ]);
        if(location != mm_loc->location)
        {
            sys_log(LOGSTDOUT, "error:get_line_no_of_location: mistached location %ld and mm location %ld\n", location, mm_loc->location);
            return (~((UINT32)0));
        }
        return (mm_loc->lineno);
    }

    sys_log(LOGSTDOUT, "error:get_line_no_of_location: invalid location %ld\n", location);
    return (~((UINT32)0));
}

/**
*
*   allocate a node block for static memory usage purpose.
*   it will allocate a dynamic memory block for nodes management purpose
*   and allocate another dynamic memory block for nodes themselves and link
*   the 2nd dynamic memory block to node block at pbaseaddr.
*
*   the 2nd dynamic memory block is splitted into several nodes and link them
*   one by one as free nodes. Note, they are all un-used at present.
*
*   set the free node index to point to the first free node in the node list.
*
**/

EC_BOOL man_debug(const UINT8 *info, const MM_MAN *pMan)
{
    MM_NODE_BLOCK *pNodeBlock;

    MAN_LOCK(pMan, LOC_MM_0190);

    sys_log(LOGSTDOUT, "[debug] ========================== man_debug beg ==========================\n\n");
    sys_log(LOGSTDOUT, "%s\n", info);
    sys_log(LOGSTDOUT, "[debug] man_debug: linknodeblockhead: (%lx, %lx, %lx)\n", MAN_LINKNODEBLOCK_HEAD(pMan), MAN_LINKNODEBLOCK_HEAD(pMan)->prev, MAN_LINKNODEBLOCK_HEAD(pMan)->next);
    sys_log(LOGSTDOUT, "[debug] man_debug: freenodeblockhead: (%lx, %lx, %lx)\n", MAN_FREENODEBLOCK_HEAD(pMan), MAN_FREENODEBLOCK_HEAD(pMan)->prev, MAN_FREENODEBLOCK_HEAD(pMan)->next);
    MAN_LINKNODEBLOCK_LOOP_NEXT(pMan, pNodeBlock)
    {
        sys_log(LOGSTDOUT, "[debug] man_debug: link nodeblock: %lx\n", pNodeBlock);
    }
    MAN_FREENODEBLOCK_LOOP_NEXT(pMan, pNodeBlock)
    {
        sys_log(LOGSTDOUT, "[debug] man_debug: free nodeblock: %lx\n", pNodeBlock);
    }
    sys_log(LOGSTDOUT, "[debug] ========================== man_debug end ==========================\n\n");

    MAN_UNLOCK(pMan, LOC_MM_0191);
    return (EC_TRUE);
}

EC_BOOL nodeblock_debug(const UINT8 *info, const MM_NODE_BLOCK *pNodeBlock)
{
    sys_log(LOGSTDOUT, "%s\n", info);
    sys_log(LOGSTDOUT, "[debug] nodeblock_debug: nodeblock %lx\n", pNodeBlock);
    sys_log(LOGSTDOUT, "[debug] nodeblock_debug: nodeblock %lx, link = (%lx, %lx, %lx)\n",
                    pNodeBlock,
                    NODEBLOCK_LINKNODE(pNodeBlock), NODEBLOCK_LINKNODE(pNodeBlock)->prev, NODEBLOCK_LINKNODE(pNodeBlock)->next);
    sys_log(LOGSTDOUT, "[debug] nodeblock_debug: nodeblock %lx, free = (%lx, %lx, %lx)\n",
                    pNodeBlock,
                    NODEBLOCK_FREENODE(pNodeBlock), NODEBLOCK_FREENODE(pNodeBlock)->prev, NODEBLOCK_FREENODE(pNodeBlock)->next);

    return (EC_TRUE);
}
/*note:pMan is locked by alloc_nodeblock_static_mem caller, so do not lock inside*/
EC_BOOL alloc_nodeblock_static_mem(const MM_TYPE_UINT32 type)
{
    MM_MAN *pMan;
    MM_NODE *pNode;
    MM_NODE_BLOCK *pNodeBlock;
    UINT32 node_idx;

    if ( MM_END <= type )
    {
        sys_log(LOGSTDOUT,"error:alloc_nodeblock_static_mem: type is invalid.\n");
        exit ( 0 );
    }

    pMan = &(g_mem_manager[ type ]);

    //man_debug("alloc_nodeblock_static_mem[1]: ", pMan);

    /*alloc a node block and intilize it */
    pNodeBlock = (MM_NODE_BLOCK *)malloc(sizeof(MM_NODE_BLOCK));
    if ( NULL_PTR == pNodeBlock )
    {
        sys_log(LOGSTDOUT,"error:alloc_nodeblock_static_mem: failed to alloc memory for node block.\n");

        return( EC_FALSE );
    }

    NODEBLOCK_LINKNODE_INIT(pNodeBlock);
    NODEBLOCK_FREENODE_INIT(pNodeBlock);
    pNodeBlock->nodenum   = pMan->nodenumperblock;
    pNodeBlock->typesize  = pMan->typesize;
    pNodeBlock->nextfree  = NODE_LIST_TAIL;
    pNodeBlock->pnodes    = NULL_PTR;
    pNodeBlock->pbaseaddr = NULL_PTR;
    pNodeBlock->ptailaddr = NULL_PTR;

    //nodeblock_debug("alloc_nodeblock_static_mem[2]: ", pNodeBlock);

    /* allocate the 1st dynamic memory block for nodes management purpose */
    pNodeBlock->pnodes = (MM_NODE *)malloc( pNodeBlock->nodenum * sizeof ( MM_NODE ) );
    if ( NULL_PTR == pNodeBlock->pnodes )
    {
        sys_log(LOGSTDOUT,"error:alloc_nodeblock_static_mem: failed to alloc memory for node with nodenum %ld.\n", pNodeBlock->nodenum);

        /*roll back*/
        free( pNodeBlock );
        return( EC_FALSE );
    }

    /* allocate another dynamic memory block for nodes themselves and link */
    /* the 2nd dynamic memory block to node block at pbaseaddr */
    pNodeBlock->pbaseaddr = (UINT8 *)malloc( pNodeBlock->nodenum * (pNodeBlock->typesize + sizeof(MM_AUX)) );
    if ( NULL_PTR == pNodeBlock->pbaseaddr )
    {
        sys_log(LOGSTDOUT,"error:alloc_nodeblock_static_mem: failed to alloc memory for type %ld with nodenum %ld.\n", type, pNodeBlock->nodenum);

        /*roll back*/
        free( pNodeBlock->pbaseaddr );
        free( pNodeBlock );
        return( EC_FALSE );
    }

    pNodeBlock->ptailaddr = pNodeBlock->pbaseaddr + (pNodeBlock->nodenum * (pNodeBlock->typesize + sizeof(MM_AUX)));

    for ( node_idx = 0; node_idx < pNodeBlock->nodenum; node_idx ++ )
    {
        MM_AUX *pAux;
        pNode = &(pNodeBlock->pnodes[ node_idx ]);

        /*link these free nodes one by one*/
        pNode->next = node_idx + 1;

        /*set the current node be un-used status*/
        pNode->usedflag = MM_NODE_NOT_USED;
#if ( SWITCH_ON == MULTI_USER_MODE_SWITCH )
        pNode->module_type = MD_END;
        pNode->module_id = ERR_MODULE_ID;
#endif/*SWITCH_ON == MULTI_USER_MODE_SWITCH*/

        /*link the memory piece which will be used by user's defined type but not memory manager*/
        pNode->pmem = pNodeBlock->pbaseaddr + ( node_idx * (pNodeBlock->typesize + sizeof(MM_AUX)) );

        pAux = (MM_AUX *)(pNode->pmem);
        pAux->type         = type;
        pAux->u.nodeblock = pNodeBlock; /*save memory block addr in each node*/
        //pAux->vmm_flag  = MM_IS_LOCAL_VMM;/*default is local mm*/
    }

    /*let the free node index point to the first free one in the node list*/
    pNodeBlock->nextfree = 0;

    /*update the node block's stat data*/
    pNodeBlock->maxusedsum = 0;
    pNodeBlock->curusedsum = 0;

    //nodeblock_debug("alloc_nodeblock_static_mem[3]: ", pNodeBlock);
    /*link the new node block to manager*/
    MAN_LINKNODEBLOCK_NODE_ADD_HEAD(pMan, pNodeBlock);

    //man_debug("alloc_nodeblock_static_mem[4]: ", pMan);
    //nodeblock_debug("alloc_nodeblock_static_mem[5]: ", pNodeBlock);

    MAN_FREENODEBLOCK_NODE_ADD_HEAD(pMan, pNodeBlock);

    //nodeblock_debug("alloc_nodeblock_static_mem[6]: ", pNodeBlock);
    //man_debug("alloc_nodeblock_static_mem[7]: ", pMan);


    /*update manager's stat data*/
    pMan->nodeblocknum ++;
    pMan->nodenumsum = pMan->nodenumsum + pNodeBlock->nodenum;
    //sys_log(LOGSTDOUT, "alloc_nodeblock_static_mem: type = %ld, nodenum = %ld, nodenumsum = %ld\n", type, pNodeBlock->nodenum, pMan->nodenumsum);

    return ( EC_TRUE );
}

/*note:pMan is locked by free_nodeblock_static_mem caller, so do not lock inside*/
EC_BOOL free_nodeblock_static_mem(MM_MAN *pMan, MM_NODE_BLOCK *pNodeBlock)
{
    /*remove node block from linknodeblock list of manager*/
    NODEBLOCK_LINKNODE_DEL(pNodeBlock);

    /*remove node block from freenodeblock list of manager*/
    if(EC_FALSE == NODEBLOCK_FREENODE_IS_EMPTY(pNodeBlock))
    {
        NODEBLOCK_FREENODE_DEL(pNodeBlock);
    }

    /*update the manager stat info before free node block*/
    pMan->nodenumsum = pMan->nodenumsum - pNodeBlock->nodenum;
    pMan->nodeblocknum --;

    /*free the dynamic memory used by nodes*/
    if ( NULL_PTR != pNodeBlock->pbaseaddr )
    {
        free( pNodeBlock->pbaseaddr );
        pNodeBlock->pbaseaddr = NULL_PTR;
    }
    pNodeBlock->ptailaddr = NULL_PTR;

    /*free the dynamic memory used by node index */
    if ( NULL_PTR != pNodeBlock->pnodes )
    {
        free( pNodeBlock->pnodes );
        pNodeBlock->pnodes = NULL_PTR;
        pNodeBlock->nodenum = 0;

        /*update the node block's stat data*/
        pNodeBlock->maxusedsum = 0;
        pNodeBlock->curusedsum = 0;
    }

    /*the node block has no any node entity now*/
    pNodeBlock->nextfree = NODE_LIST_TAIL;

    /*free the node block itself*/
    free(pNodeBlock);

    return (EC_TRUE);
}

/**
*
*   check the static memory type of node when allocate a node or free a node.
*   this function is for debug purpose only at present to find out mistakes in programming.
*
*   since all memory allocation and free is based on the node operation and limited number of
*   node type memory, we have a chance and ability to confirm the allocation and free operation
*   is used correctly.
*
*   note:
*       because MM_NAF is a memory block for NAF computation and we have not defined a data structer
*   for it, we cannot check its type normally. to make sure 100% it's being used correctly, we have
*   to check its calling points one by one.
*
**/
EC_BOOL check_static_mem_type(const MM_TYPE_UINT32 type, const UINT32 typesize)
{
    MM_MAN *pMan;

    if ( MM_END <= type )
    {
        sys_log(LOGSTDOUT,"error:check_static_mem_type: parameter type %ld is invalid.\n", type);
        exit ( 0 );
    }

    /* debug beg: mask NAF error report */
    if ( MM_NAF == type )
    {
        return EC_TRUE;
    }
    /* debug end : mask NAF error report */

    pMan = &(g_mem_manager[ type ]);

    switch( type )
    {
    case MM_STRCHAR128B:
        if( 128 != pMan->typesize )
        {
            return ( EC_FALSE );
        }
        break;
    case MM_STRCHAR256B:
        if( 256 != pMan->typesize )
        {
            return ( EC_FALSE );
        }
        break;
    case MM_STRCHAR512B:
        if( 512 != pMan->typesize )
        {
            return ( EC_FALSE );
        }
        break;
    case MM_STRCHAR1K:
        if( 1024 != pMan->typesize )
        {
            return ( EC_FALSE );
        }
        break;
    case MM_STRCHAR2K:
        if( 2048 != pMan->typesize )
        {
            return ( EC_FALSE );
        }
        break;
    case MM_STRCHAR4K:
        if( 4096 != pMan->typesize )
        {
            return ( EC_FALSE );
        }
        break;
    case MM_STRCHAR64K:
        if( 1024 *64 != pMan->typesize )
        {
            return ( EC_FALSE );
        }
        break;
    default:
        if( typesize != pMan->typesize )
        {
            return ( EC_FALSE );
        }
    }

    return EC_TRUE;
}

/**
*
*   fetch typesize of some kind of memory
*
**/
UINT32 fetch_static_mem_typesize(MM_TYPE_UINT32 type)
{
    MM_MAN *pMan;

    if ( MM_END <= type )
    {
        sys_log(LOGSTDOUT,"error:fetch_static_mem_typesize: parameter type is invalid.\n");
        exit ( 0 );
    }

    pMan = &(g_mem_manager[ type ]);
    return (pMan->typesize);
}

/**
*
*   allocate a node with the appointed node type for the appointed module usage.
*   if allocation success, this funciton will change its status to be used and
*   update the statistic datas of its node block and its manager
*
*   if the manager has no more this type node, the manager will allocate a new
*   dynamic memory block as a new node block of this type and link it to its node
*   block list, and then allocate a node from its node block list.
*
*   if the node status of being occupied is being found conflict, for example,
*   the node in the free node list is being used, then this function will exit
*   because it indcates the memory management has fatal defect which is desired to
*   fix at first. under such condition, this function cannot return only an error
*   code because the calling point does not provide checking ret code mechanism.
*
*   from its implementation, this function provides a mechanism of memory expansion
*   which makes the BGN package more flexible.
*
**/
UINT32 alloc_static_mem_0(const UINT32 location,
                                 const UINT32 module_type, const UINT32 module_id,
                                 const MM_TYPE_UINT32 type,void **ppvoid)
{
    MM_MAN *pMan;
    MM_NODE *pNode;
    UINT32 freenodeidx;
    MM_NODE_BLOCK *pNodeBlock;
    EC_BOOL ret;

    if ( EC_FALSE == g_mem_init_flag)
    {
        sys_log(LOGSTDOUT,"error:alloc_static_mem_0: mm is not initialized yet.\n");
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));
        exit ( 0 );
    }

    if ( MM_END <= type )
    {
        sys_log(LOGSTDOUT,"error:alloc_static_mem_0: parameter type is invalid.\n");
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));
        exit ( 0 );
    }

    pMan = &(g_mem_manager[ type ]);
    MAN_LOCK(pMan, LOC_MM_0192);

    /*if manager has no more free node, then alloc a new node block*/
    if ( pMan->curusedsum >= pMan->nodenumsum )
    {
        ret = alloc_nodeblock_static_mem(type);
        //sys_log(LOGSTDOUT, "=========================== debug beg ============================\n");
        //print_static_mem_diag_info(LOGSTDOUT);
        //sys_log(LOGSTDOUT, "=========================== debug end ============================\n");
        //sys_log(LOGSTDOUT, "==================== alloc_static_mem_0: type = %ld =================\n", type);
        //print_static_mem_status(LOGSTDOUT);

        /*if failed to alloc a new node block, then exit*/
        if ( EC_FALSE == ret )
        {
            sys_log(LOGSTDOUT,"error:alloc_static_mem_0: failed to alloc type = %ld node block.\n",type);
            sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));

            (*ppvoid) = NULL_PTR;

            MAN_UNLOCK(pMan, LOC_MM_0193);
            /*return ((UINT32)( -1 ));*/
            exit( 0 );
        }
    }

    pNodeBlock = MAN_FREENODEBLOCK_FIRST_NODE(pMan);
#if 0
    UINT32 count = 0;
    /*now the manager has at least one more free node, then find one out*/
    for ( pNodeBlock = pMan->pnodeblockhead; NULL_PTR != pNodeBlock; pNodeBlock = pNodeBlock->pnextlinknodeblock )
    {
        count ++;
        if ( pNodeBlock->nextfree < pNodeBlock->nodenum )
        {
            break;
        }
    }

    if(count >= 8)
    {
        sys_log(LOGSTDOUT, "alloc_static_mem_0: type = %ld, count = %ld\n", type, count);
    }
#endif
    /* that pNodeBlock is null is impossible, since a new node block is alloced just now.*/
    if ( NULL_PTR == pNodeBlock )
    {
        sys_log(LOGSTDOUT,"fatal error:alloc_static_mem_0: type = %ld has no free node.\n",type);
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));

        (*ppvoid) = NULL_PTR;

        MAN_UNLOCK(pMan, LOC_MM_0194);
        //print_static_mem_status(LOGSTDOUT);
        exit ( 0 );
    }

    freenodeidx = pNodeBlock->nextfree;
    pNode = &( pNodeBlock->pnodes[ freenodeidx ] );
    if ( MM_NODE_USED == pNode->usedflag)
    {
        sys_log(LOGSTDOUT,"error:alloc_static_mem_0: status conflict: type = %ld, block address = 0x%lx, the free node %ld is used.\n",
                        type,
                        (UINT32)pNodeBlock,
                        freenodeidx);
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));

        (*ppvoid) = NULL_PTR;

        MAN_UNLOCK(pMan, LOC_MM_0195);
        exit ( 0 );
    }

    pNodeBlock->nextfree = pNode->next;
#if ( SWITCH_ON == MULTI_USER_MODE_SWITCH )
    pNode->module_type = module_type;
    pNode->module_id   = module_id;
#endif/*SWITCH_ON == MULTI_USER_MODE_SWITCH*/
    pNode->usedflag    = MM_NODE_USED;

#if ( SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH )
    pNode->location = location;
#endif/*SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH*/

    *ppvoid = pNode->pmem + sizeof(MM_AUX);

    /*update the node block's stat data*/
    pNodeBlock->curusedsum ++;
    if( pNodeBlock->maxusedsum < pNodeBlock->curusedsum )
    {
        pNodeBlock->maxusedsum = pNodeBlock->curusedsum;
    }

    /*update the manager's stat data*/
    pMan->curusedsum ++;
    if ( pMan->maxusedsum < pMan->curusedsum)
    {
        pMan->maxusedsum = pMan->curusedsum;
    }

    /*if all nodes in nodeblock are used, then remove the node block from free nodeblock list of manager*/
    if(pNodeBlock->curusedsum >= pNodeBlock->nodenum)
    {
        NODEBLOCK_FREENODE_DEL(pNodeBlock);
        NODEBLOCK_FREENODE_INIT(pNodeBlock);/*important: point its next and prev to itself*/
    }

#if ( SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH )
/*
    sys_log(LOGSTDOUT,"alloc_static_mem_0: alloced node %lx type %ld for module type %ld id = %ld at %s:%d\n",
            *ppvoid,
            type,
            module_type,
            module_id,
            filename,
            lineno);
*/
#endif/*SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH*/

    MAN_UNLOCK(pMan, LOC_MM_0196);
    return ( 0 );
}

/**
*
*   free a node with the appointed type and the refered module.
*   this function can only free the node that is allocated by alloc_static_mem_0,
*   and the caller has to remember the ndoe's type.
*
*   normally, this function will return this node to the manager. what the manager does
*   is to return this node to the free node list of the node block via change the node's
*   status to being un-used. so note: this function does not free the dynamic memory of
*   this node, it is only to recycle this node and let it for next being allocated.
*   Meanwhile, this function will update the stat data of the node's manager and node block.
*
*   before free a node, this function will check whether this node's memory falls into some
*   node block of the manager and check it's occupied status is right or not.
*   if not, it means this node is invalid, and there's some possible reasons:
*       1) this node's memory is destroyed somewhere
*       2) this node's type is not correct.
*       3) this node is not allocated by alloc_static_mem_0
*
**/
UINT32 free_static_mem_0(const UINT32 location,
                                const UINT32 module_type, const UINT32 module_id,
                                const MM_TYPE_UINT32 type,void *pvoid)
{
    MM_MAN *pMan;
    MM_NODE *pNode;
    MM_AUX * pAux;
    MM_NODE_BLOCK *pNodeBlock;
    UINT32 offset;
    UINT32 node_idx;
    UINT32 res;

    if ( EC_FALSE == g_mem_init_flag)
    {
        sys_log(LOGSTDOUT,"error:free_static_mem_0: mm is not initialized yet.\n");
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));
        exit ( 0 );
    }

    if ( MM_END <= type )
    {
        sys_log(LOGSTDOUT,"error:free_static_mem_0: parameter type %ld is invalid.\n", type);
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));
        exit ( 0 );
    }

    if ( NULL_PTR == pvoid )
    {
        sys_log(LOGSTDOUT,"error:free_static_mem_0: parameter pvoid is null.\n");
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));
        exit ( 0 );
    }

    pMan = &(g_mem_manager[ type ]);
    MAN_LOCK(pMan, LOC_MM_0197);

    if ( 0 == pMan->curusedsum )
    {
        sys_log(LOGSTDOUT,"error:free_static_mem_0: the manager has no any node being used.\n");
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));
        print_static_mem_status(LOGSTDOUT);

        MAN_UNLOCK(pMan, LOC_MM_0198);
        exit ( 0 );
    }

    /* search the node's position and update relative info */
    //pAux = (MM_AUX *)(*(UINT32 *)((UINT8 *)pvoid - sizeof(MM_AUX)));
    pAux = (MM_AUX *)((UINT32)pvoid - sizeof(MM_AUX));
    pNodeBlock = pAux->u.nodeblock;

    /* if the address pMem does not belong to this manager, then report error */
    if ( NULL_PTR == pNodeBlock )
    {
        sys_log(LOGSTDOUT,"error:free_static_mem_0: pvoid = 0x%lx is out of this manager control.\n",
                        (UINT32)pvoid);
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));

        MAN_UNLOCK(pMan, LOC_MM_0199);
        exit ( 0 );
    }

    /* now the address pMem belong to the node block pNodeBlock*/
    offset = (UINT32)(( (UINT8 *) pvoid ) - sizeof(MM_AUX) - ( pNodeBlock->pbaseaddr ));

    res = offset % (pNodeBlock->typesize + sizeof(MM_AUX));
    if ( 0 != res )
    {
        sys_log(LOGSTDOUT,"error:free_static_mem_0: pvoid = 0x%lx is not an aligned address.\n",
                        (UINT32)pvoid);
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));

        MAN_UNLOCK(pMan, LOC_MM_0200);
        exit ( 0 );
    }

    node_idx = offset / (pNodeBlock->typesize + sizeof(MM_AUX));
    if ( node_idx >= pNodeBlock->nodenum )
    {
        sys_log(LOGSTDOUT,
        "error:free_static_mem_0:status error:the node with index = %ld to free of Manager %ld is out of management.\n",
                        node_idx,
                        type);
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));

        MAN_UNLOCK(pMan, LOC_MM_0201);
        exit ( 0 );
    }

    pNode = &( pNodeBlock->pnodes[ node_idx ] );
    if ( MM_NODE_NOT_USED == pNode->usedflag)
    {
        sys_log(LOGSTDOUT,"error:free_static_mem_0: status error:the node %lx to free of the Manager %ld is not used.\n",
                        pvoid, type);
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));

        MAN_UNLOCK(pMan, LOC_MM_0202);
        exit ( 0 );
    }

#if ( SWITCH_ON == MULTI_USER_MODE_SWITCH )
    if ( module_type != pNode->module_type || module_id != pNode->module_id )
    {
        sys_log(LOGSTDOUT,"error:free_static_mem_0:status error:the node with addr %lx to free of Manager %ld is not allocated by module type = %ld module id = %ld.\n",
                            pvoid,
                            type,
                            module_type,
                            module_id);
        sys_log(LOGSTDOUT,"error reported by: %s:%ld\n",MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));

        MAN_UNLOCK(pMan, LOC_MM_0203);
        exit ( 0 );
    }
#endif/*SWITCH_ON == MULTI_USER_MODE_SWITCH*/

    pNode->usedflag    = MM_NODE_NOT_USED;
#if ( SWITCH_ON == MULTI_USER_MODE_SWITCH )
    pNode->module_type = MD_END;
    pNode->module_id   = ERR_MODULE_ID;
#endif/*SWITCH_ON == MULTI_USER_MODE_SWITCH*/

#if ( SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH )
/*
    sys_log(LOGSTDOUT,"free_static_mem_0: freed node %lx type %ld of module type %ld id = %ld at %s:%d\n",
            pvoid,
            type,
            module_type,
            module_id,
            pNode->filename,
            pNode->lineno);
*/
#endif/*SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH*/

#if ( SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH )
    pNode->location = LOC_NONE_BASE;
#endif/*SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH*/

    pNode->next = pNodeBlock->nextfree;
    pNodeBlock->nextfree = node_idx;

    /*update the node block's stat data*/
    pNodeBlock->curusedsum --;

    /*update the manager's stat data*/
    pMan->curusedsum --;

    /*if nodeblock is NOT used and free nodeblock list of manager is not empty, then free the nodeblock*/
    /*otherwise, add the nodeblock to tail of free nodeblock list of manager if not in it*/
    if(EC_TRUE == NODEBLOCK_FREENODE_IS_EMPTY(pNodeBlock))
    {
        MAN_FREENODEBLOCK_NODE_ADD_TAIL(pMan, pNodeBlock);
    }

    if((0 == pNodeBlock->curusedsum) && (pNodeBlock != MAN_FREENODEBLOCK_FIRST_NODE(pMan)))
    {
        free_nodeblock_static_mem(pMan, pNodeBlock);
    }

    MAN_UNLOCK(pMan, LOC_MM_0204);
    return 0;
}

/**
*
*   memory breathing.
*   In fact, this function provides only memory shrinking,i.e., check all node blocks of all managers,
*   if any node block has no any node is being used, then free this node block, meanwhile, update the
*   nodes sum and node blocks sum of its manager. note: the stat data of the mananger is not necessary
*   to update.
*
*   memory expansion is provided by alloc_static_mem_0 function.
*
*   memory breathing function cannot be called frequently, otherwise it will lead to memory allocation flap
*   and decrease the performance of BGN package. So, its calling points are at the end of a module or free
*   module memory after entering X-ray.
*
*/
#if 0
UINT32 breathing_static_mem()
{
    MM_TYPE_UINT32  type;
    MM_MAN *pMan;
    MM_NODE_BLOCK *pNodeBlock;
    MM_NODE_BLOCK **ppNodeBlock;
    UINT32 node_num;

    /*memory breathing:*/
    /*clean up the node block if it has no node being used*/
    for ( type = 0; type < MM_END; type ++ )
    {
        /* do this manager */
        pMan = &(g_mem_manager[ type ]);

        ppNodeBlock = &(pMan->pnodeblockhead);
        while( NULL_PTR != (*ppNodeBlock) )
        {
            pNodeBlock = (*ppNodeBlock);

            if ( 0 == pNodeBlock->curusedsum)
            {
                /*if node block is not used, then remove it from the list*/
                /*remove this node block from the list*/
                *ppNodeBlock = pNodeBlock->pnextlinknodeblock;
            }
            else
            {
                /*move to the next node block*/
                ppNodeBlock = &(pNodeBlock->pnextlinknodeblock);
            }

            /*if no node is used in the node block, then free this node block*/
            if ( 0 == pNodeBlock->curusedsum)
            {
                /*store the node number info of this node block*/
                node_num = pNodeBlock->nodenum;

                /*free the dynamic memory used by nodes*/
                if ( NULL_PTR != pNodeBlock->pbaseaddr )
                {
                    free( pNodeBlock->pbaseaddr );
                    pNodeBlock->pbaseaddr = NULL_PTR;
                }

                pNodeBlock->ptailaddr = NULL_PTR;

                /*free the dynamic memory used by node index */
                if ( NULL_PTR != pNodeBlock->pnodes )
                {
                    free( pNodeBlock->pnodes );
                    pNodeBlock->pnodes = NULL_PTR;
                    pNodeBlock->nodenum = 0;

                    /*update the node block's stat data*/
                    pNodeBlock->maxusedsum = 0;
                    pNodeBlock->curusedsum = 0;
                }

                /*the node block has no any node entity now*/
                pNodeBlock->nextfree = NODE_LIST_TAIL;

                /*free the node block itself*/
                free(pNodeBlock);
                pNodeBlock = NULL_PTR;

                /*update the manager's relative data*/
                pMan->nodenumsum = pMan->nodenumsum - node_num;
                pMan->nodeblocknum --;
            }
        }
    }

    return 0;
}
#endif
UINT32 breathing_static_mem()
{
    MM_TYPE_UINT32  type;

    MM_MAN *pMan;
    MM_NODE_BLOCK *pNodeBlock;

    /*memory breathing:*/
    /*clean up the node block if it has no node being used*/
    for ( type = 0; type < MM_END; type ++ )
    {
        /* do this manager */
        pMan = &(g_mem_manager[ type ]);

        MAN_LOCK(pMan, LOC_MM_0205);
        //sys_log(LOGSTDOUT, "breathing_static_mem: type = %ld\n", type);
        //man_debug("breathing_static_mem: ", pMan);

        MAN_FREENODEBLOCK_LOOP_NEXT(pMan, pNodeBlock)
        {
            //nodeblock_debug("breathing_static_mem: ", pNodeBlock);
            //sys_log(LOGSTDOUT, "breathing_static_mem: pNodeBlock = %lx, curusedsum = %ld\n", pNodeBlock, pNodeBlock->curusedsum);

            if ( 0 == pNodeBlock->curusedsum )
            {
                pNodeBlock = NODEBLOCK_FREENODE_PREV(pNodeBlock);
                free_nodeblock_static_mem(pMan, NODEBLOCK_FREENODE_NEXT(pNodeBlock));
            }
        }

        MAN_UNLOCK(pMan, LOC_MM_0206);
    }

    return 0;
}

#if ( SWITCH_ON == MULTI_USER_MODE_SWITCH )
/**
*
*   free all nodes allocated by the appointed module, despite of the node type.
*
*   check all nodes of the node blocks of all managers one by one, if the node
*   is used by this module, then free it and update the stat data of its node block
*   and its manager.
*
*   then, this function by the way executes one time memory breathing to release
*   the free node blocks.
*
**/
UINT32 free_module_static_mem(UINT32 module_type, UINT32 module_id)
{
    MM_TYPE_UINT32  type;
    MM_MAN *pMan;
    MM_NODE *pNode;
    MM_NODE_BLOCK *pNodeBlock;
    UINT32 node_num;
    UINT32 node_idx;

    /* if no static memory is allocated before, then return success */
    if ( EC_FALSE == g_mem_init_flag )
    {
        return ( 0 );
    }

    for ( type = 0; type < MM_END; type ++ )
    {
        /* do this manager */
        pMan = &(g_mem_manager[ type ]);
        MAN_LOCK(pMan, LOC_MM_0207);

        MAN_LINKNODEBLOCK_LOOP_NEXT(pMan, pNodeBlock)
        {
            /* do this node block */
            node_num = pNodeBlock->nodenum;
            for ( node_idx = 0; node_idx < node_num; node_idx ++ )
            {
                /* do this node */
                pNode = &( pNodeBlock->pnodes[ node_idx ] );

                if ( MM_NODE_USED == pNode->usedflag
                  && module_type == pNode->module_type
                  && module_id == pNode->module_id )
                {
                    /* following codes must be consistent to free_static_mem_0()*/
                    pNode->usedflag    = MM_NODE_NOT_USED;
                    pNode->module_type = MD_END;
                    pNode->module_id   = ERR_MODULE_ID;

                    pNode->next = pNodeBlock->nextfree;
                    pNodeBlock->nextfree = node_idx;

                    /*update the node block's stat data*/
                    pNodeBlock->curusedsum --;

                    /*update the manager's stat data*/
                    pMan->curusedsum --;
                }
            }
        }

        MAN_UNLOCK(pMan, LOC_MM_0208);
    }

    /*at the end of module static memory free, do one time of memory breathing*/
    breathing_static_mem();

    return 0;
}
#endif/*SWITCH_ON == MULTI_USER_MODE_SWITCH*/

/**
*
*   destory the whole static memory occupied by the BGN package.
*
*   attention:
*       Do not call this function unless one is ready to exit the BGN package.
*
**/
UINT32 destory_static_mem()
{
    MM_TYPE_UINT32  type;
    MM_MAN *pMan;
    MM_NODE_BLOCK *pNodeBlock;

    if ( EC_FALSE == g_mem_init_flag)
    {
        return ( 0 );
    }
    g_mem_init_flag = EC_FALSE;

    for ( type = 0; type < MM_END; type ++ )
    {
        pMan = &(g_mem_manager[ type ]);
        MAN_LOCK(pMan, LOC_MM_0209);

        MAN_LINKNODEBLOCK_LOOP_NEXT(pMan, pNodeBlock)
        {
            pNodeBlock = NODEBLOCK_LINKNODE_PREV(pNodeBlock);
            free_nodeblock_static_mem(pMan, NODEBLOCK_LINKNODE_NEXT(pNodeBlock));
        }

        //pMan->nodenumsum = 0;
        //pMan->nodeblocknum = 0;
        /*validity checking*/
        if(0 < pMan->nodenumsum)
        {
            sys_log(LOGSTDERR, "error:destory_static_mem: manager %lx nodenumsum = %ld was not clean up to zero\n", pMan, pMan->nodenumsum);
        }

        if(0 < pMan->nodeblocknum)
        {
            sys_log(LOGSTDERR, "error:destory_static_mem: manager %lx nodeblocknum = %ld was not clean up to zero\n", pMan, pMan->nodeblocknum);
        }

        pMan->maxusedsum = 0;
        pMan->curusedsum = 0;

        MAN_LINKNODEBLOCK_HEAD_INIT(pMan);
        MAN_FREENODEBLOCK_HEAD_INIT(pMan);

        MAN_UNLOCK(pMan, LOC_MM_0210);
        MAN_CLEAN_LOCK(pMan, LOC_MM_0211);/*clean lock*/
    }

    return 0;
}

void *safe_malloc_0(const UINT32 size, const UINT32 location)
{
    void *pmem;
    void *pvoid;
    UINT32 len;
#if (32 == WORDSIZE)
    len = ((size + 3) & 0xFFFFFFFC);
#endif/*(32 == WORDSIZE)*/
#if (64 == WORDSIZE)
    len = ((size + 7) & 0xFFFFFFFFFFFFFF8);
#endif/*(64 == WORDSIZE)*/
    pmem = malloc(sizeof(UINT32) + len);
    if(NULL_PTR != pmem)
    {
        MM_COMM *mm_comm;
        alloc_static_mem(MD_TBD, 0, MM_COMM_NODE, &mm_comm, location);

        pvoid = (void *)((UINT32)pmem + sizeof(UINT32));

        *((UINT32 *)pmem) = (UINT32)mm_comm;
        mm_comm->pmem = pmem;

        return (pvoid);
    }
    return (NULL_PTR);
}

void safe_free_0(void *pvoid, const UINT32 location)
{
    void *pmem;
    MM_COMM *mm_comm;

    if(NULL_PTR == pvoid)
    {
        sys_log(LOGSTDOUT, "error:safe_free: try to free null pointer at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return ;
    }

    pmem = (void *)((UINT32)pvoid - sizeof(UINT32));
    mm_comm = (MM_COMM *)(*((UINT32 *)pmem));

    if(mm_comm->pmem != pmem)
    {
        sys_log(LOGSTDOUT, "warn:safe_free: found mismatched pmem pointer at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
    }

    free_static_mem(MD_TBD, 0, MM_COMM_NODE, mm_comm, location);
    free(pmem);
    return;
}

void *safe_realloc_x(void *old_pvoid, const UINT32 new_size, const UINT32 location)
{
    void *old_pmem;
    void *new_pmem;
    void *new_pvoid;

    MM_COMM *old_mm_comm;
    MM_COMM *new_mm_comm;

    old_pmem = (void *)((UINT32)old_pvoid - sizeof(UINT32));
    old_mm_comm = (MM_COMM *)(*((UINT32 *)old_pmem));

    alloc_static_mem(MD_TBD, 0, MM_COMM_NODE, &new_mm_comm, location);

    new_pmem = realloc(old_pmem, sizeof(UINT32) + new_size);
    if(new_pmem != old_pmem)
    {
        new_pvoid = (void *)((UINT32)new_pmem + sizeof(UINT32));

        *((UINT32 *)new_pmem) = (UINT32)new_mm_comm;
        new_mm_comm->pmem = new_pmem;

        free_static_mem(MD_TBD, 0, MM_COMM_NODE, old_mm_comm, location);

        return (new_pvoid);
    }

    *((UINT32 *)new_pmem) = (UINT32)new_mm_comm;
    new_mm_comm->pmem = new_pmem;

    free_static_mem(MD_TBD, 0, MM_COMM_NODE, old_mm_comm, location);

    return (old_pvoid);
}

#if 0
void *safe_realloc_0(void *old_pvoid, const UINT32 new_size, const UINT32 location)
{
    MM_AUX *old_pAux;
    MM_AUX *new_pAux;

    void *old_pmem;
    void *new_pmem;
    void *new_pvoid;

    MM_COMM *old_mm_comm;
    MM_COMM *new_mm_comm;

    print_static_mem_diag_info(LOGSTDOUT);

    old_pAux    = (MM_AUX *)((UINT32)old_pvoid - sizeof(MM_AUX));

    if(MM_END != old_pAux->type)
    {
        UINT32 old_size;

        old_size = fetch_static_mem_typesize(old_pAux->type);
        if(old_size >= new_size)
        {
            return old_pmem;
        }

        new_pvoid = safe_malloc(new_size, location);
        BCOPY(old_pvoid, new_pvoid, old_size);
        safe_free(old_pvoid, location);
        return new_pvoid;
    }

    sys_log(LOGSTDOUT, "[DEBUG]safe_realloc_0: old_pvoid = %lx\n", old_pvoid);

    old_mm_comm = old_pAux->u.mm_comm;
    old_pmem    = old_mm_comm->pmem;

    new_pmem = realloc(old_pmem, sizeof(MM_AUX) + new_size);
    if(new_pmem != old_pmem)
    {
        UINT32 old_size;

        new_pvoid = (void *)((UINT32)new_pmem + sizeof(MM_AUX));
        old_size = fetch_static_mem_typesize(old_pAux->type);
        BCOPY(old_pvoid, new_pvoid, old_size);

        alloc_static_mem(MD_TBD, 0, MM_COMM_NODE, &new_mm_comm, location);

        new_pAux            = (MM_AUX *)new_pmem;
        new_pAux->type      = MM_END;
        new_pAux->u.mm_comm = new_mm_comm;

        new_mm_comm->type   = MM_END;
        new_mm_comm->pmem   = new_pmem;

        free_static_mem(MD_TBD, 0, MM_COMM_NODE, old_mm_comm, location);

        sys_log(LOGSTDOUT, "[DEBUG]safe_realloc_0: new_pvoid = %lx\n", new_pvoid);
        return (new_pvoid);
    }
    return (old_pvoid);
}
#endif
UINT32 get_static_mem_type(const UINT32 size)
{
    UINT32 mm_type;

    for(mm_type = BUFF_MEM_DEF_BEG; mm_type <= BUFF_MEM_DEF_END; mm_type ++)
    {
        MM_MAN *pMan;

        pMan = &(g_mem_manager[ (mm_type) ]);
        if(size <= pMan->typesize)
        {
            return mm_type;
        }
    }

    return (MM_END);
}

void *safe_malloc(const UINT32 size, const UINT32 location)
{
    UINT32 mm_type;
    void *pmem;

    mm_type = get_static_mem_type(size);
    if(MM_END != mm_type)
    {
        void    *pvoid;
        alloc_static_mem(MD_TBD, 0, mm_type, &pvoid, location);
        return (pvoid);
    }

    pmem = malloc(sizeof(MM_AUX) + size);
    if(NULL_PTR != pmem)
    {
        void    *pvoid;
        MM_COMM *mm_comm;
        MM_AUX  *pAux;

        /*to record the malloced memory, we have to alloc a comm node :-(*/
        /*if not need to record it, pmem is enough for free*/
        alloc_static_mem(MD_TBD, 0, MM_COMM_NODE, &mm_comm, location);

        pvoid = (void *)((UINT32)pmem + sizeof(MM_AUX));
        pAux  = (MM_AUX *)pmem;
        pAux->type      = MM_END;
        pAux->u.mm_comm = mm_comm;

        mm_comm->type = MM_END;
        mm_comm->pmem = pmem;
        return (pvoid);
    }
    return (NULL_PTR);
}

void safe_free(void *pvoid, const UINT32 location)
{
    MM_AUX  *pAux;
    void    *pmem;
    MM_COMM *mm_comm;

    if(NULL_PTR == pvoid)
    {
        sys_log(LOGSTDOUT, "error:safe_free: try to free null pointer at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return ;
    }

    pAux = (MM_AUX *)((UINT32)pvoid - sizeof(MM_AUX));
    if(MM_END != pAux->type)
    {
        free_static_mem(MD_TBD, 0, pAux->type, pvoid, location);
        return;
    }

    pmem = (void *)pAux;

    mm_comm = pAux->u.mm_comm;
    if(mm_comm->pmem != pmem)
    {
        sys_log(LOGSTDOUT, "warn:safe_free: found mismatched pmem pointer at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
    }

    if((void *)pAux != (void *)pmem)
    {
        sys_log(LOGSTDOUT, "warn:safe_free: found mismatched pmem pointer at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
    }

    free_static_mem(MD_TBD, 0, MM_COMM_NODE, mm_comm, location);

    free(pmem);
    return;
}


void safe_copy(UINT8 *old_ptr, UINT8 *new_ptr, UINT32 len)
{
    BCOPY(old_ptr, new_ptr, len);
    return;
}

void *safe_realloc(void *old_pvoid, const UINT32 old_size, const UINT32 new_size, const UINT32 location)
{
    void *new_pvoid;

    new_pvoid = safe_malloc(new_size, location);
    //safe_copy((UINT8 *)old_pvoid, (UINT8 *)new_pvoid, old_size);
    BCOPY(old_pvoid, new_pvoid, DMIN(old_size, new_size));/*for both expand and shrink*/
    safe_free(old_pvoid, location);

    return (new_pvoid);
}

void print_static_mem_status(LOG *log)
{
    MM_TYPE_UINT32  type;

    if ( EC_TRUE == g_mem_init_flag )
    {
        sys_log(log,"g_mem_init_flag = EC_TRUE\n");
    }
    else
    {
        sys_log(log,"g_mem_init_flag = EC_FALSE\n");
        return;
    }

    for ( type = 0; type < MM_END; type ++ )
    {
        print_static_mem_status_of_type(log, type);
    }

    return ;
}

void print_static_mem_status_of_type(LOG *log, const MM_TYPE_UINT32  type)
{
    MM_MAN *pMan;

    if ( EC_FALSE == g_mem_init_flag )
    {
        return;
    }

    if(type >= MM_END)
    {
        sys_log(log, "error:print_static_mem_status_of_type: invalid type %ld\n", type);
        return ;
    }

    pMan = &(g_mem_manager[ type ]);
    //MAN_LOCK(pMan, LOC_MM_0212);

    if( 0 < pMan->nodeblocknum || 0 < pMan->nodenumsum || 0 < pMan->maxusedsum || 0 < pMan->curusedsum )
    {
        sys_log(log,
            "Manager %4ld [%s]: nodeblock num = %8ld, nodesum = %8ld, maxused = %8ld, curused = %8ld\n",
            type, pMan->name,
            pMan->nodeblocknum,
            pMan->nodenumsum,
            pMan->maxusedsum,
            pMan->curusedsum );
    }

    return ;
}

#if ( SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH )
UINT32 print_static_mem_diag_info(LOG *log)
{
    MM_TYPE_UINT32  type;

    /* if no static memory is allocated before, then return success */
    if ( EC_FALSE == g_mem_init_flag )
    {
        return ( 0 );
    }

    for ( type = 0; type < MM_END; type ++ )
    {
        print_static_mem_diag_info_of_type(log, type);
    }

    return 0;
}

UINT32 print_static_mem_diag_info_of_type(LOG *log, const MM_TYPE_UINT32 type)
{
    MM_MAN *pMan;
    MM_NODE *pNode;
    MM_NODE_BLOCK *pNodeBlock;
    UINT32 node_num;
    UINT32 node_idx;

    /* if no static memory is allocated before, then return success */
    if ( EC_FALSE == g_mem_init_flag )
    {
        return ( 0 );
    }

    if(type >= MM_END)
    {
        sys_log(log, "error:print_static_mem_diag_info_of_type: invalid type %ld\n", type);
        return ((UINT32)-1);
    }

    /* do this manager */
    pMan = &(g_mem_manager[ type ]);
    //MAN_LOCK(pMan, LOC_MM_0213);

    //sys_log(LOGSTDOUT, "print_static_mem_diag_info: type = %ld\n", type);
    //man_debug("print_static_mem_diag_info: ", pMan);

    MAN_LINKNODEBLOCK_LOOP_NEXT(pMan, pNodeBlock)
    {
        //sys_log(LOGSTDOUT, "[debug] pNodeBlock = %lx\n", pNodeBlock);
        /* do this node block */
        node_num = pNodeBlock->nodenum;
        for ( node_idx = 0; node_idx < node_num; node_idx ++ )
        {
            /* do this node */
            pNode = &( pNodeBlock->pnodes[ node_idx ] );

            if ( MM_NODE_USED == pNode->usedflag )
            {
#if ( SWITCH_ON == MULTI_USER_MODE_SWITCH )
                sys_log(log,"module type = %4ld, module id = %4ld, file name = %s, line no = %ld, addr = %lx\n",
                    pNode->module_type,
                    pNode->module_id,
                    MM_LOC_FILE_NAME(pNode->location),
                    MM_LOC_LINE_NO(pNode->location),
                    pNode->pmem + sizeof(MM_AUX));
#endif/*SWITCH_ON == MULTI_USER_MODE_SWITCH*/

#if ( SWITCH_OFF == MULTI_USER_MODE_SWITCH )
                sys_log(log,"Manager %4ld [%s]: file name = %s, line no = %ld, addr = %lx\n",
                    type, pMan->name,
                    MM_LOC_FILE_NAME(pNode->location),
                    MM_LOC_LINE_NO(pNode->location),
                    pNode->pmem + sizeof(MM_AUX));
#endif/*SWITCH_OFF == MULTI_USER_MODE_SWITCH*/
            }
        }
    }

    return 0;
}

UINT32 print_static_mem_stat_info(LOG *log)
{
    MM_TYPE_UINT32  type;

    /* if no static memory is allocated before, then return success */
    if ( EC_FALSE == g_mem_init_flag )
    {
        return ( 0 );
    }

    for ( type = 0; type < MM_END; type ++ )
    {
        print_static_mem_stat_info_of_type(log, type);
    }

    return 0;
}

typedef struct
{
    UINT32 type;
    UINT32 location;
    UINT32 count;
}LOCATION_STAT;

static void location_stat_tbl_init(LOCATION_STAT *location_stat_tbl, const UINT32 size)
{
    UINT32 pos;

    for(pos = 0; pos < size; pos ++)
    {
        LOCATION_STAT *location_stat;

        location_stat = (location_stat_tbl + pos);
        location_stat->type    = MM_END;
        location_stat->location = (UINT32)-1;
        location_stat->count    = 0;
    }
    return;
}

static void location_stat_tbl_update(LOCATION_STAT *location_stat_tbl, const UINT32 size, const UINT32 type, const UINT32 location)
{
    UINT32 pos;

    for(pos = 0; pos < size; pos ++)
    {
        LOCATION_STAT *location_stat;

        location_stat = (location_stat_tbl + pos);
        if(location_stat->type == type && location_stat->location == location)
        {
            location_stat->count ++;
            return;
        }
    }

    for(pos = 0; pos < size; pos ++)
    {
        LOCATION_STAT *location_stat;

        location_stat = (location_stat_tbl + pos);
        if(0 == location_stat->count && (UINT32)-1 == location_stat->location)
        {
            location_stat->type     = type;
            location_stat->location = location;
            location_stat->count ++;
            return;
        }
    }

    sys_log(LOGSTDOUT, "error:location_stat_tbl_update: location %ld overflow or invalid type %ld\n", location, type);
    return;
}

static void location_stat_tbl_print(LOG *log, const LOCATION_STAT *location_stat_tbl, const UINT32 size)
{
    UINT32 pos;

    for(pos = 0; pos < size; pos ++)
    {
        LOCATION_STAT *location_stat;

        location_stat = (LOCATION_STAT *)(location_stat_tbl + pos);
        if(0 == location_stat->count)
        {
            continue;
        }

        sys_log(log, "Manager %8ld: %8ld times at %s:%ld\n", location_stat->type, location_stat->count, MM_LOC_FILE_NAME(location_stat->location), MM_LOC_LINE_NO(location_stat->location));
    }
    return;
}

UINT32 print_static_mem_stat_info_of_type(LOG *log, const MM_TYPE_UINT32 type)
{
    MM_MAN *pMan;
    MM_NODE *pNode;
    MM_NODE_BLOCK *pNodeBlock;
    UINT32 node_num;
    UINT32 node_idx;

    LOCATION_STAT  location_stat_tbl[1024];
    UINT32         location_stat_tbl_size;

    /* if no static memory is allocated before, then return success */
    if ( EC_FALSE == g_mem_init_flag )
    {
        return ( 0 );
    }

    if(type >= MM_END)
    {
        sys_log(log, "error:print_static_mem_stat_info_of_type: invalid type %ld\n", type);
        return ((UINT32)-1);
    }

    location_stat_tbl_size = sizeof(location_stat_tbl)/sizeof(location_stat_tbl[0]);
    location_stat_tbl_init(location_stat_tbl, location_stat_tbl_size);

    /* do this manager */
    pMan = &(g_mem_manager[ type ]);

    MAN_LINKNODEBLOCK_LOOP_NEXT(pMan, pNodeBlock)
    {
        //sys_log(LOGSTDOUT, "[debug] pNodeBlock = %lx\n", pNodeBlock);
        /* do this node block */
        node_num = pNodeBlock->nodenum;
        for ( node_idx = 0; node_idx < node_num; node_idx ++ )
        {
            /* do this node */
            pNode = &( pNodeBlock->pnodes[ node_idx ] );

            if ( MM_NODE_USED == pNode->usedflag )
            {
                location_stat_tbl_update(location_stat_tbl, location_stat_tbl_size, type, pNode->location);
            }
        }
    }

    location_stat_tbl_print(log, location_stat_tbl, location_stat_tbl_size);

    return 0;
}

#endif/*SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH*/

UINT32 mm_man_occupy_node_init_0(const UINT32 md_id, MM_MAN_OCCUPY_NODE *mm_man_occupy_node)
{
    MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node) = MM_END;
    MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node)  = 0;
    MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node)  = 0;
    MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node)  = 0;

    return (0);
}


UINT32 mm_man_occupy_node_clean_0(const UINT32 md_id, MM_MAN_OCCUPY_NODE *mm_man_occupy_node)
{
    MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node) = MM_END;
    MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node)  = 0;
    MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node)  = 0;
    MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node)  = 0;

    return (0);
}

UINT32 mm_man_occupy_node_free_0(const UINT32 md_id, MM_MAN_OCCUPY_NODE *mm_man_occupy_node)
{
    free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MM_MAN_OCCUPY_NODE, mm_man_occupy_node, LOC_MM_0214);
    return (0);
}

UINT32 mm_man_occupy_node_init(MM_MAN_OCCUPY_NODE *mm_man_occupy_node)
{
    MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node) = MM_END;
    MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node)  = 0;
    MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node)  = 0;
    MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node)  = 0;

    return (0);
}

UINT32 mm_man_occupy_node_free(MM_MAN_OCCUPY_NODE *mm_man_occupy_node)
{
    free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MM_MAN_OCCUPY_NODE, mm_man_occupy_node, LOC_MM_0215);
    return (0);
}

UINT32 mm_man_occupy_node_clone(MM_MAN_OCCUPY_NODE *mm_man_occupy_node_src, MM_MAN_OCCUPY_NODE *mm_man_occupy_node_des)
{
    MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node_des) = MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node_src);
    MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node_des)  = MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node_src);
    MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node_des)  = MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node_src);
    MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node_des)  = MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node_src);

    return (0);
}

EC_BOOL mm_man_occupy_node_was_used(const MM_TYPE_UINT32 type)
{
    MM_MAN *pMan;

    if(type >= MM_END)
    {
        sys_log(LOGSTDOUT, "error:mm_man_occupy_node_was_used: invalid type %ld\n", type);
        return (EC_FALSE);
    }

    pMan = &(g_mem_manager[ type ]);

    if( 0 < pMan->nodeblocknum || 0 < pMan->nodenumsum || 0 < pMan->maxusedsum || 0 < pMan->curusedsum )
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

UINT32 mm_man_occupy_node_fetch(const MM_TYPE_UINT32 type, MM_MAN_OCCUPY_NODE *mm_man_occupy_node)
{
    MM_MAN *pMan;

    if(type >= MM_END)
    {
        sys_log(LOGSTDOUT, "error:mm_man_occupy_node_fetch: invalid type %ld\n", type);
        return ((UINT32)-1);
    }

    pMan = &(g_mem_manager[ type ]);

    MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node) = pMan->type;
    MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node)  = pMan->nodenumsum;
    MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node)  = pMan->maxusedsum;
    MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node)  = pMan->curusedsum;

    return (0);
}

UINT32 mm_man_load_node_init_0(const UINT32 md_id, MM_MAN_LOAD_NODE *mm_man_load_node)
{
    MM_MAN_LOAD_NODE_TYPE(mm_man_load_node) = MM_END;
    MM_MAN_LOAD_NODE_MAX(mm_man_load_node)  = 0.0;
    MM_MAN_LOAD_NODE_CUR(mm_man_load_node)  = 0.0;

    return (0);
}


UINT32 mm_man_load_node_clean_0(const UINT32 md_id, MM_MAN_LOAD_NODE *mm_man_load_node)
{
    MM_MAN_LOAD_NODE_TYPE(mm_man_load_node) = MM_END;
    MM_MAN_LOAD_NODE_MAX(mm_man_load_node)  = 0.0;
    MM_MAN_LOAD_NODE_CUR(mm_man_load_node)  = 0.0;

    return (0);
}

UINT32 mm_man_load_node_free_0(const UINT32 md_id, MM_MAN_LOAD_NODE *mm_man_load_node)
{
    free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MM_MAN_LOAD_NODE, mm_man_load_node, LOC_MM_0216);
    return (0);
}

UINT32 mm_man_load_node_init(MM_MAN_LOAD_NODE *mm_man_load_node)
{
    MM_MAN_LOAD_NODE_TYPE(mm_man_load_node) = MM_END;
    MM_MAN_LOAD_NODE_MAX(mm_man_load_node)  = 0.0;
    MM_MAN_LOAD_NODE_CUR(mm_man_load_node)  = 0.0;

    return (0);
}

UINT32 mm_man_load_node_free(MM_MAN_LOAD_NODE *mm_man_load_node)
{
    free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MM_MAN_LOAD_NODE, mm_man_load_node, LOC_MM_0217);
    return (0);
}

UINT32 mm_man_load_node_clone(MM_MAN_LOAD_NODE *mm_man_load_node_src, MM_MAN_LOAD_NODE *mm_man_load_node_des)
{
    MM_MAN_LOAD_NODE_TYPE(mm_man_load_node_des) = MM_MAN_LOAD_NODE_TYPE(mm_man_load_node_src);
    MM_MAN_LOAD_NODE_MAX(mm_man_load_node_des)  = MM_MAN_LOAD_NODE_MAX(mm_man_load_node_src);
    MM_MAN_LOAD_NODE_CUR(mm_man_load_node_des)  = MM_MAN_LOAD_NODE_CUR(mm_man_load_node_src);

    return (0);
}

EC_BOOL mm_man_load_node_was_used(const MM_TYPE_UINT32 type)
{
    MM_MAN *pMan;

    if(type >= MM_END)
    {
        sys_log(LOGSTDOUT, "error:mm_man_load_node_was_used: invalid type %ld\n", type);
        return (EC_FALSE);
    }

    pMan = &(g_mem_manager[ type ]);

    if( 0 < pMan->nodeblocknum || 0 < pMan->nodenumsum || 0 < pMan->maxusedsum || 0 < pMan->curusedsum )
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

UINT32 mm_man_load_node_fetch(const MM_TYPE_UINT32 type, MM_MAN_LOAD_NODE *mm_man_load_node)
{
    MM_MAN *pMan;

    if(type >= MM_END)
    {
        sys_log(LOGSTDOUT, "error:mm_man_load_node_fetch: invalid type %ld\n", type);
        return ((UINT32)-1);
    }

    pMan = &(g_mem_manager[ type ]);

    MM_MAN_LOAD_NODE_TYPE(mm_man_load_node) = pMan->type;
    MM_MAN_LOAD_NODE_MAX(mm_man_load_node)  = (100.0 * pMan->maxusedsum) / pMan->nodenumsum;
    MM_MAN_LOAD_NODE_CUR(mm_man_load_node)  = (100.0 * pMan->curusedsum) / pMan->nodenumsum;

    return (0);
}



#ifdef __cplusplus
}
#endif/*__cplusplus*/

