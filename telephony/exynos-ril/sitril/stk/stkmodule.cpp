/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "stkmodule.h"
#include "rillog.h"
#include "stkservice.h"
#include "ber.h"
#include "comprehension.h"
#include "protocolstkadapter.h"
#include "rildata.h"
#include "socket.h"
#include "string.h"
#include "stkdatabuilder.h"

static bool debug = true;

#define DB_NAME                 "/data/user_de/0/com.android.providers.telephony/databases/telephony.db"
#define TABLE_NAME_CARRIERS     "carriers"

#define COL_NAME                "name"
#define COL_APN                 "apn"
#define COL_NUMERIC             "numeric"
#define COL_MCC                 "mcc"
#define COL_MNC                 "mnc"
#define COL_USER                "user"
#define COL_PASSWORD            "password"
#define COL_TYPE                "type"
#define COL_MTU                 "mtu"
#define COL_PROTOCOL            "protocol"

#define SQL_BUF_SIZE             (1024)

// add category to display selective logs
#undef LOGV
#define LOGV(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_VERBOSE_LOG,  "%s::%s() " format, m_szTAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGI
#define LOGI(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_INFO_LOG, "%s::%s() " format, m_szTAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGW
#define LOGW(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_WARNING_LOG,  "%s::%s() " format, m_szTAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGE
#define LOGE(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_CRITICAL_LOG,  "%s::%s() " format, m_szTAG, __FUNCTION__, ##__VA_ARGS__);} while(0)

#undef ENTER_FUNC
#define ENTER_FUNC()        { LOGI("[<--"); }
#undef LEAVE_FUNC
#define LEAVE_FUNC()        { LOGI("[-->"); }

#define PARAM_NULL(msg)     { if(msg==NULL) { LOGE("Parameter = NULL"); return -1; } }
#define NULL_REQ(msg)       { if(msg==NULL || msg->GetRequestData()==NULL) { LOGE("RequestData = NULL"); return -1; } }
#define NULL_RSP(msg)       { if(msg==NULL || msg->GetModemData()==NULL) { LOGE("ModemData = NULL"); return -1; } }

CStkModule::CStkModule(StkService *pStkService, int nRilSocketID /* =0 */)
{
    snprintf(m_szTAG, sizeof(m_szTAG)-1, "%s[%d]", "CStkModule", nRilSocketID);

    ENTER_FUNC();

    m_nCmdRead = -1;
    m_nCmdWritten = -1;

    m_nCurrentProcessingCmd = TAG_NONE;
    m_bDefaultApnConnection = false;

    m_pStkService = pStkService;
    m_pStkTcpSocket =  NULL;

    m_tCmdDet.cNum = 0;
    m_tCmdDet.cType = 0;
    m_tCmdDet.cQualifier = 0;

    OpenChannelInfoInitialization();

    m_pStkTcpSocket = NULL;
    m_pStkUdpSocket = NULL;

    m_strSendDataBuffer = "";
    m_strReceiveDataBuffer = "";

    m_nRxBufferLen = 0;
    m_nReceiveDataOffset = 0;
    memset(m_szReceiveDataBuffer, 0, DEFAULT_BUFFER_SIZE);

    memset(&m_tSetupEventList, EventList::UNKNOWN_EVENT, sizeof(SETUP_EVENTLIST)); //upper 2 events support

    m_nUserOper = USER_OPERATION_UNKNOWN;

    m_database = NULL;
    m_pMutex = NULL;
    m_bChannelEstablished = true;
    m_nCommandQualifier = 0;
    m_nSourceId = 0;
    m_nDestinationId = 0;
    m_nAccumlatedSendDataLenInBuffer = 0;

    LEAVE_FUNC();
}

CStkModule::~CStkModule()
{
    ENTER_FUNC();

    if(m_bStarted) Finalize();

    LEAVE_FUNC();
}

int CStkModule::OnCreate()
{
    ENTER_FUNC();

    LEAVE_FUNC();
    return 0;
}

void CStkModule::OnStart()
{
    ENTER_FUNC();

    LEAVE_FUNC();
}

void CStkModule::OnDestroy()
{
    ENTER_FUNC();

    LEAVE_FUNC();
}

int CStkModule::Initialize()
{
    ENTER_FUNC();

    if(m_bStarted)
    {
        LOGI("Already Initialized !!!");
        return -1;
    }

    m_pMutex = new CMutex();
    if(m_pMutex == NULL) {
        LOGE("Fail to create Mutex instance");
        return -1;
    }

    if (CreateCommandPipe() < 0)
    {
        LOGE("Fail to create command read/written pipe");
        return -1;
    }

    // Start Thread
    ClearQueue();
    Start();

    LOGI("STK Mudule Initialized...");
    LEAVE_FUNC();
    return 0;
}

int CStkModule::Finalize()
{
    ENTER_FUNC();

    if(m_bStarted)
    {
        Stop();
        ClearQueue();
    } else LOGI("Already Finalized !!!");

    LEAVE_FUNC();
    return 0;
}

void CStkModule::Run()
{
    ENTER_FUNC();

    if (m_nCmdRead == -1) {
        LOGE("Command pipe is not created");
        return;
    }

    // Callback OnStart();
    OnStart();

    fd_set rfdSet;

    while(1)
    {
        //Reset FD
        FD_ZERO(&rfdSet);
        FD_SET(m_nCmdRead, &rfdSet);

        int n = select(m_nCmdWritten, &rfdSet, NULL, NULL, NULL);

        if (n > 0) {
            if (FD_ISSET(m_nCmdRead, &rfdSet)) {
                //MsgDirection direction;
                BYTE bFlag = 0;
                int ret = read(m_nCmdRead, &bFlag, 1);
                if (ret < 0) {
                    if (errno == EINTR || errno == EAGAIN) {
                        continue;
                    }
                    else {
                        CreateCommandPipe();
                        continue;
                    }
                }
                else if (ret == 0) {
                    CreateCommandPipe();
                    continue;
                }

                Process(Dequeue());
            }
        }
    }

    LEAVE_FUNC();
}

void CStkModule::ClearQueue()
{
    ENTER_FUNC();

    Message *pMsg;
    while (!m_cmdQueue.empty())
    {
        pMsg = m_cmdQueue.front();
        if (pMsg != NULL) {
            delete pMsg;
        }
        m_cmdQueue.pop();
    }

    LEAVE_FUNC();
}

int CStkModule::Enqueue(const Message *pMsg)
{
    if(NULL == pMsg) {
        LOGE("Enqueue fail: pMsg is NULL");
        return -1;
    }

    LOGI("%d. 0x%08X, Req:0x%08X, Rsp:0x%08X, Dir:%d", m_cmdQueue.size(), pMsg, ((Message *)pMsg)->GetRequestData(), ((Message *)pMsg)->GetModemData(), ((Message *)pMsg)->GetDirection());

    BYTE bFlag = 1;
    m_pMutex->lock();
    m_cmdQueue.push((Message *) pMsg);
    if(m_nCmdWritten!=-1 && write(m_nCmdWritten, &bFlag, 1)==-1) LOGE("Command Pipe Error: m_nCmdWritten=%d", m_nCmdWritten);
    m_pMutex->unlock();

    return 0;
}

Message *CStkModule::Dequeue()
{
    if (m_cmdQueue.empty()==true) return NULL;

    m_pMutex->lock();
    Message *pMsg = m_cmdQueue.front();
    m_cmdQueue.pop();
    m_pMutex->unlock();
    LOGI("0x%08X, Req:0x%08X, Rsp:0x%08X, Dir:%d", pMsg, pMsg->GetRequestData(), pMsg->GetModemData(), pMsg->GetDirection());
    return pMsg;
}

int CStkModule::CreateCommandPipe()
{
    ENTER_FUNC();

    if (m_nCmdRead != -1)
    {
        LOGV("Create command read pipe again");
        close(m_nCmdRead);
        m_nCmdRead = -1;
    }

    if (m_nCmdWritten != -1)
    {
        LOGV("Create command write pipe again");
        close(m_nCmdWritten);
        m_nCmdWritten = -1;
    }

    int fds[2];
    int n = pipe(fds);
    if (n < 0)
    {
        LOGE("Command pipe create fail");
        return -1;
    }

    m_nCmdRead = fds[0];
    m_nCmdWritten = fds[1];

    LOGI("Created Pipes: m_nCmdRead=%d, m_nCmdWritten=%d", m_nCmdRead, m_nCmdWritten);

    LEAVE_FUNC();
    return 0;
}

