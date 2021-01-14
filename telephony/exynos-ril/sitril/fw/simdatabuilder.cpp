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
 * simdatabuilder.cpp
 *
 *  Created on: 2014. 7. 3.
 *      Author: MOX
 */

#include "simdatabuilder.h"
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

#define ENTER_FUNC()        { RilLogV("SimDataBuilder::%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { RilLogV("SimDataBuilder::%s() [--> ", __FUNCTION__); }
#define BYTEBIT(byte, idx)        ((byte >> (8-idx)) & 0x01)

class SimIoResponse : public RilData {
public:
    RIL_SIM_IO_Response mResp;
    SimIoResponse() : RilData() {
        memset(&mResp, 0, sizeof(mResp));
    }

    virtual ~SimIoResponse() {
        if (mResp.simResponse != NULL) {
            delete[] mResp.simResponse;
        }
    }

    void SetSw1(int sw1) {
        mResp.sw1 = sw1;
    }

    void SetSw2(int sw2) {
        mResp.sw2 = sw2;
    }

    void SetSimResponse(char *simResponse) {
        if (!TextUtils::IsEmpty(simResponse)) {
            unsigned int len = strlen(simResponse);
            mResp.simResponse = new char[len + 1];
            if (mResp.simResponse != NULL) {
                strncpy(mResp.simResponse, simResponse, len);
                mResp.simResponse[len] = 0;
            }
        }
    }

    virtual void *GetData() const { return (void *)&mResp; }
    virtual unsigned int GetDataLength() const { return sizeof(RIL_SIM_IO_Response); }
};

const RilData *SimDataBuilder::BuildSimPinPukResponse(int nRemainCount)
{
    RilDataInts *pRilData = new RilDataInts(1);
    if(pRilData != NULL) {
        pRilData->SetInt(0, nRemainCount);
    }

    return pRilData;
}

const RilData *SimDataBuilder::BuildSimNetworkLockResponse(int nRemainCount)
{
    RilDataInts *pRilData = new RilDataInts(1);
    if(pRilData != NULL) {
        pRilData->SetInt(0, nRemainCount);
    }

    return pRilData;
}

