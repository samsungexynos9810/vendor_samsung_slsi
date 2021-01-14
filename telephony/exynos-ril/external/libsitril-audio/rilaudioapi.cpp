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
 * rilaudioapi.cpp
 *
 *  Created on: 2015. 5. 7.
 *      Author: sungwoo48.choi
 */

#include "SITRilAudio.h"

#ifdef RILAUDIO_API_EXPORT
#undef RILAUDIO_API_EXPORT
#endif

#ifdef __cplusplus
#define RILAUDIO_API_EXPORT extern "C"
#else
#define RILAUDIO_API_EXPORT
#endif

RILAUDIO_API_EXPORT int Open()
{
    return RilAudioApi::GetInstance()->Open();
}

RILAUDIO_API_EXPORT int RilAudioOpen()
{
    return RilAudioApi::GetInstance()->Open();
}

RILAUDIO_API_EXPORT int Close()
{
    return RilAudioApi::GetInstance()->Close();
}

RILAUDIO_API_EXPORT int RilAudioClose()
{
    return RilAudioApi::GetInstance()->Close();
}

RILAUDIO_API_EXPORT int RegisterEventCallback(HANDLE handle, RILAUDIO_EventCallback callback)
{
    return RilAudioApi::GetInstance()->RegisterEventCallback(handle, callback);
}

RILAUDIO_API_EXPORT int SetAudioVolume(int volume)
{
    return RilAudioApi::GetInstance()->SetAudioVolume(volume);
}

RILAUDIO_API_EXPORT int SetAudioPath(int audiopath)
{
    return RilAudioApi::GetInstance()->SetAudioPath(audiopath);
}

RILAUDIO_API_EXPORT int SetMultiMic(int mode)
{
    return RilAudioApi::GetInstance()->SetMultiMic(mode);
}

RILAUDIO_API_EXPORT int SetMute(int mode)
{
    return RilAudioApi::GetInstance()->SetMute(mode);
}

RILAUDIO_API_EXPORT int SetAudioClock(int mode)
{
    return RilAudioApi::GetInstance()->SetAudioClock(mode);
}

RILAUDIO_API_EXPORT int SetAudioLoopback(int onoff, int path)
{
    return RilAudioApi::GetInstance()->SetAudioLoopback(onoff, path);
}

RILAUDIO_API_EXPORT int SetTtyMode(int ttyMode)
{
    return RilAudioApi::GetInstance()->SetTtyMode(ttyMode);;
}