int CStkModule::Process(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    switch(pMsg->GetDirection())
    {
    case REQUEST:
        ProcessRequest(pMsg->GetMsgId(), pMsg->GetRequestData());
        break;
    case RESPONSE:
        ProcessModemData(pMsg->GetMsgId(), pMsg->GetModemData());
        break;
    default:
        LOGE("Invalid Direction: %d", pMsg->GetDirection());
        break;
    }

    delete pMsg;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::ProcessRequest(int nMsgId, RequestData *pReq)
{
    ENTER_FUNC();
    PARAM_NULL(pReq);

    LOGI("Message ID: %d(0x%02X)", nMsgId, nMsgId);

    switch(nMsgId)
    {
    case MSG_SAT_SEND_ENVELOPE_CMD:
        ProcessEnvelopeCommand(pReq);
        break;
    case MSG_SAT_SEND_TERMINAL_RSP:
        ProcessTerminalResponse(pReq);
        break;
    case MSG_SAT_SEND_ENVELOPE_STATUS:
        ProcessEnvelopeStatus(pReq);
        break;
    default:
        LOGE("Invalid Message ID: %d(0x%02X)", nMsgId, nMsgId);
        break;
    }

    LEAVE_FUNC();
    return 0;
}

int CStkModule::ProcessModemData(int nMsgId, ModemData *pModem)
{
    ENTER_FUNC();
    PARAM_NULL(pModem);

    LOGI("Message ID: %d(0x%02X)", nMsgId, nMsgId);

    switch(nMsgId)
    {
    case MSG_SAT_PROACTIVE_COMMAND:
        ProcessProactiveCommand(pModem);
        break;
    case MSG_SAT_SEND_ENVELOPE_CMD_DONE:
        ProcessEnvelopeCommandDone(pModem);
        break;
    case MSG_SAT_SEND_TERMINAL_RSP_DONE:
        ProcessTerminalResponseDone(pModem);
        break;
    case MSG_SAT_SEND_ENVELOPE_STATUS_DONE:
        ProcessEnvelopeStatusDone(pModem);
        break;
    default:
        LOGE("Invalid Message ID: %d(0x%02X)", nMsgId, nMsgId);
        break;
    }

    LEAVE_FUNC();
    return 0;
}

// Request
int CStkModule::ProcessEnvelopeCommand(RequestData *pReq)
{
    ENTER_FUNC();
    PARAM_NULL(pReq);

    LEAVE_FUNC();
    return 0;
}

int CStkModule::ProcessTerminalResponse(RequestData *pReq)
{
    ENTER_FUNC();
    PARAM_NULL(pReq);

    LEAVE_FUNC();
    return 0;
}

int CStkModule::ProcessEnvelopeStatus(RequestData *pReq)
{
    ENTER_FUNC();
    PARAM_NULL(pReq);

    LEAVE_FUNC();
    return 0;
}

// Response
int CStkModule::ProcessProactiveCommand(ModemData *pModem)
{
    ENTER_FUNC();
    PARAM_NULL(pModem);

    ProtocolStkProactiveCommandAdapter adapter(pModem);
    CBER *pBer = new CBER(adapter.GetProactiveCommand(), adapter.GetProactiveCmdLength());
    if(pBer && pBer->IsValid())
    {
        LOGI("Proactive Command is Valid");
        Comprehension *pComprehension = pBer->GetComprehension();
        //Comprehension *pComprehension = new Comprehension();
        //pComprehension->Set(pBer->GetValue(), pBer->GetLength());
        LOGI("Comprehension:%08X", pComprehension);
        if(pComprehension)
        {
            LOGI("Comprehension Count:%d", pComprehension->GetTlvCount());

            CommandDetail *pCmdDetail = (CommandDetail *) pComprehension->GetTlv((BYTE)TAG_COMMAND_DETAIL);
            LOGI("CommandDetail-Number:%d, Type:0x%02X, Qualifier:%d", pCmdDetail->GetCommandNumber(), pCmdDetail->GetCommandType(), pCmdDetail->GetQualifier());
            m_tCmdDet.cNum = (BYTE)(pCmdDetail->GetCommandNumber());
            m_tCmdDet.cType = (BYTE)(pCmdDetail->GetCommandType());
            m_tCmdDet.cQualifier = (BYTE)(pCmdDetail->GetQualifier());

            AlphaIdentifier *pAlphaID = (AlphaIdentifier *) pComprehension->GetTlv((BYTE)TAG_ALPHA_IDENTIFIER);

            if ((pAlphaID != NULL) && (pAlphaID->GetAlphaIdentifier() != "")) // check empty alphaid(27.22.4.30.3/2 senddata)
            {
                LOGI("This command has alphaid");
                StkDataBuilder builder;
                const RilData *pRilData = builder.BuildStkProactiveCommandIndicate(adapter.GetProactiveCmdLength(), adapter.GetProactiveCommand());
                m_pStkService->OnUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND, pRilData->GetData(), pRilData->GetDataLength());
                m_pComprehension = pComprehension;
                m_nCurrentProcessingCmd = pCmdDetail->GetCommandType();
                m_nUserOper = USER_OPERATION_UNKNOWN;
                LEAVE_FUNC();
                return 0;
            }
        }

        HandleCommand(pComprehension);
        delete pComprehension;
    }
    if (pBer) delete pBer;
    LEAVE_FUNC();
    return 0;
}


int CStkModule::HandleCommand(Comprehension *pComprehension)
{
    ENTER_FUNC();
    PARAM_NULL(pComprehension);

    CommandDetail *pCmdDetail = (CommandDetail *) pComprehension->GetTlv((BYTE) TAG_COMMAND_DETAIL);
    BearerDescription *pBearerDesc = NULL;
    BufferSize *pBufferSize = NULL;
    TextString *pId = NULL;
    TextString *pPassword = NULL;
    TerminalInterfaceTransportLevel *pTransPortLevel = NULL;
    OtherAddress *pDestinationAddr = NULL;
    NetworkAccessName *pNetAccessName = NULL;
    AlphaIdentifier *pAlphaId = NULL;
    DeviceIdentity *pDevId = NULL;
    EventList *pEventList = NULL;
    //BYTE *pDestAddr = NULL;

    LOGI("GetCommandType:0x%02X (%s)", pCmdDetail->GetCommandType(), pCmdDetail->GetCommandTypeString());
    switch(pCmdDetail->GetCommandType())
    {
    case CommandDetail::OPEN_CHANNEL:
        LOGI("OPEN_CHANNEL");
        LOGI("GetTlvCount:%d", pComprehension->GetTlvCount());
        for(int i=0; i<pComprehension->GetTlvCount(); i++)
        {
            CTLV *pTlv = (CTLV *) pComprehension->GetTlv(i);
            switch(pTlv->GetTag())
            {
            case TAG_BEARER_DESCRIPTION:
                pBearerDesc = (BearerDescription *) pTlv;
                m_tOpenChannelInfo.BearerType = (BearerType) pBearerDesc->GetType();
                m_tOpenChannelInfo.strBearerParameter = pBearerDesc->GetParamString();
                LOGI("BearerType:%d, %s", m_tOpenChannelInfo.BearerType, m_tOpenChannelInfo.strBearerParameter.c_str());
                break;
            case TAG_BUFFER_SIZE:
                pBufferSize = (BufferSize *) pTlv;
                m_tOpenChannelInfo.nBufferSize = pBufferSize->GetBufferSize();
                LOGI("nBufferSize:%d", m_tOpenChannelInfo.nBufferSize);
                break;
            case TAG_NETWORK_ACCESS_NAME:
                pNetAccessName = (NetworkAccessName *) pTlv;
                m_tOpenChannelInfo.strNetAccessName = pNetAccessName->GetAddress();
                LOGI("strNetAccessName:%s", m_tOpenChannelInfo.strNetAccessName.c_str());
                break;
            case TAG_UICC_TERMINAL_INTERFACE_TRANSPORT_LEVEL:
                pTransPortLevel = (TerminalInterfaceTransportLevel *) pTlv;
                m_tOpenChannelInfo.TransportType =(CSocket::SocketType) pTransPortLevel->GetTransportType();
                m_tOpenChannelInfo.nPort = pTransPortLevel->GetPort();
                LOGI("TransportType:%d, %d", m_tOpenChannelInfo.TransportType, m_tOpenChannelInfo.nPort);
                break;
            case TAG_TEXT_STRING:
                if(pId == NULL)
                {
                    pId = (TextString *) pTlv;
                    m_tOpenChannelInfo.strId = (char *) pId->GetTextString();
                    LOGI("ID:%s", m_tOpenChannelInfo.strId.c_str());
                }
                else
                {
                    pPassword = (TextString *) pTlv;
                    m_tOpenChannelInfo.strPassword= (char *) pPassword->GetTextString();
                    LOGI("Password:%s", m_tOpenChannelInfo.strPassword.c_str());
                }
                break;
            case TAG_OTHER_ADDRESS:
                pDestinationAddr = (OtherAddress *) pTlv;
                m_tOpenChannelInfo.nAddrType = pDestinationAddr->GetAddressType();
                {
                    char *pacAddr = (char *) pDestinationAddr->GetAddress();
                    char szAddr[64];
                    memset(szAddr, 0, 64);
                    if(m_tOpenChannelInfo.nAddrType==OtherAddress::ADDRESS_IPV4)
                    {
                        sprintf(szAddr, "%d.%d.%d.%d", pacAddr[0], pacAddr[1], pacAddr[2], pacAddr[3]);
                    }
                    else if(m_tOpenChannelInfo.nAddrType==OtherAddress::ADDRESS_IPV6)
                    {
                        sprintf(szAddr, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x",
                                            pacAddr[0], pacAddr[1], pacAddr[2], pacAddr[3],
                                            pacAddr[4], pacAddr[5], pacAddr[6], pacAddr[7],
                                            pacAddr[8], pacAddr[9], pacAddr[10], pacAddr[11],
                                            pacAddr[12], pacAddr[13], pacAddr[14], pacAddr[15]);
                    }
                    m_tOpenChannelInfo.strDestAddr = szAddr;
                }
                LOGI("DestinationAddr:%d, %s", m_tOpenChannelInfo.nAddrType, m_tOpenChannelInfo.strDestAddr.c_str());
                break;
            case TAG_ALPHA_IDENTIFIER:
                pAlphaId = (AlphaIdentifier *) pTlv;
                m_tOpenChannelInfo.strAlphaId = pAlphaId->GetAlphaIdentifier();
                LOGI("AlphaId:%s", m_tOpenChannelInfo.strAlphaId.c_str());
                break;
            case TAG_DEVICE_IDENTITY:
                pDevId = (DeviceIdentity *) pTlv;
                m_nSourceId = pDevId->GetSourceID();
                m_nDestinationId= pDevId->GetDestinationID();
                LOGI("DeviceId:%s -> %s", DeviceIdentity::GetDeviceIdString(pDevId->GetSourceID()), DeviceIdentity::GetDeviceIdString(pDevId->GetDestinationID()));
                break;
            }
        }
        OpenChannel(pComprehension);
        break;
    case CommandDetail::CLOSE_CHANNEL:
        LOGI("CLOSE_CHANNEL");
        CloseChannel(pComprehension);
        break;
    case CommandDetail::RECEIVE_DATA:
        LOGI("RECEIVE_DATA");
        ReceiveData(pComprehension);
        break;
    case CommandDetail::SEND_DATA:
        LOGI("SEND_DATA");
        SendData(pComprehension);
        break;
    case CommandDetail::GET_CHANNEL_STATUS:
        LOGI("GET_CHANNEL_STATUS");
        GetChannelStatus(pComprehension);
        break;
    case CommandDetail::SET_UP_EVENT_LIST:
        LOGI("SET_UP_EVENT_LIST");
        pEventList = (EventList*) pComprehension->GetTlv((BYTE) TAG_EVENT_LIST);

        if(IsSupportEvent(pEventList->GetEventList()))
        {
            LOGI("Already support event : 0x%02X", pEventList->GetEventList());
        }
        else
        {
            for(int i = 0; i < 2; i++)
            {
                if(m_tSetupEventList.nEventList[i] == EventList::UNKNOWN_EVENT)
                {
                   m_tSetupEventList.nEventList[i] = pEventList->GetEventList();
                   LOGI("Register Event : 0x%02X", pEventList->GetEventList());
                   break;
                }
            }
        }
        SendSetupEventListResponse(Result::SUCCESS, -1);
        break;

    }

    LEAVE_FUNC();
    return 0;
}

