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
 * protocolradioconfigadapter.h
 *
 *  Created on: 2019. 8. 13.
 */

#ifndef __PROTOCOL_RADIO_CONFIG_ADAPTER_H__
#define __PROTOCOL_RADIO_CONFIG_ADAPTER_H__

#include "protocoladapter.h"

class ProtocolPhoneCapabilityAdapter : public ProtocolRespAdapter {
private:
    int mLogicalModemList[4];
public:
    ProtocolPhoneCapabilityAdapter(const ModemData *pModemData);
public:
    int GetMaxActiveData() const;
    int GetMaxActiveInternetData() const;
    bool IsInternetLingeringSupported() const;
    int GetLogicalModemListSize() const;
    int *GetLogicalModemList();
};

#endif /* __PROTOCOL_RADIO_CONFIG_ADAPTER_H__ */
