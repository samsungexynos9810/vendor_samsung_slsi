 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */


#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "messageid.h"
#include "requestdata.h"
#include "modemdata.h"
#include "rildatabuilder.h"

/**
 *Message direction
 */
enum MsgDirection{
    MSG_DIRECTION_MIN = 0,
    REQUEST = 1,
    RESPONSE = 2,
    INTERNAL = 3,
    ASYNC_REQUEST = 4,
    MSG_DIRECTION_MAX
};

enum AsyncMsgReqStatus{
    ASYNC_MSG_STATUS_NONE = 0,
    ASYNC_MSG_STATUS_INITIATED = 1,
    ASYNC_MSG_STATUS_PROCESSING = 2,
    ASYNC_MSG_STATUS_SENT2CP = 3,
    ASYNC_MSG_STATUS_RESPONSE = 4,
    ASYNC_MSG_STATUS_MAX
};

const char *MessageIdToString(int messageId);

class Message
{
public:
    int GetSvcId() {return m_nSvcId;}
    void SetSvcId(int svcId)  {m_nSvcId = svcId;}

    int GetMsgId() {return m_nMsgId;}
    void SetMsgId(int msgId)  {m_nMsgId = msgId;}

    int GetTimeout() {return m_nTimeout;}
    void SetTimeout(int timeout)  {m_nTimeout = timeout;}

    MsgDirection GetDirection() {return m_nDirection;}
    void SetDirection(MsgDirection direction) {m_nDirection = direction;}

    RequestData *GetRequestData() {return m_pReqData;}

    ModemData *GetModemData() const {return m_pModemData;}
    void SetModemData(ModemData *pModem);

    char *GetInternalData() {return m_pInternalData;}

    void SetUserData(RilData *pUserData);
    RilData *GetUserData() {return m_pUserData;}

    bool IsBroadcastMessage() const;
    UINT GetToken() const;

    // Async Message Req interface
    AsyncMsgReqStatus GetAsyncMsgReqStatus() {return m_nAsyncMsgReqStatus;}
    void SetAsyncMsgReqStatus(AsyncMsgReqStatus reqStatus)  {m_nAsyncMsgReqStatus = reqStatus;}
    void SetAsyncMsgReqStartTime();
    struct timeval GetAsyncMsgReqStartTime() {return m_tAsyncMsgReqStartTv;}

public:
    virtual Message *Clone() const;
public:
    virtual Message &operator=(const Message &rhs);

public:
    Message();
    Message(int svcId, int msgId, MsgDirection direction, RequestData *pReqData = NULL, ModemData *pModemData = NULL, char *pInternalData = NULL);
    virtual ~Message();

public:
    static Message *NewInstance(int svcId, int msgId, MsgDirection direction, RequestData *pReqData = NULL, ModemData *pModemData = NULL);

public:
    static Message *ObtainMessage(RequestData *pReqData, int nTargetServiceId, int nTargetMessageId);
    static Message *ObtainMessage(ModemData *pModemData, int nTargetServiceId, int nTargetMessageId);
    static Message *ObtainMessage(int nSystemMessageId, RilData *pUserData = NULL);
    static Message *ObtainMessage(RilData *pUserData, int nTargetServiceId, int nTargetMessageId);
    const char *MsgDirectionToString(MsgDirection direction);

private:
    int m_nSvcId;
    int m_nMsgId;
    int m_nTimeout;
    MsgDirection m_nDirection;
    RequestData *m_pReqData;
    ModemData *m_pModemData;
    char *m_pInternalData;
    RilData *m_pUserData;

    // Priority Req interface
    AsyncMsgReqStatus m_nAsyncMsgReqStatus;
    struct timeval m_tAsyncMsgReqStartTv;

public:

};
#endif /*_MESSAGE_H_*/
