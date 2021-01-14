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
    RFS file class implementation
*/

#include "rfsfile.h"
#include <sys/ioctl.h>
#include <cutils/properties.h>

// NV checksum API
//#define FORCE_CRASH_BY_CSUM_ERROR

/* NV Data Header */
typedef struct
{
    unsigned int      UpdateCounter;
    unsigned int      NumberOfRecords;
    unsigned int      TotalSize;
    unsigned int      ElementOrdering;
} pal_RegFlashHeader_t;

static u16 PalRegistryDataCheckCalc(void* buf, u32 len)
{
    u16* ptr;
    u16 check = 0;
    u32 i;

    /*
     * Calculate the checksum of the data from AP.
     */
    ptr = (u16 *)( buf );
    for ( i = 0; i < (len >> 1); i++ ) {
        check += *ptr;
        ptr++;
    }

    /*
     * Sum a single byte if the number of bytes was not even.
     */
    if ( len & 1 ) {
        check += ( *ptr | 0xff00 );
    }

    return check;
}

static bool CsumCheckForData(u8* NvAddressPointer, unsigned int nvSize)
{
    u8* recordedCheckSum;
    pal_RegFlashHeader_t* NV_header;
    u16 CheckSum;

    NV_header = (pal_RegFlashHeader_t*)NvAddressPointer;

    NvAddressPointer += sizeof(pal_RegFlashHeader_t);

    if (NV_header->TotalSize == (unsigned int)(-1)) {
        // default value(All 0xFF)
        ALOGI("[%s] NV File is default.\n", __FUNCTION__);
        return true;
    }

    const unsigned int MAX_NV_SIZE = (1024 * 1024);
    if (NV_header->TotalSize > nvSize || nvSize > MAX_NV_SIZE ) {
        ALOGE("[%s] NV Size Error! NV_header->TotalSize:%u, nvSize:%u\n", __FUNCTION__, NV_header->TotalSize, nvSize);
        return false;
    }

    CheckSum = PalRegistryDataCheckCalc(NvAddressPointer, NV_header->TotalSize);

    recordedCheckSum = NvAddressPointer + NV_header->TotalSize;
    if ( NV_header->TotalSize & 1 ) {
        recordedCheckSum++;
    }

    if(CheckSum != (u16) *((u16*)recordedCheckSum)) {
        ALOGE("[%s] Check sum Error\n", __FUNCTION__);
        return false;
    }
    return true;
}

static bool CsumCheckForFileAndForceCrash(const char *filepath, unsigned int nvSize, bool forceCrash)
{
    if (filepath == NULL || nvSize == 0) {
        return false;
    }
    std::ifstream ifsNv;
    ifsNv.open(filepath, std::ios::binary | std::ios::ate);
    char* data = new char[nvSize];

    if (!ifsNv) {
        ALOGE("[%s] Failed to read %s", __FUNCTION__, filepath);
        delete[] data;
        return false;
    }

    ifsNv.seekg(0, ifsNv.beg);
    ifsNv.read(data, nvSize);
    ifsNv.close();
    bool ret = CsumCheckForData((u8*)data, nvSize);
    ALOGW("[%s] %s checksum : %s", __FUNCTION__, filepath, ret ? "verified" : "corrupted");
    delete[] data;

#ifdef FORCE_CRASH_BY_CSUM_ERROR
    ALOGW("[%s] forceCrash by NV corruption is %s", __FUNCTION__, forceCrash ? "enabled" : "disabled");
    if (!ret && forceCrash) {
        ALOGW("[%s] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", __FUNCTION__);
        ALOGW("[%s] Force CP Crash by %s corruption.", __FUNCTION__, filepath);
        ALOGW("[%s] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", __FUNCTION__);
        int fd = open("/dev/umts_ipc0", O_RDWR);
        if (fd > 0) {
            if (ioctl(fd, _IO('o', 0x34)) == -1) {
                ALOGW("[%s] error ioctl", __FUNCTION__);
            }
            close(fd);
        }
        else {
            ALOGW("[%s] Failed to open /dev/umts_ipc0.", __FUNCTION__);
        }
    }
#endif // FORCE_CRASH_BY_CSUM_ERROR

    return ret;
}

/**********************************************
    RFS base file class
***********************************************/
/* contructor */
CRfsFile::CRfsFile(u32 id, CRfsIoChannel *ioChannel, char *toc)
{
    /* save ID */
    mId = id;

    /* file closed */
    mFd = -1;

    /* closed and no I/O */
    mState     = RFS_FILE_STATE_CLOSED;
    mLastIo    = RFS_NONE;
    mTransaction = 0;

    mWriteOffset = -1;
    mReadOffset = -1;

    /* clear length */
    mWriteLength = 0;
    mReadLength  = 0;

    /* Rfs channel */
    mIoChannel = ioChannel;

    /* TOC */
    memcpy(tocBuffer, toc, TOC_MAX_SIZE);
    mNvSize = 0;
//    property_get(RFS_PROP_MODEM_BIN_PATH, mModemBin, DEF_MODEM_FILE_BIN);

    property_get(PROP_RFS_DIRECTORY_PATH, RFS_FILE_PATH, DEF_RFS_DIRECTORY_PATH);
    property_get(PROP_RFS_BACKUP_FILE_NAME, RFS_BACKUP_FILE_PATH, DEF_RFS_BACKUP_FILE_NAME);
}

/* destructor */
CRfsFile::~CRfsFile()
{
    if (mState != RFS_FILE_STATE_CLOSED) {
        close(mFd);
    }
}

/* process incoming request */
int CRfsFile::ProcessMessage(u8 *buffer)
{
    struct rfs_header *pHeader = (struct rfs_header*)buffer;

    ALOGV("[RfsService::File] %s", __FUNCTION__);

    if(pHeader->cmd == RFS_NV_BACKUP) {
        ALOGE("[RfsService::File] backup not supported!");
        return RFS_FILE_KEEP;
    }

    /* check transaction ID */
    if ((mTransaction != pHeader->tid) &&
        (mLastIo != RFS_NONE) &&
        (pHeader->cmd != RFS_OP_STATUS)) {
        /* new command received before previous was finished */
        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

        ALOGV("[RfsService::File] New tid received = %d(%d) before prev. was completed", pHeader->cmd, pHeader->tid);
        return RFS_FILE_KEEP;
    }

    /* check for pending I/O */
    if ((mLastIo != RFS_NONE) &&
        (pHeader->cmd != RFS_READ) &&
        (pHeader->cmd != RFS_WRITE) &&
        (pHeader->cmd != RFS_OP_STATUS)) {
        /* pending I/O exists */
        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

        ALOGV("[RfsService::File] New request received = %d(%d) before prev. was completed", pHeader->cmd, pHeader->tid);
        return RFS_FILE_KEEP;
    }

    buffer += sizeof(struct rfs_header);

    /* state check */
    switch (mState) {
        case RFS_FILE_STATE_CLOSED:
        {
            /* only open command allowed */
            switch (pHeader->cmd) {
                case RFS_OPEN:
                {
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Open(buffer);
                    break;
                }
                case RFS_OP_STATUS:
                {
                    struct rfs_status *pStatus = (struct rfs_status*)buffer;

                    /* file check */
                    if (mId != pStatus->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::File] Wrong file ID %d", pStatus->file);
                        return RFS_FILE_KEEP;
                    }

                    /* TID is same - I/O completion */
                    if (mTransaction == pHeader->tid) {
                        /* check status */
                        if (pStatus->status == RFS_STATUS_SUCCESS) {
                            /* file opened successfully */
                            if (mLastIo == RFS_OPEN) {
                                mState = RFS_FILE_STATE_OPENED;
                            }
                        }
                        mLastIo = RFS_NONE;
                    }
                    /* else - nothing, drop response */
                    break;
                }
                default:
                {
                    /* incorrect command for this state */
                    Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                    ALOGE("[RfsService::File] Bad command %d(%d) for CLOSE", pHeader->cmd, pHeader->tid);
                    return RFS_FILE_KEEP;
                }
            }
            break;
        }
        default:
        {
            /* open state */
            switch (pHeader->cmd) {
                case RFS_CLOSE:
                {
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Close(buffer);
                    break;
                }
                case RFS_IO_REQUEST:
                {
                    struct rfs_request_ind *pIoReq = (struct rfs_request_ind*)buffer;

                    /* save TID */
                    mTransaction = pHeader->tid;

                    /* file check */
                    if (mId != pIoReq->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::File] Wrong file ID %d", pIoReq->file);
                        return RFS_FILE_KEEP;
                    }

                    /* check I/O request */
                    switch (pIoReq->io) {
                        case RFS_IO_READ: ReadRequest(pIoReq->offset, pIoReq->length); break;
                        case RFS_IO_WRITE: WriteRequest(pIoReq->offset, pIoReq->length); break;
                        default:
                        {
                            /* incorrect I/O */
                            Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                            ALOGE("[RfsService::File] Bad I/O request %d", pIoReq->io);
                            return RFS_FILE_KEEP;
                        }
                    }
                    break;
                }
                case RFS_READ:
                {
                    ReadResponse(buffer);
                    break;
                }
                case RFS_WRITE:
                {
                    WriteResponse(buffer);
                    break;
                }
                case RFS_OP_STATUS:
                {
                    struct rfs_status *pStatus = (struct rfs_status*)buffer;

                    /* file check */
                    if (mId != pStatus->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::File] Wrong file ID %d", pStatus->file);
                        return RFS_FILE_KEEP;
                    }

                    /* TID is same - I/O completion */
                    if (mTransaction == pHeader->tid) {
                        /* check status */
                        if (pStatus->status == RFS_STATUS_SUCCESS) {
                            /* file closed successfully */
                            if (mLastIo == RFS_CLOSE) {
                                mState = RFS_FILE_STATE_CLOSED;
                            }
                        }
                        mLastIo = RFS_NONE;
                    }
                    /* else - nothing, drop response */
                    break;
                }
                default:
                {
                    /* incorrect command for this state */
                    Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                    ALOGE("[RfsService::File] Bad command %d(%d) for OPEN", pHeader->cmd, pHeader->tid);
                    return RFS_FILE_KEEP;
                }
            }
        }
    }

    /* remove file */
    if ((mState == RFS_FILE_STATE_CLOSED) &&
        (mLastIo == RFS_NONE)) {
        return RFS_FILE_DELETE_NOW;
    }

    return RFS_FILE_KEEP;
}

