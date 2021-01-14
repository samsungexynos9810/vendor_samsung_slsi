 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __AUDIOSERVICE_H__
#define __AUDIOSERVICE_H__

#include "service.h"

#define    AUDIO_DEFAULT_TIMEOUT        3000

class ServiceMgr;
class Message;
class RequestData;
class ModemData;

typedef enum
{
    MSG_AUDIO_TEST = 0,
    MSG_AUDIO_MAX,
}RIL_MSG_AUDIO;

class AudioService :public Service
{
public:
    AudioService(RilContext* pRilContext);
    virtual ~AudioService();

protected:
    static const INT32 AUDIO_TIMEOUT = 5000;

    virtual int OnCreate(RilContext *pRilContext);
    virtual void OnDestroy();

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleInternalMessage(Message* pMsg);
    virtual bool IsPossibleToPassInRadioOffState(int request_id);

private:

private:
    INT32 DoXXXDone(Message *pMsg);

    virtual INT32 DoSetVolume(Message *pMsg);
    virtual INT32 DoSetAudioPath(Message *pMsg);
    virtual INT32 DoSetMultiMic(Message *pMsg);
    virtual INT32 DoSetAudioClock(Message *pMsg);
    virtual INT32 DoSetAudioLoopback(Message *pMsg);

    virtual INT32 DoGetVolume(void);
    virtual INT32 DoGetVolumeDone(Message *pMsg);

    virtual INT32 DoGetAudioPath(void);
    virtual INT32 DoGetAudioPathDone(Message *pMsg);

    virtual INT32 DoGetMultiMic(void);
    virtual INT32 DoGetMultiMicDone(Message *pMsg);

    virtual INT32 DoSetWbAmrCapability(Message *pMsg);

    virtual INT32 DoGetWbAmrCapability(void);
    virtual INT32 DoGetWbAmrCapabilityDone(Message *pMsg);

    virtual INT32 OnWBAMRReportNtf(Message *pMsg);

    virtual int OnSetTtyModeDone(Message *pMsg);
    virtual int DoSetTtyMode(Message *pMsg);
};

#endif

