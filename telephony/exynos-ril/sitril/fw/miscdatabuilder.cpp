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
 * miscdatabuilder.cpp
 *
 *  Created on: 2014. 7. 3.
 *      Author: m.afzal
 */

#include "miscdatabuilder.h"
#include "networkutils.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)


class PrlEriNam : public RilData {
    public:
        int mCount;
        char mVersion[3][MAX_BB_PRL_ERI_VER_LEN];

    public:
        PrlEriNam()
       {
            mCount = 0;
            for (int i = 0; i < 3; i++) {
                memset(mVersion[i], 0, sizeof(mVersion[i]));
            }
        }
        virtual ~PrlEriNam() {}
        void Add(char *ver)
    {
        if (mCount < 3 && ver != NULL) {
            strncpy(mVersion[mCount], ver, sizeof(mVersion[mCount]) - 1);
            mCount++;
        }
    }
};

/**
 * Baseband version.
 */
class BasebandVersion : public RilData {
    public:
        int mMask;
        char mSwVersion[MAX_BB_SW_VER_LEN];
        char mHwVersion[MAX_BB_HW_VER_LEN];
        char mCalDate[MAX_BB_RF_CAL_DATE_LEN];
        char mProdCode[MAX_BB_PRODUCT_CODE_LEN];
        char mModelId[MAX_BB_MODEL_ID_LEN];
        PrlEriNam mPrlNam;
        PrlEriNam mEriNam;
        char mCPChipset[MAX_BB_CP_CHIPSET_LEN];

    public:
        BasebandVersion(): RilData() {
        mMask = 0;
        memset(mSwVersion, 0, sizeof(mSwVersion));
        memset(mHwVersion, 0, sizeof(mHwVersion));
        memset(mCalDate, 0, sizeof(mCalDate));
        memset(mProdCode, 0, sizeof(mProdCode));
        memset(mModelId, 0, sizeof(mModelId));
        memset(mCPChipset, 0, sizeof(mCPChipset));
        }
        virtual ~BasebandVersion() {}
     void Set(char *ver, BasebandVerInfoType type){
        // SW version string is longest one.
        char nullVer[MAX_BB_SW_VER_LEN] = {0, };
        char *toCopy = nullVer;

        if (ver == NULL) {
            mMask &= ~type;
            toCopy = nullVer;
        } else {
            mMask |= type;
            toCopy = ver;
        }

        switch (type) {
            case BB_VERINFO_SW:
                strncpy(mSwVersion, toCopy, sizeof(mSwVersion) - 1);
                break;
            case BB_VERINFO_HW:
                strncpy(mHwVersion, toCopy, sizeof(mHwVersion) - 1);
                break;
            case BB_VERINFO_CAL_DATE:
                strncpy(mCalDate, toCopy, sizeof(mCalDate) - 1);
                break;
            case BB_VERINFO_PROD_CODE:
                strncpy(mProdCode, toCopy, sizeof(mProdCode) - 1);
                break;
            case BB_VERINFO_MODEL_ID:
                strncpy(mModelId, toCopy, sizeof(mModelId) - 1);
                break;
            case BB_VERINFO_CP_CHIPSET:
                strncpy(mCPChipset, toCopy, sizeof(mCPChipset) - 1);
                break;
            default:
                break;
        }
    }
    void Set(PrlEriNam *ver, BasebandVerInfoType type){
        if (ver) {
            switch (type) {
                case BB_VERINFO_PRL_NAM:
                    mPrlNam = *ver;
                    break;
                case BB_VERINFO_ERI_NAM:
                    mEriNam = *ver;
                    break;
                default:
                    break;
            }
        }
    }
};

class GwSignalStrength : public RilData {
    public:
        int mSignalStrength;
        int mBitErrorRate;

    public:
        GwSignalStrength(){
            mSignalStrength = 99;
            mBitErrorRate = -1;
        }
        virtual ~GwSignalStrength() {}
};

class CdmaSignalStrength : public RilData {
    public:
        int mDbm;
        int mEcio;

