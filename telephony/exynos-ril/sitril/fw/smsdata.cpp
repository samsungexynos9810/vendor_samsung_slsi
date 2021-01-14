/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/**
 * @file    smsdata.cpp
 *
 * @brief   Definition of data classes used in RIL.
 *
 * @author  HyunJoon Kang (arkade.kang@samsung.com)
 *          RyongGyoung Kwon (reika.kwon@samsung.com)
 *
 * @version Unspecipied.
 */


#include "smsdata.h"
#include "util.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/**
 * PDU (Protocol Data Unit)
 */
Pdu::Pdu() : mLen(0)
{
    memset(mData, 0, sizeof(mData));
    mHex = NULL;
}

Pdu::Pdu(char *data, int len) : mLen(0)
{
    memset(mData, 0, sizeof(mData));
    if (data && (len > 0 && len <= MAX_GSM_SMS_TPDU_SIZE))
    {
        memcpy(mData, data, len);
        mLen = len;
    }
    mHex = NULL;
}

Pdu::Pdu(char *hexStr)
{
    memset(mData, 0, sizeof(mData));
    mLen = 0;
    mHex = NULL;

    if (hexStr)
    {
        SetRawData(hexStr);
    }
}

Pdu::Pdu(const Pdu &pdu)
{
    memcpy(mData, pdu.mData, sizeof(mData));
    mLen = pdu.mLen;
    mHex = NULL;
}

Pdu::~Pdu()
{
    if (mHex)
    {
        delete [] mHex;
        mHex = NULL;
    }
}

Pdu & Pdu::operator=(const Pdu &pdu)
{
    if (&pdu != this)
    {
        memcpy(mData, pdu.mData, sizeof(mData));
        mLen = pdu.mLen;

        if (mHex)
        {
            delete [] mHex;
            mHex = NULL;
        }
    }
    return *this;
}

char * Pdu::ToHexString()
{
    if (mHex == NULL)
    {
        ConvertToHexString(&mHex, mData, mLen);
    }
    return mHex;
}

void Pdu::SetRawData(char *hexStr)
{
    if (mHex)
    {
        delete [] mHex;
        mHex = NULL;
    }

    memset(mData, 0, sizeof(mData));
    mLen = ConvertToRaw(hexStr, mData, sizeof(mData));
}

/**
 * SMS GSM Address
 */
GsmSmsAddress::GsmSmsAddress()
{
    mLen = 0;
    mString = NULL;
    memset(mAddr, 0, sizeof(mAddr));
}

GsmSmsAddress::GsmSmsAddress(char *data, int len)
{
    if (len > MAX_GSM_SMS_SERVICE_CENTER_ADDR)
    {
        len = MAX_GSM_SMS_SERVICE_CENTER_ADDR;
    }

    memset(mAddr, 0, sizeof(mAddr));
    memcpy(mAddr, data, len);
    mLen = len;
    mString = NULL;
}

GsmSmsAddress::~GsmSmsAddress()
{
    if (mString)
    {
        delete [] mString;
    }
}

bool GsmSmsAddress::operator==(const GsmSmsAddress & a) const
{
    return (mLen == a.mLen) && (memcmp(mAddr, a.mAddr, mLen + 2) == 0);
}

char * GsmSmsAddress::ToString()
{
    if (mString == NULL)
    {
        int addrLen, i, n = 0;
        int b;
        addrLen = mAddr[0];
        mString = new char[addrLen + 1];
        if (mString == NULL)
        {
            RilLogE("%s: Can't alloc memory for GsmSmsAddress::mString.", __FUNCTION__);
            return NULL;
        }

        for (i = 0; i < (mLen - 2); i++)
        {
            b = mAddr[i + 2] & 0xf;
            n += sprintf(mString + n, "%d", b);

            if ((i + 1) * 2 <= addrLen)
            {
                b = (mAddr[i + 2] >> 4) & 0xf;
                n += sprintf(mString + n, "%d", b);
            }
        }
    }

    return mString;
}


/**
 * SMS center
 */
SmsCenter::SmsCenter()
{
    memset(mAddr, 0, sizeof(mAddr));
    mHex = NULL;
    mLen = 0;
    mType = 0;
}

