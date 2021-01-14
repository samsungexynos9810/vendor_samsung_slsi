 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _MISC_DATA_H_
#define _MISC_DATA_H_

#include "types.h"
#include "requestdata.h"
#include <map>
#include <string.h>
#include <stdio.h>
using namespace std;

 class ApnSettingsData : public RequestData
{
    protected:
        int m_nApnReqType;
        map<string, string> m_ApnTable;

    public:
        ApnSettingsData(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual ~ApnSettingsData();
        virtual INT32 encode(char *data, unsigned int  datalen);
        void ToString(char *data, unsigned int datalen);
        UINT8 GetReqType();
        void SetReqType(int type);
        String GetApnValues(char *key);
};

class CarrierInfoForImsiEncryptionData : public RequestData
{
public:
    CarrierInfoForImsiEncryptionData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    ~CarrierInfoForImsiEncryptionData();

    char *GetMcc() { return m_pMcc; }
    char *GetMnc() { return m_pMnc; }
    int GetCarrierKeyLen() { return m_nCarrierKeyLen; }
    BYTE *GetCarrierKey() { return m_pCarrierKey; }
    int GetKeyIdLen() { return m_nKeyIdLen; }
    char *GetKeyIdentifier() { return m_pKeyIdentifier; }
    LONG GetEpirationTime() { return m_lEpirationTime;}

    virtual INT32 encode(char *data, unsigned int length);

private:
    char m_pMcc[MAX_MCC_LEN + 1];
    char m_pMnc[MAX_MNC_LEN + 1];
    int m_nCarrierKeyLen;
    BYTE *m_pCarrierKey;
    int m_nKeyIdLen;
    char *m_pKeyIdentifier;
    LONG m_lEpirationTime;
};

class SignalStrengthReportingCriteria : public RequestData
{
public:
    SignalStrengthReportingCriteria(const int nReq, const Token tok, const ReqType type = REQ_FW);
    ~SignalStrengthReportingCriteria();

    int GetHysteresisMs() { return m_hysteresisMs; }
    int GetHysteresisDb() { return m_hysteresisDb; }
    int GetNumOfThresholdsDbm() { return m_numOfThresholdsDbm; }
    int *GetThresholdsDbm() { return m_pThresholdsDbm; }
    int GetAccessNetwork() { return m_accessNetwork; }

    virtual INT32 encode(char *data, unsigned int length);

private:
    int m_hysteresisMs;
    int m_hysteresisDb;
    int m_numOfThresholdsDbm;
    int* m_pThresholdsDbm;
    int m_accessNetwork;
};

class LinkCapacityReportingCriteria : public RequestData
{
public:
    LinkCapacityReportingCriteria(const int nReq, const Token tok, const ReqType type = REQ_FW);
    ~LinkCapacityReportingCriteria();

    int GetHysteresisMs() { return m_hysteresisMs; }
    int GetHysteresisDlKbps() { return m_hysteresisDlKbps; }
    int GetHysteresisUlKpbs() { return m_hysteresisUlKbps; }
    int GetNumOfThresholdsDownlinkKbps() { return m_numOfThresholdsDownlinkKbps; }
    int *GetThresholdsDownlinkKbps() { return m_pThresholdsDownlinkKbps; }
    int GetNumOfThresholdsUplinkKbps() { return m_numOfThresholdsUplinkKbps; }
    int *GetThresholdsUplinkKbps() { return m_pThresholdsUplinkKbps; }
    int GetAccessNetwork() { return m_accessNetwork; }

    virtual INT32 encode(char *data, unsigned int length);

private:
    int m_hysteresisMs;
    int m_hysteresisDlKbps;
    int m_hysteresisUlKbps;
    int m_numOfThresholdsDownlinkKbps;
    int* m_pThresholdsDownlinkKbps;
    int m_numOfThresholdsUplinkKbps;
    int* m_pThresholdsUplinkKbps;
    int m_accessNetwork;
};

/**
 * SetActivateVsimReqData
 */
class SetActivateVsimReqData : public RequestData {
    public:
        RIL_SetActivateVsim m_setActivateVsim;

    public:
        SetActivateVsimReqData(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual ~SetActivateVsimReqData();
        INT32 encode(char *data, unsigned int datalen);

    public:
        int GetSlotId() const;
        const char *GetIccid() const;
        const char *GetImsi() const;
        const char *GetHomePlmn() const;
        int GetVsimState() const;
        int GetVsimCardType() const;
};

/**
 * UICC Subscription
 */

class UiccSubscription : public RequestData
{
public:
    UiccSubscription(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~UiccSubscription();

    UiccSubscription & operator=(const UiccSubscription &UiccSub);
    virtual INT32 encode(char *data, unsigned int length);

    int GetSlot() { return m_nSlot; }
    int GetAppIndex() { return m_nAppIndex; }
    int GetSubscriptionType() { return m_nSubscriptionType; }
    int GetActivationStatus() { return m_nActivationStatus; }

private:
    int m_nSlot;
    int m_nAppIndex;
    int m_nSubscriptionType;
    int m_nActivationStatus;
};

#endif /*_MISC_DATA_H_*/
