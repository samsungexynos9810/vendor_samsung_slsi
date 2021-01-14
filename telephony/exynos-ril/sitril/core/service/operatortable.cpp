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
 * operatortable.cpp
 *
 *  Created on: 2014. 7. 5.
 *      Author: sungwoo48.choi
 */

#include "operatortable.h"
#include <sqlite3.h>
#include <list>
#include <map>
#include "rillog.h"
#include "textutils.h"

using namespace std;

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_OPERTABLE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_OPERTABLE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_OPERTABLE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_OPERTABLE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define DB_NAME                  "/data/vendor/rild/.operatortable.db"
#define TABLE_NAME_SETTINGS      "settings"
#define COL_VERSION              "version"

#define TABLE_NAME_OPERATOR      "operator"
#define COL_PLMN_MCC             "plmn_mcc"
#define COL_PLMN_MNC             "plmn_mnc"
#define COL_LONG_PLMN            "long_plmn"
#define COL_SHORT_PLMN           "short_plmn"
#define COL_INDEX_PLMN_MCC       (0)
#define COL_INDEX_PLMN_MNC       (1)
#define COL_INDEX_LONG_PLMN      (2)
#define COL_INDEX_SHORT_PLMN     (3)
#define SQL_BUF_SIZE             512

// database version
#define OPERATOR_NAME_DB_VERSION  (4)

// PLMN, Long PLMN, Short PLMN
static const char *GLOBAL_OPERATOR_INFO[][4] = {
#include "global_operator_info.txt"
    { 0, 0, 0, 0 }
};

static bool debug = false;
static pthread_mutex_t s_opDbMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * OperatorContentValue
 */
OperatorContentValue::OperatorContentValue()
{
    memset(m_plmn, 0, sizeof(m_plmn));
    memset(m_shortPlmn, 0, sizeof(m_shortPlmn));
    memset(m_longPlmn, 0, sizeof(m_longPlmn));
}

OperatorContentValue::~OperatorContentValue()
{
}

OperatorContentValue::OperatorContentValue(const char *plmn, const char *longPlmn, const char *shortPlmn)
{
    Update(plmn, longPlmn, shortPlmn);
}


OperatorContentValue *OperatorContentValue::NewInstance(const char *plmn, const char *longPlmn/* = NULL*/, const char *shortPlmn/* = NULL*/)
{
    if (plmn == NULL)
        return NULL;
    return new OperatorContentValue(plmn, longPlmn, shortPlmn);
}

void OperatorContentValue::SetPlmn(const char *plmn)
{
    if (plmn != NULL) {
        strncpy(m_plmn, plmn, MAX_PLMN_LEN);
        if (strlen(plmn) > 5 && m_plmn[5] == '#')
            m_plmn[5] = 0;
    }
    else {
        m_plmn[0] = 0;
    }
}

const char *OperatorContentValue::GetPlmn() const
{
    if (*m_plmn == 0)
        return NULL;
    return m_plmn;
}

void OperatorContentValue::SetShortPlmn(const char *shortPlmn)
{
    if (shortPlmn != NULL) {
        strncpy(m_shortPlmn, shortPlmn, MAX_SHORT_NAME_LEN);
    }
    else {
        m_shortPlmn[0] = 0;
    }
}
const char *OperatorContentValue::GetShortPlmn() const
{
    if (*m_shortPlmn == 0)
        return NULL;
    return m_shortPlmn;
}
void OperatorContentValue::SetLongPlmn(const char *longPlmn)
{
    if (longPlmn != NULL) {
        strncpy(m_longPlmn, longPlmn, MAX_FULL_NAME_LEN);
    }
    else {
        m_longPlmn[0] = 0;
    }
}
const char *OperatorContentValue::GetLongPlmn() const
{
    if (*m_longPlmn == 0)
        return NULL;
    return m_longPlmn;
}

void OperatorContentValue::Update(const char *plmn, const char *longPlmn, const char *shortPlmn)
{
    SetPlmn(plmn);
    if (GetPlmn() != NULL) {
        Update(longPlmn, shortPlmn);
    }
}

void OperatorContentValue::Update(const char *longPlmn, const char *shortPlmn)
{
    if (!TextUtils::IsEmpty(longPlmn))
        SetLongPlmn(longPlmn);
    if (!TextUtils::IsEmpty(shortPlmn))
        SetShortPlmn(shortPlmn);
}

