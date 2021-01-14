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

#include "simservice.h"
#include "rillog.h"
#include "protocolsimbuilder.h"
#include "protocolsimadapter.h"
#include "protocolstkbuilder.h"
#include "protocolstkadapter.h"
#include "protocolnetbuilder.h"
#include "simdata.h"
#include "simdatabuilder.h"
#include "stkdata.h"
#include "stkdatabuilder.h"
#include "util.h"
#include "systemproperty.h"
#include "servicemgr.h"
#include "mcctable.h"
#include "rilparser.h"
#include "rilapplication.h"
#include <sstream>
#include "MccMncChanger.h"
#include "textutils.h"
#include "systemproperty.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define writeRilEvent(format1, format2, ...)   CRilEventLog::writeRilEvent(RIL_LOG_CAT_SIM, format1, format2, ##__VA_ARGS__)

#define PARAM_NULL(msg)        { if(msg==NULL) { RilLogE("%s::%s() Parameter = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define NULL_REQ(msg)        { if(msg==NULL || msg->GetRequestData()==NULL) { RilLogE("%s::%s() RequestData = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define NULL_RSP(msg)        { if(msg==NULL || msg->GetModemData()==NULL) { RilLogE("%s::%s() ModemData = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define ENTER_FUNC()        { RilLogI("%s::%s() [<-- ", m_szSvcName, __FUNCTION__); }
#define LEAVE_FUNC()        { RilLogI("%s::%s() [--> ", m_szSvcName, __FUNCTION__); }
#define NOT_IMPLEMENT()        { RilLogE("%s::%s() Not Implemented", m_szSvcName, __FUNCTION__); }

#define LOGI(format, ...)        RilLogI("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)
#define LOGV(format, ...)        RilLogV("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)
#undef LOGD
#define LOGD(format, ...)        RilLog("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)
#define LOGE(format, ...)        RilLogE("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
#define PROP_AUTO_VERIFY_PIN_0    "vendor.ril.context.rebirth"
#define PROP_AUTO_VERIFY_PIN_1    "vendor.ril.context.again"
#define PROP_AUTO_PIN_0_BY_PANIC  "persist.vendor.ril.context.rebirth"
#define PROP_AUTO_PIN_1_BY_PANIC  "persist.vendor.ril.context.again"
#endif

#define ENABLE_NEW_RIL_COMMAND (0)  /* TODO: enable when SIT is implemented */

SimService::SimService(RilContext* pRilContext)
    : Service(pRilContext, RIL_SERVICE_SIM)
{
    memset(&mRilCardStatus, 0, sizeof(mRilCardStatus));
    mRilCardStatus.atr = m_szAtr;
    mRilCardStatus.iccid = m_szIccid;
    mRilCardStatus.eid = m_szEid;
    memset(mRilCardStatus.atr, 0,  sizeof(m_szAtr));
    memset(mRilCardStatus.iccid, 0,  sizeof(m_szIccid));
    memset(mRilCardStatus.eid, 0, sizeof(m_szEid));


    memset(m_aszAID, 0, sizeof(char)*RIL_CARD_MAX_APPS*36);

    // PIN2 for FDN
    m_pEnableFdnData = NULL;
    m_pSimIoData = NULL;

    // SIM HotSwap Feature
    m_nSimCardState= SIM_CARDSTATE_UNKNOWN;

    m_pIccidInfo = NULL;

    // PIN/PUK Verification
    m_nPinRemain[0] = -1;
    m_nPinRemain[1] = -1;
    m_nPukRemain[0] = -1;
    m_nPukRemain[1] = -1;

    m_nRadioState = RADIO_STATE_UNAVAILABLE;
#ifdef _WR_ISIM_AUTH_TYPE_
    m_nImsAppType = -1;
#endif

    // Radio Config
    memset(&mSimSlotStatusResultV1_2, 0, sizeof(mSimSlotStatusResultV1_2));

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
    m_nAutoPinState = AUTO_PIN_STATE_UNKNOWN;
    m_nBootState = BOOT_STATE_UNKNOWN;
#endif
    memset(m_szTempPIN, 0, sizeof(m_szTempPIN));
    memset(m_szAutoPIN, 0, sizeof(m_szAutoPIN));
    memset(m_szAutoPinPropertyKey, 0, sizeof(m_szAutoPinPropertyKey));
}

SimService::~SimService()
{
    // PIN2 for FDN
    if(m_pEnableFdnData!=NULL)
    {
        delete m_pEnableFdnData;
        m_pEnableFdnData = NULL;
    }

    if(m_pSimIoData!=NULL)
    {
        delete m_pSimIoData;
        m_pSimIoData = NULL;
    }

    if(m_pIccidInfo!=NULL)
    {
        delete m_pIccidInfo;
        m_pIccidInfo = NULL;
    }

    // PIN/PUK Verification
    m_nPinRemain[0] = -1;
    m_nPinRemain[1] = -1;
    m_nPukRemain[0] = -1;
    m_nPukRemain[1] = -1;

    // Radio Config
    memset(&mSimSlotStatusResultV1_2, 0, sizeof(mSimSlotStatusResultV1_2));
    memset(&m_aszAtr, 0, sizeof(char)*(MAX_SLOT_NUM)*(MAX_ATR_LEN_FOR_SLOT_STATUS * 2 + 1));
    memset(&m_aszIccid, 0, sizeof(char)*(MAX_SLOT_NUM)*(MAX_ICCID_LEN * 2 + 1));
    memset(&m_aszEid, 0, sizeof(char)*(MAX_SLOT_NUM)*(MAX_EID_LEN * 2 + 1));
}

int SimService::OnCreate(RilContext *pRilContext)
{
    ENTER_FUNC();

    RilProperty *pProperty = GetRilContextProperty();
    pProperty->Put(RIL_CONTEXT_UICC_STATUS, 1);

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
    if(GetRilSocketId()==0) {
        strcpy(m_szAutoPinPropertyKey, PROP_AUTO_VERIFY_PIN_0);
        strcpy(m_szAutoPinPropertyKeyByPanic, PROP_AUTO_PIN_0_BY_PANIC);
    } else if(GetRilSocketId()==1) {
        strcpy(m_szAutoPinPropertyKey, PROP_AUTO_VERIFY_PIN_1);
        strcpy(m_szAutoPinPropertyKeyByPanic, PROP_AUTO_PIN_1_BY_PANIC);
    }
    else LOGV("Fail to load GetRilSocketId(%d)", GetRilSocketId());

    ClearAutoVerifyPin();
    LoadAutoVerifyPin();
    EmptyAutoVerifyPin();
#endif

    LEAVE_FUNC();
    return 0;
}

