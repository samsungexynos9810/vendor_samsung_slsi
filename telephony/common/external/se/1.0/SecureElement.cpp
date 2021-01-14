/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#define LOG_TAG "UiccHal-SecureElement"

#include <utils/Log.h>
#include "SecureElement.h"

#define LogD(format, ...)    ALOGD("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogE(format, ...)    ALOGE("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogW(format, ...)    ALOGW("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogI(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogV(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)

#define ENTER_FUNC()        { ALOGD("%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { ALOGD("%s() [--> ", __FUNCTION__); }

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

sp<V1_0::ISecureElementHalCallback> SecureElement::mCallbackV1_0 = nullptr;

const char *SecureElement::statusToString(SecureElementStatus status) {
    switch(status) {
        case SecureElementStatus::SUCCESS:  return "SUCCESS";
        case SecureElementStatus::FAILED:   return "FAILED";
        case SecureElementStatus::CHANNEL_NOT_AVAILABLE: return "CHANNEL_NOT_AVAILABLE";
        case SecureElementStatus::NO_SUCH_ELEMENT_ERROR: return "NO_SUCH_ELEMENT_ERROR";
        case SecureElementStatus::UNSUPPORTED_OPERATION: return "UNSUPPORTED_OPERATION";
        case SecureElementStatus::IOERROR:  return "IOERROR";
        default: return "INVALID_STATUS";
    }
}

SecureElement::SecureElement() {
    mOpenedchannelCount = 0;
	mExternalProxyCookie = 0;
	memset(mOpenedChannels, 0, sizeof(mOpenedChannels));
	mUiccTerminal = NULL;

    mOpenedChannels[BASIC_CHANNEL] = true;
    mOpenedchannelCount++;
    mIsServiceDied = false;
    pthread_mutex_init(&mMutex, NULL);
}

SecureElement::~SecureElement() {
    pthread_mutex_destroy(&mMutex);
}

Return<void> SecureElement::init(
    const sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>&clientCallback) {

    ENTER_FUNC();

    if (clientCallback == nullptr) {
        LEAVE_FUNC();
        return Void();
    } else {
        mCallbackV1_0 = clientCallback;
        if (!mCallbackV1_0->linkToDeath(this, mExternalProxyCookie)) {
            LogE("Failed to register death notification");
        }
        mIsServiceDied = false;
    }

    // Avoid to initialize duplicate
    if(mUiccTerminal!=NULL)
    {
        LogE("It is already initialized");
        LEAVE_FUNC();
        return Void();
    }

    mApduRequestBuffer = new uint8_t[MAX_BUFFER_SIZE];
    if(mApduRequestBuffer == NULL) {
        LogE("Failed to allocation APDU request buffer");
        LEAVE_FUNC();
        return Void();
    }

    mApduResponseBuffer = new uint8_t[MAX_BUFFER_SIZE];
    if(mApduResponseBuffer == NULL) {
        LogE("Failed to allocation APDU response buffer");
        delete mApduRequestBuffer;
        mApduRequestBuffer = NULL;
        LEAVE_FUNC();
        return Void();
    }

    pthread_mutex_lock(&mMutex);
    mUiccTerminal = new UiccTerminal();
    pthread_mutex_unlock(&mMutex);
    if (mIsServiceDied == false) {
        clientCallback->onStateChange(true);
    }

    LEAVE_FUNC();
    return Void();
}

Return<void> SecureElement::getAtr(getAtr_cb _hidl_cb) {
    ENTER_FUNC();

    E_STATUS status = E_STATUS_SUCCESS;
    hidl_vec<uint8_t> response;
    lengthData atr;
    memset(&atr, 0x00, sizeof(lengthData));

    status = mUiccTerminal->getAtr(&atr);
    if (E_STATUS_SUCCESS != status) {
        LogE("internalGetAtr failed!!!");
    } else {
        response.resize(atr.length);
        memcpy(&response[0], atr.data, atr.length);
    }

    if (mIsServiceDied == false) {
        _hidl_cb(response);
    } else {
        LogD("Serivce Died : Can not call callback function");
    }

    LEAVE_FUNC();
    return Void();
}

