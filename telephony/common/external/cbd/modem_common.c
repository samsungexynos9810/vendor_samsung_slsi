/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 * History
 * 1. 20140927, Added silent reset("STATE_CRASH_RESET" -> modem_off -> CP boot again)
 * 2. 20141030, Support RFS Phase2(nv_normal.bin and nv_protected.bin)
 * 3. 20141111, Support CBD version2 (fast CBD for ss310 modem)
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include "cbd_header.h"
#include "util_srinfo.h"

#define PROP_CPBOOT_DONE              "vendor.ril.cbd.boot_done"
#define PROP_CPRESET_DONE             "vendor.ril.cbd.reset_done"
#define PROP_RFS_CHECKDONE            "vendor.ril.cbd.rfs_check_done"
#define PROP_MODEM_PATH               "exynos.modempath"
#define PROP_NV_PATH                  "vendor.ril.exynos.nvpath"
#define PROP_RADIO_MULTISIM_CONFIG	"persist.radio.multisim.config"

#define CMDLINE_BUF_SIZE 1024
enum cbd_debug_opt {
	CBD_NORMAL_BOOT,
	CBD_NO_BOOT,
};

struct modem_node node = {
#if defined(SS310)
    .name = "ss310",
#endif
    .node_boot = "/dev/umts_boot0",
    .node_status = "/dev/umts_boot0",
#if defined(USE_GPT)
    .path_nv = "/mnt/vendor/efs/nv_data.bin",
    .path_nv_norm = "/mnt/vendor/efs/nv_normal.bin",
    .path_nv_prot = "/mnt/vendor/efs/nv_protected.bin",

#if defined(NOT_USE_CP_PARTITION)
    .path_bin = "/system/vendor/firmware/modem.bin",
#else
#if defined(USE_UFS)
    .path_bin = "/dev/block/by-name/modem",
#else
    .path_bin = "/dev/block/mmcblk0p5",
#endif
#endif

#else
    .path_nv = "/data/nv_data.bin",
    .path_nv_norm = "/data/nv_normal.bin",
    .path_nv_prot = "/data/nv_protected.bin",
    .path_bin = "/data/modem.bin",
#endif

#if defined(USE_SPI_BOOT_LINK)
    .path_spi = "/dev/spi_boot_link",
#else
    .path_spi = "/dev/modem_boot_spi",
#endif
};

static char path_nv[256];
static char path_nv_norm[256];
static char path_nv_prot[256];

int wait_event(int fd, short events, long timeout)
{
    int ret;
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events = events;
    while (1) {
        pfd.revents = 0;
        ret = poll(&pfd, 1, timeout);
        if (pfd.revents & events)
            break;

        if (pfd.revents == (POLLERR | POLLHUP)) {
            ret = -EIO;
            goto exit;
        } else if (pfd.revents & POLLHUP) {
            usleep(20000); //20ms wait
            //cbd_info("POLLHUP\n");
            continue;
        }
    }

    return 0;

exit:
    //cbd_err(" \n");
    return ret;
}
static u32 get_resp(struct modem_args *in_args, u32 exp)
{
    struct modem_args *args = in_args;
    int ret = 0;
    u32 get;

#if !defined(DISABLE_FINAL_CMD)
    // 1. wait POLLIN event
    ret = wait_event(args->fds.fd_dev, POLLIN, TIMEOUT);
    if (ret < 0) {
        cbd_err("wait event fail!!\n");
        goto exit;
    }

    // 2. get response
    ret = read(args->fds.fd_dev, &get, sizeof(u32));
    if (ret < 0) {
        cbd_err("read resp fail!!(%s)\n", ERR2STR);
        goto exit;
    }

    // 3. verify
    if (exp != get) {
        cbd_err("G:0x%X != E:0x%X\n", get, exp);
        ret = -EFAULT;
    }
#endif
    exit:
        return ret;
}