    public:
        CdmaSignalStrength(){
            mDbm = -1;
            mEcio = -1;
        }
        virtual ~CdmaSignalStrength() {}
};

class EvdoSignalStrength : public RilData {
    public:
        int mDbm;
        int mEcio;
        int mSignalNoiseRatio;

    public:
        EvdoSignalStrength(){
            mDbm = -1;
            mEcio = -1;
            mSignalNoiseRatio = -1;
        }
        virtual ~EvdoSignalStrength() {}
};

class LteSignalStrength : public RilData {
    public:
        static const int INVALID_SNR = 0x7FFFFFFF;

    public:
        int mSignalStrength;
        int mRsrp;
        int mRsrq;
        int mRssnr;
        int mCqi;

    public:
        LteSignalStrength(){
            mSignalStrength = -1;
            mRsrp = INVALID_SNR;
            mRsrq = INVALID_SNR;
            mRssnr = INVALID_SNR;
            mCqi = INVALID_SNR;
        }
        virtual ~LteSignalStrength() {}
};

/**
 * Device identity:
 * IMEI, IMEISV, MEID
 */
class DeviceIdentity : public RilData {
    public:
        char mImei[MAX_ME_SN_LEN];
        char mImeiSv[MAX_ME_SN_LEN];
        char mMeid[MAX_ME_SN_LEN];
        char mEsn[MAX_ME_SN_LEN];

    public:
        DeviceIdentity(){
            memset(mImei, 0, sizeof(mImei));
            memset(mImeiSv, 0, sizeof(mImeiSv));
            memset(mMeid, 0, sizeof(mMeid));
            memset(mEsn, 0, sizeof(mEsn));
        }
        virtual ~DeviceIdentity() {}

        void Set(const char *id, DeviceIdentityType type){
            char nullSerial[MAX_ME_SN_LEN] = {0, };
            const char *toCopy = nullSerial;

            if (type == DEV_IDENTITY_NONE) {
                return;
            }

            if (id == NULL) {
                toCopy = nullSerial;
            } else {
                toCopy = id;
            }

            switch (type) {
                case DEV_IDENTITY_IMEI:
                    memset(mImei, 0, sizeof(mImei));
                    strncpy(mImei, toCopy, sizeof(mImei) - 1);
                    break;
                case DEV_IDENTITY_IMEISV:
                    memset(mImeiSv, 0, sizeof(mImeiSv));
                    strncpy(mImeiSv, toCopy, sizeof(mImeiSv) - 1);
                    break;
                case DEV_IDENTITY_MEID:
                    memset(mMeid, 0, sizeof(mMeid));
                    strncpy(mMeid, toCopy, sizeof(mMeid) -1);
                    break;
                case DEV_IDENTITY_ESN:
                    memset(mEsn, 0, sizeof(mEsn));
                    strncpy(mEsn, toCopy, sizeof(mEsn) -1);
                    break;
                default:
                    break;
            }
        }
};

