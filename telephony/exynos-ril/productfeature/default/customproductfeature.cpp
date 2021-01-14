/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
 #include <utils/Log.h>
#include "customproductfeature.h"

CustomProductFeature instance;

ProductFeature* ProductFeature::GetInstance()
{
    return (ProductFeature*)&instance;
}

CustomProductFeature::CustomProductFeature()
{
}

CustomProductFeature::~CustomProductFeature()
{
}

bool CustomProductFeature::CustomInit(const char* module_name)
{
    RLOGI("<%s> [%s] CustomProductFeature::Init", module_name==NULL?"unknown":module_name, PRODUCT_NAME);
    return true;
}

const char* CustomProductFeature::GetProductName()
{
    return PRODUCT_NAME;
}

/********************************************************
*
*   FUNCTION : Implementation Issue
*
********************************************************/

/********************************************************
*
*   CUSTOM : Specific implementation only for Project
*
********************************************************/

/********************************************************
*
*   ModuleXXX : External RIL connection function
*
********************************************************/
bool CustomProductFeature::ModuleAudio_VolumeStartFrom1()
{
    bool bRet = false;
#ifdef AUDIO_FEATURE_CUSTOM_VOLUME_START_FROM1
    bRet = true;
#endif
    RLOGI("[%s] CustomProductFeature::VolumeStartFrom1 == %s", PRODUCT_NAME, bRet==true?"true":"false");
    return bRet;
}

