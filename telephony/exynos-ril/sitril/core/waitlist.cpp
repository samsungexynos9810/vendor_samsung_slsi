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
 * wailtlist.cpp
 *
 *  Created on: 2014. 6. 16.
 *      Author: sungwoo48.choi
 */

#include "waitlist.h"
#include "service.h"
#include "modemdata.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

MessageHistoryRecord MessageHistoryRecord::newInstance(const Service *pSrcService, const ModemData *pModemData, UINT nResult)
{
    MessageHistoryRecord record;
    if (pSrcService != NULL && pModemData != NULL) {
        UINT nSrcServiceId, nDestServiceId;
        nSrcServiceId = nDestServiceId = pSrcService->GetServiceId();

        UINT nToken = pModemData->GetToken();
        if (nToken != TOKEN_INVALID) {
            record.m_nSrcServiceId = nSrcServiceId;
            record.m_nDestServiceId = nDestServiceId;
            record.m_nToken = nToken;
            record.m_nResult = nResult;
        }
    }

    return record;
}

MessageHistoryRecord MessageHistoryRecord::newInstance(UINT nSrcServiceId, UINT nDestServiceId, UINT nToken, UINT nResult)
 {
    MessageHistoryRecord record;
    if (nToken != TOKEN_INVALID) {
        record.m_nSrcServiceId = nSrcServiceId;
        record.m_nDestServiceId = nDestServiceId;
        record.m_nToken = nToken;
        record.m_nResult = nResult;
    }
    return record;
}


MessageHistoryRecord::MessageHistoryRecord(UINT nToken /*= TOKEN_INVALID*/)
    : m_nToken(nToken)
    , m_nSrcServiceId(-1)
    , m_nDestServiceId(-1)
    , m_nResult(-1)
{

}

MessageHistoryRecord::MessageHistoryRecord(UINT nToken, UINT nSrcServiceId, UINT nDestServiceId, UINT nResult)
    : m_nToken(nToken)
    , m_nSrcServiceId(nSrcServiceId)
    , m_nDestServiceId(nDestServiceId)
    , m_nResult(nResult)
{

}

MessageHistoryRecord::~MessageHistoryRecord() { }

MessageHistoryRecord &MessageHistoryRecord::operator=(const MessageHistoryRecord &rhs)
{
    this->m_nSrcServiceId = rhs.m_nSrcServiceId;
    this->m_nDestServiceId = rhs.m_nDestServiceId;
    this->m_nResult = rhs.m_nResult;
    this->m_nToken = rhs.m_nToken;

    return *this;
}

//bool MessageHistoryRecord::operator==(const MessageHistoryRecord &lhs, const MessageHistoryRecord &rhs)
//{
//    return (lhs.m_nToken == rhs.m_nToken &&
//            lhs.m_nSrcServiceId == rhs.m_nDestServiceId &&
//            lhs.m_nDestServiceId == rhs.m_nDestServiceId &&
//            lhs.m_nResult == rhs.m_nResult);
//}


/////////////////////////////////////////////////////////////////////////////////////////
WaitList *WaitList::_instance = NULL;
WaitList *WaitList::GetInstance()
{
    if (_instance == NULL) {
        _instance = new WaitList();
        _instance->Init();
    }
    return _instance;
}

void WaitList::Release()
{
    if (_instance != NULL) {
        delete _instance;
        _instance = NULL;
    }
}

WaitList::WaitList() : m_nSize(0)
{
}

WaitList::~WaitList()
{
}

void WaitList::Init()
{

}

void WaitList::Push(const MessageHistoryRecord &historyRecord)
{
    Lock();
    UINT nToken = historyRecord.GetToken();
    if (nToken != TOKEN_INVALID) {
        m_history[nToken] = historyRecord;
        m_nSize++;
    }
    Unlock();
}

void WaitList::Remove(UINT nId)
{
    Lock();
    if (m_history.erase(nId) > 0)
        m_nSize--;
    Unlock();
}

void WaitList::Clear()
{
    Lock();
    if (m_nSize > 0) {
        m_history.clear();
        m_nSize = 0;
    }
    Unlock();
}

bool WaitList::Contains(const MessageHistoryRecord &historyRecord)
{
    //bool ret = false;
    UINT nId = historyRecord.GetToken();
    return Contains(nId);
}

bool WaitList::Contains(UINT nId)
{
    bool ret = false;
    Lock();
    map<UINT, MessageHistoryRecord>::iterator iter = m_history.find(nId);
    if (iter != m_history.end())
        ret = true;
    Unlock();
    return ret;
}

const MessageHistoryRecord WaitList::Find(UINT nId)
{
    MessageHistoryRecord record;
    Lock();
    RilLogV("WaitList::Find() nId=0x%04X", nId);
    map<UINT, MessageHistoryRecord>::iterator iter = m_history.find(nId);
    if (iter != m_history.end()) {
        record = iter->second;
    }
    if (record.IsValid()) {
        RilLogV("found record : token=0x%04X SrcService ID =%d DestService ID=%d Result Id=%d",
                record.GetToken(), record.GetSrcServiceId(), record.GetDestServiceId(), record.GetResult());
    }
    else {
        RilLogE("found record error : token=0x%04X ", record.GetToken());
    }
#if 0
    MessageHistoryRecord test;
    for (map<UINT,MessageHistoryRecord>::iterator iter=m_history.begin(); iter != m_history.end(); ++iter)
    {
        test = iter->second;
        if ( test.GetToken() == nId )
        {
            break;
        }
    }
    RilLogV("found record(test) : token=0x%04X ", test.GetToken());
#endif
    Unlock();
    return record;
}

void WaitList::Print() const
{
//    // build log trace
//    String strTrace = this->Trace();
//    // log trace to stdout
//    cout << strTrace << endl;

    // TODO log trace to logcat

}

void WaitList::Print(char *szBuffer, UINT nSize) const
{
//    if (szBuffer != NULL) {
//        String str = this->ToString();
//        str.copy(szBuffer, nSize);
//    }
}
