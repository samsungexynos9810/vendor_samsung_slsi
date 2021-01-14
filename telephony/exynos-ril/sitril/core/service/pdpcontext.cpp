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
 * pdpcontext.cpp
 *
 *  Created on: 2014. 6. 25.
 *      Author: sungwoo48.choi
 */

#include "pdpcontext.h"
#include "netifcontroller.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_PDP, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_PDP, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_PDP, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_PDP, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define    PREFIX_IFNAME    "rmnet"

PdpContext::PdpContext(int cid)
{
    memset(&m_dataCall, 0, sizeof(m_dataCall));
    m_dataCall.suggestedRetryTime = RETRY_NO_SUGGESTED;
    memset(m_szIfname, 0, sizeof(m_szIfname));
    m_pApnSetting = NULL;
    m_dataProfileId = DATA_PROFILE_DEFAULT;
    m_state = PDP_CONTEXT_UNAVAILABLE;
    m_pNetIfController = NULL;
    if (cid > 0) {
        m_cid = cid;
        snprintf(m_szIfname, sizeof(m_szIfname), "%s%d", PREFIX_IFNAME, m_cid-1);
    }
    else
    {
        m_cid = 0;
    }
}

PdpContext::PdpContext(int cid, const char *ifprefix, int ifindex)
{
    memset(&m_dataCall, 0, sizeof(m_dataCall));
    m_dataCall.suggestedRetryTime = RETRY_NO_SUGGESTED;
    memset(m_szIfname, 0, sizeof(m_szIfname));
    m_pApnSetting = NULL;
    m_dataProfileId = DATA_PROFILE_DEFAULT;
    m_state = PDP_CONTEXT_UNAVAILABLE;
    m_pNetIfController = NULL;
    if (cid > 0) {
        m_cid = cid;
        if (ifprefix == NULL || *ifprefix == 0) {
            ifprefix = PREFIX_IFNAME;
        }
        if (ifindex < 0) {
            ifindex = cid - 1;
        }
        snprintf(m_szIfname, sizeof(m_szIfname), "%s%d", ifprefix, ifindex);
    }
    else
    {
        m_cid = 0;
    }
}

PdpContext::~PdpContext()
{
    if (m_pApnSetting != NULL) {
        delete m_pApnSetting;
    }

    // release NetIfController
    if (m_pNetIfController != NULL) {
        delete m_pNetIfController;
    }
}

void PdpContext::Init()
{
    if (m_cid > 0) {
//        if (*m_szIfname == 0) {
//            sprintf(m_szIfname, "%s%d", PREFIX_IFNAME, m_cid - 1);
//        }
        m_pNetIfController = new NetIfController(this);
        SetState(PDP_CONTEXT_DISCONNECTED);
    }
}

void PdpContext::SetApnSetting(ApnSetting *pApnSetting)
{
    if (m_pApnSetting != NULL) {
        delete m_pApnSetting;
    }
    m_pApnSetting = pApnSetting;
}

