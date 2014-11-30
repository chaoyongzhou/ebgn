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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "cmpic.inc"
#include "cmisc.h"

#include "crb.h"
#include "chashalgo.h"
#include "crfsconhash.h"

CRFSCONHASH_RNODE *crfsconhash_rnode_new()
{
    CRFSCONHASH_RNODE *crfsconhash_rnode;
    alloc_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSCONHASH_RNODE, &crfsconhash_rnode, LOC_CRFSCONHASH_0001);
    if(NULL_PTR != crfsconhash_rnode)
    {
        crfsconhash_rnode_init(crfsconhash_rnode);
    }
    return (crfsconhash_rnode);
}

CRFSCONHASH_RNODE *crfsconhash_rnode_make(const uint32_t tcid, const uint16_t replicas)
{
    CRFSCONHASH_RNODE *crfsconhash_rnode;
    alloc_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSCONHASH_RNODE, &crfsconhash_rnode, LOC_CRFSCONHASH_0002);
    if(NULL_PTR != crfsconhash_rnode)
    {
        CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode) = replicas;
        CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)     = tcid;
        
        CRFSCONHASH_RNODE_COUNTER_CLR(crfsconhash_rnode);
    }
    return (crfsconhash_rnode);
}

EC_BOOL crfsconhash_rnode_init(CRFSCONHASH_RNODE *crfsconhash_rnode)
{
    CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode) = 0;
    CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)     = (uint32_t)CMPI_ERROR_TCID;
    
    CRFSCONHASH_RNODE_COUNTER_CLR(crfsconhash_rnode);
    return (EC_TRUE);
}

EC_BOOL crfsconhash_rnode_clean(CRFSCONHASH_RNODE *crfsconhash_rnode)
{
    CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode) = 0;
    CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)     = (uint32_t)CMPI_ERROR_TCID;

    CRFSCONHASH_RNODE_COUNTER_CLR(crfsconhash_rnode);
    return (EC_TRUE);
}

EC_BOOL crfsconhash_rnode_free(CRFSCONHASH_RNODE *crfsconhash_rnode)
{
    if(NULL_PTR != crfsconhash_rnode)
    {
        crfsconhash_rnode_clean(crfsconhash_rnode);
        free_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSCONHASH_RNODE, crfsconhash_rnode, LOC_CRFSCONHASH_0003);
    }
    return (EC_TRUE);
}

EC_BOOL crfsconhash_rnode_init_0(const UINT32 md_id, CRFSCONHASH_RNODE *crfsconhash_rnode)
{
    return crfsconhash_rnode_init(crfsconhash_rnode);
}

EC_BOOL crfsconhash_rnode_clean_0(const UINT32 md_id, CRFSCONHASH_RNODE *crfsconhash_rnode)
{
    return crfsconhash_rnode_clean(crfsconhash_rnode);
}

EC_BOOL crfsconhash_rnode_free_0(const UINT32 md_id, CRFSCONHASH_RNODE *crfsconhash_rnode)
{
    return crfsconhash_rnode_free(crfsconhash_rnode);
}

EC_BOOL crfsconhash_rnode_clone(const CRFSCONHASH_RNODE *crfsconhash_rnode_src, CRFSCONHASH_RNODE *crfsconhash_rnode_des)
{
    CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode_des) = CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode_src);
    CRFSCONHASH_RNODE_TCID(crfsconhash_rnode_des)     = CRFSCONHASH_RNODE_TCID(crfsconhash_rnode_src);

    CRFSCONHASH_RNODE_COUNTER_CLONE(crfsconhash_rnode_src, crfsconhash_rnode_des);
    return (EC_TRUE);
}

EC_BOOL crfsconhash_rnode_is_equal(const CRFSCONHASH_RNODE *crfsconhash_rnode_1st, const CRFSCONHASH_RNODE *crfsconhash_rnode_2nd)
{
    if(NULL_PTR == crfsconhash_rnode_1st && NULL_PTR == crfsconhash_rnode_2nd)
    {
        return (EC_TRUE);
    }

    if(NULL_PTR == crfsconhash_rnode_1st || NULL_PTR == crfsconhash_rnode_2nd)
    {
        return (EC_FALSE);
    }

    if(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode_1st) != CRFSCONHASH_RNODE_TCID(crfsconhash_rnode_2nd))
    {
        return (EC_FALSE);
    }
#if 1
    if(CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode_1st) != CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode_2nd))
    {
        return (EC_FALSE);
    }
#endif    
    return (EC_TRUE);
}

EC_BOOL crfsconhash_rnode_cmp_tcid(const CRFSCONHASH_RNODE *crfsconhash_rnode_1st, const CRFSCONHASH_RNODE *crfsconhash_rnode_2nd)
{
    if(NULL_PTR == crfsconhash_rnode_1st && NULL_PTR == crfsconhash_rnode_2nd)
    {
        return (EC_TRUE);
    }

    if(NULL_PTR == crfsconhash_rnode_1st || NULL_PTR == crfsconhash_rnode_2nd)
    {
        return (EC_FALSE);
    }    
    
    if(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode_1st) != CRFSCONHASH_RNODE_TCID(crfsconhash_rnode_2nd))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

void crfsconhash_rnode_print(LOG *log, const CRFSCONHASH_RNODE *crfsconhash_rnode)
{
#if(SWITCH_OFF == CRFSCONHASH_RNODE_DEBUG)
    sys_log(log, "crfsconhash_rnode %p: tcid %s, replicas %u\n",
                    crfsconhash_rnode,
                    c_word_to_ipv4(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)),
                    CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode)
                   );
