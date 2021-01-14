/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __CBD_EXYNOS_H__
#define __CBD_EXYNOS_H__

#ifdef LINK_MAIN_PCIE
/*
 * When using PCIE link, it need to allocate SKB buffer for CP download
 * And it needs kmalloc memory in kernel side. 32KB memory is too big to
 * allocate in kernel side. So I change PAYLOAD value to 8KB(2 pages) to
 * increase the possibility of allocation.
 */
#define EXYNOS_PAYLOAD_LEN          (7*1024)  /* reserved room 1KB for header */
#else
#define EXYNOS_PAYLOAD_LEN          (32*1024) // payload can be 2K to 63K
#endif

#define EXYNOS_MSG_INFO_SIZE        12
#define TIMEOUT                     50000
#define MONITORING_TIMEOUT          (-1)

#define MSG_SHIFT_DIRECTION(_x)     (_x << 12)
#define MSG_SHIFT_CMD(_x)           (_x << 8)
#define MSG_SHIFT_INDEX(_x)         (_x << 4)
#define MSG_SHIFT_STAT(_x)          (_x << 0)

#define MSG(dir, cmd, index, stat)  MSG_SHIFT_DIRECTION(dir) | \
                                    MSG_SHIFT_CMD(cmd) | \
                                    MSG_SHIFT_INDEX(index) | \
                                    MSG_SHIFT_STAT(stat)

// Direction
#define MSG_AP2CP                   (0xA)
#define MSG_CP2AP                   (0xC)

// Command ID
#define MSG_READY                   (0x0)
#define MSG_DOWNLOAD                (0x1)    // for BOOT
#define MSG_UPLOAD                  (0x2)    // for DUMP
#define MSG_SECURITY                (0x3)    //security and CRC
#define MSG_FINALIZE                (0x4)
#define MSG_DEBUG                   (0xD)    // CP return current status with error code

// Index in TOC [1]
// Zero
#define MSG_NONE_TOC                (0x0)

// Stat
// Ready command
#define MSG_BOOT                    (0xB)
#define MSG_DUMP                    (0xD)

// Down/Up load commands
#define MSG_START                   (0x0)
#define MSG_DATA                    (0xB)
#define MSG_DONE                    (0xD)

// Pass/Fail
#define MSG_PASS                    (0x0)
#define MSG_FAIL                    (0xF)

// Security
#define MSG_CRC                     (0x1)
#define MSG_SIGN                    (0x2)

#define TRUE                        1
#define FALSE                       0

#define MAX_PREFIX_LEN              64
#define MAX_SUFFIX_LEN              64
#define MAX_PATH_LEN                (256) // 512 andorid max path length 256
#define MAX_TOC_INDEX               16
#define MAX_TOC_ELEMENT_SIZE        32
#define MAX_TOC_SIZE                (MAX_TOC_INDEX * MAX_TOC_ELEMENT_SIZE)    /* 512 */
#define MAX_IMG_NAME_LEN            12

#define NV_V1                       1
#define NV_V2                       2

enum modem_state {
    STATE_INIT,
    STATE_BOOTLOADER,
    STATE_FIRMWARE,
    STATE_MONITORING,
    STATE_DUMP,
    STATE_ERR,
    STATE_RECOVERY,
};

enum boot_mode {
    BOOTMODE_NORMAL,
    BOOTMODE_DUMP,
    BOOTMODE_COLD_RESET,
    BOOTMODE_MAX,
};

enum modem_type {
    MODEM_INVALID = 0,
    MODEM_SS300,
    MODEM_SS310,
    DUMMY,
    MAX_MODEM_TYPE
};

enum modem_link_device {
    LINKDEV_UNDEFINED,
    LINKDEV_SPI,
    LINKDEV_LLI,
    LINKDEV_HSIC,
    LINKDEV_SHMEM,
    LINKDEV_PCIE,
    LINKDEV_MAX,
};

