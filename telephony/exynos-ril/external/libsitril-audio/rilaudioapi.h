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
 * rilaudioapi.h
 *
 * @version 1.0.0
 * @history
 *  Created on: 2015. 4. 30.
 *  30/04/2015
 *   - re-design RIL audio API base on OOP
 *  18/05/2015
 *   - define SetAudioClock function
 *
 */

#ifndef __RIL_AUDIO_API_H__
#define __RIL_AUDIO_API_H__

// error codes
enum {
    RILAUDIO_ERROR_NONE,
    RILAUDIO_ERROR_NO_FILE,
    RILAUDIO_ERROR_NO_LIB_OPEN_FAIL,
    RILAUDIO_ERROR_SEND_FAIL,
    RILAUDIO_ERROR_NO_DEVICE,
    RILAUDIO_ERROR_INVALID_PARAM,
    RILAUDIO_ERROR_REGISTERATION_FAIL,
    RILAUDIO_ERROR_ALREADY_REGISTERD,
    RILAUDIO_ERROR_MAX
};

// audio path
enum
{
    RILAUIDO_PATH_NONE = 0,
    RILAUIDO_PATH_HANDSET = 1,
    RILAUIDO_PATH_HEADSET = 2,
    RILAUIDO_PATH_HANDSFREE = 3,
    RILAUIDO_PATH_BLUETOOTH = 4,
    RILAUIDO_PATH_STEREO_BLUETOOTH = 5,
    RILAUIDO_PATH_SPEAKRERPHONE = 6,
    RILAUIDO_PATH_35PI_HEADSET = 7,
    RILAUIDO_PATH_BT_NS_EC_OFF = 8,
    RILAUIDO_PATH_WB_BLUETOOTH = 9,
    RILAUIDO_PATH_WB_BT_NS_EC_OFF = 10,
    RILAUIDO_PATH_HANDSET_HAC = 11,
    RILAUDIO_PATH_LINEOUT = 12,
    RILAUIDO_PATH_VOLTE_HANDSET = 65,
    RILAUIDO_PATH_VOLTE_HEADSET = 66,
    RILAUIDO_PATH_VOLTE_HFK = 67,
    RILAUIDO_PATH_VOLTE_BLUETOOTH = 68,
    RILAUIDO_PATH_VOLTE_STEREO_BLUETOOTH = 69,
    RILAUIDO_PATH_VOLTE_SPEAKRERPHONE = 70,
    RILAUIDO_PATH_VOLTE_35PI_HEADSET = 71,
    RILAUIDO_PATH_VOLTE_BT_NS_EC_OFF = 72,
    RILAUIDO_PATH_VOLTE_WB_BLUETOOTH = 73,
    RILAUIDO_PATH_VOLTE_WB_BT_NS_EC_OFF = 74,
    RILAUDIO_PATH_VOLTE_HANDSET_HAC = 75,
    RILAUDIO_PATH_VOLTE_LINEOUT = 76,
    RILAUIDO_PATH_MAX
};

// Multi MIC control
enum {
    RILAUDIO_MULTI_MIC_OFF,
    RILAUDIO_MULTI_MIC_ON,
};

// Volume
enum {
    RILAUDIO_VOLUME_INVALID = -1,
    RILAUDIO_VOLUME_LEVEL0 = 0,
    RILAUDIO_VOLUME_LEVEL1,
    RILAUDIO_VOLUME_LEVEL2,
    RILAUDIO_VOLUME_LEVEL3,
    RILAUDIO_VOLUME_LEVEL4,
    RILAUDIO_VOLUME_LEVEL5,
    RILAUDIO_VOLUME_LEVEL_MAX = RILAUDIO_VOLUME_LEVEL5,
};

// Mute
enum {
    RILAUDIO_MUTE_DISABLED,
    RILAUDIO_MUTE_ENABLED,
};