bool PdpContext::UpdateDataCallInfo(const DataCall *dc)
{
    if (dc != NULL) {
        if (m_cid == dc->cid) {
            int old_active = INACTIVE;
            // AP can have higher Active state.
            if(m_dataCall.active != INACTIVE && dc->active != INACTIVE)
            {
                old_active = m_dataCall.active;
            }

            // Now Check IPV6 Global existing vs Link-Local incoming
            // CP will always report Link-Local IPV6. Currently there is no interface to sync Global address
            // If we have already generated IPV6 Global Address or something different to Link-Local
            // We will skip
            if(m_dataCall.active != INACTIVE &&
               dc->active != INACTIVE &&
               GetState() == PDP_CONTEXT_CONNECTED &&
               m_dataCall.ipv6.valid && dc->ipv6.valid &&
               !(m_dataCall.ipv6.addr[0] == 0xFE && m_dataCall.ipv6.addr[1] == 0x80) &&
               (dc->ipv6.addr[0] == 0xFE && dc->ipv6.addr[1] == 0x80) &&
               (memcmp(&m_dataCall.ipv6.addr[8], &dc->ipv6.addr[8], 8) == 0)
               )
            {
                // Now Found. So just update other paramters except for IPV6 address and active state
                PDP_ADDR_V6 temp_ipv6;
                RilLogI("IPV6 has network prefix, so keep global address, cid=%d", m_dataCall.cid);
                memcpy(&temp_ipv6, &m_dataCall.ipv6, sizeof(PDP_ADDR_V6));
                memcpy(&m_dataCall, dc, sizeof(m_dataCall));
                memcpy(&m_dataCall.ipv6, &temp_ipv6, sizeof(PDP_ADDR_V6));
            }
            else{
                memcpy(&m_dataCall, dc, sizeof(m_dataCall));
            }

            if(old_active != INACTIVE && dc->active != INACTIVE)
            {
                if(dc->active == CP_DATA_CONNECTION_DORMANT) {
                    m_dataCall.active = DATA_CONNECTION_ACTIVE_PH_LINK_DORMANT;
                } else {
                    m_dataCall.active = DATA_CONNECTION_ACTIVE_PH_LINK_UP;
                }
                RilLogI("Changing ACTIVE cid=%d, CP active=%d, FW active=%d", m_dataCall.cid, dc->active, m_dataCall.active);
            }


            // filtered requested protocol type and returned protocol type is different
            if (m_pApnSetting != NULL) {
                if (TextUtils::Equals(m_pApnSetting->GetProtocol(), DATA_PROTOCOL_IP)) {
                    m_dataCall.ipv6.valid = false;
                    m_dataCall.pdpType = PDP_TYPE_IPV4;
                }
                else if (TextUtils::Equals(m_pApnSetting->GetProtocol(), DATA_PROTOCOL_IPV6)) {
                    m_dataCall.ipv4.valid = false;
                    m_dataCall.pdpType = PDP_TYPE_IPV6;
                }
            }
            if (m_dataCall.ipv4.valid || m_dataCall.ipv6.valid) {
                return true;
            }
        }
    }
    return false;
}

int PdpContext::SetAddr(const DataCall *dc)
{
    if(m_dataCall.cid != dc->cid){
        RilLogE("PdpContext cid in't matched");
        return -1;
    }

    // TODO copy address information to PDP Context
    if (dc->ipv4.valid) {
        m_dataCall.ipv4.valid= TRUE;
        memcpy(&m_dataCall.ipv4.addr, &dc->ipv4.addr, sizeof(m_dataCall.ipv4.addr));
    }

    if (dc->ipv6.valid) {
        memcpy(&m_dataCall.ipv6.addr, &dc->ipv6.addr, sizeof(m_dataCall.ipv6.addr));
    }

    m_dataCall.active = ACTIVE_AND_LINKDOWN;

    // TODO link down and deactivate

    return 0;
}

void PdpContext::SetState(int state)
{
    if (state >= 0 && state < PDP_CONTEXT_STATE_MAX) {
        m_state = state;
    }
}

void PdpContext::SetActive(int active)
{
    if (active >= 0 && active < ACTIVE_STATE_MAX) {
        m_dataCall.active = active;
    }
}

bool PdpContext::IsAvailable() const
{
    return (GetState() == PDP_CONTEXT_DISCONNECTED);
}

int PdpContext::OnActivated(const DataCall *dc)
{
    if (dc == NULL) {
        return -1;
    }

    if (dc->active == 0) {
        return -1;
    }

    if (m_cid != dc->cid) {
        return -1;
    }

    // update data call information
    if (UpdateDataCallInfo(dc)) {
        if (m_pNetIfController != NULL && m_pNetIfController->BringUp()) {
            RilLogI("Set DataCall(cid=%d) Active State to ACTIVE_AND_LINKUP(%d)", dc->cid, ACTIVE_AND_LINKUP);
            SetActive(ACTIVE_AND_LINKUP);
        }
        else {
            // fail to bring-up network interface
            SetActive(ACTIVE_AND_LINKDOWN);

            // return error
            return -1;
        }
    }

    if (m_dataCall.ipv6.valid && m_dataCall.ipv6.addr[0] == 0xFE && m_dataCall.ipv6.addr[1] == 0x80) {
        SetState(PDP_CONTEXT_IPV6_CONFIGURING);
        return 0;
    }
    SetState(PDP_CONTEXT_CONNECTED);

    return 0;
}

int PdpContext::OnActivated()
{
    return OnActivated(&m_dataCall);
}

int PdpContext::OnDeactivated()
{
    if (m_pNetIfController != NULL) {
        m_pNetIfController->TearDown();
    }
    Reset();
    SetState(PDP_CONTEXT_DISCONNECTED);

    return 0;
}

void PdpContext::Reset()
{
    SetState(PDP_CONTEXT_DISCONNECTED);
    memset(&m_dataCall, 0, sizeof(m_dataCall));
    m_dataCall.suggestedRetryTime = RETRY_NO_SUGGESTED;
}
