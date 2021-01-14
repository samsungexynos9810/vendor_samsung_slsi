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
    RFSD main
*/

#include "main.h"

bool firstBoot = true;

int main()
{
    ALOGD("Rfs Daemon start!");
    CRfsService *m_pRfsSrv = new CRfsService();
    ModemStateMonitor *m_pModemStateMonitor = new ModemStateMonitor(m_pRfsSrv);

    if(m_pRfsSrv != NULL)
    {
        m_pRfsSrv->OnModemBootDone();
    }

    if(m_pModemStateMonitor != NULL)
    {
        ALOGD("ModemStateMonitor Start!");
        m_pModemStateMonitor->Start();
    }

    while(1)
    {
        sleep(UINT32_MAX);
    }

    ALOGD("Rfs Daemon stop!");
    return 0;
}
