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
 * protocolstkadapter.cpp
 *
 *  Created on: 2014. 10. 6.
 *      Author: mox
 */


#include "protocolstkadapter.h"
#include "rillog.h"
#include "util.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/**
 * ProtocolStkEnvelopeCommandAdapter
 */
void ProtocolStkEnvelopeCommandAdapter::Init()
{
    m_nEnvelopeCmdLength = 0;
    m_pEnvelopeCmd = NULL;

    if (m_pModemData != NULL) {
        sit_stk_send_stk_envelope_cmd_rsp *pData = (sit_stk_send_stk_envelope_cmd_rsp *) m_pModemData->GetRawData();
        if (pData != NULL)
        {
            m_nEnvelopeCmdLength = pData->envelope_rsp_len;
            m_pEnvelopeCmd = pData->envelope_rsp;
        }
    }
}

/**
 * ProtocolStkTerminalRspAdapter
 */
void ProtocolStkTerminalRspAdapter::Init()
{
    nSW1 = -1;
    nSW2 = -1;

    if (m_pModemData != NULL) {
        sit_stk_send_stk_terminal_rsp_rsp *pData = (sit_stk_send_stk_terminal_rsp_rsp *) m_pModemData->GetRawData();
        if (pData != NULL)
        {
            nSW1 = (int) pData->sw1;
            nSW2 = (int) pData->sw2;
            RilLogV("ProtocolStkTerminalRspAdapter::%s() SW1:0x%02X, SW2:0x%02X", __FUNCTION__, nSW1, nSW2);
        }
    }
}

/**
 * ProtocolStkEnvelopeStatusAdapter
 */
void ProtocolStkEnvelopeStatusAdapter::Init()
{
    nSW1 = -1;
    nSW2 = -1;
    m_nEnvelopeRspLength = 0;
    m_pEnvelopeRsp = NULL;

    if (m_pModemData != NULL) {
        sit_stk_send_stk_envelope_with_status_rsp *pData = (sit_stk_send_stk_envelope_with_status_rsp *) m_pModemData->GetRawData();
        if (pData != NULL)
        {
            nSW1 = (int) pData->sw1;
            nSW2 = (int) pData->sw2;
            m_nEnvelopeRspLength = (int) pData->envelope_rsp_len;
            m_pEnvelopeRsp = pData->envelope_rsp;
        }
    }
}

/**
 * ProtocolStkProactiveCommandAdapter
 */
