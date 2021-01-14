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
 * stkdatabuilder.cpp
 *
 *  Created on: 2014. 10. 6.
 *      Author: MOX
 */

#include <stdio.h>
#include "stkdatabuilder.h"
#include "util.h"
#include "rillog.h"
// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)


#define ENTER_FUNC()        { RilLogV("StkDataBuilder::%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { RilLogV("StkDataBuilder::%s() [--> ", __FUNCTION__); }
#define BYTEBIT(byte, idx)        ((byte >> (idx-1)) & 0x01)


const RilData *StkDataBuilder::BuildStkEnvelopeCommandResponse(int nLength, BYTE *pEnvelopeCommand)
{
    ENTER_FUNC();
    if(nLength==0 || pEnvelopeCommand==NULL) return NULL;

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        // Convert hex values to hex character string
        char *pszEnvelopeCmdStr = NULL;
        if(nLength>0 && pEnvelopeCommand!=NULL)
        {
            pszEnvelopeCmdStr = new char[(nLength*2)+1];
            if(Value2HexString(pszEnvelopeCmdStr, pEnvelopeCommand, nLength)==-1)
            {
                RilLogE("StkDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszEnvelopeCmdStr) delete [] pszEnvelopeCmdStr;
                delete rildata;
                LEAVE_FUNC();
                return NULL;
            }
        }

        if(pszEnvelopeCmdStr!=NULL)
        {
            rildata->SetString(pszEnvelopeCmdStr);
            delete [] pszEnvelopeCmdStr;
        }
    }
    LEAVE_FUNC();
    return rildata;
}