bool OperatorContentValue::operator==(const OperatorContentValue &rhs)
{
    return (strcmp(m_plmn, rhs.m_longPlmn) == 0 &&
            strcmp(m_shortPlmn, rhs.m_shortPlmn) == 0 &&
            strcmp(m_longPlmn, rhs.m_longPlmn) == 0);
}

bool OperatorContentValue::operator!=(const OperatorContentValue &rhs)
{
    return (strcmp(m_plmn, rhs.m_longPlmn) != 0 ||
            strcmp(m_shortPlmn, rhs.m_shortPlmn) != 0 ||
            strcmp(m_longPlmn, rhs.m_longPlmn) != 0);
}

OperatorContentValue &OperatorContentValue::operator=(const OperatorContentValue &rhs)
{
    this->Update(rhs.m_plmn, rhs.m_shortPlmn, rhs.m_longPlmn);
    return *this;
}

/**
 * EonsContentsValue
 */
EonsContentsValue::EonsContentsValue() : OperatorContentValue()
{
    memset(m_iccId, 0, sizeof(m_iccId));
}

EonsContentsValue::~EonsContentsValue()
{
}

EonsContentsValue::EonsContentsValue(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn)
{
    Update(netPlmn, longPlmn, shortPlmn);
    SetIccId(iccId);
}

void EonsContentsValue::SetIccId(const char *iccId)
{
    if (iccId != NULL) {
        strncpy(m_iccId, iccId, MAX_ICCID_LEN);
        m_iccId[MAX_ICCID_LEN]=0;
    }
    else {
        m_iccId[0] = 0;
    }
}

const char *EonsContentsValue::GetIccId() const
{
    if (*m_iccId == 0)
        return NULL;
    return m_iccId;
}

EonsContentsValue *EonsContentsValue::NewInstance(const char *iccId, const char *netPlmn, const char *longPlmn/* = NULL*/, const char *shortPlmn/* = NULL*/)
{
    if (iccId == NULL || netPlmn == NULL)
        return NULL;
    return new EonsContentsValue(iccId, netPlmn, longPlmn, shortPlmn);
}

/////////////////////////////////////////////////////////////////////
// OperatorNameTableDatabaseHelper
/////////////////////////////////////////////////////////////////////
class OperatorNameTableDatabaseHelper : public OperatorNameProvider {
    DECLARE_MODULE_TAG()
public:
    OperatorNameTableDatabaseHelper() {
        m_database = NULL;
        if (sqlite3_open(DB_NAME, &m_database) != SQLITE_OK) {
            RilLogE("%s : Failed to open database", __FUNCTION__);
            m_database = NULL;
        }
    }

    virtual ~OperatorNameTableDatabaseHelper() {
        if (m_database != NULL) {
            sqlite3_close(m_database);
            m_database = NULL;
        }
    }

private:
    sqlite3 *m_database;

protected:

public:
    bool Contains(const char *plmn);
    OperatorContentValue *Find(const char *plmn);
    bool Update(const char *plmn, const char *longPlmn, const char *shortPlmn);
    bool Insert(const char *plmn, const char *longPlmn, const char *shortPlmn);
    bool Update(const OperatorContentValue *contentValue) {
        if (contentValue != NULL) {
            return Update(contentValue->GetPlmn(), contentValue->GetLongPlmn(), contentValue->GetShortPlmn());
        }
        return false;
    }
    bool Insert(const OperatorContentValue *contentValue) {
        if (contentValue != NULL) {
            return Insert(contentValue->GetPlmn(), contentValue->GetLongPlmn(), contentValue->GetShortPlmn());
        }
        return false;
    }

     //EONS table access
    /* override */
    bool InsertEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn);
    bool UpdateEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn);
    EonsContentsValue *FindEons(const char *iccId, const char *netPlmn);



public:
    static int MakeDB();
    static void DropDB();

private:
    static bool TestDatabaseVersion();
    static bool TestOperatorTable();
    static bool CreateTableSettings();
    static bool CreateTableOperator();
};

IMPLEMENT_MODULE_TAG(OperatorNameTableDatabaseHelper, OperatorNameTableDatabaseHelper)

