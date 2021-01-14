/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include "rilapplication.h"
#include "rillog.h"

#include "customproductfeature.h"

#ifdef __cplusplus
extern "C" {
#endif


#if defined(ANDROID_MULTI_SIM)
static void onRequest(int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
{
    RilApplication::RIL_OnRequest(request, data, datalen, t, socket_id);
}

static RIL_RadioState onStateRequest(RIL_SOCKET_ID socket_id)
{
    return RilApplication::RIL_OnRadioStateRequest(socket_id);
}
#else
static void onRequest(int request, void *data, size_t datalen, RIL_Token t)
{
    RilApplication::RIL_OnRequest(request, data, datalen, t);
}

static RIL_RadioState onStateRequest()
{
    return RilApplication::RIL_OnRadioStateRequest();
}
#endif // ANDROID_MULTI_SIM

static int onSupports(int requestCode)
{
    return RilApplication::RIL_OnSupports(requestCode);
}

static void onCancel(RIL_Token t)
{
    RilApplication::RIL_OnCancel(t);
}

static const char * getVersion(void)
{
    return RilApplication::RIL_OnGetVersion();
}

/**
 * RIL callbacks exposed to RILD daemon
 */
static const RIL_RadioFunctions s_callbacks =
{
    RilApplication::RIL_VersionCode,
    onRequest,
    onStateRequest,
    onSupports,
    onCancel,
    getVersion
};


const RIL_RadioFunctions * RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
    CRilLog::InitRilLog();
    CRilLog::DumpResetLog("");
    CRilEventLog::InitRilEventLog();

    ProductFeature::Init("sitril");

    RilApplication *rilApp = RilApplication::CreateInstance(env);
    if (rilApp != NULL) {
        return &s_callbacks;
    }
    return NULL;
}

/* RIL vendor external */
static void externalOnRequest(int reqOemId, RIL_External_Token externalToken, void *data, size_t datalen, RIL_SOCKET_ID socket_id)
{
    RilApplication::RIL_externalOnRequest(reqOemId, data, datalen, (RIL_Token)externalToken, socket_id);
}

static const RIL_RadioExternalFunctions s_externalCallbacks =
{
    RilApplication::RIL_VersionCode,
    externalOnRequest
};

const RIL_RadioExternalFunctions * RIL_External_Init(const struct RIL_External_Env *env)
{
    RilApplication *rilApp = RilApplication::GetInstance();
    if (rilApp != NULL) {
        rilApp->setRilExternalEnv(env);
        return &s_externalCallbacks;
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif

