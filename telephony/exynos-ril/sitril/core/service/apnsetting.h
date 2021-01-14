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
 * ApnSetting.h
 *
 *  Created on: 2014. 7. 8.
 *      Author: sungwoo48.choi
 */

#ifndef __APN_SETTING_H__
#define __APN_SETTING_H__

#include "rildef.h"

class ApnSetting
{
private:
    int m_id; // DEPRECATED? Reuse for ProfileID?
    char m_carrier[MAX_PLMN_LEN + 1];
    char m_apn[MAX_PDP_APN_LEN];
    char m_type[MAX_PDP_APN_LEN];
    char m_username[MAX_AUTH_USER_NAME_LEN];
    char m_password[MAX_AUTH_PASSWORD_LEN];
    const char *m_protocol;
    int m_authType;
    const char *m_roaming_protocol;
    int mSupportedTypesBitmask;

private:
    ApnSetting();
public:
    virtual ~ApnSetting() {}

public:
    bool CanHandleType(const char *apnType);
    int GetId() const { return m_id; }
    const char *GetCarrier() const { return *m_carrier == 0 ? NULL : m_carrier; }
    const char *GetApn() const { return m_apn; }
    const char *GetUsername() const { return *m_username == 0 ? NULL : m_username; }
    const char *GetPassword() const { return *m_password == 0 ? NULL : m_password; }
    const char *GetType() const { return m_type; }
    const char *GetProtocol() const { return m_protocol; }
    const char *GetRoamingProtocol() const { return m_roaming_protocol; }
    int GetAuthType() const { return m_authType; }
    int GetSupportedTypesBitmask() const { return mSupportedTypesBitmask; }
    ApnSetting *Clone() const;
    bool Equals(ApnSetting *lhs) const;
    string ToString() const;
    static void SetProtocol(const char *&to, const char *&from);
    void SetSupportedTypesBitmask(int bitmask);
    void UpdateProtocol(const char *from);

public:
    // DEPRECATED
    //static int GetApnBitmask(const char *apn);
    // DEPRECATED
    //static int GetApnBitmask(const string &apn);
    // DEPRECATED
    //static string GetApnBitmaskToType(int bitmask);

public:
    static ApnSetting *NewInstance(const char *carrier, const char *apn, int supportedTypesBitmask, const char *username, const char *password,
                                        const char *protocol, const char *roaming_protocol, int authType = SETUP_DATA_AUTH_NONE);
};


#endif /* __APN_SETTING_H__ */
