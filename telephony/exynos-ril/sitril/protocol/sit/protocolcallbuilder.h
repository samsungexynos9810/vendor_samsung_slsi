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
 * protocolcallbuilder.h
 *
 *  Created on: 2014. 12. 2.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_CALL_BUILDER_H__
#define __PROTOCOL_CALL_BUILDER_H__

#include "protocolbuilder.h"
#include "callreqdata.h"

class ProtocolCallBuilder : public ProtocolBuilder
{
public:
    ProtocolCallBuilder() : ProtocolBuilder() {}
    virtual ~ProtocolCallBuilder() {}

public:
    virtual ModemData *BuildGetCallList();
    virtual ModemData *BuildDial(char *number, ClirType clir, UusInfo &uusInfo/*not used*/, CallType cType,
                            UINT eccType = 0x00);
    virtual ModemData *BuildAnswer();
    virtual ModemData *BuildExplicitCallTransfer();
    virtual ModemData *BuildHangup(int callId);
    virtual ModemData *BuildHangupMulti(int callId);
    virtual ModemData *BuildLastCallFailCause();
    virtual ModemData *BuildDtmfStart(bool tone_type, BYTE digit);
    virtual ModemData *BuildDtmf(int dtmf_length, char* dtmf);
    virtual ModemData *BuildDtmfStop();
#ifdef SUPPORT_CDMA
    virtual ModemData *BuildCdmaBurstDtmf(int dmtf_length, const char* dtmf,
            int on_len, const char* on_length, int off_len, const char* off_length);

    virtual ModemData *BuildCdmaSetPreferredVoicePrivacyMode(int vpMode);
    virtual ModemData *BuildCdmaQueryPreferredVoicePrivacyMode();
#endif

    virtual ModemData *BuildHangupWaitingOrBackground();
    virtual ModemData *BuildHangupForegroundResumeBackground();
    virtual ModemData *BuildSwitchWaitingOrHoldingAndActive();
    virtual ModemData *BuildConference();
#ifdef SUPPORT_CDMA
    virtual ModemData *BuildCdmaFlash(int flash_length, const char* flash);
    virtual ModemData *BuildCdmaFlash(const char* flash);
#endif
    virtual ModemData *BuildUdub();
    virtual ModemData *BuildGetClip();
#if 0    //removed by discussion with CP, SET CLIR is implemented by AP (Vendor RIL) inside.
    virtual ModemData *BuildSetClir(int aoc);
#endif
    virtual ModemData *BuildGetClir();
    virtual ModemData *BuildSetCallForwarding(SsModeType status, SsCfReason reason, SsClassX service_class, int toa, char* number, int timeseconds);
    virtual ModemData*BuildGetCallForwardingStatus(SsStatusType status, SsCfReason reason, SsClassX service_class, int toa, char* number, int timeseconds);
    virtual ModemData *BuildSetCallWaiting(SsModeType status, SsClassX service_class);
    virtual ModemData *BuildGetCallWaiting(SsClassX service_class);
    virtual ModemData *BuildChangeBarringPwd(const char* oldpasswd, const char* newpasswd, const char* newpasswd_again);
    virtual ModemData *BuildSendUssd(const char* ussd, bool user_initiated);
    virtual ModemData *BuildCancelUssd();
    virtual ModemData *BuildSeparateConnection(int callId);

    virtual ModemData *BuildQueryColp();
    virtual ModemData *BuildQueryColr();
    virtual ModemData *BuildSendEncodedUssd(BYTE dcs, const char* encodedUssd, bool user_initiated);
    virtual ModemData *BuildSetCallConfirm(int mode);
    virtual ModemData *BuildSendCallConfirm();

    virtual ModemData *BuildExitEmergencyCbMode();
private:
    int ConvertRilDefineToSitDefine(int TableIndex, int SitValue);
};

#endif /* __PROTOCOL_CALL_BUILDER_H__ */
