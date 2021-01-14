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
#include "rillog.h"
#include "eonsResolver.h"

#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/*
 * EonsResolver private member function
 */
String EonsResolver::updateEonsFromOplAndPnn(String regOperator, int lac) {
    int pnnRecord;
    String pnnName;

    pnnRecord = mpOplRecords->getMatchingPnnRecord(regOperator, lac, true);
    pnnName = mpPnnRecords->getNameFromPnnRecord(pnnRecord);
    if (DBG) RilLogV("[REG]Fetched EONS name: plmn = %s, lac = %d, name = %s", regOperator.c_str(), lac, pnnName.c_str());
    return pnnName;
}

String EonsResolver::updateEonsIfHplmn(String regOperator, String simOperator) {
    if (DBG) RilLogV("Comparing simOperator %s, with registered plmn %s", simOperator.c_str(), regOperator.c_str());

    String ret = String("");
    if (regOperator.length() < 1) {
        return ret;
    }

    // If the registered PLMN is HPLMN, then derive EONS name
    // from first record of EF_PNN
    if ((simOperator.length() > 0) && (simOperator.compare(regOperator) == 0)) {
        String pnnName = mpPnnRecords->getNameFromPnnRecord(1);
        if (DBG) RilLogV("[HPLMN]Fetched EONS name: plmn = %s, name = %s", regOperator.c_str(), pnnName.c_str());
        return pnnName;
    }
    return ret;
}

/*
 * EonsResolver public member function
 */

String EonsResolver::updateEons(String regOperator, int lac, String simOperator) {
    String ret = String("");

    if (getEonsState() == EONS_PNN_AND_OPL_PRESENT) {
        // If both PNN and OPL data is available, a match should
        // be found in OPL file for registered operator and
        // corresponding record in the PNN file should be used
        // for fetching EONS name.
        ret = updateEonsFromOplAndPnn(regOperator, lac);
    } else if (getEonsState() == EONS_PNN_PRESENT) {
        // If only PNN data is available, update EONS name from first
        // record of PNN if registered operator is HPLMN.
        ret = updateEonsIfHplmn(regOperator, simOperator);
    } else if (getEonsState() == EONS_INITING) {
        if (DBG) RilLogE("Reading data from EF_OPL or EF_PNN is not complete");
    }

    // For all other cases including both EF_PNN/EF_OPL absent use
    // operator name from ril.
    return ret;
}

String EonsResolver::getEonsForAvailableNetworks(String availOperator) {
    int pnnRecord;
    String pnnName;

    if (getEonsState() != EONS_PNN_AND_OPL_PRESENT) {
        RilLogE("OPL/PNN data is not available. Use the network names from other src.");
        return String("");
    }

    pnnRecord = mpOplRecords->getMatchingPnnRecord(availOperator, -1, false);
    pnnName = mpPnnRecords->getNameFromPnnRecord(pnnRecord);
    if (DBG) RilLogV("[AVAIL]Fetched EONS: plmn = %s, name = %s",availOperator.c_str(), pnnName.c_str());
    return pnnName;
}