BOOL SimService::OnHandleRequest(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_SIM_GET_STATUS:
        ret = DoGetSimStatus(pMsg);
        break;
    case MSG_SIM_VERIFY_PIN:
        ret = DoVerifyPin(pMsg);
        break;
    case MSG_SIM_VERIFY_PUK:
        ret = DoVerifyPuk(pMsg);
        break;
    case MSG_SIM_VERIFY_PIN2:
        ret = DoVerifyPin2(pMsg);
        break;
    case MSG_SIM_VERIFY_PUK2:
        ret = DoVerifyPuk2(pMsg);
        break;
    case MSG_SIM_CHANGE_PIN:
        ret = DoChangePin(pMsg);
        break;
    case MSG_SIM_CHANGE_PIN2:
        ret = DoChangePin2(pMsg);
        break;
    case MSG_SIM_VERIFY_NETWORK_LOCK:
        ret = DoVerifyNetworkLock(pMsg);
        break;
    case MSG_SIM_IO:
        ret = DoSimIo(pMsg);
        break;
    case MSG_SIM_GET_FACILITY_LOCK:
        ret = DoGetFacilityLock(pMsg);
        break;
    case MSG_SIM_SET_FACILITY_LOCK:
        ret = DoSetFacilityLock(pMsg);
        break;
    case MSG_SIM_GET_ISIM_AUTH:
        ret = DoGetIsimAuth(pMsg);
        break;
    case MSG_SIM_GET_ISIM_GBA_AUTH:
        //ret = DoGetIsimGbaAuth(pMsg);
        break;
    case MSG_SIM_TRANSMIT_APDU_BASIC:
        ret = DoTransmitSimApduBasic(pMsg);
        break;
    case MSG_SIM_OPEN_CHANNEL:
        ret = DoOpenSimChannel(pMsg);
        break;
    case MSG_SIM_CLOSE_CHANNEL:
        ret = DoCloseSimChannel(pMsg);
        break;
    case MSG_SIM_TRANSMIT_APDU_CHANNEL:
        ret = DoTransmitSimApduChannel(pMsg);
        break;
    case MSG_SIM_GET_IMSI:
        ret = DoGetImsi(pMsg);
        break;

    // Lollipop
    case MSG_SIM_GET_SIM_AUTH:
        ret = DoGetSimAuth(pMsg);
        break;
    case MSG_SIM_GET_ATR:
        ret = DoSimGetATR(pMsg);
        break;

    case MSG_SIM_SET_CARRIER_RESTRICTIONS:
        ret = DoSetCarrierRestrictions(pMsg);
        break;
    case MSG_SIM_GET_CARRIER_RESTRICTIONS:
        ret = DoGetCarrierRestrictions(pMsg);
        break;
    case MSG_SIM_SET_SIM_CARD_POWER:
        ret = DoSetSimCardPower(pMsg);
        break;

    // PhoneBook
    case MSG_SIM_READ_PB_ENTRY:
        ret = DoReadPbEntry(pMsg);
        break;
    case MSG_SIM_UPDATE_PB_ENTRY:
        ret = DoUpdatePbEntry(pMsg);
        break;
    case MSG_SIM_GET_PB_STORAGE_INFO:
        ret = DoGetPbStorageInfo(pMsg);
        break;
    case MSG_SIM_GET_PB_STORAGE_LIST:
        ret = DoGetPbStorageList(pMsg);
        break;
    case MSG_SIM_GET_PB_ENTRY_INFO:
        ret = DoGetPbEntryInfo(pMsg);
        break;
    case MSG_SIM_GET_3G_PB_CAPA:
        ret = DoGet3GPbCapa(pMsg);
        break;
    case MSG_SIM_OEM_IMS_SIM_IO:
        ret = DoOemImsSimIo(pMsg);
        break;
    // Secure Element
    case MSG_SIM_OEM_OPEN_CHANNEL:
        ret = DoOemOpenChannel(pMsg);
        break;
    case MSG_SIM_OEM_TRANSMIT_APDU_LOGICAL:
        ret = DoOemTransmitApduLogical(pMsg);
        break;
    case MSG_SIM_OEM_TRNASMIT_APDU_BASIC:
        ret = DoOemTransmitApduBasic(pMsg);
        break;
    case MSG_SIM_OEM_GET_CARD_PRESENT:
        ret = DoOemGetSimCardPresent(pMsg);
        break;
    case MSG_SIM_GET_SIM_LOCK_INFO:
        ret = DoGetSimLockInfo(pMsg);
        break;
    case MSG_SIM_GET_SLOT_STATUS:
        ret = DoGetSlotStatus(pMsg);
        break;
    case MSG_SIM_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING:
        ret = DoSetSlotMapping(pMsg);
        break;
    case MSG_SIM_OEM_ICC_DEPERSONALIZATION:
        ret = DoOemIccDepersonalization(pMsg);
        break;
    default:
        // TODO log unsupported message id
        return FALSE;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL SimService::OnHandleSolicitedResponse(Message* pMsg)
{
    PARAM_NULL(pMsg);

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_SIM_GET_STATUS_DONE:
        ret = OnGetSimStatusDone(pMsg);
        break;
    case MSG_SIM_VERIFY_PIN_DONE:
        ret = OnVerifyPinDone(pMsg);
        break;
    case MSG_SIM_VERIFY_PUK_DONE:
        ret = OnVerifyPukDone(pMsg);
        break;
    case MSG_SIM_VERIFY_PIN2_DONE:
        ret = OnVerifyPin2Done(pMsg);
        break;
    case MSG_SIM_VERIFY_PUK2_DONE:
        ret = OnVerifyPuk2Done(pMsg);
        break;
    case MSG_SIM_CHANGE_PIN_DONE:
        ret = OnChangePinDone(pMsg);
        break;
    case MSG_SIM_CHANGE_PIN2_DONE:
        ret = OnChangePin2Done(pMsg);
        break;
    case MSG_SIM_VERIFY_NETWORK_LOCK_DONE:
        ret = OnVerifyNetworkLockDone(pMsg);
        break;
    case MSG_SIM_IO_DONE:
        ret = OnSimIoDone(pMsg);
        break;
    case MSG_SIM_GET_FACILITY_LOCK_DONE:
        ret = OnGetFacilityLockDone(pMsg);
        break;
    case MSG_SIM_SET_FACILITY_LOCK_DONE:
        ret = OnSetFacilityLockDone(pMsg);
        break;
    case MSG_SIM_GET_ISIM_AUTH_DONE:
        ret = OnGetIsimAuthDone(pMsg);
        break;
    case MSG_SIM_GET_ISIM_GBA_AUTH_DONE:
        //ret = OnGetIsimGbaAuthDone(pMsg);
        break;
    case MSG_SIM_TRANSMIT_APDU_BASIC_DONE:
        ret = OnTransmitSimApduBasicDone(pMsg);
        break;
    case MSG_SIM_OPEN_CHANNEL_DONE:
        ret = OnOpenSimChannelDone(pMsg);
        break;
    case MSG_SIM_CLOSE_CHANNEL_DONE:
        ret = OnCloseSimChannelDone(pMsg);
        break;
    case MSG_SIM_TRANSMIT_APDU_CHANNEL_DONE:
        ret = OnTransmitSimApduChannelDone(pMsg);
        break;
    case MSG_SIM_GET_IMSI_DONE:
        ret = OnGetImsiDone(pMsg);
        break;
    // Lollipop
    case MSG_SIM_GET_SIM_AUTH_DONE:
        ret = OnGetSimAuthDone(pMsg);
        break;
    case MSG_SIM_GET_ATR_DONE:
        ret = OnSimGetATRDone(pMsg);
        break;

    case MSG_SIM_SET_CARRIER_RESTRICTIONS_DONE:
        ret = OnSetCarrierRestrictionsDone(pMsg);
        break;
    case MSG_SIM_GET_CARRIER_RESTRICTIONS_DONE:
        ret = OnGetCarrierRestrictionsDone(pMsg);
        break;
    case MSG_SIM_SET_SIM_CARD_POWER_DONE:
        ret = OnSetSimCardPowerDone(pMsg);
        break;

    // PhoneBook
    case MSG_SIM_READ_PB_ENTRY_DONE:
        ret = OnReadPbEntryDone(pMsg);
        break;
    case MSG_SIM_UPDATE_PB_ENTRY_DONE:
        ret = OnUpdatePbEntryDone(pMsg);
        break;
    case MSG_SIM_GET_PB_STORAGE_INFO_DONE:
        ret = OnGetPbStorageInfoDone(pMsg);
        break;
    case MSG_SIM_GET_PB_STORAGE_LIST_DONE:
        ret = OnGetPbStorageListDone(pMsg);
        break;
    case MSG_SIM_GET_PB_ENTRY_INFO_DONE:
        ret = OnGetPbEntryInfoDone(pMsg);
        break;
    case MSG_SIM_GET_3G_PB_CAPA_DONE:
        ret = OnGet3GPbCapaDone(pMsg);
        break;
    case MSG_SIM_OEM_IMS_SIM_IO_DONE:
        ret = OnOemImsSimIoDone(pMsg);
        break;
    // Secure Element
    case MSG_SIM_OEM_OPEN_CHANNEL_DONE:
        ret = OnOemOpenChannelDone(pMsg);
        break;
    case MSG_SIM_OEM_TRANSMIT_APDU_LOGICAL_DONE:
        ret = OnOemTransmitApduLogicalDone(pMsg);
        break;
    case MSG_SIM_OEM_TRNASMIT_APDU_BASIC_DONE:
        ret = OnOemTransmitApduBasicDone(pMsg);
        break;
    case MSG_SIM_OEM_GET_CARD_PRESENT_DONE:
        ret = OnOemGetCardPresentDone(pMsg);
        break;
    case MSG_SIM_GET_SIM_LOCK_INFO_DONE:
        ret = OnGetSimLockInfoDone(pMsg);
        break;
    case MSG_SIM_GET_SLOT_STATUS_DONE:
        ret = OnGetSlotStatusDone(pMsg);
        break;
    case MSG_SIM_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING_DONE:
        ret = OnSetSlotMappingDone(pMsg);
        break;
    case MSG_SIM_OEM_ICC_DEPERSONALIZATION_DONE:
        ret = OnOemIccDepersonalizationDone(pMsg);
        break;
    default:
        // TODO log unsupported message id
        return FALSE;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL SimService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    PARAM_NULL(pMsg);

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_SIM_STATUS_CHANGED:
        ret = OnSimStatusChanged(pMsg);
        break;
    // PhoneBook
    case MSG_SIM_PB_READY:
        ret = OnPbReady(pMsg);
        break;
    case MSG_SIM_ICCID_INFO:
        ret = OnIccidInfo(pMsg);
        break;
    case MSG_SIM_IND_UICC_SUBSCRIPTION_STATUS_CHANGED:
        ret = OnUnsolUiccSubscriptionStatusChanged(pMsg);
        break;
    case MSG_SIM_SLOT_STATUS_CHANGED:
        ret = OnUnsolSimSlotsStatusChanged(pMsg);
        break;

    default:
        // TODO log unsupported message id
        return FALSE;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL SimService::OnHandleRequestTimeout(Message* pMsg)
{
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_SIM_GET_STATUS:
        ret = OnGetSimStatusTimeout(pMsg);
        break;
    }
    return (ret < 0 ? FALSE : TRUE);
}

void SimService::OnReset()
{
#ifdef _AUTO_VERIFY_PIN_
    SaveAutoVerifyPin();
#endif // _AUTO_VERIFY_PIN_
}

void SimService::OnSimStatusChanged(int cardState, int appState)
{
    if (cardState != RIL_CARDSTATE_PRESENT) {
        // IMSI is not valid anymore
        UpdateImsi("");
    }
}

void SimService::OnRadioStateChanged(int radioState) {
    m_nRadioState = radioState;
    // Send cached ICCID to framework if condition is satisfied.
    NotifyIccidInfo();
}

int SimService::DoGetSimStatus(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimGetStatus();
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_STATUS_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

char *SimService::BuildSimStatus(ModemData *pModemData, int &nLength)
{
    ENTER_FUNC();
    if(pModemData==NULL) return NULL;

    ProtocolSimStatusAdapter adapter(pModemData);
    RIL_CardState eCardState = (RIL_CardState) adapter.GetCardState();
    int nSimCardState = (eCardState==RIL_CARDSTATE_PRESENT)? SIM_CARDSTATE_PRESENT: ((eCardState==RIL_CARDSTATE_ABSENT)? SIM_CARDSTATE_ABSENT: SIM_CARDSTATE_ERROR);

    if(m_nSimCardState==SIM_CARDSTATE_UNKNOWN)
        m_nSimCardState = nSimCardState;
    else
    {
        //if(m_nSimCardState!=eCardState)
        if(m_nSimCardState!=nSimCardState)
        {
            // SIM Insertion
            if(m_nSimCardState==SIM_CARDSTATE_ABSENT && nSimCardState==SIM_CARDSTATE_PRESENT) OnSimHotSwap(FALSE);
            // SIM Removal
            else if(m_nSimCardState==SIM_CARDSTATE_PRESENT && nSimCardState==SIM_CARDSTATE_ABSENT) OnSimHotSwap(TRUE);

            m_nSimCardState = nSimCardState;
        }
    }

    // Send cached ICCID to framework if condition is satisfied.
    NotifyIccidInfo();

    memset(&mRilCardStatus, 0, sizeof(mRilCardStatus));
    mRilCardStatus.card_state = (RIL_CardState) adapter.GetCardState();
    mRilCardStatus.universal_pin_state = (RIL_PinState) adapter.GetUniversalPinState();
    mRilCardStatus.gsm_umts_subscription_app_index = -1;
    mRilCardStatus.cdma_subscription_app_index = -1;
    mRilCardStatus.ims_subscription_app_index = -1;
    mRilCardStatus.num_applications = adapter.GetApplicationCount();
    LOGV("CardState:%d, NumApps:%d", mRilCardStatus.card_state, mRilCardStatus.num_applications);
    mRilCardStatus.atr = m_szAtr;
    mRilCardStatus.iccid = m_szIccid;
    mRilCardStatus.eid = m_szEid;
    memset(mRilCardStatus.atr, 0,  sizeof(m_szAtr));
    memset(mRilCardStatus.iccid, 0,  sizeof(m_szIccid));
    memset(mRilCardStatus.eid, 0, sizeof(m_szEid));


    int index = 0;
    for(int i=0; i<mRilCardStatus.num_applications; i++)
    {
        if ((RIL_AppType)adapter.GetAppsType(i) == RIL_APPTYPE_UNKNOWN) {
            char *pszAid = adapter.GetAID(i);
            LOGV("Skip AID: %s, AppType: %d", pszAid, adapter.GetAppsType(i));
            if(pszAid!=NULL) delete [] pszAid;
            continue;
        }

        /* test code for RUIM/CSIM only
        if ((RIL_AppType) adapter.GetAppsType(i) == RIL_APPTYPE_SIM || (RIL_AppType) adapter.GetAppsType(i) == RIL_APPTYPE_USIM) {
            LOGV("test code : skip apptype : %d", adapter.GetAppsType(i));
            continue;
        }
        */

        mRilCardStatus.applications[index].app_type = (RIL_AppType) adapter.GetAppsType(i);
        mRilCardStatus.applications[index].perso_substate = (RIL_PersoSubstate) adapter.GetPersonalSubstate(i);

        switch(mRilCardStatus.applications[index].app_type)
        {
        case RIL_APPTYPE_SIM:
        case RIL_APPTYPE_USIM:
            mRilCardStatus.gsm_umts_subscription_app_index = index;
            break;
        case RIL_APPTYPE_RUIM:
        case RIL_APPTYPE_CSIM:
            mRilCardStatus.cdma_subscription_app_index = index;
            break;
        case RIL_APPTYPE_ISIM:
            mRilCardStatus.ims_subscription_app_index = index;
            break;
        default:
            break;
        }

        if(m_nSimCardState==SIM_CARDSTATE_PRESENT)
        {
            mRilCardStatus.applications[index].app_state = (RIL_AppState) adapter.GetAppsState(i);
        }
        else
        {
            mRilCardStatus.applications[index].app_state = RIL_APPSTATE_UNKNOWN;
        }
/*
        // for sim unlock testing
        mRilCardStatus.applications[index].app_state = RIL_APPSTATE_SUBSCRIPTION_PERSO;
        mRilCardStatus.applications[index].perso_substate = RIL_PERSOSUBSTATE_SIM_NETWORK;
*/
        char *pszAID = NULL;
        char *pszAdapterAID = adapter.GetAID(i);
        if(pszAdapterAID!=NULL)
        {
            memset(m_aszAID[index], 0, 36);
            strncpy(m_aszAID[index], pszAdapterAID, strlen(pszAdapterAID));
            delete [] pszAdapterAID;
            pszAdapterAID = NULL;
            pszAID = m_aszAID[index];
        }
        mRilCardStatus.applications[index].aid_ptr = pszAID;
        mRilCardStatus.applications[index].app_label_ptr = NULL;        // Ignore
        mRilCardStatus.applications[index].pin1_replaced = adapter.GetPin1Replaced(i);
        mRilCardStatus.applications[index].pin1 = (RIL_PinState) adapter.GetPinState(i, 1);
        mRilCardStatus.applications[index].pin2 = (RIL_PinState) adapter.GetPinState(i, 2);

        RilLogV("%s::%s() %d. AppType(%d), AppState(%d), PersoState(%d), PinState(%d, %d), AID(%s)",
                                                                        m_szSvcName, __FUNCTION__, index,
                                                                        mRilCardStatus.applications[index].app_type,
                                                                        mRilCardStatus.applications[index].app_state,
                                                                        mRilCardStatus.applications[index].perso_substate,
                                                                        mRilCardStatus.applications[index].pin1,
                                                                        mRilCardStatus.applications[index].pin2,
                                                                        mRilCardStatus.applications[index].aid_ptr);

        RilLogV("%s::%s()  - PinRemain(%d, %d), PukRemain(%d, %d)", m_szSvcName, __FUNCTION__,
                                                                        adapter.GetPinRemainCount(i, 1),
                                                                        adapter.GetPinRemainCount(i, 2),
                                                                        adapter.GetPukRemainCount(i, 1),
                                                                        adapter.GetPukRemainCount(i, 2));
        m_nPinRemain[0] = adapter.GetPinRemainCount(i, 1);
        m_nPinRemain[1] = adapter.GetPinRemainCount(i, 2);
        m_nPukRemain[0] = adapter.GetPukRemainCount(i, 1);
        m_nPukRemain[1] = adapter.GetPukRemainCount(i, 2);

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        if(mRilCardStatus.applications[index].pin1==RIL_PINSTATE_ENABLED_NOT_VERIFIED && m_nAutoPinState==AUTO_PIN_STATE_READY)
        {
            m_nAutoPinState = AUTO_PIN_STATE_RECOVERY;
            RilLogI("%s::%s() Automatic Verify PIN - Recovery Mode ", m_szSvcName, __FUNCTION__);
        }
#endif
        index++;
    }

    if (mRilCardStatus.num_applications != index) {
        RilLogI("%s::%s() Changed num_applications from %d to %d", m_szSvcName, __FUNCTION__
            , mRilCardStatus.num_applications, index);
        mRilCardStatus.num_applications = index;
    }

    LOGV("AppIdx(GSM/UMTS:%d, CDMA:%d, IMS:%d)", mRilCardStatus.gsm_umts_subscription_app_index,
                                                 mRilCardStatus.cdma_subscription_app_index,
                                                 mRilCardStatus.ims_subscription_app_index);

#ifdef _WR_ISIM_AUTH_TYPE_
    int nAuthType = 0x00/*ISIM_AUTH_IMS*/;

    BOOL bExistIMS=FALSE, bExist3G=FALSE, bExistGSM=FALSE;
    for(int i=0; i<mRilCardStatus.num_applications; i++)
    {
        switch(mRilCardStatus.applications[i].app_type)
        {
        case RIL_APPTYPE_SIM: bExistGSM = TRUE; break;
        case RIL_APPTYPE_USIM: bExist3G = TRUE; break;
        case RIL_APPTYPE_ISIM: bExistIMS = TRUE; break;
        default: break;
        }
    }

    RilLogV("%s::%s() IMS(%c), 3G(%c), GSM(%c)", m_szSvcName, __FUNCTION__, bExistIMS? 'O':'X', bExist3G? 'O':'X', bExistGSM? 'O':'X');

    if(bExistIMS==TRUE) nAuthType = 0x00/*ISIM_AUTH_IMS*/;
    else if(bExist3G==TRUE) nAuthType = 0x02/*ISIM_AUTH_3G*/;
    else if(bExistGSM==TRUE) nAuthType = 0x01/*ISIM_AUTH_GSM*/;

    if(m_nImsAppType!=nAuthType)
    {
        RilProperty *pProperty = GetRilContextProperty();
        pProperty->Put(RIL_CONTEXT_IMS_APP_TYPE, nAuthType);

        m_nImsAppType = nAuthType;
    }
#endif

    nLength = sizeof(RIL_CardStatus_V1_4);

    mRilCardStatus.physicalSlotId  = adapter.GetPhysicalSlotId();
    // need to covert slotid to -1 value if value is max slot id
    // -1 means default physicalslotid value.
    if (mRilCardStatus.physicalSlotId ==  INVALID_SLOT_ID)
        mRilCardStatus.physicalSlotId = -1;

    int nHexLen = 0;

    if (mRilCardStatus.num_applications > 0)
    {
        if(adapter.GetAtrLength() > 0)
        {
            nHexLen = Value2HexString(mRilCardStatus.atr, (BYTE *) adapter.GetAtr(), adapter.GetAtrLength());
        }

        if(adapter.GetIccidLength() > 0)
        {
            int iccIdLength = adapter.GetIccidLength();
            const BYTE *iccId =(BYTE *) adapter.GetIccid();
            string orderedIccId = bchToString(iccId, iccIdLength);
            if (orderedIccId.length())
                strncpy(mRilCardStatus.iccid, orderedIccId.c_str(), orderedIccId.length());
        }

        if(adapter.GetEidLength() > 0)
        {
            nHexLen = Value2HexString(mRilCardStatus.eid, (BYTE *) adapter.GetEid(), adapter.GetEidLength());
        }
    }

    RilLogV("%s::%s() eSIM No Profile(%s), Physical Slot ID(%d), ATR(%s), ICCID(%s), EID(%s), mRilCardStatus.physicalSlotId(%d)", m_szSvcName, __FUNCTION__,
                        (adapter.GetEsimNoProfile()?"TRUE":"FALSE"), mRilCardStatus.physicalSlotId, mRilCardStatus.atr, mRilCardStatus.iccid ,mRilCardStatus.eid, mRilCardStatus.physicalSlotId);


    LEAVE_FUNC();
    return (char *) &mRilCardStatus;
}

int SimService::OnSimHotSwap(BOOL bRemoval)
{
    ENTER_FUNC();

    if(bRemoval)
    {
        LOGV("SIM Removal !!!");
        writeRilEvent(m_szSvcName, __FUNCTION__, "SIM Removal !!!");

        // PIN/PUK Verification
        m_nPinRemain[0] = -1;
        m_nPinRemain[1] = -1;
        m_nPukRemain[0] = -1;
        m_nPukRemain[1] = -1;

        // clear open carrier information
        SetOpenCarrierIndex("");

        // Initialize variables of the application property for PS service state.
        RilProperty *pProperty = GetRilContextProperty();
        pProperty = GetRilApplicationProperty();
        stringstream ss;
        ss << RIL_APP_NET_PS_SERVICE_ALLOW << GetRilSocketId();;
        string strPropName = ss.str();
        pProperty->Put(strPropName, false);

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        ClearAutoVerifyPin();
        EmptyAutoVerifyPin();
        // for kernel panic
        SystemProperty::Set(m_szAutoPinPropertyKeyByPanic, "");
#endif

#ifdef _SIM_HOTSWAP_SILENT_RESET_
        RilReset("SIM_HOTSWAP - Removal");
#endif
    }
    else
    {
        LOGV("SIM Insertion !!!");
        writeRilEvent(m_szSvcName, __FUNCTION__, "SIM Insertion !!!");
#ifdef _SIM_HOTSWAP_SILENT_RESET_
        RilReset("SIM_HOTSWAP - Insertion");
#endif
    }

    LEAVE_FUNC();
    return 0;
}

int SimService::OnGetSimStatusDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();

    // Currently, pResponse is built with highest verion(RIL_CardStatus_V1_4 = RIL_CardStatus_v6)
    // If hal version is low, nLength should be adjusted.
    int nFinalLength = 0;
    if (RilApplication::RIL_HalVersionCode == HAL_VERSION_CODE(1, 4)) {
        nFinalLength = sizeof(RIL_CardStatus_V1_4);
    } else if (RilApplication::RIL_HalVersionCode == HAL_VERSION_CODE(1, 2)) {
        nFinalLength = sizeof(RIL_CardStatus_V1_2);
    } else {
        // HAL_VERSION_CODE(1, 1) or HAL_VERSION_CODE(1, 0)
        // Remove   uint32_t physicalSlotId and  char *atr and  char *iccid;
        nFinalLength = sizeof(RIL_CardStatus_V1_2) - (sizeof(uint32_t) + sizeof(char *) * 2);
    }

    if(uErrCode==RIL_E_SUCCESS)
    {
        int nLength = 0;
        char *pResponse = BuildSimStatus(pMsg->GetModemData(), nLength);

        //LOGV("data ptr:0x%08X, length:%d", pResponse, nLength);
// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        if(m_nAutoPinState==AUTO_PIN_STATE_RECOVERY) {
            char szPin[MAX_SIM_PIN_LEN+1] = { 0, };
            GetAutoVerifyPin(szPin);

            ProtocolSimBuilder builder;
            ModemData *pModemData = builder.BuildSimVerifyPin(1, szPin, m_aszAID[0]);
            int nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_VERIFY_PIN_DONE);
            if (nResult < 0) {
                m_nAutoPinState = AUTO_PIN_STATE_READY;
                LOGE("Sending Error: Automatic Verify PIN");
            }
        } else {
            if (BOOT_STATE_UNKNOWN == m_nBootState && RIL_CARDSTATE_PRESENT == mRilCardStatus.card_state) {
                m_nBootState = CheckBootReason();
                /*
                LOGV("########## bootReason=%s", (m_nBootState==0)?"BOOT_STATE_NORMAL"
                                    :((m_nBootState==1)?"BOOT_STATE_KERNEL_PANIC":"BOOT_STATE_UNKNOWN"));
                                    */
                if (BOOT_STATE_KERNEL_PANIC == m_nBootState) {
                    m_strAutoPINForPanic = SystemProperty::Get(m_szAutoPinPropertyKeyByPanic, "");
                    if (!TextUtils::IsEmpty(m_strAutoPINForPanic)) {
                        memset(m_szAutoPIN, 0, sizeof(m_szAutoPIN));
                        strncpy(m_szAutoPIN, m_strAutoPINForPanic.c_str(), 128);
                        m_nAutoPinState = AUTO_PIN_STATE_READY;
                        GetCurrentMsg()->SetTimeout(50);
                        LEAVE_FUNC();
                        return 0;
                    }
                } else if (BOOT_STATE_UNKNOWN == m_nBootState) {
                    //LOGV("Set Timeout 3000");
                    GetCurrentMsg()->SetTimeout(2000);
                    LEAVE_FUNC();
                    return 0;
                }
            }

            OnRequestComplete(RIL_E_SUCCESS, (char *) pResponse, nLength);
        }
#else
        OnRequestComplete(RIL_E_SUCCESS, (char *) pResponse, nLength);
#endif
    }
    else    // Result must not be error.
    {
        memset(&mRilCardStatus, 0, sizeof(RIL_CardStatus_V1_4));
        mRilCardStatus.card_state = (uErrCode==RIL_E_SIM_ABSENT)? RIL_CARDSTATE_ABSENT: RIL_CARDSTATE_ERROR;
        mRilCardStatus.universal_pin_state = RIL_PINSTATE_UNKNOWN;
        mRilCardStatus.gsm_umts_subscription_app_index = -1;
        mRilCardStatus.cdma_subscription_app_index = -1;
        mRilCardStatus.ims_subscription_app_index = -1;
        mRilCardStatus.num_applications = 0;
        mRilCardStatus.physicalSlotId = -1;

        LOGV("CardState:%s", (uErrCode==RIL_E_SIM_ABSENT)? "RIL_CARDSTATE_ABSENT": "RIL_CARDSTATE_ERROR");
        OnRequestComplete(RIL_E_SUCCESS, (char *) &mRilCardStatus, nFinalLength);
    }

    LEAVE_FUNC();
    return 0;
}

