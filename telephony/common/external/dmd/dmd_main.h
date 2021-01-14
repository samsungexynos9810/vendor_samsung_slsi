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
#include <dirent.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/statvfs.h>

#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <private/android_filesystem_config.h>
#include <poll.h>
#include <sys/poll.h>
#include <ziparchive/zip_writer.h>

#include "cutils/sockets.h"

#define LOG_TAG     "DMD"
#include <utils/Log.h>


#define PATH_MODEM              "/dev/umts_dm0"
#define PATH_USB                "/dev/ttyGS1"
#define PATH_BOOT               "/dev/umts_boot0"
#define PATH_AUTOLOG            "/dev/ramdump_memshare"
#define SOCKET_DMD              "socket_dmd"
#define SNAPSHOT_DIR            "snapshot"
#define SANPSHOT_SLOG_FILENAME  "snapshot/silent_log.sdm"
#define SOCKET_MAX_BUF          4096
#define MAX_BUF                 65536
#define MAX_BUF_SYMB            65500
#define USB_DM_PROP_CHECK       "dm"
#define IOCTL_MODEM_STATUS      _IO('o', 0x27)
#define IOCTL_BTL_DUMP          _IO('o', 0x59)
#define DEF_SLOG_PATH           "/data/vendor/slog/"
#define MAX_PROP_LEN            128
#define KERNEL_TIMEOUT          50000
#define MAX_FILE_NAME           128
#define ERR2STR                 strerror(errno)
#define DEF_AUTOLOG_SIZE        "32"
#define MEGABYTE                (1024 * 1024)

// property
#define PROPERTY_DMD_ENABLE_LOG             "vendor.sys.dmd.enable.logdump"
#define PROP_SLOG_PATH                      "vendor.sys.exynos.slog.path"
#define PROPERTY_SILENTLOG_MODE             "vendor.sys.silentlog.on"
#define PROP_AUTOLOG_SIZE                   "persist.vendor.sys.autolog.size"

extern int32_t g_dmd_mode;
extern int32_t g_silent_logging_started;
extern int32_t g_silent_logging_asked;
extern int32_t g_modem_fd;
extern bool g_have_versioninfo;
extern int64_t gTimeForFile;
extern char gSlogPath[MAX_PROP_LEN];
extern int gDmFileMaxSize;

enum modem_state {
    STATE_OFFLINE,
    STATE_CRASH_RESET,          /* silent reset */
    STATE_CRASH_EXIT,           /* cp ramdump */
    STATE_BOOTING,
    STATE_ONLINE,
};

typedef enum {
    MODE_OFF,
    MODE_NNEXT,
    MODE_SILENT,
    MODE_OBDM,
    MODE_EXT_APP,
}DMD_MODE;

typedef enum {
    PROFILE_SEND_DONE,
    PROFILE_SEND,
}PROFILE_STATE;

void usb_close(void);
void modem_close(void);
void dmd_close(void);
bool usb_init(void);
bool modem_init(void);
bool init_monitor(void);
int dmdWriteUsbToModem(int fd, char* buffer, int buf_len);
int32_t dmd_modem_write(int32_t fd, char* buffer, int32_t buf_len);
int32_t dmd_write(int32_t fd, char* buffer, int32_t buf_len);
void *run_modem_monitor(void *arg);
void *run_usb_monitor(void *arg);
void run_socket_monitor(int32_t *fd);
void start_monitor_thread(void);
bool mkdir_slog(void);
void switchUser(void);
void DoFileOperation(int32_t buffno, int32_t offset, uint64_t timeStamp);
void *DoFileOperationThread(void *arg);
void trigger_save_dump(void);
void MergeLogFiles(uint64_t timeStamp);
void set_cp2usb_path(bool value);
int32_t is_cp2usb_path(void);
void OnSilentLogStop(void);
void MergeHeader(uint64_t timeStamp);
int ReadMaxFileNo(void);
int dmMsgSeqCheck( char* dmMsg, long length);
void AppendSeqNotoFile(int32_t seqNo);
void AppendTraceSeqNotoFile(int32_t seqNo);
void log_hexdump(const char *format, int  lSize, ...);
int EnableLogDump();
void DoSaveProfile(char * buff, uint32_t count);
void DoSendProfile(int32_t fd);
int CheckModemStatus(int32_t fd);
void CheckVersionInfo(char *buff, int32_t buff_len);
uint64_t CalcTime();
int DoZipOperation(uint64_t timeStamp);
int saveAutoLog();
int wait_event(int fd, short events, long timeout);
void MergeVersionInfo(uint64_t timeStamp);
