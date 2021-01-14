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
 * protocolsimbuilder.h
 *
 *  Created on: 2014. 12. 2.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_SIM_BUILDER_H__
#define __PROTOCOL_SIM_BUILDER_H__

#include "protocolbuilder.h"

class ProtocolSimBuilder : public ProtocolBuilder
{
public:
    ProtocolSimBuilder() : ProtocolBuilder() {}
    virtual ~ProtocolSimBuilder() {}

public:
    virtual ModemData *BuildSimGetStatus();
    virtual ModemData *BuildSimVerifyPin(int nPinIndex, const char *pszPin, const char *pszAID);
    virtual ModemData *BuildSimVerifyPuk(int nPukIndex, const char *pszPuk, const char *pszNewPin, const char *pszAID);
    virtual ModemData *BuildSimChangePin(int nPinIndex, const char *pszOldPin, const char *pszNewPin, const char *pszAID);
    virtual ModemData *BuildSimVerifyNetworkLock(int nFac, const char *pszPassword, int nSvcClass, const char *pszAID);
    virtual ModemData *BuildSimIO(int nCmd, int nAppType, int nFileID, const char *pPath, BYTE p1, BYTE p2, BYTE p3,
                                                int nDataLen, const BYTE *pData, const char *pszPin2, const char *pszAID);
    virtual ModemData *BuildSimGetFacilityLock(char *pszCode, char *pszPassword, int nSvcClass, char *pszAID);
    virtual ModemData *BuildSimSetFacilityLock(char *pszCode, int nLockMode, char *pszPassword, int nSvcClass, char *pszAID);
    virtual ModemData *BuildSimGetIsimAuth(int nAuthType, BYTE *pAuth, int nAuthLengh);
    virtual ModemData *BuildSimGetSimAuth(int nAuthContext, BYTE *pAuth, int nAuthLengh, int nAppType);
    //virtual ModemData *BuildSimGetIsimGbaAuth(int nAuthType, BYTE nGbaType, BYTE nGbaTag, BYTE *pAuth, int nAuthLengh);
    virtual ModemData *BuildSimGetGbaAuth(const char *pGetGbaAuthdata);
    virtual ModemData *BuildSimTransmitApduBasic(int nSessionID, int cla, int instruction,
                                                int p1, int p2, int p3, const char *pszApduData);
    virtual ModemData *BuildSimOpenChannel(const char *pszAID);
    virtual ModemData *BuildSimOpenChannelWithP2(const char *pszAID, int p2);
    virtual ModemData *BuildSimCloseChannel(int nSessionID);
    virtual ModemData *BuildSimTransmitApduChannel(int nSessionID, int cla, int instruction,
                                        int p1, int p2, int p3, const char *pszApduData);
    virtual ModemData *BuildGetImsi(const char *pszAID);
    virtual ModemData *BuildSimGetATR();
    virtual ModemData *BuildSimReadPbEntry(int pbtype, int index);
    virtual ModemData *BuildSimUpdatePbEntry1(int mode, int type, int index, int length, char *pb);
    virtual ModemData *BuildSimUpdatePbEntry2(int mode, int type, int index, int length, char *pb);
    virtual ModemData *BuildSimUpdatePb3gEntry(int mode, int type, int index, int length, char *pb);
    virtual ModemData *BuildSimUpdatePbDelete(int mode, int type, int index);
    virtual ModemData *BuildSimGetPbStorageInfo(int pbType);
    virtual ModemData *BuildSimPbStorageList();
    virtual ModemData *BuildSimGetPbEntryInfo(int pbType);
    virtual ModemData *BuildSim3GPbCapa();
    virtual ModemData *BuildSetCarrierRestrictions(int lenAllow, int lenExclude, CarrierInfo *pAllowed, CarrierInfo *pExcluded);
    virtual ModemData *BuildGetCarrierRestrictions();
    virtual ModemData *BuildSetSimCardPower(int isPowerUp);

    // Secure Element
    virtual ModemData *BuildOemSimRequest(int msgId, BYTE *pData, int nDataLength);

    // SIM lock info
    virtual ModemData *BuildGetSimLockInfo();
    // Radio Config
    virtual ModemData *BuildSimGetSlotStatus();
    virtual ModemData *BuildSimSetLogicalSlotMapping(int *pData, int nDataLength);
};

#endif /* __PROTOCOL_SIM_BUILDER_H__ */
