/* //device/libs/telephony/ril_ext_commands.h
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
    {RIL_REQUEST_QUERY_COLP, radio::samsungslsi::queryCOLPResponse},
    {RIL_REQUEST_QUERY_COLR, radio::samsungslsi::queryCOLRResponse},
    {RIL_REQUEST_SIM_GET_ATR, radio::samsungslsi::iccGetAtrResponse},
    {RIL_REQUEST_SEND_ENCODED_USSD, radio::samsungslsi::sendUSSDWithDcsResponse},
    {RIL_REQUEST_SET_UPLMN, radio::samsungslsi::setPreferredUplmnResponse},
    {RIL_REQUEST_GET_UPLMN, radio::samsungslsi::getPreferredUplmnResponse},
    {RIL_REQUEST_SET_DS_NETWORK_TYPE, radio::samsungslsi::setDsNetworkTypeResponse},
    {RIL_LOCAL_REQUEST_VSIM_NOTIFICATION, radio::samsungslsi::sendVsimNotificationResponse},
    {RIL_LOCAL_REQUEST_VSIM_OPERATION, radio::samsungslsi::sendVsimOperationResponse},
    {RIL_REQUEST_SET_EMERGENCY_CALL_STATUS, radio::samsungslsi::setEmcStatusResponse},
    {RIL_REQUEST_SET_FEMTO_CELL_SRCH, radio::samsungslsi::setFemtoCellSearchResponse},
    {RIL_REQUEST_SET_CDMA_HYBRID_MODE, radio::samsungslsi::setCdmaHybridModeResponse},
    {RIL_REQUEST_GET_CDMA_HYBRID_MODE, radio::samsungslsi::getCdmaHybridModeResponse},
    {RIL_REQUEST_SET_VOICE_OPERATION, radio::samsungslsi::setVoiceOperationResponse}, // 30
    {RIL_REQUEST_SET_DUAL_NETWORK_AND_ALLOW_DATA, radio::samsungslsi::setDualNetworkTypeAndAllowDataResponse},
    {RIL_REQUEST_QUERY_BPLMN_SEARCH, radio::samsungslsi::queryBplmnSearchResponse},
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_RAT, radio::samsungslsi::setNetworkSelectionModeManualResponse},
    {RIL_REQUEST_DIAL_WITH_CALL_TYPE, radio::samsungslsi::dialWithCallTypeResponse},
    {RIL_REQUEST_CHANGE_BARRING_PASSWORD_OVER_MMI, radio::samsungslsi::setBarringPasswordOverMmiResponse},
    {RIL_REQUEST_DEACTIVATE_DATA_CALL_WITH_REASON, radio::samsungslsi::deactivateDataCallWithReasonResponse},
    {RIL_REQUEST_EMULATE_IND, radio::samsungslsi::emulateIndResponse},
    {RIL_REQUEST_GET_SIM_LOCK_STATUS, radio::samsungslsi::getSimLockStatusResponse},
    {RIL_REQUEST_SET_ACTIVATE_VSIM, radio::samsungslsi::setActivateVsimResponse},
    {RIL_REQUEST_SET_ENDC_MODE, radio::samsungslsi::setEndcModeResponse},
    {RIL_REQUEST_GET_ENDC_MODE, radio::samsungslsi::getEndcModeResponse},
    {RIL_REQUEST_GET_SMS_STORAGE_ON_SIM, radio::samsungslsi::getSmsStorageOnSimResponse},