/* vendor/samsung_slsi/exynos-ril/libril/ril_ext_unsol_commands.h
**
** Copyright 2008, The Android Open Source Project
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

/*
** extension unsolicited id
*/
    {RIL_UNSOL_SUPP_SVC_RETURN_RESULT, radio::samsungslsi::suppSvcReturnResult, WAKE_PARTIAL},
    {RIL_UNSOL_PB_READY, NULL, DONT_WAKE},
    {RIL_UNSOL_CALL_PRESENT_IND, radio::samsungslsi::callPresentInd, WAKE_PARTIAL},
    {RIL_UNSOL_WB_AMR_REPORT_IND, radio::samsungslsi::wbAmrReportInd, WAKE_PARTIAL},
    {RIL_UNSOL_VSIM_OPERATION_INDICATION, radio::samsungslsi::vsimOperationInd, WAKE_PARTIAL},
    {RIL_UNSOL_NAS_TIMER_STATUS_IND, radio::samsungslsi::nasTimerStatusInd, WAKE_PARTIAL},
    {RIL_UNSOL_EMERGENCY_ACT_INFO, radio::samsungslsi::emergencyActInd, WAKE_PARTIAL},
    {RIL_UNSOL_ICCID_INFO, radio::samsungslsi::iccIdInfoInd, WAKE_PARTIAL},
    {RIL_UNSOL_ON_USSD_WITH_DCS, radio::samsungslsi::onUssdWithDcsInd, WAKE_PARTIAL},
    {RIL_UNSOL_VOLTE_AVAILABLE_INFO, radio::samsungslsi::volteAvailableInfoInd, WAKE_PARTIAL},
    {RIL_UNSOL_EMERGENCY_SUPPORT_RAT_MODE, radio::samsungslsi::emergencySupportRatModeInd, WAKE_PARTIAL},
    {RIL_UNSOL_USSD_CANCELED, radio::samsungslsi::ussdCanceledInd, WAKE_PARTIAL},
    {RIL_UNSOL_NR_PHYSICAL_CHANNEL_CONFIGS, radio::samsungslsi::currentPhysicalChannelConfigInd, WAKE_PARTIAL},
    {RIL_UNSOL_IND_ENDC_CAPABILITY, radio::samsungslsi::endcCapabilityInd, WAKE_PARTIAL},
