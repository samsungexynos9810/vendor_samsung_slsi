/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "dmd_main.h"
#include "DMAgent.h"
#include "DMFileManager.h"

// Silent Logging Variables
#define DR_DM_SILENT_BUF_SIZE 2*1024*1024
#define DR_DM_MAX_FILE_SIZE 100*1024*1024     //define maximum file size 100MB
#define DR_DM_READ_BUF_SIZE 4096    // Read buffer for file merge
#define DM_MSG_START    0x7F
#define DM_MSG_END      0x7E
#define MAX_TX_BYTE_CNT    512

/*  NNEXT packet  sequence check*/
#define DM_HOSTIF_HEADER_SIZE    12
#define DM_MAX_SEQ                0xFEFF
#define DM_TRACE_DATA_OUT                0x005

typedef struct ipc4hdlcHeader_t
{
    unsigned char  startFlag;
    unsigned short length;
    unsigned char control;
}__packed ipc4hdlcHeader;

typedef struct ipcMessage_t
{
    unsigned short length;
    unsigned short msgSeq;
    unsigned char mainCmd;
    unsigned char subCmd;
    unsigned char cmdType;
    //u8 parameter[1400];
}__packed ipcMessage;

typedef struct ipc4hdlcFrame_t
{
    ipc4hdlcHeader hdr;
    ipcMessage info;
}ipc4hdlcFrame;

static unsigned int msgSeq = 0;
static unsigned int traceMsgSeq = 0;
/*NNEXT packet  sequence check*/

int32_t g_usb_fd = -1;
int32_t g_modem_fd = -1;
int32_t g_dmd_mode = MODE_OFF;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

unsigned char silent_buf1[DR_DM_SILENT_BUF_SIZE]={0,}, silent_buf2[DR_DM_SILENT_BUF_SIZE]={0,};
int32_t rcvoffset=0,buff2use=1,rfileno=0, switchclearbuff=0,lockrfileno=0, rlockcount=0;
int32_t filesizeoffset=0;
int32_t silentbufthrdlock=0;
int32_t scount=DR_DM_SILENT_BUF_SIZE ;
int32_t sendzipintent = 0;         // flag to send zip intent
int32_t g_silent_logging_started = 0;
int32_t g_silent_logging_asked=0;
int32_t gsvbuffno=0, gsvoffset=0, gsvfileno=0, gnewfileno=0, gZipno=0;          // globals to pass parameters to thread for saving
int64_t gTimeForFile = 0;
bool gZipOperation = false;
unsigned char hdr[10] = {0, 0, 0x39,0x7F,0x00,0x00,0x00,0x00,0x00,0x00};
uint16_t* dmlen = (uint16_t*)hdr;
uint32_t* timelen = (uint32_t*)(hdr+4);
uint32_t fixtimestrtreq =1662072832;
time_t tmSec;
struct timeval stmMilli;
struct tm *ptmLocal;
uint64_t llTimestamp = 0UL;
int32_t n_next_disable = 0;
bool cp2usb_send_enable = false;
bool g_have_versioninfo = false;

