/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <errno.h>
#include <pthread.h>
#include <log/log.h>

#include "thread.h"

Thread::Thread(Runnable *pRunnable/* = NULL*/)
{
    m_tid = 0;
    m_bStarted = false;
    m_pRunnable = pRunnable;
}

Thread::~Thread()
{
    Stop();
}

int Thread::Start()
{
    int ret = pthread_create((pthread_t *)&m_tid, NULL, &Thread::ThreadProc, this);
    if (ret == 0) {
        m_bStarted = true;
    }

    return ret;
}

int Thread::Stop()
{
    if (m_bStarted == false) {
        ALOGW("Thread not started");
        return -1;
    }

    // TODO kill the running thread

    // waiting until thread killed
    pthread_join(m_tid, NULL);

    m_tid = 0;
    m_bStarted = false;

    return 0;
}

void Thread::Run()
{
    if (m_pRunnable != NULL) {
        m_pRunnable->Run();
    }
}

/**
 * Thread procedure
 */
void *Thread::ThreadProc(void *arg)
{
    Thread *t = (Thread *)arg;
    if (t != NULL) {
        t->Run();
    }
    return NULL;
}
