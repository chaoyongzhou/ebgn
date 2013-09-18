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

#ifndef _LIB_MOD_H
#define _LIB_MOD_H

#include <stdio.h>
#include <stdlib.h>

#include "lib_type.h"


/*--------------------------------------------- interface -----------------------------------------*/

void *mod_mgr_new(const UINT32 local_md_id, const UINT32 load_balancing_choice);

void mod_mgr_free(void *mod_mgr);

EC_BOOL mod_mgr_init(void * mod_mgr, const UINT32 local_md_id, const UINT32 load_balancing_choice);

EC_BOOL mod_mgr_default_init(void *mod_mgr_def, const UINT32 tcid, const UINT32 rank);

EC_BOOL mod_mgr_default_sync(const UINT32 max_hops, const UINT32 max_remotes, const UINT32 time_to_live, void *mod_mgr_def);

EC_BOOL mod_mgr_limited_clone(const UINT32 mod_id, const void * src_mod_mgr, void *des_mod_mgr);

EC_BOOL mod_mgr_print(LOG *log, const void * mod_mgr);

UINT32 mod_mgr_incl(const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi, void *mod_mgr);
UINT32 mod_mgr_excl(const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi, void *mod_mgr);

/*simple interface of run throung remote mod_node list while NOT consider local mod_node*/
UINT32 mod_mgr_remote_num(const void *mod_mgr, UINT32 *remote_mod_node_num);
EC_BOOL mod_mgr_first_remote(const void *mod_mgr, UINT32 *remote_mod_node_pos);
EC_BOOL mod_mgr_last_remote(const void *mod_mgr, UINT32 *remote_mod_node_pos);
EC_BOOL mod_mgr_next_remote(const void *mod_mgr, UINT32 *remote_mod_node_pos);
EC_BOOL mod_mgr_prev_remote(const void *mod_mgr, UINT32 *remote_mod_node_pos);
#endif /*_LIB_MOD_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

