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
 * build.h
 *
 *  Created on: 2015. 7. 20.
 */

#ifndef __BUILD_H__
#define __BUILD_H__

#include <string>
using namespace std;

class Build
{
public:
    static string ID();
    static string Display();
    static string Product();
    static string Device();
    static string Board();
    static string Manufacturer();
    static string Brand();
    static string Model();
    static string Bootloader();
    static string Hardware();
    static string Serial();
    static string SupportedABIS();
    static string Supported32bitABIS();
    static string Supported64bitABIS();
    static string Type();
};

#endif /* __BUILD_H__ */
