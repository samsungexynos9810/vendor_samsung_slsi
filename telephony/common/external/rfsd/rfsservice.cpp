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
    RFS service class implementation
*/

#include "rfsservice.h"

/* constructor */
CRfsService::CRfsService()
{
    char modemSuffix[PROP_VALUE_MAX] = {0, };
    mReaderThread = NULL;

    for (u32 i = 0; i < RFS_MAX_HANDLES; i++)
        mFiles.push_back(NULL);

    /* create channel */
    mIoChannel = new CRfsIoChannel();

    /* get path of modem binary */
    property_get(RFS_PROP_MODEM_BIN_PATH, mModemBin, "");
    if (strcmp(mModemBin, "") == 0) {
        property_get(RFS_PROP_MODEM_BIN_PATH_SUFFIX, modemSuffix, "");
        sprintf(mModemBin, "%s%s", DEF_MODEM_FILE_BIN, modemSuffix);
    } else {
        // use value of RFS_PROP_MODEM_BIN_PATH
    }
    ALOGD("[RfsService] Modem Path : %s", mModemBin);

    /* recover NV file ownership */
    RecoverFileOwnership();

    /* read TOC */
    int res = ReadToc(MODEM_LTE);
    if (res < 0) {
        ALOGE("[RfsService] Failed to read TOC");
        return;
    }

    /* check NV files */
    CheckNvFiles();
}

/* destructor */
CRfsService::~CRfsService()
{
    if (mReaderThread)
        delete mReaderThread;

    for (u32 i = 0; i < RFS_MAX_HANDLES; i++)
        FreeFile(i);

    if (mIoChannel)
        delete mIoChannel;
}

/* check NV data files */
void CRfsService::CheckNvFiles()
{
    ALOGI("[RfsService] %s", __FUNCTION__);

    /* start */
    property_set(RFS_PROP_NV_CHECK_DONE, "0");

    /* switch user : root -> radio */
    SwitchUser();

    /* NORMAL first */
    AllocFile(RFS_NV_NORMAL);
    int res = mFiles[RFS_NV_NORMAL]->Touch();
    if (res < 0) {
        ALOGE("[RfsService] Failed to touch normal registry file");
        return;
    }

    /* PROTECTED */
    AllocFile(RFS_NV_PROTECTED);
    res = mFiles[RFS_NV_PROTECTED]->Touch();
    if (res < 0) {
        ALOGE("[RfsService] Failed to touch protected registry file");
        return;
    }

    /* done */
    property_set(RFS_PROP_NV_CHECK_DONE, "1");
}

/* create file class */
void CRfsService::AllocFile(u32 index)
{
    ALOGI("[RfsService] %s, idx=%d", __FUNCTION__, index);

    if (mFiles[index] != NULL) {
        ALOGE("[RfsService] File is already in use (%d)", index);
        return;
    }

    switch (index) {
        case RFS_NV_NORMAL:
        case RFS_NV_USER:
            mFiles[index] = new CNvFile(index, mIoChannel, mTocBuffer.LTE_tocBuffer);
            break;
        case RFS_NV_PROTECTED:
            mFiles[index] = new CNvProtectedFile(index, mIoChannel, mTocBuffer.LTE_tocBuffer);
            break;
        default:
            mFiles[index] = new CRfsFile(index, mIoChannel, mTocBuffer.LTE_tocBuffer);
    }
}

/* remove file class */
void CRfsService::FreeFile(u32 index)
{
    ALOGI("[RfsService] %s", __FUNCTION__);

    if (mFiles[index] == NULL)
        return;

    delete mFiles[index];

    mFiles[index] = NULL;
}

