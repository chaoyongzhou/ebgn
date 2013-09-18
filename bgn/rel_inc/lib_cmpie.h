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

#ifndef _LIB_CMPIE_H
#define _LIB_CMPIE_H

#include "lib_type.h"

#include "lib_task.h"

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

UINT32 cmpi_encode_poly(const UINT32 comm, const POLY *poly, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_poly(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, POLY *poly);
UINT32 cmpi_encode_poly_size(const UINT32 comm, const POLY *poly, UINT32 *size);

UINT32 cmpi_encode_vectorr(const UINT32 comm, const VECTOR *vectorr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_vectorr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, VECTOR *vectorr);
UINT32 cmpi_encode_vectorr_size(const UINT32 comm, const VECTOR *vectorr, UINT32 *size);

UINT32 cmpi_encode_matrixr(const UINT32 comm, const MATRIX *matrixr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_matrixr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MATRIX *matrixr);
UINT32 cmpi_encode_matrixr_size(const UINT32 comm, const MATRIX *matrixr, UINT32 *size);

UINT32 cmpi_encode_mod_mgr(const UINT32 comm, const void *mod_mgr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_decode_mod_mgr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *mod_mgr);
UINT32 cmpi_encode_mod_mgr_size(const UINT32 comm, const void *mod_mgr, UINT32 *size);

UINT32 cmpi_encode_cstring(const UINT32 comm, const CSTRING *cstring, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cstring_size(const UINT32 comm, const CSTRING *cstring, UINT32 *size);
UINT32 cmpi_decode_cstring(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSTRING *cstring);

UINT32 cmpi_encode_taskc_node(const UINT32 comm, const void *taskc_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_taskc_node_size(const UINT32 comm, const void *taskc_node, UINT32 *size);
UINT32 cmpi_decode_taskc_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *taskc_node);

UINT32 cmpi_encode_taskc_mgr(const UINT32 comm, const void *taskc_mgr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_taskc_mgr_size(const UINT32 comm, const void *taskc_mgr, UINT32 *size);
UINT32 cmpi_decode_taskc_mgr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *taskc_mgr);

UINT32 cmpi_encode_vmm_node(const UINT32 comm, const void *vmm_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_vmm_node_size(const UINT32 comm, const void *vmm_node, UINT32 *size);
UINT32 cmpi_decode_vmm_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *vmm_node);

UINT32 cmpi_encode_log(const UINT32 comm, const LOG *log, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_log_size(const UINT32 comm, const LOG *log, UINT32 *size);
UINT32 cmpi_decode_log(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, LOG *log);

UINT32 cmpi_encode_kbuff(const UINT32 comm, const void *kbuff, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_kbuff_size(const UINT32 comm, const void *kbuff, UINT32 *size);
UINT32 cmpi_decode_kbuff(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *kbuff);

UINT32 cmpi_encode_cvector(const UINT32 comm, const void *cvector, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cvector_size(const UINT32 comm, const void *cvector, UINT32 *size);
UINT32 cmpi_decode_cvector(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cvector);

UINT32 cmpi_encode_csocket_cnode(const UINT32 comm, const void *csocket_cnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csocket_cnode_size(const UINT32 comm, const void *csocket_cnode, UINT32 *size);
UINT32 cmpi_decode_csocket_cnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *csocket_cnode);

UINT32 cmpi_encode_cmon_obj(const UINT32 comm, const void *cmon_obj, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cmon_obj_size(const UINT32 comm, const void *cmon_obj, UINT32 *size);
UINT32 cmpi_decode_cmon_obj(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cmon_obj);

UINT32 cmpi_encode_cmon_obj_vec(const UINT32 comm, const void *cmon_obj_vec, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cmon_obj_vec_size(const UINT32 comm, const void *cmon_obj_vec, UINT32 *size);
UINT32 cmpi_decode_cmon_obj_vec(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cmon_obj_vec);

UINT32 cmpi_encode_csys_cpu_stat(const UINT32 comm, const void *csys_cpu_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csys_cpu_stat_size(const UINT32 comm, const void *csys_cpu_stat, UINT32 *size);
UINT32 cmpi_decode_csys_cpu_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *csys_cpu_stat);

UINT32 cmpi_encode_mm_man_occupy_node(const UINT32 comm, const void *mm_man_occupy_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_mm_man_occupy_node_size(const UINT32 comm, const void *mm_man_occupy_node, UINT32 *size);
UINT32 cmpi_decode_mm_man_occupy_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *mm_man_occupy_node);

UINT32 cmpi_encode_mm_man_load_node(const UINT32 comm, const void *mm_man_load_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_mm_man_load_node_size(const UINT32 comm, const void *mm_man_load_node, UINT32 *size);
UINT32 cmpi_decode_mm_man_load_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *mm_man_load_node);

UINT32 cmpi_encode_csocket_status(const UINT32 comm, const void *csocket_status, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csocket_status_size(const UINT32 comm, const void *csocket_status, UINT32 *size);
UINT32 cmpi_decode_csocket_status(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *csocket_status);

UINT32 cmpi_encode_xmod_node(const UINT32 comm, const void *mod_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_xmod_node_size(const UINT32 comm, const void *mod_node, UINT32 *size);
UINT32 cmpi_decode_xmod_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *mod_node);

UINT32 cmpi_encode_cproc_module_stat(const UINT32 comm, const void *cproc_module_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cproc_module_stat_size(const UINT32 comm, const void *cproc_module_stat, UINT32 *size);
UINT32 cmpi_decode_cproc_module_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cproc_module_stat);

UINT32 cmpi_encode_crank_thread_stat(const UINT32 comm, const void *crank_thread_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_crank_thread_stat_size(const UINT32 comm, const void *crank_thread_stat, UINT32 *size);
UINT32 cmpi_decode_crank_thread_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *crank_thread_stat);

UINT32 cmpi_encode_csys_eth_stat(const UINT32 comm, const void *csys_eth_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csys_eth_stat_size(const UINT32 comm, const void *csys_eth_stat, UINT32 *size);
UINT32 cmpi_decode_csys_eth_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *csys_eth_stat);

UINT32 cmpi_encode_csys_dsk_stat(const UINT32 comm, const void *csys_dsk_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_csys_dsk_stat_size(const UINT32 comm, const void *csys_dsk_stat, UINT32 *size);
UINT32 cmpi_decode_csys_dsk_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *csys_dsk_stat);

UINT32 cmpi_encode_task_time_fmt(const UINT32 comm, const void *task_time_fmt, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_task_time_fmt_size(const UINT32 comm, const void *task_time_fmt, UINT32 *size);
UINT32 cmpi_decode_task_time_fmt(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *task_time_fmt);

UINT32 cmpi_encode_task_report_node(const UINT32 comm, const void *task_report_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_task_report_node_size(const UINT32 comm, const void *task_report_node, UINT32 *size);
UINT32 cmpi_decode_task_report_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *task_report_node);

UINT32 cmpi_encode_cdfs_buff(const UINT32 comm, const void *cdfs_buff, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfs_buff_size(const UINT32 comm, const void *cdfs_buff, UINT32 *size);
UINT32 cmpi_decode_cdfs_buff(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cdfs_buff);

UINT32 cmpi_encode_cdfsnp_inode(const UINT32 comm, const void *cdfsnp_inode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsnp_inode_size(const UINT32 comm, const void *cdfsnp_inode, UINT32 *size);
UINT32 cmpi_decode_cdfsnp_inode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cdfsnp_inode);

UINT32 cmpi_encode_cdfsnp_fnode(const UINT32 comm, const void *cdfsnp_fnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsnp_fnode_size(const UINT32 comm, const void *cdfsnp_fnode, UINT32 *size);
UINT32 cmpi_decode_cdfsnp_fnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cdfsnp_fnode);

UINT32 cmpi_encode_cdfsnp_item(const UINT32 comm, const void *cdfsnp_item, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsnp_item_size(const UINT32 comm, const void *cdfsnp_item, UINT32 *size);
UINT32 cmpi_decode_cdfsnp_item(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cdfsnp_item);

UINT32 cmpi_encode_cdfsdn_stat(const UINT32 comm, const void *cdfsdn_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cdfsdn_stat_size(const UINT32 comm, const void *cdfsdn_stat, UINT32 *size);
UINT32 cmpi_decode_cdfsdn_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cdfsdn_stat);

UINT32 cmpi_encode_cload_stat(const UINT32 comm, const void *cload_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cload_stat_size(const UINT32 comm, const void *cload_stat, UINT32 *size);
UINT32 cmpi_decode_cload_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cload_stat);

UINT32 cmpi_encode_cload_node(const UINT32 comm, const void *cload_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cload_node_size(const UINT32 comm, const void *cload_node, UINT32 *size);
UINT32 cmpi_decode_cload_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cload_node);

UINT32 cmpi_encode_cload_mgr(const UINT32 comm, const void *cload_mgr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position);
UINT32 cmpi_encode_cload_mgr_size(const UINT32 comm, const void *cload_mgr, UINT32 *size);
UINT32 cmpi_decode_cload_mgr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, void *cload_mgr);

#endif/*_LIB_CMPIE_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

