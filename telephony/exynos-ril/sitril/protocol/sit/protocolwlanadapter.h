/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __PROTOCOL_WLAN_ADAPTER_H__
#define __PROTOCOL_WLAN_ADAPTER_H__

#include "protocoladapter.h"

class ProtocolWLanSimAuthenticate : public ProtocolRespAdapter {
public:
    ProtocolWLanSimAuthenticate(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    BYTE GetAuthType() const;
    BYTE GetAuthLen() const;
    BYTE GetAuth(BYTE *) const;
};

#endif /* __PROTOCOL_WLAN_ADAPTER_H__ */
