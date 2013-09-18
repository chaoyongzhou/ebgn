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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_typeconst.h"
#include "lib_type.h"
#include "lib_mm.h"
#include "lib_char2int.h"
#include "lib_task.h"
#include "lib_mod.h"
#include "lib_log.h"
#include "lib_debug.h"
#include "lib_rank.h"

#include "lib_cmpic.inc"

#include "lib_cstring.h"
#include "lib_cvector.h"

#include "lib_findex.inc"

#include "lib_super.h"

#define __ICT_OBJ_ZONE_SIZE     ((UINT32) 100)
#define __ICT_OBJ_ZONE_NUM      ((UINT32)   2)

#define __TCID_TO_ZONE_ID_MASK                         ((UINT32)0xFF)

#define __OBJ_DATA_BIT_OFFSET                          ((UINT32)(WORDSIZE / 2))

#define __GET_ZONE_ID_FROM_TCID(tcid)                  (((tcid) & __TCID_TO_ZONE_ID_MASK) - 1)

#define __MAKE_OBJ_ID(zone_id, zone_size, obj_idx)     ((zone_id) * (zone_size) + (obj_idx))

#define __MAKE_OBJ_DATA(obj_id, data_idx)              (((data_idx) << __OBJ_DATA_BIT_OFFSET) | (obj_id))

#define __MAKE_DES_TCID(tcid, zone_id)                 (((tcid) & (~__TCID_TO_ZONE_ID_MASK)) | ((zone_id) + 1))

#define __GET_ZONE_ID_FROM_OBJ_ID(obj_id, zone_size)   ((obj_id) / (zone_size))

#define __GET_OBJ_IDX_FROM_OBJ_ID(obj_id, zone_size)   ((obj_id) % (zone_size))

EC_BOOL __test_ict_data_supplier()
{
    UINT32 super_md_id;
    UINT32 obj_zone_size;

    super_md_id   = 0;
    obj_zone_size = __ICT_OBJ_ZONE_SIZE;

    super_set_zone_size(super_md_id, obj_zone_size);
    super_load_data(super_md_id);
    return (EC_TRUE);
}

EC_BOOL __test_ict_data_consumer_1()
{
    UINT32 super_md_id;
    UINT32 obj_zone_num;
    UINT32 obj_zone_size;
    UINT32 obj_num;
    UINT32 obj_id;
    void * obj_vec;
    
    super_md_id   = 0;
    obj_zone_size = __ICT_OBJ_ZONE_SIZE;
    obj_zone_num  = __ICT_OBJ_ZONE_NUM;
    obj_num       = obj_zone_num * obj_zone_size; 

    super_set_zone_size(super_md_id, obj_zone_size);

    //super_print_data_all(super_md_id, obj_zone_num, LOGSTDOUT);

    ASSERT(obj_vec = cvector_new(super_md_id, MM_UINT32, 0));

    for(obj_id = 0; obj_id < obj_num; obj_id ++)
    {
        ASSERT(EC_TRUE == super_get_data(super_md_id, obj_id, obj_vec));
        sys_log(LOGCONSOLE, "__test_ict_data_consumer_1: get data from zone %lx: obj_id = %lx ==> ", 
                            __GET_ZONE_ID_FROM_OBJ_ID(obj_id, __ICT_OBJ_ZONE_SIZE), obj_id);
        super_print_obj_vec(super_md_id, obj_vec, LOGCONSOLE);
        cvector_clean(obj_vec, NULL_PTR, 0);
    }
    cvector_free(obj_vec, 0);
    sys_log(LOGCONSOLE, "__test_ict_data_consumer_1: done\n");
    
    return (EC_TRUE);
}

EC_BOOL __test_ict_data_consumer_2()
{
    UINT32 super_md_id;
    UINT32 obj_zone_num;
    UINT32 obj_zone_size;
    UINT32 obj_num;
    UINT32 obj_count;
    UINT32 obj_loop;
    void * obj_vec;
    
    super_md_id   = 0;
    obj_zone_size = __ICT_OBJ_ZONE_SIZE;
    obj_zone_num  = __ICT_OBJ_ZONE_NUM;
    obj_num       = obj_zone_num * obj_zone_size; 
    obj_count     = 1000;

    super_set_zone_size(super_md_id, obj_zone_size);

    //super_print_data_all(super_md_id, obj_zone_num, LOGSTDOUT);

    ASSERT(obj_vec = cvector_new(super_md_id, MM_UINT32, 0));

    for(obj_loop = 0; obj_loop < obj_count; obj_loop ++)
    {
        UINT32 obj_id;

        obj_id = (random() % obj_num);
        ASSERT(EC_TRUE == super_get_data(super_md_id, obj_id, obj_vec));
        //sys_log(LOGCONSOLE, "loop %8ld, obj_id = %8lx ==> ", obj_loop, obj_id);
        sys_log(LOGCONSOLE, "__test_ict_data_consumer_2: loop %8ld, get data from zone %lx: obj_id = %lx ==> ", 
                             obj_loop,
                            __GET_ZONE_ID_FROM_OBJ_ID(obj_id, __ICT_OBJ_ZONE_SIZE), obj_id);
        
        super_print_obj_vec(super_md_id, obj_vec, LOGCONSOLE);
        cvector_clean(obj_vec, NULL_PTR, 0);
    }
    cvector_free(obj_vec, 0);
    sys_log(LOGCONSOLE, "done\n");
    
    return (EC_TRUE);
}

int main_ict(int argc, char **argv)
{
    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_ict: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }
    
    /*define specific runner for each (tcid, rank)*/
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.1") , CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.2") , CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.3") , CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.4") , CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.5") , CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.6") , CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.7") , CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.8") , CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.9") , CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.10"), CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.11"), CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.12"), CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.13"), CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.14"), CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.15"), CMPI_FWD_RANK, __test_ict_data_supplier);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.16"), CMPI_FWD_RANK, __test_ict_data_supplier);

    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.20"), CMPI_FWD_RANK, __test_ict_data_consumer_1);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.21"), CMPI_FWD_RANK, __test_ict_data_consumer_2);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.22"), CMPI_FWD_RANK, __test_ict_data_consumer_2);
    task_brd_default_add_runner(ipv4_to_uint32("10.10.10.23"), CMPI_FWD_RANK, __test_ict_data_consumer_2);

    /*start the defined runner on current (tcid, rank)*/
    task_brd_default_start_runner();

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

