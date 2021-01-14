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
 * telephonyprovider.cpp
 *
 *  Created on: 2014. 10. 24.
 *      Author: sungwoo48.choi
 */
#include <sqlite3.h>
#include "telephonyprovider.h"
#include "rillog.h"
#include "build.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_TELPRO, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_TELPRO, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_TELPRO, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_TELPRO, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

// attach PDN connection type
#define PROPERTY_ATTACH_APN_TYPE    "persist.vendor.ril.attachtype"

static bool debug = false;


// singleton instance
IMPLEMENT_MODULE_TAG(TelephonyProvider, TelephonyProvider)

TelephonyProvider TelephonyProvider::instance;


/**
 * Deprecated
 *
 * all accessing database are deprecated by "neverallow" rule of seoplicy.
 */
TelephonyProvider *TelephonyProvider::GetInstance()
{
    string type = Build::Type();
    debug = !TextUtils::Equals(type.c_str(), "user");

    return &instance;
}


TelephonyProvider::TelephonyProvider()
{
    // do not use yet
    m_AttachApnTypeTable.insert(pair<string, string>("46000", APN_TYPE_IMS));
    m_AttachApnTypeTable.insert(pair<string, string>("46002", APN_TYPE_IMS));
    m_AttachApnTypeTable.insert(pair<string, string>("46007", APN_TYPE_IMS));
    m_AttachApnTypeTable.insert(pair<string, string>("45008", APN_TYPE_IMS));
}

TelephonyProvider::~TelephonyProvider()
{

}

ApnSetting *TelephonyProvider::FindAttachApnSetting(const char *carrier, const char *apnType/* = APN_TYPE_IA*/)
{
    // deprecated since Android P
    // neverallow to access coredomain
    return NULL;
}

ApnSetting *TelephonyProvider::FindAttachApnSetting(const char *carrier, const char *apn, const char *protocol, int authtype)
{
    // deprecated since Android P
    // neverallow to access coredomain
    return NULL;
}

ApnSetting *TelephonyProvider::FindApnSetting(const char *carrier, const char *apn)
{
    // deprecated since Android P
    // neverallow to access coredomain
    return NULL;
}

ApnSetting *TelephonyProvider::FindApnSetting(const char *carrier, const char *apn, const char *protocol, int authtype)
{
    // deprecated since Android P
    // neverallow to access coredomain
    return NULL;
}

ApnSetting *TelephonyProvider::FindPreferredApnSetting()
{
    return NULL;
}

const char *TelephonyProvider::GetAttachApnType(const char *carrier)
{
    char buf[256] = { 0 };
    property_get(PROPERTY_ATTACH_APN_TYPE, buf, "");
    if (*buf != 0) {
        if (strcmp(buf, APN_TYPE_IMS) == 0) {
            RilLogV("[%s] test attach apn type APN_TYPE_IMS", TAG);
            return APN_TYPE_IMS;
        }
        else if (strcmp(buf, APN_TYPE_IA) == 0) {
            RilLogV("[%s] test attach apn type APN_TYPE_IA", TAG);
            return APN_TYPE_IA;
        }
        else if (strcmp(buf, APN_TYPE_DEFAULT) == 0) {
            RilLogV("[%s] test attach apn type APN_TYPE_DEFAULT", TAG);
            return APN_TYPE_DEFAULT;
        }
    }

    //RilLogE("%s %s carrier=%s", TAG, __FUNCTION__, carrier);

    // no valid value in system property, use hard-coded table
    if (carrier != NULL) {
        map<string, string>::iterator iter = m_AttachApnTypeTable.find(carrier);
        if (iter != m_AttachApnTypeTable.end()) {
            //RilLogV("[%s] carrier=%s attach apn type %s", TAG, carrier, (iter->second).c_str());
            return (iter->second).c_str();
        }
    }

    //RilLogV("[%s] carrier=%s attach apn type APN_TYPE_DEFAULT", TAG, carrier);
    return APN_TYPE_DEFAULT;
}

bool TelephonyProvider::SetAttachApnType(const char *carrier, const char *apnType)
{
    if (TextUtils::IsEmpty(carrier) || TextUtils::IsEmpty(carrier)) {
        return false;
    }

    string old = GetAttachApnType(carrier);
    m_AttachApnTypeTable[carrier] = apnType;
    RilLogV("Update attach APN type %s -> %s", old.c_str(), apnType);
    return true;
}

ApnSetting *TelephonyProvider::FindApnSettingByApnType(const char *carrier, const char *apnType, bool carrierEnabled/* = false*/)
{
    // deprecated since Android P
    // neverallow to access coredomain
    return NULL;
}