#endif/*(SWITCH_OFF == CRFSCONHASH_RNODE_DEBUG)*/                   
#if(SWITCH_ON == CRFSCONHASH_RNODE_DEBUG)
    sys_log(log, "crfsconhash_rnode %p: tcid %s, replicas %u, counter %ld\n",
                    crfsconhash_rnode,
                    c_word_to_ipv4(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)),
                    CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode),
                    CRFSCONHASH_RNODE_COUNTER(crfsconhash_rnode)
                   );
#endif/*(SWITCH_ON == CRFSCONHASH_RNODE_DEBUG)*/
    return;
}

CRFSCONHASH_VNODE *crfsconhash_vnode_new()
{
    CRFSCONHASH_VNODE *crfsconhash_vnode;
    alloc_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSCONHASH_VNODE, &crfsconhash_vnode, LOC_CRFSCONHASH_0004);
    if(NULL_PTR != crfsconhash_vnode)
    {
        crfsconhash_vnode_init(crfsconhash_vnode);
    }
    return (crfsconhash_vnode);
}

CRFSCONHASH_VNODE *crfsconhash_vnode_make(const uint32_t hash, const uint16_t rnode_pos)
{
    CRFSCONHASH_VNODE *crfsconhash_vnode;
    alloc_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSCONHASH_VNODE, &crfsconhash_vnode, LOC_CRFSCONHASH_0005);
    if(NULL_PTR != crfsconhash_vnode)
    {
        CRFSCONHASH_VNODE_HASH(crfsconhash_vnode) = hash;
        CRFSCONHASH_VNODE_POS(crfsconhash_vnode)  = rnode_pos;
    }
    return (crfsconhash_vnode);
}

EC_BOOL crfsconhash_vnode_init(CRFSCONHASH_VNODE *crfsconhash_vnode)
{
    CRFSCONHASH_VNODE_HASH(crfsconhash_vnode) = 0;
    CRFSCONHASH_VNODE_POS(crfsconhash_vnode)  = (uint32_t)CVECTOR_ERR_POS;
    return (EC_TRUE);
}

EC_BOOL crfsconhash_vnode_clean(CRFSCONHASH_VNODE *crfsconhash_vnode)
{
    CRFSCONHASH_VNODE_HASH(crfsconhash_vnode) = 0;
    CRFSCONHASH_VNODE_POS(crfsconhash_vnode)  = (uint32_t)CVECTOR_ERR_POS;
    return (EC_TRUE);
}

EC_BOOL crfsconhash_vnode_free(CRFSCONHASH_VNODE *crfsconhash_vnode)
{
    if(NULL_PTR != crfsconhash_vnode)
    {
        crfsconhash_vnode_clean(crfsconhash_vnode);
        free_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSCONHASH_VNODE, crfsconhash_vnode, LOC_CRFSCONHASH_0006);
    }
    return (EC_TRUE);
}

EC_BOOL crfsconhash_vnode_init_0(const UINT32 md_id, CRFSCONHASH_VNODE *crfsconhash_vnode)
{
    return crfsconhash_vnode_init(crfsconhash_vnode);
}

EC_BOOL crfsconhash_vnode_clean_0(const UINT32 md_id, CRFSCONHASH_VNODE *crfsconhash_vnode)
{
    return crfsconhash_vnode_clean(crfsconhash_vnode);
}

EC_BOOL crfsconhash_vnode_free_0(const UINT32 md_id, CRFSCONHASH_VNODE *crfsconhash_vnode)
{
    return crfsconhash_vnode_free(crfsconhash_vnode);
}

EC_BOOL crfsconhash_vnode_clone(const CRFSCONHASH_VNODE *crfsconhash_vnode_src, CRFSCONHASH_VNODE *crfsconhash_vnode_des)
{
    CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_des) = CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_src);
    CRFSCONHASH_VNODE_POS(crfsconhash_vnode_des)  = CRFSCONHASH_VNODE_POS(crfsconhash_vnode_src);
    return (EC_TRUE);
}

EC_BOOL crfsconhash_vnode_is_equal(const CRFSCONHASH_VNODE *crfsconhash_vnode_1st, const CRFSCONHASH_VNODE *crfsconhash_vnode_2nd)
{
    if(NULL_PTR == crfsconhash_vnode_1st && NULL_PTR == crfsconhash_vnode_2nd)
    {
        return (EC_TRUE);
    }

    if(NULL_PTR == crfsconhash_vnode_1st || NULL_PTR == crfsconhash_vnode_2nd)
    {
        return (EC_FALSE);
    }

    if(do_log(SEC_0144_CRFSCONHASH, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] crfsconhash_vnode_is_equal: check them:\n");
        crfsconhash_vnode_print(LOGSTDOUT, crfsconhash_vnode_1st);
        crfsconhash_vnode_print(LOGSTDOUT, crfsconhash_vnode_2nd);
    }

    if(CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_1st) != CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_2nd))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 6)(LOGSTDOUT, "[DEBUG] crfsconhash_vnode_is_equal: hash: %x != %x\n", 
                           CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_1st),
                           CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_2nd));
        return (EC_FALSE);
    }
#if 1
    if(CRFSCONHASH_VNODE_POS(crfsconhash_vnode_1st) != CRFSCONHASH_VNODE_POS(crfsconhash_vnode_2nd))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 6)(LOGSTDOUT, "[DEBUG] crfsconhash_vnode_is_equal: pos: %u != %u\n", 
                           CRFSCONHASH_VNODE_POS(crfsconhash_vnode_1st),
                           CRFSCONHASH_VNODE_POS(crfsconhash_vnode_2nd));    
        return (EC_FALSE);
    }
#endif    
    return (EC_TRUE);
}

int crfsconhash_vnode_cmp(const CRFSCONHASH_VNODE *crfsconhash_vnode_1st, const CRFSCONHASH_VNODE *crfsconhash_vnode_2nd)
{
    if(CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_1st) > CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_2nd))
    {
        return (1);
    }

    if(CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_1st) < CRFSCONHASH_VNODE_HASH(crfsconhash_vnode_2nd))
    {
        return (-1);
    }    
    return (0);
}

