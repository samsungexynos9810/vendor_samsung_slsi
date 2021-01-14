/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef SIT_RIL_EMBMS_H
#define SIT_RIL_EMBMS_H

#include "EwLteConnector.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SITRil eMbms Errors */
enum {
    SITRIL_EMBMS_ERROR_NONE,
    SITRIL_EMBMS_ERROR_FAILURE,
    SITRIL_EMBMS_ERROR_NOT_OPENED_LIB,
    SITRIL_EMBMS_ERROR_LIB_LOAD_FAIL,
    SITRIL_EMBMS_ERROR_NO_SYMBOL,
    SITRIL_EMBMS_ERROR_OPEN_FAIL,
    SITRIL_EMBMS_ERROR_SEND_FAIL,
    SITRIL_EMBMS_ERROR_INVALID_SOCKET_ID,
    SITRIL_EMBMS_ERROR_INVALID_PARAM,
    SITRIL_EMBMS_ERROR_REGISTRATION_FAIL,
    SITRIL_EMBMS_ERROR_ALREADY_LIB_LOADED,
    SITRIL_EMBMS_ERROR_ALREADY_REGISTERD,
    SITRIL_EMBMS_ERROR_TIMEOUT,
    SITRIL_EMBMS_ERROR_MAX
} SitRileMbmsError;

typedef enum
{
    DISABLE_SERVICE = 0x00,
    ENABLE_SERVICE = 0x01,
} SiteMbmsServiceState;


#define MAX_SAILIST_NUM     (64)
#define MAX_FREQLIST_NUM    (8)
#define MAX_IPADDR_LEN      (41)


typedef struct {
    uint32_t uSAICount;
    EwLTECON_SAI_t pSAIList[MAX_SAILIST_NUM];
    uint32_t uFreqCount;
    EwLTECON_EARFCN_t pFreqList[MAX_FREQLIST_NUM];
} __attribute__((packed)) RIL_EMBMS_InfoBinding_t;

typedef struct {
    uint32_t uSetActive;
    uint8_t uPriority;
    EwLTECON_TMGI_t uTMGI;
    RIL_EMBMS_InfoBinding_t pInfoBind;      // MW does not define a MAX value. It depends only on SA content.
                                            // However generally, there is only one infobinding element in SA.
    uint32_t uInfoBindCount;
} __attribute__((packed)) RIL_EMBMS_AvailablityInfo_t;

#define MAX_TMGI_NUM        (64)
typedef struct {
    int state;
    int count;
    uint64_t tmgi[MAX_TMGI_NUM];
} RIL_EMBMS_SessionListInfo_t;

#define MAX_SIGNALINFO_NUM  (8)
typedef struct {
    EwLTECON_SignalInformationType_t type;
    uint32_t value;
} RIL_EMBMS_SignalInfo_t;

typedef struct {
    uint32_t count;
    RIL_EMBMS_SignalInfo_t info[MAX_SIGNALINFO_NUM];
} RIL_SignalInfoList;

typedef struct {
    uint32_t mcc;
    uint32_t mnc;
    uint32_t cellId;
} RIL_EMBMS_NetworkInformation_t;

// SAIL List
#define MAX_INTER_SAI_NUM        (64)
#define MAX_MULTI_BAND_NUM       (8)
#define MAX_INTRA_SAILIST_NUM    (64)
#define MAX_INTER_SAILIST_NUM    (8)

typedef struct
{
    int Frequency;
    uint8_t InterSaiNumber; // max 64
    uint8_t MultiBandInfoNumber; // max 8
    uint16_t InterSaiInfo[MAX_INTER_SAI_NUM];
    uint8_t MultiBandInfo[MAX_MULTI_BAND_NUM];
} RIL_EMBMS_InterSaiList;

typedef struct
{
    int IntraSaiListNum; // max 64
    int InterSaiListNum; // max 8
    uint16_t IntraSaiList[MAX_INTRA_SAILIST_NUM];
    RIL_EMBMS_InterSaiList InterSaiList[MAX_INTER_SAILIST_NUM];
} RIL_EMBMS_SaiList;

typedef struct {
    EwLTECON_ControlType_t type;
    EwLTECON_Status_t status;
    EwLTECON_TMGI_t tmgi;
} RIL_EMBMS_SessionControl;

#define HANDLE_MAX      (1)
#define SOCKET_ID       (0)

/* Callbacks */
typedef void(*SITRileMbmsOnResponse)(unsigned int msgId, int status, void* data, size_t length, unsigned int channel);
typedef int(*SITRileMbmsUnsolCallback)(unsigned int msgId, void* data, size_t length, unsigned int socket_id);

struct EMBMS_CONTEXT{
    int mClient;
    EwLTECON_ModemStatus_t mModemStatus;
    EwLTECON_Configuration_t *mConfig;
    EwLTECON_SignalInformation_t mSignalInfo;
    int mSocketId;
};

#define SLSI_EMBMS_COVERAGE_STATE(status) \
        (status == EWLTECON_NO_COVERAGE ?            "EWLTECON_NO_COVERAGE" : \
         status == EWLTECON_UNICAST_COVERAGE ?       "EWLTECON_UNICAST_COVERAGE" : \
         status == EWLTECON_FULL_COVERAGE ?          "EWLTECON_FULL_COVERAGE" : \
                                                     "EWLTECON-UNKNOW-COVERAGE-STATE")
#define SLSI_EMBMS_MODEM_STATUS(status) \
        (status == EWLTECON_MS_READY ?               "EWLTECON_MS_READY" : \
         status == EWLTECON_MS_EMBMS_ENABLE ?        "EWLTECON_MS_EMBMS_ENABLE" : \
         status == EWLTECON_MS_EMBMS_DISABLE ?       "EWLTECON_MS_EMBMS_DISABLE" : \
         status == EWLTECON_MS_DEVICE_OFF ?          "EWLTECON_MS_DEVICE_OFF" : \
                                                     "EWLTECON-UNKNOW-MS-STATUS")
#define SLSI_EMBMS_TMGI_STATUS(status) \
        (status == EWLTECON_TMGI_ACTIVATED ?         "EWLTECON_TMGI_ACTIVATED" : \
         status == EWLTECON_TMGI_AVAILABLE ?         "EWLTECON_TMGI_AVAILABLE" : \
         status == EWLTECON_TMGI_OOS ?               "EWLTECON_TMGI_OOS" : \
                                                     "EWLTECON-UNKNOW-TMGI-STATUS")


int Open();
void Close();
int eMbmsModemStatusNoti(void *data); // pModemStatusCB
int eMbmsCoverageNoti(void *data); // pCoverageCB
int eMbmsNetworkInformationNoti(void *data);
int eMbmsServiceAreaListUpdatedNoti(void *data);
int eMbmsSessionListUpdatedNoti(void *data);
int eMbmsSessionControlNoti(void *data);
int eMbmsSignalInformationNoti(void *data);
int eMbmsTimeNoti(void *data);   // pTimeCB

int eMbmsRadioStateChanged(void *data);

#ifdef __cplusplus
};
#endif

#endif  /* SIT_RIL_EMBMS_H */