/* open file */
void CRfsFile::Open(u8 *buffer)
{
    struct rfs_open_ind *pOpen = (struct rfs_open_ind*)buffer;

    ALOGV("[RfsService::File] %s", __FUNCTION__);

    /* file check */
    if (mId != pOpen->file) {
        /* wrong file ID */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::File] Wrong file ID %d", pOpen->file);
        return;
    }

    /* set I/O */
    mLastIo = RFS_OPEN;

    std::string name(RFS_FILE_PATH);
    name += (char*)(buffer + sizeof(struct rfs_open_ind));

    /* open or create file */
    mFd = open(name.c_str(), O_RDWR);
    if((mFd < 0) && (errno == ENOENT)) {
        std::string dir = name;
        size_t found = dir.find_last_of("/");
        if(found != std::string::npos) {
            dir = dir.substr(0,found);
            if (mkdir(dir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) != 0)
                ALOGE("%s : mkdir error(%s)", __FUNCTION__, strerror(errno));
            ALOGV("[RfsService::File] dir name %s", dir.c_str());
            mFd = open(name.c_str(), O_CREAT | O_RDWR | O_APPEND, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
        }
    }

    if (mFd < 0) {
        /* failed to open file */
        Status(RFS_STATUS_OPEN_FAIL);
        ALOGE("[RfsService::File] Failed to open file %s (%d) (reason:%s)", name.c_str(), mId, ERR2STR);
        return;
    }

    /* send status */
    Status(RFS_STATUS_SUCCESS);
}

/* close file */
void CRfsFile::Close(u8 *buffer)
{
    struct rfs_close_ind *pClose = (struct rfs_close_ind*)buffer;

    ALOGV("[RfsService::File] %s", __FUNCTION__);

    /* file check */
    if (mId != pClose->file) {
        /* wrong file ID */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::File] Wrong file ID %d", pClose->file);
        return;
    }

    /* close */
    close(mFd);

    /* set I/O */
    mLastIo = RFS_CLOSE;

    /* send status */
    Status(RFS_STATUS_SUCCESS);
}

/* read file */
void CRfsFile::Read()
{
    ALOGV("[RfsService::File] %s", __FUNCTION__);

    ALOGV("[RfsService::File] read %d bytes from %d", mReadLength, mReadOffset);

    /* calculate frame read size */
    u32 length = (mReadLength > RFS_FRAME_DATA_PAYLOAD_SIZE) ? RFS_FRAME_DATA_PAYLOAD_SIZE : mReadLength;

    /* send response */
    int allocLen = sizeof(struct rfs_header) + sizeof(struct rfs_read) + length;
    u8 *pBuf = new u8[allocLen];
    if (pBuf == NULL) {
        Status(RFS_STATUS_READ_FAIL);

        ALOGE("[RfsService::File] Failed to allocate %d bytes", allocLen);
        return;
    }

    struct rfs_header *pHeader = (struct rfs_header*)pBuf;
    pHeader->cmd = RFS_READ;
    pHeader->tid = mTransaction;
    pHeader->len = allocLen - (int)sizeof(struct rfs_header);

    struct rfs_read *pRead = (struct rfs_read*)(pBuf + sizeof(struct rfs_header));
    pRead->file   = mId;
    pRead->offset = mReadOffset;

    int readLen = read(mFd, pRead->data, length);
    if (readLen > 0) {
        pRead->length = readLen;

        /* calculate remained */
        mReadLength -= readLen;
        mReadOffset += readLen;

        /* recalculate total length */
        if ((u32)readLen != length) {
            pHeader->len -= (length - readLen);
            allocLen -= (length - readLen);
        }
    } else {
        delete[] pBuf;

        Status(RFS_STATUS_READ_FAIL);

        ALOGE("[RfsService::File] Failed to read file");
        return;
    }

    mIoChannel->Write((char*)pBuf, allocLen);

    delete[] pBuf;
}

/* read file request */
void CRfsFile::ReadRequest(u32 offset, u32 length)
{
    ALOGI("[RfsService::File] %s", __FUNCTION__);

    /* save request */
    mReadLength = length;
    mReadOffset = offset;

    /* move pointer */
    int err = lseek(mFd, mReadOffset, SEEK_SET);
    if (err < 0) {
        Status(RFS_STATUS_READ_FAIL);

        ALOGE("[RfsService::File] Failed to change file offset (%d)", mReadOffset);
        return;
    }

    /* set I/O */
    mLastIo = RFS_READ;

    /* read */
    Read();
}

/* read response */
void CRfsFile::ReadResponse(u8* buffer)
{
    ALOGV("[RfsService::File] %s", __FUNCTION__);

    struct rfs_read_res *pReadRes = (struct rfs_read_res*)buffer;

    /* response check */
    if (mLastIo != RFS_READ) {
        /* wrong response */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGV("[RfsService::File] READ response for %d", mLastIo);
        return;
    }

    /* file check */
    if (mId != pReadRes->file) {
        /* wrong file ID */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::File] Wrong file ID %d", pReadRes->file);
        return;
    }

    /* status check */
    if (pReadRes->status != RFS_STATUS_SUCCESS) {
        ALOGW("[RfsService::File] Unsuccessful status %d", pReadRes->status);

        /* done */
        mLastIo = RFS_NONE;
        return;
    }

    if (mReadLength) {
        /* read */
        Read();
    } else {
        /* done */
        mLastIo = RFS_NONE;
    }
}

/* write request response */
void CRfsFile::WriteRequest(u32 offset, u32 length)
{
    ALOGV("[RfsService::File] %s", __FUNCTION__);

    /* save request */
    mWriteLength = length;
    mWriteOffset = offset;

    /* move pointer */
    int err = lseek(mFd, mWriteOffset, SEEK_SET);
    if (err < 0) {
        Status(RFS_STATUS_WRITE_FAIL);

        ALOGE("[RfsService::File] Failed to change file offset (%d)", offset);
        return;
    }

    /* set I/O */
    mLastIo = RFS_WRITE;

    ALOGV("[RfsService::File] write request %d bytes from %d", length, offset);

    /* send response */
    int allocLen = sizeof(struct rfs_header) + sizeof(struct rfs_write);
    u8 *pBuf = new u8[allocLen];
    if (pBuf == NULL) {
        Status(RFS_STATUS_WRITE_FAIL);

        ALOGE("[RfsService::File] Failed to allocate %d bytes", allocLen);
        return;
    }

    struct rfs_header *pHeader = (struct rfs_header*)pBuf;
    pHeader->cmd = RFS_WRITE;
    pHeader->tid = mTransaction;
    pHeader->len = sizeof(struct rfs_write);

    struct rfs_write *pWrite = (struct rfs_write*)(pBuf + sizeof(struct rfs_header));
    pWrite->file   = mId;
    pWrite->offset = mWriteOffset;
    pWrite->length = (mWriteLength > RFS_FRAME_DATA_PAYLOAD_SIZE) ? RFS_FRAME_DATA_PAYLOAD_SIZE : mWriteLength;

    mIoChannel->Write((char*)pBuf, allocLen);

    delete[] pBuf;
}

/* write file */
void CRfsFile::WriteResponse(u8* buffer)
{
    ALOGV("[RfsService::File] %s", __FUNCTION__);

    /* response check */
    if (mLastIo != RFS_WRITE) {
        /* wrong response */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::File] WRITE response for %d", mLastIo);
        return;
    }

    struct rfs_write_res *pWriteRes = (struct rfs_write_res*)buffer;

    /* file check */
    if (mId != pWriteRes->file) {
        /* wrong file ID */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::File] Wrong file ID %d", pWriteRes->file);
        return;
    }

    /* status check */
    if (pWriteRes->status != RFS_STATUS_SUCCESS) {
        ALOGW("[RfsService::File] Unsuccessful status %d", pWriteRes->status);

        /* done */
        mLastIo = RFS_NONE;
        return;
    }

    /* length check */
    if (mWriteLength < pWriteRes->length) {
        /* size is too big */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::File] Too big size for write (%d < %d)", mWriteLength, pWriteRes->length);
        return;
    }

    ALOGV("[RfsService::File] write %d bytes, remained %d", pWriteRes->length, mWriteLength - pWriteRes->length);

    /* write */
    int err = write(mFd, pWriteRes->data, pWriteRes->length);
    if ((u32)err != pWriteRes->length) {
        /* write error */
        Status(RFS_STATUS_WRITE_FAIL);

        ALOGE("[RfsService::File] Failed to write data");
        return;
    }

    /* calculate remained */
    mWriteLength -= pWriteRes->length;
    mWriteOffset += pWriteRes->length;

    /* continue */
    if (mWriteLength) {
        /* don't need to move pointer */
        /* send response */
        int allocLen = sizeof(struct rfs_header) + sizeof(struct rfs_write);
        u8 *pBuf = new u8[allocLen];
        if (pBuf == NULL) {
            Status(RFS_STATUS_WRITE_FAIL);

            ALOGE("[RfsService::File] Failed to allocate %d bytes", allocLen);
            return;
        }

        struct rfs_header *pHeader = (struct rfs_header*)pBuf;
        pHeader->cmd = RFS_WRITE;
        pHeader->tid = mTransaction;
        pHeader->len = sizeof(struct rfs_write);

        struct rfs_write *pWrite = (struct rfs_write*)(pBuf + sizeof(struct rfs_header));
        pWrite->file   = mId;
        pWrite->offset = mWriteOffset;
        pWrite->length = (mWriteLength > RFS_FRAME_DATA_PAYLOAD_SIZE) ? RFS_FRAME_DATA_PAYLOAD_SIZE : mWriteLength;

        mIoChannel->Write((char*)pBuf, allocLen);

        delete[] pBuf;
    } else {
        /* done */
        mLastIo = RFS_NONE;
        fsync(mFd);

        // post event
        OnWriteDone();

        /* send status */
        Status(RFS_STATUS_SUCCESS);
    }
}

