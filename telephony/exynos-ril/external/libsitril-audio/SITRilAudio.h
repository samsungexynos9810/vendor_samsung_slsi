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
 * SITRilAudio.h
 *
 *  Created on: 2015. 4. 30.
 *      Author: sungwoo48.choi
 */

#ifndef __SIT_RIL_AUDIO_H__
#define __SIT_RIL_AUDIO_H__

#include "base.h"
#include "rilaudioapi.h"
#include "rilclienthelper.h"

class RilAudioApi {
    DECLARE_MODULE_TAG()
private:
    RilClientHelper *m_pRilClientHelper;
    HANDLE m_client;
    RILAUDIO_EventCallback m_fpEventHandler;
    HANDLE m_adev;
    bool m_transaction[256];
    bool m_OnClosing;

    // constructor
public:
    RilAudioApi();
    virtual ~RilAudioApi();

protected:
    virtual void HandleSolicitedResponse(unsigned int msgId, int status, void* data, unsigned int length, unsigned int channel);
    virtual void HandleUnsolicitedResponse(unsigned int msgId, void *data, unsigned int length, unsigned int channel);
    bool IsTransactionExisted() const;
    void WaitAllTransactionCompleted();

    // APIs
public:
    int Open();
    int Close();
    int RegisterEventCallback(HANDLE handle, RILAUDIO_EventCallback callback);
    int SetAudioVolume(int volume);
    int SetAudioPath(int audioPath);
    int SetMultiMic(int mode);
    int SetMute(int mode);
    int SetAudioClock(int mode);
    int SetAudioLoopback(int onoff, int path);
    int SetTtyMode(int ttyMode);

    // static
public:
    static RilAudioApi *GetInstance();

    // handler
    static void OnSolicitedResponse(unsigned msgId, int status, void* data, size_t length, unsigned int channel);
    static void OnUnsolicitedResponse(unsigned msgId, void *data, size_t length, unsigned int channel);
};

#endif /* __SIT_RIL_AUDIO_H__ */
