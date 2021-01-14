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
 * protocolsimbuilder.cpp
 *
 *  Created on: 2014. 6. 28.
 *      Author: MOX
 */

#include "protocolsimbuilder.h"
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

#define    SWAP16(val)        ( (((val) << 8) & 0xFF00) | (((val) >> 8) & 0x00FF) )
#define    SWAP32(val)        ( (((val) & 0x000000FF) << 24) | (((val) & 0x0000FF00) <<  8) | (((val) & 0x00FF0000) >>  8) | (((val) & 0xFF000000) >> 24) )

ModemData *ProtocolSimBuilder::BuildSimGetStatus()
{
    null_data_format req;
    int length = sizeof(null_data_format);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_SIM_STATUS, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimVerifyPin(int nPinIndex, const char *pszPin, const char *pszAID)
{
    sit_sim_verify_sim_pin_req req;
    int length = sizeof(req);
    int nReqID;

    switch(nPinIndex)
    {
    case 1: nReqID = SIT_VERIFY_SIM_PIN; break;
    case 2: nReqID = SIT_VERIFY_SIM_PIN2; break;
    default:
        return NULL;
    }

    if(pszPin==NULL) return NULL;

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, nReqID, length);

    req.pin_len = (strlen(pszPin)<=MAX_SIM_PIN_LEN)? strlen(pszPin): MAX_SIM_PIN_LEN;
    memcpy(req.pin, pszPin, req.pin_len);
    //req.pin[req.pin_len] = '\0';

    if (pszAID != NULL) {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimVerifyPuk(int nPukIndex, const char *pszPuk, const char *pszNewPin, const char *pszAID)
{
    sit_sim_verify_sim_puk_req req;
    int length = sizeof(req);
    int nReqID;

    switch(nPukIndex)
    {
    case 1: nReqID = SIT_VERIFY_SIM_PUK; break;
    case 2: nReqID = SIT_VERIFY_SIM_PUK2; break;
    default:
        return NULL;
    }

    if(pszPuk==NULL || pszNewPin==NULL) return NULL;

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, nReqID, length);

    req.puk_len = (strlen(pszPuk)<=MAX_SIM_PUK_LEN)? strlen(pszPuk): MAX_SIM_PUK_LEN;
    memcpy(req.puk, pszPuk, req.puk_len);
    //req.puk[req.puk_len] = '\0';

    req.new_pin_len = (strlen(pszNewPin)<=MAX_SIM_PIN_LEN)? strlen(pszNewPin): MAX_SIM_PIN_LEN;
    memcpy(req.new_pin, pszNewPin, req.new_pin_len);
    //req.new_pin[req.new_pin_len] = '\0';

    if (pszAID != NULL) {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimChangePin(int nPinIndex, const char *pszOldPin, const char *pszNewPin, const char *pszAID)
{
    sit_sim_change_sim_pin_req req;
    int length = sizeof(req);
    int nReqID, nFAC = 0;

    switch(nPinIndex)
    {
    case 1: nReqID = SIT_CHG_SIM_PIN; nFAC = SIT_SIM_FAC_SC; break;
    case 2: nReqID = SIT_CHG_SIM_PIN2; nFAC = SIT_SIM_FAC_SC2; break;
    default:
        return NULL;
    }

    if(pszOldPin==NULL || pszNewPin==NULL) return NULL;

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, nReqID, length);

    req.fac = nFAC;
    req.old_pin_len = (strlen(pszOldPin)<=MAX_SIM_PIN_LEN)? strlen(pszOldPin): MAX_SIM_PIN_LEN;
    memcpy(req.old_pin, pszOldPin, req.old_pin_len);
    //req.old_pin[req.old_pin_len] = '\0';

    req.new_pin_len = (strlen(pszNewPin)<=MAX_SIM_PIN_LEN)? strlen(pszNewPin): MAX_SIM_PIN_LEN;
    memcpy(req.new_pin, pszNewPin, req.new_pin_len);
    //req.new_pin[req.new_pin_len] = '\0';

    if (pszAID != NULL) {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimVerifyNetworkLock(int nFac, const char *pszPassword, int nSvcClass, const char *pszAID)
{
    sit_sim_verify_network_lock_req req;
    int length = sizeof(req);

    if(pszPassword==NULL) return NULL;

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_VERIFY_NETWORK_LOCK, length);
    req.fac = (BYTE) nFac;

    req.fac = nFac;
    req.password_len = (strlen(pszPassword)<=MAX_SIM_FACILITY_PASSWORD_LEN)? strlen(pszPassword): MAX_SIM_FACILITY_PASSWORD_LEN;
    memcpy(req.password, pszPassword, req.password_len);
    //req.password[req.password_len] = '\0';

    req.service_class = (BYTE) nSvcClass;
    if (pszAID != NULL) {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimIO(int nCmd, int nAppType, int nFileID, const char *pPath, BYTE p1, BYTE p2, BYTE p3,
                                                int nDataLen, const BYTE *pData, const char *pszPin2, const char *pszAID)
{
    sit_sim_sim_io_req req;
    int length = sizeof(req);

    if(pPath==NULL || pData==NULL) return NULL;

    BYTE abCommandValues[] = { 0xB0, 0xB2, 0xC0, 0xD6, 0xDC, 0xF2 };
    char aszCommands[][16] = {"READ_BINARY", "READ_RECORD", "GET_RESPONSE", "UPDATE_BINARY", "UPDATE_RECORD", "STATUS" };
    char szCmd[16] = "Unknown";
    for(int i=0; i<(int)(sizeof(abCommandValues)/sizeof(BYTE)); i++) {
        if(nCmd==(int) abCommandValues[i]) {
            SECURELIB::strncpy(szCmd, sizeof(szCmd), aszCommands[i], SECURELIB::strlen(aszCommands[i]));
        }
    }
    RilLogV("ProtocolSimBuilder::%s() Cmd:%s(0x%02X), AppType:%d, FileID:%X, Path:%s", __FUNCTION__, szCmd, nCmd, nAppType, nFileID, pPath);
    RilLogV("ProtocolSimBuilder::%s() P1:%d=0x%02X, P2:%d=0x%02X, P3:%d=0x%02X", __FUNCTION__, p1, p1, p2, p2, p3, p3);
    if(pszPin2!=NULL && strlen(pszPin2)>0) RilLogV("ProtocolSimBuilder::%s() PIN2:%s", __FUNCTION__, pszPin2);

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SIM_IO, length);
    req.command = (BYTE) nCmd;
    req.app_type = (BYTE) nAppType;
    req.file_id = (INT16) nFileID;

    if (pPath != NULL && strlen(pPath)>0)
    {
        // convert HEX string to HEX value
        BYTE arPath[MAX_SIM_IO_PATH_LEN] = { 0, };
        req.path_len = (BYTE)HexString2Value(arPath, pPath);
        if ( req.path_len <= 0 )
        {
            return NULL;
        }

        // Swap for Big-Endian
        for(int i=0; i<req.path_len; i+=2)
        {
            //WORD wPath16 = *((WORD *) &arPath[i]);
            //*((WORD *) &arPath[i]) = SWAP16(wPath16);
            *((WORD *) &arPath[i]) = SWAP16(*((WORD *) &arPath[i]));
        }
        memcpy(req.path, arPath, (size_t)req.path_len);
    }

    req.p1 = p1;
    req.p2 = p2;
    req.p3 = p3;

    if(pData!=NULL)
    {
        //req.data_len = (WORD) (nDataLen/2);
        //memcpy(req.data, pData, nDataLen/2);
        req.data_len = (WORD) HexString2Value(req.data, (char *) pData);
        RilLogV("ProtocolSimBuilder::%s() Data Length: %d", __FUNCTION__, req.data_len);
    }

    if(pszPin2!=NULL)
    {
        req.pin2_len = (strlen(pszPin2)<=MAX_SIM_PIN_LEN)? strlen(pszPin2): MAX_SIM_PIN_LEN;
        memcpy(req.pin2, pszPin2, req.pin2_len);
        //req.pin2[req.pin2_len] = '\0';
    }

    if (pszAID != NULL) {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimGetFacilityLock(char *pszCode, char *pszPassword, int nSvcClass, char *pszAID)
{
    sit_sim_get_facility_lock_req req;
    int length = sizeof(req);

    char aszFacCode[SIT_SIM_FAC_MAX][MAX_FACILITY_CODE_LEN+1] = { "CS", "PS", "PF", "SC", "AO",
                                                              "OI", "OX", "AI", "IR", "NT",
                                                              "NM", "NS", "NA", "AB", "AG",
                                                              "AC", "FD", "PN", "PU", "PP",
                                                              "PC", "SC2" };

    int nCode = SIT_SIM_FAC_MAX;
    for(int i=0; i<SIT_SIM_FAC_MAX; i++)
    {
        if(strcmp(pszCode, aszFacCode[i])==0) { nCode = i; break; }
    }

    RilLogV("ProtocolSimBuilder::%s() Code:%d, Password:%s, ServiceClass:%d, AID:%s", __FUNCTION__, nCode, pszPassword, nSvcClass, pszAID);

    if(pszPassword==NULL) return NULL;

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_FACILITY_LOCK, length);
    req.code = (BYTE) nCode;

    if(strlen(pszPassword)>0)
    {
        req.password_len = (strlen(pszPassword)<=MAX_SIM_FACILITY_PASSWORD_LEN)? strlen(pszPassword): MAX_SIM_FACILITY_PASSWORD_LEN;
        memcpy(req.password, pszPassword, req.password_len);
        //req.password[req.password_len] = '\0';
    }

    req.service_class = (BYTE) nSvcClass;
    if (pszAID != NULL) {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimSetFacilityLock(char *pszCode, int nLockMode, char *pszPassword, int nSvcClass, char *pszAID)
{
    sit_sim_set_facility_lock_req req;
    int length = sizeof(req);

    char aszFacCode[SIT_SIM_FAC_MAX][MAX_FACILITY_CODE_LEN+1] = { "CS", "PS", "PF", "SC", "AO",
                                                                  "OI", "OX", "AI", "IR", "NT",
                                                                  "NM", "NS", "NA", "AB", "AG",
                                                                  "AC", "FD", "PN", "PU", "PP",
                                                                  "PC", "SC2" };

    int nCode = SIT_SIM_FAC_MAX;
    for(int i=0; i<SIT_SIM_FAC_MAX; i++)
    {
        if(strcmp(pszCode, aszFacCode[i])==0) { nCode = i; break; }
    }

    RilLogV("ProtocolSimBuilder::%s() Code:%d, Password:%s, ServiceClass:%d, AID:%s", __FUNCTION__, nCode, pszPassword, nSvcClass, pszAID);

    if(pszPassword==NULL) return NULL;

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_FACILITY_LOCK, length);
    req.code = (BYTE) nCode;
    req.lock_mode = (BYTE) nLockMode;


    req.password_len = (strlen(pszPassword)<=MAX_SIM_FACILITY_PASSWORD_LEN)? strlen(pszPassword): MAX_SIM_FACILITY_PASSWORD_LEN;
    memcpy(req.password, pszPassword, req.password_len);
    //req.password[req.password_len] = '\0';

    req.service_class = (BYTE) nSvcClass;
    if (pszAID != NULL) {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimGetIsimAuth(int nAuthType, BYTE *pAuth, int nAuthLengh)
{
    sit_sim_get_sim_auth_req req;
    int length = sizeof(req.hdr) + sizeof(req.auth_type) + sizeof(req.auth_len) + nAuthLengh;

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_SIM_AUTH, length);
    req.auth_type = (BYTE) nAuthType;
    if(pAuth!=NULL)
    {
        req.auth_len = (BYTE) nAuthLengh;
        memcpy(req.auth, pAuth, nAuthLengh);
    }
    return new ModemData((char *) &req, length);
}

ModemData *ProtocolSimBuilder::BuildSimGetSimAuth(int nAuthContext, BYTE *pAuth, int nAuthLengh, int nAppType)
{
    RilLogV("ProtocolSimBuilder::%s() nAuthContext:0x%X, nAuthLengh:%d, nAppType:0x%X", __FUNCTION__, nAuthContext, nAuthLengh, nAppType);
    PrintBufferDump("Auth", pAuth, nAuthLengh);

    int nAuthType = -1;
    if(nAppType==RIL_APPTYPE_ISIM) nAuthType = SIT_SIM_AUTH_IMS;
    else
    {
        /* _____XXX
         * 000 : GSM context
         * 001 : 3G context
         * 010 : VGCS/VBS context
         * 100 : GBA context
         */
        const int AUTH_GSM_CONTEXT = 0b00000000;
        const int AUTH_3G_CONTEXT = 0b00000001;
        const int AUTH_VGCS_VBS_CONTEXT = 0b00000010;
        const int AUTH_GBA_CONTEXT = 0b00000100;

        // After removing Specific reference data
        switch((nAuthContext & 0x00000007))
        {
        case AUTH_GSM_CONTEXT: nAuthType = SIT_SIM_AUTH_GSM; break;
        case AUTH_3G_CONTEXT: nAuthType = SIT_SIM_AUTH_3G; break;
        case AUTH_VGCS_VBS_CONTEXT:
        case AUTH_GBA_CONTEXT:
        default:
            RilLogE("%s() Not Supported AuthContext(0x%08X)", __FUNCTION__, nAuthContext);
            return NULL;
        }
    }

    sit_sim_get_sim_auth_req req;
    memset(&req, 0, sizeof(sit_sim_get_sim_auth_req));
    InitRequestHeader(&req.hdr, SIT_GET_SIM_AUTH, sizeof(sit_sim_get_sim_auth_req));
    req.auth_type = (BYTE) nAuthType;
    if(pAuth!=NULL && nAuthLengh>0)
    {
        req.auth_len = (BYTE) nAuthLengh;
        memcpy(req.auth, pAuth, nAuthLengh);
    }

    return new ModemData((char *) &req, sizeof(sit_sim_get_sim_auth_req));
}

ModemData *ProtocolSimBuilder::BuildSimTransmitApduBasic(int nSessionID, int cla, int instruction,
                                                int p1, int p2, int p3, const char *pszApduData)
{
    int nLength = sizeof(sit_sim_transmit_sim_apdu_basic_req);
    if(pszApduData!=NULL && strlen(pszApduData)>0) nLength += strlen(pszApduData);
    BYTE *pBuffer = new BYTE[nLength];
    sit_sim_transmit_sim_apdu_basic_req *pReq = (sit_sim_transmit_sim_apdu_basic_req *) pBuffer;
    memset(pReq, 0, nLength);
    InitRequestHeader(&pReq->hdr, SIT_TRANSMIT_SIM_APDU_BASIC, nLength);
    pReq->session_id = nSessionID;
    pReq->apdu_len = sizeof(sit_sim_apdu);
    pReq->entry.sim_apdu.cla = cla;
    pReq->entry.sim_apdu.instruction = instruction;
    pReq->entry.sim_apdu.p1 = p1;
    pReq->entry.sim_apdu.p2 = p2;
    pReq->entry.sim_apdu.p3 = p3;

    int nDataLen = 0;
    if(pszApduData != NULL && strlen(pszApduData)>0)
    {
        int nDataLength = strlen(pszApduData);
        BYTE *pData = new BYTE[nDataLength];
        memset(pData, 0, nDataLength);
        nDataLen = HexString2Value(pData, pszApduData);
        if((nDataLen+sizeof(sit_sim_apdu))>MAX_APDU_LEN) nDataLen = MAX_APDU_LEN - sizeof(sit_sim_apdu);
        memcpy(pReq->entry.sim_apdu.data, pData, nDataLen);
        delete []pData;
    }

    pReq->apdu_len += nDataLen;
    ModemData *pModemData = new ModemData((char *) pReq, nLength);
    delete [] pBuffer;

    return pModemData;
}

ModemData *ProtocolSimBuilder::BuildSimOpenChannel(const char *pszAID)
{
    sit_sim_open_sim_channel_req req;
    int length = sizeof(req);

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_OPEN_SIM_CHANNEL, length);
    if(pszAID!=NULL)
    {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimOpenChannelWithP2(const char *pszAID, int p2)
{
    sit_sim_open_sim_channel_with_p2_req req;
    int length = sizeof(req);

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_OPEN_SIM_CHANNEL_WITH_P2, length);
    if(pszAID!=NULL)
    {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }

    req.p2 = (BYTE) p2;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimCloseChannel(int nSessionID)
{
    sit_sim_close_sim_channel_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_CLOSE_SIM_CHANNEL, length);
    req.session_id = nSessionID;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimTransmitApduChannel(int nSessionID, int cla, int instruction,
                                                int p1, int p2, int p3, const char *pszApduData)
{
    int nLength = sizeof(sit_sim_transmit_sim_apdu_channel_req);
    if(pszApduData!=NULL && strlen(pszApduData)>0) nLength += strlen(pszApduData);
    BYTE *pBuffer = new BYTE[nLength];
    sit_sim_transmit_sim_apdu_channel_req *pReq = (sit_sim_transmit_sim_apdu_channel_req *) pBuffer;
    memset(pReq, 0, nLength);
    InitRequestHeader(&pReq->hdr, SIT_TRANSMIT_SIM_APDU_CHANNEL, nLength);
    pReq->session_id = nSessionID;
    pReq->cla = cla;
    pReq->instruction = instruction;
    pReq->p1 = p1;
    pReq->p2 = p2;
    pReq->p3 = p3;

    if(pszApduData != NULL && strlen(pszApduData)>0)
    {
        int nDataLength = strlen(pszApduData);
        BYTE *pData = new BYTE[nDataLength];
        memset(pData, 0, nDataLength);
        int nDataLen = HexString2Value(pData, pszApduData);
        pReq->data_len = (nDataLen<=MAX_APDU_LEN)? nDataLen: MAX_APDU_LEN;
        memcpy(pReq->data, pData, pReq->data_len);
        delete []pData;
    }

    ModemData *pModemData = new ModemData((char *) pReq, nLength);
    delete [] pBuffer;

    return pModemData;
}

ModemData *ProtocolSimBuilder::BuildGetImsi(const char *pszAID)
{
    sit_id_get_imsi_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_IMSI, length);
    if (pszAID != NULL && *pszAID != 0) {
        // convert HEX string to HEX value
        req.aid_len = (BYTE) HexString2Value(req.aid, pszAID);
    }
    return new ModemData((char *)&req, length);
}
ModemData *ProtocolSimBuilder::BuildSimGetGbaAuth(const char *pGetGbaAuthdata)
{
    if(pGetGbaAuthdata==NULL) return NULL;

    sit_sim_get_sim_gba_auth_req req;
    int length = sizeof(sit_sim_get_sim_gba_auth_req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_GBA_CONTEXT, length);

    if (pGetGbaAuthdata[0] != 0 && pGetGbaAuthdata[0] != 1) return NULL;
    req.auth_type = pGetGbaAuthdata[0];

    if (pGetGbaAuthdata[1] != 0 && pGetGbaAuthdata[1] != 1) return NULL;
    req.gba_type = pGetGbaAuthdata[1];

    req.gba_tag = pGetGbaAuthdata[2];
    req.data1_len = pGetGbaAuthdata[3];
    memcpy(req.data1, &pGetGbaAuthdata[4], req.data1_len);
    req.data2_len = pGetGbaAuthdata[4+255];
    memcpy(req.data2, &pGetGbaAuthdata[4+255+1], req.data2_len);
    //memcpy(req.auth, pAuth, nAuthLengh);
    return new ModemData((char *) &req, length);
}

ModemData *ProtocolSimBuilder::BuildSimGetATR()
{
    null_data_format req;
    int length = sizeof(null_data_format);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_ATR, length);
    return new ModemData((char *)&req, length);
}

/* PhoneBook */
ModemData *ProtocolSimBuilder::BuildSimReadPbEntry(int pb_type, int index)
{
    sit_read_pb_entry_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_READ_PB_ENTRY, length);
    req.pb_type = pb_type;
    req.index = index;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimUpdatePbDelete(int mode, int type, int index)
{
    sit_update_pb_entry_req req;
    memset(&req, 0, sizeof(sit_update_pb_entry_req));

    req.mode = mode;
    req.pb_type = type;
    req.index = index;
    int length = sizeof(null_data_format) + 5;

    InitRequestHeader(&req.hdr, SIT_UPDATE_PB_ENTRY, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimUpdatePbEntry1(int mode, int type, int index, int length, char *pb)
{
    char req_pkt[MAX_PB_ENTRY_LEN] = { 0, };
    sit_update_pb_entry_req *req = (sit_update_pb_entry_req *)req_pkt;
    int tlen = sizeof(null_data_format) + 5;

    req->mode = mode;
    req->pb_type = type;
    req->index = index;
    req->entry_len = length;
    memcpy(req_pkt+tlen, pb, length);
    tlen += length;

    InitRequestHeader(&req->hdr, SIT_UPDATE_PB_ENTRY, tlen);
    return new ModemData((char *)req_pkt, tlen);
}

ModemData *ProtocolSimBuilder::BuildSimUpdatePbEntry2(int mode, int type, int index, int length, char *pb)
{
    char req_pkt[MAX_PB_ENTRY_LEN] = { 0, };
    sit_update_pb_entry_req *req = (sit_update_pb_entry_req *)req_pkt;
    int tlen = sizeof(null_data_format) + 5;

    req->mode = mode;
    req->pb_type = type;
    req->index = index;
    req->entry_len = length;
    memcpy(req_pkt+tlen, pb, length);
    tlen += length;

    InitRequestHeader(&req->hdr, SIT_UPDATE_PB_ENTRY, tlen);
    return new ModemData((char *)req_pkt, tlen);
}

ModemData *ProtocolSimBuilder::BuildSimUpdatePb3gEntry(int mode, int type, int index, int length, char *pb)
{
    char req_pkt[MAX_PB_ENTRY_LEN] = { 0, };
    sit_update_pb_entry_req *req = (sit_update_pb_entry_req *)req_pkt;
    int tlen = sizeof(null_data_format) + 5;

    req->mode = mode;
    req->pb_type = type;
    req->index = index;
    req->entry_len = length;
    memcpy(req_pkt+tlen, pb, length);
    tlen += length;

    InitRequestHeader(&req->hdr, SIT_UPDATE_PB_ENTRY, tlen);
    return new ModemData((char *)req_pkt, tlen);
}

ModemData *ProtocolSimBuilder::BuildSimGetPbStorageInfo(int pbType)
{
    sit_sim_pb_entry_info req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_PB_STORAGE_INFO, length);
    req.pb_type = pbType;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimPbStorageList()
{
    null_data_format req;
    int length = sizeof(null_data_format);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_PB_STORAGE_LIST, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimGetPbEntryInfo(int pbType)
{
    sit_sim_pb_entry_info req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_PB_ENTRY_INFO, length);
    req.pb_type = pbType;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSim3GPbCapa()
{
    null_data_format req;
    int length = sizeof(null_data_format);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_3G_PB_CAPA, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSetCarrierRestrictions(int lenAllow, int lenExclude, CarrierInfo *pAllowed, CarrierInfo *pExcluded)
{
    sit_net_set_carrier_restriction_req req;
    int length = sizeof(sit_net_set_carrier_restriction_req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_CARRIER_RESTRICTIONS, length);

    req.allowed_carriers_len = (lenAllow > MAX_CARRIER_INFO_NUM) ? MAX_CARRIER_INFO_NUM: lenAllow;
    req.excluded_carriers_len = (lenExclude > MAX_CARRIER_INFO_NUM) ? MAX_CARRIER_INFO_NUM: lenExclude;

    int index;
    for(index = 0; index < req.allowed_carriers_len; index++) {
        CarrierInfo *pRead = pAllowed + index;
        SIT_CARRIER_INFO *pWrite = req.allowed_carrier_list + index;

        memcpy(pWrite->mcc, pRead->mcc, MAX_MCC_LEN);
        memcpy(pWrite->mnc, pRead->mnc, MAX_MNC_LEN);
        pWrite->match_type = pRead->match_type;
        pWrite->match_len = strlen(pRead->match_data);
        if (pWrite->match_len > MAX_CR_MATCH_DATA_SIZE) pWrite->match_len = MAX_CR_MATCH_DATA_SIZE;
        memcpy(pWrite->match_data, pRead->match_data, pWrite->match_len);
    }

    for(index = 0; index < req.excluded_carriers_len; index++) {
        CarrierInfo *pRead = pExcluded + index;
        SIT_CARRIER_INFO *pWrite = req.excluded_carrier_list + index;

        memcpy(pWrite->mcc, pRead->mcc, MAX_MCC_LEN);
        memcpy(pWrite->mnc, pRead->mnc, MAX_MNC_LEN);
        pWrite->match_type = pRead->match_type;
        pWrite->match_len = strlen(pRead->match_data);
        if (pWrite->match_len > MAX_CR_MATCH_DATA_SIZE) pWrite->match_len = MAX_CR_MATCH_DATA_SIZE;
        memcpy(pWrite->match_data, pRead->match_data, pWrite->match_len);
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildGetCarrierRestrictions()
{
    sit_net_get_carrier_restriction_req req;
    int length = sizeof(sit_net_get_carrier_restriction_req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_CARRIER_RESTRICTIONS, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSetSimCardPower(int isPowerUp)
{
    sit_sim_set_sim_card_power_req req;
    int length = sizeof(sit_sim_set_sim_card_power_req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_SIM_CARD_POWER, length);

    req.mode = isPowerUp;

    return new ModemData((char *)&req, length);
}

// Secure Element
ModemData *ProtocolSimBuilder::BuildOemSimRequest(int msgId, BYTE *pData, int nDataLength)
{
    int nSitMsg;
    switch(msgId)
    {
    // IMS SIM IO
    case RIL_REQUEST_SIM_IO: nSitMsg = SIT_SIM_IO; break;
    // Secure Element
    case RIL_REQUEST_SIM_OPEN_CHANNEL: nSitMsg = SIT_OPEN_SIM_CHANNEL_WITH_P2; break;
    case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL: nSitMsg = SIT_TRANSMIT_SIM_APDU_CHANNEL; break;
    case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC: nSitMsg = SIT_TRANSMIT_SIM_APDU_BASIC; break;
    default: return NULL;
    }

    BYTE *pBuffer = new BYTE[sizeof(null_data_format)+nDataLength];
    null_data_format *pReq = (null_data_format *) pBuffer;
    int length = sizeof(null_data_format) + nDataLength;
    InitRequestHeader(&pReq->hdr, nSitMsg, length);
    memcpy(pBuffer+sizeof(null_data_format), pData, nDataLength);

    ModemData *pModemData = new ModemData((char *)pReq, length);
    delete [] pBuffer;

    return pModemData;
}

ModemData *ProtocolSimBuilder::BuildGetSimLockInfo()
{
    sit_sim_get_sim_lock_info_req req;
    int length = sizeof(sit_sim_get_sim_lock_info_req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_OEM_GET_SIM_LOCK_INFO, length);
    return new ModemData((char *)&req, length);
}

// Radio Config
ModemData *ProtocolSimBuilder::BuildSimGetSlotStatus()
{
    null_data_format req;
    int length = sizeof(null_data_format);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_SLOT_STATUS, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSimBuilder::BuildSimSetLogicalSlotMapping(int *pData, int nDataLength)
{
    sit_sim_set_logical_to_physical_slot_mapping_req req;
    int length = sizeof(sit_sim_set_logical_to_physical_slot_mapping_req);

    if (nDataLength > MAX_SLOT_NUM || nDataLength <= 0)
        return NULL;

    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING, length);

    req.slotMaplen = (BYTE) nDataLength;
    for(int i=0; i<nDataLength; i++)
        req.slotMap[i] = (BYTE)pData[i];

    ModemData *pModemData = new ModemData((char *)&req, length);

    return pModemData;
}
