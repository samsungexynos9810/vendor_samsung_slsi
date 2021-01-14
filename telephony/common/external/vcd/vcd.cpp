/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#include <cutils/properties.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <cutils/android_filesystem_config.h>

#include "vcd.h"
#include "vcd_socket.h"
#include "rilclienthelper.h"

#define PATH_MODEM            "/dev/umts_router"
#define PATH_ATC              "/dev/umts_atc0"
#define PATH_USB              "/dev/ttyGS0"
#define PATH_USB2             "/dev/ttyGS2"
#define PATH_SERIAL           "/dev/ttySAC1"
#define UART_PATH_UARTSEL     "/sys/class/sec/switch/uart_sel"
#define COMMAND_LINE_FILE     "/proc/cmdline"
#define KERNEL_CONSOLE_CHECK  "console=ttySAC1,115200n8"
#define MAX_BUF               65536
#define OK_STRING             "OK\n"
#define AT_STRING             "AT\r\n"
#define BOOT_COMPLETE_STRING  "BOOT COMPLETE\n"
#define IOCTL_MODEM_STATUS    _IO('o', 0x27)
#define PROPERTY_DUN_SETTINGS "persist.vendor.radio.dun_settings"

enum modem_state {
    STATE_OFFLINE,
    STATE_CRASH_RESET,          /* silent reset */
    STATE_CRASH_EXIT,           /* cp ramdump */
    STATE_BOOTING,
    STATE_ONLINE,
};

static int32_t g_usb_fd = -1;
static int32_t g_usb_fd2 = -1;
static int32_t g_modem_fd = -1;
static int32_t g_atc_fd = -1;
static int32_t g_serial_fd = -1;
static int32_t g_serial_decision_check = -1;
static int32_t g_app_decision_check = 0;
static int32_t g_board_type_check = 0;
static int32_t g_cp_at_ok_check = 0;
static char g_dun_mode[100] = {0, };
extern int32_t g_current_type;
RilClientHelper *g_pRilClientHelper = NULL;
HANDLE g_client;
FILE *infile;

static int s_fdWakeupRead;
static int s_fdWakeupWrite;
static int s_fdWakeupRead2;
static int s_fdWakeupWrite2;

int get_client_socket(void);
void *run_socket_monitor(void *arg);
int get_modem_write_from_app(void);

static int RouteCommandsToModem(const char *data, size_t datalen);
static int RouteCommandsToConnectivity(const char *data, size_t datalen);
static int RouteCommandsToAIMSConnectivity(const char *data, size_t datalen);
static int RouteCommandsToRIL(const char *data, size_t datalen);
static int RouteCommandsFilterredHandler(const char *data, size_t datalen);
static int RouteCommandsDefaultHandler(const char *data, size_t datalen);
static int RouteCommandsErrorHandler(const char *data, size_t datalen);
static int RouteCommands(const char *data, size_t datalen);
static int BufferedInput(const char *data, size_t datalen);
static int ATCommandsResponseHandler(int device, char *data, size_t datalen);
static int ATCommandsRequestHandler(int device, char *data, size_t datalen);
static void OnSolicitedResponse(unsigned msgId, int status, void* data, size_t datalen, unsigned int channel);
static void OnUnsolicitedResponse(unsigned int msgId, void* data, size_t length, unsigned int channel);

void close_modem(void)
{
    if (g_modem_fd != -1) {
        close(g_modem_fd);
        g_modem_fd = -1;
    }
}

void close_atc(void)
{
    if (g_atc_fd != -1) {
        close(g_atc_fd);
        g_atc_fd = -1;
    }
}

void close_usb(void)
{
    if (g_usb_fd != -1) {
        int ret = close(g_usb_fd);
        if (ret < 0) {
            ALOGD("%s close fd=%d ret=%d errno=%d", __FUNCTION__, g_usb_fd, ret, errno);
        }
        g_usb_fd = -1;
    }
}

