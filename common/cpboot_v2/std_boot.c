#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <ctype.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <cutils/properties.h>
#include <sched.h>

#include "boot.h"
#include "std_boot.h"
#include "shannon.h"
#include "util.h"
#ifdef CONFIG_SEC_CP_SECURE_BOOT
#include "cp_protect.h"
#endif

#define WAIT_POLL_TIME		30000	/* 30secs */

static struct std_boot_args std_boot;
static struct std_dump_args std_dump;
static char path[MAX_PATH_LEN];

#if 1
/* Common static functions for both STD_BOOT and STD_DUMP */
#endif

static int std_reboot_system(char *str)
{
#ifdef CONFIG_PROTOCOL_SIT
	return 0;
#else
	int ret;
	char reboot[PROPERTY_VALUE_MAX] = "reboot,";

	strcat(reboot, str);
	ret = property_set(PROP_SYS_POWERCTL, reboot);
	if (ret < 0)
		cbd_log("ERR! failed to setprop [sys.powerctl](%s)\n",
				reboot);
	return ret;
#endif
}

static unsigned char check_csc_sales_code()
{
#ifdef CONFIG_PROTOCOL_SIT
	return 0;
#else
	char secure_reboot[PROPERTY_VALUE_MAX];

	property_get(PROP_SALES_CODE, secure_reboot, "");

	cbd_log("sales_code:%s\n", secure_reboot);

	if (strcmp(secure_reboot, "TMB") == 0)
		return 1;
	else if (strcmp(secure_reboot, "VZW") == 0)
		return 1;

	return 0;
#endif
}

#ifdef CONFIG_PROTOCOL_SIT
static int std_udl_poll_sit(int fd, short events, long timeout)
{
	int ret;
	struct pollfd pfd;
	int count = 0;

	pfd.fd = fd;
	pfd.events = events;
	while (1) {
		pfd.revents = 0;

		/* Wait "events" up to "timeout" msec */
		ret = poll(&pfd, 1, timeout);
		if (pfd.revents & events)
			break;
		if (pfd.revents == (POLLERR | POLLHUP)) {
			ret = -EIO;
			goto exit;
		} else if (pfd.revents & POLLHUP) {
			/* TODO */
			if (count++ > 100)
				return -EIO;

			usleep(200000);
			continue;
		}
	}

	return 0;

exit:
	return ret;
}
#endif

static int std_udl_poll(int fd, short events, long timeout)
{
	int ret;
	struct pollfd pfd;

	pfd.fd = fd;
	pfd.events = events;
	while (1) {
		pfd.revents = 0;

		/* Wait "events" up to "timeout" msec */
		ret = poll(&pfd, 1, timeout);
		if (pfd.revents & events)
			break;

		if (ret > 0)
			cbd_log("ERR! poll fail (events 0x%X != revents 0x%X)\n", events, pfd.revents);
		else if (ret == 0)
			cbd_log("ERR! poll fail (events 0x%X, TIMEOUT)\n", events);
		else
			cbd_log("ERR! poll fail (events 0x%X, %s)\n", events, ERR2STR);

		ret = -EIO;
		goto exit;
	}

	return 0;

exit:
	return ret;
}

#ifdef CONFIG_PROTOCOL_SIT
static int std_udl_req_resp_sit(int fd, u32 req, u32 exp)
{
	int ret = 0;
	u32 resp;

	/* Send a request to CP if exists */
	if (req) {
		cbd_log("request:0x%08X\n", req);
		ret = write(fd, &req, sizeof(u32));
		if (ret < 0) {
			cbd_log("ERR! write fail (%s)\n", ERR2STR);
			goto exit;
		}
	}

	/* Receive and verify a response from CP if expected */
	if (exp) {
		/* Wait for a response from CP up to WAIT_POLL_TIME ms */
		ret = std_udl_poll_sit(fd, POLLIN, WAIT_POLL_TIME);
		if (ret < 0) {
			cbd_log("ERR! std_udl_poll fail\n");
			goto exit;
		}

		/* Receive a response from CP */
		ret = read(fd, &resp, sizeof(u32));
		if (ret < 0) {
			cbd_log("ERR! read fail (%s)\n", ERR2STR);
			goto exit;
		}

		/* Verify the response */
		if (resp != exp) {
			cbd_log("ERR! resp 0x%X != exp 0x%X\n", resp, exp);
			ret = -EFAULT;
			goto exit;
		}
	}

	return 0;

exit:
	return ret;
}
#endif

static int std_udl_req_resp(int fd, u32 req, u32 exp)
{
	int ret = 0;
	u32 resp;

	/* Send a request to CP if exists */
	if (req) {
		cbd_log("request:0x%08X\n", req);
		ret = write(fd, &req, sizeof(u32));
		if (ret < 0) {
			cbd_log("ERR! write fail (%s)\n", ERR2STR);
			goto exit;
		}
	}

	/* Receive and verify a response from CP if expected */
	if (exp) {
		/* Wait for a response from CP up to WAIT_POLL_TIME ms */
		ret = std_udl_poll(fd, POLLIN, WAIT_POLL_TIME);
		if (ret < 0) {
			cbd_log("ERR! std_udl_poll fail\n");
			goto exit;
		}

		/* Receive a response from CP */
		ret = read(fd, &resp, sizeof(u32));
		if (ret < 0) {
			cbd_log("ERR! read fail (%s)\n", ERR2STR);
			goto exit;
		}

		/* Verify the response */
		if (resp != exp) {
			cbd_log("ERR! resp 0x%X != exp 0x%X\n", resp, exp);
			ret = -EFAULT;
			goto exit;
		}
	}

	return 0;

exit:
	cbd_log("ERR! request:0x%08X expected:0x%08X\n", req, exp);
	return ret;
}

static int std_udl_stage_start(int fd, u32 stage)
{
	int ret;
	u32 req = 0xFFFF;
	u32 exp = 0xFFFF;

#ifndef CONFIG_PROTOCOL_SIT
	req = STD_UDL_AP2CP | (stage << STD_UDL_STAGE_SHIFT) | STD_UDL_STAGE_START;
	exp = STD_UDL_CP2AP | (stage << STD_UDL_STAGE_SHIFT) | STD_UDL_STAGE_START;
#else
	if (stage == STD_UDL_FIN_STAGE) {
		req = MSG(MSG_AP2CP, MSG_FINALIZE, MSG_NONE_TOC, MSG_PASS);
		exp = MSG(MSG_CP2AP, MSG_FINALIZE, MSG_NONE_TOC, MSG_PASS);
	} else {
		req = MSG(MSG_AP2CP, MSG_DOWNLOAD, stage, MSG_START);
		exp = MSG(MSG_CP2AP, MSG_DOWNLOAD, stage, MSG_START);
	}
#endif

	/* Send a request to CP, then receive and check a response from CP */
	ret = std_udl_req_resp(fd, req, exp);
	if (ret < 0) {
		cbd_log("ERR! [stage %d] START fail (req:0x%X exp:0x%X)\n",
			stage, req, exp);
	}

	return ret;
}

