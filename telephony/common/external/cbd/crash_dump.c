/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <sys/klog.h>
#include "cbd_header.h"

int get_crash_handling_mode(void)
{
	int ret, tmp;

	tmp = property_get_int32(CRASH_MODE_SYS_PROP, CRASH_MODE_DEFAULT);

	if (tmp < 0 || tmp >= CRASH_MODE_MAX)
		ret = CRASH_MODE_DEFAULT;
	else
		ret = tmp;

	cbd_info("crash_handling_mode=%d, using mode(%d)\n", tmp, ret);

	return ret;
}

static int check_free_space(char *root_dir)
{
#define FREE_SPACE (256)
	int err = 0;
	int fsize_mb = 0;
	int free_blks = 0;
	int blk_size = 0;
	struct statfs stat;

	memset(&stat, 0, sizeof(struct statfs));
	err = statfs(root_dir, &stat);
	if (err < 0) {
		err = -errno;
		cbd_err("statfs(%s) fail (err %d)\n", root_dir, err);
		return err;
	}

	blk_size = (int)stat.f_bsize;
	free_blks = (int)stat.f_bfree;
	fsize_mb = (blk_size >> 10) * (free_blks >> 10);

	if (fsize_mb < FREE_SPACE) {
		cbd_err("lack of space %s: free:%d (%dMB)\n", root_dir, fsize_mb, FREE_SPACE);
		return -ENOSPC;
	}

	cbd_info("enough free space!! : %d MB\n", fsize_mb);
	return 0;
}

int dump_prepare_args(struct modem_args *in_args)
{
	int ret = 0;
	struct modem_args *args = in_args;
	char path[MAX_PATH_LEN];
	char suffix[MAX_SUFFIX_LEN];
	time_t now;
	struct tm result;

	memset(path, 0x0, MAX_PATH_LEN);
	memset(suffix, 0x0, MAX_SUFFIX_LEN);

	if (in_args == NULL) {
		cbd_err("in_args is null\n");
		ret = -1;
		goto exit;
	}

	if (check_free_space(LOG_ROOT_DIR) < 0) {
		cbd_err("check_free_space fail\n");
		ret = -1;
		goto exit;
	}

	time(&now);
	localtime_r(&now, &result);
	strftime(suffix, MAX_SUFFIX_LEN, "%Y%m%d_%H%M%S", &result);

	ret = mkdir(LOG_DIR, 0644);
	if (ret < 0 && ret != -EEXIST) {
		cbd_err("mkdir fail: %s (%s)\n", LOG_DIR, ERR2STR);
	}

	if (args->fds.fd_info <= 0) {
		// 1. info data
		sprintf(path, "%s/info-%s.log", LOG_DIR, suffix);
		args->fds.fd_info = ret = open(path, O_WRONLY | O_CREAT, 0666);
		if (args->fds.fd_info < 0) {
			cbd_err("open fail : info file (%s)\n", ERR2STR);
			goto exit;
		}
		chmod(path, 0644);
	}

	if (args->fds.fd_dump <= 0) {
		// 2. dump data
		sprintf(path, "%s/dump-%s.log", LOG_DIR, suffix);
		args->fds.fd_dump = ret = open(path, O_WRONLY | O_CREAT, 0666);
		if (args->fds.fd_dump < 0) {
			cbd_err("open fail : dump file (%s)\n", ERR2STR);
			goto exit;
		}
		chmod(path, 0644);
	}
exit:
	return ret;
}

int modem_monitoring(struct modem_args *in_args)
{
	struct modem_args *args = in_args;
	int status = 0;
	int ret = 0;
	int crash_handling_mode = 0;

	while (1) {
		crash_handling_mode = get_crash_handling_mode();

		if (crash_handling_mode == CRASH_MODE_SKIP_RESET)
			/* just wait here to skip cp reset - POLLNVAL not happened */
			ret = wait_event(args->fds.fd_dev, POLLNVAL, MONITORING_TIMEOUT);
		else
			/* wait POLLHUP event */
			ret = wait_event(args->fds.fd_dev, POLLHUP, MONITORING_TIMEOUT);

		if (ret < 0) {
			cbd_err("wait event fail!!\n");
			goto exit;
		}

		status = modem_status(args->fds.fd_boot);

		crash_handling_mode = get_crash_handling_mode();

		/* silent reset only mode, skip dump */
		if (crash_handling_mode == CRASH_MODE_SILENT_RESET) {
			status = STATE_CRASH_RESET;
		}

		switch (status) {
		case STATE_CRASH_RESET:
		case STATE_CRASH_EXIT:
		case STATE_CRASH_WATCHDOG:
			cbd_info("change modem status(%d)\n", status);
			return status;
			break;
		default:
			cbd_info("modem status others(%d)\n", status);
			break;
		}
	}

exit:
	return (-1);
}

