#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/select.h>

#include <cutils/properties.h>

#include "boot.h"
#include "std_boot.h"
#include "util.h"
#include "shannon.h"

static struct shannon_boot_args shannon_boot_arguments;

static void build_std_dload_control(struct std_boot_args *std_args,
				    struct shannon_boot_args *args)
{
	struct std_dload_control *dl_ctrl = std_args->dl_ctrl;
	struct std_toc_element *toc = std_args->toc;
	int bin_fd = args->bin_fd;
	int nv_fd = args->nv_fd;
	int nv_prot_fd = args->nv_prot_fd;
	int i;
	struct std_dload_control *tmp;
	struct modem_comp *cpn = std_args->cbd_args->cpn;
	int TOC_STAGE = cpn->toc_stage;
	int BOOT_STAGE = cpn->boot_stage;

	memset(dl_ctrl, 0, sizeof(struct std_dload_control) * MAX_DLOAD_STAGE);

	for (i = 0; i < (int)std_args->num_stages; i++) {
		/* End of TOC */
		if (strcmp(toc[i].name, "OFFSET") == 0)
			break;

		/* TOC */
		if (strcmp(toc[i].name, "TOC") == 0) {
			dl_ctrl[TOC_STAGE].stage = TOC_STAGE;
			dl_ctrl[TOC_STAGE].start = 1;
			dl_ctrl[TOC_STAGE].download = 1;
			dl_ctrl[TOC_STAGE].validate = 0;
			dl_ctrl[TOC_STAGE].finish = 1;
			dl_ctrl[TOC_STAGE].b_fd = bin_fd;
			dl_ctrl[TOC_STAGE].b_offset = toc[i].b_offset;
			/* CP receive TOC for only size 0x100 */
			dl_ctrl[TOC_STAGE].b_size = toc[IMG_TOC].size;
			dl_ctrl[TOC_STAGE].crc = toc[i].crc;
			goto next;
		}

		/*
		 * During CP download with S5100 PCIE interface,
		 * AP should download CP boot binary via SPI and
		 * the other binaries via PCIE.
		 * So CP boot should be separated with others
		 */
		if (strcmp(toc[i].name, "BOOT") == 0) {
			dl_ctrl[BOOT_STAGE].stage = BOOT_STAGE;
			dl_ctrl[BOOT_STAGE].start = 0;
			dl_ctrl[BOOT_STAGE].download = 0;
			dl_ctrl[BOOT_STAGE].validate = 1;
			dl_ctrl[BOOT_STAGE].finish = 1;
			dl_ctrl[BOOT_STAGE].b_fd = bin_fd;
			dl_ctrl[BOOT_STAGE].b_offset = toc[IMG_BOOT].b_offset;
			dl_ctrl[BOOT_STAGE].b_size = toc[IMG_BOOT].size;
			dl_ctrl[TOC_STAGE].crc = toc[i].crc;
			goto next;
		}

		/* MAIN */
		if (strcmp(toc[i].name, "MAIN") == 0)
			dl_ctrl[i].validate = 1;
		else
			dl_ctrl[i].validate = 0;

		if (strcmp(toc[i].name, "NV") == 0)
			dl_ctrl[i].b_fd = nv_fd;
		else if (strcmp(toc[i].name, "NV_NORM") == 0)
			dl_ctrl[i].b_fd = nv_fd;
		else if (strcmp(toc[i].name, "NV_PROT") == 0)
			dl_ctrl[i].b_fd = nv_prot_fd;
		else
			dl_ctrl[i].b_fd = bin_fd;

		dl_ctrl[i].stage = i;
		dl_ctrl[i].b_offset = toc[i].b_offset;
		dl_ctrl[i].b_size = toc[i].size;
		dl_ctrl[i].crc = toc[i].crc;
		dl_ctrl[i].start = 1;
		dl_ctrl[i].download = 1;
		dl_ctrl[i].finish = 1;

next:
		tmp = &dl_ctrl[i];
		cbd_log("stage=%u, name:%s b_off=0x%08x, m_offset=0x%08x b_size=0x%08x\n",
			tmp->stage, toc[i].name, tmp->b_offset, tmp->m_offset,
			tmp->b_size);
	}

	dl_ctrl[i].stage = STD_UDL_FIN_STAGE;
	dl_ctrl[i].start = 1;
}

