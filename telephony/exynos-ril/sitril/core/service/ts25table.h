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
 * ts25table.h
 *
 *  Created on: 2019. 3. 5.
 */

#ifndef __TS25_TABLE_H__
#define __TS25_TABLE_H__

#include <string>
using namespace std;

class TS25Record {
private:
    // MCCMNC code which using long name in TS25 table.
    static int MCCMNC_CODES_USE_LONG_NAME_IN_TS25[][2];

public:
    TS25Record();
    virtual ~TS25Record() {}
    string networkName; // Abrev. Network Name
    string ppcin;       // Preferred Presentation of Country Initials and Mobile Network Name (PPCI&N)
    int mcc;
    int mnc;
    bool IsValid() const;
    static bool isUsingLongNameOfT32Table(int _mcc, int _mnc);
    virtual TS25Record &operator=(const TS25Record &rhs);
};

class TS25Table {
public:
    TS25Table();
    virtual ~TS25Table();

public:
    bool Init();
    TS25Record GetRecord(int mcc, int mnc);
    void Dump();
public:
    static TS25Table *GetInstance();
    static TS25Table *MakeInstance();
private:
    static TS25Table *instance;
};

#endif /* __TS25_TABLE_H__ */
