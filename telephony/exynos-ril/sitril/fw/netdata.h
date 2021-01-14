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
 * netdata.h
 *
 *  Created on: 2014. 6. 28.
 *      Author: mox
 */

#ifndef _NET_DATA_H_
#define _NET_DATA_H_

#include "requestdata.h"

#define MAX_UUID_LENGTH 64

/**
 * Network RadioCapability
 */

class NetRCData : public RequestData
{
public:
    NetRCData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    NetRCData(const NetRCData &netrc);
    virtual ~NetRCData();

    int GetVersion() { return m_nVersion; }
    int GetSession() { return m_nSession; }
    int GetPhase() { return m_nPhase; }
    int GetRat() { return m_nRat; }
    char *GetString() { return m_nLogicalModemUuid; }
    int GetStatus() { return m_nStatus; }

    virtual INT32 encode(char *data, unsigned int length);

    INT32 m_nVersion;
    INT32 m_nSession;
    INT32 m_nPhase;
    INT32 m_nRat;
    char m_nLogicalModemUuid[MAX_UUID_LENGTH];
    INT32 m_nStatus;
};

class NetworkScanReqData : public RequestData
{
private:
    bool mIsLegacyRequest;  // IRadio@1.1 compatible request

public:
    NetworkScanReqData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    NetworkScanReqData(const NetworkScanReqData &netScanReqData);
    virtual ~NetworkScanReqData();

    int GetScanType() { return m_scanType; }
    int GetTimeInterval() { return m_timeInterval; }
    int GetSpecifiersLength() { return m_specifiersLength; }
    RIL_RadioAccessSpecifier *GetRadioAccessSpecifier() { return m_radioAccessSpecifiers; }
    int GetMaxSearchTime() { return m_maxSearchTime; }
    bool GetIncrementalResults() { return m_incrementalResults; }
    int GetIncrementalResultsPeriodicity() { return m_incrementalResultsPeriodicity; }
    int GetNumOfMccMncs() { return m_numOfMccMncs; }
    char **GetMccMncs() { return m_mccMncs; }
    bool IsLegacyRequest() const { return mIsLegacyRequest; }

    virtual INT32 encode(char *data, unsigned int length);

private:
    int m_scanType;
    int m_timeInterval;
    int m_specifiersLength;
    RIL_RadioAccessSpecifier m_radioAccessSpecifiers[MAX_RADIO_ACCESS_NETWORKS];
    int m_maxSearchTime;
    bool m_incrementalResults;
    int m_incrementalResultsPeriodicity;
    int m_numOfMccMncs;
    char **m_mccMncs;
};

#endif /*_NET_DATA_H_*/