bool OperatorNameTableDatabaseHelper::TestDatabaseVersion()
{
    RilLogI("%s %s", TAG, __FUNCTION__);

    char sql[SQL_BUF_SIZE] = { 0, };
    sqlite3 *database = NULL;
    sqlite3_stmt *statement = NULL;
    bool test = false;
    int err = SQLITE_OK;
    pthread_mutex_lock(&s_opDbMutex);
    if ((err = sqlite3_open_v2(DB_NAME, &database, SQLITE_OPEN_READONLY, NULL)) == SQLITE_OK) {

        // check database version
        snprintf(sql, sizeof(sql), "SELECT %s FROM %s", COL_VERSION, TABLE_NAME_SETTINGS);
        if ((err = sqlite3_prepare(database, sql, -1, &statement, NULL)) == SQLITE_OK) {
            if (sqlite3_step(statement) == SQLITE_ROW) {
                int version = sqlite3_column_int(statement, 0);
                RilLogV("%s %s current version=%d, new version=%d", TAG, __FUNCTION__, version, OPERATOR_NAME_DB_VERSION);
                if (version >= OPERATOR_NAME_DB_VERSION) {
                    test = true;
                }
            }
            sqlite3_finalize(statement);
        }
        else {
            RilLogE("%s %s : sqlite3_prepare error : error code(%d)", TAG, __FUNCTION__, err);
        }
        sqlite3_close(database);
    }
    pthread_mutex_unlock(&s_opDbMutex);
    return test;
}

bool OperatorNameTableDatabaseHelper::TestOperatorTable()
{
    RilLogI("%s %s", TAG, __FUNCTION__);

    char sql[SQL_BUF_SIZE] = { 0, };
    sqlite3 *database = NULL;
    sqlite3_stmt *statement = NULL;
    bool test = false;
    int err = SQLITE_OK;
    pthread_mutex_lock(&s_opDbMutex);
    if ((err = sqlite3_open_v2(DB_NAME, &database, SQLITE_OPEN_READONLY, NULL)) == SQLITE_OK) {
        // check database version
        snprintf(sql, sizeof(sql), "SELECT count(*) FROM %s", TABLE_NAME_OPERATOR);
        if ((err = sqlite3_prepare(database, sql, -1, &statement, NULL)) == SQLITE_OK) {
            if (sqlite3_step(statement) == SQLITE_ROW) {
                int counts = sqlite3_column_int(statement, 0);
                RilLogV("%s %s Total records is(are) = %d", TAG, __FUNCTION__, counts);
                if (counts > 0) {
                    test = true;
                }
            }
            else {
                RilLogW("%s %s : No record(s) in database", TAG, __FUNCTION__);
            }
            sqlite3_finalize(statement);
        }
        else {
            RilLogE("%s %s : sqlite3_prepare error : error code(%d)", TAG, __FUNCTION__, err);
        }
        sqlite3_close(database);
    }
    pthread_mutex_unlock(&s_opDbMutex);
    return test;
}

bool OperatorNameTableDatabaseHelper::CreateTableSettings()
{
    //int err = 0;
    bool ret = false;
    char sql[SQL_BUF_SIZE] = { 0, };
    sqlite3 *database = NULL;
    char *errMsg = NULL;

    if (sqlite3_open(DB_NAME, &database) != SQLITE_OK) {
        /*  DB create error */
        RilLogE("%s %s Failed to create DB file %s", TAG, __FUNCTION__, DB_NAME);
        return false;
    }

    // create settings table
    snprintf(sql, sizeof(sql)-1, "CREATE TABLE %s (%s INTEGER PRIMARY KEY);", TABLE_NAME_SETTINGS, COL_VERSION);
    if (sqlite3_exec(database, sql, NULL, NULL, &errMsg) == SQLITE_OK) {
        snprintf(sql, sizeof(sql)-1, "INSERT INTO %s (%s) VALUES(\"%d\")",
                TABLE_NAME_SETTINGS, COL_VERSION, OPERATOR_NAME_DB_VERSION);
        if (sqlite3_exec(database, sql, NULL, NULL, &errMsg) == SQLITE_OK) {
            if (debug)
                RilLogV("%s %s set database version=%d", TAG, __FUNCTION__, OPERATOR_NAME_DB_VERSION);
            ret = true;
        }
        else {
            RilLogW("%s %s Failed to insert content : error(%s)", TAG, __FUNCTION__, errMsg);
            sqlite3_free(errMsg);
        }
    }
    else {
        // create table error
        RilLogE("%s %s Failed to create %s table : error(%s)", TAG, __FUNCTION__, TABLE_NAME_SETTINGS, errMsg);
        sqlite3_free(errMsg);
    }
    sqlite3_close(database);

    return ret;
}

