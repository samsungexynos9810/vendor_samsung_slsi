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
 * protocolpsbuilder.cpp
 *
 *  Created on: 2014. 6. 27.
 *      Author: sungwoo48.choi
 */
#include "protocolpsbuilder.h"
#include "pdpcontext.h"
#include "apnsetting.h"
#include "rillog.h"
#include "util.h"
#include "rilproperty.h"

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define PROPERTY_HANDOVER                "vendor.ril.allow.handover"
#define PROPERTY_HANDOVER_IPV4_ADDRESS   "vendor.ril.ipv4.address"
#define PROPERTY_HANDOVER_IPV6_ADDRESS   "vendor.ril.ipv6.address"
#define  INVALID_IP   "0.0.0.0"


static int ConvertDataProfileIdToProtocolDataProfile(int dataProfileId)
{
    if (dataProfileId == RIL_DATA_PROFILE_TETHERED) {
        return SIT_PDP_DATA_PROFILE_TETHERED;
    }
    return SIT_PDP_DATA_PROFILE_DEFAULT;
}

static int GetProtocolApnType(int dataProfileId, ApnSetting *apnSetting)
{
    int apntype = SIT_PDP_APN_TYPE_DEFAULT;
    int matched = 0;

    // In all type '*' case, we just follows profileID.
    // IMS and Emergency is considered as decidated PDN.
    // SIT interface just concern about special APN like IMS and emergency.
    if(apnSetting->CanHandleType(APN_TYPE_EMERGENCY)) {
       apntype = SIT_PDP_APN_TYPE_EMERGENCY;
       matched++;
    }
    if(apnSetting->CanHandleType(APN_TYPE_IMS)) {
       apntype = SIT_PDP_APN_TYPE_IMS;
       matched++;
    }
    // When multiple type is supported, DEFAULT type will be set.
    if(apnSetting->CanHandleType(APN_TYPE_DEFAULT)) {
       apntype = SIT_PDP_APN_TYPE_DEFAULT;
       matched++;
    }
    if(matched > 1)
    {
        // Second chance to adjust apn type just for IMS
        switch (dataProfileId) {
        case DATA_PROFILE_IMS:
            apntype = SIT_PDP_APN_TYPE_IMS;
            break;
        default:
            apntype = SIT_PDP_APN_TYPE_DEFAULT;
            break;
        } // end switch ~
        RilLogV("GetProtocolApnType: Multiple Type is requested, follow ProfileID");
    }
    RilLogV("GetProtocolApnType: apnSetting->GetType()=%s, ProfileID:%d, SIT apntype=%d", apnSetting->GetType(), dataProfileId, apntype);

    return apntype;
}

static int ConvertAuthTypeToProtocolAuthType(int authtype)
{
    int type = SIT_PDP_AUTH_TYPE_NONE;
    switch (authtype) {
    case SETUP_DATA_AUTH_NONE:
        type = SIT_PDP_AUTH_TYPE_NONE;
        break;
    case SETUP_DATA_AUTH_PAP:
        type = SIT_PDP_AUTH_TYPE_PAP;
        break;
    case SETUP_DATA_AUTH_CHAP:
        type = SIT_PDP_AUTH_TYPE_CHAP;
        break;
    case SETUP_DATA_AUTH_PAP_CHAP:
        type = SIT_PDP_AUTH_TYPE_PAP_CHAP;
        break;
    default:
        type = SIT_PDP_AUTH_TYPE_NONE;
        break;
    } // end switch ~
    return type;
}

static int GetPdpType(const char *protocol)
{
    // default
    if (TextUtils::IsEmpty(protocol)) {
        return SIT_PDP_PDP_TYPE_IPV4;
    }

    if (strcmp(protocol, STR_PDP_TYPE_IPV4) == 0) {
        return SIT_PDP_PDP_TYPE_IPV4;
    }
    else if (strcmp(protocol, STR_PDP_TYPE_IPV6) == 0) {
        return SIT_PDP_PDP_TYPE_IPV6;
    }
    else if (strcmp(protocol, STR_PDP_TYPE_IPV4V6) == 0) {
        return SIT_PDP_PDP_TYPE_IPV4IPV6;
    }

    // default
    return SIT_PDP_PDP_TYPE_IPV4;
}

