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
 * rillogcapture.h
 *
 *  Created on: 2018. 4. 24.
 *      Author: sungwoo48.choi
 */

#ifndef __RIL_LOG_CAPTURE_H__
#define __RIL_LOG_CAPTURE_H__

#define NAME_SIZE 100

#include "thread.h"

class RilLogCapture : public Runnable
{
    DECLARE_MODULE_TAG()
public:
    RilLogCapture();
    virtual ~RilLogCapture();
    int Start();
    int Init();
    void Run();
    static RilLogCapture *MakeInstance();
    int OpenMessagePipe();
    int notifyNewRilLog(const char *rilLogMsg);
    int getCurFileName();
    int setCurFileName(int num);
    int getFileSize();

private:
    Thread *m_pRilLogCaptureThread;
    int m_nRilLogPipeW;
    int m_nRilLogPipeR;
    static bool m_RilLogIsInited;
    char filestr[NAME_SIZE];
    char filelist[NAME_SIZE];

protected:
    virtual void OnClose();
    CMutex *m_pRilLogRMutex;
};

#endif /* __RIL_LOG_CAPTURE_H__ */