SmsCenter::SmsCenter(SmsCenter *smsc)
{
    mHex = NULL;
    mType = 0;
    // mLen = 1 and mAddr[0] = 0 are needed to create SMS TPDU when invalid
    // or no smsc is received from F/W for normal SMS or SIM SMS.
    mLen = 1;
    memset(mAddr, 0, sizeof(mAddr));

    if (smsc)
    {
        memcpy(mAddr, smsc->mAddr, sizeof(smsc->mAddr));
        mLen = smsc->mLen;
    }
}

/* smsc: hexastring including SCA length */
SmsCenter::SmsCenter(char *smsc)
{
    mHex = NULL;
    mType = 0;
    // mLen = 1 and mAddr[0] = 0 are needed to create SMS TPDU when invalid
    // or no smsc is received from F/W for normal SMS or SIM SMS.
    mLen = 1;
    memset(mAddr, 0, sizeof(mAddr));

    if (smsc && (strlen(smsc) <= (MAX_GSM_SMS_SERVICE_CENTER_ADDR*2)))
    {
        mLen = ConvertToRaw(smsc, mAddr, MAX_GSM_SMS_SERVICE_CENTER_ADDR);
    }
}

/* smsc: byte stream including SCA length
 * len: length filed of SCA */
SmsCenter::SmsCenter(char *smsc, int len)
{
    int scaLen;

    mHex = NULL;
    mType = 0;

    if (len > MAX_GSM_SMS_SERVICE_CENTER_ADDR)
    {
        RilLogE("invalid SCA length.");
        mLen = 1;
        memset(mAddr, 0, sizeof(mAddr));
        return;
    }

    scaLen = smsc[0];
    if (scaLen > (len - 1))
    {
        RilLogW("invalid SCA length.");
        scaLen = len - 1;
    }

    mLen = scaLen + 1;

    memcpy(mAddr, smsc, mLen);
}

SmsCenter::~SmsCenter()
{
    if (mHex)
    {
        delete [] mHex;
        mHex = NULL;
    }
}

char * SmsCenter::ToHexString()
{
    if (mHex == NULL)
    {
        ConvertToHexString(&mHex, mAddr, mLen);
    }
    return mHex;
}

bool SmsCenter::UseRilSmsc()
{
    return (mLen <= 1);
}

/**
 * SMS message
 */
SmsMessage::SmsMessage(const int nReq, const Token tok, const ReqType type)
:RequestData(nReq, tok, type)
{
    Init();
}

SmsMessage::~SmsMessage()
{
    if (mPdu)
    {
        delete mPdu;
    }

    if (mHex)
    {
        delete [] mHex;
    }
}

void SmsMessage::Init()
{
    mPdu = NULL;
    mHex = NULL;
    mSimIndex = 0xFFFF;
}


int SmsMessage::encode(char *data, unsigned int datalen)
{
    return 0;
}


/*
 * GSM SMS message
 */
GsmSmsMessage::GsmSmsMessage(const int nReq, const Token tok, const ReqType type)
:SmsMessage(nReq, tok, type)
{
    Init();
}
/*

GsmSmsMessage::GsmSmsMessage(SmsCenter *smsc, Pdu *pdu)
{
    Init();

    mSmsc = smsc;
    mPdu = pdu;
}
*/
GsmSmsMessage::GsmSmsMessage(GsmSmsMessage *msg):SmsMessage(-1, NULL, REQ_FW)
{
    Init();

    if (msg->mSmsc)
    {
        mSmsc = new SmsCenter(msg->mSmsc->mAddr, msg->mSmsc->mLen);
    }
    if (msg->mPdu)
    {
        mPdu = new Pdu(msg->mPdu->mData, msg->mPdu->mLen);
    }

    mSimIndex = msg->mSimIndex;
}

GsmSmsMessage::~GsmSmsMessage()
{
    if (mSmsc)
    {
        delete mSmsc;
    }
}

void GsmSmsMessage::Init()
{
    SmsMessage::Init();

    mSmsc = NULL;
    mClass = -1;                 // Message Class
    mMessageRef = -1;
}


int GsmSmsMessage::encode(char *data, unsigned int datalen)
{
    Init();

    char **p = (char **)data;
    mSmsc = new SmsCenter(p[0]);
    mPdu = new Pdu(p[1]);
    return 0;
}

