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
 * protocolstkbuilder.h
 *
 *  Created on: 2014. 12. 2.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_STK_BUILDER_H__
#define __PROTOCOL_STK_BUILDER_H__

#include "protocolbuilder.h"

class ProtocolStkBuilder : public ProtocolBuilder
{
public:
    ProtocolStkBuilder() : ProtocolBuilder() {}
    virtual ~ProtocolStkBuilder() {}

public:
    virtual ModemData *BuildStkEnvelopeCommand(int nLength, BYTE *pEnvelopeCmd);
    virtual ModemData *BuildStkTerminalResponse(int nLength, BYTE *pTerminalRsp);
    virtual ModemData *BuildStkEnvelopeStatus(int nLength, BYTE *pEnvelopeStatus);

    virtual ModemData *BuildStkCallSetup(int nUserOper);
};


#endif /* __PROTOCOL_STK_BUILDER_H__ */