bool OperatorNameTableDatabaseHelper::CreateTableOperator()
{
    //int err = 0;
    bool ret = false;
    char sql[SQL_BUF_SIZE] = { 0, };
    sqlite3 *database = NULL;
    char *errMsg = NULL;

    if (sqlite3_open(DB_NAME, &database) != SQLITE_OK) {
        /*  DB create error */
        RilLogE("%s %s Failed to create DB file %s", TAG, __FUNCTION__, DB_NAME);
        return false;
    }

    // create operator table
    snprintf(sql, sizeof(sql)-1, "CREATE TABLE %s (" \
                    "%s INTEGER, " \
                    "%s INTEGER, " \
                    "%s TEXT, " \
                    "%s TEXT);",
            TABLE_NAME_OPERATOR, COL_PLMN_MCC, COL_PLMN_MNC, COL_LONG_PLMN, COL_SHORT_PLMN);

    if (sqlite3_exec(database, sql, NULL, NULL, &errMsg) == SQLITE_OK) {
        int records = 0;
        // insert new records
        int size = sizeof(GLOBAL_OPERATOR_INFO) / sizeof(GLOBAL_OPERATOR_INFO[0]);
        for (int i = 0; i < size; i++) {
            const char **entry = GLOBAL_OPERATOR_INFO[i];
            if (entry[0] == NULL)
                break;
            // InsertIntoDB(entry[0], entry[1], entry[2]);
            snprintf(sql, sizeof(sql)-1, "INSERT INTO %s (%s, %s, %s, %s) VALUES(%d, %d, \"%s\", \"%s\")",
                            TABLE_NAME_OPERATOR, COL_PLMN_MCC, COL_PLMN_MNC, COL_LONG_PLMN, COL_SHORT_PLMN,
                            atoi(entry[0]), atoi(entry[1]), entry[2], entry[3]);
            if (sqlite3_exec(database, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
                // insert error
                RilLogW("%s %s Failed to insert content : error(%s)", TAG, __FUNCTION__, errMsg);
                RilLogV("%s %s MCC=%d, MNC=%d", TAG, __FUNCTION__, atoi(entry[0]), atoi(entry[1]));
                sqlite3_free(errMsg);
                continue;
            }
            records++;
            if (debug) {
                RilLogV("%s %s INSERT VALUES(%d, %d, \"%s\", \"%s\")", TAG, __FUNCTION__,
                        atoi(entry[0]), atoi(entry[1]), entry[2], entry[3]);
            }
            ret = true;
        } // end for i ~
        RilLogV("%s %s total %d record(s) is(are) added.", TAG, __FUNCTION__, records);
    }
    else {
        // create table error
        RilLogE("%s %s Failed to create %s table : error(%s)", TAG, __FUNCTION__, TABLE_NAME_OPERATOR, errMsg);
        sqlite3_free(errMsg);
    }
    sqlite3_close(database);

    return ret;
}

int OperatorNameTableDatabaseHelper::MakeDB()
{
    //  make database file
    // 1. check whether database is already existed or not
    // 2-1. if no file, create new database and insert operator information
    // 2-2. if already existed, check open state and counts of records.
    RilLogI("%s %s", TAG, __FUNCTION__);

    if (!TestDatabaseVersion() || !TestOperatorTable()) {
        RilLogV("%s %s Create new database", TAG, __FUNCTION__);

        // drop first
        DropDB();

        if (!CreateTableSettings() || !CreateTableOperator()) {
            DropDB();
            RilLogW("%s %s Failed to create database table : Drop database.", TAG, __FUNCTION__);
            return -1;
        }
    }
    else {
        RilLogW("%s %s Don't need to create database and tables", TAG, __FUNCTION__);
    }
    return 0;
}

void OperatorNameTableDatabaseHelper::DropDB()
{
    // drop database file
    int ret = unlink(DB_NAME);
    RilLogV("%s %s Drop database(%d)", TAG, __FUNCTION__, ret);
}

bool OperatorNameTableDatabaseHelper::Contains(const char *plmn)
{
    OperatorContentValue *contentValue = Find(plmn);
    if (contentValue != NULL) {
        delete contentValue;
        return true;
    }

    return false;
}

OperatorContentValue *OperatorNameTableDatabaseHelper::Find(const char *plmn)
{
    sqlite3_stmt *statement = NULL;
    char sql[SQL_BUF_SIZE] = { 0, };
    OperatorContentValue *result = NULL;

    if (m_database == NULL) {
        RilLogW("%s : Invalid database handle", __FUNCTION__);
        return NULL;
    }

    if (isValidPlmn(plmn) == false) return NULL;

    int len = strlen(plmn);
    char mccStr[5] = {0, };
    char mncStr[5] = {0, };
    memcpy(mccStr, plmn, 3);
    memcpy(mncStr, plmn + 3, (len - 3));
    int mcc = atoi(mccStr);
    int mnc = atoi(mncStr);
    snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE %s=%d and %s=%d", TABLE_NAME_OPERATOR, COL_PLMN_MCC, mcc, COL_PLMN_MNC, mnc);

    int err = SQLITE_OK;
    pthread_mutex_lock(&s_opDbMutex);
    if ((err = sqlite3_prepare(m_database, sql, -1, &statement, NULL)) == SQLITE_OK) {
        // select the first record only
        if (sqlite3_step(statement) == SQLITE_ROW) {
            char *longPlmn = (char *)sqlite3_column_text(statement, COL_INDEX_LONG_PLMN);
            char *shortPlmn = (char *)sqlite3_column_text(statement, COL_INDEX_SHORT_PLMN);
            result = OperatorContentValue::NewInstance(plmn, longPlmn, shortPlmn);
            if (debug)
                RilLogV("SELECT VALUES(%d, %d, \"%s\", \"%s\")", mcc, mnc, longPlmn, shortPlmn);
        }
        else {
            if (debug) RilLogV("%s : No record for carrier %s", __FUNCTION__, plmn);
        }
    }
    else {
        RilLogE("%s : sqlite3_prepare error : error code(%d)", __FUNCTION__, err);
        RilLogE("%s : sql=%s", __FUNCTION__, sql);

        // TODO may re-create DB file.
    }
    sqlite3_finalize(statement);
    pthread_mutex_unlock(&s_opDbMutex);

    return result;
}

bool OperatorNameTableDatabaseHelper::Insert(const char *plmn, const char *longPlmn, const char *shortPlmn)
{
    if (isValidPlmn(plmn) == false) return false;

    if (longPlmn == NULL || *longPlmn == 0) {
        return false;
    }

    if (shortPlmn == NULL || *shortPlmn == 0) {
        return false;
    }

    char sql[SQL_BUF_SIZE] = { 0, };

    if (m_database == NULL) {
        return false;
    }

    int len = strlen(plmn);
    char mccStr[5] = {0, };
    char mncStr[5] = {0, };
    memcpy(mccStr, plmn, 3);
    memcpy(mncStr, plmn + 3, (len - 3));
    int mcc = atoi(mccStr);
    int mnc = atoi(mncStr);
    snprintf(sql, SQL_BUF_SIZE, "INSERT INTO %s (%s, %s, %s, %s) VALUES(%d, %d, \"%s\", \"%s\")",
                        TABLE_NAME_OPERATOR,
                        COL_PLMN_MCC, COL_PLMN_MNC, COL_LONG_PLMN, COL_SHORT_PLMN,
                        mcc, mnc, longPlmn, shortPlmn);

    char *errMsg = NULL;
    if (sqlite3_exec(m_database, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        // log error
        RilLogE("%s : sqlite3_exec error : %s", __FUNCTION__, errMsg);
        RilLogE("%s : sql=%s", __FUNCTION__, sql);

        sqlite3_free(errMsg);
        return false;
    }
    if (debug)
        RilLogV("INSERT VALUES(%d, %d, \"%s\", \"%s\")", mcc, mnc, longPlmn, shortPlmn);
    return true;
}

bool OperatorNameTableDatabaseHelper::Update(const char *plmn, const char *longPlmn, const char *shortPlmn)
{
    if (isValidPlmn(plmn) == false) return false;

    if (longPlmn == NULL || *longPlmn == 0) {
        return false;
    }

    if (shortPlmn == NULL || *shortPlmn == 0) {
        return false;
    }

    char sql[SQL_BUF_SIZE] = { 0, };

    if (m_database == NULL) {
        return false;
    }

    int len = strlen(plmn);
    char mccStr[5] = {0, };
    char mncStr[5] = {0, };
    memcpy(mccStr, plmn, 3);
    memcpy(mncStr, plmn + 3, (len - 3));
    int mcc = atoi(mccStr);
    int mnc = atoi(mncStr);
    snprintf(sql, SQL_BUF_SIZE, "UPDATE %s SET %s=\"%s\", %s=\"%s\" WHERE %s=%d and %s=%d",
            TABLE_NAME_OPERATOR,
            COL_LONG_PLMN, longPlmn, COL_SHORT_PLMN, shortPlmn, COL_PLMN_MCC, mcc, COL_PLMN_MNC, mnc);

    char *errMsg = NULL;
    if (sqlite3_exec(m_database, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        // TODO log error
        RilLogE("%s : sqlite3_exec error : %s", __FUNCTION__, errMsg);
        RilLogE("%s : sql=%s", __FUNCTION__, sql);

        sqlite3_free(errMsg);
        return false;
    }
    if (debug)
        RilLogV("UPDATE VALUES(%d, %d, \"%s\", \"%s\")", mcc, mnc, longPlmn, shortPlmn);
    return true;
}

bool OperatorNameTableDatabaseHelper::InsertEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn)
{
    return true;
}

bool OperatorNameTableDatabaseHelper::UpdateEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn)
{
    return true;
}

EonsContentsValue *OperatorNameTableDatabaseHelper::FindEons(const char *iccId, const char *netPlmn)
{
    EonsContentsValue *result = NULL;
    return result;
}

class OperatorNameTable : public OperatorNameProvider {
    DECLARE_MODULE_TAG()
private:
    list<OperatorContentValue *> m_cache;
    list<EonsContentsValue *> m_cacheEons;

public:
    OperatorNameTable();
    virtual ~OperatorNameTable();

public:
    bool Contains(const char *plmn);
    OperatorContentValue *Find(const char *plmn);
    bool Insert(const char *plmn, const char *longPlmn, const char *shortPlmn);
    bool Update(const char *plmn, const char *longPlmn, const char *shortPlmn);
    bool Insert(const OperatorContentValue *contentValue) {
        if (contentValue != NULL) {
            return Insert(contentValue->GetPlmn(), contentValue->GetLongPlmn(), contentValue->GetShortPlmn());
        }
        return false;
    }
    bool Update(const OperatorContentValue *contentValue) {
        if (contentValue != NULL) {
            return Update(contentValue->GetPlmn(), contentValue->GetLongPlmn(), contentValue->GetShortPlmn());
        }
        return false;
    }

    // EONS table access
    /* override */
    bool InsertEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn);
    bool UpdateEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn);
    EonsContentsValue *FindEons(const char *iccId, const char *netPlmn);

    void Clear();
