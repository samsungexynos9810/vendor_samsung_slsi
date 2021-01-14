 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __RIL_APPLICATION_FACTORY_H__
#define __RIL_APPLICATION_FACTORY_H__

#include "rilapplication.h"

class RilApplicationFactory
{
public:
    /**
     * Create RilApplication instance.
     *
     * @return RilApplication instance. It can be different for each product.
     */
    static RilApplication *CreateRilApplication(const struct RIL_Env *pRilEnv);
};

#endif /* __RIL_APPLICATION_FACTORY_H__ */