void crfsconhash_vnode_print(LOG *log, const CRFSCONHASH_VNODE *crfsconhash_vnode)
{
    sys_log(log, "crfsconhash_vnode %p: hash %x, rnode pos %u\n",
                    crfsconhash_vnode,
                    CRFSCONHASH_VNODE_HASH(crfsconhash_vnode),
                    CRFSCONHASH_VNODE_POS(crfsconhash_vnode)
                   );
    return;
}

CRFSCONHASH *crfsconhash_new(const UINT32 hash_id)
{
    CRFSCONHASH *crfsconhash;
    
    alloc_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSCONHASH, &crfsconhash, LOC_CRFSCONHASH_0007);
    if(NULL_PTR == crfsconhash)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_new: alloc crfsconhash failed\n");
        return (NULL_PTR);
    }
    
    if(EC_FALSE == crfsconhash_init(crfsconhash, hash_id))
    {
        free_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSCONHASH, crfsconhash, LOC_CRFSCONHASH_0008);
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_new: init crfsconhash failed\n");
        return (NULL_PTR);
    }
    
    return (crfsconhash);
}

EC_BOOL crfsconhash_init(CRFSCONHASH *crfsconhash, const UINT32 hash_id)
{
    CRFSCONHASH_HASH_FUNC(crfsconhash) = chash_algo_fetch(hash_id);
    if(NULL_PTR == CRFSCONHASH_HASH_FUNC(crfsconhash))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_init: invalid hash_id %ld\n", hash_id);
        return (EC_FALSE);
    }
    CRFSCONHASH_HASH_ID(crfsconhash)   = hash_id;

    cvector_init(CRFSCONHASH_RNODE_VEC(crfsconhash), 0, MM_CRFSCONHASH_RNODE, CVECTOR_LOCK_ENABLE, LOC_CRFSCONHASH_0009);
    crb_tree_init(CRFSCONHASH_VNODE_TREE(crfsconhash), (CRB_DATA_CMP)crfsconhash_vnode_cmp, (CRB_DATA_FREE)crfsconhash_vnode_free,(CRB_DATA_PRINT)crfsconhash_vnode_print);
    
    return (EC_TRUE);
}

EC_BOOL crfsconhash_clean(CRFSCONHASH *crfsconhash)
{
    cvector_clean(CRFSCONHASH_RNODE_VEC(crfsconhash), (CVECTOR_DATA_CLEANER)crfsconhash_rnode_free, LOC_CRFSCONHASH_0010);
    crb_tree_clean(CRFSCONHASH_VNODE_TREE(crfsconhash));

    CRFSCONHASH_HASH_ID(crfsconhash)   = CHASH_ERR_ALGO_ID;
    CRFSCONHASH_HASH_FUNC(crfsconhash) = NULL_PTR;
    return (EC_TRUE);
}

EC_BOOL crfsconhash_free(CRFSCONHASH *crfsconhash)
{
    if(NULL_PTR != crfsconhash)
    {
        crfsconhash_clean(crfsconhash);
        free_static_mem(MD_CRFS, CMPI_ANY_MODI, MM_CRFSCONHASH, crfsconhash, LOC_CRFSCONHASH_0011);
    }
    return (EC_TRUE);
}

void crfsconhash_print(LOG *log, const CRFSCONHASH *crfsconhash)
{
    sys_log(log, "crfsconhash %p: hash_id %ld, rnode num %ld, vnode num %u\n", 
                    crfsconhash,
                    CRFSCONHASH_HASH_ID(crfsconhash),
                    cvector_size(CRFSCONHASH_RNODE_VEC(crfsconhash)),
                    crb_tree_node_num(CRFSCONHASH_VNODE_TREE(crfsconhash))
                    );

    if(do_log(SEC_0144_CRFSCONHASH, 6))
    {
        sys_log(log, "crfsconhash %p: rnode vec:\n", crfsconhash);
        cvector_print(log, CRFSCONHASH_RNODE_VEC(crfsconhash), (CVECTOR_DATA_PRINT)crfsconhash_rnode_print);
    }

    if(do_log(SEC_0144_CRFSCONHASH, 7))
    {
        sys_log(log, "crfsconhash %p: vnode tree:\n", crfsconhash);
        crb_tree_print(log, CRFSCONHASH_VNODE_TREE(crfsconhash));
    }    
    
    return;
}

void crfsconhash_print_rnode_vec(LOG *log, const CRFSCONHASH *crfsconhash)
{
    sys_log(log, "crfsconhash %p: hash_id %ld\n", 
                    crfsconhash,
                    CRFSCONHASH_HASH_ID(crfsconhash));

    sys_log(log, "crfsconhash %p: rnode vec:\n", crfsconhash);
    cvector_print(log, CRFSCONHASH_RNODE_VEC(crfsconhash), (CVECTOR_DATA_PRINT)crfsconhash_rnode_print);
    
    return;
}

void crfsconhash_print_vnode_tree(LOG *log, const CRFSCONHASH *crfsconhash)
{
    sys_log(log, "crfsconhash %p: hash_id %ld\n", 
                    crfsconhash,
                    CRFSCONHASH_HASH_ID(crfsconhash));
                    
    sys_log(log, "crfsconhash %p: vnode tree:\n", crfsconhash);
    crb_tree_print(log, CRFSCONHASH_VNODE_TREE(crfsconhash));
    
    return;
}

UINT32 crfsconhash_add_rnode(CRFSCONHASH *crfsconhash, const CRFSCONHASH_RNODE *crfsconhash_rnode)
{
    return cvector_add(CRFSCONHASH_RNODE_VEC(crfsconhash), (void *)crfsconhash_rnode);
}

