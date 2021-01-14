/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
 * opeartorloader.cpp
 *
 *  Created on: 2019. 5. 22.
 */

#include "carrierloader.h"
#include "rillog.h"
#include "systemproperty.h"

#define RO_CARRIER "ro.carrier"
#define RO_VENDOR_CARRIER "ro.vendor.config.build_carrier"

static bool VDBG = false;

IMPLEMENT_MODULE_TAG(CarrierLoader, CarrierLoader)

CarrierLoader::CarrierLoader()
{
    // RO_CARRIER or RO_VENDOR_CARRIER
    mCarrierMap[string("cmcc")] = TARGET_OPER_CMCC;
    mCarrierMap[string("cm")] = TARGET_OPER_CMCC;
    mCarrierMap[string("ctc")] = TARGET_OPER_CTC;
    mCarrierMap[string("ct")] = TARGET_OPER_CTC;
    mCarrierMap[string("cucc")] = TARGET_OPER_CU;
    mCarrierMap[string("cu")] = TARGET_OPER_CU;
    mCarrierMap[string("chnopen")] = TARGET_OPER_CHNOPEN;
    mCarrierMap[string("retcn")] = TARGET_OPER_CHNOPEN;
    mCarrierMap[string("att")] = TARGET_OPER_ATT;
    mCarrierMap[string("tmo")] = TARGET_OPER_TMO;
    mCarrierMap[string("vzw")] = TARGET_OPER_VZW;
    mCarrierMap[string("spr")] = TARGET_OPER_SPR;
    mCarrierMap[string("europen")] = TARGET_OPER_EUROPEN;
    mCarrierMap[string("euopen")] = TARGET_OPER_EUROPEN;
    mCarrierMap[string("latin")] = TARGET_OPER_LATIN;
    mCarrierMap[string("ntt")] = TARGET_OPER_NTT;
    mCarrierMap[string("kddi")] = TARGET_OPER_KDDI;
}

// member
int CarrierLoader::GetTargetOperator()
{
    string carrier = CarrierLoader::GetCarrier("unknown");
    int ret = this->GetTargetOperator(carrier);
    return ret;
}

int CarrierLoader::GetVendorTargetOperator()
{
    string carrier = CarrierLoader::GetVendorCarrier("unknown");
    int ret = this->GetTargetOperator(carrier);
    if (ret < 0) {
        ret = TARGET_OPER_EUROPEN;
    }
    if (VDBG) {
        RilLogV("%s %s %s ret=%d", TAG, __FUNCTION__, carrier.c_str(), ret);
    }
    return ret;
}

int CarrierLoader::GetTargetOperator(string carrier)
{
    map<string, int>::const_iterator iter = mCarrierMap.find(carrier);
    if (iter == mCarrierMap.end()) {
        if (VDBG) {
            RilLogV("%s %s %s not found", TAG, __FUNCTION__, carrier.c_str());
        }
        return -1;
    }
    if (VDBG) {
        RilLogV("%s %s %s ret=%d", TAG, __FUNCTION__, carrier.c_str(), iter->second);
    }
    return iter->second;
}

int CarrierLoader::GetTargetOperator(const char *carrier)
{
    if (carrier == NULL) {
        if (VDBG) {
            RilLogV("%s %s invalid carrier", TAG, __FUNCTION__);
        }
        return -1;
    }
    return CarrierLoader::GetTargetOperator(string(carrier));
}

// static
CarrierLoader CarrierLoader::instance;
CarrierLoader& CarrierLoader::GetInstance() { return instance; }

string CarrierLoader::GetCarrier()
{
    return CarrierLoader::GetCarrier("");
}

string CarrierLoader::GetCarrier(const char *defVal)
{
    if (defVal == NULL) {
        defVal = "";
    }
    return SystemProperty::Get(RO_CARRIER, defVal);
}

string CarrierLoader::GetVendorCarrier()
{
    return CarrierLoader::GetVendorCarrier("");
}

string CarrierLoader::GetVendorCarrier(const char *defVal)
{
    if (defVal == NULL) {
        defVal = "";
    }
    return SystemProperty::Get(RO_VENDOR_CARRIER, defVal);
}
