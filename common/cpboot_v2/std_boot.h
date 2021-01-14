#ifndef __CBD_STD_BOOT_H__
#define __CBD_STD_BOOT_H__

#include "boot.h"

/*
** Standard BOOT/DUMP protocol based on SIPC5
*/

#define STD_CRASH_REASON_SIZE	512

#define STD_WDT_RESET_STR	"CP WDOG Reset"

/* Maximum Segment Size for payload */
#define STD_UDL_MSS		2048
/* cmd (4) + num_frames (4) + curr_frame (4) + len (4) */
#define STD_UDL_HDR_LEN		16
/* Maximum Transmission Unit = Header + Payload */
#define STD_UDL_MTU		(STD_UDL_HDR_LEN + STD_UDL_MSS)

#define STD_UDL_AP2CP		0x9000
#define STD_UDL_CP2AP		0xA000

#define STD_UDL_STAGE_SHIFT	8
#define STD_UDL_DUMP_STAGE	0xD
#define STD_UDL_FIN_STAGE	0xF
#define MAX_DLOAD_STAGE		STD_UDL_DUMP_STAGE

#define STD_UDL_STAGE_START	0x0
#define STD_UDL_SEND		0x1
#define STD_UDL_CRC		0xC
#define STD_UDL_STAGE_DONE	0xD
#define STD_UDL_STAGE_FAIL	0xF

#define EXYNOS_PAYLOAD_LEN	(62*1024)

#ifdef CONFIG_PROTOCOL_SIT
/* Direction */
#define MSG_AP2CP                   (0xA)
#define MSG_CP2AP                   (0xC)

/* Command ID */
#define MSG_READY                   (0x0)
#define MSG_DOWNLOAD                (0x1)    // for BOOT
#define MSG_UPLOAD                  (0x2)    // for DUMP
#define MSG_SECURITY                (0x3)    //security and CRC
#define MSG_FINALIZE                (0x4)
#define MSG_DEBUG                   (0xD)    // CP return current status with error code

#define MSG_NONE_TOC                (0x0)

#define MSG_BOOT                    (0xB)
#define MSG_DUMP                    (0xD)

/* Down/Up load commands */
#define MSG_START                   (0x0)
#define MSG_DATA                    (0xB)
#define MSG_DONE                    (0xD)

/* Pass/Fail */
#define MSG_PASS                    (0x0)
#define MSG_FAIL                    (0xF)

/* Security */
#define MSG_CRC                     (0x1)
#define MSG_SIGN                    (0x2)
#endif /* CONFIG_PROTOCOL_SIT */

#ifdef CONFIG_PROTOCOL_SIT
#define MSG_SHIFT_DIRECTION(_x)     (_x << 12)
#define MSG_SHIFT_CMD(_x)           (_x << 8)
#define MSG_SHIFT_INDEX(_x)         (_x << 4)
#define MSG_SHIFT_STAT(_x)          (_x << 0)

#define MSG(dir, cmd, index, stat)  ((u16)(MSG_SHIFT_DIRECTION(dir) | \
					MSG_SHIFT_CMD(cmd) | \
					MSG_SHIFT_INDEX(index) | \
					MSG_SHIFT_STAT(stat)))
#endif

struct std_toc_element {
	char name[MAX_IMG_NAME_LEN];	/* Binary name			*/
	u32 b_offset;			/* Binary offset in the file	*/
	u32 m_offset;			/* Memory Offset to be loaded	*/
	u32 size;			/* Binary size			*/
	u32 crc;			/* CRC value			*/
	u32 toc_count;			/* Reserved			*/
} __packed;

struct std_udl_crc_frame {
	u32 cmd;
	u32 crc;
} __packed;

struct std_udl_frame {
	u32 cmd;
	u32 num_frames;
	u32 curr_frame;
	u32 len;
	u8 data[STD_UDL_MSS];
} __packed;

//for sit (dump sequence) --------------------------------
struct sit_header {
	u16 cmd;
	u16 length;
} __packed;

struct sit_send_header {
	u16 cmd;
	u16 length;
	u32 total_size;
	u32 data_offset;
} __packed;

struct sit_recv_header {
	u16 cmd;
	u16 length;
	u32 total_size;
	u32 data_offset;
} __packed;

struct sit_dump_info {
	u32 dump_size;
	u32 reason_len;
	u8 reason[512];
} __packed;

struct sit_crash_reason {
	u32 owner;
	char string[512];
} __packed;
//--------------------------------------------------------
struct exynos_data_info {
    u32 total_size;
    u32 data_offset;
} __packed;

struct exynos_payload {
    u16 cmd;
    u16 length;
    struct exynos_data_info dataInfo;
    u8 pdata[STD_UDL_MSS];
} __packed;

struct std_dload_info {
	u32 size;
	u32 mtu;
	u32 num_frames;
} __packed;

struct std_uload_info {
	u32 dump_size;
	u32 num_steps;
	u32 reason_len;
} __packed;

struct std_dload_control {
	/* Stage information */
	u32 stage;
	/* Handshaking control */
	int start;
	int download;
	int validate;
	int finish;
	/* Binary information */
	int b_fd;
	u32 b_offset;
	u32 m_offset;
	u32 b_size;
	u32 crc;
	/* Download once */
	int dl_once;
};

struct std_boot_args {
	struct boot_args *cbd_args;
	int dev_fd;
	u32 num_stages;
	u32 start_stages;
	struct std_toc_element toc[MAX_TOC_INDEX];
	struct std_dload_control dl_ctrl[MAX_DLOAD_STAGE];
	struct std_udl_frame frame_buff;
};

struct std_dump_args {
	struct boot_args *cbd_args;
	int dev_fd;
	int log_fd;
	int info_fd;
	int dump_fd;
	struct std_uload_info info;
	char reason[STD_CRASH_REASON_SIZE];
	struct std_udl_frame frame_buff;
};

#ifdef CONFIG_SEC_CP_SECURE_BOOT
#define CP_FIRM_MAGIC_OFFSET		0x0FFFC
#define CP_FIRM_DL_OFFSET		0x10000

struct shdmem_info {
	unsigned int base;
	unsigned int size;
};
#endif

struct std_boot_args *std_boot_prepare_args(struct boot_args *cbd_args, u32 num_stages);
void std_boot_close_args(struct std_boot_args *args);

int std_boot_power_on(struct std_boot_args *args);
int std_boot_power_off(struct std_boot_args *args);
int std_boot_power_reset(struct std_boot_args *args, enum cp_boot_mode mode);
int std_boot_load_cp_bootloader(struct std_boot_args *args, u32 stage);
int std_boot_start_cp_bootloader(struct std_boot_args *args, enum cp_boot_mode mode);
int std_boot_load_cp_images(struct std_boot_args *args);
int std_boot_finish_handshake_sit(struct std_boot_args *args);
int std_boot_finish_handshake(struct std_boot_args *args);
int std_boot_complete_normal_bootup(struct std_boot_args *args);
int std_security_req(struct std_boot_args *args, u32 mode, u32 p1, u32 p2);

struct std_dump_args *std_dump_prepare_args(struct boot_args *cbd_args);
int std_ul_rx_frame(struct std_dump_args *args, void *buff, u32 size);
void std_dump_close_args(struct std_dump_args *args);

int std_dump_receive_cp_dump_sit(struct std_dump_args *args);
int std_dump_receive_cp_dump(struct std_dump_args *args);
int std_dump_trigger_kernel_panic(struct std_dump_args *args);

#endif