void close_usb2(void)
{
    if (g_usb_fd2 != -1) {
        int ret = close(g_usb_fd2);
        if (ret < 0) {
            ALOGD("%s close fd2=%d ret=%d errno=%d", __FUNCTION__, g_usb_fd2, ret, errno);
        }
        g_usb_fd2 = -1;
    }
}

void close_serial(void)
{
     if (g_serial_fd != -1) {
        close(g_serial_fd);
        g_serial_fd = -1;
    }
}

void close_interface(void)
{
    close_usb();
    close_usb2();
    close_modem();
    close_atc();
    close_serial();
}

int32_t init_usb(void)
{
    struct termios usb_termios;

    while(1) {
        g_usb_fd = open(PATH_USB, O_RDWR);
        if (g_usb_fd < 0) {
            ALOGE("%s : Fail to open %s, errno = %d", __func__, PATH_USB, errno);
            sleep(1);
            continue;
        } else {
            ALOGD("%s : Success to open %s, fd = %d", __func__, PATH_USB, g_usb_fd);
            break;
        }
    }

    //set params
    memset((char *)&usb_termios, 0, sizeof(struct termios));

    if (tcgetattr(g_usb_fd, &usb_termios) < 0) {
        ALOGE("%s : Fail to get attributes from %s, error = %d", __func__, PATH_USB, errno);
        return -1;
    }

    cfmakeraw(&usb_termios);

    usb_termios.c_iflag &= ~ICRNL;
    usb_termios.c_iflag &= ~INLCR;
    usb_termios.c_oflag &= ~OCRNL;
    usb_termios.c_oflag &= ~ONLCR;
    usb_termios.c_lflag &= ~ICANON;
    usb_termios.c_lflag &= ~ECHO;

    if (tcsetattr(g_usb_fd, TCSANOW, &usb_termios) < 0) {
        ALOGE("%s : Fail to set attributes to %s, error = %d", __func__, PATH_USB, errno);
        return -1;
    }
    return 0;
}

int32_t init_usb2(void)
{
    struct termios usb_termios;

    while(1) {
        g_usb_fd2 = open(PATH_USB2, O_RDWR);
        if (g_usb_fd2 < 0) {
            sleep(1);
            continue;
        } else {
            ALOGD("%s : Success to open %s, fd = %d", __func__, PATH_USB2, g_usb_fd2);
            break;
        }
    }

    //set params
    memset((char *)&usb_termios, 0, sizeof(struct termios));

    if (tcgetattr(g_usb_fd2, &usb_termios) < 0) {
        ALOGE("%s : Fail to get attributes from %s, error = %d", __func__, PATH_USB2, errno);
        return -1;
    }

    cfmakeraw(&usb_termios);

    usb_termios.c_iflag &= ~ICRNL;
    usb_termios.c_iflag &= ~INLCR;
    usb_termios.c_oflag &= ~OCRNL;
    usb_termios.c_oflag &= ~ONLCR;
    usb_termios.c_lflag &= ~ICANON;
    usb_termios.c_lflag &= ~ECHO;

    if (tcsetattr(g_usb_fd2, TCSANOW, &usb_termios) < 0) {
        ALOGE("%s : Fail to set attributes to %s, error = %d", __func__, PATH_USB2, errno);
        return -1;
    }
    return 0;
}

int32_t init_modem(void)
{
    while(1) {
        g_modem_fd = open(PATH_MODEM, O_RDWR);
        if (g_modem_fd < 0) {
            ALOGE("%s : Fail to open %s, errno = %d", __func__, PATH_MODEM, errno);
            sleep(1);
            continue;
        } else {
            break;
        }
    }
    return 0;
}

int32_t init_atc(void)
{
    while(1) {
        g_atc_fd = open(PATH_ATC, O_RDWR);
        if (g_atc_fd < 0) {
            sleep(1);
            continue;
        } else {
            ALOGD("%s : Success to open %s, fd = %d", __func__, PATH_ATC, g_atc_fd);
            break;
        }
    }
    return 0;
}


