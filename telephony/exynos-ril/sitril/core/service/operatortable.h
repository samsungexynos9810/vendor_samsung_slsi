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
 * operatortable.h
 *
 *  Created on: 2014. 7. 5.
 *      Author: sungwoo48.choi
 */

#ifndef __OPERATOR_TABLE_H__
#define __OPERATOR_TABLE_H__

#include "rildef.h"

class OperatorContentValue
{
public:
    char m_plmn[MAX_PLMN_LEN + 1];
    char m_shortPlmn[MAX_SHORT_NAME_LEN + 1];
    char m_longPlmn[MAX_FULL_NAME_LEN + 1];

public:
    OperatorContentValue();
    OperatorContentValue(const char *plmn, const char *longPlmn, const char *shortPlmn);
    virtual ~OperatorContentValue();

public:
    void SetPlmn(const char *plmn);
    const char *GetPlmn() const;
    void SetShortPlmn(const char *shortPlmn);
    const char *GetShortPlmn() const;
    void SetLongPlmn(const char *longPlmn);
    const char *GetLongPlmn() const;
    void Update(const char *plmn, const char *longPlmn, const char *shortPlmn);
    void Update(const char *longPlmn, const char *shortPlmn);

    // operator override
public:
    virtual bool operator==(const OperatorContentValue &rhs);
    virtual bool operator!=(const OperatorContentValue &rhs);
    virtual OperatorContentValue &operator=(const OperatorContentValue &rhs);

    static OperatorContentValue *NewInstance(const char *plmn, const char *longPlmn = NULL, const char *shortPlmn = NULL);
};

class EonsContentsValue : public OperatorContentValue
{
public:
    char m_iccId[MAX_ICCID_LEN + 1];

public:
    EonsContentsValue();
    EonsContentsValue(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn);
    virtual ~EonsContentsValue();

public:
    void SetIccId(const char *iccId);
    const char *GetIccId() const;

    // operator override
public:
    static EonsContentsValue *NewInstance(const char *iccId, const char *netPlmn, const char *longPlmn = NULL, const char *shortPlmn = NULL);
};

class OperatorNameProvider {
public:
    OperatorNameProvider() {}
    virtual ~OperatorNameProvider() {}

    bool isValidPlmn(const char *plmn);

public:
    virtual bool Contains(const char *plmn)=0;
    virtual OperatorContentValue *Find(const char *plmn)=0;
    virtual bool Insert(const char *plmn, const char *longPlmn, const char *shortPlmn)=0;
    virtual bool Insert(const OperatorContentValue *contentValue)=0;
    virtual bool Update(const char *plmn, const char *longPlmn, const char *shortPlmn)=0;
    virtual bool Update(const OperatorContentValue *contentValue)=0;

   virtual  bool InsertEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn)=0;
   virtual bool UpdateEons(const char *iccId, const char *netPlmn, const char *longPlmn, const char *shortPlmn)=0;
   virtual EonsContentsValue *FindEons(const char *iccId, const char *netPlmn)=0;
public:
    static OperatorNameProvider *MakeInstance();
    static OperatorNameProvider *GetInstance();

    static OperatorContentValue GetVendorCustomOperatorName(const char *simOperatorNumeric, const char *operatorNumeric);
};

#endif /* __OPERATOR_TABLE_H__ */