// nRetryError: 0(No Retry), -1(Always Retry), >0(Designated Error Retry)
// nDuration: 0(No Retry)
int CStkModule::CreateChannelSocket(string &strIFName, int nRetryError /* =0 */, int nDuration /* =0 */, int nInterval /* =500 */)
{
    ENTER_FUNC();

    if (m_tOpenChannelInfo.TransportType == CSocket::SOCK_TCP)
    {
        if(m_pStkTcpSocket == NULL)
        {
           m_pStkTcpSocket = new CSocket(this);
           m_pStkTcpSocket->SetDestination(m_tOpenChannelInfo.strDestAddr, m_tOpenChannelInfo.nPort);
           if(strIFName != "") m_pStkTcpSocket->SetInterface(strIFName);

           if(m_pStkTcpSocket->Create(m_tOpenChannelInfo.TransportType)<0)
           {
               LOGE("m_pStkTcpSocket->Create() Error !!!");
               return -1;
           }

           int nError = 0;
           int nRemain = nDuration;
           do {
               if((nError = m_pStkTcpSocket->Connect())!=0)
               {
                   LOGE("m_pStkTcpSocket->Connect() Error(%d) !!!", nError);
                   // Check errno for Retry
                   if(nRetryError!=-1 && nRetryError!=nError) return -1;

                   // Retry more?
                   if(nRemain>0)
                   {
                       usleep(nInterval * 1000);
                       LOGI("m_pStkTcpSocket->Connect() retry");
                   }
               } else break;    // Success

               nRemain -= (nInterval>0? nInterval: nDuration);
           } while(nError!=0 && nRemain>0);

           // Fianlly Error
           if(nError!=0) return -1;
        }
        else
        {
            LOGE("Already TCP Socket Created!!!");
        }
    }
   else if (m_tOpenChannelInfo.TransportType == CSocket::SOCK_UDP)
   {
        if (m_pStkUdpSocket == NULL)
        {
            m_pStkUdpSocket = new CSocket(this);
            m_pStkUdpSocket->SetDestination(m_tOpenChannelInfo.strDestAddr, m_tOpenChannelInfo.nPort);
            if(strIFName != "") m_pStkUdpSocket->SetInterface(strIFName);

            if(m_pStkUdpSocket->Create(m_tOpenChannelInfo.TransportType)<0)
            {
                LOGE("m_pStkUdpSocket->Create() Error !!!");
                return -1;
            }
        }
       else
        {
            LOGE("Already UDP Socket Created!!!");
            return -1;
        }
   }

    LEAVE_FUNC();
    return 0;
}

int CStkModule::CloseChannelSocket()
{
    ENTER_FUNC();

    if(m_pStkTcpSocket != NULL)
    {
        m_pStkTcpSocket->Close();
        delete m_pStkTcpSocket;
        m_pStkTcpSocket = NULL;
    }

    if(m_pStkUdpSocket != NULL)
    {
        m_pStkUdpSocket->Close();
        delete m_pStkUdpSocket;
        m_pStkUdpSocket = NULL;
    }

    LEAVE_FUNC();
    return 0;
}


int CStkModule::ProcessEnvelopeCommandDone(ModemData *pModem)
{
    ENTER_FUNC();
    PARAM_NULL(pModem);

    LEAVE_FUNC();
    return 0;
}


