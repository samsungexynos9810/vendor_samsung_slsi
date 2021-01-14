/*
 * cOPyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#define LOG_TAG "UiccHal-UiccTerminal"

#include <stdio.h>
#include <string.h>

#include <utils/Log.h>
#include <cutils/properties.h>
#include "UiccTerminal.h"
#include "Interface.h"

#define PROPERTY_MULTI_SIM             "persist.radio.multisim.config"

#define LogD(format, ...)    ALOGD("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogE(format, ...)    ALOGE("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogW(format, ...)    ALOGW("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogI(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogV(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)

#define ENTER_FUNC()        { ALOGI("%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { ALOGI("%s() [--> ", __FUNCTION__); }


UiccTerminal::UiccTerminal() {
    ENTER_FUNC();

    memset(mChannelIds, 0x00, sizeof(int)*MAX_CHANNEL_NUM);
    memset(&mAtr, 0x00, sizeof(lengthData));

    int SimSlotNum = 1;
    char prop[100];
    memset(prop, 0, sizeof(prop));

    mApduRequestBuffer = new uint8_t[MAX_BUFFER_SIZE];
    if(mApduRequestBuffer == NULL) {
        LogE("Memory allocation failed for mApduResquestBuffer");
        goto return_fail;
    }

    mApduResponseBuffer = new uint8_t[MAX_BUFFER_SIZE];
    if(mApduResponseBuffer == NULL) {
        LogE("Memory allocation failed for mApduResponseBuffer");
        goto return_fail;
    }

    if (SITRIL_SE_ERROR_NONE != (SitRilSeError)rilOpen()) {
        LEAVE_FUNC();
        LogE("ERROR: rilOpen");
        goto return_fail;
    }

    // Set number of SIM slot
    property_get(PROPERTY_MULTI_SIM, prop , "");
    if (strcmp(prop, "dsds")==0) {
        SimSlotNum = 2;
    }
    LogD("Number of SIM slot: %d", SimSlotNum);

    mClientId = 0;
    for(int i = 0; i < SimSlotNum; i++) {
        if (SITRIL_SE_ERROR_NONE != (SitRilSeError)rilSetClient(i)) {
            LogE("ERROR: rilSetClient(%d)", i);
            continue;
        }

        mIsPresent = rilIsCardPresent(i);
        if (mIsPresent == true) {
            mClientId = i;
            break;
        }
    }

    LogI("mClient : %d, isPresent : %d", mClientId, mIsPresent);
    LEAVE_FUNC();
    return;

return_fail:
    if(mApduRequestBuffer != NULL) {
        delete mApduRequestBuffer;
        mApduRequestBuffer = NULL;
    }

    if(mApduResponseBuffer != NULL) {
        delete mApduResponseBuffer;
        mApduResponseBuffer = NULL;
    }

    LEAVE_FUNC();
    return;
}

UiccTerminal::~UiccTerminal() {
    memset(mChannelIds, 0x00, sizeof(int)*MAX_CHANNEL_NUM);
    memset(&mAtr, 0x00, sizeof(lengthData));

    if(mApduRequestBuffer != NULL) {
        delete mApduRequestBuffer;
        mApduRequestBuffer = NULL;
    }

    if(mApduResponseBuffer != NULL) {
        delete mApduResponseBuffer;
        mApduResponseBuffer = NULL;
    }

    rilClose();
}

E_STATUS UiccTerminal::getAtr(lengthData *pATR) {
    if (pATR == NULL) {
        return E_STATUS_FAIL;
    }

    if (mIsPresent == false) {
        LogE("Absent SIM");
        return E_STATUS_SIM_ABSENT;
    }

    if (0 != rilGetAtr(pATR)) {
        return E_STATUS_FAIL;
    }
    return E_STATUS_SUCCESS;
}

E_STATUS UiccTerminal::openLogicalChannel(openChannelResponse *response, lengthData *aid, int p2) {
    if (aid== NULL || response == NULL) {
        return E_STATUS_FAIL;
    }

    if (mIsPresent == false) {
        LogE("Absent SIM");
        return E_STATUS_SIM_ABSENT;
    }

    if (E_STATUS_SUCCESS != rilOpenLogicalChannel(response, aid, p2)) {
        return E_STATUS_FAIL;
    }

    return E_STATUS_SUCCESS;
}

E_STATUS UiccTerminal::internalTransmit(transmitApduBasicResponse *pResponse, trasmitLengthData *pCommand) {
    ENTER_FUNC();
    if (pResponse == NULL || pCommand == NULL) {
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    if (mIsPresent == false) {
        LogE("Absent SIM");
        LEAVE_FUNC();
        return E_STATUS_SIM_ABSENT;
    }

    int cla = pCommand->data[0] & 0xFF;
    int ins = pCommand->data[1] & 0xff;
    int p1 = pCommand->data[2] & 0xff;
    int p2 = pCommand->data[3] & 0xff;
    int p3 = 0;
    if (4 < pCommand->length) {
        p3 = pCommand->data[4] & 0xff;
    }

    trasmitLengthData *pData = (trasmitLengthData *) mApduRequestBuffer;
    memset(pData, 0x00, MAX_BUFFER_SIZE);

    if (5 < pCommand->length) {
        pData->length = (pCommand->length - 5);
        memcpy(&pData->data[0], &pCommand->data[5], pData->length); // 6.5.3 id 6
    }

    int channelNumber = parseChannelNumber(pCommand->data[0]);
    E_STATUS status = E_STATUS_FAIL;
    if (channelNumber == 0) {
        status = transmitApduBasicChannel(pResponse, cla, ins, p1, p2, p3, pData);
    } else {
        transmitApduChannelResponse *pApduChannelResponse = (transmitApduChannelResponse *) mApduResponseBuffer;
        if (pApduChannelResponse != NULL) {
            status = transmitApduLogicalChannel(pApduChannelResponse, channelNumber, cla, ins, p1, p2, p3, pData);
            if (status == E_STATUS_SUCCESS) {
                pResponse->rilErrno = pApduChannelResponse->rilErrno;
                pResponse->nResponseCount = pApduChannelResponse->nResponseCount;
                memcpy(&pResponse->aResponse[0], &pApduChannelResponse->aResponse[0], pResponse->nResponseCount);
            }
        }
    }

    LEAVE_FUNC();
    return status;
}

E_STATUS UiccTerminal::transmitApduBasicChannel(transmitApduBasicResponse *pResponse, int cla, int ins, int p1, int p2, int p3, trasmitLengthData *pData) {
    ENTER_FUNC();
    if (pResponse == NULL) {
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    if (mIsPresent == false) {
        LogE("Absent SIM");
        LEAVE_FUNC();
        return E_STATUS_SIM_ABSENT;
    }

    transmitApduChannelResponse *pAdpuChannelResponse = (transmitApduChannelResponse *) mApduResponseBuffer;
    if (pAdpuChannelResponse != NULL) {
        if (E_STATUS_SUCCESS != rilTransmitApduLogicalChannel(pAdpuChannelResponse, BASIC_CHANNEL, cla, ins, p1, p2, p3, pData)) {
            LEAVE_FUNC();
            return E_STATUS_FAIL;
        }

        pResponse->rilErrno = pAdpuChannelResponse->rilErrno;
        pResponse->nResponseCount = pAdpuChannelResponse->nResponseCount;
        memcpy(&pResponse->aResponse[0], pAdpuChannelResponse->aResponse, pResponse->nResponseCount);
    } else {
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    LEAVE_FUNC();
    return E_STATUS_SUCCESS;
}

E_STATUS UiccTerminal::transmitApduLogicalChannel(transmitApduChannelResponse *pResponse, int channel, int cla, int ins, int p1, int p2, int p3, trasmitLengthData *pData) {
    if (pResponse == NULL) {
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    if (mIsPresent == false) {
        LogE("Absent SIM");
        LEAVE_FUNC();
        return E_STATUS_SIM_ABSENT;
    }

    if (E_STATUS_SUCCESS != rilTransmitApduLogicalChannel(pResponse, channel, cla, ins, p1, p2, p3, pData)) {
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    LEAVE_FUNC();
    return E_STATUS_SUCCESS;
}

E_STATUS UiccTerminal::closeLogicalChannel(int channel) {
    if (mIsPresent == false) {
        LogE("Absent SIM");
        return E_STATUS_SIM_ABSENT;
    }

    if (E_STATUS_SUCCESS != rilCloseLogicalChannel(channel)) {
        return E_STATUS_FAIL;
    }

    return E_STATUS_SUCCESS;
}

bool UiccTerminal::isCardPresent(void) {
    mIsPresent = rilIsCardPresent(mClientId);
    return mIsPresent;
}

uint8_t UiccTerminal::clearChannelNumber(uint8_t cla) {
    bool isFirstInterindustryClassByteCoding = (cla & 0x40) == 0x00;
    if (isFirstInterindustryClassByteCoding) {
        return (uint8_t) (cla & 0xFC);
    }

    return (uint8_t) (cla & 0xF0);
}
int UiccTerminal::parseChannelNumber(uint8_t cla) {
    bool isFirstInterindustryClassByteCoding = (cla & 0x40) == 0x00;
    if (isFirstInterindustryClassByteCoding) {
        return cla & 0x03;
    }

    return (cla & 0x0F) + 4;
}

uint8_t UiccTerminal::setChannelToClassByte(uint8_t cla, int channelNumber) {
    if (channelNumber < 4) {
        // b7 = 0 indicates the first interindustry class byte coding
        cla = (uint8_t) ((cla & 0xBC) | channelNumber);
    } else if (channelNumber < 20) {
        // b7 = 1 indicates the further interindustry class byte coding
        bool isSM = (cla & 0x0C) != 0;
        cla = (uint8_t) ((cla & 0xB0) | 0x40 | (channelNumber - 4));
        if (isSM) {
            cla |= 0x20;
        }
    } else {
        LogE("IllegalArgumentException(\"Channel number must be within [0..19]\")");
    }

    return cla;
}

