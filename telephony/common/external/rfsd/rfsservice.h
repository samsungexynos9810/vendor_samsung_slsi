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
    RFS service class
*/
#ifndef _rfs_service_h
#define _rfs_service_h

#include <cutils/properties.h>
#include <fstream>
#include <private/android_filesystem_config.h>
#include <pwd.h>
#include <strings.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "rfs.h"
#include "rfschannel.h"
#include "rfsfile.h"
#include "thread.h"

class  Thread;
class  CRfsIoChannel;
struct CRfsFile;

/* maximum number of open files */
#define RFS_MAX_HANDLES        1024
/* bad handle */
#define RFS_BAD_HANDLE        (-1)
/* buffer size */
#define RFS_MAX_READ_BUFFER    (2048 + 64)
/* channel name */
#define RFS_CHANNEL_NAME    "/dev/umts_rfs0"
/* property */
#define RFS_PROP_NV_CHECK_DONE    "vendor.ril.cbd.rfs_check_done"
/* switch user */
#define MAX_CAP_NUM         (CAP_TO_INDEX(CAP_LAST_CAP) + 1)
/* TOC */
#define RFS_PROP_MODEM_BIN_PATH         "vendor.sys.exynos.modempath"
#define RFS_PROP_MODEM_BIN_PATH_SUFFIX  "ro.boot.slot_suffix"
#define MODEM_FILE_BIN_NAME_LEN         128
#define TOC_MAX_SIZE                    512

struct tocbuffer {
    char LTE_tocBuffer[TOC_MAX_SIZE];
    char CDMA_tocBuffer[TOC_MAX_SIZE];
};

enum modem_index {
    MODEM_LTE,
    MODEM_CDMA
};

/* service class */
class CRfsService {
    /* I/O channel */
    CRfsIoChannel     *mIoChannel;
    /* Reader */
    Thread         *mReaderThread;
#if 0   // not use
    Thread         *mLoggingThread;
#endif

    /* Files */
    std::vector <CRfsFile*>    mFiles;

    /* TOC */
    char mModemBin[MODEM_FILE_BIN_NAME_LEN];
    tocbuffer mTocBuffer;

    /* Func */
    u32  GetFileId(u8 *buffer);
    void AllocFile(u32 index);
    void FreeFile(u32 index);
    int ReadToc(int index);
public:
    void CheckNvFiles();

public:
    CRfsService();
    ~CRfsService();

    int  OnModemBootDone();
    int  ProcessRfsPacket(char *data, int dataSize);
    int  Backup();
    int  Restore();
    void SwitchUser();
    int RecoverFileOwnership();
};

#endif
