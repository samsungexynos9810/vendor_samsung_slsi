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
 * protocolbuilder.cpp
 *
 *  Created on: 2014. 6. 27.
 *      Author: sungwoo48.choi
 */

#include "protocolbuilder.h"
#include "util.h"

/**
 * ProtocolBuilder
 */
ProtocolBuilder::ProtocolBuilder() : m_pTokenGen(NULL)
{
    m_pTokenGen = TokenGen::GetInstacne();
}

ProtocolBuilder::~ProtocolBuilder()
{

}

void ProtocolBuilder::InitRequestHeader(RCM_HEADER *hdr, int id)
{
    if (hdr != NULL) {
        memset(hdr, 0, sizeof(RCM_HEADER));
        hdr->type = RCM_TYPE_REQUEST;
        hdr->id = (UINT16)(id & 0xFFFF);
        hdr->ext.req.token = m_pTokenGen->GetNext();
    }
}

void ProtocolBuilder::InitRequestHeader(RCM_HEADER *hdr, int id, int length)
{
    if (hdr != NULL) {
        memset(hdr, 0, sizeof(RCM_HEADER));
        hdr->type = RCM_TYPE_REQUEST;
        hdr->id = (UINT16)(id & 0xFFFF);
        hdr->length = (UINT16)(length & 0xFFFF);
        hdr->ext.req.token = m_pTokenGen->GetNext();
    }
}

/**
 * For GPS IND type request to CP
 */
void ProtocolBuilder::InitIndRequestHeader(RCM_IND_HEADER *hdr, int id, int length)
{
    if (hdr != NULL) {
        memset(hdr, 0, sizeof(RCM_IND_HEADER));
        hdr->type = RCM_TYPE_INDICATION;
        hdr->id = (UINT16)(id & 0xFFFF);
        hdr->length = (UINT16)(length & 0xFFFF);
    }
}

void ProtocolBuilder::InitRequestHeader(RCM_HEADER *hdr, int id, int length, RCM_TOKEN token)
{
    if (hdr != NULL) {
        memset(hdr, 0, sizeof(RCM_HEADER));
        hdr->type = RCM_TYPE_REQUEST;
        hdr->id = (UINT16)(id & 0xFFFF);
        hdr->length = (UINT16)(length & 0xFFFF);
        hdr->ext.req.token = token;
    }
}

int ProtocolBuilder::switchRafValueForCP(int raf)
{
    int result = 0;

    if((raf & RAF_UNKNOWN) != 0)
        result |= RAF_CP_UNKNOWN;
    if((raf & RAF_GPRS) != 0)
        result |= RAF_CP_GPRS;
    if((raf & RAF_EDGE) != 0)
        result |= RAF_CP_EDGE;
    if((raf & RAF_UMTS) != 0)
        result |= RAF_CP_UMTS;
    if((raf & RAF_HSDPA) != 0)
        result |= RAF_CP_HSDPA;
    if((raf & RAF_HSUPA) != 0)
        result |= RAF_CP_HSUPA;
    if((raf & RAF_HSPA) != 0)
        result |= RAF_CP_HSPA;
    if((raf & RAF_LTE) != 0)
        result |= RAF_CP_LTE;
    if((raf & RAF_HSPAP) != 0)
        result |= RAF_CP_HSPAP;
    if((raf & RAF_GSM) != 0)
        result |= RAF_CP_GSM;
    if((raf & RAF_TD_SCDMA) != 0)
        result |= RAF_CP_TD_SCDMA;
    if((raf & RAF_IS95A) != 0)
        result |= RAF_CP_IS95A;
    if((raf & RAF_IS95B) != 0)
        result |= RAF_CP_IS95B;
    if((raf & RAF_1xRTT) != 0)
        result |= RAF_CP_1xRTT;
    if((raf & RAF_EVDO_0) != 0)
        result |= RAF_CP_EVDO_0;
    if((raf & RAF_EVDO_A) != 0)
        result |= RAF_CP_EVDO_A;
    if((raf & RAF_EVDO_B) != 0)
        result |= RAF_CP_EVDO_B;
    if((raf & RAF_EHRPD) != 0)
        result |= RAF_CP_EHRPD;
    if((raf & RAF_NR) != 0)
        result |= RAF_CP_5G;
    return result;
}
