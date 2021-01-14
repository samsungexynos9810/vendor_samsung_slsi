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
 * telephonyprovider.h
 *
 *  Created on: 2014. 10. 24.
 *      Author: sungwoo48.choi
 */

#ifndef __TELEPHONY_PROVIDER_H__
#define __TELEPHONY_PROVIDER_H__

#include <map>
#include <string>
#include "apnsetting.h"

using namespace std;

class TelephonyProvider {
    DECLARE_MODULE_TAG()

private:
    static TelephonyProvider instance;
    TelephonyProvider();
public:
    ~TelephonyProvider();

private:
    map<string, string> m_AttachApnTypeTable;

public:
    static TelephonyProvider *GetInstance();
    ApnSetting *FindAttachApnSetting(const char *carrier, const char *apnType = APN_TYPE_IA);
    ApnSetting *FindAttachApnSetting(const char *carrier, const char *apn, const char *protocol, int authtype);
    ApnSetting *FindPreferredApnSetting();
    ApnSetting *FindApnSetting(const char *carrier, const char *apn);
    ApnSetting *FindApnSetting(const char *carrier, const char *apn, const char *protocol, int authtype);
    ApnSetting *FindApnSettingByApnType(const char *carrier, const char *apnType, bool carrierEnabled = false);

    const char *GetAttachApnType(const char *carrier);
    bool SetAttachApnType(const char *carrier, const char *apnType);
};


#endif /* __TELEPHONY_PROVIDER_H__ */