// Audio clock
enum {
    RILAUDIO_TURN_OFF_I2S,
    RILAUDIO_TURN_ON_I2S,
};

// Audio loopback
enum {
    RILAUDIO_LOOPBACK_STOP,
    RILAUDIO_LOOPBACK_START,
};

enum {
    RILAUDIO_LOOPBACK_PATH_NA = 0,    //0: N/A
    RILAUDIO_LOOPBACK_PATH_HANDSET = 1,    //1: handset
    RILAUDIO_LOOPBACK_PATH_HEADSET = 2,    //2: headset
    RILAUDIO_LOOPBACK_PATH_HANDSFREE = 3,    //3: handsfree
    RILAUDIO_LOOPBACK_PATH_BT = 4,    //4: Bluetooth
    RILAUDIO_LOOPBACK_PATH_STEREO_BT = 5,    //5: stereo Bluetooth
    RILAUDIO_LOOPBACK_PATH_SPK = 6,    //6: speaker phone
    RILAUDIO_LOOPBACK_PATH_35PI_HEADSET = 7,    //7: 3.5pi headset
    RILAUDIO_LOOPBACK_PATH_BT_NS_EC_OFF = 8,    //8: BT NS/EC off
    RILAUDIO_LOOPBACK_PATH_WB_BT = 9,    //9: WB Bluetooth
    RILAUDIO_LOOPBACK_PATH_WB_BT_NS_EC_OFF = 10,    //10: WB BT NS/EC
    RILAUDIO_LOOPBACK_PATH_HANDSET_HAC = 11,    //11: handset HAC
    RILAUDIO_LOOPBACK_PATH_LINEOUT = 12,

    RILAUDIO_LOOPBACK_PATH_VOLTE_HANDSET = 65,  //65: VOLTE handset
    RILAUDIO_LOOPBACK_PATH_VOLTE_HEADSET = 66,  //66: VOLTE headset
    RILAUDIO_LOOPBACK_PATH_VOLTE_HANDSFREE = 67,    //67: VOLTE hands
    RILAUDIO_LOOPBACK_PATH_VOLTE_BT = 68,   //68: VOLTE Bluetooth
    RILAUDIO_LOOPBACK_PATH_VOLTE_STEREO_BT = 69,    //69: VOLTE stere
    RILAUDIO_LOOPBACK_PATH_VOLTE_SPK = 70,  //70: VOLTE speaker phone
    RILAUDIO_LOOPBACK_PATH_VOLTE_35PI_HEADSET = 71, //71: VOLTE 3.5pi
    RILAUDIO_LOOPBACK_PATH_VOLTE_BT_NS_EC_OFF = 72, //72: VOLTE BT NS
    RILAUDIO_LOOPBACK_PATH_VOLTE_WB_BT = 73,    //73: VOLTE WB Blueto
    RILAUDIO_LOOPBACK_PATH_VOLTE_WB_BT_NS_EC_OFF = 74,  //74: VOLTE W
    RILAUDIO_LOOPBACK_PATH_VOLTE_HANDSET_HAC = 75,
    RILAUDIO_LOOPBACK_PATH_VOLTE_LINEOUT = 76,

    RILAUDIO_LOOPBACK_PATH_HEADSET_MIC1 = 129,  //129: Headset ? MIC1
    RILAUDIO_LOOPBACK_PATH_HEADSET_MIC2 = 130,  //130: Headset ? MIC2
    RILAUDIO_LOOPBACK_PATH_HEADSET_MIC3 = 131,  //131: Headset ? MIC3
    RILAUDIO_LOOPBACK_PATH_MAX,
};

enum {
    TTY_MODE_OFF,
    TTY_MODE_FULL,
    TTY_MODE_HCO,   // Hearing carryover
    TTY_MODE_VCO,   // Voice carryover
};

// event
#define RILAUDIO_EVENT_BASE                     10000

