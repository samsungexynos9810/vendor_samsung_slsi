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
 * protocolpsbuilder.h
 *
 *  Created on: 2014. 12. 2.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_PS_BUILDER_H__
#define __PROTOCOL_PS_BUILDER_H__

#include "protocolbuilder.h"

class PdpContext;

class ProtocolPsBuilder : public ProtocolBuilder
{
public:
    ProtocolPsBuilder() : ProtocolBuilder() {}
    virtual ~ProtocolPsBuilder() {}

public:
    virtual ModemData *BuildSetupDataCall(int rat, PdpContext *pPdpContext);
    virtual ModemData *BuildSetInitialAttachApn(PdpContext *pPdpContext, bool isEsmFlagZero = false);
    virtual ModemData *BuildSetDataProfile(const RIL_DataProfileInfo_v15 *dpi, bool isVzw);
    virtual ModemData *BuildSetDataProfile(const RIL_DataProfileInfo_V1_4 *dpi, bool isVzw);
    virtual ModemData *BuildDeactDataCall(int cid, int reason);
    virtual ModemData *BuildGetDataCallList();
    virtual ModemData *BuildSetFastDormancyInfo(BYTE lcdOn, BYTE lcdOff, BYTE rel8LcdOn, BYTE rel8LcdOff);
    virtual ModemData *BuildStartKeepAlive(RIL_KeepaliveRequest reqData);
    virtual ModemData *BuildStopKeepAlive(int sessionHande);
    virtual ModemData *BuildSetPreferredDataModem(int stackId);
private:
    template <typename T>
    bool FillApnInfo(T &req, PdpContext *pPdpContext, bool isEsmFlagZero, bool forInitialAttach);
    template <typename T>
    bool FillDataProfileId(T &req, PdpContext *pPdpContext, bool forInitialAttach);
    bool isRatForCDMA(int);
};

#endif /* __PROTOCOL_PS_BUILDER_H__ */
