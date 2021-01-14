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
 * mcctable.h
 *
 *  Created on: 2014. 10. 24.
 *      Author: sungwoo48.choi
 *
 *  C++ version, class MccTable is existed in telephony framework
 */

#ifndef __MCC_TABLE_H__
#define __MCC_TABLE_H__

#include "types.h"
#include <map>
#include <list>
using namespace std;

class MccEntry {
public:
    int    mcc;
    const char *iso;
    int smallestDigitsMcc;
    const char *language;

public:
    MccEntry(int mcc, const char *iso, int smallestDigitsMcc);
    MccEntry(int mcc, const char *iso, int smallestDigitsMcc, const char *language);
};

class MccTable {
    DECLARE_MODULE_TAG()
private:
    map<int, MccEntry *> m_mcctable;
    list<string> m_EsmZeroOperator;
    list<string> m_NotUsePnnOplOperator;

private:
    MccTable();
public:
    ~MccTable();
private:
    void InitMccTable();
    void InitEsmZeroFlagNetworkList();
    void InitNotUsePnnOplOperator();
    void AddEntry(MccEntry *entry);

    // static
private:
    static MccTable instance;
public:
    static MccTable *GetInstance();
    static const MccEntry *GetEntryForMcc(int mcc);
    static const char *GetCountryCodeForMcc(int mcc);
    static const char *GetDefaultLanguageForMcc(int mcc);
    static int GetSmallestDigitsMccForMcc(int mcc);
    static int GetSmallestDigitsMccForMcc(const string &mcc);
    static int GetSmallestDigitsMccForImsi(const char *imsi);
    static int GetSmallestDigitsMccForImsi(const string &mcc);
    static bool FetchCarrierForImsi(const char *imsi, char *carrier, int size);
    static bool IsCarrierUsePnnOplForEons(const char *carrier);
    static bool IsEsmFlagZeroOperator(const char *carrier);
    static void SetEsmFlagZeroOperator(const char *carrier);
    static int isUnknowNetwork(const char *plmn);
    static int isMvnoNetwork(const char *plmn);
    static bool isUsingSpnForOperatorNameInRegHome(const char *plmn);
    static bool isNeedCheckPlmnMatcingForSpnUsing(const char *plmn);
    static bool isNitzHasPriority(const char *simPlmn, const char *netPlmn);
    static bool isUsingSpnForAvailablePlmnSrch(const char *plmn);
};

#endif /* __MCC_TABLE_H__ */