static int ConvertPdpProtocolTypeToSitPdpType(int protocol)
{
    switch(protocol){
      case 0: /* IP */
          return SIT_PDP_PDP_TYPE_IPV4;
      case 1: /* IPV6 */
          return SIT_PDP_PDP_TYPE_IPV6;
      case 2: /* IPV4V6 */
          return SIT_PDP_PDP_TYPE_IPV4IPV6;
      case 3: /* PPP */
          return SIT_PDP_PDP_TYPE_PPP;
      case -1: /* UNKNOWN */
      case 4: /* NON_IP */
      case 5: /* UNSTRUCTURED */
          return SIT_PDP_PDP_TYPE_UNKNOWN;
      default:
          return SIT_PDP_PDP_TYPE_IPV4;
    };
}

static int GetPcscfReqType(ApnSetting *apnSetting, const char *protocol)
{
    if (TextUtils::IsEmpty(apnSetting->GetApn()) ) {
        RilLogV("GetPcscfReqType: Null APN IPv4v6");
        return SIT_PDP_PCSCF_REQ_IPV4V6;
    } else if (apnSetting->CanHandleType(APN_TYPE_IMS) || apnSetting->CanHandleType(APN_TYPE_EMERGENCY) ) {
        // default
        if (TextUtils::IsEmpty(protocol)) {
            return SIT_PDP_PCSCF_REQ_IPV4;
        }

        if (strcmp(protocol, STR_PDP_TYPE_IPV4) == 0) {
            return SIT_PDP_PCSCF_REQ_IPV4;
        }
        else if (strcmp(protocol, STR_PDP_TYPE_IPV6) == 0) {
            return SIT_PDP_PCSCF_REQ_IPV6;
        }
        else if (strcmp(protocol, STR_PDP_TYPE_IPV4V6) == 0) {
            return SIT_PDP_PCSCF_REQ_IPV4V6;
        }

        // default
        return SIT_PDP_PCSCF_REQ_IPV4;
    }

    return SIT_PDP_PCSCF_REQ_NONE;
}

static int GetPcscfReqTypeforNullAPN(ApnSetting *apnSetting, const char *protocol)
{
       if (TextUtils::IsEmpty(apnSetting->GetApn()) ) {
          RilLogV("GetPcscfReqType: Null APN in DB");
       }

       if (!TextUtils::IsEmpty(protocol)) {
          RilLogV("GetPcscfReqTypeforNullAPN : Following APNSetting");
          if (strcmp(protocol, STR_PDP_TYPE_IPV4) == 0) {
              return SIT_PDP_PCSCF_REQ_IPV4;
          }
          else if (strcmp(protocol, STR_PDP_TYPE_IPV6) == 0) {
              return SIT_PDP_PCSCF_REQ_IPV6;
          }
          else if (strcmp(protocol, STR_PDP_TYPE_IPV4V6) == 0) {
              return SIT_PDP_PCSCF_REQ_IPV4V6;
          }
       }else{
             RilLogV("GetPcscfReqTypeforNullAPN : No protocol value exists in DB ,set as default (v4v6)");
             return SIT_PDP_PCSCF_REQ_IPV4V6;
       }

    return SIT_PDP_PCSCF_REQ_NONE;
}

static int GetVzwProfileId(ApnSetting *apnSetting)
{
    if ( true == apnSetting->CanHandleType(APN_TYPE_DEFAULT) ) {
        return SIT_IA_PROFILE_ID_VZWDEFAULT;
    }
    else if ( true == apnSetting->CanHandleType(APN_TYPE_IMS) ) {
        return SIT_IA_PROFILE_ID_VZWIMS;
    }
    else if ( true == apnSetting->CanHandleType(APN_TYPE_FOTA) ) {
        return SIT_IA_PROFILE_ID_VZWFOTA;
    }
    else if ( true == apnSetting->CanHandleType(APN_TYPE_CBS) ) {
        return SIT_IA_PROFILE_ID_VZWCBS;
    }
    else if ( true == apnSetting->CanHandleType(APN_TYPE_EMERGENCY) ) {
        return SIT_IA_PROFILE_ID_VZWE911;
    }
    // default
    return SIT_IA_PROFILE_ID_UNKNOWN;
}

