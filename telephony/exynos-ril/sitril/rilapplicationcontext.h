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
 * rilapplicationcontext.h
 *
 *  Created on: 2014. 11. 20.
 *      Author: sungwoo48.choi
 */

#ifndef __RIL_APPLICATION_CONTEXT_H__
#define __RIL_APPLICATION_CONTEXT_H__

#include "rildef.h"

class RilApplicationContext {
public:
    RilApplicationContext() {}
    virtual ~RilApplicationContext() {}
public:
    // response
    virtual void OnRequestComplete(RIL_Token t, RIL_Errno e, void *response, unsigned int responselen)=0;
    virtual void OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen)=0;
    virtual void OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen, RIL_SOCKET_ID socket_id)=0;
    virtual void OnRequestAck(RIL_Token t)=0;
};


#endif /* __RIL_APPLICATION_CONTEXT_H__ */