/* send status */
void CRfsFile::Status(u32 tid, u32 error)
{
    ALOGV("[RfsService::File] %s", __FUNCTION__);

    int allocLen = sizeof(struct rfs_header) + sizeof(struct rfs_status);
    u8 *pBuf = new u8[allocLen];
    if (pBuf == NULL) {
        ALOGE("[RfsService::File] Failed to allocate %d bytes", allocLen);
        return;
    }

    struct rfs_header *pHeader = (struct rfs_header*)pBuf;
    pHeader->cmd = RFS_OP_STATUS;
    pHeader->tid = tid;
    pHeader->len = sizeof(struct rfs_status);

    struct rfs_status *pStatus = (struct rfs_status*)(pBuf + sizeof(struct rfs_header));
    pStatus->file   = mId;
    pStatus->status = error;

    mIoChannel->Write((char*)pBuf, allocLen);

    delete[] pBuf;
}


/**********************************************
    NV basic file class
***********************************************/
/* destructor */
CNvFile::~CNvFile()
{
    /* nothing to do */
}

/* process incoming request */
int CNvFile::ProcessMessage(u8 *buffer)
{
    struct rfs_header *pHeader = (struct rfs_header*)buffer;

    ALOGV("[RfsService::Nv] %s", __FUNCTION__);
    if(pHeader->cmd == RFS_NV_BACKUP) {
        ALOGW("[RfsService::Nv] backup not supported!");
        return RFS_FILE_KEEP;
    }

    /* check transaction ID */
    if ((mTransaction != pHeader->tid) &&
        (mLastIo != RFS_NONE) &&
        (pHeader->cmd != RFS_OP_STATUS)) {
        /* new command received before previous was finished */
        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

        ALOGV("[RfsService::Nv] New tid recived  = %d(%d) before prev. was completed", pHeader->cmd, pHeader->tid);
        return RFS_FILE_KEEP;
    }

    /* check for pending I/O */
    if ((mLastIo != RFS_NONE) &&
        (pHeader->cmd != RFS_READ) &&
        (pHeader->cmd != RFS_WRITE) &&
        (pHeader->cmd != RFS_OP_STATUS)) {
        /* pending I/O exists */
        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

        ALOGV("[RfsService::Nv] New request received = %d(%d) before prev. was completed", pHeader->cmd, pHeader->tid);
        return RFS_FILE_KEEP;
    }

    buffer += sizeof(struct rfs_header);

    /* state check */
    switch (mState) {
        case RFS_FILE_STATE_CLOSED:
        {
            switch (pHeader->cmd) {
                case RFS_OPEN:
                {
                    ALOGI("[RfsService::Nv] %s : RFS_OPEN", __FUNCTION__);
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Open(buffer);
                    break;
                }
                case RFS_IO_REQUEST:
                {
                    ALOGI("[RfsService::Nv] %s : RFS_IO_REQUEST", __FUNCTION__);
                    struct rfs_request_ind *pIoReq = (struct rfs_request_ind*)buffer;

                    /* save TID */
                    mTransaction = pHeader->tid;

                    /* file check */
                    if (mId != pIoReq->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::Nv] Wrong file ID %d", pIoReq->file);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Wrong file ID %d", pIoReq->file);
                        return RFS_FILE_KEEP;
                    }

                    std::string name(RFS_FILE_PATH);
                    name += rfs_nv_file_names[mId - 1];

                    /* open file */
                    mFd = open(name.c_str(), O_RDWR);
                    if (mFd < 0) {
                        /* failed to open file */
                        Status(RFS_STATUS_OPEN_FAIL);

                        ALOGE("[RfsService::Nv] Failed to open file %s (%d)", name.c_str(), mId);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to open file %s (%d)", name.c_str(), mId);
                        return RFS_FILE_KEEP;
                    }

                    /* change state */
                    mState = RFS_FILE_STATE_OPENED;

                    /* get file size */
                    mSize = Size();

                    /* do not allow exceed file size */
                    if (mSize < (pIoReq->offset + pIoReq->length)) {
                            /* bad I/O */
                            Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                            ALOGE("[RfsService::Nv] Bad I/O request %d, from %d, %d bytes", pIoReq->io, pIoReq->offset, pIoReq->length);
                            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Bad I/O request %d, from %d, %d bytes", pIoReq->io, pIoReq->offset, pIoReq->length);
                            return RFS_FILE_KEEP;
                    }

                    /* check I/O request */
                    switch (pIoReq->io) {
                        case RFS_IO_READ: ReadRequest(pIoReq->offset, pIoReq->length); break;
                        case RFS_IO_WRITE: WriteRequest(pIoReq->offset, pIoReq->length); break;
                        default:
                        {
                            /* incorrect I/O */
                            Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                            ALOGE("[RfsService::Nv] Bad I/O request %d", pIoReq->io);
                            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Bad I/O request %d", pIoReq->io);
                            return RFS_FILE_KEEP;
                        }
                    }
                    break;
                }
                case RFS_OP_STATUS:
                {
                    ALOGI("[RfsService::Nv] %s : RFS_OP_STATUS", __FUNCTION__);
                    struct rfs_status *pStatus = (struct rfs_status*)buffer;

                    /* file check */
                    if (mId != pStatus->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::Nv] Wrong file ID %d", pStatus->file);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Wrong file ID %d", pStatus->file);
                        return RFS_FILE_KEEP;
                    }

                    /* TID is same - I/O completion */
                    if (mTransaction == pHeader->tid) {
                        /* check status */
                        if (pStatus->status == RFS_STATUS_SUCCESS) {
                            ALOGI("[RfsService::Nv] %s : RFS_STATUS_SUCCESS", __FUNCTION__);
                            /* file opened successfully */
                            if (mLastIo == RFS_OPEN) {
                                mState = RFS_FILE_STATE_OPENED;
                            }
                        }
                        mLastIo = RFS_NONE;
                    }
                    /* else - nothing, drop response */
                    break;
                }
                default:
                {
                    /* incorrect command for this state */
                    Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                    ALOGE("[RfsService::Nv] Bad command %d(%d) for CLOSE", pHeader->cmd, pHeader->tid);
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Bad command %d(%d) for CLOSE", pHeader->cmd, pHeader->tid);
                    return RFS_FILE_KEEP;
                }
            }
            break;
        }
        default:
        {
            /* open state */
            switch (pHeader->cmd) {
                case RFS_CLOSE:
                {
                    ALOGI("[RfsService::Nv] %s : RFS_CLOSE", __FUNCTION__);
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Close(buffer);
                    break;
                }
                case RFS_IO_REQUEST:
                {
                    ALOGI("[RfsService::Nv] %s : RFS_IO_REQUEST", __FUNCTION__);
                    struct rfs_request_ind *pIoReq = (struct rfs_request_ind*)buffer;

                    /* save TID */
                    mTransaction = pHeader->tid;

                    /* file check */
                    if (mId != pIoReq->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::Nv] Wrong file ID %d", pIoReq->file);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Wrong file ID %d", pIoReq->file);
                        return RFS_FILE_KEEP;
                    }

                    /* do not allow exceed file size */
                    if (mSize < (pIoReq->offset + pIoReq->length)) {
                            /* bad I/O */
                            Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                            ALOGE("[RfsService::Nv] Bad I/O request %d, from %d, %d bytes", pIoReq->io, pIoReq->offset, pIoReq->length);
                            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Bad I/O request %d, from %d, %d bytes", pIoReq->io, pIoReq->offset, pIoReq->length);
                            return RFS_FILE_KEEP;
                    }

                    /* check I/O request */
                    switch (pIoReq->io) {
                        case RFS_IO_READ: ReadRequest(pIoReq->offset, pIoReq->length); break;
                        case RFS_IO_WRITE: WriteRequest(pIoReq->offset, pIoReq->length); break;
                        default:
                        {
                            /* incorrect I/O */
                            Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                            ALOGE("[RfsService::Nv] Bad I/O request %d", pIoReq->io);
                            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Bad I/O request %d", pIoReq->io);
                            return RFS_FILE_KEEP;
                        }
                    }
                    break;
                }
                case RFS_READ:
                {
                    ReadResponse(buffer);
                    break;
                }
                case RFS_WRITE:
                {
                    WriteResponse(buffer);
                    break;
                }
                case RFS_OP_STATUS:
                {
                    ALOGI("[RfsService::Nv] %s : RFS_OP_STATUS", __FUNCTION__);
                    struct rfs_status *pStatus = (struct rfs_status*)buffer;

                    /* file check */
                    if (mId != pStatus->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::Nv] Wrong file ID %d", pStatus->file);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Wrong file ID %d", pStatus->file);
                        return RFS_FILE_KEEP;
                    }

                    /* TID is same - I/O completion */
                    if (mTransaction == pHeader->tid) {
                        /* check status */
                        if (pStatus->status == RFS_STATUS_SUCCESS) {
                            ALOGI("[RfsService::Nv] %s : RFS_STATUS_SUCCESS", __FUNCTION__);
                            /* file closed successfully */
                            if (mLastIo == RFS_CLOSE) {
                                mState = RFS_FILE_STATE_CLOSED;
                            }
                        }
                        mLastIo = RFS_NONE;
                    }
                    /* else - nothing, drop response */
                    break;
                }
                default:
                {
                    /* incorrect command for this state */
                    Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                    ALOGE("[RfsService::Nv] Bad command %d(%d) for OPEN", pHeader->cmd, pHeader->tid);
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Bad command %d(%d) for OPEN", pHeader->cmd, pHeader->tid);
                    return RFS_FILE_KEEP;
                }
            }
        }
    }

    /* remove file */
    if ((mState == RFS_FILE_STATE_CLOSED) &&
        (mLastIo == RFS_NONE)) {
        return RFS_FILE_DELETE_NOW;
    }

    return RFS_FILE_KEEP;
}

/* open file */
void CNvFile::Open(u8 *buffer)
{
    struct rfs_open_ind *pOpen = (struct rfs_open_ind*)buffer;

    ALOGV("[RfsService::Nv] %s", __FUNCTION__);

    /* file check */
    if (mId != pOpen->file) {
        /* wrong file ID */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::Nv] Wrong file ID %d", pOpen->file);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Wrong file ID %d", pOpen->file);
        return;
    }

    /* set I/O */
    mLastIo = RFS_OPEN;

    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];

    /* open file */
    mFd = open(name.c_str(), O_RDWR);
    if (mFd < 0) {
        /* failed to open file */
        Status(RFS_STATUS_OPEN_FAIL);

        ALOGE("[RfsService::Nv] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        return;
    }

    /* get file size */
    mSize = Size();

    /* send status */
    Status(RFS_STATUS_SUCCESS);
}

