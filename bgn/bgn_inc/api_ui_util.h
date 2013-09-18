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

/***************************************************************************
* Module Name   :   api_ui_ui.h
*
* Description:
* This file contains the definition for UI User Interface.
*
* Dependencies:
*
******************************************************************************/
#ifndef _API_UI_UTIL_H
#define _API_UI_UTIL_H


#include "type.h"
#include "api_ui.inc"

/* Function Prototypes */
char*   next_token(STRTOK_INSTANCE* instance);
void    strtok_init(char* string, char* token, STRTOK_INSTANCE* instance);
EC_BOOL strtoint(char* string, int* value);

#endif /* _API_UI_UTIL_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/
