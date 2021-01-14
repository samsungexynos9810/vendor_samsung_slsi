/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include "util.h"
#include "rillog.h"
#include <map>
#include "rilproperty.h"

int RilReset(const char* pReason)
{
    RilLogE("RIL RESET by %s", pReason == NULL?"Unknown":pReason);
    exit(0);
    return 0;
}

int RilErrorReset(const char* pReason)
{
    RilLogE("RIL Error RESET by %s", pReason == NULL?"Unknown":pReason);
    if ( RilProperty::IsUserMode() == true )
    {
        // TODO silent reset without restart process
        exit(0);
    }
    else
    {
        exit(0);
    }
    return 0;
}
