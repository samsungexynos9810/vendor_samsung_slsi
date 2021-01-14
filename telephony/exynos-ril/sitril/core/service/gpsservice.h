/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __GPSSERVICE_H__
#define __GPSSERVICE_H__

#include "service.h"

#define    TIMEOUT_GPS_DEFAULT        5000
#define    TIMEOUT_GPS_UNSOL_IND_AP_TO_NETWORK        10

class ServiceMgr;
class Message;
class RequestData;
class ModemData;

class GpsService :public Service
{
public:
    GpsService(RilContext* pRilContext);
    virtual ~GpsService();

protected:
    static const int GPS_TIMEOUT = 5000;

    virtual int OnCreate(RilContext *pRilContext);

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);

protected:
    virtual int DoAgpsDefaultRequestHandler(Message *pMsg);

    //handle solicited response
    virtual int OnAgpsDefaultResponseHandler(Message *pMsg);

    //handle unsolicited response
    int OnAgpsDefaultUnsolRespHandler(Message *pMsg);

    //handle Request(Ind) message from AP to network
    int DoAgpsDefaultRequestIndHandler(Message *pMsg);

};

#endif