CRB_NODE *crfsconhash_add_vnode(CRFSCONHASH *crfsconhash, const CRFSCONHASH_VNODE *crfsconhash_vnode)
{
    return crb_tree_insert_data(CRFSCONHASH_VNODE_TREE(crfsconhash), (void *)crfsconhash_vnode);
}

EC_BOOL crfsconhash_add_vnode_replicas(CRFSCONHASH *crfsconhash, const UINT32 crfsconhash_rnode_pos)
{
    CRFSCONHASH_RNODE *crfsconhash_rnode;
    
    uint32_t tcid;
    uint16_t replica;

    crfsconhash_rnode = (CRFSCONHASH_RNODE *)cvector_get(CRFSCONHASH_RNODE_VEC(crfsconhash), crfsconhash_rnode_pos);
    if(NULL_PTR == crfsconhash_rnode)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_add_vnode_replicas: not found rnode at pos %ld\n", 
                           crfsconhash_rnode_pos);
        return (EC_FALSE);
    }

    ASSERT(0 == (crfsconhash_rnode_pos >> 16));

    tcid = CRFSCONHASH_RNODE_TCID(crfsconhash_rnode);
    for(replica = 0; replica < CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode); replica ++)
    {
        char     str[64];
        uint32_t len;
        uint32_t hash;
        
        CRFSCONHASH_VNODE *crfsconhash_vnode;
        CRB_NODE          *crb_node;
        
        len  = snprintf(str, sizeof(str), "%s.%u", c_word_to_ipv4(tcid), replica);
        hash = (uint32_t)CRFSCONHASH_HASH_FUNC(crfsconhash)(len, str);

        crfsconhash_vnode = crfsconhash_vnode_make(hash, (uint16_t)crfsconhash_rnode_pos);
        if(NULL_PTR == crfsconhash_vnode)
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_add_vnode_replicas: make %s vnode (hash %x, rnode pos %u) failed\n",
                               str, hash, (uint16_t)crfsconhash_rnode_pos);
            return (EC_FALSE);
        }

        crb_node = crb_tree_insert_data(CRFSCONHASH_VNODE_TREE(crfsconhash), (void *)crfsconhash_vnode);
        if(NULL_PTR == crb_node)
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_add_vnode_replicas: insert %s vnode (hash %x, rnode pos %u) to rbtree failed\n",
                               str, hash, (uint16_t)crfsconhash_rnode_pos);
            crfsconhash_vnode_free(crfsconhash_vnode);
            return (EC_FALSE);
        }

        /*fix*/
        if(crfsconhash_vnode != CRB_NODE_DATA(crb_node))
        {
            crfsconhash_vnode_free(crfsconhash_vnode);
        }
    }

    return (EC_TRUE);
}

EC_BOOL crfsconhash_del_vnode_replicas(CRFSCONHASH *crfsconhash, const UINT32 crfsconhash_rnode_pos)
{
    CRFSCONHASH_RNODE *crfsconhash_rnode;
    
    uint32_t tcid;
    uint16_t replica;

    crfsconhash_rnode = (CRFSCONHASH_RNODE *)cvector_get(CRFSCONHASH_RNODE_VEC(crfsconhash), crfsconhash_rnode_pos);
    if(NULL_PTR == crfsconhash_rnode)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_del_vnode_replicas: not found rnode at pos %ld\n", 
                           crfsconhash_rnode_pos);
        return (EC_FALSE);
    }

    ASSERT(0 == (crfsconhash_rnode_pos >> 16));

    tcid = CRFSCONHASH_RNODE_TCID(crfsconhash_rnode);
    for(replica = 0; replica < CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode); replica ++)
    {
        char     str[64];
        uint32_t len;
        uint32_t hash;
        
        CRFSCONHASH_VNODE *crfsconhash_vnode;
        
        len  = snprintf(str, sizeof(str), "%s.%u", c_word_to_ipv4(tcid), replica);
        hash = (uint32_t)CRFSCONHASH_HASH_FUNC(crfsconhash)(len, str);

        crfsconhash_vnode = crfsconhash_vnode_make(hash, (uint16_t)crfsconhash_rnode_pos);
        if(NULL_PTR == crfsconhash_vnode)
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_del_vnode_replicas: make %s vnode (hash %x, rnode pos %u) failed\n",
                               str, hash, (uint16_t)crfsconhash_rnode_pos);
            return (EC_FALSE);
        }

        if(EC_FALSE == crb_tree_delete_data(CRFSCONHASH_VNODE_TREE(crfsconhash), (void *)crfsconhash_vnode))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_del_vnode_replicas: del %s vnode (hash %x, rnode pos %u) from rbtree failed\n",
                               str, hash, (uint16_t)crfsconhash_rnode_pos);
            crfsconhash_vnode_free(crfsconhash_vnode);
            return (EC_FALSE);
        }
        
        crfsconhash_vnode_free(crfsconhash_vnode);
    }

    return (EC_TRUE);
}

