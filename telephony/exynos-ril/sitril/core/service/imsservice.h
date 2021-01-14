 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __IMSSERVICE_H__
#define __IMSSERVICE_H__

#include "service.h"
#include "imsreqdata.h"

#define    IMS_DEFAULT_TIMEOUT        5000
#define    IMS_AIMS_REQUEST_TIMEOUT   180000
#define    IMS_SEND_SMS_TIMEOUT       135000

class Message;
class RilContext;
class ModemData;

typedef struct tagImsCallState{
    BYTE callState;
}ImsCallState;

class ImsService : public Service
{
public:
    ImsService(RilContext* pRilContext);
    virtual ~ImsService();

    INT32 GetImsRegState(void);

protected:
    virtual int OnCreate(RilContext *pRilContext);
    virtual void OnDestroy();

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleInternalMessage(Message* pMsg);

    virtual bool IsPossibleToPassInRadioOffState(int request_id);

protected:
    INT32 DoXXXDone(Message *pMsg);

    virtual INT32 DoSetConfig(Message *pMsg);
    virtual INT32 OnSetConfigDone(Message *pMsg);

    virtual INT32 DoGetConfig(Message *pMsg);
    virtual INT32 OnGetConfigDone(Message *pMsg);

    virtual INT32 DoSimAuth(Message *pMsg);
    virtual INT32 OnSimAuthDone(Message *pMsg);

    virtual INT32 DoSetEmergencyCallStatus(Message *pMsg);
    virtual INT32 OnSetEmergencyCallStatusDone(Message *pMsg);

    virtual INT32 DoSetSrvccCallList(Message *pMsg);
    virtual INT32 OnSetSrvccCallListDone(Message *pMsg);

    virtual INT32 DoGetGbaAuth(Message * pMsg);
    virtual INT32 OnGetGbaAuthDone(Message * pMsg);

    virtual INT32 OnImsConfiguration(Message *pMsg);
    virtual INT32 OnImsDedicatedPdnInfo(Message *pMsg);
    virtual INT32 OnImsEmergencyActInfo(Message *pMsg);
    virtual INT32 OnImsSetSrvccInfo(Message *pMsg);
    virtual INT32 OnImsEmergencyCallList(Message *pMsg);


    //AIMS support start ---------------------
    virtual int DoAimsDefaultRequestHandler(Message *pMsg);
    virtual int OnAimsDefaultResponseHandler(Message *pMsg);
    virtual int OnAimsResponseHandlerWithErrorCode(Message *pMsg);
    virtual int DoAimsDefaultRequestIndHandler(Message *pMsg);
    virtual int OnAimsDefaultUnsolRespHandler(Message *pMsg);

    virtual INT32 Do_AIMS_SET_CALL_WAITING(Message *pMsg);
    virtual INT32 On_AIMS_SET_CALL_WAITINGDone(Message *pMsg);

    virtual INT32 OnUNSOL_AIMS_REGISTRATION(Message *pMsg);
    virtual INT32 Do_AIMS_GET_REGISTRATION(Message *pMsg);
    //AIMS support end ----------------------

protected:
    INT32 mImsRegState;
    RIL_SrvccState m_srvccState;

private:
    BYTE mResp2Byte[2];
};

#endif        //__IMSSERVICE_H__