int32_t init_serial(void)
{
    if(g_board_type_check==1)       //board is mochagw01
    {
    if(g_serial_decision_check == -1 || g_serial_decision_check == 1)   // re-init allowed only when in console mode
    {
        struct termios newtio;

        char cmdlinebuf[200];

        int readcnt =0;
        memset(cmdlinebuf, 0, 200);

        FILE *cmdline_file = NULL;

        readcnt = GetSysFS(COMMAND_LINE_FILE , cmdlinebuf);

        if(g_serial_decision_check == -1)   //only first time show log
        {
            ALOGD("%s : COMMAND_LINE_FILE open = %s", __func__, cmdlinebuf);
            ALOGD("%s : COMMAND_LINE_FILE readcnt = %d", __func__, readcnt);
        }

        if(readcnt>0)
        {
            int ii=0;
            char *confind;
            for(ii;ii<readcnt -24; ii++)
            {
                confind= &cmdlinebuf[ii];
                if(commandcmp(confind,KERNEL_CONSOLE_CHECK,24)==0)
                {
                    ALOGD("%s : KERNEL_CONSOLE_CHECK found at index = %d", __func__, ii);
                    g_serial_decision_check = 0;
                    break;
                }
                g_serial_decision_check = 1;    //kernel is not using console , do init console
            }
        }

        if(g_serial_decision_check==1)
        {
             ALOGD("%s : KERNEL_CONSOLE_CHECK not found !! do console initialization", __func__);

            while(1) {

                g_serial_fd = open( PATH_SERIAL, O_RDWR | O_NOCTTY );
                if (g_serial_fd < 0) {
                    ALOGE("%s : Fail to open %s, errno = %d", __func__, PATH_SERIAL, errno);
                    sleep(1);
                    continue;
                } else {
                    break;
                }
            }
            ALOGD("%s : g_serial_fd open = %d", __func__, g_serial_fd);

            memset((char *)&newtio, 0, sizeof(newtio));
            if (tcgetattr(g_serial_fd, &newtio) < 0) {
                ALOGE("%s : Fail to get attributes from %s, error = %d", __func__, PATH_SERIAL, errno);
                return -1;
            }

            cfmakeraw(&newtio);

            newtio.c_iflag = IGNPAR; // non-parity
            newtio.c_oflag = 0;
            newtio.c_cflag = CS8 | CLOCAL | CREAD; // NO-rts/cts
            newtio.c_cflag |= B115200;

            //set input mode (non-canonical, no echo,.....)
            newtio.c_lflag = 0;
            newtio.c_cc[VTIME] = 10; // timeout
            newtio.c_cc[VMIN] = 32;

             if (tcsetattr(g_serial_fd, TCSANOW, &newtio) < 0) {
                ALOGE("%s : Fail to set attributes to %s, error = %d", __func__, PATH_SERIAL, errno);
                return -1;
                }
            }
        }
    }
    return 0;
}

int32_t init_interface(void)
{
    if (init_usb() < 0) {
            return -1;
    }

    if (init_modem() < 0) {
            return -1;
    }

    if (init_serial() < 0) {
            return -1;
    }
    return 0;
}

int32_t write_to_interface(int32_t fd, const char* buffer, int32_t buf_len)
{
    int32_t write_len = 0;
    int32_t len = 0;

    if (buffer == NULL || buf_len <= 0) {
        ALOGD("%s : invalid parameter\n", __FUNCTION__);
        return -1;
    }

    do {
        if ((len = write(fd, buffer + write_len, (buf_len - write_len))) < 0) {
            ALOGE("%s : Fail to write", __func__);
            if (errno == EINTR || errno == EAGAIN)
                continue;
            return -1;
        }
        write_len += len;
    } while (write_len < buf_len);
    return write_len;
}

