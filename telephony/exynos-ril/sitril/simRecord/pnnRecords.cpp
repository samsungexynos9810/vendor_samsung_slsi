/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include "pnnRecords.h"
#include "iccUtil.h"
#include "rillog.h"

#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)

/*
 * PnnRecord
 */
PnnRecord::PnnRecord() {
    mFullName = mShortName = mAddInfo = String("");
}

PnnRecord::PnnRecord(BYTE *record, int len) : PnnRecord() {
    SimTlv *tlv = new SimTlv(record, 0, len);
    if (tlv == NULL) return;

    BYTE *tlvData = NULL;
    int tlvDataLen = 0;

    if (tlv->isValidObject() && tlv->getTag() == TAG_FULL_NAME_IEI) {
        tlvData = tlv->getData();
        tlvDataLen = tlv->getDataLength();
        if (tlvData != NULL) {
            mFullName = IccUtil::networkNameToString(tlvData, 0, tlvDataLen);
            delete [] tlvData;
        }
    } else {
        if(DBG) RilLogV("Invalid tlv Object for Full Name, tag= %d, valid=%d", tlv->getTag(), tlv->isValidObject());
    }

    tlv->nextObject();;
    if (tlv->isValidObject() && tlv->getTag() == TAG_SHORT_NAME_IEI) {
        tlvData = tlv->getData();
        tlvDataLen = tlv->getDataLength();
        if (tlvData != NULL) {
            mShortName = IccUtil::networkNameToString(tlvData, 0, tlvDataLen);
            delete [] tlvData;
        }
    } else {
        if(DBG) RilLogV("Invalid tlv Object for Full Name, tag= %d, valid=%d", tlv->getTag(), tlv->isValidObject());
    }

    tlv->nextObject();
    if (tlv->isValidObject() && tlv->getTag() == TAG_ADDL_INFO) {
        tlvData = tlv->getData();
        tlvDataLen = tlv->getDataLength();
        if (tlvData != NULL) {
            mAddInfo = IccUtil::networkNameToString(tlvData, 0, tlvDataLen);
            delete [] tlvData;
        }
    } else {
        if(DBG) RilLogV("Invalid tlv Object for Full Name, tag= %d, valid=%d", tlv->getTag(), tlv->isValidObject());
    }

    delete tlv;
}

PnnRecord::~PnnRecord() {
}

/*
 * PnnRecords
 */
PnnRecords::PnnRecords() {
    for (unsigned int i = 0; i < mRecords.size(); ++i) {
        PnnRecord *record = mRecords[i];
        if (record != NULL) delete record;
    }
    mRecords.clear();
}

PnnRecords::PnnRecords(BYTE **records, int recordSize, int numRecords) : PnnRecords() {
    for (int i = 0; i < numRecords; ++i) {
         int recSize = recordSize;
         PnnRecord *pnnRecord = new PnnRecord(records[i], recSize);
         if (pnnRecord != NULL) {
             if (TextUtils::IsEmpty(pnnRecord->getFullName())) {
                 /*if (DBG)*/ {
                     RilLogV("Input PNN index %d (start form 1) is empty. so skip it", i+1);
                 }
                 delete pnnRecord;
                 continue;
             }
             mRecords.push_back(pnnRecord);
             /*if (DBG)*/ {
                 RilLogV("PNN Record[%d]: %s, %s", mRecords.size(), pnnRecord->getFullName().c_str(), pnnRecord->getShortName().c_str());
             }
         }
    }
}

PnnRecords::~PnnRecords() {
    for (unsigned int i = 0; i < mRecords.size(); ++i) {
        PnnRecord *record = mRecords[i];
        if (record != NULL) delete record;
    }
    mRecords.clear();
}

/**
 * Function to get Full Name from given PNN record number.
 * @param pnnRecord, PNN record number
 * @param update, specifies whether to update currentEons or not
 * @return returns Full Name from given PNN record.
 */
String PnnRecords::getNameFromPnnRecord(int recordNumber){
    String fullName = String("");

    if (recordNumber < 1 || recordNumber > (int)mRecords.size()) {
        if (DBG) RilLogV("Invalid PNN record number %d", recordNumber);
    } else {
        PnnRecord *record = mRecords[recordNumber - 1];
        if (record != NULL) fullName = record->getFullName();
    }

    return fullName;
}


/**
 * Get home PNN from EF_PNN data.
 * @param void
 * @return PNN string which is the first avaliabe PNN from EF_PNN data, otherwise null
 */
String PnnRecords::getHomePnn() {
    String fullName = String("");

    for (unsigned int i = 0; i < mRecords.size(); ++i) {
        PnnRecord *record = mRecords[i];
        if (record != NULL) fullName = record->getFullName();
        if (fullName.length() > 0) break;
    }

    return fullName;
}
