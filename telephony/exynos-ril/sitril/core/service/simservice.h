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
 * simservice.cpp
 *
 *  Created on: 2014. 7. 1.
 *      Author: MOX
 */

#ifndef __SIM_SERVICE_H__
#define __SIM_SERVICE_H__

#include "service.h"

#define    TIMEOUT_SIM_DEFAULT        30000
#define    TIMEOUT_SIM_ACT_DEACT      30000

// Work Around or Temporary(Test) Implementation
#define _WR_ISIM_AUTH_TYPE_
// MOX: Automatic Verifying PIN
#define _AUTO_VERIFY_PIN_

class Message;
class RilContext;
class ModemData;
class StkService;

//from TS 11.11 9.1 or elsewhere
#define COMMAND_READ_BINARY     0xb0
#define COMMAND_READ_RECORD     0xb2
#define COMMAND_GET_RESPONSE    0xc0
#define COMMAND_UPDATE_BINARY   0xd6
#define COMMAND_UPDATE_RECORD   0xdc
#define COMMAND_STATUS          0xf2
#define COMMAND_RETRIEVE_DATA   0xa2
#define COMMAND_SET_DATA        0xdb


// GSM SIM file ids from TS 51.011
#define EF_AD   0x6FAD

// IccIoResult
class IccIoResult
{
public:
    int mSw1;
    int mSw2;

private:
    int mDataLen;
    char *mData;

public:
    IccIoResult(int sw1, int sw2, char *data, int dataLen) : mDataLen(0), mData(NULL)
    {
        mSw1 = sw1;
        mSw2 = sw2;
        if (dataLen > 0) {
            mDataLen = (dataLen < MAX_SIM_IO_DATA_LEN) ? dataLen : MAX_SIM_IO_DATA_LEN;
            mData = new char[mDataLen];
            memcpy(mData, data, mDataLen);
        }
    }

    ~IccIoResult() {
        if (mData != NULL) {
            delete[] mData;
            mData = NULL;
        }

        mDataLen = 0;
    }

public:
    bool IsSuccess() { return mSw1 == 0x90 || mSw1 == 0x91 || mSw1 == 0x9e || mSw1 == 0x9f; }
    const char *GetData() const { return mData; };
    int GetDataLength() const { return mDataLen; };
};

class SimService : public Service
{
    friend StkService;

protected:
    // Member Variables
    static const INT32 SIM_EFID_FDN = 0x6F3B;

    typedef enum {
        SIM_CARDSTATE_UNKNOWN,
        SIM_CARDSTATE_ABSENT,
        SIM_CARDSTATE_PRESENT,
        SIM_CARDSTATE_ERROR,
    } SIM_CARD_STATE;

    typedef enum {
        SIM_PIN,
        SIM_PUK
    } SIM_PIN_PUK;

public:
    SimService(RilContext* pRilContext);
    virtual ~SimService();

protected:
    virtual int OnCreate(RilContext *pRilContext);

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleRequestTimeout(Message* pMsg);

    virtual void OnReset();
    virtual void OnSimStatusChanged(int cardState, int appState);
    virtual void OnRadioStateChanged(int radioState);

protected:
    virtual int DoGetSimStatus(Message *pMsg);
    virtual int OnGetSimStatusDone(Message *pMsg);
    virtual int OnGetSimStatusTimeout(Message *pMsg);
    char *BuildSimStatus(ModemData *pModemData, int &nLength);
    virtual int OnSimHotSwap(BOOL bRemoval);

    BOOL IsValidPinPuk(int nSimPinPuk, const char *pszPinPuk);
    int SendPasswordInvalid(int nSimPinPuk, int nPinPukIndex);

    virtual int DoVerifyPin(Message *pMsg);
    virtual int OnVerifyPinDone(Message *pMsg);

    virtual int DoVerifyPuk(Message *pMsg);
    virtual int OnVerifyPukDone(Message *pMsg);

    virtual int DoVerifyPin2(Message *pMsg);
    virtual int OnVerifyPin2Done(Message *pMsg);

    virtual int DoVerifyPuk2(Message *pMsg);
    virtual int OnVerifyPuk2Done(Message *pMsg);

    virtual int DoChangePin(Message *pMsg);
    virtual int OnChangePinDone(Message *pMsg);

    virtual int DoChangePin2(Message *pMsg);
    virtual int OnChangePin2Done(Message *pMsg);

