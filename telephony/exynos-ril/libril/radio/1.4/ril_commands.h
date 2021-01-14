/* //device/libs/telephony/ril_commands.h
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
    {RIL_REQUEST_GET_SIM_STATUS, radio::getIccCardStatusResponse_1_4},
    {RIL_REQUEST_DATA_REGISTRATION_STATE, radio::getDataRegistrationStateResponse_1_4},
    {RIL_REQUEST_GET_CELL_INFO_LIST, radio::getCellInfoListResponse_1_4},
    {ENCODE_REQUEST(RIL_REQUEST_SIGNAL_STRENGTH, HAL_VERSION_CODE(1,4)), radio::getSignalStrengthResponse_1_4},
    {ENCODE_REQUEST(RIL_REQUEST_SETUP_DATA_CALL, HAL_VERSION_CODE(1,4)), radio::setupDataCallResponse_1_4},
    {ENCODE_REQUEST(RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, HAL_VERSION_CODE(1,4)), radio::setPreferredNetworkTypeBitmapResponse},
    {ENCODE_REQUEST(RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, HAL_VERSION_CODE(1,4)), radio::getPreferredNetworkTypeBitmapResponse},
    {ENCODE_REQUEST(RIL_REQUEST_SET_INITIAL_ATTACH_APN, HAL_VERSION_CODE(1,4)), radio::setInitialAttachApnResponse},
    {ENCODE_REQUEST(RIL_REQUEST_SET_DATA_PROFILE, HAL_VERSION_CODE(1,4)), radio::setDataProfileResponse},
    {ENCODE_REQUEST(RIL_REQUEST_SET_CARRIER_RESTRICTIONS, HAL_VERSION_CODE(1,4)), radio::setAllowedCarriersResponse_1_4},
    {ENCODE_REQUEST(RIL_REQUEST_GET_CARRIER_RESTRICTIONS, HAL_VERSION_CODE(1,4)), radio::getAllowedCarriersResponse_1_4},
    {ENCODE_REQUEST(RIL_REQUEST_START_NETWORK_SCAN, HAL_VERSION_CODE(1,4)), radio::startNetworkScanResponse_1_4},
    {RIL_REQUEST_EMERGENCY_DIAL, radio::emergencyDialResponse},
