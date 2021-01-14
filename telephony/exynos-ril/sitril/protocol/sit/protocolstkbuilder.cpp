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
 * protocolstkbuilder.cpp
 *
 *  Created on: 2014. 10. 6.
 *      Author: MOX
 */
#include "protocolstkbuilder.h"
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

ModemData *ProtocolStkBuilder::BuildStkEnvelopeCommand(int nLength, BYTE *pEnvelopeCmd)
{
    sit_stk_send_stk_envelope_cmd_req req;
    int length = sizeof(req);
    memset(&req, 0, length);

    if(nLength==0 || pEnvelopeCmd==NULL) return NULL;

    InitRequestHeader(&req.hdr, SIT_SEND_STK_ENVELOPE_CMD, length);
    //req.envelope_cmd_len = (WORD) HexString2Value(req.envelope_cmd, (char *) pEnvelopeCmd);
    req.envelope_cmd_len = (WORD) nLength;
    memcpy(req.envelope_cmd, pEnvelopeCmd, nLength);
    RilLogV("ProtocolSimBuilder::%s() Data Length: %d", __FUNCTION__, req.envelope_cmd_len);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolStkBuilder::BuildStkTerminalResponse(int nLength, BYTE *pTerminalRsp)
{
    RilLogV("ProtocolSimBuilder::%s()", __FUNCTION__);

    sit_stk_send_stk_terminal_rsp_req req;
    int length = sizeof(req);
    memset(&req, 0, length);

    if(nLength==0 || pTerminalRsp==NULL) return NULL;

    // Make Limitation for Over Length
    nLength = (nLength>MAX_SIM_IO_DATA_LEN)? MAX_SIM_IO_DATA_LEN: nLength;
    InitRequestHeader(&req.hdr, SIT_SEND_STK_TERMINAL_RSP, length);
    //req.terminal_rsp_len = (WORD) HexString2Value(req.terminal_rsp, (char *) pTerminalRsp);
    req.terminal_rsp_len = (WORD) nLength;
    memcpy(req.terminal_rsp, pTerminalRsp, nLength);
    RilLogV("ProtocolSimBuilder::%s() Data Length: %d", __FUNCTION__, req.terminal_rsp_len);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolStkBuilder::BuildStkEnvelopeStatus(int nLength, BYTE *pEnvelopeStatus)
{
    sit_stk_send_stk_envelope_with_status_req req;
    int length = sizeof(req);
    memset(&req, 0, length);

    if(nLength==0 || pEnvelopeStatus==NULL) return NULL;

    InitRequestHeader(&req.hdr, SIT_SEND_STK_ENVELOPE_WITH_STATUS, length);
    //req.envelope_cmd_len = (WORD) HexString2Value(req.envelope_cmd, (char *) pEnvelopeStatus);
    req.envelope_cmd_len = (WORD) nLength;
    memcpy(req.envelope_cmd, pEnvelopeStatus, nLength);
    RilLogV("ProtocolSimBuilder::%s() Data Length: %d", __FUNCTION__, req.envelope_cmd_len);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolStkBuilder::BuildStkCallSetup(int nUserOper)
{
	sit_stk_call_setup_req req;
    int length = sizeof(req);
    memset(&req, 0, length);

    InitRequestHeader(&req.hdr, SIT_STK_CALL_SETUP, length);
    req.user_operation = ((BYTE) nUserOper==SIT_STK_CALL_SETUP_ACCEPT)? SIT_STK_CALL_SETUP_ACCEPT : SIT_STK_CALL_SETUP_REJECT;
    RilLogV("ProtocolSimBuilder::%s() Data Length: %d", __FUNCTION__, length);

    return new ModemData((char *)&req, length);
}