ModemData *ProtocolPsBuilder::BuildSetupDataCall(int rat, PdpContext *pPdpContext)
{
    RilLog("%s", __FUNCTION__);
    if (pPdpContext == NULL) {
        RilLog("%s pPdpContext == NULL", __FUNCTION__);
        return NULL;
    }

    ApnSetting *pApnSetting = pPdpContext->GetApnSetting();
    if (pApnSetting == NULL) {
        RilLog("%s pApnSetting == NULL", __FUNCTION__);
        return NULL;
    }

    sit_pdp_setup_data_call_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SETUP_DATA_CALL, length);

    // CID
    int cid = pPdpContext->GetCID();
    if (cid < 0) {
        RilLog("%s cid < 0", __FUNCTION__);
        return NULL;
    }
    req.cid = (BYTE)(cid & 0xFF);

    // RAT
    req.rat = (BYTE)(rat & 0xFF);

    if ( !FillApnInfo(req, pPdpContext, false, false) )
        return NULL;

    //Handover
    //clear address
    memset(req.IPv4Address, 0, MAX_IPV4_ADDR_LEN);
    memset(req.IPv6Address, 0, MAX_IPV6_ADDR_LEN);

    char allow_handover[MAX_ADDRESS_STRING_LEN];
    char buf[MAX_ADDRESS_STRING_LEN] = {0, };
    //char address[MAX_ADDRESS_STRING_LEN] = {0, };
    char nullIpv4[4] = {0, };
    char nullIpv6[16] = {0, };

    property_get(PROPERTY_HANDOVER, allow_handover, "0");

    if (strcmp(allow_handover, "1") == 0) {
        int err;
        //get ipv4 address
        property_get(PROPERTY_HANDOVER_IPV4_ADDRESS, buf, NULL);
        RilLogV("handover: IPv4 address:%s", buf);
        if(memcmp(buf, INVALID_IP, sizeof(INVALID_IP)) == 0) {
            RilLogV("handover: IPv4 zero set APN protocol as IPv6");
            req.pdp_type = SIT_PDP_PDP_TYPE_IPV6;
        }
        if(memcmp(buf, nullIpv4, 4) != 0) {
            err = inet_pton(AF_INET, buf, req.IPv4Address);
            if (err != 1) RilLogV("inet_pton for IPv4 failed: err:%d", err);
        }
        //get ipv6 address
        property_get(PROPERTY_HANDOVER_IPV6_ADDRESS, buf, NULL);
        RilLogV("handover: IPv6 address:%s", buf);
        if(memcmp(buf, nullIpv6, 16) != 0) {
            err = inet_pton(AF_INET6, buf, req.IPv6Address);
            if (err != 1) RilLogV("inet_pton for IPv6 failed: err:%d", err);
        }
    } else {
        RilLogW("Not handover");
    }
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolPsBuilder::BuildSetInitialAttachApn(PdpContext *pPdpContext, bool isEsmFlagZero/* = false*/)
{
    if (pPdpContext == NULL) {
        return NULL;
    }

    ApnSetting *pApnSetting = pPdpContext->GetApnSetting();
    if (pApnSetting == NULL) {
        return NULL;
    }

    sit_pdp_set_initial_attach_apn_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_INITIAL_ATTACH_APN, length);

    // CID
    int cid = pPdpContext->GetCID();
    if (cid < 0) {
        return NULL;
    }
    req.cid = (BYTE)(cid & 0xFF);
    req.rat = SIT_RAT_TYPE_LTE;

    if ( !FillApnInfo(req, pPdpContext, isEsmFlagZero, true) )
        return NULL;

    // ignore return
    FillDataProfileId(req, pPdpContext, true);

    // For TMO TC : L_LTE_ROAM_54650_INT Roam_302370_Data Roaming ON
    // roaming protocol
    req.roaming_pdp_type = ::GetPdpType(pApnSetting->GetRoamingProtocol());

    if(isEsmFlagZero ||TextUtils::IsEmpty(pApnSetting->GetApn()) ){
        req.roaming_pcscf_req_type =  ::GetPcscfReqTypeforNullAPN(pApnSetting, pApnSetting->GetRoamingProtocol());
    }
    else{
        // Roaming P-CSCF req type
        req.roaming_pcscf_req_type = ::GetPcscfReqType(pApnSetting, pApnSetting->GetRoamingProtocol());
        RilLogV("roaming_pcscf_req_type: %d", req.roaming_pcscf_req_type);
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolPsBuilder::BuildSetDataProfile(const RIL_DataProfileInfo_v15 *dpi, bool isVzw)
{
    if (dpi == NULL) {
        return NULL;
    }

    sit_pdp_set_data_profile_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_DATA_PROFILE, length);

    // profile id, vzw checks
    // Don't care operator. just pass profile_id directly
    req.profile_id = dpi->profileId;
    // 0xFF will be required for DualVoLTE, but higher layer will set this
    //if(!isVzw // APN==DEFAULT) req.profile_id = SIT_IA_PROFILE_ID_INTERNET;
    RilLogV("profile_id: %d", req.profile_id);

    // APN
    if (!TextUtils::IsEmpty(dpi->apn)) {
        strncpy(req.apn, dpi->apn, MAX_PDP_APN_LEN - 1);
    }
    else {
        RilLogV("Null APN is not allowed in DataProfile");
        return NULL;
    }

    // protocol
    req.pdp_type = ::GetPdpType(dpi->protocol);
    // roaming protocol
    req.roaming_pdp_type = ::GetPdpType(dpi->roamingProtocol);
    // auth type
    req.auth_type = ::ConvertAuthTypeToProtocolAuthType(dpi->authType);
    // username and password
    if (!TextUtils::IsEmpty(dpi->user))
        strncpy(req.username, dpi->user, MAX_AUTH_USER_NAME_LEN - 1);
    if (!TextUtils::IsEmpty(dpi->password))
        strncpy(req.password, dpi->password, MAX_AUTH_PASSWORD_LEN - 1);

    // apn disable flag
    req.enabled = dpi->enabled;

    // ? Vzw only parameters ?
    req.max_conns_time = 300;
    req.max_conns = 1023;
    req.wait_time = 0;

    req.radio_access_family = dpi->bearerBitmask;
    req.apn_type = dpi->supportedTypesBitmask;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolPsBuilder::BuildSetDataProfile(const RIL_DataProfileInfo_V1_4 *dpi, bool isVzw)
{
    if (dpi == NULL) {
        return NULL;
    }

    sit_pdp_set_data_profile_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_DATA_PROFILE, length);

    // profile id, vzw checks
    // Don't care operator. just pass profile_id directly
    req.profile_id = dpi->profileId;
    // 0xFF will be required for DualVoLTE, but higher layer will set this
    //if(!isVzw // APN==DEFAULT) req.profile_id = SIT_IA_PROFILE_ID_INTERNET;
    RilLogV("profile_id: %d", req.profile_id);

    // APN
    if (!TextUtils::IsEmpty(dpi->apn)) {
        strncpy(req.apn, dpi->apn, MAX_PDP_APN_LEN - 1);
    }
    else {
        RilLogV("Null APN is not allowed in DataProfile");
        return NULL;
    }

    // protocol
    req.pdp_type = ::ConvertPdpProtocolTypeToSitPdpType(dpi->protocol);
    // roaming protocol
    req.roaming_pdp_type = ::ConvertPdpProtocolTypeToSitPdpType(dpi->roamingProtocol);
    // auth type
    req.auth_type = ::ConvertAuthTypeToProtocolAuthType(dpi->authType);
    // username and password
    if (!TextUtils::IsEmpty(dpi->user))
        strncpy(req.username, dpi->user, MAX_AUTH_USER_NAME_LEN - 1);
    if (!TextUtils::IsEmpty(dpi->password))
        strncpy(req.password, dpi->password, MAX_AUTH_PASSWORD_LEN - 1);

    // apn disable flag
    req.enabled = dpi->enabled;

    // ? Vzw only parameters ?
    req.max_conns_time = 300;
    req.max_conns = 1023;
    req.wait_time = 0;

    req.radio_access_family = dpi->bearerBitmap;
    req.apn_type = dpi->supportedApnTypesBitmap;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolPsBuilder::BuildDeactDataCall(int cid, int reason)
{
    if (cid < 0) {
        return NULL;
    }

    sit_pdp_deact_data_call_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_DEACT_DATA_CALL, length);
    req.cid = (BYTE)(cid & 0xFF);
    req.deact_reason = (BYTE)(reason & 0xFF);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolPsBuilder::BuildGetDataCallList()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_DATA_CALL_LIST, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolPsBuilder::BuildSetFastDormancyInfo(BYTE lcdOn, BYTE lcdOff, BYTE rel8LcdOn, BYTE rel8LcdOff)
{
    sit_pdp_set_fd_info_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_FD_INFO, length);
    req.lcd_on = lcdOn & 0xFF;
    req.lcd_off = lcdOff & 0xFF;
    req.rel8_lcd_on = rel8LcdOn & 0xFF;
    req.rel8_lcd_off = rel8LcdOff & 0xFF;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolPsBuilder::BuildStartKeepAlive(RIL_KeepaliveRequest reqData)
{
    sit_pdp_start_keepalive_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_START_KEEPALIVE, length);

    req.keepalive_type = reqData.type;

    memcpy(req.src_addr, reqData.sourceAddress, MAX_IPV6_ADDR_LEN);
    req.source_port = reqData.sourcePort;

    memcpy(req.dst_addr, reqData.destinationAddress, MAX_IPV6_ADDR_LEN);
    req.dst_port = reqData.destinationPort;

    req.max_interval = reqData.maxKeepaliveIntervalMillis;
    req.cid = reqData.cid;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolPsBuilder::BuildStopKeepAlive(int sessionHande)
{
    sit_pdp_stop_keepalive_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_STOP_KEEPALIVE, length);

    req.keepalive_handle = sessionHande;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolPsBuilder::BuildSetPreferredDataModem(int stackId)
{
    if (stackId < 0) {
        return NULL;
    }

    sit_net_set_preferred_data_modem_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_PREFERRED_DATA_MODEM, length);
    req.stackId = (BYTE)(stackId & 0xFF);
    return new ModemData((char *)&req, length);
}

