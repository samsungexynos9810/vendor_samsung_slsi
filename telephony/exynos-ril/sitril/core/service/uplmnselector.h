 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __UPLMN_SELECTOR_H__
#define __UPLMN_SELECTOR_H__

#include "rildef.h"

class UplmnSelector
{
private:
    map<int, PreferredPlmn> m_PreferredPlmn;
    bool m_Prepared;

    bool m_OnTransaction;
    int m_TransactionMode;
    PreferredPlmn m_TransactionPlmn;

public:
    UplmnSelector();

public:
    int GetCount() const;
    bool Set(int index, const char *plmn, int act);
    bool Set(PreferredPlmn &preferredPlmn);
    int Set(PreferredPlmn *preferredPlmn, int size);
    bool Modify(int index, const char *plmn, int act);
    bool Modify(PreferredPlmn &preferredPlmn);
    int Modify(PreferredPlmn *preferredPlmn, int size);
    void Remove(int index);
    PreferredPlmn Get(int index) const;
    bool IsExists(int index) const;
    bool IsEmpty() const;
    void Clear();
    list<PreferredPlmn> GetList() const;
    bool Prepared() const;
    void Prepare();
    void Reset();
    bool SetTransaction(int mode, int index, const char *plmn, int act);
    void Commit();
    void Cancel();

private:
    void ClearTransaction();

public:
    static bool IsValidUplmnAct(int act);
    static bool IsValidUplmnMode(int mode);
    static bool IsValidPlmn(const char *plmn);
    static bool IsValidIndex(int index);
};

#endif /* __UPLMN_SELECTOR_H__ */
