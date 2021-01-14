/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __PROTOCOL_GPS_BUILDER_H__
#define __PROTOCOL_GPS_BUILDER_H__

#include "protocolbuilder.h"
#include "rillog.h"

class ProtocolGpsBuilder  : public ProtocolBuilder
{
public:
    ProtocolGpsBuilder() {}
    virtual ~ProtocolGpsBuilder() {}
public:
    virtual ModemData *BuildAgpsPDU(int requestId, void *data, unsigned int datalen);
    virtual ModemData *BuildAgpsIndPDU(int requestId, void *data, unsigned int datalen);
};

#endif /* __PROTOCOL_GPS_BUILDER_H__ */