int recv_raw_data(struct modem_args *in_args, void *buffer, u32 size)
{
	int ret = -1;
	int readLen = 0;
	struct modem_args *args = in_args;

	if (args == NULL) {
		cbd_err("in_args is null\n");
		goto exit;
	}

	// 1. wait POLLIN event
	ret = wait_event(args->fds.fd_dev, POLLIN, TIMEOUT);
	if (ret < 0) {
		cbd_err("wait_event fail\n");
		goto exit;
	}

	/* 2. received raw data */
	readLen = ret = read(args->fds.fd_dev, buffer, size);
	if (ret < 0) {
		cbd_err("read fail (%s)\n", ERR2STR);
		goto exit;
	}
	ret = readLen;

exit:
	return ret;
}

int recv_data(struct modem_args *in_args, struct exynos_dump_info *pinfo)
{
	int ret = 0, saveLen = 0;
	int recv_length = 0, prev_length = 0;
	struct modem_args *args = in_args;
	struct exynos_send_header sendHeader;
	struct exynos_recv_header recvHeader;
	int readLen = 0, writeLen = 0;
	u8 *pRawdata = NULL;

	u16 exp = MSG(MSG_CP2AP, MSG_UPLOAD, MSG_NONE_TOC, MSG_DATA);

	cbd_info("+++recv_data\n");

	if (NULL == in_args || NULL == pinfo) {
		cbd_err("Parameter is null\n");
		goto exit;
	}

	memset(&sendHeader, 0x0, sizeof(struct exynos_send_header));
	memset(&recvHeader, 0x0, sizeof(struct exynos_recv_header));

	sendHeader.cmd = MSG(MSG_AP2CP, MSG_UPLOAD, MSG_NONE_TOC, MSG_DATA);
	sendHeader.length = sizeof(struct exynos_data_info);
	sendHeader.sendInfo.total_size = pinfo->dump_size;
	sendHeader.sendInfo.data_offset = 0;

	while (1) {

#ifdef USE_SPI_BOOT_LINK
		// Send command for receiving dump
		ret = write(args->fds.fd_dev, &sendHeader, sizeof(struct exynos_send_header));
		if (ret < 0) {
			cbd_err("write fail (%s)\n", ERR2STR);
			goto exit;
		}
#endif
		// Receiving dump header data
		ret = recv_raw_data(args, &recvHeader, sizeof(struct exynos_recv_header));
		if (ret < 0) {
			cbd_err("recv Header fail (%s)\n", ERR2STR);
			goto exit;
		}

		if (recvHeader.cmd != exp) {
			cbd_err("G:0x%X != E:0x%X\n", recvHeader.cmd, exp);
			goto exit;
		}

		recv_length = (recvHeader.length - sizeof(struct exynos_data_info)); // real dump data length
		if (recv_length <= 0) {
			cbd_info("recv_length is %d\n", recv_length);
			goto exit;
		}

		// manage raw data buffer
		if (pRawdata != NULL) {
			if (prev_length < recv_length) {
				free(pRawdata);
				pRawdata = NULL;
			} else {
				memset(pRawdata, 0x0, prev_length);
			}
		}

		if (pRawdata == NULL) {
			pRawdata = (u8 *)calloc(recv_length, sizeof(u8));
			if (pRawdata == NULL) {
				cbd_err("malloc fail (pRawdata is null)\n");
				goto exit;
			}
			prev_length = recv_length;
		}

		// 3. Receiving dump raw data
		readLen = ret = recv_raw_data(args, pRawdata, recv_length);
		if (ret < 0) {
			cbd_err("recv data fail (%s)\n", ERR2STR);
			goto exit;
		}

		sendHeader.sendInfo.data_offset += readLen;

		writeLen = ret = write(args->fds.fd_dump, pRawdata, readLen);
		if (ret < 0) {
			cbd_err("fail to write dump data (%s)\n", ERR2STR);
		}

		saveLen += writeLen;

		if (sendHeader.sendInfo.data_offset >= sendHeader.sendInfo.total_size) {
			if (sendHeader.sendInfo.data_offset > sendHeader.sendInfo.total_size)
				cbd_info("Warning::Receiving data is bigger than total size\n");

			cbd_info("Complete Crash Dump!!!\n");
			break;
		}
	}

	if (saveLen != (int)sendHeader.sendInfo.total_size) {
		cbd_err("T:%d != S:%d\n", sendHeader.sendInfo.total_size, saveLen);
		ret = -EFAULT;
		goto exit;
	}

	fsync(args->fds.fd_dump);
	ret = saveLen;

exit:
	free(pRawdata);
	return ret;
}

