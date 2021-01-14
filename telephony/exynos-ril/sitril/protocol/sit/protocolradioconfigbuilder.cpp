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
 * protocolradioconfigbuilder.cpp
 *
 *  Created on: 2019. 8. 13.
 */

#include "protocolradioconfigbuilder.h"

ModemData *ProtocolRadioConfigBuilder::BuildGetPhoneCapability()
{
    null_data_format req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_PHONE_CAPABILITY);
    return new ModemData((char *)&req, length);
}
