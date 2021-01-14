/*
 * Copyright (c) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RIL_RADIOCONFIG_H
#define RIL_RADIOCONFIG_H

#include <slsi/radioconfig_v1_0.h>
#include <slsi/radioconfig_v1_1.h>
#include <slsi/radioconfig_v1_2.h>
#include <ril_service.h>

namespace radioconfig {
/* 1.0 RIL RadioConfig */
void registerService(RIL_RadioFunctions *callbacks, android::CommandInfo *commands);
int getSimSlotsStatusResponse(int slotId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
int setSimSlotsMappingResponse(int slotId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
int simSlotsStatusChanged(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responseLen);
pthread_rwlock_t * getRadioConfigServiceRwlock();
int sendRequestRawResponse(int rilcMsgId, int slotId, int serial, RIL_Errno e,
        void *response, size_t responseLen);
int sendRequestRawResponseSeg(int rilcMsgId, int slotId, int serial, RIL_Errno e,
        void *response, size_t responseLen);
void rilRadioConfigRawIndication(int rilcMsgId, int slotId, const void *indication, size_t indicationLen);
/* 1.1 RIL RadioConfig */
int getPhoneCapabilityResponse(int slotId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
int setPreferredDataModemResponse(int slotId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
int setModemsConfigResponse(int slotId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
int getModemsConfigResponse(int slotId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
/* 1.2 RIL RadioConfig */
int getSimSlotsStatusResponse_1_2(int slotId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
int simSlotsStatusChanged_1_2(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responseLen);
} // namespace radioconfig

#endif  // RIL_RADIOCONFIG_H