static int std_udl_stage_done(int fd, u32 stage, u32 boot_stage)
{
	int ret;
	u32 req = 0xFFFF;
	u32 exp = 0xFFFF;

#ifndef CONFIG_PROTOCOL_SIT
	req = STD_UDL_AP2CP | (stage << STD_UDL_STAGE_SHIFT) | STD_UDL_STAGE_DONE;
	exp = STD_UDL_CP2AP | (stage << STD_UDL_STAGE_SHIFT) | STD_UDL_STAGE_DONE;
#else
	if (stage == boot_stage) {
		req = MSG(MSG_AP2CP, MSG_READY, stage, MSG_BOOT);
		exp = MSG(MSG_CP2AP, MSG_READY, stage, MSG_BOOT);
	} else {
		req = MSG(MSG_AP2CP, MSG_DOWNLOAD, stage, MSG_DONE);
		exp = MSG(MSG_CP2AP, MSG_DOWNLOAD, stage, MSG_DONE);
	}
#endif

	/* Send a request to CP, then receive and check a response from CP */
	ret = std_udl_req_resp(fd, req, exp);
	if (ret < 0) {
		cbd_log("ERR! [stage %d] DONE fail (req:0x%X exp:0x%X)\n",
			stage, req, exp);
	}

	return ret;
}

static int std_dl_tx_frame(int fd, int b_fd, struct std_udl_frame *frm)
{
	int ret = 0;
	u32 frm_len = STD_UDL_HDR_LEN + frm->len;
	u32 rcvd;
	u32 sent;

	memset(frm->data, 0, STD_UDL_MSS);

	/* Read a segment of a CP binary */
	rcvd = ret = read(b_fd, frm->data, frm->len);
	if (ret < 0) {
		cbd_log("ERR! read fail (%s)\n", ERR2STR);
		goto exit;
	}

	if (rcvd != frm->len) {
		cbd_log("ERR! rcvd %d != frm->len %d\n", rcvd, frm->len);
		ret = -EFAULT;
		goto exit;
	}

	/* Send the CP binary segment */
	sent = ret = write(fd, frm, frm_len);
	if (ret < 0) {
		cbd_log("ERR! write fail (%s)\n", ERR2STR);
		goto exit;
	}

	if (sent != frm_len) {
		cbd_log("ERR! sent %d != frm_len %d\n", sent, frm_len);
		ret = -EFAULT;
		goto exit;
	}

	return 0;

exit:
	return ret;
}

#ifdef CONFIG_PROTOCOL_SIT
static int std_dl_tx_frame_sit(int fd, int b_fd, struct exynos_payload *payload)
{
	int ret = 0;
	u32 len = payload->length;
	u32 rcvd;
	u32 sent;

	/* Read a segment of a CP binary */
	rcvd = ret = read(b_fd, payload->pdata, len);
	if (ret < 0) {
		cbd_log("ERR! read fail (%s)\n", ERR2STR);
		goto exit;
	}

	if (rcvd != len) {
		cbd_log("ERR! rcvd %d != frm->len %d\n", rcvd, payload->length);
		ret = -EFAULT;
		goto exit;
	}

	payload->length = len + sizeof(struct exynos_data_info);
	len += 12; /* cmd, length, dataInfo */

	/* Send the CP binary segment */
	sent = ret = write(fd, payload, len);
	if (ret < 0) {
		cbd_log("ERR! write fail (%s)\n", ERR2STR);
		goto exit;
	}

	if (sent != len) {
		cbd_log("ERR! sent %d != frm_len %d\n", sent, len);
		ret = -EFAULT;
		goto exit;
	}

	return 0;

exit:
	return ret;
}
#endif

static int std_dl_send_bin(struct std_boot_args *args, u32 stage, int b_fd,
			u32 b_offset, u32 size)
{
	int ret;
	int dev_fd = args->dev_fd;
	u32 rest = size;
#ifndef CONFIG_PROTOCOL_SIT
	u32 exp = STD_UDL_CP2AP | (stage << STD_UDL_STAGE_SHIFT) | STD_UDL_SEND;
#else
	u32 exp = MSG(MSG_CP2AP, MSG_DOWNLOAD, stage, MSG_DATA);
#endif
	struct std_udl_frame *frm = &args->frame_buff;
#ifndef CONFIG_PROTOCOL_SIT
	struct std_dload_info dl_info;
#else
	struct exynos_payload payload;
#endif

	memset(frm, 0, sizeof(struct std_udl_frame));

	/* Set a file pointer for a CP binary (MAIN, NV, etc.) */
	ret = lseek(b_fd, b_offset, SEEK_SET);
	if (ret < 0) {
		cbd_log("ERR! lseek fail (%s)\n", ERR2STR);
		goto exit;
	}

	/* Set DLOAD command */
#ifndef CONFIG_PROTOCOL_SIT
	frm->cmd = STD_UDL_AP2CP | (stage << STD_UDL_STAGE_SHIFT) | STD_UDL_SEND;
#else
	frm->cmd = MSG(MSG_AP2CP, MSG_DOWNLOAD, stage, MSG_DATA);
#endif

	/* Calculate the number of frames to be trsnamitted to CP */
	frm->num_frames = (size / STD_UDL_MSS);
	if (size > (STD_UDL_MSS * frm->num_frames))
		frm->num_frames++;

	/* Print DLOAD information at each stage */
	cbd_log("stage = %d\n", stage);
	cbd_log("size = %d (0x%X)\n", size, size);
	cbd_log("mtu = %d\n", STD_UDL_MTU);
	cbd_log("mss = %d\n", STD_UDL_MSS);
	cbd_log("frames = %d\n", frm->num_frames);

#ifndef CONFIG_PROTOCOL_SIT
	/* Set DLOAD information for works in kernel */
	dl_info.size = size;
	dl_info.mtu = STD_UDL_MTU;
	dl_info.num_frames = frm->num_frames;
#endif

	/* Read and send the CP binary */
	while (rest > 0) {
		frm->curr_frame++;
		frm->len = (rest < STD_UDL_MSS) ? rest : STD_UDL_MSS;

#ifndef CONFIG_PROTOCOL_SIT
		ret = std_dl_tx_frame(dev_fd, b_fd, frm);
#else
		memset(&payload, 0x0, sizeof(struct exynos_payload));

		payload.cmd = frm->cmd;
		payload.length = frm->len;

		payload.dataInfo.total_size = size;
		payload.dataInfo.data_offset = size - rest;

		ret = std_dl_tx_frame_sit(dev_fd, b_fd, &payload);
#endif
		if (ret < 0) {
			cbd_log("ERR! std_dl_tx_frame fail\n");
			goto exit;
		}

		rest -= frm->len;

#ifdef CONFIG_PROTOCOL_SIT
		/* Receive and check a response from CP */
		ret = std_udl_req_resp(dev_fd, 0, exp);
		if (ret < 0) {
			cbd_log("ERR! std_udl_req_resp fail\n");
			goto exit;
		}
#endif
	}

#ifndef CONFIG_PROTOCOL_SIT
	/* Receive and check a response from CP */
	ret = std_udl_req_resp(dev_fd, 0, exp);
	if (ret < 0) {
		cbd_log("ERR! std_udl_req_resp fail\n");
		goto exit;
	}
#endif

	return 0;

exit:
	return ret;
}

