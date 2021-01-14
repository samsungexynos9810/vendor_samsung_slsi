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
 * apnsetting.cpp
 *
 *  Created on: 2014. 7. 8.
 *      Author: sungwoo48.choi
 */
#include "apnsetting.h"
#include "mcctable.h"
#include "textutils.h"
#include "rillog.h"

#include <string>
#include <sstream>

using namespace std;

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

static bool DEBUG = false;

ApnSetting::ApnSetting() : m_id(-1)
{
    m_protocol = DATA_PROTOCOL_IP;
    m_authType = SETUP_DATA_AUTH_NONE;

    memset(m_carrier, 0, sizeof(m_carrier));
    memset(m_apn, 0, sizeof(m_apn));
    memset(m_type, 0, sizeof(m_type));
    memset(m_username, 0, sizeof(m_username));
    memset(m_password, 0, sizeof(m_password));
    m_roaming_protocol = DATA_PROTOCOL_IP;
    mSupportedTypesBitmask = APN_TYPE_BIT_NONE;
}

bool ApnSetting::CanHandleType(const char *apnType)
{
    if (strcmp(m_type, APN_TYPE_ALL) == 0) {
        return true;
    }

    if (apnType == NULL) {
        return false;
    }

    return (strcasestr(m_type, apnType) == NULL ? false : true);
}

ApnSetting *ApnSetting::Clone() const
{
    ApnSetting *newInstance = new ApnSetting();
    if (newInstance != NULL) {
        newInstance->m_id = m_id;
        newInstance->m_protocol = m_protocol;
        newInstance->m_authType = m_authType;

        memcpy(newInstance->m_carrier, m_carrier, sizeof(m_carrier));
        memcpy(newInstance->m_apn, m_apn, sizeof(m_apn));
        memcpy(newInstance->m_type, m_type, sizeof(m_type));
        memcpy(newInstance->m_username, m_username, sizeof(m_username));
        memcpy(newInstance->m_password, m_password, sizeof(m_password));

        newInstance->m_roaming_protocol = m_roaming_protocol;
        newInstance->mSupportedTypesBitmask = mSupportedTypesBitmask;
    }

    return newInstance;
}

bool ApnSetting::Equals(ApnSetting *rhs) const
{
    if (rhs == NULL) {
        return false;
    }

    bool bAuthtype = m_authType == rhs->m_authType;
    bool bProtocol = TextUtils::Equals(m_protocol, rhs->m_protocol);
    bool bApn = TextUtils::Equals(m_apn, rhs->m_apn);
    bool bUsername = TextUtils::Equals(m_username, rhs->m_username);
    bool bPassword = TextUtils::Equals(m_password, rhs->m_password);
    bool bRoamingProtocol = TextUtils::Equals(m_roaming_protocol, rhs->m_roaming_protocol);
    if (DEBUG) {
        RilLogV("bAuthtype=%d bProtocol=%d bApn=%d bUsername=%d bPassword=%d bRoamingProtocol=%d",
                bAuthtype, bProtocol, bApn, bUsername, bPassword, bRoamingProtocol);
    }

    return (bAuthtype && bProtocol && bApn && bUsername && bPassword);
}

void ApnSetting::SetProtocol(const char *&to, const char *&from)
{
    static const char *AVAILABLE_DATA_PROTOCOL[] = {
        DATA_PROTOCOL_IP,
        DATA_PROTOCOL_IPV6,
        DATA_PROTOCOL_IPV4V6,
        NULL
    };
    int size = sizeof(AVAILABLE_DATA_PROTOCOL) / sizeof(AVAILABLE_DATA_PROTOCOL[0]);
    for (int i = 0; i < size; i++) {
        if (from == NULL || AVAILABLE_DATA_PROTOCOL[i] == NULL) {
            to = DATA_PROTOCOL_IP;    // default
            break;
        }
        if (strcasecmp(AVAILABLE_DATA_PROTOCOL[i], from) == 0) {
            to = AVAILABLE_DATA_PROTOCOL[i];
            break;
        }
    } // end for i ~
}

ApnSetting *ApnSetting::NewInstance(const char *carrier, const char *apn,
    int supportedTypesBitmask, const char *username, const char *password,
    const char *protocol, const char *roaming_protocol, int authType/* = SETUP_DATA_AUTH_NONE*/)
{
    ApnSetting *apnSetting = new ApnSetting();
    if (!TextUtils::IsEmpty(carrier)) {
        strncpy(apnSetting->m_carrier, carrier, MAX_PLMN_LEN);
    }

    if (!TextUtils::IsEmpty(apn)) {
        strncpy(apnSetting->m_apn, apn, MAX_PDP_APN_LEN - 1);
    }

    apnSetting->SetSupportedTypesBitmask(supportedTypesBitmask);

    SetProtocol(apnSetting->m_protocol, protocol);
    SetProtocol(apnSetting->m_roaming_protocol, roaming_protocol);

    if (!TextUtils::IsEmpty(username)) {
        strncpy(apnSetting->m_username, username, MAX_AUTH_USER_NAME_LEN - 1);
    }
    if (!TextUtils::IsEmpty(password)) {
        strncpy(apnSetting->m_password, password, MAX_AUTH_PASSWORD_LEN - 1);
    }

    switch (authType) {
    case SETUP_DATA_AUTH_NONE:
    case SETUP_DATA_AUTH_CHAP:
    case SETUP_DATA_AUTH_PAP:
    case SETUP_DATA_AUTH_PAP_CHAP:
        apnSetting->m_authType = authType;
        break;
    default:
        apnSetting->m_authType = SETUP_DATA_AUTH_NONE;
        break;
    } // end switch ~

    return apnSetting;
}

