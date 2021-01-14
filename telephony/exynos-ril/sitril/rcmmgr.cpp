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
 * rcmmgr.cpp
 *
 *  Created on: 2014. 11. 18.
 *      Author: sungwoo48.choi
 */
#include "rcmmgr.h"
#include "modemcontrol.h"
#include "rillog.h"
#include "rilproperty.h"
#include "reset_util.h"
// Enable/Disable to print SIM IO
#include "systemproperty.h"


// Enable/Disable to print SIM IO
#define PROPERTY_SIM_IO_PRINT   "persist.vendor.dbg.ril.sim_io"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

IMPLEMENT_MODULE_TAG(RadioControlMessageMgr, RadioControlMessageMgr)

RadioControlMessageMgr::RadioControlMessageMgr(RilContext *pRilContext, IoChannel *io)
    : m_pRilContext(pRilContext), m_pIo(io)
{
    m_pReaderThread = NULL;
    mCrashExit = false;
    m_buffer = NULL;
}

RadioControlMessageMgr::~RadioControlMessageMgr()
{
    if (m_pIo != NULL) {
        m_pIo->Close();
    }

    if (m_pReaderThread != NULL) {
        m_pReaderThread->Stop();
        delete m_pReaderThread;
        m_pReaderThread = NULL;
    }

    if(m_buffer != NULL) {
        delete[] m_buffer;
    }
}

int RadioControlMessageMgr::Start()
{
    RilLogI("[%s] %s", TAG, __FUNCTION__);
    if (m_pIo == NULL || m_pIo->Init() < 0) {
        return -1;
    }

    if(m_buffer==NULL) {
        m_buffer = new char[MAX_READ_BUF_SIZE];
        if(m_buffer==NULL) {
            RilLogE("[%s] %s(), Error: Memory Allocation Failed for Read/Write Buffer", TAG, __FUNCTION__);
            return -1;
        }
    }

    m_pReaderThread = new Thread(this);
    if (m_pReaderThread == NULL || m_pReaderThread->Start() < 0) {
        return -1;
    }

    // Enable/Disable to print SIM IO
    string strValue = SystemProperty::Get(PROPERTY_SIM_IO_PRINT);
    if(strValue.compare("")==0) m_nPrintSimIo = 1/*LOGLV_SIM_IO_COMMAND*/;
    else m_nPrintSimIo = atoi(strValue.c_str());

    return 0;
}

void RadioControlMessageMgr::Run()
{
    RilLogI("[%s] Start reader procedure", TAG);

    //INT32 i = 0;
    INT32 n = 0, size = 0;
    INT32 maxfd = -1;
    fd_set rfdSet;

    if (m_pRilContext == NULL || m_pIo == NULL) {
        RilLogE("[%s] %s Invalid parameters", TAG, __FUNCTION__);
        return ;
    }

    m_pIo->Open();

    INT32 nStatus = 0;
    bool runnalble = true;

    while (runnalble && !mCrashExit) {
        int fd = m_pIo->GetFd();
        FD_ZERO(&rfdSet);
        FD_SET(fd, &rfdSet);

        maxfd = fd + 1;

        n = select(maxfd, &rfdSet, NULL, NULL, NULL);
        if (n > 0) {
            if (FD_ISSET(fd, &rfdSet)) {
                nStatus = ModemControl::CheckModemStatus(fd);
                if (nStatus < 0) {
                    sleep(3);
                    continue;
                }
                memset(m_buffer, 0, MAX_READ_BUF_SIZE);
                size = m_pIo->Read(m_buffer, MAX_READ_BUF_SIZE);
                if (size <= 0) {
                    if (size < 0 && (errno == EINTR || errno == EAGAIN)) {
                        continue;
                    } else {
                        RilLogE("[%s] Reader: read(%d,%d)", TAG, size, errno);
                        m_pIo->Close();
                        usleep(20000);  //sleep 20 ms
                        m_pIo->Open();
                        continue;
                    }
                }

                // TODO better to extract dump function to dump class (or standalone functions)
                if ( ((size_t)size) <= MAX_READ_BUF_SIZE && DumpData(m_buffer, size, TRUE) != 0) {
                    RilLogE("DumpData Error!");
                }
                m_pRilContext->ProcessModemData(m_buffer, size);
            }
        } else if (n == 0) {
            RilLogE("Reader: TIMEOUT");
            break;
        }
    }

    m_pIo->Close();

    RilLogE("ReaderThread run stopped");
    if (mCrashExit) {
        RilLogE("Due to crash & exit");
    }
}

INT32 RadioControlMessageMgr::Read(char *buf, int size)
{
    if (m_pIo != NULL) {
        return m_pIo->Read(buf, size);
    }
    return -1;
}

INT32 RadioControlMessageMgr::Write(char *buf, int size)
{
    if (m_pIo != NULL) {
        if(DumpData(buf, size, FALSE) != 0) {
            RilLogE("DumpData Error!");
        }

        return m_pIo->Write(buf, size);
    }
    return -1;
}
