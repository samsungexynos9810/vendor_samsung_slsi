/* //device/libs/telephony/ril_unsol_commands.h
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
    {RIL_UNSOL_SIGNAL_STRENGTH, radio::currentSignalStrengthInd_1_4, DONT_WAKE},
    {ENCODE_REQUEST(RIL_UNSOL_DATA_CALL_LIST_CHANGED, HAL_VERSION_CODE(1,4)), radio::dataCallListChangedInd_1_4, DONT_WAKE},
    {RIL_UNSOL_CELL_INFO_LIST, radio::cellInfoListInd_1_4, DONT_WAKE},
    {RIL_UNSOL_NETWORK_SCAN_RESULT, radio::networkScanResultInd_1_4, DONT_WAKE},
    {RIL_UNSOL_PHYSICAL_CHANNEL_CONFIG, radio::currentPhysicalChannelConfigs_1_4, DONT_WAKE},
    {RIL_UNSOL_EMERGENCY_NUMBER_LIST, radio::currentEmergencyNumberListInd, DONT_WAKE},