int send_cmd(struct modem_args *in_args, u32 cmd, u32 toc, u32 stat)
{
    struct modem_args *args = in_args;
    int ret = 0;
    u32 msg,resp = 0;

    msg = MSG(MSG_AP2CP, cmd, toc, stat);
    resp = MSG(MSG_CP2AP, cmd, toc, stat);

    cbd_info("CMD:0x%X Resp:0x%X \n", msg, resp);

    // 1. send cmd
    ret = write(args->fds.fd_dev, &msg, sizeof(u32));
    if (ret < 0) {
        cbd_err("send cmd (%s)\n", ERR2STR);
        goto exit;
    }

    // 2. get & verify response
    ret = get_resp(args, resp);
    if (ret < 0) {
        cbd_err("get resp fail\n");
        goto exit;
    }

exit:
    return ret;
}

static int readn_tx_frame(struct modem_args *data_args, struct exynos_payload *exynos, int fd, int tx_len)
{
    struct modem_args *args = data_args;
    struct exynos_payload *exynos_payload = exynos;
    int ret = 0;
    int len = tx_len;
    int fd_data = fd;

    memset(exynos_payload->pdata, 0, EXYNOS_PAYLOAD_LEN);

    ret = read(fd_data, exynos_payload->pdata, len);
    if (ret < 0) {
        cbd_err("read bin fail (%s)\n", ERR2STR);
        goto exit;
    }

    if (ret != len) {
        cbd_err("read[%d] != expect[%d] ", ret, len);
        cbd_info("Format mmc block6 to solve this error\n");
        ret = -EFAULT;
        goto exit;
    }

    exynos_payload->length = tx_len + sizeof(struct exynos_data_info);

    len += EXYNOS_MSG_INFO_SIZE;
    ret = write(args->fds.fd_dev, exynos_payload, len);
    if (ret < 0) {
        cbd_err("write fail (%s)\n", ERR2STR);
        goto exit;
    }

    if (ret != len) {
        cbd_err("write[%d] != expect[%d]", ret, len);
        ret = -EFAULT;
        goto exit;
    }

    return 0;

exit:
    return ret;
}

static int send_data(struct modem_args *data_args, struct download_stage_control *data_ctrl)
{
    struct modem_args *args = data_args;
    struct download_stage_control *ctrl = data_ctrl;
    struct exynos_payload exynos_data;
    struct exynos_data_info exynos_data_info;
    int len, ret = 0;
    int last = 0;
    u32 exp = MSG(MSG_CP2AP, MSG_DOWNLOAD, ctrl->stage, MSG_DATA);

    // 0. preparation
    memset(&exynos_data, 0x0, sizeof(struct exynos_payload));

    exynos_data_info.total_size = ctrl->size;
    exynos_data_info.data_offset = 0;
    //CBD_info("size:%d\n", exynos_data_info.total_size);

    if (exynos_data_info.total_size < EXYNOS_PAYLOAD_LEN)
        len = exynos_data_info.total_size;
    else
        len = EXYNOS_PAYLOAD_LEN;

    //CBD_info("len:%d\n", len);
    // 1. seek each state-TOC, BOOT, MAIN, NV, etc
    cbd_info("offset:0x%x\n", ctrl->offset);
    ret = lseek(args->fds.fd_bin, ctrl->offset, SEEK_SET);
    if (ret < 0) {
        cbd_err("lseek fail (%s)\n", ERR2STR);
        goto exit;
    }

    // 2. read and write
    while (1) {
        if (exynos_data_info.total_size == exynos_data_info.data_offset)
            break;

        if (exynos_data_info.total_size - exynos_data_info.data_offset < EXYNOS_PAYLOAD_LEN) {
            len = exynos_data_info.total_size - exynos_data_info.data_offset;
            last = 1;
        }

        // 2.1 read and write
        exynos_data.cmd = MSG(MSG_AP2CP, MSG_DOWNLOAD, ctrl->stage, MSG_DATA);
        readn_tx_frame(args, &exynos_data, ctrl->fd, len);
        exynos_data_info.data_offset += len;

        // 2.2 get and verify  ACK
        if (exp) {
            ret = get_resp(args, exp);
            if (ret < 0) {
                cbd_err("get resp fail\n");
                goto exit;
            }
        }
        if (last == 1)
            break;
    }

    return 0;

exit:
    return ret;
}

