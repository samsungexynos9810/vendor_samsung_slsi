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
 * protocolmiscbuilder.cpp
 *
 *  Created on: 2014. 6. 30.
 *      Author: m.afzal
 */

#include "protocolmiscbuilder.h"
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

ModemData *ProtocolMiscBuilder::GetBaseBandVersion(BYTE mask/* = 0xFF*/)
{
    sit_misc_get_baseband_version_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_BASEBAND_VERSION, length);
    req.ver_mask = mask;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetSignalStrength()
{
    sit_misc_get_signal_strength_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_SIGNAL_STRENGTH, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetTtyMode()
{
    sit_misc_get_tty_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_TTY_MODE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::SetTtyMode(int mode)
{
    sit_misc_set_tty_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_TTY_MODE, length);
    req.tty_mode = mode;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::SetScreenState(int state)
{
    sit_misc_set_screen_state_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_SCREEN_STATE, length);
    req.screen_state = state;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetIMEI()
{
    sit_id_get_imei_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_IMEI, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetIMEISV()
{
    sit_id_get_imeisv_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_IMEISV, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetDevID()
{
    sit_id_get_deviceid_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_DEVICE_ID, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetEngMode(BYTE mode)
{
    sit_misc_set_eng_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_ENG_MODE, length);
    req.eng_mode = mode;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetEngMode(BYTE mode, BYTE sub_mode)
{
    sit_misc_set_eng_mode_ex_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_ENG_MODE, length);
    req.eng_mode = mode;
    req.sub_mode = sub_mode;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetScrLine(BYTE lineno)
{
    sit_misc_set_scr_line_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_SCREEN_LINE, length);
    req.scr_line= lineno;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::SetDebugTrace(BYTE value)
{
    sit_misc_set_debug_trace_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_DEBUG_TRACE, length);
    req.debug_trace= value;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::SetEngStringInput(BYTE len, char* input)
{
    sit_misc_set_eng_string_input_req req;
    int length = sizeof(req.hdr) + len + 1; // len+data
    InitRequestHeader(&req.hdr, SIT_SET_ENG_STRING_INPUT, length);

    req.len = len;
    memset(req.input, 0, sizeof(req.input));
    memcpy(req.input, input, len);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetMslCode()
{
    sit_misc_get_msl_code_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_MSL_CODE, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::SetPinControl(BYTE signal, BYTE status)
{
    sit_misc_set_pin_control_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_PIN_CONTROL, length);
    req.signal = signal;
    req.status = status;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::GetManualBandMode()
{
    sit_misc_get_manual_band_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_MANUAL_BAND_MODE, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::SetManualBandMode(void *data, unsigned int datalen)
{
    char buf[2048] = {0, };
    int length = 0;
    int headerlen = sizeof(RCM_HEADER);
    length = headerlen + datalen;
    InitRequestHeader((RCM_HEADER *)buf, SIT_SET_MANUAL_BAND_MODE, length);

    if (data != NULL && datalen > 0) {
        memcpy(buf + headerlen, data, datalen);
    }
    return new ModemData(buf, length);
}

ModemData *ProtocolMiscBuilder::GetRfDesenseMode()
{
    sit_misc_get_rf_desense_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_RF_DESENSE_MODE, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::SetRfDesenseMode(void *data, unsigned int datalen)
{
    char buf[2048] = {0, };
    int length = 0;
    int headerlen = sizeof(RCM_HEADER);
    length = headerlen + datalen;
    InitRequestHeader((RCM_HEADER *)buf, SIT_SET_RF_DESENSE_MODE, length);

    if (data != NULL && datalen > 0) {
        memcpy(buf + headerlen, data, datalen);
    }
    return new ModemData(buf, length);
}

ModemData *ProtocolMiscBuilder::StoreAdbSerialNumber(void *data, unsigned int datalen)
{
    sit_oem_store_adb_serial_number_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_STORE_ADB_SERIAL_NUMBER_REQ, length);
    memset(req.adbSerialNumber, 0, sizeof(req.adbSerialNumber));
    strncpy(req.adbSerialNumber, (char *)data, MAX_ADB_SERIAL_NUMBER-1);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::ReadAdbSerialNumber()
{
    sit_oem_read_adb_serial_number_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_READ_ADB_SERIAL_NUMBER_REQ, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildDtmf(int dtmf_length, char* dtmf)
{
    sit_call_dtmf_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_DTMF, length);

    data.dtmf_len = dtmf_length>=MAX_DTMF_LEN?MAX_DTMF_LEN:dtmf_length;
    memcpy(data.dtmf_digit,dtmf, data.dtmf_len);

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolMiscBuilder::BuildDtmfStart(bool tone_type, BYTE digit)
{
    sit_call_dtmf_start_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_DTMF_START, length);

    data.tone_type = tone_type==true?SIT_CALL_LOCAL_DTMF_ON:SIT_CALL_LOCAL_DTMF_OFF;
    data.tone_len = SIT_CALL_DTMF_TONE_LENGTH_SHORT;
    data.dtmf_digit= digit;

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolMiscBuilder::BuildDtmfStop()
{
    sit_call_dtmf_stop_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_DTMF_STOP, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolMiscBuilder::BuildNvReadItem(int nvItemId)
{
#if 0   //removed
    sit_misc_get_oem_nv_item_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_OEM_NV_ITEM, length);

    req.nv_index = (unsigned int)nvItemId;

    return new ModemData((char *)&req, length);
#else
    return NULL;
#endif
}

ModemData *ProtocolMiscBuilder::BuildNvWriteItem(int nvItemId, const char *value)
{
#if 0   //removed
    sit_misc_set_oem_nv_item_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_OEM_NV_ITEM, length);

    req.nv_index = (unsigned int)nvItemId;
    memset(req.data, 0, sizeof(req.data));
    if (value != NULL) {
        int size = strlen(value);
        if (size > MAX_NV_ITEM_SIZE) {
            size = MAX_NV_ITEM_SIZE;
        }

        memcpy(req.data, value, size);
    }

    return new ModemData((char *)&req, length);
#else
    return NULL;
#endif
}

ModemData *ProtocolMiscBuilder::GetModemActivityInfo()
{
    sit_misc_get_activity_info_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_ACTIVITY_INFO, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetOpenCarierInfo(unsigned int opc, const char *plmn)
{
    if (plmn == NULL || *plmn == 0) {
        return NULL;
    }

    int len = strlen(plmn);
    if (!(len == 5 || len == 6)) {
        return NULL;
    }

    sit_misc_set_operator_info_req req;
    int length = sizeof(sit_misc_set_operator_info_req);

    memset(&req, 0, sizeof(sit_misc_set_operator_info_req));
    InitRequestHeader(&req.hdr, SIT_SET_OPERATOR_INFO, length);
    req.plmn[5] = '#';
    memcpy(req.plmn, plmn, len);
    req.openCarrierIndex = opc;

    return new ModemData((char *)&req, length);
}

#ifdef SUPPORT_CDMA
ModemData *ProtocolMiscBuilder::BuildCdmaSubscription()
{
    sit_misc_get_cdma_subscription_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_CDMA_SUBSCRIPTION, length);
    return new ModemData((char *)&req, length);
}
#endif // SUPPORT_CDMA

ModemData * ProtocolMiscBuilder::BuildSetVoiceOperation(int mode)
{
    sit_misc_set_voice_operation_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_VOICE_OPERATION, length);
    req.mode = (mode == 0) ? 0 : 3;     // for backward compatibility, use 0x03 for enabled
    return new ModemData((char *)&req, length);
}

ModemData * ProtocolMiscBuilder::BuildSetPreferredCallCapability(int mode)
{
    sit_misc_set_preferred_call_capability_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_PREFERRED_CALL_CAPABILITY, length);
    req.mode = mode;
    return new ModemData((char *)&req, length);
}

ModemData * ProtocolMiscBuilder::BuildGetPreferredCallCapability()
{
    sit_misc_get_preferred_call_capability_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_PREFERRED_CALL_CAPABILITY, length);
    return new ModemData((char *)&req, length);
}

UINT32 MappingSGCValue(int targetOp)
{
    // index 0 : OEM RIL ID
    // index 1 : SIT message ID
    static const int sgcMap[][2] = {
        // Global SGC
        { TARGET_OPER_CHNOPEN, SIT_ID_SGC_SP_CHNOPEN },
        { TARGET_OPER_CHNOPEN_GCF, SIT_ID_SGC_SP_CHNOPEN_GCF },
        { TARGET_OPER_CMCC, SIT_ID_SGC_SP_CMCC },
        { TARGET_OPER_CTC, SIT_ID_SGC_SP_CTC },
        { TARGET_OPER_CU, SIT_ID_SGC_SP_CUCC },
        { TARGET_OPER_ATT, SIT_ID_SGC_SP_ATT },
        { TARGET_OPER_TMO, SIT_ID_SGC_SP_TMOUSA },
        { TARGET_OPER_VZW, SIT_ID_SGC_SP_VZW },
        { TARGET_OPER_SPR, SIT_ID_SGC_SP_SPR },
        { TARGET_OPER_LATIN, SIT_ID_SGC_SP_LATIN },
        { TARGET_OPER_LATIN_GCF, SIT_ID_SGC_SP_LATIN_GCF },
        { TARGET_OPER_EUROPEN, SIT_ID_SGC_SP_EUROPEN },
        { TARGET_OPER_EUROPEN_GCF, SIT_ID_SGC_SP_GCF },
        { TARGET_OPER_NTT, SIT_ID_SGC_SP_NTT },
        { TARGET_OPER_KDDI, SIT_ID_SGC_SP_KDDI },

        // Latam specific
        { TARGET_OPER_CLARO_AR, SIT_ID_SGC_CLARO_AR },
        { TARGET_OPER_MOV_AR, SIT_ID_SGC_MOV_AR },
        { TARGET_OPER_TUENTI_AR, SIT_ID_SGC_TUENTI_AR },
        { TARGET_OPER_NII_AR, SIT_ID_SGC_NII_AR },
        { TARGET_OPER_NUESTRO_AR, SIT_ID_SGC_NUESTRO_AR },
        { TARGET_OPER_PERSONAL_AR, SIT_ID_SGC_PERSONAL_AR },
        { TARGET_OPER_TIGO_BO, SIT_ID_SGC_TIGO_BO },
        { TARGET_OPER_VIVA_BO, SIT_ID_SGC_VIVA_BO },
        { TARGET_OPER_CLARO_BR, SIT_ID_SGC_CLARO_BR },
        { TARGET_OPER_VIVO_BR, SIT_ID_SGC_VIVO_BR },
        { TARGET_OPER_NII_BR, SIT_ID_SGC_NII_BR },
        { TARGET_OPER_OI_BR, SIT_ID_SGC_OI_BR },
        { TARGET_OPER_PORTO_CONECTA_BR, SIT_ID_SGC_PORTO_CONECTA_BR },
        { TARGET_OPER_SURF_BR, SIT_ID_SGC_SURF_BR },
        { TARGET_OPER_TIM_BR, SIT_ID_SGC_TIM_BR },
        { TARGET_OPER_CLARO_CL, SIT_ID_SGC_CLARO_CL },
        { TARGET_OPER_MOV_CL, SIT_ID_SGC_MOV_CL },
        { TARGET_OPER_ENTEL_CL, SIT_ID_SGC_ENTEL_CL },
        { TARGET_OPER_CLARO_BR, SIT_ID_SGC_WOM_CL },
        { TARGET_OPER_CLARO_CO, SIT_ID_SGC_CLARO_CO },
        { TARGET_OPER_MOV_CO, SIT_ID_SGC_MOV_CO },
        { TARGET_OPER_AVANTEL_CO, SIT_ID_SGC_AVANTEL_CO },
        { TARGET_OPER_ETB_CO, SIT_ID_SGC_ETB_CO },
        { TARGET_OPER_TIGO_CO, SIT_ID_SGC_TIGO_CO },
        { TARGET_OPER_CLARO_CR, SIT_ID_SGC_CLARO_CR },
        { TARGET_OPER_CLARO_DO, SIT_ID_SGC_CLARO_DO },
        { TARGET_OPER_CLARO_EC, SIT_ID_SGC_CLARO_EC },
        { TARGET_OPER_MOV_EC, SIT_ID_SGC_MOV_EC },
        { TARGET_OPER_CNT_EC, SIT_ID_SGC_CNT_EC },
        { TARGET_OPER_CLARO_SV, SIT_ID_SGC_CLARO_SV },
        { TARGET_OPER_TIGO_SV, SIT_ID_SGC_TIGO_SV },
        { TARGET_OPER_CLARO_GT, SIT_ID_SGC_CLARO_GT },
        { TARGET_OPER_TIGO_GT, SIT_ID_SGC_TIGO_GT },
        { TARGET_OPER_CLARO_HN, SIT_ID_SGC_CLARO_HN },
        { TARGET_OPER_TIGO_HO, SIT_ID_SGC_TIGO_HO },
        { TARGET_OPER_TELCEL_MX, SIT_ID_SGC_TELCEL_MX },
        { TARGET_OPER_MOV_MX, SIT_ID_SGC_MOV_MX },
        { TARGET_OPER_ALTAN_MX, SIT_ID_SGC_ALTAN_MX },
        { TARGET_OPER_ATT_MX, SIT_ID_SGC_ATT_MX },
        { TARGET_OPER_CLARO_NI, SIT_ID_SGC_CLARO_NI },
        { TARGET_OPER_CLARO_PA, SIT_ID_SGC_CLARO_PA },
        { TARGET_OPER_CLARO_PY, SIT_ID_SGC_CLARO_PY },
        { TARGET_OPER_PERSONAL_PY, SIT_ID_SGC_PERSONAL_PY },
        { TARGET_OPER_TIGO_PY, SIT_ID_SGC_TIGO_PY },
        { TARGET_OPER_CLARO_PE, SIT_ID_SGC_CLARO_PE },
        { TARGET_OPER_MOV_PE, SIT_ID_SGC_MOV_PE },
        { TARGET_OPER_ENTEL_PE, SIT_ID_SGC_ENTEL_PE },
        { TARGET_OPER_CLARO_PR, SIT_ID_SGC_CLARO_PR },
        { TARGET_OPER_OPEN_MOBILE_PR, SIT_ID_SGC_OPEN_MOBILE_PR },
        { TARGET_OPER_CLARO_UY, SIT_ID_SGC_CLARO_UY },
        { TARGET_OPER_MOV_UY, SIT_ID_SGC_MOV_UY },
        { TARGET_OPER_ANTEL_UY, SIT_ID_SGC_ANTEL_UY },
        { TARGET_OPER_MOV_UZ, SIT_ID_SGC_MOV_UZ },
        { TARGET_OPER_BELL, SIT_ID_SGC_SP_BMC },
        { TARGET_OPER_TELUS, SIT_ID_SGC_SP_TELUS },
        { TARGET_OPER_ROGERS, SIT_ID_SGC_SP_ROGERS },
        { TARGET_OPER_FREEDOM, SIT_ID_SGC_SP_FREEDOM },
    };

    if (targetOp < 0) {
        return SIT_ID_SGC_SP_CHNOPEN;
    }

    UINT32 sgcId = SIT_ID_SGC_SP_CHNOPEN; // default
    int size = sizeof(sgcMap) / sizeof(sgcMap[0]);
    for (int i = 0; i < size; i++) {
        if (sgcMap[i][0] == targetOp) {
            sgcId = sgcMap[i][1];
            break;
        }
    } // end for i ~

    return sgcId;
}

ModemData *ProtocolMiscBuilder::SendSGCValue(const int TargetOp, const int Rsv1, const int Rsv2)
{
    sit_id_set_sgc_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_SGC, length);
    req.SGC = MappingSGCValue(TargetOp);
    req.Rsv1 = Rsv1;
    req.Rsv2 = Rsv2;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::SendDeviceInfo(const char* model, const char* swVer, const char* productName)
{
    sit_misc_set_device_info_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_SW_VERSION, length);

    memset(req.model_name, 0x00, sizeof(req.model_name));
    memset(req.sw_version, 0x00, sizeof(req.sw_version));
    memset(req.product_name, 0x00, sizeof(req.product_name));

    strncpy((char*)req.model_name, model, sizeof(req.model_name)-1);
    strncpy((char*)req.sw_version, swVer, sizeof(req.sw_version)-1);
    strncpy((char*)req.product_name, productName, sizeof(req.product_name)-1);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildGetHwConfig()
{
    sit_misc_get_hw_config_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_HW_CONFIG, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildLceStart(int lceMode, int interval)
{
    sit_pdp_start_lce_info_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_START_LCE_INFO, length);

    req.mode = lceMode;
    req.interval = interval;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildLceStop()
{
    sit_pdp_stop_lce_info_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_STOP_LCE_INFO, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildLcePullLceData()
{
    sit_pdp_get_lce_data_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_LCE_DATA, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetUnsolicitedResponseFilter(unsigned int bitMask)
{
    sit_misc_set_ind_cmd_filter_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_IND_CMD_FILTER, length);
    req.ind_cmd_filter = bitMask;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetCarrierInfoImsiEncryption(char *pMcc, char *pMnc, int keyLen, BYTE *pKey, int keyIdLen, char *pKeyId, LONG expTime)
{
    sit_id_set_carrier_info_imsi_encription_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_CARRIER_INFO_IMSI_ENCRYPTION, length);

    if (pMcc != NULL) memcpy(req.mcc, pMcc, MAX_MCC_LEN);
    if (pMnc != NULL) memcpy(req.mnc, pMnc, MAX_MNC_LEN);
    if (keyLen > 0 && pKey != NULL) {
        req.carrier_len = (keyLen > MAX_IMSI_ENCRIPTION_KEY_LEN) ? MAX_IMSI_ENCRIPTION_KEY_LEN:keyLen;
        memcpy(req.carrier_key, pKey, req.carrier_len);
    }
    if (keyIdLen > 0 && pKeyId != NULL) {
        req.key_id_len = (keyIdLen > MAX_IMSI_ENCRIPTION_KEY_LEN) ? MAX_IMSI_ENCRIPTION_KEY_LEN:keyIdLen;;
        memcpy(req.key_id, pKeyId, req.key_id_len);
    }
    req.expire_time = expTime;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetSuppSvcNotification(int enable)
{
    RilLogI("%s need to implement", __FUNCTION__);
    return NULL;
}

ModemData *ProtocolMiscBuilder::BuildPSensorStatus(int pSensorStatus)
{
    sit_misc_psensor_set_psensor_status_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_SENSOR_STATUS, length);
    req.psensor_status= pSensorStatus;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetSarState(int sarState)
{
    sit_misc_sar_set_sar_state_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_SET_SAR_STATE, length);
    req.sar_status = sarState;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildGetSarState()
{
    sit_misc_sar_get_sar_state_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_GET_SAR_STATE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildScanRssi(int rat, int band, int rbw, int scanMode, int startFreq, int endFreq, int step, int antenna, int sampling,
                                                int tx1, int tx1Band, int tx1Bw, int tx1Freq, int tx1Power, int tx1RbNum, int tx1RbOffset, int tx1Mcs,
                                                int tx2, int tx2Band, int tx2Bw, int tx2Freq, int tx2Power, int tx2RbNum, int tx2RbOffset, int tx2Mcs)
{
    sit_misc_set_rssi_scan_req req;
    memset(&req, 0, sizeof(req));
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_RSSI_SCAN, length);
    req.rat = (BYTE)(rat & 0xFF);
    req.band = (BYTE)(band & 0xFF);
    req.rbw = (BYTE)(rbw & 0xFF);
    req.scan_mode = (BYTE)(scanMode & 0xFF);
    if (scanMode == SCAN_PARTIAL) {
        req.start_frequency = (UINT16)(startFreq & 0xFFFF);
        req.end_frequency = (UINT16)(endFreq & 0xFFFF);
    }
    req.step = (BYTE)(step & 0xFF);
    req.antenna_selection = (BYTE)(antenna & 0xFF);
    req.sampling_count = (UINT16)(sampling & 0xFFFF);
    req.tx1 = (BYTE)(tx1 & 0xFF);
    req.tx1_band = (BYTE)(tx1Band & 0xFF);
    req.tx1_bw = (BYTE)(tx1Bw & 0xFF);
    req.tx1_freq = (UINT16)(tx1Freq & 0xFFFF);
    req.tx1_power = (UINT16)(tx1Power & 0xFFFF);
    req.tx1_rb_num = (BYTE)(tx1RbNum & 0xFF);
    req.tx1_rb_offset = (BYTE)(tx1RbOffset & 0xFF);
    req.tx1_mcs = (BYTE)(tx1Mcs & 0xFF);
    if (rat == SCAN_RAT_LTE || rat == SCAN_RAT_LTE_CA) {
        req.tx2 = (BYTE)(tx2 & 0xFF);
        req.tx2_band = (BYTE)(tx2Band & 0xFF);
        req.tx2_bw = (BYTE)(tx2Bw & 0xFF);
        req.tx2_freq = (UINT16)(tx2Freq & 0xFFFF);
        req.tx2_power = (UINT16)(tx2Power & 0xFFFF);
        req.tx2_rb_num = (BYTE)(tx2RbNum & 0xFF);
        req.tx2_rb_offset = (BYTE)(tx2RbOffset & 0xFF);
        req.tx2_mcs = (BYTE)(tx2Mcs & 0xFF);
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildATCommand(const char *command)
{
    if (TextUtils::IsEmpty(command))  {
        return NULL;
    }

    sit_misc_forwarding_at_command_req req;
    memset(&req, 0, sizeof(req));
    unsigned int commandLength = strlen(command);
    if (commandLength > MAX_SIT_AT_COMMAND_LENGTH) {
        commandLength = MAX_SIT_AT_COMMAND_LENGTH;
    }
    unsigned int length = sizeof(RCM_HEADER) + sizeof(req.length) + commandLength;
    InitRequestHeader(&req.hdr, SIT_SET_FORWARDING_AT_COMMAND, length);
    req.length = commandLength & 0xFFFF;
    memcpy(req.data, command, commandLength);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildGetRadioNode(const char *path)
{
    if (TextUtils::IsEmpty(path))  {
        return NULL;
    }

    if (strlen(path) >= MAX_RADIO_NODE_DATA_LEN) {
        return NULL;
    }

    sit_misc_get_radio_node_req req;
    int length = sizeof(sit_misc_get_radio_node_req);
    memset(&req, 0, sizeof(sit_misc_get_radio_node_req));
    InitRequestHeader(&req.hdr, SIT_GET_RADIO_NODE, length);
    strncpy(req.path, path, MAX_RADIO_NODE_DATA_LEN-1);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetRadioNode(const char *path, const char *value)
{
    if (TextUtils::IsEmpty(path) || TextUtils::IsEmpty(value))  {
        return NULL;
    }

    if (strlen(path) >= MAX_RADIO_NODE_DATA_LEN || strlen(value) >= MAX_RADIO_NODE_DATA_LEN) {
        return NULL;
    }

    sit_misc_set_radio_node_req req;
    int length = sizeof(sit_misc_set_radio_node_req);
    memset(&req, 0, sizeof(sit_misc_set_radio_node_req));
    InitRequestHeader(&req.hdr, SIT_SET_RADIO_NODE, length);
    strncpy(req.path, path, MAX_RADIO_NODE_DATA_LEN-1);
    strncpy(req.value, value, MAX_RADIO_NODE_DATA_LEN-1);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildGetVoLteProvisionUpdate()
{
    sit_misc_get_volte_provision_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_VOLTE_PROVISION_UPDATE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetVoLteProvisionUpdate()
{
    sit_misc_set_volte_provision_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_VOLTE_PROVISION_UPDATE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildRadioConfigReset(int type)
{
    sit_misc_set_cfg_default_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_CFG_DEFAULT, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetStatckStatus(int mode)
{
    sit_pwr_set_stack_status_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_STACK_STATUS, length);
    req.mode = (BYTE) mode;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildGetStatckStatus()
{
    sit_pwr_get_stack_status_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_STACK_STATUS, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetModemsConfig(int numOfLiveModems)
{
    sit_misc_set_modems_config_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_MODEM_CONFIG, length);
    // single(0) or multi(1) even if more than 2
    req.config = (numOfLiveModems == 1) ? 0 : 1;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildModemInfo(int type, char *data, unsigned int size)
{
    if (data == NULL && size > 0) {
        size = 0;
    }

    int length = sizeof(sit_oem_nw_info_req) + size;
    char *buf = new char[length];
    ModemData *ret = NULL;
    if (buf != NULL) {
        memset(buf, 0, length);
        sit_oem_nw_info_req *req = (sit_oem_nw_info_req *)buf;
        InitRequestHeader(&req->hdr, SIT_OEM_NW_INFO, length);
        req->command = type;
        if (size > 0) {
            req->length = size;
            memcpy(req->data, data, size);
        }
        ret = new ModemData(buf, length);
        delete[] buf;
    }
    return ret;
}

ModemData *ProtocolMiscBuilder::BuildSetRtplossThr(BYTE interval, BYTE pktLossThr)
{
    sit_oem_set_rtp_pktloss_thr_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_SET_RTP_PKTLOSS_THRESHOLD, length);
    req.interval = interval;
    req.pktLossThr = pktLossThr;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSwitchModemFunction(int feature, BYTE enable)
{
    sit_oem_set_func_switch_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_SET_FUNC_SWITCH_REQ, length);
    req.feature = feature;
    req.enable = enable;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetPdcpDiscardTimer(int discardTimer)
{
    sit_oem_set_pdcp_discard_timer_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_SET_PDCP_DISCARD_TIMER, length);
    req.discardTimer = discardTimer;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetSelflog(int mode, int size)
{
    sit_misc_set_selflog_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_SELFLOG, length);
    req.mode = (BYTE) mode;
    req.size = (BYTE) size;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildGetSelflogStatus()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_SELFLOG_STATUS, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetActivateVsim(int slot, int iccidLen, const char *pIccid,
                                                int imsiLen, const char *pImsi, const char *pHplmn,
                                                int vsimState, int vsimCardType)
{
    if (pIccid == NULL || iccidLen < 0) {
        iccidLen = 0;
    }

    if (pImsi == NULL || imsiLen < 0) {
        imsiLen = 0;
    }

    sit_oem_set_activate_visim_req req;
    int length = sizeof(sit_oem_set_activate_visim_req);
    memset(&req, 0, sizeof(sit_oem_set_activate_visim_req));
    InitRequestHeader(&req.hdr, SIT_OEM_SET_ACTIVATE_VSIM, length);

    int cplen = 0;
    req.simSlot = slot;

    req.iccidLen = iccidLen;
    cplen = iccidLen < MAX_ICCID_STRING_LEN ? iccidLen : MAX_ICCID_STRING_LEN;
    if (pIccid != NULL) memcpy(req.iccid, pIccid, cplen);

    req.imsiLen = imsiLen;
    cplen = imsiLen < MAX_IMSI_LEN ? imsiLen : MAX_IMSI_LEN;
    if (pImsi != NULL) memcpy(req.imsi, pImsi, cplen);

    if (pHplmn != NULL) memcpy(req.hplmn, pHplmn, MAX_PLMN_LEN);
    if (req.hplmn[5] == 0) req.hplmn[5] = '#';

    req.vsimState = vsimState;
    req.vsimCardType = vsimCardType;

    return new ModemData((char *)&req, length);
}
ModemData *ProtocolMiscBuilder::BuildGetCqiInfo()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_GET_CQI_INFO, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetSarSetting(int dsi)
{
    sit_oem_set_sar_setting_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_SET_SAR_SETTING, length);
    req.dsi = dsi;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetImsTestMode(int mode)
{
    sit_pdp_set_ims_test_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_IMS_TEST_MODE, length);
    req.mode = (BYTE)(mode & 0xFF);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetGmoSwitch(int feature)
{
    sit_oem_set_gmo_switch_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_SET_GMO_SWITCH, length);
    req.feature = feature;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetTcsFci(int state, int len, char *fci)
{
    sit_oem_set_tcs_fci_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_SET_TCS_FCI_REQ, length);
    req.state = (BYTE)state;
    memset(req.fci, 0, MAX_FCI_LEN);
    memcpy(req.fci, fci, MAX_FCI_LEN);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildGetTcsFci()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_OEM_GET_TCS_FCI_INFO, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetCABandwidthFilter(int enable)
{
    sit_oem_set_ca_bw_filter_req req;
    int length = sizeof(req);
    memset(&req, 0, sizeof(req));
    InitRequestHeader(&req.hdr, SIT_OEM_SET_CA_BW_FILTER, length);
    req.enable = enable > 0 ? 1 : 0;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetModemLogDump()
{
    sit_misc_set_modem_log_dump_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_MODEM_LOG_DUMP, length);
    req.type = 0x01;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetElevatorSensor(int enable)
{
    sit_set_elevator_sensor_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_ELEVATOR_SENSOR, length);
    req.enable = enable;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetUicc(int activeStatus)
{
    sit_sim_set_uicc_sub_req req;
    int length = sizeof(sit_sim_set_uicc_sub_req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_SET_UICC_SUBSCRIPTION, length);
    req.state = activeStatus;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetLocationUpdates(int enable)
{
    sit_set_location_update_setting_req req;
    int length = sizeof(req);
    memset(&req, 0, sizeof(req));
    InitRequestHeader(&req.hdr, SIT_SET_LOCATION_UPDATE_SETTING, length);
    req.update_setting = enable > 0 ? 1 : 0;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetSignalReportCriteria(int ms, int db, int len, int *dbm, int radioAccessNet)
{
    sit_misc_set_signal_strength_report_criteria_req req;
    int length = sizeof(req);
    memset(&req, 0, sizeof(req));
    InitRequestHeader(&req.hdr, SIT_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA, length);

    req.ms = ms;
    req.db = db;
    req.len = (BYTE)len;
    if (len > 0 && dbm != NULL) {
        for(int i = 0; i < len; i++)
            req.dbm[i] = dbm[i];
    }
    req.radio_acc_net = radioAccessNet;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetLinkCapaReportCriteria(int hMs, int hDlKbps, int hUlKbps, int tDlLen, int *tDlKbps, int tUlLen, int *tUlKbps, int radioAccessNet)
{
    sit_misc_set_link_capacity_report_criteria_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_LINK_CAPACITY_REPORTING_CRITERIA, length);

    int i = 0;
    req.h_ms = hMs;
    req.h_dl_kbps = hDlKbps;
    req.h_ul_kbps = hUlKbps;
    req.t_dl_len = (BYTE)tDlLen;
    if (tDlLen > 0) {
        for(i = 0; i < tDlLen; i++)
            req.t_dl_kbps[i] = tDlKbps[i];
    }
    req.t_ul_len = (BYTE)tUlLen;
    if (tUlLen > 0) {
        for(i = 0; i < tUlLen; i++)
            req.t_ul_kbps[i] = tUlKbps[i];
    }
    req.radio_acc_net = radioAccessNet;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolMiscBuilder::BuildSetSelflogProfile()
{
    std::string profile("/data/vendor/rild/profile.hex");
    std::ifstream ifs(profile.c_str());
    if (!ifs) {
        RilLogE("Failed to read %s", profile.c_str());
        return NULL;
    }
    ifs.seekg(0, ios::end);
    size_t profileLen = ifs.tellg();
    char *data = new char[profileLen];
    if (data == NULL) {
        RilLogE("Failed to alloc");
        return NULL;
    }
    ifs.seekg(0, ios::beg);
    ifs.read(data, profileLen);
    ifs.close();

    size_t length = sizeof(RCM_HEADER) + profileLen;
    char *buff = new char[length];
    if (buff == NULL) {
        RilLogE("Failed to alloc");
        delete[] data;
        return NULL;
    }
    sit_set_selflog_profile_req * req = (sit_set_selflog_profile_req *)buff;
    memset(req, 0, length);
    InitRequestHeader(&req->hdr, SIT_SET_SELFLOG_PROFILE, length);
    memcpy(req->data, data, profileLen);
    ModemData *pModemData = new ModemData((char *)req, length);

    delete[] data;
    delete [] buff;

    return pModemData;
}

ModemData *ProtocolMiscBuilder::BuildSetForbidLteCell(int mode, int cellId, int forbiddenTimer, char *plmn)
{
    sit_oem_set_forbid_lte_cell_req req;
    int length = sizeof(sit_oem_set_forbid_lte_cell_req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_OEM_SET_FORBID_LTE_CELL, length);
    req.mode = mode;
    req.cellid = cellId;
    req.forbidden_timer = forbiddenTimer;
    if (sizeof(plmn) < MAX_PLMN_LEN - 1) {
        return NULL;
    }
    req.plmn[5] == '#';
    memcpy(req.plmn, plmn, MAX_PLMN_LEN);
    return new ModemData((char *)&req, length);
}