/* calculate MD5 checksum */
int CNvFile::CsumCalc(int fd, char* csum, const char* signature)
{
    ALOGI("[RfsService::Nv] %s", __FUNCTION__);

    MD5_CTX md5;
    MD5_Init(&md5);

    u8 digest[16] = {0};
    u8 buf[1024];
    int  rd = 0;

    lseek(fd, 0, SEEK_SET);
    do {
        rd = read(fd, buf, 1024);
        if (rd < 0) {
            ALOGE("[RfsService::Nv] Failed to read file(%s)", ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to read file(%s)", ERR2STR);
            return (-1);
        }
        MD5_Update(&md5, buf, rd);
    } while (rd);

//    const char* signature = "Samsung_SIT_RIL";
    MD5_Update(&md5, signature, strlen(signature));
    MD5_Final(digest, &md5);

    // done
    for (int i = 0; i < 16; i++) {
        snprintf(csum+(i*2), 3, "%02x", digest[i]);
    }
    return 0;
}

/* update checksum file */
int CNvFile::CsumUpdate()
{
    ALOGI("[RfsService::Nv] %s", __FUNCTION__);

    char csum[RFS_NV_CSUM_LENGTH + 1];
    memset(csum, 0, RFS_NV_CSUM_LENGTH + 1);

    /* calculate */
    if (mFd < 0) {
        std::string name(RFS_FILE_PATH);
        name += rfs_nv_file_names[mId - 1];

        int fd = open(name.c_str(), O_RDONLY);
        if (fd < 0) {
            ALOGE("[RfsService::Nv] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            return (-1);
        }

        CsumCalc(fd, csum, SIGNATURE_SIT);

        close(fd);
    } else {
        CsumCalc(mFd, csum, SIGNATURE_SIT);
    }

    /* write to file */
    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];
    name += RFS_NV_CSUM_FILEEXT;

    std::ofstream ofs(name.c_str(), std::ofstream::trunc);
    if (!ofs) {
        ALOGE("[RfsService::Nv] Failed to open %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to open %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        return (-2);
    }
    ofs.write(csum, RFS_NV_CSUM_LENGTH);
    if (!ofs) {
        ALOGE("[RfsService::Nv] Failed to write %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to write %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        ofs.close();
        return (-3);
    }
    ofs.close();

    return 0;
}

/* check file consistency */
int CNvFile::IsCsumCorrect()
{
    ALOGI("[RfsService::Nv] %s", __FUNCTION__);

    char csumNew[RFS_NV_CSUM_LENGTH + 1];
    char csumNewSmi[RFS_NV_CSUM_LENGTH + 1];
    memset(csumNew, 0, RFS_NV_CSUM_LENGTH + 1);
    memset(csumNewSmi, 0, RFS_NV_CSUM_LENGTH + 1);

    /* calculate */
    if (mFd < 0) {
        std::string name(RFS_FILE_PATH);
        name += rfs_nv_file_names[mId - 1];

        int fd = open(name.c_str(), O_RDONLY);
        if (fd < 0) {
            ALOGE("[RfsService::Nv] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            return (-1);
        }

        CsumCalc(fd, csumNew, SIGNATURE_SIT);
        CsumCalc(fd, csumNewSmi, SIGNATURE_SMI);

        close(fd);
    } else {
        CsumCalc(mFd, csumNew, SIGNATURE_SIT);
        CsumCalc(mFd, csumNewSmi, SIGNATURE_SMI);
    }

    /* read checksum file */
    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];
    name += RFS_NV_CSUM_FILEEXT;

    std::ifstream ifs(name.c_str());
    if (!ifs) {
        ALOGE("[RfsService::Nv] Failed to open %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to open %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        return (-2);
    }

    char csumFile[RFS_NV_CSUM_LENGTH + 1];
    memset(csumFile, 0, RFS_NV_CSUM_LENGTH + 1);

    ifs.read(csumFile, RFS_NV_CSUM_LENGTH);
    if (!ifs) {
        ALOGE("[RfsService::Nv] Failed to read %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to read %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        ifs.close();
        return (-3);
    }
    ifs.close();

    /* compare */
    if (strncmp(csumNew, csumFile, RFS_NV_CSUM_LENGTH)) {
        ALOGD("[RfsService::Nv] Checksum(SIT) mismatch for %s file", rfs_nv_file_names[mId - 1]);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Checksum(SIT) mismatch for %s file", rfs_nv_file_names[mId - 1]);
        if (strncmp(csumNewSmi, csumFile, RFS_NV_CSUM_LENGTH)) {
            ALOGE("[RfsService::Nv] Checksum(SMI) mismatch for %s file", rfs_nv_file_names[mId - 1]);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Checksum(SMI) mismatch for %s file", rfs_nv_file_names[mId - 1]);
            return 1;
        }
    }

    return 0;
}

/* check file */
int CNvFile::Touch()
{
    ALOGI("[RfsService::Nv] %s", __FUNCTION__);

    /* check TOC */
    struct toc_record *pToc = (struct toc_record*)tocBuffer;
    if (strcmp(pToc->name, "TOC")) {
        ALOGE("[RfsService::Nv] TOC record is broken");
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] TOC record is broken");
        return (-3);
    }

    /* get requested size */
    u32 tocSize = 0;
    u32 tocRecords = TOC_RECORDS;
    for (u32 i = 0; i < tocRecords; i++, pToc++) {
        if (!strcmp(pToc->name, toc_section_names[mId - 1])) {
            tocSize = pToc->size;
            break;
        }
    }
    if (tocSize == 0) {
        ALOGE("[RfsService::Nv] No %s record in TOC", toc_section_names[mId - 1]);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] No %s record in TOC", toc_section_names[mId - 1]);
        return (-4);
    }
    mNvSize = tocSize;

    /* open file */
    std::string nvPath(RFS_FILE_PATH);
    std::ifstream ifsNv;
    std::ofstream ofsNv;

    nvPath += rfs_nv_file_names[mId - 1];
    ifsNv.open(nvPath.c_str(), std::ios::binary | std::ios::ate);
    if (ifsNv && !IsCsumCorrect()) {
        /* file exists, check data checksum */
        char* data = new char[tocSize];
        /* read */
        ifsNv.seekg(0, ifsNv.beg);
        ifsNv.read(data, tocSize);
        if (!ifsNv) {
            ALOGE("[RfsService::Nv] Failed to read %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to read %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        }

        if (!CsumCheckForData((u8*)data, mNvSize)) {
            ALOGW("[RfsService::Nv] NV data checksum is broken. Recover NV data.");

            /* prepare buffer */
            char* dataBuf = new char[tocSize];
            if (dataBuf == NULL) {
                ALOGE("[RfsService::Nv] Failed to allocate data buffer");
                return (-8);
            }
            memset(dataBuf, 0xFF, tocSize);

            /* create file */
            ofsNv.open(nvPath.c_str(), std::ofstream::binary | std::ofstream::trunc);
            if (!ofsNv) {
                ALOGE("[RfsService::Nv] Failed to open file for output(%s)", ERR2STR);
                CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to open file for output(%s)", ERR2STR);
                delete[] dataBuf;
                return (-10);
            }

            /* write */
            ofsNv.write(dataBuf, tocSize);
            if (!ofsNv) {
                ALOGE("[RfsService::Nv] Failed to write file(%s)", ERR2STR);
                CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to write file(%s)", ERR2STR);
                delete[] dataBuf;
                ofsNv.close();
                return (-11);
            }

            delete[] data;
            delete[] dataBuf;
            ofsNv.close();

            /* read/write by owner, read only for everyone */
            mode_t fm = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            chmod(nvPath.c_str(), fm);
        }
        else {
            ALOGE("[RfsService::Nv] %s : Verified", nvPath.c_str());
            delete[] data;
        }

        /* file exists, check size*/
        u32 size = ifsNv.tellg();
        if (size < tocSize) {
            ifsNv.close();

            /* expand */
            ALOGD("[RfsService::Nv] File %s smaller (%d bytes) than TOC size (%d bytes). Expand", rfs_nv_file_names[mId - 1],
                size, tocSize);

            u32 tailroom = tocSize - size;

            char* wrBuf = new char[tailroom];
            if (wrBuf == NULL) {
                ALOGE("[RfsService::Nv] Failed to allocate write memory buffer");
                CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to allocate write memory buffer");
                return (-5);
            }
            memset(wrBuf, 0xFF, tailroom);

            ofsNv.open(nvPath.c_str(), std::ofstream::binary | std::ios::app);
            if (!ofsNv) {
                ALOGE("[RfsService::Nv] Failed to open file for output(%s)", ERR2STR);
                CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to open file for output(%s)", ERR2STR);
                delete[] wrBuf;
                return (-6);
            }

            ofsNv.write(wrBuf, tailroom);
            if (!ofsNv) {
                ALOGE("[RfsService::Nv] Failed to write file(%s)", ERR2STR);
                CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to write file(%s)", ERR2STR);
                delete[] wrBuf;
                ofsNv.close();
                return (-7);
            }

            delete[] wrBuf;
            ofsNv.close();
        } else {
            ifsNv.close();
        }
    } else {
        /* no file */
        ALOGI("[RfsService::Nv] Create %s. Size = %d bytes", rfs_nv_file_names[mId - 1], tocSize);

        /* prepare buffer */
        char* dataBuf = new char[tocSize];
        if (dataBuf == NULL) {
            ALOGE("[RfsService::Nv] Failed to allocate data buffer");
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to allocate data buffer");
            return (-8);
        }
        memset(dataBuf, 0xFF, tocSize);

        /* create file */
        ofsNv.open(nvPath.c_str(), std::ofstream::binary | std::ofstream::trunc);
        if (!ofsNv) {
            ALOGE("[RfsService::Nv] Failed to open file for output(%s)", ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to open file for output(%s)", ERR2STR);
            delete[] dataBuf;
            return (-10);
        }

        /* write */
        ofsNv.write(dataBuf, tocSize);
        if (!ofsNv) {
            ALOGE("[RfsService::Nv] Failed to write file(%s)", ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Failed to write file(%s)", ERR2STR);
            delete[] dataBuf;
            ofsNv.close();
            return (-11);
        }

        delete[] dataBuf;
        ofsNv.close();

        /* read/write by owner, read only for everyone */
        mode_t fm = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        chmod(nvPath.c_str(), fm);
    }

    /* done */
    ALOGI("[RfsService::Nv] %s file is ok", rfs_nv_file_names[mId - 1]);
    return 0;
}

int CNvFile::CheckAndUpdateCsum() {

    bool enableCrash = false;
#ifdef FORCE_CRASH_BY_CSUM_ERROR
    const int defVal = 1;
    int mode = property_get_int32(CRASH_MODE_SYS_PROP, defVal);
    // 0 : kernel panic after dump
    // 1 : silent reset after dump
    // 2 : only silent reset
    enableCrash = (mode == 0);
#endif // FORCE_CRASH_BY_CSUM_ERROR

    std::string nvPath(RFS_FILE_PATH);
    nvPath += rfs_nv_file_names[mId - 1];
    if (::CsumCheckForFileAndForceCrash(nvPath.c_str(), mNvSize, enableCrash)) {
        CsumUpdate();
    }
    else {
        ALOGE("[RfsService::Nv] %s : Do not create md5 for recovery in next CP booting.", __FUNCTION__);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::Nv] Do not create md5 for recovery in next CP booting.");
    }
    return 0;
}

void CNvFile::OnWriteDone()
{
    ALOGI("CNvFile::OnWriteDone");
    /* write completed, do some work */
    /* update checksum */
    /* make md5 for nv normal after write */
    CheckAndUpdateCsum();
    usleep(100000);
}


/**********************************************
    NV protected file class
***********************************************/
/* destructor */
CNvProtectedFile::~CNvProtectedFile()
{
    if (mState == RFS_FILE_STATE_OPENED_UNPROTECTED) {
        if (mFd > 0)
            close(mFd);

        std::string name(RFS_FILE_PATH);
        name += rfs_nv_file_names[mId - 1];

        /* protect */
        mode_t fm = S_IRUSR | S_IRGRP | S_IROTH;
        if (chmod(name.c_str(), fm) < 0) {
            // TODO error handling
        }

        /* set state */
        mState = RFS_FILE_STATE_CLOSED;
    }
}

/* process incoming request */
int CNvProtectedFile::ProcessMessage(u8 *buffer)
{
    struct rfs_header *pHeader = (struct rfs_header*)buffer;

    ALOGV("[RfsService::NvProtected] %s", __FUNCTION__);

    if(pHeader->cmd == RFS_NV_BACKUP) {
        ALOGI("[RfsService::NvProtected] %s : RFS_NV_BACKUP", __FUNCTION__);
        /* backup */
        int ret = Backup();
        if(ret == 0) {
            ALOGD("[RfsService::NvProtected] Backup Complete!!!");
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Backup Complete!!!");
        } else if(ret == -1) {
            /* request to backup while write I/O in progress*/
            ALOGD("[RfsService::NvProtected] Backup is delayed because of I/O progress.");
        } else {
            /* Failed to backup */
            ALOGD("[RfsService::NvProtected] Failed to backup!");
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to backup!");
        }
        return RFS_FILE_KEEP;
    }

    /* check transaction ID */
    if ((mTransaction != pHeader->tid) &&
        (mLastIo != RFS_NONE) &&
        (pHeader->cmd != RFS_OP_STATUS)) {
        /* new command received before previous was finished */
        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

        ALOGV("[RfsService::NvProtected] New tid received = %d(%d) before prev. was completed", pHeader->cmd, pHeader->tid);
        return RFS_FILE_KEEP;
    }

    /* check for pending I/O */
    if ((mLastIo != RFS_NONE) &&
        (pHeader->cmd != RFS_READ) &&
        (pHeader->cmd != RFS_WRITE) &&
        (pHeader->cmd != RFS_OP_STATUS)) {
        /* pending I/O exists */
        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

        ALOGV("[RfsService::NvProtected] New request received = %d(%d) before prev. was completed", pHeader->cmd, pHeader->tid);
        return RFS_FILE_KEEP;
    }

    buffer += sizeof(struct rfs_header);

    /* state check */
    switch (mState) {
        case RFS_FILE_STATE_CLOSED:
        {
            switch (pHeader->cmd) {
                case RFS_OPEN:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_OPEN", __FUNCTION__);
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Open(buffer);
                    break;
                }
                case RFS_NV_UNPROTECT:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_NV_UNPROTECT", __FUNCTION__);
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Unprotect(buffer);
                    break;
                }
                case RFS_IO_REQUEST:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_IO_REQUEST", __FUNCTION__);
                    struct rfs_request_ind *pIoReq = (struct rfs_request_ind*)buffer;

                    /* save TID */
                    mTransaction = pHeader->tid;

                    /* only read allowed */
                    if (pIoReq->io != RFS_IO_READ) {
                        /* incorrect I/O */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::NvProtected] Bad I/O request %d", pIoReq->io);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Bad I/O request %d", pIoReq->io);
                        return RFS_FILE_KEEP;
                    }

                    /* file check */
                    if (mId != pIoReq->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::NvProtected] Wrong file ID %d", pIoReq->file);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Wrong file ID %d", pIoReq->file);
                        return RFS_FILE_KEEP;
                    }

                    std::string name(RFS_FILE_PATH);
                    name += rfs_nv_file_names[mId - 1];

                    /* open file */
                    mFd = open(name.c_str(), O_RDONLY);
                    if (mFd < 0) {
                        /* failed to open file */
                        Status(RFS_STATUS_OPEN_FAIL);

                        ALOGE("[RfsService::NvProtected] Failed to open file %s (%d)", name.c_str(), mId);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open file %s (%d)", name.c_str(), mId);
                        return RFS_FILE_KEEP;
                    }

                    /* change state */
                    mState = RFS_FILE_STATE_OPENED_PROTECTED;

                    /* get file size */
                    mSize = Size();

                    /* do not allow exceed file size */
                    if (mSize < (pIoReq->offset + pIoReq->length)) {
                            /* bad I/O */
                            Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                            ALOGE("[RfsService::NvProtected] Bad I/O request %d, from %d, %d bytes", pIoReq->io, pIoReq->offset, pIoReq->length);
                            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Bad I/O request %d, from %d, %d bytes", pIoReq->io, pIoReq->offset, pIoReq->length);
                            return RFS_FILE_KEEP;
                    }

                    /* read only */
                    ReadRequest(pIoReq->offset, pIoReq->length);
                    break;
                }
                case RFS_OP_STATUS:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_OP_STATUS", __FUNCTION__);
                    struct rfs_status *pStatus = (struct rfs_status*)buffer;

                    /* file check */
                    if (mId != pStatus->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::NvProtected] Wrong file ID %d", pStatus->file);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Wrong file ID %d", pStatus->file);
                        return RFS_FILE_KEEP;
                    }

                    /* TID is same - I/O completion */
                    if (mTransaction == pHeader->tid) {
                        /* check status */
                        if (pStatus->status == RFS_STATUS_SUCCESS) {
                            ALOGI("[RfsService::NvProtected] %s : RFS_STATUS_SUCCESS", __FUNCTION__);
                            switch (mLastIo) {
                                case RFS_OPEN:
                                    mState = RFS_FILE_STATE_OPENED_PROTECTED;
                                    break;
                                case RFS_NV_UNPROTECT:
                                    mState = RFS_FILE_STATE_OPENED_UNPROTECTED;
                                    break;
                                default:
                                    /* do nothing */;
                            }
                        }
                        mLastIo = RFS_NONE;
                    }
                    /* else - nothing, drop response */
                    break;
                }
                default:
                {
                    /* incorrect command for this state */
                    Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                    ALOGE("[RfsService::NvProtected] Bad command %d(%d) for CLOSE", pHeader->cmd, pHeader->tid);
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Bad command %d(%d) for CLOSE", pHeader->cmd, pHeader->tid);
                    return RFS_FILE_KEEP;
                }
            }
            break;
        }
        case RFS_FILE_STATE_OPENED_PROTECTED:
        {
            switch (pHeader->cmd) {
                case RFS_CLOSE:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_CLOSE", __FUNCTION__);
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Close(buffer);
                    break;
                }
                case RFS_NV_UNPROTECT:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_NV_UNPROTECT", __FUNCTION__);
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Unprotect(buffer);
                    break;
                }
                case RFS_READ:
                {
                    ReadResponse(buffer);
                    break;
                }
                case RFS_OP_STATUS:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_OP_STATUS", __FUNCTION__);
                    struct rfs_status *pStatus = (struct rfs_status*)buffer;

                    /* file check */
                    if (mId != pStatus->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::NvProtected] Wrong file ID %d", pStatus->file);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Wrong file ID %d", pStatus->file);
                        return RFS_FILE_KEEP;
                    }

                    /* TID is same - I/O completion */
                    if (mTransaction == pHeader->tid) {
                        /* check status */
                        if (pStatus->status == RFS_STATUS_SUCCESS) {
                            ALOGI("[RfsService::NvProtected] %s : RFS_STATUS_SUCCESS", __FUNCTION__);
                            switch (mLastIo) {
                                case RFS_NV_UNPROTECT:
                                    mState = RFS_FILE_STATE_OPENED_UNPROTECTED;
                                    break;
                                case RFS_CLOSE:
                                    mState = RFS_FILE_STATE_CLOSED;
                                    break;
                                default:
                                    /* do nothing */;
                            }
                        }
                        mLastIo = RFS_NONE;
                    }
                    /* else - nothing, drop response */
                    break;
                }
                default:
                {
                    /* incorrect command for this state */
                    Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                    ALOGE("[RfsService::NvProtected] Bad command %d(%d) for PROTECTED", pHeader->cmd, pHeader->tid);
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Bad command %d(%d) for PROTECTED", pHeader->cmd, pHeader->tid);
                    return RFS_FILE_KEEP;
                }
            }
            break;
        }
        default:
        {
            /* open state */
            switch (pHeader->cmd) {
                case RFS_NV_PROTECT:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_NV_PROTECT", __FUNCTION__);
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Protect(buffer);
                    break;
                }
                case RFS_IO_REQUEST:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_IO_REQUEST", __FUNCTION__);
                    struct rfs_request_ind *pIoReq = (struct rfs_request_ind*)buffer;

                    /* save TID */
                    mTransaction = pHeader->tid;

                    /* file check */
                    if (mId != pIoReq->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::NvProtected] Wrong file ID %d", pIoReq->file);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Wrong file ID %d", pIoReq->file);
                        return RFS_FILE_KEEP;
                    }

                    /* do not allow exceed file size */
                    if (mSize < (pIoReq->offset + pIoReq->length)) {
                            /* bad I/O */
                            Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                            ALOGE("[RfsService::NvProtected] Bad I/O request %d, from %d, %d bytes", pIoReq->io, pIoReq->offset, pIoReq->length);
                            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Bad I/O request %d, from %d, %d bytes", pIoReq->io, pIoReq->offset, pIoReq->length);
                            return RFS_FILE_KEEP;
                    }

                    /* check I/O request */
                    switch (pIoReq->io) {
                        case RFS_IO_READ: ReadRequest(pIoReq->offset, pIoReq->length); break;
                        case RFS_IO_WRITE: WriteRequest(pIoReq->offset, pIoReq->length); break;
                        default:
                        {
                            /* incorrect I/O */
                            Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                            ALOGE("[RfsService::NvProtected] Bad I/O request %d", pIoReq->io);
                            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Bad I/O request %d", pIoReq->io);
                            return RFS_FILE_KEEP;
                        }
                    }
                    break;
                }
                case RFS_READ:
                {
                    ReadResponse(buffer);
                    break;
                }
                case RFS_WRITE:
                {
                    WriteResponse(buffer);
                    break;
                }
                case RFS_OP_STATUS:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_OP_STATUS", __FUNCTION__);
                    struct rfs_status *pStatus = (struct rfs_status*)buffer;

                    /* file check */
                    if (mId != pStatus->file) {
                        /* wrong file ID */
                        Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                        ALOGE("[RfsService::NvProtected] Wrong file ID %d", pStatus->file);
                        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Wrong file ID %d", pStatus->file);
                        return RFS_FILE_KEEP;
                    }

                    /* TID is same - I/O completion */
                    if (mTransaction == pHeader->tid) {
                        /* check status */
                        if (pStatus->status == RFS_STATUS_SUCCESS) {
                            ALOGI("[RfsService::NvProtected] %s : RFS_STATUS_SUCCESS", __FUNCTION__);
                            /* file protected */
                            if (mLastIo == RFS_NV_PROTECT) {
                                mState = RFS_FILE_STATE_OPENED_PROTECTED;
                            }
                        }
                        mLastIo = RFS_NONE;
                    }
                    /* else - nothing, drop response */
                    break;
                }
                case RFS_NV_UNPROTECT:
                {
                    ALOGI("[RfsService::NvProtected] %s : RFS_NV_UNPROTECT, Last State = OPENED_UNPROTECTED", __FUNCTION__);
                    /* save TID */
                    mTransaction = pHeader->tid;

                    Unprotect(buffer);
                    break;
                }
                default:
                {
                    /* incorrect command for this state */
                    Status(pHeader->tid, RFS_STATUS_INVALID_PARAM);

                    ALOGE("[RfsService::NvProtected] Bad command %d(%d) for OPEN", pHeader->cmd, pHeader->tid);
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Bad command %d(%d) for OPEN", pHeader->cmd, pHeader->tid);
                    return RFS_FILE_KEEP;
                }
            }
        }
    }

    /* remove file */
    if ((mState == RFS_FILE_STATE_CLOSED) &&
        (mLastIo == RFS_NONE)) {
        return RFS_FILE_DELETE_NOW;
    }

    return RFS_FILE_KEEP;
}

