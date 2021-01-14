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
 * protocolsmsbuilder.h
 *
 *  Created on: 2014. 12. 2.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_SMS_BUILDER_H__
#define __PROTOCOL_SMS_BUILDER_H__

#include "protocolbuilder.h"

class ProtocolSmsBuilder : public ProtocolBuilder
{
public:
    ProtocolSmsBuilder() : ProtocolBuilder() {}
    virtual ~ProtocolSmsBuilder() {}

public:
    virtual ModemData *BuildSendSms(const char *smsc, int smscLen, const char *pdu, int pduSize, bool bExpectMore);
    virtual ModemData *BuildSmsAck(int result, int tpid, int error);
    virtual ModemData *BuildSmsAck(int result, int tpid, const char *pdu, int pduSize);
    virtual ModemData *BuildSmscAddress();
    virtual ModemData *BuildSmscAddress(int sca_len, const char *sca);
    virtual ModemData *BuildSmsMemoryStatus(int status);
    virtual ModemData *BuildWriteSmsToSim(int status, int index, int pduSize, const char * pdu);
    virtual ModemData *BuildDeleteSmsOnSim(int index);
    virtual ModemData *BuildGetBroadcastSmsConfig();
    virtual ModemData *BuildSetBroadcastSmsConfig(const RIL_GSM_BroadcastSmsConfigInfo *rgbsci, int num);
    virtual ModemData *BuildSmsBroadcastActivation(int bcst_act);
    virtual ModemData *BuildGetStoredSmsCount(int sim_id);

#ifdef SUPPORT_CDMA
    virtual ModemData *BuildSendCdmaSms(const char *msg, int msgLen);
    virtual ModemData *BuildSendCdmaSmsAck(int tpid, int errClass, int errCode);
    virtual ModemData *BuildWriteCdmaSmsToRuim(int status, const char *msg, int msgLen);
    virtual ModemData *BuildDeleteCdmaSmsOnRuim(int index);
    virtual ModemData *BuildGetCdmaBroadcastSmsConfig();
    virtual ModemData *BuildSetCdmaBroadcastSmsConfig(const RIL_CDMA_BroadcastSmsConfigInfo *rcbsci, int num);
    virtual ModemData *BuildCdmaSmsBroadcastActivation(int act);
#endif // SUPPORT_CDMA
};
#endif /* __PROTOCOL_SMS_BUILDER_H__ */
