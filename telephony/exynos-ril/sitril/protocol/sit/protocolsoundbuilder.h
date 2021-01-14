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
 * protocolsoundbuilder.h
 *
 *  Created on: 2014. 12. 2.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_SOUND_BUILDER_H__
#define __PROTOCOL_SOUND_BUILDER_H__

#include "protocolbuilder.h"

class ProtocolSoundBuilder : public ProtocolBuilder
{
public:
    ProtocolSoundBuilder() : ProtocolBuilder() {}
    virtual ~ProtocolSoundBuilder() {}

public:
    virtual ModemData *BuildGetMute();
    virtual ModemData *BuildSetMute(int muteMode);
    virtual ModemData *BuildSetVolume(int volume);
    virtual ModemData *BuildGetVolume(void);
    virtual ModemData *BuildSetAudioPath(int audiopath);
    virtual ModemData *BuildGetAudioPath(void);
    virtual ModemData *BuildSetMultiMic(int mode);
    virtual ModemData *BuildGetMultiMic(void);
    virtual ModemData *BuildSetAudioClock(int mode);
    virtual ModemData *BuildSetAudioLoopback(int onoff, int path);
    virtual ModemData *BuildSwitchVoiceCallAudio(BYTE siminfo);
    virtual ModemData *BuildSetWbAmrCapability(int wbamr_capa);
    virtual ModemData *BuildGetWbAmrCapability(void);
};

#endif /* __PROTOCOL_SOUND_BUILDER_H__ */
