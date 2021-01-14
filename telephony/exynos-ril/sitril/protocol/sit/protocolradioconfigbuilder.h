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
 * protocolradioconfigbuilder.h
 *
 *  Created on: 2019. 8. 13.
 */

#ifndef __PROTOCOL_RADIO_CONFIG_BUILDER_H__
#define __PROTOCOL_RADIO_CONFIG_BUILDER_H__

#include "protocolbuilder.h"

class ProtocolRadioConfigBuilder : public ProtocolBuilder {
public:
    ModemData *BuildGetPhoneCapability();
};

#endif /* __PROTOCOL_RADIO_CONFIG_BUILDER_H__ */
