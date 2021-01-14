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
 * rilresponselistener.h
 *
 *  Created on: 2014. 11. 20.
 *      Author: sungwoo48.choi
 */

#ifndef    __RIL_RESPONSE_LISTENER_H__
#define    __RIL_RESPONSE_LISTENER_H__

#include "rilapplicationcontext.h"

class RilApplication;

class RilResponseListener : public RilApplicationContext
{
    DECLARE_MODULE_TAG()
private:
    RilApplication *m_pRilApp;
    const struct RIL_Env *m_pRilEnv;

public:
    RilResponseListener(const struct RIL_Env *pRilEnv);
    virtual ~RilResponseListener();

private:
    bool FilterOut(int unsolResponse, const void *data, unsigned int datalen);

public:
    // response
    void OnRequestComplete(RIL_Token t, RIL_Errno e, void *response, unsigned int responselen);
    void OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen);
    void OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen, RIL_SOCKET_ID socket_id);
    void OnRequestAck(RIL_Token t);

    void SetRilApplication(RilApplication *pRilApp) { m_pRilApp = pRilApp; }
};


#endif // __RIL_RESPONSE_LISTENER_H__