const RilData *StkDataBuilder::BuildStkEnvelopeStatusResponse(int nSW1, int nSW2, int nLength, BYTE *pEnvelopeStatus)
{
    ENTER_FUNC();

    RilData *pRilData = new RilData();
    if(pRilData != NULL) {

        // Convert hex values to hex character string
        char *pszEnvelopeStatusString = NULL;
        if(nLength>0 && pEnvelopeStatus!=NULL)
        {
            pszEnvelopeStatusString = new char[(nLength*2)+1];
            if(Value2HexString(pszEnvelopeStatusString, pEnvelopeStatus, nLength)==-1)
            {
                RilLogE("SimDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszEnvelopeStatusString) delete[] pszEnvelopeStatusString;
                delete pRilData;
                LEAVE_FUNC();
                return NULL;
            }
        }

        int nDataLength = sizeof(RIL_SIM_IO_Response);
        if(pszEnvelopeStatusString!=NULL) nDataLength += (strlen(pszEnvelopeStatusString)+1);
        BYTE *pData = new BYTE[nDataLength];
        memset(pData, 0, nDataLength);

        pRilData->rildata = pData;
        pRilData->length = sizeof(RIL_SIM_IO_Response);

        RIL_SIM_IO_Response *pRilRsp = (RIL_SIM_IO_Response *) pRilData->rildata;
        pRilRsp->sw1 = nSW1;
        pRilRsp->sw2 = nSW2;
        RilLogV("StkDataBuilder::%s() SW1:%d=0x%02X, SW2:%d=0x%02X", __FUNCTION__, pRilRsp->sw1, pRilRsp->sw1, pRilRsp->sw2, pRilRsp->sw2);

        if(pszEnvelopeStatusString!=NULL)
        {
            char *pStr = (char *) &pRilRsp[1];
            pRilRsp->simResponse = pStr;
            SECURELIB::strncpy(pStr, nDataLength, pszEnvelopeStatusString, SECURELIB::strlen(pszEnvelopeStatusString));
            pStr[strlen(pszEnvelopeStatusString)] = '\0';
            RilLogV("StkDataBuilder::%s() Response:[0x%08X=(%d)%s]", __FUNCTION__, pRilRsp->simResponse, strlen(pRilRsp->simResponse), pRilRsp->simResponse);
            delete [] pszEnvelopeStatusString;
            pszEnvelopeStatusString = NULL;
        }
        else
        {
            pRilRsp->simResponse = NULL;
            RilLogV("StkDataBuilder::%s() Response:NULL", __FUNCTION__);
        }
    }

    LEAVE_FUNC();
    return pRilData;
}

const RilData *StkDataBuilder::BuildStkProactiveCommandIndicate(int nLength, BYTE *pProactiveCommand)
{
    ENTER_FUNC();

    if(nLength==0 || pProactiveCommand==NULL) return NULL;

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        // Convert hex values to hex character string
        char *pszProactiveCmdStr = NULL;
        if(nLength>0 && pProactiveCommand!=NULL)
        {
            pszProactiveCmdStr = new char[(nLength*2)+1];
            if(Value2HexString(pszProactiveCmdStr, pProactiveCommand, nLength)==-1)
            {
                RilLogE("StkDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszProactiveCmdStr) {
                    delete [] pszProactiveCmdStr;
                    pszProactiveCmdStr = NULL;
                }
                delete rildata;
                LEAVE_FUNC();
                return NULL;
            }
        }

        if(pszProactiveCmdStr!=NULL)
        {
            rildata->SetString(pszProactiveCmdStr);
            delete [] pszProactiveCmdStr;
            pszProactiveCmdStr = NULL;
        }
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *StkDataBuilder::BuildStkSimRefreshIndicate(int nResult, int nEFID, int nAidLen, BYTE *pAID)
{
    ENTER_FUNC();

    RilData *rildata = new RilData();
    if (rildata != NULL) {
        RIL_SimRefreshResponse_v7 *pSimRefresh = new RIL_SimRefreshResponse_v7;
        memset(pSimRefresh, 0, sizeof(RIL_SimRefreshResponse_v7));

        pSimRefresh->result = (RIL_SimRefreshResult) nResult;
        pSimRefresh->ef_id = nEFID;

        if(pAID!=NULL && nAidLen>0)
        {
            char *pszAID = new char[(nAidLen*2)+1];
            if(Value2HexString(pszAID, pAID, nAidLen)==-1)
            {
                RilLogW("StkDataBuilder::%s() Value2HexString(AID)", __FUNCTION__);
                delete [] pszAID;
                pszAID = NULL;
            }

            pSimRefresh->aid = pszAID;
        }

        rildata->rildata = pSimRefresh;
        rildata->length = sizeof(RIL_SimRefreshResponse_v7);
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *StkDataBuilder::BuildStkSsReturnResultIndicate(int nLength, BYTE *pReturnResult)
{
    ENTER_FUNC();

    if(nLength==0 || pReturnResult==NULL) return NULL;

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        // Convert hex values to hex character string
        char *pszpReturnResultStr = NULL;
        if(nLength>0 && pReturnResult!=NULL)
        {
            pszpReturnResultStr = new char[(nLength*2)+1];
            if(Value2HexString(pszpReturnResultStr, pReturnResult, nLength)==-1)
            {
                RilLogE("StkDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszpReturnResultStr!=NULL) {
                    delete [] pszpReturnResultStr;
                    pszpReturnResultStr = NULL;
                }
                delete rildata;
                LEAVE_FUNC();
                return NULL;
            }
        }

        if(pszpReturnResultStr!=NULL)
        {
            rildata->SetString(pszpReturnResultStr);
            delete [] pszpReturnResultStr;
            pszpReturnResultStr = NULL;
        }
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *StkDataBuilder::BuildStkEventNotify(int nLength, BYTE *pSatUsatCommand)
{
    ENTER_FUNC();

    if(nLength==0 || pSatUsatCommand==NULL) return NULL;

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        // Convert hex values to hex character string
        char *pszSatUsatCmdStr = NULL;
        if(nLength>0 && pSatUsatCommand!=NULL)
        {
            pszSatUsatCmdStr = new char[(nLength*2)+1];
            if(Value2HexString(pszSatUsatCmdStr, pSatUsatCommand, nLength)==-1)
            {
                RilLogE("StkDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszSatUsatCmdStr) {
                    delete [] pszSatUsatCmdStr;
                    pszSatUsatCmdStr = NULL;
                }
                delete rildata;
                LEAVE_FUNC();
                return NULL;
            }
        }

        if(pszSatUsatCmdStr!=NULL)
        {
            rildata->SetString(pszSatUsatCmdStr);
            delete [] pszSatUsatCmdStr;
            pszSatUsatCmdStr = NULL;
        }
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *StkDataBuilder::BuildStkCallSetupIndicate(int nTimeout)
{
    ENTER_FUNC();

    RilDataInts *rildata = new RilDataInts(1);
    if (rildata != NULL) {
        rildata->SetInt(0, nTimeout);
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *StkDataBuilder::BuildStkCcAlphaNtf(int lenAlpha, const BYTE *pAlpha)
{
    ENTER_FUNC();

    RilLogW("%s() need to implement", __FUNCTION__);
    return NULL;

    /*
    RilDataRaw *pRilData = new RilDataRaw();

    if(pRilData != NULL) {
        int size = lenAlpha + 1;
        BYTE *pData = (BYTE *)calloc(size, sizeof(BYTE));
        if (pData != NULL) {
            memcpy(pData, pAlpha, lenAlpha);
            *(pData + lenAlpha) = 0;
            pRilData->SetData(pData, size);
            free(pData);
        }
    }

    LEAVE_FUNC();
    return pRilData;
    */
}