int SimService::OnGetSimStatusTimeout(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    for(int i=0; i<mRilCardStatus.num_applications; i++) {
        mRilCardStatus.applications[i].app_state = RIL_APPSTATE_UNKNOWN;
    }

    OnRequestComplete(RIL_E_SUCCESS, (char *) &mRilCardStatus, sizeof(mRilCardStatus));
    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED);
    LEAVE_FUNC();
    return 0;
}

BOOL SimService::IsValidPinPuk(int nSimPinPuk, const char *pszPinPuk)
{
    if(pszPinPuk==NULL) return false;

    if((nSimPinPuk==SIM_PIN && (strlen(pszPinPuk)<4 || strlen(pszPinPuk)>8))
            || (nSimPinPuk==SIM_PUK && strlen(pszPinPuk)<8)) {
        return FALSE;
    }

    for(int i=0; i<(int)strlen(pszPinPuk); i++) {
        if(pszPinPuk[i]<'0' || pszPinPuk[i]>'9') {
            return FALSE;
        }
    }

    return TRUE;
}

int SimService::SendPasswordInvalid(int nSimPinPuk, int nPinPukIndex)
{
    ENTER_FUNC();

    int nRemainCount = -1;
    if(nSimPinPuk==SIM_PIN) nRemainCount = m_nPinRemain[nPinPukIndex];
    else if(nSimPinPuk==SIM_PUK) nRemainCount = m_nPukRemain[nPinPukIndex];

    SimDataBuilder builder;
    const RilData *pRilData = builder.BuildSimPinPukResponse(nRemainCount);
    if ( pRilData == NULL )
    {
        return OnRequestComplete(RIL_E_NO_MEMORY);
    }

    int nResult = OnRequestComplete(RIL_E_PASSWORD_INCORRECT, pRilData->GetData(), pRilData->GetDataLength());
    delete pRilData;

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::DoVerifyPin(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    VerifyPIN *rildata = (VerifyPIN *)pMsg->GetRequestData();
    const char *pszPIN = rildata->GetPIN();
    const char *pszAID = rildata->GetAID();
    if (pszPIN != NULL && strlen(pszPIN)>0) {
        if(IsValidPinPuk(SIM_PIN, pszPIN)==FALSE) {
            nResult = SendPasswordInvalid(SIM_PIN, 0);
        }
        else
        {
            ProtocolSimBuilder builder;
            ModemData *pModemData = builder.BuildSimVerifyPin(1, pszPIN, pszAID);
            nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_VERIFY_PIN_DONE);

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
            SetAutoVerifyPin(pszPIN);
            m_nAutoPinState = AUTO_PIN_STATE_VERIFY;
#endif
        }
    }
    //Empty PIN is to get remain count
    else
    {
        LOGV("Empty PIN, Response Remain Count(%d)", m_nPinRemain[0]);
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimPinPukResponse(m_nPinRemain[0]);
        if ( pRilData == NULL )
        {
            return OnRequestComplete(RIL_E_NO_MEMORY);
        }

        nResult = OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnVerifyPinDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    SimDataBuilder builder;

    ProtocolSimVerifyPinAdapter adapter(pMsg->GetModemData());
    m_nPinRemain[0] = adapter.GetRemainCount();
    UINT uErrCode = adapter.GetErrorCode();
    const RilData *pRilData = builder.BuildSimPinPukResponse(adapter.GetRemainCount());
    if ( pRilData == NULL )
    {
        return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    if (uErrCode==RIL_E_SUCCESS)
    {
// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        if(m_nAutoPinState==AUTO_PIN_STATE_RECOVERY)
        {
            m_nAutoPinState = AUTO_PIN_STATE_RECOVERED;
            RilLogI("%s::%s() Automatic Verify PIN Done", m_szSvcName, __FUNCTION__);

            DoGetSimStatus(pMsg);
        }
        else if(m_nAutoPinState==AUTO_PIN_STATE_VERIFY)
        {
            m_nAutoPinState = AUTO_PIN_STATE_RECOVERED;
            SetAutoVerifyPinDone(TRUE);

            OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        }
        else
#endif
        {
            OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        }
    }
    else
    {
// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        if(m_nAutoPinState!=AUTO_PIN_STATE_RECOVERY)
#endif
        OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());
// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        SetAutoVerifyPinDone(FALSE);
        ClearAutoVerifyPin();
#endif
    }

    if (pRilData != NULL) delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int SimService::DoVerifyPuk(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    VerifyPUK *rildata = (VerifyPUK *)pMsg->GetRequestData();
    const char *pszPUK = rildata->GetPUK();
    const char *pszNewPIN = rildata->GetNewPIN();
    const char *pszAID = rildata->GetAID();
    if (pszPUK != NULL && strlen(pszPUK)>0 && pszNewPIN != NULL && strlen(pszNewPIN)>0) {
        if(IsValidPinPuk(SIM_PUK, pszPUK)==FALSE || IsValidPinPuk(SIM_PIN, pszNewPIN)==FALSE) {
            nResult = SendPasswordInvalid(SIM_PUK, 0);
        }
        else
        {
            ProtocolSimBuilder builder;
            ModemData *pModemData = builder.BuildSimVerifyPuk(1, pszPUK, pszNewPIN, pszAID);
            nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_VERIFY_PUK_DONE);

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
            SetAutoVerifyPin(pszNewPIN);
            m_nAutoPinState = AUTO_PIN_STATE_UNLOCK;
#endif
        }
    }
    //Empty PUK is to get remain count
    else
    {
        LOGV("Empty PUK, Response Remain Count(%d)", m_nPukRemain[0]);
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimPinPukResponse(m_nPukRemain[0]);
        if ( pRilData == NULL )
        {
            return OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }

        nResult = OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnVerifyPukDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    SimDataBuilder builder;

    ProtocolSimVerifyPukAdapter adapter(pMsg->GetModemData());
    m_nPukRemain[0] = adapter.GetRemainCount();
    UINT uErrCode = adapter.GetErrorCode();
    const RilData *pRilData = builder.BuildSimPinPukResponse(adapter.GetRemainCount());
    if ( pRilData == NULL )
    {
        return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    if (uErrCode==RIL_E_SUCCESS)
    {
// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        m_nAutoPinState = AUTO_PIN_STATE_RECOVERED;
        SetAutoVerifyPinDone(TRUE);
#endif
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
    }
    else
    {
        OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        SetAutoVerifyPinDone(FALSE);
        ClearAutoVerifyPin();
#endif
    }

    if ( pRilData != NULL ) delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int SimService::DoVerifyPin2(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    VerifyPIN *rildata = (VerifyPIN *)pMsg->GetRequestData();
    const char *pszPIN = rildata->GetPIN();
    const char *pszAID = rildata->GetAID();
    if (pszPIN != NULL && strlen(pszPIN)>0) {
        if(IsValidPinPuk(SIM_PIN, pszPIN)==FALSE) {
            nResult = SendPasswordInvalid(SIM_PIN, 1);
        }
        else
        {
            ProtocolSimBuilder builder;
            ModemData *pModemData = builder.BuildSimVerifyPin(2, pszPIN, pszAID);
            nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_VERIFY_PIN2_DONE);
        }
    }
    //Empty PIN2 is to get remain count
    else
    {
        LOGV("Empty PIN2, Response Remain Count(%d)", m_nPinRemain[1]);
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimPinPukResponse(m_nPinRemain[1]);
        if ( pRilData == NULL )
        {
            return OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }

        nResult = OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnVerifyPin2Done(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSimVerifyPinAdapter adapter(pMsg->GetModemData());
    m_nPinRemain[1] = adapter.GetRemainCount();
    UINT uErrCode = adapter.GetErrorCode();
    SimDataBuilder builder;
    const RilData *pRilData = builder.BuildSimPinPukResponse(adapter.GetRemainCount());
    if ( pRilData == NULL )
    {
        return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    if(uErrCode==RIL_E_SUCCESS)
    {
        // PIN2 Verification for FDN enable/disable
        if(m_pEnableFdnData!=NULL)
        {
            if(SendRequest(m_pEnableFdnData, TIMEOUT_SIM_DEFAULT, MSG_SIM_SET_FACILITY_LOCK_DONE)==-1)
            {
                OnRequestComplete(RIL_E_GENERIC_FAILURE);
            }

            // delete m_pEnableFdnData;
            m_pEnableFdnData = NULL;
        }
        // PIN2 Verification for FDN update
        else if(m_pSimIoData!=NULL)
        {
            if(SendRequest(m_pSimIoData, TIMEOUT_SIM_DEFAULT, MSG_SIM_IO_DONE)==-1)
            {
                OnRequestComplete(RIL_E_GENERIC_FAILURE);
            }

            //delete m_pSimIoData;
            m_pSimIoData = NULL;
        }
        else
        {
            OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        }
    }
    else
    {
        // PIN2 is incorrect for FDN operation
        if(m_pEnableFdnData!=NULL)
        {
            OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());

            delete m_pEnableFdnData;
            m_pEnableFdnData = NULL;
        }
        else if(m_pSimIoData!=NULL)
        {
            OnRequestComplete(RIL_E_SIM_PIN2);

            delete m_pSimIoData;
            m_pSimIoData = NULL;
        }
        else
        {
            OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());
        }
    }

    if( NULL!=pRilData)
        delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int SimService::DoVerifyPuk2(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    VerifyPUK *rildata = (VerifyPUK *)pMsg->GetRequestData();
    const char *pszPUK = rildata->GetPUK();
    const char *pszNewPIN = rildata->GetNewPIN();
    const char *pszAID = rildata->GetAID();
    if (pszPUK != NULL && strlen(pszPUK)>0 && pszNewPIN != NULL && strlen(pszNewPIN)>0) {
        if(IsValidPinPuk(SIM_PUK, pszPUK)==FALSE || IsValidPinPuk(SIM_PIN, pszNewPIN)==FALSE) {
            nResult = SendPasswordInvalid(SIM_PUK, 1);
        }
        else
        {
            ProtocolSimBuilder builder;
            ModemData *pModemData = builder.BuildSimVerifyPuk(2, pszPUK, pszNewPIN, pszAID);
            nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_VERIFY_PUK2_DONE);
        }
    }
    //Empty PUK2 is to get remain count
    else
    {
        LOGV("Empty PUK2, Response Remain Count(%d)", m_nPukRemain[1]);
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimPinPukResponse(m_nPukRemain[1]);
        if ( pRilData == NULL )
        {
            return OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }

        nResult = OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnVerifyPuk2Done(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    SimDataBuilder builder;

    ProtocolSimVerifyPukAdapter adapter(pMsg->GetModemData());
    m_nPukRemain[1] = adapter.GetRemainCount();
    UINT uErrCode = adapter.GetErrorCode();
    const RilData *pRilData = builder.BuildSimPinPukResponse(adapter.GetRemainCount());
    if ( pRilData == NULL )
    {
        return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    if (uErrCode==RIL_E_SUCCESS)
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
    else
        OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());

    if (pRilData != NULL)
        delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int SimService::DoChangePin(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    ChangePIN *rildata = (ChangePIN *)pMsg->GetRequestData();
    const char *pszOldPIN = rildata->GetOldPIN();
    const char *pszNewPIN = rildata->GetNewPIN();
    const char *pszAID = rildata->GetAID();
    if (pszOldPIN != NULL && strlen(pszOldPIN)>0 && pszNewPIN != NULL && strlen(pszNewPIN)>0) {
        if(IsValidPinPuk(SIM_PIN, pszOldPIN)==FALSE || IsValidPinPuk(SIM_PIN, pszNewPIN)==FALSE) {
            nResult = SendPasswordInvalid(SIM_PIN, 0);
        }
        else
        {
            ProtocolSimBuilder builder;
            ModemData *pModemData = builder.BuildSimChangePin(1, pszOldPIN, pszNewPIN, pszAID);
            nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_CHANGE_PIN_DONE);

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
            SetAutoVerifyPin(pszNewPIN);
            m_nAutoPinState = AUTO_PIN_STATE_CHANGE;
#endif
        }
    }
    else
    {
        nResult = SendPasswordInvalid(SIM_PIN, 0);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnChangePinDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    SimDataBuilder builder;

    ProtocolSimVerifyPinAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    const RilData *pRilData = builder.BuildSimPinPukResponse(adapter.GetRemainCount());
    if ( pRilData == NULL )
    {
        return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    if (uErrCode==RIL_E_SUCCESS)
    {
// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        m_nAutoPinState = AUTO_PIN_STATE_RECOVERED;
        SetAutoVerifyPinDone(TRUE);
#endif
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
    }
    else
    {
        OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        SetAutoVerifyPinDone(FALSE);
        ClearAutoVerifyPin();
#endif
    }

    if (pRilData != NULL)
        delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int SimService::DoChangePin2(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    ChangePIN *rildata = (ChangePIN *)pMsg->GetRequestData();
    const char *pszOldPIN = rildata->GetOldPIN();
    const char *pszNewPIN = rildata->GetNewPIN();
    const char *pszAID = rildata->GetAID();
    if (pszOldPIN != NULL && strlen(pszOldPIN)>0 && pszNewPIN != NULL && strlen(pszNewPIN)>0) {
        if(IsValidPinPuk(SIM_PIN, pszOldPIN)==FALSE || IsValidPinPuk(SIM_PIN, pszNewPIN)==FALSE) {
            nResult = SendPasswordInvalid(SIM_PIN, 1);
        }
        else
        {
            ProtocolSimBuilder builder;
            ModemData *pModemData = builder.BuildSimChangePin(2, pszOldPIN, pszNewPIN, pszAID);
            nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_CHANGE_PIN2_DONE);
        }
    }
    else
    {
        nResult = SendPasswordInvalid(SIM_PIN, 1);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnChangePin2Done(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    SimDataBuilder builder;

    ProtocolSimVerifyPinAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    const RilData *pRilData = builder.BuildSimPinPukResponse(adapter.GetRemainCount());
    if ( pRilData == NULL )
    {
        return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    if (uErrCode==RIL_E_SUCCESS)
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
    else
        OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());

    if (pRilData != NULL)
        delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int SimService::DoVerifyNetworkLock(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    // check last saved sim status and decide FAC type.
    int nFac = -1; // PN or PU or PP or PC
    for(int i=0; i<mRilCardStatus.num_applications; i++)
    {
        if(mRilCardStatus.applications[i].app_state == RIL_APPSTATE_SUBSCRIPTION_PERSO) {
            if (mRilCardStatus.applications[i].perso_substate == RIL_PERSOSUBSTATE_SIM_NETWORK) {
                nFac = FacilityLock::FAC_PN;
                LOGV("FAC type is FacilityLock::FAC_PN");
                break;
            } else if (mRilCardStatus.applications[i].perso_substate == RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET) {
                nFac = FacilityLock::FAC_PU;
                LOGV("FAC type is FacilityLock::FAC_PU");
                break;
            } else if (mRilCardStatus.applications[i].perso_substate == RIL_PERSOSUBSTATE_SIM_CORPORATE) {
                nFac = FacilityLock::FAC_PP;
                LOGV("FAC type is FacilityLock::FAC_PP");
                break;
            } else if (mRilCardStatus.applications[i].perso_substate == RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER) {
                nFac = FacilityLock::FAC_PC;
                LOGV("FAC type is FacilityLock::FAC_PC");
                break;
            }
        }
    }
    if ( nFac == -1) {
        RilLogE("%s(): FAC type is invalid", __FUNCTION__);
        LEAVE_FUNC();
        return -1;
    }

    VerifyNetLock *rildata = (VerifyNetLock *)pMsg->GetRequestData();
    const char *pszCode = rildata->GetDepersonalCode();
    if (pszCode != NULL && strlen(pszCode)>0) {
        ProtocolSimBuilder builder;
        ModemData *pModemData = builder.BuildSimVerifyNetworkLock(FacilityLock::FAC_PN, pszCode, 0, NULL);
        nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_VERIFY_NETWORK_LOCK_DONE);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnVerifyNetworkLockDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    SimDataBuilder builder;

    ProtocolSimVerifyNetLockAdapter adapterSimVerifyNetLock(pMsg->GetModemData());
    UINT uErrCode = adapterSimVerifyNetLock.GetErrorCode();
    const RilData *pRilData = builder.BuildSimNetworkLockResponse(adapterSimVerifyNetLock.GetRemainCount());
    RilLogV("remain count : %d", adapterSimVerifyNetLock.GetRemainCount());
    if ( pRilData == NULL )
    {
        return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    if (uErrCode==RIL_E_SUCCESS)
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
    else
        OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());

    if (pRilData != NULL)
        delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int SimService::DoSimIo(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    SimIoData *rildata = (SimIoData *)pMsg->GetRequestData();

    int nAppType = RIL_APPTYPE_UNKNOWN;
    RilLogV("%s::%s() RIL AID:%s, App Num:%d", m_szSvcName, __FUNCTION__, rildata->m_strAid, mRilCardStatus.num_applications);
    if(strlen(rildata->m_strAid)>0)
    {
        for(int i=0; i<mRilCardStatus.num_applications; i++)
        {
            RilLogV("%s::%s() %d. Stored AID:%s", m_szSvcName, __FUNCTION__, i, mRilCardStatus.applications[i].aid_ptr);

            if(mRilCardStatus.applications[i].aid_ptr!=NULL
                    && strcmp(rildata->m_strAid, mRilCardStatus.applications[i].aid_ptr)==0)
            {
                nAppType = mRilCardStatus.applications[i].app_type;
                break;
            }
        }
    }

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimIO(rildata->m_nCmd, nAppType, rildata->m_nFileId, rildata->m_strPath,
                                                        rildata->m_nP1, rildata->m_nP2, rildata->m_nP3,
                                                        rildata->m_nDataLen, (BYTE *) rildata->m_strData,
                                                        rildata->m_strPin2, rildata->m_strAid);

    // UPDATE_BINARY, UPDATE_RECORD
    UINT uResultMsg = 0;
    // PIN2 for FDN
    if(SIM_EFID_FDN==rildata->m_nFileId && (rildata->m_nCmd==0xD6 || rildata->m_nCmd==0xDC) && strlen(rildata->m_strPin2)>0)
    {
        if(m_pSimIoData!=NULL) delete m_pSimIoData;
        m_pSimIoData = pModemData;

        pModemData = builder.BuildSimVerifyPin(2, rildata->m_strPin2, rildata->m_strAid);
        uResultMsg = MSG_SIM_VERIFY_PIN2_DONE;
    }
    else uResultMsg = MSG_SIM_IO_DONE;

    if(pModemData!=NULL) nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, uResultMsg);
    //if(pModemData!=NULL) nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_IO_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnSimIoDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        ProtocolSimIOAdapter adapter(pMsg->GetModemData());

        SimIoData *rildata = (SimIoData *)GetCurrentMsg()->GetRequestData();
        if (rildata != NULL) {
            // Fetch a correct MNC length from IMSI using EF_AD payload
            if (rildata->m_nCmd == COMMAND_READ_BINARY && rildata->m_nFileId == EF_AD) {
                IccIoResult iccioResult(adapter.GetSw1(), adapter.GetSw2(), (char *)adapter.GetResponse(), adapter.GetResponseLength());

                string carrier = FetchSimOperator(iccioResult);
                RilProperty *pProperty = GetRilContextProperty();
                if (pProperty != NULL && carrier.length() >= 5) {
                    pProperty->Put(RIL_CONTEXT_SIM_OPERATOR, carrier.c_str());
                }

                // set open carrier index
                SetOpenCarrierIndex(carrier.c_str());
                SendOpenCarrierInfoRilReq(carrier.c_str());

                // notify SIM operator updated to PS Service
                RilDataStrings *rildata = new RilDataStrings(2);
                if (rildata != NULL) {
                    rildata->SetString(0, mImsi.c_str());
                    rildata->SetString(1, carrier.c_str());

                    Message *pMsg = Message::ObtainMessage(rildata, RIL_SERVICE_PS, MSG_PS_SIM_OPERATOR_INFO_UPDATED);
                    ServiceMgr *pServigMrg = GetRilContext()->GetServiceManager();
                    if (pServigMrg != NULL) {
                        if (pServigMrg->SendMessage(pMsg) < 0) {
                            RilLogE("%s(): SendMessage error", __FUNCTION__);
                            delete pMsg;
                        }
                    } else {
                        delete pMsg;
                    }
                }
            }
        }
        else {
            RilLogV("[%s] transaction had been completed or not acting", GetServiceName());
        }

        //SimIoData *pSimIoData = (SimIoData *)pMsg->GetRequestData();
        UINT uRilResult = RIL_E_SUCCESS;
        if(adapter.GetSw1()!=0x90 && adapter.GetSw1()!=0x91 && adapter.GetSw1()!=0x9e && adapter.GetSw1()!=0x9f)
        {
            uRilResult = RIL_E_GENERIC_FAILURE;
        }

        SimDataBuilder builder;
        const RilData *pRilData = NULL;
        if(rildata != NULL && rildata->m_nCmd == COMMAND_STATUS)
        {
            pRilData = builder.BuildSimIoFcpTemplateResponse(adapter.GetSw1(), adapter.GetSw2(), adapter.GetResponseLength(), adapter.GetResponse());
        }
        else
        {
            pRilData = builder.BuildSimIoResponse(adapter.GetSw1(), adapter.GetSw2(), adapter.GetResponseLength(), adapter.GetResponse());
        }

        if ( pRilData == NULL )
        {
            return OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }
        OnRequestComplete(uRilResult, pRilData->GetData(), pRilData->GetDataLength());

        if ( pRilData != NULL )
            delete pRilData;
    }
    else OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoGetFacilityLock(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    FacilityLock rildata;

    StringsRequestData *pReq = (StringsRequestData *) pMsg->GetRequestData();
    if (pReq == NULL) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        RilLogE("%s::%s() Invalid arguments", m_szSvcName, __FUNCTION__);
        LEAVE_FUNC();
        return 0;
    }

    char **ppString = pReq->GetStringsContent();
    if (ppString == NULL || ppString[0] == NULL) { // facility is null
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        RilLogE("%s::%s() Invalid arguments", m_szSvcName, __FUNCTION__);
        nResult = 0;
    } else {
        if (rildata.Parse(*pReq) == FALSE) {
            OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
            RilLogE("%s::%s() Invalid arguments", m_szSvcName, __FUNCTION__);
            LEAVE_FUNC();
            return 0;
        }

        ProtocolSimBuilder builder;
        ModemData *pModemData = builder.BuildSimGetFacilityLock((char *) rildata.GetCode(), rildata.GetPassword(),
                                                                         rildata.GetServiceClass(), rildata.GetAID());
        nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_FACILITY_LOCK_DONE);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGetFacilityLockDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        //int nLength = 0;
        ProtocolSimGetFacilityLockAdapter adapter(pMsg->GetModemData());
        RilLogV("%s::%s() Lock Mode:%d, Service Class:%d", m_szSvcName, __FUNCTION__, adapter.GetLockMode(), adapter.GetServiceClass());
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimGetFacilityLockResponse(adapter.GetServiceClass());
        if (pRilData != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
            delete pRilData;
        }
        else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }

    }
    else OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoSetFacilityLock(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    FacilityLock rildata;

    StringsRequestData *pReq = (StringsRequestData *) pMsg->GetRequestData();
    if (pReq == NULL) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        RilLogE("%s::%s() Invalid arguments", m_szSvcName, __FUNCTION__);
        LEAVE_FUNC();
        return 0;
    }

    char **ppString = pReq->GetStringsContent();
    if (ppString == NULL || ppString[0] == NULL) { // facility is null
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        RilLogE("%s::%s() Invalid arguments", m_szSvcName, __FUNCTION__);
        nResult = 0;
    } else {
        if (rildata.Parse(*pReq)==FALSE) {
            OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
            RilLogE("%s::%s() Invalid arguments", m_szSvcName, __FUNCTION__);
            LEAVE_FUNC();
            return 0;
        }

        UINT uResultMsg = 0;
        ProtocolSimBuilder builder;
        ModemData *pModemData = builder.BuildSimSetFacilityLock((char *) rildata.GetCode(), rildata.GetLockState(),
                                                            rildata.GetPassword(), rildata.GetServiceClass(), rildata.GetAID());

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        RilLogV("%s::%s() FAC(%s), LockState(%d)", m_szSvcName, __FUNCTION__, rildata.GetCode(), rildata.GetLockState());
        if(strcmp(rildata.GetCode(), "SC")==0) {
            SetAutoVerifyPin(rildata.GetPassword());
            if(rildata.GetLockState()==SIM_FAC_LOCK_STATE_UNLOCK) m_nAutoPinState = AUTO_PIN_STATE_DISABLE;
            else if(rildata.GetLockState()==SIM_FAC_LOCK_STATE_LOCK) m_nAutoPinState = AUTO_PIN_STATE_ENABLE;
        }
#endif

        // PIN2 for FDN
        if(strcmp(rildata.GetCode(), "FD")==0) {
            if (rildata.GetPassword() != NULL && strlen(rildata.GetPassword())>0) {
                if(m_pEnableFdnData!=NULL) delete m_pEnableFdnData;

                m_pEnableFdnData = pModemData;
                pModemData = builder.BuildSimVerifyPin(2, rildata.GetPassword(), rildata.GetAID());
                uResultMsg = MSG_SIM_VERIFY_PIN2_DONE;
            }
        } else {
            uResultMsg = MSG_SIM_SET_FACILITY_LOCK_DONE;
        }

        if(uResultMsg>0 && pModemData!=NULL) {
            nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, uResultMsg);
        } else {
            if (pModemData!=NULL) delete pModemData;
        }
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnSetFacilityLockDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    SimDataBuilder builder;

    ProtocolSimSetFacilityLockAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    const RilData *pRilData = builder.BuildSimSetFacilityLockResponse(adapter.GetRemainCount());

    if (uErrCode == RIL_E_SUCCESS)
    {
        if (pRilData != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        }
        else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        if(m_nAutoPinState==AUTO_PIN_STATE_ENABLE)
        {
            m_nAutoPinState = AUTO_PIN_STATE_RECOVERED;
            SetAutoVerifyPinDone(TRUE);
        }
        else if(m_nAutoPinState==AUTO_PIN_STATE_DISABLE) {
            ClearAutoVerifyPin();
            // for kernel panic
            SystemProperty::Set(m_szAutoPinPropertyKeyByPanic, "");
        }
#endif
    }
    else
    {
        if (pRilData != NULL) {
            OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());
        }
        else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        SetAutoVerifyPinDone(FALSE);
        ClearAutoVerifyPin();
#endif
    }

    if (pRilData != NULL)
        delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int SimService::DoGetIsimAuth(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    if (m_nSimCardState==SIM_CARDSTATE_ABSENT) {
        OnRequestComplete(RIL_E_SIM_ABSENT);
        LEAVE_FUNC();
        return -1;
    }

    IsimAuth rildata;
    if(rildata.Parse((StringRequestData *) pMsg->GetRequestData())==FALSE
            || rildata.GetLength()==0 || rildata.GetAuth()==NULL) {
        LEAVE_FUNC();
        return -1;
    }

    ProtocolSimBuilder builder;
#ifdef _WR_ISIM_AUTH_TYPE_
    int nAuthType = 0x00/*ISIM_AUTH_IMS*/;

    BOOL bExistIMS=FALSE, bExist3G=FALSE, bExistGSM=FALSE;
    for(int i=0; i<mRilCardStatus.num_applications; i++)
    {
        switch(mRilCardStatus.applications[i].app_type)
        {
        case RIL_APPTYPE_SIM: bExistGSM = TRUE; break;
        case RIL_APPTYPE_USIM: bExist3G = TRUE; break;
        case RIL_APPTYPE_ISIM: bExistIMS = TRUE; break;
        default: break;
        }
    }

    RilLogV("%s::%s() IMS(%c), 3G(%c), GSM(%c)", m_szSvcName, __FUNCTION__, bExistIMS? 'O':'X', bExist3G? 'O':'X', bExistGSM? 'O':'X');

    if(bExistIMS==TRUE) nAuthType = 0x00/*ISIM_AUTH_IMS*/;
    else if(bExist3G==TRUE) nAuthType = 0x02/*ISIM_AUTH_3G*/;
    else if(bExistGSM==TRUE) nAuthType = 0x01/*ISIM_AUTH_GSM*/;

    ModemData *pModemData = builder.BuildSimGetIsimAuth(nAuthType, rildata.GetAuth(), rildata.GetLength());
#else
    ModemData *pModemData = builder.BuildSimGetIsimAuth(0x00/*ISIM_AUTH_IMS*/, rildata.GetAuth(), rildata.GetLength());
#endif
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_ISIM_AUTH_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGetIsimAuthDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        ProtocolSimGetSimAuthAdapter adapter(pMsg->GetModemData());
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimGetIsimAuthResponse(adapter.GetAuthLength(), adapter.GetAuth());
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }
    else OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

/*
int SimService::DoGetIsimGbaAuth(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);
    int nResult = -1;
    SimGbaAuth rildata;
    if(rildata.Parse((StringRequestData *) pMsg->GetRequestData())==FALSE
            || rildata.GetLength()==0 || rildata.GetAuth()==NULL) {
        LEAVE_FUNC();
        return -1;
    }
    ProtocolSimBuilder builder;
    int nAuthType = 0x00; // ISIM_AUTH_USIM;
    BOOL bExistIMS=FALSE, bExist3G=FALSE, bExistGSM=FALSE;
    for(int i=0; i<mRilCardStatus.num_applications; i++)
    {
        switch(mRilCardStatus.applications[i].app_type)
        {
        case 0x01: bExistGSM = TRUE; break; // SIM_APPS_TYPE_SIM
        case 0x02: bExist3G = TRUE; break;  // SIM_APPS_TYPE_USIM
        case 0x05: bExistIMS = TRUE; break; // SIM_APPS_TYPE_ISIM
        default: break;
        }
    }
    RilLogV("%s::%s() IMS(%c), 3G(%c), GSM(%c)", m_szSvcName, __FUNCTION__, bExistIMS? 'O':'X', bExist3G? 'O':'X', bExistGSM? 'O':'X');
    if(bExistIMS==TRUE) nAuthType = 0x05; // ISIM_AUTH_ISIM
    else if(bExist3G==TRUE) nAuthType = 0x02; // ISIM_AUTH_USIM
    else if(bExistGSM==TRUE) nAuthType = 0x02; // ISIM_AUTH_USIM
    ModemData *pModemData = builder.BuildSimGetIsimGbaAuth(nAuthType, rildata.GetGbaType(), rildata.GetGbaTag(), rildata.GetAuth(), rildata.GetLength());
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_ISIM_GBA_AUTH_DONE);
    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}
int SimService::OnGetIsimGbaAuthDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);
    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        ProtocolSimGetIsimGbaAuthAdapter adapter(pMsg->GetModemData());
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimGetIsimGbaAuthResponse(adapter.GetGbaAuthLength(), adapter.GetGbaAuth());
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }
    else
    {
        OnRequestComplete(uErrCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }
    LEAVE_FUNC();
    return 0;
}
*/

int SimService::DoTransmitSimApduBasic(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    SimAPDU *rildata = (SimAPDU *)pMsg->GetRequestData();
    if (NULL == rildata) {
        LEAVE_FUNC();
        return -1;
    }

    int sessionId = rildata->GetSessionId();
    // bits b1, b2 of the CLA should contain the logical channel value. from Table 10.3, ETSI 102 221 spec.
    int cla = rildata->GetCla() | (0x03 & sessionId);
    int instruction = rildata->GetInstruction();
    int p1 = rildata->GetP1();
    int p2 = rildata->GetP2();
    int p3 = rildata->GetP3();
    const char *pszApduData = rildata->GetDataLength()? (const char*) rildata->GetData() : NULL;

    LOGV("sessionId(%d), cla(%d), instruction(%d), p1(%d), p2(%d), p3(%d) dataLen(%d)",
                            sessionId, cla, instruction, p1, p2, p3, rildata->GetDataLength());

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimTransmitApduBasic(sessionId, cla, instruction, p1, p2, p3, pszApduData);
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_TRANSMIT_APDU_BASIC_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnTransmitSimApduBasicDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSimTransmitApduBasicAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();

    if (uErrCode == RIL_E_SUCCESS)
    {
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimTransmitApduBasicResponse(adapter.GetApduLength(), adapter.GetApdu());
        if (pRilData != NULL) {
            // ETSI TS 102.201, 7.2.2.3.1
            // SW1 : '6C' : The terminal shall wait for a second procedure byte then immediately
            // repeat the previous command header to the UICC using a length of
            // 'XX', where 'XX' is the value of the second procedure byte (SW2).
            int nSW1 = adapter.GetSw1();
            int nSW2 = adapter.GetSw2();
            if(nSW1==0x6C)
            {
                LOGV("SW1:0x%02X, Retransmission with P3(%d:0x%X)", nSW1, nSW2, nSW2);
                Message *pCurrMsg = GetCurrentMsg();
                SimAPDU *rildata = (SimAPDU *)pCurrMsg->GetRequestData();
                rildata->SetP3(nSW2);
                if(DoTransmitSimApduBasic(pCurrMsg)==-1) OnRequestComplete(RIL_E_INTERNAL_ERR);
            }
            else    // Not 0x6C
                OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());

            delete pRilData;
        }

    }
    else
        OnRequestComplete(RIL_E_SIM_ERR);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoOpenSimChannel(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    SimOpenChannel *rildata = (SimOpenChannel *)pMsg->GetRequestData();
    const char *pszAID = rildata->GetAid();
    int p2 = rildata->GetP2();
    LOGD("AID=%s, P2=0x%02X", pszAID, p2);

    /* hardware/interfaces/radio/1.0/types.hal
    enum P2Constant : int32_t {
        NO_P2 = -1,             // No P2 value is provided
    };
    */

    ProtocolSimBuilder builder;
    ModemData *pModemData = NULL;
    if (p2 == -1) pModemData = builder.BuildSimOpenChannel(pszAID);
    else pModemData = builder.BuildSimOpenChannelWithP2(pszAID, p2);

    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_OPEN_CHANNEL_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnOpenSimChannelDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSimOpenChannelAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();

    RilLogV("Session ID : %d, uErrCode : %d", adapter.GetSessionID(), uErrCode);

    SimDataBuilder builder;
    const RilData *pRilData = builder.BuildSimOpenChannelResponse(adapter.GetSessionID(), adapter.GetSw1(), adapter.GetSw2(), adapter.GetResponseLength(), adapter.GetResponse());
    if (uErrCode == RIL_E_SUCCESS)
    {
        /* When error code is NO_SUCH_ELEMENT, need to close channel.
                so, sends session id to framework. */
        if (pRilData != NULL)
            OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        else
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }
    else {
        if (pRilData != NULL) {
            if (uErrCode == RIL_E_GENERIC_FAILURE) {
                BYTE sw1 = adapter.GetSw1();
                BYTE sw2 = adapter.GetSw2();
                if (sw1 == 0x69 && (sw2 == 0x85 || sw2 == 0x99)) {
                    OnRequestComplete(RIL_E_NO_SUCH_ELEMENT, pRilData->GetData(), pRilData->GetDataLength());
                    RilLogV("%s() CHANGE ERROR CODE : RIL_E_GENERIC_FAILURE -> RIL_E_NO_SUCH_ELEMENT", __FUNCTION__);
                    if (sw2 == 0x99) {
                        RilLogV("%s() CRC2: NoSuchElementError", __FUNCTION__);
                        RilLogV("%s() if the AID on the SE is not available (or cannot be selected) or a logical channel is already open to a non-multiselectable applet.", __FUNCTION__);
                    } else if (sw2 == 0x85) {
                        RilLogV("%s() Conditions of use not satisfied", __FUNCTION__);
                    }
                    RilLogV("%s() sw1: 0x%X, sw2: 0x%X", __FUNCTION__, sw1, sw2);
                } else {
                    OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());
                }
            } else {
                OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());
            }
        } else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }
    }

    if (pRilData != NULL)
        delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int SimService::DoCloseSimChannel(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        LEAVE_FUNC();
        return -1;
    }

    INT32 nSessionID = rildata->GetInt();
    RilLogV("session id : %d", nSessionID);

    if (nSessionID == 0) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        RilLogE("%s::%s() Invalid arguments", m_szSvcName, __FUNCTION__);
        nResult = 0;
    } else {
        ProtocolSimBuilder builder;
        ModemData *pModemData = builder.BuildSimCloseChannel(nSessionID);
        nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_CLOSE_CHANNEL_DONE);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnCloseSimChannelDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        LEAVE_FUNC();
        return -1;
    }

    ProtocolSimResponseAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS)
        OnRequestComplete(RIL_E_SUCCESS);
    else
        OnRequestComplete(errorCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoTransmitSimApduChannel(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    SimAPDU *rildata = (SimAPDU *)pMsg->GetRequestData();
    if (NULL == rildata) {
        LEAVE_FUNC();
        return -1;
    }

    int sessionId = rildata->GetSessionId();
    // bits b1, b2 of the CLA should contain the logical channel value. from Table 10.3, ETSI 102 221 spec.
    int cla = rildata->GetCla() | (0x03 & sessionId);
    int instruction = rildata->GetInstruction();
    int p1 = rildata->GetP1();
    int p2 = rildata->GetP2();
    int p3 = rildata->GetP3();
    const char *pszApduData = rildata->GetDataLength()? (const char*) rildata->GetData() : NULL;

    LOGV("sessionId(%d), cla(%d), instruction(%d), p1(%d), p2(%d), p3(%d) dataLen(%d)",
                            sessionId, cla, instruction, p1, p2, p3, rildata->GetDataLength());

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimTransmitApduChannel(sessionId, cla, instruction, p1, p2, p3, pszApduData);
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_TRANSMIT_APDU_CHANNEL_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnTransmitSimApduChannelDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSimResponseAdapter response(pMsg->GetModemData());
    UINT uErrCode = response.GetErrorCode();

    if (uErrCode == RIL_E_SUCCESS)
    {
        ProtocolSimTransmitApduChannelAdapter adapter(pMsg->GetModemData());

        SimDataBuilder builder;

        //RilLogV("%s::%s() sw1(%d), sw2(%d), length(%d), data(%s)", m_szSvcName, __FUNCTION__,
        //                            adapter.GetSw1(), adapter.GetSw2(), adapter.GetResponseLength());

        const RilData *pRilData = builder.BuildSimTransmitApduChannelResponse(adapter.GetSw1(), adapter.GetSw2(), adapter.GetApduLength(), adapter.GetApdu());
        if (pRilData != NULL) {
            // ETSI TS 102.201, 7.2.2.3.1
            // SW1 : '6C' : The terminal shall wait for a second procedure byte then immediately
            // repeat the previous command header to the UICC using a length of
            // 'XX', where 'XX' is the value of the second procedure byte (SW2).
            int nSW1 = adapter.GetSw1();
            int nSW2 = adapter.GetSw2();
            if(nSW1==0x6C)
            {
                LOGV("SW1:0x%02X, Retransmission with P3(%d:0x%X)", nSW1, nSW2, nSW2);
                Message *pCurrMsg = GetCurrentMsg();
                SimAPDU *rildata = (SimAPDU *)pCurrMsg->GetRequestData();
                rildata->SetP3(nSW2);
                if(DoTransmitSimApduChannel(pCurrMsg)==-1) OnRequestComplete(RIL_E_GENERIC_FAILURE);
            }
            else    // Not 0x6C
                OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());

            delete pRilData;
        }
        else
            OnRequestComplete(RIL_E_GENERIC_FAILURE);

    }
    else
        OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::OnSimStatusChanged(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED);

    LEAVE_FUNC();
    return 0;
}

// Lollipop
int SimService::DoGetSimAuth(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    SimAuthentication *rildata = (SimAuthentication *) pMsg->GetRequestData();

    int nAppType = RIL_APPTYPE_UNKNOWN;
    RilLogV("%s::%s() RIL AID:%s, App Num:%d", m_szSvcName, __FUNCTION__, rildata->GetAid(), mRilCardStatus.num_applications);
    if(strlen(rildata->GetAid())>0)
    {
        for(int i=0; i<mRilCardStatus.num_applications; i++)
        {
            if(mRilCardStatus.applications[i].aid_ptr!=NULL
                    && strcmp(rildata->GetAid(), mRilCardStatus.applications[i].aid_ptr)==0)
            {
                nAppType = mRilCardStatus.applications[i].app_type;
                RilLogV("%s::%s() App Type:%d", m_szSvcName, __FUNCTION__, nAppType);
                break;
            }
        }
    }

    RilLogV("%s::%s() AuthContext:0x%08X", m_szSvcName, __FUNCTION__, rildata->GetAuthContext());

    BYTE *pAuthentication = NULL;
    int nAuthLength = 0;
    if(rildata->GetAuthentication()!=NULL)
    {
        char *pEncodedAuth = rildata->GetAuthentication();
        if(strlen(pEncodedAuth)%4!=0)
        {
            RilLogV("%s::%s() not a multiple of 4 bytes", m_szSvcName, __FUNCTION__);
            OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
            LEAVE_FUNC();
            return 0;
        }

        RilLogV("%s::%s() Authentication:%s", m_szSvcName, __FUNCTION__, rildata->GetAuthentication());
        nAuthLength = Base64_Decode((BYTE **) &pAuthentication, rildata->GetAuthentication());
    }

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimGetSimAuth(rildata->GetAuthContext(), pAuthentication, nAuthLength, nAppType);
    if(pModemData!=NULL) nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_SIM_AUTH_DONE);
    else OnRequestComplete(RIL_E_NO_MEMORY);
    if(pAuthentication!=NULL) delete [] pAuthentication;

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGetSimAuthDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        ProtocolSimGetSimAuthAdapter adapter(pMsg->GetModemData());
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimGetSimAuthResponse(adapter.GetAuthType(), adapter.GetParameterLength(), adapter.GetAuthLength(), adapter.GetAuth());
        if(pRilData!=NULL)
        {
            OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
            delete pRilData;
        }
        else OnRequestComplete(RIL_E_INTERNAL_ERR);
    }
    else if(uErrCode==RIL_E_GENERIC_FAILURE) OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
    else OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoGetImsi(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    StringsRequestData *rildata = (StringsRequestData *)pMsg->GetRequestData();
    const char *aid = rildata->GetString(0);
    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildGetImsi(aid);
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_IMSI_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGetImsiDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimImsiAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();
    if (uErrCode == RIL_E_SUCCESS) {

        const char *imsi = checkMmcMnc(adapter.GetImsi());
        // update IMSI
        if (GetCurrentReqeustData() != NULL) {
            const char *aid = ((StringsRequestData *)GetCurrentReqeustData())->GetString(0);
            UpdateImsi(aid, imsi);
        }
        else {
            RilLogW("[%s]%s Invalid IMSI transaction.", GetServiceName(), __FUNCTION__);
        }

        SimDataBuilder builder;
        const RilData *rildata = builder.BuildGetImsiResponse(imsi);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoSimGetATR(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimGetATR();
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_ATR_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnSimGetATRDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimATRAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();
    if (uErrCode == RIL_E_SUCCESS) {
        BYTE result = adapter.GetResult();
        if (result == TRUE) {
            BYTE length = adapter.GetATRLength();
            const char *atr = adapter.GetATR();

            SimDataBuilder builder;
            const RilData *rildata = builder.BuildGetATRResponse(atr, length);
            if (rildata != NULL) {
                OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
                delete rildata;
            }
        } else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }
    }
    else OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

/* PhoneBook */
int SimService::DoReadPbEntry(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    IntsRequestData *pReq = (IntsRequestData *)pMsg->GetRequestData();
    int type = pReq->GetInt(0);
    int index = pReq->GetInt(1);

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimReadPbEntry(type, index);
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_READ_PB_ENTRY_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnReadPbEntryDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimReadPbEntry adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();

    if (uErrCode == RIL_E_SUCCESS) {
        RIL_ReadPbEntry pb;
        RIL_PbEntry1 *pb1;
        RIL_3GPb *p;
        RIL_PbEntry2 *pb2;
        int pbLen = 0, cnt = 0;

        pb.type = adapter.GetPbType();
        pb.index = adapter.GetIndex();
        int length = adapter.GetDataLength();
        char *entry = adapter.GetEntryData();
        char entry_data[MAX_PB_ENTRY_LEN] = { 0,};

        switch(pb.type) {
        case PB_EN:
        case PB_ADN_2G:
        case PB_FDN:
        case PB_SDN:
        case PB_LDN:
        case PB_MSISDN:
            pb1 = (RIL_PbEntry1 *) entry_data;
            pb1->num_len = *entry;
            pb1->num_type = *(entry+1);
            pb1->number = new char[pb1->num_len];
            memcpy(pb1->number, entry+2, pb1->num_len);

            entry = entry + 2 + pb1->num_len;
            pb1->text_len = *entry;
            pb1->text_type = *(entry+1);
            pb1->text = new char[pb1->text_len];
            memcpy(pb1->text, entry+2, pb1->text_len);

            pbLen = pb1->num_len + pb1->text_len + sizeof(int)*6;
            OnRequestComplete(uErrCode, &pb, pbLen);
            delete [](pb1->number);
            delete [](pb1->text);
            break;
        case PB_ADN_3G:
            p = (RIL_3GPb *) entry_data;
            while (length > 0) {
                if (*entry == 0xFF)
                    break;

                p->type3g = *entry;
                p->data_len = (*(entry+1)&0xFF) | (*(entry+2)<<8);
                p->data_type = *(entry+3);
                if (p->data_type == 0xFF)
                    break;

                p->data = new char[p->data_len-1];
                memcpy(p->data, entry+4, p->data_len-1);
                entry += p->data_len + 3;

                length = length - p->data_len - 3;
                pbLen += sizeof(int)*3 + p->data_len;
                /* RilLogV("* type3G:%d len:%d dataType:0x%02x data:%s remain_len:%d",
                    p->type3g, p->data_len, p->data_type, p->data, length); */
                p++;
                cnt++;
            }
            pbLen += sizeof(int)*2;

            OnRequestComplete(uErrCode, &pb, pbLen);
            while (cnt > 0) {
                delete p[cnt].data;
                cnt--;
            }
            break;
        case PB_AAS:
        case PB_GAS:
            pb2 = (RIL_PbEntry2 *) entry_data;
            pb2->text_len = *entry;
            pb2->text_type = *(entry+1);
            pb2->text = new char[pb2->text_len];
            memcpy(pb2->text, entry+2, pb2->text_len);
            pbLen = pb2->text_len + sizeof(int)*4;
            OnRequestComplete(uErrCode, &pb, pbLen);
            delete [](pb2->text);
            break;
        default:
            break;
        }
    }else
        OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoUpdatePbEntry(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    SimUpdatePbEntry *pReq = (SimUpdatePbEntry *)pMsg->GetRequestData();
    ProtocolSimBuilder builder;
    ModemData *pModemData= NULL;

    int mode = pReq->GetMode();
    int type = pReq->GetPbType();
    int index = pReq->GetIndex();

    /* delete */
    if (mode == 2) {
        pModemData = builder.BuildSimUpdatePbDelete(mode, type, index);
        nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_UPDATE_PB_ENTRY_DONE);
        LEAVE_FUNC();
        return (nResult<0)? -1: 0;
    }

    RilLogV("DoUpdatePbEntry, mode:%d, type:%d, index:%d, length:%d", mode, type, index, pReq->GetLength());

    if (!pReq->GetLength()) {
        RilLogW("DoUpdatePbEntry, data length is zero");
        LEAVE_FUNC();
        return -1;
    }

    switch(type) {
    case PB_EN:
    case PB_ADN_2G:
    case PB_FDN:
    case PB_SDN:
    case PB_LDN:
    case PB_MSISDN:
        pModemData = builder.BuildSimUpdatePbEntry1(mode, type, index, pReq->GetLength(), pReq->GetEntryData());
        break;
    case PB_ADN_3G:
        pModemData = builder.BuildSimUpdatePb3gEntry(mode, type, index, pReq->GetLength(), pReq->GetEntryData());
        break;
    case PB_GAS:
    case PB_AAS:
        pModemData = builder.BuildSimUpdatePbEntry2(mode, type, index, pReq->GetLength(), pReq->GetEntryData());
        break;
    default:
        break;
    }

    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_UPDATE_PB_ENTRY_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnUpdatePbEntryDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimUpdatePbEntry adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();

    if (uErrCode == RIL_E_SUCCESS) {
        RIL_UpdatePbRsp resp;
        resp.mode = adapter.GetMode();
        resp.type = adapter.GetPbtype();
        resp.index = adapter.GetIndex();
        OnRequestComplete(RIL_E_SUCCESS, &resp, sizeof(resp));
    } else OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoGetPbStorageInfo(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        LOGE("rildata is null");
        return -1;
    }

    int pb_type = rildata->GetInt();
    RilLogV("%s, phonebook type is %d", __FUNCTION__, pb_type);

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimGetPbStorageInfo(pb_type);
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_PB_STORAGE_INFO_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGetPbStorageInfoDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimPbStorageInfoAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();

    if (uErrCode == RIL_E_SUCCESS) {
        RIL_PbStorageInfo rsp;
        rsp.pb_type = adapter.GetPbType();
        rsp.total_count = adapter.GetTotalCount();
        rsp.used_count = adapter.GetUsedCount();

        RilLogV("OnGetPbStorageInfoDone type:%d total count:%d used count:%d ",
            rsp.pb_type, rsp.total_count, rsp.used_count);

        OnRequestComplete(RIL_E_SUCCESS, &rsp, sizeof(rsp));
    } else
        OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoGetPbStorageList(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimPbStorageList();
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_PB_STORAGE_LIST_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGetPbStorageListDone(Message * pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimPbStorageListAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();
    if (uErrCode == RIL_E_SUCCESS) {
        int pb_list = adapter.GetPbList();
        RilLogV("[%s] %s() - PbList = %d", m_szSvcName, __FUNCTION__, pb_list);
        OnRequestComplete(RIL_E_SUCCESS, &pb_list, sizeof(int));
    } else
        OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoGetPbEntryInfo(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        LOGE("rildata is null");
        return -1;
    }

    int pb_type = rildata->GetInt();
    RilLogV("phonebook type is %d", pb_type);

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimGetPbEntryInfo(pb_type);
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_PB_ENTRY_INFO_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGetPbEntryInfoDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimPbEntryInfoAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();
    if (uErrCode == RIL_E_SUCCESS) {
        RIL_PbEntryInfo rsp;
        rsp.pb_type = adapter.GetPbType();
        rsp.index_min = adapter.GetIndexMin();
        rsp.index_max = adapter.GetIndexMax();
        rsp.num_max = adapter.GetNumMax();
        rsp.text_max = adapter.GetTextMax();

        RilLogV("OnGetPbEntryInfoDone type:%d index:%d~%d numsize:%d textsize:%d",
            rsp.pb_type, rsp.index_min, rsp.index_max, rsp.num_max, rsp.text_max);

        OnRequestComplete(RIL_E_SUCCESS, &rsp, sizeof(rsp));
    } else
        OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::DoGet3GPbCapa(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSim3GPbCapa();
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_3G_PB_CAPA_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGet3GPbCapaDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimPbCapaAdapter adapter(pModemData);
    SimDataBuilder builder;

    UINT uErrCode = adapter.GetErrorCode();

    if (uErrCode == RIL_E_SUCCESS) {
        int entry_num = adapter.GetEntryNum();
        int pb_capa[MAX_PB_ENTRY_NUM*4] = {0};

        RilLogV("OnGet3GPbCapaDone EntryNum:%d", entry_num);
        adapter.GetPbCapa(pb_capa, entry_num);

        int j=0;
        for (int i=0; i < entry_num; i++) {
                RilLogV("No%d. type:%d index:%d entry:%d usedCnt:%d",
                i, pb_capa[j], pb_capa[j+1], pb_capa[j+2], pb_capa[j+3]);
                j +=4;
        }

        const RilData *pRilData = builder.BuildGet3GPbCapaResponse(entry_num, pb_capa);
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
    } else
        OnRequestComplete(uErrCode);

    LEAVE_FUNC();
    return 0;
}

int SimService::OnPbReady(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimPbReadyAdapter adapter(pModemData);
    int pb_ready = adapter.GetPbReady();
    RilLogV("PB Ready indication %d", pb_ready);

#ifdef RIL_SUPPORT_SIMPB_GENERIC_IPC
    OnUnsolicitedResponse(RIL_UNSOL_PB_READY, &pb_ready, sizeof(int));
#else
#endif // RIL_SUPPORT_SIMPB_GENERIC_IPC
    LEAVE_FUNC();
    return 0;
}

void SimService::NotifyIccidInfo()
{
    if(m_pIccidInfo != NULL)
    {
        // Boot up
        if(GetRadioState() != RADIO_STATE_UNAVAILABLE)
        {
            OnUnsolicitedResponse(RIL_UNSOL_ICCID_INFO, m_pIccidInfo->GetData(), m_pIccidInfo->GetDataLength());
            delete m_pIccidInfo;
            m_pIccidInfo = NULL;
        }
    }
}

int SimService::OnIccidInfo(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolSimIccidInfoAdapter adapter(pModemData);

    int iccIdLength = adapter.GetIccIdLen();
    const BYTE *iccId = adapter.GetIccId();
    string orderedIccId = bchToString(iccId,iccIdLength);
    RilProperty *pProperty = GetRilContextProperty();
    if(pProperty != NULL) pProperty->Put(RIL_CONTEXT_SIM_ICC_ID,orderedIccId.c_str());

    SimDataBuilder builder;
    const RilData *pRilData = builder.BuildIccidInfoIndicate(adapter.GetIccIdLen(), adapter.GetIccId());
    if(m_pIccidInfo!=NULL) {
        delete m_pIccidInfo;
        m_pIccidInfo = NULL;
    }
    m_pIccidInfo = (RilData *) pRilData;
    NotifyIccidInfo();

    LEAVE_FUNC();
    return 0;
}

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
int SimService::CheckBootReason()
{
    ENTER_FUNC();
    int state = BOOT_STATE_UNKNOWN;
    string strReason = SystemProperty::Get(SYS_BOOT_REASON, "");

    if (TextUtils::IsEmpty(strReason)) {
        state = BOOT_STATE_UNKNOWN;
    } else if (strReason.find("kernel_panic") != string::npos) {
        state = BOOT_STATE_KERNEL_PANIC;
    } else {
        state = BOOT_STATE_NORMAL;
    }

    LOGV("state : %d", state);
    LEAVE_FUNC();
    return state;
}

char* SimService::checkMmcMnc(const char* pImsi) {
    MccMncChanger changer;
    char *pMccMnc = new char[6];
    char *pCopiedImsi = new char[MAX_IMSI_LEN + 1];
    strncpy(pCopiedImsi, pImsi, MAX_IMSI_LEN + 1);
    string pszImsi = pImsi;
    bool success = changer.checkMccMncToChange((pszImsi.substr(0, 6)).c_str(), pMccMnc);
    if (true == success && 0 != strcmp((pszImsi.substr(0, 6)).c_str(), pMccMnc))
    {
        strncpy(pCopiedImsi, pMccMnc, 6);
        RilLogV("[%s] %s(): checked IMSI: [%s]", m_szSvcName, __FUNCTION__, pCopiedImsi);
    }
    else
    {
        RilLogV("[%s] %s(): checked IMSI doesn't need to be changed", m_szSvcName, __FUNCTION__);
    }
    free(pMccMnc);
    return pCopiedImsi;
}

void SimService::SetAutoVerifyPin(const char *pszPin)
{
    ENTER_FUNC();

    ClearAutoVerifyPin();
    if(pszPin!=NULL && strlen(pszPin)>0)
    {
        memset(m_szTempPIN, 0, sizeof(char) * 128);
        EncryptAutoVerifyPin(pszPin);

        RilLogV("[%s] %s(): Auto Verify Temporary PIN[%s]", m_szSvcName, __FUNCTION__, m_szTempPIN);
        m_nAutoPinState = AUTO_PIN_STATE_RECOVERED;
    }

    LEAVE_FUNC();
}

void SimService::SetAutoVerifyPinDone(BOOL bSuccess)
{
    ENTER_FUNC();

    if(bSuccess) {
        if(strlen(m_szTempPIN)>0) {
            memset(m_szAutoPIN, 0, sizeof(m_szAutoPIN));
            strncpy(m_szAutoPIN, m_szTempPIN, 128);
            memset(m_szTempPIN, 0, sizeof(m_szTempPIN));
            RilLogV("[%s] %s(): Auto Verify PIN[%s]", m_szSvcName, __FUNCTION__, m_szAutoPIN);

            // for kernel panic
            SystemProperty::Set(m_szAutoPinPropertyKeyByPanic, m_szAutoPIN);
        }
        else RilLogW("[%s] %s(): Auto Verify Temporary PIN is empty", m_szSvcName, __FUNCTION__);
    }

    memset(m_szTempPIN, 0, sizeof(char) * 128);

    LEAVE_FUNC();
}

void SimService::GetAutoVerifyPin(char *pszPin)
{
    ENTER_FUNC();

    if(pszPin!=NULL)
    {
        DecryptAutoVerifyPin(pszPin);
    }

    LEAVE_FUNC();
}

void SimService::ClearAutoVerifyPin()
{
    ENTER_FUNC();

    m_nAutoPinState = AUTO_PIN_STATE_EMPTY;
    memset(m_szAutoPIN, 0, sizeof(char) * 128);
    memset(m_szTempPIN, 0, sizeof(char) * 128);

    LEAVE_FUNC();
}

void SimService::SaveAutoVerifyPin()
{
    ENTER_FUNC();

    if(strlen(m_szAutoPIN)>0)
    {
        SystemProperty::Set(m_szAutoPinPropertyKey, m_szAutoPIN);
        m_nAutoPinState = AUTO_PIN_STATE_READY;
    }

    LEAVE_FUNC();
}

void SimService::LoadAutoVerifyPin()
{
    ENTER_FUNC();

    m_nAutoPinState = AUTO_PIN_STATE_INIT;
    string strPin = SystemProperty::Get(m_szAutoPinPropertyKey, "");

    if(strPin.length())
    {
        strcpy(m_szAutoPIN, strPin.c_str());
        RilLogV("[%s] %s(): Auto Verify PIN[%s]", m_szSvcName, __FUNCTION__, m_szAutoPIN);
        m_nAutoPinState = AUTO_PIN_STATE_READY;
    }
    else m_nAutoPinState = AUTO_PIN_STATE_EMPTY;

    LEAVE_FUNC();
}

void SimService::EmptyAutoVerifyPin()
{
    ENTER_FUNC();

    SystemProperty::Set(m_szAutoPinPropertyKey, "");

    LEAVE_FUNC();
}

void SimService::EncryptAutoVerifyPin(const char *pszPin)
{
    ENTER_FUNC();

    // Zigzag Scan from H.264
    int nScramble[MAX_SIM_PIN_LEN*2] = {  1,  2,  5,  9,
                                          6,  3,  4,  7,
                                         10, 13, 14, 11,
                                          8, 12, 15, 16 };

    RilLogI("[%s] %s(): ----- Encryption -----", m_szSvcName, __FUNCTION__);

    if(pszPin!=NULL)
    {
        char szCode[MAX_SIM_PIN_LEN];
        char szHexStr[128];
        char szBuffer[128];
        memset(szHexStr, 0, sizeof(char)*128);
        memset(szBuffer, 0, sizeof(char)*128);

        RilLogV("[%s] %s(): PIN-[%s]", m_szSvcName, __FUNCTION__, pszPin);

        // Fill space character
        memset(szCode, 0x20, sizeof(szCode));
        memcpy(szCode, pszPin, strlen(pszPin));
        int nHexLen = Value2HexString(szHexStr, (BYTE *) szCode, sizeof(szCode));
        RilLogV("[%s] %s(): HexString-[%s]", m_szSvcName, __FUNCTION__, szHexStr);

        // Scrambling
        for(int i=0; i<MAX_SIM_PIN_LEN*2; i++) szBuffer[i] = szHexStr[nScramble[i]-1];
        RilLogV("[%s] %s(): Scrambled-[%s]", m_szSvcName, __FUNCTION__, szBuffer);

        // GSM 7bit Coding
        unsigned char curr, next;
        for(int nIdx=0, i=0; i<nHexLen; i++, nIdx++)
        {
            curr = (unsigned char) szBuffer[i];
            next = (unsigned char) (((i+1)<nHexLen)? szBuffer[i+1]: 0);

            szHexStr[nIdx] = (unsigned char) (((curr >> (nIdx==0? 0: nIdx%7)) & (0x00FF >> ((nIdx==0? 0: nIdx%7) + 1)))
                                | (((next << 8) & 0xFF00) >> (((nIdx==0? 0: nIdx%7) + 1))));
            if((nIdx+1)%7==0) i++;
        }

        // XOR Key
        for(int i=0; i<nHexLen; i++) szHexStr[i] ^= m_szAutoPinPropertyKey[i%strlen(m_szAutoPinPropertyKey)];
        RilLogV("[%s] %s(): XorKey-[%s]", m_szSvcName, __FUNCTION__, szHexStr);

        // To Hex String
        memset(m_szTempPIN, 0, sizeof(char)*128);
        nHexLen = Value2HexString(m_szTempPIN, (BYTE *) szHexStr, nHexLen-(nHexLen/8));
        RilLogV("[%s] %s(): Encrypted-[%s]", m_szSvcName, __FUNCTION__, m_szTempPIN);
    }

    LEAVE_FUNC();
}

void SimService::DecryptAutoVerifyPin(char *pszPin)
{
    ENTER_FUNC();

    // Zigzag Scan from H.264
    int nScramble[MAX_SIM_PIN_LEN*2] = {  1,  2,  5,  9,
                                          6,  3,  4,  7,
                                         10, 13, 14, 11,
                                          8, 12, 15, 16 };


        RilLogI("[%s] %s(): ----- Decryption -----", m_szSvcName, __FUNCTION__);

        if(pszPin!=NULL)
        {
            RilLogV("[%s] %s(): Encrypted-[%s]", m_szSvcName, __FUNCTION__, m_szAutoPIN);

            char szHexDec[128];
            char szBuffer[128];
            memset(szHexDec, 0, sizeof(char)*128);
            memset(szBuffer, 0, sizeof(char)*128);
            int nValLen = HexString2Value((BYTE *) szHexDec, (const char *) m_szAutoPIN);

            // XOR Key
            for(int i=0; i<nValLen; i++) szHexDec[i] ^= m_szAutoPinPropertyKey[i%strlen(m_szAutoPinPropertyKey)];

            // 8bit Character Coding
            unsigned char by = 0;
            for(int nIdx=0, i=0; i<nValLen; i++, nIdx++)
            {
                unsigned char curr = szHexDec[i];
                szBuffer[nIdx] = (unsigned char) 0x00 | ((curr & (0x00FF >> (((i>0)? i%7:0)+1))) << ((i>0)? (i%7):0)) | by;
                by = (unsigned char) 0x00 | ((curr & (0xFF00 >> (((i>0)? i%7:0)+1))) >> (7-(((i>0)? (i%7):0))));

                if((i+1)%7==0)
                {
                    szBuffer[++nIdx] = (unsigned char) 0x00 | by;
                    by = 0;
                }
            }

            RilLogV("[%s] %s(): Scrambled-[%s]", m_szSvcName, __FUNCTION__, szBuffer);

            // Unscrambling
            memset(szHexDec, 0, 128);
            for(int i=0; i<MAX_SIM_PIN_LEN*2; i++) szHexDec[nScramble[i]-1] = szBuffer[i];
            RilLogV("[%s] %s(): Unscrambled-[%s]", m_szSvcName, __FUNCTION__, szHexDec);

            char szCode[MAX_SIM_PIN_LEN];

            // Fill space character
            memset(szCode, 0x20, sizeof(szCode));
            nValLen = HexString2Value((BYTE *) szCode, (const char *) szHexDec);

            // Remove Space Character
            if (nValLen > 0) {
                for(; nValLen>0 && szCode[nValLen-1]==0x20; nValLen--) szCode[nValLen-1] = '\0';
                strncpy(pszPin, szCode, nValLen);
            }
            RilLogV("[%s] %s(): PIN-[%s]", m_szSvcName, __FUNCTION__, pszPin);
        }

    LEAVE_FUNC();
}
#endif

string SimService::FetchSimOperator(IccIoResult &iccioResult)
{
    string carrier = "";

    // fetch mcc length from EF_AD
    // use low 4bit of 4th byte in EF_AD data
    // 1.mncLength is 2 or invalid, try to look up
    //   1-1. MCCMNC_CODES_HAVING_3DIGITS_MNC
    //   1-2. MCC based smallest mnc length
    // 2.mncLength is more than 2 (normally 3), copy mcc 3 digit and mnc 3 digit.
    if (iccioResult.IsSuccess()) {
        int mncLength = -1;
        if (iccioResult.GetDataLength() > 3 && iccioResult.GetData() != NULL) {
            mncLength = *(iccioResult.GetData() + 3) & 0xF;
        }

        if ((mncLength < 0 || mncLength == 0xF || mncLength == 2) && mImsi.length() >= 6) {
            if (MccTable::GetSmallestDigitsMccForImsi(mImsi) > 0) {
                mncLength = MccTable::GetSmallestDigitsMccForImsi(mImsi);
            }

            if (mncLength < 0 || mncLength == 0xF) {
                mncLength = MccTable::GetSmallestDigitsMccForMcc(mImsi.substr(0, 3));
            }
        }

        string mccmnccode = mImsi.substr(0, 6);
        if (mncLength >= 2) {
            carrier = mccmnccode.substr(0, 3 + mncLength);
            RilLogV("[%s] mncLength=%d carrier=%s", GetServiceName(), mncLength, carrier.c_str());
        } else {
            RilLogW("[%s] EF_AD corrupted or invalid mnc length=%d. IMSI=%sXXXXXXXXX", GetServiceName(), mncLength, mccmnccode.c_str());
        }
    }

    return carrier;
}

void SimService::UpdateImsi(const char *imsi)
{
    UpdateImsi(NULL, imsi);
}

void SimService::UpdateImsi(const char *aid, const char *imsi)
{
    // Update and broadcast an IMSI of (U)SIM subscription.
    // An IMSI of CSIM subscription is not handled.
    bool isGsmUmtsSubscription = false;
    bool isCdmaSubscription = false;
    int index = mRilCardStatus.gsm_umts_subscription_app_index;
    char imsiforVapp[40] = {0,};
    if (index >= 0 && index < RIL_CARD_MAX_APPS) {
        RIL_AppStatus &gsmUmtsSubscription = mRilCardStatus.applications[index];
        if (aid != NULL && TextUtils::Equals(aid, gsmUmtsSubscription.aid_ptr)) {
            isGsmUmtsSubscription = true;
        }
    }

    // An IMSI of CDMA subscription
    if (!isGsmUmtsSubscription) {
        index = mRilCardStatus.cdma_subscription_app_index;
        if (index >= 0 && index < RIL_CARD_MAX_APPS) {
            RIL_AppStatus &cdmaSubscription = mRilCardStatus.applications[index];
            if (aid != NULL && TextUtils::Equals(aid, cdmaSubscription.aid_ptr)) {
                isCdmaSubscription = true;
            }
        }
    }

    // store IMSI in RilContext Property
    RilProperty *property = GetRilContextProperty();
    if (property != NULL) {
        if (isGsmUmtsSubscription) {
            // legacy
            property->Put(RIL_CONTEXT_SIM_IMSI, imsi);
            // IMSI and AID for USIM/SIM
            property->Put(RIL_CONTEXT_GSM_SIM_AID, aid);
            property->Put(RIL_CONTEXT_GSM_SIM_IMSI, imsi);
            sprintf(imsiforVapp, "vendor.ril.%s_%d", RIL_CONTEXT_GSM_SIM_IMSI, GetRilSocketId());
            SystemProperty::Set(imsiforVapp, imsi);
        }

        if (isCdmaSubscription) {
            // IMSI and AID for CSIM/RUIM
            property->Put(RIL_CONTEXT_CDMA_SIM_AID, aid);
            property->Put(RIL_CONTEXT_CDMA_SIM_IMSI, imsi);
            sprintf(imsiforVapp, "vendor.ril.%s_%d", RIL_CONTEXT_CDMA_SIM_IMSI, GetRilSocketId());
            SystemProperty::Set(imsiforVapp, imsi);
        }
    }

    if (aid == NULL || isGsmUmtsSubscription) {
        string oldImsi = mImsi;
        mImsi = (imsi != NULL) ? imsi : "";
        string printableImsi = (mImsi.length() >= 6) ? mImsi.substr(0, 6) + "XXXXXXXX" : "";
        RilLogI("[%d]IMSI updated: AID=%s IMSI=%s", GetRilSocketId(), aid, printableImsi.c_str());
    }
    else {
        RilLogV("[%d]%s AID(%s) is not for (U)SIM subscription.", GetRilSocketId(), __FUNCTION__, aid);
    }
}

bool SimService::IsPossibleToPassInRadioUnavailableState(int request_id)
{
    return IsPossibleToPassInRadioOffState(request_id);
}

bool SimService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_REQUEST_GET_SIM_STATUS:
        case RIL_REQUEST_SIM_IO:
        case RIL_REQUEST_GET_IMSI:
        case RIL_REQUEST_ENTER_SIM_PIN:
        case RIL_REQUEST_ENTER_SIM_PIN2:
        case RIL_REQUEST_ENTER_SIM_PUK:
        case RIL_REQUEST_ENTER_SIM_PUK2:
        case RIL_REQUEST_CHANGE_SIM_PIN:
        case RIL_REQUEST_CHANGE_SIM_PIN2:
        case RIL_REQUEST_SET_FACILITY_LOCK:
        case RIL_REQUEST_QUERY_FACILITY_LOCK:
        case RIL_REQUEST_ISIM_AUTHENTICATION:
        case RIL_REQUEST_SIM_OPEN_CHANNEL:
        case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC:
        case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL:
        case RIL_REQUEST_SIM_CLOSE_CHANNEL:
#if (RIL_VERSION >= 10)
        case RIL_REQUEST_SIM_AUTHENTICATION:
#endif
        // Secure Element
        case RIL_REQUEST_SIM_GET_ATR:
        case RIL_REQUEST_OEM_SIM_OPEN_CHANNEL:
        case RIL_REQUEST_OEM_SIM_TRANSMIT_APDU_LOGICAL:
        case RIL_REQUEST_OEM_SIM_TRANSMIT_APDU_BASIC:
        case RIL_REQUEST_OEM_SIM_PRESENT:
        // SIM lock status
        case RIL_REQUEST_GET_SIM_LOCK_STATUS:
        // Radio Config
        case RIL_REQUEST_GET_SLOT_STATUS:
        case RIL_REQUEST_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING:
        // OEM-IMS
        case RIL_REQUEST_OEM_IMS_SIM_IO:
        case RIL_REQUEST_OEM_ICC_DEPERSONALIZATION:
            break;
        default:
            return false;
    }
    return true;

}

void SimService::SendOpenCarrierInfoRilReq(const char *plmn)
{
    char info[2][15];
    char **ptrStrings;
    unsigned int len, openCarrierIndex;

    ptrStrings = (char **)new char*[2];
    memset(ptrStrings, 0, 2*sizeof(char *));

    memset(info, 0, sizeof(info));
    openCarrierIndex = GetOpenCarrierIndex();
    len = snprintf(info[0], 15, "%d", openCarrierIndex);
    memcpy(info[1], plmn, strlen(plmn));

    ptrStrings[0] = info[0];
    ptrStrings[1] = info[1];

    RequestData *pData = RilParser::CreateStrings(RIL_REQUEST_SET_OPEN_CARRIER_INFO, 0, (char*)ptrStrings, 2*sizeof(char *));
    //StringsRequestData* test = (StringsRequestData*)pData;

    if(pData != NULL) {
        Message *pMsg = Message::ObtainMessage(pData,  RIL_SERVICE_MISC, MSG_MISC_SET_OPEN_CARRIER_INFO);
        ServiceMgr *pServigMrg = GetRilContext()->GetServiceManager();
        if (pServigMrg != NULL) {
            if (pServigMrg->SendMessage(pMsg) < 0) {
                RilLogV("%s(): SendMessage error", __FUNCTION__);
                delete pMsg;
            }
        }
        else if(pMsg != NULL) {
            delete pMsg;
        }
    }
    delete[] ptrStrings;
}

int SimService::DoSetCarrierRestrictions(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    //CarrierRestrictionsData *rildata = (CarrierRestrictionsData *)pMsg->GetRequestData();

#if ENABLE_NEW_RIL_COMMAND
    int lenAllowedCarriers = rildata->GetLenAllowCarrier();
    int lenExcludedCcarriers = rildata->GetLenExcludeCarrier();
    CarrierInfo *pAllowedCarriers = rildata->GetAllowCarrier();
    CarrierInfo *pExcludedCarriers = rildata->GetExcludeCarrier();
    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSetCarrierRestrictions(lenAllowedCarriers,
                                    lenExcludedCcarriers,
                                    pAllowedCarriers,
                                    pExcludedCarriers);
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_SET_CARRIER_RESTRICTIONS_DONE);
#else
    OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
#endif

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnSetCarrierRestrictionsDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimSetCarrierRestrictionsAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();
    if (uErrCode == RIL_E_SUCCESS) {
        int numAllowedCarriers = adapter.GetLenAllowedCarriers();
        OnRequestComplete(RIL_E_SUCCESS, &numAllowedCarriers, sizeof(numAllowedCarriers));
    } else {
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    }

    LEAVE_FUNC();
    return 0;
}

int SimService::DoGetCarrierRestrictions(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildGetCarrierRestrictions();
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_CARRIER_RESTRICTIONS_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGetCarrierRestrictionsDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimGetCarrierRestrictionsAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();
    if (uErrCode == RIL_E_SUCCESS) {

        int lenAllowed = adapter.GetLenAllowedCarriers();
        int lenExcluded = adapter.GetLenExcludedCarriers();

        RIL_Carrier *pAllowed = NULL;
        RIL_Carrier *pExcluded = NULL;
        if (lenAllowed > 0) {
            pAllowed = (RIL_Carrier *)calloc(lenAllowed, sizeof(RIL_Carrier));
            if (pAllowed == NULL) {
                LOGE("setAllowedCarriers: Memory allocation failed for request");
                lenAllowed = 0;
            }

            lenAllowed = adapter.GetAllowedCarriers(pAllowed, lenAllowed);
        }

        if (lenExcluded > 0) {
            pExcluded = (RIL_Carrier *)calloc(lenExcluded, sizeof(RIL_Carrier));
            if (pExcluded == NULL) {
                LOGE("setExcludedCarriers: Memory allocation failed for request");
                lenExcluded = 0;
            }
            lenExcluded = adapter.GetExcludedCarriers(pExcluded, lenExcluded);
        }

        SimDataBuilder builder;
        const RilData *rildata = builder.BuildGetCarrierRestrictionsResponse(lenAllowed, lenExcluded,
                                                pAllowed, pExcluded);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        } else {
            OnRequestComplete(RIL_E_INTERNAL_ERR);
        }

        if (pAllowed != NULL) free(pAllowed);
        if (pExcluded != NULL) free(pExcluded);

    } else {
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    }

    LEAVE_FUNC();
    return 0;
}

int SimService::DoSetSimCardPower(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();

    int isPowerUp = rildata->GetInt();
    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSetSimCardPower(isPowerUp);
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_SET_SIM_CARD_POWER_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnSetSimCardPowerDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimResponseAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();
    if (uErrCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, NULL, 0);
    } else {
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    }
    LEAVE_FUNC();
    return 0;
}

int SimService::OnUnsolUiccSubscriptionStatusChanged(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolUiccSubStatusChangeAdapter adapter(pModemData);
    int state = adapter.GetState();
    OnUnsolicitedResponse(RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED, &state, sizeof(state));

    LEAVE_FUNC();
    return 0;
}

// IMS
int SimService::DoOemImsSimIo(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildOemSimRequest(RIL_REQUEST_SIM_IO, (BYTE *) rildata->GetRawData(), rildata->GetSize());
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_OEM_IMS_SIM_IO_DONE);

    LEAVE_FUNC();
    return nResult;
}

int SimService::OnOemImsSimIoDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    OnRequestComplete(uErrCode, (void *) adapter.GetParameter(), adapter.GetParameterLength());

    LEAVE_FUNC();
    return 0;
}

