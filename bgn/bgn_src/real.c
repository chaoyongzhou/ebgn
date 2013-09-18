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

#include "type.h"
#include "mm.h"
#include "log.h"

#include "cmpic.inc"

UINT32 real_init(const UINT32 real_md_id, REAL *real)
{
    (*real) = 0.0;
    return (0);
}

UINT32 real_clean(const UINT32 real_md_id, REAL *real)
{
    (*real) = 0.0;
    return (0);
}

UINT32 real_free(const UINT32 real_md_id, REAL *real)
{
    free_static_mem(MD_TBD, real_md_id, MM_REAL, real, LOC_REAL_0001);
    return (0);
}

REAL * real_new(const UINT32 real_md_id)
{
    REAL *real;
    alloc_static_mem(MD_TBD, real_md_id, MM_REAL, &real, LOC_REAL_0002);

    (*real) = 0.0;
    return (real);
}

UINT32 real_init_0(REAL *real)
{
    (*real) = 0.0;
    return (0);
}

UINT32 real_clean_0(REAL *real)
{
    (*real) = 0.0;
    return (0);
}

UINT32 real_free_0(REAL *real)
{
    free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_REAL, real, LOC_REAL_0003);
    return (0);
}

void real_print(LOG *log, const REAL *real)
{
    sys_log(log, "%.2f\n", (*real));
    return;
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