private:
    void LoadDefaultContentValues();

    static CMutex lock;
    static OperatorNameTable *_instance;
public:
    static OperatorNameTable *MakeInstance();
    static OperatorNameTable *GetInstance();
};


/////////////////////////////////////////////////////////////////////
// OperatorNameTable
/////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE_TAG(OperatorNameTable, OperatorNameTable)
CMutex OperatorNameTable::lock;
OperatorNameTable *OperatorNameTable::_instance = NULL;

OperatorNameTable *OperatorNameTable::MakeInstance()
{
    OperatorNameTable::lock.lock();
    if (debug) RilLogV("%s::%s lock", TAG, __FUNCTION__);
    if (_instance == NULL) {
        _instance = new OperatorNameTable();
        if (OperatorNameTableDatabaseHelper::MakeDB() < 0) {
            RilLogE("%s::%s Load default set of operator name without creating database", TAG, __FUNCTION__);
            // Exceptional case, failed to create operator name table
            // maintain all operator name list in OperatorNameTable instead of database
            _instance->LoadDefaultContentValues();
        }
    }
    OperatorNameTable::lock.unlock();
    if (debug) RilLogV("%s::%s unlock", TAG, __FUNCTION__);
    return _instance;
}

OperatorNameTable *OperatorNameTable::GetInstance()
{
    OperatorNameTable::lock.lock();
    do {
        if (debug) RilLogV("%s::%s lock", TAG, __FUNCTION__);
    } while (0);
    OperatorNameTable::lock.unlock();
    if (debug) RilLogV("%s::%s unlock", TAG, __FUNCTION__);
    return _instance;
}

