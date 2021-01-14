/*
 * Copyright (C) 2018 Samsung Electronics Co. Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "WLBTD"
//#define LOG_NDEBUG 0

#include <linux/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <log/log.h>

#include <sys/types.h>      /* needed to use pid_t, etc. */
#include <sys/wait.h>       /* needed to use waitpid() */
#include <sys/system_properties.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>

/* WLBTD version
 * =============
 * v3.2 - set vendor.wlbtd.board_id property
 * v3.1 - use response_codes to convey status to kernel driver
 * v3.0 - parse .memdump.info
 * v2.0 - SABLE - EVENT_SABLE
 * v1.0 - pre-SABLE EVENT_SCSC, EVENT_SYSTEM_PROPERTY, EVENT_WRITE_FILE
 * */
#define WLBTD_VERSION "3.2"
#define TAR_FILES_PER_TRIGGER "5"

#define MAP_SIZE 16 * 1024 * 1024
#define LOG_COLLECT_DEV_ENTRY "/dev/scsc_log_collector_0"
#define SABLETARPATH "/data/vendor/log/wifi"
#define MEMDUMP_FILE_NO_MOREDUMP	0
#define MEMDUMP_FILE_RECOVERY_ON	2
#define MEMDUMP_FILE_PANIC_KERNEL	3
#define MEMDUMP_FILE_OTHER		4

static struct nl_sock *sk = NULL;
static int group;
static int family;

pthread_mutex_t tar_mutex = PTHREAD_MUTEX_INITIALIZER;
static int can_create_sbl_tar = 1;
static int memdump_file_contents = -1;
/**
 * Attributes and commands have to be the same as in kernelspace, so you might
 * want to move these enums to a .h and just #include that from both files.
 */
enum attributes {
	/* must be first */
	ATTR_UNSPEC,

	ATTR_STR,
	ATTR_INT,
	ATTR_PATH,
	ATTR_CONTENT,
	ATTR_INT8,

	/* This must be last! */
	__ATTR_MAX,
};

enum events {
	/* must be first */
	EVENT_UNSPEC,

	EVENT_SCSC,
	EVENT_SYSTEM_PROPERTY,
	EVENT_WRITE_FILE,
	EVENT_SABLE,

	/* This must be last! */
	__EVENT_MAX,
};

enum scsc_log_reason {
	SCSC_LOG_UNKNOWN = 0,
	SCSC_LOG_FW_PANIC,
	SCSC_LOG_USER,
	SCSC_LOG_FW,
	SCSC_LOG_DUMPSTATE,
	SCSC_LOG_HOST_WLAN,
	SCSC_LOG_HOST_BT,
	SCSC_LOG_HOST_COMMON,
	/* Add others */
};

enum scsc_wlbtd_response_codes {
	/* NOTE: keep the enum in sync with driver */
	/* parse failed */
	SCSC_WLBTD_ERR_PARSE_FAILED,

	/* fw_panic trigger */
	SCSC_WLBTD_FW_PANIC_TAR_GENERATED,
	SCSC_WLBTD_FW_PANIC_ERR_SCRIPT_FILE_NOT_FOUND,
	SCSC_WLBTD_FW_PANIC_ERR_NO_DEV,
	SCSC_WLBTD_FW_PANIC_ERR_MMAP,
	SCSC_WLBTD_FW_PANIC_ERR_SABLE_FILE,
	SCSC_WLBTD_FW_PANIC_ERR_TAR,

	/* other triggers */
	SCSC_WLBTD_OTHER_SBL_GENERATED,
	SCSC_WLBTD_OTHER_TAR_GENERATED,
	SCSC_WLBTD_OTHER_ERR_SCRIPT_FILE_NOT_FOUND,
	SCSC_WLBTD_OTHER_ERR_NO_DEV,
	SCSC_WLBTD_OTHER_ERR_MMAP,
	SCSC_WLBTD_OTHER_ERR_SABLE_FILE,
	SCSC_WLBTD_OTHER_ERR_TAR,
	SCSC_WLBTD_OTHER_IGNORE_TRIGGER,
};

