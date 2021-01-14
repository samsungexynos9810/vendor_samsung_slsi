#ifndef __CBD_BOOT_H__
#define __CBD_BOOT_H__

#define DEBUG_RADIO_MSG
#define DEBUG_KERNEL_MSG

#ifndef s8
typedef char		s8;
#endif

#ifndef s16
typedef short		s16;
#endif

#ifndef s32
typedef int		s32;
#endif

#ifndef u8
typedef unsigned char	u8;
#endif

#ifndef u16
typedef unsigned short	u16;
#endif

#ifndef u32
typedef unsigned int	u32;
#endif

/*============================================================================*\
	Definitions for printing CP BOOT/DUMP log messages
\*============================================================================*/
#define ERR2STR		strerror(errno)

#define CBD_ID		LOG_ID_RADIO
#define CBD_TAG		"boot"

#define PRI_ERR		ANDROID_LOG_ERROR
#define PRI_DBG		ANDROID_LOG_DEBUG

#define FMT_ERR		"cbd: (ERR! %s) %s: "
#define FMT_INFO	"cbd: %s: "
#define FMT_KERN	"<3>mif: cbd: %s: "

void print_data(char *data, int len);
int get_kmsg_fd(void);

#ifdef DEBUG_RADIO_MSG
#include <log/log.h>
#define cbd_err(s, args...) \
	__android_log_buf_print(CBD_ID, PRI_ERR, CBD_TAG, FMT_ERR s, ERR2STR, __func__, ##args)
#define cbd_info(s, args...) \
	__android_log_buf_print(CBD_ID, PRI_DBG, CBD_TAG, FMT_INFO s, __func__, ##args)
#define cbd_kernel(s, args...)	dprintf(get_kmsg_fd(), FMT_KERN s, __func__, ##args)
#else
#define cbd_err(s, args...)	printf(FMT_ERR s, ERR2STR, __func__, ##args)
#define cbd_info(s, args...)	printf(FMT_INFO s, __func__, ##args)
#define cbd_kernel(s, args...)	printf(FMT_KERN s, __func__, ##args)
#endif

#define cbd_log(s, args...) \
	do { \
		if (errno == 0) { \
			cbd_info(s, ##args); \
		} else { \
			cbd_err(s, ##args); \
			errno = 0; \
		} \
		cbd_kernel(s, ##args); \
	} while (0)

/*
 * IOCTL commands
 */
#define IOCTL_MAGIC	'o'

#define IOCTL_POWER_ON			_IO(IOCTL_MAGIC, 0x19)
#define IOCTL_POWER_OFF			_IO(IOCTL_MAGIC, 0x20)

enum cp_boot_mode {
	CP_BOOT_MODE_NORMAL,
	CP_BOOT_MODE_DUMP,
	CP_BOOT_RE_INIT,
	CP_BOOT_REQ_CP_RAM_LOGGING = 5,
	CP_BOOT_MODE_MANUAL = 7,
	MAX_CP_BOOT_MODE
};
struct boot_mode {
	enum cp_boot_mode idx;
} __packed;
#define IOCTL_POWER_RESET		_IOW(IOCTL_MAGIC, 0x21, struct boot_mode)
#define IOCTL_START_CP_BOOTLOADER	_IOW(IOCTL_MAGIC, 0x22, struct boot_mode)
#define IOCTL_COMPLETE_NORMAL_BOOTUP	_IO(IOCTL_MAGIC, 0x23)
#define IOCTL_GET_CP_STATUS		_IO(IOCTL_MAGIC, 0x27)
#define IOCTL_TRIGGER_CP_CRASH		_IO(IOCTL_MAGIC, 0x34)
#define IOCTL_TRIGGER_KERNEL_PANIC	_IO(IOCTL_MAGIC, 0x35)

struct cp_image {
	u8 *binary;
	u32 size;
	u32 m_offset;
	u32 b_offset;
	u32 mode;
	u32 len;
} __packed;
#define IOCTL_LOAD_CP_IMAGE		_IOW(IOCTL_MAGIC, 0x40, struct cp_image)

#define IOCTL_GET_SRINFO		_IO(IOCTL_MAGIC, 0x45)
#define IOCTL_SET_SRINFO		_IO(IOCTL_MAGIC, 0x46)
#define IOCTL_GET_CP_BOOTLOG		_IO(IOCTL_MAGIC, 0x47)
#define IOCTL_CLR_CP_BOOTLOG		_IO(IOCTL_MAGIC, 0x48)

/* Log dump */
#define IOCTL_MIF_LOG_DUMP		_IO(IOCTL_MAGIC, 0x51)

enum cp_log_dump_index {
	LOG_IDX_SHMEM,
	LOG_IDX_VSS,
	LOG_IDX_ACPM,
	LOG_IDX_CP_BTL,
	LOG_IDX_DATABUF,
	MAX_LOG_DUMP_IDX
};
struct cp_log_dump {
	char name[32];
	enum cp_log_dump_index idx;
	u32 size;
} __packed;
#define IOCTL_GET_LOG_DUMP		_IOWR(IOCTL_MAGIC, 0x52, struct cp_log_dump)

struct modem_sec_req {
	u32 mode;
	u32 param2;
	u32 param3;
	u32 param4;
} __packed;
#define IOCTL_REQ_SECURITY		_IOW('o', 0x53, struct modem_sec_req)

/* Crash reason */
#define CRASH_REASON_SIZE	512
enum crash_type {
	CRASH_REASON_CP_ACT_CRASH = 0,
	CRASH_REASON_RIL_MNR,
	CRASH_REASON_RIL_REQ_FULL,
	CRASH_REASON_RIL_PHONE_DIE,
	CRASH_REASON_RIL_RSV_MAX,
	CRASH_REASON_USER = 5,
	CRASH_REASON_MIF_TX_ERR = 6,
	CRASH_REASON_MIF_RIL_BAD_CH,
	CRASH_REASON_MIF_RX_BAD_DATA,
	CRASH_REASON_RIL_TRIGGER_CP_CRASH,
	CRASH_REASON_MIF_FORCED,
	CRASH_REASON_CP_WDOG_CRASH,
	CRASH_REASON_MIF_RSV_MAX = 12,
	CRASH_REASON_CP_SRST,
	CRASH_REASON_CP_RSV_0,
	CRASH_REASON_CP_RSV_MAX = 15,
	CRASH_REASON_NONE = 0xFFFF,
};
struct crash_reason {
	u32 owner;
	char string[CRASH_REASON_SIZE];
} __packed;
#define IOCTL_GET_CP_CRASH_REASON	_IOR('o', 0x55, struct crash_reason)

#define CPIF_VERSION_SIZE	20
struct cpif_version {
	char string[CPIF_VERSION_SIZE];
} __packed;
#define IOCTL_GET_CPIF_VERSION		_IOR('o', 0x56, struct cpif_version)

#define CPDUMP_ROOT		"/data"
#define CPDUMP_PATH		"/sdcard/log"
#define FACTORY_CPDUMP_PATH	"/data/vendor/log/cbd"

#define SWITCH_PATH		"/sys/class/sec/switch/attached_dev"

/* property for vendor */
#define VPROP_CPBOOT            "vendor.cbd.cpboot"
#define VPROP_CPBOOT_DONE       "vendor.cbd.boot_done"
#define VPROP_CPRESET_DONE      "vendor.cbd.reset_done"
#define VPROP_FIRST_XMIT_DONE   "vendor.cbd.first_xmit_done"
#define VPROP_DT_REVISION       "vendor.cbd.dt_revision"
#define VPROP_DEBUG_CP_OPT      "persist.vendor.cbd.debug_cp_opt"

#ifdef CONFIG_DUMP_LIMIT
#define VPROP_CDUMP_INDEX	"persist.vendor.cbd.crash_dump_index"
#define VPROP_CDUMP_LIMIT	"persist.vendor.cbd.crash_dump_limit"
#endif

#ifdef CONFIG_PROTOCOL_SIT
#define VPROP_RFS_CHECKDONE     "vendor.ril.cbd.rfs_check_done"
#define VPROP_RILD_RESET	"vendor.sys.rild_reset"
#define VPROP_CRASH_MODE	"persist.vendor.ril.crash_handling_mode"
#else
#define VPROP_RFS_CHECKDONE     "vendor.cbd.rfs_check_done"
#define VPROP_DEV_OFFRES        "vendor.cbd.deviceOffRes"
#endif

/* property for system */
#ifdef CONFIG_PROTOCOL_SIT
#define PROP_RADIO_MULTISIM_CONFIG	"persist.radio.multisim.config"
#else
#define PROP_SERIAL_NO          "ro.serialno"
#define PROP_SALES_CODE         "ro.csc.sales_code"
#define PROP_DEV_OFFREQ         "sys.shutdown.requested"
#define PROP_SYS_POWERCTL       "sys.powerctl"
#endif

#define STAGE_VSS		2

#define MAX_CMD_LINE_LEN	1024

#define MAX_NAME_LEN		32
#define MAX_PREFIX_LEN		64
#define MAX_SUFFIX_LEN		64
#define MAX_PATH_LEN		512

#define MAX_TOC_INDEX		16
#define MAX_TOC_ELEMENT_SIZE	32
#define MAX_TOC_SIZE		(MAX_TOC_INDEX * MAX_TOC_ELEMENT_SIZE)	/* 512 */
#define MAX_IMG_NAME_LEN	12

#define MAX_ERROR_INFO_BUF_SIZE 512

#ifdef CONFIG_PROTOCOL_SIT
enum crash_handling_mode {
	CRASH_MODE_DUMP_PANIC = 0,		/* kernel panic after dump */
	CRASH_MODE_DUMP_SILENT_RESET = 1,	/* silent reset after dump */
	CRASH_MODE_SILENT_RESET = 2,		/* only silent reset */
	MAX_CRASH_MODE,
};
#endif

enum modem_state {
	STATE_OFFLINE,
	STATE_CRASH_RESET,	/* silent reset */
	STATE_CRASH_EXIT,	/* cp ramdump */
	STATE_BOOTING,
	STATE_ONLINE,
	STATE_NV_REBUILDING,	/* NV rebuilding start */
	STATE_LOADER_DONE,
	STATE_SIM_ATTACH,
	STATE_SIM_DETACH,
	STATE_CRASH_WATCHDOG,	/* cp watchdog crash */
};

enum modem_t {
	MODEM_INVALID = 0,
	IMC_XMM626X,
	IMC_XMM7160,
	SEC_CMC22X,
	VIA_CBP72,
	SEC_SS222,
	QC_ESC6270,
	SEC_SHANNON_HSIC,
	SEC_SS300,
	SEC_SS333,
	IMC_XMM72XX,
	IMC_XMM72XX_LLI,
	SEC_SS310,
	SEC_MODAP_AP,
	SEC_S5100,
	SEC_MODAP_SIT,
	SEC_S5100_SIT,
	DUMMY,
	MAX_MODEM_TYPE
};

enum modem_link {
	LINKDEV_UNDEFINED,
	LINKDEV_MIPI,
	LINKDEV_DPRAM,
	LINKDEV_SPI,
	LINKDEV_USB,
	LINKDEV_HSIC,
	LINKDEV_C2C,
	LINKDEV_UART,
	LINKDEV_PLD,
	LINKDEV_SHMEM,
	LINKDEV_LLI,
	LINKDEV_PCIE,
	LINKDEV_MAX,
};

enum sec_debug_level {
	DBG_LOW,
	DBG_MID,
	DBG_HIGH,
	DBG_AUTO,
};

enum sec_cp_debug {
	DBG_CP_NORMAL,
	DBG_CP_NOCRASH,		/* hang cbd on cp crash situation */
	DBG_CP_NORESET,		/* on 2nd boot trying, skip CP boot */
	DBG_CP_NOBOOT,		/* skip CP boot */
	DBG_CP_AUTORESET,	/* reset CP repeatedly */
	DBG_CP_FORCEPANIC,	/* force a kernel panic on cp crash*/
};

enum TYPE_LOG {
	LOG_DMESG,
	LOG_DUMPSTATE,
	LOG_BOOT_FAIL,
};

#define LOGB_DMESG		(0x1 << LOG_DMESG)
#define LOGB_DUMPSTATE		(0x1 << LOG_DUMPSTATE)
#define LOGB_BOOTFAIL		(0x1 << LOG_BOOT_FAIL)

struct boot_args {
	enum modem_t type;
	enum modem_link lnk_boot;
	enum modem_link lnk_main;
	unsigned daemon;
	struct modem_comp *cpn; /*component*/
	unsigned options;	/*wildcard?*/
	void *modem_data;	/*extentions*/

	unsigned flb_mode; /* FLB MODE 1 for ITP and 0 for NORMAL MODE */
	char *printf_level; /* For IBP, holds the value to reply when modem asks for debug level */

	int debug_level;
	char reason[CRASH_REASON_SIZE];
};

struct modem_comp {
	enum modem_t type;
	char *name;
	char *rat;

	int (*start_boot)(struct boot_args *args);
	int (*start_dump)(struct boot_args *args);
	int (*shutdown)(struct boot_args *args);
	int (*upload_modem)(struct boot_args *args);

	char *node_boot;
	char *node_main;
	char *node_status;
	char *node_dump;
	char *path_bin;
	char *dnt_bin;

	char *path_nv;
	char *path_nv_prot;
	u32 nv_size;

	char *prop_boot_done;
	char *prop_reset_done;

	int num_stages;
	int toc_stage;
	int boot_stage;
	int main_stage;
};

/*============================================================================*\
	Definitions for file system (directory & file) management
\*============================================================================*/
/* void set_log_root(char *path);*/
char *get_log_root(void);
int create_log_directory(char *path);
/* void set_log_dir(char *path); */
char *get_log_dir(void);

/*============================================================================*\
	Definitions for saving debug log to a file
\*============================================================================*/
struct save_logs_arg {
	int type;
	char *prefix;
};
void save_logs(int type, char *prefix);

const char *get_cbd_version(void);

/* boot option*/
#define BOPT_ROOT		0x100
#define BOPT_EHCI_TEGRA		0x2000
#define BOPT_CPUPLOAD		0x10000

#define CP_MEMORY_MASK	(0x40000000 - 1)

int start_shannon310_boot(struct boot_args *args);
int start_shannon310_dump(struct boot_args *args);
int shutdown_shannon310_modem(struct boot_args *args);
int start_shannon310_dummy_dump(struct boot_args *args);
int upload_shannon310_modem(struct boot_args *args);

int start_shannon5100_boot(struct boot_args *args);
int start_shannon5100_dump(struct boot_args *args);
int shutdown_shannon5100_modem(struct boot_args *args);
int upload_shannon5100_modem(struct boot_args *args);

#endif
