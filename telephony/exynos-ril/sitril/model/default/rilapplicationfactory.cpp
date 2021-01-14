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
 * rilapplicationfactory.cpp
 *
 *  Created on: 2015. 7. 16.
 */

#include "rilapplicationfactory.h"

RilApplication *RilApplicationFactory::CreateRilApplication(const struct RIL_Env *pRilEnv)
{
    return new RilApplication(pRilEnv);
}
