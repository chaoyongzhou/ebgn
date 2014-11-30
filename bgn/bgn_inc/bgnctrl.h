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

#ifndef _CONTROL_H
#define _CONTROL_H

/*Switch Status Definition*/
//#define SWITCH_ON  1
//#define SWITCH_OFF 0


/*Print Maple Debug Info switch*/
#define PRINT_MAPLE_INFO_SWITCH SWITCH_OFF

/*Bigint Package (special) Debug Switch*/
#define BIGINT_DEBUG_SWITCH SWITCH_ON

/*Bigint Length Checking Switch*/
#define BGN_LEN_CHECK_SWITCH SWITCH_ON

/*Super Package Debug Switch*/
#define SUPER_DEBUG_SWITCH SWITCH_ON

/*Poly Package Debug Switch*/
#define POLY_DEBUG_SWITCH SWITCH_ON

/*Matrix Package Debug Switch*/
#define MATRIX_DEBUG_SWITCH SWITCH_ON

/*CFile Package Debug Switch*/
#define CFILE_DEBUG_SWITCH SWITCH_ON

/*CDir Package Debug Switch*/
#define CDIR_DEBUG_SWITCH SWITCH_ON

/*GANGLIA Package Debug Switch*/
#define GANGLIA_DEBUG_SWITCH SWITCH_ON

/*CDFS Package Debug Switch*/
#define CDFS_DEBUG_SWITCH SWITCH_ON

/*CRFS Package Debug Switch*/
#define CRFS_DEBUG_SWITCH SWITCH_ON

/*CXMPPC2S Package Debug Switch*/
#define CXMPPC2S_DEBUG_SWITCH SWITCH_ON

/*CRFSC Package Debug Switch*/
#define CRFSC_DEBUG_SWITCH SWITCH_ON

/*CCURL Package Debug Switch*/
#define CCURL_DEBUG_SWITCH SWITCH_ON

/*CHFS Package Debug Switch*/
#define CHFS_DEBUG_SWITCH SWITCH_ON

/*CBGT Package Debug Switch*/
#define CBGT_DEBUG_SWITCH SWITCH_ON

/*CSESSION Package Debug Switch*/
#define CSESSION_DEBUG_SWITCH SWITCH_ON

/*CSCORE Package Debug Switch*/
#define CSCORE_DEBUG_SWITCH SWITCH_ON

/*CSOLR Package Debug Switch*/
#define CSOLR_DEBUG_SWITCH SWITCH_ON

/*CMon Package Debug Switch*/
#define CMON_DEBUG_SWITCH SWITCH_ON

/*CTimer Package Debug Switch*/
#define CTIMER_DEBUG_SWITCH SWITCH_ON

/*BIGINT Type of Poly Degree or Poly Item Degree Switch*/
#define POLY_DEG_IS_BGN_SWITCH SWITCH_ON

/*UINT32 Type of Poly Degree or Poly Item Degree Switch*/
#define POLY_DEG_IS_UINT32_SWITCH SWITCH_OFF

/*SEA Algorithm Package Debug Switch*/
#define SEA_DEBUG_SWITCH SWITCH_ON

/*ECC Basic Algorithm Debug Switch*/
#define ECC_DEBUG_SWITCH SWITCH_ON

/*ECC point multiplication for fixed base point */
#define ECC_POINT_MUL_FIXED_BASE_SWITCH SWITCH_ON

/*SHA-256 Debug Switch  */
#define SHA256_DEBUG_SWITCH SWITCH_ON

/*ECC Encryption and Decryption Debug Switch*/
#define ENC_DEC_DEBUG_SWITCH SWITCH_ON

/*ECDSA Debug Switch*/
#define ECDSA_DEBUG_SWITCH SWITCH_ON

/*VMM Debug Switch*/
#define VMM_DEBUG_SWITCH SWITCH_ON

/*SOFTREG Debug Switch*/
#define SOFTREG_DEBUG_SWITCH SWITCH_ON

/*Print Functions Debug Switch*/
#define PRINT_DEBUG_SWITCH SWITCH_ON

/*Convert Functions Debug Switch*/
#define CONV_DEBUG_SWITCH SWITCH_ON

/*Encode/Decode Functions Debug Switch*/
#define ENCODE_DEBUG_SWITCH SWITCH_ON

/*Task Functions Debug Switch*/
#define TASK_DEBUG_SWITCH SWITCH_ON

/*TASKC Functions Debug Switch*/
#define TASKC_DEBUG_SWITCH SWITCH_ON

/*Slower Multiplication Algorithm Switch*/
#define BIGINT_SLOW_MUL_SWITCH SWITCH_OFF

/*ECC Point Multiplication Control Switch*/
#define ECC_PM_CTRL_SWITCH SWITCH_OFF/*not support at present*/

/*Static Memory Control Switch*/
#define STATIC_MEMORY_SWITCH SWITCH_ON

/*Print Static Memory Stats Info Switch*/
#define STATIC_MEM_STATS_INFO_PRINT_SWITCH SWITCH_OFF

/*Static Memory Diagnostication Location Switch*/
#define STATIC_MEM_DIAG_LOC_SWITCH SWITCH_ON

/*Stack Memory Control Switch*/
#define STACK_MEMORY_SWITCH SWITCH_OFF

/*CLIST Memory Control Switch*/
#define CLIST_STATIC_MEM_SWITCH SWITCH_ON

/*CSET Memory Control Switch*/
#define CSET_STATIC_MEM_SWITCH SWITCH_OFF

/*CSTACK Memory Control Switch*/
#define CSTACK_STATIC_MEM_SWITCH SWITCH_OFF

/*CQUEUE Memory Control Switch*/
#define CQUEUE_STATIC_MEM_SWITCH SWITCH_OFF

#if (STATIC_MEMORY_SWITCH == STACK_MEMORY_SWITCH)
#error "fatal error: STATIC_MEMORY_SWITCH equal to STACK_MEMORY_SWITCH"
#endif/* STATIC_MEMORY_SWITCH == STACK_MEMORY_SWITCH */

#if (POLY_DEG_IS_BGN_SWITCH == POLY_DEG_IS_UINT32_SWITCH)
#error "fatal error: POLY_DEG_IS_BGN_SWITCH equal to POLY_DEG_IS_UINT32_SWITCH"
#endif/* POLY_DEG_IS_BGN_SWITCH == POLY_DEG_IS_UINT32_SWITCH */

/*MYSQL Debug Info Switch*/
#define MYSQL_DEBUG_INFO_SWITCH SWITCH_OFF

#define ASM_DISABLE_SWITCH SWITCH_ON


#endif /* _CONTROL_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

