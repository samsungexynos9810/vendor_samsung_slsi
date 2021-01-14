/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _BASE_DEF_H_
#define _BASE_DEF_H_

#ifdef HAVE_ANDROID_OS
    #include <cutils/properties.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include <list>
#include <map>

#include "secure_c.h"
#include "textutils.h"
#include "systemproperty.h"

using namespace std;

#define MIN(a, b)       ((a) > (b) ? (b) : (a))
#define MAX(a, b)       ((a) > (b) ? (a) : (b))

#define    DECLARE_MODULE_TAG()    static const char *TAG;
#define    IMPLEMENT_MODULE_TAG(theClassName, theTag)    const char *theClassName::TAG = #theTag;
#define    IMPLEMENT_TAG(theTag)    const char *TAG = #theTag;

#endif /*_BASE_DEF_H_*/
