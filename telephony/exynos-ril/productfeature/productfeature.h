/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef    __PRODUCT_FEATURE_H__
#define    __PRODUCT_FEATURE_H__

class ProductFeature {
public:
    ProductFeature();
    virtual ~ProductFeature();

public:
    static bool Init(const char* module_name);

protected:
    virtual bool CustomInit(const char* module_name) = 0;
    virtual const char* GetProductName() = 0;

public:
    static ProductFeature* GetInstance();

protected:
    /********************************************************
    *
    *   FUNCTION : Implementation Issue
    *
    ********************************************************/
    //bool Function_XXX();
    bool Function_SendDebugTraceOffAtBootup();
    virtual bool Function_GetFastDormancyInfo(unsigned char &lcdOn, unsigned char &lcdOff, unsigned char &rel8LcdOn, unsigned char &rel8LcdOff);
    bool Function_DeviceBasedCwInCs();
    bool Function_RilResetByCrashExitInUserMode();
    /********************************************************
    *
    *   CUSTOM : Specific implementation only for Project
    *
    ********************************************************/
    virtual const char* Custom_GetRfsPath() { return "/efs/"; }
    virtual const char* Custom_GetRfsBackupFilePath() { return "/efs/backup/nv_protected"; }
    virtual bool Custom_SupportOnly5Mode();
    virtual bool Custom_SupportOnlyBasicPreferredNetworkType();

public:
    /********************************************************
    *
    *   FUNCTION : Implementation Issue
    *
    ********************************************************/
    ///static bool SupportXXX() { return GETINSTANCE()->Function_XXX(); }
    static bool SupportSendDebugTraceOffAtBootup() { return GetInstance()->Function_SendDebugTraceOffAtBootup(); }
    static bool GetFastDormancyInfo(unsigned char &lcdOn, unsigned char &lcdOff, unsigned char &rel8LcdOn, unsigned char &rel8LcdOff) {
            return GetInstance()->Function_GetFastDormancyInfo(lcdOn, lcdOff, rel8LcdOn, rel8LcdOff); }
    static bool DeviceBasedCwInCs() { return GetInstance()->Function_DeviceBasedCwInCs(); }
    static bool RilResetByCrashExitInUserMode() { return GetInstance()->Function_RilResetByCrashExitInUserMode(); }

    static const char* GetRfsPath() { return GetInstance()->Custom_GetRfsPath(); }
    static const char* GetRfsBackupFilePath() { return GetInstance()->Custom_GetRfsBackupFilePath(); }

    static bool SupportOnly5Mode() { return GetInstance()->Custom_SupportOnly5Mode(); }
    static bool SupportOnlyBasicPreferredNetworkType() { return GetInstance()->Custom_SupportOnlyBasicPreferredNetworkType(); }
};

#endif  //__PRODUCT_FEATURE_H__