int send_dump_start(struct modem_args *in_args, struct exynos_dump_info *pInfo)
{
	int ret = 0;
	struct exynos_header sendHeader;
	struct exynos_header recvHeader;
	struct modem_args *args = in_args;
	struct crash_reason reason;
	char string_crash_by_AP[] = ": Crash by AP - ";
	char string_crash_by_RIL[] = ": Crash by RIL - ";
	char *buff;

	u16 exp = MSG(MSG_CP2AP, MSG_UPLOAD, MSG_NONE_TOC, MSG_START);

	if (in_args == NULL || pInfo == NULL) {
		cbd_err("args is null\n");
		ret = -1;
		goto exit;
	}

	cbd_info("+++send_dump_start\n");
	sendHeader.cmd = MSG(MSG_AP2CP, MSG_UPLOAD, MSG_NONE_TOC, MSG_START);
	sendHeader.length = 0;

	// 1. send cmd
	ret = write(args->fds.fd_dev, &sendHeader, sizeof(struct exynos_header));
	if (ret < 0) {
		cbd_err("send cmd (%s)\n", ERR2STR);
		goto exit;
	}

	ret = wait_event(args->fds.fd_dev, POLLIN, TIMEOUT);
	if (ret < 0) {
		cbd_err("wait event(%s)\n", ERR2STR);
		goto exit;
	}

	ret = read(args->fds.fd_dev, &recvHeader, sizeof(struct exynos_header));
	if (ret < 0) {
		cbd_err("read (%s)\n", ERR2STR);
		goto exit;
	}

	if (recvHeader.cmd != exp) {
		cbd_err("G:0x%X != E:0x%X\n", recvHeader.cmd, exp);
		goto exit;
	}

	ret = read(args->fds.fd_dev, pInfo, recvHeader.length);
	if (ret < 0) {
		cbd_err("read (%s)\n", ERR2STR);
		goto exit;
	}

	/* Get crash reason for crash_by_AP or crash_by_RIL */
	ret = ioctl(args->fds.fd_dev, IOCTL_MODEM_CRASH_REASON, &reason);
	if (ret < 0) {
		cbd_err("ERR! IOCTL_MODEM_CRASH_REASON fail\n");
	} else {
		if (reason.owner == MEM_CRASH_REASON_AP) {
			memset(pInfo->reason, 0, CRASH_REASON_LEN);
			strcpy((char *)pInfo->reason, string_crash_by_AP);
			buff = (char *)pInfo->reason + strlen(string_crash_by_AP);
			strcpy(buff, reason.string);
		} else if (reason.owner == MEM_CRASH_REASON_RIL) {
			memset(pInfo->reason, 0, CRASH_REASON_LEN);
			strcpy((char *)pInfo->reason, string_crash_by_RIL);
			buff = (char *)pInfo->reason + strlen(string_crash_by_RIL);
			strcpy(buff, reason.string);
		}
	}

	ret = write(args->fds.fd_info, pInfo->reason, CRASH_REASON_LEN);
	if (ret < 0) {
		cbd_err("write info file (%s)\n", ERR2STR);
		goto exit;
	}

	cbd_info("dump info - total size : %d, length : %d, reason : %s\n"
				, pInfo->dump_size, pInfo->reason_len, pInfo->reason);

	if (fsync(args->fds.fd_info) < 0)
		cbd_err("fsync info file (%s)\n", ERR2STR);

exit:
	if (close(args->fds.fd_info) < 0)
		cbd_err("close info file (%s)\n", ERR2STR);
	else
		args->fds.fd_info = 0;
	return ret;
}

int pre_dump_process(struct modem_args *args)
{
	int ret;

	// 0. dump shmem, vss, acpm region
	if (args->ops.full_dump_from_kernel) {
		ret = args->ops.full_dump_from_kernel(args, "shmem");
		if (ret < 0) {
			cbd_err("shmem_full_dump_from_kernel fail\n");
			goto exit;
		}
		ret = args->ops.full_dump_from_kernel(args, "vss");
		if (ret < 0) {
			cbd_err("vss_full_dump_from_kernel fail\n");
			goto exit;
		}
		ret = args->ops.full_dump_from_kernel(args, "acpm");
		if (ret < 0) {
			cbd_err("acpm_full_dump_from_kernel fail\n");
			goto exit;
		}
	}

	/* 1.save_kernel_log */
	save_kernel_log();

exit:
	return 0;
}

