/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __PROTOCOL_EMBMS_BUILDER_H__
#define __PROTOCOL_EMBMS_BUILDER_H__

#include <string.h>
#include "protocolbuilder.h"

class ProtocolEmbmsBuilder  : public ProtocolBuilder
{
private:
    char m_szTmgi[7];

private:
    char *getStringTypeTmgi(uint64_t tmgi);
public:
    ProtocolEmbmsBuilder() {}
    virtual ~ProtocolEmbmsBuilder() {}
public:
    virtual ModemData *BuildSetService(int netType);
    virtual ModemData *BuildSetSession(int state, uint64_t tmgi, int saiListLen, const uint32_t *pSaiList, int freqListLen, const uint32_t *pFreqList);
    virtual ModemData *BuildGetSessionList(int state);
    virtual ModemData *BuildSignalStrength();
    virtual ModemData *BuildNetworkTime();
};

#endif /* __PROTOCOL_EMBMS_BUILDER_H__ */

