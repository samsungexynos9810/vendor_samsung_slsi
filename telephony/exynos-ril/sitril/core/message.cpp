/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "message.h"

const char *MessageIdToString(int messageId)
{
    return "";
}

Message *Message::NewInstance(int svcId, int msgId, MsgDirection direction, RequestData *pReqData/* = NULL*/, ModemData *pModemData/* = NULL*/)
{
    return new Message(svcId, msgId, direction, pReqData, pModemData);
}

Message::Message()
    :m_nSvcId(-1), m_nMsgId(-1), m_nTimeout(0), m_pReqData(NULL), m_pModemData(NULL), m_pInternalData(NULL)
{
    m_pUserData = NULL;
    m_nDirection = REQUEST;
    m_nAsyncMsgReqStatus = ASYNC_MSG_STATUS_NONE;
    m_tAsyncMsgReqStartTv = {0, 0};
}


Message::Message(int nSvcId, int nMsgId, MsgDirection direction, RequestData *pReqData, ModemData *pModemData, char *pInternalData)
    :m_nSvcId(nSvcId), m_nMsgId(nMsgId), m_nDirection(direction), m_pReqData(pReqData), m_pModemData(pModemData), m_pInternalData(pInternalData)
{
    m_nTimeout = 0;
    m_pUserData = NULL;
    m_nAsyncMsgReqStatus = ASYNC_MSG_STATUS_NONE;
    m_tAsyncMsgReqStartTv = {0, 0};
}

Message::~Message()
{
    if(NULL != m_pReqData)
    {
        delete m_pReqData;
           m_pReqData = NULL;
    }

    if(NULL != m_pModemData)
    {
        delete m_pModemData;
        m_pModemData = NULL;
    }

    if(NULL != m_pInternalData)
    {
        delete m_pInternalData;
        m_pInternalData = NULL;
    }

    if (m_pUserData != NULL) {
        delete m_pUserData;
        m_pUserData = NULL;
    }
}

bool Message::IsBroadcastMessage() const
{
    /*if (m_pModemData == NULL)
        return false;
    ///TODO: remove dependency of sitdef.h
    return ((m_nDirection == RESPONSE) && (m_pModemData->GetType() == RCM_TYPE_INDICATION));
    */
    return false;
}

UINT Message::GetToken() const
{
    if (m_pModemData == NULL)
        return TOKEN_INVALID;

    return m_pModemData->GetToken();
}

void Message::SetModemData(ModemData *pModem)
{
    if (m_pModemData != NULL) {
        delete m_pModemData;
    }
    m_pModemData = pModem;
}

void Message::SetUserData(RilData *pUserData)
{
    if (m_pUserData != NULL) {
        delete m_pUserData;
    }
    m_pUserData = pUserData;
}

Message *Message::Clone() const
{
    Message *msg = new Message(m_nSvcId, m_nMsgId, m_nDirection);
    if (msg != NULL) {
        if (this->m_pReqData != NULL) {
            // implement clone if needed
            // 2014.08.27 sungwoo48.choi
            // apply RequestData Clone
            msg->m_pReqData = this->m_pReqData->Clone();
        }

        // 2014.07.21 sungwoo48.choi
        // Consider cloning RequestData and Internal data
        // currently, cloning ModemData is only allowed
        if (this->m_pModemData != NULL) {
            msg->m_pModemData = this->m_pModemData->Clone();
        }

        // 2014.08.07 sungwoo48.choi
        // m_pUserData can be clone
        if (this->m_pUserData != NULL) {
            msg->m_pUserData = this->m_pUserData->Clone();
        }
    }

    return msg;
}

Message &Message::operator=(const Message &rhs)
{
    this->m_nSvcId = rhs.m_nSvcId;
    this->m_nMsgId = rhs.m_nMsgId;
    this->m_nDirection = rhs.m_nDirection;
    this->m_pReqData = (rhs.m_pReqData != NULL ? rhs.m_pReqData->Clone() : NULL);
    this->m_pModemData = (rhs.m_pModemData != NULL ? rhs.m_pModemData->Clone() : NULL);
    this->m_pInternalData = NULL;
    this->m_pUserData = (rhs.m_pUserData != NULL ? rhs.m_pUserData->Clone() : NULL);

    return *this;
}

Message *Message::ObtainMessage(RequestData *pReqData, int nTargetServiceId, int nTargetMessageId)
{
    return Message::NewInstance(nTargetServiceId, nTargetMessageId, REQUEST, pReqData);
}

Message *Message::ObtainMessage(ModemData *pModemData, int nTargetServiceId, int nTargetMessageId)
{
    return Message::NewInstance(nTargetServiceId, nTargetMessageId, RESPONSE, NULL, pModemData);
}

Message *Message::ObtainMessage(int nSystemMessageId, RilData *pUserData/* = NULL*/)
{
    Message *msg = Message::NewInstance(0, nSystemMessageId, INTERNAL);
    if (msg != NULL) {
        msg->m_pUserData = pUserData;
    }
    return msg;
}

Message *Message::ObtainMessage(RilData *pUserData, int nTargetServiceId, int nTargetMessageId)
{
    Message *msg = Message::NewInstance(nTargetServiceId, nTargetMessageId, INTERNAL);
    if (msg != NULL) {
        msg->m_pUserData = pUserData;
    }
    return msg;
}

const char *Message::MsgDirectionToString(MsgDirection direction)
{
    switch(direction)
    {
        case REQUEST: return "Request";
        case RESPONSE: return "Response";
        case INTERNAL: return "Internal";
        case ASYNC_REQUEST: return "Async Request";
        default: return "<unknown msg direction>";
    }
}

void Message::SetAsyncMsgReqStartTime()
{
    struct timespec tvStart;
    clock_gettime(CLOCK_MONOTONIC, &tvStart);
    m_tAsyncMsgReqStartTv.tv_sec = tvStart.tv_sec;
    m_tAsyncMsgReqStartTv.tv_usec = tvStart.tv_nsec/1000;
}
