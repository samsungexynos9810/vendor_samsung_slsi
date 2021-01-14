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
 * protocolbuilder.h
 *
 *  Created on: 2014. 6. 19.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_BUILDER_H__
#define __PROTOCOL_BUILDER_H__

#include "sitdef.h"
#include "rildef.h"
#include "tokengen.h"
#include "modemdata.h"

class ProtocolBuilder
{
protected:
    TokenGen *m_pTokenGen;

public:
    ProtocolBuilder();
    virtual ~ProtocolBuilder();

protected:
    void InitRequestHeader(RCM_HEADER *hdr, int id);
    void InitRequestHeader(RCM_HEADER *hdr, int id, int length);
    void InitIndRequestHeader(RCM_IND_HEADER *hdr, int id, int length);
    void InitRequestHeader(RCM_HEADER *hdr, int id, int length, RCM_TOKEN token);

    int switchRafValueForCP(int raf);
};

#endif /* __PROTOCOL_BUILDER_H__ */
