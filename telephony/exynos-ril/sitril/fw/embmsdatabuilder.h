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
 * gpsdatabuilder.h
 *
 *  Created on: 2015. 1. 23.
 *      Author: m.afzal
 */

#ifndef __EMBMS_DATA_BUILDER_H__
#define __EMBMS_DATA_BUILDER_H__

#include "rildatabuilder.h"
#include "rildef.h"


#define MAX_TMGI_NUM            (64)
#define MAX_TMGI_LEN            (6)
#define MAX_SIGNALINFO_NUM      (8)

// SAIL List
#define MAX_INTER_SAI_NUM        (64)
#define MAX_MULTI_BAND_NUM       (8)
#define MAX_INTRA_SAILIST_NUM    (64)
#define MAX_INTER_SAILIST_NUM    (8)

typedef struct {
    UINT32 state;
    UINT32 count;
    uint64_t tmgi[MAX_TMGI_NUM];
} RIL_SessionListInfo;

typedef struct {
    UINT32 type;
    UINT32 value;
} RIL_SignalInfo;

typedef struct {
    UINT32 count;
    RIL_SignalInfo Info[MAX_SIGNALINFO_NUM];
} RIL_SignalInfoList;

typedef struct {
    UINT32 mcc;
    UINT32 mnc;
    UINT32 cellid;
} RIL_GlobalCellIdType;

typedef struct {
    UINT32 type;
    int status;
    uint64_t tmgi;
} RIL_SessionControlType;

typedef struct
{
    UINT32 Frequency;
    BYTE InterSaiInfoNum; // max 64
    BYTE MultiBandInfoNum; // max 8
    WORD InterSaiInfo[MAX_INTER_SAI_NUM];
    BYTE MultiBandInfo[MAX_MULTI_BAND_NUM];
} RIL_InterSaiList;

typedef struct
{
    UINT32 IntraSaiListNum; // max 64
    UINT32 InterSaiListNum; // max 8
    UINT16 IntraSaiList[MAX_INTRA_SAILIST_NUM];
    RIL_InterSaiList InterSaiList[MAX_INTER_SAILIST_NUM];
} RIL_SaiList;

class EmbmsDataBuilder : public RilDataBuilder {
private:
    uint64_t getLongTypeTmgi(const BYTE* pTmgi);

public:
    const RilData *BuildEmbmsCoverageIndicate(int nCoverage);
    const RilData *BuildEmbmsSessionListResponse(int nState, int nOosReason, int nRecordNum, const BYTE *pTmgi);
    const RilData *BuildEmbmsSignalStrengthResponse(uint32_t count, const uint32_t *pSnrList);
    const RilData *BuildEmbmsNetworkTimeResponse(const uint64_t networkTime);
    const RilData *BuildEmbmsSaiIndicate(int nIntraNum, int InterNum, const UINT16 *pIntraSaiList, const EMBMS_InterSaiList *pInterSaiList);
    const RilData *BuildEmbmsGlobalCellIdIndicate(const char *mcc, const char *mnc, uint32_t cellId);
    const RilData *BuildEmbmsSessionControlIndicate(uint32_t controlType, int status, uint64_t uTmgi);
};

#endif /* __EMBMS_DATA_BUILDER_H__ */
