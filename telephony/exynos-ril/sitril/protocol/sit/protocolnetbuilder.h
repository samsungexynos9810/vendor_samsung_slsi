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
 * protocolnetbuilder.h
 *
 *  Created on: 2014. 12. 2.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_NET_BUILDER_H__
#define __PROTOCOL_NET_BUILDER_H__

#include "protocolbuilder.h"

class ProtocolNetworkBuilder : public ProtocolBuilder
{
public:
    ProtocolNetworkBuilder() : ProtocolBuilder() {}
    virtual ~ProtocolNetworkBuilder() {}

private:
    int translateNetworktype(int netType);

public:
    virtual ModemData *BuildNetworkRegistrationState(int domain);
    virtual ModemData *BuildOperator();
    virtual ModemData *BuildRadioPower(int powerState);
    virtual ModemData *BuildShutdown();
    virtual ModemData *BuildGetRadioState();
    virtual ModemData *BuildQueryNetworkSelectionMode();
    virtual ModemData *BuildSetNetworkSelectionAuto();
    virtual ModemData *BuildSetNetworkSelectionManual(int rat, const char *plmn);
    virtual ModemData *BuildQueryAvailableNetwork();
    virtual ModemData *BuildQueryAvailableNetwork(int ran);
    virtual ModemData *BuildCancelQueryAvailableNetwork();
    virtual ModemData *BuildSetBandMode(int bandMode);
    virtual ModemData *BuildQueryAvailableBandMode();
    virtual ModemData *BuildSetPreferredNetworkType(int netType);
    virtual ModemData *BuildGetPreferredNetworkType();
    virtual ModemData *BuildGetCellInfoList();
    virtual ModemData *BuildSetCellInfoListReportRate(int rate);
    virtual ModemData *BuildAllowData(int state);
    virtual ModemData *BuildGetPsService();
    virtual ModemData *BuildSetUplmn(int mode, int index, const char *plmn, int act);
    virtual ModemData *BuildGetUplmn();
    virtual ModemData *BuildSetDSNetworkType(int netType);
    virtual ModemData *BuildSetRCNetworkType(int rcVersion, int rcSession, int rcPhase, int rcRaf, char *pUuid, int rcStatus);
    virtual ModemData *BuildGetRCNetworkType();
    virtual ModemData *BuildSetDuplexMode(BYTE mode_4g, BYTE mode_3g);
    virtual ModemData *BuildSetDuplexMode(int mode);
    virtual ModemData *BuildGetDuplexMode();
    virtual ModemData *BuildSetEmergencyCallStatus(int status, int rat);
    virtual ModemData *BuildSetMicroCellSearch(BYTE srch_mode);
    virtual ModemData *BuildRestartModem();
#ifdef SUPPORT_CDMA
    virtual ModemData *BuildSetCdmaSetRoamingType(int cdmaRoamingType);
    virtual ModemData *BuildQueryCdmaRoamingType();
    virtual ModemData *BuildSetCdmaHybridMode(int hybridMode);
    virtual ModemData *BuildGetCdmaHybridMode();
#endif
    virtual ModemData *BuildSetDualNetworkAndAllowData(int typeForPrimary, int typeForSecondary,
                                        int allowedForPrimary, int allowedForSecondary);
    virtual ModemData *BuildStartNetworkScan(int scantype, int timeInterval, int lenSpecifiers,
                                        RIL_RadioAccessSpecifier *pSpecifiers);
    virtual ModemData *BuildStartNetworkScan(
            int scantype, int timeInterval, int specifiersLength, RIL_RadioAccessSpecifier *specifiers,
            int maxSearchTime, bool incrementalResults, int incrementalResultsPeriodicity,
            int numOfMccMncs, char **mccMncs);
    virtual ModemData *BuildStopNetworkScan();
    virtual ModemData *BuildSvNumber(char* svn);
    virtual ModemData *BuildGetManualRatMode();
    virtual ModemData *BuildSetManualRatMode(int mode, int rat);
    virtual ModemData *BuildGetFrequencyLock();
    virtual ModemData *BuildSetFrequencyLock(int mode, int rat, int ltePci, int lteEarfcn, int gsmArfcn, int wcdmaPsc, int wcdmaUarfcn);
    virtual ModemData *BuildSetEndcMode(int mode);
    virtual ModemData *BuildGetEndcMode();
    virtual ModemData *BuildGetFrequencyInfo();

};

#endif /* __PROTOCOL_NET_BUILDER_H__ */