Return<bool> SecureElement::isCardPresent() {
    ENTER_FUNC();
    bool ret = mUiccTerminal->isCardPresent();
    LEAVE_FUNC();
    return ret;
}

Return<void> SecureElement::transmit(const hidl_vec<uint8_t>& data,
                                     transmit_cb _hidl_cb) {
    ENTER_FUNC();

    E_STATUS status = E_STATUS_SUCCESS;
    hidl_vec<uint8_t> resBuf;
    memset(&resBuf, 0x00, sizeof(resBuf));

    trasmitLengthData *pCommand = (trasmitLengthData *) mApduRequestBuffer;
    transmitApduBasicResponse *pResponse = (transmitApduBasicResponse *) mApduResponseBuffer;
    if (pCommand == NULL || pResponse == NULL) {
        if (pCommand == NULL) LogE("pCommand is NULL!!!");
        if (pResponse == NULL) LogE("pResponse is NULL!!!");
        goto exitTransmit;
    }

    memset(pCommand, 0x00, MAX_BUFFER_SIZE);
    memset(pResponse, 0x00, MAX_BUFFER_SIZE);

    pCommand->length = data.size();

    if (MIN_APDU_SIZE <= pCommand->length) {
        memcpy(&pCommand->data[0], data.data(), pCommand->length);
        status = mUiccTerminal->internalTransmit(pResponse, pCommand);
    }

    if (E_STATUS_SUCCESS != status) {
        LogE("transmit failed!!!");
    } else {
        uint8_t sessionId = (uint8_t)mUiccTerminal->parseChannelNumber(pCommand->data[0]);
        uint8_t sw1 = pResponse->aResponse[pResponse->nResponseCount-2] & 0xFF;
        uint8_t sw2 = pResponse->aResponse[pResponse->nResponseCount-1] & 0xFF;

        // when sw is 62 xx, 63 xx and command is "SELECT" command
        // if data is only status word, request GET_RESPONSE
        if ((pCommand->data[1] == 0xA4 && (sw1 == 0x62 || sw1 == 0x63)) && (pResponse->nResponseCount < 3)) {
            trasmitLengthData *pTransmitCmd = (trasmitLengthData *) mApduRequestBuffer;
            transmitApduBasicResponse *pTransmitRsp = (transmitApduBasicResponse *) mApduResponseBuffer;

            if (pTransmitCmd != NULL && pTransmitRsp != NULL) {
                memset(pTransmitCmd, 0x00, MAX_BUFFER_SIZE);
                memset(pTransmitRsp, 0x00, MAX_BUFFER_SIZE);

                pTransmitCmd->data[0] = mUiccTerminal->setChannelToClassByte(0x00, sessionId);
                pTransmitCmd->data[1] = 0xc0;
                pTransmitCmd->data[2] = 0x00;
                pTransmitCmd->data[3] = 0x00;
                pTransmitCmd->data[4] = 0x00;
                pTransmitCmd->length = 5;

                status = mUiccTerminal->internalTransmit(pTransmitRsp, pTransmitCmd);
                if (E_STATUS_SUCCESS == status) {
                    int tmpSw1 = pTransmitRsp->aResponse[pTransmitRsp->nResponseCount-2];
                    int tmpSw2 = pTransmitRsp->aResponse[pTransmitRsp->nResponseCount-1];

                    if (2 < pTransmitRsp->nResponseCount && (tmpSw1 == 0x90 && tmpSw2 == 0x00)) {
                        resBuf.resize(pTransmitRsp->nResponseCount);
                        memcpy(&resBuf[0], pTransmitRsp->aResponse, (pTransmitRsp->nResponseCount-2));
                        resBuf[pTransmitRsp->nResponseCount-2] = sw1;
                        resBuf[pTransmitRsp->nResponseCount-1] = sw2;
                    } else {
                        resBuf.resize(2);
                        resBuf[0] = sw1;
                        resBuf[1] = sw2;
                    }
                } else {
                    resBuf.resize(2);
                    resBuf[0] = sw1;
                    resBuf[1] = sw2;
                }
                delete pTransmitCmd; pTransmitCmd = NULL;
            }
        } else {
            resBuf.resize(pResponse->nResponseCount);
            memcpy(&resBuf[0], pResponse->aResponse, pResponse->nResponseCount);
        }
    }

exitTransmit:

    if (mIsServiceDied == false) {
        _hidl_cb(resBuf);
    } else {
        LogD("Serivce Died : Can not call callback function");
    }

    LEAVE_FUNC();
    return Void();
}