/* get file ID */
u32 CRfsService::GetFileId(u8 *buffer)
{
    struct rfs_header *pHeader = (struct rfs_header*)buffer;

    ALOGV("[RfsService] %s", __FUNCTION__);

    u32 id = RFS_BAD_HANDLE;

    buffer += sizeof(struct rfs_header);
    switch (pHeader->cmd) {
        case RFS_READ:
        {
            struct rfs_read_res *p = (struct rfs_read_res*)buffer;
            id = p->file;

            ALOGV("[RfsService] READ for %d", id);
            break;
        }
        case RFS_WRITE:
        {
            struct rfs_write_res *p = (struct rfs_write_res*)buffer;
            id = p->file;

            ALOGV("[RfsService] WRITE for %d", id);
            break;
        }
        case RFS_OP_STATUS:
        {
            struct rfs_status *p = (struct rfs_status*)buffer;
            id = p->file;

            ALOGV("[RfsService] STATUS for %d", id);
            break;
        }
        case RFS_OPEN:
        {
            struct rfs_open_ind *p = (struct rfs_open_ind*)buffer;

            if (p->name_len > (pHeader->len - sizeof(struct rfs_open_ind))) {
                ALOGE("[RfsService] File name is truncated");
                break;
            }

            id = p->file;

            ALOGV("[RfsService] OPEN for %d", id);

            /* check for pre-defined names */
            buffer += sizeof(struct rfs_open_ind);
            switch (id) {
                case RFS_NV_NORMAL:
                case RFS_NV_USER:
                case RFS_NV_PROTECTED:
                    ALOGV("[RfsService] %s file, %s default", buffer, rfs_nv_file_names[id - 1]);
                    if (strcasecmp((char*)buffer, rfs_nv_file_names[id - 1])) {
                        ALOGV("[RfsService] File ID is used for NV files (%d, %s)", id, buffer);

                        id = RFS_BAD_HANDLE;
                    }
                    break;
                default:
                    /* do nothing */;
            }
            break;
        }
        case RFS_CLOSE:
        {
            struct rfs_close_ind *p = (struct rfs_close_ind*)buffer;
            id = p->file;

            ALOGV("[RfsService] CLOSE for %d", id);
            break;
        }
        case RFS_IO_REQUEST:
        {
            struct rfs_request_ind *p = (struct rfs_request_ind*)buffer;
            id = p->file;

            ALOGV("[RfsService] I/O REQ for %d", id);
            break;
        }
        case RFS_NV_UNPROTECT:
        {
            struct rfs_protect_ind *p = (struct rfs_protect_ind*)buffer;
            id = p->file;

            ALOGV("[RfsService] UNPROTECT for %d", id);
            break;
        }
        case RFS_NV_PROTECT:
        {
            struct rfs_unprotect_ind *p = (struct rfs_unprotect_ind*)buffer;
            id = p->file;

            ALOGV("[RfsService] PROTECT for %d", id);
            break;
        }
        case RFS_NV_BACKUP:
        {
            struct rfs_backup_ind *p = (struct rfs_backup_ind*)buffer;
            id = p->file;

            ALOGV("[RfsService] BACKUP for %d", id);
            break;
        }
        default:
            /* do nothing*/;
    }

    return id;
}

/* process modem packet */
int CRfsService::ProcessRfsPacket(char *data, int dataSize)
{
    int res;

    ALOGV("[RfsService] %s", __FUNCTION__);

    if (mIoChannel == NULL) {
        ALOGE("[RfsService] I/O channel is not opened yet");
        return (-1);
    }

    if ((u32)dataSize < sizeof(struct rfs_header)) {
        ALOGE("[RfsService] Incoming message is too small (%d)", dataSize);
        return (-1);
    }

#if 0
    char print_buffer[1024];
    memset(print_buffer, 0, 1024);

    u32 len = (dataSize > 64) ? 64 : dataSize;
    u32 ofs = sprintf(print_buffer, "MSG[%d]: ", dataSize);
    for (u32 i = 0; i < len; i++) {
        ofs += sprintf(print_buffer + ofs, "%02x", data[i]);
    }
    ALOGV("[RfsService] %s", print_buffer);
#endif

    struct rfs_header *pHeader = (struct rfs_header*)data;

    if ((u32)dataSize < (sizeof(struct rfs_header) + pHeader->len)) {
        ALOGE("[RfsService] Multi-fragment packet is not supported (received = %d, length = %d)", dataSize, pHeader->len);
        return (-1);
    }

    /* get file handler */
    u32 file = GetFileId((u8*)data);

    if (file > RFS_MAX_HANDLES) {
        ALOGE("[RfsService] Too many opened files (%d)", file);
        return (-1);
    }

    if (mFiles[file] == NULL)
        AllocFile(file);

    /* send request to file */
    res = mFiles[file]->ProcessMessage((u8*)data);

    /* don't need it anymore */
    if (res == RFS_FILE_DELETE_NOW)
        FreeFile(file);

    return 0;
}

