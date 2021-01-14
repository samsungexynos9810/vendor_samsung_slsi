/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __UTILS_H__
#define __UTILS_H__

int32_t commandcmp(const char *src, const char *dst, int length);

int32_t set_sysfs(const char *devnm, const char *cmd);

int32_t GetSysFS(const char *devnm, char *buffer);

void PrintATCommands(const char *tag, const char *cmd, int cmdlen);
void HexDump(const char *data, size_t datalen);
#endif  //__UTILS_H__