OperatorNameTable::OperatorNameTable() {

}

OperatorNameTable::~OperatorNameTable() {
    Clear();
}

void OperatorNameTable::LoadDefaultContentValues()
{
    RilLogI("%s", __FUNCTION__);
    Clear();

    int size = sizeof(GLOBAL_OPERATOR_INFO) / sizeof(GLOBAL_OPERATOR_INFO[0]);
    for (int i = 0; i < size; i++) {
        const char **entry = GLOBAL_OPERATOR_INFO[i];
        if (entry[0] == NULL)
            break;

        OperatorContentValue *contentValue = OperatorContentValue::NewInstance(entry[0], entry[1], entry[2]);
        // insert into cache
        m_cache.push_back(contentValue);
    }
    RilLogV("%s List loaded total %d item(s)", __FUNCTION__, m_cache.size());
}

bool OperatorNameTable::Contains(const char *plmn)
{
    OperatorContentValue *contentValue = Find(plmn);
    return (contentValue != NULL);
}

OperatorContentValue *OperatorNameTable::Find(const char *plmn)
{
    if (isValidPlmn(plmn) == false) {
        return NULL;
    }

    list<OperatorContentValue *>::iterator iter;
    for (iter = m_cache.begin(); iter != m_cache.end(); iter++) {
        OperatorContentValue *contentValue = *iter;
        if (contentValue != NULL && contentValue->GetPlmn() != NULL && strcmp(contentValue->GetPlmn(), plmn) == 0) {
            return contentValue;
        }
    }

    // search from database
    OperatorNameTableDatabaseHelper database;
    OperatorContentValue *contentValue = database.Find(plmn);
    if (contentValue == NULL) {
        return NULL;
    }

    // insert into cache
    m_cache.push_back(contentValue);

    return contentValue;
}