static struct shannon_boot_args *prepare_boot_args(struct boot_args *cbd_args,
						   enum cp_boot_mode mode)
{
	int ret;
	int bin_fd = -1;
	int nv_fd = -1;
	int nv_prot_fd = -1;
	struct modem_comp *cpn = cbd_args->cpn;
	struct shannon_boot_args *args = &shannon_boot_arguments;
	struct std_boot_args *std_args;
	struct std_toc_element *toc;
	size_t toc_size;
	int max_stage;
	u32 nv_size = 0;
	int i;

	memset(args, 0, sizeof(struct shannon_boot_args));

	/*
	** Prepare BOOT arguments which are common to all modems
	*/
	std_args = std_boot_prepare_args(cbd_args, cpn->num_stages);
	if (!std_args) {
		cbd_log("ERR! std_boot_prepare_args fail\n");
		goto exit;
	}

	/*
	** Open CP binary file
	*/
	bin_fd = open(cpn->path_bin, O_RDONLY);
	if(bin_fd < 0) {
		cbd_log("ERR! BIN(%s) open fail (%s)\n", cpn->path_bin, ERR2STR);
		goto exit;
	}
	cbd_log("BIN(%s) opened (fd %d)\n", cpn->path_bin, bin_fd);

	/*
	** Load and check TOC
	*/
	toc = std_args->toc;
	toc_size = sizeof(struct std_toc_element) * MAX_TOC_INDEX;

	ret = read(bin_fd, toc, toc_size);
	if (ret < 0) {
		cbd_log("ERR! TOC read fail (%s)\n", ERR2STR);
		goto exit;
	}

	if (strcmp(toc[IMG_TOC].name, "TOC")) {
		cbd_log("ERR! invalid TOC: No TOC\n");
		goto exit;
	}

	if (toc[IMG_TOC].toc_count > MAX_TOC_INDEX) {
		cbd_log("ERR! invalid TOC: Total TOC count is %d\n",
			toc[IMG_TOC].toc_count);
		goto exit;
	}

	if (toc[IMG_TOC].toc_count == 1)
		max_stage = cpn->num_stages;
	else
		max_stage = toc[IMG_TOC].toc_count;

	std_args->num_stages = max_stage;
	cbd_log("max_stage: %d\n", max_stage);

	for (i = 0; i < max_stage; i++) {
		if (strcmp(toc[i].name, "NV") == 0)
			nv_size = toc[i].size;
		else if (strcmp(toc[i].name, "NV_NORM") == 0)
			nv_size = toc[i].size;

		cbd_log("TOC[%d].name = %s, b_off=0x%08x, m_off=0x%08x, size=0x%08x crc=0x%08x\n",
			i, toc[i].name, toc[i].b_offset, toc[i].m_offset,
			toc[i].size, toc[i].crc);
	}

	if ((mode != CP_BOOT_MODE_DUMP) && (nv_size == 0) && cpn->path_nv[0]) {
		cbd_log("ERR! invalid TOC : There is no NV\n");
		goto exit;
	}

	/*
	** Open NV data file
	*/
	if (mode == CP_BOOT_MODE_NORMAL) {
		/* Open NV data file */
		nv_fd = open(cpn->path_nv, O_RDONLY);
		if (nv_fd < 0) {
			if (errno != ENOENT) {
				cbd_log("ERR! NV(%s) open fail (%s)\n",
					cpn->path_nv, ERR2STR);
				goto exit;
			}

			cbd_log("ERR! no NV(%s) file\n", cpn->path_nv);

			ret = create_empty_nv(cpn->path_nv, nv_size);
			if (ret < 0) {
				cbd_log("ERR! create_empty_nv(%s, %d) fail\n",
					cpn->path_nv, toc[IMG_NV].size);
				goto exit;
			}

			nv_fd = open(cpn->path_nv, O_RDONLY);
			if (nv_fd < 0) {
				cbd_log("ERR! NV(%s) open fail (%s)\n",
					cpn->path_nv, ERR2STR);
				goto exit;
			}
		}
		cbd_log("NV(%s) opened (fd %d)\n", cpn->path_nv, nv_fd);
	}

#ifdef CONFIG_PROTOCOL_SIT
	if (mode == CP_BOOT_MODE_NORMAL) {
		/* Open NV data file */
		nv_prot_fd = open(cpn->path_nv_prot, O_RDONLY);
		if (nv_prot_fd < 0) {
			if (errno != ENOENT) {
				cbd_log("ERR! NV(%s) open fail (%s)\n",
					cpn->path_nv_prot, ERR2STR);
				goto exit;
			}

			cbd_log("ERR! no NV(%s) file\n", cpn->path_nv_prot);

			ret = create_empty_nv(cpn->path_nv_prot, nv_size);
			if (ret < 0) {
				cbd_log("ERR! create_empty_nv(%s, %d) fail\n",
					cpn->path_nv_prot, toc[IMG_NV].size);
				goto exit;
			}

			nv_prot_fd = open(cpn->path_nv_prot, O_RDONLY);
			if (nv_prot_fd < 0) {
				cbd_log("ERR! NV(%s) open fail (%s)\n",
					cpn->path_nv_prot, ERR2STR);
				goto exit;
			}
		}
		cbd_log("NV(%s) opened (fd %d)\n", cpn->path_nv_prot, nv_prot_fd);
	}
#endif