// Secure Element
int SimService::DoOemOpenChannel(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildOemSimRequest(RIL_REQUEST_SIM_OPEN_CHANNEL, (BYTE *) rildata->GetRawData(), rildata->GetSize());
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_OEM_OPEN_CHANNEL_DONE);

    LEAVE_FUNC();
    return 0;
}

int SimService::OnOemOpenChannelDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    OnRequestComplete(uErrCode, (void *) adapter.GetParameter(), adapter.GetParameterLength());

    LEAVE_FUNC();
    return 0;
}

int SimService::DoOemTransmitApduLogical(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildOemSimRequest(RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL, (BYTE *) rildata->GetRawData(), rildata->GetSize());
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_OEM_TRANSMIT_APDU_LOGICAL_DONE);

    LEAVE_FUNC();
    return 0;
}

int SimService::OnOemTransmitApduLogicalDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();

    if(uErrCode==RIL_E_SUCCESS)
    {
        ProtocolSimTransmitApduChannelAdapter apduAdapter(pMsg->GetModemData());

        int nSW1 = apduAdapter.GetSw1();
        int nSW2 = apduAdapter.GetSw2();

        // ETSI TS 102.201, 7.2.2.3.1
        // SW1 : '6C' : The terminal shall wait for a second procedure byte then immediately
        // repeat the previous command header to the UICC using a length of
        // 'XX', where 'XX' is the value of the second procedure byte (SW2).
        if (nSW1==0x6C) {
            LOGV("SW1:0x%02X, Retransmission with P3(%d:0x%X)", nSW1, nSW2, nSW2);

            Message *pCurrMsg = GetCurrentMsg();
            if (pCurrMsg != NULL) {
                RawRequestData *rildata = (RawRequestData *)pCurrMsg->GetRequestData();
                if (rildata != NULL) {
                    char *pRawData = (char *)rildata->GetRawData();
                    memcpy(&pRawData[20/*p3*/], &nSW2, sizeof(int));
                    if(DoOemTransmitApduLogical(pCurrMsg)==-1) OnRequestComplete(RIL_E_GENERIC_FAILURE);
                }
            }
        } else {   // Not 0x6C
            OnRequestComplete(RIL_E_SUCCESS, (void *) adapter.GetParameter(), adapter.GetParameterLength());
        }
    }
    else
    {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    LEAVE_FUNC();
    return 0;
}