static const char *get_trigger_str(int trigger_int) {

	switch (trigger_int) {
	case 1:	return "scsc_log_fw_panic";
	case 2:	return "scsc_log_user";
	case 3:	return "scsc_log_fw";
	case 4:	return "scsc_log_dumpstate";
	case 5:	return "scsc_log_host_wlan";
	case 6:	return "scsc_log_host_bt";
	case 7:	return "scsc_log_host_common";
	case 8:	return "scsc_log_sys_error";
	default:
	case 0:
		return "scsc_log_unknown";
	}
}

static int file_exists(const char* filename) {
	struct stat buffer;
	int exist = stat(filename, &buffer);
	if (exist == 0)
		return 1;
	else {
		return 0;
	}
}

static int process_cmd_system_property_get(struct nl_msg *msg)
{
	/* The total bytes copied will be no greater than PROP_VALUE_MAX */
	char value[PROP_VALUE_MAX];
	int error, length;
	struct nl_msg *ack_msg;
	struct nlmsghdr *nl_hdr;
	struct genlmsghdr *genl_hdr;
	struct nlattr *attrs[__ATTR_MAX];

	nl_hdr = nlmsg_hdr(msg);
	genl_hdr = genlmsg_hdr(nl_hdr);
	error = genlmsg_parse(nl_hdr, 0, attrs, __ATTR_MAX - 1, NULL);
	if (error) {
		ALOGE("genlmsg_parse failed : %s (%d)", nl_geterror(error), error);
		return error;
	}

	/* Remember: attrs[0] is a throwaway. */
	if (attrs[1] == 0) {
		ALOGE("ATTR_STR: null\n");
		return -1;
	}

	ALOGI("ATTR_STR: len:%u type:%u data:%s\n", attrs[1]->nla_len, attrs[1]->nla_type, (char *)nla_data(attrs[1]));

	length = __system_property_get((char *)nla_data(attrs[1]), value);
	if (length <= 0) {
		ALOGE("__system_property_get failed: length=%d", length);
		return -1;
	}

	ALOGI("__system_property_get: length=%d value=%s", length, value);

	ALOGI("Sending EVENT_SYSTEM_PROPERTY cmd reply.");
	ack_msg = nlmsg_alloc();
	if (ack_msg) {
		genlmsg_put(ack_msg,
			NL_AUTO_PORT,
			NL_AUTO_SEQ,
			family, 0, 0,
			genl_hdr->cmd/*EVENT_SYSTEM_PROPERTY*/,
			0);
		nla_put_string(ack_msg, ATTR_STR, value);
		error = nl_send_auto(sk, ack_msg);
		if (error < 0)
			ALOGE("nl_send_auto failed !");
		nlmsg_free(ack_msg);
		ALOGI("sent (%d byte) reply msg to kernel.", error);
	} else {
		ALOGE("nlmsg_alloc failed");
	}
	return 0;
}

static void send_response_to_kernel(enum events e, const char *msg_str, int status)
{
	int error = -1;
	struct nl_msg *ack_msg = nlmsg_alloc();
	if (ack_msg) {
		genlmsg_put(ack_msg, NL_AUTO_PORT, NL_AUTO_SEQ, family, 0, 0, e, 0);
		nla_put_string(ack_msg, ATTR_STR, msg_str);
		if (e == EVENT_SABLE)
			nla_put_u16(ack_msg, ATTR_INT, status);
		else
			nla_put_u32(ack_msg, ATTR_INT, status);
		error = nl_send_auto(sk, ack_msg);
		if (error < 0)
			ALOGE("nl_send_auto failed !");
		nlmsg_free(ack_msg);
		ALOGI("sent (%d byte) ack msg to kernel.", error);
	} else {
		/* not sending ack back to kernel so there will be completion timeout */
		ALOGE("nlmsg_alloc failed");
	}

}

