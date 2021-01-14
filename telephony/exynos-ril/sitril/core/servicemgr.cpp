/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "servicemgr.h"
#include "service.h"
#include "rillog.h"
#include "oemril.h"
#include "oemreqdata.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

static bool debug = true;

#include "ril_commands.h"
#include "ril_oem_commands.h"
#include "protocol_unsol_commands.h"
#include "ril_async_message.h"

ServiceMgr::ServiceMgr(RilContext* pCRilContext)
{
    m_pRilContext = pCRilContext;
}

ServiceMgr::~ServiceMgr()
{
    Finalize();
}

int ServiceMgr::Init(ServiceFactory *pServiceFactory/* = NULL*/)
{
    if (pServiceFactory == NULL) {
        return -1;
    }

    pServiceFactory->CreateService(m_pRilContext, this);
    OnInitialize();

    return 0;
}

int ServiceMgr::OnInitialize()
{
    return 0;
}

int ServiceMgr::Finalize()
{
    if (!m_mapServiceMap.empty()) {
        ServicesMap::iterator iter;
        for(iter = m_mapServiceMap.begin(); iter != m_mapServiceMap.end(); ++iter) {
            Service* pService = (Service*)iter->second;
            if(pService != NULL) {
                pService->Finalize();
                delete pService;
            }
        }
        m_mapServiceMap.clear();
    }

    return 0;
}

int ServiceMgr::SendMessage(Message *msg)
{
    if (msg == NULL) {
        RilLogE("Send message fail: message is NULL");
        return -1;
    }

    Service* pService = FindService(msg->GetSvcId());
    if(pService == NULL) {
        RilLogE("Send message fail: cannot find service %s, Did you forget AddService in model Init?", Service::GetServiceName(msg->GetSvcId()));
        return -1;
    }

    if (debug)
        RilLogV("[%s] has been called, and msg(%p), msgid=%d, svc=%s(%d), direction=%s", __FUNCTION__,
                        msg,
                        msg->GetMsgId(), pService->GetServiceName(), msg->GetSvcId(),
                        msg->MsgDirectionToString(msg->GetDirection()));

    pService->AsyncMsgReqPreProcessing(msg);
    pService->EnQueue(msg);
    pService->NotifyNewMessage(msg);

    return 0;
}

void ServiceMgr::BroadcastSystemMessage(Message* msg)
{
    if (msg != NULL) {
        if (!m_mapServiceMap.empty()) {
            ServicesMap::iterator iter;
            for (iter = m_mapServiceMap.begin(); iter != m_mapServiceMap.end(); ++iter) {
                Service *pService = (Service *) iter->second;
                if (pService != NULL) {
                    Message* pCopyMsg = msg->Clone();
                    if (pCopyMsg != NULL) {
                        pCopyMsg->SetSvcId(pService->GetServiceId());
                        pService->EnQueue(pCopyMsg);
                        pService->NotifyNewMessage(pCopyMsg);
                    }
                }
            } // end for iter ~
        }
        delete msg;
    }
}

void ServiceMgr::BroadcastSystemMessage(int messageId, RilData *data/* = NULL*/)
{
    Message *message = Message::ObtainMessage(messageId, data);
    if (message != NULL) {
        BroadcastSystemMessage(message);
    }
    else {
        if (data != NULL) {
            delete data;
        }
    }
}

Service* ServiceMgr::FindService(int nServiceId)
{
    if (!m_mapServiceMap.empty()) {
        ServicesMap::iterator iter = m_mapServiceMap.find(nServiceId);;
        if (iter != m_mapServiceMap.end()) {
            return (Service*)iter->second;;
        }
    }
    return NULL;
}

int ServiceMgr::RouteRequest(int requestId, int &nServiceId, int &nMessageId)
{
    RouteMap::iterator iter;

    if (requestId <= 0) {
        return -1;
    }

    nServiceId = -1;
    nMessageId = 0;

    iter = s_RilRequestServiceMap.find(requestId);
    if (iter != s_RilRequestServiceMap.end()) {
        std::pair<UINT, UINT> val = s_RilRequestServiceMap[iter->first];
        nServiceId = val.first;
        nMessageId = val.second;
    }
    //RilLogV("(%d) Route Service(%d) and Message ID(%d)", requestId, nServiceId, nMessageId);

    return 0;
}

int ServiceMgr::RouteRequest(RequestData *pRequestData, int &nServiceId, int &nMessageId)
{
    RouteMap::iterator iter;

    if (pRequestData == NULL) {
        return -1;
    }

    int requestId = pRequestData->GetReqId();
    if (requestId <= 0) {
        return -1;
    }

    nServiceId = -1;
    nMessageId = 0;
    // pre-process for OEM_HOOK_RAW
    if (requestId == RIL_REQUEST_OEM_HOOK_RAW) {
        return RouteOemRequest(pRequestData, nServiceId, nMessageId);
    }

    // divide RIL Request and RIL OEM Request
    if (requestId >= RIL_OEM_REQUEST_BASE) {
        iter = s_RilOemRequestServiceMap.find(requestId);
        if (iter != s_RilOemRequestServiceMap.end()) {
            std::pair<UINT, UINT> val = s_RilOemRequestServiceMap[iter->first];
            nServiceId = val.first;
            nMessageId = val.second;
        }
    } else {
        iter = s_RilRequestServiceMap.find(requestId);
        if (iter != s_RilRequestServiceMap.end()) {
            std::pair<UINT, UINT> val = s_RilRequestServiceMap[iter->first];
            nServiceId = val.first;
            nMessageId = val.second;
        }
    }
    //RilLogV("(%d) Route Service(%d) and Message ID(%d)", requestId, nServiceId, nMessageId);

    return ((nServiceId > 0 && nMessageId > 0) ? 0 : -1);
}

