/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __CBD_DUMP_H__
#define __CBD_DUMP_H__

#define MEM_CRASH_REASON_CP	0
#define MEM_CRASH_REASON_AP	1
#define MEM_CRASH_REASON_RIL	2
#define MEM_CRASH_REASON_SIZE	512

#define LOG_ROOT_DIR	"/data"
#define LOG_DIR		"/data/vendor/log/cbd"

struct crash_reason {
	u32 owner;
	char string[MEM_CRASH_REASON_SIZE];
} __packed;

#define CRASH_MODE_SYS_PROP	"persist.vendor.ril.crash_handling_mode"

enum crash_handling_mode {
	CRASH_MODE_DUMP_PANIC = 0,		/* kernel panic after dump */
	CRASH_MODE_DUMP_SILENT_RESET = 1,	/* silent reset after dump */
	CRASH_MODE_SILENT_RESET = 2,		/* only silent reset */
	CRASH_MODE_SKIP_RESET = 3,		/* skip reset */
	CRASH_MODE_MAX,
	CRASH_MODE_DEFAULT = CRASH_MODE_DUMP_PANIC,
};

/* Does modem ctl structure will use state ? or status defined below ?*/
enum modem_monitoring_state {
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

extern int modem_monitoring(struct modem_args *in_args);
extern int dump_process(struct modem_args *in_args, int crash_handling);
extern int pre_dump_process(struct modem_args *args);
extern int get_crash_handling_mode(void);
extern int save_kernel_log(void);

#endif // #ifndef __CBD_DUMP_H__