/* open file */
void CNvProtectedFile::Open(u8 *buffer)
{
    struct rfs_open_ind *pOpen = (struct rfs_open_ind*)buffer;

    ALOGV("[RfsService::NvProtected] %s", __FUNCTION__);

    /* file check */
    if (mId != pOpen->file) {
        /* wrong file ID */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::NvProtected] Wrong file ID %d", pOpen->file);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Wrong file ID %d", pOpen->file);
        return;
    }

    /* set I/O */
    mLastIo = RFS_OPEN;

    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];

    /* open file in read-only mode */
    mFd = open(name.c_str(), O_RDONLY);
    if (mFd < 0) {
        /* failed to open file */
        Status(RFS_STATUS_OPEN_FAIL);

        ALOGE("[RfsService::NvProtected] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        return;
    }

    /* get file size */
    mSize = Size();

    /* send status */
    Status(RFS_STATUS_SUCCESS);
}

/* close file */
void CNvProtectedFile::Close(u8 *buffer)
{
    struct rfs_close_ind *pClose = (struct rfs_close_ind*)buffer;

    ALOGV("[RfsService::NvProtected] %s", __FUNCTION__);

    /* file check */
    if (mId != pClose->file) {
        /* wrong file ID */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::NvProtected] Wrong file ID %d", pClose->file);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Wrong file ID %d", pClose->file);
        return;
    }

    /* close file */
    close(mFd);
    mFd = -1;

    /* set I/O */
    mLastIo = RFS_CLOSE;

    /* send status */
    Status(RFS_STATUS_SUCCESS);
}

