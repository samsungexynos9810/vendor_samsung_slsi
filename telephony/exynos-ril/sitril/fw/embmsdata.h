/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _EMBMS_DATA_H_
#define _EMBMS_DATA_H_

#include "sitdef.h"
#include "requestdata.h"

#define MAX_INFOBIND_NUM                (1)
#define MAX_MULTICAST_IP                (40)

/**
 * eMbms Session Data
 */

class EmbmsSessionData : public RequestData
{
public:
    EmbmsSessionData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    ~EmbmsSessionData();
    int GetState() { return m_State; }
    UINT8 GetPriority() {return m_Priority; }
    uint64_t GetTmgi() { return m_TMGI; }
    RIL_InfoBinding GetInfoBind() { return m_InfoBind; }
    int GetInfoBindCount() { return m_InfoBindCount; }

    virtual INT32 encode(char *data, unsigned int length);

private:
    INT32 m_State;
    UINT8 m_Priority;
    uint64_t m_TMGI;             // Temporary Mobile Group Identity
    RIL_InfoBinding m_InfoBind;  // MAX_Count value
    INT32 m_InfoBindCount;
};

#endif /*_EMBMS_DATA_H_*/
