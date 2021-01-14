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
 * protocolvsimbuilder.h
 *
 *  Created on: 2016. 02. 26.
 */

#ifndef __PROTOCOL_VSIM_BUILDER_H__
#define __PROTOCOL_VSIM_BUILDER_H__

#include "protocolbuilder.h"

class ProtocolVsimBuilder : public ProtocolBuilder
{
public:
    ProtocolVsimBuilder() : ProtocolBuilder() {}
    virtual ~ProtocolVsimBuilder() {}

public:
    virtual ModemData *BuildVsimNotification(int tid, int eventid, int simType);
    virtual ModemData *BuildVsimOperation(int tid, int eventid, int result, int dataLen, const char *data);
};

#endif /* __PROTOCOL_VSIM_BUILDER_H__ */