Return<void> SecureElement::openLogicalChannel(const hidl_vec<uint8_t>& aid,
                                               uint8_t p2,
                                               openLogicalChannel_cb _hidl_cb) {
    ENTER_FUNC();

    LogicalChannelResponse resBuf;
    memset(&resBuf, 0x00, sizeof(resBuf));
    resBuf.channelNumber = 0xff;

    SecureElementStatus sestatus = SecureElementStatus::IOERROR;
    E_STATUS status = E_STATUS_FAIL;
    openChannelResponse openRsp;
    lengthData AID;

    memset(&AID, 0x00, sizeof(lengthData));
    memset(&openRsp, 0x00, sizeof(openChannelResponse));

    AID.length = aid.size();
    memcpy(&AID.data[0], aid.data(), aid.size());

    if (AID.length == 0) {
        if (isCardPresent() == true) {
            LogI("AID is null, return Basic channel with 9000");
            // AID is null, return Basic channel with 9000
            sestatus = SecureElementStatus::SUCCESS;
            resBuf.channelNumber = (uint8_t)BASIC_CHANNEL;
            resBuf.selectResponse.resize(2);
            resBuf.selectResponse[0] = 0x90;
            resBuf.selectResponse[1] = 0x00;
        } else {
            sestatus = SecureElementStatus::CHANNEL_NOT_AVAILABLE;
        }

        LogD("return sestatus : %s", statusToString(sestatus));
        if (mIsServiceDied == false) {
            _hidl_cb(resBuf, sestatus);
        } else {
            LogD("Serivce Died : Can not call callback function");
        }

        LEAVE_FUNC();
        return Void();
    }

    status = mUiccTerminal->openLogicalChannel(&openRsp, &AID, p2);
    if (status != E_STATUS_SUCCESS) {
        /*Transceive failed*/
        if (status == E_STATUS_SIM_ABSENT) {
            sestatus = SecureElementStatus::CHANNEL_NOT_AVAILABLE;
        } else {
            sestatus = SecureElementStatus::IOERROR;
        }
    } else {
        if (openRsp.rilErrno == RIL_E_MISSING_RESOURCE) {
            sestatus = SecureElementStatus::CHANNEL_NOT_AVAILABLE;
        } else if (openRsp.rilErrno == RIL_E_NO_SUCH_ELEMENT) {
            sestatus = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
            LogD("SecureElementStatus::NO_SUCH_ELEMENT_ERROR");
        } else {
            uint8_t sw1 = openRsp.sw1;
            uint8_t sw2 = openRsp.sw2;

            LogD("Response sw1 : 0x%02x, sw2 : 0x%02x, sessionId : %d", sw1, sw2, openRsp.sessionId);

            // Return response on success, empty vector on failure
            // Status is success
            if (sw1 == 0x90 && sw2 == 0x00) {
                sestatus = SecureElementStatus::SUCCESS;
            }
            else if (sw1 == 0x6A && sw2 == 0x82) {
                sestatus = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
            } else if (sw1 == 0x69 && (sw2 == 0x85 || sw2 == 0x99)) {
                if (sw2 == 0x99) {
                    LogI("CRC2: NoSuchElementError");
                    LogI("if the AID on the SE is not available (or cannot be selected) or a logical channel is already open to a non-multiselectable applet.");
                } else if (sw2 == 0x85) {
                    LogI("Conditions of use not satisfied");
                }
                sestatus = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
            } else if ((sw1 == 0x6A && sw2 == 0x86) ||
                (((sw1 == 0x6E) || (sw1 == 0x6D)) && sw2 == 0x00)) {
                sestatus = SecureElementStatus::UNSUPPORTED_OPERATION;
            }

            if (sw1 == 0x62 || sw1 == 0x63) {
                sestatus = SecureElementStatus::SUCCESS;
            }

            // when sw is 62 xx, 63, data is only status word, request GET_RESPONSE
            if ((openRsp.nResponseCount < 3) && (sw1 == 0x62 || sw1 == 0x63)) {
                trasmitLengthData *pTransmitCmd = new trasmitLengthData();
                transmitApduBasicResponse *pTransmitRsp = new transmitApduBasicResponse();

                if (pTransmitCmd != NULL && pTransmitRsp != NULL) {
                    memset(pTransmitCmd, 0x00, sizeof(trasmitLengthData));
                    memset(pTransmitRsp, 0x00, sizeof(transmitApduBasicResponse));

                    pTransmitCmd->data[0] = mUiccTerminal->setChannelToClassByte(0x00, openRsp.sessionId);
                    pTransmitCmd->data[1] = 0xc0;
                    pTransmitCmd->data[2] = 0x00;
                    pTransmitCmd->data[3] = 0x00;
                    pTransmitCmd->data[4] = 0x00;
                    pTransmitCmd->length = 5;

                    status = mUiccTerminal->internalTransmit(pTransmitRsp, pTransmitCmd);
                    if (E_STATUS_SUCCESS == status) {
                        resBuf.channelNumber = (uint8_t)openRsp.sessionId;
                        int tmpSw1 = pTransmitRsp->aResponse[pTransmitRsp->nResponseCount-2];
                        int tmpSw2 = pTransmitRsp->aResponse[pTransmitRsp->nResponseCount-1];

                        if (2 < pTransmitRsp->nResponseCount && (tmpSw1 == 0x90 && tmpSw2 == 0x00)) {
                            resBuf.selectResponse.resize(pTransmitRsp->nResponseCount);
                            memcpy(&resBuf.selectResponse[0], pTransmitRsp->aResponse, (pTransmitRsp->nResponseCount-2));
                            resBuf.selectResponse[pTransmitRsp->nResponseCount-2] = sw1;
                            resBuf.selectResponse[pTransmitRsp->nResponseCount-1] = sw2;
                        } else {
                            resBuf.selectResponse.resize(2);
                            resBuf.selectResponse[0] = sw1;
                            resBuf.selectResponse[1] = sw2;
                        }
                    } else {
                        resBuf.selectResponse.resize(2);
                        resBuf.selectResponse[0] = sw1;
                        resBuf.selectResponse[1] = sw2;
                    }

                    delete pTransmitCmd; pTransmitCmd = NULL;
                    delete pTransmitRsp; pTransmitRsp = NULL;
                } else {
                    if (pTransmitCmd != NULL) {
                        LogE("pTransmitCmd is not NULL");
                        delete pTransmitCmd; pTransmitCmd = NULL;
                    }
                    if (pTransmitRsp != NULL) {
                        LogE("pTransmitRsp is not NULL");
                        delete pTransmitRsp; pTransmitRsp = NULL;
                    }
                }
            } else {
                resBuf.channelNumber = (uint8_t)openRsp.sessionId;
                if (0 < openRsp.nResponseCount) {
                    resBuf.selectResponse.resize(openRsp.nResponseCount);
                    memcpy(&resBuf.selectResponse[0], openRsp.aResponse, openRsp.nResponseCount);
                }
            }

            if (sestatus == SecureElementStatus::SUCCESS) {
                mOpenedChannels[resBuf.channelNumber] = true;
                mOpenedchannelCount++;
            }
        }
    }

    LogD("return sestatus : %s", statusToString(sestatus));
    if (mIsServiceDied == false) {
        _hidl_cb(resBuf, sestatus);
    } else {
        LogD("Serivce Died : Can not call callback function");
    }
    LEAVE_FUNC();
    return Void();
}

