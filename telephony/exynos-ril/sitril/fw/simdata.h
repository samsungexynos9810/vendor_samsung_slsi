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
 * simdata.h
 *
 *  Created on: 2014. 6. 28.
 *      Author: mox
 */

#ifndef _SIM_DATA_H_
#define _SIM_DATA_H_

#include "requestdata.h"

/**
 * Verify PIN and PUK, and Change PIN
 */
class VerifyPIN : public StringsRequestData
{
public:
    char *GetPIN() { return (m_nStringCount>0)? m_ptrStrings[0]: NULL; }
    char *GetAID() { return (m_nStringCount>1)? m_ptrStrings[1]: NULL; }
};

class VerifyPUK : public StringsRequestData
{
public:
    char *GetPUK() { return (m_nStringCount>0)? m_ptrStrings[0]: NULL; }
    char *GetNewPIN() { return (m_nStringCount>1)? m_ptrStrings[1]: NULL; }
    char *GetAID() { return (m_nStringCount>2)? m_ptrStrings[2]: NULL; }
};

class ChangePIN : public StringsRequestData
{
public:
    char *GetOldPIN() { return (m_nStringCount>0)? m_ptrStrings[0]: NULL; }
    char *GetNewPIN() { return (m_nStringCount>1)? m_ptrStrings[1]: NULL; }
    char *GetAID() { return (m_nStringCount>2)? m_ptrStrings[2]: NULL; }
};

/**
 * Verify Network Lock
 */
class VerifyNetLock : public StringsRequestData
{
public:
    char *GetDepersonalCode() { return (m_nStringCount>0)? m_ptrStrings[0]: NULL; }
};

/**
 * SIM IO
 */
class SimIoData : public RequestData
{
    public:
        SimIoData(const int nReq, const Token tok, const ReqType type = REQ_FW);
        SimIoData(const SimIoData &simIo);
        virtual ~SimIoData();

        SimIoData & operator=(const SimIoData &simIo);
        INT32 GetCmd() { return m_nCmd; }
        INT32 GetFileId() { return m_nFileId; }
        INT32 GetIndex() { return m_nP1; }
        virtual INT32 encode(char *data, unsigned int length);

    private:
        void SetRawData(char *hexStr);

    public:
        INT32 m_nCmd;
        INT32 m_nFileId;
        char m_strPath[MAX_SIM_PATH_LEN+1];
        INT32 m_nP1;
        INT32 m_nP2;
        INT32 m_nP3;
        INT32 m_nDataLen;       // Data length
        char m_strData[MAX_SIM_DATA_LEN+1];
        char m_strPin2[MAX_SIM_PIN_LEN+1];
        char m_strAid[(MAX_SIM_AID_LEN*2)+1];

    protected:
        char *m_strHex;
};

/**
 * Facility Lock
 */
class FacilityLock
{
public:
    FacilityLock();

    typedef enum {
        FAC_UNKNOWN = -1,
        FAC_CS,
        FAC_PS,
        FAC_PF,
        FAC_SC,
        FAC_AO,
        FAC_OI,
        FAC_OX,
        FAC_AI,
        FAC_IR,
        FAC_NT,
        FAC_NM,
        FAC_NS,
        FAC_NA,
        FAC_AB,
        FAC_AG,
        FAC_AC,
        FAC_FD,
        FAC_PN,
        FAC_PU,
        FAC_PP,
        FAC_PC,
        FAC_SC2,

        FAC_MAX_COUNT
    } FAC_CODE;

    const char *GetCode() { return m_szCode; }
    int GetLockState() { return m_nLockState; }
    char *GetPassword() { return m_szPassword; }
    int GetServiceClass() { return m_nServiceClass; }
    char *GetAID() { return m_szAID; }

    BOOL Parse(StringsRequestData &strReqData);

private:
    char m_szCode[MAX_FACILITY_CODE_LEN+1];
    int m_nLockState;
    char m_szPassword[MAX_PASSWORD_LEN+1];
    int m_nServiceClass;
    char m_szAID[(MAX_SIM_AID_LEN*2)+1];
};

/**
 * ISIM Authentication
 */
class IsimAuth
{
public:
    IsimAuth();
    virtual ~IsimAuth();

    BYTE *GetAuth() { return m_pAuth; }
    int GetLength() { return m_nLength; }
    int GetAuthType() { return m_nAuthType; }

