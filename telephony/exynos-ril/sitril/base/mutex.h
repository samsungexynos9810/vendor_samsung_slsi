/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>

class CMutex
{
private:
    CMutex(const CMutex &);
public:
    inline CMutex()
    {
        pthread_mutex_init(&m_mutex, NULL);
    }

    inline ~CMutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    inline void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    inline void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }

    const CMutex &operator=(const CMutex &);

private:
    pthread_mutex_t m_mutex;
};
#endif /*__MUTEX_H__*/
