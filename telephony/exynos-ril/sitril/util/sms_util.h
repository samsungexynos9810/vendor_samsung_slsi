/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __SMS_UTIL_H__
#define __SMS_UTIL_H__

#include "rildef.h"

int ConvertSmscBcdToNumber(const char *tpdu, int tpduLen, char *smsc, int smscLen);
int ConvertSmscNumberToBcd(const char *smsc, int smscLen, char *tpdu, int tpduLen);
bool IsSmscLenValid(const char *smsc, int smscLen);

#endif /* __SMS_UTIL_H__ */

