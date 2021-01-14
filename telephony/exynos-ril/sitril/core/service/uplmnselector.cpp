 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "uplmnselector.h"

#define UPLMN_ALLOW_ALWAYS  1

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

bool UplmnSelector::IsValidUplmnAct(int act)
{
    // allows only UPLMN_ACT_GSM,UPLMN_ACT_GSM_COMPACT,UPLMN_ACT_UTRAN,UPLMN_ACT_EUTRAN
    int mask = UPLMN_ACT_BIT_ALL;
    return !(act & ~mask);
}

bool UplmnSelector::IsValidUplmnMode(int mode)
{
    switch (mode) {
    case UPLMN_MODE_ADD:
    case UPLMN_MODE_EDIT:
    case UPLMN_MODE_DELETE:
        return true;
    } // end switch ~

    return false;
}

bool UplmnSelector::IsValidPlmn(const char *plmn)
{
    if (TextUtils::IsEmpty(plmn)) {
        return false;
    }

    if (!(strlen(plmn) == 5 || strlen(plmn) == 6)) {
        return false;
    }

    return true;
}

bool UplmnSelector::IsValidIndex(int index)
{
    if (index < 0 || index > UPLMN_INDEX_MAX || index == UPLMN_INDEX_INVALID) {
        return false;
    }
    return true;
}

UplmnSelector::UplmnSelector() : m_Prepared(false), m_OnTransaction(false)
{
    m_TransactionMode = UPLMN_MODE_INVALID;
    memset(&m_TransactionPlmn, 0, sizeof(m_TransactionPlmn));

}

int UplmnSelector::GetCount() const
{
    if (!m_Prepared) {
        return 0;
    }

    return m_PreferredPlmn.size();
}

bool UplmnSelector::Set(int index, const char *plmn, int act)
{
    if (TextUtils::IsEmpty(plmn)) {
        return false;
    }

    PreferredPlmn preferredPlmn;
    memset(&preferredPlmn, 0, sizeof(preferredPlmn));
    preferredPlmn.index = index;
    strncpy(preferredPlmn.plmn, plmn, MAX_PLMN_LEN);
    if (preferredPlmn.plmn[5] == '#') {
        preferredPlmn.plmn[5] = 0;
    }
    preferredPlmn.act = act;

    return Set(preferredPlmn);
}

bool UplmnSelector::Set(PreferredPlmn &preferredPlmn)
{
    if (preferredPlmn.index == UPLMN_INDEX_INVALID) {
        return false;
    }

    if (preferredPlmn.index < 0 || preferredPlmn.index > UPLMN_INDEX_MAX) {
        return false;
    }

    if (TextUtils::IsEmpty(preferredPlmn.plmn)) {
        return false;
    }

    if (!IsValidUplmnAct(preferredPlmn.act)) {
        return false;
    }

    if (IsExists(preferredPlmn.index)) {
        return false;
    }

    m_PreferredPlmn[preferredPlmn.index] = preferredPlmn;

    return true;
}

int UplmnSelector::Set(PreferredPlmn *preferredPlmn, int size)
{
    int count = 0;
    if (preferredPlmn == NULL) {
        return 0;
    }

    if (size < 0 || size > UPLMN_INDEX_MAX) {
        return 0;
    }

    for (int i = 0; i < size; i++) {
        if (Set(*(preferredPlmn + i))) {
            count++;
        }
    } // end for i ~
    return count;
}

bool UplmnSelector::Modify(PreferredPlmn &preferredPlmn)
{
    if (!m_Prepared) {
        return false;
    }

    if (preferredPlmn.index == UPLMN_INDEX_INVALID) {
        return false;
    }

    if (preferredPlmn.index < 0 || preferredPlmn.index > UPLMN_INDEX_MAX) {
        return false;
    }

    if (TextUtils::IsEmpty(preferredPlmn.plmn)) {
        return false;
    }

    if (!IsValidUplmnAct(preferredPlmn.act)) {
        return false;
    }

    if (!IsExists(preferredPlmn.index)) {
        return false;
    }

    m_PreferredPlmn[preferredPlmn.index] = preferredPlmn;
    return true;
}

