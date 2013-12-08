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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"

#include "conv.h"

#include "input.h"

UINT8 g_decstr[BIGINTSIZE];

UINT32 input_bgn_dec(BIGINT *des,const char *info)
{
    UINT8  *decstr;
    UINT32  decstrmaxlen;
    UINT32  decstrlen;

    UINT32 conv_md_id;

    decstr = g_decstr;
    decstrmaxlen = sizeof(g_decstr)/sizeof(g_decstr[0]);

    fflush(stdout);
    fflush(stdin);

    fprintf(stdout,"%s",info);

    /*fscanf %s maybe overflow here*/
    fscanf(stdin,"%s",decstr);

    fflush(stdout);
    fflush(stdin);

    decstrlen = strlen((char *)decstr);

    conv_md_id = conv_start();

    conv_dec_to_bgn(conv_md_id, decstr, decstrlen, des);
    conv_end(conv_md_id);

    return 0;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

