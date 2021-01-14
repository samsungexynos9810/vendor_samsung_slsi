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
 * protocolvsimbuilder.cpp
 *
 *  Created on: 2016. 02. 26.
 */

#include "protocolvsimbuilder.h"
#include "util.h"
#include "rillog.h"

static int ConvertVsimNotiRilEventIdToSitEventId(int eventid)
{
    switch(eventid)
    {
        case REQUEST_TYPE_ENABLE_EXTERNAL_SIM:
            return SIT_VSIM_NOTI_EN_EXTERNAL_SIM;
        case REQUEST_TYPE_DISABLE_EXTERNAL_SIM:
            return SIT_VSIM_NOTI_DIS_EXTERNAL_SIM;
        case REQUEST_TYPE_PLUG_OUT:
            return SIT_VSIM_NOTI_PLUG_OUT;
        case REQUEST_TYPE_PLUG_IN:
            return SIT_VSIM_NOTI_PLUG_IN;
        default:
            RilLogE("%s: invalid eventId:%d", __FUNCTION__, eventid);
            return -1;
    }
}

static int ConvertVsimOpRilEventIdToSitEventId(int eventid)
{
    switch(eventid)
    {
        case REQUEST_TYPE_ATR_EVENT:
            return SIT_VSIM_OPERATION_ATR;
        case REQUEST_TYPE_APDU_EVENT:
            return SIT_VSIM_OPERATION_APDU;
        case REQUEST_TYPE_CARD_POWER_DOWN:
            return SIT_VSIM_OPERATION_POWERDOWN;
        default:
            RilLogE("%s: invalid eventId:%d", __FUNCTION__, eventid);
            return -1;
    }
}

ModemData *ProtocolVsimBuilder::BuildVsimNotification(int tid, int eventid, int simType)
{
    sit_vsim_notification_req req;
    int length = sizeof(req);

    InitRequestHeader(&req.hdr, SIT_VSIM_NOTIFICATION, length);

    req.tid = tid;
    req.event_id = ConvertVsimNotiRilEventIdToSitEventId(eventid);
    req.sim_type = simType;

    if ( req.event_id == -1 )
    {
        return NULL;
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolVsimBuilder::BuildVsimOperation(int tid, int eventid, int result, int dataLen, const char *data)
{
    sit_vsim_opertaion_req req;
    int length = sizeof(req);

    InitRequestHeader(&req.hdr, SIT_VSIM_OPERATION, length);

    req.tid= tid;
    req.event_id = ConvertVsimOpRilEventIdToSitEventId(eventid);
    req.result = result;

    if ( req.event_id == -1 )
    {
        return NULL;
    }

    // copy data
    req.datalength = dataLen;
    memset(req.data, 0, sizeof(req.data));
    if (data != NULL) {
        if ((strlen(data) + 1) / 2 <= MAX_VSIM_DATA_LEN) {
            int data_len = 0;
            BYTE rawdata[MAX_VSIM_DATA_LEN*2] = {0, };
            data_len = ::HexString2Value(rawdata, data);
            if (0 < data_len && data_len <= MAX_VSIM_DATA_LEN) {
                memcpy(req.data, rawdata, data_len);
                req.datalength = data_len;
                RilLogE("%s: data length:%d", __FUNCTION__, data_len);
            }
            else {
                // converting error
                req.result = RCM_E_GENERIC_FAILURE;
                RilLogE("%s: failed to convert Hexadecimal string", __FUNCTION__);
            }
        }
        else {
            // APDU overflow
            req.result = RCM_E_GENERIC_FAILURE;
            RilLogE("%s: Data is lager than %d bytes", __FUNCTION__, MAX_VSIM_DATA_LEN);
        }
    }

    return new ModemData((char *)&req, length);
}