int SimService::DoOemTransmitApduBasic(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildOemSimRequest(RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC, (BYTE *) rildata->GetRawData(), rildata->GetSize());
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_OEM_TRNASMIT_APDU_BASIC_DONE);

    LEAVE_FUNC();
    return 0;
}

int SimService::OnOemTransmitApduBasicDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS, (void *) adapter.GetParameter(), adapter.GetParameterLength());
    }
    else
    {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    LEAVE_FUNC();
    return 0;
}

int SimService::DoOemGetSimCardPresent(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    if(mRilCardStatus.card_state!=RIL_CARDSTATE_PRESENT) {
        ProtocolSimBuilder builder;
        ModemData *pModemData = builder.BuildSimGetStatus();
        int nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_OEM_GET_CARD_PRESENT_DONE);
        LEAVE_FUNC();
        return (nResult<0)? -1: 0;
    }

    int nCardState = RIL_CARDSTATE_PRESENT;
    RilDataInts *pRilData = new RilDataInts(1);
    if(pRilData != NULL)
    {
        pRilData->SetInt(0, nCardState);
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
        pRilData = NULL;
    }
    else
    {
        LOGE("Error: pRilData = NULL");
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    LEAVE_FUNC();
    return 0;
}

int SimService::OnOemGetCardPresentDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        ProtocolSimStatusAdapter adapter(pMsg->GetModemData());
        RIL_CardState eCardState = (RIL_CardState) adapter.GetCardState();

        int nResult = RIL_CARDSTATE_PRESENT;
        if (eCardState != RIL_CARDSTATE_PRESENT) {
            nResult = RIL_CARDSTATE_ABSENT;
        }

        RilDataInts *pRilData = new RilDataInts(1);
        if(pRilData != NULL)
        {
            pRilData->SetInt(0, nResult);
            OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
            delete pRilData;
        }
        else
        {
            LOGE("Error: pRilData = NULL");
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }
    }
    else
    {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    LEAVE_FUNC();
    return 0;
}

