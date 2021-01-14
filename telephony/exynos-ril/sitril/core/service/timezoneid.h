/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/**
 * Copyright (c) 2013 SAMSUNG Co. Ltd,
 * All rights reserved.
 *
 * File: timezoneid.h
 *
 * Release:
 *
 * Description: timezone identifier header
 *
 *
 * Revision History
 * April/24/2013   shijin.chen  Initial revision

 */

#ifndef _TIME_ZONE_ID_H_
#define _TIME_ZONE_ID_H_

#include "rildef.h"


class CTimeZoneIdEntry
{
private:
    static const INT32 MAX_ID_SIZE = 7;
    static const INT32 MAX_ZONE_ID_SIZE = 36;
    static const INT32 MAX_ISO_SIZE = 3;

public:
    CTimeZoneIdEntry();
    virtual ~CTimeZoneIdEntry();

protected:
    BYTE m_Id[MAX_ID_SIZE];
    BYTE m_zoneId[MAX_ZONE_ID_SIZE];
    BYTE m_iso[MAX_ISO_SIZE];
};

class CTimeZoneId
{
public:
    CTimeZoneId();
    virtual ~CTimeZoneId();

protected:
};


#endif /*_TIME_ZONE_ID_H_*/