const RilData *MiscDataBuilder::BuildBaseBandVersionResponse( const char *SwVer)
{
    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        rildata->SetString((char *)SwVer);
    }
    return rildata;
    /*
    PrlEriNam prlNam;
    PrlEriNam eriNam;
    BasebandVersion *rildata = new BasebandVersion();

    if (rildata != NULL) {
        if (Mask & BB_VER_PRL_NAM) {
            int prlCount = PrlNam;
            char *prlStart = (char *)PrlVer;

            if (prlCount > 0 && prlCount != 0xFF) {
                for (int i = 0; (i < prlCount && i < 3); i++) {
                    prlNam.Add(prlStart + (i * MAX_BB_PRL_ERI_VER_LEN));
                }
            }
        }

        if (Mask & BB_VER_ERI_NAM) {
            int eriCount = EriNam;
            char *eriStart = (char *)EriVer;

            if (eriCount > 0 && eriCount != 0xFF) {
                for (int i = 0; (i < eriCount && i < 3); i++) {
                    eriNam.Add(eriStart + (i * MAX_BB_PRL_ERI_VER_LEN));
                }
            }
        }

        rildata->Set(&prlNam, BB_VER_PRL_NAM);
        rildata->Set(&eriNam, BB_VER_ERI_NAM);

        if (Mask & BB_VER_SW)
            rildata->Set((char *)SwVer, BB_VER_SW);
        if (Mask & BB_VER_HW)
            rildata->Set((char *)HwVer, BB_VER_HW);
        if (Mask & BB_VER_CAL_DATE)
            rildata->Set((char *)RfCal, BB_VER_CAL_DATE);
        if (Mask & BB_VER_PROD_CODE)
            rildata->Set((char *)ProdCode, BB_VER_PROD_CODE);
        if (Mask & BB_VER_MODEL_ID)
            rildata->Set((char *)ModelId, BB_VER_MODEL_ID);
        if (Mask & BB_VER_CP_CHIPSET)
            rildata->Set((char *)CPChipSet, BB_VER_CP_CHIPSET);
    }
    return rildata;
    */
}


const RilData *MiscDataBuilder::BuildGetTtyModeResponse(int TtyMode)
{
    RilDataInts *rildata = new RilDataInts(1);
    if (rildata != NULL) {
        rildata->SetInt(0, TtyMode);
    }

    return rildata;
}

const RilData *MiscDataBuilder::BuildNitzTimeIndication(int DayLightValid, int Year, int Month, int Day, int Hour,
        int Minute, int Second, int TimeZone, int DayLightAdjust, int DayofWeek)
{
    char mRespTimeInfo[28];    //static const int MAX_TIME_INFO_LEN = 28;
    snprintf(mRespTimeInfo, sizeof(mRespTimeInfo)-1,
         "%02d/%02d/%02d,%02d:%02d:%02d%s%02d,%02d",
         Year, Month, Day, Hour, Minute, Second,
         TimeZone >= 0 ? "+" : "-",
         abs(TimeZone), DayLightAdjust);

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        rildata->SetString(mRespTimeInfo);
    }

    return rildata;
}

const RilData *MiscDataBuilder::BuildIMEIResponse(int IMEILen, const BYTE *IMEI)
{
    RilLogI("%s()", __FUNCTION__);
    if (IMEI == NULL || IMEILen <= 0) {
        return NULL;
    }

    char buf[MAX_IMEI_LEN + 1] = { 0, };
    memcpy(buf, IMEI, IMEILen);
    buf[IMEILen] = 0;

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        rildata->SetString(buf);
    }
    return rildata;
}

const RilData *MiscDataBuilder::BuildIMEISVResponse(int IMEISVLen, const BYTE *IMEISV)
{
    RilLogI("%s()", __FUNCTION__);
    if (IMEISV == NULL || IMEISVLen <= 0) {
        return NULL;
    }

    char buf[MAX_IMEISV_LEN + 1] = { 0, };
    memcpy(buf, IMEISV, IMEISVLen);
    buf[IMEISVLen] = 0;

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        rildata->SetString(buf);
    }
    return rildata;
}

const RilData *MiscDataBuilder::BuildDevIDResponse(int IMEILen, const BYTE *IMEI, int IMEISVLen, const BYTE *IMEISV, int MEIDLen, const BYTE *MEID,
        int ESNLen, const BYTE *ESN)
{
    char buf[32+1] = { 0, };

    RilDataStrings *rildata = new RilDataStrings(4);

    if (rildata != NULL) {
        if(IMEILen > 0 && IMEI != NULL) {
            memcpy(buf, IMEI, IMEILen);
            buf[IMEILen] = 0;
            rildata->SetString(0, buf);
        }
        if(IMEISVLen > 0 && IMEISV != NULL) {
            memcpy(buf, IMEISV, IMEISVLen);
            buf[IMEISVLen] = 0;
            rildata->SetString(1, buf);
        }
        if(ESNLen > 0 && ESN != NULL) {
            memcpy(buf, ESN, ESNLen);
            buf[ESNLen] = 0;
            rildata->SetString(2, buf);
        }
        if(MEIDLen > 0 && MEID != NULL) {
            memcpy(buf, MEID, MEIDLen);
            buf[MEIDLen] = 0;
            rildata->SetString(3, buf);
        }
    }
    return rildata;
}