int SimService::DoGetSimLockInfo(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildGetSimLockInfo();
    if (SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_SIM_LOCK_INFO_DONE) < 0) {
        return -1;
    }

    return 0;
}

int SimService::OnGetSimLockInfoDone(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolSimLockInfoAdapter adapter(pModemData);
    int policy = adapter.GetPolicy();
    int status = adapter.GetStatus();
    int lockType = adapter.GetLockType();
    int maxRetryCount = adapter.GetMaxRetryCount();
    int remainCount = adapter.GetRemainCount();
    int lockCodeCount = adapter.GetLockCodeCount();
    const char *lockCode = adapter.GetLockCode();

    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        SimDataBuilder builder;
        const RilData *rildata = builder.BuildGetSimLockInfoResponse(policy, status, lockType,
                maxRetryCount, remainCount, lockCodeCount, lockCode);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else {
        OnRequestComplete(RIL_E_INTERNAL_ERR);
    }

    return 0;
}

// Radio Config
int SimService::DoGetSlotStatus(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimGetSlotStatus();
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_GET_SLOT_STATUS_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnGetSlotStatusDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolSimSlotStatusAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();

    if(uErrCode==RIL_E_SUCCESS)
    {
        mSimSlotStatusResultV1_2.num_slots = adapter.GetNumOfSlotStatus();
        LOGV("[OnGetSlotStatusDoneV1_2] status size: %d", mSimSlotStatusResultV1_2.num_slots);
        for (int i=0; i<mSimSlotStatusResultV1_2.num_slots; i++)
        {
            memset(&(mSimSlotStatusResultV1_2.mSimSlotStatus[i]), 0, sizeof(RIL_SimSlotStatus_1_2));
            memset(&(m_aszAtr[i]), 0, sizeof(char)*(MAX_ATR_LEN_FOR_SLOT_STATUS * 2 + 1));
            memset(&(m_aszIccid[i]), 0, sizeof(char)*(MAX_ICCID_LEN * 2 + 1));
            memset(&(m_aszEid[i]), 0, sizeof(char)*(MAX_EID_LEN * 2 + 1));
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].cardState = (RIL_CardState) adapter.GetCardState(i);
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].slotState = (RIL_SlotState) adapter.GetSlotState(i);
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].logicalSlotId = adapter.GetLogicalSlotId(i);
            if (mSimSlotStatusResultV1_2.mSimSlotStatus[i].cardState==(RIL_CardState) RIL_CARDSTATE_PRESENT ||
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].cardState ==(RIL_CardState) RIL_CARDSTATE_RESTRICTED)
            {
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size = adapter.GetAtrSize(i);
                if (mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size > 0)
                {
                    char *pszAdapterATR = adapter.GetAtr(i);
                    if(pszAdapterATR!=NULL)
                    {
                        strncpy(m_aszAtr[i], pszAdapterATR, strlen(pszAdapterATR));
                        delete [] pszAdapterATR;
                        pszAdapterATR = NULL;
                        mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr = m_aszAtr[i];
                    }
                }
                else
                {
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr = NULL;
                    LOGE("[OnGetSlotStatusDoneV1_2] atr parsing failed / atr size: %d", mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size);
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size = 0;
                }
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size = adapter.GetIccIdSize(i);
                if (mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size > 0)
                {
                    string iccId = adapter.GetIccId(i);
                    strncpy(m_aszIccid[i], iccId.c_str(), mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size);
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid = m_aszIccid[i];
                }
                else
                {
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid = NULL;
                    LOGE("[OnGetSlotStatusDoneV1_2] iccid parsing failed / iccid size: %d", mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size);
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size = 0;
                }
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size = adapter.GetEidSize(i);
                if (mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size > 0)
                {
                    char *pszAdapterEid = adapter.GetEid(i);
                    if(pszAdapterEid!=NULL)
                    {
                        strncpy(m_aszEid[i], pszAdapterEid, strlen(pszAdapterEid));
                        delete [] pszAdapterEid;
                        pszAdapterEid = NULL;
                        mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid = m_aszEid[i];
                    }
                }
                else
                {
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid = NULL;
                    LOGE("[OnGetSlotStatusDoneV1_2] eid parsing failed / eid size: %d", mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size);
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size = 0;
                }
            }
            else
            {
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size = 0;
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr = NULL;
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size = 0;
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid = NULL;
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size = 0;
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid = NULL;
            }
            LOGV("[OnGetSlotStatusDoneV1_2] slot[%d], CardState:%d, SlotState:%d, logicalSlotId: %d, atr: %s, iccid: %s, eid: %s", i, mSimSlotStatusResultV1_2.mSimSlotStatus[i].cardState,
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].slotState, mSimSlotStatusResultV1_2.mSimSlotStatus[i].logicalSlotId, mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr,
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid,
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid);
        }
        int nLength = sizeof(RIL_SimSlotStatus_1_2) * mSimSlotStatusResultV1_2.num_slots;
        LOGV("[OnGetSlotStatusDoneV1_2] nLength: %d", nLength);
        OnRequestComplete(RIL_E_SUCCESS, (char *) &mSimSlotStatusResultV1_2, nLength);
    }
    else
    {
        memset(&m_aszAtr, 0, sizeof(char)*(MAX_SLOT_NUM)*(MAX_ATR_LEN_FOR_SLOT_STATUS * 2 + 1));
        memset(&m_aszIccid, 0, sizeof(char)*(MAX_SLOT_NUM)*(MAX_ICCID_LEN * 2 + 1));
        memset(&m_aszEid, 0, sizeof(char)*(MAX_SLOT_NUM)*(MAX_EID_LEN * 2 + 1));

        LOGE("[OnGetSlotStatusDoneV1_2] uErrCode:%d", uErrCode);
        OnRequestComplete(uErrCode, NULL, 0);
    }
    LEAVE_FUNC();
    return 0;
}