static int std_dl_send_crc(struct std_boot_args *args, u32 stage, u32 crc)
{
	int ret;
	int dev_fd = args->dev_fd;
#ifndef CONFIG_PROTOCOL_SIT
	u32 exp = STD_UDL_CP2AP | (stage << STD_UDL_STAGE_SHIFT) | STD_UDL_CRC;
#else
	u32 exp = MSG(MSG_CP2AP, MSG_SECURITY, stage, MSG_PASS);
#endif
	struct std_udl_crc_frame crc_frm;

#ifndef CONFIG_PROTOCOL_SIT
	crc_frm.cmd = STD_UDL_AP2CP | (stage << STD_UDL_STAGE_SHIFT) | STD_UDL_CRC;
#else
	crc_frm.cmd = MSG(MSG_AP2CP, MSG_SECURITY, stage, MSG_CRC);
#endif
	crc_frm.crc = crc;
	cbd_log("cmd = 0x%x, crc = 0x%x\n", crc_frm.cmd, crc_frm.crc);

	/* Send a CRC data */
	ret = write(dev_fd, &crc_frm, sizeof(struct std_udl_crc_frame));
	if (ret < 0) {
		cbd_log("ERR! write fail (%s)\n", ERR2STR);
		goto exit;
	}

	/* Receive and check a response from CP */
	ret = std_udl_req_resp(dev_fd, 0, exp);
	if (ret < 0) {
		cbd_log("ERR! std_udl_req_resp fail\n");
		goto exit;
	}

	return 0;

exit:
	return ret;
}

#if 1
/* Static functions for STD_DUMP */
#endif

int std_ul_rx_frame(struct std_dump_args *args, void *buff, u32 size)
{
	int ret;
	int dev_fd = args->dev_fd;
	int log_fd = args->log_fd;

	/* Wait for a DUMP frame from CP up to WAIT_POLL_TIME ms */
	ret = std_udl_poll(dev_fd, POLLIN, WAIT_POLL_TIME);
	if (ret < 0) {
		cbd_log("ERR! DUMP std_udl_poll fail\n");
		dprintf(log_fd, "%s: ERR! DUMP std_udl_poll fail (%s)\n",
			__func__, ERR2STR);
		goto exit;
	}

	/* Receive a DUMP frame from CP */
	ret = read(dev_fd, buff, size);
	if (ret < 0) {
		cbd_log("ERR! DUMP read fail (%s)\n", ERR2STR);
		dprintf(log_fd, "%s: ERR! DUMP read fail (%s)\n", __func__, ERR2STR);
		goto exit;
	}

exit:
	return ret;
}

static int std_ul_recv_info(struct std_dump_args *args)
{
	int ret;
	int log_fd = args->log_fd;
	int info_fd = args->info_fd;
	struct std_uload_info *ul_info = &args->info;
	char *buff;
	int status;
	struct crash_reason reason;
	char string_crash_reason[][5] = { "RIL", "USER", "CPIF", "CP"};
	char string[CRASH_REASON_SIZE];

	/* Receive DUMP information */
	ret = std_ul_rx_frame(args, ul_info, sizeof(struct std_uload_info));
	if (ret < 0) {
		cbd_log("ERR! INFO std_ul_rx_frame fail\n");
		dprintf(log_fd, "%s: ERR! INFO std_ul_rx_frame fail\n", __func__);
		goto exit;
	}

	/* Print DUMP information */
	cbd_log("dump_size = %d\n", ul_info->dump_size);
	cbd_log("num_steps = %d\n", ul_info->num_steps);
	cbd_log("reason_len = %d\n", ul_info->reason_len);

	/* Receive CP CRASH reason  */
	buff = args->reason + strlen(args->reason);
	ret = std_ul_rx_frame(args, buff, ul_info->reason_len);
	if (ret < 0) {
		cbd_log("ERR! REASON std_ul_rx_frame fail\n");
		dprintf(log_fd, "%s: ERR! REASON std_ul_rx_frame fail\n", __func__);
		goto exit;
	}

	/* Receive the CP CRASH reason */
	memset(string, 0, STD_CRASH_REASON_SIZE);
	memset(args->cbd_args->reason, 0, STD_CRASH_REASON_SIZE);
	ret = ioctl(args->dev_fd, IOCTL_GET_CP_CRASH_REASON, &reason);
	if (ret < 0)
		cbd_log("ERR! IOCTL_GET_CP_CRASH_REASON fail (%s)\n", ERR2STR);
	else {
		cbd_log("reason owner:%d\n", reason.owner);

		switch (reason.owner) {
		case CRASH_REASON_RIL_MNR:
		case CRASH_REASON_RIL_REQ_FULL:
		case CRASH_REASON_RIL_PHONE_DIE:
		case CRASH_REASON_RIL_RSV_MAX:
		case CRASH_REASON_RIL_TRIGGER_CP_CRASH:
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[0]); /* RIL */
			strcat(string, reason.string);
			break;

		case CRASH_REASON_USER:
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[1]); /* USER */
			strcat(string, reason.string);
			break;

		case CRASH_REASON_MIF_TX_ERR:
		case CRASH_REASON_MIF_RIL_BAD_CH:
		case CRASH_REASON_MIF_RX_BAD_DATA:
		case CRASH_REASON_MIF_FORCED:
		case CRASH_REASON_MIF_RSV_MAX:
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[2]); /* CPIF */
			strcat(string, reason.string);
			break;

		case CRASH_REASON_CP_SRST:
		case CRASH_REASON_CP_RSV_0:
		case CRASH_REASON_CP_RSV_MAX:
		case CRASH_REASON_CP_ACT_CRASH:
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[3]); /* CP */
			strcat(string, buff);
			break;

		case CRASH_REASON_CP_WDOG_CRASH:
			/* In watchdog case, reason string should be set by AP */
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[3]); /* CP */
			strcat(string, STD_WDT_RESET_STR);
			break;

		default:
			sprintf(string, ": Crash by (%d)", reason.owner);
			break;
		}
		strcpy(args->cbd_args->reason, string);
	}
	cbd_log("CP CRASH: %s\n", args->cbd_args->reason);

	/* Store the CP CRASH reason */
	ret = write(info_fd, args->cbd_args->reason, STD_CRASH_REASON_SIZE);
	if (ret < 0) {
		cbd_log("ERR! REASON write fail (%s)\n", ERR2STR);
		dprintf(log_fd, "%s: ERR! REASON write fail (%s)\n", __func__, ERR2STR);
		goto exit;
	}

	if (fsync(info_fd)) {
		cbd_log("ERR! fsync(info_fd) fail (%s)\n", ERR2STR);
		dprintf(log_fd, "%s: ERR! fsync(info_fd) fail (%s)\n",
			__func__, ERR2STR);
		goto exit;
	}
	cbd_log("DUMP INFO saved\n");
	return 0;

exit:
	return ret;
}

