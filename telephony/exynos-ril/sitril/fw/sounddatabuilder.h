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
 * sounddatabuilder.h
 *
 *  Created on: 2014. 11. 24.
 *      Author: sungwoo48.choi
 */

#ifndef __SOUND_DATA_BUILDER_H__
#define __SOUND_DATA_BUILDER_H__

#include "rildatabuilder.h"

class SoundDataBuilder : public RilDataBuilder {
public:
    const RilData *BuildGetMuteResponse(int muteMode);
    const RilData *BuildWBAMRReportUnsolResponse(int status, int calltype);
};

#endif /* __SOUND_DATA_BUILDER_H__ */
