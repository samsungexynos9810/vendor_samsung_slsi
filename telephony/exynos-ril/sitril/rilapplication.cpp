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
 * rilappliaction.cpp
 *
 *  Created on: 2014. 11. 15.
 *      Author: sungwoo48.choi
 */

#include "carrierloader.h"
#include "rilapplication.h"
#include "rilapplicationfactory.h"
#include "rilcontextwrapper.h"
#include "rilapptoken.h"
#include "modemstatemonitor.h"
#include "tokengen.h"
#include "operatortable.h"
#include "rilresponselistener.h"
/* RIL vendor external */
#include "rilexternalresponselistener.h"
#include "rillog.h"
#include "rilversioninfo.h"
#include "signal_handler.h"
#include "reset_util.h"
#include "textutils.h"
#include "wlandata.h"
#include <libxml/parser.h>

// Auto Verify PIN
#include "simservice.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

IMPLEMENT_MODULE_TAG(RilApplication, RilApplication)

///////////////////////////////////////////////////////////////////////////////
// static
///////////////////////////////////////////////////////////////////////////////
RilApplication *RilApplication::instance = NULL;

RilApplication *RilApplication::GetInstance()
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    return instance;
}

RilApplicationContext *RilApplication::GetRilApplicationContext()
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    return static_cast<RilApplicationContext *>(instance);
}

RilApplication * RilApplication::CreateInstance(const struct RIL_Env *pRilEnv)
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    if (pRilEnv != NULL) {
        if (instance == NULL) {
            instance = RilApplicationFactory::CreateRilApplication(pRilEnv);

            // returns NULL if initialization is failed.
            if (instance->InitInstance() < 0) {
                delete instance;
                instance = NULL;

                // if RilApplication is NULL, RIL cannot work properly. try to reset RILD
                // to guarantee thread closing while initialization, wait some time before reset
                sleep(2);
                RilReset("RilApplication Init Fail");
            }
        }
    }
    return instance;
}

void RilApplication::RIL_OnRequest(int request, void *data, unsigned int datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
{
    //RilLogV("%s::%s", TAG, __FUNCTION__);
    if (instance != NULL) {
        instance->OnRequest(request, data, datalen, t, socket_id);
    }
}
void RilApplication::RIL_OnRequest(int request, void *data, unsigned int datalen, RIL_Token t)
{
    //RilLogV("%s::%s", TAG, __FUNCTION__);
    if (instance != NULL) {
        instance->OnRequest(request, data, datalen, t);
    }
}
RIL_RadioState RilApplication::RIL_OnRadioStateRequest(RIL_SOCKET_ID socket_id)
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    RIL_RadioState radioState = RADIO_STATE_UNAVAILABLE;
    if (instance != NULL) {
        radioState = instance->OnRadioStateRequest(socket_id);
    }
    return radioState;
}
RIL_RadioState RilApplication::RIL_OnRadioStateRequest()
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    RIL_RadioState radioState = RADIO_STATE_UNAVAILABLE;
    if (instance != NULL) {
        radioState = instance->OnRadioStateRequest();
    }
    return radioState;
}
int RilApplication::RIL_OnSupports(int requestCode)
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    // Not to support
    RilLogW("%s::%s Not support", TAG, __FUNCTION__);
    return 0;
}
void RilApplication::RIL_OnCancel(RIL_Token t)
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    // Not to support
    RilLogW("%s::%s Not support", TAG, __FUNCTION__);
}

const char *RilApplication::RIL_OnGetVersion()
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    // TODO returns Vendor RIL version string
    return RILVersionInfo::getString(true);
}

void RilApplication::RIL_OnTimedCallback(void *param)
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    // Not to support
    RilLogW("%s::%s Not support", TAG, __FUNCTION__);
}