char * GsmSmsMessage::GetRawByte()
{
    static char data[MAX_GSM_SMS_TPDU_SIZE + MAX_GSM_SMS_SERVICE_CENTER_ADDR];

    memcpy(data, mSmsc->mAddr, mSmsc->mLen);
    memcpy(data + mSmsc->mLen, mPdu->mData, mPdu->mLen);

    return data;
}

int GsmSmsMessage::GetLength()
{
    return mSmsc->mLen + mPdu->mLen;
}

char * GsmSmsMessage::ToHexString()
{
    if (mHex == NULL)
    {
        char *smsc = NULL, *pdu = NULL;
        unsigned int smscLen, pduLen;

        smsc = mSmsc->ToHexString();
        pdu = mPdu->ToHexString();
        if (smsc != NULL && pdu != NULL) {
            smscLen = strlen(smsc);
            pduLen = strlen(pdu);

            /*if ( smscLen >= (SIZE_MAX/2) || pduLen >= (SIZE_MAX/2) )
            {
                RilLogE("%s: Integer overflowed can be happen -> block", __FUNCTION__);
                return NULL;
            }*/

            if ( smscLen <= (MAX_GSM_SMS_SERVICE_CENTER_ADDR*2)
                && pduLen <= (MAX_GSM_SMS_TPDU_SIZE*2) ) {
                unsigned int size = smscLen + pduLen + 1;
                mHex = new char[size];
                if (mHex == NULL)
                {
                    RilLogE("%s: Can't allocate memory.", __FUNCTION__);
                    return NULL;
                }
                memset(mHex, 0, size);
                snprintf(mHex, size - 1, "%s%s", smsc, pdu);
            }
        }
    }
    return mHex;
}

PduParser::PduParser()
{
    mPdu = NULL;
    m_cPdu = NULL;
    mCur = 0;
    m_scalen = 0;
    *m_sca = '\0';
    m_smsclass = MESSAGE_CLASS_0;
}

PduParser::PduParser(Pdu *pdu)
{
    mPdu = pdu;
    mCur = 0;
    m_cPdu = NULL;
    m_scalen = 0;
    *m_sca = '\0';
    m_smsclass = MESSAGE_CLASS_0;
}

PduParser::~PduParser()
{
}

int PduParser::GetByte()
{
    int b;

    b = mPdu->mData[mCur++] & 0xff;

    if (mCur == mPdu->mLen)
    {
        mCur = mPdu->mLen - 1;
    }

    return b;
}


long PduParser::GetSCTimestamp()
{
    // TODO

    mCur += 7;

    return 0;
}

GsmSmsAddress * PduParser::GetAddress()
{
    GsmSmsAddress *ret;

    int addressLength = mPdu->mData[mCur] & 0xff;
    int lengthBytes = 2 + (addressLength + 1) / 2;

    ret = new GsmSmsAddress(&mPdu->mData[mCur], lengthBytes);
    mCur += lengthBytes;

    return ret;
}

int PduParser::GetScaLen(const void *pdu)
{
    m_cPdu = (char *)pdu;
    if (SMS_DBG) RilLogV("[1] PDU = %s", m_cPdu);

    m_scalen = (HexChar2Value(m_cPdu[0]) << 4 )| (HexChar2Value(m_cPdu[1]));
    if (SMS_DBG) RilLogV("[2] SCA_Len = %d", m_scalen);

    return m_scalen;
}

char *PduParser::GetSca(const void *pdu)
{
    if (GetScaLen(pdu) == 0)
    {
        RilLogW("SCA length is 0");
    }

    memcpy(m_sca, m_cPdu, (m_scalen+1)*2);
    if (SMS_DBG) RilLogV("[3] SCA = %s", m_sca);

    return m_sca;
}

