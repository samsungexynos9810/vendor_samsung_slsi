/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stddef.h>
#include "productfeature.h"
#include <utils/Log.h>

ProductFeature::ProductFeature()
{
}

ProductFeature::~ProductFeature()
{
}


bool ProductFeature::Init(const char* module_name)
{
    GetInstance()->CustomInit(module_name);
    return true;
}

/********************************************************
*
*   CUSTOM : Specific implementation only for Project
*
********************************************************/
bool ProductFeature::Custom_SupportOnly5Mode()
{
    bool bRet = false;
#ifdef SUPPORT_ONLY_5MODE
    bRet = true;
#endif
    RLOGI("[%s]  ProductFeature::SupportOnly5Mode == %s", GetProductName(), bRet==true?"true":"false");
    return bRet;
}

bool ProductFeature::Custom_SupportOnlyBasicPreferredNetworkType()
{
    bool bRet = false;
#ifdef SUPPORT_ONLY_BASIC_PREFERRED_NETWORK_TYPE
    bRet = true;
#endif
    RLOGI("[%s]  ProductFeature::SupportOnlyBasicPreferredNetworkType == %s", GetProductName(), bRet==true?"true":"false");
    return bRet;
}

/********************************************************
*
*   FUNCTION : Implementation Issue
*
********************************************************/
bool ProductFeature::Function_SendDebugTraceOffAtBootup()
{
    bool bRet = false;
#ifdef SEND_DEBUG_TRACE_OFF_AT_BOOTUP_TIME
    bRet = true;
#endif
    RLOGI("[%s]  ProductFeature::SendDebugTraceOffAtBootup == %s", GetProductName(), bRet==true?"true":"false");
    return bRet;
}

bool ProductFeature::Function_GetFastDormancyInfo(unsigned char &lcdOn, unsigned char &lcdOff, unsigned char &rel8LcdOn, unsigned char &rel8LcdOff)
{
    bool bRet = false;
#ifdef SUPPORT_FAST_DORMANCY_INFO_SET
    bRet = true;

    lcdOn = 0;
    lcdOff = 10;    //default 5sec
    rel8LcdOn = 0;
    rel8LcdOff = 0;
#else
    lcdOn = 0;
    lcdOff = 0;    //default 5sec
    rel8LcdOn = 0;
    rel8LcdOff = 0;
#endif
    RLOGI("[%s]  ProductFeature::SupportFastDormancyInfo == %s(%d,%d,%d,%d [.5sec/unit]))",
        GetProductName(), bRet==true?"true":"false", lcdOn, lcdOff, rel8LcdOn, rel8LcdOff);
    return bRet;
}

bool ProductFeature::Function_RilResetByCrashExitInUserMode()
{
    bool bRet = false;
#ifndef BLOCK_RILRESET_BY_MODEM_CRASH_IN_USERMODE
    bRet = true;
#endif
    RLOGI("[%s]  ProductFeature::RilResetByModemCrash == %s", GetProductName(), bRet==true?"true":"false");
    return bRet;
}

bool ProductFeature::Function_DeviceBasedCwInCs()
{
    bool bRet = true;
#ifdef NW_BASED_CW_CS
    bRet = false;
#endif
    RLOGI("[%s]  ProductFeature::DeviceBasedCWinCS == %s", GetProductName(), bRet==true?"true":"false");
    return bRet;
}