string ApnSetting::ToString() const
{
    stringstream ss;
    ss << "ApnSetting{";
    ss << "_id=";
    ss << m_id;
    ss << ",apn=";
    ss << m_apn;
    ss << ",type=";
    ss << m_type;
    ss << ",authtype=";
    ss << m_authType;
    ss << ",protocol=";
    ss << m_protocol;
    ss << ",username=";
    ss << m_username;
    ss << ",password=";
    ss << m_password;
    ss << ",m_roaming_protocol=";
    ss << m_roaming_protocol;
    ss << "}";
    return ss.str();
}

void ApnSetting::SetSupportedTypesBitmask(int bitmask) {
    mSupportedTypesBitmask = (bitmask & APN_TYPE_BIT_ALL);
    memset(m_type, 0, MAX_PDP_APN_LEN);

    if (mSupportedTypesBitmask == APN_TYPE_BIT_ALL) {
        strncpy(m_type, APN_TYPE_ALL, strlen(APN_TYPE_ALL));
    }
    else {
        stringstream ss;
        const char *types[] = {
            APN_TYPE_DEFAULT,
            APN_TYPE_MMS,
            APN_TYPE_SUPL,
            APN_TYPE_DUN,
            APN_TYPE_HIPRI,
            APN_TYPE_FOTA,
            APN_TYPE_IMS,
            APN_TYPE_CBS,
            APN_TYPE_IA,
            APN_TYPE_EMERGENCY,
        };

        bool match = false;
        size_t size = sizeof(types) / sizeof(types[0]);
        for (size_t i = 0; i < size; i++) {
            if (mSupportedTypesBitmask & (1 << i)) {
                if (match)
                    ss << ",";
                ss << types[i];
                match = true;
            }
        } // end for i ~

        if (match) {
            strncpy(m_type, ss.str().c_str(), MAX_PDP_APN_LEN - 1);
        }
    }
    RilLogV("supportedTypesBitmask=0x%0X types=%s", mSupportedTypesBitmask, m_type);
}

void ApnSetting::UpdateProtocol(const char *from)
{
    SetProtocol(m_protocol, from);
}

/*
// DEPRECATED
int ApnSetting::GetApnBitmask(const char *apn) {
    if (TextUtils::IsEmpty(apn))
        return APN_TYPE_BIT_NONE;

    if (TextUtils::Equals(apn, APN_TYPE_DEFAULT)) {
        return APN_TYPE_BIT_DEFAULT;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_MMS)) {
        return APN_TYPE_BIT_MMS;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_SUPL)) {
        return APN_TYPE_BIT_SUPL;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_DUN)) {
        return APN_TYPE_BIT_DUN;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_HIPRI)) {
        return APN_TYPE_BIT_HIPRI;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_FOTA)) {
        return APN_TYPE_BIT_FOTA;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_IMS)) {
        return APN_TYPE_BIT_IMS;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_CBS)) {
        return APN_TYPE_BIT_CBS;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_IA)) {
        return APN_TYPE_BIT_IA;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_EMERGENCY)) {
        return APN_TYPE_BIT_EMERGENCY;
    }
    else if (TextUtils::Equals(apn, APN_TYPE_ALL)) {
        return APN_TYPE_BIT_ALL;
    }

    return APN_TYPE_BIT_NONE;
}

// DEPRECATED
int ApnSetting::GetApnBitmask(const string &apn) {
    return GetApnBitmask(apn.c_str());
}

// DEPRECATED
string ApnSetting::GetApnBitmaskToType(int bitmask) {
    const char *apn = "";
    switch (bitmask) {
    case APN_TYPE_BIT_DEFAULT:
        apn = APN_TYPE_DEFAULT;
        break;
    case APN_TYPE_BIT_MMS:
        apn = APN_TYPE_MMS;
        break;
    case APN_TYPE_BIT_SUPL:
        apn = APN_TYPE_SUPL;
        break;
    case APN_TYPE_BIT_DUN:
        apn = APN_TYPE_DUN;
        break;
    case APN_TYPE_BIT_HIPRI:
        apn = APN_TYPE_HIPRI;
        break;
    case APN_TYPE_BIT_FOTA:
        apn = APN_TYPE_FOTA;
        break;
    case APN_TYPE_BIT_IMS:
        apn = APN_TYPE_IMS;
        break;
    case APN_TYPE_BIT_CBS:
        apn = APN_TYPE_CBS;
        break;
    case APN_TYPE_BIT_IA:
        apn = APN_TYPE_IA;
        break;
    case APN_TYPE_BIT_EMERGENCY:
        apn = APN_TYPE_EMERGENCY;
        break;
    case APN_TYPE_BIT_ALL:
        apn = APN_TYPE_ALL;
        break;
    }

    return string(apn);
}
*/
