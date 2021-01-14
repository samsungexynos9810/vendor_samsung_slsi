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
    RFS file class
*/
#ifndef _rfs_file_h
#define _rfs_file_h

#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <openssl/md5.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "rfs.h"
#include "rfschannel.h"

#define ERR2STR     strerror(errno)

/*
    Checksum defines
*/
#define RFS_NV_CSUM_LENGTH    32
#define RFS_NV_CSUM_FILEEXT    ".md5"
#define RFS_NV_BACKUP_FILEEXT  ".bak"

/*
    File states
*/
enum rfs_file_state {
    RFS_FILE_STATE_CLOSED,
    RFS_FILE_STATE_OPENED,
    RFS_FILE_STATE_OPENED_PROTECTED,
    RFS_FILE_STATE_OPENED_UNPROTECTED
};

/*
    Message processing response
*/
enum rfs_file_response {
    RFS_FILE_KEEP,
    RFS_FILE_DELETE_NOW
};

#define RFS_PATH_LEN    128

/* Modem TOC */
#define DEF_MODEM_FILE_BIN              "/dev/block/by-name/modem"
#define DEF_RFS_DIRECTORY_PATH          "/mnt/vendor/efs/"
#define DEF_RFS_BACKUP_FILE_NAME        "/mnt/vendor/efs/backup/nv_protected"
#define TOC_MAX_SIZE        512
#define TOC_MAX_NAME_LEN    12
#define TOC_RECORDS        (TOC_MAX_SIZE / sizeof(struct toc_record))

//Property
#define PROP_RFS_DIRECTORY_PATH         "vendor.ril.exynos.nvpath"
#define PROP_RFS_BACKUP_FILE_NAME       "vendor.ril.exynos.nvpath.backup"
#define CRASH_MODE_SYS_PROP             "persist.vendor.ril.crash_handling_mode"

#define SIGNATURE_SMI            "Samsung_SMI_RIL"
#define SIGNATURE_SIT            "Samsung_SIT_RIL"

/* TOC record */
struct toc_record {
    char name[TOC_MAX_NAME_LEN];    /* Image name */
    u32  file_offset;        /* File offset */
    u32  mem_offset;        /* Memory Offset */
    u32  size;            /* Binary size */
    u32  crc;            /* CRC value */
    u32  index;            /* Index */
} __packed;

/* TOC names */
static const char *toc_section_names[] = {
    "NV_NORM",
    "NONE",
    "NV_PROT"
};

/*
    Base RFS file class
*/
struct CRfsFile {
    /* Mains */
    u32        mId;
    int        mFd;
    u32        mNvSize;

    /* State and state machine helpers */
    int         mState;
    int         mLastIo;
    u32        mTransaction;

    /* read and write */
    u32        mWriteLength;    /* remained length */
    u32        mWriteOffset;
    u32        mReadLength;    /* remained length */
    u32        mReadOffset;

    /* Rfs channel - to send data */
    CRfsIoChannel     *mIoChannel;
//    char mModemBin[MODEM_FILE_BIN_NAME_LEN];
    char RFS_FILE_PATH[RFS_PATH_LEN];
    char RFS_BACKUP_FILE_PATH[RFS_PATH_LEN];

    /* TOC buffer */
    char tocBuffer[TOC_MAX_SIZE];

    /* Func */
    virtual void Open(u8 *buffer);
    virtual void Close(u8 *buffer);
    void Read();
    void ReadRequest(u32 offset, u32 length);
    void ReadResponse(u8* buffer);
    void WriteRequest(u32 offset, u32 length);
    void WriteResponse(u8* buffer);
    void Status(u32 tid, u32 error);
    void Status(u32 error) {Status(mTransaction, error);}
    u32  Size() {return lseek(mFd, 0, SEEK_END);}

    CRfsFile(u32 id, CRfsIoChannel *ioChannel, char *toc);
    virtual ~CRfsFile();

    virtual int ProcessMessage(u8 *buffer);
    virtual int Touch() {return 0;}
    virtual int Backup() {return 0;}
    virtual int Restore() {return 0;}
    virtual void OnWriteDone() {}
};

/*
    NV file class
*/
struct CNvFile : CRfsFile {
    u32        mSize;            /* file size */

    /* Func */
    virtual void Open(u8 *buffer);
    int  CsumCalc(int fd, char* csum, const char* signature);
    int  CsumUpdate();
    int  IsCsumCorrect();
    int  CheckAndUpdateCsum();

    CNvFile(u32 id, CRfsIoChannel *ioChannel, char *toc) : CRfsFile(id, ioChannel, toc) {mSize = 0;}
    virtual ~CNvFile();

    virtual int ProcessMessage(u8 *buffer);
    virtual int Touch();
    virtual void OnWriteDone();
};

/*
    Protected NV file class
*/
struct CNvProtectedFile : CRfsFile {
    u32        mSize;            /* file size */
    bool mBackupFlag = false;

    /* Func */
    virtual void Open(u8* buffer);
    virtual void Close(u8* buffer);
    void Protect(u8* buffer);
    void Unprotect(u8* buffer);

    void Protect();
    int  CsumCalc(int fd, char* csum, const char* signature);
    int  CsumUpdate();
    int  IsCsumCorrect();
    int  CheckAndUpdateCsum();
    int  IsBackupCsumCorrect();

    CNvProtectedFile(u32 id, CRfsIoChannel *ioChannel, char *toc) : CRfsFile(id, ioChannel, toc) {mSize = 0;}
    virtual ~CNvProtectedFile();

    virtual int ProcessMessage(u8 *buffer);
    virtual int Touch();
    virtual int Backup();
    virtual int Restore();
    virtual int EqualCsum();
    virtual void OnWriteDone();
};

#endif
