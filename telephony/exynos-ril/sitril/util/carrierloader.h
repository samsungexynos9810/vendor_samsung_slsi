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
 * opeartorloader.h
 *
 *  Created on: 2019. 5. 22.
 */

#ifndef __CARRIER_LOADER_H__
#define __CARRIER_LOADER_H__

#include "rildef.h"
#include <map>
#include <string>
using namespace std;

class CarrierLoader {
    DECLARE_MODULE_TAG()
private:
    CarrierLoader();
private:
    static CarrierLoader instance;
    map<string, int> mCarrierMap;
public:
    int GetTargetOperator();
    int GetVendorTargetOperator();
    int GetTargetOperator(string carrier);
    int GetTargetOperator(const char *carrier);
public:
    static CarrierLoader& GetInstance();
    static string GetCarrier();
    static string GetCarrier(const char *defVal);
    static string GetVendorCarrier();
    static string GetVendorCarrier(const char *defVal);
};

#endif // __CARRIER_LOADER_H__
