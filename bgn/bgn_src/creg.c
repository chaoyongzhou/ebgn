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

#include "type.h"
#include "mm.h"
#include "log.h"

#include "creg.h"

#include "bgnf2n.h"
#include "bgnfp.h"
#include "bgnz2.h"
#include "bgnz.h"
#include "bgnzn.h"
#include "conv.h"
#include "debug.h"
#include "ebgnz.h"
#include "eccf2n.h"
#include "eccfp.h"
#include "ecf2n.h"
#include "ecfp.h"

#include "polyf2n.h"
#include "polyfp.h"
#include "poly.h"
#include "polyz2.h"
#include "polyz.h"
#include "polyzn.h"
#include "seafp.h"
#include "vectorr.h"
#include "matrixr.h"
#include "dmatrixr.h"
#include "vmatrixr.h"

#include "tbd.h"
#include "crun.h"
#include "super.h"
#include "vmm.h"

#include "cmpie.h"
#include "cstring.h"

#include "kbuff.h"
#include "cfile.h"
#include "cdir.h"
#include "cdfs.h"
#include "cdfsnp.h"
#include "cdfsdn.h"
#include "cmon.h"
#include "csocket.h"
#include "real.h"
#include "cload.h"
#include "cfuse.h"
#include "csolr.h"
#include "cbgt.h"
#include "ganglia.h"
#include "csession.h"
#include "cscore.h"

#include "findex.inc"

#include "bgnf2n.inc"
#include "bgnfp.inc"
#include "bgnz2.inc"
#include "bgnz.inc"
#include "bgnzn.inc"
#include "conv.inc"
#include "ebgnz.inc"
#include "eccf2n.inc"
#include "eccfp.inc"
#include "ecf2n.inc"
#include "ecfp.inc"
#include "polyf2n.inc"
#include "polyfp.inc"
#include "poly.inc"
#include "polyz2.inc"
#include "polyz.inc"
#include "polyzn.inc"
#include "seafp.inc"
#include "vectorr.inc"
#include "matrixr.inc"
#include "dmatrixr.inc"
#include "vmatrixr.inc"

#include "tbd.inc"
#include "crun.inc"
#include "super.inc"
#include "vmm.inc"

#include "cfile.inc"
#include "cdir.inc"
#include "cdfs.inc"
#include "csolr.inc"
#include "cmon.inc"
#include "cbgt.inc"
#include "ganglia.inc"
#include "csession.inc"
#include "cscore.inc"

#include "task.inc"

TYPE_CONV_ITEM *creg_type_conv_item_new()
{
    TYPE_CONV_ITEM *type_conv_item;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_TYPE_CONV_ITEM, &type_conv_item, LOC_CREG_0001);
    if(NULL_PTR == type_conv_item)
    {
        sys_log(LOGSTDOUT, "error:creg_type_conv_item_new: new type conv item failed\n");
        return (NULL_PTR);
    }
    creg_type_conv_item_init(type_conv_item);
    return (type_conv_item);
}

EC_BOOL creg_type_conv_item_init(TYPE_CONV_ITEM *type_conv_item)
{
    TYPE_CONV_ITEM_VAR_DBG_TYPE(type_conv_item)     = e_dbg_type_end;
    TYPE_CONV_ITEM_VAR_SIZEOF(type_conv_item)       = 0;
    TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) = EC_FALSE;
    TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item)      = MM_END;
    TYPE_CONV_ITEM_VAR_INIT_FUNC(type_conv_item)    = 0;
    TYPE_CONV_ITEM_VAR_CLEAN_FUNC(type_conv_item)   = 0;
    TYPE_CONV_ITEM_VAR_FREE_FUNC(type_conv_item)    = 0;
    TYPE_CONV_ITEM_VAR_ENCODE_FUNC(type_conv_item)  = 0;
    TYPE_CONV_ITEM_VAR_DECODE_FUNC(type_conv_item)  = 0;
    TYPE_CONV_ITEM_VAR_ENCODE_SIZE(type_conv_item)  = 0;

    return (EC_TRUE);
}

EC_BOOL creg_type_conv_item_clean(TYPE_CONV_ITEM *type_conv_item)
{
    TYPE_CONV_ITEM_VAR_DBG_TYPE(type_conv_item)     = e_dbg_type_end;
    TYPE_CONV_ITEM_VAR_SIZEOF(type_conv_item)       = 0;
    TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) = EC_FALSE;
    TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item)      = MM_END;
    TYPE_CONV_ITEM_VAR_INIT_FUNC(type_conv_item)    = 0;
    TYPE_CONV_ITEM_VAR_CLEAN_FUNC(type_conv_item)   = 0;
    TYPE_CONV_ITEM_VAR_FREE_FUNC(type_conv_item)    = 0;
    TYPE_CONV_ITEM_VAR_ENCODE_FUNC(type_conv_item)  = 0;
    TYPE_CONV_ITEM_VAR_DECODE_FUNC(type_conv_item)  = 0;
    TYPE_CONV_ITEM_VAR_ENCODE_SIZE(type_conv_item)  = 0;
    return (EC_TRUE);
}

EC_BOOL creg_type_conv_item_free(TYPE_CONV_ITEM *type_conv_item)
{
    if(NULL_PTR != type_conv_item)
    {
        creg_type_conv_item_clean(type_conv_item);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_TYPE_CONV_ITEM, type_conv_item, LOC_CREG_0002);
    }
    return (EC_TRUE);
}

CVECTOR *creg_type_conv_vec_fetch()
{
    TASK_BRD *task_brd;
    task_brd = task_brd_default_get();
    if(NULL_PTR == task_brd)
    {
        sys_log(LOGSTDOUT, "error:creg_type_conv_vec_fetch: task_brd not init\n");
        return (NULL_PTR);
    }
    return TASK_BRD_TYPE_CONV_VEC(task_brd);
}

EC_BOOL creg_type_conv_vec_init(CVECTOR *type_conv_vec)
{
    cvector_init(type_conv_vec, CREG_TYPE_CONV_ITEM_DEFAULT_NUM, MM_TYPE_CONV_ITEM, CVECTOR_LOCK_ENABLE, LOC_CREG_0003);
    return (EC_TRUE);
}

EC_BOOL creg_type_conv_vec_clean(CVECTOR *type_conv_vec)
{
    cvector_clean(type_conv_vec, (CVECTOR_DATA_CLEANER)creg_type_conv_item_free, LOC_CREG_0004);
    return (EC_TRUE);
}

TYPE_CONV_ITEM *creg_type_conv_vec_get(CVECTOR *type_conv_vec, const UINT32 var_dbg_type)
{
    return (TYPE_CONV_ITEM *)cvector_get(type_conv_vec, var_dbg_type);
}

EC_BOOL creg_type_conv_vec_add(CVECTOR *type_conv_vec,
                                         const UINT32 var_dbg_type, const UINT32 var_sizeof, const UINT32 var_pointer_flag, const UINT32 var_mm_type,
                                         const UINT32 var_init_func, const UINT32 var_clean_func, const UINT32 var_free_func,
                                         const UINT32 var_encode_func, const UINT32 var_decode_func, const UINT32 var_encode_size
                                         )
{
    TYPE_CONV_ITEM *type_conv_item;
    UINT32 pos;

    if(NULL_PTR != cvector_get(type_conv_vec, var_dbg_type))
    {
        sys_log(LOGSTDOUT, "error:creg_type_conv_vec_add: type conv item for var_dbg_type %ld was already defined\n", var_dbg_type);
        return (EC_FALSE);
    }

    for(pos = cvector_size(type_conv_vec); pos <= var_dbg_type; pos ++)
    {
        cvector_push(type_conv_vec, NULL_PTR);
    }

    type_conv_item = creg_type_conv_item_new();
    if(NULL_PTR == type_conv_item)
    {
        sys_log(LOGSTDOUT, "error:creg_type_conv_vec_add: new type conv item failed\n");
        return (EC_FALSE);
    }

    TYPE_CONV_ITEM_VAR_DBG_TYPE(type_conv_item)     = var_dbg_type;
    TYPE_CONV_ITEM_VAR_SIZEOF(type_conv_item)       = var_sizeof;
    TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) = var_pointer_flag;
    TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item)      = var_mm_type;
    TYPE_CONV_ITEM_VAR_INIT_FUNC(type_conv_item)    = var_init_func;
    TYPE_CONV_ITEM_VAR_CLEAN_FUNC(type_conv_item)   = var_clean_func;
    TYPE_CONV_ITEM_VAR_FREE_FUNC(type_conv_item)    = var_free_func;
    TYPE_CONV_ITEM_VAR_ENCODE_FUNC(type_conv_item)  = var_encode_func;
    TYPE_CONV_ITEM_VAR_DECODE_FUNC(type_conv_item)  = var_decode_func;
    TYPE_CONV_ITEM_VAR_ENCODE_SIZE(type_conv_item)  = var_encode_size;

    cvector_set(type_conv_vec, var_dbg_type, (void *)type_conv_item);
    return (EC_TRUE);
}

