/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef    __TOKEN_GEN_H__
#define    __TOKEN_GEN_H__

#include "mutex.h"

typedef unsigned int RCM_TOKEN;

class TokenGen
{
private:
    CMutex            m_lock;
    unsigned int     m_nToken;

    // constructor
private:
    TokenGen();
public:
    ~TokenGen() {}

private:
    static TokenGen *instance;

public:
    static TokenGen* GetInstacne();
    static void Init();
    static void Release();

public:
    RCM_TOKEN GetNext();
};



#endif //__TOKEN_GEN_H__