unsigned char symbhdr[19] = {0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint16_t* hdlclen = (uint16_t*)(symbhdr+1);
uint16_t* ipclen = (uint16_t*)(symbhdr+4);
uint16_t* fragcount = (uint16_t*)(symbhdr+15);
uint16_t* currfrag = (uint16_t*)(symbhdr+17);

unsigned char StopDmReq[12] ={0x7F, 0x0a, 0x00, 0x00, 0x07, 0x00, 0x00, 0xFF, 0xa0, 0x00, 0x02, 0x7E};

char gSlogPath[MAX_PROP_LEN];
int gDmFileMaxSize = DR_DM_MAX_FILE_SIZE;

struct VersionInfo{
    int hdrSize;
    char *hdr;
    int size;
    char *buff;
} gVersionInfo;

void usb_close(void)
{
    int32_t ret = -1;
    if (g_usb_fd != -1) {
        ret = close(g_usb_fd);
        g_usb_fd = -1;
    }
}

void modem_close(void)
{
    if (g_modem_fd != -1) {
        close(g_modem_fd);
        g_modem_fd = -1;
    }
}

void dmd_close(void)
{
    usb_close();
    modem_close();
}

bool usb_init(void)
{
    struct termios usb_termios;

    if (g_usb_fd != -1 )
        return true;

    while(1) {
        g_usb_fd = open(PATH_USB, O_RDWR);
        if (g_usb_fd < 0) {
            ALOGE("%s : Fail to open %s, errno = %d  strerr=%s", __func__, PATH_USB, errno, strerror(errno));
            sleep(1);
            continue;
        } else {
            break;
		}
    }

    //set params
    memset((char*)&usb_termios, 0, sizeof(struct termios));

    if (tcgetattr(g_usb_fd, &usb_termios) < 0) {
        ALOGE("%s : Fail to get attributes from %s, error = %d", __func__, PATH_USB, errno);
        return false;
    }

    cfmakeraw(&usb_termios);

    usb_termios.c_iflag &= ~ICRNL;
    usb_termios.c_iflag &= ~INLCR;
    usb_termios.c_oflag &= ~OCRNL;
    usb_termios.c_oflag &= ~ONLCR;
    usb_termios.c_lflag &= ~ICANON;
    usb_termios.c_lflag &= ~ECHO;

    if (tcsetattr(g_usb_fd, TCSANOW, &usb_termios) <0) {
        ALOGE("%s : Fail to set attributes to %s, error = %d", __func__, PATH_USB, errno);
        return false;
    }
    return true;
}

bool modem_init(void)
{
    int n = 10;
    while(n-- > 0) {
        g_modem_fd = open(PATH_MODEM, O_RDWR | O_NONBLOCK | O_NOCTTY);
        if (g_modem_fd < 0) {
            ALOGE("%s : Fail to open %s, errno = %d", __func__, PATH_MODEM, errno);
            sleep(1);
            continue;
        } else {
            break;
        }
    }
    ALOGE("%s : g_modem_fd=%d", __FUNCTION__, g_modem_fd);
    return (g_modem_fd > 0);
}

bool init_monitor(void)
{
    if (!usb_init()) {
        return false;
    }

    if (!modem_init()) {
        return false;
    }
    return true;
}

int dmdWriteUsbToModem(int fd, char* buffer, int buf_len)
{
    int w, written = 0, cur = 0;
    unsigned int msg_len, to_send;
    int remain = 0;

    while (cur < buf_len) {

        while (1) {
            if (buffer[cur] != 0x7F)
                cur++;
            else
                break;

            if (cur >= buf_len) {
                ALOGE("%s : Cannot find start flag.", __func__);
                return 0;
            }
        }

        remain = buf_len - cur;

        if (remain < 3) {
            memcpy(buffer, buffer + cur, remain);
            ALOGE("%s : remain buffer size < 3", __func__);
            ALOGI("%s : ReadBuf=%d Written=%d Remain=%d", __func__, buf_len, written, remain);
            return remain;
        }

        msg_len = (buffer[cur + 1] | (buffer[cur + 2] << 8)) & 0xFFFF;

        if ((int)(cur + msg_len + 1) > buf_len) {
            memcpy(buffer, buffer + cur, remain);
            ALOGE("%s : msg length is over buffer length. remain=%d", __func__, remain);
            ALOGI("%s : ReadBuf=%d Written=%d Remain=%d", __func__, buf_len, written, remain);
            return remain;
        }

        if (buffer[cur + msg_len + 1] != 0x7E) {
            ALOGE("%s : Cannot find end flag, Throw away bad packet.", __func__);
            cur = cur + msg_len + 2;
            continue;
        }

        to_send = msg_len + 2;

        if ((w = write(fd, buffer + cur, to_send)) < 0) {
            ALOGE("%s : Write Error(%s)", __func__, strerror(errno));
            if (errno == EINTR || errno == EAGAIN)
                continue;

            return -1;
        }

        if (EnableLogDump()) {
            ALOGD("%s : after write .. wrote to modem = %d", __func__, w);
        }

        if (!w) {
            ALOGE("%s : Write 0", __func__);
            return 0;
        }

        written += w;
        cur += to_send;
    }

    remain = buf_len - written;
    ALOGI("%s : ReadBuf=%d Written=%d Remain=%d", __func__, buf_len, written, remain);
    if (remain > 0)
        memcpy(buffer, buffer + cur, remain);

    return remain;
}

int32_t dmd_modem_write(int32_t fd, char* buffer, int32_t buf_len)
{
    int32_t t = 0, w;
    uint32_t to_send, msg_len;
    uint32_t msg_lennwf;
    char *bufnfm = buffer;
    int32_t wholeSize = buf_len;

    while(buf_len > 0) {
        if (buffer[0] != 0x7F) {
            break;
        }

        msg_len = (buffer[1] | (buffer[2] << 8)) & 0xFFFF;

        if ((int32_t)msg_len + 1 >= buf_len) {
            break;
        }

        if (buffer[msg_len + 1] != 0x7E) {
            break;
        }

        to_send = msg_len + 2;

        if (g_dmd_mode == MODE_SILENT) { // socket monitor call this func, when silent mode.
            while(1)
            {
                if(silentbufthrdlock == true)    // Is bufferthread Locked?
                {
                    ALOGD("Buffer Thread Locked  = %d", silentbufthrdlock);
                    usleep(100000);
                    continue;
                }
                else
                {
                    //ALOGD("Buffer Thread Un-Locked ");
                    silentbufthrdlock = true;    // Make Thread Locked
                    break;
                }
            }

            msg_lennwf = (buffer[4] | (buffer[5] << 8)) & 0xFFFF;
            if (msg_lennwf < 2) {
                break;
            }
            msg_lennwf =msg_lennwf -2;
            memset(&hdr[0],0,2);
            *dmlen=(msg_lennwf +8);    //new format
            bufnfm=buffer+6;

            //START Enter Time Based on Packet Time Stamp
            time(&tmSec);
            gettimeofday(&stmMilli, NULL);
            ptmLocal = localtime(&tmSec);
            tmSec = mktime(ptmLocal);
            //ALOGD( "mktime: %lu(%lX)\n", tmSec);
            llTimestamp = tmSec;
            llTimestamp *= 1000;
            llTimestamp += (stmMilli.tv_usec / 1000);
            //ALOGD( "LL: %llu(%llX)\n", llTimestamp, llTimestamp);

            memset(&hdr[4], 0, 6);
            memcpy(&hdr[4], &llTimestamp, 6);
            //END Enter Time Based on Packet Time Stamp

            if(buff2use==1)
            {
               memcpy(silent_buf1+rcvoffset, hdr, sizeof(hdr));
               rcvoffset+=sizeof(hdr);

               memcpy(silent_buf1+rcvoffset, bufnfm, msg_lennwf); // write byte to buffer
               rcvoffset=rcvoffset+msg_lennwf;        // as 7E skipped
            }

            else if (buff2use==2)
            {
               memcpy(silent_buf2+rcvoffset, hdr, sizeof(hdr));
               rcvoffset+=sizeof(hdr);

               memcpy(silent_buf2+rcvoffset, bufnfm,msg_lennwf); // write byte to buffer
               rcvoffset=rcvoffset+msg_lennwf;        // as 7E skipped
            }

            silentbufthrdlock = false;    // Make Buffer Thread Un-Locked

        }
        if ((w = write(fd, buffer, to_send)) < 0) {
            ALOGE("%s : Write Error(%s)", __func__, strerror(errno));
            if (errno == EINTR || errno == EAGAIN)
                continue;

            return -1;
        }
        if (EnableLogDump()){
            ALOGD("%s : after write .. wrote to modem = %d", __func__, w);
        }
        if( w > (int32_t)to_send)
            w = to_send;

        if (!w)
            return 0;
        buf_len -= w;
        buffer = (char *)(buffer + w);
        t += w;
        ALOGD("%s : Written %d/%d", __func__, t, wholeSize);
    }
    return t;
}

int32_t dmd_write(int32_t fd, char* buffer, int32_t buf_len)
{
    int32_t write_len = 0;
    int32_t len = 0;

    if (fd < 0) {
        ALOGE("%s : write fail. fd is not initialized.", __FUNCTION__);
        return -1;
    }

    if (buffer == NULL || buf_len <= 0) {
        return -1;
    }

    const int TIMEOUT_DM_STOP_REQUEST = (10 * 60 * 1000); // 10 minutes (msec)
    const int TICK_WRITABLE = 500; // 500 msec
    const int MAX_COUNT =  TIMEOUT_DM_STOP_REQUEST / TICK_WRITABLE;
    static int count = 0;

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
    struct timeval tv = { 0, (TICK_WRITABLE * 1000) };  // usec
    int n = select(fd + 1, NULL, &wfds, NULL, &tv);
    if (n == 0) {
        ALOGE("%s : couldn't write to the fd=%d. DM packet dropped(%d bytes)", __FUNCTION__, fd, buf_len);
        if (count++ > MAX_COUNT) {
            // stop DM service
            set_cp2usb_path(false);
            if (g_modem_fd != -1) {
                ALOGI("%s : Never sent data to DM Host. Stop DM Service by force", __FUNCTION__);
                dmd_modem_write(g_modem_fd,(char *) StopDmReq, sizeof(StopDmReq)/sizeof(char));
            }
            count = 0;
        }
        return -1;
    }
    else if (n > 0) {
        if (count > 0) {
            ALOGI("%s : Recovered to be writable state. fd=%d", __FUNCTION__, fd);
            count = 0;
        }
    }

    // write data
    do {
        //log_hexdump((const char *)buffer,buf_len);
        if ((len = write(fd, buffer + write_len, (buf_len - write_len))) < 0) {
            ALOGE("%s : write fail errno = %d", __func__, errno);
            if (errno == EINTR || errno == EAGAIN)
                continue;
            return -1;
        }
        write_len += len;
    } while(write_len < buf_len);

    return write_len;
}

void *modem_status_monitor(void *arg)
{
    int32_t fd;
    struct pollfd pollfd;
    int ret = 0;
    int state = -1;

    fd = open(PATH_BOOT, O_RDWR);
    pollfd.fd = fd;
    pollfd.events = POLLHUP | POLLIN | POLLRDNORM;
    pollfd.revents = 0;

    ALOGD("%s : start!!!", __FUNCTION__);
    while(1)
    {
        pollfd.revents = 0;
        ret = poll(&pollfd, 1, -1);

        if((pollfd.revents & POLLHUP) || (pollfd.revents & POLLIN) || (pollfd.revents & POLLRDNORM))
        {
            if(g_dmd_mode != MODE_SILENT)
            {
                ALOGD("%s : A change of modem status is sensed, but dmd mode is not Silent mode.", __FUNCTION__);
                usleep(100 * 1000);
                continue;
            }

            ALOGD("%s : receive poll event!!!", __FUNCTION__);
            const unsigned int interval = 2;
            bool restartDm = false;
            while (true) {
                state = ioctl(fd, IOCTL_MODEM_STATUS);
                ALOGD("%s : modem status [%d]", __FUNCTION__, state);
                if (state == STATE_OFFLINE || state == STATE_BOOTING) {
                    restartDm = true;
                    int spin = 300;
                    while (spin--) {
                        state = ioctl(fd, IOCTL_MODEM_STATUS);
                        if (state == STATE_ONLINE) {
                            break;
                        }
                        usleep(100 * 1000);
                    }

                    if (spin < 0) {
                        ALOGE("%s : Modem boot timeout", __FUNCTION__);
                        restartDm = false;
                    }
                }
                else if (state == STATE_CRASH_EXIT || state == STATE_CRASH_RESET) {
                    ALOGD("%s : Modem  is STATE_CRASH_EXIT or STATE_CRASH_RESET", __FUNCTION__);
                    ALOGD("%s : Check status after %d seconds again.", __FUNCTION__, interval);
                    restartDm = true;
                    sleep(interval);
                    continue;
                }

                if (state == STATE_ONLINE) {
                    ALOGD("%s : Modem is ONLINE", __FUNCTION__);
                    if (restartDm) {
                        DoSendProfile(g_modem_fd);
                    }
                }
                break;
            }
        }
        else {
            ALOGE("%s : unknown poll event!", __FUNCTION__);
            usleep(200000);
        }
    }
    close(fd);
    return NULL;
}

void *run_modem_monitor(void *arg)
{
    int32_t n;
    fd_set rfds;
    char buffer[MAX_BUF];
    int32_t fd = g_modem_fd;

    int32_t to_send, msg_len, pktlen, msg_lennwf;
    char* p;
    char* bufnfm;
    pthread_t thread1;

    while(1)
    {
        if (g_modem_fd < 0) {
            ALOGE("%s : Invalid g_modem_fd. thread exit", __FUNCTION__);
            break;
        }
        fd = g_modem_fd;

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        if (EnableLogDump()){
            ALOGD("before select\n");
        }
        n = select(fd + 1, &rfds, NULL, NULL, NULL);
        //ALOGD("%s : after select", __func__);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            ALOGE("%s : select err = %d", __func__, errno);
            continue;
        }

        if (FD_ISSET(fd, &rfds))
        {
            n = read(fd, &buffer, MAX_BUF - 1);
            if (n < 0) {
                if (errno == EINTR)
                    continue;
                ALOGE("%s : read err", __func__);
                continue;
            }
            else if (n == 0) // status unpluged
            {
                ALOGE("%s : %s EOF", __FUNCTION__, PATH_MODEM);
                modem_close();
                modem_init();
                continue;
            }
            if (g_dmd_mode == MODE_SILENT) { // silent mode
                //ALOGD("%s : MODE_SILENT SilentLog write", __func__);
                while(1){
                    if(silentbufthrdlock == true)    // Is bufferthread Locked?
                    {
                        ALOGD("Buffer Thread Locked  = %d", silentbufthrdlock);
                        usleep(100000);
                        continue;
                    }
                    else
                    {
                        //ALOGD("Buffer Thread Un-Locked ");
                        silentbufthrdlock=true;    // Make Thread Locked
                        break;
                    }
                }
                // Start Convert to sdm format
                p = buffer;
                bufnfm = buffer;        //new format data buffer
                pktlen=n;

                while (pktlen > 0) {
                    //ALOGD( "----------inside  while (pktlen > 0)-----\n");
                    if(p[0]  != DM_MSG_START)
                    {
                        ALOGD( "SAVESILENTLOG-- Convert to Sdm--DM_MSG_START_FLAG(0x7F) is not found. \n");
                        break;
                    }

                    msg_len = p[1] + (p[2] << 8);
                    msg_lennwf = p[4] + (p[5] << 8);
                    msg_lennwf =msg_lennwf -2;
                    bufnfm = p+6;

                    if (msg_len > MAX_BUF-2)
                    {
                        ALOGD( "msg_len > MAX_BUF-2 \n");
                        break;
                    }

                    if(p[msg_len+1]  != DM_MSG_END)
                    {
                        ALOGD( "SAVESILENTLOG-- Convert to Sdm-- DM_MSG_END_FLAG(0x7E) is not found. \n");
                        break;
                    }

                    if(g_silent_logging_started == 0 && g_silent_logging_asked == 1)
                    {
                        property_set(PROPERTY_SILENTLOG_MODE, "On");
                        ALOGD( "received silent modem log : property set on\n");
                        g_silent_logging_started = 1;
                    }

                    to_send = msg_len +1;
                    memset(&hdr[0],0,2);
                    *dmlen=(msg_lennwf +8);
                    //Start Enter Time Based on Packet Time Stamp

                    time(&tmSec);
                    gettimeofday(&stmMilli, NULL);
                    ptmLocal = localtime(&tmSec);
                    tmSec = mktime(ptmLocal);
                    //ALOGD( "mktime: %lu(%lX)\n", tmSec);

                    llTimestamp = tmSec;
                    llTimestamp *= 1000;
                    llTimestamp += (stmMilli.tv_usec / 1000);
                    //ALOGD( "LL: %llu(%llX)\n", llTimestamp, llTimestamp);

                    //unsigned char acBuffer[16];
                    memset(&hdr[4], 0, 6);
                    memcpy(&hdr[4], &llTimestamp, 6);

                    //END Enter Time Based on Packet Time Stamp

                    // check DM start response in header file for version information
                    if(!g_have_versioninfo)
                    {
                        if(EnableLogDump())
                        {
                            log_hexdump((const char *)bufnfm, msg_lennwf);
                        }
                        CheckVersionInfo(bufnfm, msg_lennwf);
                    }

                    // select buffer to use
                    if(buff2use==1)
                    {
                        memcpy(silent_buf1+rcvoffset, hdr, sizeof(hdr));
                        rcvoffset+=sizeof(hdr);
                        memcpy(silent_buf1+rcvoffset, bufnfm, msg_lennwf); // write byte to buffer
                        rcvoffset=rcvoffset+msg_lennwf;        // as 7E skipped
                    }
                    else if (buff2use==2)
                    {
                        memcpy(silent_buf2+rcvoffset, hdr, sizeof(hdr));
                        rcvoffset+=sizeof(hdr);
                        memcpy(silent_buf2+rcvoffset, bufnfm, msg_lennwf); // write byte to buffer
                        rcvoffset=rcvoffset+msg_lennwf;        // as 7E skipped
                    }

                    //ALOGD( "----------msg_len = %d    -----\n",msg_len);
                    p =p+to_send+1;    //skip 7E
                    pktlen = pktlen- (to_send+1);
                }
                //End convet to sdm

                //rcvoffset+=n;
                scount =scount-(n+4);    // 4 bytes added while converting to sdm format
                //ALOGD( "---------------scount= %d    rcvoffset= %d \n",scount,rcvoffset);
                if (scount<1000 ||rcvoffset>(DR_DM_SILENT_BUF_SIZE-4576) )    // rcvoffset margin to avoid buffer over flow
                {
                    // Write to file
                    ALOGD( "----------DoFileOperation-----scount= %d\n",scount);
                    gsvbuffno=buff2use;
                    gsvoffset=rcvoffset;

                    pthread_attr_t attr;
                    pthread_attr_init(&attr);
                    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

                    pthread_create(&thread1, &attr, DoFileOperationThread, NULL);

                    ALOGD( "    sount = [%d] , rcvoffset = [%d]  \n",scount ,rcvoffset);
                    scount=DR_DM_SILENT_BUF_SIZE;
                    filesizeoffset += rcvoffset;
                    // Switch Buffer
                    if(buff2use==1)
                    {
                        switchclearbuff=1; // ON
                        buff2use =2;
                        rcvoffset=0;
                        // check file size max reached
                        if(filesizeoffset > gDmFileMaxSize) {
                            gZipOperation = true;
                            ALOGD( " filesizeoffset = [%d]  \n", filesizeoffset);
                            filesizeoffset =0;
                        }
                        ALOGD( "----------Buffer Swicthed to Second OnE --buff2use==2-----\n");
                    }
                    else if(buff2use==2)
                    {
                        switchclearbuff=1; // ON
                        buff2use=1;
                        rcvoffset=0;
                        // check file size max reached
                        if(filesizeoffset > gDmFileMaxSize) {
                            gZipOperation = true;
                            ALOGD( " filesizeoffset = [%d]  \n", filesizeoffset);
                            filesizeoffset =0;
                        }
                        ALOGD( "----------Buffer Swicthed to First OnE --buff2use==1-----\n");
                    }
                }
                // clearing buffered delayed due to Thread operation writing to file from previous buffer
                if(((scount<(DR_DM_SILENT_BUF_SIZE/2))&&switchclearbuff==1))
                {
                    if(buff2use==1)
                    {
//                        memset(silent_buf2,0,sizeof(silent_buf2));
                        switchclearbuff=0; // OFF , buffer cleared
//                        ALOGD( "----------current buffer 1, silent_buf2 cleared-----\n");
                    }
                    else if(buff2use==2)
                    {
//                        memset(silent_buf1,0,sizeof(silent_buf1));
                        switchclearbuff=0; // OFF , buffer cleared
//                        ALOGD( "----------current buffer 2, silent_buf1 cleared-----\n");
                    }
                }
                silentbufthrdlock=false;    // Make Buffer Thread Un-Locked

                 //if( 0 == n_next_disable)// &&  1 == is_cp2usb_path())
                //{
                //    dmd_write(g_usb_fd, buffer, n);
                //}
                //ALOGD("%s : SilentLog write", __func__);
                //dmMsgSeqCheck(buffer, n);

            } else if (g_dmd_mode == MODE_NNEXT) { // nnext mode
                /* For usb block after silent log long run [when silent log stopped packets go
                to usb instantly and thread block occurs, after that Silent Log or myShannon does not work] by m.afzal 27/8/2014*/
                if(!is_cp2usb_path()) {
                    ALOGD("Path Changed .. Still Receiving Packets ..Droping Packet\n");
                } else {
                    //dmMsgSeqCheck(buffer, n);
                    pthread_mutex_lock(&mut);
                    //ALOGD("%s : nnext write", __func__);
                    if (EnableLogDump()){
                        ALOGD("write to usb : %d\n" , n);
                    }
                    dmd_write(g_usb_fd, buffer, n);

                    pthread_mutex_unlock(&mut);
                }
            } else if (g_dmd_mode == MODE_EXT_APP) {
                // send to HIDL
                DMAgent_OnResponse(buffer, n);

            }
        }
    }
    return NULL;
}