Return<void> SecureElement::openBasicChannel(const hidl_vec<uint8_t>& aid,
                                             uint8_t p2,
                                             openBasicChannel_cb _hidl_cb) {
    ENTER_FUNC();

    hidl_vec<uint8_t> resBuf;
    memset(&resBuf, 0x00, sizeof(resBuf));

    SecureElementStatus sestatus = SecureElementStatus::IOERROR;
    E_STATUS status = E_STATUS_FAIL;
    trasmitLengthData *pCommand = new trasmitLengthData();
    transmitApduBasicResponse *pResponse = new transmitApduBasicResponse();
    if (pCommand == NULL || pResponse == NULL) {
        if (pCommand == NULL) LogE("pCommand is NULL!!!");
        if (pResponse == NULL) LogE("pResponse is NULL!!!");
        goto exitOpenBasicChannel;
    }

    memset(pCommand, 0x00, sizeof(trasmitLengthData));
    memset(pResponse, 0x00, sizeof(transmitApduBasicResponse));

    pCommand->length= (int32_t)(5 + aid.size());
    pCommand->data[0] = BASIC_CHANNEL;          // basic channel
    pCommand->data[1] = (uint8_t)0xA4;          // INS
    pCommand->data[2] = 0x04;                   // P1
    pCommand->data[3] = p2;                     // P2
    pCommand->data[4] = (uint8_t)aid.size();    // Lc
    memcpy(&pCommand->data[5], aid.data(), aid.size());

    status = mUiccTerminal->internalTransmit(pResponse, pCommand);
    if (status != E_STATUS_SUCCESS) {
        /* Transceive failed */
        if (status == E_STATUS_SIM_ABSENT) {
            sestatus = SecureElementStatus::CHANNEL_NOT_AVAILABLE;
        } else {
            sestatus = SecureElementStatus::IOERROR;
        }
    } else {
        uint8_t sw1 = pResponse->aResponse[pResponse->nResponseCount - 2];
        uint8_t sw2 = pResponse->aResponse[pResponse->nResponseCount - 1];

        /*Return response on success, empty vector on failure*/
        /*Status is success*/
        LogD("Response sw1 : 0x%02x, sw2 : 0x%02x", sw1, sw2);

        if ((sw1 == 0x90) && (sw2 == 0x00)) {
            sestatus = SecureElementStatus::SUCCESS;
        }

        else if (sw1 == 0x6A && sw2 == 0x82) {
            sestatus = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
        } else if (sw1 == 0x69 && (sw2 == 0x85 || sw2 == 0x99)) {
            if (sw2 == 0x99) {
                LogI("CRC2: NoSuchElementError");
                LogI("if the AID on the SE is not available (or cannot be selected) or a logical channel is already open to a non-multiselectable applet.");
            } else if (sw2 == 0x85) {
                LogI("Conditions of use not satisfied");
            }
            sestatus = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
        } else if ((sw1 != 0x62 && sw1 != 0x63) && (sw1 != 0x90 || sw2 != 0x00)) {
            LogI("Secure Element cannot be selected");
            sestatus = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
        /*
        } else if ((sw1 == 0x6A && sw2 == 0x86) ||
            (((sw1 == 0x6E) || (sw1 == 0x6D)) && sw2 == 0x00)) {
            sestatus = SecureElementStatus::UNSUPPORTED_OPERATION;
        */
        }

        if ((sw1 == 0x62) || (sw1 == 0x63)) {
            sestatus = SecureElementStatus::SUCCESS;
        }

        // when sw is 62 xx, 63 xx and command is "SELECT" command
        // if data is only status word, request GET_RESPONSE
        if ((pCommand->data[1] == 0xA4 && (sw1 == 0x62 || sw1 == 0x63)) && (pResponse->nResponseCount < 3)) {
            trasmitLengthData *pTransmitCmd = new trasmitLengthData();
            transmitApduBasicResponse *pTransmitRsp = new transmitApduBasicResponse();

            if (pTransmitCmd != NULL && pTransmitRsp != NULL) {
                memset(pTransmitCmd, 0x00, sizeof(trasmitLengthData));
                memset(pTransmitRsp, 0x00, sizeof(transmitApduBasicResponse));

                pTransmitCmd->data[0] = mUiccTerminal->setChannelToClassByte(0x00, BASIC_CHANNEL);
                pTransmitCmd->data[1] = 0xc0;
                pTransmitCmd->data[2] = 0x00;
                pTransmitCmd->data[3] = 0x00;
                pTransmitCmd->data[4] = 0x00;
                pTransmitCmd->length = 5;

                status = mUiccTerminal->internalTransmit(pTransmitRsp, pTransmitCmd);
                if (E_STATUS_SUCCESS == status) {
                    int tmpSw1 = pTransmitRsp->aResponse[pTransmitRsp->nResponseCount-2];
                    int tmpSw2 = pTransmitRsp->aResponse[pTransmitRsp->nResponseCount-1];

                    if (2 < pTransmitRsp->nResponseCount && (tmpSw1 == 0x90 && tmpSw2 == 0x00)) {
                        resBuf.resize(pTransmitRsp->nResponseCount);
                        memcpy(&resBuf[0], pTransmitRsp->aResponse, (pTransmitRsp->nResponseCount-2));
                        resBuf[pTransmitRsp->nResponseCount-2] = sw1;
                        resBuf[pTransmitRsp->nResponseCount-1] = sw2;
                    } else {
                        resBuf.resize(2);
                        resBuf[0] = sw1;
                        resBuf[1] = sw2;
                    }
                } else {
                    resBuf.resize(2);
                    resBuf[0] = sw1;
                    resBuf[1] = sw2;
                }

                delete pTransmitCmd; pTransmitCmd = NULL;
                delete pTransmitRsp; pTransmitRsp = NULL;
            } else {
                if (pTransmitCmd != NULL) {
                    LogE("pTransmitCmd is not NULL");
                    delete pTransmitCmd; pTransmitCmd = NULL;
                }
                if (pTransmitRsp != NULL) {
                    LogE("pTransmitRsp is not NULL");
                    delete pTransmitRsp; pTransmitRsp = NULL;
                }
            }
        } else {
            if (0 < pResponse->nResponseCount) {
                resBuf.resize(pResponse->nResponseCount);
                memcpy(&resBuf[0], pResponse->aResponse, pResponse->nResponseCount);
            }
        }
    }

exitOpenBasicChannel:
    if (pCommand != NULL) {
        delete pCommand; pCommand = NULL;
    }

    if (pResponse != NULL) {
        delete pResponse; pResponse = NULL;
    }

    LogD("return sestatus : %s", statusToString(sestatus));
    if (mIsServiceDied == false) {
        _hidl_cb(resBuf, sestatus);
    } else {
        LogD("Serivce Died : Can not call callback function");
    }
    LEAVE_FUNC();
    return Void();
}