EC_BOOL crfsconhash_add_node(CRFSCONHASH *crfsconhash, const uint32_t tcid, const uint16_t replicas)
{
    CRFSCONHASH_RNODE  crfsconhash_rnode_t;
    CRFSCONHASH_RNODE *crfsconhash_rnode;
    UINT32             crfsconhash_rnode_pos;

    CRFSCONHASH_RNODE_TCID(&crfsconhash_rnode_t) = tcid;
    crfsconhash_rnode_pos = cvector_search_front(CRFSCONHASH_RNODE_VEC(crfsconhash), 
                                                 (void *)&crfsconhash_rnode_t, 
                                                 (CVECTOR_DATA_CMP)crfsconhash_rnode_cmp_tcid);
    if(CVECTOR_ERR_POS != crfsconhash_rnode_pos)
    {
        crfsconhash_rnode = (CRFSCONHASH_RNODE *)cvector_get(CRFSCONHASH_RNODE_VEC(crfsconhash), crfsconhash_rnode_pos);
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_add_node: tcid %s was already in rnode (tcid %s, replicas %u)\n", 
                           c_word_to_ipv4(tcid),
                           c_word_to_ipv4(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)),
                           CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode));
        return (EC_FALSE);
    }

    crfsconhash_rnode = crfsconhash_rnode_make(tcid, replicas);
    if(NULL_PTR == crfsconhash_rnode)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_add_node: make rnode (tcid %s, replicas %u) failed\n", 
                           c_word_to_ipv4(tcid), replicas);
        return (EC_FALSE);
    }

    /*add rnode*/
    crfsconhash_rnode_pos = cvector_add(CRFSCONHASH_RNODE_VEC(crfsconhash), (void *)crfsconhash_rnode);
    if(CVECTOR_ERR_POS == crfsconhash_rnode_pos)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_add_node: add rnode (tcid %s, replicas %u) failed\n", 
                           c_word_to_ipv4(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)),
                           CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode));
        crfsconhash_rnode_free(crfsconhash_rnode);
        return (EC_FALSE);
    }

    ASSERT(0 == (crfsconhash_rnode_pos >> 16));

    /*add vnode replicas*/
    if(EC_FALSE == crfsconhash_add_vnode_replicas(crfsconhash, crfsconhash_rnode_pos))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_add_node: add vnode replicas of rnode (tcid %s, replicas %u) failed\n", 
                           c_word_to_ipv4(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)),
                           CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode));

        cvector_set(CRFSCONHASH_RNODE_VEC(crfsconhash), crfsconhash_rnode_pos, NULL_PTR);
        crfsconhash_rnode_free(crfsconhash_rnode);
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

/*for any replica: replicas = 0*/
EC_BOOL crfsconhash_del_node(CRFSCONHASH *crfsconhash, const uint32_t tcid, const uint16_t replicas)
{
    CRFSCONHASH_RNODE  crfsconhash_rnode_t;
    CRFSCONHASH_RNODE *crfsconhash_rnode;
    UINT32             crfsconhash_rnode_pos;

    CRFSCONHASH_RNODE_TCID(&crfsconhash_rnode_t) = tcid;
    crfsconhash_rnode_pos = cvector_search_front(CRFSCONHASH_RNODE_VEC(crfsconhash), 
                                                 (void *)&crfsconhash_rnode_t, 
                                                 (CVECTOR_DATA_CMP)crfsconhash_rnode_cmp_tcid);
    if(CVECTOR_ERR_POS == crfsconhash_rnode_pos)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_del_node: tcid %s is not in rnode\n", 
                           c_word_to_ipv4(tcid));
        return (EC_FALSE);
    }

    crfsconhash_rnode = (CRFSCONHASH_RNODE *)cvector_get(CRFSCONHASH_RNODE_VEC(crfsconhash), crfsconhash_rnode_pos);
    if(CRFSCONHASH_ANY_REPLICAS != replicas)
    {
        if(replicas != CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode))
        {
            /*not support replicas changes*/
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_del_node: rnode (tcid %s, replicas %u) mismatch the expected replicas %u\n", 
                               c_word_to_ipv4(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)),
                               CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode),
                               replicas);
            return (EC_FALSE);        
        }
    }

    ASSERT(0 == (crfsconhash_rnode_pos >> 16));

    /*del vnode replicas*/
    if(EC_FALSE == crfsconhash_del_vnode_replicas(crfsconhash, crfsconhash_rnode_pos))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_del_node: del vnode replicas of rnode (tcid %s, replicas %u) failed\n", 
                           c_word_to_ipv4(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)),
                           CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode));        
        return (EC_FALSE);
    }    

    /*del rnode*/
    cvector_set(CRFSCONHASH_RNODE_VEC(crfsconhash), crfsconhash_rnode_pos, NULL_PTR);
    crfsconhash_rnode_free(crfsconhash_rnode);
    
    return (EC_TRUE);
}

EC_BOOL crfsconhash_has_node(const CRFSCONHASH *crfsconhash, const uint32_t tcid)
{
    CRFSCONHASH_RNODE  crfsconhash_rnode_t;
    UINT32             crfsconhash_rnode_pos;

    CRFSCONHASH_RNODE_TCID(&crfsconhash_rnode_t) = tcid;
    crfsconhash_rnode_pos = cvector_search_front(CRFSCONHASH_RNODE_VEC(crfsconhash), 
                                                 (void *)&crfsconhash_rnode_t, 
                                                 (CVECTOR_DATA_CMP)crfsconhash_rnode_cmp_tcid);
    if(CVECTOR_ERR_POS == crfsconhash_rnode_pos)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 9)(LOGSTDOUT, "info:crfsconhash_has_node: tcid %s is not in rnode\n", 
                           c_word_to_ipv4(tcid));
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

CRFSCONHASH_RNODE *crfsconhash_get_rnode(const CRFSCONHASH *crfsconhash, const uint32_t tcid, const uint16_t replicas)
{
    CRFSCONHASH_RNODE  crfsconhash_rnode_t;
    CRFSCONHASH_RNODE *crfsconhash_rnode;
    UINT32             crfsconhash_rnode_pos;

    CRFSCONHASH_RNODE_TCID(&crfsconhash_rnode_t) = tcid;
    crfsconhash_rnode_pos = cvector_search_front(CRFSCONHASH_RNODE_VEC(crfsconhash), 
                                                 (void *)&crfsconhash_rnode_t, 
                                                 (CVECTOR_DATA_CMP)crfsconhash_rnode_cmp_tcid);
    if(CVECTOR_ERR_POS == crfsconhash_rnode_pos)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_get_rnode: tcid %s is not in rnode\n", 
                           c_word_to_ipv4(tcid));
        return (NULL_PTR);
    }

    crfsconhash_rnode = (CRFSCONHASH_RNODE *)cvector_get(CRFSCONHASH_RNODE_VEC(crfsconhash), crfsconhash_rnode_pos);
    if(CRFSCONHASH_ANY_REPLICAS != replicas)
    {
        if(replicas != CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode))
        {
            /*not support replicas changes*/
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_get_rnode: rnode (tcid %s, replicas %u) mismatch the expected replicas %u\n", 
                               c_word_to_ipv4(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)),
                               CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode),
                               replicas);
            return (NULL_PTR);        
        }
    }

    return (crfsconhash_rnode);
}

