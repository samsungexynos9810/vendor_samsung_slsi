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
 * rilContext2.h
 *
 *  Created on: 2014. 11. 12.
 *      Author: sungwoo48.choi
 */

#ifndef _RIL_CONTEXT_H_
#define _RIL_CONTEXT_H_

#include "rildef.h"

class RilApplication;
class Service;
class ServiceMgr;
class Message;
class ModemData;
class RequestData;
class ProductFactory;
class ServiceFactory;
class RilProperty;
class ServiceState;

typedef struct tagRilContextParam {
    RIL_SOCKET_ID socket_id;
    char iochannel_name[MAX_IOCHANNEL_NAME_LEN+1];
    char ifprefix[MAX_IFPREFIX_LEN+1];
    unsigned int ifstart;
    unsigned int ifmaxsize;
    ProductFactory *productFactory;
    ServiceFactory *serviceFactory;
} RilContextParam;


class RilContext
{
public:
    RilContext() {}
    virtual ~RilContext() {}

public:
    virtual void OnRequest(int request, void *data, unsigned int datalen, RIL_Token t)=0;
    virtual RIL_RadioState OnRadioStateRequest()=0;
    virtual void OnRequestComplete(RIL_Token t, int request, int result, void *response = NULL, unsigned int responselen = 0)=0;
    virtual void OnUnsolicitedResponse(int unsolResponse, const void *data = NULL, unsigned int datalen = 0)=0;
    virtual void OnRequestAck(RIL_Token t)=0;

    virtual void OnRequestComplete(RequestData*req, int result, void *response = NULL, unsigned int responselen = 0)=0;
    virtual void OnRequestTimeout(RequestData*req, unsigned int token)=0;
    virtual void OnModemStateChanged(int state)=0;

    virtual RIL_SOCKET_ID GetRilSocketId()=0;
    virtual unsigned int GetOpenCarrierIndex()=0;
    virtual void SetOpenCarrierIndex(const char* mccmnc)=0;
    virtual ServiceMgr *GetServiceManager()=0;
    virtual RilProperty *GetProperty()=0;
    virtual RilProperty *GetApplicationProperty()=0;

    virtual int Send(ModemData *pModemData, UINT nDstServiceId, UINT nResult)=0;
    virtual int Send(void *data, unsigned int datalen)=0;
    virtual int ProcessModemData(void *data, unsigned int datalen)=0;
    virtual void ClearGarbage(UINT nToken)=0;
    virtual bool IsInRequestWaitHistory(UINT nToken)=0;

    virtual RIL_RadioState GetCurrentRadioState() const=0;

    virtual RilApplication *GetRilApplication()=0;
    virtual const RilContextParam *GetRilContextParam() const=0;
    virtual int OnCreate()=0;
    virtual int OnStart()=0;
    virtual void OnDestroy()=0;

    virtual RilContext *GetRilContext(RIL_SOCKET_ID socket_id)=0;
    virtual RilContext *GetOppositeRilContext(void)=0;
    virtual Service* GetService(int nServiceId)=0;
    virtual const ServiceState &GetServiceState() const=0;
    virtual void ResetModem(const char *reason)=0;
};

#endif
