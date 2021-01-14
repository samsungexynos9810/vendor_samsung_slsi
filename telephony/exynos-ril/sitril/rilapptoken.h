/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __RIL_APP_TOKEN_H__
#define __RIL_APP_TOKEN_H__

#include "rilapplicationcontext.h"

// RIL Application Token
class RilAppToken
{
public:
    RIL_Token t;
    RilApplicationContext *context;

    RilAppToken(RilApplicationContext *context, RIL_Token t) {
        this->context = context;
        this->t = t;
    }

public:
    static RilAppToken *NewInstance(RilApplicationContext *context, RIL_Token t) {
        return new RilAppToken(context, t);
    }
    static RilAppToken *NewInstance(RilApplicationContext *context) {
        return NewInstance(context, 0);
    }
};

#endif // __RIL_APP_TOKEN_H__