static void parse_memdump_file_wlbtd(void) {
	FILE *fp = nullptr;
	int c = 0;
	char android_version[PROP_VALUE_MAX];
	int len, disable_recovery = 1;
	char memdump_file[64];

	len = __system_property_get("ro.build.version.release", android_version);

	if (atoi(android_version) >= 9)
		snprintf(memdump_file, sizeof(memdump_file), "%s","/sys/wifi/memdump");
	else
		snprintf(memdump_file, sizeof(memdump_file), "%s","/data/misc/conn/.memdump.info");

	if(file_exists(memdump_file)) {
		fp = fopen(memdump_file, "r");
		if (fp != NULL) {
			c = fgetc(fp);
			switch (c) {
			case '3' :
				memdump_file_contents = MEMDUMP_FILE_PANIC_KERNEL;
				disable_recovery = 1;
				break;
			case '0':
				memdump_file_contents = MEMDUMP_FILE_NO_MOREDUMP;
				disable_recovery = 0;
				break;
			case '2':
				memdump_file_contents = MEMDUMP_FILE_RECOVERY_ON;
				disable_recovery = 0;
				break;
			default:
				memdump_file_contents = c - '0';
				disable_recovery = 1;
				break;
			}

			ALOGI("memdump_file_contents %d Auto-recovery: %s", memdump_file_contents, disable_recovery ? "off" : "on");
			fclose(fp);
		} else
			ALOGE("failed to open %s", memdump_file);
	} else
		ALOGE("%s not found", memdump_file);
}

static void check_setup()
{
	__system_property_set("vendor.wlbtd.version", WLBTD_VERSION);
	__system_property_set("vendor.wlbtd.tar_files_per_trigger", TAR_FILES_PER_TRIGGER);

	parse_memdump_file_wlbtd();
}

