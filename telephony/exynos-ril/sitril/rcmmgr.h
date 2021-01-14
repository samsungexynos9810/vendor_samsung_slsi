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
 * rcmmgr.h.h
 *
 *  Created on: 2014. 11. 18.
 *      Author: sungwoo48.choi
 */

#ifndef __RCM_MGR_H__
#define __RCM_MGR_H__

#include "rilcontext.h"
#include "thread.h"
#include "iochannel.h"

class RadioControlMessageMgr : public Runnable {
    DECLARE_MODULE_TAG()
private:
    RilContext *m_pRilContext;
    IoChannel *m_pIo;
    Thread *m_pReaderThread;
    bool mCrashExit;

    // Enable/Disable to print SIM IO
    int m_nPrintSimIo = 0;

    char *m_buffer;
    static const unsigned int MAX_READ_BUF_SIZE = (64 * 1024 + 32);    // max size 64 KB for supporting RCS multiframe packet

public:
    RadioControlMessageMgr(RilContext *pRilContext, IoChannel *io);
    virtual ~RadioControlMessageMgr();

public:
    INT32 Read(char *buf, int size);
    INT32 Write(char *buf, int size);
    INT32 DumpData(char *buffer, UINT32 bufLen, BOOL rx);
    int Start();
    void Run();

    void StopRcmMonitoring() { mCrashExit = true; }
};


#endif /* __RCM_MGR_H__ */