bool OperatorNameTable::Insert(const char *plmn, const char *longPlmn, const char *shortPlmn)
{
    if (Contains(plmn)) {
        return false;
    }

    if (isValidPlmn(plmn) == false) {
        return false;
    }

    if (longPlmn == NULL || *longPlmn == 0) {
        return false;
    }

    if (shortPlmn == NULL || *shortPlmn == 0) {
        return false;
    }

    // insert into database
    OperatorNameTableDatabaseHelper database;
    return database.Insert(plmn, longPlmn, shortPlmn);
}

bool OperatorNameTable::Update(const char *plmn, const char *longPlmn, const char *shortPlmn)
{
    OperatorContentValue *contentValueInCache = Find(plmn);
    if (contentValueInCache == NULL) {
        return false;
    }

    if (isValidPlmn(plmn) == false) {
        return false;
    }

    if (longPlmn == NULL || *longPlmn == 0) {
        return false;
    }
    contentValueInCache->SetLongPlmn(longPlmn);

    if (shortPlmn == NULL || *shortPlmn == 0) {
        return false;
    }
    contentValueInCache->SetShortPlmn(shortPlmn);

    // update database
    OperatorNameTableDatabaseHelper database;
    return database.Update(contentValueInCache);
}

bool OperatorNameTable::InsertEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn)
{
    if (iccId == NULL || *iccId == 0) {
        return false;
    }

    if (isValidPlmn(netPlmn) == false) {
        return false;
    }

    if (longPlmn == NULL || *longPlmn == 0) {
        return false;
    }

    if (shortPlmn == NULL || *shortPlmn == 0) {
        return false;
    }

    // insert into cache
    EonsContentsValue *contentsValue = EonsContentsValue::NewInstance(iccId, netPlmn, longPlmn, shortPlmn);
    m_cacheEons.push_back(contentsValue);

   return true;
}

bool OperatorNameTable::UpdateEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn)
{
    EonsContentsValue *contentValueInCache = FindEons(iccId,netPlmn);
    if ( contentValueInCache == NULL){
        return false;
    }

    if (longPlmn != NULL && *longPlmn != 0) {
        contentValueInCache->SetLongPlmn(longPlmn);
    }

    if (shortPlmn != NULL && *shortPlmn != 0) {
        contentValueInCache->SetShortPlmn(shortPlmn);
    }

    return true;
}