int ServiceMgr::RouteOemRequest(RequestData *pRequestData, int &nServiceId, int &nMessageId)
{
    RouteMap::iterator iter;

    RilLogI("%s", __FUNCTION__);
    if (pRequestData == NULL) {
        RilLogW("pRequestData is NULL");
        return -1;
    }

    nServiceId = -1;
    nMessageId = 0;

    OemHookRawRequestData *pOemHookRawData = static_cast<OemHookRawRequestData *>(pRequestData);
    if (pOemHookRawData == NULL) {
        RilLogW("pRequestData is not a instance of OemHookRawRequestData");
        return -1;
    }

    int nOemRequestId = pOemHookRawData->GetReqId();
    RilLogI("OemHookRawRequest has been translated to OEM Request ID %d", nOemRequestId);
    iter = s_RilOemRequestServiceMap.find(nOemRequestId);
    if (iter != s_RilOemRequestServiceMap.end()) {
        std::pair<UINT, UINT> val = s_RilOemRequestServiceMap[iter->first];
        nServiceId = val.first;
        nMessageId = val.second;
    }
    //RilLogV("(%d) Route Oem Service(%d) and Message ID(%d)", nOemRequestId, nServiceId, nMessageId);

    return ((nServiceId > 0 && nMessageId > 0) ? 0 : -1);
}

int ServiceMgr::RouteProtocolInd(int nId, int &nServiceId, int &nMessageId)
{
    RouteMap::iterator iter;
    //int targetServiceId = -1;
    //int targetMessageId = 0;

    nServiceId = -1;
    nMessageId = 0;

    iter = s_ProtocolServiceMap.find(nId);
    if (iter != s_ProtocolServiceMap.end()) {
        std::pair<UINT, UINT> val = s_ProtocolServiceMap[iter->first];
        nServiceId = val.first;
        nMessageId = val.second;
    }
    //RilLogV("(%d) Route Protocol Ind Service(%d) and Message ID(%d)", nId, nServiceId, nMessageId);

    return ((nServiceId > 0 && nMessageId > 0) ? 0 : -1);
}

int ServiceMgr::SetRequestMap(int nRequestId, int nServiceId, int nMessageId)
{
    RouteMap::iterator iter;

    iter = s_RilRequestServiceMap.find(nRequestId);
    if (iter == s_RilRequestServiceMap.end()) {
        s_RilRequestServiceMap.insert(
                make_pair(nRequestId, make_pair(nServiceId, nMessageId)));
    } else {
        s_RilRequestServiceMap[nRequestId] = make_pair(nServiceId, nMessageId);
    }

    return 0;
}

int ServiceMgr::SetOemRequestMap(int nOemRequestId, int nServiceId, int nMessageId)
{
    RouteMap::iterator iter;

    iter = s_RilOemRequestServiceMap.find(nOemRequestId);
    if (iter == s_RilOemRequestServiceMap.end()) {
        s_RilOemRequestServiceMap.insert(
                make_pair(nOemRequestId, make_pair(nServiceId, nMessageId)));
    } else {
        s_RilOemRequestServiceMap[nOemRequestId] = make_pair(nServiceId, nMessageId);
    }

    return 0;
}

int ServiceMgr::SetProtocolIndMap(int nIndId, int nServiceId, int nMessageId)
{
    RouteMap::iterator iter;

    iter = s_ProtocolServiceMap.find(nIndId);
    if (iter == s_ProtocolServiceMap.end()) {
        s_ProtocolServiceMap.insert(
                make_pair(nIndId, make_pair(nServiceId, nMessageId)));
    } else {
        s_ProtocolServiceMap[nIndId] = make_pair(nServiceId, nMessageId);
    }

    return 0;
}

int ServiceMgr::AddService(int nServiceId, Service *pService)
{
    if (nServiceId <= 0 || pService == NULL) {
        return -1;
    }

    Service *pCur = FindService(nServiceId);
    if (pCur != NULL) {
        pCur->Finalize();
        delete pCur;
    }

    m_mapServiceMap[nServiceId] = pService;
    pService->Init();

    return 0;
}

int ServiceMgr::StartServices()
{
    if (m_mapServiceMap.empty()) {
        RilLogW("%s: There is no service at all!.", __FUNCTION__);
        return -1;
    }

    ServicesMap::iterator iter;
    for (iter = m_mapServiceMap.begin(); iter != m_mapServiceMap.end(); ++iter) {
        Service *pService = (Service *) iter->second;
        if (pService != NULL) {
            if (pService->GetServiceState() == Service::CREATED) {
                pService->Start();
            }
            else {
                RilLogW("%s: %s is not initialized yet.", __FUNCTION__, pService->GetServiceName());
            }
        }
    } // end for iter ~
    return 0;
}

bool ServiceMgr::IsAsyncMessage(int nServiceId, int nMessageId)
{
    bool ret = FALSE;
    AsyncMsgMap::iterator iter;

    iter = s_AsyncMsgMap.find(nMessageId);
    if (iter != s_AsyncMsgMap.end()) {
        if (iter->second == (UINT)nServiceId) ret = TRUE;
    }

    return ret;
}
