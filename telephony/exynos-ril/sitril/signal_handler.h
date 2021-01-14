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
 * signal_handler.h
 *
 *  Created on: 2015. 01. 21.
 *      Author: jhdaniel.kim
 */


#ifndef __SIGNAL_HANDLER_H__
#define __SIGNAL_HANDLER_H__

#include "types.h"

class RilApplication;

class SignalMonitor {
    DECLARE_MODULE_TAG()
public:

private:
    RilApplication *m_pRilApp;
public:
    int HandleSignal(int signo);
    int HandleSignalReserved();
    int HandleSignalDevAction(int dev_id);

public:
    explicit SignalMonitor(RilApplication *pApp);
    virtual ~SignalMonitor();

    int Start();
    int Stop();
};


#endif /* __SIGNAL_HANDLER_H__ */