/*
 *  retrun false if failed, it needs to return NULL int the parent
 */
template <typename T>
bool ProtocolPsBuilder::FillApnInfo(T &req, PdpContext *pPdpContext, bool isEsmFlagZero, bool forInitialAttach)
{
    ApnSetting *pApnSetting = pPdpContext->GetApnSetting();
    if (pApnSetting == NULL) {
        return false;
    }

    // APN
    if (!isEsmFlagZero) {
        // APN
        const char *apn = pApnSetting->GetApn();
        if (!TextUtils::IsEmpty(apn)) {
            strncpy(req.apn, apn, MAX_PDP_APN_LEN - 1);
        }
        else {
            if ( forInitialAttach )
            {
                if (!pApnSetting->CanHandleType(APN_TYPE_IA) ||
                    pApnSetting->CanHandleType(APN_TYPE_EMERGENCY) ) {
                    return false;
                }
            } else if ( isRatForCDMA(req.rat) ) {
                // Allow null APN for SetupDataCall on CDMA RATs
                RilLogV("Allow null APN for CDMA RAT:%d", req.rat);
            } else if ( !pApnSetting->CanHandleType(APN_TYPE_EMERGENCY) ) {
                return false;
            }
        }

        // username and password
        if (!TextUtils::IsEmpty(pApnSetting->GetUsername()))
            strncpy(req.username, pApnSetting->GetUsername(), MAX_AUTH_USER_NAME_LEN - 1);
        if (!TextUtils::IsEmpty(pApnSetting->GetPassword()))
            strncpy(req.password, pApnSetting->GetPassword(), MAX_AUTH_PASSWORD_LEN - 1);

        // auth type
        req.auth_type = ::ConvertAuthTypeToProtocolAuthType(pApnSetting->GetAuthType());
        // P-CSCF req type
        req.pcscf_addr_req = ::GetPcscfReqType(pApnSetting, pApnSetting->GetProtocol());
    }
    else {
        // use NULL APN
       *req.apn = 0;
       *req.username = 0;
       *req.password = 0;
       req.auth_type = SIT_PDP_AUTH_TYPE_NONE;
       // P-CSCF req type
       req.pcscf_addr_req = ::GetPcscfReqTypeforNullAPN(pApnSetting, pApnSetting->GetProtocol());
    }

    // legacy data profile and APN type
    req.data_profile = ::ConvertDataProfileIdToProtocolDataProfile(pPdpContext->GetDataProfileId());
    req.apn_type = ::GetProtocolApnType(pPdpContext->GetDataProfileId(), pApnSetting);

    // protocol
    req.pdp_type = ::GetPdpType(pApnSetting->GetProtocol());


    return true;
}

