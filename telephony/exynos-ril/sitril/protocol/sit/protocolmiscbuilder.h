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
 * protocolmiscbuilder.h
 *
 *  Created on: 2014. 12. 2.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_MISC_BUILDER_H__
#define __PROTOCOL_MISC_BUILDER_H__

#include <fstream>

#include "protocolbuilder.h"

class ProtocolMiscBuilder  : public ProtocolBuilder
{
public:
    ProtocolMiscBuilder() {}
    virtual ~ProtocolMiscBuilder() {}
public:
    virtual ModemData *GetBaseBandVersion(BYTE mask = 0xFF);
    virtual ModemData *GetSignalStrength();
    virtual ModemData *GetTtyMode();
    virtual ModemData *SetTtyMode(INT32 mode);
    virtual ModemData *SetScreenState(INT32 state);
    virtual ModemData *GetIMEI();
    virtual ModemData *GetIMEISV();
    virtual ModemData *GetDevID();
    virtual ModemData *GetEngMode(BYTE mode);
    virtual ModemData *GetEngMode(BYTE mode, BYTE sub_mode);
    virtual ModemData *GetScrLine(BYTE lineno);
    virtual ModemData *SetDebugTrace(BYTE value);
    virtual ModemData *SetEngStringInput(BYTE len, char* input);
    virtual ModemData *GetMslCode();
    virtual ModemData *SetPinControl(BYTE signal, BYTE status);
    virtual ModemData *GetManualBandMode();
    virtual ModemData *SetManualBandMode(void *data, unsigned int datalen);
    virtual ModemData *GetRfDesenseMode();
    virtual ModemData *SetRfDesenseMode(void *data, unsigned int datalen);
    virtual ModemData *StoreAdbSerialNumber(void *data, unsigned int datalen);
    virtual ModemData *ReadAdbSerialNumber();

    virtual ModemData *BuildDtmfStart(bool tone_type, BYTE digit);
    virtual ModemData *BuildDtmf(int dtmf_length, char* dtmf);
    virtual ModemData *BuildDtmfStop();

    virtual ModemData *BuildNvReadItem(int nvItemId);
    virtual ModemData *BuildNvWriteItem(int nvItemId, const char *value);
    virtual ModemData *BuildSetOpenCarierInfo(unsigned int opc, const char *plmn);
    virtual ModemData *GetModemActivityInfo();

#ifdef SUPPORT_CDMA
    virtual ModemData *BuildCdmaSubscription();
#endif // SUPPORT_CDMA
    virtual ModemData *BuildSetVoiceOperation(int mode);
    virtual ModemData *BuildSetPreferredCallCapability(int mode);
    virtual ModemData *BuildGetPreferredCallCapability();
    virtual ModemData *SendSGCValue(const int TargetOp, const int Rvs1, const int Rvs2);
    virtual ModemData *SendDeviceInfo(const char* model, const char* swVer, const char* productName);
    virtual ModemData *BuildGetHwConfig();
    virtual ModemData *BuildLceStart(int lceMode, int interval);
    virtual ModemData *BuildLceStop();
    virtual ModemData *BuildLcePullLceData();
    virtual ModemData *BuildSetUnsolicitedResponseFilter(unsigned int bitMask);
    virtual ModemData *BuildSetCarrierInfoImsiEncryption(char *pMcc, char *pMnc, int keyLen, BYTE *pKey, int keyIdLen, char *pKeyId, LONG expTime);
    virtual ModemData *BuildSetSuppSvcNotification(int enable);
    virtual ModemData *BuildPSensorStatus(int pSensorStatus);
    virtual ModemData *BuildSetSarState(int sarState);
    virtual ModemData *BuildGetSarState();
    virtual ModemData *BuildScanRssi(int rat, int band, int rbw, int scanMode, int startFreq, int endFreq, int step, int antenna, int sampling,
                                    int tx1, int tx1Band, int tx1Bw, int tx1Freq, int tx1Power, int tx1RbNum, int tx1RbOffset, int tx1Mcs,
                                    int tx2, int tx2Band, int tx2Bw, int tx2Freq, int tx2Power, int tx2RbNum, int tx2RbOffset, int tx2Mcs);
    virtual ModemData *BuildATCommand(const char *command);
    virtual ModemData *BuildGetRadioNode(const char *path);
    virtual ModemData *BuildSetRadioNode(const char *path, const char *value);
    virtual ModemData *BuildGetVoLteProvisionUpdate();
    virtual ModemData *BuildSetVoLteProvisionUpdate();
    virtual ModemData *BuildRadioConfigReset(int type);
    virtual ModemData *BuildSetStatckStatus(int mode);
    virtual ModemData *BuildGetStatckStatus();
    virtual ModemData *BuildSetModemsConfig(int config);
    virtual ModemData *BuildModemInfo(int type, char *data, unsigned int size);
    virtual ModemData *BuildSetRtplossThr(BYTE interval, BYTE pktLossThr);
    virtual ModemData *BuildSwitchModemFunction(int feature, BYTE enable);
    virtual ModemData *BuildSetPdcpDiscardTimer(int discardTimer);
    virtual ModemData *BuildSetSelflog(int mode, int size);
    virtual ModemData *BuildGetSelflogStatus();
    virtual ModemData *BuildSetActivateVsim(int slot, int iccidLen, const char *pIccid,
            int imsiLen, const char *pImsi, const char *pHplmn, int vsimState, int vsimCardType);
    virtual ModemData *BuildGetCqiInfo();
    virtual ModemData *BuildSetSarSetting(int dsi);
    virtual ModemData *BuildSetImsTestMode(int mode);
    virtual ModemData *BuildSetGmoSwitch(int feature);
    virtual ModemData *BuildSetTcsFci(int state, int len, char *fci);
    virtual ModemData *BuildGetTcsFci();
    virtual ModemData *BuildSetCABandwidthFilter(int enable);
    virtual ModemData *BuildSetModemLogDump();
    virtual ModemData *BuildSetElevatorSensor(int enable);
    virtual ModemData *BuildSetUicc(int activeStatus);
    virtual ModemData *BuildSetLocationUpdates(int enable);
    virtual ModemData *BuildSetSignalReportCriteria(int ms, int db, int len, int *dbm, int radioAccessNet);
    virtual ModemData *BuildSetLinkCapaReportCriteria(int hMs, int hDlKbps, int hUlKbps, int tDlLen, int *tDlKbps, int tUlLen, int *tUlKbps, int radioAccessNet);
    virtual ModemData *BuildSetSelflogProfile();
    virtual ModemData *BuildSetForbidLteCell(int mode, int cellId, int forbiddenTimer, char *plmn);
};

#endif /* __PROTOCOL_MISC_BUILDER_H__ */
