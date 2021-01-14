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

#include "util_srinfo.h"

static struct shannon_boot_args shannon_boot_arguments;

static void boot_wake_lock(int lock)
{
	char *path = lock ? "/sys/power/wake_lock" : "/sys/power/wake_unlock";
	char *name = "ss310";
	int fd, ret;

	fd = open(path,  O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd < 0) {
		cbd_log("user wake_%s open fail(%d)\n", lock ? "lock" : "unlock", fd);
		return;
	}
	ret = write(fd, name, strlen(name));
	if (ret < 0) {
		cbd_log("write fail - %s (%d)\n", name, ret);
		goto exit;

	}
	cbd_log("%s/%s\n", path, name);
exit:
	if (fd >= 0)
		close(fd);
	return;
}

static void build_std_dload_control(struct std_boot_args *std_args,
				    struct shannon_boot_args *args)
{
	struct std_dload_control *dl_ctrl = std_args->dl_ctrl;
	struct std_toc_element *toc = std_args->toc;
	int bin_fd = args->bin_fd;
	int nv_fd = args->nv_fd;
	struct std_dload_control *tmp;
	u32 i;

	for (i = std_args->start_stages; i < std_args->num_stages + 1; i++) {
		dl_ctrl[i].stage = i;
		dl_ctrl[i].start = 0;
		dl_ctrl[i].download = 0;
		dl_ctrl[i].validate = 0;
		dl_ctrl[i].finish = 0;

		if (strcmp(toc[i].name, "VSS") == 0)
			dl_ctrl[i].dl_once = 1;
		else
			dl_ctrl[i].dl_once = 0;

		if (strcmp(toc[i].name, "NV") == 0)
			dl_ctrl[i].b_fd = nv_fd;
		else if (strcmp(toc[i].name, "NV_NORM") == 0)
			dl_ctrl[i].b_fd = args->nv_fd;
		else if (strcmp(toc[i].name, "NV_PROT") == 0)
			dl_ctrl[i].b_fd = args->nv_prot_fd;
		else
			dl_ctrl[i].b_fd = bin_fd;

		dl_ctrl[i].b_offset = toc[i].b_offset;
		dl_ctrl[i].m_offset = (toc[i].m_offset & CP_MEMORY_MASK);
		dl_ctrl[i].b_size = toc[i].size;
		dl_ctrl[i].crc = toc[i].crc;

		tmp = &dl_ctrl[i];
		cbd_log("stage=%u, name:%s b_off=0x%08x, m_offset=0x%08x b_size=0x%08x\n",
			tmp->stage, toc[i].name, tmp->b_offset, tmp->m_offset,
			tmp->b_size);
	}
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
	u32 i;
	u32 nv_size = 0;

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


	/*
	** Set binary stage information
	*/
	std_args->start_stages = cpn->boot_stage;

	if (toc[IMG_TOC].toc_count == 1) {
		/* Support legacy TOC table */
		std_args->num_stages = SHANNON_LEGACY_MAX_DL_STAGE;
	} else {
#ifdef CONFIG_SEC_CP_VERIFYING_ALL
		/* Support verifying TOC/VSS signing : if TOC has m_offset, load it to cp dram */
		if (toc[IMG_TOC].m_offset != 0)
			std_args->start_stages = cpn->toc_stage;
#endif
		std_args->num_stages = toc[IMG_TOC].toc_count - 1;
	}

	if (!cpn->path_nv[0]) {
		/* For wifi model */
		std_args->num_stages = cpn->main_stage;
	}

	if (mode == CP_BOOT_MODE_DUMP) {
		/* For Dump mode */
		std_args->num_stages = cpn->boot_stage;
	}

	for (i = std_args->start_stages; i < std_args->num_stages + 1; i++) { /* Included 'start TOC' */
		if ((strcmp(toc[i].name, "NV") == 0) || (strcmp(toc[i].name, "NV_NORM") == 0))
			nv_size = toc[i].size;

		cbd_log("toc[%d].name = %s, b_off=0x%08x, m_off=0x%08x, size=0x%08x\n",
			i, toc[i].name, toc[i].b_offset, toc[i].m_offset, toc[i].size);
	}

	if ((mode != CP_BOOT_MODE_DUMP) && (nv_size == 0) && cpn->path_nv[0]) {
		cbd_log("ERR! invalid TOC : There is no NV\n");
		goto exit;
	}