	/*
	** Assign SHANNON BOOT arguments
	*/
	args->cbd_args = cbd_args;
	args->std_args = std_args;
	args->bin_fd = bin_fd;
	args->nv_fd = nv_fd;
	args->nv_prot_fd = nv_prot_fd;

	/*
	** Set standard DLOAD control parameters with SHANNON BOOT arguments
	*/
	build_std_dload_control(std_args, args);

	return args;

exit:
	if (std_args)
		std_boot_close_args(std_args);

	if (bin_fd >= 0)
		close(bin_fd);

	if (nv_fd >= 0)
		close(nv_fd);

	return NULL;
}

static void close_boot_args(struct shannon_boot_args *args)
{
	if (args) {
		if (args->std_args)
			std_boot_close_args(args->std_args);

		if (args->bin_fd >= 0)
			close(args->bin_fd);

		if (args->nv_fd >= 0)
			close(args->nv_fd);

		memset(args, 0, sizeof(struct shannon_boot_args));
	}
}

static int get_log_dump(struct std_dump_args *args, char *name, enum cp_log_dump_index idx)
{
	int ret;
	int dev_fd = args->dev_fd;
	int log_fd = args->log_fd;
	unsigned long copied = 0;
	char buf[PAGE_SIZE];
	int dump_fd = -1;
	char path[MAX_PATH_LEN];
	char suffix[MAX_SUFFIX_LEN];
	time_t now;
	struct tm result;
	struct cp_log_dump log_dump;

	/* Get size and trigger log dump */
	memset(&log_dump, 0, sizeof(log_dump));
	strncpy(log_dump.name, name, sizeof(log_dump.name));
	log_dump.idx = idx;
	cbd_log("name:%s idx:%u\n", log_dump.name, log_dump.idx);
	ret = ioctl(dev_fd, IOCTL_GET_LOG_DUMP, &log_dump);
	if (ret < 0) {
		cbd_log("ERR! ioctl fail (%s)\n", ERR2STR);
		dprintf(log_fd, "ERR! ioctl fail (%s)\n", ERR2STR);
		return -EINVAL;
	}
	cbd_log("size:%u\n", log_dump.size);
	dprintf(log_fd, "%s: size:%u\n", __func__, log_dump.size);

	/* Open a log dump file */
	time(&now);
	localtime_r(&now, &result);
	strftime(suffix, MAX_SUFFIX_LEN, "%Y%m%d-%H%M", &result);
	sprintf(path, "%s/cpcrash_%s_dump_%s_%s.log", get_log_dir(), name, args->cbd_args->cpn->rat, suffix);
	dump_fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (dump_fd < 0) {
		cbd_log("ERR! %s open fail (%s)\n", path, ERR2STR);
		dprintf(log_fd, "%s: ERR! %s open fail (%s)\n", __func__, path, ERR2STR);
		return -EINVAL;
	}
	cbd_log("%s opened (fd %d)\n", path, dump_fd);
	dprintf(log_fd, "%s: %s opened (fd %d)\n", __func__, path, dump_fd);

	/* Read & save log dump */
	while (copied < log_dump.size) {
		ret = std_ul_rx_frame(args, buf, sizeof(buf));
		if (ret < 0) {
			cbd_log("ERR! log_dump std_ul_rx_frame fail (ret %d)\n", ret);
			dprintf(log_fd, "%s: ERR! shmem, std_ul_rx_frame fail (ret %d)\n", __func__, ret);
			goto exit;
		}

		/* not verified */
		copied += ret;

		ret = write(dump_fd, buf, ret);
		if (ret < 0) {
			cbd_log("ERR! log_dump write fail (%s)\n", ERR2STR);
			dprintf(log_fd, "%s: ERR! log_dump, write fail (%s)\n", __func__, ERR2STR);
			goto exit;
		}
	}

	cbd_log("Complete! (%lu bytes)\n", copied);
	dprintf(log_fd, "%s: %s Complete! (%lu bytes)\n", __func__, path, copied);
exit:
	if (dump_fd >= 0)
		close(dump_fd);

	return ret;
}