int dump_process(struct modem_args *in_args, int crash_handling)
{
	int ret = 0;
	struct modem_args *args = in_args;
	struct exynos_dump_info dumpInfo;
	char prop_buf[PROPERTY_VALUE_MAX] = {0, };

	memset(&dumpInfo, 0x0, sizeof(struct exynos_dump_info));

	// 0. prepare
	ret = dump_prepare_args(args);
	if (ret < 0) {
		cbd_err("dump prepare args\n");
		goto exit;
	}

	property_get("ro.debuggable", prop_buf, "0");
	if (prop_buf[0] == '1') { /* if platform is build as user bin, it will be ignored */
		cbd_err("am start ALERT_MESSAGE (ro.debuggalbe == 1)\n");
		//system("am start -a com.samsung.slsi.action.ALERT_MESSAGE -c android.intent.category.CP_CRASH -e Alert \"Doing modem dumping now. Please don't touch anything for 10 mins...\" -n com.samsung.slsi.sysdebugmode/com.samsung.slsi.logdump.ReceiverActivity --activity-clear-top");
	}

	// 1. dump ready
	ret = send_cmd(args, MSG_READY, MSG_NONE_TOC, MSG_DUMP);
	if (ret < 0) {
		cbd_err("dump ready\n");
		goto exit;
	}

	// 2. start
	ret = send_dump_start(args, &dumpInfo);
	if (ret < 0) {
		cbd_err("start dump\n");
		goto exit;
	}

	// 3. receive
	ret = recv_data(args, &dumpInfo);
	if (ret < 0) {
		cbd_err("fail to receive dump\n");
		goto exit;
	}

	// 4. done
	ret = send_cmd(args, MSG_UPLOAD, MSG_NONE_TOC, MSG_DONE);
	if (ret < 0) {
		cbd_err("done dump\n");
		goto exit;
	}

	sync();

	if (crash_handling != CRASH_MODE_DUMP_PANIC)
		goto exit;

	ret = modem_cp_upload(args->fds.fd_dev, (const char *)dumpInfo.reason);
	if (ret < 0) {
		cbd_err("modem cp upload fail\n");
		goto exit;
	}

exit:
	if (args->fds.fd_info) {
		if (close(args->fds.fd_info) < 0)
			cbd_err("closing info (%s)\n", ERR2STR);
		else
			args->fds.fd_info = 0;
	}

	if (args->fds.fd_dump) {
		if (close(args->fds.fd_dump) < 0)
			cbd_err("closing fd_dump (%s)\n", ERR2STR);
		else
			args->fds.fd_dump = 0;
	}

	return ret;
}

#define KLOG_BUF_SHIFT 19
#define KLOG_BUF_LEN (1 << KLOG_BUF_SHIFT)
static char klog_buf[KLOG_BUF_LEN + 1];
int save_kernel_log(void)
{
	time_t now;
	struct tm result;
	char path[MAX_PATH_LEN];
	char suffix[MAX_SUFFIX_LEN];
	int fd;
	ssize_t ret;
	int rest;
	char *p = klog_buf;

	cbd_info("save_kernel_log+++\n");

	time(&now);
	localtime_r(&now, &result);
	strftime(suffix, MAX_SUFFIX_LEN, "%Y%m%d-%H%M", &result);
	sprintf(path, "%s/dmesg_%s.log", LOG_DIR, suffix);

	fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0664);
	if (fd < 0) {
		cbd_err("Fail to open file(%s)\n", path);
		return fd;
	}
	fchmod(fd, 0664);

	ret = lseek(fd, 0, SEEK_END);
	if (ret < 0) {
		cbd_err("Fail to lseek (%s)\n", path);
		close(fd);
		return -EINVAL;
	}

	rest = klogctl(KLOG_READ_ALL, p, KLOG_BUF_LEN);
	if (rest < 0) {
		cbd_err("Fail to klogctl\n");
		close(fd);
		return -EINVAL;
	}
	klog_buf[rest] = '\0';

	while ((ret = write(fd, p, rest))) {
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			cbd_err("Fail to write\n");
			close(fd);
			return -EINVAL;
		}
		p += ret;
		rest -= ret;
	}

	cbd_info("Save kernel log : %s\n", path);

	close(fd);
	return 0;
}