    virtual int DoVerifyNetworkLock(Message *pMsg);
    virtual int OnVerifyNetworkLockDone(Message *pMsg);

    virtual int DoSimIo(Message *pMsg);
    virtual int OnSimIoDone(Message *pMsg);

    virtual int DoGetFacilityLock(Message *pMsg);
    virtual int OnGetFacilityLockDone(Message *pMsg);

    virtual int DoSetFacilityLock(Message *pMsg);
    virtual int OnSetFacilityLockDone(Message *pMsg);

    virtual int DoGetIsimAuth(Message *pMsg);
    virtual int OnGetIsimAuthDone(Message *pMsg);

    //virtual int DoGetIsimGbaAuth(Message *pMsg);
    //virtual int OnGetIsimGbaAuthDone(Message *pMsg);
    virtual int DoTransmitSimApduBasic(Message *pMsg);
    virtual int OnTransmitSimApduBasicDone(Message *pMsg);

    virtual int DoOpenSimChannel(Message *pMsg);
    virtual int OnOpenSimChannelDone(Message *pMsg);

    virtual int DoCloseSimChannel(Message *pMsg);
    virtual int OnCloseSimChannelDone(Message *pMsg);

    virtual int DoTransmitSimApduChannel(Message *pMsg);
    virtual int OnTransmitSimApduChannelDone(Message *pMsg);

    virtual int DoGetImsi(Message *pMsg);
    virtual int OnGetImsiDone(Message *pMsg);

    virtual int OnSimStatusChanged(Message *pMsg);

    // Lollipop
    virtual int DoGetSimAuth(Message *pMsg);
    virtual int OnGetSimAuthDone(Message *pMsg);

    virtual int DoSimGetATR(Message *pMsg);
    virtual int OnSimGetATRDone(Message *pMsg);

    virtual int DoSetCarrierRestrictions(Message *pMsg);
    virtual int OnSetCarrierRestrictionsDone(Message *pMsg);
    virtual int DoGetCarrierRestrictions(Message *pMsg);
    virtual int OnGetCarrierRestrictionsDone(Message *pMsg);
    virtual int DoSetSimCardPower(Message *pMsg);
    virtual int OnSetSimCardPowerDone(Message *pMsg);
    virtual int OnUnsolUiccSubscriptionStatusChanged(Message *pMsg);

    // PhoneBook
    virtual int DoReadPbEntry(Message *pMsg);
    virtual int OnReadPbEntryDone(Message *pMsg);

    virtual int DoUpdatePbEntry(Message *pMsg);
    virtual int OnUpdatePbEntryDone(Message *pMsg);

    virtual int DoGetPbStorageInfo(Message *pMsg);
    virtual int OnGetPbStorageInfoDone(Message *pMsg);

    virtual int DoGetPbStorageList(Message *pMsg);
    virtual int OnGetPbStorageListDone(Message *pMsg);

    virtual int DoGetPbEntryInfo(Message *pMsg);
    virtual int OnGetPbEntryInfoDone(Message *pMsg);

    virtual int DoGet3GPbCapa(Message *pMsg);
    virtual int OnGet3GPbCapaDone(Message *pMsg);

    virtual int OnPbReady(Message *pMsg);
    virtual int OnIccidInfo(Message *pMsg);

    // IMS
    virtual int DoOemImsSimIo(Message *pMsg);
    virtual int OnOemImsSimIoDone(Message *pMsg);

    // Secure Element
    virtual int DoOemOpenChannel(Message *pMsg);
    virtual int OnOemOpenChannelDone(Message *pMsg);
    virtual int DoOemTransmitApduLogical(Message *pMsg);
    virtual int OnOemTransmitApduLogicalDone(Message *pMsg);
    virtual int DoOemTransmitApduBasic(Message *pMsg);
    virtual int OnOemTransmitApduBasicDone(Message *pMsg);
    virtual int DoOemGetSimCardPresent(Message *pMsg);
    virtual int OnOemGetCardPresentDone(Message *pMsg);

    virtual int DoGetSimLockInfo(Message *pMsg);
    virtual int OnGetSimLockInfoDone(Message *pMsg);

    // Radio Config
    virtual int DoGetSlotStatus(Message *pMsg);
    virtual int OnGetSlotStatusDone(Message *pMsg);
    virtual int DoSetSlotMapping(Message *pMsg);
    virtual int OnSetSlotMappingDone(Message *pMsg);
    virtual int OnUnsolSimSlotsStatusChanged(Message *pMsg);