const RilData *SimDataBuilder::BuildSimIoResponse(BYTE sw1, BYTE sw2, int nLength, BYTE *pResponse)
{
    ENTER_FUNC();

    SimIoResponse *pRilData = new SimIoResponse();
    if(pRilData != NULL) {

        // Convert SIM response for especial case
        BYTE acSimRsp[15];
        BYTE *pSimResponse = pResponse;
        memset(acSimRsp, 0, 15);

        if(pResponse!=NULL && nLength>0)
        {
            //RilLogV("SimDataBuilder::%s() pResponse[0]: 0x%02X, %d+2==%d?", __FUNCTION__, pResponse[0], pResponse[1], nLength);
            int nFcpLength = ((int) pResponse[1]) & 0x000000FF;
            //RilLogV("SimDataBuilder::%s() nFcpLength: %d", __FUNCTION__, nFcpLength);
            if(pResponse[0]==0x62 && (nFcpLength+2)==nLength)        // 2 means 1st(Type) and 2nd(Length) byte
            {
                int nOffset = 2;        // Start Offset

                acSimRsp[0] = 0;        // RFU
                acSimRsp[1] = 0;        // RFU
                acSimRsp[6] = 0x04;        // EF - Type of file :hard fixed
                acSimRsp[7] = 0;        // RFU

                /**
                 * Access condition - Temporary
                 * : 3G response does not include below information.
                 * ALW(0), CHV1(1), CHV(2), RFU(3), ADM(4), ......ADM(0xE), NEV(0xF)
                 */
                acSimRsp[8] = 0x00;        // read(4) and update(4)
                acSimRsp[9] = 0xFF;        // increase(4) and RFU(4)
                acSimRsp[10] = 0xFF;    // rehablilitate and invalidate

                acSimRsp[11] = 0x01;    // File status : 1 is not invalidate status
                acSimRsp[12] = 0x02;    // Length of following data : file struct(1) + rec size(1)

                for(int nLimit=0; (nOffset-2)<nFcpLength && nLimit<200; nLimit++)        // it wil be fix for exception process
                {
                    //RilLogV("SimDataBuilder::%s() TAG: 0x%02X", __FUNCTION__, pResponse[nOffset]);
                    switch(pResponse[nOffset])    // TAG
                    {
                    case 0x80:
                        // File Size
                        acSimRsp[2] = pResponse[nOffset + 2];
                        acSimRsp[3] = pResponse[nOffset + 3];
                        break;
                    case 0x82:
                        acSimRsp[13] = pResponse[nOffset + 2];    // File Struct
                        //RilLogV("SimDataBuilder::%s() BIT[8]:%d, BIT[7]:%d, BIT[6]:%d", __FUNCTION__, BYTEBIT(acSimRsp[13], 8), BYTEBIT(acSimRsp[13], 7), BYTEBIT(acSimRsp[13], 6));

                        if(BYTEBIT(acSimRsp[13], 8)==1 && BYTEBIT(acSimRsp[13], 7)==0 && BYTEBIT(acSimRsp[13], 6)==0) acSimRsp[13] = EF_TYPE_TRANSPARENT;
                        else if(BYTEBIT(acSimRsp[13], 8)==0 && BYTEBIT(acSimRsp[13], 7)==1 && BYTEBIT(acSimRsp[13], 6)==0) acSimRsp[13] = EF_TYPE_LINEAR_FIXED;
                        else if(BYTEBIT(acSimRsp[13], 8)==0 && BYTEBIT(acSimRsp[13], 7)==1 && BYTEBIT(acSimRsp[13], 6)==1) acSimRsp[13] = EF_TYPE_CYCLIC;

                        if(pResponse[nOffset+1]>0x02)            // Length
                        {
                            if(acSimRsp[13]!=EF_TYPE_TRANSPARENT)
                            {
                                // It's too big for 2G specification
                                if(pResponse[nOffset+4]>0) acSimRsp[14] = 0xFF;        // Record Size
                                else acSimRsp[14] = pResponse[nOffset + 5];
                            }
                        }
                        else acSimRsp[14] = 0x02;                // Need to fix
                        break;
                    // File ID
                    case 0x83:
                        acSimRsp[4] = pResponse[nOffset + 2];
                        acSimRsp[5] = pResponse[nOffset + 3];
                        break;
                    case 0x81:
                    case 0x88:
                    case 0x8A:        // Life Cycle Status Integer
                    case 0x8B:
                    case 0x8C:
                    case 0xA5:        // Proprietary Information
                    case 0xAB:
                        break;
                    }

                    nOffset += pResponse[nOffset + 1] + 2;        // Length Field + Length
                }

                pSimResponse = acSimRsp;
                nLength = 15;
            }
        }

        // Convert hex values to hex character string
        char *pszSimIoString = NULL;
        if(nLength>0 && pSimResponse!=NULL)
        {
            pszSimIoString = new char[(nLength*2)+1];
            if(Value2HexString(pszSimIoString, pSimResponse, nLength)==-1)
            {
                RilLogE("SimDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszSimIoString) delete [] pszSimIoString;
                delete pRilData;
                LEAVE_FUNC();
                return NULL;
            }
        }

        pRilData->SetSw1(sw1);
        pRilData->SetSw2(sw2);
        RilLogV("SimDataBuilder::%s() SW1:%d=0x%02X, SW2:%d=0x%02X", __FUNCTION__, sw1, sw1, sw2, sw2);

        if(pszSimIoString!=NULL)
        {
            pRilData->SetSimResponse(pszSimIoString);
            RilLogV("SimDataBuilder::%s() Response:[0x%08X=(%d)%s]", __FUNCTION__, pszSimIoString, strlen(pszSimIoString), pszSimIoString);
            delete [] pszSimIoString;
        }
        else
        {
            RilLogV("SimDataBuilder::%s() Response:NULL", __FUNCTION__);
        }
    }

    LEAVE_FUNC();
    return pRilData;
}

const RilData *SimDataBuilder::BuildSimIoFcpTemplateResponse(BYTE sw1, BYTE sw2, int nLength, BYTE *pResponse)
{
    ENTER_FUNC();

    SimIoResponse *pRilData = new SimIoResponse();
    if(pRilData != NULL) {
        // Convert hex values to hex character string
        char *pszSimIoString = NULL;
        if(nLength>0 && pResponse!=NULL)
        {
            pszSimIoString = new char[(nLength*2)+1];
            if(Value2HexString(pszSimIoString, pResponse, nLength)==-1)
            {
                RilLogE("SimDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszSimIoString) delete [] pszSimIoString;
                delete pRilData;
                LEAVE_FUNC();
                return NULL;
            }
        }

        pRilData->SetSw1(sw1);
        pRilData->SetSw2(sw2);
        RilLogV("SimDataBuilder::%s() SW1:%d=0x%02X, SW2:%d=0x%02X", __FUNCTION__, sw1, sw1, sw2, sw2);

        if(pszSimIoString!=NULL)
        {
            pRilData->SetSimResponse(pszSimIoString);
            RilLogV("SimDataBuilder::%s() Response:[0x%08X=(%d)%s]", __FUNCTION__, pszSimIoString, strlen(pszSimIoString), pszSimIoString);
            delete [] pszSimIoString;
        }
        else
        {
            RilLogV("SimDataBuilder::%s() Response:NULL", __FUNCTION__);
        }
    }

    LEAVE_FUNC();
    return pRilData;
}

const RilData *SimDataBuilder::BuildSimGetFacilityLockResponse(int nServiceClass)
{
    RilDataInts *pRilData = new RilDataInts(1);
    if(pRilData != NULL) {
        pRilData->SetInt(0, nServiceClass);
    }

    return pRilData;
}

const RilData *SimDataBuilder::BuildSimSetFacilityLockResponse(int nRemainCount)
{
    RilDataInts *pRilData = new RilDataInts(1);
    if(pRilData != NULL) {
        pRilData->SetInt(0, nRemainCount);
    }

    return pRilData;
}

const RilData *SimDataBuilder::BuildSimGetIsimAuthResponse(const int nAuthLen, const BYTE *pAuthResp)
{
    if (nAuthLen == 0 || pAuthResp == NULL) {
        return NULL;
    }

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        char *pszAuthString = new char[(nAuthLen*2)+1];
        int nAuthLength = Value2HexString(pszAuthString, pAuthResp, nAuthLen);
        pszAuthString[nAuthLength] = '\0';
        rildata->SetString(pszAuthString);
        delete [] pszAuthString;
    }
    return rildata;
}

const RilData *SimDataBuilder::BuildSimGetSimAuthResponse(int nAuthType, const int nParameterLength, const int nAuthLen, const BYTE *pAuthResp)
{
    ENTER_FUNC();
    if (nAuthLen == 0 || pAuthResp == NULL) {
        RilLogE("%s(): Parameter Error!!!", __FUNCTION__);
        LEAVE_FUNC();
        return NULL;
    }

    SimIoResponse *pRilData = new SimIoResponse();
    if(pRilData != NULL) {
        if(nParameterLength>=(nAuthLen+2+2)) // AuthType(1) + AuthLength(1) + SW1(1) + SW2(1)
        {
            // Encode BASE64
            char *pResponse = NULL;
            if(nAuthType==ISIM_AUTH_GSM)
            {
                // pAuthResp+1, nAuthLen-1 ==> Remove Auth Result for Base64
                if(nAuthLen-1>0) pResponse = Base64_Encode((const BYTE *) pAuthResp+1, nAuthLen-1);
            }
            else
            {
                // Change Auth Result Value
                BYTE *pAuth = new BYTE[nAuthLen];
                if(pAuth==NULL)
                {
                    RilLogE("SimDataBuilder::%s() memory allocation failed for pAuth", __FUNCTION__);
                    delete pRilData;
                    LEAVE_FUNC();
                    return NULL;
                }
                memcpy(pAuth, pAuthResp, nAuthLen);
                // Success
                if(pAuthResp[0]==0) pAuth[0] = 0xDB;
                // Failed
                else if(pAuthResp[0]==5) pAuth[0] = 0xDC;

                // Auth Result must be included to auth response
                pResponse = Base64_Encode((const BYTE *) pAuth, nAuthLen);
                delete [] pAuth;
            }

            int sw1 = pAuthResp[nAuthLen] & 0xFF;
            int sw2 = pAuthResp[nAuthLen+1] & 0xFF;
            pRilData->SetSw1(sw1);
            pRilData->SetSw2(sw2);
            pRilData->SetSimResponse(pResponse);

            RilLogV("SimDataBuilder::%s() Base64_Encode:%s", __FUNCTION__, pResponse);
            RilLogV("SimDataBuilder::%s() SW1(0x%02X), SW2(0x%02X)", __FUNCTION__, sw1, sw2);
            if(pResponse!=NULL) delete[] pResponse;
        }
        else
        {
            RilLogE("SimDataBuilder::%s() Both SW1 and SW2 are not included, nParameterLength(%d), nAuthLen(%d)", __FUNCTION__, nParameterLength, nAuthLen);
            delete pRilData;
            LEAVE_FUNC();
            return NULL;
        }
    }

    LEAVE_FUNC();
    return pRilData;
}

const RilData *SimDataBuilder::BuildSimGetGbaAuthResponse(const int nAuthLen, const BYTE *pAuthResp)
{
    if (nAuthLen == 0 || pAuthResp == NULL) {
        return NULL;
    }
    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        char *pszAuthString = new char[(nAuthLen*2)+1];
        int nAuthLength = Value2HexString(pszAuthString, pAuthResp, nAuthLen);
        pszAuthString[nAuthLength] = '\0';
        rildata->SetString(pszAuthString);
        delete [] pszAuthString;
    }
    return rildata;
}

