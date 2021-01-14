/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef    __PDP_CONTEXT_H__
#define    __PDP_CONTEXT_H__

#include "rildef.h"
#include "apnsetting.h"

enum {
    PDP_CONTEXT_UNAVAILABLE,
    PDP_CONTEXT_DISCONNECTED,
    PDP_CONTEXT_CONNECTING,
    PDP_CONTEXT_CONNECTED,
    PDP_CONTEXT_DISCONNECTING,
    PDP_CONTEXT_IPV6_CONFIGURING,
    PDP_CONTEXT_STATE_MAX,
};

enum {
    INACTIVE = 0,
    ACTIVE = 1,
    ACTIVE_AND_LINKDOWN = 1,
    ACTIVE_AND_LINKUP = 2,
    ACTIVE_STATE_MAX,
};

enum {
    CP_DATA_CONNECTION_ACTIVE = 1,
    CP_DATA_CONNECTION_DORMANT = 2,
};

enum {
    DATA_CONNECTION_ACTIVE_PH_LINK_INACTIVE = 0,
    DATA_CONNECTION_ACTIVE_PH_LINK_DORMANT = 1,
    DATA_CONNECTION_ACTIVE_PH_LINK_UP = 2,
};

class NetIfController;

class PdpContext {
protected:
    int m_cid;
    char m_szIfname[20+1];
    ApnSetting *m_pApnSetting;
    int m_dataProfileId;
    int m_state;
    DataCall m_dataCall;
    NetIfController *m_pNetIfController;

public:
    PdpContext(int cid);
    PdpContext(int cid, const char *ifprefix, int ifindex);
    ~PdpContext();

public:
    void Init();

    void SetApnSetting(ApnSetting *pApnSetting);
    ApnSetting *GetApnSetting() { return m_pApnSetting; }
    int GetCID() const { return m_cid; }
    int GetState() const { return m_state; }
    void SetState(int state);
    void SetActive(int active);
    int GetActive() const { return m_dataCall.active; }
    bool IsAvailable() const;
    void SetDataProfileId(int dataProfileId) { m_dataProfileId = dataProfileId; }
    int GetDataProfileId() const { return m_dataProfileId; }
    int SetAddr(const DataCall *dc);
    DataCall *GetDataCallInfo() { return &m_dataCall;}
    bool UpdateDataCallInfo(const DataCall *dc);
    const char *GetInterfaceName() const { return m_szIfname; }

public:
    int OnActivated(const DataCall *dc);
    int OnActivated();
    int OnDeactivated();
    void Reset();
};

#endif // __PDP_CONTEXT_H__
