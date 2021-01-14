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
 * textutils.h
 *
 *  Created on: 2015. 1. 5.
 *      Author: sungwoo48.choi
 */

#ifndef __TEXT_UTILS_H__
#define __TEXT_UTILS_H__

#include <string.h>
#include <string>
using namespace std;

class TextUtils {
public:
    static bool IsEmpty(const char *str);
    static bool Equals(const char *l, const char *r);
    static bool IsEmpty(const string &str);
    static bool Equals(const string &l, const string &r);
    static bool IsDigitsOnly(const char *str);
    static bool IsDigitsOnly(const string &str);
    static bool DupString(char **dest, char *src, bool allowEmpty = true);
    static int ParseInt(const char *str);
    static int ParseInt(const string &str);
};

#endif /* __TEXT_UTILS_H__ */