static int std_ul_recv_data(struct std_dump_args *args, u32 step)
{
	int ret;
	int saved;
	int dev_fd = args->dev_fd;
	int log_fd = args->log_fd;
	int dump_fd = args->dump_fd;
	struct std_udl_frame *frm = &args->frame_buff;
	u32 req = STD_UDL_AP2CP | (STD_UDL_DUMP_STAGE << STD_UDL_STAGE_SHIFT) | step;
	u32 exp = STD_UDL_CP2AP | (STD_UDL_DUMP_STAGE << STD_UDL_STAGE_SHIFT) | step;
	u32 seqn;

	memset(frm, 0, sizeof(struct std_udl_frame));

	/*
	** Send "START of each step" command to CP
	*/
	ret = std_udl_req_resp(dev_fd, req, 0);
	if (ret < 0) {
		cbd_log("ERR! [step %d] start fail (ret %d)\n",
			step, ret);
		dprintf(log_fd, "%s: ERR! [step %d] start fail (ret %d)\n",
			__func__, step, ret);
		goto exit;
	}

	/*
	** Receive DUMP frames of each step from CP and store them
	*/
	seqn = 1;
	saved = 0;
	do {
		/* Receive a DUMP frame from CP */
		ret = std_ul_rx_frame(args, frm, sizeof(struct std_udl_frame));
		if (ret < 0) {
			cbd_log("ERR! [step %d] std_ul_rx_frame fail (ret %d)\n",
				step, ret);
			dprintf(log_fd, "%s: ERR! [step %d] std_ul_rx_frame fail (ret %d)\n",
				__func__, step, ret);
			goto exit;
		}

		/* Verify the command in the frame */
		if (frm->cmd != exp) {
			cbd_log("ERR! [step %d] cmd 0x%X != exp 0x%X\n",
				step, frm->cmd, exp);
			dprintf(log_fd, "%s: ERR! [step %d] cmd 0x%X != exp 0x%X\n",
				__func__, step, frm->cmd, exp);
			ret = -EFAULT;
			goto exit;
		}

		/* Verify the sequence number in the frame */
		if (frm->curr_frame != seqn) {
			cbd_log("ERR! [step %d] curr_frame %d != seqn %d\n",
				step, frm->curr_frame, seqn);
			dprintf(log_fd, "%s: ERR! [step %d] curr_frame %d != seqn %d\n",
				__func__, step, frm->curr_frame, seqn);
			goto exit;
		}
		seqn++;

		/* Record the information of each step at the start of the step */
		if (frm->curr_frame == 1) {
			cbd_log("[step %d] num_frames = %d\n", step, frm->num_frames);
			cbd_log("[step %d] command = 0x%X\n", step, frm->cmd);
			dprintf(log_fd, "%s: [step %d] num_frames = %d\n",
				__func__, step, frm->num_frames);
			dprintf(log_fd, "%s: [step %d] command = 0x%X\n",
				__func__, step, frm->cmd);
		}

		/* Store the DUMP data in the frame */
		ret = write(dump_fd, frm->data, frm->len);
		if (ret < 0) {
			cbd_log("ERR! [step %d] seq# %d write fail (%s)\n",
				step, frm->curr_frame, ERR2STR);
			dprintf(log_fd, "%s: ERR! [step %d] seq# %d write fail (%s)\n",
				__func__, step, frm->curr_frame, ERR2STR);
			goto exit;
		}

		/* Update "saved" variable */
		saved += frm->len;
	} while (frm->curr_frame < frm->num_frames);

	cbd_log("[step %d] saved = %d\n", step, saved);
	return saved;

exit:
	return ret;
}

#if 1
/* Functions for STD_BOOT */
#endif

struct std_boot_args *std_boot_prepare_args(struct boot_args *cbd_args, u32 num_stages)
{
	int dev_fd;
	struct modem_comp *cpn = cbd_args->cpn;
	struct std_boot_args *args = &std_boot;
#ifdef CONFIG_SEC_CP_SECURE_BOOT
	int ret;
#endif

	memset(args, 0, sizeof(struct std_boot_args));

	/* Open the boot device */
	dev_fd = open(cpn->node_boot, O_RDWR);
	if (dev_fd < 0) {
		cbd_log("ERR! DEV(%s) open fail (%s)\n", cpn->node_boot, ERR2STR);
		goto exit;
	}
	cbd_log("DEV(%s) opened (fd %d)\n", cpn->node_boot, dev_fd);

	args->cbd_args = cbd_args;
	args->dev_fd = dev_fd;
	args->num_stages = num_stages;

#ifdef CONFIG_SEC_CP_SECURE_BOOT
	ret = sec_cp_init();
	if (ret) {
		cbd_log("ERR! sec_cp_init fail (err %d)\n", ret);
		goto exit;
	}
	cbd_log("SECURE_BOOT initialized\n");
#endif

	return args;

exit:
	return NULL;
}

void std_boot_close_args(struct std_boot_args *args)
{
	if (args) {
		if (args->dev_fd >= 0)
			close(args->dev_fd);

		memset(args, 0, sizeof(struct std_boot_args));
	}
}

int std_boot_load_cp_bootloader(struct std_boot_args *args, u32 stage)
{
	int ret = 0;
	int dev_fd = args->dev_fd;
	struct std_dload_control *dlc = &args->dl_ctrl[stage];
	struct cp_image img;

	cbd_log("size = %d\n", dlc->b_size);

	/* Prepare an image buffer */
	img.binary = malloc(dlc->b_size);
	if (!img.binary) {
		cbd_log("ERR! malloc(%d) fail\n", dlc->b_size);
		ret = -ENOMEM;
		goto exit;
	}
	img.size = dlc->b_size;

	/* Read BOOT loader */
	ret = lseek(dlc->b_fd, dlc->b_offset, SEEK_SET);
	if (ret < 0) {
		cbd_log("ERR! lseek fail (%s)\n", ERR2STR);
		goto exit;
	}

	ret = read(dlc->b_fd, img.binary, img.size);
	if (ret < 0) {
		cbd_log("ERR! read fail (%s)\n", ERR2STR);
		goto exit;
	}
	if ((u32)ret != img.size) {
		cbd_log("ERR! read %d != img.size %d\n", ret, img.size);
		ret = -EFAULT;
		goto exit;
	}

	/* Send BOOT loader */
	ret = ioctl(dev_fd, IOCTL_LOAD_CP_IMAGE, &img);
	if (ret < 0) {
		cbd_log("ERR! IOCTL_LOAD_CP_IMAGE fail (%s)\n", ERR2STR);
		goto exit;
	}

#ifdef CONFIG_SEC_CP_SECURE_BOOT
	if (dlc->validate) {
		struct shdmem_info mem_info;

		ioctl(dev_fd, IOCTL_MODEM_GET_SHMEM_INFO, &mem_info);
		cbd_log("SHMEM base:0x%08x size:%d\n", mem_info.base, mem_info.size);

		/* Validate CP bootloader */
		ret = sec_cp_validate_boot(mem_info.base, mem_info.size, mem_info.base, dlc->b_size);
		if (ret) {
			cbd_log("ERR! BOOT validation fail (err %d)\n", ret);
			goto exit;
		}
		cbd_log("BOOT validated\n");
	}
#endif

	cbd_log("xmit bootloader complete!\n");
exit:
	if (img.binary)
		free(img.binary);

	return ret;
}