#define    SWAP16(val)        ( (((val) << 8) & 0xFF00) | (((val) >> 8) & 0x00FF) )
void ProtocolStkProactiveCommandAdapter::Init()
{
    RilLogI("StkProactiveCmdAdapter::%s() [<-- ", __FUNCTION__);

    m_nProactiveCmdLength = 0;
    m_pProactiveCmd = NULL;

    m_nCount = 0;
    memset(m_arEFID, 0, sizeof(UINT)*MAX_EFID_COUNT);
    m_nAidLen = 0;
    memset(m_acAID, 0, MAX_SIM_AID_LEN);

    if (m_pModemData != NULL) {
        sit_stk_stk_proactive_cmd_ind *pData = (sit_stk_stk_proactive_cmd_ind *) m_pModemData->GetRawData();

        if (pData != NULL)
        {
            m_nProactiveCmdLength = pData->proactive_cmd_len;
            m_pProactiveCmd = pData->proactive_cmd;

            BER_TLV *pTlv = (BER_TLV *) m_pProactiveCmd;
            if(pTlv && pTlv->cTag==PROACTIVE_CMD_BER_TAG && pTlv->cLength<m_nProactiveCmdLength)
            {
                RilLogV("StkProactiveCmdAdapter::%s() PROACTIVE_CMD_BER_TAG", __FUNCTION__);

                CMPH_CMD_DETAIL *ptCmdDetail = NULL;
                CMPH_DEVICE_ID *ptDevID = NULL;
                CMPH_RESULT *ptResult = NULL;
                CMPH_DURATION *ptDuration = NULL;
                CMPH_ADDRESS *ptAddress = NULL;
                CMPH_ALPHA_ID *ptAlpha = NULL;
                CMPH_TEXT_STRING *ptTextString = NULL;
                CMPH_FILE_LIST *ptFileList = NULL;
                CMPH_AID *ptAID = NULL;

                int nOffset = 2;    // TAG and Length
                TAG_COMMON *ptTagCommon = (TAG_COMMON *) &m_pProactiveCmd[nOffset];
                while(ptTagCommon && ptTagCommon->cLength>0 && (nOffset+ptTagCommon->cLength)<m_nProactiveCmdLength)
                {
                    switch((ptTagCommon->cTag & CAT_CMPH_TAG_MASK))
                    {
                    case CAT_CMPH_TAG_CMD_DETAIL:
                        RilLogV("StkProactiveCmdAdapter::%s() Tag: Command Detail(0x%02X), Length:%d", __FUNCTION__, ptTagCommon->cTag, ptTagCommon->cLength);
                        ptCmdDetail = (CMPH_CMD_DETAIL *) &m_pProactiveCmd[nOffset];
                        break;
                    case CAT_CMPH_TAG_DEVICE_ID:
                        RilLogV("StkProactiveCmdAdapter::%s() Tag: Device Identifier(0x%02X), Length:%d", __FUNCTION__, ptTagCommon->cTag, ptTagCommon->cLength);
                        ptDevID = (CMPH_DEVICE_ID *) &m_pProactiveCmd[nOffset];
                        break;
                    case CAT_CMPH_TAG_RESULT:
                        RilLogV("StkProactiveCmdAdapter::%s() Tag: Result(0x%02X), Length:%d", __FUNCTION__, ptTagCommon->cTag, ptTagCommon->cLength);
                        ptResult = (CMPH_RESULT *) &m_pProactiveCmd[nOffset];
                        break;
                    case CAT_CMPH_TAG_DURATION:
                        RilLogV("StkProactiveCmdAdapter::%s() Tag: Duration(0x%02X), Length:%d", __FUNCTION__, ptTagCommon->cTag, ptTagCommon->cLength);
                        ptDuration = (CMPH_DURATION *) &m_pProactiveCmd[nOffset];
                        break;
                    case CAT_CMPH_TAG_ALPHA:
                        RilLogV("StkProactiveCmdAdapter::%s() Tag: Alpha(0x%02X), Length:%d", __FUNCTION__, ptTagCommon->cTag, ptTagCommon->cLength);
                        ptAlpha = (CMPH_ALPHA_ID *) &m_pProactiveCmd[nOffset];
                        break;
                    case CAT_CMPH_TAG_ADDRESS:
                        RilLogV("StkProactiveCmdAdapter::%s() Tag: Address(0x%02X), Length:%d", __FUNCTION__, ptTagCommon->cTag, ptTagCommon->cLength);
                        ptAddress = (CMPH_ADDRESS *) &m_pProactiveCmd[nOffset];
                        break;
                    case CAT_CMPH_TAG_TEXT_STRING:
                        RilLogV("StkProactiveCmdAdapter::%s() Tag: Text String(0x%02X), Length:%d", __FUNCTION__, ptTagCommon->cTag, ptTagCommon->cLength);
                        ptTextString = (CMPH_TEXT_STRING *) &m_pProactiveCmd[nOffset];
                        break;
                    case CAT_CMPH_TAG_FILE_LIST:
                        RilLogV("StkProactiveCmdAdapter::%s() Tag: File List(0x%02X), Length:%d", __FUNCTION__, ptTagCommon->cTag, ptTagCommon->cLength);
                        ptFileList = (CMPH_FILE_LIST *) &m_pProactiveCmd[nOffset];
                        break;
                    case CAT_CMPH_TAG_AID:
                        RilLogV("StkProactiveCmdAdapter::%s() Tag: AID(0x%02X), Length:%d", __FUNCTION__, ptTagCommon->cTag, ptTagCommon->cLength);
                        ptAID = (CMPH_AID *) &m_pProactiveCmd[nOffset];
                        break;
                    }

                    nOffset += (2+ptTagCommon->cLength);
                    ptTagCommon = (TAG_COMMON *) &m_pProactiveCmd[nOffset];
                }

                if(ptCmdDetail)
                {
                    switch(ptCmdDetail->cType)
                    {
                    case CAT_CMD_TYPE_REFRESH:
                        RilLogV("StkProactiveCmdAdapter::%s() CAT_CMD_TYPE_REFRESH, Length:%d", __FUNCTION__, ptCmdDetail->common.cLength);

                        if(ptFileList!=NULL && ptFileList->cNumOfFiles>0)
                        {
                            RilLogV("StkProactiveCmdAdapter::%s() File List, Length:%d, NumOfEFID:%d", __FUNCTION__, ptFileList->common.cLength, ptFileList->cNumOfFiles);
                            int nCandidateCount = (ptFileList->common.cLength - 1) / 2;   // 1 is number of EFID
                            //RilLogV("StkProactiveCmdAdapter::%s() File List, Candidate Count:%d", __FUNCTION__, nCandidateCount);
                            for(int i=0; i<nCandidateCount && m_nCount<(int)ptFileList->cNumOfFiles; i++)
                            {
                                WORD wEFID = SWAP16(ptFileList->awFiles[i]);
                                //RilLogV("StkProactiveCmdAdapter::%s() %d.Candidate EFID: 0x%04X", __FUNCTION__, i, wEFID);
                                // Except first
                                if(i>0 && (wEFID & CAT_EFID_ROOT_MASK)==CAT_EFID_ROOT_PREFIX)
                                {
                                    wEFID = SWAP16(ptFileList->awFiles[i-1]);
                                    m_arEFID[m_nCount] = wEFID;
                                    m_nCount++;

                                    RilLogV("StkProactiveCmdAdapter::%s() EFID: 0x%04X", __FUNCTION__, wEFID);
                                }
                                // Last one
                                else if(nCandidateCount==(i+1))
                                {
                                    m_arEFID[m_nCount] = wEFID;
                                    m_nCount++;

                                    RilLogV("StkProactiveCmdAdapter::%s() EFID: 0x%04X", __FUNCTION__, wEFID);
                                }
                            }
                        }

                        if(ptAID!=NULL && ptAID->common.cLength>0)
                        {
                            RilLogV("StkProactiveCmdAdapter::%s() AID, Length:%d", __FUNCTION__, ptAID->common.cLength);
                            m_nAidLen = ptAID->common.cLength;
                            memcpy(m_acAID, ptAID->acAID, m_nAidLen);
                        }
                        break;
                    case CAT_CMD_TYPE_DISPLAY_TEXT:
                        RilLogV("StkProactiveCmdAdapter::%s() CAT_CMD_TYPE_DISPLAY_TEXT, Length:%d", __FUNCTION__, ptCmdDetail->common.cLength);
                        if(ptTextString)
                        switch(ptTextString->cDataCodingScheme)
                        {
                        case CAT_TEXT_CODING_GSM7BIT:
                            RilLogV("StkProactiveCmdAdapter::%s() CAT_TEXT_CODING_GSM7BIT", __FUNCTION__);
                            break;
                        case CAT_TEXT_CODING_GSM8BIT:
                            RilLogV("StkProactiveCmdAdapter::%s() CAT_TEXT_CODING_GSM8BIT", __FUNCTION__);
                            break;
                        case CAT_TEXT_CODING_UCS2:
                            RilLogV("StkProactiveCmdAdapter::%s() CAT_TEXT_CODING_UCS2", __FUNCTION__);
                            break;
                        }
                        break;
                    case CAT_CMD_TYPE_SETUP_CALL:
                        RilLogV("StkProactiveCmdAdapter::%s() CAT_CMD_TYPE_SETUP_CALL, Length:%d", __FUNCTION__, ptCmdDetail->common.cLength);
                        if(ptDevID && ptDevID->cSrcId==CAT_DEVICE_ID_UICC && ptDevID->cDstId==CAT_DEVICE_ID_NETWORK)
                        {
                            RilLogV("StkProactiveCmdAdapter::%s() UICC -> Network", __FUNCTION__);
                            if(ptAddress && ptAddress->cTonNpiFlag==1)
                            {
                                RilLogV("StkProactiveCmdAdapter::%s() TON:0x%X", __FUNCTION__, ptAddress->cTon);
                                RilLogV("StkProactiveCmdAdapter::%s() NPI:0x%X", __FUNCTION__, ptAddress->cNpi);

                                // Parsing Dailing Number to String
                                BOOL bParsing = TRUE;
                                int nDialNumLen = 0;
                                char szDialNumString[64];
                                memset(szDialNumString, 0, 64);
                                if(ptAddress->common.cLength>64) bParsing = FALSE;

                                int i = 0;
                                for(i=0; bParsing && i<((ptAddress->common.cLength-1)*2); i++)
                                {
                                    int idx = i==0? 0: i/2;
                                    unsigned char ch = ptAddress->acDialNumString[idx];
                                    ch = (i==0? ch: (i%2==1? (ch >> 4): ch)) & 0x0F;

                                    if(/*ch>=0 &&*/ ch<=9)
                                    {
                                        szDialNumString[i] = ch + 0x30;
                                    }
                                    else
                                    {
                                        switch(ch)
                                        {
                                        case 0x0A: szDialNumString[i] = '*'; break;
                                        case 0x0B: szDialNumString[i] = '#'; break;
                                        case 0x0C: szDialNumString[i] = 'p'; break;
                                        case 0x0D:
                                        case 0x0E: break;
                                        case 0x0F: bParsing = FALSE; break;
                                        }
                                    }
                                }

                                nDialNumLen = i;
                                szDialNumString[nDialNumLen] = '\0';

                                RilLogV("StkProactiveCmdAdapter::%s() Dialing Number String:[%s]", __FUNCTION__, szDialNumString);
                            }
                            else if ( ptAddress != NULL ){
                                RilLogV("StkProactiveCmdAdapter::%s() 0x%X, 0x%X, 0x%X", __FUNCTION__, ptAddress->cTonNpiFlag, ptAddress->cTon, ptAddress->cNpi);
                            }
                            else {
                                RilLogV("StkProactiveCmdAdapter : Address NULL");
                            }

                            //m_bStkCallSetup = TRUE;
                        }
                        else if(ptDevID) RilLogV("StkProactiveCmdAdapter::%s() 0x%02X -> 0x%02X, 0x%02X", __FUNCTION__, ptDevID->cSrcId, ptDevID->cDstId, ptCmdDetail->cQualifier);
                        break;
                    }
                }
            }
        }
    }

    RilLogI("StkProactiveCmdAdapter::%s() [--> ", __FUNCTION__);
}

