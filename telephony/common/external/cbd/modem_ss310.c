/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "cbd_header.h"

#define PROP_VSS_BOOT_DONE    "vendor.ril.cbd.vss_boot_done"

static int boot_stage;

void DL_CTRL(struct modem_args *args, int image_type, int stage,
                 int start_req, int data_available, int security_check, int finish_req)
{
    struct modem_args *private_args = args;

    private_args->ctrl[stage].stage = stage;
    if (private_args->nv_ver == NV_V2) {
        if(stage == NV_NORM_STAGE)
            private_args->ctrl[stage].fd = private_args->fds.fd_nv_norm;
        else if(stage == NV_PROT_STAGE)
            private_args->ctrl[stage].fd = private_args->fds.fd_nv_prot;
        else
            private_args->ctrl[stage].fd = private_args->fds.fd_bin;
    } else {
        if(stage == NV_STAGE)
            private_args->ctrl[stage].fd = private_args->fds.fd_nv;
        else
            private_args->ctrl[stage].fd = private_args->fds.fd_bin;
    }
    private_args->ctrl[stage].start_req = start_req;
    if (stage == FIN_STAGE)
        return;
    private_args->ctrl[stage].data_available = data_available;
    private_args->ctrl[stage].security_check = security_check;
    private_args->ctrl[stage].finish_req = finish_req;
    private_args->ctrl[stage].offset = private_args->toc[image_type].b_offset;
    private_args->ctrl[stage].m_offset = private_args->toc[image_type].m_offset;
    private_args->ctrl[stage].size = private_args->toc[image_type].size;
    if (security_check == CRC_CHECK)
        private_args->ctrl[stage].crc = private_args->toc[image_type].crc;
    else
        private_args->ctrl[stage].crc = 0;
}



/******************************************************************************
 * xmit_bin
 * description :
 * 1. copy image to CP mem
 *
 ******************************************************************************/
int xmit_bin(void *in_args, int stage)
{
    struct modem_args *args = (struct modem_args *)in_args;
    struct download_stage_control *ctrl = &args->ctrl[stage];
    struct data_info binary;
    int last = 0;
    int ret = 0;
    void * p;

    p = malloc(EXYNOS_PAYLOAD_LEN);
    if (p == NULL) {
        ret = -ENOMEM;
        cbd_err("1. malloc fail (%s)\n", ERR2STR);
        goto exit;
    }

    if(sizeof(p)==4) { /* 32bit machine */
		binary.buff[0] = (u32)p;
		binary.buff[1] = (u32)0x0;
    }
    else { /* 64bit machine */
		binary.buff[0] = (unsigned int)p;
		binary.buff[1] = (0xffffffff00000000&((unsigned long long)p))>>32;
    }

    binary.stage = stage;
    binary.total_size = ctrl->size;
    binary.m_offset = ctrl->m_offset;
    binary.len = EXYNOS_PAYLOAD_LEN;
    binary.offset = 0;

#ifdef VERBOSE_DEBUG_ENABLE
    cbd_info("*** %d*** [0x%x] \n", stage, ctrl->offset);
#endif

    if (ctrl->offset > 0) {
        ret = lseek(ctrl->fd, ctrl->offset, SEEK_SET);
        if (ret < 0) {
            cbd_err("2. lseek fail (%s)\n", ERR2STR);
            goto exit;
        }
    }

    while (1) {
        if (binary.total_size == binary.offset)
            break;

        if ((binary.total_size - binary.offset) < EXYNOS_PAYLOAD_LEN) {
            binary.len = binary.total_size - binary.offset;
            last = 1;
        }

        ret = read(ctrl->fd, p, binary.len);
        if (ret < 0) {
            cbd_err("3. read fail (%s)\n", ERR2STR);
            goto exit;
        }

        ret = modem_xmit_bin(args->fds.fd_boot, &binary);
        if (ret < 0) {
            cbd_err("4. modem_xmit_bin (%s)\n", ERR2STR);
            goto exit;
        }
        if (last == 1)
            break;
        binary.offset += EXYNOS_PAYLOAD_LEN;
        binary.m_offset += binary.len;
    }

exit:
    free(p);
    return ret;

}

/******************************************************************************
 * prepare_download_control
 * description :
 * 1. prepare : download control
 * 2. copy bootloader
 * 3. copy main and NV
 * 4. set secure mode
 * 5. request security check
 *
 ******************************************************************************/
static int send_bootloader_ss310(void *in_args)
{
    struct modem_args *args = in_args;
    int ret;

    ret = xmit_bin(args, boot_stage);
    if (ret < 0) {
        cbd_err("send bootloader failed!!!\n");
    }
    return ret;
}