template <typename T>
bool ProtocolPsBuilder::FillDataProfileId(T &req, PdpContext *pPdpContext, bool forInitialAttach)
{
    // DataProfileId
    if (forInitialAttach) {
        req.profile_id = SIT_IA_PROFILE_ID_UNKNOWN;
        if(TextUtils::IsEmpty(req.apn))
            pPdpContext->SetDataProfileId(DATA_PROFILE_NULL_APN_IA);
    } else {
        req.profile_id = pPdpContext->GetDataProfileId();
        if(req.profile_id == DATA_PROFILE_DEFAULT) {
            req.profile_id = SIT_IA_PROFILE_ID_INTERNET;
        } else {
            RilLogV("ignore non-Internet ProfileId: %d", req.profile_id);
            return false;
        }
    }
    return true;
}

bool ProtocolPsBuilder::isRatForCDMA(int rat)
{
    switch(rat) {
      case RADIO_TECH_IS95A: [[fallthrough]];
      case RADIO_TECH_IS95B: [[fallthrough]];
      case RADIO_TECH_1xRTT: [[fallthrough]];
      case RADIO_TECH_EVDO_0: [[fallthrough]];
      case RADIO_TECH_EVDO_A: [[fallthrough]];
      case RADIO_TECH_EVDO_B:
          break;
      default:
          return false;
    }
    return true;
}
