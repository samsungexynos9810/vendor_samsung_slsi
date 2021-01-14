/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "servicemonitorrunnable.h"
#include "service.h"
#include "rillog.h"
#include "message.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

ServiceMonitorRunnable::ServiceMonitorRunnable(Service *pService)
{
    m_pSerV = pService;
}

ServiceMonitorRunnable::~ServiceMonitorRunnable()
{
}

void ServiceMonitorRunnable::Run()
{
    if (m_pSerV == NULL || m_pSerV->GetReadPipe() == -1) {
        RilLogE("Command pipe is not open");
        return;
    }

    // callback Service thread procedure started
    m_pSerV->OnStart();

    int cmdFd = -1;
    fd_set rfdSet;

    struct timeval tv;
    struct timeval *timeout = NULL;
    int maxfd;

    while (1)
    {
        // Set timeout
        timeout = &tv;
        memset(&tv, 0, sizeof(tv));
        if (m_pSerV->CalcNextTimeout(&tv) < 0) {
            timeout = NULL;
        }

        //Reset FD
        cmdFd = m_pSerV->GetReadPipe();
        maxfd = cmdFd + 1;
        FD_ZERO(&rfdSet);
        FD_SET(cmdFd, &rfdSet);

        int n = select(maxfd, &rfdSet, NULL, NULL, timeout);

        if (n > 0) {
            if (FD_ISSET(cmdFd, &rfdSet)) {
                //char buf[256];
                //memset(buf,0,sizeof(buf));
                MsgDirection direction;
                INT8 bVal = -1;
                int ret = read(cmdFd, &bVal, 1);
                if (ret < 0) {
                    if (errno == EINTR || errno == EAGAIN) {
                        continue;
                    }
                    else {
                        m_pSerV->OpenMessagePipe();
                        continue;
                    }
                }
                else if (ret == 0) {
                    m_pSerV->OpenMessagePipe();
                    continue;
                }

                direction = (MsgDirection)bVal;
                switch(direction) {
                case REQUEST:
                    m_pSerV->HandleRequest();
                    break;
                case RESPONSE:
                    m_pSerV->HandleResponse();
                    break;
                case INTERNAL:
                    m_pSerV->HandleInternalMessage();
                    break;
                case ASYNC_REQUEST:
                    m_pSerV->HandleAsycMsgRequest();
                    break;
                default:
                    //these value shouldn't be set, report error
                    RilLogE("Service: message direction error");
                    continue;
                } // end switch ~
            }
        }
        else if(n==0) {
            m_pSerV->HandleRequestTimeout();
        }
    }
}