int std_boot_power_on(struct std_boot_args *args)
{
	int ret;
	int dev_fd = args->dev_fd;

	ret = ioctl(dev_fd, IOCTL_POWER_ON, NULL);
	if (ret < 0) {
		cbd_log("ERR! IOCTL_POWER_ON fail\n");
		goto exit;
	}

	return 0;

exit:
	return ret;
}

int std_boot_power_off(struct std_boot_args *args)
{
	int ret;
	int dev_fd = args->dev_fd;

	ret = ioctl(dev_fd, IOCTL_POWER_OFF, NULL);
	if (ret < 0) {
		cbd_log("ERR! IOCTL_POWER_OFF fail\n");
		goto exit;
	}

	return 0;

exit:
	return ret;
}

int std_boot_power_reset(struct std_boot_args *args, enum cp_boot_mode mode_idx)
{
	int ret;
	int dev_fd = args->dev_fd;
	struct boot_mode mode;

	mode.idx = mode_idx;
	ret = ioctl(dev_fd, IOCTL_POWER_RESET, &mode);
	if (ret < 0) {
		cbd_log("ERR! IOCTL_POWER_RESET fail\n");
		goto exit;
	}

	return 0;

exit:
	return ret;
}

int std_boot_finish_handshake_sit(struct std_boot_args *args)
{
	int ret = -1;

#ifdef CONFIG_PROTOCOL_SIT
	int dev_fd = args->dev_fd;
	u32 req = MSG(MSG_AP2CP, MSG_FINALIZE, MSG_NONE_TOC, MSG_PASS);
	u32 exp = MSG(MSG_CP2AP, MSG_FINALIZE, MSG_NONE_TOC, MSG_PASS);

	ret = std_udl_req_resp(dev_fd, req, exp);
	if (ret < 0) {
		cbd_log("ERR! req:0x%X exp:0x%X\n", req, exp);
	}
#endif

	return ret;
}

int std_boot_finish_handshake(struct std_boot_args *args)
{
	int ret;
	int dev_fd = args->dev_fd;
	int boot_stage = args->cbd_args->cpn->boot_stage;

	/* BOOT_DONE command */
	ret = std_udl_stage_done(dev_fd, STD_UDL_STAGE_START, boot_stage);
	if (ret < 0) {
		cbd_log("ERR! std_udl_stage_done fail\n");
		goto exit;
	}

	/* FIN command */
	ret = std_udl_stage_start(dev_fd, STD_UDL_FIN_STAGE);
	if (ret < 0) {
		cbd_log("ERR! std_udl_stage_done fail\n");
		goto exit;
	}

	return 0;
exit:
	return ret;
}

int std_boot_start_cp_bootloader(struct std_boot_args *args, enum cp_boot_mode mode_idx)
{
	int ret;
	int dev_fd = args->dev_fd;
	struct boot_mode mode;

	mode.idx = mode_idx;
	ret = ioctl(dev_fd, IOCTL_START_CP_BOOTLOADER, &mode);
	if (ret < 0) {
		cbd_log("ERR! IOCTL_START_CP_BOOTLOADER fail\n");
		goto exit;
	}

	return 0;

exit:
	return ret;
}

int std_boot_load_cp_images(struct std_boot_args *args)
{
	int ret = 0;
	int max_stages = args->num_stages;
	int i;
	int start_stage;
	int boot_stage = args->cbd_args->cpn->boot_stage;

	if (args->cbd_args->lnk_boot == LINKDEV_SPI &&
			args->cbd_args->lnk_main == LINKDEV_PCIE)
		/* BOOT was already downloaded by SPI link */
		start_stage = args->cbd_args->cpn->toc_stage;
	else
		start_stage = boot_stage;

#ifdef CONFIG_PROTOCOL_SIT
	max_stages += 1;
#endif

	/* {BOOT(?), TOC, MAIN, NV, ... , FIN} stages */
	for (i = start_stage; i < max_stages; i++) {
		int dev_fd = args->dev_fd;
		struct std_dload_control *dlc = &args->dl_ctrl[i];
		u32 stage = dlc->stage;

		if (dlc->start) {
			ret = std_udl_stage_start(dev_fd, stage);

			if (ret < 0) {
				cbd_log("ERR! [%d] std_udl_stage_start fail\n", stage);
				goto exit;
			}
		}

		if (dlc->download) {
			ret = std_dl_send_bin(args, stage, dlc->b_fd, dlc->b_offset, dlc->b_size);
			if (ret < 0) {
				cbd_log("ERR! [%d] std_dl_send_bin fail\n", stage);
				goto exit;
			}
		}

		if (dlc->download && dlc->validate) {
#ifdef CONFIG_SEC_CP_SECURE_BOOT
			if (args->cbd_args->lnk_boot == LINKDEV_SHMEM) {
				struct shdmem_info mem_info;
				u32 magic_base;
				u32 dl_base;
				u32 dl_size;

				ioctl(dev_fd, IOCTL_MODEM_GET_SHMEM_INFO, &mem_info);
				magic_base = mem_info.base + CP_FIRM_MAGIC_OFFSET;
				dl_base = mem_info.base + CP_FIRM_DL_OFFSET;
				dl_size = dlc->b_size;
				cbd_log("magic@0x%08x dload@0x%08x size:%d\n",
					magic_base, dl_base, dl_size);

				/* Validate CP MAIN binary */
				ret = sec_cp_validate_main(magic_base, dl_base, dl_size);
				if (ret) {
					cbd_log("ERR! [%d] binary validation fail\n",
						stage);
					goto exit;
				}
				cbd_log("[%d] binary validated\n", stage);
			}
#else
			ret = std_dl_send_crc(args, stage, dlc->crc);
			if (ret < 0) {
				if (check_csc_sales_code()) {
					cbd_log("secure err: Invalid Main image\n");
					std_reboot_system("secure");
				} else {
					cbd_log("ERR! [%d] std_dl_send_crc fail\n", stage);
				}

				goto exit;
			}
#endif
		}

		if (dlc->finish) {
			ret = std_udl_stage_done(dev_fd, stage, boot_stage);
			if (ret < 0) {
				cbd_log("ERR! [%d] std_udl_stage_done fail\n", stage);
				goto exit;
			}
		}
	}

	return 0;

exit:
	return ret;
}

int std_boot_complete_normal_bootup(struct std_boot_args *args)
{
	int ret;
	int dev_fd = args->dev_fd;

	ret = ioctl(dev_fd, IOCTL_COMPLETE_NORMAL_BOOTUP, NULL);
	if (ret < 0) {
		cbd_log("ERR! IOCTL_COMPLETE_NORMAL_BOOTUP fail\n");
		ioctl(dev_fd, IOCTL_GET_CP_BOOTLOG, NULL);
		goto exit;
	}

	ioctl(dev_fd, IOCTL_CLR_CP_BOOTLOG, NULL);

	return 0;

exit:
	return ret;
}

