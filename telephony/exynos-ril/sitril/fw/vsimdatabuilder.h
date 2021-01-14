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
 * vsimdatabuilder.h
 *
 *  Created on: 2016. 02. 26.
 */

#ifndef __VSIM_DATA_BUILDER_H__
#define __VSIM_DATA_BUILDER_H__

#include "rildatabuilder.h"

class VsimDataBuilder : public RilDataBuilder {
public:
    const RilData *BuildVsimOperation(int tid, int eventid, int result, int datalen, const char* data);
    const RilData *BuildVsimOperationExt(int tid, int eventid, int result, int datalen, const char* data);
};

#endif /* __VSIM_DATA_BUILDER_H__ */
