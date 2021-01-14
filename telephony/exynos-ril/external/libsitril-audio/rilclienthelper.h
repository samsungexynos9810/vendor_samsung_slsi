/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#ifndef __RIL_CLIENT_HELPER_H__
#define __RIL_CLIENT_HELPER_H__

#include "base.h"
#include "sitril-client.h"

class RilClientHelperFactory;

class RilClientHelper {
    friend RilClientHelperFactory;
public:
    RilClientHelper() {}
    virtual ~RilClientHelper() {}

protected:
    virtual bool Init() { return true; }
    virtual void Release() {}

public:
    // wrapper functions
    virtual void *Open()=0;
    virtual int Close(void* client)=0;
    virtual int Reconnect(void* client)=0;
    virtual int Send(void* client, unsigned msgId, void* data, size_t length, Rilc_OnResponse handler)=0;
    virtual int Send(void* client, unsigned msgId, void* data, size_t length, Rilc_OnResponse handler, unsigned int channel)=0;
    virtual int RegisterUnsolicitedResponseHandler(void* client, Rilc_OnUnsolicitedResponse handler)=0;
};

class RilClientHelperFactory {
    DECLARE_MODULE_TAG()
public:
    enum { RILCLIENT_LINK_STATIC, RILCILENT_DYNAMIC_LOAD };
public:
    static RilClientHelper *CreateHelperInstance(int type);
};


#endif // __RIL_CLIENT_HELPER_H__