int SimService::DoSetSlotMapping(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    IntsRequestData *rildata = (IntsRequestData *) pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    int data_size = rildata->GetSize();
    int *tmpData = new int[data_size];
    for(int i=0; i<data_size; i++) {
        tmpData[i] = rildata->GetInt(i);
        LOGV("[DoSetSlotMapping] data[%d]: %d", i, tmpData[i]);
    }

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimSetLogicalSlotMapping(tmpData, data_size);
    if (tmpData != NULL)
        delete[] tmpData;

    if (pModemData == NULL)
    {
        LOGE("[DoSetSlotMapping] invalid argument");
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS, NULL, 0);
        LEAVE_FUNC();
        return nResult;
    }
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnSetSlotMappingDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolRespAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if (uErrCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else {
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    }

    LEAVE_FUNC();
    return 0;
}

int SimService:: OnUnsolSimSlotsStatusChanged(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSlotStatusChangedAdapter adapter(pMsg->GetModemData());
    mSimSlotStatusResultV1_2.num_slots = adapter.GetNumOfSlotStatus();
    LOGV("[OnUnsolSimSlotsStatusChangedV1_2] status size: %d", mSimSlotStatusResultV1_2.num_slots);

    for (int i=0; i<mSimSlotStatusResultV1_2.num_slots; i++)
    {
        memset(&(mSimSlotStatusResultV1_2.mSimSlotStatus[i]), 0, sizeof(RIL_SimSlotStatus_1_2));
        memset(&(m_aszAtr[i]), 0, sizeof(char)*(MAX_ATR_LEN_FOR_SLOT_STATUS * 2 + 1));
        memset(&(m_aszIccid[i]), 0, sizeof(char)*(MAX_ICCID_LEN * 2 + 1));
        memset(&(m_aszEid[i]), 0, sizeof(char)*(MAX_EID_LEN * 2 + 1));
        mSimSlotStatusResultV1_2.mSimSlotStatus[i].cardState = (RIL_CardState) adapter.GetCardState(i);
        mSimSlotStatusResultV1_2.mSimSlotStatus[i].slotState = (RIL_SlotState) adapter.GetSlotState(i);
        mSimSlotStatusResultV1_2.mSimSlotStatus[i].logicalSlotId = adapter.GetLogicalSlotId(i);
        if (mSimSlotStatusResultV1_2.mSimSlotStatus[i].cardState==(RIL_CardState) RIL_CARDSTATE_PRESENT ||
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].cardState ==(RIL_CardState) RIL_CARDSTATE_RESTRICTED)
        {
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size = adapter.GetAtrSize(i);
            if (mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size > 0)
            {
                char *pszAdapterATR = adapter.GetAtr(i);
                if(pszAdapterATR!=NULL)
                {
                    strncpy(m_aszAtr[i], pszAdapterATR, strlen(pszAdapterATR));
                    delete [] pszAdapterATR;
                    pszAdapterATR = NULL;
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr = m_aszAtr[i];
                }
            }
            else
            {
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr = NULL;
                LOGE("[OnGetSlotStatusDoneV1_2] atr parsing failed / atr size: %d", mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size);
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size = 0;
            }
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size = adapter.GetIccIdSize(i);
            if (mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size > 0)
            {
                string iccId = adapter.GetIccId(i);
                strncpy(m_aszIccid[i], iccId.c_str(), mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size);
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid = m_aszIccid[i];
            }
            else
            {
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid = NULL;
                LOGE("[OnGetSlotStatusDoneV1_2] iccid parsing failed / iccid size: %d", mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size);
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size = 0;
            }
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size = adapter.GetEidSize(i);
            if (mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size > 0)
            {
                char *pszAdapterEid = adapter.GetEid(i);
                if(pszAdapterEid!=NULL)
                {
                    strncpy(m_aszEid[i], pszAdapterEid, strlen(pszAdapterEid));
                    delete [] pszAdapterEid;
                    pszAdapterEid = NULL;
                    mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid = m_aszEid[i];
                }
            }
            else
            {
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid = NULL;
                LOGE("[OnGetSlotStatusDoneV1_2] eid parsing failed / eid size: %d", mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size);
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size = 0;
            }
        }
        else
        {
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr_size = 0;
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr = NULL;
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid_size = 0;
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid = NULL;
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid_size = 0;
            mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid = NULL;
        }
        LOGV("[OnUnsolSimSlotsStatusChangedV1_2] slot[%d], CardState:%d, SlotState:%d, logicalSlotId: %d, atr: %s, iccid: %s, eid: %s", i, mSimSlotStatusResultV1_2.mSimSlotStatus[i].cardState,
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].slotState, mSimSlotStatusResultV1_2.mSimSlotStatus[i].logicalSlotId, mSimSlotStatusResultV1_2.mSimSlotStatus[i].atr,
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].iccid,
                mSimSlotStatusResultV1_2.mSimSlotStatus[i].eid);
    }

    int nLength = sizeof(RIL_SimSlotStatus_1_2) * mSimSlotStatusResultV1_2.num_slots;
    LOGV("[OnUnsolSimSlotsStatusChangedV1_2] nLength: %d", nLength);
    OnUnsolicitedResponse(RIL_UNSOL_ICC_SLOT_STATUS, (char *) &mSimSlotStatusResultV1_2, nLength); /* supports RadioConfig ver 1.2 */
    LEAVE_FUNC();
    return 0;
}