/* RIL vendor external */
void RilApplication::RIL_externalOnRequest(int reqOemId, void *data, unsigned int datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
{
    if (instance != NULL) {
        instance->externalOnRequest(reqOemId, data, datalen, t, socket_id);
    }
}

// Current supported RIL_VERSION
int RilApplication::RIL_VersionCode = RIL_VERSION;
int RilApplication::RIL_HalVersionCode = HAL_VERSION_CODE(1, 4);

///////////////////////////////////////////////////////////////////////////////
// OperatorDbMakeRunnable
///////////////////////////////////////////////////////////////////////////////
OperatorDbMakeRunnable::OperatorDbMakeRunnable(){
}

OperatorDbMakeRunnable::~OperatorDbMakeRunnable() {
}

void OperatorDbMakeRunnable::Run(){
    OperatorNameProvider::MakeInstance();
}

///////////////////////////////////////////////////////////////////////////////
// RilApplication
///////////////////////////////////////////////////////////////////////////////
RilApplication::RilApplication(const struct RIL_Env *pRilEnv/* = NULL*/)
    : m_pRilEnv(pRilEnv)
{
    for (int i = 0; i < SIM_COUNT; i++) {
        m_RilContext[i] = NULL;
    } // end for i ~

    m_pModemStateMonitor = NULL;
    m_pRilRespListener = NULL;
    m_pSignalMonitor = NULL;

    m_pProductFactory = NULL;
    m_pServiceFactory = NULL;

    /* RIL vendor external */
    m_pRilExternalEnv = NULL;
    m_pRilExternalRespListener = NULL;

    m_pOpDbThread = NULL;
    m_pOpDbRunnable = NULL;
}

RilApplication::~RilApplication()
{
    ExitInstance();
}

int RilApplication::InitInstance()
{
    RilLogI("[%s] %s RIL_VERSION %d", TAG, __FUNCTION__, RilApplication::RIL_VersionCode);

    RilLogI("[%s] new  SignalMonitor", __FUNCTION__);
    m_pSignalMonitor = new SignalMonitor(this);
    if ( m_pSignalMonitor != NULL )
    {
        RilLogI("[%s] start SignalMonitor", __FUNCTION__);
        m_pSignalMonitor->Start();
    }

    // Init TokenGen instance
    TokenGen::Init();

    // dereived class MUST register RilContext instance
    if (OnInitialize() < 0) {
        return -1;
    }

    // init operator dB
    m_pOpDbRunnable = new OperatorDbMakeRunnable();
    m_pOpDbThread = new Thread(m_pOpDbRunnable);
    if ( m_pOpDbThread != NULL ) {
        if (m_pOpDbThread->Start() < 0) {
            RilLogE("Fail to start Operator DB creator");
        }
    } else {
        RilLogE("Can't create Operator DB Thread");
    }

    m_pRilRespListener = new RilResponseListener(m_pRilEnv);

    m_pModemStateMonitor = ModemStateMonitor::MakeInstance(this);
    if (m_pModemStateMonitor == NULL || m_pModemStateMonitor->Start() < 0) {
        return -1;
    }

    /* RilApplicationProperty Set*/
    LoadConfigToRilProperty();

    xmlInitParser();

    return 0;
}

// a carrier information of product
int RilApplication::LoadTargetOperator()
{
    CarrierLoader &cl = CarrierLoader::GetInstance();
    int ret = cl.GetTargetOperator();
    if (ret < 0) {
        ret = cl.GetVendorTargetOperator();
    }
    return ret;
}

void RilApplication::LoadConfigToRilProperty()
{
    /* build option : user / eng / ... */
    RilProperty *property = GetProperty();
    char szProp[PROP_VALUE_MAX];
    int nProp = -1;

    if ( property == NULL ) {
        RilLogW("cannnot get RIL application property");
        return;
    }

    memset(szProp, 0x00, sizeof(szProp));
    property_get(RO_BUILD_TYPE, szProp, "eng");

    if (strcmp(szProp, "user") == 0) {
        RilLogI("[%s] user mode", __FUNCTION__);
        property->Put(RIL_APP_USERMODE, true);
    }

    std::string testSgc = SystemProperty::Get(RIL_SGC_TEST_CONFIG, "");
    if (TextUtils::IsDigitsOnly(testSgc)) {
        nProp = std::stoi(testSgc);
        RilLogI("[%s] target operator(sgc test): %d", __FUNCTION__, nProp);
        property->Put(RIL_APP_TARGET_OPER, nProp);
    }

    if (nProp < 0) {
        // check target operator
        nProp = LoadTargetOperator();
        RilLogI("[%s] target operator: %s(%d)", __FUNCTION__, szProp, nProp);
        property->Put(RIL_APP_TARGET_OPER, nProp);
    }

    // multi-sim mode: single or ds
    string config = SystemProperty::Get("persist.radio.multisim.config");
    int phoneCount = 1;  // default as single
    if (TextUtils::Equals(config.c_str(), "dsds") ||
        TextUtils::Equals(config.c_str(), "dsda") ||
        TextUtils::Equals(config.c_str(), "tsts")) {
        phoneCount = 2;
    }

    SystemProperty::Set("persist.vendor.radio.target_oper", nProp);
    m_RilAppProperty.Put(RIL_APP_PHONE_COUNT, phoneCount);
    m_RilAppProperty.Put(RIL_APP_MULTISIM, phoneCount > 1);
}

void RilApplication::ExitInstance()
{
    RilLogI("[%s] %s", TAG, __FUNCTION__);

    // release resources of dereived class
    OnFinalize();

    //if (m_pOemClientReceiver != NULL) {
    //    delete m_pOemClientReceiver;
    //    m_pOemClientReceiver = NULL;
    //}

    if (m_pModemStateMonitor != NULL) {
        delete m_pModemStateMonitor;
        m_pModemStateMonitor = NULL;
    }

    for (int i = 0; i < SIM_COUNT; i++) {
        if (m_RilContext[i] != NULL) {
            delete m_RilContext[i];
            m_RilContext[i] = NULL;
        }
    } // end for i ~

    if (m_pRilRespListener != NULL) {
        delete m_pRilRespListener;
        m_pRilRespListener = NULL;
    }

    if ( m_pSignalMonitor != NULL )
    {
        m_pSignalMonitor->Stop();
        delete m_pSignalMonitor;
        m_pSignalMonitor = NULL;
    }

    /* RIL vendor external */
    if (m_pRilExternalRespListener != NULL) {
        delete m_pRilExternalRespListener;
        m_pRilExternalRespListener = NULL;
    }
}

RilContext *RilApplication::GetDefaultRilContext()
{
    return m_RilContext[RIL_SOCKET_1];
}

RilContext *RilApplication::GetRilContext(RIL_SOCKET_ID socket_id)
{
    //RilLogI("[%s] %s socket_id=%d", TAG, __FUNCTION__, socket_id);
    if (socket_id < SIM_COUNT)
        return m_RilContext[socket_id];
    return NULL;
}

int RilApplication::RegisterRilContext(unsigned int id, RilContext *pRilContext)
{
    if (id < SIM_COUNT) {
        if (m_RilContext[id] == NULL) {
            m_RilContext[id] = pRilContext;

            // init RilContext resources
            if (m_RilContext[id]->OnCreate() == 0) {
                return 0;
            }
        }
    }

    return -1;
}

int RilApplication::OnInitialize()
{
    // Create RilContext
    RilContextParam params[SIM_COUNT] = {
        {RIL_SOCKET_1, "/dev/umts_ipc0", "rmnet", 0, 4, m_pProductFactory, m_pServiceFactory},
#ifdef ANDROID_MULTI_SIM
#if (SIM_COUNT >= 2)
        {RIL_SOCKET_2, "/dev/umts_ipc1", "rmnet", 4, 4, m_pProductFactory, m_pServiceFactory},
#endif
#endif // ANDROID_MULTI_SIM
    };

    for (int i = 0; i < SIM_COUNT; i++) {
        RilContext *pRilContext = new RilContextWrapper(this, &params[i]);
        if (pRilContext == NULL || RegisterRilContext(i, pRilContext) < 0) {
            return -1;
        }
    } // end for i ~

    // virtual sim feature
    property_set(RIL_UIM_REMOTE_MCC, "");
    property_set(RIL_UIM_REMOTE_SLOT, "-1");

#ifdef SUPPORT_NR
    int support = SystemProperty::GetInt(VENDOR_RIL_SUPPORT_NR, -1);
    if (support < 0) {
        SystemProperty::Set(VENDOR_RIL_SUPPORT_NR, "1");
    }
#endif

    // RilContext ready
    StartModemIpc();

    return 0;
}

int RilApplication::OnFinalize()
{
    if (m_pOpDbThread != NULL) {
        m_pOpDbThread->Stop();

        if (m_pOpDbRunnable != NULL) {
            delete m_pOpDbRunnable;
            m_pOpDbRunnable = NULL;
        }

        delete m_pOpDbThread;
        m_pOpDbThread = NULL;
    }

    return 0;
}

void RilApplication::OnModemStateChanged(int state)
{
    RilLogV("[%s] %s state=%d", TAG, __FUNCTION__, state);
    // TODO do something before invoke callback of RilContext

    for (int i = 0; i < SIM_COUNT; i++) {
        if (m_RilContext[i] != NULL) {
            m_RilContext[i]->OnModemStateChanged(state);
        }
    } // end for i ~
}

void RilApplication::StartModemIpc()
{
    // invoke callback of RilContext
    for (int i = 0; i < SIM_COUNT; i++) {
        if (m_RilContext[i] != NULL) {
            if (m_RilContext[i]->OnStart() < 0) {
                ExitInstance();
                RilErrorReset("FAIL_TO_START_MODEMIPC1");
            }
        }
    } // end for i ~
}

void RilApplication::ResetModem(const char *reason) {
    if (m_pModemStateMonitor != NULL) {
        m_pModemStateMonitor->ResetModem(reason);
    }
}

#include "rilcontext.h"
RIL_SOCKET_ID RilApplication::GetProperSocketIdForOem(OEM_CATEGORY OemCategory)
{
    RilLogV("[%s] %s Finding proper SOCKET ID for Category(%d)", TAG, __FUNCTION__, OemCategory);

    switch(OemCategory)
    {
    case OEM_CAT_AUDIO:
        {
            RilContext *pRilContext;
            RIL_SOCKET_ID socket_id_array[RIL_SOCKET_NUM] = {
                RIL_SOCKET_1,
#if (SIM_COUNT >= 2)
                RIL_SOCKET_2,
#if (SIM_COUNT >= 3)
                RIL_SOCKET_3,
#endif
#if (SIM_COUNT >= 4)
                RIL_SOCKET_4
#endif
#endif
            };
            RIL_SOCKET_ID socket_id;

            // If any rilcontext has an active call, return socket_id for that rilcontext <- can be applied only to DSDS
            ///TODO: need to consider DSDA condition
            for ( unsigned int i = 0; i < RIL_SOCKET_NUM; i++)
            {
                socket_id = socket_id_array[i];
                RilLogV("[%s] Find RilContext for socketid(%d)", __FUNCTION__, socket_id);
                pRilContext = GetRilContext(socket_id);
                if ( pRilContext == NULL )
                {
                    break;
                }

                RilProperty* ContextProperty = pRilContext->GetProperty();
                if ( ContextProperty == NULL )
                {
                    break;
                }

                if ( -1 != ContextProperty->GetInt(RIL_CONTEXT_CS_ACTIVE_CID, -1) )
                {
                    RilLogV("[%s] Found proper SOCKET ID for Category(%d) - Result SocketID : %d", __FUNCTION__, OemCategory, socket_id);
                    return socket_id;
                }

                RilLogW("[%s] No Active call in context of sock id(%d)", __FUNCTION__, socket_id);
            }
        }
        break;

    case OEM_CAT_IMS:
        {
            RilProperty *property = GetProperty();
            if (property != NULL) {
                int socketid = property->GetInt(RIL_APP_PS_ACTIVE_SIM, -1);
                if (socketid > -1) {
                    return (RIL_SOCKET_ID)socketid;
                }
            }
            // return default RIL_SOCKET_ID
            return RIL_SOCKET_1;
        }
        break;
    case OEM_CAT_SIM:
        // return default RIL_SOCKET_ID
        return RIL_SOCKET_1;
    default:
        break;
    }

    RilLogW("[%s] Cannot find proper socketID - return default ID", __FUNCTION__);
    return RIL_SOCKET_1;
}

RIL_SOCKET_ID RilApplication::GetProperSocketIdForWLan(int request, void *data, unsigned int datalen)
{
    int socket_id = RIL_SOCKET_1;

    switch(request)
    {
    case RIL_REQUEST_GET_IMSI:
        {
            socket_id = *(int*)data;
            return (RIL_SOCKET_ID)socket_id;
        }

    case RIL_REQUEST_OEM_SIM_AUTHENTICATION:
        {
            auth_request_type *request = (auth_request_type*)data;
            if (request != NULL) {
                socket_id = request->socket_id;
                return (RIL_SOCKET_ID)socket_id;
            }
            break;
        }
    default:
        break;
    }

    RilLogW("[%s] Cannot find proper socketID - return default ID", __FUNCTION__);
    return RIL_SOCKET_1;
}

// @Overriding
void RilApplication::OnRequest(int request, void *data, unsigned int datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
{
    //RilLogI("[%s] %s ", TAG, __FUNCTION__);
    RilContext *pRilContext = GetRilContext(socket_id);
    if (pRilContext != NULL) {
        RilAppToken *token = new RilAppToken(m_pRilRespListener, t);
        pRilContext->OnRequest(request, data, datalen, (RIL_Token)token);
    }
}
void RilApplication::OnRequest(int request, void *data, unsigned int datalen, RIL_Token t)
{
    OnRequest(request, data, datalen, t, RIL_SOCKET_1);
}

RIL_RadioState RilApplication::OnRadioStateRequest(RIL_SOCKET_ID socket_id)
{
    //RilLogI("[%s] %s ", TAG, __FUNCTION__);
    RIL_RadioState radioState = RADIO_STATE_UNAVAILABLE;
    RilContext *pRilContext = GetRilContext(socket_id);
    if (pRilContext != NULL) {
        radioState = pRilContext->OnRadioStateRequest();
    }
    return radioState;
}

RIL_RadioState RilApplication::OnRadioStateRequest()
{
    return OnRadioStateRequest(RIL_SOCKET_1);
}

void RilApplication::OnRequestComplete(RIL_Token t, RIL_Errno e, void *response, unsigned int responselen)
{
    //RilLogI("[%s] %s", TAG, __FUNCTION__);
    if (t) {
        RilAppToken *token = static_cast<RilAppToken *>(t);
        if (token != NULL) {
            token->context->OnRequestComplete(token->t, e, response, responselen);
            delete token;
        }
    }
}
void RilApplication::OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen)
{
    //RilLogI("[%s] %s ", TAG, __FUNCTION__);
    // To the RIL daemon
    if (m_pRilRespListener != NULL) {
        m_pRilRespListener->OnUnsolicitedResponse(unsolResponse, data, datalen);
    }

    /* RIL vendor external */
    if (m_pRilExternalRespListener != NULL) {
        m_pRilExternalRespListener->OnUnsolicitedResponse(unsolResponse, data, datalen);
    }
}
void RilApplication::OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen, RIL_SOCKET_ID socket_id)
{
    //RilLogI("[%s] %s ", TAG, __FUNCTION__);

    // To the RIL daemon
    if (m_pRilRespListener != NULL) {
#ifdef ANDROID_MULTI_SIM
        m_pRilRespListener->OnUnsolicitedResponse(unsolResponse, data, datalen, socket_id);
#else
        m_pRilRespListener->OnUnsolicitedResponse(unsolResponse, data, datalen);
#endif // ANDROID_MULTI_SIM
    }

    /* RIL vendor external */
    if (m_pRilExternalRespListener != NULL) {
        m_pRilExternalRespListener->OnUnsolicitedResponse(unsolResponse, data, datalen, socket_id);
    }
}
// Extended
void RilApplication::OnOemRequest(int request, void *data, unsigned int datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
{
    // Deprecated
}
void RilApplication::OnOemRequest(int request, void *data, unsigned int datalen, RIL_Token t)
{
    // Deprecated
}

RIL_SOCKET_ID RilApplication::GetProperSocketId(int request, void *data, unsigned int datalen)
{
    RIL_SOCKET_ID rilsockid = RIL_SOCKET_1;

    switch (request)
    {
    case RIL_REQUEST_SET_MUTE:
    case RIL_REQUEST_GET_MUTE:
    case RIL_REQUEST_OEM_VOLUME_SET :
    case RIL_REQUEST_OEM_VOLUME_GET :
    case RIL_REQUEST_OEM_AUDIO_PATH_SET:
    case RIL_REQUEST_OEM_AUDIO_PATH_GET:
    case RIL_REQUEST_OEM_MICROPHONE_SET:
    case RIL_REQUEST_OEM_MICROPHONE_GET:
    case RIL_REQUEST_OEM_AUDIO_CLOCK_SET:
    case RIL_REQUEST_OEM_AUDIO_LOOPBACK_SET:
        rilsockid = GetProperSocketIdForOem(OEM_CAT_AUDIO);
        break;
    case RIL_REQUEST_OEM_IMS_SET_CONFIGURATION :
    case RIL_REQUEST_OEM_IMS_GET_CONFIGURATION :
    case RIL_REQUEST_OEM_IMS_SIM_AUTH :
    case RIL_REQUEST_OEM_IMS_SET_EMERGENCY_CALL_STATUS :
    case RIL_REQUEST_OEM_IMS_SET_SRVCC_CALL_LIST :
    case RIL_REQUEST_OEM_IMS_GET_GBA_AUTH :
    case RIL_REQUEST_OEM_GET_IMS_SUPPORT_SERVICE:

    //AIMS
    case RIL_REQUEST_OEM_AIMS_DIAL:
    case RIL_REQUEST_OEM_AIMS_ANSWER:
    case RIL_REQUEST_OEM_AIMS_HANGUP:
    case RIL_REQUEST_OEM_AIMS_DEREGISTRATION:
    case RIL_REQUEST_OEM_AIMS_HIDDEN_MENU:
    case RIL_REQUEST_OEM_AIMS_ADD_PDN_INFO:
    case RIL_REQUEST_OEM_AIMS_CALL_MANAGE:
    case RIL_REQUEST_OEM_AIMS_SEND_DTMF:
    case RIL_REQUEST_OEM_AIMS_SET_FRAME_TIME:
    case RIL_REQUEST_OEM_AIMS_GET_FRAME_TIME:
    case RIL_REQUEST_OEM_AIMS_CALL_MODIFY:
    case RIL_REQUEST_OEM_AIMS_RESPONSE_CALL_MODIFY:
    case RIL_REQUEST_OEM_AIMS_TIME_INFO:
    case RIL_REQUEST_OEM_AIMS_CONF_CALL_ADD_REMOVE_USER:
    case RIL_REQUEST_OEM_AIMS_ENHANCED_CONF_CALL:
    case RIL_REQUEST_OEM_AIMS_GET_CALL_FORWARD_STATUS:
    case RIL_REQUEST_OEM_AIMS_SET_CALL_FORWARD_STATUS:
    case RIL_REQUEST_OEM_AIMS_GET_CALL_WAITING:
    case RIL_REQUEST_OEM_AIMS_SET_CALL_WAITING:
    case RIL_REQUEST_OEM_AIMS_GET_CALL_BARRING:
    case RIL_REQUEST_OEM_AIMS_SET_CALL_BARRING:
    case RIL_REQUEST_OEM_AIMS_SEND_SMS:
    case RIL_REQUEST_OEM_AIMS_SEND_EXPECT_MORE:
    case RIL_REQUEST_OEM_AIMS_SEND_SMS_ACK:
    case RIL_REQUEST_OEM_AIMS_SEND_ACK_INCOMING_SMS:
    case RIL_REQUEST_OEM_AIMS_CHG_BARRING_PWD:
    case RIL_REQUEST_OEM_AIMS_SEND_USSD_INFO:
    case RIL_REQUEST_OEM_AIMS_GET_PRESENTATION_SETTINGS:
    case RIL_REQUEST_OEM_AIMS_SET_PRESENTATION_SETTINGS:
    case RIL_REQUEST_OEM_AIMS_SET_SELF_CAPABILITY:
    case RIL_REQUEST_OEM_AIMS_HO_TO_WIFI_READY:
    case RIL_REQUEST_OEM_AIMS_HO_TO_WIFI_CANCEL_IND:
    case RIL_REQUEST_OEM_AIMS_HO_PAYLOAD_IND:
    case RIL_REQUEST_OEM_AIMS_HO_TO_3GPP:
    case RIL_REQUEST_OEM_AIMS_SEND_ACK_INCOMING_CDMA_SMS:
    case RIL_REQUEST_OEM_AIMS_MEDIA_STATE_IND:
    case RIL_REQUEST_OEM_AIMS_DEL_PDN_INFO:
    case RIL_REQUEST_OEM_AIMS_STACK_START_REQ:
    case RIL_REQUEST_OEM_AIMS_STACK_STOP_REQ:
    case RIL_REQUEST_OEM_AIMS_XCAPM_START_REQ:
    case RIL_REQUEST_OEM_AIMS_XCAPM_STOP_REQ:
    case RIL_REQUEST_OEM_AIMS_SET_GEO_LOCATION_INFO:
    case RIL_REQUEST_OEM_AIMS_CDMA_SEND_SMS:
    case RIL_REQUEST_OEM_AIMS_SET_PDN_EST_STATUS:
    case RIL_REQUEST_OEM_AIMS_SET_HIDDEN_MENU_ITEM:
    case RIL_REQUEST_OEM_AIMS_GET_HIDDEN_MENU_ITEM:
    case RIL_REQUEST_OEM_AIMS_SET_RTP_RX_STATISTICS:
    case RIL_REQUEST_OEM_WFC_SET_VOWIFI_HO_THRESHOLD:
    case RIL_REQUEST_OEM_AIMS_RTT_SEND_TEXT:
    case RIL_REQUEST_OEM_AIMS_EXIT_EMERGENCY_CB_MODE:
    case RIL_REQUEST_OEM_AIMS_RCS_MULTI_FRAME:
    case RIL_REQUEST_OEM_AIMS_RCS_CHAT:
    case RIL_REQUEST_OEM_AIMS_RCS_GROUP_CHAT:
    case RIL_REQUEST_OEM_AIMS_RCS_OFFLINE_MODE:
    case RIL_REQUEST_OEM_AIMS_RCS_FILE_TRANSFER:
    case RIL_REQUEST_OEM_AIMS_RCS_COMMON_MESSAGE:
    case RIL_REQUEST_OEM_AIMS_RCS_CONTENT_SHARE:
    case RIL_REQUEST_OEM_AIMS_RCS_PRESENCE:
    case RIL_REQUEST_OEM_AIMS_XCAP_MANAGE:
    case RIL_REQUEST_OEM_AIMS_RCS_CONFIG_MANAGE:
    case RIL_REQUEST_OEM_AIMS_RCS_TLS_MANAGE:
        rilsockid = GetProperSocketIdForOem(OEM_CAT_IMS);
        break;
    case RIL_REQUEST_OEM_WFC_MEDIA_CHANNEL_CONFIG:
    case RIL_REQUEST_OEM_WFC_DTMF_START:
        rilsockid = RIL_SOCKET_1;
        break;
    case RIL_REQUEST_GET_IMSI:
    case RIL_REQUEST_OEM_SIM_AUTHENTICATION:
        rilsockid = GetProperSocketIdForWLan(request, data, datalen);
        break;
    case RIL_REQUEST_OEM_SIM_OPEN_CHANNEL:
    case RIL_REQUEST_OEM_SIM_TRANSMIT_APDU_LOGICAL:
    case RIL_REQUEST_OEM_SIM_TRANSMIT_APDU_BASIC:
    case RIL_REQUEST_SIM_CLOSE_CHANNEL:
    case RIL_REQUEST_SIM_GET_ATR:
    case RIL_REQUEST_OEM_SIM_PRESENT:
        rilsockid = GetProperSocketIdForOem(OEM_CAT_SIM);
        break;
    default:
        break;
    }

    return rilsockid;
}

void RilApplication::OnRequestComplete(RilContext *context, RIL_Token t, RIL_Errno e, void *response, unsigned int responselen)
{
    if (context != NULL) {
        OnRequestComplete(t, e, response, responselen);
    }
}

void RilApplication::OnUnsolicitedResponse(RilContext *context, int unsolResponse, const void *data, unsigned int datalen)
{
    if (context != NULL) {
        OnUnsolicitedResponse(unsolResponse, data, datalen, context->GetRilSocketId());
    }
}

void RilApplication::OnRequestAck(RIL_Token t)
{
    RilLogI("[%s] %s", TAG, __FUNCTION__);
    if (t) {
        RilAppToken *token = static_cast<RilAppToken *>(t);
        if (token != NULL) {
            token->context->OnRequestAck(token->t);
        }
    }
}

/* RIL vendor external */
void RilApplication::externalOnRequest(int reqOemId, void *data, unsigned int datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
{
    RilLogV("[%s] %s reqOemId=%d, t=%p, socketid=%d%s",
            TAG, __FUNCTION__, reqOemId, t, socket_id, (socket_id==RIL_SOCKET_UNKNOWN?"(unknown)":""));

    if ( socket_id == RIL_SOCKET_UNKNOWN )
    {
        //RadioExternalRequestInfo* p_reqInfo = (RadioExternalRequestInfo *)t;
        socket_id = GetProperSocketId(reqOemId, data, datalen);
        //if(p_reqInfo != NULL) p_reqInfo->slotId = socket_id;
        RilLogV("[%s] found proper socketid=%d", TAG, socket_id);
    }

    RilContext *pRilContext = GetRilContext(socket_id);
    if (pRilContext != NULL) {
        RilAppToken *token = new RilAppToken(m_pRilExternalRespListener, t);
        pRilContext->OnRequest(reqOemId, data, datalen, (RIL_Token)token);
    }
}

void RilApplication::setRilExternalEnv(const struct RIL_External_Env *pRilExtEnv)
{
    m_pRilExternalEnv = pRilExtEnv;
    m_pRilExternalRespListener = new RilExternalResponseListener(pRilExtEnv);
}

// multi-sim
// initial values can be set in LoadConfigToRilProperty
bool RilApplication::IsMultiSimEnabled()
{
    return (RilApplication::GetPhoneCount() > 1);
}

int RilApplication::GetPhoneCount() {
    int phoneCount = 1;
    RilProperty *property = RilApplication::GetInstance()->GetProperty();
    if (property != NULL) {
        phoneCount = property->GetInt(RIL_APP_PHONE_COUNT, 1);
    }
    return phoneCount;
}
