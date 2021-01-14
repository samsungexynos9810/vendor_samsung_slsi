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
 * ril_async_message.h
 *
 *  Created on: 2014. 7. 24.
 *      Author: jhdaniel.kim
 */

#ifndef __ASYNC_MESSAGE_H__
#define __ASYNC_MESSAGE_H__

#include <map>

using namespace std;
// { MSG_ID, SERVICE_ID }
typedef std::map<UINT, UINT> AsyncMsgMap;

AsyncMsgMap s_AsyncMsgMap = {
    // For CSC Service Messages
    //{ MSG_CS_CALL_HANGUP, RIL_SERVICE_CSC},

    // For PS Service Messages

    // For SIM Service Messages

    // For MISC Service Messages

    // For NETWORK Service Messages
    { MSG_NET_QUERY_AVAILABLE_NETWORKS, RIL_SERVICE_NETWORK},
    { MSG_NET_QUERY_BPLMN_SEARCH, RIL_SERVICE_NETWORK},
    { MSG_NET_SET_NETWORK_SELECTION_MANUAL, RIL_SERVICE_NETWORK},
    { MSG_NET_SET_NETWORK_SELECTION_MANUAL_WITH_RAT, RIL_SERVICE_NETWORK},

    // For SMS Service Messages

    // For AUDIO Service Messages

    // For IMS Service Messages

    // For GPS Service Messages

    // For WLAN Service Messages

    // For VSIM Service Messages
};

#endif // __ASYNC_MESSAGE_H__