void *run_modem_monitor(void *arg)
{
    int32_t n;
    fd_set rfds;
    char buffer[MAX_BUF];
    int32_t fd = *((int32_t*)arg);
    CommandResponseHandler fpResponseHandler = ATCommandsResponseHandler;
    CommandRequestHandler fpRequestHandler = ATCommandsRequestHandler;

    while(1) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        n = select(fd + 1, &rfds, NULL, NULL, NULL);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            ALOGE("%s : Fail to monitor %s, errno = %d", __func__, PATH_MODEM, errno);
            break;
        }

        if (FD_ISSET(fd, &rfds)) {
            n = read(fd, &buffer, MAX_BUF - 1);
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                ALOGE("%s : Fail to read %s, errno = %d", __func__, PATH_MODEM, errno);
                close_modem();
                break;
            } else if (n == 0) {// status unpluged
                ALOGE("%s : No data on %s", __func__, PATH_MODEM);
                close_modem();
                break;
            }

            //ALOGD("%s : Read %s buffer = %s n = %d", __func__, PATH_MODEM, buffer, n);
            ALOGD("%s : Read %s n = %d", __func__, PATH_MODEM, n);
            char *checkok = &buffer[0];
            buffer[n] = 0;
            if(strstr (checkok, "OK") != NULL && g_cp_at_ok_check==0)
            {
                ALOGD("%s :Success OK found", __func__);
                g_cp_at_ok_check =1;
            }

            if (!g_current_type && fpResponseHandler != NULL) {
                fpResponseHandler(DEVICE_TYPE_MODEM, buffer, n);
            } else if (SendDataToConnectivity(buffer, n, ID_APP) > 0) {
                PrintATCommands("To APPConnectivity", buffer, n);
            }
        }
    }
    return NULL;
}

void *run_atc_monitor(void *arg)
{
    int32_t n;
    fd_set rfds;
    char buffer[MAX_BUF];

    while(1) {
        if (g_atc_fd < 0) {
            init_atc();
        }
        int32_t fd = g_atc_fd;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        n = select(fd + 1, &rfds, NULL, NULL, NULL);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            ALOGE("%s : Fail to monitor %s, errno = %d", __func__, PATH_ATC, errno);
            close_atc();
            continue;
        }

        if (FD_ISSET(fd, &rfds)) {
            n = read(fd, &buffer, MAX_BUF - 1);
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                ALOGE("%s : Fail to read %s, errno = %d", __func__, PATH_ATC, errno);
                close_atc();
                continue;
            } else if (n == 0) {// status unpluged
                ALOGE("%s : No data on %s", __func__, PATH_ATC);
                close_atc();
                continue;
            }

            ALOGD("%s : Read %s n = %d", __func__, PATH_ATC, n);
            char *checkok = &buffer[0];
            buffer[n] = 0;
            if(strstr (checkok, "OK") != NULL && g_cp_at_ok_check==0)
            {
                ALOGD("%s :Success OK found", __func__);
                g_cp_at_ok_check =1;
            }

            if (n > 0) {
                HexDump(buffer, n);
                PrintATCommands("To USB2", buffer, n);
                write_to_interface(g_usb_fd2, buffer, n);
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
    int32_t fd = *((int32_t*)arg);

    while(1) {
        fd = g_usb_fd;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        FD_SET(s_fdWakeupRead, &rfds);
        int maxFd = fd;
        if (maxFd < s_fdWakeupRead)
            maxFd = s_fdWakeupRead;

        n = select(maxFd + 1, &rfds, NULL, NULL, NULL);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            ALOGE("%s : Fail to monitor %s, errno = %d", __func__, PATH_USB, errno);
            //return NULL;
            continue;
        }

        if (FD_ISSET(fd, &rfds)) {
            memset(buffer, '\0', MAX_BUF);
            n = read(fd, &buffer, MAX_BUF - 1);
            ALOGE("read from fd=%d", fd);
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                ALOGE("%s : Fail to read %s, errno = %d", __func__, PATH_USB, errno);
                close_usb();
                init_usb();
                //break;
                continue;

            } else if (n == 0) { // status unpluged
                ALOGE("%s : No data on %s", __func__, PATH_USB);
                close_usb();
                init_usb();
                //break;
                continue;
            }

            if (g_dun_mode[0] == '0') { // normal mode
                RouteCommandsDefaultHandler(buffer, n);
            } else {
                BufferedInput(buffer, n);
            }
        }
        else if (FD_ISSET(s_fdWakeupRead, &rfds)) {
            ALOGE("%s : wakeup", __FUNCTION__);
            n = read(s_fdWakeupRead, &buffer, 1);
            close_usb();
            init_usb();
        }
    }
    return NULL;
}

