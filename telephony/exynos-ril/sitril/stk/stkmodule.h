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
 * stkmodule.h
 *
 *  Created on: 2017.02.21.
 *      Author: MOX
 */

#ifndef __STK_MODULE_H__
#define __STK_MODULE_H__

#include <stddef.h>
#include <sqlite3.h>
#include "types.h"
#include "thread.h"
#include "message.h"
#include "mutex.h"
#include "socket.h"
#include "tlvparser.h"


#include <queue>
using namespace std;

class StkService;
class Comprehension;

//CommandQualifier for Send data
#define SEND_ON_STORE 0
#define SEND_IMMEDIATELY 1


#define DEFAULT_BUFFER_SIZE 0x0578 // 1400
#define RECEIVE_SENDING_DATA_LEN 0xc8 // 200
#define RECEIVE_DATA_MAX_TR_LEN 0xff // 256

#define DEFAULT_CHANNEL_ID 1

#define TYPE_MOBILE_DEFAULT 1
#define TYPE_MOBILE_CAT 2


typedef enum BearerType {
    BEARER_CSD = 0x01,
    BEARER_GPRS = 0x02,
    BEARER_DEFAULT = 0x03,
    BEARER_UTRAN = 0x0B,
} BEARER;

typedef enum UserOper {
    USER_OPERATION_OK = 0x01,
    USER_OPERATION_REJECT = 0x00,
    USER_OPERATION_UNKNOWN = 0xFF,
} USER_OPERATION;

typedef struct tCmdDetail {
    BYTE cNum;
    BYTE cType;
    BYTE cQualifier;
} COMMAND_DETAIL_SET;

typedef struct tOpenChannelSetting {
    BearerType BearerType;
    string strBearerParameter;
    int nBufferSize;
    string strNumeric;
    string strNetAccessName;
    string strLocalAddr;
    string strId;
    string strPassword;
    CSocket::SocketType TransportType;
    int nPort;
    int nAddrType;
    string strDestAddr;
    string strAlphaId;
} OPEN_CHANNEL_SET;

typedef struct tagChannelStatus
{
    int nChannelId;
    BYTE cChannelState;
    BYTE cExtraInfo;
} CHANNEL_STATUS;

typedef struct SetupEventList
{
    BYTE nEventList[2];
} SETUP_EVENTLIST;

typedef enum PAP_CHAP_AuthType {
    PAP_CHAP_NOT_PERFORM = 0,
    PAP_PERFORM,
    CHAP_PERFORM,
    PAP_CHAP_PERFORM
} PAP_CHAP_AUTHENTICATION_TYPE;

class CStkModule : public Thread
{
private:
    sqlite3 *m_database;
public:
    CStkModule(StkService *pStkService, int nRilSocketID = 0);
    virtual ~CStkModule();

    // Not Used Yet
    virtual int OnCreate();
    // Not Used Yet
    virtual void OnStart();
    virtual void OnDestroy();

    bool IsStarted() { return m_bStarted; }

    int Initialize();
    int Finalize();
    virtual void Run();

    int GetCurrentProcessingCmd() { return m_nCurrentProcessingCmd; }
    int SendDataAvailableEvent();
    int SetReceiveDataLen(int nRxDataLen);

protected:
    // Pipe
    int CreateCommandPipe();

    // Queue
    int Enqueue(const Message *pMsg);
    Message *Dequeue();
    void ClearQueue();

    int Process(Message *pMsg);
    int ProcessRequest(int nMsgId, RequestData *pReq);
    int ProcessModemData(int nMsgId, ModemData *pModem);

    // Request
    int ProcessEnvelopeCommand(RequestData *pReq);
    int ProcessTerminalResponse(RequestData *pReq);
    int ProcessEnvelopeStatus(RequestData *pReq);

    // Response
    int ProcessProactiveCommand(ModemData *pModem);
    int ProcessEnvelopeCommandDone(ModemData *pModem);
    int ProcessTerminalResponseDone(ModemData *pModem);
    int ProcessEnvelopeStatusDone(ModemData *pModem);

