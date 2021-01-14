/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
 
#define LOG_TAG "SE_INTERFACE"
#define LOG_NDEBUG 0

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include <utils/Log.h>
#include <cutils/properties.h>

#include "Interface.h"

#define LogD(format, ...)    ALOGD("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogE(format, ...)    ALOGE("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogW(format, ...)    ALOGW("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogI(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogV(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)

#define ENTER_FUNC()        { ALOGD("%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { ALOGD("%s() [--> ", __FUNCTION__); }

static struct ril_handle rilHandle;

/* Function pointers */
int (*_ril_open_client)(void);
int (*_ril_close_client)(void);
int (*_ril_open_logical_channel)(openChannelResponse *pResponse, lengthData *pAid, int p2);
int (*_ril_transmit_apdu_logical_channel)(transmitApduChannelResponse *pApdu, int channel, int cla, int instruction, int p1, int p2, int p3, trasmitLengthData *pData);
int (*_ril_transmit_apdu_basic_channel)(transmitApduBasicResponse *pApdu, int cla, int instruction, int p1, int p2, int p3, trasmitLengthData *pData);
int (*_ril_close_logical_channel)(int channel);
int (*_ril_get_atr)(lengthData *pAtr);
int (*_ril_is_card_present)(int socketId, int *pCardState);
int (*_ril_set_client)(int socketId);

static int ril_open(struct ril_handle *ril)
{
    if (ril == NULL) {
        return -1;
    }

    ril->handle = (void*)dlopen(RIL_CLIENT_LIBPATH, RTLD_NOW);
    if (ril->handle == NULL) {
        LogE("Cannot open '%s'", RIL_CLIENT_LIBPATH);
        return SITRIL_SE_ERROR_LIB_LOAD_FAIL;
    }

    _ril_open_client = (int (*)(void))dlsym(ril->handle, "Open");
    _ril_close_client = (int (*)(void))dlsym(ril->handle, "Close");
    _ril_open_logical_channel = (int (*)(openChannelResponse *, lengthData *, int))dlsym(ril->handle, "OpenLogicalChannel");
    _ril_transmit_apdu_logical_channel = (int (*)(transmitApduChannelResponse *, int, int, int, int, int, int, trasmitLengthData *))dlsym(ril->handle, "TransmitApduLogicalChannel");
    _ril_transmit_apdu_basic_channel = (int (*)(transmitApduBasicResponse *, int, int, int, int , int, trasmitLengthData *))dlsym(ril->handle, "TransmitApduBasicChannel");
    _ril_close_logical_channel = (int (*)(int))dlsym(ril->handle, "CloseLogicalChannel");
    _ril_get_atr = (int (*)(lengthData *))dlsym(ril->handle, "GetAtr");
    _ril_is_card_present = (int (*)(int, int *))dlsym(ril->handle, "IsCardPresent");
    _ril_set_client = (int (*)(int))dlsym(ril->handle, "SetSocketId");

    LogI(":::::::: HANDLE OBTAINING FOR ALL SMIRIL API DONE :::\n");
    if (!_ril_open_client || !_ril_close_client || !_ril_open_logical_channel
        || !_ril_transmit_apdu_logical_channel || !_ril_transmit_apdu_basic_channel || !_ril_close_logical_channel
        || !_ril_get_atr || !_ril_is_card_present || !_ril_set_client) {
        LogE("Cannot get symbols from '%s'", RIL_CLIENT_LIBPATH);
        dlclose(ril->handle);
        return -1;
    }

    if (_ril_open_client() != 0){
        LogE("ril_open_client() failed");
        dlclose(ril->handle);
        return -1;
    }

    return SITRIL_SE_ERROR_NONE;
}

static int ril_close(struct ril_handle *ril)
{
    if (!ril || !ril->handle){
	LogE(" Something missing try to close later \n");
        return -1;
    }

    if (_ril_close_client() != 0) {
        LogE("ril_close_client() failed");
        return -1;
    }

    dlclose(ril->handle);
    return 0;
}

E_STATUS rilOpen(void) {
    if (SITRIL_SE_ERROR_NONE != ril_open(&rilHandle)) {
        LogE("ERROR: ril_open");
        return E_STATUS_FAIL;
    }
    return E_STATUS_SUCCESS;
}

E_STATUS rilClose(void) {
    if (SITRIL_SE_ERROR_NONE != ril_close(&rilHandle)) {
        LogE("ERROR: ril_close");
        return E_STATUS_FAIL;
    }
    return E_STATUS_SUCCESS;
}

E_STATUS rilSetClient(int socketId) {
    if (SITRIL_SE_ERROR_NONE != _ril_set_client(socketId)) {
        LogE("ERROR: _ril_set_client");
        return E_STATUS_FAIL;
    }
    return E_STATUS_SUCCESS;
}

E_STATUS rilOpenLogicalChannel(openChannelResponse *pResponse, lengthData *pAid, int p2) {
    ENTER_FUNC();
    if (pResponse == NULL || pAid == NULL) {
        LogI("pResponse or pAid is null");
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    if (SITRIL_SE_ERROR_NONE != _ril_open_logical_channel(pResponse, pAid, p2)) {
        LogI("ERROR: _ril_open_logical_channel");
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }
    LEAVE_FUNC();
    return E_STATUS_SUCCESS;
}

E_STATUS rilTransmitApduLogicalChannel(transmitApduChannelResponse *pRespone, int channel, int cla, int instruction, int p1, int p2, int p3, trasmitLengthData *pData) {
    ENTER_FUNC();

    if (pRespone == NULL) {
        LogI("pResponse is null");
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    if (SITRIL_SE_ERROR_NONE != _ril_transmit_apdu_logical_channel(pRespone, channel, cla, instruction, p1, p2, p3, pData)) {
        LogI("ERROR: _ril_transmit_apdu_logical_channel");
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    LEAVE_FUNC();
    return E_STATUS_SUCCESS;
}

E_STATUS rilTransmitApduBasicChannel(transmitApduBasicResponse *pRespone, int cla, int instruction, int p1, int p2, int p3, trasmitLengthData *pData) {
    ENTER_FUNC();

    if (pRespone == NULL) {
        LogI("pResponse is null");
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    if (SITRIL_SE_ERROR_NONE != _ril_transmit_apdu_basic_channel(pRespone, cla, instruction, p1, p2, p3, pData)) {
        LogI("ERROR: _ril_transmit_apdu_basic_channel");
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    LEAVE_FUNC();
    return E_STATUS_SUCCESS;
}

E_STATUS rilCloseLogicalChannel(int channel) {
    ENTER_FUNC();

    if (SITRIL_SE_ERROR_NONE != _ril_close_logical_channel(channel)) {
        LogI("ERROR: _ril_close_logical_channel");
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    LEAVE_FUNC();
    return E_STATUS_SUCCESS;
}

E_STATUS rilGetAtr(lengthData *pAtr) {
    ENTER_FUNC();

    if (SITRIL_SE_ERROR_NONE != _ril_get_atr(pAtr)) {
        LogI("ERROR: _ril_get_atr");
        LEAVE_FUNC();
        return E_STATUS_FAIL;
    }

    LEAVE_FUNC();
    return E_STATUS_SUCCESS;
}

bool rilIsCardPresent(int socketId) {
    ENTER_FUNC();

    int result = -1;
    if (SITRIL_SE_ERROR_NONE != _ril_is_card_present(socketId, &result)) {
        LogI("ERROR: _ril_is_card_present");
        LEAVE_FUNC();
        return false;
    }

    LogI("result : %d", result);
    LEAVE_FUNC();
    return result;
}