static int shannon_normal_boot(struct shannon_boot_args *args)
{
	int ret;
	struct std_boot_args *std_args = args->std_args;

	cbd_log("Power on CP\n");
	ret = std_boot_power_on(std_args);
	if (ret < 0) {
		cbd_log("ERR! std_boot_power_on fail\n");
		goto exit;
	}

	cbd_log("Load CP bootloader\n");
	ret = std_boot_load_cp_bootloader(std_args, std_args->cbd_args->cpn->boot_stage);
	if (ret < 0) {
		cbd_log("ERR! std_boot_load_cp_image fail\n");
		goto exit;
	}

	cbd_log("Start CP bootloader\n");
	ret = std_boot_start_cp_bootloader(std_args, CP_BOOT_MODE_NORMAL);
	if (ret < 0) {
		cbd_log("ERR! std_boot_start_cp_bootloader fail\n");
		goto exit;
	}

	cbd_log("Load CP images\n");
	ret = std_boot_load_cp_images(std_args);
	if (ret < 0) {
		cbd_log("ERR! std_boot_dload fail\n");
		goto exit;
	}

	cbd_log("Complete normal bootup\n");
	ret = std_boot_complete_normal_bootup(std_args);
	if (ret < 0) {
		cbd_log("ERR! std_boot_complete_normal_bootup fail\n");
		goto exit;
	}

exit:
	return ret;
}

static int shannon_dump_boot(struct shannon_boot_args *args)
{
	int ret;
	struct std_boot_args *std_args = args->std_args;

	cbd_log("Power reset CP for CP_BOOT_MODE_DUMP\n");
	ret = std_boot_power_reset(std_args, CP_BOOT_MODE_DUMP);
	if (ret < 0) {
		cbd_log("ERR! std_boot_power_reset fail\n");
		goto exit;
	}

	cbd_log("Load CP bootloader\n");
	ret = std_boot_load_cp_bootloader(std_args, std_args->cbd_args->cpn->boot_stage);
	if (ret < 0) {
		cbd_log("ERR! std_boot_load_cp_bootloader fail\n");
		goto exit;
	}

	cbd_log("Start CP bootloader for crash dump\n");
	ret = std_boot_start_cp_bootloader(std_args, CP_BOOT_MODE_DUMP);
	if (ret < 0) {
		cbd_log("ERR! std_boot_start_cp_bootloader fail\n");
		goto exit;
	}

exit:
	return ret;
}

int start_shannon5100_boot(struct boot_args *cbd_args)
{
	int ret = 0;
	int spin = 50;
	char prop_buf[PROPERTY_VALUE_MAX] = {0, };
	struct shannon_boot_args *dl_args = NULL;

	cbd_log("CP boot device = %s\n", cbd_args->cpn->node_boot);

	cbd_log("CP binary file = %s\n", cbd_args->cpn->path_bin);

	cbd_log("CP NV file = %s\n", cbd_args->cpn->path_nv);

	switch (cbd_args->lnk_boot) {
	case LINKDEV_SPI:
		cbd_log("BOOT link SPI\n");
		break;

	case LINKDEV_SHMEM:
		cbd_log("BOOT link SHMEM\n");
		break;

	default:
		cbd_log("ERR! BOOT link# %d not supported\n", cbd_args->lnk_boot);
		ret = -ENODEV;
		goto exit;
	}

	switch (cbd_args->lnk_main) {
	case LINKDEV_C2C:
		cbd_log("MAIN link C2C\n");
		break;

	case LINKDEV_SHMEM:
		cbd_log("MAIN link SHMEM\n");
		break;

	case LINKDEV_LLI:
		cbd_log("MAIN link MIPI-LLI\n");
		break;

	case LINKDEV_PCIE:
		cbd_log("MAIN link PCIE\n");
		break;

	default:
		cbd_log("ERR! MAIN link# %d not supported\n", cbd_args->lnk_main);
		ret = -ENODEV;
		goto exit;
	}

	/* Wait for completion of RILD's NV validity check */
	while (spin--) {
		property_get(VPROP_RFS_CHECKDONE, prop_buf, "0");
		if (prop_buf[0] == '1')
			break;
		usleep(100000);
	}
	cbd_log("NV validation %s\n", spin < 0 ? "TIMEOUT" : "done");

	/*
	** Start CP BOOT
	*/

	/* Prepare BOOT arguments which are specific to SHANNON */
	dl_args = prepare_boot_args(cbd_args, CP_BOOT_MODE_NORMAL);
	if (!dl_args) {
		cbd_log("ERR! prepare_boot_args fail\n");
		ret = -EFAULT;
		goto exit;
	}

	ret = shannon_normal_boot(dl_args);
	if (ret < 0) {
		cbd_log("ERR! shannon_normal_boot fail\n");
		goto exit;
	}

exit:
	if (dl_args)
		close_boot_args(dl_args);

	return ret;
}