const RilData *MiscDataBuilder::BuildDisplayEngIndication( const char * pResponse, int Length)
{
    RilDataRaw *pRilData = new RilDataRaw();
    if (pRilData != NULL) {
        pRilData->SetData(pResponse, Length);
    }

    return pRilData;
}

const RilData *MiscDataBuilder::BuildLceDataIndication(int dlLc, int ulLc, int confLevel, int isSuspended)
{
    RilLogW("%s() need to implement", __FUNCTION__);
    return NULL;
}

#ifdef SUPPORT_CDMA
const RilData *MiscDataBuilder::BuildCdmaSubscriptionSource(int nSubscriptionSource)
{
    RilDataInts *pRilData = new RilDataInts(1);
    if(pRilData != NULL) {
        pRilData->SetInt(0, nSubscriptionSource);
    }

    return pRilData;
}

const RilData *MiscDataBuilder::BuildCdmaSubscription(char *mdn, WORD sid, WORD nid, char *min, UINT prl_version)
{
    RilDataStrings *pRilData = new RilDataStrings(5);
    if(pRilData != NULL)
    {
        char szBuffer[32];

        if(mdn!=NULL) pRilData->SetString(0, mdn);
        sprintf(szBuffer, "%d", sid);
        pRilData->SetString(1, szBuffer);
        sprintf(szBuffer, "%d", nid);
        pRilData->SetString(2, szBuffer);
        if(min!=NULL) pRilData->SetString(3, min);
        sprintf(szBuffer, "%d", prl_version);
        pRilData->SetString(4, szBuffer);
    }

    return pRilData;
}

const RilData *MiscDataBuilder::BuildHardwareConfigNV(char *pszUUID, int nState, int nModel, UINT uRat, int nMaxCS, int nMaxPS, int nMaxStandby)
{
    RilDataRaw *pRilData = new RilDataRaw();
    if (pRilData != NULL) {
        RIL_HardwareConfig tRilHwCfg;
        memset(&tRilHwCfg, 0, sizeof(tRilHwCfg));
        tRilHwCfg.uuid[0] = '\0';
        //memset(tRilHwCfg.uuid, 0, sizeof(tRilHwCfg.uuid));
        //memcpy(tRilHwCfg.uuid, DEFAULT_UUID, MAX_UUID_LENGTH - 1);
        tRilHwCfg.type = RIL_HARDWARE_CONFIG_MODEM;
        if(pszUUID!=NULL) strncpy(tRilHwCfg.uuid, pszUUID, MAX_UUID_LENGTH-1);
        tRilHwCfg.state = (RIL_HardwareConfig_State) nState;

        tRilHwCfg.cfg.modem.rilModel = nModel;
        tRilHwCfg.cfg.modem.rat = uRat;
        tRilHwCfg.cfg.modem.maxVoice = nMaxCS;
        tRilHwCfg.cfg.modem.maxData = nMaxPS;
        tRilHwCfg.cfg.modem.maxStandby = nMaxStandby;

        pRilData->SetData(&tRilHwCfg, sizeof(tRilHwCfg));
    }
    return pRilData;
}

