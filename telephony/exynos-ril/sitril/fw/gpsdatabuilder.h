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
 * gpsdatabuilder.h
 *
 *  Created on: 2015. 1. 23.
 *      Author: m.afzal
 */

#ifndef __GPS_DATA_BUILDER_H__
#define __GPS_DATA_BUILDER_H__

#include "rildatabuilder.h"

class GpsDataBuilder : public RilDataBuilder {
public:
    const RilData *BuildFreqAidingResponse( const char * pResponse, int Length);
    const RilData *BuildLPPSuplEcidInfoResponse( const char * pResponse, int Length);
    const RilData *BuildRRLPSuplEcidInfoResponse( const char * pResponse, int Length);
    const RilData *BuildMOLocationResponse( const char * pResponse, int Length);
    const RilData *BuildLPPServingCellInfoResponse( const char * pResponse, int Length);
    const RilData *BuildSuplNIReadyResponse( const char * pResponse, int Length);
    const RilData *BuildMeasurePositionIndication( const char * pResponse, int Length);
    const RilData *BuildAssistDataIndication( const char * pResponse, int Length);
    const RilData *BuildMTLocReqIndication( const char * pResponse, int Length);
    const RilData *BuildLppReqCapIndication( const char * pResponse, int Length);
    const RilData *BuildLppProvideAssistDataIndication( const char * pResponse, int Length);
    const RilData *BuildLppReqLocInfoIndication( const char * pResponse, int Length);
    const RilData *BuildLppErrorIndication( const char * pResponse, int Length);
    const RilData *BuildSuplLppDataInfoIndication( const char * pResponse, int Length);
    const RilData *BuildSuplNIMessageIndication( const char * pResponse, int Length);
};

#endif /* __GPS_DATA_BUILDER_H__ */
