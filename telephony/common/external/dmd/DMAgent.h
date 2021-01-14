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
 * DMAgent.h
 *
 *  Created on: 2018. 5. 18.
 */

#ifndef __DM_AGENT_H__
#define __DM_AGENT_H__

#include "DMConstants.h"

int DMAgent_Init();
int DMAgent_GetActiveMode();
void DMAgent_SetActiveMode(int activeMode);
int DMAgent_OnResponse(void *data, unsigned int datalen);

#endif /* __DM_AGENT_H__ */