void *run_usb2_monitor(void *arg)
{
    int32_t n;
    fd_set rfds;
    char buffer[MAX_BUF];
    int32_t fd;

    while(1) {
        if (g_usb_fd2 < 0) {
            init_usb2();
        }

        fd = g_usb_fd2;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        FD_SET(s_fdWakeupRead2, &rfds);
        int maxFd = fd;
        if (maxFd < s_fdWakeupRead2)
            maxFd = s_fdWakeupRead2;

        n = select(maxFd + 1, &rfds, NULL, NULL, NULL);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            ALOGE("%s : Fail to monitor %s, errno = %d", __func__, PATH_USB2, errno);
            close_usb2();
            continue;
        }

        if (FD_ISSET(fd, &rfds)) {
            memset(buffer, '\0', MAX_BUF);
            n = read(fd, &buffer, MAX_BUF - 1);
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                ALOGE("%s : Fail to read %s, errno = %d", __func__, PATH_USB2, errno);
                close_usb2();
                continue;

            } else if (n == 0) { // status unpluged
                ALOGE("%s : No data on %s", __func__, PATH_USB2);
                close_usb2();
                continue;
            }

            PrintATCommands("From USB2", buffer, n);

            // route to ATC
            if (write_to_interface(g_atc_fd, (char *)buffer, n) > 0) {
                PrintATCommands("To ATC", buffer, n);
            }
        }
        else if (FD_ISSET(s_fdWakeupRead2, &rfds)) {
            ALOGE("%s : wakeup", __FUNCTION__);
            n = read(s_fdWakeupRead2, &buffer, 1);
            close_usb2();
        }
    }
    return NULL;
}

//test start app socket conn
int get_modem_fd(void)
{
return g_modem_fd;
}

//set application swicth to write to modem
int set_modem_write_from_app(int appswitch)
{
    g_app_decision_check = appswitch;
    return 0;
}
int get_modem_write_from_app(void)
{
    return g_app_decision_check;
}
// test end
void start_modem_monitor_thread(void)
{
    pthread_t thread_id_modem = 0;
    //run thread
    pthread_create(&thread_id_modem, NULL, run_modem_monitor, &g_modem_fd);
}

void start_atc_monitor_thread(void)
{
    pthread_t thread_id_atc = 0;
    //run thread
    pthread_create(&thread_id_atc, NULL, run_atc_monitor, &g_atc_fd);
}

void start_usb_monitor_thread(void)
{
    pthread_t thread_id_usb = 0;
    //run thread
    pthread_create(&thread_id_usb, NULL, run_usb_monitor, &g_usb_fd);
}

void start_usb2_monitor_thread(void)
{
    pthread_t thread_id_usb2 = 0;
    //run thread
    pthread_create(&thread_id_usb2, NULL, run_usb2_monitor, &g_usb_fd2);
}

void start_app_socket_monitor_thread(void)
{
#if 0
    pthread_t thread_id_app_socket = 0;
    //run thread
    pthread_create(&thread_id_app_socket, NULL, run_socket_monitor, &g_modem_fd);
#endif
    StartConnectivityServer(ATCommandsResponseHandler, ATCommandsRequestHandler);
}

void start_monitor_thread(void)
{
    start_modem_monitor_thread();
    start_atc_monitor_thread();
    start_usb_monitor_thread();
    start_usb2_monitor_thread();
    start_app_socket_monitor_thread();
}

void switch_user(void)
{
    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
    setuid(AID_SYSTEM);

    struct __user_cap_header_struct header;
    struct __user_cap_data_struct cap;
    header.version = _LINUX_CAPABILITY_VERSION;
    header.pid = 0;

    cap.effective = cap.permitted = ( 1 << CAP_SYS_BOOT) | (1 << CAP_NET_ADMIN) |\
                    (1 << CAP_SYS_ADMIN) | (1 << CAP_NET_RAW) | (1<<CAP_DAC_OVERRIDE);

    cap.inheritable = 0;
    capset(&header, &cap);
}