int PduParser::GetSmsClass(const void *pdu)
{
    int do_len;

    if (GetSca(pdu) == NULL)
    {
        RilLogW("SCA is null");
    }

    m_cPdu += (m_scalen+1)*2;                               // SCA skip
    if (SMS_DBG) RilLogV("[4] PDU(SCA Skip) = %s", m_cPdu);

    m_cPdu += 2;                                               // PDU Type skip
    if (SMS_DBG) RilLogV("[5] PDU(PDU_Type Skip) = %s", m_cPdu);

    do_len = (HexChar2Value(m_cPdu[0]) << 4 )| (HexChar2Value(m_cPdu[1]));
    if (SMS_DBG) RilLogV("[6] DO_Len = %d", do_len);

    if ((do_len % 2) == 0)    m_cPdu = m_cPdu + (do_len+4);
    else                    m_cPdu = m_cPdu + (do_len+5);

    m_cPdu += 2;                                            // PID skip
    if (SMS_DBG) RilLogV("[7] PDU(PID Skip) = %s", m_cPdu);

    m_smsclass = HexChar2Value(m_cPdu[1]) & 0x3;

    if (HexChar2Value(m_cPdu[0])&0x1)    RilLogV("SMS Class: %d", m_smsclass);
    else                                RilLogV("SMS Class: %s", "No Class");

    return m_smsclass;
}

SmsAcknowledge::SmsAcknowledge():RequestData(-1, NULL, REQ_FW)
{
    mResult = -1;
    mFailureCause = -1;
}

SmsAcknowledge::SmsAcknowledge(int result, int cause):RequestData(-1, NULL, REQ_FW)
{
    mResult = result;
    mFailureCause = cause;
}

SmsAcknowledge::SmsAcknowledge(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type)
{
    mResult = -1;
    mFailureCause = -1;
}

SmsAcknowledgePdu::SmsAcknowledgePdu(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type)
{
    mResult = -1;
    mPdu = NULL;
}

int SmsAcknowledge::encode(char *data, unsigned int datalen)
{
    int *p = (int *)data;
    mResult = p[0];
    mFailureCause = p[1];
    return 0;
}

int SmsAcknowledgePdu::encode(char *data, unsigned int datalen)
{
    char **p = (char **)data;
    mResult = *p[0];
    mPdu = new Pdu(p[1]);

    return 0;
}

/**
 * SimSMS message
 */
SimSmsMessage::SimSmsMessage(const int nReq, const Token tok, const ReqType type)
:GsmSmsMessage(nReq, tok, type)
{
    mStatus = -1;
    mStorage = -1;
}

/*
SimSmsMessage::SimSmsMessage(int index)
{
    mStatus = -1;
    mStorage = -1;
    mSimIndex = index;
}

SimSmsMessage::SimSmsMessage(int status,  Pdu *pdu, SmsCenter *smsc, bool class2Flag)
{
    mStatus = status;
    mPdu = pdu;
    mSmsc = smsc;
    if (class2Flag) {
        mClass = MESSAGE_CLASS_2;
    } else {
        mClass = MESSAGE_CLASS_UNKNOWN;
    }
    mStorage = -1;
}
*/

int SimSmsMessage::encode(char *data, unsigned int datalen)
{
    if(0 == datalen || NULL == data)
        return -1;
    RIL_SMS_WriteArgs *arg_wrg = (RIL_SMS_WriteArgs *)data;
    mPdu = new Pdu(arg_wrg->pdu);
    mSmsc = new SmsCenter(arg_wrg->smsc);
    mStatus = arg_wrg->status;
    mClass = MESSAGE_CLASS_0;

#if 0
    if (arg_wrg->smsc != NULL)
    {
        if (strcmp(arg_wrg->smsc, "class2") == 0)
        {
            mClass = MESSAGE_CLASS_2;
        }
    }
#endif

#if defined (GLOBALCONFIG_RIL_SAMSUNG_LIBRIL_INTF_EXTENSION)
    mSimIndex = arg_wrg->index;
#endif
    return 0;
}

SimSmsMessageResult::SimSmsMessageResult(int result, int index)
{
    mResult = result;
    mIndex = index;
}


/**
 * SMS Storage
 */
SmsStorage::SmsStorage()
{
    mUsedCount = 0;
    mMaxCount = 10;
    mMemoryType = -1;
    mCurrentIndex = -1;
    mStatus = -1;
}
SmsStorage::SmsStorage(int status)
{
    mUsedCount = 0;
    mMaxCount = 10;
    mMemoryType = -1;
    mCurrentIndex = -1;
    mStatus = status;
}

