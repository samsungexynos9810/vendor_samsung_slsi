/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#ifndef __CALLDATA_H__
#define __CALLDATA_H__

class ServiceMgr;
class Message;
class RequestData;
class CallDialReqData;
class CallEmergencyDialReqData;
class CallInfo;
class CallList;
class DtmfInfo;
class ModemData;

class ClirInfo
{
public:
    ClirInfo();
private:
    int m_aoc;
    //int m_status;

public:
    void SetClirAoc(int aoc, RIL_SOCKET_ID socketid);
    //void SetClirStatus(int status);
    int GetClirAoc(RIL_SOCKET_ID socketid);
    //int GetClirStatus();
};
#endif
