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
 * systemproperty.cpp
 *
 *  Created on: 2015. 7. 18.
 */
#include "systemproperty.h"
#include "textutils.h"
#include <cutils/properties.h>

#define MAX_SIZE 126

string SystemProperty::Get(const string &name)
{
    string defValue("");
    return SystemProperty::Get(name, defValue);
}

string SystemProperty::Get(const string &name, const string &defValue)
{
    char buf[MAX_SIZE] = {0, };
    property_get(name.c_str(), buf, defValue.c_str());
    return string(buf);
}

string SystemProperty::Get(const char *name)
{
    return SystemProperty::Get(name, (const char *)"");
}

string SystemProperty::Get(const char *name, const char *defValue)
{
    char buf[MAX_SIZE] = {0, };
    if (!TextUtils::IsEmpty(name) && defValue != NULL) {
        property_get(name, buf, defValue);
    }
    return string(buf);
}

int SystemProperty::GetInt(const char *name)
{
    return SystemProperty::GetInt(name, INT_MAX);
}

int SystemProperty::GetInt(const char *name, int defValue)
{
    string ret = SystemProperty::Get(name);
    if (ret.length() == 0) {
        return defValue;
    }
    return TextUtils::ParseInt(ret);
}

void SystemProperty::Set(const string &name, const string &value)
{
    if (name.length() > 0) {
        property_set(name.c_str(), value.c_str());
    }
}

void SystemProperty::Set(const char *name, const char *value)
{
    if (!TextUtils::IsEmpty(name) && value != NULL) {
        property_set(name, value);
    }
}

void SystemProperty::Set(const string &name, int value)
{
    stringstream ss;
    ss << value;
    SystemProperty::Set(name, ss.str());
}

void SystemProperty::Set(const char *name, int value)
{
    if (!TextUtils::IsEmpty(name)) {
        string strName(name);
        SystemProperty::Set(strName, value);
    }
}