int SimService::DoOemIccDepersonalization(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    char *data = (char *)rildata->GetRawData();
    unsigned int datalen = rildata->GetSize();
    if (data == NULL || datalen < sizeof(int) * 2) {
        return -1;
    }

    int nFac = *((int *)data);
    unsigned int size = *((int *)data + 1);
    if (size + sizeof(int) * 2 > datalen) {
        return -1;
    }

    char *pszCode = NULL;
    if (size > 0) {
        pszCode = new char[size + 1];
        memcpy(pszCode, (char *)data + (sizeof(int) * 2), size);
        pszCode[size] = '\0';
    }
    RilLog("[%d] %s nFac=%d pszCode length=%d", GetRilSocketId(), __FUNCTION__, nFac, size);

    if (pszCode != NULL && strlen(pszCode) > 0) {
        ProtocolSimBuilder builder;
        ModemData *pModemData = builder.BuildSimVerifyNetworkLock(nFac, pszCode, 0, NULL);
        nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SIM_OEM_ICC_DEPERSONALIZATION_DONE);
    }

    if (pszCode != NULL)
        delete[] pszCode;

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int SimService::OnOemIccDepersonalizationDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    SimDataBuilder builder;

    ProtocolSimVerifyNetLockAdapter adapterSimVerifyNetLock(pMsg->GetModemData());
    UINT uErrCode = adapterSimVerifyNetLock.GetErrorCode();
    const RilData *pRilData = builder.BuildSimNetworkLockResponse(adapterSimVerifyNetLock.GetRemainCount());
    RilLogV("remain count : %d", adapterSimVerifyNetLock.GetRemainCount());
    if ( pRilData == NULL )
    {
        return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    if (uErrCode==RIL_E_SUCCESS)
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
    else
        OnRequestComplete(uErrCode, pRilData->GetData(), pRilData->GetDataLength());

    if (pRilData != NULL)
        delete pRilData;

    LEAVE_FUNC();
    return 0;
}