enum modem_image_type {
    IMG_TOC = 0,
    IMG_BOOT,
    IMG_MAIN,
#ifdef HAS_VSS
    IMG_VSS,
    IMG_NV = 4,
    IMG_NV_NORM = 4,
#else
    IMG_NV = 3,
    IMG_NV_NORM = 3,
#endif
    IMG_NV_PROT,
    MAX_IMAGE_TYPE
};

enum modem_boot_stage {
    BOOT_STAGE = 0,
    TOC_STAGE,
    MAIN_STAGE,
#ifdef HAS_VSS
    VSS_STAGE,
    NV_STAGE = 4,
    NV_NORM_STAGE = 4,
#else
    NV_STAGE = 3,
    NV_NORM_STAGE = 3,
#endif
    NV_PROT_STAGE,
    FIN_STAGE = 0xFF,
    MAX_DL_STAGE
};

enum security_option {
    CRC_CHECK = 1,
    SECURITY_CHECK
};

enum data_available {
    NO = 0,
    YES = 1,
    MAX
};

struct toc_structure {
    char name[MAX_IMG_NAME_LEN];    /* Binary name */
    u32 b_offset;                   /* Binary offset in the file */
    u32 m_offset;                   /* Memory Offset to be loaded */
    u32 size;                       /* Binary size */
    u32 crc;                        /* CRC value */
    u32 toc_count;                  /* TOC count */
} __packed;

struct exynos_header {
    u16 cmd;
    u16 length;
} __packed;

struct exynos_data_info {
    u32 total_size;
    u32 data_offset;
} __packed;

struct exynos_payload {
    u16 cmd;
    u16 length;
    struct exynos_data_info dataInfo;
    u8 pdata[EXYNOS_PAYLOAD_LEN];
} __packed;

#define CRASH_REASON_LEN (512)

struct exynos_send_header {
    u16 cmd;
    u16 length;
    struct exynos_data_info sendInfo;
} __packed;

struct exynos_recv_header {
    u16 cmd;
    u16 length;
    struct exynos_data_info recvInfo;
} __packed;

struct exynos_dump_info {
    u32 dump_size;
    u32 reason_len;
    u8 reason[CRASH_REASON_LEN];
} __packed;

struct download_stage_control {
    u32 stage;
    u32 data_available;

    u16 start_req;
    u16 finish_req;
    u32 security_check;    //CRC_CHECK , SECURITY_CHECK

    u32 fd;
    u32 offset;
    u32 m_offset;
    u32 size;
    u32 crc;
};

struct modem_node {
    char *name;
    char *rat;
    char *node_boot;
    char *node_main;
    char *node_status;
    char *node_dump;
    char *path_bin;
    char *path_nv;
    char *path_nv_norm;
    char *path_nv_prot;
    char *path_spi;
};

struct modem_fd {
    int fd_boot;
    int fd_status;
    int fd_dump;
    int fd_dev;
    int fd_bin;
    int fd_nv;
    int fd_nv_norm;
    int fd_nv_prot;
    int fd_spi;
    int fd_info;
};

struct modem_info {
    enum modem_type type;
    enum modem_link_device link_boot;
    enum modem_link_device link_main;
};

struct public_ops {
    int (*send_bootloader) (void *args);
    int (*full_dump_from_kernel) (void *args, char *from);
};

struct modem_args {
    int nv_ver;
    u32 num_stages;

    enum modem_state state;
    enum boot_mode bmode;
    struct modem_info modem;
    struct modem_node node;
    struct modem_fd fds;

    struct public_ops ops;
    struct toc_structure toc[MAX_TOC_INDEX];
    struct exynos_payload exynos;
    struct download_stage_control ctrl[MAX_TOC_INDEX];
};

struct modem_data {
    u8 *data;
    u32 size;
} __packed;

struct data_info {
    u32 stage;
    u32 m_offset;
    u32 buff[2]; /* to be used both 32bit and 64bit machine */
    u32 total_size;
    u32 offset;
    u32 len;
} __packed;

struct sec_info {
    int bmode;
    u32 boot_size;
    u32 main_size;
};

struct crc_info {
    u32 cmd;
    u32 crc;
} __packed;

#endif //__CBD_exynos_H__