CRFSCONHASH_RNODE *crfsconhash_lookup_rnode(const CRFSCONHASH *crfsconhash, const uint32_t hash)
{
    CRFSCONHASH_VNODE  crfsconhash_vnode_t;
    CRFSCONHASH_VNODE *crfsconhash_vnode;
    CRFSCONHASH_RNODE *crfsconhash_rnode;
    CRB_NODE *crb_node;

    if(EC_TRUE == crb_tree_is_empty(CRFSCONHASH_VNODE_TREE(crfsconhash)))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_lookup_rnode: vnode tree is empty\n");    
        return (NULL_PTR);    
    }

    CRFSCONHASH_VNODE_HASH(&crfsconhash_vnode_t) = hash;
    crb_node = crb_tree_lookup_data(CRFSCONHASH_VNODE_TREE(crfsconhash), (void *)&crfsconhash_vnode_t);
    if(NULL_PTR == crb_node)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_lookup_rnode: hash %x, should never reach here due to rbtree be circled\n", 
                           hash);    
        return (NULL_PTR);
    }

    crfsconhash_vnode = (CRFSCONHASH_VNODE *)CRB_NODE_DATA(crb_node);
    if(NULL_PTR == crfsconhash_vnode)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_lookup_rnode: hash %x, crb_node %p, should never reach here due to CRB_NODE_DATA be null!\n", 
                           hash, crb_node);    
        return (NULL_PTR);
    }    
#if 0
    if(do_log(SEC_0144_CRFSCONHASH, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] crfsconhash_lookup_rnode: hash %x => vnode ", hash);
        crfsconhash_vnode_print(LOGSTDOUT, crfsconhash_vnode);
    }
#endif
    crfsconhash_rnode = (CRFSCONHASH_RNODE *)cvector_get(CRFSCONHASH_RNODE_VEC(crfsconhash), 
                                                         CRFSCONHASH_VNODE_POS(crfsconhash_vnode));
    if(NULL_PTR == crfsconhash_rnode)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_lookup_rnode: hash %x, rnode_pos %u, should never reach here due to rnode not existing\n", 
                           hash, CRFSCONHASH_VNODE_POS(crfsconhash_vnode));    
        return (NULL_PTR);
    }
    return (crfsconhash_rnode);
}

EC_BOOL crfsconhash_flush_size(const CRFSCONHASH *crfsconhash, UINT32 *size)
{
    (*size) = sizeof(UINT32) /*hash_id*/
            + sizeof(UINT32) /*rnode_vec size*/
            + cvector_size(CRFSCONHASH_RNODE_VEC(crfsconhash)) * (
                                                                    sizeof(uint16_t) /*replicas*/
                                                                  + sizeof(uint32_t) /*tcid*/
                                                                  )
            + sizeof(uint32_t) /*vnode_tree size*/
            + crb_tree_node_num(CRFSCONHASH_VNODE_TREE(crfsconhash)) * (
                                                                     sizeof(uint32_t) /*hash*/
                                                                   + sizeof(uint32_t) /*pos*/
                                                                   );
    return (EC_FALSE);
}

EC_BOOL crfsconhash_rnode_flush(const CRFSCONHASH_RNODE *crfsconhash_rnode, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/
    
    if(NULL_PTR == crfsconhash_rnode)
    {
        uint32_t     tcid;
        uint16_t     replicas;
    
        replicas = CRFSCONHASH_ERR_REPLICAS;
        tcid     = (uint32_t)CMPI_ERROR_TCID;

        osize = sizeof(uint16_t);
        if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(replicas)))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_rnode_flush: flush replicas at offset %u of fd %d failed\n", (*offset), fd);
            return (EC_FALSE);
        }

        osize = sizeof(uint32_t);
        if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(tcid)))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_rnode_flush: flush tcid at offset %u of fd %d failed\n", (*offset), fd);
            return (EC_FALSE);
        }        
    }
    else
    {
        osize = sizeof(uint16_t);
        if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode))))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_rnode_flush: flush replicas at offset %u of fd %d failed\n", (*offset), fd);
            return (EC_FALSE);
        }

        osize = sizeof(uint32_t);
        if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode))))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_rnode_flush: flush tcid at offset %u of fd %d failed\n", (*offset), fd);
            return (EC_FALSE);
        }        
    }
    return (EC_TRUE);
}

EC_BOOL crfsconhash_rnode_load(CRFSCONHASH_RNODE *crfsconhash_rnode, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/

    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode))))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_rnode_load: load replicas at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    osize = sizeof(uint32_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode))))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_rnode_load: load tcid at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfsconhash_flush_rnodes(const CRFSCONHASH *crfsconhash, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/
    
    UINT32   rnode_num;
    UINT32   rnode_pos;

    rnode_num = cvector_size(CRFSCONHASH_RNODE_VEC(crfsconhash));
    
    osize = sizeof(UINT32);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(rnode_num)))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_flush_rnodes: flush rnode_num at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }
    
    for(rnode_pos = 0; rnode_pos < rnode_num; rnode_pos ++)
    {
        CRFSCONHASH_RNODE *crfsconhash_rnode;
        crfsconhash_rnode = (CRFSCONHASH_RNODE *)cvector_get(CRFSCONHASH_RNODE_VEC(crfsconhash), rnode_pos);
        if(EC_FALSE == crfsconhash_rnode_flush(crfsconhash_rnode, fd, offset))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_flush_rnodes: flush rnode %u# at offset %u of fd %d failed\n", rnode_pos, (*offset), fd);
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);    
}