Return<::android::hardware::secure_element::V1_0::SecureElementStatus>
SecureElement::closeChannel(uint8_t channelNumber) {

    ENTER_FUNC();

    E_STATUS status = E_STATUS_FAIL;
    SecureElementStatus sestatus = SecureElementStatus::FAILED;

    LogI("channelNumber : %d", channelNumber);

    if (channelNumber == BASIC_CHANNEL) {
        sestatus = SecureElementStatus::SUCCESS;
        LogI("return sestatus : %s", statusToString(sestatus));
        LEAVE_FUNC();
        return sestatus;
    } else if (channelNumber >= MAX_CHANNEL_NUM) {
        LogE("invalid channel!!!");
        sestatus = SecureElementStatus::FAILED;
    } else if (mOpenedChannels[channelNumber] == false) {
        sestatus = SecureElementStatus::CHANNEL_NOT_AVAILABLE;
    } else if (channelNumber > BASIC_CHANNEL) {
        status = mUiccTerminal->closeLogicalChannel(channelNumber);

        if (status == E_STATUS_SUCCESS) {
            sestatus = SecureElementStatus::SUCCESS;
        } else {
            sestatus = SecureElementStatus::FAILED;
        }
    }

    if (sestatus == SecureElementStatus::SUCCESS) {
        mOpenedChannels[channelNumber] = false;
        mOpenedchannelCount--;
    }

    LogI("return sestatus : %s", statusToString(sestatus));
    LEAVE_FUNC();
    return sestatus;
}

void SecureElement::serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) {

    ENTER_FUNC();

    mIsServiceDied = true;
    if (mCallbackV1_0 != nullptr) {
        mCallbackV1_0->unlinkToDeath(this);
    }

    pthread_mutex_lock(&mMutex);
    if (mUiccTerminal != NULL) {
        delete mUiccTerminal;
        mUiccTerminal=NULL;
    }
    pthread_mutex_unlock(&mMutex);

    if (mApduResponseBuffer != NULL) {
        delete mApduResponseBuffer;
        mApduResponseBuffer = NULL;
    }

    if (mApduRequestBuffer != NULL) {
        delete mApduRequestBuffer;
        mApduRequestBuffer = NULL;
    }

    LEAVE_FUNC();
    return;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