/* backup protected file */
int CRfsService::Backup()
{
    ALOGI("[RfsService] %s", __FUNCTION__);

    if (mFiles[RFS_NV_PROTECTED] == NULL)
        AllocFile(RFS_NV_PROTECTED);

    return mFiles[RFS_NV_PROTECTED]->Backup();
}

/* restore protected file */
int CRfsService::Restore()
{
    ALOGI("[RfsService] %s", __FUNCTION__);

    if (mFiles[RFS_NV_PROTECTED] == NULL)
        AllocFile(RFS_NV_PROTECTED);

    return mFiles[RFS_NV_PROTECTED]->Restore();
}

/* RFS reader class */
class CRfsReaderThread: public Runnable
{
    CRfsService *mRfsService;
    CRfsIoChannel *mIoChannel;
    char *mBuf;

public:
    explicit CRfsReaderThread(CRfsService *service, CRfsIoChannel *channel)
    {
        mRfsService = service;
        mIoChannel = channel;
        mBuf = new char[RFS_MAX_READ_BUFFER];
    }

    virtual ~CRfsReaderThread()
    {
        if (mBuf)
            delete[] mBuf;
    }

    void Run()
    {
        INT32 n = 0, size = 0;
        INT32 maxfd = -1;
        fd_set rfdSet;

        if (mBuf == NULL)
            return;

        ALOGV("[RfsService::Reader] %s", __FUNCTION__);

        while (1) {
            FD_ZERO(&rfdSet);
            FD_SET(mIoChannel->GetFd(), &rfdSet);
            maxfd = mIoChannel->GetFd();

            n = select(maxfd + 1, &rfdSet, NULL, NULL, NULL);
            if (n > 0) {
                if (FD_ISSET(mIoChannel->GetFd(), &rfdSet)) {
                    size = mIoChannel->Read(mBuf, RFS_MAX_READ_BUFFER);
                    if (size > 0) {
                        mRfsService->ProcessRfsPacket(mBuf, size);
                    } else {
                        if (size < 0 && (errno == EINTR || errno == EAGAIN)) {
                            continue;
                        } else {
                            ALOGE("[RfsService::Reader] read error [%d, %d]. Re-open", size, errno);
                            mIoChannel->Close();
                            mIoChannel->Open(RFS_CHANNEL_NAME);
                            continue;
                        }
                    }
                }
            } else {
                break;
            }
        }

        ALOGE("[RfsService::Reader] stopped");
    }
};

/* RFS logging class */
class CRfsLoggingThread: public Runnable
{
public:
    explicit CRfsLoggingThread()
    {
    }

    virtual ~CRfsLoggingThread()
    {
    }

    void Run()
    {
        ALOGD("[RfsService::Logging] %s", __FUNCTION__);

        /* store nv log in /data/log/nv.log, keep size as 500K */
        char Command[100];
        char *logCmd;
        char *logLocation;
        char *logFilter;

        logCmd = (char *)"logcat -b radio -v time -r 500 -f";
        logLocation = (char *)RFS_LOG_PATH;
        logFilter = (char *)"RIL:I *:W";
        sprintf(Command, "%s %s %s", logCmd, logLocation, logFilter);
        system(Command);

        ALOGE("[RfsService::Logging] stopped");
    }
};

/* on modem boot complete */
int CRfsService::OnModemBootDone()
{
    ALOGI("[RfsService] %s", __FUNCTION__);

    /* open channel */
    int ret = mIoChannel->Open(RFS_CHANNEL_NAME);
    if (ret < 0) {
        ALOGE("[RfsService] failed to open RFS channel");
        return ret;
    }

    /* start reader */
    mReaderThread = new Thread(new CRfsReaderThread(this, mIoChannel));
    if (mReaderThread == NULL) {
        ALOGE("[RfsService] failed to create reader");
        return -1;
    }
#if 0   // disable because of unnecessary memory use
    /* start Logging */
    mLoggingThread = new Thread(new CRfsLoggingThread());
    if (mLoggingThread == NULL) {
        ALOGE("[RfsService] Logging failed");
        return -1;
    }
    mLoggingThread->Start();
#endif
    return mReaderThread->Start();
}

/*
 *  * switchUser - Switches UID to radio, preserving CAP_NET_ADMIN capabilities.
 *   * Our group, cache, was set by init.
 *    */