EonsContentsValue *OperatorNameTable::FindEons(const char *iccId, const char *netPlmn)
{
    if (iccId == NULL || *iccId == 0) {
        return NULL;
    }

    if (isValidPlmn(netPlmn) == false) {
        return NULL;
    }

    list<EonsContentsValue *>::iterator iterEons;
    for (iterEons = m_cacheEons.begin(); iterEons != m_cacheEons.end(); iterEons++) {
        EonsContentsValue *contentValue = *iterEons;
        if (contentValue != NULL && contentValue->GetPlmn() != NULL && contentValue->GetIccId() != NULL) {
            if (strcmp(contentValue->GetPlmn(), netPlmn) == 0 && strcmp(contentValue->GetIccId(), iccId) == 0 ) {
                return contentValue;
            }
        }
    }

    return NULL;
}

void OperatorNameTable::Clear()
{
    list<OperatorContentValue *>::iterator iter;
    for (iter = m_cache.begin(); iter != m_cache.end(); iter++) {
        OperatorContentValue * contentValue = *iter;
        if (contentValue != NULL) {
            delete contentValue;
        }
    } // end for iter ~
    m_cache.clear();

    list<EonsContentsValue *>::iterator iterEons;
    for (iterEons = m_cacheEons.begin(); iterEons != m_cacheEons.end(); iterEons++) {
        EonsContentsValue * contentValue = *iterEons;
        if (contentValue != NULL) {
            delete contentValue;
        }
    } // end for iter ~
    m_cacheEons.clear();
}


/////////////////////////////////////////////////////////////////////
// OperatorNameProvider
/////////////////////////////////////////////////////////////////////
bool OperatorNameProvider::isValidPlmn(const char *plmn) {
    if (plmn == NULL || *plmn == 0) return false;

    if (!(strlen(plmn) == 5 || strlen(plmn) == 6) || TextUtils::IsDigitsOnly(plmn) == false) {
        RilLogW("Invalid carrier(%s)", plmn);
        return false;
    }

    return true;
}

OperatorNameProvider *OperatorNameProvider::MakeInstance()
{
    return OperatorNameTable::MakeInstance();
}

OperatorNameProvider *OperatorNameProvider::GetInstance()
{
    return OperatorNameTable::GetInstance();
}

#define NET_GR_QTELCOM_NUMERIC              "20209"
#define NET_GR_WIND_NUMERIC                 "20210"
#define NET_GR_QTELCOM_LONG_ALPHA_EONS      "Q-TELCOM"
#define NET_GR_QTELCOM_SHORT_ALPHA_EONS     "Q-TELCOM"

static void UpdateVendorCustomOperatorName(
        const char *simOperatorNumeric, const char *operatorNumeric,
        char *longAlpha, char *shortAlpha) {
    // vendor customized
    // 1. GR Q-TELCOM(20209) had been disbanded in May 2007.
    //    Q-TELCOM bands are currently used by WIND(20210)
    if (!TextUtils::IsEmpty(simOperatorNumeric)
        && TextUtils::Equals(simOperatorNumeric, NET_GR_QTELCOM_NUMERIC)) {

        if (TextUtils::Equals(operatorNumeric, NET_GR_WIND_NUMERIC)) {
            if (longAlpha != NULL) {
                int len = strlen(NET_GR_QTELCOM_LONG_ALPHA_EONS);
                strncpy(longAlpha, NET_GR_QTELCOM_LONG_ALPHA_EONS, len);
                *(longAlpha + len) = 0;
            }

            if (shortAlpha != NULL) {
                int len = strlen(NET_GR_QTELCOM_SHORT_ALPHA_EONS);
                strncpy(shortAlpha, NET_GR_QTELCOM_SHORT_ALPHA_EONS, len);
                *(shortAlpha + len) = 0;
            }
        }
    }
}

OperatorContentValue OperatorNameProvider::GetVendorCustomOperatorName(const char *simOperatorNumeric, const char *operatorNumeric)
{
    char longAlpha[MAX_FULL_NAME_LEN + 1];
    char shortAlpha[MAX_SHORT_NAME_LEN + 1];
    memset(longAlpha, 0, sizeof(longAlpha));
    memset(shortAlpha, 0, sizeof(shortAlpha));
    UpdateVendorCustomOperatorName(simOperatorNumeric, operatorNumeric, longAlpha, shortAlpha);

    OperatorContentValue ret;
    if (!TextUtils::IsEmpty(longAlpha) || !TextUtils::IsEmpty(shortAlpha)) {
        ret.SetPlmn(operatorNumeric);
        ret.SetLongPlmn(longAlpha);
        ret.SetShortPlmn(shortAlpha);
    }
    return ret;
}