    virtual bool IsPossibleToPassInRadioOffState(int request_id);
    virtual bool IsPossibleToPassInRadioUnavailableState(int request_id);

    string FetchSimOperator(IccIoResult &iccioResult);
    void UpdateImsi(const char *imsi);
    void UpdateImsi(const char *aid, const char *imsi);
    void SendOpenCarrierInfoRilReq(const char *plmn);

    RIL_CardStatus_V1_4 *GetRilCardStatus() { return &mRilCardStatus; }
    virtual void NotifyIccidInfo(void);

    virtual int DoOemIccDepersonalization(Message *pMsg);
    virtual int OnOemIccDepersonalizationDone(Message *pMsg);

protected:
    //RIL_AppStatus mRilAppStatus[RIL_CARD_MAX_APPS];
    RIL_CardStatus_V1_4 mRilCardStatus;
    char m_szAtr[MAX_ATR_LEN_GET_SIM*2 + 1];
    char m_szIccid[MAX_ICCID_LEN*2 + 1];
    char m_szEid[MAX_EID_LEN*2 + 1];

    char m_aszAID[RIL_CARD_MAX_APPS][36];        // AID string for mRilCardStatus's pointer

    // PIN2 for FDN
    ModemData *m_pEnableFdnData;
    ModemData *m_pSimIoData;

    // SIM HotSwap Feature
    int m_nSimCardState;

    RilData *m_pIccidInfo;

    // PIN/PUK Verification
    int m_nPinRemain[2];
    int m_nPukRemain[2];

    INT32 m_nRadioState;

    // Radio Config
    char m_aszAtr[MAX_SLOT_NUM][MAX_ATR_LEN_FOR_SLOT_STATUS * 2 + 1];
    char m_aszIccid[MAX_SLOT_NUM][MAX_ICCID_LEN * 2 + 1];
    char m_aszEid[MAX_SLOT_NUM][MAX_EID_LEN * 2 + 1];
    RIL_SimSlotStatusResult_1_2 mSimSlotStatusResultV1_2;

#ifdef _WR_ISIM_AUTH_TYPE_
    int m_nImsAppType;
#endif

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
    typedef enum {
        BOOT_STATE_NORMAL,
        BOOT_STATE_KERNEL_PANIC,
        BOOT_STATE_UNKNOWN,
    } BOOT_STATE;

    typedef enum {
        AUTO_PIN_STATE_UNKNOWN,     // Initializing Value
        AUTO_PIN_STATE_INIT,        // Loading PIN State
        AUTO_PIN_STATE_EMPTY,       // Not Set PIN
        AUTO_PIN_STATE_READY,       // Normal State with PIN
        AUTO_PIN_STATE_RECOVERY,    // Automatic Verify PIN
        AUTO_PIN_STATE_RECOVERED,   // Recovery has been worked.
        AUTO_PIN_STATE_VERIFY,
        AUTO_PIN_STATE_CHANGE,
        AUTO_PIN_STATE_DISABLE,
        AUTO_PIN_STATE_ENABLE,
        AUTO_PIN_STATE_UNLOCK,       // PUK
    } AUTO_VERIFY_PIN_STATE;

    int m_nAutoPinState;
    char m_szTempPIN[128+1];
    char m_szAutoPIN[128+1];
    char m_szAutoPinPropertyKey[128+1];

    string m_strAutoPINForPanic;
    char m_szAutoPinPropertyKeyByPanic[128+1];
    int m_nBootState;

public:
    virtual void SaveAutoVerifyPin();

protected:
    virtual void SetAutoVerifyPin(const char *pszPin);
    virtual void SetAutoVerifyPinDone(BOOL bSuccess);
    virtual void GetAutoVerifyPin(char *pszPin);
    virtual void ClearAutoVerifyPin();

    virtual void LoadAutoVerifyPin();
    virtual void EmptyAutoVerifyPin();

    virtual void EncryptAutoVerifyPin(const char *pszPin);
    virtual void DecryptAutoVerifyPin(char *pszPin);
    virtual int CheckBootReason();
    virtual char* checkMmcMnc(const char* pImsi);
#endif

    string mImsi;
};

#endif /*__SIM_SERVICE_H__*/