/* unprotect file */
void CNvProtectedFile::Unprotect(u8 *buffer)
{
    struct rfs_unprotect_ind *pUnprotect = (struct rfs_unprotect_ind*)buffer;

    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    /* file check */
    if (mId != pUnprotect->file) {
        /* wrong file ID */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::NvProtected] Wrong file ID %d", pUnprotect->file);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Wrong file ID %d", pUnprotect->file);
        return;
    }

    /* set I/O */
    mLastIo = RFS_NV_UNPROTECT;

    /* close file */
    close(mFd);
    mFd = -1;

    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];

    /* unprotect */
    mode_t fm = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    if (chmod(name.c_str(), fm) < 0) {
        ALOGE("[RfsService::NvProtected] Failed to unprotect file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to unprotect file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
    }

    /* open file */
    mFd = open(name.c_str(), O_RDWR);
    if (mFd < 0) {
        /* failed to open file */
        Status(RFS_STATUS_OPEN_FAIL);

        ALOGE("[RfsService::NvProtected] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        return;
    }

    /* get file size */
    mSize = Size();

    /* send status */
    Status(RFS_STATUS_SUCCESS);
}

/* protect file */
void CNvProtectedFile::Protect(u8 *buffer)
{
    struct rfs_protect_ind *pProtect = (struct rfs_protect_ind*)buffer;

    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    /* file check */
    if (mId != pProtect->file) {
        /* wrong file ID */
        Status(RFS_STATUS_INVALID_PARAM);

        ALOGE("[RfsService::NvProtected] Wrong file ID %d", pProtect->file);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Wrong file ID %d", pProtect->file);
        return;
    }

    /* set I/O */
    mLastIo = RFS_NV_PROTECT;

    /* close file */
    close(mFd);
    mFd = -1;

    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];

    /* protect */
    mode_t fm = S_IRUSR | S_IRGRP | S_IROTH;
    if (chmod(name.c_str(), fm) < 0) {
        ALOGE("[RfsService::NvProtected] Failed to protect file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to protect file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
    }

    /* open file in read-only mode */
    mFd = open(name.c_str(), O_RDONLY);
    if (mFd < 0) {
        /* failed to open file */
        Status(RFS_STATUS_OPEN_FAIL);

        ALOGE("[RfsService::NvProtected] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        return;
    }

    /* send status */
    Status(RFS_STATUS_SUCCESS);
}

/* protect file without command */
void CNvProtectedFile::Protect()
{
    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    /* close file */
    close(mFd);
    mFd = -1;

    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];

    /* protect */
    mode_t fm = S_IRUSR | S_IRGRP | S_IROTH;
    if (chmod(name.c_str(), fm) < 0) {
        ALOGE("[RfsService::NvProtected] Failed to protect file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to protect file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
    }

    /* open file in read-only mode */
    mFd = open(name.c_str(), O_RDONLY);
    if (mFd < 0) {
        ALOGE("[RfsService::NvProtected] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open file %s (%d) (%s)", name.c_str(), mId, ERR2STR);
        return;
    }

    /* set state */
    mState = RFS_FILE_STATE_OPENED_PROTECTED;
}


/* calculate MD5 checksum */
int CNvProtectedFile::CsumCalc(int fd, char* csum, const char* signature)
{
    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    MD5_CTX md5;
    MD5_Init(&md5);

    u8 digest[16] = {0};
    u8 buf[1024];
    int  rd = 0;

    lseek(fd, 0, SEEK_SET);
    do {
        rd = read(fd, buf, 1024);
        if (rd < 0) {
            ALOGE("[RfsService::NvProtected] Failed to read file(%s)", ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to read file(%s)", ERR2STR);
            return (-1);
        }
        MD5_Update(&md5, buf, rd);
    } while (rd);

//    const char* signature = "Samsung_SIT_RIL";
    MD5_Update(&md5, signature, strlen(signature));
    MD5_Final(digest, &md5);

    // done
    for (int i = 0; i < 16; i++) {
        snprintf(csum+(i*2), 3, "%02x", digest[i]);
    }
    return 0;
}

/* update checksum file */
int CNvProtectedFile::CsumUpdate()
{
    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    char csum[RFS_NV_CSUM_LENGTH + 1];
    memset(csum, 0, RFS_NV_CSUM_LENGTH + 1);

    /* calculate */
    if (mFd < 0) {
        std::string name(RFS_FILE_PATH);
        name += rfs_nv_file_names[mId - 1];

        int fd = open(name.c_str(), O_RDONLY);
        if (fd < 0) {
            ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            return (-1);
        }

        CsumCalc(fd, csum, SIGNATURE_SIT);

        close(fd);
    } else {
        CsumCalc(mFd, csum, SIGNATURE_SIT);
    }

    /* write to file */
    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];
    name += RFS_NV_CSUM_FILEEXT;

    std::ofstream ofs(name.c_str(), std::ofstream::trunc);
    if (!ofs) {
        ALOGE("[RfsService::NvProtected] Failed to open %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        return (-2);
    }
    ofs.write(csum, RFS_NV_CSUM_LENGTH);
    if (!ofs) {
        ALOGE("[RfsService::NvProtected] Failed to write %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to write %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        ofs.close();
        return (-3);
    }
    ofs.close();

    return 0;
}

/* check file consistency */
int CNvProtectedFile::IsCsumCorrect()
{
    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    char csumNew[RFS_NV_CSUM_LENGTH + 1];
    char csumNewSmi[RFS_NV_CSUM_LENGTH + 1];
    memset(csumNew, 0, RFS_NV_CSUM_LENGTH + 1);
    memset(csumNewSmi, 0, RFS_NV_CSUM_LENGTH + 1);

    /* calculate */
    if (mFd < 0) {
        std::string name(RFS_FILE_PATH);
        name += rfs_nv_file_names[mId - 1];

        int fd = open(name.c_str(), O_RDONLY);
        if (fd < 0) {
            ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            return (-1);
        }

        CsumCalc(fd, csumNew, SIGNATURE_SIT);
        CsumCalc(fd, csumNewSmi, SIGNATURE_SMI);

        close(fd);
    } else {
        CsumCalc(mFd, csumNew, SIGNATURE_SIT);
        CsumCalc(mFd, csumNewSmi, SIGNATURE_SMI);
    }

    /* read checksum file */
    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];
    name += RFS_NV_CSUM_FILEEXT;

    std::ifstream ifs(name.c_str());
    if (!ifs) {
        ALOGE("[RfsService::NvProtected] Failed to open %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        return (-2);
    }

    char csumFile[RFS_NV_CSUM_LENGTH + 1];
    memset(csumFile, 0, RFS_NV_CSUM_LENGTH + 1);

    ifs.read(csumFile, RFS_NV_CSUM_LENGTH);
    if (!ifs) {
        ALOGE("[RfsService::NvProtected] Failed to read %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to read %s.md5 file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        ifs.close();
        return (-3);
    }
    ifs.close();

    /* compare */
    if (strncmp(csumNew, csumFile, RFS_NV_CSUM_LENGTH)) {
        ALOGD("[RfsService::NvProtected] Checksum(SIT) mismatch for %s file", rfs_nv_file_names[mId - 1]);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Checksum(SIT) mismatch for %s file", rfs_nv_file_names[mId - 1]);
        if (strncmp(csumNewSmi, csumFile, RFS_NV_CSUM_LENGTH)) {
            ALOGE("[RfsService::NvProtected] Checksum(SMI) mismatch for %s file", rfs_nv_file_names[mId - 1]);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Checksum(SMI) mismatch for %s file", rfs_nv_file_names[mId - 1]);
            return 1;
        }
    }

    return 0;
}

/* backup file consistence */
int CNvProtectedFile::IsBackupCsumCorrect()
{
    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    char csumNew[RFS_NV_CSUM_LENGTH + 1];
    char csumNewSmi[RFS_NV_CSUM_LENGTH + 1];
    memset(csumNew, 0, RFS_NV_CSUM_LENGTH + 1);
    memset(csumNewSmi, 0, RFS_NV_CSUM_LENGTH + 1);

    /* calculate */
    std::string name(RFS_BACKUP_FILE_PATH);
    name += ".bak";

    int fd = open(name.c_str(), O_RDONLY);
    if (fd < 0) {
        ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", name.c_str(), ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", name.c_str(), ERR2STR);
        return (-1);
    }
    CsumCalc(fd, csumNew, SIGNATURE_SIT);
    CsumCalc(fd, csumNewSmi, SIGNATURE_SMI);
    close(fd);

    /* read checksum file */
    name += RFS_NV_CSUM_FILEEXT;

    std::ifstream ifs(name.c_str());
    if (!ifs) {
        ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", name.c_str(), ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", name.c_str(), ERR2STR);
        return (-2);
    }

    char csumFile[RFS_NV_CSUM_LENGTH + 1];
    memset(csumFile, 0, RFS_NV_CSUM_LENGTH + 1);

    ifs.read(csumFile, RFS_NV_CSUM_LENGTH);
    if (!ifs) {
        ALOGE("[RfsService::NvProtected] Failed to read %s file(%s)", name.c_str(), ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to read %s file(%s)", name.c_str(), ERR2STR);
        ifs.close();
        return (-3);
    }
    ifs.close();

    if (strncmp(csumNew, csumFile, RFS_NV_CSUM_LENGTH)) {
        ALOGD("[RfsService::NvProtected] Checksum(SIT) mismatch for backup file");
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Checksum(SIT) mismatch for backup file");
        if (strncmp(csumNewSmi, csumFile, RFS_NV_CSUM_LENGTH)) {
            ALOGE("[RfsService::NvProtected] Checksum(SMI) mismatch for backup file");
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Checksum(SMI) mismatch for backup file");
            return 1;
        }
    }

    return 0;
}

int CNvProtectedFile::EqualCsum()
{
    char csum1[RFS_NV_CSUM_LENGTH + 1];
    char csum2[RFS_NV_CSUM_LENGTH + 1];
    memset(csum1, 0, RFS_NV_CSUM_LENGTH + 1);
    memset(csum2, 0, RFS_NV_CSUM_LENGTH + 1);

    /* Read md5 file */
    std::string name1(RFS_FILE_PATH);
    name1 += rfs_nv_file_names[2];
    name1 += RFS_NV_CSUM_FILEEXT;
    std::ifstream ifs1(name1.c_str());
    if (!ifs1) {
        ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", name1.c_str(), ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", name1.c_str(), ERR2STR);
        return (-1);
    }

    ifs1.read(csum1, RFS_NV_CSUM_LENGTH);
    if (!ifs1) {
        ALOGE("[RfsService::NvProtected] Failed to read %s file(%s)", name1.c_str(), ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to read %s file(%s)", name1.c_str(), ERR2STR);
        ifs1.close();
        return (-2);
    }
    ifs1.close();

    /* Read backup md5 file */
    std::string name2(RFS_BACKUP_FILE_PATH);
    name2 += RFS_NV_BACKUP_FILEEXT;
    name2 += RFS_NV_CSUM_FILEEXT;
    std::ifstream ifs2(name2.c_str());
    if (!ifs2) {
        ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", name2.c_str(), ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", name2.c_str(), ERR2STR);
        return (-3);
    }

    ifs2.read(csum2, RFS_NV_CSUM_LENGTH);
    if (!ifs2) {
        ALOGE("[RfsService::NvProtected] Failed to read %s file(%s)", name2.c_str(), ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to read %s file(%s)", name2.c_str(), ERR2STR);
        ifs2.close();
        return (-4);
    }
    ifs2.close();

    /* Compare md5 */
    if (strncmp(csum1, csum2, RFS_NV_CSUM_LENGTH)) {
        ALOGW("[RfsService::NvProtected] Checksum mismatch for backup file");
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Checksum mismatch for backup file");
        return (-5);
    }
    return 0;
}

/* backup file */
int CNvProtectedFile::Backup()
{
    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    /* write operation in progress */
    if (mWriteLength) {
        ALOGD("[RfsService::NvProtected] Could not backup while write I/O in progress");
        if (mBackupFlag) {
            ALOGD("[RfsService::NvProtected] Backup command already is reserved.");
            ALOGD("[RfsService::NvProtected] After write I/O, Backup will be carried out.");
        } else {
            ALOGD("[RfsService::NvProtected] Set Backup flag to wait for complete write I/O.");
            mBackupFlag = true;
        }
        return (-1);
    }

    /* open NV file */
    /* open forcibly here because we don't want to change file offset pointers */
    std::string name(RFS_FILE_PATH);
    name += rfs_nv_file_names[mId - 1];

    std::ifstream ifs(name.c_str(), std::ios::binary | std::ios::ate);
    if (!ifs) {
        ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        return (-2);
    }
    u32 size = ifs.tellg();

    char* dataBuf = new char[size];
    if (dataBuf == NULL) {
        ALOGE("[RfsService::NvProtected] Failed to allocate data buffer");
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to allocate data buffer");
        ifs.close();
        return (-3);
    }

    /* read */
    ifs.seekg(0, ifs.beg);
    ifs.read(dataBuf, size);
    if (!ifs) {
        ALOGE("[RfsService::NvProtected] Failed to read %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to read %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        ifs.close();
        delete[] dataBuf;
        return (-4);
    }
    ifs.close();

    /* create backup */
    name.clear();
    name += RFS_BACKUP_FILE_PATH;
    name += ".bak";

    int fd = open(name.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if((fd < 0) && (errno == ENOENT)) {
        std::string dir = name;
        size_t found = dir.find_last_of("/");
        if(found != std::string::npos) {
            dir = dir.substr(0,found);
            if (mkdir(dir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) != 0) {
                ALOGE("%s : mkdir error(%s)", __FUNCTION__, strerror(errno));
                CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] mkdir error(%s)", strerror(errno));
            }
            fd = open(name.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
        }
    }

    if (fd < 0) {
        ALOGE("[RfsService::NvProtected] Failed to create backup file (reason:%s)", ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to create backup file (reason:%s)", ERR2STR);
        delete[] dataBuf;
        return (-5);
    }

    /* write to file */
    int wr = write(fd, dataBuf, size);
    if (wr < 0) {
        ALOGE("[RfsService::NvProtected] Failed to write to backup file(%s)", ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to write to backup file(%s)", ERR2STR);
        delete[] dataBuf;
        close(fd);
        return (-6);
    }
    delete[] dataBuf;

    /* calculate checksum */
    char csum[RFS_NV_CSUM_LENGTH + 1];
    memset(csum, 0, RFS_NV_CSUM_LENGTH + 1);

    CsumCalc(fd, csum, SIGNATURE_SIT);
    close(fd);

    /* update checksum file */
    name += RFS_NV_CSUM_FILEEXT;

    std::ofstream ofs(name.c_str(), std::ofstream::trunc);
    if (!ofs) {
        ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", name.c_str(), ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", name.c_str(), ERR2STR);
        return (-7);
    }
    ofs.write(csum, RFS_NV_CSUM_LENGTH);
    if (!ofs) {
        ALOGE("[RfsService::NvProtected] Failed to write %s file(%s)", name.c_str(), ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to write %s file(%s)", name.c_str(), ERR2STR);
        ofs.close();
        return (-8);
    }
    ofs.close();

    if((IsBackupCsumCorrect() == 0) && (EqualCsum() == 0)) {
        return 0;
    } else {
        return (-9);
    }
}

/* restore file */
int CNvProtectedFile::Restore()
{
    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    /* I/O in progress */
    if (mReadLength || mWriteLength) {
        ALOGE("[RfsService::NvProtected] Could not restore while I/O in progress");
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Could not restore while I/O in progress");
        return (-1);
    }

    /* backup file correct? */
    if (IsBackupCsumCorrect()) {
        ALOGE("[RfsService::NvProtected] Backup file checksum check failed");
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Backup file checksum check failed");
        return (-2);
    }

    /* open backup file */
    std::string bakName(RFS_BACKUP_FILE_PATH);
    bakName += ".bak";

    std::ifstream ifs(bakName.c_str(), std::ios::binary | std::ios::ate);
    if (!ifs) {
        ALOGE("[RfsService::NvProtected] Failed to open backup file(%s)", ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open backup file(%s)", ERR2STR);
        return (-3);
    }
    u32 size = ifs.tellg();

    char* dataBuf = new char[size];
    if (dataBuf == NULL) {
        ALOGE("[RfsService::NvProtected] Failed to allocate data buffer");
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to allocate data buffer");
        ifs.close();
        return (-4);
    }

    /* read from backup */
    ifs.seekg(0, ifs.beg);
    ifs.read(dataBuf, size);
    if (!ifs) {
        ALOGE("[RfsService::NvProtected] Failed to read backup file(%s)", ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to read backup file(%s)", ERR2STR);
        ifs.close();
        delete[] dataBuf;
        return (-5);
    }
    ifs.close();

    std::string nvName(RFS_FILE_PATH);
    nvName += rfs_nv_file_names[mId - 1];

    /* unprotect */
    if (mState != RFS_FILE_STATE_OPENED_UNPROTECTED) {
        mode_t fm = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        if (chmod(nvName.c_str(), fm) < 0) {
            ALOGE("[RfsService::NvProtected] Failed to unprotect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to unprotect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        }
    }

    /* open file */
    std::ofstream ofs(nvName.c_str(), std::ofstream::trunc);
    if (!ofs) {
        ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        delete[] dataBuf;
        return (-6);
    }

    /* write data */
    ofs.write(dataBuf, size);
    if (!ofs) {
        ALOGE("[RfsService::NvProtected] Failed to write %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to write %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        ofs.close();
        delete[] dataBuf;
        return (-7);
    }
    delete[] dataBuf;
    ofs.close();

    /* protect */
    if (mState != RFS_FILE_STATE_OPENED_UNPROTECTED) {
        mode_t fm = S_IRUSR | S_IRGRP | S_IROTH;
        if (chmod(nvName.c_str(), fm) < 0) {
            ALOGE("[RfsService::NvProtected] Failed to protect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to protect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        }
    }

    /* done */
    CsumUpdate();
    ALOGI("[RfsService::NvProtected] %s Done!!!", __FUNCTION__);
    return 0;
}

/* check file */
int CNvProtectedFile::Touch()
{
    ALOGI("[RfsService::NvProtected] %s", __FUNCTION__);

    /* check TOC */
    struct toc_record *pToc = (struct toc_record*)tocBuffer;
    if (strcmp(pToc->name, "TOC")) {
        ALOGE("[RfsService::NvProtected] TOC record is broken");
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] TOC record is broken");
        return (-3);
    }

    /* get total size */
    u32 tocSize = 0;
    u32 tocRecords = TOC_RECORDS;
    for (u32 i = 0; i < tocRecords; i++, pToc++) {
        if (!strcmp(pToc->name, toc_section_names[mId - 1])) {
            tocSize = pToc->size;
            break;
        }
    }
    if (tocSize == 0) {
        ALOGE("[RfsService::NvProtected] No %s record in TOC", toc_section_names[mId - 1]);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] No %s record in TOC", toc_section_names[mId - 1]);
        return (-4);
    }
    mNvSize = tocSize;

    std::string nvPath(RFS_FILE_PATH);
    nvPath += rfs_nv_file_names[mId - 1];
    std::ifstream ifsNv;
    std::ofstream ofsNv;

    /* checksum it first */
    if (IsCsumCorrect()) {
        /* failed, restore from backup */
        if (!Restore()) {
            ALOGD("[RfsService::NvProtected] File %s restored from backup", rfs_nv_file_names[mId - 1]);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] File %s restored from backup", rfs_nv_file_names[mId - 1]);
            return 0;
        }

        /* no file or bad file */
        ALOGI("[RfsService::NvProtected] Create %s. Size = %d bytes", rfs_nv_file_names[mId - 1], tocSize);

        /* prepare buffer */
        char* dataBuf = new char[tocSize];
        if (dataBuf == NULL) {
            ALOGE("[RfsService::NvProtected] Failed to allocate data buffer");
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to allocate data buffer");
            return (-5);
        }
        memset(dataBuf, 0xFF, tocSize);

        /* unprotect */
        if (mState != RFS_FILE_STATE_OPENED_UNPROTECTED) {
            mode_t fm = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            if (chmod(nvPath.c_str(), fm) < 0) {
                ALOGE("[RfsService::NvProtected] Failed to unprotect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
                CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to unprotect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            }
        }

        /* create file */
        ofsNv.open(nvPath.c_str(), std::ofstream::binary | std::ofstream::trunc);
        if (!ofsNv) {
            ALOGE("[RfsService::NvProtected] Failed to open file for output(%s)", ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open file for output(%s)", ERR2STR);
            delete[] dataBuf;
            return (-7);
        }

        /* write */
        ofsNv.write(dataBuf, tocSize);
        if (!ofsNv) {
            ALOGE("[RfsService::NvProtected] Failed to write file(%s)", ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to write file(%s)", ERR2STR);
            delete[] dataBuf;
            ofsNv.close();
            return (-8);
        }

        delete[] dataBuf;
        ofsNv.close();

        /* read only */
        mode_t fm = S_IRUSR | S_IRGRP | S_IROTH;
        if (chmod(nvPath.c_str(), fm) < 0) {
            // TODO error handling
        }

    } else {
        /* open file */
        ifsNv.open(nvPath.c_str(), std::ios::binary | std::ios::ate);
        if (ifsNv) {
            /* file exists, check size*/
            u32 size = ifsNv.tellg();
            if (size < tocSize) {
                ifsNv.close();

                /* expand */
                ALOGD("[RfsService::NvProtected] File %s smaller (%d bytes) than TOC size (%d bytes). Expand", rfs_nv_file_names[mId - 1],
                    size, tocSize);

                u32 tailroom = tocSize - size;

                char* wrBuf = new char[tailroom];
                if (wrBuf == NULL) {
                    ALOGE("[RfsService::NvProtected] Failed to allocate write memory buffer");
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to allocate write memory buffer");
                    return (-9);
                }
                memset(wrBuf, 0xFF, tailroom);

                /* unprotect */
                mode_t fm = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                if (chmod(nvPath.c_str(), fm) < 0) {
                    ALOGE("[RfsService::NvProtected] Failed to unprotect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to unprotect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
                }

                ofsNv.open(nvPath.c_str(), std::ofstream::binary | std::ios::app);
                if (!ofsNv) {
                    ALOGE("[RfsService::NvProtected] Failed to open file for output(%s)", ERR2STR);
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open file for output(%s)", ERR2STR);
                    delete[] wrBuf;
                    return (-10);
                }

                ofsNv.write(wrBuf, tailroom);
                if (!ofsNv) {
                    ALOGE("[RfsService::NvProtected] Failed to write file(%s)", ERR2STR);
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to write file(%s)", ERR2STR);
                    delete[] wrBuf;
                    ofsNv.close();
                    return (-11);
                }

                delete[] wrBuf;
                ofsNv.close();

                /* protect */
                fm = S_IRUSR | S_IRGRP | S_IROTH;
                if (chmod(nvPath.c_str(), fm) < 0) {
                    ALOGE("[RfsService::NvProtected] Failed to protect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
                    CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to protect file %s(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
                }

                CsumUpdate();
            } else {
                ifsNv.close();
            }
        } else {
            ALOGE("[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open %s file(%s)", rfs_nv_file_names[mId - 1], ERR2STR);
        }
    }

    /* check data checksum */
    char* data = new char[tocSize];
    /* read */
    ifsNv.open(nvPath.c_str(), std::ios::binary | std::ios::in);
    if (!ifsNv) {
        ALOGE("[RfsService::NvProtected] Failed to open file for output(%s)", ERR2STR);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] Failed to open file for output(%s)", ERR2STR);
        delete[] data;
        ifsNv.close();
        return (-7);
    }
    ifsNv.seekg(0, ifsNv.beg);
    ifsNv.read(data, tocSize);

    if (!CsumCheckForData((u8*)data, mNvSize)) {
        ALOGW("[RfsService::NvProtected] NV data checksum is broken. Recover NV data.");
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] NV data checksum is broken. Recover NV data.");
        if (!Restore()) {
            ALOGD("[RfsService::NvProtected] File %s restored from backup", rfs_nv_file_names[mId - 1]);
            CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] File %s restored from backup", rfs_nv_file_names[mId - 1]);
            ifsNv.close();
            delete[] data;
            return 0;
        }
    }
    delete[] data;
    ifsNv.close();

    /* done */
    ALOGI("[RfsService::NvProtected] %s file is ok", rfs_nv_file_names[mId - 1]);
    return 0;
}

int CNvProtectedFile::CheckAndUpdateCsum() {
    bool enableCrash = false;
#ifdef FORCE_CRASH_BY_CSUM_ERROR
    const int defVal = 1;
    int mode = property_get_int32(CRASH_MODE_SYS_PROP, defVal);
    // 0 : kernel panic after dump
    // 1 : silent reset after dump
    // 2 : only silent reset
    enableCrash = (mode == 0);
#endif // FORCE_CRASH_BY_CSUM_ERROR

    std::string nvPath(RFS_FILE_PATH);
    nvPath += rfs_nv_file_names[mId - 1];
    if (::CsumCheckForFileAndForceCrash(nvPath.c_str(), mNvSize, enableCrash)) {
        CsumUpdate();
    }
    else {
        ALOGE("[RfsService::NvProtected] %s : Do not create md5 for recovery in next CP booting.", __FUNCTION__);
        CRfsLog::WriteRfsLog(__FUNCTION__, "[RfsService::NvProtected] : Do not create md5 for recovery in next CP booting.");
    }
    return 0;
}

void CNvProtectedFile::OnWriteDone()
{
    ALOGI("CNvProtectedFile::OnWriteDone");
    /* write completed, do some work */
    /* protect */
    Protect();

    /* update checksum */
    /* make md5 for nv normal after write */
    CheckAndUpdateCsum();
    usleep(100000);
}

