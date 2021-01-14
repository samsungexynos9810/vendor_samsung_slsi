/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ril_service.h>
#include <radioconfig_service.h>

#include <map>
using namespace std;

#define NUM_ELEMS(a)     (sizeof (a) / sizeof (a)[0])

enum WakeType {DONT_WAKE, WAKE_PARTIAL};

namespace android {
typedef struct {
    int requestNumber;
    int (*responseFunction) (int slotId, int responseType, int token,
            RIL_Errno e, void *response, size_t responselen);
    WakeType wakeType;
} UnsolResponseInfo;

static CommandInfo s_commands[] = {
#include "ril_commands.h"
#include "radio/1.2/ril_commands.h"
#include "radio/1.3/ril_commands.h"
#include "radio/1.4/ril_commands.h"
#include "radioconfig/ril_commands.h"
#include "slsi/ril_ext_commands.h"
};

static UnsolResponseInfo s_unsolResponses[] = {
#include "ril_unsol_commands.h"
#include "radio/1.2/ril_unsol_commands.h"
#include "radio/1.4/ril_unsol_commands.h"
#include "radioconfig/ril_unsol_commands.h"
#include "slsi/ril_ext_unsol_commands.h"
};

static bool debug = false;

class CommandInfoTable {
private:
    map<int,int> mCommandInfo;
    map<int,int> mUnsolRespInfo;
public:
    CommandInfoTable();
public:
    CommandInfo *getCommandInfo(int request);
    UnsolResponseInfo *getUnsolRespInfo(int unsolResponse);
};
static CommandInfoTable s_instance;

CommandInfoTable::CommandInfoTable() {
    int32_t size = 0;
    size = (int32_t)NUM_ELEMS(s_commands);
    RLOGI("s_commands size: %d", size);
    for (int32_t i = 0; i < size; i++) {
        mCommandInfo[s_commands[i].requestNumber] = i;
        if (debug) {
            RLOGI("mapping: mCommandInfo[%d] = %d", s_commands[i].requestNumber, i);
        }
    } // end for i ~

    size = (int32_t)NUM_ELEMS(s_unsolResponses);
    RLOGI("s_unsolResponses size: %d", size);
    for (int32_t i = 0; i < size; i++) {
        mUnsolRespInfo[s_unsolResponses[i].requestNumber] = i;
        if (debug) {
            RLOGI("mapping: mUnsolRespInfo[%d] = %d", s_unsolResponses[i].requestNumber, i);
        }
    } // end for i ~
}

CommandInfo *CommandInfoTable::getCommandInfo(int request) {
    bool found = false;
    map<int, int>::iterator iter = mCommandInfo.find(request);
    if (iter != mCommandInfo.end()) {
        found = true;
    }
    else {
        int decodeId = DECODE_REQUEST(request);
        if (DECODE_HAL(request) > HAL_VERSION_CODE(1,0)) {
            iter = mCommandInfo.find(decodeId);
            if (iter != mCommandInfo.end()) {
                found = true;
            }
        }
    }

    if (found) {
        int size = NUM_ELEMS(s_commands);
        int p = iter->second;
        if (p >= 0 && p < size) {
            RLOGI("%s found {request=%d halVer=%02X p=%d}", __FUNCTION__,
                    DECODE_REQUEST(request), DECODE_HAL(request), p);
            return &s_commands[p];
        }
    }

    RLOGI("%s not found(request=%d halVer=%02X)", __FUNCTION__,
            DECODE_REQUEST(request), DECODE_HAL(request));
    return NULL;
}

UnsolResponseInfo *CommandInfoTable::getUnsolRespInfo(int unsolResponse) {
    bool found = false;
    map<int, int>::iterator iter = mUnsolRespInfo.find(unsolResponse);
    if (iter != mUnsolRespInfo.end()) {
        found = true;
    }
    else {
        int decodeId = DECODE_REQUEST(unsolResponse);
        if (DECODE_HAL(unsolResponse) > HAL_VERSION_CODE(1,0)) {
            iter = mUnsolRespInfo.find(decodeId);
            if (iter != mUnsolRespInfo.end()) {
                found = true;
            }
        }
    }

    if (found) {
        int size = NUM_ELEMS(s_unsolResponses);
        int p = iter->second;
        if (p >= 0 && p < size) {
            RLOGI("%s found {unsolResponse=%d halVer=%02X p=%d}", __FUNCTION__,
                    DECODE_REQUEST(unsolResponse), DECODE_HAL(unsolResponse), p);
            return &s_unsolResponses[p];
        }
    }
    RLOGI("%s not found(unsolResponse=%d halVer=%02X)", __FUNCTION__,
            DECODE_REQUEST(unsolResponse), DECODE_HAL(unsolResponse));
    return NULL;
}

CommandInfo *getCommandInfo(int request) {
    return s_instance.getCommandInfo(request);
}

UnsolResponseInfo *getUnsolRespInfo(int unsolResponse) {
    return s_instance.getUnsolRespInfo(unsolResponse);
}
}
