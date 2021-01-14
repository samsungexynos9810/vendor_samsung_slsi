/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "tokengen.h"

#define    MAX_INT        (0xFFFFFFFF)

//////////////////////////////////////////////////////////
// global and static
//////////////////////////////////////////////////////////
TokenGen *TokenGen::instance = NULL;


// @waring
// thread unsafe. Init should be called during RIL initializing
void TokenGen::Init()
{
    if (instance == NULL) {
        instance = new TokenGen();
    }
}

TokenGen* TokenGen::GetInstacne()
{
    return instance;
}

void TokenGen::Release()
{
    if (instance != NULL) {
        delete instance;
        instance = NULL;
    }
}

TokenGen::TokenGen()
{
    m_nToken = 0;
}

//////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////
RCM_TOKEN TokenGen::GetNext()
{
    m_lock.lock();
    unsigned int ret = m_nToken;
    if (++m_nToken == MAX_INT)
        m_nToken = 0;
    m_lock.unlock();
    return ret;
}

