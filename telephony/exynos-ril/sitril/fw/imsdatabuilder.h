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
 * miscdatabuilder.h
 *
 *  Created on: 2014. 11. 24.
 *      Author: sungwoo48.choi
 */

#ifndef __IMS_DATA_BUILDER_H__
#define __IMS_DATA_BUILDER_H__

#include "rildatabuilder.h"

class ImsDataBuilder : public RilDataBuilder {
public:
    const RilData *BuildImsGetConfigResponse(const char * pResponse, int Length);
    const RilData *BuildImsGetFrameSyncResponse(const char * pResponse, int Length);
    const RilData *BuildImsGbaSimAuth(const char * pResponse, int Length);

//AIMS support start ---------------------
    const RilData *BuildAImsGetFrameTimeResponse(const char * pResponse, int Length);
    const RilData *BuildAImsCallModifyResponse(const char * pResponse, int Length);
    const RilData *BuildAImsGetDataResponse(const char * pResponse, int Length);
    const RilData *BuildAImsGetCallForwardResponse(const char * pResponse, int Length);
    const RilData *BuildAImsGetCallWaitingResponse(const char * pResponse, int Length);
    const RilData *BuildAImsGetCallBarringResponse(const char * pResponse, int Length);
    const RilData *BuildAImsSendSMSResponse(const char * pResponse, int Length);
    const RilData *BuildAImsSendExpectMoreResponse(const char * pResponse, int Length);
//AIMS support end ---------------------

};

#endif /* __IMS_DATA_BUILDER_H__ */
