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
 * systemproperty.h
 *
 *  Created on: 2015. 7. 18.
 */

#ifndef __SYSTEM_PROPERTY_H__
#define __SYSTEM_PROPERTY_H__

#include <string>
#include <sstream>
using namespace std;

class SystemProperty
{
public:
    static string Get(const string &name);
    static string Get(const string &name, const string &defValue);
    static string Get(const char *name);
    static string Get(const char *name, const char *defValue);
    static int GetInt(const char *name);
    static int GetInt(const char *name, int defValue);
    static void Set(const string &name, const string &value);
    static void Set(const char *name, const char *value);
    static void Set(const string &name, int value);
    static void Set(const char *name, int value);
};

#endif /* __SYSTEM_PROPERTY_H__ */