static int wlbtd_popen(const char *command)
{
	pid_t pid, w;
	int error = -1;
	int status = -1;
	struct timeval t0, t1;
	double diff;

	/* fork and run script in child process otherwise wlbtd service gets killed */
	ALOGI("%s start", __func__);
	gettimeofday(&t0, 0);
	pid = fork();
	if (pid < 0) {
		ALOGE("fork() failed.");
		return error;
	} else if (pid == 0) {
		/* child */
		error = execl("/vendor/bin/sh", "/vendor/bin/sh", "-c", command, (char *)0);
		/* execl only returns -1 on error, so following code should not execute
		 * on success
		 */
		if (error == -1 ) {
			ALOGE("%s execl failed error %d", __func__, error);
			return error;
		}
	} else {
		do {
			/* parent
			 * wait for child to terminate and send ack to kernel that script is done.
			 *
			 * When WIFEXITED() is nonzero, WEXITSTATUS() evaluates to the low-order 8 bits
			 * of the status argument that the child passed to the exit() or _exit() function,
			 * or the value the child process returned from main().
			 */
			w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
			if (w == -1) {
				ALOGE("%s waitpid failed", __func__);
				return error;
			}
		}while (!WIFEXITED(status));
	}
	gettimeofday(&t1, 0);
	diff = (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
	ALOGI("%s end. done in %dms", __func__, (int)diff);
	return 0;
}

struct script_arguments
{
	uint8_t trigger_uint8;
	uint16_t error_code;
};

void *sable_tar_thread(void *args)
{
	struct script_arguments *arg = (struct script_arguments *)args;
	int status = -1;
	char err_code_str[5];
	char cmd[128];
	struct timeval t0, t1;
	double diff;
	const char *trigger_str = get_trigger_str((int)arg->trigger_uint8);

	snprintf(err_code_str, sizeof(err_code_str), "%04x", arg->error_code);

	ALOGI("%s: start:trigger - %s", __func__, trigger_str);
	snprintf(cmd, 128, "mx_log_collection.sh %s %s", trigger_str, err_code_str);
	gettimeofday(&t0, 0);

	status = wlbtd_popen(cmd);
	if (status == 0) {
		gettimeofday(&t1, 0);
		diff = (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;

		ALOGI("%s:   end:trigger - %s status %d", __func__, trigger_str, WEXITSTATUS(status));
		ALOGI("tar done in %0dms", (int)diff);
		snprintf(cmd, 128, "%s tar generated in %dms", trigger_str, (int)diff);

		parse_memdump_file_wlbtd();

		if (arg->trigger_uint8 == SCSC_LOG_FW_PANIC) {
			if (memdump_file_contents == MEMDUMP_FILE_PANIC_KERNEL)
				status = MEMDUMP_FILE_PANIC_KERNEL;
			else
				status = SCSC_WLBTD_FW_PANIC_TAR_GENERATED;
		}
		else
			status = SCSC_WLBTD_OTHER_TAR_GENERATED;

		send_response_to_kernel(EVENT_SABLE, cmd, status);
	} else {
		ALOGE("generating tar failed. status %d", WEXITSTATUS(status));
		snprintf(cmd, 128, "generating tar failed. status %d", WEXITSTATUS(status));
		send_response_to_kernel(EVENT_SABLE, cmd,
			(arg->trigger_uint8 == SCSC_LOG_FW_PANIC) ? SCSC_WLBTD_FW_PANIC_ERR_TAR : SCSC_WLBTD_OTHER_ERR_TAR);
	}

	pthread_mutex_lock(&tar_mutex);
	/* set flag so we can process next trigger */
	can_create_sbl_tar = 1;
	pthread_mutex_unlock(&tar_mutex);

	/* Free the script_args passed by creator */
	ALOGI("free script_args %p", arg);
	free(arg);

	pthread_exit(0);

	return NULL;
}

static int process_cmd_sable(struct nl_msg *msg)
{
	struct nlmsghdr *nl_hdr;
	struct nlattr *attrs[__ATTR_MAX];
	int error = -1;
	int errn;

	struct nl_msg *ack_msg;
	uint8_t trigger_uint8 = 0;
	uint16_t error_code;
	int sable_fd, dev_fd;
	char outfile_name[64];
	char errmsg[128];
	const char *trigger_str;
	struct script_arguments *script_args = NULL;

	pthread_t tar_thread;

	nl_hdr = nlmsg_hdr(msg);
	error = genlmsg_parse(nl_hdr, 0, attrs, __ATTR_MAX - 1, NULL);
	if (error) {
		ALOGE("genlmsg_parse failed : %s (%d)", nl_geterror(error), error);
		send_response_to_kernel(EVENT_SABLE, "genlmsg_parse failed", SCSC_WLBTD_ERR_PARSE_FAILED);
		return error;
	}

	error_code = nla_get_u16(attrs[2]);
	trigger_uint8 = nla_get_u8(attrs[5]);
	trigger_str = get_trigger_str((int)trigger_uint8);

	if (!file_exists("/vendor/bin/mx_log_collection.sh")){
		ALOGE("/vendor/bin/mx_log_collection.sh not found. SABLE tar will not be generated");
		send_response_to_kernel(EVENT_SABLE, "/vendor/bin/mx_log_collection.sh not found",
			(trigger_uint8 == SCSC_LOG_FW_PANIC) ? SCSC_WLBTD_FW_PANIC_ERR_SCRIPT_FILE_NOT_FOUND : SCSC_WLBTD_OTHER_ERR_SCRIPT_FILE_NOT_FOUND);
		return -1;
	}

	ALOGD("trigger_str %s trigger_uint8 %d", trigger_str, trigger_uint8);

	/* ignore triggers other than scsc_log_fw_panic while writing .sbl & generating tar */
	if ((trigger_uint8 == SCSC_LOG_FW_PANIC) || can_create_sbl_tar) {
		pthread_mutex_lock(&tar_mutex);
		can_create_sbl_tar = 0;
		pthread_mutex_unlock(&tar_mutex);

		dev_fd = open(LOG_COLLECT_DEV_ENTRY, O_RDWR);
		if (dev_fd < 0) {
			errn = errno;
			ALOGE("failed %d to open %s\n", errn, LOG_COLLECT_DEV_ENTRY);
			ack_msg = nlmsg_alloc();
			snprintf(errmsg, 128, "failed %d to open %s. No .sbl or tar generated.", errn, LOG_COLLECT_DEV_ENTRY);
			send_response_to_kernel(EVENT_SABLE, errmsg,
				(trigger_uint8 == SCSC_LOG_FW_PANIC) ? SCSC_WLBTD_FW_PANIC_ERR_NO_DEV : SCSC_WLBTD_OTHER_ERR_NO_DEV);
			close(dev_fd);
			return -1;
		}

		char *addr = (char *)mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, 0);
		if (addr == nullptr) {
			errn = errno;
			ALOGE("mmap failed %d", errn);
			snprintf(errmsg, 128, "mmap failed %d", errn);
			send_response_to_kernel(EVENT_SABLE, errmsg,
				(trigger_uint8 == SCSC_LOG_FW_PANIC) ? SCSC_WLBTD_FW_PANIC_ERR_MMAP : SCSC_WLBTD_OTHER_ERR_MMAP);
		}

		snprintf(outfile_name, 64, "%s/.tmp-%s.sbl", SABLETARPATH, trigger_str);
		sable_fd = open(outfile_name, O_WRONLY | O_CREAT, 0644);
		if (sable_fd < 0)  {
			errn = errno;
			ALOGE("failed %d to open %s\n", errn, outfile_name);
			snprintf(errmsg, 128, "failed %d to open %s", errn, outfile_name);
			send_response_to_kernel(EVENT_SABLE, errmsg,
				(trigger_uint8 == SCSC_LOG_FW_PANIC) ? SCSC_WLBTD_FW_PANIC_ERR_SABLE_FILE : SCSC_WLBTD_OTHER_ERR_SABLE_FILE);
			close(dev_fd);
			munmap((void *)addr, MAP_SIZE);
			close(sable_fd);
			return -1;
		}

		/* write sable file */
		error = write(sable_fd, addr, MAP_SIZE);
		ALOGD("%s.sbl written", trigger_str);

		close(dev_fd);
		munmap((void *)addr, MAP_SIZE);
		close(sable_fd);

		/* wait untill fw_panic moredump tar is ready before sending reply to driver */
		if (trigger_uint8 != SCSC_LOG_FW_PANIC) {
			/* send response as soon as sable is generated and tar in the background */
			snprintf(errmsg, 128, "%s generated", outfile_name);
			send_response_to_kernel(EVENT_SABLE, errmsg, SCSC_WLBTD_OTHER_SBL_GENERATED);
		}

		/* thread to create tar in parallel */
		script_args = (struct script_arguments *)malloc(sizeof(*script_args)); /* sable_tar_thread() must free */
		if (script_args == NULL) {
			snprintf(errmsg, 128, "no arg memory for %s", trigger_str);
			send_response_to_kernel(EVENT_SABLE, errmsg,
				(trigger_uint8 == SCSC_LOG_FW_PANIC) ? SCSC_WLBTD_FW_PANIC_ERR_SABLE_FILE : SCSC_WLBTD_OTHER_ERR_SABLE_FILE);
		} else {
			ALOGI("script_args %p (trigger %u, code %u)", script_args, trigger_uint8, error_code);
			script_args->trigger_uint8 = trigger_uint8;
			script_args->error_code = error_code;
			pthread_create(&tar_thread, NULL, sable_tar_thread, (void *)script_args);
		}
	} else {
		snprintf(errmsg, 128, "ignoring trigger %s", trigger_str);
		send_response_to_kernel(EVENT_SABLE, errmsg, SCSC_WLBTD_OTHER_IGNORE_TRIGGER);
	}

	return 0;
}

static int process_cmd_scsc(struct nl_msg *msg)
{
	struct nlmsghdr *nl_hdr;
	struct nlattr *attrs[__ATTR_MAX];
	int error, status = -1;
	char *script;
	char command[64];

	nl_hdr = nlmsg_hdr(msg);
	error = genlmsg_parse(nl_hdr, 0, attrs, __ATTR_MAX - 1, NULL);
	if (error) {
		ALOGE("genlmsg_parse failed : %s (%d)", nl_geterror(error), error);
		return error;
	}

	/* Remember: attrs[0] is a throwaway. */

	if (attrs[1])
		ALOGD("ATTR_STR: len:%u type:%u data:%s\n",
				attrs[1]->nla_len,
				attrs[1]->nla_type,
				(char *)nla_data(attrs[1]));
	else
		ALOGD("ATTR_STR: null\n");

	if (attrs[2])
		ALOGD("ATTR_INT: len:%u type:%u data:%u\n",
				attrs[2]->nla_len,
				attrs[2]->nla_type,
				*((__u32 *)nla_data(attrs[2])));
	else
		ALOGD("ATTR_INT: null\n");

	script = (char *)nla_data(attrs[1]);

	/* moredump script calls moredump.bin and mx_logger_dump.sh internally */
	if (!strcmp(script, "moredump")) {
		if (!file_exists("/vendor/bin/moredump")){
			ALOGE("/vendor/bin/moredump not found. Moredump will not be generated");
			if (file_exists("/vendor/bin/mx_logger_dump.sh"))
				strncpy(script, "mx_logger_dump.sh", sizeof("mx_logger_dump.sh")+1);
			else {
				ALOGE("/vendor/bin/mx_logger_dump.sh not found. Wifi disconnect logs will not be generated");
				send_response_to_kernel(EVENT_SCSC, "moredump & mx_logger_dump.sh not found", 1);
				return status;
			}
		}
		if (!file_exists("/vendor/bin/moredump.bin")){
			ALOGE("/vendor/bin/moredump.bin not found. Moredump will not be generated");
			if (file_exists("/vendor/bin/mx_logger_dump.sh"))
				strncpy(script ,"mx_logger_dump.sh", sizeof("mx_logger_dump.sh")+1);
			else {
				ALOGE("/vendor/bin/mx_logger_dump.sh not found. Wifi disconnect logs will not be generated");
				send_response_to_kernel(EVENT_SCSC, "moredump.bin & mx_logger_dump.sh not found", 2);
				return status;
			}
		}
		if (!file_exists("/vendor/bin/mx_logger_dump.sh"))
			ALOGE("/vendor/bin/mx_logger_dump.sh not found. Wifi disconnect logs will not be generated");

	}

	if (!strcmp(script, "mx_logger_dump.sh")) {
		if (!file_exists("/vendor/bin/mx_logger_dump.sh")){
			ALOGE("/vendor/bin/mx_logger_dump.sh not found. Wifi disconnect logs will not be generated");
			send_response_to_kernel(EVENT_SCSC, "mx_logger_dump.sh not found", 3);
			return status;
		}
	}

	parse_memdump_file_wlbtd();
	if (!strcmp(script, "moredump") && (memdump_file_contents == MEMDUMP_FILE_NO_MOREDUMP)) {
		send_response_to_kernel(EVENT_SCSC, " memdump_file_contents == 0, no moredump generated", MEMDUMP_FILE_NO_MOREDUMP);
		return 0;
	}

	snprintf(command, 64, "/vendor/bin/%s", script);

	ALOGI("%s: %s - start", __func__, script);

	status = wlbtd_popen(command);
	if (status == 0) {
		ALOGI("%s: %s - end", __func__, script);
		if (memdump_file_contents == MEMDUMP_FILE_PANIC_KERNEL)
			status = MEMDUMP_FILE_PANIC_KERNEL;
		send_response_to_kernel(EVENT_SCSC, strcat(script, " done ack !"), status);
	} else {
		ALOGE("%s: %s failed to run. status %d", __func__, script, status);
		send_response_to_kernel(EVENT_SCSC, strcat(script, " failed to run"), status);
	}

	return 0;
}

/*
 * This function will be called for each valid netlink message received
 * in nl_recvmsgs_default()
 */
static int rcv_msg_cb(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nl_hdr;
	struct genlmsghdr *genl_hdr;

	(void)arg;

	nl_hdr = nlmsg_hdr(msg);
	genl_hdr = genlmsg_hdr(nl_hdr);

	if (genl_hdr->cmd == EVENT_SABLE) {
		ALOGI("Received EVENT_SABLE v%s\n", WLBTD_VERSION);
		return process_cmd_sable(msg);
	} else if (genl_hdr->cmd == EVENT_SYSTEM_PROPERTY) {
		ALOGI("Received EVENT_SYSTEM_PROPERTY\n");
		return process_cmd_system_property_get(msg);
	} else if (genl_hdr->cmd == EVENT_SCSC) {
		ALOGI("Received EVENT_SCSC\n");
		return process_cmd_scsc(msg);
	} else {
		ALOGI("The message type is not supported; ignoring.\n");
		send_response_to_kernel(EVENT_SCSC, "message type not supported", 0);
		return 0;
	}

	return 0;
}


/*
 * Write the board_id from sysfs into an Android property
 */
#define WLBTD_BOARD_ID_FILE		"/sys/devices/system/chip-id/board_id"
#define WLBTD_BOARD_ID_PROPERTY		"vendor.wlbtd.board_id"

static void get_board_id()
{
	FILE *fd = fopen(WLBTD_BOARD_ID_FILE, "r");
	char value[128];
	size_t n;
	int r;

	if (!fd) {
		ALOGE("open board_id file %s failed", WLBTD_BOARD_ID_FILE);
		return;
	}

	n = fscanf(fd, "%127s", value);
	if (n > 0) {
		ALOGI("read board_id '%s' from %s", value, WLBTD_BOARD_ID_FILE);
		r = __system_property_set(WLBTD_BOARD_ID_PROPERTY, value);
		if (r)
			ALOGE("__system_property_set %s: failed %d", WLBTD_BOARD_ID_PROPERTY, r);
		else
			ALOGI("property_set: %s to %s", WLBTD_BOARD_ID_PROPERTY, value);
	}
	fclose(fd);
}

static int do_things(void)
{
	int error = -1;

	/* Socket allocation */
	sk = nl_socket_alloc();
	if (!sk) {
		ALOGE("nl_socket_alloc failed");
		return error;
	}

	nl_socket_disable_seq_check(sk);
	nl_socket_disable_auto_ack(sk);

	error = nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, rcv_msg_cb, NULL);
	if (error) {
		ALOGE("nl_socket_modify_cb failed : %s (%d)", nl_geterror(error), error);
		return error;
	}

	error = genl_connect(sk);
	if (error) {
		ALOGE("genl_connect failed : %s (%d)", nl_geterror(error), error);
		return error;
	}

	/* Find the multicast group identifier and register ourselves to it. */
	group = genl_ctrl_resolve_grp(sk, "scsc_mdp_family", "scsc_mdp_grp");
	if (group < 0) {
		ALOGE("genl_ctrl_resolve_grp failed : %s (%d)", nl_geterror(error), error);
		return error;
	}

	family = genl_ctrl_resolve(sk, "scsc_mdp_family");
	if (family < 0) {
		ALOGE("genl_ctrl_resolve failed : %s (%d)", nl_geterror(error), error);
		return error;
	}

	ALOGI("add membership to group %u family %u", group, family);

	error = nl_socket_add_memberships(sk, group, 0);
	if (error) {
		ALOGE("nl_socket_add_memberships() failed  : %s (%d)", nl_geterror(error), error);
		return error;
	}

	check_setup();

	get_board_id();

	ALOGI("calling nl_recvmsgs_default");

	/* Finally, receive the messages */
	while (1)
		nl_recvmsgs_default(sk);

	return 0;
}

int main(void)
{
	int error;

	error = do_things();

	if (sk)
		nl_socket_free(sk);
	ALOGI("%s: end error = %d",__func__, error);
	return error;
}