/**
 * ProtocolStkSimRefreshAdapter
 */
void ProtocolStkSimRefreshAdapter::Init()
{
    m_nResult = 0;

    if (m_pModemData != NULL) {
        sit_stk_sim_refresh_ind *pData = (sit_stk_sim_refresh_ind *) m_pModemData->GetRawData();
        if (pData != NULL) {
            m_nResult = pData->result;
        }
    }
}

/**
 * ProtocolSsReturnResultAdapter
 */
void ProtocolSsReturnResultAdapter::Init()
{
    m_nReturnResultLength = 0;
    m_pReturnResult = NULL;

    if (m_pModemData != NULL) {
        sit_ss_return_result_ind *pData = (sit_ss_return_result_ind *) m_pModemData->GetRawData();
        if (pData != NULL)
        {
            m_nReturnResultLength = pData->return_result_len;
            m_pReturnResult = pData->return_result;
        }
    }
}

/**
 * ProtocolStkCcAlphaNtfAdapter
 */
int ProtocolStkCcAlphaNtfAdapter::GetAlphaLength() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_stk_cc_alpha_notify_ind *data = (sit_stk_cc_alpha_notify_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_STK_CC_ALPHA_NOTIFY)
            ret = data->alpha_len;
    }

    return ret;
}

BYTE *ProtocolStkCcAlphaNtfAdapter::GetAlpha() const
{
    BYTE *pRet = NULL;
    if (m_pModemData != NULL) {
        sit_stk_cc_alpha_notify_ind *data = (sit_stk_cc_alpha_notify_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_STK_CC_ALPHA_NOTIFY)
            pRet = data->alpha_buf;
    }

    return pRet;
}
