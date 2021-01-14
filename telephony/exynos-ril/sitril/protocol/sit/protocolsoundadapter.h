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
 * protocolsoundadapter.h
 *
 *  Created on: 2014. 6. 28.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_SOUND_ADAPTER_H__
#define __PROTOCOL_SOUND_ADAPTER_H__

#include "protocoladapter.h"

class ProtocolSoundGetMuteRespAdapter : public ProtocolRespAdapter
{
public:
    ProtocolSoundGetMuteRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetMuteState() const;
};

class ProtocolSoundRingbackToneIndAdapter : public ProtocolIndAdapter
{
public:
    ProtocolSoundRingbackToneIndAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) { }
public:
    int GetRingbackToneState() const;
    int GetFlag() const;
};

class ProtocolSoundGetVolumeRespAdapter : public ProtocolRespAdapter
{
public:
    ProtocolSoundGetVolumeRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetVolume() const;
};

class ProtocolSoundGetAudiopathRespAdapter : public ProtocolRespAdapter
{
public:
    ProtocolSoundGetAudiopathRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetAudiopath() const;
};

class ProtocolSoundGetMultiMICRespAdapter : public ProtocolRespAdapter
{
public:
    ProtocolSoundGetMultiMICRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetMultimicmode() const;
};

class ProtocolSoundWBAMRReportAdapter : public ProtocolIndAdapter
{
public:
    ProtocolSoundWBAMRReportAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) { }
public:
    int GetStatus() const;
    int GetCallType() const;
};

class ProtocolSoundGetWBAMRCapabilityAdapter : public ProtocolRespAdapter
{
public:
    ProtocolSoundGetWBAMRCapabilityAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetWbAmr() const;
};



#endif /* __PROTOCOL_SOUND_ADAPTER_H__ */