	/*
	** Open NV data file
	*/
	if (mode == CP_BOOT_MODE_NORMAL && cpn->path_nv[0]) {
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
					cpn->path_nv, nv_size);
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
	if (mode == CP_BOOT_MODE_NORMAL && cpn->path_nv_prot[0]) {
		nv_prot_fd = open(cpn->path_nv_prot, O_RDONLY);
		if(nv_prot_fd < 0) {
			if (errno != ENOENT) {
				cbd_log("ERR! NV(%s) open fail (%s)\n",
					cpn->path_nv_prot, ERR2STR);
				goto exit;
			}

			cbd_log("ERR! no NV(%s) file\n", cpn->path_nv_prot);

			ret = create_empty_nv(cpn->path_nv_prot, nv_size);
			if (ret < 0) {
				cbd_log("ERR! create_empty_nv(%s, %d) fail\n",
					cpn->path_nv_prot, nv_size);
				goto exit;
			}

			nv_prot_fd = open(cpn->path_nv_prot, O_RDONLY);
			if (nv_prot_fd < 0) {
				cbd_log("ERR! NV(%s) open fail (%s)\n",
					cpn->path_nv_prot, ERR2STR);
				goto exit;
			}
		}

		cbd_log("NV_PROT(%s) opened (fd %d)\n", cpn->path_nv_prot, nv_prot_fd);
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

static int get_modem_state(struct std_boot_args *args)
{
	int status;

	status = ioctl(args->dev_fd, IOCTL_GET_CP_STATUS);
	cbd_log("modem_status: %d\n", status);
	return status;
}

static int load_cp_image_by_stage(struct std_boot_args *args, u32 stage, enum cp_boot_mode mode)
{
	int ret = 0;
	int last = 0;
	int dev_fd = args->dev_fd;
	struct std_dload_control *dlc = &args->dl_ctrl[stage];
	struct modem_img img;
	void *binary;
	unsigned total = 0;

	/* Prepare an image buffer */
	binary = malloc(EXYNOS_PAYLOAD_LEN);
	if (!binary) {
		cbd_log("ERR! malloc(%d) fail\n", dlc->b_size);
		ret = -ENOMEM;
		goto exit;
	}

	img.binary = (unsigned long)binary;
	img.size = dlc->b_size;
	img.m_offset = dlc->m_offset;
	img.b_offset = dlc->b_offset;
	img.mode = mode;
	img.len = EXYNOS_PAYLOAD_LEN;

	cbd_log("stage=%u(%u), b_off=0x%08x, m_off=0x%08x, b_size=0x%08x, mode=0x%08x\n",
		stage, dlc->stage, dlc->b_offset, dlc->m_offset, dlc->b_size, img.mode);

	ret = lseek(dlc->b_fd, img.b_offset, SEEK_SET);
	if (ret < 0) {
		cbd_log("ERR! lseek fail (%u)\n", stage);
		goto exit;
	}

	while(1) {
		if(img.size == img.b_offset)
			break;

		if((img.size - total) < EXYNOS_PAYLOAD_LEN) {
			img.len = img.size - total;
			last = 1;
		}

		ret = read(dlc->b_fd, (void *)img.binary, img.len);
		if (ret < 0) {
			cbd_log("ERR! read fail (%u)\n", stage);
			goto exit;
		}

		if ((u32)ret != img.len) {
			cbd_log("ERR! read %d != img.len %d\n", ret, img.len);
			ret = -EFAULT;
			goto exit;
		}

		ret = ioctl(dev_fd, IOCTL_LOAD_CP_IMAGE, &img);
		if (ret < 0) {
			cbd_log("ERR! IOCTL_LOAD_CP_IMAGE fail (%u)\n", stage);
			goto exit;
		}

		if(last == 1)
			break;

		total += img.len;
		img.m_offset += img.len;
	}
	cbd_log("%u stage complelte\n", stage);
exit:
	if (binary)
		free(binary);

	return ret;
}

static int load_cp_images(struct std_boot_args *args, enum cp_boot_mode mode)
{
	int ret = 0;
	u32 stage;
	char prop_buf[PROPERTY_VALUE_MAX] = {0, };
	struct std_dload_control *dl_ctrl = args->dl_ctrl;

	property_get(VPROP_FIRST_XMIT_DONE, prop_buf, "0");

	for (stage = args->start_stages; stage < args->num_stages + 1; stage++) {
		if (dl_ctrl[stage].dl_once && prop_buf[0] == '1') {
			cbd_log("stage[%u] : stage is already xmit once\n", stage);
			continue;
		}
		ret = load_cp_image_by_stage(args, stage, mode);
		if(ret < 0) {
			cbd_log("ERR! load_cp_image_by_stage stage[%u] fail\n", stage);
			return ret;
		}
	}

	/* return xmit_boot state */
	ret = prop_buf[0] - '0';

	property_set(VPROP_FIRST_XMIT_DONE, "1");
	return ret;
}


#define DT_REVISION_PATH "/proc/device-tree/model_info-system_rev"
void check_board_revision(void)
{
	int fd, ret;
	char rev[256];

	fd = open(DT_REVISION_PATH, O_RDONLY);
	if (fd < 0) {
		cbd_log("%s open fail\n", DT_REVISION_PATH);
		return;
	}
	ret = read(fd, rev, 256);
	if (ret < 0) {
		cbd_log("%s read fail\n", DT_REVISION_PATH);
		close(fd);
		return;
	}
	cbd_log("model_info-system_rev: %s\n", rev);
	close(fd);
	property_set(VPROP_DT_REVISION, rev);
}

#if defined(CONFIG_PROTOCOL_SIT)
int get_factory_prop(void)
{
	char prop_buf[PROPERTY_VALUE_MAX] = {0, };

	property_get(PROP_RADIO_MULTISIM_CONFIG, prop_buf, "");

	if (strncmp(prop_buf, "dsds", 4) == 0) {
		cbd_log("dsds\n");
		return 2;
	}
	cbd_log("no dsds\n");

	return 1;
}
#else	/* if NOT CONFIG_PROTOCOL_SIT */
#define SIM_CONF_PATH		"/efs/factory.prop"
#define MAX_PROP_STRING_LEN	128
int get_factory_prop(void)
{
	int fd, ret;
	char full_string[MAX_PROP_STRING_LEN];
	char *sim_value, *token = "=";

	fd = open(SIM_CONF_PATH, O_RDONLY);
	if (fd < 0) {
		cbd_log("%s open fail(not support)\n", SIM_CONF_PATH);
		return -EINVAL;
	}

	ret = read(fd, full_string, MAX_PROP_STRING_LEN);
	if (ret < 0) {
		cbd_log("%s read fail\n", SIM_CONF_PATH);
		goto exit;
	}

	sim_value = strstr(full_string, token);
	if (sim_value == NULL) {
		cbd_log("can't find token!\n");
		ret = -EINVAL;
		goto exit;
	} else {
		ret = (++sim_value)[0] - '0';
		cbd_log("sim_count: %d\n", ret);
	}

exit:
	close(fd);
	return ret;
}
#endif /* End of CONFIG_PROTOCOL_SIT */

#define SIM_CONF_KERN_PATH	"/sys/devices/platform/cpif/sim/ds_detect"
void set_sim_configuration(void)
{
	char cmd_string[128];
	int sim_count;

	sim_count = get_factory_prop();
	if (sim_count > 0) {
		cbd_log("sim count: %d (echo to ds_detect file)\n", sim_count);
		sprintf(cmd_string, "echo %d > %s", sim_count, SIM_CONF_KERN_PATH);
		system(cmd_string);
	}
}

int start_shannon310_boot(struct boot_args *cbd_args)
{
	int ret = 0;
	int spin = 50;
	char prop_buf[PROPERTY_VALUE_MAX] = {0, };
	struct shannon_boot_args *dl_args = NULL;

	check_board_revision();

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
	** Start CP bootup
	*/
	boot_wake_lock(1);

	cbd_log("Prepare arguments\n");
	dl_args = prepare_boot_args(cbd_args, CP_BOOT_MODE_NORMAL);
	if (!dl_args) {
		cbd_log("ERR! prepare_boot_args fail\n");
		ret = -EFAULT;
		goto exit;
	}

	store_srinfo(cbd_args->cpn->rat, dl_args->std_args->dev_fd);

	if (get_modem_state(dl_args->std_args) != STATE_OFFLINE) {
		cbd_log("Power reset CP_BOOT_MODE_NORMAL\n");
		ret = std_boot_power_reset(dl_args->std_args, CP_BOOT_MODE_NORMAL);
		if (ret < 0) {
			cbd_log("ERR! std_boot_power_reset fail\n");
			goto exit;
		}
	}

	cbd_log("Power on CP\n");
	ret = std_boot_power_on(dl_args->std_args);
	if (ret < 0) {
		cbd_log("ERR! std_boot_power_on fail\n");
		goto exit;
	}

	cbd_log("Request security : non-secure mode\n");
	ret = std_security_req(dl_args->std_args, CP_BOOT_RE_INIT, 0, 0);
	if (ret < 0) {
		cbd_log("ERR! security check fail\n");
		goto exit;
	}

	cbd_log("Send CP image\n");
	ret = load_cp_images(dl_args->std_args, CP_BOOT_MODE_NORMAL);
	if (ret < 0) {
		cbd_log("ERR! BOOT_STAGE fail\n");
		goto exit;
	}

#ifdef CONFIG_SEC_CP_VERIFYING_ALL
	cbd_log("Request security : verifying VSS, TOC\n");
	if (dl_args->std_args->start_stages == cbd_args->cpn->boot_stage) {
		cbd_log("ERR! TOC has no m_offset\n");
		ret = -EINVAL;
		goto exit;
	}

	if (ret) {
		/* if xmit_boot was already done once, skip vss verifying */
		cbd_log("skip VSS check\n");
	} else {
		ret = std_security_req(dl_args->std_args, CP_BOOT_MODE_MANUAL,
				       dl_args->std_args->dl_ctrl[IMG_VSS].m_offset,
				       dl_args->std_args->dl_ctrl[IMG_VSS].b_size);
		if (ret < 0) {
			cbd_log("ERR! security check fail for VSS\n");
			goto exit;
		}
	}

	ret = std_security_req(dl_args->std_args, CP_BOOT_MODE_MANUAL,
			       dl_args->std_args->dl_ctrl[IMG_TOC].m_offset,
			       dl_args->std_args->dl_ctrl[IMG_TOC].b_size);
	if (ret < 0) {
		cbd_log("ERR! security check fail for TOC\n");
		goto exit;
	}
#endif

	cbd_log("Request security : secure mode\n");
	ret = std_security_req(dl_args->std_args, CP_BOOT_MODE_NORMAL,
			       dl_args->std_args->dl_ctrl[IMG_BOOT].b_size,
			       dl_args->std_args->dl_ctrl[IMG_MAIN].b_size);
	if (ret < 0) {
		cbd_log("ERR! security check fail\n");
		goto exit;
	}

	/* set SIM configuration using /efs/factory.prop */
	set_sim_configuration();

	cbd_log("Start CP bootloader\n");
	ret = std_boot_start_cp_bootloader(dl_args->std_args, CP_BOOT_MODE_NORMAL);
	if (ret < 0) {
		cbd_log("ERR! std_boot_start_cp_bootloader fail\n");
		goto exit;
	}

	cbd_log("Handshake\n");
	switch (cbd_args->type) {
		case SEC_MODAP_SIT:
			ret = std_boot_finish_handshake_sit(dl_args->std_args);
			break;
		default:
			ret = std_boot_finish_handshake(dl_args->std_args);
			break;
	}
	if (ret < 0) {
		cbd_log("ERR! std_boot_finish_handshake fail\n");
		goto exit;
	}

	cbd_log("Complete normal bootup\n");
	ret = std_boot_complete_normal_bootup(dl_args->std_args);
	if (ret < 0) {
		cbd_log("ERR! std_boot_complete_normal_bootup fail\n");
		goto exit;
	}
	restore_srinfo(cbd_args->cpn->rat, dl_args->std_args->dev_fd);

exit:
	if (dl_args)
		close_boot_args(dl_args);

	boot_wake_lock(0);

	return ret;
}

//#define CBD_CPCRASH_HISTORY_LOG
#ifdef CBD_CPCRASH_HISTORY_LOG
/* save the cp crash dump history to /data/cp_log/cpcrash_history.txt */
static int open_crash_history_file(void)
{
	struct stat ldir_st;
	int ret;

	ret = stat("/data/cp_log", &ldir_st);
	if (!ret) { /* path exist */
		if (!S_ISDIR(ldir_st.st_mode)) {
			cbd_log("(%s) is not a directory\n", "/data/cp_log");
			goto exit;
		}
	} else {
		ret = mkdir("/data/cp_log", 0755);
		if (ret) {
			cbd_log("log path create fail(%d)\n", ret);
			goto exit;
		}
		cbd_log("log path (%s) created\n", "/data/cp_log");
	}
	return open("/data/cp_log/cpcrash_history.txt",
		O_WRONLY | O_APPEND | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
exit:
	return ret;
}
#else
static inline int open_crash_history_file(void)
{
	return -1;
}
#endif

int start_shannon310_dump(struct boot_args *cbd_args)
{
	int ret;
	int log_fd = -1, history_fd = -1;
	struct std_dump_args *ul_args;
	struct shannon_boot_args *dl_args = NULL;
	char reason[MAX_PREFIX_LEN];

	boot_wake_lock(1);

	/*
	** Save kernel log
	*/
	snprintf(reason, MAX_PREFIX_LEN, "%s_crash", cbd_args->cpn->rat);
	save_logs(LOGB_DMESG, reason);

#ifndef CONFIG_PROTOCOL_SIT
	history_fd = open_crash_history_file();
	if (history_fd < 0) {
		cbd_log("cp dump history file open fail\n");
	} else {
		char timestr[MAX_SUFFIX_LEN];
		time_t now;
		struct tm result;
		char serial[PROPERTY_VALUE_MAX];

		property_get(PROP_SERIAL_NO, serial, 0);
		time(&now);
		localtime_r(&now, &result);
		strftime(timestr, MAX_SUFFIX_LEN, "%Y%m%d-%H%M", &result);
		dprintf(history_fd, "***** %s (%s)*****\n\r", timestr, serial);
	}
#endif

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

	/* Save srinfo from shmem to file */
	store_srinfo(cbd_args->cpn->rat, dl_args->std_args->dev_fd);

	cbd_log("Get log dump\n");
	get_log_dump(ul_args, "shmem", LOG_IDX_SHMEM);
	get_log_dump(ul_args, "vss", LOG_IDX_VSS);
	get_log_dump(ul_args, "databuf", LOG_IDX_DATABUF);
	get_log_dump(ul_args, "acpm", LOG_IDX_ACPM);
	get_log_dump(ul_args, "cp_btl", LOG_IDX_CP_BTL);

	cbd_log("Power reset CP_BOOT_MODE_DUMP\n");
	ret = std_boot_power_reset(dl_args->std_args, CP_BOOT_MODE_DUMP);
	if (ret < 0) {
		cbd_log("ERR! std_boot_power_reset fail\n");
		goto exit;
	}

	cbd_log("Load CP bootloader\n");
	ret = load_cp_images(dl_args->std_args, CP_BOOT_MODE_DUMP);
	if (ret < 0) {
		cbd_log("ERR! load_cp_images fail\n");
		goto exit;
	}

	cbd_log("Request security : dump mode\n");
	ret = std_security_req(dl_args->std_args, CP_BOOT_MODE_DUMP,
			       dl_args->std_args->dl_ctrl[IMG_BOOT].b_size,
			       dl_args->std_args->dl_ctrl[IMG_MAIN].b_size);
	if (ret < 0) {
		cbd_log("ERR! security check fail\n");
		goto exit;
	}

	cbd_log("Start CP bootloader for crash dump\n");
	ret = std_boot_start_cp_bootloader(dl_args->std_args, CP_BOOT_MODE_DUMP);
	if (ret < 0) {
		cbd_log("ERR! std_boot_start_cp_bootloader fail\n");
		goto exit;
	}

	cbd_log("Receive CP crash dump\n");
	switch(cbd_args->type){
		case SEC_MODAP_SIT:
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
	if (history_fd >= 0) {
		ret = write(history_fd, ul_args->reason, 64);
		if (ret)
			cbd_log("write crash info to history file\n");
		dprintf(history_fd, "--\n\r");
		fsync(history_fd);
		system("ls -al /sdcard/log/cpcrash_dump_* >> /data/cp_log/cpcrash_filelist.txt");
	}

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

	if (history_fd >= 0) {
		dprintf(history_fd, "***** dump fail *****\n");
		close(history_fd);
	}

	boot_wake_lock(0);

	return ret;
}

int start_shannon310_dummy_dump(struct boot_args *cbd_args)
{
	/* Do nothing */
	return 0;
}

int shutdown_shannon310_modem(struct boot_args *cbd_args)
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

int upload_shannon310_modem(struct boot_args *cbd_args)
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
