 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _IMS_REQ_DATA_H_
#define _IMS_REQ_DATA_H_

#include "requestdata.h"

/**
 * Setup IMS
 */
class SetupIMS : public StringRequestData
{
public:
    BYTE GetImsSetupSelection()
    {
        return m_nSetup;
    }

    PdnIpType GetPdnIpType()
    {
        return m_nIpType;
    }

    char* GetPdnIpAddr()
    {
        return m_pPdnIp;
    }

private:
    BYTE m_nSetup;
    PdnIpType m_nIpType;
    char m_pPdnIp[MAX_PDN_IP_LEN];
};


#endif /*_IMS_REQ_DATA_H_*/