static int std_dump_write_versions(struct std_dump_args *args)
{
	int ret;
	struct cpif_version version;

	ret = ioctl(args->dev_fd, IOCTL_GET_CPIF_VERSION, &version);
	if (ret < 0) {
		dprintf(args->log_fd, "ERR! IOCTL_GET_CPIF_VERSION fail (%s)\n", ERR2STR);
		return -EINVAL;
	}

	cbd_log("CPIF version: %s\n", version.string);
	cbd_log("CP Boot Daemon (CBD) version: %s\n", get_cbd_version());
	dprintf(args->log_fd, "CPIF version: %s\n", version.string);
	dprintf(args->log_fd, "CP Boot Daemon (CBD) version: %s\n", get_cbd_version());

	return 0;
}

struct std_dump_args *std_dump_prepare_args(struct boot_args *cbd_args)
{
	int i;
	int len;
	char *log_root = get_log_root();
	char *log_dir = get_log_dir();
	struct modem_comp *cpn = cbd_args->cpn;
	struct std_dump_args *args = &std_dump;
	int dev_fd = -1;
	int log_fd = -1;
	int info_fd = -1;
	int dump_fd = -1;
	char prefix[MAX_PREFIX_LEN];
	char suffix[MAX_SUFFIX_LEN];
	time_t now;
	struct tm result;

#ifdef CONFIG_DUMP_LIMIT
	/* remove garbage directory if necessary */
	remove_directory(log_dir);
#endif

	if (create_log_directory(log_dir) < 0)
		goto exit;

	if (check_fs_free_space(log_root) < 0) {
		remove_logs(LOG_DMESG, log_dir, cpn->rat);
		if (check_fs_free_space(log_root) < 0)
			goto exit;
	}

	memset(args, 0, sizeof(struct std_dump_args));

	len = strlen(cpn->rat);
	for (i = 0; i < len; i++)
		args->reason[i] = toupper(cpn->rat[i]);
	strcat(args->reason, ": ");

	/* Open the DUMP device */
	dev_fd = open(cpn->node_dump, O_RDWR);
	if (dev_fd < 0) {
		cbd_log("ERR! %s open fail (%s)\n", cpn->node_dump, ERR2STR);
		goto exit;
	}
	cbd_log("%s opened (fd %d)\n", cpn->node_dump, dev_fd);
	args->dev_fd = dev_fd;

	/* Set prefix and suffix for DUMP file paths */
	snprintf(prefix, MAX_PREFIX_LEN, "cpcrash_%s", cpn->rat);
	time(&now);
	localtime_r(&now, &result);
	strftime(suffix, MAX_SUFFIX_LEN, "%Y%m%d-%H%M", &result);

