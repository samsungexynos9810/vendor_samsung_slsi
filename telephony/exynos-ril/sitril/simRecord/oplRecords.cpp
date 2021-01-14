/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "oplRecords.h"
#include "iccUtil.h"
#include "rillog.h"
#include "textutils.h"

#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/*
 * OplRecord
 */
OplRecord::OplRecord() {
    mLac1 = 0;
    mLac2 = 0;
    mPnnRecordNumber = 0;
}

OplRecord::OplRecord(BYTE *record, int len) : OplRecord() {
    getPlmn(record, plmnOffset, len);
    getLac(record, lacOffset, len);
    mPnnRecordNumber = IccUtil::bytesToInt(record, len, pnnRecordNumberOffset, 1);
}

OplRecord::~OplRecord() {
}

// PLMN decoding as per 3GPP TS 24.008 section 10.5.1.13
void OplRecord::getPlmn(BYTE *record, int offset, int len) {
    String plmnCode = IccUtil::bcdPlmnToString(record, offset, len);
    for (int i = 0; i < MAX_PLMN_LEN; ++i) {
        if (i < (int)plmnCode.length()) mPlmn[i] = IccUtil::hexCharToInt(plmnCode.at(i));
        else mPlmn[i] = 0;  /* Certain operators support 2 digit MNCs. */
    }
}

// LAC decoding as per 3GPP TS 24.008
void OplRecord::getLac(BYTE *record, int offset, int len) {
    // LAC bytes are in big endian. Bytes 3 and 4 are for LAC1 and
    // bytes 5 and 6 are for LAC2.
    mLac1 = IccUtil::bytesToInt(record, len, offset, 2);
    mLac2 = IccUtil::bytesToInt(record, len, offset + 2, 2);
}

String OplRecord::toString() {
    char dbgBuf[100] = { 0, };
    sprintf(dbgBuf, "PLMN=%1x%1x%1x%1x%1x%1x, LAC1=%d, LAC2=%d, PNN Record=%d",
        mPlmn[0], mPlmn[1], mPlmn[2], mPlmn[3], mPlmn[4], mPlmn[5], mLac1, mLac2, mPnnRecordNumber);
    return String(dbgBuf);
}


/*
 * OplRecords
 */
OplRecords::OplRecords() {
    for (unsigned int i = 0; i < mRecords.size(); ++i) {
        OplRecord *record = mRecords[i];
        if (record != NULL) delete record;
    }
    mRecords.clear();
}

OplRecords::OplRecords(BYTE **records, int recordSize, int numRecords) {
    for (int i = 0; i < numRecords; ++i) {
         int recSize = recordSize;

         OplRecord *oplRecord = new OplRecord(records[i], recSize);
         if (oplRecord != NULL) {
             if (oplRecord->isPlmnEmpty()) {
                 /*if (DBG)*/ {
                     RilLogV("Input OPL index %d (start from 1) is empty. so skip it", i+1);
                 }
                 delete oplRecord;
                 continue;
             }
             mRecords.push_back(oplRecord);
             /*if (DBG)*/ {
                 RilLogV("OPL Record[%d]: %s", mRecords.size(), oplRecord->toString().c_str());
             }
         }
    }
}

OplRecords::~OplRecords() {
    for (unsigned int i = 0; i < mRecords.size(); ++i) {
        OplRecord *record = mRecords[i];
        if (record != NULL) delete record;
    }
    mRecords.clear();
}

/**
 * Function to get PNN record number from matching OPL record for registered plmn.
 * @param operator, registered plmn (mcc+mnc)
 * @param lac, current lac
 * @param useLac, whether to match lac or not
 * @return returns PNN record number from matching OPL record.
 */
int OplRecords::getMatchingPnnRecord(String operatorPlmn, int lac, bool useLac) {
    int bcchPlmn[MAX_PLMN_LEN] = {0,0,0,0,0,0};

    if (operatorPlmn.length() < 1) {
        if (DBG) RilLogV("No registered operator.");
        return 0;
    } else if (useLac && (lac == -1)) {
        if (DBG) RilLogV("Invalid LAC");
        return 0;
    }

    int length = operatorPlmn.length();
    if ((length != 5) && (length != 6)) {
        if (DBG) RilLogV("Invalid registered operator length %d", length);
        return 0;
    }

    // Convert operator sting into MCC/MNC digits.
    for (int i = 0; i < length; i++) {
         bcchPlmn[i] = operatorPlmn.at(i) - '0';
    }

    for (unsigned int i = 0; i < mRecords.size(); ++i) {
        OplRecord *record = mRecords[i];
        if (matchPlmn(record->getPlmn(), bcchPlmn)) {
            // While deriving EONS for Available Networks, we do
            // not have Lac, hence just match the plmn.
            if (!useLac || ((record->getLac1() <= lac) && (lac <= record->getLac2()))) {
                // Matching OPL record found, return PNN record number.
                return record->getPnnRecordNumber();
            }
        }
    }

    // No matching OPL record found, return 0 so that operator name from Ril
    // can be used.
    if (DBG) RilLogV("No matching OPL record found.");
    return 0;
}

/**
 * Function to match plmn from EF_OPL record with the registered plmn.
 * @param simPlmn, plmn read from EF_OPL record, size will always be 6
 * @param bcchPlmn, registered plmn, size is 5 or 6
 * @return true if plmns match, otherwise false.
 */
bool OplRecords::matchPlmn (int simPlmn[MAX_PLMN_LEN], int bcchPlmn[MAX_PLMN_LEN]) {
    bool match = true;

    for (int i = 0; i < MAX_PLMN_LEN; i++) {
         match = match & ((bcchPlmn[i] == simPlmn[i]) ||
               (simPlmn[i] == wildCardDigit));
    }

    return match;
}
