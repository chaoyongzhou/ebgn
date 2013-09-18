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

#ifndef _CMPIE_H
#define _CMPIE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "type.h"

#include "cvector.h"

#include "task.h"
#include "super.h"
#include "vmm.h"

#include "kbuff.h"
#include "cfile.h"
#include "cdir.h"
#include "cdfs.h"
#include "cdfsnp.h"
#include "cdfsdn.h"
#include "csocket.h"

#include "csys.h"
#include "cmon.h"
#include "cload.h"
#include "cfuse.h"
#include "cbytes.h"
#include "csession.h"
#include "cscore.h"

UINT32 cmpi_encode_uint8(const UINT32 comm, const UINT8 num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_uint8_ptr(const UINT32 comm, const UINT8 *num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_uint8(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT8 *num);
UINT32 cmpi_encode_uint8_size(const UINT32 comm, const UINT8 num, UINT32 *size);
UINT32 cmpi_encode_uint8_ptr_size(const UINT32 comm, const UINT8 *num, UINT32 *size);

UINT32 cmpi_encode_uint16(const UINT32 comm, const UINT16 num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_uint16_ptr(const UINT32 comm, const UINT16 *num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_uint16(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT16 *num);
UINT32 cmpi_encode_uint16_size(const UINT32 comm, const UINT16 num, UINT32 *size);
UINT32 cmpi_encode_uint16_ptr_size(const UINT32 comm, const UINT16 *num, UINT32 *size);

UINT32 cmpi_encode_uint32(const UINT32 comm, const UINT32 num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_uint32_ptr(const UINT32 comm, const UINT32 *num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_uint32(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT32 *num);
UINT32 cmpi_encode_uint32_size(const UINT32 comm, const UINT32 num, UINT32 *size);
UINT32 cmpi_encode_uint32_ptr_size(const UINT32 comm, const UINT32 *num, UINT32 *size);

UINT32 cmpi_encode_real(const UINT32 comm, const REAL *real, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_real(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, REAL *real);
UINT32 cmpi_encode_real_size(const UINT32 comm, const REAL *real, UINT32 *size);

UINT32 cmpi_encode_macaddr(const UINT32 comm, const UINT8 *macaddr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_macaddr_size(const UINT32 comm, const UINT8 *macaddr, UINT32 *size);
UINT32 cmpi_decode_macaddr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT8 *macaddr);

/*internal interface only: beg*/
UINT32 cmpi_encode_uint8_array(const UINT32 comm, const UINT8 *num, const UINT32 len, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_uint8_array(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT8 *num, UINT32 *len);
UINT32 cmpi_encode_uint8_array_size(const UINT32 comm, const UINT8 *num, const UINT32 len, UINT32 *size);

UINT32 cmpi_encode_uint16_array(const UINT32 comm, const UINT16 *num, const UINT32 len, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_uint16_array(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT16 *num, UINT32 *len);
UINT32 cmpi_encode_uint16_array_size(const UINT32 comm, const UINT16 *num, const UINT32 len, UINT32 *size);

UINT32 cmpi_encode_uint32_array(const UINT32 comm, const UINT32 *num, const UINT32 len, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_uint32_array(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT32 *num, UINT32 *len);
UINT32 cmpi_encode_uint32_array_size(const UINT32 comm, const UINT32 *num, const UINT32 len, UINT32 *size);

UINT32 cmpi_encode_real_array(const UINT32 comm, const REAL *real, const UINT32 len, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_real_array(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, REAL *real, UINT32 *len);
UINT32 cmpi_encode_real_array_size(const UINT32 comm, const REAL *real, const UINT32 len, UINT32 *size);
/*internal interface only: end*/

UINT32 cmpi_encode_bgn(const UINT32 comm, const BIGINT *bgn, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_bgn(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, BIGINT *bgn);
UINT32 cmpi_encode_bgn_size(const UINT32 comm, const BIGINT *bgn, UINT32 *size);

UINT32 cmpi_encode_ebgn(const UINT32 comm, const EBIGINT *ebgn, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_ebgn(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, EBIGINT *ebgn);
UINT32 cmpi_encode_ebgn_size(const UINT32 comm, const EBIGINT *ebgn, UINT32 *size);

UINT32 cmpi_encode_ec_curve_point(const UINT32 comm, const EC_CURVE_POINT *ec_curve_point, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_ec_curve_point(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, EC_CURVE_POINT *ec_curve_point);
UINT32 cmpi_encode_ec_curve_point_size(const UINT32 comm, const EC_CURVE_POINT *ec_curve_point, UINT32 *size);

UINT32 cmpi_encode_ec_curve_aff_point(const UINT32 comm, const EC_CURVE_AFF_POINT *ec_curve_aff_point, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_ec_curve_aff_point(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, EC_CURVE_AFF_POINT *ec_curve_aff_point);
UINT32 cmpi_encode_ec_curve_aff_point_size(const UINT32 comm, const EC_CURVE_AFF_POINT *ec_curve_aff_point, UINT32 *size);

UINT32 cmpi_encode_ecf2n_curve(const UINT32 comm, const ECF2N_CURVE *ecf2n_curve, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_ecf2n_curve(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, ECF2N_CURVE *ecf2n_curve);
UINT32 cmpi_encode_ecf2n_curve_size(const UINT32 comm, const ECF2N_CURVE *ecf2n_curve, UINT32 *size);

UINT32 cmpi_encode_ecfp_curve(const UINT32 comm, const ECFP_CURVE *ecfp_curve, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_ecfp_curve(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, ECFP_CURVE *ecfp_curve);
UINT32 cmpi_encode_ecfp_curve_size(const UINT32 comm, const ECFP_CURVE *ecfp_curve, UINT32 *size);

UINT32 cmpi_encode_ecc_keypair(const UINT32 comm, const ECC_KEYPAIR *ecc_keypair, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_ecc_keypair(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, ECC_KEYPAIR *ecc_keypair);
UINT32 cmpi_encode_ecc_keypair_size(const UINT32 comm, const ECC_KEYPAIR *ecc_keypair, UINT32 *size);

UINT32 cmpi_encode_ecc_signature(const UINT32 comm, const ECC_SIGNATURE *ecc_signature, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_ecc_signature(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, ECC_SIGNATURE *ecc_signature);
UINT32 cmpi_encode_ecc_signature_size(const UINT32 comm, const ECC_SIGNATURE *ecc_signature, UINT32 *size);

UINT32 cmpi_encode_degree(const UINT32 comm, const DEGREE *degree, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_degree(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, DEGREE *degree);
UINT32 cmpi_encode_degree_size(const UINT32 comm, const DEGREE *degree, UINT32 *size);

UINT32 cmpi_encode_poly_item(const UINT32 comm, const POLY_ITEM *poly_item, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_poly_item(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, POLY_ITEM *poly_item);
UINT32 cmpi_encode_poly_item_size(const UINT32 comm, const POLY_ITEM *poly_item, UINT32 *size);

UINT32 cmpi_encode_poly(const UINT32 comm, const POLY *poly, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_poly(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, POLY *poly);
UINT32 cmpi_encode_poly_size(const UINT32 comm, const POLY *poly, UINT32 *size);

UINT32 cmpi_encode_vectorr_block(const UINT32 comm, const VECTOR_BLOCK *vectorr_block, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_vectorr_block(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, VECTOR_BLOCK *vectorr_block);
UINT32 cmpi_encode_vectorr_block_size(const UINT32 comm, const VECTOR_BLOCK *vectorr_block, UINT32 *size);

UINT32 cmpi_encode_vectorr(const UINT32 comm, const VECTOR *vectorr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_vectorr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, VECTOR *vectorr);
UINT32 cmpi_encode_vectorr_size(const UINT32 comm, const VECTOR *vectorr, UINT32 *size);

UINT32 cmpi_encode_matrixr_block(const UINT32 comm, const MATRIX_BLOCK *matrixr_block, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_matrixr_block(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MATRIX_BLOCK *matrixr_block);
UINT32 cmpi_encode_matrixr_block_size(const UINT32 comm, const MATRIX_BLOCK *matrixr_block, UINT32 *size);

UINT32 cmpi_encode_matrixr(const UINT32 comm, const MATRIX *matrixr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_matrixr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MATRIX *matrixr);
UINT32 cmpi_encode_matrixr_size(const UINT32 comm, const MATRIX *matrixr, UINT32 *size);

UINT32 cmpi_encode_mod_node(const UINT32 comm, const MOD_NODE *mod_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_mod_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MOD_NODE *mod_node);
UINT32 cmpi_encode_mod_node_size(const UINT32 comm, const MOD_NODE *mod_node, UINT32 *size);

UINT32 cmpi_encode_mod_mgr(const UINT32 comm, const MOD_MGR *mod_mgr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_mod_mgr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MOD_MGR *mod_mgr);
UINT32 cmpi_encode_mod_mgr_size(const UINT32 comm, const MOD_MGR *mod_mgr, UINT32 *size);

UINT32 cmpi_encode_cstring(const UINT32 comm, const CSTRING *cstring, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cstring_size(const UINT32 comm, const CSTRING *cstring, UINT32 *size);
UINT32 cmpi_decode_cstring(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSTRING *cstring);

UINT32 cmpi_encode_taskc_node(const UINT32 comm, const TASKC_NODE *taskc_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_taskc_node_size(const UINT32 comm, const TASKC_NODE *taskc_node, UINT32 *size);
UINT32 cmpi_decode_taskc_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, TASKC_NODE *taskc_node);

UINT32 cmpi_encode_taskc_mgr(const UINT32 comm, const TASKC_MGR *taskc_mgr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_taskc_mgr_size(const UINT32 comm, const TASKC_MGR *taskc_mgr, UINT32 *size);
UINT32 cmpi_decode_taskc_mgr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, TASKC_MGR *taskc_mgr);

UINT32 cmpi_encode_vmm_node(const UINT32 comm, const VMM_NODE *vmm_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_vmm_node_size(const UINT32 comm, const VMM_NODE *vmm_node, UINT32 *size);
UINT32 cmpi_decode_vmm_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, VMM_NODE *vmm_node);

UINT32 cmpi_encode_log(const UINT32 comm, const LOG *log, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_log_size(const UINT32 comm, const LOG *log, UINT32 *size);
UINT32 cmpi_decode_log(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, LOG *log);

UINT32 cmpi_encode_kbuff(const UINT32 comm, const KBUFF *kbuff, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_kbuff_size(const UINT32 comm, const KBUFF *kbuff, UINT32 *size);
UINT32 cmpi_decode_kbuff(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, KBUFF *kbuff);

UINT32 cmpi_encode_cfile_seg(const UINT32 comm, const CFILE_SEG *cfile_seg, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cfile_seg_size(const UINT32 comm, const CFILE_SEG *cfile_seg, UINT32 *size);
UINT32 cmpi_decode_cfile_seg(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFILE_SEG *cfile_seg);
/*
UINT32 cmpi_encode_cfile_seg_vec(const UINT32 comm, const CFILE_SEG_VEC *cfile_seg_vec, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cfile_seg_vec_size(const UINT32 comm, const CFILE_SEG_VEC *cfile_seg_vec, UINT32 *size);
UINT32 cmpi_decode_cfile_seg_vec(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFILE_SEG_VEC *cfile_seg_vec);
*/
UINT32 cmpi_encode_cfile_node(const UINT32 comm, const CFILE_NODE *cfile_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cfile_node_size(const UINT32 comm, const CFILE_NODE *cfile_node, UINT32 *size);
UINT32 cmpi_decode_cfile_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFILE_NODE *cfile_node);

UINT32 cmpi_encode_cdir_seg(const UINT32 comm, const CDIR_SEG *cdir_seg, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdir_seg_size(const UINT32 comm, const CDIR_SEG *cdir_seg, UINT32 *size);
UINT32 cmpi_decode_cdir_seg(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDIR_SEG *cdir_seg);

UINT32 cmpi_encode_cdir_node(const UINT32 comm, const CDIR_NODE *cdir_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdir_node_size(const UINT32 comm, const CDIR_NODE *cdir_node, UINT32 *size);
UINT32 cmpi_decode_cdir_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDIR_NODE *cdir_node);


/*codec of cvector: must define data_encoder, data_encoder_size, data_decoder, data_init*/
UINT32 cmpi_encode_cvector(const UINT32 comm, const CVECTOR *cvector, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cvector_size(const UINT32 comm, const CVECTOR *cvector, UINT32 *size);
UINT32 cmpi_decode_cvector(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CVECTOR *cvector);

UINT32 cmpi_encode_csocket_cnode(const UINT32 comm, const CSOCKET_CNODE *csocket_cnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csocket_cnode_size(const UINT32 comm, const CSOCKET_CNODE *csocket_cnode, UINT32 *size);
UINT32 cmpi_decode_csocket_cnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSOCKET_CNODE *csocket_cnode);

UINT32 cmpi_encode_cmon_obj(const UINT32 comm, const CMON_OBJ *cmon_obj, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cmon_obj_size(const UINT32 comm, const CMON_OBJ *cmon_obj, UINT32 *size);
UINT32 cmpi_decode_cmon_obj(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CMON_OBJ *cmon_obj);

UINT32 cmpi_encode_cmon_obj_vec(const UINT32 comm, const CMON_OBJ_VEC *cmon_obj_vec, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cmon_obj_vec_size(const UINT32 comm, const CMON_OBJ_VEC *cmon_obj_vec, UINT32 *size);
UINT32 cmpi_decode_cmon_obj_vec(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CMON_OBJ_VEC *cmon_obj_vec);

UINT32 cmpi_encode_csys_cpu_stat(const UINT32 comm, const CSYS_CPU_STAT *csys_cpu_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csys_cpu_stat_size(const UINT32 comm, const CSYS_CPU_STAT *csys_cpu_stat, UINT32 *size);
UINT32 cmpi_decode_csys_cpu_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSYS_CPU_STAT *csys_cpu_stat);

UINT32 cmpi_encode_mm_man_occupy_node(const UINT32 comm, const MM_MAN_OCCUPY_NODE *mm_man_occupy_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_mm_man_occupy_node_size(const UINT32 comm, const MM_MAN_OCCUPY_NODE *mm_man_occupy_node, UINT32 *size);
UINT32 cmpi_decode_mm_man_occupy_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MM_MAN_OCCUPY_NODE *mm_man_occupy_node);

UINT32 cmpi_encode_mm_man_load_node(const UINT32 comm, const MM_MAN_LOAD_NODE *mm_man_load_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_mm_man_load_node_size(const UINT32 comm, const MM_MAN_LOAD_NODE *mm_man_load_node, UINT32 *size);
UINT32 cmpi_decode_mm_man_load_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MM_MAN_LOAD_NODE *mm_man_load_node);

UINT32 cmpi_encode_xmod_node(const UINT32 comm, const MOD_NODE *mod_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_xmod_node_size(const UINT32 comm, const MOD_NODE *mod_node, UINT32 *size);
UINT32 cmpi_decode_xmod_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MOD_NODE *mod_node);

UINT32 cmpi_encode_cproc_module_stat(const UINT32 comm, const CPROC_MODULE_STAT *cproc_module_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cproc_module_stat_size(const UINT32 comm, const CPROC_MODULE_STAT *cproc_module_stat, UINT32 *size);
UINT32 cmpi_decode_cproc_module_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CPROC_MODULE_STAT *cproc_module_stat);

UINT32 cmpi_encode_crank_thread_stat(const UINT32 comm, const CRANK_THREAD_STAT *crank_thread_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_crank_thread_stat_size(const UINT32 comm, const CRANK_THREAD_STAT *crank_thread_stat, UINT32 *size);
UINT32 cmpi_decode_crank_thread_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRANK_THREAD_STAT *crank_thread_stat);

UINT32 cmpi_encode_csys_eth_stat(const UINT32 comm, const CSYS_ETH_STAT *csys_eth_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csys_eth_stat_size(const UINT32 comm, const CSYS_ETH_STAT *csys_eth_stat, UINT32 *size);
UINT32 cmpi_decode_csys_eth_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSYS_ETH_STAT *csys_eth_stat);

UINT32 cmpi_encode_csys_dsk_stat(const UINT32 comm, const CSYS_DSK_STAT *csys_dsk_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csys_dsk_stat_size(const UINT32 comm, const CSYS_DSK_STAT *csys_dsk_stat, UINT32 *size);
UINT32 cmpi_decode_csys_dsk_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSYS_DSK_STAT *csys_dsk_stat);

UINT32 cmpi_encode_task_time_fmt(const UINT32 comm, const TASK_TIME_FMT *task_time_fmt, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_task_time_fmt_size(const UINT32 comm, const TASK_TIME_FMT *task_time_fmt, UINT32 *size);
UINT32 cmpi_decode_task_time_fmt(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, TASK_TIME_FMT *task_time_fmt);

UINT32 cmpi_encode_task_report_node(const UINT32 comm, const TASK_REPORT_NODE *task_report_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_task_report_node_size(const UINT32 comm, const TASK_REPORT_NODE *task_report_node, UINT32 *size);
UINT32 cmpi_decode_task_report_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, TASK_REPORT_NODE *task_report_node);

UINT32 cmpi_encode_cdfsnp_inode(const UINT32 comm, const CDFSNP_INODE *cdfsnp_inode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsnp_inode_size(const UINT32 comm, const CDFSNP_INODE *cdfsnp_inode, UINT32 *size);
UINT32 cmpi_decode_cdfsnp_inode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSNP_INODE *cdfsnp_inode);

UINT32 cmpi_encode_cdfsnp_fnode(const UINT32 comm, const CDFSNP_FNODE *cdfsnp_fnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsnp_fnode_size(const UINT32 comm, const CDFSNP_FNODE *cdfsnp_fnode, UINT32 *size);
UINT32 cmpi_decode_cdfsnp_fnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSNP_FNODE *cdfsnp_fnode);

UINT32 cmpi_encode_cdfsnp_item(const UINT32 comm, const CDFSNP_ITEM *cdfsnp_item, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsnp_item_size(const UINT32 comm, const CDFSNP_ITEM *cdfsnp_item, UINT32 *size);
UINT32 cmpi_decode_cdfsnp_item(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSNP_ITEM *cdfsnp_item);

UINT32 cmpi_encode_cdfsdn_stat(const UINT32 comm, const CDFSDN_STAT *cdfsdn_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsdn_stat_size(const UINT32 comm, const CDFSDN_STAT *cdfsdn_stat, UINT32 *size);
UINT32 cmpi_decode_cdfsdn_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSDN_STAT *cdfsdn_stat);

UINT32 cmpi_encode_cload_stat(const UINT32 comm, const CLOAD_STAT *cload_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cload_stat_size(const UINT32 comm, const CLOAD_STAT *cload_stat, UINT32 *size);
UINT32 cmpi_decode_cload_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CLOAD_STAT *cload_stat);

UINT32 cmpi_encode_cload_node(const UINT32 comm, const CLOAD_NODE *cload_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cload_node_size(const UINT32 comm, const CLOAD_NODE *cload_node, UINT32 *size);
UINT32 cmpi_decode_cload_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CLOAD_NODE *cload_node);

UINT32 cmpi_encode_cload_mgr(const UINT32 comm, const CLOAD_MGR *cload_mgr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cload_mgr_size(const UINT32 comm, const CLOAD_MGR *cload_mgr, UINT32 *size);
UINT32 cmpi_decode_cload_mgr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CLOAD_MGR *cload_mgr);

UINT32 cmpi_encode_cfuse_mode(const UINT32 comm, const CFUSE_MODE *cfuse_mode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cfuse_mode_size(const UINT32 comm, const CFUSE_MODE *cfuse_mode, UINT32 *size);
UINT32 cmpi_decode_cfuse_mode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFUSE_MODE *cfuse_mode);

UINT32 cmpi_encode_cfuse_stat(const UINT32 comm, const CFUSE_STAT *cfuse_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cfuse_stat_size(const UINT32 comm, const CFUSE_STAT *cfuse_stat, UINT32 *size);
UINT32 cmpi_decode_cfuse_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFUSE_STAT *cfuse_stat);

UINT32 cmpi_encode_cdfsdn_record(const UINT32 comm, const CDFSDN_RECORD *cdfsdn_record, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsdn_record_size(const UINT32 comm, const CDFSDN_RECORD *cdfsdn_record, UINT32 *size);
UINT32 cmpi_decode_cdfsdn_record(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSDN_RECORD *cdfsdn_record);

UINT32 cmpi_encode_cdfsdn_block(const UINT32 comm, const CDFSDN_BLOCK *cdfsdn_block, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsdn_block_size(const UINT32 comm, const CDFSDN_BLOCK *cdfsdn_block, UINT32 *size);
UINT32 cmpi_decode_cdfsdn_block(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSDN_BLOCK *cdfsdn_block);

UINT32 cmpi_encode_cbytes(const UINT32 comm, const CBYTES *cbytes, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cbytes_size(const UINT32 comm, const CBYTES *cbytes, UINT32 *size);
UINT32 cmpi_decode_cbytes(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CBYTES *cbytes);

UINT32 cmpi_encode_ctimet(const UINT32 comm, const CTIMET *ctimet, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_ctimet_size(const UINT32 comm, const CTIMET *ctimet, UINT32 *size);
UINT32 cmpi_decode_ctimet(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CTIMET *ctimet);

UINT32 cmpi_encode_csession_node(const UINT32 comm, const CSESSION_NODE *csession_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csession_node_size(const UINT32 comm, const CSESSION_NODE *csession_node, UINT32 *size);
UINT32 cmpi_decode_csession_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSESSION_NODE *csession_node);

UINT32 cmpi_encode_csession_item(const UINT32 comm, const CSESSION_ITEM *csession_item, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csession_item_size(const UINT32 comm, const CSESSION_ITEM *csession_item, UINT32 *size);
UINT32 cmpi_decode_csession_item(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSESSION_ITEM *csession_item);

UINT32 cmpi_encode_csword(const UINT32 comm, const CSWORD *csword, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csword_size(const UINT32 comm, const CSWORD *csword, UINT32 *size);
UINT32 cmpi_decode_csword(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSWORD *csword);

UINT32 cmpi_encode_csdoc(const UINT32 comm, const CSDOC *csdoc, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csdoc_size(const UINT32 comm, const CSDOC *csdoc, UINT32 *size);
UINT32 cmpi_decode_csdoc(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSDOC *csdoc);

UINT32 cmpi_encode_csdoc_words(const UINT32 comm, const CSDOC_WORDS *csdoc_words, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csdoc_words_size(const UINT32 comm, const CSDOC_WORDS *csdoc_words, UINT32 *size);
UINT32 cmpi_decode_csdoc_words(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSDOC_WORDS *csdoc_words);

UINT32 cmpi_encode_csword_docs(const UINT32 comm, const CSWORD_DOCS *csword_docs, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csword_docs_size(const UINT32 comm, const CSWORD_DOCS *csword_docs, UINT32 *size);
UINT32 cmpi_decode_csword_docs(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSWORD_DOCS *csword_docs);

UINT32 cmpi_encode_clist(const UINT32 comm, const CLIST *clist, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_clist_size(const UINT32 comm, const CLIST *clist, UINT32 *size);
UINT32 cmpi_decode_clist(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CLIST *clist);


#endif/*_CMPIE_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