	/* Open (create) a CP crash log file */
	sprintf(path, "%s/%s_log_%s.log", log_dir, prefix, suffix);
	log_fd = open(path, O_WRONLY | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (log_fd < 0) {
		cbd_log("ERR! %s open fail (%s)\n", path, ERR2STR);
		goto exit;
	}
	fchmod(log_fd, 0664);
	args->log_fd = log_fd;
	std_dump_write_versions(args);
	cbd_log("%s opened (fd %d)\n", path, log_fd);
	dprintf(log_fd, "%s: %s opened (fd %d)\n", __func__, path, log_fd);

	/* Open (create) a CP crash info file */
	sprintf(path, "%s/%s_info_%s_%s.log", log_dir, prefix, cpn->name, suffix);
	info_fd = open(path, O_WRONLY | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (info_fd < 0) {
		cbd_log("ERR! %s open fail (%s)\n", path, ERR2STR);
		dprintf(log_fd, "%s: ERR! %s open fail (%s)\n", __func__, path, ERR2STR);
		goto exit;
	}
	fchmod(info_fd, 0664);
	args->info_fd = info_fd;
	cbd_log("%s opened (fd %d)\n", path, info_fd);
	dprintf(log_fd, "%s: %s opened (fd %d)\n", __func__, path, info_fd);

	/* Open (create) a CP crash dump file */
	sprintf(path, "%s/%s_dump_%s.log", log_dir, prefix, suffix);
	dump_fd = open(path, O_WRONLY | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (dump_fd < 0) {
		cbd_log("ERR! %s open fail (%s)\n", path, ERR2STR);
		dprintf(log_fd, "%s: ERR! %s open fail (%s)\n", __func__, path, ERR2STR);
		goto exit;
	}
	fchmod(dump_fd, 0664);
	args->dump_fd = dump_fd;
	cbd_log("%s opened (fd %d)\n", path, dump_fd);
	dprintf(log_fd, "%s: %s opened (fd %d)\n", __func__, path, dump_fd);

	/* Set standard DUMP arguments */
	args->cbd_args = cbd_args;
	return args;

exit:
	return NULL;
}

void std_dump_close_args(struct std_dump_args *args)
{
	if (args) {
		if (args->dev_fd >= 0)
			close(args->dev_fd);

		if (args->info_fd >= 0)
			close(args->info_fd);

		if (args->dump_fd >= 0)
			close(args->dump_fd);

		if (args->log_fd >= 0)
			close(args->log_fd);

		memset(args, 0, sizeof(struct std_dump_args));
	}
}

#ifdef CONFIG_PROTOCOL_SIT
static int std_recv_raw_data_sit(int dev_fd, void *buffer, u32 size)
{
	int ret = -1;
	int readLen = 0;

	ret = std_udl_poll_sit(dev_fd, POLLIN, WAIT_POLL_TIME);
	if (ret < 0) {
		cbd_err("ERR! poll fail\n");
		goto exit;
	}

	readLen = ret = read(dev_fd, buffer, size);
	if (ret < 0) {
		cbd_err("ERR! read fail\n");
		goto exit;
	}
	ret = readLen;

exit:
	return ret;
}
#endif

int std_dump_receive_cp_dump_sit(struct std_dump_args *args)
{
	int ret = -1;

#ifdef CONFIG_PROTOCOL_SIT
	struct sit_header sendHeader;
	struct sit_header recvHeader;
	struct sit_crash_reason reason;
	struct sit_dump_info dump_info;
	char string_crash_by_AP[] = ": Crash by AP -";
	char string_crash_by_RIL[] = ": Crash by RIL -";
	char *buff;
	int dev_fd = args->dev_fd;
	int info_fd = args->info_fd;
	struct sit_send_header ssendHeader;
	struct sit_recv_header rrecvHeader;
	int recv_length = 0, prev_length = 0;
	int readLen = 0, writeLen = 0, saveLen = 0;
	u8 *pRawdata = NULL;
	u32 req, exp;

	int sequence = 0;
	int dump_fd = args->dump_fd;
	char string_crash_reason[][5] = { "RIL", "USER", "CPIF", "CP"};
	char string[CRASH_REASON_SIZE];

	/* Send DUMP Ready */
	req = MSG(MSG_AP2CP, MSG_READY, MSG_NONE_TOC, MSG_DUMP);
	exp = MSG(MSG_CP2AP, MSG_READY, MSG_NONE_TOC, MSG_DUMP);

	ret = std_udl_req_resp_sit(dev_fd, req, exp);
	if (ret < 0) {
		cbd_log("ERR!  Send READY request and wait response (req:0x%X exp:0x%X)\n",
			req, exp);
		goto exit;
	}

	/* Send DUMP START request and wait for the dump information from CP */
	sendHeader.cmd = MSG(MSG_AP2CP, MSG_UPLOAD, MSG_NONE_TOC, MSG_START);
	sendHeader.length = 0;

	ret = write(dev_fd, &sendHeader, sizeof(struct sit_header));
	if (ret < 0) {
		cbd_log("ERR! Send DUMP START request (req:0x%X)\n",
			req);
		goto exit;
	}

	ret = std_udl_poll_sit(dev_fd, POLLIN, WAIT_POLL_TIME);
	if (ret < 0) {
		cbd_log("ERR! Wait for dump information\n");
		goto exit;
	}

	ret = read(dev_fd, &recvHeader, sizeof(struct sit_header));
	if (ret < (int)sizeof(struct sit_header)) {
		cbd_err("ERR! Try reading dump information\n");
		ret = -1;
		goto exit;
	}

	if(recvHeader.cmd != MSG(MSG_CP2AP, MSG_UPLOAD, MSG_NONE_TOC, MSG_START)) {
		cbd_err("ERR! Wrong CMD received, (recv:0x%x)\n", recvHeader.cmd);
		ret = -1;
		goto exit;
	}

	memset(&dump_info, 0x0, sizeof(struct sit_dump_info));
	ret = read(dev_fd, &dump_info, recvHeader.length);
	if (ret < recvHeader.length) {
		cbd_err("ERR! Receiving Dump info\n");
		goto exit;
	}

	/* Receive the CP CRASH reason */
	memset(string, 0, STD_CRASH_REASON_SIZE);
	memset(args->cbd_args->reason, 0, STD_CRASH_REASON_SIZE);
	ret = ioctl(args->dev_fd, IOCTL_GET_CP_CRASH_REASON, &reason);
	if (ret < 0)
		cbd_log("ERR! IOCTL_GET_CP_CRASH_REASON fail (%s)\n", ERR2STR);
	else {
		cbd_log("reason owner:%d\n", reason.owner);

		switch (reason.owner) {
		case CRASH_REASON_RIL_MNR:
		case CRASH_REASON_RIL_REQ_FULL:
		case CRASH_REASON_RIL_PHONE_DIE:
		case CRASH_REASON_RIL_RSV_MAX:
		case CRASH_REASON_RIL_TRIGGER_CP_CRASH:
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[0]); /* RIL */
			strcat(string, reason.string);
			break;

		case CRASH_REASON_USER:
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[1]); /* USER */
			strcat(string, reason.string);
			break;

		case CRASH_REASON_MIF_TX_ERR:
		case CRASH_REASON_MIF_RIL_BAD_CH:
		case CRASH_REASON_MIF_RX_BAD_DATA:
		case CRASH_REASON_MIF_FORCED:
		case CRASH_REASON_MIF_RSV_MAX:
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[2]); /* CPIF */
			strcat(string, reason.string);
			break;

		case CRASH_REASON_CP_SRST:
		case CRASH_REASON_CP_RSV_0:
		case CRASH_REASON_CP_RSV_MAX:
		case CRASH_REASON_CP_ACT_CRASH:
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[3]); /* CP */
			strcat(string, dump_info.reason);
			break;

		case CRASH_REASON_CP_WDOG_CRASH:
			/* In watchdog case, reason string should be set by AP */
			sprintf(string, ": Crash by %s - ",
					string_crash_reason[3]); /* CP */
			strcat(string, STD_WDT_RESET_STR);
			break;

		default:
			sprintf(string, ": Crash by (%d)", reason.owner);
			break;
		}
		strcpy(args->cbd_args->reason, string);
	}
	cbd_log("CP CRASH: %s\n", args->cbd_args->reason);

	ret = write(info_fd, args->cbd_args->reason, STD_CRASH_REASON_SIZE);
	if (ret < 0) {
		cbd_err("ERR! writing dump info to info fd\n");
	}
	if (fsync(info_fd) < 0) {
		cbd_err("ERR! fsync info file\n");
	}

	if(close(info_fd) < 0) {
		cbd_err("ERR! close info file\n");
	}
	else
		args->info_fd = 0;

	memset(&ssendHeader, 0x0, sizeof(struct sit_send_header));
	memset(&rrecvHeader, 0x0, sizeof(struct sit_recv_header));

	ssendHeader.cmd = MSG(MSG_AP2CP, MSG_UPLOAD, MSG_NONE_TOC, MSG_DATA);
	ssendHeader.length = 8;
	ssendHeader.total_size = dump_info.dump_size;
	ssendHeader.data_offset = 0;

	while (1) {
#ifdef USE_SPI_BOOT_LINK
		ret = write(dev_fd, &ssendHeader, sizeof(struct sit_send_header));
		if (ret < 0) {
			cbd_err("ERR! write fail\n");
			goto exit;
		}
#endif
		// receiving dump header data
		ret = std_recv_raw_data_sit(dev_fd, &rrecvHeader, sizeof(struct sit_recv_header));
		if (ret < (int)sizeof(struct sit_recv_header)) {
			cbd_err("ERR! receive rrecvHeader ret: %d\n", ret);
			ret = -1;
			goto exit;
		}

		if (rrecvHeader.cmd != MSG(MSG_CP2AP, MSG_UPLOAD, MSG_NONE_TOC, MSG_DATA)) {
			cbd_err("received: 0x%X != expected : 0xC20B\n", rrecvHeader.cmd);
			goto exit;
		}
		recv_length = (rrecvHeader.length - 8);
		if (recv_length <=0) {
			cbd_err("ERR! recv_length is %d\n", recv_length);
			goto exit;
		}

		//manage raw data buffer
		if (pRawdata != NULL) {
			if (prev_length < recv_length) {
				free(pRawdata);
				pRawdata = NULL;
			} else {
				memset(pRawdata, 0x0, prev_length);
			}
		}

		if (pRawdata == NULL) {
			pRawdata = (u8*)calloc(recv_length, sizeof(u8));
			if (pRawdata == NULL) {
				cbd_err("ERR! malloc fail\n");
				goto exit;
			}
			prev_length = recv_length;
		}

		//receive dump raw data!
		readLen = ret = std_recv_raw_data_sit(dev_fd, pRawdata, recv_length);
		if (ret < 0) {
			cbd_err("ERR recv dump data fail\n");
			goto exit;
		}

		ssendHeader.data_offset += readLen;

		writeLen = ret = write(dump_fd, pRawdata, readLen);
		if (ret < 0) {
			cbd_err("ERR write dump data fail\n");
			goto exit;
		}

		saveLen += writeLen;

		if (ssendHeader.data_offset >= ssendHeader.total_size) {
			if (ssendHeader.data_offset > ssendHeader.total_size)
				cbd_info("Warning:: too much data received\n");
			cbd_info("Complete Crash Dump!!\n");
			break;
		}
		sequence++;
	}
	if (saveLen != (int)ssendHeader.total_size) {
		cbd_err("ERR! wrong size\n");
		ret = -EFAULT;
		goto exit;
	}

	fsync(dump_fd);

	/* Send DUMP Done */
	req = MSG(MSG_AP2CP, MSG_UPLOAD, MSG_NONE_TOC, MSG_DONE);
	exp = MSG(MSG_CP2AP, MSG_UPLOAD, MSG_NONE_TOC, MSG_DONE);

	ret = std_udl_req_resp_sit(dev_fd, req, exp);
	if (ret < 0) {
		cbd_log("ERR!  Send DONE request and wait response (req:0x%X exp:0x%X)\n",
			req, exp);
		goto exit;
	}

	ret = 0;
