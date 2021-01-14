/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "rcmmgr.h"
#include "sitdef.h"
#include "rillog.h"
#include "protocoladapter.h"

typedef enum {
    LOGLV_SIM_IO_NONE,
    LOGLV_SIM_IO_COMMAND,
    LOGLV_SIM_IO_CMD_DUMP
} LOG_LEVEL_SIM_IO;

int log_cat_by_rcmid[RCM_CATEGORY] =
{
    RIL_LOG_CAT_CALL,    //SIT_BASE_ID_CALL = 0x0000,
    RIL_LOG_CAT_SMS,     //SIT_BASE_ID_SMS = 0x0100,
    RIL_LOG_CAT_SIM,    //SIT_BASE_ID_SIM = 0x0200,
    RIL_LOG_CAT_STK,    //SIT_BASE_ID_SAT = 0x0300,
    RIL_LOG_CAT_MISC,    //SIT_BASE_ID_DEVICEID =  0x0400,
    RIL_LOG_CAT_CALL,    //SIT_BASE_ID_SS = 0x0500,
    RIL_LOG_CAT_PDP,    //SIT_BASE_ID_PDP = 0x0600,
    RIL_LOG_CAT_NET,    //SIT_BASE_ID_NET = 0x0700,
    RIL_LOG_CAT_MISC,    //SIT_BASE_ID_PWR = 0x0800,
    RIL_LOG_CAT_MISC,    //SIT_BASE_ID_MISC = 0x0900,
    RIL_LOG_CAT_SOUND,    //SIT_BASE_ID_SND = 0x0A00,
    RIL_LOG_CAT_IMS,    //SIT_BASE_ID_IMS = 0x0B00,
    RIL_LOG_CAT_GPS,    //SIT_BASE_ID_GPS = 0x0C00,
    RIL_LOG_CAT_AIMS,    //SIT_BASE_ID_SND = 0x0D00,
    RIL_LOG_CAT_VSIM,   //SIT_BASE_ID_VSIM = 0x0E00,
    RIL_LOG_CAT_IMS,    //SIT_BASE_ID_WFC = 0x0F00,
};

INT32 RadioControlMessageMgr::DumpData(char *buffer,UINT32 bufLen, BOOL rx)
{
    UINT16 rcm_len, rcm_id, rcm_category, rcm_num;
    UINT rcm_rsp_token, rcm_req_token;
    BYTE rcm_type, rcm_rsp_err;
    char headerInfo[128];
    char dump[CRilLog::m_uiMaxLogBufferSize] = {0};
    int i, j=0;
    RCM_HEADER  *rcm_hdr = (RCM_HEADER *) buffer;

    if ( rcm_hdr == NULL )
    {
        return -1;
    }

    rcm_len = rcm_hdr->length;
    rcm_type = rcm_hdr->type;
    rcm_id = rcm_hdr->id;
    rcm_rsp_token = rcm_hdr->ext.rsp.token;
    rcm_req_token = rcm_hdr->ext.req.token;
    rcm_rsp_err = rcm_hdr->ext.rsp.error;

    rcm_category = rcm_id >> 8;

    if (rcm_category >= (SIT_BASE_ID_OEM >> 8)) {
        rcm_category = (SIT_BASE_ID_MISC >> 8);
    }

    rcm_num = rcm_id & 0xFF;
    rcm_id &= 0xFFFF;

    if ( rcm_category >= RCM_CATEGORY )
    {
        RilLogE("undefined RCM category(%d)", rcm_category);
        return -1;
    }

    if ( rcm_num >= RCM_ID )
    {
        RilLogE("undefined RCM id(%d)", rcm_num);
        return -1;
    }

    const char *ioname = m_pIo->GetIoChannelName();

    // Disable to print SIM IO
    //if(m_bPrintSimIo==FALSE && rcm_category==((SIT_BASE_ID_SIM>>8)&0x00FF) && rcm_id==(SIT_SIM_IO&0x00FF)) return 0;
    if(m_nPrintSimIo==LOGLV_SIM_IO_NONE && rcm_category==((SIT_BASE_ID_SIM>>8)&0x00FF) && rcm_num==(SIT_SIM_IO&0x00FF)) return 0;

    switch(rcm_type) {
        case RCM_TYPE_RESPONSE:
            if (rcm_rsp_err >= RCM_E_MAX) { //last RCM Error
                RilLogE("RCM error index(%d) is not described", (int)rcm_rsp_err);
            }
            snprintf(headerInfo, sizeof(headerInfo) - 1,
                        "[%s][%s] %s, RSP, %d bytes, Token(0x%04x), %s(%s)",
                        ioname, rx==TRUE?"RX":"TX",
                        rcmMsgToString(rcm_id), rcm_len, rcm_rsp_token, (rcm_rsp_err)? "Error" : "OK", rcmErrorToString(rcm_rsp_err));
            break;

        case RCM_TYPE_INDICATION:
            if (rcm_category > RCM_CATEGORY-1) {
                RilLogE("RCM category is beyond description");
                return -1;
            }
            snprintf(headerInfo, sizeof(headerInfo) - 1,
                        "[%s][%s] %s, IND, %d bytes",
                        ioname, rx==TRUE?"RX":"TX",
                        rcmMsgToString(rcm_id), rcm_len);
            break;

        case RCM_TYPE_REQUEST:
            if (rcm_category > RCM_CATEGORY-1) {
                RilLogE("RCM category is beyond description");
                return -1;
            }
            snprintf(headerInfo, sizeof(headerInfo) - 1,
                        "[%s][%s] %s, REQ, %d bytes, Token(0x%04x)",
                        ioname, rx==TRUE?"RX":"TX",
                        rcmMsgToString(rcm_id), rcm_len, rcm_req_token);
            break;

        default :
            RilLogE("RCM Type is not proper.");
            return -1;
            break;
    }

    snprintf(dump, sizeof(dump)-1, "%s\n", headerInfo);
    j = strlen(dump);

    // Disable to print SIM IO Dump Data
    if(m_nPrintSimIo!=LOGLV_SIM_IO_CMD_DUMP && rcm_category==((SIT_BASE_ID_SIM>>8)&0x00FF) && rcm_num==(SIT_SIM_IO&0x00FF))
    {
        CRilLog::Log(log_cat_by_rcmid[rcm_category], CRilLog::E_RIL_VERBOSE_LOG, "%s", headerInfo);
        return 0;
    }

    if (rcm_len > CRilLog::m_uiMaxLogBufferSize) rcm_len = 32;

    char** ppLog = NULL;
    for (i=0; i<rcm_len; i++) {
        snprintf(dump+j, sizeof(dump)-j-1, " %02x", buffer[i]);
        j = j + 3;
        if ((i+1)%8 == 0) {
            snprintf(dump+j, sizeof(dump)-j-1, "  ");
            j=j+2;
        }
        if ((i+1)%16 == 0) {
            snprintf(dump+j, sizeof(dump)-j-1, "\n");
            j++;
        }

        if ( (i+1)%160 == 0 )    //flush every 10 lines
        {
            ppLog = CRilLog::BufferedLog(ppLog,  "%s", dump);
            memset(dump, 0x00, sizeof(dump));
            j = 0;
        }
    }

    ppLog = CRilLog::BufferedLog(ppLog,  "%s", dump);
    CRilLog::BufferedLogFlash(log_cat_by_rcmid[rcm_category], CRilLog::E_RIL_VERBOSE_LOG,  ppLog);
    return 0;
}