int32_t main(void)
{
    ALOGE("%s : enter ", __func__);

    int fdSet[2];
    int retpipe = pipe(fdSet);
    if (retpipe == 0) {
        s_fdWakeupRead = fdSet[0];
        s_fdWakeupWrite = fdSet[1];
    }
    int ret2pipe = pipe(fdSet);
    if (ret2pipe == 0) {
        s_fdWakeupRead2 = fdSet[0];
        s_fdWakeupWrite2 = fdSet[1];
    }

    if (init_interface() < 0) {
        ALOGE("Exit");
        return 0;
    }

    // get property for checking dun mode
    property_get(PROPERTY_DUN_SETTINGS, g_dun_mode, "0");
    ALOGD("dun mode = %c", g_dun_mode[0]);
    if (g_pRilClientHelper == NULL) {
        g_pRilClientHelper = RilClientHelperFactory::CreateHelperInstance();
    }

    /*
    if (g_pRilClientHelper != NULL) {
        for (int i = 0; i < 10; ++i) {
            g_client = g_pRilClientHelper->Open();
            if (g_client != 0) {
                ALOGD("g_pRilClientHelper is opened");
                break;
            }
            else usleep(200000);
        }
        g_pRilClientHelper->RegisterUnsolicitedResponseHandler(g_client, OnUnsolicitedResponse);
    }
    */

    start_monitor_thread();


    while(1) {
        if(g_modem_fd == -1) {
            if (init_modem() > -1) {
                start_modem_monitor_thread();
            }
        }
        sleep(1);
    }
    close_interface();
    return 0;
}

#define MAX_CMD_BUF_SIZE    (1024 * 4)
int BufferedInput(const char *data, size_t datalen)
{
    static char buffer[MAX_CMD_BUF_SIZE + 1] = {0, };
    static int c = 0;
    static int clear = 0;
    unsigned int i = 0;

    if (data == NULL || *data == 0 || datalen == 0/* || datalen > MAX_CMD_BUF_SIZE*/) {
        return -1;
    }

    // echo
    PrintATCommands("echo", data, datalen);
    write_to_interface(g_usb_fd, data, datalen);

    while (i < datalen) {
        char ch = data[i++];
        int flush = 0;
        buffer[c++] = ch;
        flush = (ch == '\r' || ch == '\n');
        if (i < datalen && ch == '\r' && data[i] == '\n') {
            if (c < MAX_CMD_BUF_SIZE) {
                buffer[c++] = data[i++];
            }
            else {
                i++;
            }
        }
        buffer[c] = 0;

        if (c == MAX_CMD_BUF_SIZE) {
            flush = 1;
        }

        if (flush) {
            ALOGE("%s flush c=%d", __FUNCTION__, c);
            RouteCommands(buffer, strlen(buffer));
            memset(buffer, 0, sizeof(buffer));
            c = 0;
            PrintATCommands("Buffered Input", buffer, c);
        }
    }

    return 0;
}

int RouteCommands(const char *data, size_t datalen)
{

    const int MAX_DEVICE = 3;
    static CommandsRouteHandler fpDestDevices[] = {
        RouteCommandsToConnectivity,
        RouteCommandsToAIMSConnectivity,
        RouteCommandsToRIL,
        RouteCommandsToModem,
        RouteCommandsFilterredHandler,
        RouteCommandsDefaultHandler,
        //RouteCommandsErrorHandler,
        NULL,
    };

    if (g_dun_mode[0] == '0') {
        fpDestDevices[0] = RouteCommandsDefaultHandler;
        fpDestDevices[1] = NULL;
    }

    int size = sizeof(fpDestDevices) / sizeof(fpDestDevices[0]);
    int i = 0;
    while (i < size) {
        if (fpDestDevices[i] == NULL) {
            break;
        }

        if (fpDestDevices[i](data, datalen) == 0) {
            return 0;
        }
        i++;
    }
    PrintATCommands("Not found destination", data, datalen);
    return 0;
}