EC_BOOL crfsconhash_load_rnodes(CRFSCONHASH *crfsconhash, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/
    
    UINT32   rnode_num;
    UINT32   rnode_pos;
   
    osize = sizeof(UINT32);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(rnode_num)))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load_rnodes: load rnode_num at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }
    
    for(rnode_pos = 0; rnode_pos < rnode_num; rnode_pos ++)
    {
        CRFSCONHASH_RNODE *crfsconhash_rnode;

        crfsconhash_rnode = crfsconhash_rnode_new();
        if(NULL_PTR == crfsconhash_rnode)
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load_rnodes: new rnode at offset %u of fd %d failed\n", (*offset), fd);
            return (EC_FALSE);
        }
        
        if(EC_FALSE == crfsconhash_rnode_load(crfsconhash_rnode, fd, offset))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load_rnodes: load rnode %u# at offset %u of fd %d failed\n", rnode_pos, (*offset), fd);
            crfsconhash_rnode_free(crfsconhash_rnode);
            return (EC_FALSE);
        }
        
        if(CRFSCONHASH_ERR_REPLICAS == CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode)
        && ((uint32_t)CMPI_ERROR_TCID) == CRFSCONHASH_RNODE_TCID(crfsconhash_rnode))
        {
            cvector_push(CRFSCONHASH_RNODE_VEC(crfsconhash), NULL_PTR);
            crfsconhash_rnode_free(crfsconhash_rnode);
        }
        else
        {
            cvector_push(CRFSCONHASH_RNODE_VEC(crfsconhash), crfsconhash_rnode);
        }
    }

    return (EC_TRUE);    
}

EC_BOOL crfsconhash_vnode_flush(const CRFSCONHASH_VNODE *crfsconhash_vnode, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/

    osize = sizeof(uint32_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_VNODE_HASH(crfsconhash_vnode))))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_vnode_flush: flush hash at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }
    
    osize = sizeof(uint32_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_VNODE_POS(crfsconhash_vnode))))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_vnode_flush: flush pos at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfsconhash_vnode_load(CRFSCONHASH_VNODE *crfsconhash_vnode, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/

    osize = sizeof(uint32_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_VNODE_HASH(crfsconhash_vnode))))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_vnode_load: load hash at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    osize = sizeof(uint32_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_VNODE_POS(crfsconhash_vnode))))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_vnode_load: load pos at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

static EC_BOOL __crfsconhash_flush_vnodes_inorder(const CRFSCONHASH *crfsconhash, const CRB_NODE *node, int fd, UINT32 *offset)
{
    CRFSCONHASH_VNODE *crfsconhash_vnode;
    if(NULL_PTR == node)
    {
        return (EC_TRUE);
    }
    
    if(NULL_PTR != CRB_NODE_LEFT(node))
    {
        if(EC_FALSE == __crfsconhash_flush_vnodes_inorder(crfsconhash, CRB_NODE_LEFT(node), fd, offset))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__crfsconhash_flush_vnodes_inorder: flush left subtree %p at offset %u of fd %d failed\n", CRB_NODE_LEFT(node), (*offset), fd);
            return (EC_FALSE);
        }
    }

    crfsconhash_vnode = (CRFSCONHASH_VNODE *)CRB_NODE_DATA(node);
    if(NULL_PTR == crfsconhash_vnode)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__crfsconhash_flush_vnodes_inorder: data of crb node %p is null at offset %u of fd %d failed\n", node, (*offset), fd);
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfsconhash_vnode_flush(crfsconhash_vnode, fd, offset))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__crfsconhash_flush_vnodes_inorder: flush vnode %p at offset %u of fd %d failed\n", crfsconhash_vnode, (*offset), fd);
        return (EC_FALSE);
    }

    if(NULL_PTR != CRB_NODE_RIGHT(node))
    {
        if(EC_FALSE == __crfsconhash_flush_vnodes_inorder(crfsconhash, CRB_NODE_RIGHT(node), fd, offset))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__crfsconhash_flush_vnodes_inorder: flush right subtree %p at offset %u of fd %d failed\n", CRB_NODE_RIGHT(node), (*offset), fd);
            return (EC_FALSE);
        }
    }    
    
    return (EC_TRUE);
}

EC_BOOL crfsconhash_flush_vnodes(const CRFSCONHASH *crfsconhash, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/
    
    uint32_t   vnode_num;
    
    vnode_num = crb_tree_node_num(CRFSCONHASH_VNODE_TREE(crfsconhash));

    osize = sizeof(UINT32);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(vnode_num)))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_flush_vnodes: flush vnode num at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    if(EC_FALSE == __crfsconhash_flush_vnodes_inorder(crfsconhash, CRB_TREE_ROOT(CRFSCONHASH_VNODE_TREE(crfsconhash)), fd, offset))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_flush_vnodes: flush vnode tree at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL crfsconhash_load_vnodes(CRFSCONHASH *crfsconhash, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/
    
    uint32_t   vnode_num;
    uint32_t   vnode_pos;
    
    osize = sizeof(UINT32);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(vnode_num)))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load_vnodes: load vnode num at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    for(vnode_pos = 0; vnode_pos < vnode_num; vnode_pos ++)
    {
        CRFSCONHASH_VNODE *crfsconhash_vnode;

        crfsconhash_vnode = crfsconhash_vnode_new();
        if(NULL_PTR == crfsconhash_vnode)
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load_vnodes: new vnode at offset %u of fd %d failed\n", (*offset), fd);
            return (EC_FALSE);
        }
        
        if(EC_FALSE == crfsconhash_vnode_load(crfsconhash_vnode, fd, offset))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load_vnodes: load vnode %u# at offset %u of fd %d failed\n", vnode_pos, (*offset), fd);
            crfsconhash_vnode_free(crfsconhash_vnode);
            return (EC_FALSE);
        }        

        if(NULL_PTR == crfsconhash_add_vnode(crfsconhash, crfsconhash_vnode))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load_vnodes: add vnode %u# at offset %u of fd %d failed\n", vnode_pos, (*offset), fd);
            crfsconhash_vnode_free(crfsconhash_vnode);
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