void CRfsService::SwitchUser() {
    char debuggable[PROP_VALUE_MAX];
    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
    if (setresuid(AID_RADIO, AID_RADIO, AID_RADIO) == -1) {
        ALOGE("setresuid failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct __user_cap_header_struct header;
    memset(&header, 0, sizeof(header));
    header.version = _LINUX_CAPABILITY_VERSION_3;
    header.pid = 0;

    struct __user_cap_data_struct data[MAX_CAP_NUM]; // data[2]
    memset(&data, 0, sizeof(data));

    data[CAP_TO_INDEX(CAP_NET_ADMIN)].effective |= CAP_TO_MASK(CAP_NET_ADMIN);
    data[CAP_TO_INDEX(CAP_NET_ADMIN)].permitted |= CAP_TO_MASK(CAP_NET_ADMIN);

    data[CAP_TO_INDEX(CAP_NET_RAW)].effective |= CAP_TO_MASK(CAP_NET_RAW);
    data[CAP_TO_INDEX(CAP_NET_RAW)].permitted |= CAP_TO_MASK(CAP_NET_RAW);

    data[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].effective |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);
    data[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].permitted |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);

    if (capset(&header, &data[0]) == -1) {
        ALOGE("capset failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*
     *      * Debuggable build only:
     *           * Set DUMPABLE that was cleared by setuid( to have tombstone on RIL crash
     *                */
    property_get("ro.debuggable", debuggable, "0");
    if (strcmp(debuggable, "1") == 0) {
        prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
    }
}

/* read TOC */
int CRfsService::ReadToc(int index)
{
    ALOGI("[RfsService] %s", __FUNCTION__);

    try {
        std::ifstream ifsModemBin(mModemBin);
        if (!ifsModemBin) {
            ALOGE("[RfsService::Nv] Failed to open modem file (%s)", mModemBin);
            return (-1);
        }
        switch(index)
        {
            case MODEM_LTE:
                ifsModemBin.read(mTocBuffer.LTE_tocBuffer, TOC_MAX_SIZE);
                break;
            case MODEM_CDMA:
                ifsModemBin.read(mTocBuffer.CDMA_tocBuffer, TOC_MAX_SIZE);
                break;
            default:
                ifsModemBin.read(mTocBuffer.LTE_tocBuffer, TOC_MAX_SIZE);
                break;
        }

        if (!ifsModemBin) {
            ALOGE("[RfsService::Nv] Failed to read TOC from modem file");
            ifsModemBin.close();
            return (-2);
        }
        ifsModemBin.close();
    }
    catch (const std::ifstream::failure& e) {
        ALOGE("[RfsService::Nv] Failed to open modem file (%s)", mModemBin);
        return (-1);
    }
    return 0;
}

/* recover right of NV files */
int CRfsService::RecoverFileOwnership()
{
    ALOGI("[RfsService] %s", __FUNCTION__);

    struct passwd *radio_pw;
    char RFS_FILE_PATH[RFS_PATH_LEN];

    int normal = 0;
    int protect = 2;

    radio_pw = getpwnam("radio");

    property_get(PROP_RFS_DIRECTORY_PATH, RFS_FILE_PATH, DEF_RFS_DIRECTORY_PATH);

    std::string name_normal(RFS_FILE_PATH);
    name_normal += rfs_nv_file_names[normal];
    if (chown(name_normal.c_str(), radio_pw->pw_uid, radio_pw->pw_gid) < 0 ) {
        ALOGE("[RfsService::Nv] Failed to chown (%s)", name_normal.c_str());
    }

    name_normal += RFS_NV_CSUM_FILEEXT;
    if (chown(name_normal.c_str(), radio_pw->pw_uid, radio_pw->pw_gid) < 0 ) {
        ALOGE("[RfsService::Nv] Failed to chown (%s)", name_normal.c_str());
    }

    std::string name_protected(RFS_FILE_PATH);
    name_protected += rfs_nv_file_names[protect];
    if (chown(name_protected.c_str(), radio_pw->pw_uid, radio_pw->pw_gid) < 0 ) {
        ALOGE("[RfsService::Nv] Failed to chown (%s)", name_protected.c_str());
    }

    name_protected += RFS_NV_CSUM_FILEEXT;
    if (chown(name_protected.c_str(), radio_pw->pw_uid, radio_pw->pw_gid) < 0 ) {
        ALOGE("[RfsService::Nv] Failed to chown (%s)", name_protected.c_str());
    }

    return 0;
}
