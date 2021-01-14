/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "rildef.h"
#include "types.h"
#include "pnnRecords.h"
#include "oplRecords.h"

class EonsResolver {
private:
    static const bool DBG = true;

    enum EonsControlState {
        EF_INITING,
        EF_PRESENT,
        EF_ABSENT
    };

    // ***** Instance Variables
    EonsControlState mPnnDataState = EF_INITING;
    EonsControlState mOplDataState = EF_INITING;
    OplRecords *mpOplRecords = NULL;
    PnnRecords *mpPnnRecords = NULL;

public:
    /**
     * INITING: OPL or PNN records not read yet, in this state we do not
     *          know whether EONS is enabled or not, operator name display will be
     *          supressed in this state
     * DISABLED: Exception in reading PNN and or OPL records, EONS is
     *          disabled, use operator name from ril
     * PNN_PRESENT: Only PNN is present, Operator name from first record of PNN can be used
     *          if the registered operator is HPLMN otherwise use name from ril
     * PNN_AND_OPL_PRESENT: Both PNN and OPL files are available, EONS name can be
     *          derived using both these files
     */
    enum EonsState {
        EONS_INITING,
        EONS_DISABLED,
        EONS_PNN_PRESENT,
        EONS_PNN_AND_OPL_PRESENT
    };

private:
    String updateEonsFromOplAndPnn(String regOperator, int lac);
    String updateEonsIfHplmn(String regOperator, String simOperator);

    EonsState getEonsState() {
        // OPL or PNN data read is not complete.
        if ((mPnnDataState == EF_INITING) || (mOplDataState == EF_INITING)) {
            return EONS_INITING;
        } else if (mPnnDataState == EF_PRESENT) {
            if(mOplDataState == EF_PRESENT) {
               return EONS_PNN_AND_OPL_PRESENT;
            } else {
               return EONS_PNN_PRESENT;
            }
        } else {
            // If PNN is not present, disable EONS algorithm.
            return EONS_DISABLED;
        }
    }

public:
    // ***** Constructor
    EonsResolver() { reset(); }

    // ***** Public Methods
    void reset() {
        mPnnDataState = EF_INITING;
        mOplDataState = EF_INITING;
        if (mpOplRecords != NULL) delete mpOplRecords;
        if (mpPnnRecords != NULL) delete mpPnnRecords;
        mpOplRecords = NULL;
        mpPnnRecords = NULL;
    }

    void setOplData(BYTE **records, int recordSize, int numRecords) {
        if (mpOplRecords != NULL) { delete mpOplRecords; mpOplRecords = NULL; }
        mpOplRecords = new OplRecords(records, recordSize, numRecords);
        if (mpOplRecords != NULL && mpOplRecords->size() > 0) mOplDataState = EF_PRESENT;
    }

    void resetOplData() {
        mOplDataState = EF_ABSENT;
        if (mpOplRecords != NULL) delete mpOplRecords;
        mpOplRecords = NULL;
    }

    void setPnnData(BYTE **records, int recordSize, int numRecords) {
        if (mpPnnRecords != NULL) { delete mpPnnRecords; mpPnnRecords = NULL; }
        mpPnnRecords = new PnnRecords(records, recordSize, numRecords);
        if (mpPnnRecords != NULL && mpPnnRecords->size() > 0) mPnnDataState = EF_PRESENT;
    }

    void resetPnnData() {
        mPnnDataState = EF_ABSENT;
        if (mpPnnRecords != NULL) delete mpPnnRecords;
        mpPnnRecords = NULL;
    }

    String updateEons(String regOperator, int lac, String simOperator);
    String getEonsForAvailableNetworks(String availOperator);
};
