 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _RIL_PARSER_H_
#define _RIL_PARSER_H_

#include "rildef.h"
#include "requestdata.h"

typedef RequestData *(*parseFunc)(int id, Token tok, char *data, unsigned int datalen);

typedef struct _tParseFuncMap
{
    int nid;
    parseFunc pFunc;
}ParseFuncMap_t;

class RilParser
{
public:
    RequestData *GetRequestData(int id, Token tok, void *data = NULL, unsigned int datalen = 0);

public:
    RilParser();
    ~RilParser();

public:
    static RequestData *CreateRequestData(int id, Token tok, char *data, unsigned int datalen);

    static RequestData *CreateInt(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateInts(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateString(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateStrings(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateRawData(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateHookRaw(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateOemRequest(int id, Token tok, char *data, unsigned int datalen);

    static RequestData *CreateCallDial(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateCallEmergencyDial(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateCallForward(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateCallNumber(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateGsmSms(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSmsAck(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSmsAckPdu(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateDeflection(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSetSmsConfig(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSendDtmf(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateStartDtmf(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateStopDtmf(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSimIoData(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSimUiccSubscription(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSimAuthentication(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSimSmsData(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSetupDataCall(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateDeactivateDataCall(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSetInitialAttachApn(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSetDataProfile(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSimAPDU(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateOemSimAuthRequest(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSimOpenChannel(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateUpdatePbEntry(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateNetRCData(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateNvReadItem(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateNvWriteItem(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateVsimOperation(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateVsimOperationExt(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateApnSettings(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateCarrierRestrictions(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateCarrierInfoForImsiEncryption(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateRequestKeepalive(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateNetworkScanRequest(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSignalStrengthReportingCriteria(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateLinkCapacityReportingCriteria(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateEmbmsSessionData(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateSetActivateVsim(int id, Token tok, char *data, unsigned int datalen);

#ifdef SUPPORT_CDMA
    static RequestData *CreateCdmaSms(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateCdmaSmsAck(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateCdmaSetBroadcastSmsConfig(int id, Token tok, char *data, unsigned int datalen);
    static RequestData *CreateRuimSmsData(int id, Token tok, char *data, unsigned int datalen);
#endif // SUPPORT_CDMA

protected:
    parseFunc FindFunc(const int id);
};

#endif /*_RIL_PARSER_H_*/
