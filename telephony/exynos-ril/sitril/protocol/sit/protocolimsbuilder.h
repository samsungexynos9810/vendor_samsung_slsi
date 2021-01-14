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
 * protocolcallbuilder.h
 *
 *  Created on: 2014. 12. 2.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_IMS_BUILDER_H__
#define __PROTOCOL_IMS_BUILDER_H__

#include "protocolbuilder.h"
#include "imsreqdata.h"

class ProtocolImsBuilder  : public ProtocolBuilder
{
public:
    ProtocolImsBuilder() {}
    virtual ~ProtocolImsBuilder() {}
public:
    virtual ModemData *BuildSetConfig(const char *pSetConfigData);
    virtual ModemData *BuildGetConfig();
    virtual ModemData *BuildSetSrvccCallList(const char *pSrvccCallList);
    virtual ModemData *BuildEmergencyCallStatus(BYTE nStatus, BYTE nRat);

private:
    static const int MAX_IMS_RCM_SIZE = (16 * 1024 + 30);  // max size 16 KB for supporting RCS multiframe packet

public:
//AIMS support start ---------------------
    ModemData *BuildAimsPDU(int requestId, void *data, unsigned int datalen);
    ModemData *BuildAimsIndPDU(int requestId, void *data, unsigned int datalen);
#if 0
    virtual ModemData *BuildSIT_AIMS_DIAL(const char *pSetConfigData);
    virtual ModemData *BuildSIT_AIMS_ANSWER();
    virtual ModemData *BuildSIT_AIMS_ANSWER(const char *data, int datalen);
    virtual ModemData *BuildSIT_AIMS_HANGUP(const char *pSetConfigData);
    virtual ModemData *BuildSIT_AIMS_DEREGISTRATION();
    virtual ModemData *BuildSIT_SET_AIMS_HIDDEN_MENU(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SET_AIMS_IMS_PDN(const char *pSetConfigData);
    virtual ModemData *BuildSIT_AIMS_CALL_MANAGE(const char *pSetConfigData);
    virtual ModemData *BuildSIT_AIMS_SEND_DTMF(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SET_AIMS_FRAME_TIME(const char *pSetConfigData);
    virtual ModemData *BuildSIT_GET_AIMS_FRAME_TIME(const char *pSetConfigData);
    virtual ModemData *BuildSIT_AIMS_MODIFY(const char *pSetConfigData);
    virtual ModemData *BuildSIT_AIMS_RESPONSE_MODIFY(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SET_AIMS_MISC_TIME_INFO(const char *pSetConfigData);
    virtual ModemData *BuildSIT_CONF_CALL_ADD_USER(const char *pSetConfigData);
    virtual ModemData *BuildSIT_ENHANCED_CONF_CALL(const char *pSetConfigData);
    virtual ModemData *BuildSIT_GET_CALL_FORWARD_STATUS(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SET_CALL_FORWARD_STATUS(const char *pSetConfigData);
    virtual ModemData *BuildSIT_GET_CALL_WAITING(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SET_CALL_WAITING(const char *pSetConfigData);
    virtual ModemData *BuildSIT_GET_CALL_BARRING(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SET_CALL_BARRING(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SEND_SMS(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SEND_EXPECT_MORE(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SEND_SMS_ACK(const char *pSetConfigData);
    virtual ModemData *BuildSIT_SEND_ACK_INCOMING_SMS(const char *pSetConfigData);
    virtual ModemData *BuildSIT_CHG_BARRING_PWD(const char *pSetConfigData);
#endif
//AIMS support end ---------------------

};

#endif /* __PROTOCOL_IMS_BUILDER_H__ */
