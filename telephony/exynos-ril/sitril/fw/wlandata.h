/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _WLAN_DATA_H_
#define _WLAN_DATA_H_

#include "requestdata.h"

typedef struct __auth_request
{
    int socket_id;
    unsigned char auth_type;
    unsigned char auth_len;
    unsigned char auth[MAX_AUTH_LEN];
} auth_request_type;

typedef struct _auth_response
{
    unsigned char auth_type;
    unsigned char auth_rsp_len;
    unsigned char auth_rsp[MAX_AUTH_RSP_LEN];
} auth_response_type;

/**
 * OEM SIM Authentication
 */
class OemSimAuthRequest : public RequestData
{
public:
    OemSimAuthRequest(const int nReq, const Token tok, const ReqType type = REQ_OEM);
    ~OemSimAuthRequest();

    int GetAuthType() { return m_nAuthType; }
    BYTE *GetAuth() { return m_pAuth; }
    int GetLength() { return m_nAuthLen; }

    INT32 encode(char *data, unsigned int length);

private:
    int m_nAuthType;
    int m_nAuthLen;
    BYTE m_pAuth[MAX_SIM_AUTH_REQ_LEN];
};

#endif /*_WLAN_DATA_H_*/