EC_BOOL crfsconhash_flush(const CRFSCONHASH *crfsconhash, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/

    /*flush hash_id*/
    osize = sizeof(UINT32);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_HASH_ID(crfsconhash))))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_flush: flush hash id at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*flush rnode vec*/
    if(EC_FALSE == crfsconhash_flush_rnodes(crfsconhash, fd, offset))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_flush: flush rnodes at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*flush vnode tree*/
    if(EC_FALSE == crfsconhash_flush_vnodes(crfsconhash, fd, offset))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_flush: flush vnodes at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfsconhash_load(CRFSCONHASH *crfsconhash, int fd, UINT32 *offset)
{
    UINT32   osize;/*write once size*/

    /*load hash_id*/
    osize = sizeof(UINT32);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CRFSCONHASH_HASH_ID(crfsconhash))))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load: load hash id at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }
    
    CRFSCONHASH_HASH_FUNC(crfsconhash) = chash_algo_fetch(CRFSCONHASH_HASH_ID(crfsconhash));
    if(NULL_PTR == CRFSCONHASH_HASH_FUNC(crfsconhash))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load: invalid hash id %ld\n", CRFSCONHASH_HASH_ID(crfsconhash));
        return (EC_FALSE);
    }    

    /*load rnode vec*/
    if(EC_FALSE == crfsconhash_load_rnodes(crfsconhash, fd, offset))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load: load rnodes at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*load vnode tree*/
    if(EC_FALSE == crfsconhash_load_vnodes(crfsconhash, fd, offset))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:crfsconhash_load: load vnodes at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL crfsconhash_rnodes_is_equal(const CRFSCONHASH *crfsconhash_1st, const CRFSCONHASH *crfsconhash_2nd)
{
    return cvector_cmp(CRFSCONHASH_RNODE_VEC(crfsconhash_1st), 
                       CRFSCONHASH_RNODE_VEC(crfsconhash_2nd), 
                       (CVECTOR_DATA_CMP)crfsconhash_rnode_is_equal);
}

EC_BOOL crfsconhash_vnodes_is_equal(const CRFSCONHASH *crfsconhash_1st, const CRFSCONHASH *crfsconhash_2nd)
{
    return crb_tree_cmp(CRFSCONHASH_VNODE_TREE(crfsconhash_1st),
                        CRFSCONHASH_VNODE_TREE(crfsconhash_2nd),
                        (CRB_DATA_IS_EQUAL)crfsconhash_vnode_is_equal);
}

EC_BOOL crfsconhash_is_equal(const CRFSCONHASH *crfsconhash_1st, const CRFSCONHASH *crfsconhash_2nd)
{
    if(CRFSCONHASH_HASH_ID(crfsconhash_1st) != CRFSCONHASH_HASH_ID(crfsconhash_2nd))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "crfsconhash_is_equal: hash id: %u != %u\n", 
                           CRFSCONHASH_HASH_ID(crfsconhash_1st),
                           CRFSCONHASH_HASH_ID(crfsconhash_2nd));
        return (EC_FALSE);
    }

    if(CRFSCONHASH_HASH_FUNC(crfsconhash_1st) != CRFSCONHASH_HASH_FUNC(crfsconhash_2nd))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "crfsconhash_is_equal: hash func: %p != %p\n", 
                           CRFSCONHASH_HASH_FUNC(crfsconhash_1st),
                           CRFSCONHASH_HASH_FUNC(crfsconhash_2nd));
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsconhash_rnodes_is_equal(crfsconhash_1st, crfsconhash_2nd))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "crfsconhash_is_equal: rnodes is not equal\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsconhash_vnodes_is_equal(crfsconhash_1st, crfsconhash_2nd))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "crfsconhash_is_equal: vnodes is not equal\n");
        return (EC_FALSE);
    }    
    
    return (EC_TRUE);
}

EC_BOOL crfsconhash_clone(const CRFSCONHASH *crfsconhash_src, CRFSCONHASH *crfsconhash_des)
{
    cvector_clone(CRFSCONHASH_RNODE_VEC(crfsconhash_src), 
                  CRFSCONHASH_RNODE_VEC(crfsconhash_des), 
                  (CVECTOR_DATA_MALLOC)crfsconhash_rnode_new, 
                  (CVECTOR_DATA_CLONE)crfsconhash_rnode_clone);

    if(EC_FALSE == crb_tree_clone(CRFSCONHASH_VNODE_TREE(crfsconhash_src), 
                                   CRFSCONHASH_VNODE_TREE(crfsconhash_des), 
                                   (CRB_DATA_NEW)crfsconhash_vnode_new,
                                   (CRB_DATA_CLONE)crfsconhash_vnode_clone))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "crfsconhash_clone: clone vnodes failed\n");
        return (EC_FALSE);
    }

    CRFSCONHASH_HASH_ID(crfsconhash_des)   = CRFSCONHASH_HASH_ID(crfsconhash_src);
    CRFSCONHASH_HASH_FUNC(crfsconhash_des) = CRFSCONHASH_HASH_FUNC(crfsconhash_src);

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

