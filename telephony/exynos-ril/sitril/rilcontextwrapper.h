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
 * RilContextWrapper.h
 *
 *  Created on: 2014. 11. 12.
 *      Author: sungwoo48.choi
 */

#ifndef __RIL_CONTEXT_WRAPPER_H__
#define __RIL_CONTEXT_WRAPPER_H__

#include "rilcontext.h"
#include "waitlist.h"
#include "mutex.h"
#include "rilproperty.h"
#include "servicestate.h"

class RilData;
class RilParser;
class IoChannel;
class RadioControlMessageMgr;

class RilContextWrapper : public RilContext
{
    DECLARE_MODULE_TAG()

    // member
protected:
    RilApplication *m_pRilApp;
    RilContextParam m_Param;
    RIL_SOCKET_ID m_rilSocketId;
    IoChannel *m_pIoChannel;
    RadioControlMessageMgr *m_pRcmMgr;
    RilParser *m_pRilParser;
    ServiceMgr *m_pServiceMgr;
    WaitList m_RequestWaitList;
    CMutex m_Lock;
    RilProperty m_RilContextProperty;

    // data
    RIL_RadioState m_RadioState;
    unsigned int m_nOpenCarrierIndex;
    bool m_bDataCallStateArray[MAX_DATA_CALL_SIZE*2+1];

    // service state
    ServiceState mServiceState;

    // modem state
    int mModemState;

public:
    RilContextWrapper(RilApplication *rilApp, const RilContextParam *param);
    virtual ~RilContextWrapper();

public:
    RIL_SOCKET_ID GetRilSocketId() const;
    unsigned int GetOpenCarrierIndex();
    void SetOpenCarrierIndex(const char* mccmnc);

protected:


    //@Overriding
public:
    virtual void OnRequest(int request, void *data, unsigned int datalen, RIL_Token t);
    virtual RIL_RadioState OnRadioStateRequest();
    virtual void OnRequestComplete(RIL_Token t, int request, int result, void *response = NULL, unsigned int responselen = 0);
    virtual void OnUnsolicitedResponse(int unsolResponse, const void *data = NULL, unsigned int datalen = 0);
    virtual void OnRequestAck(RIL_Token t);

    virtual void OnRequestComplete(RequestData*req, int result, void *response = NULL, unsigned int responselen = 0);
    virtual void OnRequestTimeout(RequestData*req, unsigned int token);
    virtual void OnModemStateChanged(int state);

    RIL_SOCKET_ID GetRilSocketId();
    virtual ServiceMgr *GetServiceManager();
    virtual RilProperty *GetProperty() { return &m_RilContextProperty; }
    virtual RilProperty *GetApplicationProperty();

    virtual int Send(ModemData *pModemData, UINT nDstServiceId, UINT nResult);
    virtual int Send(void *data, unsigned int datalen);
    virtual int ProcessModemData(void *data, unsigned int datalen);
    void ClearGarbage(UINT nToken);
    bool IsInRequestWaitHistory(UINT nToken);
    RIL_RadioState GetCurrentRadioState() const;

    RilApplication *GetRilApplication() { return m_pRilApp; }
    const RilContextParam *GetRilContextParam() const { return &m_Param; }
    virtual int OnCreate();
    virtual int OnStart();
    virtual void OnDestroy();

    RilContext *GetRilContext(RIL_SOCKET_ID socket_id);
    RilContext *GetOppositeRilContext(void);
    Service* GetService(int nServiceId);
    const ServiceState &GetServiceState() const { return mServiceState; }
    void ResetModem(const char *reason);

private:
    int SendMessage(RequestData *pRequestData);
    int SendMessage(ModemData *pModemData);
    UINT RecordRequestHistory(UINT nSrcServiceId, UINT nDestServiceId, UINT nToken, UINT nResult);
    void RemoveRequestHistory(UINT nToken);
    void SetCardStatus(RIL_CardStatus_v6 *cardStatus);
    void SetNoSim();
    void ProcessNetworkServiceState(int requestId, const void *data, unsigned int  length);
    void ProcessOperatorInfo(ServiceState& ss, const void *data, unsigned int datalen);
    void ProcessNetworkSelectionMode(ServiceState& ss, const void *data, unsigned int datalen);
    void ProcessNetworkRegStateResult(ServiceState& ss, int domain, const void *networkStateResult, unsigned int datalen);
    void ProcessDataRegStateResult(ServiceState& ss, RIL_DataRegistrationStateResponse *dataRegResult);
    void ProcessDataRegStateResult(ServiceState& ss, RIL_DataRegistrationStateResponse_V1_2 *dataRegResult);
    void ProcessDataRegStateResult(ServiceState& ss, RIL_DataRegistrationStateResponse_V1_4 *dataRegResult);
    void ProcessVoiceRegStateResult(ServiceState& ss, RIL_VoiceRegistrationStateResponse *voiceRegResult);
    void ProcessVoiceRegStateResult(ServiceState& ss, RIL_VoiceRegistrationStateResponse_V1_2 *voiceRegResult);
    void ProcessIccCardStatus(const void *data, unsigned int datalen);
    void ProcessImsi(const void *data, unsigned int datalen);

protected:
    void SetRadioState(RIL_RadioState radioState);
    virtual void ProcessSolicitedResponse(int id, const void *data, unsigned int  length);
    virtual void ProcessUnsolicitedResponse(int id, const void *data, unsigned int  length);
    int BroadcastSystemMessage(int messageId, RilData *data = NULL);

    static RilContext *NewInstance(RilApplication *rilApp, const RilContextParam *param);
};

#endif /* __RIL_CONTEXT_WRAPPER_H__ */
