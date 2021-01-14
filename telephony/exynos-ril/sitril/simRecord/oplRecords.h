/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#ifndef __OPL_RECORDS_H__
#define __OPL_RECORDS_H__

#include <vector>
#include "rildef.h"
#include "types.h"

// EF_OPL record parsing as per 3GPP TS 31.102 section 4.2.59
class OplRecord {
private:
    static const int plmnOffset = 0;
    static const int lacOffset = 3;
    static const int pnnRecordNumberOffset = 7;

    int mPlmn[MAX_PLMN_LEN] = {0,0,0,0,0,0};
    int mLac1;
    int mLac2;
    int mPnnRecordNumber;

    void getPlmn(BYTE *record, int offset, int len);
    void getLac(BYTE *record, int offset, int len);
public:

    OplRecord();
    OplRecord(BYTE *record, int len);
    virtual ~OplRecord();

    int *getPlmn() { return mPlmn; }
    int getLac1() { return mLac1; }
    int getLac2() { return mLac2; }
    int getPnnRecordNumber() { return mPnnRecordNumber; }
    bool isPlmnEmpty() {
        bool ret = true;
        for (int i = 0; i < MAX_PLMN_LEN; ++i) {
            if (mPlmn[i] != 0) {
                ret = false;
                break;
            }
        }
        return ret;
    }
    String toString();
};


class OplRecords {
private:
    static const bool DBG = false;;
    static const int wildCardDigit = 0x0D;;

    // ***** Instance Variables
    vector<OplRecord*> mRecords;

    bool matchPlmn(int simPlmn[MAX_PLMN_LEN], int bcchPlmn[MAX_PLMN_LEN]);
public:
    // ***** Constructor
    OplRecords();
    OplRecords(BYTE **records, int recordSize, int numRecords);
    virtual ~OplRecords();

    // ***** Public Methods
    int size() { return mRecords.size(); }
    int getMatchingPnnRecord(String operatorPlmn, int lac, bool useLac);
};
#endif