const RilData *MiscDataBuilder::BuildHardwareConfigRuim(char *pszUUID, int nState, char *pszModemUUID)
{
    RilDataRaw *pRilData = new RilDataRaw();
    if (pRilData != NULL) {
        RIL_HardwareConfig tRilHwCfg;
        memset(&tRilHwCfg, 0, sizeof(tRilHwCfg));
        tRilHwCfg.uuid[0] = '\0';
        //memset(tRilHwCfg.uuid, 0, sizeof(tRilHwCfg.uuid));
        //memcpy(tRilHwCfg.uuid, DEFAULT_UUID, MAX_UUID_LENGTH - 1);
        tRilHwCfg.cfg.sim.modemUuid[0] = '\0';
        tRilHwCfg.type = RIL_HARDWARE_CONFIG_SIM;
        if(pszUUID!=NULL) strncpy(tRilHwCfg.uuid, pszUUID, MAX_UUID_LENGTH-1);
        tRilHwCfg.state = (RIL_HardwareConfig_State) nState;

        if(pszModemUUID!=NULL) strncpy(tRilHwCfg.cfg.sim.modemUuid, pszModemUUID, MAX_UUID_LENGTH-1);

        pRilData->SetData(&tRilHwCfg, sizeof(tRilHwCfg));
    }
    return pRilData;
}
#endif // SUPPORT_CDMA

const RilData *MiscDataBuilder::BuildRssiScanResult(int total, int current, int startFrequency, int endFrequency, int step, INT16* result, int resultSize)
{
    int totalLen = 4 * 5 + 2 * resultSize;
    char *buf = new char[totalLen];
    if (buf == NULL) {
        return NULL;
    }
    ((int *)buf)[0] = total;
    ((int *)buf)[1] = current;
    ((int *)buf)[2] = startFrequency;
    ((int *)buf)[3] = endFrequency;
    ((int *)buf)[4] = step;
    if (resultSize > 0 && result != NULL) {
        memcpy(&((int *)buf)[5], result, sizeof(INT16) * resultSize);
    }
    RilDataRaw *rildata = new RilDataRaw(buf, totalLen);
    delete[] buf;
    return rildata;
}

const RilData *MiscDataBuilder::BuildATCommand(const char *command)
{
    const int MAX_BUF = 1000;
    char buf[MAX_BUF + 1] = {0, };
    RilDataRaw *rildata = NULL;
    if (!TextUtils::IsEmpty(command)) {
        int len = strlen(command);
        if (len > MAX_BUF)
            len = MAX_BUF;

        strncpy(buf, command, len);
        buf[len] = 0;

        rildata = new RilDataRaw(buf, len);
    }
    return rildata;
}

const RilData *MiscDataBuilder::BuildGetModemStatus(int status)
{
    RilDataInts *pRilData = new RilDataInts(1);
    if(pRilData != NULL) {
        pRilData->SetInt(0, status);
    }

    return pRilData;
}

/**
 * SignalStrengthBuilder
 */
SignalStrengthBuilder::SignalStrengthBuilder(int halVer)
{
    mHalVer = halVer;
}

/**
 * SignalStrengthBuilder
 */
const RilData *SignalStrengthBuilder::Build(RIL_SignalStrength_V1_4& currentSignalStrength)
{
    // mHalVer <= 1.2, use RIL_SignalStrength_V1_2 as a response.
    if (mHalVer <= HAL_VERSION_CODE(1, 2)) {
        RIL_SignalStrength_V1_2 signalStrength;
        NetworkUtils::convertSignalStrengthResult(signalStrength, currentSignalStrength);
        return new RilDataRaw(&signalStrength, sizeof(RIL_SignalStrength_V1_2));
    }

    // mHalVer == 1.4, use RIL_SignalStrength_V1_4 as a response.
    return new RilDataRaw(&currentSignalStrength, sizeof(RIL_SignalStrength_V1_4));
}

const RilData *MiscDataBuilder::BuildModemInfo(int type, void *data, unsigned int datalen)
{
    int length = sizeof(int) * 2 + datalen;
    RilDataRaw *rildata = NULL;
    char *buf = new char[length];
    if (buf != NULL) {
        ((int *)buf)[0] = type;
        ((int *)buf)[1] = datalen;
        if (datalen > 0) {
            memcpy(&((int *)buf)[2], data, datalen);
        }
        rildata = new RilDataRaw(buf, length);
        delete[] buf;
    }
    return rildata;
}