void *run_usb_monitor(void *arg)
{
    int32_t n;
    fd_set rfds;
    char buffer[MAX_BUF];
    int32_t fd;

    int index = 0;

    ALOGE("Enter %s ", __FUNCTION__);
    while(1) {
        fd = g_usb_fd;
        for( ;; ) {
            FD_ZERO(&rfds);
            FD_SET(fd, &rfds);

            n = select(fd + 1, &rfds, NULL, NULL, NULL);

            if (n < 0) {
                if (errno == EINTR)
                    continue;
                ALOGE("%s : select err = %d", __func__, errno);
                return NULL;
            }

            if (FD_ISSET(fd, &rfds)) {
                n = read(fd, &buffer[index], MAX_BUF - 1 - index);
                ALOGD("%s : new read=%d index=%d", __func__, n, index);
                n += index ;
                //hexdump(buffer, n);
                if (n < 0) {
                    if (errno == EINTR)
                        continue;
                    ALOGE("%s : read err", __func__);
                    break;
                } else if (n == 0) { // status unpluged
                    ALOGE("%s : read buffer count : 0", __func__);
                    // patch afzal send stop message to CP April20,2015
                    if(is_cp2usb_path())
                    {
                        //set_cp2usb_path(false);
                        //dmd_modem_write(g_modem_fd,(char *) StopDmReq, 12);
                        ALOGE("Warning! USB connection may be unplugged.");
                    }

                    break;
                }

                ALOGD("%s : read usb len = %d ", __func__, n);
                set_cp2usb_path(true);                // enable data to usb, so that pc NNEXT can read from usb
                if (EnableLogDump()) {
                    log_hexdump((const char *) buffer, n);
                }
                index = dmdWriteUsbToModem(g_modem_fd, buffer, n);
                if (index < 0)
                    index = 0;
            }
        } // end for ~
        sleep(2);
        pthread_mutex_lock(&mut);
        ALOGE("USB reopen close");
        usb_close();
        ALOGE("USB reopen init");
        usb_init();
        pthread_mutex_unlock(&mut);
    } // end while ~
    return NULL;
}

