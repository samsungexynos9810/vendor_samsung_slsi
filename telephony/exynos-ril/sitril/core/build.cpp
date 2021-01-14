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
 * build.cpp
 *
 *  Created on: 2015. 7. 20.
 */

#include "build.h"
#include "systemproperty.h"


string Build::ID()
{
    return SystemProperty::Get("ro.build.id");
}

string Build::Display()
{
    return SystemProperty::Get("ro.build.display.id");
}

string Build::Product()
{
    return SystemProperty::Get("ro.product.name");
}

string Build::Device()
{
    return SystemProperty::Get("ro.product.device");
}

string Build::Board()
{
    return SystemProperty::Get("ro.product.board");
}

string Build::Manufacturer()
{
    return SystemProperty::Get("ro.product.manufacturer");
}

string Build::Brand()
{
    return SystemProperty::Get("ro.product.brand");
}

string Build::Model()
{
    return SystemProperty::Get("ro.product.model");
}

string Build::Bootloader()
{
    return SystemProperty::Get("ro.bootloader");
}

string Build::Hardware()
{
    return SystemProperty::Get("ro.hardware");
}

string Build::Serial()
{
    return SystemProperty::Get("ro.serialno");
}

string Build::SupportedABIS()
{
    return SystemProperty::Get("ro.product.cpu.abilist");
}

string Build::Supported32bitABIS()
{
    return SystemProperty::Get("ro.product.cpu.abilist32");
}

string Build::Supported64bitABIS()
{
    return SystemProperty::Get("ro.product.cpu.abilist64");
}

string Build::Type()
{
    return SystemProperty::Get("ro.build.type");
}