EC_BOOL creg_type_conv_vec_add_default(CVECTOR *type_conv_vec)
{
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_UINT32,
        /* type_sizeof            */sizeof(UINT32),
        /* pointer_flag           */EC_FALSE,
        /* var_mm_type            */MM_UINT32,
        /* init_type_func         */0,/*(UINT32)dbg_init_uint32_ptr*/
        /* clean_type_func        */0,/*(UINT32)dbg_clean_uint32_ptr*/
        /* free_type_func         */0,/*(UINT32)dbg_free_uint32_ptr*/
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_uint32,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_uint32,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_uint32_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_UINT16,
        /* type_sizeof            */sizeof(UINT16),
        /* pointer_flag           */EC_FALSE,
        /* var_mm_type            */MM_UINT16,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_uint16,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_uint16,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_uint16_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_UINT8,
        /* type_sizeof            */sizeof(UINT8),
        /* pointer_flag           */EC_FALSE,
        /* var_mm_type            */MM_UINT8,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_uint8,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_uint8,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_uint8_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_void,
        /* type_sizeof            */sizeof(UINT32),
        /* pointer_flag           */EC_FALSE,
        /* var_mm_type            */MM_UINT32,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_uint32,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_uint32,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_uint32_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_EC_BOOL,
        /* type_sizeof            */sizeof(EC_BOOL),
        /* pointer_flag           */EC_FALSE,
        /* var_mm_type            */MM_UINT32,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_uint32,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_uint32,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_uint32_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_BIGINT_ptr,
        /* type_sizeof            */sizeof(BIGINT *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_BIGINT,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_bgn,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_bgn,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_bgn_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_EBGN_ptr,
        /* type_sizeof            */sizeof(EBGN *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_EBGN,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_ebgn,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_ebgn,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_ebgn_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_EC_CURVE_POINT_ptr,
        /* type_sizeof            */sizeof(EC_CURVE_POINT *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CURVE_POINT,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_ec_curve_point,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_ec_curve_point,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_ec_curve_point_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_EC_CURVE_AFF_POINT_ptr,
        /* type_sizeof            */sizeof(EC_CURVE_AFF_POINT *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CURVE_AFFINE_POINT,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_ec_curve_aff_point,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_ec_curve_aff_point,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_ec_curve_aff_point_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_ECF2N_CURVE_ptr,
        /* type_sizeof            */sizeof(ECF2N_CURVE *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_ECF2N_CURVE,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_ecf2n_curve,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_ecf2n_curve,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_ecf2n_curve_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_ECFP_CURVE_ptr,
        /* type_sizeof            */sizeof(ECFP_CURVE *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_ECFP_CURVE,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_ecfp_curve,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_ecfp_curve,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_ecfp_curve_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_ECC_KEYPAIR_ptr,
        /* type_sizeof            */sizeof(ECC_KEYPAIR *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_KEY_PAIR,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_ecc_keypair,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_ecc_keypair,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_ecc_keypair_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_ECC_SIGNATURE_ptr,
        /* type_sizeof            */sizeof(ECC_SIGNATURE *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_END,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_ecc_signature,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_ecc_signature,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_ecc_signature_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_POLY_ptr,
        /* type_sizeof            */sizeof(POLY *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_POLY,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_poly,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_poly,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_poly_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_POLY_ITEM_ptr,
        /* type_sizeof            */sizeof(POLY_ITEM *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_POLY_ITEM,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_poly_item,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_poly_item,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_poly_item_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_REAL_ptr,
        /* type_sizeof            */sizeof(REAL *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_REAL,
        /* init_type_func         */(UINT32)real_init,
        /* clean_type_func        */(UINT32)real_clean,
        /* free_type_func         */(UINT32)real_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_real,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_real,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_real_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_VECTORR_ptr,
        /* type_sizeof            */sizeof(VECTOR *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_VECTOR,
        /* init_type_func         */(UINT32)vector_r_init_vector,
        /* clean_type_func        */(UINT32)vector_r_clean_vector,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_vectorr,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_vectorr,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_vectorr_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_VECTORR_BLOCK_ptr,
        /* type_sizeof            */sizeof(VECTOR_BLOCK *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_VECTOR_BLOCK,
        /* init_type_func         */(UINT32)vector_r_init_block,
        /* clean_type_func        */(UINT32)vector_r_clean_block,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_vectorr_block,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_vectorr_block,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_vectorr_block_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_MATRIXR_ptr,
        /* type_sizeof            */sizeof(MATRIX *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_MATRIX,
        /* init_type_func         */(UINT32)matrix_r_init_matrix,
        /* clean_type_func        */(UINT32)matrix_r_clean_matrix,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_matrixr,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_matrixr,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_matrixr_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_MATRIXR_BLOCK_ptr,
        /* type_sizeof            */sizeof(MATRIX_BLOCK *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_MATRIX_BLOCK,
        /* init_type_func         */(UINT32)matrix_r_init_block,
        /* clean_type_func        */(UINT32)matrix_r_clean_block,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_matrixr_block,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_matrixr_block,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_matrixr_block_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_MOD_MGR_ptr,
        /* type_sizeof            */sizeof(MOD_MGR *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_MOD_MGR,
        /* init_type_func         */(UINT32)mod_mgr_init_0,
        /* clean_type_func        */(UINT32)mod_mgr_clean_0,
        /* free_type_func         */(UINT32)mod_mgr_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_mod_mgr,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_mod_mgr,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_mod_mgr_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSTRING_ptr,
        /* type_sizeof            */sizeof(CSTRING *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSTRING,
        /* init_type_func         */(UINT32)cstring_init_0,
        /* clean_type_func        */(UINT32)cstring_clean_0,
        /* free_type_func         */(UINT32)cstring_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cstring,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cstring,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cstring_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_TASKC_MGR_ptr,
        /* type_sizeof            */sizeof(TASKC_MGR *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_TASKC_MGR,
        /* init_type_func         */(UINT32)super_init_taskc_mgr,
        /* clean_type_func        */(UINT32)super_clean_taskc_mgr,
        /* free_type_func         */(UINT32)super_free_taskc_mgr,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_taskc_mgr,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_taskc_mgr,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_taskc_mgr_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_VMM_NODE_ptr,
        /* type_sizeof            */sizeof(VMM_NODE *),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_VMM_NODE,
        /* init_type_func         */(UINT32)vmm_init_node,
        /* clean_type_func        */(UINT32)vmm_clean_node,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_vmm_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_vmm_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_vmm_node_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_UINT32_ptr,
        /* type_sizeof            */sizeof(UINT32),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_UINT32,
        /* init_type_func         */(UINT32)dbg_init_uint32_ptr,
        /* clean_type_func        */(UINT32)dbg_clean_uint32_ptr,
        /* free_type_func         */(UINT32)dbg_free_uint32_ptr,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_uint32_ptr,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_uint32,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_uint32_ptr_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_LOG_ptr,
        /* type_sizeof            */sizeof(LOG),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_LOG,
        /* init_type_func         */(UINT32)log_init_0,
        /* clean_type_func        */(UINT32)log_clean_0,
        /* free_type_func         */(UINT32)log_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_log,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_log,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_log_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CFILE_SEG_ptr,
        /* type_sizeof            */sizeof(CFILE_SEG),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CFILE_SEG,
        /* init_type_func         */(UINT32)cfile_seg_init,
        /* clean_type_func        */(UINT32)cfile_seg_clean,
        /* free_type_func         */(UINT32)cfile_seg_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cfile_seg,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cfile_seg,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cfile_seg_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CVECTOR_ptr,
        /* type_sizeof            */sizeof(CVECTOR),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CVECTOR,
        /* init_type_func         */(UINT32)cvector_init_0,
        /* clean_type_func        */(UINT32)cvector_clean_0,
        /* free_type_func         */(UINT32)cvector_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cvector,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cvector,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cvector_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CFILE_NODE_ptr,
        /* type_sizeof            */sizeof(CFILE_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CFILE_NODE,
        /* init_type_func         */(UINT32)cfile_node_init,
        /* clean_type_func        */(UINT32)cfile_node_clean,
        /* free_type_func         */(UINT32)cfile_node_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cfile_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cfile_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cfile_node_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_KBUFF_ptr,
        /* type_sizeof            */sizeof(KBUFF),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_KBUFF,
        /* init_type_func         */(UINT32)kbuff_init_0,
        /* clean_type_func        */(UINT32)kbuff_clean_0,
        /* free_type_func         */(UINT32)kbuff_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_kbuff,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_kbuff,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_kbuff_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CDIR_SEG_ptr,
        /* type_sizeof            */sizeof(CDIR_SEG),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CDIR_SEG,
        /* init_type_func         */(UINT32)cdir_seg_init,
        /* clean_type_func        */(UINT32)cdir_seg_clean,
        /* free_type_func         */(UINT32)cdir_seg_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cdir_seg,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cdir_seg,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cdir_seg_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CDIR_NODE_ptr,
        /* type_sizeof            */sizeof(CDIR_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CDIR_NODE,
        /* init_type_func         */(UINT32)cdir_node_init,
        /* clean_type_func        */(UINT32)cdir_node_clean,
        /* free_type_func         */(UINT32)cdir_node_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cdir_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cdir_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cdir_node_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CMON_OBJ_ptr,
        /* type_sizeof            */sizeof(CMON_OBJ),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CMON_OBJ,
        /* init_type_func         */(UINT32)cmon_obj_init_undef,
        /* clean_type_func        */(UINT32)cmon_obj_clean_0,
        /* free_type_func         */(UINT32)cmon_obj_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cmon_obj,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cmon_obj,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cmon_obj_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CMON_OBJ_VEC_ptr,
        /* type_sizeof            */sizeof(CMON_OBJ_VEC),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CMON_OBJ_VEC,
        /* init_type_func         */(UINT32)cmon_obj_vec_init,
        /* clean_type_func        */(UINT32)cmon_obj_vec_clean,
        /* free_type_func         */(UINT32)cmon_obj_vec_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cmon_obj_vec,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cmon_obj_vec,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cmon_obj_vec_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSOCKET_CNODE_ptr,
        /* type_sizeof            */sizeof(CSOCKET_CNODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSOCKET_CNODE,
        /* init_type_func         */(UINT32)csocket_cnode_init_0,
        /* clean_type_func        */(UINT32)csocket_cnode_clean_0,
        /* free_type_func         */(UINT32)csocket_cnode_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csocket_cnode,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csocket_cnode,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csocket_cnode_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_TASKC_NODE_ptr,
        /* type_sizeof            */sizeof(TASKC_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_TASKC_NODE,
        /* init_type_func         */(UINT32)taskc_node_init_0,
        /* clean_type_func        */(UINT32)taskc_node_clean_0,
        /* free_type_func         */(UINT32)taskc_node_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_taskc_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_taskc_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_taskc_node_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSYS_CPU_STAT_ptr,
        /* type_sizeof            */sizeof(CSYS_CPU_STAT),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSYS_CPU_STAT,
        /* init_type_func         */(UINT32)csys_cpu_stat_init_0,
        /* clean_type_func        */(UINT32)csys_cpu_stat_clean_0,
        /* free_type_func         */(UINT32)csys_cpu_stat_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csys_cpu_stat,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csys_cpu_stat,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csys_cpu_stat_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_MM_MAN_OCCUPY_NODE_ptr,
        /* type_sizeof            */sizeof(MM_MAN_OCCUPY_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_MM_MAN_OCCUPY_NODE,
        /* init_type_func         */(UINT32)mm_man_occupy_node_init_0,
        /* clean_type_func        */(UINT32)mm_man_occupy_node_clean_0,
        /* free_type_func         */(UINT32)mm_man_occupy_node_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_mm_man_occupy_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_mm_man_occupy_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_mm_man_occupy_node_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_MM_MAN_LOAD_NODE_ptr,
        /* type_sizeof            */sizeof(MM_MAN_LOAD_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_MM_MAN_LOAD_NODE,
        /* init_type_func         */(UINT32)mm_man_load_node_init_0,
        /* clean_type_func        */(UINT32)mm_man_load_node_clean_0,
        /* free_type_func         */(UINT32)mm_man_load_node_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_mm_man_load_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_mm_man_load_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_mm_man_load_node_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_MM_MOD_NODE_ptr,
        /* type_sizeof            */sizeof(MM_MOD_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_MOD_NODE,
        /* init_type_func         */(UINT32)mod_node_init_0,
        /* clean_type_func        */(UINT32)mod_node_clean_0,
        /* free_type_func         */(UINT32)mod_node_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_mod_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_mod_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_mod_node_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CPROC_MODULE_STAT_ptr,
        /* type_sizeof            */sizeof(CPROC_MODULE_STAT),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CPROC_MODULE_STAT,
        /* init_type_func         */(UINT32)cproc_module_stat_init_0,
        /* clean_type_func        */(UINT32)cproc_module_stat_clean_0,
        /* free_type_func         */(UINT32)cproc_module_stat_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cproc_module_stat,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cproc_module_stat,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cproc_module_stat_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CRANK_THREAD_STAT_ptr,
        /* type_sizeof            */sizeof(CRANK_THREAD_STAT),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CRANK_THREAD_STAT,
        /* init_type_func         */(UINT32)crank_thread_stat_init_0,
        /* clean_type_func        */(UINT32)crank_thread_stat_clean_0,
        /* free_type_func         */(UINT32)crank_thread_stat_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_crank_thread_stat,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_crank_thread_stat,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_crank_thread_stat_size
    );

    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSYS_ETH_STAT_ptr,
        /* type_sizeof            */sizeof(CSYS_ETH_STAT),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSYS_ETH_STAT,
        /* init_type_func         */(UINT32)csys_eth_stat_init_0,
        /* clean_type_func        */(UINT32)csys_eth_stat_clean_0,
        /* free_type_func         */(UINT32)csys_eth_stat_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csys_eth_stat,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csys_eth_stat,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csys_eth_stat_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSYS_DSK_STAT_ptr,
        /* type_sizeof            */sizeof(CSYS_DSK_STAT),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSYS_DSK_STAT,
        /* init_type_func         */(UINT32)csys_dsk_stat_init_0,
        /* clean_type_func        */(UINT32)csys_dsk_stat_clean_0,
        /* free_type_func         */(UINT32)csys_dsk_stat_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csys_dsk_stat,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csys_dsk_stat,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csys_dsk_stat_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_TASK_REPORT_NODE_ptr,
        /* type_sizeof            */sizeof(TASK_REPORT_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_TASK_REPORT_NODE,
        /* init_type_func         */(UINT32)task_report_node_init_0,
        /* clean_type_func        */(UINT32)task_report_node_clean_0,
        /* free_type_func         */(UINT32)task_report_node_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_task_report_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_task_report_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_task_report_node_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CDFSNP_FNODE_ptr,
        /* type_sizeof            */sizeof(CDFSNP_FNODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CDFSNP_FNODE,
        /* init_type_func         */(UINT32)cdfs_fnode_init,
        /* clean_type_func        */(UINT32)cdfs_fnode_clean,
        /* free_type_func         */(UINT32)cdfs_fnode_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cdfsnp_fnode,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cdfsnp_fnode,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cdfsnp_fnode_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CDFSNP_ITEM_ptr,
        /* type_sizeof            */sizeof(CDFSNP_ITEM),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CDFSNP_ITEM,
        /* init_type_func         */(UINT32)cdfsnp_item_init_0,
        /* clean_type_func        */(UINT32)cdfsnp_item_clean_0,
        /* free_type_func         */(UINT32)cdfsnp_item_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cdfsnp_item,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cdfsnp_item,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cdfsnp_item_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CDFSDN_STAT_ptr,
        /* type_sizeof            */sizeof(CDFSDN_STAT),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CDFSDN_STAT,
        /* init_type_func         */(UINT32)cdfsdn_stat_init_0,
        /* clean_type_func        */(UINT32)cdfsdn_stat_clean_0,
        /* free_type_func         */(UINT32)cdfsdn_stat_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cdfsdn_stat,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cdfsdn_stat,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cdfsdn_stat_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CLOAD_STAT_ptr,
        /* type_sizeof            */sizeof(CLOAD_STAT),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CLOAD_STAT,
        /* init_type_func         */(UINT32)cload_stat_init_0,
        /* clean_type_func        */(UINT32)cload_stat_clean_0,
        /* free_type_func         */(UINT32)cload_stat_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cload_stat,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cload_stat,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cload_stat_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CLOAD_NODE_ptr,
        /* type_sizeof            */sizeof(CLOAD_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CLOAD_NODE,
        /* init_type_func         */(UINT32)cload_node_init_0,
        /* clean_type_func        */(UINT32)cload_node_clean_0,
        /* free_type_func         */(UINT32)cload_node_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cload_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cload_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cload_node_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CLOAD_MGR_ptr,
        /* type_sizeof            */sizeof(CLOAD_MGR),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CLIST,
        /* init_type_func         */(UINT32)cload_mgr_init_0,
        /* clean_type_func        */(UINT32)cload_mgr_clean_0,
        /* free_type_func         */(UINT32)cload_mgr_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cload_mgr,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cload_mgr,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cload_mgr_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CFUSE_MODE_ptr,
        /* type_sizeof            */sizeof(CFUSE_MODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CFUSE_MODE,
        /* init_type_func         */(UINT32)cfuse_mode_init_0,
        /* clean_type_func        */(UINT32)cfuse_mode_clean_0,
        /* free_type_func         */(UINT32)cfuse_mode_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cfuse_mode,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cfuse_mode,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cfuse_mode_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CFUSE_STAT_ptr,
        /* type_sizeof            */sizeof(CFUSE_STAT),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CFUSE_STAT,
        /* init_type_func         */(UINT32)cfuse_stat_init_0,
        /* clean_type_func        */(UINT32)cfuse_stat_clean_0,
        /* free_type_func         */(UINT32)cfuse_stat_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cfuse_stat,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cfuse_stat,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cfuse_stat_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CDFSDN_RECORD_ptr,
        /* type_sizeof            */sizeof(CDFSDN_RECORD),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CDFSDN_RECORD,
        /* init_type_func         */(UINT32)cdfsdn_record_init_0,
        /* clean_type_func        */(UINT32)cdfsdn_record_clean_0,
        /* free_type_func         */(UINT32)cdfsdn_record_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cdfsdn_record,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cdfsdn_record,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cdfsdn_record_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CDFSDN_BLOCK_ptr,
        /* type_sizeof            */sizeof(CDFSDN_BLOCK) + sizeof(CDFSDN_CACHE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CDFSDN_BLOCK,
        /* init_type_func         */(UINT32)cdfsdn_block_init_0,
        /* clean_type_func        */(UINT32)cdfsdn_block_clean_0,
        /* free_type_func         */(UINT32)cdfsdn_block_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cdfsdn_block,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cdfsdn_block,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cdfsdn_block_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CBYTES_ptr,
        /* type_sizeof            */sizeof(CBYTES),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CBYTES,
        /* init_type_func         */(UINT32)cbytes_init_0,
        /* clean_type_func        */(UINT32)cbytes_clean_0,
        /* free_type_func         */(UINT32)cbytes_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_cbytes,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_cbytes,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_cbytes_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_MOD_NODE_ptr,
        /* type_sizeof            */sizeof(MOD_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_MOD_NODE,
        /* init_type_func         */(UINT32)mod_node_init_0,
        /* clean_type_func        */(UINT32)mod_node_clean_0,
        /* free_type_func         */(UINT32)mod_node_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_mod_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_mod_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_mod_node_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CTIMET_ptr,
        /* type_sizeof            */sizeof(CTIMET),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CTIMET,
        /* init_type_func         */0,
        /* clean_type_func        */0,
        /* free_type_func         */0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_ctimet,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_ctimet,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_ctimet_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSESSION_NODE_ptr,
        /* type_sizeof            */sizeof(CSESSION_NODE),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSESSION_NODE,
        /* init_type_func         */(UINT32)csession_node_init,
        /* clean_type_func        */(UINT32)csession_node_clean,
        /* free_type_func         */(UINT32)csession_node_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csession_node,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csession_node,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csession_node_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSESSION_ITEM_ptr,
        /* type_sizeof            */sizeof(CSESSION_ITEM),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSESSION_ITEM,
        /* init_type_func         */(UINT32)csession_item_init,
        /* clean_type_func        */(UINT32)csession_item_clean,
        /* free_type_func         */(UINT32)csession_item_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csession_item,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csession_item,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csession_item_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSWORD_ptr,
        /* type_sizeof            */sizeof(CSWORD),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSWORD,
        /* init_type_func         */(UINT32)cscore_csword_init,
        /* clean_type_func        */(UINT32)cscore_csword_clean,
        /* free_type_func         */(UINT32)cscore_csword_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csword,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csword,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csword_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSDOC_ptr,
        /* type_sizeof            */sizeof(CSDOC),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSDOC,
        /* init_type_func         */(UINT32)cscore_csdoc_init,
        /* clean_type_func        */(UINT32)cscore_csdoc_clean,
        /* free_type_func         */(UINT32)cscore_csdoc_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csdoc,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csdoc,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csdoc_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSDOC_WORDS_ptr,
        /* type_sizeof            */sizeof(CSDOC_WORDS),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSDOC_WORDS,
        /* init_type_func         */(UINT32)cscore_csdoc_words_init,
        /* clean_type_func        */(UINT32)cscore_csdoc_words_clean,
        /* free_type_func         */(UINT32)cscore_csdoc_words_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csdoc_words,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csdoc_words,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csdoc_words_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CSWORD_DOCS_ptr,
        /* type_sizeof            */sizeof(CSWORD_DOCS),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CSWORD_DOCS,
        /* init_type_func         */(UINT32)cscore_csword_docs_init,
        /* clean_type_func        */(UINT32)cscore_csword_docs_clean,
        /* free_type_func         */(UINT32)cscore_csword_docs_free,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_csword_docs,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_csword_docs,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_csword_docs_size
    );
    creg_type_conv_vec_add(type_conv_vec,
        /* type                   */e_dbg_CLIST_ptr,
        /* type_sizeof            */sizeof(CLIST),
        /* pointer_flag           */EC_TRUE,
        /* var_mm_type            */MM_CLIST,
        /* init_type_func         */(UINT32)clist_init_0,
        /* clean_type_func        */(UINT32)clist_clean_0,
        /* free_type_func         */(UINT32)clist_free_0,
        /* cmpi_encode_type_func  */(UINT32)cmpi_encode_clist,
        /* cmpi_decode_type_func  */(UINT32)cmpi_decode_clist,
        /* cmpi_encode_type_size  */(UINT32)cmpi_encode_clist_size
    );
    return (EC_TRUE);
}

FUNC_ADDR_MGR *creg_func_addr_mgr_new()
{
    FUNC_ADDR_MGR *func_addr_mgr;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_FUNC_ADDR_MGR, &func_addr_mgr, LOC_CREG_0005);
    if(NULL_PTR == func_addr_mgr)
    {
        sys_log(LOGSTDOUT, "error:creg_func_addr_mgr_new: new func addr mgr failed\n");
        return (NULL_PTR);
    }
    creg_func_addr_mgr_init(func_addr_mgr);
    return (func_addr_mgr);
}

EC_BOOL creg_func_addr_mgr_init(FUNC_ADDR_MGR *func_addr_mgr)
{
    FUNC_ADDR_MGR_MD_TYPE(func_addr_mgr)            = MD_END;
    FUNC_ADDR_MGR_FUNC_NUM_PTR(func_addr_mgr)       = NULL_PTR;
    FUNC_ADDR_MGR_FUNC_ADDR_NODE(func_addr_mgr)     = NULL_PTR;
    FUNC_ADDR_MGR_MD_START_FUNC_ID(func_addr_mgr)   = ERR_FUNC_ID;
    FUNC_ADDR_MGR_MD_END_FUNC_ID(func_addr_mgr)     = ERR_FUNC_ID;
    FUNC_ADDR_MGR_MD_SET_MOD_FUNC_ID(func_addr_mgr) = ERR_FUNC_ID;
    FUNC_ADDR_MGR_MD_GET_MOD_FUNC_ID(func_addr_mgr) = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL creg_func_addr_mgr_clean(FUNC_ADDR_MGR *func_addr_mgr)
{
    FUNC_ADDR_MGR_MD_TYPE(func_addr_mgr)            = MD_END;
    FUNC_ADDR_MGR_FUNC_NUM_PTR(func_addr_mgr)       = NULL_PTR;
    FUNC_ADDR_MGR_FUNC_ADDR_NODE(func_addr_mgr)     = NULL_PTR;
    FUNC_ADDR_MGR_MD_START_FUNC_ID(func_addr_mgr)   = ERR_FUNC_ID;
    FUNC_ADDR_MGR_MD_END_FUNC_ID(func_addr_mgr)     = ERR_FUNC_ID;
    FUNC_ADDR_MGR_MD_SET_MOD_FUNC_ID(func_addr_mgr) = ERR_FUNC_ID;
    FUNC_ADDR_MGR_MD_GET_MOD_FUNC_ID(func_addr_mgr) = NULL_PTR;
    return (EC_TRUE);
}

EC_BOOL creg_func_addr_mgr_free(FUNC_ADDR_MGR *func_addr_mgr)
{
    if(NULL_PTR != func_addr_mgr)
    {
        creg_func_addr_mgr_clean(func_addr_mgr);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_FUNC_ADDR_MGR, func_addr_mgr, LOC_CREG_0006);
    }
    return (EC_TRUE);
}

CVECTOR *creg_func_addr_vec_fetch()
{
    TASK_BRD *task_brd;
    task_brd = task_brd_default_get();
    return TASK_BRD_FUNC_ADDR_VEC(task_brd);
}

EC_BOOL creg_func_addr_vec_init(CVECTOR *func_addr_vec)
{
    cvector_init(func_addr_vec, CREG_FUNC_ADDR_MGR_DEFAULT_NUM, MM_FUNC_ADDR_MGR, CVECTOR_LOCK_ENABLE, LOC_CREG_0007);
    return (EC_TRUE);
}

EC_BOOL creg_func_addr_vec_clean(CVECTOR *func_addr_vec)
{
    cvector_clean(func_addr_vec, (CVECTOR_DATA_CLEANER)creg_func_addr_mgr_free, LOC_CREG_0008);
    return (EC_TRUE);
}

FUNC_ADDR_MGR *creg_func_addr_vec_get(const CVECTOR *func_addr_vec, const UINT32 md_type)
{
    return (FUNC_ADDR_MGR *)cvector_get(func_addr_vec, md_type);
}

EC_BOOL creg_func_addr_vec_add(CVECTOR *func_addr_vec,
                                        const UINT32 md_type, const UINT32 *func_num_ptr, const FUNC_ADDR_NODE *func_addr_node,
                                        const UINT32 md_start_func_id, const UINT32 md_end_func_id,
                                        const UINT32 md_set_mod_mgr_func_id, void * (*md_fget_mod_mgr)(const UINT32)
                                        )
{
    FUNC_ADDR_MGR *func_addr_mgr;
    UINT32 pos;

    if(NULL_PTR != cvector_get(func_addr_vec, md_type))
    {
        sys_log(LOGSTDOUT, "error:creg_func_addr_vec_add: func addr mgr for md_type %ld was already defined\n", md_type);
        return (EC_FALSE);
    }

    for(pos = cvector_size(func_addr_vec); pos <= md_type; pos ++)
    {
        cvector_push(func_addr_vec, NULL_PTR);
    }

    func_addr_mgr = creg_func_addr_mgr_new();
    if(NULL_PTR == func_addr_mgr)
    {
        sys_log(LOGSTDOUT, "error:creg_func_addr_vec_add: new type conv item failed\n");
        return (EC_FALSE);
    }

    FUNC_ADDR_MGR_MD_TYPE(func_addr_mgr)            = md_type;
    FUNC_ADDR_MGR_FUNC_NUM_PTR(func_addr_mgr)       = (UINT32 *)func_num_ptr;
    FUNC_ADDR_MGR_FUNC_ADDR_NODE(func_addr_mgr)     = (FUNC_ADDR_NODE *)func_addr_node;
    FUNC_ADDR_MGR_MD_START_FUNC_ID(func_addr_mgr)   = md_start_func_id;
    FUNC_ADDR_MGR_MD_END_FUNC_ID(func_addr_mgr)     = md_end_func_id;
    FUNC_ADDR_MGR_MD_SET_MOD_FUNC_ID(func_addr_mgr) = md_set_mod_mgr_func_id;
    FUNC_ADDR_MGR_MD_GET_MOD_FUNC_ID(func_addr_mgr) = md_fget_mod_mgr;

    cvector_set(func_addr_vec, md_type, (void *)func_addr_mgr);
    return (EC_TRUE);
}

EC_BOOL creg_func_addr_vec_add_default(CVECTOR *func_addr_vec)
{
    creg_func_addr_vec_add(func_addr_vec, MD_BGNZ   ,  &g_bgnz_func_addr_list_len,     (FUNC_ADDR_NODE *)g_bgnz_func_addr_list    , FI_bgn_z_start    , FI_bgn_z_end    , FI_bgn_z_set_mod_mgr    , (dbg_md_fget_mod_mgr) bgn_z_get_mod_mgr);
    creg_func_addr_vec_add(func_addr_vec, MD_BGNZ2  ,  &g_bgnz2_func_addr_list_len,    (FUNC_ADDR_NODE *)g_bgnz2_func_addr_list   , FI_bgn_z2_start   , FI_bgn_z2_end   , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_BGNZN  ,  &g_bgnzn_func_addr_list_len,    (FUNC_ADDR_NODE *)g_bgnzn_func_addr_list   , FI_bgn_zn_start   , FI_bgn_zn_end   , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_BGNF2N ,  &g_bgnf2n_func_addr_list_len,   (FUNC_ADDR_NODE *)g_bgnf2n_func_addr_list  , FI_bgn_f2n_start  , FI_bgn_f2n_end  , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_BGNFP  ,  &g_bgnfp_func_addr_list_len,    (FUNC_ADDR_NODE *)g_bgnfp_func_addr_list   , FI_bgn_fp_start   , FI_bgn_fp_end   , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_ECF2N  ,  &g_ecf2n_func_addr_list_len,    (FUNC_ADDR_NODE *)g_ecf2n_func_addr_list   , FI_ec_f2n_start   , FI_ec_f2n_end   , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_ECFP   ,  &g_ecfp_func_addr_list_len,     (FUNC_ADDR_NODE *)g_ecfp_func_addr_list    , FI_ec_fp_start    , FI_ec_fp_end    , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_ECCF2N ,  &g_eccf2n_func_addr_list_len,   (FUNC_ADDR_NODE *)g_eccf2n_func_addr_list  , FI_ecc_f2n_start  , FI_ecc_f2n_end  , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_ECCFP  ,  &g_eccfp_func_addr_list_len,    (FUNC_ADDR_NODE *)g_eccfp_func_addr_list   , FI_ecc_fp_start   , FI_ecc_fp_end   , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_CONV   ,  &g_conv_func_addr_list_len,     (FUNC_ADDR_NODE *)g_conv_func_addr_list    , FI_conv_start     , FI_conv_end     , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_POLYZ  ,  &g_polyz_func_addr_list_len,    (FUNC_ADDR_NODE *)g_polyz_func_addr_list   , ERR_FUNC_ID       , ERR_FUNC_ID     , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_POLYZ2 ,  &g_polyz2_func_addr_list_len,   (FUNC_ADDR_NODE *)g_polyz2_func_addr_list  , FI_poly_z2_start  , FI_poly_z2_end  , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_POLYZN ,  &g_polyzn_func_addr_list_len,   (FUNC_ADDR_NODE *)g_polyzn_func_addr_list  , FI_poly_zn_start  , FI_poly_zn_end  , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_POLYFP ,  &g_polyfp_func_addr_list_len,   (FUNC_ADDR_NODE *)g_polyfp_func_addr_list  , FI_poly_fp_start  , FI_poly_fp_end  , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_POLYF2N,  &g_polyf2n_func_addr_list_len,  (FUNC_ADDR_NODE *)g_polyf2n_func_addr_list , FI_poly_f2n_start , FI_poly_f2n_end , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_SEAFP  ,  &g_seafp_func_addr_list_len,    (FUNC_ADDR_NODE *)g_seafp_func_addr_list   , FI_sea_fp_start   , FI_sea_fp_end   , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_TASK   ,  NULL_PTR,                        NULL_PTR                                  , ERR_FUNC_ID       , ERR_FUNC_ID     , ERR_FUNC_ID             , NULL_PTR                               );
    creg_func_addr_vec_add(func_addr_vec, MD_MATRIXR,  &g_matrixr_func_addr_list_len,  (FUNC_ADDR_NODE *)g_matrixr_func_addr_list , FI_matrix_r_start , FI_matrix_r_end , FI_matrix_r_set_mod_mgr , (dbg_md_fget_mod_mgr) matrix_r_get_mod_mgr );
    creg_func_addr_vec_add(func_addr_vec, MD_EBGNZ  ,  &g_ebgnz_func_addr_list_len,    (FUNC_ADDR_NODE *)g_ebgnz_func_addr_list   , FI_ebgn_z_start   , FI_ebgn_z_end   , ERR_FUNC_ID             , NULL_PTR                                   );
    creg_func_addr_vec_add(func_addr_vec, MD_EBGNZ2 ,  NULL_PTR,                        NULL_PTR                                  , ERR_FUNC_ID       , ERR_FUNC_ID     , ERR_FUNC_ID             , NULL_PTR                                   );
    creg_func_addr_vec_add(func_addr_vec, MD_VECTORR,  &g_vectorr_func_addr_list_len,  (FUNC_ADDR_NODE *)g_vectorr_func_addr_list , FI_vector_r_start , FI_vector_r_end , FI_vector_r_set_mod_mgr , (dbg_md_fget_mod_mgr) vector_r_get_mod_mgr );
    creg_func_addr_vec_add(func_addr_vec, MD_PARSER ,  NULL_PTR                      ,  NULL_PTR                                  , ERR_FUNC_ID       , ERR_FUNC_ID     , ERR_FUNC_ID             , NULL_PTR                                   );
    creg_func_addr_vec_add(func_addr_vec, MD_EBGNR  ,  NULL_PTR,                        NULL_PTR                                  , ERR_FUNC_ID       , ERR_FUNC_ID     , ERR_FUNC_ID             , NULL_PTR                                   );
    creg_func_addr_vec_add(func_addr_vec, MD_DMATRIXR,  &g_dmatrixr_func_addr_list_len,(FUNC_ADDR_NODE *)g_dmatrixr_func_addr_list, FI_dmatrix_r_start, FI_dmatrix_r_end, FI_dmatrix_r_set_mod_mgr, (dbg_md_fget_mod_mgr) dmatrix_r_get_mod_mgr);
    creg_func_addr_vec_add(func_addr_vec, MD_VMATRIXR,  &g_vmatrixr_func_addr_list_len,(FUNC_ADDR_NODE *)g_vmatrixr_func_addr_list, FI_vmatrix_r_start, FI_vmatrix_r_end, FI_vmatrix_r_set_mod_mgr, (dbg_md_fget_mod_mgr) vmatrix_r_get_mod_mgr);
    creg_func_addr_vec_add(func_addr_vec, MD_CFILE   ,  &g_cfile_func_addr_list_len  ,   (FUNC_ADDR_NODE *)g_cfile_func_addr_list  , FI_cfile_start   , FI_cfile_end    , FI_cfile_set_mod_mgr    , (dbg_md_fget_mod_mgr) cfile_get_mod_mgr    );
    creg_func_addr_vec_add(func_addr_vec, MD_CDIR    ,  &g_cdir_func_addr_list_len   ,   (FUNC_ADDR_NODE *)g_cdir_func_addr_list   , FI_cdir_start    , FI_cdir_end     , FI_cdir_set_mod_mgr     , (dbg_md_fget_mod_mgr) cdir_get_mod_mgr     );
    creg_func_addr_vec_add(func_addr_vec, MD_CMON    ,  &g_cmon_func_addr_list_len   ,   (FUNC_ADDR_NODE *)g_cmon_func_addr_list   , FI_cmon_start    , FI_cmon_end     , FI_cmon_set_mod_mgr     , (dbg_md_fget_mod_mgr) cmon_get_mod_mgr     );
    creg_func_addr_vec_add(func_addr_vec, MD_TBD     ,  &g_tbd_func_addr_list_len    ,   (FUNC_ADDR_NODE *)g_tbd_func_addr_list    , FI_tbd_start     , FI_tbd_end      , FI_tbd_set_mod_mgr      , (dbg_md_fget_mod_mgr) tbd_get_mod_mgr      );
    creg_func_addr_vec_add(func_addr_vec, MD_CRUN    ,  &g_crun_func_addr_list_len   ,   (FUNC_ADDR_NODE *)g_crun_func_addr_list   , ERR_FUNC_ID      , ERR_FUNC_ID     , ERR_FUNC_ID             , NULL_PTR                                   );
    creg_func_addr_vec_add(func_addr_vec, MD_VMM     ,  &g_vmm_func_addr_list_len    ,   (FUNC_ADDR_NODE *)g_vmm_func_addr_list    , FI_vmm_start     , FI_vmm_end      , FI_vmm_set_mod_mgr      , (dbg_md_fget_mod_mgr) vmm_get_mod_mgr      );
    creg_func_addr_vec_add(func_addr_vec, MD_SUPER   ,  &g_super_func_addr_list_len  ,   (FUNC_ADDR_NODE *)g_super_func_addr_list  , FI_super_start   , FI_super_end    , ERR_FUNC_ID             , NULL_PTR                                   );
    creg_func_addr_vec_add(func_addr_vec, MD_CTIMER  ,  NULL_PTR                     ,   NULL_PTR                                  , ERR_FUNC_ID      , ERR_FUNC_ID     , ERR_FUNC_ID             , NULL_PTR                                   );
    creg_func_addr_vec_add(func_addr_vec, MD_CDFS    ,  &g_cdfs_func_addr_list_len   ,   (FUNC_ADDR_NODE *)g_cdfs_func_addr_list   , FI_cdfs_start    , FI_cdfs_end     , ERR_FUNC_ID             , (dbg_md_fget_mod_mgr) cdfs_get_dn_mod_mgr  );
    creg_func_addr_vec_add(func_addr_vec, MD_CSOLR   ,  &g_csolr_func_addr_list_len  ,   (FUNC_ADDR_NODE *)g_csolr_func_addr_list  , FI_csolr_start   , FI_csolr_end    , FI_csolr_set_mod_mgr    , (dbg_md_fget_mod_mgr) csolr_get_mod_mgr    );
    creg_func_addr_vec_add(func_addr_vec, MD_CBGT    ,  &g_cbgt_func_addr_list_len   ,   (FUNC_ADDR_NODE *)g_cbgt_func_addr_list   , FI_cbgt_start    , FI_cbgt_end     , FI_cbgt_set_mod_mgr     , (dbg_md_fget_mod_mgr) cbgt_get_mod_mgr     );
    creg_func_addr_vec_add(func_addr_vec, MD_GANGLIA ,  &g_ganglia_func_addr_list_len,   (FUNC_ADDR_NODE *)g_ganglia_func_addr_list, FI_ganglia_start , FI_ganglia_end  , ERR_FUNC_ID             , NULL_PTR                                   );
    creg_func_addr_vec_add(func_addr_vec, MD_CSESSION,  &g_csession_func_addr_list_len,  (FUNC_ADDR_NODE *)g_csession_func_addr_list, FI_csession_start , FI_csession_end, ERR_FUNC_ID            , NULL_PTR                                   );
    creg_func_addr_vec_add(func_addr_vec, MD_CSCORE  ,  &g_cscore_func_addr_list_len ,   (FUNC_ADDR_NODE *)g_cscore_func_addr_list , FI_cscore_start  , FI_cscore_end   , ERR_FUNC_ID             , NULL_PTR                                   );

    return (EC_TRUE);
}


EC_BOOL creg_static_mem_tbl_add(const UINT32 mm_type, const char *mm_name, const UINT32 block_num, const UINT32 type_size, const UINT32 location)
{
    if(0 != reg_mm_man(mm_type, mm_name, block_num, type_size, location))
    {
        sys_log(LOGSTDOUT, "error:creg_static_mem_tbl_add: add static mem failed for type %ld, name %s, block num %ld, type size %ld at location %ld\n",
                            mm_type, mm_name, block_num, type_size, location);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

#if 0
EC_BOOL creg_static_mem_tbl_add_default()
{
    creg_static_mem_tbl_add(MM_UINT32                       ,"MM_UINT32                       ", 32      , sizeof(UINT32)                     , LOC_CREG_0009);
    creg_static_mem_tbl_add(MM_UINT16                       ,"MM_UINT16                       ", 32      , sizeof(UINT16)                     , LOC_CREG_0010);
    creg_static_mem_tbl_add(MM_UINT8                        ,"MM_UINT8                        ", 32      , sizeof(UINT8)                      , LOC_CREG_0011);

    creg_static_mem_tbl_add(MM_BIGINT                       ,"MM_BIGINT                       ", 1024    , sizeof(BIGINT)                     , LOC_CREG_0012);
    creg_static_mem_tbl_add(MM_NAF                          ,"MM_NAF                          ", 2       , (MAX_NAF_ARRAY_LEN * sizeof (int )), LOC_CREG_0013);

    creg_static_mem_tbl_add(MM_CURVE_POINT                  ,"MM_CURVE_POINT                  ", 1024    , sizeof(EC_CURVE_POINT)              , LOC_CREG_0014);
    creg_static_mem_tbl_add(MM_CURVE_AFFINE_POINT           ,"MM_CURVE_AFFINE_POINT           ", 16      , sizeof(EC_CURVE_AFF_POINT)          , LOC_CREG_0015);
    creg_static_mem_tbl_add(MM_ECF2N_CURVE                  ,"MM_ECF2N_CURVE                  ", 8       , sizeof(ECF2N_CURVE)                 , LOC_CREG_0016);
    creg_static_mem_tbl_add(MM_ECFP_CURVE                   ,"MM_ECFP_CURVE                   ", 8       , sizeof(ECFP_CURVE)                  , LOC_CREG_0017);
    creg_static_mem_tbl_add(MM_KEY_PAIR                     ,"MM_KEY_PAIR                     ", 128     , sizeof(ECC_KEYPAIR)                 , LOC_CREG_0018);
    creg_static_mem_tbl_add(MM_EBGN                         ,"MM_EBGN                         ", 128     , sizeof(EBGN)                        , LOC_CREG_0019);
    creg_static_mem_tbl_add(MM_EBGN_ITEM                    ,"MM_EBGN_ITEM                    ", 128     , sizeof(EBGN_ITEM)                   , LOC_CREG_0020);
    creg_static_mem_tbl_add(MM_POLY                         ,"MM_POLY                         ", 512     , sizeof(POLY)                        , LOC_CREG_0021);
    creg_static_mem_tbl_add(MM_POLY_ITEM                    ,"MM_POLY_ITEM                    ", 1024    , sizeof(POLY_ITEM)                   , LOC_CREG_0022);
    creg_static_mem_tbl_add(MM_DEGREE                       ,"MM_DEGREE                       ", 8       , sizeof(DEGREE)                      , LOC_CREG_0023);

    creg_static_mem_tbl_add(MM_STRCHAR128B                  ,"MM_STRCHAR128B                  ", 8       , 128                                 , LOC_CREG_0024);
    creg_static_mem_tbl_add(MM_STRCHAR256B                  ,"MM_STRCHAR256B                  ", 64      , 256                                 , LOC_CREG_0025);
    creg_static_mem_tbl_add(MM_STRCHAR512B                  ,"MM_STRCHAR512B                  ", 8       , 512                                 , LOC_CREG_0026);
    creg_static_mem_tbl_add(MM_STRCHAR1K                    ,"MM_STRCHAR1K                    ", 8       , 1024                                , LOC_CREG_0027);
    creg_static_mem_tbl_add(MM_STRCHAR2K                    ,"MM_STRCHAR2K                    ", 8       , 2048                                , LOC_CREG_0028);
    creg_static_mem_tbl_add(MM_STRCHAR4K                    ,"MM_STRCHAR4K                    ", 4       , 4096                                , LOC_CREG_0029);
    creg_static_mem_tbl_add(MM_STRCHAR10K                   ,"MM_STRCHAR10K                   ", 128     , 10240                               , LOC_CREG_0030);
    creg_static_mem_tbl_add(MM_STRCHAR64K                   ,"MM_STRCHAR64K                   ", 128     , 1024 * 64                           , LOC_CREG_0031);
    creg_static_mem_tbl_add(MM_STRCHAR1M                    ,"MM_STRCHAR1M                    ", 4       , 1024 * 1024 * 1                     , LOC_CREG_0032);
    creg_static_mem_tbl_add(MM_STRCHAR2M                    ,"MM_STRCHAR2M                    ", 1       , 1024 * 1024 * 2                     , LOC_CREG_0033);
    creg_static_mem_tbl_add(MM_STRCHAR4M                    ,"MM_STRCHAR4M                    ", 1       , 1024 * 1024 * 4                     , LOC_CREG_0034);
    creg_static_mem_tbl_add(MM_STRCHAR8M                    ,"MM_STRCHAR8M                    ", 1       , 1024 * 1024 * 8                     , LOC_CREG_0035);
    creg_static_mem_tbl_add(MM_STRCHAR16M                   ,"MM_STRCHAR16M                   ", 1       , 1024 * 1024 * 16                    , LOC_CREG_0036);
    creg_static_mem_tbl_add(MM_STRCHAR32M                   ,"MM_STRCHAR32M                   ", 1       , 1024 * 1024 * 32                    , LOC_CREG_0037);
    creg_static_mem_tbl_add(MM_STRCHAR64M                   ,"MM_STRCHAR64M                   ", 1       , 1024 * 1024 * 64                    , LOC_CREG_0038);
    creg_static_mem_tbl_add(MM_STRCHAR128M                  ,"MM_STRCHAR128M                  ", 1       , 1024 * 1024 * 128                   , LOC_CREG_0039);

    creg_static_mem_tbl_add(MM_REAL                         ,"MM_REAL                         ", 1024    , sizeof(REAL)                        , LOC_CREG_0040);
    creg_static_mem_tbl_add(MM_MATRIX_BLOCK                 ,"MM_MATRIX_BLOCK                 ", 8 * 8   , sizeof(MATRIX_BLOCK)                , LOC_CREG_0041);
    creg_static_mem_tbl_add(MM_MATRIX_HEADER                ,"MM_MATRIX_HEADER                ", 8 * 2   , sizeof(void *)                      , LOC_CREG_0042);/*MATRIX_HEADER is removed, so this line is not meaningful*/
    creg_static_mem_tbl_add(MM_MATRIX                       ,"MM_MATRIX                       ", 4       , sizeof(MATRIX)                      , LOC_CREG_0043);
    creg_static_mem_tbl_add(MM_VECTOR_BLOCK                 ,"MM_VECTOR_BLOCK                 ", 8       , sizeof(VECTOR_BLOCK)                , LOC_CREG_0044);
    creg_static_mem_tbl_add(MM_VECTOR                       ,"MM_VECTOR                       ", 32      , sizeof(VECTOR)                      , LOC_CREG_0045);

#ifdef PARSER_MEM_MGR
    creg_static_mem_tbl_add(MM_PROD                         ,"MM_PROD                         ", 1024    , sizeof(struct prod_token_item)      , LOC_CREG_0046);
    creg_static_mem_tbl_add(MM_TREE                         ,"MM_TREE                         ", 1024    , sizeof(union tree_node)             , LOC_CREG_0047);
#else
    creg_static_mem_tbl_add(MM_PROD_RSVD                    ,"MM_PROD_RSVD                    ", 1024    , 1                                   , LOC_CREG_0048);
    creg_static_mem_tbl_add(MM_TREE_RSVD                    ,"MM_TREE_RSVD                    ", 1024    , 1                                   , LOC_CREG_0049);
#endif/*PARSER_MEM_MGR*/

    creg_static_mem_tbl_add(MM_TASK_NODE                    ,"MM_TASK_NODE                    ", 1024    , sizeof(TASK_NODE)                   , LOC_CREG_0050);
    creg_static_mem_tbl_add(MM_TASK_MGR                     ,"MM_TASK_MGR                     ", 32      , sizeof(TASK_MGR)                    , LOC_CREG_0051);
    creg_static_mem_tbl_add(MM_TASK_CONTEXT                 ,"MM_TASK_CONTEXT                 ", 32      , sizeof(TASK_CONTEXT)                , LOC_CREG_0052);

    creg_static_mem_tbl_add(MM_MOD_NODE                     ,"MM_MOD_NODE                     ", 1024    , sizeof(MOD_NODE)                    , LOC_CREG_0053);
    creg_static_mem_tbl_add(MM_MOD_MGR                      ,"MM_MOD_MGR                      ", 32      , sizeof(MOD_MGR)                     , LOC_CREG_0054);

    creg_static_mem_tbl_add(MM_TASKC_MGR                    ,"MM_TASKC_MGR                    ", 32      , sizeof(TASKC_MGR)                   , LOC_CREG_0055);

    creg_static_mem_tbl_add(MM_CLIST_DATA                   ,"MM_CLIST_DATA                   ", 1024    , sizeof(CLIST_DATA)                  , LOC_CREG_0056);
    creg_static_mem_tbl_add(MM_CSTACK_DATA                  ,"MM_CSTACK_DATA                  ", 1024    , sizeof(CSTACK_DATA)                 , LOC_CREG_0057);
    creg_static_mem_tbl_add(MM_CSET_DATA                    ,"MM_CSET_DATA                    ", 1024    , sizeof(CSET_DATA)                   , LOC_CREG_0058);
    creg_static_mem_tbl_add(MM_CQUEUE_DATA                  ,"MM_CQUEUE_DATA                  ", 1024    , sizeof(CQUEUE_DATA)                 , LOC_CREG_0059);
    creg_static_mem_tbl_add(MM_CSTRING                      ,"MM_CSTRING                      ", 32      , sizeof(CSTRING)                     , LOC_CREG_0060);

    creg_static_mem_tbl_add(MM_FUNC_ADDR_MGR                ,"MM_FUNC_ADDR_MGR                ", 16      , sizeof(FUNC_ADDR_MGR)               , LOC_CREG_0061);

    creg_static_mem_tbl_add(MM_UINT8_064B                   ,"MM_UINT8_064B                   ", 64      , 64                                  , LOC_CREG_0062);
    creg_static_mem_tbl_add(MM_UINT8_128B                   ,"MM_UINT8_128B                   ", 32      , 128                                 , LOC_CREG_0063);
    creg_static_mem_tbl_add(MM_UINT8_256B                   ,"MM_UINT8_256B                   ", 16      , 256                                 , LOC_CREG_0064);
    creg_static_mem_tbl_add(MM_UINT8_512B                   ,"MM_UINT8_512B                   ", 8       , 512                                 , LOC_CREG_0065);

    creg_static_mem_tbl_add(MM_UINT8_001K                   ,"MM_UINT8_001K                   ", 64      , 1 * 1024                            , LOC_CREG_0066);
    creg_static_mem_tbl_add(MM_UINT8_002K                   ,"MM_UINT8_002K                   ", 32      , 2 * 1024                            , LOC_CREG_0067);
    creg_static_mem_tbl_add(MM_UINT8_004K                   ,"MM_UINT8_004K                   ", 16      , 4 * 1024                            , LOC_CREG_0068);
    creg_static_mem_tbl_add(MM_UINT8_008K                   ,"MM_UINT8_008K                   ", 8       , 5 * 1024                            , LOC_CREG_0069);
    creg_static_mem_tbl_add(MM_UINT8_016K                   ,"MM_UINT8_016K                   ", 4       , 16 * 1024                           , LOC_CREG_0070);
    creg_static_mem_tbl_add(MM_UINT8_032K                   ,"MM_UINT8_032K                   ", 2       , 32 * 1024                           , LOC_CREG_0071);
    creg_static_mem_tbl_add(MM_UINT8_064K                   ,"MM_UINT8_064K                   ", 8       , 64 * 1024                           , LOC_CREG_0072);
    creg_static_mem_tbl_add(MM_UINT8_128K                   ,"MM_UINT8_128K                   ", 4       , 128 * 1024                          , LOC_CREG_0073);
    creg_static_mem_tbl_add(MM_UINT8_256K                   ,"MM_UINT8_256K                   ", 2       , 256 * 1024                          , LOC_CREG_0074);
    creg_static_mem_tbl_add(MM_UINT8_512K                   ,"MM_UINT8_512K                   ", 1       , 512 * 1024                          , LOC_CREG_0075);

    creg_static_mem_tbl_add(MM_UINT8_001M                   ,"MM_UINT8_001M                   ", 64      , 1 * 1024 * 1024                     , LOC_CREG_0076);
    creg_static_mem_tbl_add(MM_UINT8_002M                   ,"MM_UINT8_002M                   ", 32      , 2 * 1024 * 1024                     , LOC_CREG_0077);
    creg_static_mem_tbl_add(MM_UINT8_004M                   ,"MM_UINT8_004M                   ", 16      , 4 * 1024 * 1024                     , LOC_CREG_0078);
    creg_static_mem_tbl_add(MM_UINT8_008M                   ,"MM_UINT8_008M                   ", 8       , 5 * 1024 * 1024                     , LOC_CREG_0079);
    creg_static_mem_tbl_add(MM_UINT8_016M                   ,"MM_UINT8_016M                   ", 4       , 16 * 1024 * 1024                    , LOC_CREG_0080);
    creg_static_mem_tbl_add(MM_UINT8_032M                   ,"MM_UINT8_032M                   ", 2       , 32 * 1024 * 1024                    , LOC_CREG_0081);
    creg_static_mem_tbl_add(MM_UINT8_064M                   ,"MM_UINT8_064M                   ", 2       , 64 * 1024 * 1024                    , LOC_CREG_0082);
    creg_static_mem_tbl_add(MM_UINT8_128M                   ,"MM_UINT8_128M                   ", 2       , 128 * 1024 * 1024                   , LOC_CREG_0083);
    creg_static_mem_tbl_add(MM_UINT8_256M                   ,"MM_UINT8_256M                   ", 1       , 256 * 1024 * 1024                   , LOC_CREG_0084);
    creg_static_mem_tbl_add(MM_UINT8_512M                   ,"MM_UINT8_512M                   ", 1       , 512 * 1024 * 1024                   , LOC_CREG_0085);

    creg_static_mem_tbl_add(MM_REAL_BLOCK_2_2               ,"MM_REAL_BLOCK_2_2               ", 128     , 2 *    2 * sizeof(REAL)             , LOC_CREG_0086);
    creg_static_mem_tbl_add(MM_REAL_BLOCK_4_4               ,"MM_REAL_BLOCK_4_4               ", 128     , 4 *    4 * sizeof(REAL)             , LOC_CREG_0087);
    creg_static_mem_tbl_add(MM_REAL_BLOCK_8_8               ,"MM_REAL_BLOCK_8_8               ", 128     , 8 *    8 * sizeof(REAL)             , LOC_CREG_0088);
    creg_static_mem_tbl_add(MM_REAL_BLOCK_16_16             ,"MM_REAL_BLOCK_16_16             ", 128     , 16 *   16 * sizeof(REAL)            , LOC_CREG_0089);
    creg_static_mem_tbl_add(MM_REAL_BLOCK_32_32             ,"MM_REAL_BLOCK_32_32             ", 64      , 32 *   32 * sizeof(REAL)            , LOC_CREG_0090);
    creg_static_mem_tbl_add(MM_REAL_BLOCK_64_64             ,"MM_REAL_BLOCK_64_64             ", 32      , 64 *   64 * sizeof(REAL)            , LOC_CREG_0091);
    creg_static_mem_tbl_add(MM_REAL_BLOCK_128_128           ,"MM_REAL_BLOCK_128_128           ", 16      , 128 *  128 * sizeof(REAL)           , LOC_CREG_0092);
    creg_static_mem_tbl_add(MM_REAL_BLOCK_256_256           ,"MM_REAL_BLOCK_256_256           ", 8       , 256 *  256 * sizeof(REAL)           , LOC_CREG_0093);
    creg_static_mem_tbl_add(MM_REAL_BLOCK_512_512           ,"MM_REAL_BLOCK_512_512           ", 4       , 512 *  512 * sizeof(REAL)           , LOC_CREG_0094);
    creg_static_mem_tbl_add(MM_REAL_BLOCK_1024_1024         ,"MM_REAL_BLOCK_1024_1024         ", 4       , 1024 * 1024 * sizeof(REAL)          , LOC_CREG_0095);

    creg_static_mem_tbl_add(MM_CLIST                        ,"MM_CLIST                        ", 32      , sizeof(CLIST)                       , LOC_CREG_0096);
    creg_static_mem_tbl_add(MM_CSTACK                       ,"MM_CSTACK                       ", 32      , sizeof(CSTACK)                      , LOC_CREG_0097);
    creg_static_mem_tbl_add(MM_CSET                         ,"MM_CSET                         ", 32      , sizeof(CSET)                        , LOC_CREG_0098);
    creg_static_mem_tbl_add(MM_CQUEUE                       ,"MM_CQUEUE                       ", 32      , sizeof(CQUEUE)                      , LOC_CREG_0099);
    creg_static_mem_tbl_add(MM_CVECTOR                      ,"MM_CVECTOR                      ", 32      , sizeof(CVECTOR)                     , LOC_CREG_0100);

    creg_static_mem_tbl_add(MM_CTHREAD_TASK                 ,"MM_CTHREAD_TASK                 ", 32      , sizeof(CTHREAD_TASK)                , LOC_CREG_0101);

    creg_static_mem_tbl_add(MM_TASKS_CFG                    ,"MM_TASKS_CFG                    ", 32      , sizeof(TASKS_CFG)                   , LOC_CREG_0102);
    creg_static_mem_tbl_add(MM_TASKR_CFG                    ,"MM_TASKR_CFG                    ", 32      , sizeof(TASKR_CFG)                   , LOC_CREG_0103);
    creg_static_mem_tbl_add(MM_TASK_CFG                     ,"MM_TASK_CFG                     ", 1       , sizeof(TASK_CFG)                    , LOC_CREG_0104);

    creg_static_mem_tbl_add(MM_TASKS_NODE                   ,"MM_TASKS_NODE                   ", 32      , sizeof(TASKS_NODE)                  , LOC_CREG_0105);
    creg_static_mem_tbl_add(MM_TASKC_NODE                   ,"MM_TASKC_NODE                   ", 32      , sizeof(TASKC_NODE)                  , LOC_CREG_0106);
    creg_static_mem_tbl_add(MM_CROUTER_NODE                 ,"MM_CROUTER_NODE                 ", 128     , sizeof(CROUTER_NODE)                , LOC_CREG_0107);
    creg_static_mem_tbl_add(MM_CROUTER_NODE_VEC             ,"MM_CROUTER_NODE_VEC             ", 128     , sizeof(CROUTER_NODE_VEC)            , LOC_CREG_0108);
    creg_static_mem_tbl_add(MM_CROUTER_CFG                  ,"MM_CROUTER_CFG                  ", 128     , sizeof(CROUTER_CFG)                 , LOC_CREG_0109);

    creg_static_mem_tbl_add(MM_VMM_NODE                     ,"MM_VMM_NODE                     ", 32      , sizeof(VMM_NODE)                    , LOC_CREG_0110);
    creg_static_mem_tbl_add(MM_LOG                          ,"MM_LOG                          ", 4       , sizeof(LOG)                         , LOC_CREG_0111);

    creg_static_mem_tbl_add(MM_COMM_NODE                    ,"MM_COMM_NODE                    ", 128     , sizeof(MM_COMM)                     , LOC_CREG_0112);
    creg_static_mem_tbl_add(MM_CFILE_SEG                    ,"MM_CFILE_SEG                    ", 128     , sizeof(CFILE_SEG)                   , LOC_CREG_0113);
    creg_static_mem_tbl_add(MM_CFILE_NODE                   ,"MM_CFILE_NODE                   ", 128     , sizeof(CFILE_NODE)                  , LOC_CREG_0114);
    creg_static_mem_tbl_add(MM_KBUFF                        ,"MM_KBUFF                        ", 128     , sizeof(KBUFF)                       , LOC_CREG_0115);
    creg_static_mem_tbl_add(MM_CDIR_SEG                     ,"MM_CDIR_SEG                     ", 128     , sizeof(CDIR_SEG)                    , LOC_CREG_0116);
    creg_static_mem_tbl_add(MM_CDIR_NODE                    ,"MM_CDIR_NODE                    ", 128     , sizeof(CDIR_NODE)                   , LOC_CREG_0117);

    creg_static_mem_tbl_add(MM_CMON_OBJ                     ,"MM_CMON_OBJ                     ", 128     , sizeof(CMON_OBJ)                    , LOC_CREG_0118);
    creg_static_mem_tbl_add(MM_CMON_OBJ_VEC                 ,"MM_CMON_OBJ_VEC                 ", 128     , sizeof(CMON_OBJ_VEC)                , LOC_CREG_0119);
    creg_static_mem_tbl_add(MM_CMON_OBJ_MERGE               ,"MM_CMON_OBJ_MERGE               ", 128     , sizeof(CMON_OBJ_MERGE)              , LOC_CREG_0120);

    creg_static_mem_tbl_add(MM_CSOCKET_CNODE                ,"MM_CSOCKET_CNODE                ", 128     , sizeof(CSOCKET_CNODE)               , LOC_CREG_0121);
    creg_static_mem_tbl_add(MM_CTIMER_NODE                  ,"MM_CTIMER_NODE                  ", 128     , sizeof(CTIMER_NODE)                 , LOC_CREG_0122);

    creg_static_mem_tbl_add(MM_CSYS_CPU_STAT                ,"MM_CSYS_CPU_STAT                ", 128     , sizeof(CSYS_CPU_STAT)               , LOC_CREG_0123);
    creg_static_mem_tbl_add(MM_CSYS_CPU_AVG_STAT            ,"MM_CSYS_CPU_AVG_STAT            ", 128     , sizeof(CSYS_CPU_AVG_STAT)           , LOC_CREG_0124);
    creg_static_mem_tbl_add(MM_CSYS_MEM_STAT                ,"MM_CSYS_MEM_STAT                ", 4       , sizeof(CSYS_MEM_STAT)               , LOC_CREG_0125);
    creg_static_mem_tbl_add(MM_CPROC_MEM_STAT               ,"MM_CPROC_MEM_STAT               ", 4       , sizeof(CPROC_MEM_STAT)              , LOC_CREG_0126);
    creg_static_mem_tbl_add(MM_CPROC_CPU_STAT               ,"MM_CPROC_CPU_STAT               ", 4       , sizeof(CPROC_CPU_STAT)              , LOC_CREG_0127);
    creg_static_mem_tbl_add(MM_CPROC_THREAD_STAT            ,"MM_CPROC_THREAD_STAT            ", 4       , sizeof(CPROC_THREAD_STAT)           , LOC_CREG_0128);
    creg_static_mem_tbl_add(MM_CRANK_THREAD_STAT            ,"MM_CRANK_THREAD_STAT            ", 4       , sizeof(CRANK_THREAD_STAT)           , LOC_CREG_0129);
    creg_static_mem_tbl_add(MM_CPROC_MODULE_STAT            ,"MM_CPROC_MODULE_STAT            ", 4       , sizeof(CPROC_MODULE_STAT)           , LOC_CREG_0130);
    creg_static_mem_tbl_add(MM_CSYS_ETH_STAT                ,"MM_CSYS_ETH_STAT                ", 4       , sizeof(CSYS_ETH_STAT)               , LOC_CREG_0131);
    creg_static_mem_tbl_add(MM_CSYS_DSK_STAT                ,"MM_CSYS_DSK_STAT                ", 4       , sizeof(CSYS_DSK_STAT)               , LOC_CREG_0132);
    creg_static_mem_tbl_add(MM_MM_MAN_OCCUPY_NODE           ,"MM_MM_MAN_OCCUPY_NODE           ", 128     , sizeof(MM_MAN_OCCUPY_NODE)          , LOC_CREG_0133);
    creg_static_mem_tbl_add(MM_MM_MAN_LOAD_NODE             ,"MM_MM_MAN_LOAD_NODE             ", 128     , sizeof(MM_MAN_LOAD_NODE)            , LOC_CREG_0134);
    creg_static_mem_tbl_add(MM_CTHREAD_NODE                 ,"MM_CTHREAD_NODE                 ", 16      , sizeof(CTHREAD_NODE)                , LOC_CREG_0135);
    creg_static_mem_tbl_add(MM_CTHREAD_POOL                 ,"MM_CTHREAD_POOL                 ", 1       , sizeof(CTHREAD_POOL)                , LOC_CREG_0136);
    creg_static_mem_tbl_add(MM_TASK_RANK_NODE               ,"MM_TASK_RANK_NODE               ", 1       , sizeof(TASK_RANK_NODE)              , LOC_CREG_0137);
    creg_static_mem_tbl_add(MM_CMD_SEG                      ,"MM_CMD_SEG                      ", 128     , sizeof(CMD_SEG)                     , LOC_CREG_0138);
    creg_static_mem_tbl_add(MM_CMD_PARA                     ,"MM_CMD_PARA                     ", 8       , sizeof(CMD_PARA)                    , LOC_CREG_0139);
    creg_static_mem_tbl_add(MM_CMD_HELP                     ,"MM_CMD_HELP                     ", 32      , sizeof(CMD_HELP)                    , LOC_CREG_0140);
    creg_static_mem_tbl_add(MM_CMD_ELEM                     ,"MM_CMD_ELEM                     ", 32      , sizeof(CMD_ELEM)                    , LOC_CREG_0141);
    creg_static_mem_tbl_add(MM_TASK_REPORT_NODE             ,"MM_TASK_REPORT_NODE             ", 128     , sizeof(TASK_REPORT_NODE)            , LOC_CREG_0142);

    creg_static_mem_tbl_add(MM_CHASH_NODE                   ,"MM_CHASH_NODE                   ", 32      , sizeof(CHASH_NODE)                  , LOC_CREG_0143);
    creg_static_mem_tbl_add(MM_CHASH_VEC                    ,"MM_CHASH_VEC                    ", 4       , sizeof(CHASH_VEC)                   , LOC_CREG_0144);

    creg_static_mem_tbl_add(MM_CHASHDB_ITEM                 ,"MM_CHASHDB_ITEM                 ", 4       , sizeof(CHASHDB_ITEM)                , LOC_CREG_0145);
    creg_static_mem_tbl_add(MM_CHASHDB                      ,"MM_CHASHDB                      ", 4       , sizeof(CHASHDB)                     , LOC_CREG_0146);
    creg_static_mem_tbl_add(MM_CHASHDB_BUCKET               ,"MM_CHASHDB_BUCKET               ", 4       , sizeof(CHASHDB_BUCKET)              , LOC_CREG_0147);

    creg_static_mem_tbl_add(MM_CDFSNP                       ,"MM_CDFSNP                       ", 4       , sizeof(CDFSNP)                      , LOC_CREG_0148);
    creg_static_mem_tbl_add(MM_CDFSNP_ITEM                  ,"MM_CDFSNP_ITEM                  ", 4       , sizeof(CDFSNP_ITEM)                 , LOC_CREG_0149);
    creg_static_mem_tbl_add(MM_CDFSNP_INODE                 ,"MM_CDFSNP_INODE                 ", 4       , sizeof(CDFSNP_INODE)                , LOC_CREG_0150);
    creg_static_mem_tbl_add(MM_CDFSNP_FNODE                 ,"MM_CDFSNP_FNODE                 ", 4       , sizeof(CDFSNP_FNODE)                , LOC_CREG_0151);
    creg_static_mem_tbl_add(MM_CDFSNP_DNODE                 ,"MM_CDFSNP_DNODE                 ", 4       , sizeof(CDFSNP_DNODE)                , LOC_CREG_0152);

    creg_static_mem_tbl_add(MM_CDFSDN_BUFF                  ,"MM_CDFSDN_BUFF                  ", 4       , sizeof(CDFSDN_BUFF)                 , LOC_CREG_0153);
    creg_static_mem_tbl_add(MM_CDFSDN_CACHE                 ,"MM_CDFSDN_CACHE                 ", 4       , sizeof(CDFSDN_CACHE)                , LOC_CREG_0154);
    creg_static_mem_tbl_add(MM_CDFSDN_BLOCK                 ,"MM_CDFSDN_BLOCK                 ", 4       , sizeof(CDFSDN_BLOCK)                , LOC_CREG_0155);
    creg_static_mem_tbl_add(MM_CDFSDN                       ,"MM_CDFSDN                       ", 4       , sizeof(CDFSDN)                      , LOC_CREG_0156);
    creg_static_mem_tbl_add(MM_CDFSDN_RECORD                ,"MM_CDFSDN_RECORD                ", 4       , sizeof(CDFSDN_RECORD)               , LOC_CREG_0157);
    creg_static_mem_tbl_add(MM_CDFSDN_RECORD_MGR            ,"MM_CDFSDN_RECORD_MGR            ", 4       , sizeof(CDFSDN_RECORD_MGR)           , LOC_CREG_0158);
    creg_static_mem_tbl_add(MM_CDFS_BUFF                    ,"MM_CDFS_BUFF                    ", 4       , sizeof(CDFS_BUFF)                   , LOC_CREG_0159);

    creg_static_mem_tbl_add(MM_CLOAD_STAT                   ,"MM_CLOAD_STAT                   ", 8       , sizeof(CLOAD_STAT)                  , LOC_CREG_0160);
    creg_static_mem_tbl_add(MM_CLOAD_NODE                   ,"MM_CLOAD_NODE                   ", 8       , sizeof(CLOAD_NODE)                  , LOC_CREG_0161);

    creg_static_mem_tbl_add(MM_CDFSDN_STAT                  ,"MM_CDFSDN_STAT                  ", 8       , sizeof(CDFSDN_STAT)                 , LOC_CREG_0162);

    return (EC_TRUE);
}
#endif

#ifdef __cplusplus
}
#endif/*__cplusplus*/