void start_monitor_thread(void)
{
    pthread_t thread_id_modem = 0;
    pthread_t thread_id_usb = 0;
    pthread_t thread_id_modem_status = 0;
    //run thread
    pthread_create(&thread_id_usb, NULL, run_usb_monitor, NULL);
    pthread_create(&thread_id_modem, NULL, run_modem_monitor, NULL);
    pthread_create(&thread_id_modem_status, NULL, modem_status_monitor, NULL);
}

bool mkdir_slog(void)
{
    if (chdir(gSlogPath) < 0) {
        if (mkdir(gSlogPath, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
            ALOGE("mkdir %s create fail", gSlogPath);
            return false;
        }
        else
            ALOGD("%s : %s", __func__, gSlogPath);
    }

    std::string ssPath(gSlogPath);
    ssPath += SNAPSHOT_DIR;
    if (chdir(ssPath.c_str()) < 0) {
        if (mkdir(ssPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
            ALOGE("mkdir %s create fail", ssPath.c_str());
            return false;
        }
        else
            ALOGD("%s : %s", __func__, ssPath.c_str());
    }
    ALOGD("%s : success mkdir!", __func__);

    return true;
}

void switchUser(void)
{
    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
    setuid(AID_SYSTEM);

    struct __user_cap_header_struct header;
    struct __user_cap_data_struct cap;
    header.version = _LINUX_CAPABILITY_VERSION;
    header.pid = 0;

    cap.effective = cap.permitted = ( 1 << CAP_SYS_BOOT) | (1 << CAP_NET_ADMIN) | (1 << CAP_SYS_ADMIN) | (1 << CAP_NET_RAW) | (1<<CAP_DAC_OVERRIDE);

    cap.inheritable = 0;
    if (capset(&header, &cap) < 0)
    {
        ALOGE("%s : capset error(%s)", __FUNCTION__, strerror(errno));
    }
}

//slient logging start
void DoFileOperation(int32_t buffno, int32_t offset, uint64_t timeStamp)
{
    FILE *sbuff_file = NULL;

    char filestr[100];
    sprintf(filestr, "%ssbuff_%ld.sdm", gSlogPath, timeStamp);
    ALOGD( "----------File String is  : %s  ----------\n", filestr);

    /* merge header to new file */
    gTimeForFile = CalcTime();
    MergeHeader(gTimeForFile);
    MergeVersionInfo(gTimeForFile);

    sbuff_file = fopen(filestr, "ab");

    if(NULL!=sbuff_file)
    {
        if(buffno==1)
        {
            fwrite(silent_buf1, sizeof(silent_buf1[0]), offset, sbuff_file);
        }
        else if(buffno==2)
        {
            fwrite(silent_buf2, sizeof(silent_buf2[0]), offset, sbuff_file);
        }

        //ALOGD("----------Wrote to DM /data/log/SBUFF_FILE---------sount=%d\n",scount );
        fclose(sbuff_file);
        sbuff_file=NULL;
    }
    else
    {
        ALOGD("----------Opening tmpsbuff_file2 Fail-----\n");
    }
}

void *DoFileOperationThread(void *arg)
{
    unsigned char *write_buf;
    uint64_t privTime = gTimeForFile;

    write_buf = (unsigned char *)malloc(DR_DM_SILENT_BUF_SIZE * sizeof(unsigned char));

    // Write to file
    ALOGD( "----------DoFileOperationThread-----\n");
    // Switch file no
    FILE *sbuff_file=NULL;

    if(gZipOperation)
    {
        privTime = gTimeForFile;
        gTimeForFile = CalcTime();
        MergeHeader(gTimeForFile);
        MergeVersionInfo(gTimeForFile);
        sendzipintent = 1;       // set zip intent
    }

    memset(write_buf, 0, DR_DM_SILENT_BUF_SIZE * sizeof(unsigned char));
    if(gsvbuffno==1)
    {
        memcpy(write_buf, silent_buf1, DR_DM_SILENT_BUF_SIZE * sizeof(unsigned char));
    }
    else if(gsvbuffno==2)
    {
        memcpy(write_buf, silent_buf2, DR_DM_SILENT_BUF_SIZE * sizeof(unsigned char));
    }

    char filestr[100];
    sprintf(filestr, "%ssbuff_%ld.sdm", gSlogPath, privTime);
    ALOGD("----------File String is gsvfileno : %s  ----------\n" ,filestr);

    sbuff_file = fopen(filestr, "ab");

    if(NULL!=sbuff_file)
    {
        fwrite(write_buf, sizeof(write_buf[0]), gsvoffset, sbuff_file);
        //ALOGD("----------Wrote to DM /data/log/SBUFF_FILE---------sount=%d\n",scount );
        fclose(sbuff_file);
        sbuff_file=NULL;
    }
    else
    {
        ALOGD("----------Opening tmpsbuff_file2 Fail-----\n");
    }
    if(sendzipintent==1){
        sendzipintent = 0;
        gZipOperation = false;

        DoZipOperation(privTime);
    }
    free(write_buf);
    return NULL;
}

int DoZipOperation(uint64_t timeStamp)
{
    ALOGD("%s++", __FUNCTION__);

    char zipfilestr[100] = {0, };
    char zipexstr[256] = {0, };

    sprintf(zipfilestr, "sbuff_%ld", timeStamp);

    int cmdLength = strlen(gSlogPath) + strlen(zipfilestr);
    if(cmdLength > (int)sizeof(zipexstr) - 1)
    {
        ALOGW("%s : zip command length is too long. Skip ZIP operation.", __func__);
        return -1;
    }

    sprintf(zipexstr, "%s%s.sdm.zip", gSlogPath, zipfilestr);
    ALOGD("%s : Command  : %s\n", __func__, zipexstr);
    FILE* zfile = fopen(zipexstr, "wb");
    ZipWriter writer(zfile);

    string srcFilePath = gSlogPath;
    srcFilePath += zipfilestr;
    srcFilePath += ".sdm";

    string fileName = zipfilestr;
    fileName += ".sdm";

    writer.StartEntry(fileName.c_str(), ZipWriter::kCompress | ZipWriter::kAlign32);
    ALOGD("%s : StartEntry %s", __func__, srcFilePath.c_str());
    FILE* input = fopen(srcFilePath.c_str(), "r");
    if (input != NULL) {
        char buf[2048];
        size_t cnt = 0;
        while ((cnt = fread(buf, 1, sizeof(buf), input)) > 0) {
            writer.WriteBytes(buf, cnt);
        }
        fclose(input);
    }
    writer.FinishEntry();
    writer.Finish();
    fclose(zfile);

    string filename = zipfilestr;
    filename += ".sdm";
    DMFileManager *fileManager = DMFileManager::getInstance();

    int ret = access(zipexstr, F_OK);

    if (ret == 0) {
        char filedelstr[100];
        sprintf(filedelstr, "%ssbuff_%ld.sdm", gSlogPath, timeStamp);    // delete sdm file

        int rv = remove(filedelstr);
        if (rv == 0) {
            ALOGD("%s : File Deleted after ZIP : %s\n", __FUNCTION__, filedelstr);
            if (fileManager != NULL) {
                filename += ".zip";
                fileManager->add(filename);
            }
        }
        else {
            ALOGE("%s : Failed to remove file : %s\n", __FUNCTION__, filedelstr);
            if (fileManager != NULL) {
                fileManager->add(filename);
            }
        }
    }
    else
    {
        ALOGE("%s : Failed to execute zip command.\n", __FUNCTION__);
        if (fileManager != NULL) {
            fileManager->add(filename);
        }
        return -2;
    }

    ALOGD("%s--", __FUNCTION__);
    return 0;
}

void trigger_save_dump(void)
{
    int32_t tmpbuff2use=0, tmprcvoffset=0, currentfileno =0;

    if(g_dmd_mode == MODE_SILENT)
    {
        // Do file operation
        // first save current values to tmp, and switch buffer
        tmpbuff2use = buff2use;
        tmprcvoffset = rcvoffset;
        scount = DR_DM_SILENT_BUF_SIZE;
        rcvoffset = 0;

        if(buff2use==1)        //switching buffer
        {
            buff2use=2;
        }
        else if(buff2use==2)
        {
            buff2use=1;
        }

        uint64_t privTime = gTimeForFile;
        DoFileOperation(tmpbuff2use, tmprcvoffset, privTime);

        if(tmpbuff2use==1)    // clear buffer special case
        {
            memset(silent_buf1,0,sizeof(silent_buf1));
        }
        else if(tmpbuff2use==2)
        {
            memset(silent_buf2,0,sizeof(silent_buf2));
        }

        filesizeoffset =0;

        //Send file no. back
        ALOGD("----------MergeLogFiles  [%d]----------\n", currentfileno);
        MergeLogFiles(privTime);
    }//end silent log enable disable check --- phase 2
}

void CheckVersionInfo(char *buff, int32_t buff_len)
{
    if(buff != NULL && buff_len > 0)
    {
        if((buff[2]==0xA0 || buff[2]==0xA1) && buff[3]==0x00 && buff[4]==0x01)
        {
            ALOGD( "%s : Find version information packet.", __FUNCTION__);
            gVersionInfo.hdrSize = sizeof(hdr);
            gVersionInfo.hdr = new char[gVersionInfo.hdrSize];
            memset(gVersionInfo.hdr, 0, gVersionInfo.hdrSize);
            memcpy(gVersionInfo.hdr, hdr, sizeof(char)*gVersionInfo.hdrSize);

            gVersionInfo.size = buff_len;
            gVersionInfo.buff = new char[gVersionInfo.size];
            memset(gVersionInfo.buff, 0, gVersionInfo.size);
            memcpy(gVersionInfo.buff, buff, sizeof(char)*gVersionInfo.size);
            g_have_versioninfo = true;
        }
    } else {
        if (buff == NULL) {
            ALOGW("%s : buff is null.", __FUNCTION__);
        } else if (buff_len <= 0) {
            ALOGW("%s : buff_len is abnormal(%d).", __FUNCTION__, buff_len);
        }
    }
    return;
}

void MergeVersionInfo(uint64_t timeStamp)
{
    char filestr[100];
    FILE *sbuff_file = NULL;

    if (gVersionInfo.hdrSize == 0 || gVersionInfo.size == 0) {
        ALOGW("%s : Invalid version info.", __FUNCTION__);
        return;
    }

    sprintf(filestr, "%ssbuff_%ld.sdm", gSlogPath, timeStamp);
    ALOGD("%s() : %s", __FUNCTION__, filestr);
    sbuff_file = fopen(filestr, "ab");
    fwrite(gVersionInfo.hdr, sizeof(char), gVersionInfo.hdrSize, sbuff_file);
    fwrite(gVersionInfo.buff, sizeof(char), gVersionInfo.size, sbuff_file);

    if(NULL != sbuff_file)
    {
        fclose(sbuff_file);
        sbuff_file = NULL;
    }
    return;
}

// Merging all files .. function moved from framework for speed
void MergeHeader(uint64_t timeStamp)
{
    char filestr[100], strHeaderFile[100];
    unsigned char merge_buf[100]={0,};
    FILE *sbuff_file=NULL;
    FILE *HeaderFile=NULL;
    struct stat HeadFileSt;
    int i = 0;
    int nread = 0;

    sprintf(filestr, "%ssbuff_%ld.sdm", gSlogPath, timeStamp);
    ALOGD( "----------MergeHeader : %s  ----------\n" , filestr);

    sbuff_file = fopen(filestr, "ab");

    //merge header file first
    sprintf(strHeaderFile,"%s.sbuff_header.sdm", gSlogPath);
    HeaderFile = fopen(strHeaderFile, "rb");
    if(HeaderFile!=NULL)
    {
        ALOGD( "Header exists-----merging header\n");
        if (stat(strHeaderFile, &HeadFileSt) != 0)
        {
            ALOGE("%s : stat error(%s)", __FUNCTION__, strerror(errno));
        }
        ALOGD( "HeadFileSt.st_size = %ld\n",  HeadFileSt.st_size);
        while(i < HeadFileSt.st_size)
        {
            memset(merge_buf,0,sizeof(merge_buf));
            nread = fread(merge_buf, sizeof(merge_buf[0]) , sizeof(merge_buf), HeaderFile);
            fwrite(merge_buf, sizeof(merge_buf[0]), nread, sbuff_file);
            i += nread;
        }
        fclose(HeaderFile);
        HeaderFile=NULL;
    }

    if(NULL!=sbuff_file)
    {
        fclose(sbuff_file);
        sbuff_file=NULL;
    }

     return ;
}

// Read property for DMD log dump
int EnableLogDump()
{
    int printlog = 0;
    char logstr[100];
    property_get(PROPERTY_DMD_ENABLE_LOG, logstr , "0");      // 0 to disable
    printlog = atoi(logstr);
    return printlog;
}

uint64_t CalcTime()
{
    uint64_t Timestamp = 0UL;
    time_t timeSec;
    struct timeval tmMilli;
    struct tm *pTmLocal;

    time(&timeSec);
    gettimeofday(&tmMilli, NULL);
    pTmLocal = localtime(&timeSec);

    Timestamp = pTmLocal->tm_sec + (uint64_t)pTmLocal->tm_min*100 + (uint64_t)pTmLocal->tm_hour*100*100 + (uint64_t)pTmLocal->tm_mday*100*100*100 + (uint64_t)(pTmLocal->tm_mon+1)*100*100*100*100 + (uint64_t)(pTmLocal->tm_year+1900)*100*100*100*100*100;

    return Timestamp;
}

// Merging all files .. function moved from framework for speed -- modified merge ony last file
void MergeLogFiles(uint64_t timeStamp)
{
    unsigned char merge_buf[DR_DM_READ_BUF_SIZE]={0,};
    char strMergedFile[100], strSegment[100];
    FILE *MergedFile=NULL, *SegmentFile=NULL;
    size_t n_size;

    std::string ssFilePath(gSlogPath);
    ssFilePath += SANPSHOT_SLOG_FILENAME;
    sprintf(strMergedFile, "%s", ssFilePath.c_str());

    MergedFile = fopen(strMergedFile,"wb");

    SegmentFile=NULL;
    memset(strSegment,0,sizeof(strSegment));
    sprintf(strSegment,"%ssbuff_%ld.sdm", gSlogPath, timeStamp);
    ALOGD("File(%ld): %s\n", timeStamp, strSegment);

    SegmentFile = fopen(strSegment, "rb");
    if(SegmentFile)
    {
        memset(merge_buf,0,sizeof(merge_buf));
        while (0 < (n_size = fread(merge_buf, sizeof(merge_buf[0]), DR_DM_READ_BUF_SIZE, SegmentFile))) {
        fwrite(merge_buf,  sizeof(merge_buf[0]), n_size, MergedFile);
        memset(merge_buf,0,sizeof(merge_buf));
        }
        fclose(SegmentFile);
        SegmentFile=NULL;
    }
    else
    {
        ALOGD("Skip file: donot exist %s\n" , strSegment);
    }

    fclose(MergedFile);
    return ;

}

/*enable-disable data to usb, so that pc NNEXT can read from usb--
patch for silent log start error : reason - after stop msg from silent log app,
packets are sent to usb, and usb queue full occurs, thread hanged m.afzal 27/8/2014
*/
void set_cp2usb_path(bool value)
{
    cp2usb_send_enable = value;
}


int32_t is_cp2usb_path(void)
{
    return cp2usb_send_enable;
}

void OnSilentLogStop(void)
{
    unsigned char *write_buf;
    FILE *sbuff_file=NULL;

    if(rcvoffset == 0)
    {
        ALOGD("%s : Currently used buffer is empty!", __FUNCTION__);
    }
    else if(rcvoffset > 0)
    {
        // Save current buffer before stoping silent logging
        write_buf = (unsigned char *)malloc(DR_DM_SILENT_BUF_SIZE * sizeof(unsigned char));

        memset(write_buf, 0, DR_DM_SILENT_BUF_SIZE * sizeof(unsigned char));
        if(buff2use==1)
        {
            memcpy(write_buf, silent_buf1, DR_DM_SILENT_BUF_SIZE * sizeof(unsigned char));
        }
        else if(buff2use==2)
        {
            memcpy(write_buf, silent_buf2, DR_DM_SILENT_BUF_SIZE * sizeof(unsigned char));
        }

        char filestr[100];
        sprintf(filestr, "%ssbuff_%ld.sdm", gSlogPath, gTimeForFile);
        ALOGD("----------File String is gsvfileno : %s  ----------\n" , filestr);
        ALOGD("----------rcvoffset : %d  ----------\n" , rcvoffset);

        sbuff_file = fopen(filestr, "ab");

        if(NULL!=sbuff_file)
        {
            fwrite(write_buf, sizeof(write_buf[0]), rcvoffset, sbuff_file);
            fclose(sbuff_file);
            sbuff_file=NULL;
        }
        else
        {
            ALOGD("----------Opening temp sbuff_file Fail-----\n");
        }
        free(write_buf);
    }

    // All files will be deleted by framework before next start call, so reset the value of rfileno
    rfileno = 0;

    // Clear buffers

    memset(silent_buf1,0,sizeof(silent_buf1));

    memset(silent_buf2,0,sizeof(silent_buf2));

    // Reset write offset
    rcvoffset=0;
    scount=DR_DM_SILENT_BUF_SIZE;

    // Reset file to track for zip
    gnewfileno = 0;

    // set to default
    buff2use = 1;

    // reset file size check
    filesizeoffset = 0;
}

int saveAutoLog()
{
    int ret = 0;
    unsigned long mem_size, copied = 0;
    int dev_fd = -1;
    int autoLogFile = -1;
    char buff[PAGE_SIZE];
    char autoLogSize[MAX_PROP_LEN];

    // open device node
    dev_fd = open(PATH_AUTOLOG, O_RDWR);
    if (dev_fd < 0) {
        ALOGE("[%s]%s open fail (%s)\n", __FUNCTION__, PATH_AUTOLOG, ERR2STR);
        return -ENODEV;
    }

    /* Get AutoLog size */
    property_get(PROP_AUTOLOG_SIZE, autoLogSize, DEF_AUTOLOG_SIZE);
    mem_size = atoi(autoLogSize) * MEGABYTE;

    /* open destination file */
    // To do.
    string srcFilePath = gSlogPath;
    char srcFileName[MAX_FILE_NAME];
    sprintf(srcFileName, "autolog_%ld.btl", CalcTime());
    srcFilePath += srcFileName;

    autoLogFile = open(srcFilePath.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (autoLogFile < 0) {
        ALOGE("[%s]%s open fail (%s)\n", __FUNCTION__, srcFilePath.c_str(), ERR2STR);
        return -ENOENT;
    }

    /* Read & Save shared memory dump */
    while (copied < mem_size) {
        ret = wait_event(dev_fd, POLLIN, KERNEL_TIMEOUT);
        if (ret < 0) {
            ALOGE("[%s]wait event fail!!\n", __FUNCTION__);
            goto exit;
        }

        /* Receive a DUMP frame from CP */
        ret = read(dev_fd, buff, sizeof(buff));
        if (ret < 0) {
            ALOGE("[%s]DUMP read fail (%s)\n", __FUNCTION__, ERR2STR);
            goto exit;
        }

        /* not verified */
        copied += ret;

        ret = write(autoLogFile, buff, ret);
        if (ret < 0) {
            ALOGE("[%s]write fail (%s)\n", __FUNCTION__, ERR2STR);
            goto exit;
        }
    }

    if (fsync(autoLogFile))
	    ALOGE("[%s]fsync is failed (%s)\n", __FUNCTION__, ERR2STR);

    ALOGI("[%s] save complete! (%lu bytes)\n", __FUNCTION__, copied);
exit:
    if (autoLogFile >= 0)
        close(autoLogFile);

    return ret;
}

int wait_event(int fd, short events, long timeout)
{
    int ret;
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events = events;
    while (1) {
        pfd.revents = 0;
        ret = poll(&pfd, 1, timeout);
        if (pfd.revents & events)
            break;

        if (pfd.revents == (POLLERR | POLLHUP)) {
            ret = -EIO;
            goto exit;
        } else if (pfd.revents & POLLHUP) {
            usleep(20000); //20ms wait
            continue;
        }
    }

    return 0;

exit:
    return ret;
}


//slient logging end

/*NNEXT packet  sequence check*/

int dmMsgSeqCheck(char* dmMsg, long length)
{
    /* Use different message seq num, normal DM message and DM trace message */
    /* Ack Seq is not used in DM message, so use it to expand Msg Seq filed */

    //dr_log_print(DR_NORMAL, "----------dmMsgSeqCheck  ----------\n");
    if(dmMsg == NULL)
    {
        ALOGD("----------dmMsg == NULL  ----------\n");
        return 0;
    }

    long junklen = length;
    long countlen = 0;
    unsigned short  xlen =0;
    unsigned short  xmsgseq =0;

    while (junklen >0)
            {

            if(length>= DM_HOSTIF_HEADER_SIZE)
            {
                ipc4hdlcFrame* hdlcFrame = (ipc4hdlcFrame*)dmMsg;

                // Check Start Flag
                if(dmMsg[countlen] != 0x7F)
                {
                    ALOGD("----------Start Flag Error Check again with next byte ----------\n");
                    // Check again with next byte
                    return -1;
                }

                if(hdlcFrame->hdr.length + 2 > length)
                {
                    ALOGD("----------Fragmented message, and whole DM packet is not recieved yet, try again after receiving next packet ----------\n");
                    // Fragmented message, and whole DM packet is not recieved yet, try again after receiving next packet
                    return -2;
                }

                xlen =  dmMsg[countlen+2];
                xlen = xlen<<8;
                xlen += dmMsg[countlen+1];

                // Check End Flag
                if(dmMsg[countlen+xlen + 1] != 0x7E)
                {
                    ALOGD("----------End Flag Error Check again with next byte ----------\n");
                    // Check with next byte
                    return -3;
                }

                 xmsgseq = dmMsg[countlen+7];
                 xmsgseq = xmsgseq <<8;
                 xmsgseq += dmMsg[countlen+6];
                 //ALOGD("xmsgseq value = [%x] \n",  xmsgseq);

                switch (dmMsg[countlen+9])
                {
                    case DM_TRACE_DATA_OUT:
                    case DM_TRACE_DATA_OUT + 0x20:
                    case DM_TRACE_DATA_OUT + 0x40:
                        if(xmsgseq == 0)
                        {
                            if(traceMsgSeq != (DM_MAX_SEQ - 1))

                            {
                                ALOGD("-traceMsgSeq != (DM_MAX_SEQ - 1)---------Sequence error ------ current seq no = [%d]  previous seq no = [%d] missing seq no = [%d] \n" ,
                                xmsgseq, traceMsgSeq, xmsgseq-traceMsgSeq);
                                // Sequence error
                            }
                        }
                        else
                        {
                            if(traceMsgSeq + 1 != xmsgseq)
                            {
                                if(!(traceMsgSeq == 0xffff && xmsgseq == 0xff00))
                                {
                                    ALOGD("----------Sequence error ------ current seq no = [%d]  previous seq no = [%d] missing seq no = [%d] \n" ,
                                    xmsgseq, traceMsgSeq, xmsgseq-traceMsgSeq);
                                }
                                // Sequence Error
                            }
                        }
                        traceMsgSeq = xmsgseq;
                        //ALOGD("xmsgseq value trace= [%x] \n",  xmsgseq);
                        AppendTraceSeqNotoFile(traceMsgSeq);
                        break;
                    case 0x09:
                        // DSP trace message, ignore this.
                        break;
                    default:
                        if(xmsgseq == 0)
                        {
                            if(msgSeq != (DM_MAX_SEQ - 1))
                            {
                                ALOGD("default--traceMsgSeq != (DM_MAX_SEQ - 1)--------Sequence error ------ current seq no = [%d]  previous seq no = [%d]  missing seq no = [%d] \n" ,
                                    xmsgseq, msgSeq, xmsgseq-msgSeq);
                                // Sequence error
                            }
                        }
                        else
                        {
                            if(msgSeq + 1 != xmsgseq)
                            {
                                if(!(msgSeq == 0xffff && xmsgseq == 0xff00))
                                {
                                    ALOGD("default----------Sequence error ------ current seq no = [%d]  previous seq no = [%d] missing seq no = [%d] \n" ,
                                    xmsgseq, msgSeq, xmsgseq-msgSeq);
                                 }
                                // Sequence Error
                            }
                        }
                        msgSeq = xmsgseq;
                        //ALOGD("xmsgseq value normal= [%x] \n",  xmsgseq);
                        AppendSeqNotoFile(msgSeq);
                        break;
                   }
            }
      countlen =   countlen+xlen +2;
      junklen = junklen - (xlen +2);
      // goto next packet
      //return hdlcFrame->hdr.length + 2;
    }
    return 1;
}

// put sequence no. in file
void AppendSeqNotoFile(int32_t seqNo)
{
    unsigned char merge_buf[100]={0,};
    char strSeqNo[100];
    FILE *seq_file=NULL;
    sprintf(strSeqNo,"%02x : ",seqNo);

    std::string seqFile(gSlogPath);
    seqFile += "sequence_num.txt";
    seq_file = fopen(seqFile.c_str(), "ab");

    if(seq_file!=NULL)
    {
        memset(merge_buf, 0, sizeof(merge_buf));
        fwrite(strSeqNo, sizeof(strSeqNo[0]), 8, seq_file);
        fclose(seq_file);
        seq_file=NULL;
    }
    return ;
}

// put trace sequence no. in file
void AppendTraceSeqNotoFile(int32_t seqNo)
{
    unsigned char merge_buf[100]={0,};
    char strSeqNo[100];
    FILE *seq_file=NULL;
    sprintf(strSeqNo,"%02x : ",seqNo);

    std::string traceFile(gSlogPath);
    traceFile += "trace_seq_num.txt";
    seq_file = fopen(traceFile.c_str(), "ab");

    if(seq_file!=NULL)
    {
        memset(merge_buf, 0, sizeof(merge_buf));
        fwrite(strSeqNo, sizeof(strSeqNo[0]), 8, seq_file);
        fclose(seq_file);
        seq_file=NULL;
    }
    return ;
}

// Dump logs to screen
#define isprint(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))

void log_hexdump(const char *format, int lSize, ...)
{
    char str[80], octet[10];
    int ofs, i, l;

    if (lSize < 0) {
        ALOGD("+log_hexdump() invalid size\n");
        return ;
    }

    const int MAX_DUMP_SIZE = (1024 * 2);
    if (lSize >= MAX_DUMP_SIZE) {
        ALOGD("+log_hexdump() dump data over %d KB\n", (MAX_DUMP_SIZE / 1024));
        lSize = MAX_DUMP_SIZE;
    }

    ALOGD("+log_hexdump()\n");

    for (ofs = 0; ofs < lSize; ofs += 16) {
        sprintf( str, "%03d: ", ofs );

        for (i = 0; i < 16; i++) {
            if ((i + ofs) < lSize)
                sprintf( octet, "%02x ", format[ofs + i] );
            else
                strcpy( octet, "   " );

                strcat( str, octet );
            }
            strcat( str, "  " );
            l = strlen( str );

            for (i = 0; (i < 16) && ((i + ofs) < lSize); i++)
                str[l++] = isprint( format[ofs + i] ) ? format[ofs + i] : '.';

            str[l] = '\0';
            ALOGD("%s\n", str);
    }

    ALOGD("-log_hexdump()\n");

}

int32_t main(void)
{
    umask(000);

    /* get path of silent log */
    property_get(PROP_SLOG_PATH, gSlogPath, DEF_SLOG_PATH);
    ALOGD("Slog path : %s", gSlogPath);

    if (!mkdir_slog())
        return 0;

    // Init DMAgent
    DMAgent_Init();

    // Init DMFileManager
    DMFileManager *fileManager = DMFileManager::getInstance();
    if (fileManager != NULL) {
        fileManager->init();
    }

    if (!init_monitor()) {
        ALOGE("init monitor fail");
        return 0;
    }

    start_monitor_thread();
    run_socket_monitor(&g_modem_fd);

    while(1) //never enter and stop
    {
    }

    dmd_close();

    return 0;
}