exit:
	free(pRawdata);
#endif

	return ret;
}

int std_dump_receive_cp_dump(struct std_dump_args *args)
{
	int ret;
	int dev_fd = args->dev_fd;
	int log_fd = args->log_fd;
	int dump_fd = args->dump_fd;
	struct std_uload_info *ul_info;
	u32 saved = 0;
	u32 step;
	int boot_stage = args->cbd_args->cpn->boot_stage;

	/* Send DUMP START request and wait for the response from CP */
	ret = std_udl_stage_start(dev_fd, STD_UDL_DUMP_STAGE);
	if (ret < 0) {
		cbd_log("ERR! std_udl_stage_start fail\n");
		dprintf(log_fd, "%s: ERR! std_udl_stage_start fail\n", __func__);
		goto exit;
	}

	/* Receive DUMP information and the CP CRASH reason */
	ret = std_ul_recv_info(args);
	if (ret < 0) {
		cbd_log("ERR! std_ul_recv_info fail\n");
		dprintf(log_fd, "%s: ERR! std_ul_recv_info fail\n", __func__);
		goto exit;
	}

	/* Receive DUMP data at every DUMP step */
	ul_info = &args->info;
	for (step = 1; step <= ul_info->num_steps; step++) {
		ret = std_ul_recv_data(args, step);
		if (ret < 0) {
			cbd_log("ERR! std_ul_recv_data fail\n");
			dprintf(log_fd, "%s: ERR! std_ul_recv_data fail\n", __func__);
			goto exit;
		}
		saved += ret;
	}

	/* Verify the size of total DUMP data */
	if (saved != ul_info->dump_size) {
		cbd_log("ERR! saved %d != dump_size %d\n", saved, ul_info->dump_size);
		dprintf(log_fd, "%s: ERR! saved %d != dump_size %d\n",
			__func__, saved, ul_info->dump_size);
		ret = -EFAULT;
		goto exit;
	}

	if (fsync(dump_fd)) {
		cbd_log("ERR! fsync(dump_fd) fail (%s)\n", ERR2STR);
		dprintf(log_fd, "%s: ERR! fsync(dump_fd) fail (%s)\n",
			__func__, ERR2STR);
		ret = errno;
		goto exit;
	}
	cbd_log("DUMP DATA saved\n");

	/* Send DUMP DONE request and wait for the response from CP */
	ret = std_udl_stage_done(dev_fd, STD_UDL_DUMP_STAGE, boot_stage);
	if (ret < 0) {
		cbd_log("ERR! DUMP std_udl_stage_done fail\n");
		dprintf(log_fd, "%s: ERR! DUMP std_udl_stage_done fail\n", __func__);
		goto exit;
	}

	return 0;

exit:
	return ret;
}

int std_dump_trigger_kernel_panic(struct std_dump_args *args)
{
	int ret = -1;
	int dev_fd;
	int log_fd;

	if (!args)
		goto exit;

	/* close files */
	if (args->dump_fd >= 0) {
		fsync(args->dump_fd);
		close(args->dump_fd);
		args->dump_fd = -1;
	}

	if (args->info_fd >= 0) {
		fsync(args->info_fd);
		close(args->info_fd);
		args->info_fd = -1;
	}

	dev_fd = args->dev_fd;
	log_fd = args->log_fd;

	cbd_log("Go to UPLOAD mode\n");
	dprintf(log_fd, "%s: Go to UPLOAD mode\n", __func__);

	ret = ioctl(dev_fd, IOCTL_TRIGGER_KERNEL_PANIC, args->cbd_args->reason);
	if (ret < 0) {
		cbd_log("ERR! IOCTL_TRIGGER_KERNEL_PANIC fail (%s)\n", ERR2STR);
		dprintf(log_fd, "%s: ERR! IOCTL_TRIGGER_KERNEL_PANIC fail (%s)\n",
			__func__, ERR2STR);
		goto exit;
	}

	if (fsync(log_fd)) {
		cbd_log("ERR! fsync(log_fd) fail (%s)\n", ERR2STR);
		dprintf(log_fd, "%s: ERR! fsync(log_fd) fail (%s)\n",
			__func__, ERR2STR);
		return -1;
	}
	return 0;

exit:
	return ret;
}

static int std_check_cp_secure_fail(u32 value)
{
	/*
	 * Only for ModAP model.
	 * CP Secure fail err code.
	 */
	u32 err_code[] = {
		0xFEED02,	/* Exynos3475 CP Boot */
		0xFEED04,	/* Exynos3475 CP Main */
		0xFEED0002,	/* Exynos7580, 8890 (EL3) */
		0x50E00,	/* 0xFEED0002 -> 0x50E00(from J-series) */
	};

	int count = sizeof(err_code) / sizeof(u32);

	while (count--) {
		if (value == err_code[count])
			return 1;
	}

	return 0;
}

int std_security_req(struct std_boot_args *args, u32 mode, u32 p2, u32 p3)
{
	int ret;
	int dev_fd = args->dev_fd;
	struct modem_sec_req msr;

	msr.mode = mode;
	msr.param2 = p2;
	msr.param3 = p3;
	msr.param4 = 0;

	cbd_log("security_req: %x:%x:%x:%x\n", 
		msr.mode, msr.param2, msr.param3, msr.param4);

	cpu_set_t cpu_set;
	sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
	cbd_log("orig cpu_set[0]=0x%08lx\n", cpu_set.__bits[0]);
	CPU_CLR(0, &cpu_set);
	sched_setaffinity(0, sizeof(cpu_set), &cpu_set);
	sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
	cbd_log("new cpu_set[0]=0x%08lx\n", cpu_set.__bits[0]);
	ret = ioctl(dev_fd, IOCTL_REQ_SECURITY, &msr);
	if (ret != 0) {
		cbd_log("ERR! IOCTL_CHECK_SECURITY fail (%d)\n", ret);
		if (std_check_cp_secure_fail(ret) &&
				check_csc_sales_code()) {
			cbd_log("secure err: Invalid Main image\n");
			std_reboot_system("secure");
		} else if (mode != CP_BOOT_RE_INIT) {
			ret = -1;
		}
	}
	CPU_SET(0, &cpu_set);
	sched_setaffinity(0, sizeof(cpu_set), &cpu_set);
	sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
	cbd_log("restore cpu_set[0]=0x%08lx\n", cpu_set.__bits[0]);

	return ret;
}