static int IsValidATCommand(const char *cmd, size_t cmdlen)
{
    if (cmd != NULL && *cmd != 0) {
        size_t len = strlen(cmd);
        if (len == cmdlen) {
            if (len > 2 && cmd[len - 2] == '\r' && cmd[len - 1] == '\n')
                return 1;
            if (len > 1 && (cmd[len - 1] == '\r' || cmd[len - 1] == '\n'))
                return 1;
        }
    }
    return  0;
}

static int IsEmptyString(const char *str)
{
    if (str == NULL || *str == 0) {
        return 1;
    }

    int i = strlen(str);
    while (i >= 0) {
        if (!(*(str + i - 1) == '\r' || *(str + i - 1) == '\n' || *(str + i - 1) == ' ')) {
            break;
        }
        i--;
    }

    return (i < 0);
}

int RouteCommandsToModem(const char *data, size_t datalen)
{
    static const char *startWith = {
        "AT+",
    };

    if (IsValidATCommand(data, datalen)) {
        if (strncasecmp(data, startWith, strlen(startWith)) == 0 && !g_current_type) {
            // route to Modem
            if (write_to_interface(g_modem_fd, (char *)data, datalen) > 0) {
                PrintATCommands("To Modem", data, datalen);
            } else {
                ALOGE("RouteCommandsToModem###############RouteCommandsErrorHandler: Warning ###############");
                RouteCommandsErrorHandler(data, datalen);
            }
            return 0;
        }
    }
    return -1;
}

int RouteCommandsToConnectivity(const char *data, size_t datalen)
{
    static const char *startWith = "AT^";
    if (IsValidATCommand(data, datalen)) {
        if (strncasecmp(data, startWith, strlen(startWith)) == 0) {
            // route to Connectivities
            if (SendDataToConnectivity(data, datalen, ID_NV) > 0) {
                PrintATCommands("To Connectivity", data, datalen);
            } else {
                RouteCommandsErrorHandler(data, datalen);
            }
            return 0;
        }
    }
    return -1;
}

int RouteCommandsToAIMSConnectivity(const char *data, size_t datalen)
{
    static const char *startWith = "AT#AIMS";
    if (IsValidATCommand(data, datalen)) {
        if (strncasecmp(data, startWith, strlen(startWith)) == 0) {
            // route to AIMSConnectivities
            if (SendDataToConnectivity(data, datalen, ID_IMS) > 0) {
                PrintATCommands("To IMSConnectivity", data, datalen);
            } else {
                ALOGE("###############RouteCommandsErrorHandler: Warning ###############");
                RouteCommandsErrorHandler(data, datalen);
            }
            return 0;
        }
    }
    return -1;
}

int RouteCommandsToRIL(const char *data, size_t datalen)
{
    static const char *startWith = "AT+VZWAPNE";
    if (IsValidATCommand(data, datalen)) {
        if (strncasecmp(data, startWith, strlen(startWith)) == 0) {
            // route to RIL
            if (g_client == NULL) {
                g_client = g_pRilClientHelper->Open();
                g_pRilClientHelper->RegisterUnsolicitedResponseHandler(g_client,OnUnsolicitedResponse);
            }

            if (g_dun_mode[0] == '1' && g_client != NULL) {
                g_pRilClientHelper->Send(g_client, RILC_REQ_MISC_APN_SETTINGS, (void *)data, datalen, OnSolicitedResponse);
                return 0;
            }
        }
    }
    return -1;
}

int RouteCommandsFilterredHandler(const char *data, size_t datalen)
{
    static const char *ignoreStartWith[] = {
        "\r\n",
        "\r",
        "\n",
    };

    unsigned int i = 0;
	unsigned int size = (unsigned int)(sizeof(ignoreStartWith)/sizeof(ignoreStartWith[0]));
    // ignore
    for (i = 0; i < size; i++) {
        if (strcasecmp(data, ignoreStartWith[i]) == 0 || IsEmptyString(data)) {
            PrintATCommands("Filterred Handler", data, datalen);
            // ignore
            return 0;
        }
    }
    return -1;
}

