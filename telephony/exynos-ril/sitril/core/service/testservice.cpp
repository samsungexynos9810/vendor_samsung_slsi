/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <sqlite3.h>
#include <arpa/inet.h>
#include "testservice.h"
#include "servicemgr.h"
#include "thread.h"
#include "servicemonitorrunnable.h"
#include "rillog.h"
#include "message.h"
#include "testpacketbuilder.h"
#include "rilparser.h"
#include "netifcontroller.h"
#include <sys/time.h>
#include <map>

#include "customproductfeature.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_PDP, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_PDP, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)

TestService::TestService(RilContext* pRilContext)
    : Service(pRilContext, RIL_SERVICE_TEST)
{
    m_pNetLinkMonitor = NULL;
}

TestService::~TestService()
{
    if (m_pNetLinkMonitor != NULL) {
        delete m_pNetLinkMonitor;
        m_pNetLinkMonitor = NULL;
    }
}

int TestService::OnCreate(RilContext *pRilContext)
{
    RilLogV("[%s] %s", m_szSvcName, __FUNCTION__);

    /*
    m_pNetLinkMonitor = new NetLinkMonitor(pRilContext);
    if (m_pNetLinkMonitor != NULL) {
        m_pNetLinkMonitor->Start();
    }
    */

    return 0;
}

void TestService::OnDestroy()
{
}