int UplmnSelector::Modify(PreferredPlmn *preferredPlmn, int size)
{
    if (!m_Prepared) {
        return 0;
    }

    int count = 0;
    if (preferredPlmn == NULL) {
        return 0;
    }

    if (size < 0 || size > UPLMN_INDEX_MAX) {
        return 0;
    }

    for (int i = 0; i < size; i++) {
        if (Modify(*(preferredPlmn + i))) {
            count++;
        }
    } // end for i ~
    return count;
}

void UplmnSelector::Remove(int index)
{
    if (m_Prepared) {
        m_PreferredPlmn.erase(index);
    }
}

PreferredPlmn UplmnSelector::Get(int index) const
{
    PreferredPlmn result;
    memset(&result, 0, sizeof(result));
    result.index = UPLMN_INDEX_INVALID;

    map<int,PreferredPlmn>::const_iterator iter = m_PreferredPlmn.find(index);
    if (m_Prepared && iter != m_PreferredPlmn.end()) {
        result = iter->second;
    }

    return result;
}

bool UplmnSelector::IsExists(int index) const
{
    if (!m_Prepared) {
        return false;
    }

    if (index < 0 || index > UPLMN_INDEX_MAX) {
        return false;
    }

    map<int, PreferredPlmn>::const_iterator iter = m_PreferredPlmn.find(index);
    return (iter != m_PreferredPlmn.end());
}

bool UplmnSelector::IsEmpty() const
{
    if (!m_Prepared) {
        return true;
    }

    return m_PreferredPlmn.empty();
}

void UplmnSelector::Clear()
{
    m_PreferredPlmn.clear();
}

list<PreferredPlmn> UplmnSelector::GetList() const
{
    list<PreferredPlmn> preferredPlmnList;

    for (int i = 0; i <= UPLMN_INDEX_MAX && m_Prepared; i++) {
        if (IsExists(i)) {
            preferredPlmnList.push_back(Get(i));
        }
    } // end for i ~

    return preferredPlmnList;
}

bool UplmnSelector::Prepared() const
{
#if (UPLMN_ALLOW_ALWAYS == 1)
    return true;
#else
    return m_Prepared;
#endif
}

void UplmnSelector::Prepare()
{
    m_Prepared = true;
}

void UplmnSelector::Reset()
{
    m_Prepared = false;
    m_PreferredPlmn.clear();
}

bool UplmnSelector::SetTransaction(int mode, int index, const char *plmn, int act)
{
    if (IsValidUplmnMode(mode) && IsValidIndex(index) && IsValidPlmn(plmn) && IsValidUplmnAct(act)) {
        ClearTransaction();

        m_TransactionPlmn.index = index;
        if (!TextUtils::IsEmpty(plmn)) {
            strncpy(m_TransactionPlmn.plmn, plmn, MAX_PLMN_LEN);
        }
        m_TransactionPlmn.act = act;
        m_OnTransaction = true;

        return true;
    }
    return false;
}

void UplmnSelector::Commit()
{
    if (m_OnTransaction) {
        if (m_TransactionMode == UPLMN_MODE_ADD) {
            Set(m_TransactionPlmn);
        }
        else if (m_TransactionMode == UPLMN_MODE_EDIT) {
            Modify(m_TransactionPlmn);
        }
        else if (m_TransactionMode == UPLMN_MODE_DELETE) {
            Remove(m_TransactionPlmn.index);
        }
    }
    ClearTransaction();
}

void UplmnSelector::Cancel()
{
    ClearTransaction();
}

void UplmnSelector::ClearTransaction()
{
    m_OnTransaction = false;
    memset(&m_TransactionPlmn, 0, sizeof(m_TransactionPlmn));
    m_TransactionPlmn.index = UPLMN_INDEX_INVALID;
}
