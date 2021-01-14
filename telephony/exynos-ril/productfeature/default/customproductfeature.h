/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef    __CUSTOM_PRODUCT_FEATURE_H__
#define    __CUSTOM_PRODUCT_FEATURE_H__

#include "productfeature.h"

#define GETINSTANCE()   ((CustomProductFeature*)GetInstance())

class CustomProductFeature : public ProductFeature {
public:
    CustomProductFeature();
    virtual ~CustomProductFeature();

protected:
    bool CustomInit(const char* module_name);
    const char* GetProductName();

private:
    /********************************************************
    *
    *   FUNCTION : Implementation Issue
    *
    ********************************************************/
    //bool Function_XXX();

    /********************************************************
    *
    *   CUSTOM : Specific implementation only for Project
    *
    ********************************************************/
    //bool Custom_XXX();

    /********************************************************
    *
    *   ModuleXXX : External RIL connection function
    *
    ********************************************************/
    bool ModuleAudio_VolumeStartFrom1();

public:
    /********************************************************
    *
    *   FUNCTION : Implementation Issue
    *
    ********************************************************/
    ///static bool SupportXXX() { return GETINSTANCE()->Function_XXX(); }
    /********************************************************
    *
    *   CUSTOM : Specific implementation only for Project
    *
    ********************************************************/
    //static bool SupportXXX() { return GETINSTANCE()->Custom_XXX(); }
    /********************************************************
    *
    *   ModuleXXX : External RIL connection function
    *
    ********************************************************/
    static bool SupportVolumeStartFrom1() { return GETINSTANCE()->ModuleAudio_VolumeStartFrom1(); }
};

#endif  //__CUSTOM_PRODUCT_FEATURE_H__