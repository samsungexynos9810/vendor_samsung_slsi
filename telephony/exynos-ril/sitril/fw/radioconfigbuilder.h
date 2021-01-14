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
 * radioconfigbuilder.h
 *
 *  Created on: 2019. 8. 13.
 */

#ifndef __RADIO_CONFIG_BUILDER_H__
#define __RADIO_CONFIG_BUILDER_H__

#include "rildatabuilder.h"

class RadioConfigBuildler : public RilDataBuilder {
public:
    const RilData *BuildPhoneCapability(int maxActiveData, int maxActiveInternetData,
            bool isInternetLingeringSupported, int logicalModemListSize, int *logicalModemList);
};

#endif /* __RADIO_CONFIG_BUILDER_H__ */