int start_shannon5100_dump(struct boot_args *cbd_args)
{
	int ret;
	int log_fd = -1;
	struct std_dump_args *ul_args;
	struct shannon_boot_args *dl_args = NULL;
	char reason[MAX_PREFIX_LEN];

	cbd_log("Prepare CP crash dump\n");
	ul_args = std_dump_prepare_args(cbd_args);
	if (!ul_args) {
		cbd_log("ERR! std_dump_prepare_args fail\n");
		ret = -EFAULT;
		goto exit;
	}
	log_fd = ul_args->log_fd;

	dl_args = prepare_boot_args(cbd_args, CP_BOOT_MODE_DUMP);
	if (!dl_args) {
		cbd_log("ERR! prepare_boot_args fail\n");
		ret = -EFAULT;
		goto exit;
	}

	cbd_log("Get log dump\n");
	get_log_dump(ul_args, "shmem", LOG_IDX_SHMEM);
	get_log_dump(ul_args, "databuf", LOG_IDX_DATABUF);

	cbd_log("Prepare CP crash dump\n");
	ret = shannon_dump_boot(dl_args);
	if (ret < 0) {
		cbd_log("ERR! shannon_dump_boot fail\n");
		dprintf(log_fd, "%s: ERR! shannon_dump_boot fail\n", __func__);
		goto exit;
	}

	cbd_log("Receive CP crash dump\n");
	switch(cbd_args->type) {
		case SEC_S5100_SIT:
			ret = std_dump_receive_cp_dump_sit(ul_args);
			break;
		default:
			ret = std_dump_receive_cp_dump(ul_args);
			break;

	}
	if (ret < 0) {
		cbd_log("ERR! std_dump_receive_cp_dump fail\n");
		dprintf(log_fd, "%s: ERR! std_dump_receive_cp_dump fail\n", __func__);
		goto exit;
	}

	cbd_log("Save kernel log\n");
	snprintf(reason, MAX_PREFIX_LEN, "dmesg");
	save_logs(LOGB_DMESG, reason);

	sync();

#ifdef CONFIG_PROTOCOL_SIT
	if (cbd_args->debug_level == DBG_AUTO) {
		cbd_log("No trigger kernel panic\n");
		ret = 0;
		goto exit;
	}
#endif

	cbd_log("Trigger kernel panic\n");
	ret = std_dump_trigger_kernel_panic(ul_args);
	if (ret < 0) {
		cbd_log("ERR! std_dump_trigger_kernel_panic fail\n");
		dprintf(log_fd, "%s: ERR! std_dump_trigger_kernel_panic fail\n", __func__);
		goto exit;
	}

exit:
	if (ret < 0) {
		snprintf(reason, MAX_PREFIX_LEN, "%s_dump_fail", cbd_args->cpn->rat);
		save_logs(LOGB_DMESG, reason);
	}

	if (ul_args)
		std_dump_close_args(ul_args);

	if (dl_args)
		close_boot_args(dl_args);

	return ret;
}

int shutdown_shannon5100_modem(struct boot_args *cbd_args)
{
	int ret = 0;
	char *node_boot = cbd_args->cpn->node_boot;
	int fd;

	fd = open(node_boot, O_RDWR | O_NDELAY);
	if (fd < 0) {
		cbd_log("%s open fail\n", node_boot);
		ret = -errno;
		goto exit;
	}

	ioctl(fd, IOCTL_POWER_OFF, NULL);

	close(fd);

exit:
	return ret;
}

int upload_shannon5100_modem(struct boot_args *cbd_args)
{
	int ret = -1;
	char *node_boot = cbd_args->cpn->node_boot;
	int fd;

	fd = open(node_boot, O_RDWR | O_NDELAY);
	if (fd < 0) {
		cbd_log("%s open fail\n", node_boot);
		ret = -errno;
		goto exit;
	}

	cbd_log("Go to UPLOAD mode\n");

	ret = ioctl(fd, IOCTL_TRIGGER_KERNEL_PANIC, cbd_args->reason);
	if (ret < 0) {
		cbd_log("ERR! IOCTL_TRIGGER_KERNEL_PANIC fail (%s)\n", ERR2STR);
		goto exit;
	}

	return 0;

exit:
	return ret;
}