static int full_dump_from_kernel(void *in_args, char *from)
{
    int ret = 0;
    int dev_fd;
    unsigned long mem_size, copied = 0;
    char buff[PAGE_SIZE];
    int dump_fd = -1;
    char path[MAX_PATH_LEN];
    char suffix[MAX_SUFFIX_LEN];
    time_t now;
    struct tm result;
    int cmd = 0;

    struct modem_args *args = in_args;

    if (strncmp("acpm", from, 4) == 0)
        cmd = IOCTL_ACPM_FULL_DUMP;
    else if (strncmp("vss", from, 3) == 0)
        cmd = IOCTL_VSS_FULL_DUMP;
    else if (strncmp("shmem", from, 5) == 0)
        cmd = IOCTL_SHMEM_FULL_DUMP;
    else {
        cbd_err("ERR! it does not dump support from %s\n", from);
        goto exit;
    }

    dev_fd = args->fds.fd_dev;

    /* Get shared memory size and Trigger mem dump */
    ret = ioctl(dev_fd, cmd, &mem_size);
    if (ret < 0) {
	if (cmd == IOCTL_VSS_FULL_DUMP || cmd == IOCTL_ACPM_FULL_DUMP)
		return 0;

        cbd_err("ERR! ioctl fail (%s)\n", ERR2STR);
        return -EINVAL;
    }

    /* Open (create) a shared memory dump file */
    time(&now);
    localtime_r(&now, &result);
    strftime(suffix, MAX_SUFFIX_LEN, "%Y%m%d-%H%M", &result);
    sprintf(path, "%s/cpcrash_%s_dump_%s.log", LOG_DIR, from, suffix);

    cbd_info("%s(%s):: file: %s, size: %lu\n", __func__, from, path, mem_size);

    dump_fd = open(path, O_WRONLY | O_CREAT,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (dump_fd < 0) {
        cbd_err("ERR! %s open fail (%s)\n", path, ERR2STR);
        return -EINVAL;
    }

    cbd_info("%s(%s):: %s opened (fd %d)\n", __func__, from, path, dump_fd);

    /* Read & Save shared memory dump */
    while (copied < mem_size) {
        ret = wait_event(dev_fd, POLLIN, TIMEOUT);
        if (ret < 0) {
            cbd_err("wait event fail!!\n");
            goto exit;
        }

        /* Receive a DUMP frame from CP */
        ret = read(dev_fd, buff, sizeof(buff));
        if (ret < 0) {
            cbd_err("ERR! DUMP read fail (%s)\n", ERR2STR);
            goto exit;
        }

        /* not verified */
        copied += ret;

        ret = write(dump_fd, buff, ret);
        if (ret < 0) {
            cbd_err("ERR! mem, write fail (%s)\n", ERR2STR);
            goto exit;
        }
    }

    if (fsync(dump_fd))
	    cbd_err("ERR! fsync is failed (%s)\n", ERR2STR);

    cbd_info("%s(%s):: complete! (%lu bytes)\n", __func__, from, copied);
exit:
    if (dump_fd >= 0)
        close(dump_fd);

    return ret;
}

int prepare_download_control(struct modem_args *args)
{
    struct sec_info info;
    int ret;
    int i;
    int count;
    int vss_stage = 0;
    char prop_buf[PROPERTY_VALUE_MAX] = {0, };

    property_get(PROP_VSS_BOOT_DONE, prop_buf, "0");

    args->ops.send_bootloader = send_bootloader_ss310;
    args->ops.full_dump_from_kernel = full_dump_from_kernel;

    /* Remove "TOC" element */
    count = (int)(args->num_stages) - 1;

    for (i = 0; i< count; i++) {
        args->ctrl[i].stage = i;

        if (strcmp(args->toc[i+1].name, "BOOT") == 0)
            boot_stage = i;

        if (strcmp(args->toc[i+1].name, "VSS") == 0)
            vss_stage = i;

        if (args->nv_ver == NV_V2) {
            if (strcmp(args->toc[i+1].name, "NV_NORM") == 0)
                args->ctrl[i].fd = args->fds.fd_nv_norm;
            else if (strcmp(args->toc[i+1].name, "NV_PROT") == 0)
                args->ctrl[i].fd = args->fds.fd_nv_prot;
            else
                args->ctrl[i].fd = args->fds.fd_bin;
        } else {
            if (strcmp(args->toc[i+1].name, "NV") == 0)
                args->ctrl[i].fd = args->fds.fd_nv;
            else
                args->ctrl[i].fd = args->fds.fd_bin;
        }

        args->ctrl[i].data_available = NO;
        args->ctrl[i].security_check = NO;
        args->ctrl[i].finish_req = NO;
        args->ctrl[i].offset = args->toc[i+1].b_offset;
        args->ctrl[i].m_offset = (args->toc[i+1].m_offset & CP_MEM_SIZE_MASK);
        args->ctrl[i].size = args->toc[i+1].size;
        args->ctrl[i].crc = args->toc[i+1].crc;

        cbd_info("stage=%u, name:%s b_off=%u, m_offset=%u b_size=%u\n",
            args->ctrl[i].stage, args->toc[i+1].name, args->ctrl[i].offset,
            args->ctrl[i].m_offset, args->ctrl[i].size);

    }
    args->ctrl[i].start_req = YES;
    args->ctrl[i].stage = FIN_STAGE;

    for (i = 0; i < count; i++) {
        if (i == vss_stage && vss_stage != 0) {
            if (prop_buf[0] == '1') {
                cbd_info("stage[%d] : VSS stage is already done\n", i);
                continue;
            }
        }
        cbd_info("xmit_bin: stage=%u, name:%s b_off=%u, m_offset=%u b_size=%u\n",
            args->ctrl[i].stage, args->toc[i+1].name, args->ctrl[i].offset,
            args->ctrl[i].m_offset, args->ctrl[i].size);
        ret = xmit_bin(args, i);
        if (ret < 0) {
            cbd_err("xmit_bin failed!!! stage:%d name:%s\n", i, args->toc[i+1].name);
            return ret;
        }
    }
    property_set(PROP_VSS_BOOT_DONE, "1");

    info.bmode = BOOTMODE_NORMAL;
    info.boot_size = args->toc[IMG_BOOT].size;
    info.main_size = args->toc[IMG_MAIN].size;
    ret = modem_request_security(args->fds.fd_boot, &info);
    if (ret < 0) {
        cbd_err("modem_request_security failed!!!\n");
        return ret;
    }
    return ret;
}