BOOL TestService::OnHandleRequest(Message* pMsg)
{
    int ret = -1;
    if (pMsg == NULL) {
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_DEBUG_TRIGGER_IND:
        ret = DoTriggerInd(pMsg);
        break;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL TestService::OnHandleSolicitedResponse(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    int ret = -1;
    switch (pMsg->GetMsgId()) {
      default:
          break;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL TestService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
      default:
          break;
    } // end switch ~

    return TRUE;
}

BOOL TestService::OnHandleInternalMessage(Message* pMsg)
{
    int ret = -1;
    if (pMsg == NULL) {
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_DEBUG_TRIGGER_IND:
        ret = DoTriggerInd(pMsg);
        break;
      default:
          break;
    } // end switch ~
    return TRUE;
}

BOOL TestService::OnHandleRequestTimeout(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    int ret = -1;
    switch (pMsg->GetMsgId()) {
      default:
          break;
    } // end switch ~
    return (ret < 0 ? FALSE : TRUE);
}

int TestService::isSupportedInd(int ind)
{

    if(ind == SIT_IND_NAS_TIMER_STATUS || ind == SIT_IND_PCO_DATA)
        return 1;
    else
        return 0;
}

int TestService::DoTriggerInd(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);
    const unsigned int MAX_READ_BUF_SIZE = (2 * 1500);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    // Check IND+type for Emulation
    IntsRequestData *rildata = (IntsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    // Currently we just implement NAS_TIMER_STATUS_IND = 0x0607(SIT_IND_NAS_TIMER_STATUS)
    int ind = rildata->GetInt(0);
    int opt = rildata->GetInt(1);
    int ret = isSupportedInd(ind);

    if(!ret)
    {
          RilLog("Not supported IND, ind : 0x%x, opt:%d", ind, opt);
          OnRequestComplete(RIL_E_GENERIC_FAILURE);
          return -1;
    }

    //int scenario = rildata->GetScenario();
    // used for each test ind
    char buffer[MAX_READ_BUF_SIZE] = {0, };
    int size = 0;
    TestPacketBuilder builder;

    // Search Emulated IND ModemData
    // Process ModemData
    switch(ind){
      case SIT_IND_NAS_TIMER_STATUS:
          RilLog("Create ModemData for ind : 0x%x, opt:%d", ind, opt);

          if(opt==0){
              size = builder.BuildNasTimerStartInd(buffer, MAX_READ_BUF_SIZE);
              if ( ((size_t)size) <= MAX_READ_BUF_SIZE && builder.DumpData(buffer, size, TRUE) != 0) {
                  RilLogE("DumpData Error!");
              }
              m_pRilContext->ProcessModemData(buffer, size);
          }
          else if(opt==1){
              size = builder.BuildNasTimerExpiredInd(buffer, MAX_READ_BUF_SIZE);
              if ( ((size_t)size) <= MAX_READ_BUF_SIZE && builder.DumpData(buffer, size, TRUE) != 0) {
                  RilLogE("DumpData Error!");
              }
              m_pRilContext->ProcessModemData(buffer, size);
          }
          break;
      case SIT_IND_PCO_DATA:
          RilLog("Create ModemData for ind : 0x%x, opt:%d", ind, opt);

          if(opt==0){
              size = builder.BuildPcoDataInd(buffer, MAX_READ_BUF_SIZE);
              if ( ((size_t)size) <= MAX_READ_BUF_SIZE && builder.DumpData(buffer, size, TRUE) != 0) {
                  RilLogE("DumpData Error!");
              }
              m_pRilContext->ProcessModemData(buffer, size);
          }
          break;
    };

    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

void TestService::TriggerTest(ServiceMgr *pServigMrg)
{
    // Just keeping this before making HIDL Test Trigger app */
//#define TEST_EMULATE_NAS_TIMER_IND
#ifdef TEST_EMULATE_NAS_TIMER_IND
    { /* Trigger Nas Timer IND */
        int testdata[2] = {0x607, 0};
        RequestData *pData = RilParser::CreateInts(RIL_REQUEST_EMULATE_IND, -1, (char *)testdata, sizeof(testdata));
        RilLogI("%s(): pData:%x", __FUNCTION__,pData);

        Message *pMsg = Message::ObtainMessage(pData, RIL_SERVICE_TEST, MSG_DEBUG_TRIGGER_IND);
        IntsRequestData* pReqCheck = (IntsRequestData*)pMsg->GetRequestData();
        if(pReqCheck){
            RilLogI("%s(): size: %d", __FUNCTION__, pReqCheck->GetSize());
            for(int i=-1; i<pReqCheck->GetSize(); i++)
            {
                RilLogI("%s(): item[%d] = %d", __FUNCTION__, i, pReqCheck->GetInt(i));
            }
        } else {
            RilLogE("%s(): Failed to get IntsRequestData", __FUNCTION__);
        }

        if (pServigMrg != NULL) {
            if (pServigMrg->SendMessage(pMsg) < -1) {
                RilLogE("%s(): SendMessage error", __FUNCTION__);
                delete pMsg;
            }
        } else {
            RilLogE("%s() unavailable ServiceMgr instance", __FUNCTION__);
            delete pMsg;
        }

        int testdata1[2] = {0x607, 1};
        pData = RilParser::CreateInts(RIL_REQUEST_EMULATE_IND, -1, (char *)testdata2, sizeof(testdata2));
        RilLogI("%s(): pData:%x", __FUNCTION__,pData);
        pMsg = Message::ObtainMessage(pData, RIL_SERVICE_TEST, MSG_DEBUG_TRIGGER_IND);
        pReqCheck = (IntsRequestData*)pMsg->GetRequestData();
        if(pReqCheck){
            RilLogI("%s(): size: %d", __FUNCTION__, pReqCheck->GetSize());
            for(int i=-1; i<pReqCheck->GetSize(); i++)
            {
                RilLogI("%s(): item[%d] = %d", __FUNCTION__, i, pReqCheck->GetInt(i));
            }
        } else {
            RilLogE("%s(): Failed to get IntsRequestData", __FUNCTION__);
        }

        if (pServigMrg != NULL) {
            if (pServigMrg->SendMessage(pMsg) < -1) {
                RilLogE("%s(): SendMessage error", __FUNCTION__);
                delete pMsg;
            }
        } else {
            RilLogE("%s() unavailable ServiceMgr instance", __FUNCTION__);
            delete pMsg;
        }
    }
#endif
//#define TEST_EMULATE_PCO_DATA_IND
#ifdef TEST_EMULATE_PCO_DATA_IND
    { /* Trigger PCO DATA IND */
        static int testdata[2] = {0x60D, 0};
        RequestData *pData = RilParser::CreateInts(RIL_REQUEST_EMULATE_IND, (void *)-1, (char *)testdata, sizeof(testdata));
        RilLogI("%s(): pData:%x", __FUNCTION__,pData);

        Message *pMsg = Message::ObtainMessage(pData, RIL_SERVICE_TEST, MSG_DEBUG_TRIGGER_IND);
        IntsRequestData* pReqCheck = (IntsRequestData*)pMsg->GetRequestData();
        if(pReqCheck){
            RilLogI("%s(): size: %d", __FUNCTION__, pReqCheck->GetSize());
            for(int i=-1; i<pReqCheck->GetSize(); i++)
            {
                RilLogI("%s(): item[%d] = %d", __FUNCTION__, i, pReqCheck->GetInt(i));
            }
        } else {
            RilLogE("%s(): Failed to get IntsRequestData", __FUNCTION__);
        }

        if (pServigMrg != NULL) {
            if (pServigMrg->SendMessage(pMsg) < -1) {
                RilLogE("%s(): SendMessage error", __FUNCTION__);
                delete pMsg;
            }
        } else {
            RilLogE("%s() unavailable ServiceMgr instance", __FUNCTION__);
            delete pMsg;
        }
    }
#endif
}
