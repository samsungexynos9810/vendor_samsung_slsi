/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "requestdata.h"

#define    INVALID_INT        0x7FFFFFFF

RequestData::RequestData()
    :m_nReq(-1), m_tok(0), m_reqType((ReqType)0)
{
    mHalVer = HAL_VERSION_CODE(1,0);
}

RequestData::RequestData(const int nReq, const Token tok, const ReqType type)
    : m_tok(tok), m_reqType(type)
{
    m_nReq = DECODE_REQUEST(nReq);
    mHalVer = DECODE_HAL(nReq);
}

RequestData *RequestData::Clone() const
{
    // base cloning function, cloning RequestData instance
    return new RequestData(m_nReq, m_tok, m_reqType);
}


/**
 * IntRequestData
 */
IntRequestData::IntRequestData(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type), m_nInt(INVALID_INT)
{
}

IntRequestData::~IntRequestData()
{
}

int IntRequestData::encode(char *data, unsigned int datalen)
{
    if((0 == datalen) || (NULL == data))
        return -1;
    m_nInt = *(int *)data;
    return 0;
}

IntRequestData *IntRequestData::Clone() const
{
    IntRequestData *p = new IntRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL) {
        p->m_nInt = m_nInt;
    }
    return p;
}

/**
 * IntsRequestData
 */
IntsRequestData::IntsRequestData(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type)
    , m_pInts(NULL), m_nSize(0)
{
}

IntsRequestData::~IntsRequestData()
{
    if (m_pInts != 0) {
        delete[] m_pInts;
        m_pInts = NULL;
    }
    m_nSize = 0;
}

int IntsRequestData::encode(char *data, unsigned int datalen)
{
    if (data != NULL && datalen > 0 && datalen % sizeof(int) == 0) {
        m_nSize = datalen / sizeof(int);
        m_pInts = new int[m_nSize];

        for (int i = 0; i < m_nSize; i++) {
            *(m_pInts + i) = *((int *)data + i);
        } // end for i ~
        return 0;
    }
    return -1;
}

IntsRequestData *IntsRequestData::Clone() const
{
    IntsRequestData *p = new IntsRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL) {
        p->m_nSize = m_nSize;
        p->m_pInts = new int[m_nSize];
        for (int i = 0; i < m_nSize; i++) {
            *(p->m_pInts + i) = *(m_pInts + i);
        } // end for i ~
    }
    return p;
}

int IntsRequestData::GetInt(int index) const
{
    if (m_pInts != NULL && 0 <= index && index < m_nSize) {
        return *(m_pInts + index);
    }
    return 0x7FFFFFFF;
}


/**
 * StringRequestData
 */
StringRequestData::StringRequestData(const int nReq,const Token tok,const ReqType type)
    :RequestData(nReq, tok, type), m_szData(NULL)
{
}


StringRequestData::~StringRequestData()
{
    if(m_szData != NULL) {
        delete []m_szData;
        m_szData = NULL;
    }
}

int StringRequestData::encode(char *data, unsigned int datalen)
{
    if((0 == datalen) || (NULL == data) || (0==strlen(data)))
        return 0;

    int stringLength = strlen(data);

    m_szData = new char[stringLength + 1];
    memset(m_szData, 0, stringLength + 1);
    strncpy(m_szData, data, stringLength);

    return 0;
}

StringRequestData *StringRequestData::Clone() const
{
    StringRequestData *p = new StringRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL && m_szData != NULL) {
        int len = strlen(m_szData);
        p->m_szData = new char[len + 1];
        memset(m_szData, 0, len + 1);
        strncpy(p->m_szData, m_szData, len);
    }
    return p;
}

/**
 * StringsRequestData
 */
StringsRequestData::StringsRequestData(const int nReq,const Token tok,const ReqType type)
    :RequestData(nReq, tok, type), m_nStringCount(-1), m_ptrStrings(NULL)
{
}


StringsRequestData::~StringsRequestData()
{
    if(m_nStringCount > 0 && m_ptrStrings)
    {
        for(int i=0;i<m_nStringCount;i++)
        {
            delete [](m_ptrStrings[i]);
            m_ptrStrings[i] = NULL;
        }
    }
    delete[] m_ptrStrings;
    m_ptrStrings = NULL;
}

int StringsRequestData::encode(char *data, unsigned int datalen)
{
    if((0 == datalen) || (NULL == data))
        return 0;

    m_nStringCount = datalen/sizeof(char *);
    char *pVal = NULL;
    char **pCur = (char **)data;

    m_ptrStrings = (char **)new char*[m_nStringCount];
    memset(m_ptrStrings, 0, m_nStringCount*sizeof(char *));

    for(int i = 0; i < m_nStringCount; i++) {
        if(pCur[i] == NULL) {
            m_ptrStrings[i] = NULL;
        }
        else {
            int nCount = strlen(pCur[i]);
            pVal = new char[nCount +1];
            if (pVal != NULL) {
                memset(pVal, 0, nCount+1);
                strncpy(pVal, pCur[i], nCount);
            }
            m_ptrStrings[i] = pVal;
        }
    } // end for i ~
    return 0;
}

StringsRequestData *StringsRequestData::Clone() const
{
    StringsRequestData *p = new StringsRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL) {
        p->encode((char *)m_ptrStrings, m_nStringCount * sizeof(char *));
    }
    return p;
}

const char *StringsRequestData::GetString(int index) const
{
    if (m_ptrStrings != NULL && 0 <= index && index < m_nStringCount) {
        return m_ptrStrings[index];
    }
    return NULL;
}

/**
 * RawRequestData
 */
RawRequestData::RawRequestData(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type), m_pRawData(NULL), m_nSize(0)
{
}

RawRequestData::~RawRequestData()
{
    if (m_pRawData != NULL) {
        delete[] m_pRawData;
        m_pRawData = NULL;
    }
    m_nSize = 0;
}

int RawRequestData::encode(char *data, unsigned int datalen)
{
    if (data != NULL && datalen > 0) {
        m_pRawData = new char[datalen];
        if (m_pRawData != NULL) {
            memcpy(m_pRawData, data, datalen);
            m_nSize = (int)datalen;
        }
    }

    return 0;
}

RawRequestData *RawRequestData::Clone() const
{
    RawRequestData *p = new RawRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL) {
        p->encode((char *)m_pRawData, m_nSize);
    }
    return p;
}
