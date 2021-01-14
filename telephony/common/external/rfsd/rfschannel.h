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
    RFS I/O Reader
*/
#ifndef _rfs_io_channel_h
#define _rfs_io_channel_h

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "rfslog.h"

class CRfsIoChannel {
        int mFd;

public:
        CRfsIoChannel();
        virtual ~CRfsIoChannel();

        int Open(const char *name);
        int Close();
    int GetFd() {return mFd;}
    int Read(char *buf, int size);
        int Write(char *buf, int size);
};

#endif
