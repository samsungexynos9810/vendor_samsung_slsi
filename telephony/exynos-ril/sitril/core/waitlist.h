/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef    __WAIT_LIST_H__
#define    __WAIT_LIST_H__

#include "rildef.h"
#include "mutex.h"
#include <map>
using namespace std;

class Service;
class ModemData;

class MessageHistoryRecord
{
private:
    UINT    m_nToken;
    UINT    m_nSrcServiceId;
    UINT    m_nDestServiceId;
    UINT    m_nResult;

public:
    MessageHistoryRecord(UINT nToken = TOKEN_INVALID);
    MessageHistoryRecord(UINT nToken, UINT nSrcServiceId, UINT nDestServiceId, UINT nResult);
    ~MessageHistoryRecord();

public:
    UINT GetToken() const { return m_nToken; }
    UINT GetSrcServiceId() const { return m_nSrcServiceId; }
    UINT GetDestServiceId() const { return m_nDestServiceId; }
    UINT GetResult() const { return m_nResult; }
    bool IsValid() const { return m_nToken != TOKEN_INVALID; }

    // operator
    MessageHistoryRecord &operator=(const MessageHistoryRecord &rhs);
    //bool operator==(const MessageHistoryRecord &lhs, const MessageHistoryRecord &rhs);  // [ARKADE] 2014-07-01, Comment-out for SIT Compilation

public:
    static MessageHistoryRecord newInstance(const Service *pSrcService, const ModemData *pModemData, UINT nResult);
    static MessageHistoryRecord newInstance(UINT nSrcServiceId, UINT nDestServiceId, UINT nToken, UINT nResult);
};

class WaitList
{
private:
    CMutex    m_lock;
    int m_nSize;
    map<UINT, MessageHistoryRecord> m_history;

    /**
     * constructor
     */
private:
public:
    WaitList();
    virtual ~WaitList();

public:
    void Push(const MessageHistoryRecord &historyRecord);
    void Remove(UINT nId);
    bool Contains(UINT nId);
    bool Contains(const MessageHistoryRecord &historyRecord);
    const MessageHistoryRecord Find(UINT nId);
    void Clear();
    const String &ToString() const;
    void Print() const;
    void Print(char *szBuffer, UINT nSize) const;

protected:
    virtual void Init();
    inline void Lock() { m_lock.lock(); }
    inline void Unlock() { m_lock.unlock(); }

private:
    static WaitList *_instance;

public:
    static WaitList *GetInstance();
    static void Release();
};

#endif // __WAIT_LIST_H__