const RilData *SimDataBuilder::BuildSimOpenChannelResponse(const int sessionid, BYTE sw1, BYTE sw2, int nLength, BYTE *pResponse)
{
    ENTER_FUNC();

    RilLogI("%s() sessionid : %d, sw1 : %X, sw2 : %X, length : %d", __FUNCTION__, sessionid, sw1, sw2, nLength);

    int size = 3 + nLength; // sessionid + sw1 + sw2 + nLength

    RilDataInts *pRilData = new RilDataInts(size);
    if(pRilData != NULL) {

        pRilData->SetInt(0, sessionid);
        if (pResponse != NULL && 0 < nLength) {
            for(int i = 0; i < nLength; i++) {
                pRilData->SetInt(i+1, pResponse[i]);
            }
        }
        pRilData->SetInt(nLength+1, sw1);
        pRilData->SetInt(nLength+2, sw2);
    }

    LEAVE_FUNC();
    return pRilData;
}

const RilData *SimDataBuilder::BuildSimCloseChannelResponse()
{
    return NULL;
}

const RilData *SimDataBuilder::BuildSimTransmitApduBasicResponse(int nLength, BYTE *pResponse)
{
    ENTER_FUNC();

    RilData *pRilData = new RilData();
    BYTE *pSimResponse = pResponse;

    if(pRilData != NULL) {
        if(pResponse!=NULL && 2<nLength)
        {
            // Convert hex values to hex character string
            char *pszSimIoString = NULL;
            pszSimIoString = new char[(nLength*2)+1];
            if(Value2HexString(pszSimIoString, pSimResponse, nLength-2)==-1)
            {
                RilLogE("SimDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszSimIoString) delete [] pszSimIoString;
                delete pRilData;
                LEAVE_FUNC();
                return NULL;
            }

            int nDataLength = sizeof(RIL_SIM_IO_Response);
            if (pszSimIoString!=NULL)
                nDataLength += (strlen(pszSimIoString)+1);

            BYTE *pData = new BYTE[nDataLength];
            memset(pData, 0, nDataLength);

            pRilData->rildata = pData;
            pRilData->length = sizeof(RIL_SIM_IO_Response);

            RIL_SIM_IO_Response *pRilRsp = (RIL_SIM_IO_Response *) pRilData->rildata;
            pRilRsp->sw1 = pSimResponse[nLength-2];
            pRilRsp->sw2 = pSimResponse[nLength-1];
            RilLogV("SimDataBuilder::%s() SW1:%d=0x%02X, SW2:%d=0x%02X", __FUNCTION__, pRilRsp->sw1, pRilRsp->sw1, pRilRsp->sw2, pRilRsp->sw2);
            if (pszSimIoString!=NULL)
            {
                char *pStr = (char *) &pRilRsp[1];
                pRilRsp->simResponse = pStr;
                strncpy(pStr, pszSimIoString, strlen(pszSimIoString));
                pStr[strlen(pszSimIoString)] = '\0';
                RilLogV("SimDataBuilder::%s() Response:[0x%08X=(%d)%s]", __FUNCTION__, pRilRsp->simResponse, strlen(pRilRsp->simResponse), pRilRsp->simResponse);
                delete [] pszSimIoString;
            }
            else
            {
                pRilRsp->simResponse = NULL;
                RilLogV("SimDataBuilder::%s() Response:NULL", __FUNCTION__);
            }
        }
        else
        {
            int nDataLength = sizeof(RIL_SIM_IO_Response);

            BYTE *pData = new BYTE[nDataLength];
            memset(pData, 0, nDataLength);

            pRilData->rildata = pData;
            pRilData->length = sizeof(RIL_SIM_IO_Response);

            RIL_SIM_IO_Response *pRilRsp = (RIL_SIM_IO_Response *) pRilData->rildata;
            pRilRsp->sw1 = pSimResponse[nLength-2];
            pRilRsp->sw2 = pSimResponse[nLength-1];
            pRilRsp->simResponse = NULL;
            RilLogV("SimDataBuilder::%s() SW1:%d=0x%02X, SW2:%d=0x%02X", __FUNCTION__, pRilRsp->sw1, pRilRsp->sw1, pRilRsp->sw2, pRilRsp->sw2);
        }
    }

    LEAVE_FUNC();
    return pRilData;
}


const RilData *SimDataBuilder::BuildSimTransmitApduChannelResponse(BYTE sw1, BYTE sw2, int nLength, BYTE *pResponse)
{
    ENTER_FUNC();

    RilData *pRilData = new RilData();
    BYTE *pSimResponse = pResponse;

    if(pRilData != NULL) {
        if(pResponse!=NULL && 0<nLength)
        {
            // Convert hex values to hex character string
            char *pszSimIoString = NULL;
            pszSimIoString = new char[(nLength*2)+1];
            if(Value2HexString(pszSimIoString, pSimResponse, nLength)==-1)
            {
                RilLogE("SimDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszSimIoString) delete [] pszSimIoString;
                delete pRilData;
                LEAVE_FUNC();
                return NULL;
            }

            int nDataLength = sizeof(RIL_SIM_IO_Response);
            if (pszSimIoString!=NULL)
                nDataLength += (strlen(pszSimIoString)+1);

            BYTE *pData = new BYTE[nDataLength];
            memset(pData, 0, nDataLength);

            pRilData->rildata = pData;
            pRilData->length = sizeof(RIL_SIM_IO_Response);

            RIL_SIM_IO_Response *pRilRsp = (RIL_SIM_IO_Response *) pRilData->rildata;
            pRilRsp->sw1 = sw1;
            pRilRsp->sw2 = sw2;
            RilLogV("SimDataBuilder::%s() SW1:%d=0x%02X, SW2:%d=0x%02X", __FUNCTION__, pRilRsp->sw1, pRilRsp->sw1, pRilRsp->sw2, pRilRsp->sw2);
            if (pszSimIoString!=NULL)
            {
                char *pStr = (char *) &pRilRsp[1];
                pRilRsp->simResponse = pStr;
                strncpy(pStr, pszSimIoString, strlen(pszSimIoString));
                pStr[strlen(pszSimIoString)] = '\0';
                RilLogV("SimDataBuilder::%s() Response:[0x%08X=(%d)%s]", __FUNCTION__, pRilRsp->simResponse, strlen(pRilRsp->simResponse), pRilRsp->simResponse);
                delete [] pszSimIoString;
            }
            else
            {
                pRilRsp->simResponse = NULL;
                RilLogV("SimDataBuilder::%s() Response:NULL", __FUNCTION__);
            }
        }
        else
        {
            int nDataLength = sizeof(RIL_SIM_IO_Response);

            BYTE *pData = new BYTE[nDataLength];
            memset(pData, 0, nDataLength);

            pRilData->rildata = pData;
            pRilData->length = sizeof(RIL_SIM_IO_Response);

            RIL_SIM_IO_Response *pRilRsp = (RIL_SIM_IO_Response *) pRilData->rildata;
            pRilRsp->sw1 = sw1;
            pRilRsp->sw2 = sw2;
            pRilRsp->simResponse = NULL;
            RilLogV("SimDataBuilder::%s() SW1:%d=0x%02X, SW2:%d=0x%02X", __FUNCTION__, pRilRsp->sw1, pRilRsp->sw1, pRilRsp->sw2, pRilRsp->sw2);
        }
    }

    LEAVE_FUNC();
    return pRilData;
}

const RilData *SimDataBuilder::BuildGetImsiResponse(const char *imsi)
{
    if (imsi == NULL || *imsi == 0) {
        return NULL;
    }

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        rildata->SetString(imsi);
    }
    return rildata;
}

const RilData *SimDataBuilder::BuildGetATRResponse(const char *atr, int nLength)
{
    if (atr == NULL || *atr == 0) {
        return NULL;
    }

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        char *pszATRString = NULL;
        pszATRString = new char[(nLength*2)+1];

        if(Value2HexString(pszATRString, (const BYTE*)atr, nLength) == -1)
        {
            RilLogE("SimDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
            if(pszATRString) delete [] pszATRString;
            delete rildata;
            LEAVE_FUNC();
            return NULL;
        }

        RilLogV("%s() ATR : %s", __FUNCTION__, pszATRString);
        rildata->SetString(pszATRString);
        if(pszATRString) delete [] pszATRString;
    }
    return rildata;
}

const RilData *SimDataBuilder::BuildGet3GPbCapaResponse(int entryNum, int *pb)
{
    if (entryNum == 0)
        return NULL;

    int size = entryNum*4;
    RilDataInts *rildata = new RilDataInts(size);

    for(int i=0; i<size; i++)
        rildata->SetInt(i, pb[i]);

    return rildata;
}

const RilData *SimDataBuilder::BuildIccidInfoIndicate(int nLength, const BYTE *pIccId)
{
    ENTER_FUNC();

    if(nLength==0 || pIccId==NULL) return NULL;

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {
        // Convert hex values to hex character string
        char *pszIccIdStr = NULL;
        if(nLength>0 && pIccId!=NULL)
        {
            pszIccIdStr = new char[(nLength*2)+1];
            if(Value2HexString(pszIccIdStr, pIccId, nLength)==-1)
            {
                RilLogE("SimDataBuilder::%s() Value2HexString() failed, Response Length:%d", __FUNCTION__, nLength);
                if(pszIccIdStr) {
                    delete [] pszIccIdStr;
                    pszIccIdStr = NULL;
                }
                delete rildata;
                LEAVE_FUNC();
                return NULL;
            }
        }

        if(pszIccIdStr!=NULL)
        {
            rildata->SetString(pszIccIdStr);
            delete [] pszIccIdStr;
            pszIccIdStr = NULL;
        }
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *SimDataBuilder::BuildGetCarrierRestrictionsResponse(int lenAllowed, int lenExcluded, void *pAllowed, void *pExcluded)
{
    ENTER_FUNC();

    RilLogW("%s() need to implement", __FUNCTION__);
    return NULL;

    /*
    RilDataRaw *pRilData = new RilDataRaw();

    if(pRilData != NULL) {
        RIL_CarrierRestrictions carrierRestrictions;
        carrierRestrictions.len_allowed_carriers = lenAllowed;
        carrierRestrictions.len_excluded_carriers= lenExcluded;
        carrierRestrictions.allowed_carriers = (RIL_Carrier *)pAllowed;
        carrierRestrictions.excluded_carriers = (RIL_Carrier *)pExcluded;

        pRilData->SetData(&carrierRestrictions, sizeof(RIL_CarrierRestrictions));
    }

    LEAVE_FUNC();
    return pRilData;
    */
}

class RilDataSimLockStatus : public RilData {
private:
    RIL_SimLockStatus mSimLockStatus;

public:
    RilDataSimLockStatus(int policy, int status, int lockType,
            int maxRetryCount, int remainCount, int lockCodeCount, const char *lockCode) {
        memset(&mSimLockStatus, 0, sizeof(mSimLockStatus));
        mSimLockStatus.policy = policy;
        mSimLockStatus.status = status;
        mSimLockStatus.lockType = lockType;
        mSimLockStatus.maxRetryCount = maxRetryCount;
        mSimLockStatus.remainCount = remainCount;

        SetData(lockType, lockCodeCount, lockCode);
    }

    virtual ~RilDataSimLockStatus() {
        if (mSimLockStatus.lockCode != NULL) {
            for (int i = 0; i < mSimLockStatus.numOfLockCode; i++) {
                char *entry = mSimLockStatus.lockCode[i];
                if (entry != NULL) {
                    delete[] entry;
                }
            } // end for i ~
            delete[] mSimLockStatus.lockCode;
        }
    }

    void SetData(int lockType, int lockCodeCount, const char *lockCode) {
        int lockCodeSize = 0;
        if (lockCodeCount > 0 && lockCode != NULL) {
            int entrySize = (lockType == LOCK_TYPE_PN) ? PN_LOCK_CODE_SIZE : DEFAULT_LOCK_CODE_SIZE;
            lockCodeSize = (entrySize * lockCodeCount);
            if (lockCodeSize <= MAX_LOCK_CODE_SIZE) {
                mSimLockStatus.numOfLockCode = lockCodeCount;
                mSimLockStatus.lockCode = new char *[lockCodeCount];
                memset(mSimLockStatus.lockCode, 0, sizeof(char *) * lockCodeCount);

                for (int i = 0; i < lockCodeCount; i++) {
                    char *entry = new char[entrySize + 1];
                    if (entry != NULL) {
                        memcpy(entry, lockCode + (i * entrySize), entrySize);
                        *(entry + entrySize) = 0;
                        if (entrySize >= PN_LOCK_CODE_SIZE && *(entry + 5) == '#') {
                            *(entry + 5) = 0;
                        }
                        mSimLockStatus.lockCode[i] = entry;
                    }
                } // end for i ~
            }
        }
    }

    void *GetData() const { return (void *)&mSimLockStatus; }
    unsigned int GetDataLength() const { return sizeof(mSimLockStatus); }
};

const RilData *SimDataBuilder::BuildGetSimLockInfoResponse(int policy, int status, int lockType,
        int maxRetryCount, int remainCount, int lockCodeCount, const char *lockCode)
{
    RilDataSimLockStatus *rildata = new RilDataSimLockStatus(policy, status, lockType,
            maxRetryCount, remainCount, lockCodeCount, lockCode);
    return rildata;
}