BroadcastSmsConfigs::BroadcastSmsConfigs(RIL_GSM_BroadcastSmsConfigInfo* rgbsci, UINT8 num)
    :m_uNum(0), m_ptRgbsci(NULL), m_ptRgbsciPtrs(NULL)
{
    if (rgbsci == NULL) {
        RilLogE("Fail to create BroadcastSmsConfigs Class!!!");
    } else {
        if (num > MAX_BCST_INFO_NUM) {
            RilLogW("Number of BCST(%d) is larger than MAX_BCST_INFO_NUM(%d).",\
                    num, MAX_BCST_INFO_NUM);
            m_uNum = MAX_BCST_INFO_NUM;
        } else {
            m_uNum = num;
        }
        m_ptRgbsci = new RIL_GSM_BroadcastSmsConfigInfo[m_uNum];
        memcpy(m_ptRgbsci, rgbsci, m_uNum * sizeof(RIL_GSM_BroadcastSmsConfigInfo));
        m_ptRgbsciPtrs = new RIL_GSM_BroadcastSmsConfigInfo *[m_uNum];
        for (int i = 0; i < m_uNum; i++) {
            m_ptRgbsciPtrs[i] = &m_ptRgbsci[i];
        }
    }
}

BroadcastSmsConfigs::BroadcastSmsConfigs(const BroadcastSmsConfigs& bsc)
    :m_uNum(0), m_ptRgbsci(NULL), m_ptRgbsciPtrs(NULL)
{
    if ((bsc.m_uNum > 0) && bsc.m_ptRgbsci) {
        m_uNum = bsc.m_uNum;
        m_ptRgbsci = new RIL_GSM_BroadcastSmsConfigInfo[m_uNum];
        memcpy(m_ptRgbsci, bsc.m_ptRgbsci, m_uNum * sizeof(RIL_GSM_BroadcastSmsConfigInfo));
        m_ptRgbsciPtrs = new RIL_GSM_BroadcastSmsConfigInfo *[m_uNum];
        for (int i = 0; i < m_uNum; i++) {
            m_ptRgbsciPtrs[i] = &m_ptRgbsci[i];
        }
    } else {
        RilLogE("Fail to create BroadcastSmsConfigs Class!!!");
    }
}

BroadcastSmsConfigs::~BroadcastSmsConfigs()
{
    if (m_ptRgbsci) {
        delete[] m_ptRgbsci;
    }
    if (m_ptRgbsciPtrs) {
        delete[] m_ptRgbsciPtrs;
    }
}
BroadcastSmsConfigsRequestData::BroadcastSmsConfigsRequestData
    (const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type), m_pCBcsc(NULL)
{
}

BroadcastSmsConfigsRequestData::~BroadcastSmsConfigsRequestData()
{
    if (m_pCBcsc) {
        delete m_pCBcsc;
    }
}

INT32 BroadcastSmsConfigsRequestData::encode(char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0) {
        return -1;
    }
    int num = datalen/sizeof(RIL_GSM_BroadcastSmsConfigInfo *);
    RIL_GSM_BroadcastSmsConfigInfo rgbsci[num];
    RIL_GSM_BroadcastSmsConfigInfo **rgbsciPtrs = (RIL_GSM_BroadcastSmsConfigInfo **)data;
    for (int i = 0; i < num; i++) {
        if (rgbsciPtrs[i] == NULL) {
            return -1;
        }
        memcpy(&rgbsci[i], rgbsciPtrs[i], sizeof(RIL_GSM_BroadcastSmsConfigInfo));
    }
    m_pCBcsc = new BroadcastSmsConfigs(rgbsci, num);
    return 0;
}

UINT8 BroadcastSmsConfigsRequestData::GetConfigsNumber()
{
    if (m_pCBcsc) {
        return m_pCBcsc->GetConfigsNumber();
    } else {
        return 0;
    }
}

const RIL_GSM_BroadcastSmsConfigInfo* BroadcastSmsConfigsRequestData::GetConfigsInfo()
{
    if (m_pCBcsc) {
        return m_pCBcsc->GetConfigsInfo();
    } else {
        return NULL;
    }
}
