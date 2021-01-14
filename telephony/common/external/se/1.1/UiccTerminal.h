/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef UICC_TERMINAL_H
#define UICC_TERMINAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"

struct UiccTerminal {
    UiccTerminal();
    ~UiccTerminal();

    E_STATUS openLogicalChannel(openChannelResponse *pResponse, lengthData *aid, int p2);
    E_STATUS closeLogicalChannel(int channel);
    E_STATUS internalTransmit(transmitApduBasicResponse *pResponse, trasmitLengthData *command);
    E_STATUS transmitApduBasicChannel(transmitApduBasicResponse *pResponse, int cla, int ins, int p1, int p2, int p3, trasmitLengthData *pData);
    E_STATUS transmitApduLogicalChannel(transmitApduChannelResponse *pResponse, int channel, int cla, int ins, int p1, int p2, int p3, trasmitLengthData *pData);
    E_STATUS getAtr(lengthData *pATR);
    bool isCardPresent(void);
    int parseChannelNumber(uint8_t cla);
    uint8_t setChannelToClassByte(uint8_t cla, int channelNumber);
    uint8_t clearChannelNumber(uint8_t cla);

private:
    lengthData mAtr;
    uint32_t mChannelIds[MAX_CHANNEL_NUM];
    uint8_t *mApduRequestBuffer = NULL;
    uint8_t *mApduResponseBuffer = NULL;
    int mClientId;
    bool mIsPresent;
};

#ifdef __cplusplus
};
#endif

#endif // #ifndef UICC_TERMINAL_H