int CStkModule::ProcessTerminalResponseDone(ModemData *pModem)
{
    ENTER_FUNC();
    PARAM_NULL(pModem);

    m_nCurrentProcessingCmd = TAG_NONE;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::ProcessEnvelopeStatusDone(ModemData *pModem)
{
    ENTER_FUNC();
    PARAM_NULL(pModem);

    LEAVE_FUNC();
    return 0;
}

int CStkModule::SendSendDataResponse(int nLen, int nResult, int nAddResult, int CommandQualifier)
{
    ENTER_FUNC();
    // build terminal response
    Comprehension *pComprehension = new Comprehension;
    CommandDetail *pCmdDetail = new CommandDetail(m_tCmdDet.cType, (BYTE)CommandQualifier);
    DeviceIdentity *pDevId = new DeviceIdentity(DeviceIdentity::TERMINAL, DeviceIdentity::UICC);
    Result *pResult = new Result;
    ChannelDataLength *pChnDataLen = new ChannelDataLength(nLen);

    pComprehension->AppendTlv((CTLV *) pCmdDetail);
    pComprehension->AppendTlv((CTLV *) pDevId);
    if (nAddResult >= 0)
    {
        pResult->Set(2, nResult, (BYTE *)&nAddResult);
        pComprehension->AppendTlv((CTLV *) pResult);
    }
    else
    {
        pResult->Set(1, nResult, NULL);
        pComprehension->AppendTlv((CTLV *) pResult);
    }

    if ((Result::GeneralResult)nResult == Result::SUCCESS)
    {
        pComprehension->AppendTlv((CTLV *) pChnDataLen);
    }

    int nRawDataLength = pComprehension->GetRawDataLength();
    BYTE *pRawData = pComprehension->GetRawData();
    LOGI("RawDataLength: [%d]", nRawDataLength);
    PrintBufferDump("nRawData", pRawData, nRawDataLength);

    // Send terminal response request to StkService
    int sendResult = TerminalResponse(pRawData, nRawDataLength);
    // Sending TerminalResponse is failed
    if (sendResult < 0) {
        LOGE("Sending TerminalResponse is failed");
        delete pComprehension;
        return -1;
    }
    // Delete new allocated memory after sending
    delete pComprehension;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::SendReceiveDataResponse(string strData, int nLen, int nResult)
{
    ENTER_FUNC();
    // build terminal response
    Comprehension *pComprehension = new Comprehension;
    CommandDetail *pCmdDetail = new CommandDetail(m_tCmdDet.cType, m_tCmdDet.cQualifier, m_tCmdDet.cNum);
    DeviceIdentity *pDevId = new DeviceIdentity(DeviceIdentity::TERMINAL, DeviceIdentity::UICC);
    Result *pResult = new Result(1, nResult, NULL);
    ChannelData *pChnData = new ChannelData(strData);
    ChannelDataLength *pChnDataLen = new ChannelDataLength(nLen);
    pComprehension->AppendTlv((CTLV *) pCmdDetail);
    pComprehension->AppendTlv((CTLV *) pDevId);
    pComprehension->AppendTlv((CTLV *) pResult);
    if ((Result::GeneralResult)nResult == Result::SUCCESS)
    {
        pComprehension->AppendTlv((CTLV *) pChnData);
        pComprehension->AppendTlv((CTLV *) pChnDataLen);
    }
    int nRawDataLength = pComprehension->GetRawDataLength();
    BYTE *pRawData = pComprehension->GetRawData();
    LOGI("RawDataLength: [%d]", nRawDataLength);
    PrintBufferDump("nRawData", pRawData, nRawDataLength);

    // Send terminal response request to StkService
    int sendResult = TerminalResponse(pRawData, nRawDataLength);
    // Sending TerminalResponse is failed
    if (sendResult < 0) {
        LOGE("Sending TerminalResponse is failed");
        delete pComprehension;
        return -1;
    }
    // Delete new allocated memory after sending
    delete pComprehension;
    LEAVE_FUNC();
    return 0;

}

int CStkModule::OpenChannelInfoInitialization()
{
    ENTER_FUNC();
    m_tOpenChannelInfo.BearerType = (BearerType) 0;
    m_tOpenChannelInfo.strBearerParameter = "";
    m_tOpenChannelInfo.nBufferSize = -1;
    m_tOpenChannelInfo.strNumeric = "";
    m_tOpenChannelInfo.strNetAccessName = "";
    m_tOpenChannelInfo.strLocalAddr = "";
    m_tOpenChannelInfo.strId = "";
    m_tOpenChannelInfo.strPassword = "";
    m_tOpenChannelInfo.TransportType = CSocket::SOCK_TCP;
    m_tOpenChannelInfo.nPort = 0;
    m_tOpenChannelInfo.nAddrType = -1;
    m_tOpenChannelInfo.strDestAddr = "";
    m_tOpenChannelInfo.strAlphaId = "";

    m_tChannelStatus.nChannelId = DEFAULT_CHANNEL_ID;
    m_tChannelStatus.cChannelState = ChannelStatus::CLOSED;
    m_tChannelStatus.cExtraInfo = 0;
    m_tCachedChannelStatus.nChannelId = DEFAULT_CHANNEL_ID;
    m_tCachedChannelStatus.cChannelState = ChannelStatus::CLOSED;
    m_tCachedChannelStatus.cExtraInfo = 0;

    m_nBipCid = 0;
    m_tChannelStatus.cChannelState = ChannelStatus::CLOSED;
    m_nConnectType = -1;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::SendOpenChannelResponse(int nResult, int nAddResult)
{
    ENTER_FUNC();
    // build terminal response
    Comprehension *pComprehension = new Comprehension;
    CommandDetail *pCmdDetail = new CommandDetail(m_tCmdDet.cType, m_tCmdDet.cQualifier);
    DeviceIdentity *pDevId = new DeviceIdentity(DeviceIdentity::TERMINAL, DeviceIdentity::UICC);
    Result *pResult = new Result();
    if (nAddResult >= 0)
    {
        pResult->Set(2, nResult, (BYTE *)&nAddResult);
    }
    else
    {
        pResult->Set(1, nResult, NULL);
    }
    ChannelStatus *pChStatus = new ChannelStatus(m_tChannelStatus.nChannelId, (ChannelStatus::Channel_Status)m_tChannelStatus.cChannelState, false, 0);
    BearerDescription *pBearerDesc = new BearerDescription(m_tOpenChannelInfo.BearerType, m_tOpenChannelInfo.strBearerParameter.c_str());
    BufferSize *pBufSize = new BufferSize(m_tOpenChannelInfo.nBufferSize);

    pComprehension->AppendTlv((CTLV *) pCmdDetail);
    pComprehension->AppendTlv((CTLV *) pDevId);
    pComprehension->AppendTlv((CTLV *) pResult);
    // success
    if ((Result::GeneralResult)nResult == Result::SUCCESS || (Result::GeneralResult)nResult == Result::PERFORM_WITH_MODIFICATION)
    {
        if ((TerminalInterfaceTransportLevel::TransportProtocolType) m_tOpenChannelInfo.TransportType == TerminalInterfaceTransportLevel::TCP_SERVER)
        {
            pChStatus->Set(m_tChannelStatus.nChannelId, ChannelStatus::LISTEN, false, 0);
            pComprehension->AppendTlv((CTLV *) pChStatus);
        }
        else
        {
            pChStatus->Set(m_tChannelStatus.nChannelId, ChannelStatus::ESTABLISHED, false, 0);
            pComprehension->AppendTlv((CTLV *) pChStatus);
        }
    }
    pComprehension->AppendTlv((CTLV *) pBearerDesc);
    pComprehension->AppendTlv((CTLV *) pBufSize);

    int nRawDataLength = pComprehension->GetRawDataLength();
    BYTE *pRawData = pComprehension->GetRawData();
    LOGI("RawDataLength: [%d]", nRawDataLength);
    PrintBufferDump("nRawData", pRawData, nRawDataLength);

    // Send terminal response request to StkService
    int sendResult = TerminalResponse(pRawData, nRawDataLength);
    // Sending TerminalResponse is failed
    if (sendResult < 0) {
        LOGE("Sending TerminalResponse is failed");
        delete pComprehension;
        return -1;
    }
    // Delete new allocated memory after sending
    delete pComprehension;

    LEAVE_FUNC();
    return 0;

}

int CStkModule::SendCloseChannelResponse(int nResult, int nAddResult)
{
    ENTER_FUNC();
    // build terminal response
    Comprehension *pComprehension = new Comprehension;
    CommandDetail *pCmdDetail = new CommandDetail(m_tCmdDet.cType, RFU);
    DeviceIdentity *pDevId = new DeviceIdentity(DeviceIdentity::TERMINAL, DeviceIdentity::UICC);
    Result *pResult = new Result;
    if (nAddResult >= 0)
    {
        pResult->Set(2, nResult, (BYTE *)&nAddResult);
    }
    else
    {
        pResult->Set(1, nResult, NULL);
        m_bChannelEstablished = false;
    }
    pComprehension->AppendTlv((CTLV *) pCmdDetail);
    pComprehension->AppendTlv((CTLV *) pDevId);
    pComprehension->AppendTlv((CTLV *) pResult);

    int nRawDataLength = pComprehension->GetRawDataLength();
    BYTE *pRawData = pComprehension->GetRawData();
    LOGI("RawDataLength: [%d]", nRawDataLength);
    PrintBufferDump("nRawData", pRawData, nRawDataLength);

    // Send terminal response request to StkService
    int sendResult = TerminalResponse(pRawData, nRawDataLength);
    // Sending TerminalResponse is failed
    if (sendResult < 0) {
        LOGE("Sending TerminalResponse is failed");
        delete pComprehension;
        return -1;
    }
    // Delete new allocated memory after sending
    delete pComprehension;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::SendSetupEventListResponse(int nResult, int nAddResult)
{
    ENTER_FUNC();
    // build terminal response
    Comprehension *pComprehension = new Comprehension;
    CommandDetail *pCmdDetail = new CommandDetail(m_tCmdDet.cType, m_tCmdDet.cQualifier);
    DeviceIdentity *pDevId = new DeviceIdentity(DeviceIdentity::TERMINAL, DeviceIdentity::UICC);
    Result *pResult = new Result();
    if (nAddResult >= 0)
    {
        pResult->Set(2, nResult, (BYTE *)&nAddResult);
    }
    else
    {
        pResult->Set(1, nResult, NULL);
    }

    pComprehension->AppendTlv((CTLV *) pCmdDetail);
    pComprehension->AppendTlv((CTLV *) pDevId);
    pComprehension->AppendTlv((CTLV *) pResult);

    int nRawDataLength = pComprehension->GetRawDataLength();
    BYTE *pRawData = pComprehension->GetRawData();
    LOGI("RawDataLength: [%d]", nRawDataLength);
    PrintBufferDump("nRawData", pRawData, nRawDataLength);

    // Send terminal response request to StkService
    int sendResult = TerminalResponse(pRawData, nRawDataLength);
    // Sending TerminalResponse is failed
    if (sendResult < 0) {
        LOGE("Sending TerminalResponse is failed");
        delete pComprehension;
        return -1;
    }
    // Delete new allocated memory after sending
    delete pComprehension;

    LEAVE_FUNC();
    return 0;

}

int CStkModule::TerminalResponse(BYTE *pData, int nDataLength)
{
    ENTER_FUNC();

    char *pszHexStr = new char[(nDataLength*2)+1];
    memset(pszHexStr, 0, (nDataLength*2)+1);
    int nHexStrLen = Value2HexString(pszHexStr, pData, nDataLength);

    StringRequestData *pReq = new StringRequestData(RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, 0, REQ_PRIVATE);
    pReq->encode(pszHexStr, nHexStrLen);
    Message *pMsg = new Message(RIL_SERVICE_SIM, MSG_SAT_SEND_TERMINAL_RSP, REQUEST, pReq);
    delete []pszHexStr;

    int result = Transfer(pMsg);
    if (result < 0) return -1;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::EnvelopeCommand(BYTE *pData, int nDataLength)
{
    ENTER_FUNC();

    CEnvelopeBER *pEnvBer = new CEnvelopeBER();
    if(pEnvBer==NULL) { LOGE("pEnvBer = NULL"); LEAVE_FUNC(); return -1; }

    pEnvBer->Set(nDataLength, pData);
    BYTE *pEnvRawData = pEnvBer->GetRawData();
    int EnvRawDataLength = pEnvBer->GetRawDataLength();

    char *pszHexStr = new char[(EnvRawDataLength*2)+1];
    if(pszHexStr==NULL) { LOGE("pszHexStr = NULL"); LEAVE_FUNC(); return -1; }
    memset(pszHexStr, 0, (EnvRawDataLength*2)+1);
    int nHexStrLen = Value2HexString(pszHexStr, (BYTE *)pEnvRawData, EnvRawDataLength);

    StringRequestData *pReq = new StringRequestData(RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, 0, REQ_FW);
    if(pReq==NULL) { LOGE("pReq = NULL"); LEAVE_FUNC(); return -1; }
    pReq->encode(pszHexStr, nHexStrLen);
    Message *pMsg = new Message(RIL_SERVICE_SIM, MSG_SAT_SEND_ENVELOPE_CMD, REQUEST, pReq);
    if(pMsg==NULL) { LOGE("pMsg = NULL"); LEAVE_FUNC(); return -1; }
    delete []pszHexStr;
    delete pEnvBer;

    int result = Transfer(pMsg);
    if (result < 0) return -1;
    LEAVE_FUNC();
    return 0;
}

int CStkModule::DefaultAPNOpenChannel()
{
    ENTER_FUNC();

    string strIFName = "";
    if(CreateChannelSocket(strIFName, ENETUNREACH, 1500) < 0)
    {
       LOGE("CreateChannelSocket() FAIL");
       if (SendOpenChannelResponse(Result::BEYOND_TERMINAL_CAPABILITY, -1) < 0) {
           LOGE("OnOpenBipConnectionDone->SendOpenChannelResponse was failed");
       }
       LEAVE_FUNC();
       return -1;
    }

    int nResultCode = Result::SUCCESS;
    if(m_tOpenChannelInfo.nBufferSize > DEFAULT_BUFFER_SIZE)
    {
       m_tOpenChannelInfo.nBufferSize = DEFAULT_BUFFER_SIZE;
       nResultCode = Result::PERFORM_WITH_MODIFICATION;
    }

    if (SendOpenChannelResponse(nResultCode, -1) < 0) {
       LOGE("OnOpenBipConnectionDone->SendOpenChannelResponse was failed");
       LEAVE_FUNC();
       return -1;
    }

   LEAVE_FUNC();
   return 0;
}

int CStkModule::OpenChannel(Comprehension *pComprehension)
{
    ENTER_FUNC();
    PARAM_NULL(pComprehension);

    if ((AlphaIdentifier *) pComprehension->GetTlv((BYTE)TAG_ALPHA_IDENTIFIER) != NULL)
    {
        if(m_nUserOper == USER_OPERATION_REJECT)
        {
            return SendOpenChannelResponse(Result::USER_UNACCEPT_PROACTIVE_COMMAND, -1);
        }
    }

    if((m_tOpenChannelInfo.BearerType != BEARER_GPRS) &&
           (m_tOpenChannelInfo.BearerType != BEARER_UTRAN) &&
           (m_tOpenChannelInfo.BearerType != BEARER_DEFAULT))
    {
         LOGE("openChannel failed : BearerType not supported");
         return -1;
    }

    if (m_tOpenChannelInfo.strNetAccessName == "")
        LOGI("[OpenChannel] No NetworkAcceeName");

    if ((m_tOpenChannelInfo.BearerType == (BearerType)BEARER_DEFAULT)
        || (m_tOpenChannelInfo.strNetAccessName == ""))
    {
        LOGI("[OpenChannel] Bearer Default");
        m_nConnectType = TYPE_MOBILE_DEFAULT;
        if(m_bDefaultApnConnection){
            DefaultAPNOpenChannel();
            LEAVE_FUNC();
            return 0;
        }
        else
        {
            LOGI("[OpenChannel] Not yet default data connection");
            LEAVE_FUNC();
            return 0;
        }
    }

    m_tOpenChannelInfo.strNumeric = SystemProperty::Get(PROPERTY_ICC_OPERATOR_NUMERIC);

    DeleteOpenChannelApn();
    InsertOpenChannelApn(m_tOpenChannelInfo);
    DoOpenBipConnection();

    LEAVE_FUNC();
    return 0;
}

int CStkModule::CloseChannel(Comprehension *pComprehension)
{
    ENTER_FUNC();

    DeviceIdentity *pDevId = (DeviceIdentity *) pComprehension->GetTlv((BYTE)TAG_DEVICE_IDENTITY);
    LOGI("[CloseChannel] m_bChannelEstablished : %d",m_bChannelEstablished);

    if(m_nConnectType == TYPE_MOBILE_DEFAULT && ((pDevId->GetDestinationID()  & 0x0F ) == DEFAULT_CHANNEL_ID))
    {
        SendCloseChannelResponse(Result::SUCCESS, -1);
        OpenChannelInfoInitialization();
        LEAVE_FUNC();
        return 0;
    }
    if(m_bChannelEstablished)
    {
        if((pDevId->GetDestinationID()  & 0x0F ) != m_tChannelStatus.nChannelId)
        {
            SendCloseChannelResponse(Result::BIP_ERROR, Result::CHANNEL_ID_NOT_VAILD);
        }
        else
        {
            CloseChannelSocket();

            if(m_nConnectType == TYPE_MOBILE_CAT)
            {
                DoCloseBipConnection();
                DeleteOpenChannelApn();
                SendCloseChannelResponse(Result::SUCCESS, -1);
                OpenChannelInfoInitialization();
            }
        }
    }
    else
    {
        SendCloseChannelResponse(Result::BIP_ERROR, Result::CHANNEL_CLOSED);
    }

    LEAVE_FUNC();
    return 0;
}

int CStkModule::SetReceiveDataLen(int nRxDataLen)
{
    ENTER_FUNC();

    m_nRxBufferLen = nRxDataLen;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::ReceiveData(Comprehension *pComprehension)
{
    ENTER_FUNC();
    CommandDetail *pCmdDetail = (CommandDetail *) pComprehension->GetTlv((BYTE)TAG_COMMAND_DETAIL);
    DeviceIdentity *pDevId = (DeviceIdentity *) pComprehension->GetTlv((BYTE)TAG_DEVICE_IDENTITY);
    ChannelDataLength *pChannelDataLength = (ChannelDataLength *) pComprehension->GetTlv((BYTE)TAG_CHANNEL_DATA_LENGTH);

    if (pCmdDetail == NULL || pDevId == NULL || pChannelDataLength == NULL)
    {
        LOGE("TLV is NULL!!");
        LEAVE_FUNC();
        return -1;
    }

    //int nMode = (int)pCmdDetail->GetQualifier();
    //int nDevId = pDevId->GetDestinationID();
    int nRemainDataLenFromNetwork = 0;
    int nCurrentSendingDataLenToUicc = 0;
    int nRemainDataLenFromNetworkOnNextstep = 0;
    unsigned char szData[RECEIVE_DATA_MAX_TR_LEN];

    int nRequestedChnDataLenFromUicc = pChannelDataLength->GetChannelDataLength();

    if(m_nRxBufferLen > m_nReceiveDataOffset) nRemainDataLenFromNetwork = m_nRxBufferLen - m_nReceiveDataOffset;

    LOGI("[ReceiveData] RxBufferLen:[%d], RemainDataLenFromNetwork:[%d], RequestedChnDataLenFromUicc:[%d]", m_nRxBufferLen, nRemainDataLenFromNetwork, nRequestedChnDataLenFromUicc);

    if(nRemainDataLenFromNetwork > 0)
    {
        nCurrentSendingDataLenToUicc = (nRemainDataLenFromNetwork>nRequestedChnDataLenFromUicc)? nRequestedChnDataLenFromUicc: nRemainDataLenFromNetwork;

        memcpy(szData, m_szReceiveDataBuffer + m_nReceiveDataOffset, nCurrentSendingDataLenToUicc);

        char *pszHexStr = new char[(nCurrentSendingDataLenToUicc*2)+1];
        if(pszHexStr==NULL)
        {
            LOGE("pszHexStr is NULL!!");
            LEAVE_FUNC();
            return -1;
        }

        memset(pszHexStr, 0, (nCurrentSendingDataLenToUicc*2)+1);
        int nHexStrLen = Value2HexString(pszHexStr, szData, nCurrentSendingDataLenToUicc);
        pszHexStr[nHexStrLen] = '\0';

        nRemainDataLenFromNetworkOnNextstep = nRemainDataLenFromNetwork - nCurrentSendingDataLenToUicc;
        LOGI("[ReceiveData] nRemainDataLenFromNetworkOnNextstep : %d", nRemainDataLenFromNetworkOnNextstep);

        if (nRemainDataLenFromNetworkOnNextstep >= RECEIVE_DATA_MAX_TR_LEN){
            LOGI("[ReceiveData] Sending Data Length with 0xFF");
            SendReceiveDataResponse(pszHexStr, 0xFF, Result::SUCCESS);
        } else if (nRemainDataLenFromNetworkOnNextstep > 0){
            LOGI("[ReceiveData] Sending Data Length with %d ", nRemainDataLenFromNetworkOnNextstep);
            SendReceiveDataResponse(pszHexStr, nRemainDataLenFromNetworkOnNextstep, Result::SUCCESS);
        } else { // this is last
            LOGI("[ReceiveData] Sending Data Length with 0 ");
            SendReceiveDataResponse(pszHexStr, 0, Result::SUCCESS);
        }

        m_nReceiveDataOffset += nCurrentSendingDataLenToUicc;
        delete []pszHexStr;
    }
    else
    {
        memset(m_szReceiveDataBuffer, 0, DEFAULT_BUFFER_SIZE);
        m_nRxBufferLen = 0;
        m_nReceiveDataOffset = 0;
    }

    LEAVE_FUNC();
    return 0;
}

int CStkModule::SendData(Comprehension *pComprehension)
{
    ENTER_FUNC();
    CommandDetail *pCmdDetail = (CommandDetail *) pComprehension->GetTlv((BYTE)TAG_COMMAND_DETAIL);
    DeviceIdentity *pDevId = (DeviceIdentity *) pComprehension->GetTlv((BYTE)TAG_DEVICE_IDENTITY);
    ChannelData *pChannelData = (ChannelData *) pComprehension->GetTlv((BYTE)TAG_CHANNEL_DATA);
    int nMode = (int)pCmdDetail->GetQualifier();
    int nDevId = pDevId->GetDestinationID();
    string strChnData = pChannelData->GetChannelDataString();
    LOGI("strChnData: [%s]", strChnData.c_str());

    if(m_tOpenChannelInfo.TransportType == CSocket::SOCK_TCP)
    {
        if(nMode == SEND_ON_STORE)
        {
            SendOnStore(nMode, strChnData, nDevId);
        }
        else
        {
            SendOnTcp(nMode, strChnData, nDevId);
        }
    }
    else if(m_tOpenChannelInfo.TransportType == CSocket::SOCK_UDP)
    {
        if(nMode == SEND_ON_STORE)
        {
            SendOnStore(nMode, strChnData, nDevId);
        }
        else
        {
            SendOnUdp(nMode, strChnData, nDevId);
        }
    }
    else
    {
        LOGE("[SendData] Transport type not supported");
        SendSendDataResponse(0, Result::BEYOND_TERMINAL_CAPABILITY, -1, SD_SEND_IMMEDIATELY);
    }
    LEAVE_FUNC();
    return 0;
}

//strlen(x) only works on char arrays
//x.size() and x.length(), where x is std::string.

int CStkModule::SendOnStore(int nMode, string strData, int nDevId)
{
    ENTER_FUNC();
    int nSendLen = strData.length() / 2;
    LOGI("[SendOnStore] Send Data Len : %d", nSendLen);

    m_strSendDataBuffer.append(strData);

    int nChnLen = CalcAvailableChannelBufferLen(nMode, nSendLen);
    SendSendDataResponse(nChnLen, Result::SUCCESS, -1, SD_STORE_DATA_IN_TX_BUFFER);
    LEAVE_FUNC();
    return 0;
}

int CStkModule::SendOnTcp(int nMode, string strData, int nDevId)
{
    ENTER_FUNC();
    if(m_pStkTcpSocket == NULL)
    {
        LOGE("[SendData] TCP socket is null, channel not established");
        SendSendDataResponse(0, Result::BEYOND_TERMINAL_CAPABILITY, -1, SD_SEND_IMMEDIATELY);
        return 0;
    }
    LOGI("[SendOnTcp] Send String Data Len : %d", strData.length());

    int nDataLen = strData.length() / 2;
    m_strSendDataBuffer.append(strData);

    LOGI("[SendOnTcp] Send Data Len : %d", nDataLen);

    unsigned char buffer[1600];
    int buffer_len = HexString2Value((unsigned char*)buffer, (char *)m_strSendDataBuffer.c_str());

    LOGI("[SendOnTcp] Send Buffer Len : %d", buffer_len);

    m_pStkTcpSocket->Send(buffer, buffer_len, 0);
    int nChnLen = CalcAvailableChannelBufferLen(nMode, nDataLen);
    SendSendDataResponse(nChnLen, Result::SUCCESS, -1, SD_SEND_IMMEDIATELY);

    // Start receive
    LOGI("[SendData] Start TCP receive thread...");
    m_pStkTcpSocket->Start();

    LEAVE_FUNC();
    return 0;
}

int CStkModule::SendOnUdp(int nMode, string strData, int nDevId)
{
    ENTER_FUNC();
    if(m_pStkUdpSocket == NULL)
    {
        LOGE("[SendData] UDP socket is null, channel not established");
        SendSendDataResponse(0, Result::BEYOND_TERMINAL_CAPABILITY, -1, SD_SEND_IMMEDIATELY);
        return 0;
    }
    if(m_tChannelStatus.nChannelId != (nDevId & 0x0F))
    {
        LOGE("[SendData] invalid channel ID");
        SendSendDataResponse(0, Result::BIP_ERROR, Result::CHANNEL_ID_NOT_VAILD, SD_SEND_IMMEDIATELY);
        return 0;
    }

    LOGI("[SendOnUdp] Send String Data Len : %d", strData.length());

    int nDataLen = strData.length() / 2;
    m_strSendDataBuffer.append(strData);

    LOGI("[SendOnUdp] Send Data Len : %d", nDataLen);

    unsigned char buffer[1600];
    int buffer_len = HexString2Value((unsigned char*)buffer, (char *)m_strSendDataBuffer.c_str());

    LOGI("[SendOnUdp] Send Buffer Len : %d", buffer_len);

    m_pStkUdpSocket->Sendto(buffer, buffer_len, 0);

    int nChnLen = CalcAvailableChannelBufferLen(nMode, nDataLen);
    SendSendDataResponse(nChnLen, Result::SUCCESS, -1, SD_SEND_IMMEDIATELY);
    m_strSendDataBuffer = "";

    LOGI("[SendData] Start UDP receive thread...");
    m_pStkUdpSocket->Start();

    LEAVE_FUNC();
    return 0;
}

int CStkModule::CalcAvailableChannelBufferLen(int nMode, int nLen)
{
    ENTER_FUNC();
    LOGI("calcChannelDataLen: Mode : %d, Len : %d", nMode, nLen);
    int nResult = 0;
    m_nAccumlatedSendDataLenInBuffer += nLen;
    if(nMode == SEND_IMMEDIATELY)
    {
        m_nAccumlatedSendDataLenInBuffer = 0;
    }
    if(m_tOpenChannelInfo.nBufferSize - m_nAccumlatedSendDataLenInBuffer > RECEIVE_DATA_MAX_TR_LEN)
    {
        nResult = 0xFF;
    }
    else
    {
        nResult = m_tOpenChannelInfo.nBufferSize - m_nAccumlatedSendDataLenInBuffer;
    }
    //nResult means the available buffer len in Tx buffer.
    LOGI("calcChannelDataLen: nResult : %d", nResult);
    LEAVE_FUNC();
    return nResult;
}

int CStkModule::SendChannelStatusResponse(bool bChnStatus, int nResult)
{
    ENTER_FUNC();
     // build terminal response

    Comprehension *pComprehension = new Comprehension;
    CommandDetail *pCmdDetail = new CommandDetail(CommandDetail::GET_CHANNEL_STATUS, RFU);
    DeviceIdentity *pDevId = new DeviceIdentity(DeviceIdentity::TERMINAL, DeviceIdentity::UICC);
    Result *pResult = new Result();
    pResult->Set(1, nResult, NULL);
    ChannelStatus *pChStatus = new ChannelStatus(m_tChannelStatus.nChannelId, (ChannelStatus::Channel_Status)m_tChannelStatus.cChannelState, false, 0);

    pComprehension->AppendTlv((CTLV *) pCmdDetail);
    pComprehension->AppendTlv((CTLV *) pDevId);
    pComprehension->AppendTlv((CTLV *) pResult);

    // success
    if ((Result::GeneralResult)nResult == Result::SUCCESS)
    {
        if (bChnStatus)
        {
            pChStatus->Set(m_tChannelStatus.nChannelId, ChannelStatus::ESTABLISHED, false, 0);
            pComprehension->AppendTlv((CTLV *) pChStatus);
        }
    }

    //should add  link drop detection (WITH SETUP_EVENT_LIST)
    int nRawDataLength = pComprehension->GetRawDataLength();
    BYTE *pRawData = pComprehension->GetRawData();
    LOGI("RawDataLength: [%d]", nRawDataLength);
    PrintBufferDump("nRawData", pRawData, nRawDataLength);

    // Send terminal response request to StkService
    int sendResult = TerminalResponse(pRawData, nRawDataLength);
    // Sending TerminalResponse is failed
    if (sendResult < 0) {
        LOGE("Sending TerminalResponse is failed");
        delete pComprehension;
        return -1;
    }
    // Delete new allocated memory after sending
    delete pComprehension;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::GetChannelStatus(Comprehension *pComprehension)
{
    ENTER_FUNC();
    if(m_tChannelStatus.cChannelState == ChannelStatus::ESTABLISHED)
        SendChannelStatusResponse(1, Result::SUCCESS);
    else
        SendChannelStatusResponse(0, Result::SUCCESS);
    LEAVE_FUNC();
    return 0;
}

bool CStkModule::IsSupportEvent(BYTE cEvent)
{
    ENTER_FUNC();
    bool bEventPresent = false;
    for(int i = 0; i < 2; i++)
    {
        LOGI("EventValue: [0x%X]", m_tSetupEventList.nEventList[i]);
        if(m_tSetupEventList.nEventList[i] == cEvent)
        {
            bEventPresent = true;
            LOGI("Event Already Present : 0x%02X", cEvent);
            break;
        }
    }
    LEAVE_FUNC();
    return bEventPresent;
}

int CStkModule::SendChannelStatusEvent()
{
    ENTER_FUNC();
    if((m_tCachedChannelStatus.nChannelId == m_tChannelStatus.nChannelId)
        && (m_tCachedChannelStatus.cChannelState == m_tChannelStatus.cChannelState))
    {
         LOGI("Channel status has no change, don't sent event download(channel status)");
         return 0;
    }

    memcpy(&m_tCachedChannelStatus, &m_tChannelStatus, sizeof(CHANNEL_STATUS));
    if(!IsSupportEvent((BYTE)(EventList::CHANNEL_STATUS_EVENT)))
    {
         LOGI("The UIM didn't set up event for (Channel status)");
         return 0;
    }

    // build event envelope command
    Comprehension *pComprehension = new Comprehension;
    EventList *pEventList = new EventList((BYTE)EventList::CHANNEL_STATUS_EVENT);
    DeviceIdentity *pDevId = new DeviceIdentity(DeviceIdentity::TERMINAL, DeviceIdentity::UICC);

    pComprehension->AppendTlv((CTLV *) pEventList);
    pComprehension->AppendTlv((CTLV *) pDevId);

    if(m_bChannelEstablished && (m_tChannelStatus.cChannelState == ChannelStatus::CLOSED))
    {
        ChannelStatus *pChStatus = new ChannelStatus(m_tChannelStatus.nChannelId, ChannelStatus::CLOSED, true, ChannelStatus::LINK_DROPPED); //link drop case
        pComprehension->AppendTlv((CTLV *) pChStatus);
    }
    else
    {
        ChannelStatus *pChStatus = new ChannelStatus(m_tChannelStatus.nChannelId, ChannelStatus::ESTABLISHED, false, 0);
        pComprehension->AppendTlv((CTLV *) pChStatus);
    }

    int nRawDataLength = pComprehension->GetRawDataLength();
    BYTE *pRawData = pComprehension->GetRawData();
    LOGI("RawDataLength: [%d]", nRawDataLength);
    PrintBufferDump("nRawData", pRawData, nRawDataLength);

    // Send EnvelopeCommand request to StkService
    int sendResult = EnvelopeCommand(pRawData, nRawDataLength);
    // Sending EnvelopeCommand is failed
    if (sendResult < 0) {
        LOGE("Sending EnvelopeCommand is failed");
        delete pComprehension;
        return -1;
    }
    // Delete new allocated memory after sending
    delete pComprehension;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::SendDataAvailableEvent()
{
    ENTER_FUNC();
    if(!IsSupportEvent((BYTE)(EventList::DATA_AVAILABLE_EVENT)))
    {
         LOGI("The UIM didn't set up event for (Data available)");
         return 0;
    }

    int nlength = 0;

    // build event envelope command
    Comprehension *pComprehension = new Comprehension;
    EventList *pEventList = new EventList((BYTE)EventList::DATA_AVAILABLE_EVENT);
    DeviceIdentity *pDevId = new DeviceIdentity(DeviceIdentity::TERMINAL, DeviceIdentity::UICC);
    ChannelStatus *pChStatus = new ChannelStatus(m_tChannelStatus.nChannelId, ChannelStatus::ESTABLISHED, false, 0);

    if (m_nRxBufferLen > 0xFF)
        nlength = 0xFF;
    else
        nlength = m_nRxBufferLen;
    ChannelDataLength *pChannelDataLen = new ChannelDataLength(nlength);

    pComprehension->AppendTlv((CTLV *) pEventList);
    pComprehension->AppendTlv((CTLV *) pDevId);
    pComprehension->AppendTlv((CTLV *) pChStatus);
    pComprehension->AppendTlv((CTLV *) pChannelDataLen);

    int nRawDataLength = pComprehension->GetRawDataLength();
    BYTE *pRawData = pComprehension->GetRawData();
    LOGI("RawDataLength: [%d]", nRawDataLength);
    PrintBufferDump("nRawData", pRawData, nRawDataLength);

    // Send EnvelopeCommand request to StkService
    int sendResult = EnvelopeCommand(pRawData, nRawDataLength);
    // Sending EnvelopeCommand is failed
    if (sendResult < 0) {
        LOGE("Sending EnvelopeCommand is failed");
        delete pComprehension;
        return -1;
    }
    // Delete new allocated memory after sending
    delete pComprehension;

    LEAVE_FUNC();
    return 0;
}

int CStkModule::Transfer(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    if(m_pStkService) m_pStkService->OnStkModule(pMsg);
    else {
        LOGE("m_pStkService = NULL, cannot transfer to STK service");
        return -1;
    }

    LEAVE_FUNC();
    return 0;
}

// BIP
int CStkModule::DoOpenBipConnection()
{
    ENTER_FUNC();

    RilDataStrings strings(8);

    // Test Code for BIP
    strings.SetString(0, "1");
    strings.SetString(1, "0");
    strings.SetString(2, m_tOpenChannelInfo.strNetAccessName.c_str());
    strings.SetString(3, m_tOpenChannelInfo.strId.c_str());
    strings.SetString(4, m_tOpenChannelInfo.strPassword.c_str());
    strings.SetString(5, std::to_string(PAP_CHAP_PERFORM).c_str());
    strings.SetString(6, "IP");
    strings.SetString(7, (char *) NULL);

    if(m_pStkService) m_pStkService->OnRequestInternal(RIL_REQUEST_SETUP_DATA_CALL, strings.GetData(), strings.GetDataLength());
    else LOGE("m_pStkService = NULL, cannot execute");


    LEAVE_FUNC();
    return 0;
}

int CStkModule::OnOpenBipConnectionDone(int nResult, void *pData /* =NULL */, int nLength /* =0 */)
{
    ENTER_FUNC();
    string strIFName = "";
    int nBipActive = 0;

    LOGI("pData:0x%08X, Length:%d", pData, nLength);

    if(nResult!=RIL_E_SUCCESS || (pData == NULL))
    {
        LOGE("BipConnection FAIL");
        if (SendOpenChannelResponse(Result::BEYOND_TERMINAL_CAPABILITY, -1) < 0) {
            LOGE("SendOpenChannelResponse() was failed");
        }
        m_nBipCid = 0;
        m_tChannelStatus.cChannelState = ChannelStatus::CLOSED;
        LEAVE_FUNC();
        return -1;
    }

    RIL_Data_Call_Response_v11 *pRilDataCallRsp = (RIL_Data_Call_Response_v11 *) pData;
    m_nBipCid = pRilDataCallRsp->cid;
    nBipActive = pRilDataCallRsp->active;
    if (pRilDataCallRsp->ifname != NULL)
        strIFName = pRilDataCallRsp->ifname;

    LOGI("cid : %d, active : %d, strIFName : %s", m_nBipCid, nBipActive, strIFName.c_str());
    if(CreateChannelSocket(strIFName) < 0)
    {
        LOGE("CreateChannelSocket() FAIL");
        if (SendOpenChannelResponse(Result::BEYOND_TERMINAL_CAPABILITY, -1) < 0) {
            LOGE("OnOpenBipConnectionDone->SendOpenChannelResponse was failed");
        }
        m_nBipCid = 0;
        m_tChannelStatus.cChannelState = ChannelStatus::CLOSED;
        LEAVE_FUNC();
        return -1;
    }

    int nResultCode = Result::SUCCESS;
    if(m_tOpenChannelInfo.nBufferSize > DEFAULT_BUFFER_SIZE)
    {
        m_tOpenChannelInfo.nBufferSize = DEFAULT_BUFFER_SIZE;
        nResultCode = Result::PERFORM_WITH_MODIFICATION;
    }

    if (SendOpenChannelResponse(nResultCode, -1) < 0) {
        LOGE("OnOpenBipConnectionDone->SendOpenChannelResponse was failed");
        LEAVE_FUNC();
        return -1;
    }

    m_tChannelStatus.cChannelState = ChannelStatus::ESTABLISHED;
    m_bChannelEstablished = true;
    m_nConnectType = TYPE_MOBILE_CAT;

    memcpy(&m_tCachedChannelStatus, &m_tChannelStatus, sizeof(CHANNEL_STATUS));

//    TerminalResponse();
    LEAVE_FUNC();
    return 0;
}

int CStkModule::DoCloseBipConnection()
{
    ENTER_FUNC();

    if(m_nBipCid)
    {
        RilDataStrings strings(2);
        char szCid[8];
        snprintf(szCid, 8, "%d", m_nBipCid);
        strings.SetString(0, szCid);        // CID
        strings.SetString(1, "0");          // Reason

        if(m_pStkService) m_pStkService->OnRequestInternal(RIL_REQUEST_DEACTIVATE_DATA_CALL, strings.GetData(), strings.GetDataLength());
        else LOGE("m_pStkService = NULL, cannot execute");
    }
    else LOGE("Cannot execute, Current CID=%d", m_nBipCid);

    LEAVE_FUNC();
    return 0;
}

int CStkModule::OnCloseBipConnectionDone(int nResult, void *pData /* =NULL */, int nLength /* =0 */)
{
    ENTER_FUNC();

    if(nResult!=RIL_E_SUCCESS)
    {
         LOGE("OnCloseBipConnectionDone() FAIL");
         return -1;
    }

    LEAVE_FUNC();
    return 0;
}

void CStkModule::OnDataRegistrationStateChanged(int regState)
{
    ENTER_FUNC();

    switch(regState)
    {
    case REGISTERED_HOME:
    case REGISTERED_ROAMING:
        if(m_nCurrentProcessingCmd==CommandDetail::OPEN_CHANNEL && m_nBipCid==0)
        {
            DoOpenBipConnection();
        }
        break;
    }

    LEAVE_FUNC();
}

void CStkModule::OnDataCallStateChanged(int nCid, bool bActive)
{
    ENTER_FUNC();

    LOGI("CID:%d, Active:%s", nCid, bActive? "true": "false");
    //SIM #1 : 1,2,3,4  SIM #2 : 5,6,7,8
    if(((nCid == 1)||(nCid == 5)) && !m_bDefaultApnConnection && bActive)
    {
        //Check whether exisiting openchannel request
        if((m_tCmdDet.cType == (BYTE)CommandDetail::OPEN_CHANNEL)
            && ((m_tOpenChannelInfo.BearerType == BEARER_DEFAULT)|| (m_tOpenChannelInfo.strNetAccessName == "")))
            DefaultAPNOpenChannel();

        m_bDefaultApnConnection = bActive;
    }

    if(nCid == m_nBipCid)
    {
        if(m_bChannelEstablished && !bActive)
        //linkdrop case
        {
            m_tChannelStatus.cChannelState = ChannelStatus::CLOSED;
            SendChannelStatusEvent();
        }
    }

    LEAVE_FUNC();
}

bool CStkModule::IsDefaultApnConnecting()
{
    ENTER_FUNC();

    bool bResult = false;
    if(m_pStkService) bResult = m_pStkService->IsDefaultApnConnecting();

    LEAVE_FUNC();
    return bResult;
}

int CStkModule::DeleteOpenChannelApn(void)
{
    ENTER_FUNC();

    char *errMsg = NULL;

    string sql = "DELETE FROM carriers WHERE type = \"cat\"";

    if (sqlite3_open_v2(DB_NAME, &m_database, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK)
    {
        LOGE("Failed to open database");
        m_database = NULL;
        return 0;
    }

    if (sqlite3_exec(m_database, sql.c_str(), NULL, NULL, &errMsg) != SQLITE_OK)
    {
        // log error
        LOGE("sqlite3_exec error : %s", errMsg);
        LOGE("sql=%s", sql.c_str());
        sqlite3_free(errMsg);
    }

    if (debug) LOGI("%s", sql.c_str());
    sqlite3_close(m_database);
    LEAVE_FUNC();
    return 0;

}

int CStkModule::InsertOpenChannelApn(OPEN_CHANNEL_SET tOpenChannelInfo)
{
    ENTER_FUNC();

    char *errMsg = NULL;
    string sql = "";
    string mcc = "", mnc = "";
    char value_str[SQL_BUF_SIZE];

    if (tOpenChannelInfo.strNumeric.empty() == false)
    {
        mcc = tOpenChannelInfo.strNumeric.substr(0,3);
        mnc = tOpenChannelInfo.strNumeric.substr(3);
    }

    sql.append("INSERT INTO carriers (name, apn, numeric, mcc, mnc, user, password, type, mtu");

    if (sqlite3_open_v2(DB_NAME, &m_database, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK)
    {
        LOGE("Failed to open database");
        m_database = NULL;
    }

    if(tOpenChannelInfo.strBearerParameter.empty() == false) {
        int len = tOpenChannelInfo.strBearerParameter.length();
        if ("02" == tOpenChannelInfo.strBearerParameter.substr(len-2,len)) {

            sql.append(", protocol)");
            snprintf(value_str, SQL_BUF_SIZE, " VALUES (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"cat\", %d, \"IP\")",
                tOpenChannelInfo.strBearerParameter.c_str(),
                tOpenChannelInfo.strNetAccessName.c_str(),
                tOpenChannelInfo.strNumeric.c_str(),
                mcc.c_str(),
                mnc.c_str(),
                tOpenChannelInfo.strId.c_str(),
                tOpenChannelInfo.strPassword.c_str(),
                tOpenChannelInfo.nBufferSize);
            sql.append(value_str);
        }
        else
        {
            sql.append(")");
            snprintf(value_str, SQL_BUF_SIZE, " VALUES (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"cat\", %d)",
                tOpenChannelInfo.strBearerParameter.c_str(),
                tOpenChannelInfo.strNetAccessName.c_str(),
                tOpenChannelInfo.strNumeric.c_str(),
                mcc.c_str(),
                mnc.c_str(),
                tOpenChannelInfo.strId.c_str(),
                tOpenChannelInfo.strPassword.c_str(),
                tOpenChannelInfo.nBufferSize);
            sql.append(value_str);
        }
    }
    else
    {
        sql.append(")");
        snprintf(value_str, SQL_BUF_SIZE, " VALUES (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"cat\", %d)",
            tOpenChannelInfo.strBearerParameter.c_str(),
            tOpenChannelInfo.strNetAccessName.c_str(),
            tOpenChannelInfo.strNumeric.c_str(),
            mcc.c_str(),
            mnc.c_str(),
            tOpenChannelInfo.strId.c_str(),
            tOpenChannelInfo.strPassword.c_str(),
            tOpenChannelInfo.nBufferSize);
        sql.append(value_str);
    }


    if (sqlite3_exec(m_database, sql.c_str(), NULL, NULL, &errMsg) != SQLITE_OK)
    {
        LOGE("sqlite3_exec error : %s", errMsg);
        LOGE("sql=%s", sql.c_str());

        sqlite3_free(errMsg);
        return false;
    }

    if (debug)  LOGI("%s", sql.c_str());

    sqlite3_close(m_database);
    LEAVE_FUNC();
    return 0;

}

int CStkModule::OnUserOperation(bool bAccept /* =true */)
{
    ENTER_FUNC();

    if(m_nCurrentProcessingCmd==TAG_NONE)
    {
        LOGE("Current Processing Command is TAG_NONE !!!");
        LEAVE_FUNC();
        return 0;
    }

    m_nUserOper = bAccept;
    HandleCommand(m_pComprehension);

    LEAVE_FUNC();
    return 0;
}

