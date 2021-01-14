 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __PROTOCOL_IO_CHANNEL_H__
#define __PROTOCOL_IO_CHANNEL_H__

#include "rilcontext.h"

class IoChannel {
public:
    IoChannel(const char *name);
    ~IoChannel();

public:
    INT32 Init();
    INT32 GetFd();
    INT32 Open();
    INT32 Close();
    INT32 Read(char *buf, int size);
    INT32 Write(char *buf, int size);
    const char *GetIoChannelName() const { return m_szIoChannelName; }

private:
    INT32 m_Fd;
    char m_szIoChannelName[MAX_IOCHANNEL_NAME_LEN+1];
};

#endif

