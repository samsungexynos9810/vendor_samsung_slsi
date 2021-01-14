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
 * radioconfigbuilder.cpp
 *
 *  Created on: 2019. 8. 13.
 */

#include "radioconfigbuilder.h"

const RilData *RadioConfigBuildler::BuildPhoneCapability(int maxActiveData, int maxActiveInternetData,
        bool isInternetLingeringSupported, int logicalModemListSize, int *logicalModemList)
{
    // VTS IRadioConfig#1_2
    //  maxActiveData should be greater than or equal to maxActiveInternetData.
    if (!(maxActiveData >= maxActiveInternetData)) {
        return NULL;
    }

    // maxActiveData and maxActiveInternetData should be 0 or positive numbers.
    if (!(maxActiveInternetData >= 0)) {
        return NULL;
    }

    // logicalModemListSize should be positive
    if (!(logicalModemListSize > 0)) {
        return NULL;
    }

    RIL_PhoneCapability capability;
    memset(&capability, 0, sizeof(capability));

    capability.maxActiveData = maxActiveData;
    capability.maxActiveInternetData = maxActiveInternetData;
    capability.isInternetLingeringSupported = isInternetLingeringSupported;

    if (logicalModemListSize > MAX_LOGICAL_MODEM_SIZE) {
        logicalModemListSize = MAX_LOGICAL_MODEM_SIZE;
    }

    capability.len_logicalModemList = logicalModemListSize;
    if (logicalModemListSize > 0) {
        for (int i = 0; i < logicalModemListSize; i++) {
            uint8_t modemId = i;
            // if logicalModemList NULL, set modemId as an index
            if (logicalModemList != NULL) {
                modemId = (uint8_t)logicalModemList[i];
            }
            capability.logicalModemList[i].modemId = modemId;
        } // end for i ~
    }

    return new RilDataRaw(&capability, sizeof(capability));
}