    int TerminalResponse(BYTE *pData, int nDataLength);
    int EnvelopeCommand(BYTE *pData, int nDataLength);

    int HandleCommand(Comprehension *pComprehension);

    // BIP
    int DoOpenBipConnection();
    int OnOpenBipConnectionDone(int nResult, void *pData = NULL, int nLength = 0);
    int DoCloseBipConnection();
    int OnCloseBipConnectionDone(int nResult, void *pData = NULL, int nLength = 0);
    int Transfer(Message *pMsg);

    virtual void OnDataRegistrationStateChanged(int regState);
    virtual void OnDataCallStateChanged(int nCid, bool bActive);
    virtual bool IsDefaultApnConnecting();

    //Socket
    int CreateChannelSocket(string &pstrIFName, int nRetryError = 0, int nDuration = 0, int nInterval = 500);
    int CloseChannelSocket();

    //Proactive Commnad for BIP
    int OpenChannel(Comprehension *pComprehension);
    int CloseChannel(Comprehension *pComprehension);
    int ReceiveData(Comprehension *pComprehension);
    int SendData(Comprehension *pComprehension);

    int SendOnStore(int nMode, string strData, int nDevId);
    int SendOnTcp(int nMode, string strData, int nDevId);
    int SendOnUdp(int nMode, string strData, int nDevId);

    int SendOpenChannelResponse(int nResult, int nAddResult);
    int SendCloseChannelResponse(int nResult, int nAddResult);
    int SendSendDataResponse(int nLen, int nResult, int nAddResult, int CommandQualifier);
    int SendReceiveDataResponse(string strData, int nLen, int nResult);
    int SendSetupEventListResponse(int nResult, int nAddResult);

    int CalcAvailableChannelBufferLen(int nMode, int nLen);

    int GetChannelStatus(Comprehension *pComprehension);
    int SendChannelStatusResponse(bool bChnStatus, int nResult);

    bool IsSupportEvent(BYTE cEvent);

    // Event Envelope command
    int SendChannelStatusEvent();

    // sql db
    int InsertOpenChannelApn(OPEN_CHANNEL_SET tOpenChannelSettingInfo);
    int DeleteOpenChannelApn(void);

    // with Alpha Id
    int OnUserOperation(bool bAccept = true);

    int DefaultAPNOpenChannel();
    int OpenChannelInfoInitialization();

public:
    char m_szReceiveDataBuffer[DEFAULT_BUFFER_SIZE];

private:
    char m_szTAG[32];
    StkService *m_pStkService;
    friend class StkService;
    CMutex *m_pMutex;
    bool m_bWait = false;
    bool m_bUserOper = false;
    Comprehension *m_pComprehension = NULL;

    // Command Pipe to be Read/Written to Queue
    int m_nCmdRead;
    int m_nCmdWritten;

    // STD Queue
    queue<Message *> m_cmdQueue;

    COMMAND_DETAIL_SET m_tCmdDet;
    OPEN_CHANNEL_SET m_tOpenChannelInfo;
    CHANNEL_STATUS m_tChannelStatus;
    CHANNEL_STATUS m_tCachedChannelStatus;

    int m_nConnectType;

    int m_nCurrentProcessingCmd;

    //Open Channel Status
    bool m_bChannelEstablished;

    bool m_bDefaultApnConnection;
    int m_nBipCid;      // 0 means no connection

    // socket info
    CSocket *m_pStkTcpSocket;
    CSocket *m_pStkUdpSocket;

    //Send Data information
    string m_strChnData;
    int m_nCommandQualifier;
    string m_strSendDataBuffer;

    //Receive Data information
    string m_strReceiveDataBuffer;
    int m_nRxBufferLen;
    int m_nReceiveDataOffset;

    //DeviceIdentity
    int m_nSourceId;
    int m_nDestinationId;

    int m_nAccumlatedSendDataLenInBuffer;

    SetupEventList m_tSetupEventList;

    int m_nUserOper;

};
#endif // __STK_MODULE_H__