/**
 * RILAUDIO_EVENT_RINGBACK_STATE_CHANGED
 *
 * indicates in-band(network) start/stop information
 * "data" is an int *
 * ((int *)data)[0] ringback tone status. 0 stop / 1 start
 * ((int *)data)[1] reserved
 */
#define RILAUDIO_EVENT_RINGBACK_STATE_CHANGED   (RILAUDIO_EVENT_BASE + 1)

/**
 * RILAUDIO_EVENT_IMS_SRVCC_HANDOVER
 *
 */
#define RILAUDIO_EVENT_IMS_SRVCC_HANDOVER       (RILAUDIO_EVENT_BASE + 2)

/**
 * RILAUDIO_EVENT_WB_AMR_REPORT
 *
 * "data" is an int *
 * ((int *)data)[0] audio quality.
 *          0 - Narrow Band
 *          1 - Wide Band
 *          8 - Super WB
 *          9 - Full Band
 * ((int *)data)[1] call type.
 *          0 - Unknown
 *          1 - GSM voice call case
 *          2 - CDMA Voice call case
 *          4 - IMS call case
 *          8 - Etc
 */
#define RILAUDIO_EVENT_WB_AMR_REPORT            (RILAUDIO_EVENT_BASE + 3)

#ifdef __cplusplus
extern "C" {
#endif

typedef void * HANDLE;
typedef int (*RILAUDIO_EventCallback)(HANDLE handle, int event, const void *data, unsigned int datalen);

/**
 * Open
 * @deprecated
 * @description open communication channel between Audio HAL and RIL
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int Open();

/**
 * RilAudioOpen
 * @description open communication channel between Audio HAL and RIL
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int RilAudioOpen();

/**
 * Close
 * @deprecated
 * @description close communication channel.
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int Close();

/**
 * RilAudioClose
 * @description close communication channel.
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int RilAudioClose();

/**
 * RegisterEventCallback
 * @description register event handler which an RIL audio event will be occurred from RIL.
 * @param handle audio device handle
 *        callback event handler to be invoked
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int RegisterEventCallback(HANDLE handle, RILAUDIO_EventCallback callback);

/**
 * SetAudioVolume
 * @description controls volume level.
 * @param volume a level of volume. max value is RILAUDIO_VOLUME_LEVEL5.
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int SetAudioVolume(int volume);

/**
 * SetAudioPath
 * @description set audio path.
 * @param audiopath a type of audio path
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int SetAudioPath(int audiopath);

/**
 * SetMultiMic
 * @description control multi-mic state.
 * @param mode RILAUDIO_MULTI_MIC_OFF/RILAUDIO_MULTI_MIC_ON
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int SetMultiMic(int mode);

/**
 * SetMute
 * @description control mute state
 * @param mode RILAUDIO_MUTE_DISABLED/RILAUDIO_MUTE_ENABLED
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int SetMute(int mode);

/**
 * SetAudioClock
 * @description set audio path.
 * @param mode RILAUDIO_TURN_OFF_I2S/RILAUDIO_TURN_ON_I2S
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int SetAudioClock(int mode);

/**
 * SetAudioLoopback
 * @description set the audio input/output path for loop back test.
 * @param onoff RILAUDIO_LOOPBACK_STOP/RILAUDIO_LOOPBACK_START
 * @param path ...
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int SetAudioLoopback(int onoff, int path);

/**
 * SetTtyMode
 * @description set TTY mode.
 * @param ttyMode is
 *   ttyMode == is 0 for TTY off
 *   ttyMode is == 1 for TTY Full
 *   ttyMode is == 2 for TTY HCO (hearing carryover)
 *   ttyMode is == 3 for TTY VCO (voice carryover)
 * @return returns RILAUDIO_ERROR_NONE success, otherwise error
 */
int SetTtyMode(int ttyMode);

#ifdef __cplusplus
};
#endif


#endif /* __RIL_AUDIO_API_H__ */
