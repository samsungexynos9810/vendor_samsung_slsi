/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#ifndef __PNN_RECORDS_H__
#define __PNN_RECORDS_H__

#include <vector>
#include "rildef.h"
#include "types.h"

// EF_PNN record parsing as per 3GPP TS 31.102 section 4.2.58
class PnnRecord {
private:
    static const int TAG_FULL_NAME_IEI = 0x43;
    static const int TAG_SHORT_NAME_IEI = 0x45;
    static const int TAG_ADDL_INFO = 0x80;

    static const bool DBG = false;

    String mFullName;
    String mShortName;
    String mAddInfo;

public:
    PnnRecord();
    PnnRecord(BYTE *record, int len);
    virtual ~PnnRecord();

    String getFullName() { return mFullName; }
    String getShortName() { return mShortName; }
    String getAddlInfo() { return mAddInfo; }
};

class PnnRecords {
private:
    static const bool DBG = false;

    // ***** Instance Variables
    vector<PnnRecord*> mRecords;

public:

    // ***** Constructor
    PnnRecords();
    PnnRecords(BYTE **records, int recordSize, int numRecords);
    virtual ~PnnRecords();

    // ***** Public Methods
    int size() {
        return mRecords.size();
    }

    String getNameFromPnnRecord(int recordNumber);
    String getHomePnn();
};
#endif