static int send_crc(struct modem_args *data_args, struct download_stage_control *data_ctrl)
{
    struct modem_args *args = data_args;
    struct download_stage_control *ctrl = data_ctrl;
    struct crc_info crc_frame;
    int ret;
    u32 exp = MSG(MSG_CP2AP, MSG_SECURITY, MSG_NONE_TOC, MSG_PASS);

    crc_frame.cmd = MSG(MSG_AP2CP, MSG_SECURITY, MSG_NONE_TOC, MSG_CRC);
    crc_frame.crc = ctrl->crc;
    cbd_info("cmd:0x%X, crc:0x%x\n", crc_frame.cmd, crc_frame.crc);

    // 1. write CRC
    ret = write(args->fds.fd_dev, &crc_frame, sizeof(struct crc_info));
    if (ret < 0) {
        cbd_err("crc write fail (%s)\n", ERR2STR);
        goto exit;
    }

    // get and verify resp
    ret = get_resp(args, exp);
    if (ret < 0) {
        cbd_err("get resp fail\n");
        goto exit;
    }
    return 0;

exit:
    return ret;
}

int exynos_process(struct modem_args *exynos_args)
{
    struct modem_args *args = exynos_args;
    int ret = 0;
    size_t i = 0;
#if defined(SS310)
    struct sec_info info;

    if (args->state == STATE_BOOTLOADER) {
        if (args->bmode == BOOTMODE_DUMP) {
            if (args->ops.send_bootloader != NULL) {
                args->ops.send_bootloader(args);
                info.bmode = BOOTMODE_DUMP;
                info.boot_size = args->toc[IMG_BOOT].size;
                info.main_size = 0;
                ret = modem_request_security(args->fds.fd_boot, &info);
                if (ret != 0) {
                    cbd_err("modem_request_security failed!!!\n");
                    return -EPERM;
                }
                ret = modem_dump_start(args->fds.fd_boot);
                if (ret < 0) {
                    cbd_err("modem_dump_start failed!!!\n");
                    return ret;
                }
                return TRUE;
            } else
                goto exit;
        } else {
            return TRUE;
        }
    }
#else
    if (args->state == STATE_BOOTLOADER) {
        if (args->ops.send_bootloader != NULL) {
            args->ops.send_bootloader(args);
            return TRUE;
        } else
            goto exit;
    }
#endif

    //BOOT_STAGE, TOC_STAGE, MAIN_STAGE, NV_STAGE, FIN_STAGE
#ifdef LINK_MAIN_PCIE
    for(i = TOC_STAGE; i < (args->num_stages + 1); i++) {
#else
    for(i = 0; i < (args->num_stages + 1); i++) {
#endif
        struct download_stage_control *ctrl = &args->ctrl[i];

#ifdef VERBOSE_DEBUG_ENABLE
        cbd_info("ctrl->stage[%d]=%d \n", i, ctrl->stage);
        cbd_info("ctrl->start_req[%d]=%d \n", i, ctrl->start_req);
        cbd_info("ctrl->data_available[%d]=%d \n", i, ctrl->data_available);
        cbd_info("ctrl->size[%d]=0x%x \n", i, ctrl->size);
        cbd_info("ctrl->offset[%d]=0x%x \n", i, ctrl->offset);
        cbd_info("ctrl->security_check[%d]=%d \n", i, ctrl->security_check);
        cbd_info("ctrl->finish_req[%d]=%d \n", i, ctrl->finish_req);
#endif

        if (ctrl->start_req) {
            if (ctrl->stage == FIN_STAGE)
                ret = send_cmd(args, MSG_FINALIZE, MSG_NONE_TOC, MSG_PASS);
            else
                ret = send_cmd(args, MSG_DOWNLOAD, ctrl->stage, MSG_START);

            if (ret < 0) {
                cbd_err("start fail!!\n");
                goto exit;
            }
        }

        if (ctrl->data_available) {
            ret = send_data(args, ctrl);
            if (ret < 0) {
                cbd_err("send_data fail stage:%d\n", ctrl->stage);
                goto exit;
            }
        }

        if (ctrl->security_check == CRC_CHECK) {
            cbd_info("CRC\n");
            ret = send_crc(args, ctrl);
            if (ret < 0) {
                cbd_err("send_crc fail\n");
                goto exit;
            }
        }

        if (ctrl->finish_req) {
            cbd_info("FINISH\n");
            if (ctrl->stage == BOOT_STAGE)
                ret = send_cmd(args, MSG_READY, ctrl->stage, MSG_BOOT);
            else
                ret = send_cmd(args, MSG_DOWNLOAD, ctrl->stage, MSG_DONE);

            if (ret < 0) {
                cbd_err("stage done fail\n");
                goto exit;
            }
        }
    }

    exit:
        return ret;
}

int check_radio_multisim_config()
{
	char prop_buf[PROPERTY_VALUE_MAX] = {0, };
	property_get(PROP_RADIO_MULTISIM_CONFIG, prop_buf, "");

	if (strcmp(prop_buf, "dsds") == 0) {
		cbd_logcat_debug("dsds\n");
		return 2;

	} else {
		cbd_logcat_debug("no dsds\n");
		return 0;
	}
}

void check_nv_validity()
{
    int spin = 50;
    char prop_buf[PROPERTY_VALUE_MAX] = {0, };

    while (spin--) {
        property_get(PROP_RFS_CHECKDONE, prop_buf, "0");
        if (prop_buf[0] == '1')
            break;
        usleep(100000);
    }
    cbd_info("%s\n", spin < 0 ? "TIMEOUT" : "DONE");

}

int create_default_nv(char *path, size_t size)
{
    int ret = 0;
    int saved = 0;
    int nv_fd;
    char *nv_data = NULL;
    struct passwd *pwd;

    nv_fd = open(path, O_RDWR | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (nv_fd < 0) {
        cbd_err("open fail(%s)\n", ERR2STR);
        ret = -EFAULT;
        goto exit;
    }

    /* Set ownership to radio */
    pwd = getpwnam("radio");
    if (pwd != NULL)
    {
        chown(path, pwd->pw_uid, pwd->pw_gid);
    }

    nv_data = (char *)malloc(size);
    if (!nv_data) {
        cbd_err("malloc fail\n");
        ret = -ENOMEM;
        goto exit;
    }
    memset(nv_data, 0xFF, size);

    saved = write(nv_fd, nv_data, size);
    if (saved < 0) {
        cbd_err("write fail\n");
        ret = -EFAULT;
        goto exit;
    }
    fsync(nv_fd);

exit:
    if (nv_fd >= 0)
        close(nv_fd);

    if (nv_data)
        free(nv_data);

    return ret;
}

int open_NV(char *nv_path, size_t nv_size) {
    char *path = nv_path;
    size_t size = nv_size;
    int fd_ret;

    fd_ret = open(path, O_RDONLY);
    if (fd_ret < 0) {
        if (errno != ENOENT) {
            cbd_err("NV(%s) open fail (%s)\n", path, ERR2STR);
            goto exit;
        }

        cbd_info("Create default NV file\n");
        fd_ret = create_default_nv(path, size);
        if (fd_ret < 0) {
            cbd_err("create_default_nv fail\n");
            goto exit;
        }
        fd_ret = open(path, O_RDONLY);
        if (fd_ret < 0) {
            cbd_err("nv_file (%s) open fail (%s)\n", path, ERR2STR);
            goto exit;
        }
    }
    exit:
        return fd_ret;
}

void NV_init_seek(struct modem_args *nv_args)
{
    struct modem_args *args = nv_args;

    if (args->nv_ver == NV_V2) {
        lseek(args->fds.fd_nv_norm, 0, SEEK_SET);
        lseek(args->fds.fd_nv_prot, 0, SEEK_SET);
    } else
        lseek(args->fds.fd_nv, 0, SEEK_SET);
}

int prepare_args(struct modem_args *args)
{
    int ret;
    struct modem_args *cbd_args = args;
    int toc_total_size;
    int i, max_stage;
    u32 nv_norm_size = 0;
    u32 nv_prot_size = 0;
    char nv_path[PROPERTY_VALUE_MAX];

    // 0. initialize args and set initial state
    memset(cbd_args, 0, sizeof(struct modem_args));
    cbd_args->state = STATE_INIT;
    cbd_args->bmode = BOOTMODE_NORMAL;

    // 1. set modem type and link, define from Android.mk
    cbd_args->modem.type = DEFAULT_MODEM;
    cbd_args->modem.link_boot = DEFAULT_BOOT_LINK;
    cbd_args->modem.link_main = DEFAULT_MAIN_LINK;

    if (cbd_args->modem.link_boot == LINKDEV_SPI) {
        cbd_args->fds.fd_spi = open(node.path_spi, O_RDWR);
        if (cbd_args->fds.fd_spi < 0) {
            ret = cbd_args->fds.fd_spi;
            cbd_err("SPI (%s) open fail (%s)\n", node.path_spi, ERR2STR);
            cbd_info("First time? Check kernel driver can support (%s)\n", node.path_spi);
            goto exit;
        }
    }

    // 2. open modem image path at predefined path.
    cbd_info("MODEM path:%s\n", node.path_bin);
    cbd_args->fds.fd_bin = open(node.path_bin, O_RDONLY);

    if (cbd_args->fds.fd_bin < 0) {
        ret = cbd_args->fds.fd_bin;
        cbd_err("modem image open fail (%s)\n", ERR2STR);
        cbd_info("If GPT is supported, flash modem binary using the command 'fastboot flash modem modem.bin'\n");
        cbd_info("If GPT is not supported, copy modem binary using the command 'adb push modem.bin /data'\n");
        goto exit;
    }

    property_get(PROP_NV_PATH, nv_path, "0");

    if (!strcmp(nv_path, "0")) {
        cbd_info("There is no data in property: %s\n", PROP_NV_PATH);
    } else {
        cbd_info("property %s: %s\n", PROP_NV_PATH, nv_path);

        /* nv_data path */
        sprintf(path_nv, "%snv_data.bin", nv_path);
        node.path_nv = path_nv;

        /* nv_normal path */
        sprintf(path_nv_norm, "%snv_normal.bin", nv_path);
        node.path_nv_norm = path_nv_norm;

        /* nv_protected path */
        sprintf(path_nv_prot, "%snv_protected.bin", nv_path);
        node.path_nv_prot = path_nv_prot;
    }

    cbd_info("path_nv: %s\n", node.path_nv);
    cbd_info("path_nv_norm: %s\n", node.path_nv_norm);
    cbd_info("path_nv_prot: %s\n", node.path_nv_prot);

    // 3. read TOC from modem image
    toc_total_size = sizeof(struct toc_structure) * MAX_TOC_INDEX;
    ret = read(cbd_args->fds.fd_bin, cbd_args->toc, toc_total_size);
    if (ret < 0) {
        cbd_err("TOC read fail (%s)\n", ERR2STR);
        goto exit;
    }

#if defined(SS310)
    cbd_info("MODEM SS310 CBD V2\n");
#endif
#ifdef HAS_VSS
    cbd_info("VSS partition is defined!\n");
#endif

    if (strcmp(cbd_args->toc[IMG_TOC].name, "TOC")) {
        cbd_err("ERR! invalid TOC: No TOC\n");
        goto exit;
    }

    if (cbd_args->toc[IMG_TOC].toc_count > MAX_TOC_INDEX) {
        cbd_err("ERR! invalid TOC: Total TOC count is %d\n",
            cbd_args->toc[IMG_TOC].toc_count);
        goto exit;
    }

    /* Support legacy TOC table */
    if (cbd_args->toc[IMG_TOC].toc_count == 1)
        max_stage = MAX_IMAGE_TYPE;
    else
        max_stage = cbd_args->toc[IMG_TOC].toc_count;

    cbd_args->num_stages = max_stage;

    for (i=0; i < (int)(max_stage); i++) {
        if (strcmp(cbd_args->toc[i].name, "NV") == 0) {
            cbd_args->nv_ver = NV_V1;
            nv_norm_size = cbd_args->toc[i].size;
        } else if (strcmp(cbd_args->toc[i].name, "NV_NORM") == 0) {
            cbd_args->nv_ver = NV_V2;
            nv_norm_size = cbd_args->toc[i].size;
        } else if (strcmp(cbd_args->toc[i].name, "NV_PROT") == 0) {
            nv_prot_size = cbd_args->toc[i].size;
        }

        cbd_info("TOC[%d] name= %s, b_off= %u, m_off= %u, size= %u crc= 0x%08X\n",
            i, cbd_args->toc[i].name, cbd_args->toc[i].b_offset,
            cbd_args->toc[i].m_offset, cbd_args->toc[i].size,
            cbd_args->toc[i].crc);
    }

    if (cbd_args->nv_ver == NV_V1)
        cbd_info("NV Version 1\n");
    else
        cbd_info("NV Version 2\n");

    if (nv_norm_size == 0) {
        cbd_err("ERR! invalid TOC : There is no NV\n");
        goto exit;
    }

    // 4. Open NV file , if there is no NV file, create default NV file
    if (cbd_args->nv_ver == NV_V2) {
        cbd_args->fds.fd_nv_norm = open_NV(node.path_nv_norm, nv_norm_size);
        cbd_args->fds.fd_nv_prot = open_NV(node.path_nv_prot, nv_prot_size);
        if (cbd_args->fds.fd_nv_norm < 0) {
            ret = cbd_args->fds.fd_nv_norm;
            cbd_err("NV normal file open fail (%s)\n", ERR2STR);
            goto exit;
        }

        if (cbd_args->fds.fd_nv_prot < 0) {
            ret = cbd_args->fds.fd_nv_prot;
            cbd_err("NV protected file open fail (%s)\n", ERR2STR);
            goto exit;
        }
    } else {
        cbd_args->fds.fd_nv = open_NV(node.path_nv, nv_norm_size);
        if (cbd_args->fds.fd_nv < 0) {
            ret = cbd_args->fds.fd_nv;
            cbd_err("NV file open fail (%s)\n", ERR2STR);
            goto exit;
        }
    }

    // 5. open boot node
    cbd_args->fds.fd_boot = open(node.node_boot, O_RDWR);
    if (cbd_args->fds.fd_boot < 0) {
        ret = cbd_args->fds.fd_boot;
        cbd_err("ERR! DEV(%s) open fail (%s)\n", node.node_boot, ERR2STR);
        cbd_err("Kernel does not support MODEM IF, pls download MODEM KERNEL again!!!\n");
        goto exit;
    } else
        cbd_args->fds.fd_dev = cbd_args->fds.fd_boot;

    return TRUE;

exit:
    if (cbd_args->fds.fd_spi >= 0)
        close(cbd_args->fds.fd_spi);

    if (cbd_args->fds.fd_bin >= 0)
        close(cbd_args->fds.fd_bin);

    if (cbd_args->fds.fd_nv >= 0)
        close(cbd_args->fds.fd_nv);
    if (cbd_args->fds.fd_nv_norm >= 0)
        close(cbd_args->fds.fd_nv_norm);
    if (cbd_args->fds.fd_nv_prot >= 0)
        close(cbd_args->fds.fd_nv_prot);

    if (cbd_args->fds.fd_dev >= 0)
        close(cbd_args->fds.fd_dev);

    return ret;
}

char *get_cmdline_str(char *buf, unsigned size, char *key)
{
    char *ptr;
    int fd;

    fd = open("/proc/cmdline", O_RDONLY);
    if (fd >= 0) {
	int n = read(fd, buf, size - 1);

	if (n < 0)
	    n = 0;
	    /* get rid of trailing newline, it happens */
	    if (n > 0 && buf[n-1] == '\n')
	    n--;

	    buf[n] = 0;
	    close(fd);
	} else
	    buf[0] = 0;

	ptr = buf;
	while (ptr && *ptr) {
	    char *x = strchr(ptr, ' ');
	    if (x != 0)
		*x++ = 0;

	    if (strncmp(ptr, key, strlen(key)) == 0)
			return ptr;
	    ptr = x;
	}

	return NULL;
}

int get_cbd_debug_opt()
{
    char buf[CMDLINE_BUF_SIZE];
    char *key = "cbd_debug_opt=";
    char *str_cbd_debug_opt;
    int ret = CBD_NORMAL_BOOT;

    memset(buf, 0, CMDLINE_BUF_SIZE);
    str_cbd_debug_opt = get_cmdline_str(buf, CMDLINE_BUF_SIZE, key);
    if (str_cbd_debug_opt) {
	    str_cbd_debug_opt = str_cbd_debug_opt + strlen(key);

	    if (strncmp(str_cbd_debug_opt, "noboot", strlen("noboot")) == 0)
		ret = CBD_NO_BOOT;

	    return ret;
    }

    return ret;
}

#define CRASH_RECOVERY_RETRY_COUNT 5
struct modem_args pub_args;
int modem_manager()
{
    int exit;
    int ret = 0;
    struct modem_args *args = &pub_args;
    int crash_handling_mode;
    int cbd_debug_opt = get_cbd_debug_opt();
    int sim_slot_cnt = 0;
    int recovery_retry = CRASH_RECOVERY_RETRY_COUNT;

    cbd_info("cbd_debug_opt=%d\n", cbd_debug_opt);
    if (cbd_debug_opt == CBD_NO_BOOT) {
        cbd_info("CBD_NO_BOOT\n");
        args->fds.fd_boot = open(node.node_boot, O_RDWR);
        if (args->fds.fd_boot < 0) {
            cbd_info("ERR! DEV(%s) open fail (%s). Just sleep\n", node.node_boot, ERR2STR);
            while(1)
                sleep(10);
        }
        cbd_info("Keep monitoring\n");
        args->fds.fd_dev = args->fds.fd_boot;
        modem_monitoring(args);
        return -1;
    }

    check_nv_validity();

    ret = prepare_args(args);
    if (ret < 0) {
        cbd_err("prepare args failed!!!\n");
        return ret;
    }

    ret = modem_status(args->fds.fd_boot);
    switch (ret) {
    case STATE_OFFLINE:
        break;

    case STATE_CRASH_EXIT:
    case STATE_CRASH_WATCHDOG:
        args->state = STATE_DUMP;
        args->bmode = BOOTMODE_NORMAL;
        break;

    default:
        modem_reset(args->fds.fd_boot);
        sec_cp_init(args->fds.fd_boot);
        break;
    }

    ret = prepare_download_control(args);
    if (ret < 0) {
        cbd_err("prepare_download_control failed!!!\n");
        return ret;
    }

    if (cbd_debug_opt == CBD_NO_BOOT) {
        cbd_info("cbd_debug_opt == CBD_NO_BOOT\n");
	args->state = STATE_MONITORING;
    }

    do
    {
        exit = FALSE;
        switch(args->state)
        {
            case STATE_INIT:
                cbd_info("+STATE_INIT\n");
                // 1. modem on
		sim_slot_cnt = check_radio_multisim_config();
		ret = modem_on(args->fds.fd_boot, sim_slot_cnt);
                if (ret < 0) {
                    cbd_err("modem_on\n");
                    cbd_logcat_err("Err:modem_on\n");
                    goto state_err;
                }
                // 2. boot on
                ret = modem_boot_on(args->fds.fd_boot);
                if (ret < 0) {
                    cbd_err("boot on\n");
                    cbd_logcat_err("Err:boot on\n");
                    goto state_err;
                }
                args->state = STATE_BOOTLOADER;
                break;

            case STATE_BOOTLOADER:
                cbd_info("+STATE_BOOTLOADER\n");
                // 3. download start
                ret = modem_start_download(args->fds.fd_boot);
                if (ret < 0) {
                    cbd_err("download start\n");
                    cbd_logcat_err("Err:download start\n");
                    goto state_err;
                }
                // 4. download bootloader
                ret = exynos_process(args);
                if (ret < 0) {
                    cbd_err("exynos_process\n");
                    cbd_logcat_err("Err:exynos_process\n");
                    goto state_err;
                }
#ifdef LINK_MAIN_PCIE
                ret = modem_register_pcie(args->fds.fd_boot);
#endif
                if (args->bmode == BOOTMODE_DUMP)
                    args->state = STATE_DUMP;
                else
                    args->state = STATE_FIRMWARE;
                break;

            case STATE_FIRMWARE:
                cbd_info("+STATE_FIRMWARE\n");
                // 1. send bin
                ret = exynos_process(args);
                if (ret < 0) {
                    cbd_err("exynos_process\n");
                    cbd_logcat_err("Err:exynos_process\n");
                    goto state_err;
                }
                // 2. boot off
                ret = modem_boot_off(args->fds.fd_boot);
                if (ret < 0) {
                    cbd_err("boot off\n");
                    cbd_logcat_err("Err:boot off\n");
                    goto state_err;
                }
                // 3. boot done
                ret = modem_boot_done(args->fds.fd_boot);
                if (ret < 0) {
                    cbd_err("boot done\n");
                    cbd_logcat_err("Err:boot done\n");
                    goto state_err;
                }
                cbd_info("CP booting successfully finished!!!\n\n");
                args->state = STATE_MONITORING;
                break;

            case STATE_DUMP:
                cbd_info("+STATE_DUMP\n");

                crash_handling_mode = get_crash_handling_mode();

                if (args->bmode == BOOTMODE_NORMAL) {
                    ret = modem_dump_reset(args->fds.fd_boot);
                    if (ret < 0) {
                        cbd_err("dump reset\n");
                        cbd_logcat_err("Err:dump reset\n");
                        goto state_err;
                    }
                    args->bmode = BOOTMODE_DUMP;
                    args->state = STATE_BOOTLOADER;
                } else {
                    // do the dump sequence
                    ret = dump_process(args, crash_handling_mode);
                    if (ret < 0) {
                        cbd_err("ERR!! Retry dump process\n");
                        cbd_logcat_err("Err: Retry dump process\n");
                        args->bmode = BOOTMODE_NORMAL;
                        args->state = STATE_DUMP;
                    } else {
                        // dump succeeded and didn't cause kernel panic
                        if (crash_handling_mode == CRASH_MODE_DUMP_SILENT_RESET) {
                            exit = 1;
                        }
                    }
                }
                break;

            case STATE_RECOVERY:
                cbd_info("+STATE_RECOVERY\n");
                if (recovery_retry-- <= 0) {
			exit = 1;
			break;
		}

                NV_init_seek(args);
                modem_reset(args->fds.fd_boot);
#if defined(SS310) //download modem image again
                sec_cp_init(args->fds.fd_boot);
                ret = prepare_download_control(args);
                if(ret < 0) {
                    cbd_err("prepare_download_control failed!!!\n");
                    return ret;
                }
#endif
                args->state = STATE_INIT;
                break;

            case STATE_MONITORING:
                cbd_info("+STATE_MONITORING\n");
                cbd_logcat_debug("MODEM BOOT DONE!!!\n");
                cbd_logcat_debug("+STATE_MONITORING\n");

                ret = modem_monitoring(args);

                if (ret < 0) {
                    cbd_err("error modem_monitoring\n");
                    cbd_logcat_err("Err:modem_monitoring\n");
                } else {
                    if(ret == STATE_CRASH_RESET) {
                        store_srinfo(node.rat, args->fds.fd_dev);
                        restore_srinfo(node.rat, args->fds.fd_dev);
                        exit = 1;  // go to silent reset
                    } else if (ret == STATE_CRASH_EXIT || ret == STATE_CRASH_WATCHDOG) {
                        pre_dump_process(args);
                        args->state = STATE_DUMP;
                    }
                }
                break;

            state_err:
            case STATE_ERR:
                cbd_err("state err\n");
                cbd_logcat_err("state err\n");
                args->state = STATE_RECOVERY;

		if (modem_status(args->fds.fd_boot) == STATE_CRASH_EXIT || modem_status(args->fds.fd_boot) == STATE_CRASH_WATCHDOG) {
			cbd_err("CP crash is observed during CP boot-up\n");
			args->state = STATE_DUMP;
		}

                break;
        }

    } while (!exit);

    ret = modem_off(args->fds.fd_boot);
    if (ret < 0) {
        cbd_err("modem off\n");
        cbd_logcat_err("Err:modem off\n");
    }
    ret = sec_cp_init(args->fds.fd_boot);
    if (ret < 0) {
        cbd_err("cp init\n");
        cbd_logcat_err("cp init\n");
    }

    if (args->fds.fd_boot)
        close(args->fds.fd_boot);

    /* For a controlled exit and restart, reset rild and wait for
     * the rild service restart to restart cbd service. */
    property_set("vendor.sys.rild_reset", "1");
    sleep(10);

    return ret;
}
