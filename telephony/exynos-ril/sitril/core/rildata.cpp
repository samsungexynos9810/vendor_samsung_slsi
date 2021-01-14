/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "rildata.h"

/**
 * RilDataString
 */
RilDataString::RilDataString()
{
    m_string = NULL;
}
RilDataString::~RilDataString()
{
    if (m_string != NULL) {
        delete[] m_string;
    }
}

bool RilDataString::SetString(const char *str)
{
    if (m_string != NULL) {
        delete[] m_string;
        m_string = NULL;
    }

    if (str != NULL && *str != 0) {

        int len = strlen(str);
        m_string = new char[len + 1];
        memset(m_string, 0, len + 1);
        strncpy(m_string, str, len);

        length = len + 1;
    }

    return true;
}

bool RilDataString::SetString(int value)
{
    char buf[11] = {0,};
    snprintf(buf, sizeof(buf)-1, "%d", value);
    return SetString(buf);
}

char *RilDataString::GetString()
{
    return m_string;
}

RilDataString *RilDataString::Clone()
{
    RilDataString *p = new RilDataString();
    if (p != NULL) {
        p->SetString(m_string);
    }
    return p;
}

/**
 * RilDataStrings
 */
RilDataStrings::RilDataStrings(int c) : RilData(), m_strings(NULL), m_count(0)
{
    if (c > 0) {
        m_count = c;
        m_strings = (char **)new char*[m_count];
        memset(m_strings, 0, sizeof(char *) * c);
    }
}

RilDataStrings::~RilDataStrings()
{
    if (m_strings != NULL) {
        for (int i = 0; i < m_count; i++) {
            char *p = *(m_strings + i);
            if (p != NULL) {
                delete[] p;
            }
        } // end for i ~
        delete[] m_strings;
    }
}

bool RilDataStrings::SetString(int index, const char *str)
{
    int len = 0;
    if (str != NULL) {
        len = strlen(str);
    }
    return SetString(index, str, len);
}

bool RilDataStrings::SetString(int index, int value)
{
    char buf[10] = {0,};
    int len = 0;
    len = snprintf(buf, sizeof(buf)-1,"%d", value);
    return SetString(index, buf, len);
}

bool RilDataStrings::SetString(int index, const char *str, int len)
{
    if (index < 0 || index >= m_count)
        return false;

    char **p = m_strings + index;
    if (*p != NULL) {
        delete[] *p;
    }

    if (str == NULL || *str == 0 || len == 0) {
        *p = NULL;
        return true;
    }

    *p = new char[len + 1];
    if (*p != NULL) {
        memset(*p, 0, len + 1);
        strncpy(*p, str, len);
    }
    return true;
}

char *RilDataStrings::GetString(int index)
{
    if (index < 0 || index >= m_count)
        return NULL;
    if (m_strings == NULL)
        return NULL;

    return *(m_strings + index);
}

RilDataStrings *RilDataStrings::Clone()
{
    RilDataStrings *p = new RilDataStrings(m_count);
    if (p != NULL) {
        for (int i = 0; i < m_count; i++) {
            p->SetString(i, this->GetString(i));
        } // end for i ~
    }
    return p;
}


/**
 * RilDataInts
 */
RilDataInts::RilDataInts(int c) : RilData(), m_ints(NULL), m_count(0)
{
    if (c > 0) {
        m_count = c;
        m_ints = new int[c];
        memset(m_ints, 0, sizeof(int) * c);
    }
}
RilDataInts::~RilDataInts()
{
    if (m_ints != NULL) {
        delete[] m_ints;
        m_ints = NULL;
    }
}

bool RilDataInts::SetInt(int index, int value)
{
    if (index < 0 || index >= m_count)
        return false;

    m_ints[index] = value;
    return true;
}

int RilDataInts::GetInt(int index) const
{
    return m_ints[index];
}

RilDataInts *RilDataInts::Clone()
{
    RilDataInts *p = new RilDataInts(m_count);
    if (p != NULL) {
        for (int i = 0; i < m_count; i++) {
            p->SetInt(i, this->GetInt(i));
        } // end for i ~
    }
    return p;
}


RilDataRaw::RilDataRaw() : m_pData(NULL), m_nSize(0)
{

}

RilDataRaw::RilDataRaw(const void *pData, int nSize) : m_pData(NULL), m_nSize(0)
{
    SetData(pData, nSize);
}

RilDataRaw::~RilDataRaw()
{
    if(m_pData != NULL) {
        delete[] (char *)m_pData;
        m_pData = NULL;
    }
}

bool RilDataRaw::SetData(const void *pData, int nSize)
{
    if(m_pData != NULL) {
        delete[] (char *)m_pData;
        m_pData = NULL;
    }

    if (pData != NULL && nSize > 0) {
        m_pData = new char[nSize];
        m_nSize = nSize;
        memcpy(m_pData, pData, nSize);
    }

    return true;
}

RilDataRaw *RilDataRaw::Clone()
{
    RilDataRaw *p = new RilDataRaw(m_pData, m_nSize);
    return p;
}