    BOOL Parse(StringRequestData *pString);
    BOOL ParseWithAuthType(StringRequestData *pString);
    void Erase(void);

private:
    int m_nLength;
    BYTE *m_pAuth;
    int m_nAuthType;
};

/**
 * SIM Authentication
 */
class SimAuthentication : public RequestData
{
public:
    SimAuthentication(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~SimAuthentication();

    virtual INT32 encode(char *data, unsigned int length);

    int GetAuthContext() { return m_nAuthContext; }
    char *GetAuthentication() { return m_pAuth; }
    char *GetAid() { return m_aid; }

private:
    int m_nAuthContext;
    char *m_pAuth;
    char *m_aid;
};

class SimGbaAuth
{
public:
    SimGbaAuth();
    virtual ~SimGbaAuth();
    BYTE *GetAuth() { return m_pAuth; }
    int GetLength() { return m_nLength; }
    BYTE GetGbaType() { return m_gba_type; }
    BYTE GetGbaTag() { return m_gba_tag; }
    BOOL Parse(StringRequestData *pString);
    void Erase(void);
private:
    BYTE m_gba_type;
    BYTE m_gba_tag;
    int m_nLength;
    BYTE *m_pAuth;
};

/**
 * Sim APDU
 */

class SimAPDU : public RequestData
{
public:
    SimAPDU(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~SimAPDU();

    int GetSessionId() { return m_nSessionid; }
    int GetCla() { return m_nCla; }
    int GetInstruction() { return m_nInstruction; }
    int GetP1() { return m_nP1; }
    int GetP2() { return m_nP2; }
    int GetP3() { return m_nP3; }
    void SetP3(int p3);
    int GetDataLength() { return m_nDataLength; }
    char *GetData() { return m_pData;}

    virtual INT32 encode(char *data, unsigned int length);


private:
    int m_nSessionid;
    int m_nCla;
    int m_nInstruction;
    int m_nP1;
    int m_nP2;
    int m_nP3;
    int m_nDataLength;
    char *m_pData;
};

class SimUpdatePbEntry : public RequestData
{
public:
    SimUpdatePbEntry(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~SimUpdatePbEntry();
    virtual INT32 encode(char *data, unsigned int length);

    int GetMode() { return m_mode; };
    int GetPbType() { return m_type; };
    int GetIndex() { return m_index; };
    int GetLength() { return m_length; };
    char *GetEntryData() { return m_entry; };

private:
    int m_mode;
    int m_type;
    int m_index;
    int m_length;
    char m_entry[MAX_PB_ENTRY_LEN];
};

/**
 * Sim Open Channel
 */

class SimOpenChannel : public RequestData
{
public:
    SimOpenChannel(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~SimOpenChannel();
    virtual INT32 encode(char *data, unsigned int length);

    char * GetAid() { return m_aid; };
    int GetP2() { return m_p2; };

private:
    char * m_aid;
    int m_length;
    int m_p2;
};

/**
 * Sim Open Channel with P2
 */

class SimOpenChannelWithP2 : public RequestData
{
public:
    SimOpenChannelWithP2(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~SimOpenChannelWithP2();

    int GetP2() { return m_nP2; }
    int GetAidLen() { return m_nAidLen; }
    char *GetAid() { return m_pAid;}

    virtual INT32 encode(char *data, unsigned int length);

private:
    int m_nP2;
    int m_nAidLen;
    char m_pAid[MAX_SIM_AID_LEN];
};

class CarrierRestrictionsData : public RequestData
{
public:
    CarrierRestrictionsData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~CarrierRestrictionsData();

    int GetLenAllowCarrier() { return m_nLenAllowedCarriers; }
    int GetLenExcludeCarrier() { return m_nLenExcludedCcarriers; }
    CarrierInfo *GetAllowCarrier() { return m_pAllowedCarriers;}
    CarrierInfo *GetExcludeCarrier() { return m_pExcludedCarriers;}

    virtual INT32 encode(char *data, unsigned int length);

private:
    int m_nLenAllowedCarriers;
    int m_nLenExcludedCcarriers;
    CarrierInfo * m_pAllowedCarriers;
    CarrierInfo * m_pExcludedCarriers;

    int copyCarrierInfo(CarrierInfo *dstInfo, RIL_Carrier *srcInfo);
};

#endif /*_SIM_DATA_H_*/
