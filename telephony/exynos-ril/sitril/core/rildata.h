/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __RIL_DATA_H__
#define    __RIL_DATA_H__

#include "types.h"
#include "rildef.h"

class RilData {
public:
    void *rildata;
    unsigned int length;

public:
    RilData() : rildata(NULL), length(0) {}
    virtual ~RilData() {
        if (rildata != NULL) delete (BYTE *)rildata;
        rildata = NULL;
    }

public:
    virtual void *GetData() const { return rildata; }
    virtual unsigned int GetDataLength() const { return length; }
    virtual RilData *Clone() { return NULL; }
};

class RilDataString : public RilData {
private:
    char *m_string;
public:
    RilDataString();
    virtual ~RilDataString();

public:
    void *GetData() const { return m_string; }
    bool SetString(const char *str);
    bool SetString(int value);
    char *GetString();
    RilDataString *Clone();
};

class RilDataStrings : public RilData {
private:
    char **m_strings;
    int m_count;
public:
    RilDataStrings(int c);
    virtual ~RilDataStrings();

public:
    void *GetData() const { return m_strings; }
    unsigned int GetDataLength() const { return sizeof(char *) * m_count; }
    bool SetString(int index, const char *str);
    bool SetString(int index, int value);
    bool SetString(int index, const char *str, int len);
    char *GetString(int index);
    RilDataStrings *Clone();
};

class RilDataInts : public RilData {
private:
    int *m_ints;
    int m_count;
public:
    RilDataInts(int c);
    virtual ~RilDataInts();

public:
    void *GetData() const { return m_ints; }
    unsigned int GetDataLength() const { return sizeof(int) * m_count; }
    bool SetInt(int index, int value);
    int GetInt(int index) const;
    RilDataInts *Clone();
};

class RilDataRaw : public RilData {
private:
    void *m_pData;
    int m_nSize;
public:
    RilDataRaw();
    RilDataRaw(const void *pData, int nSize);
    virtual ~RilDataRaw();

public:
    void *GetData() const { return m_pData; }
    unsigned int GetDataLength() const { return m_nSize; }
    bool SetData(const void *pData, int nSize);
    RilDataRaw *Clone();
};

#endif // __RIL_DATA_H__