int RouteCommandsDefaultHandler(const char *data, size_t datalen)
{
    PrintATCommands("Default Handler", data, datalen);

    // route to Modem
    if (write_to_interface(g_modem_fd, (char *)data, datalen) > 0) {
        PrintATCommands("To Modem", data, datalen);
        return 0;
    }

    return -1;
}

int RouteCommandsErrorHandler(const char *data, size_t datalen)
{
    PrintATCommands("Error Handler", data, datalen);

    // send back result
    const char *retError = "\r\nERROR\r\n";
    write_to_interface(g_usb_fd, retError, strlen(retError));
    PrintATCommands("Error Return", retError, strlen(retError));
    return 0;
}

int ATCommandsResponseHandler(int device, char *data, size_t datalen)
{
    const char *tag = "";
    switch (device) {
    case DEVICE_TYPE_MODEM:
        tag = "Rx(Modem)";
        break;
    case DEVICE_TYPE_CONNECTIVITY:
        tag = "Rx(Connectivity)";
        break;
    case DEVICE_TYPE_IMSCONNECTIVITY:
        tag = "Rx(IMSConnectivity)";
        break;
    case DEVICE_TYPE_APP:
        tag = "Rx(APP)";
        break;
    case DEVICE_TYPE_RIL:
        tag = "Rx(RIL)";
        break;
    default:
        tag = "Rx";
        break;
    } // end switch ~

    if (data != NULL && datalen > 0) {
        HexDump(data, datalen);
        PrintATCommands(tag, data, datalen);
        write_to_interface(g_usb_fd, data, datalen);
    }
    return 0;
}

int ATCommandsRequestHandler(int device, char *data, size_t datalen)
{
    const char *tag = "";
    switch (device) {
    case DEVICE_TYPE_MODEM:
        tag = "Tx(Modem)";
        break;
    case DEVICE_TYPE_CONNECTIVITY:
        tag = "Tx(Connectivity)";
        break;
    case DEVICE_TYPE_IMSCONNECTIVITY:
        tag = "Tx(IMSConnectivity)";
        break;
    case DEVICE_TYPE_APP:
        tag = "Tx(APP)";
        break;
    default:
        tag = "Tx";
        break;
    } // end switch ~

    if (data != NULL && datalen > 0) {
        HexDump(data, datalen);
        PrintATCommands(tag, data, datalen);
        write_to_interface(g_modem_fd, data, datalen);
    }

    return 0;
}

void OnSolicitedResponse(unsigned msgId, int status, void* data, size_t datalen, unsigned int channel)
{
    ALOGD("%s()", __FUNCTION__);

    if (msgId == RILC_REQ_MISC_APN_SETTINGS)
    {
        ATCommandsResponseHandler(DEVICE_TYPE_RIL, (char *)data, datalen);
        //write_to_interface(g_usb_fd, (char *)data, datalen);
    }
}

void OnUnsolicitedResponse(unsigned int msgId, void* data, size_t length, unsigned int channel)
{
    if (msgId == RILC_UNSOL_PIN_CONTROL)
    {
        char *res = (char *)data;
        ALOGD("%s() msgId = %d, signal = %d, status = %d", __FUNCTION__, msgId, res[0], res[1]);
        /*
                * data[0] - signal
                * 0x00 = PIN_CTRL_SIGNAL_NONE, 0x01 = PIN_CTRL_SIGNAL_DCD, 0x02 = PIN_CTRL_SIGNAL_DTR
                * data[1] = status
                * 0x00 = PIN_CTRL_STATUS_OFF, 0x01 = PIN_CTRL_STATUS_ON
                */
        if (res[0] == 1 && res[1] == 0)
        {
            ALOGD("%s close", __FUNCTION__);
            int ret;
            do {
                ret = write (s_fdWakeupWrite, " ", 1);
            } while (ret < 0 && errno == EINTR);
            do {
                ret = write (s_fdWakeupWrite2, " ", 1);
            } while (ret < 0 && errno == EINTR);
        }
    }
}

