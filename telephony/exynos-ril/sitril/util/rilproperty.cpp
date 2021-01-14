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
 * rilproperty.h
 *
 *  Created on: 2015. 1. 9.
 */

#include "rilproperty.h"
#include "rillog.h"
#include "util.h"

IMPLEMENT_MODULE_TAG(RilProperty, RilProperty)
static bool debug = false;


RilProperty::RilProperty()
{
    m_sockId = 9;
    snprintf(m_modName, sizeof(m_modName)-1, "%s", TAG);
}

RilProperty::~RilProperty()
{
    mData.clear();
}

void RilProperty::SetSocketId(int sockId)
{
    m_sockId = sockId;
    snprintf(m_modName, sizeof(m_modName)-1, "%s_%d", TAG, m_sockId);
}

int RilProperty::GetInt(string &key)
{
    return GetInt(key, 0);
}

int RilProperty::GetInt(string &key, int defValue)
{
    map<string, string>::iterator iter = mData.find(key);
    if (iter != mData.end()) {
        string value = iter->second;
        if (debug) RilLogV("%s GetInt(%s) value %s defaValue %d", m_modName, key.c_str(), value.c_str(), defValue);
        return atoi(value.c_str());
    }

    if (debug) RilLogV("%s GetInt(%s) defaValue %d", m_modName, key.c_str(), defValue);
    return defValue;
}

int RilProperty::GetInt(const char *key)
{
    return GetInt(key, 0);
}

int RilProperty::GetInt(const char *key, int defValue)
{
    if (key == NULL)
        key = "";
    string k = key;
    return GetInt(k, defValue);
}

string RilProperty::GetString(string &key)
{
    string v = "";
    return GetString(key, v);
}

string RilProperty::GetString(string &key, string &defValue)
{
    map<string, string>::iterator iter = mData.find(key);
    if (iter != mData.end()) {
        string value = iter->second;
        if (debug) RilLogV("%s GetString(%s) value %s", m_modName, key.c_str(), value.c_str());
        return value;
    }
    if (debug) RilLogV("%s GetString(%s) defValue %s", m_modName, key.c_str(), defValue.c_str());
    return defValue;
}

string RilProperty::GetString(const char *key)
{
    if (key == NULL)
        key = "";
    return GetString(key, "");
}

string RilProperty::GetString(const char *key, const char *defValue)
{
    if (key == NULL)
        key = "";
    if (defValue == NULL)
        defValue = "";
    string k = key;
    string v = defValue;
    return GetString(k, v);
}

bool RilProperty::GetBool(string &key)
{
    return GetBool(key, false);
}

bool RilProperty::GetBool(string &key, bool defValue)
{
    string value = GetString(key);
    if (TextUtils::Equals(value.c_str(), "true") || TextUtils::Equals(value.c_str(), "1")) {
        return true;
    }
    else if (TextUtils::Equals(value.c_str(), "false") || TextUtils::Equals(value.c_str(), "0")){
        return false;
    }

    if (debug) RilLogV("%s GetString(%s) defValue %d", TAG, key.c_str(), defValue ? "true" : "false");
    return defValue;
}

bool RilProperty::GetBool(const char *key)
{
    return GetBool(key, false);
}

bool RilProperty::GetBool(const char *key, bool defValue)
{
    if (key == NULL)
        key = "";
    string k = key;
    return GetBool(k, defValue);
}

void RilProperty::Put(string &key, int value)
{
    char buf[15] = { 0, };
    snprintf(buf, sizeof(buf), "%d", value);
    string v = buf;
    Put(key, v);
}

void RilProperty::Put(const char *key, int value)
{
    if (key == NULL)
        key = "";
    string k = key;
    Put(k, value);
}

void RilProperty::Put(string &key, string &value)
{
    if (key.size() == 0) {
        if (debug) RilLogV("%s Put invalid key", m_modName);
    }
    else {
        if (debug) RilLogV("%s Put(%s,%s)", m_modName, key.c_str(), value.c_str());
        mLock.lock();
        mData[key] = value;
        mLock.unlock();
    }
}

void RilProperty::Put(const char *key, const char *value)
{
    if (key == NULL)
        key = "";
    if (value == NULL)
        value = "";
    string k = key;
    string v = value;
    Put(k, v);
}

void RilProperty::Put(string &key, bool value)
{
    string v = value ? "true" : "false";
    Put(key, v);
}

void RilProperty::Put(const char *key, bool value)
{
    if (key == NULL)
        key = "";
    string k = key;
    Put(k, value);
}

void RilProperty::Dump() const
{
    RilLogV("RilProperty Dump:");
    map<string, string>::const_iterator iter;
    for (iter = mData.begin(); iter != mData.end(); ++iter) {
        string key = iter->first;
        string value = iter->second;
        RilLogV("<%s,%s>", key.c_str(), value.c_str());
    } // end for iter ~
}

bool RilProperty::IsUserMode(void)
{
    char szBuff[PROP_VALUE_MAX];
    memset(szBuff, 0x00, sizeof(szBuff));
    property_get(RO_BUILD_TYPE, szBuff, "none");

    if (strcmp(szBuff, "user") == 0) {
        return true;
    }
    return false;
}

void SetCallWaiting(bool bEnable, RIL_SOCKET_ID socketid)
{
    char szProp[64]  ={0,};
    char szBuff[PROP_VALUE_MAX];
    memset(szBuff, 0x00, sizeof(szBuff));
    snprintf(szBuff, sizeof(szBuff)-1, "%d", bEnable);
    sprintf(szProp, "%s%d", RIL_CALL_WAITING_PREFIX, (int)socketid);

    //RilLogV("set property : %s : %s", szProp, szBuff);
    property_set(szProp, szBuff);
}

bool GetCallWaiting(RIL_SOCKET_ID socketid)
{
    char szProp[64]  ={0,};
    char szBuff[PROP_VALUE_MAX];
    memset(szBuff, 0x00, sizeof(szBuff));
    sprintf(szProp, "%s%d", RIL_CALL_WAITING_PREFIX, (int)socketid);

    property_get(szProp, szBuff, "1");
    bool bEnable = atoi(szBuff)==1?true:false;
    return bEnable;
}

