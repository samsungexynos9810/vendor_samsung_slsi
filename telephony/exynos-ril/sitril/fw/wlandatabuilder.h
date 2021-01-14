/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __WLAN_DATA_BUILDER_H__
#define __WLAN_DATA_BUILDER_H__

#include "rildatabuilder.h"

class WLanDataBuilder : public RilDataBuilder {
public:
    const RilData *BuildSimAuthenticate(BYTE authType, BYTE authLen, BYTE *pBuffer);
};

#endif /* __WLAN_DATA_BUILDER_H__ */
