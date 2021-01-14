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

void DL_CTRL(struct modem_args *args, int image_type, int stage,
                 int start_req, int data_available, int security_check, int finish_req)
{
    struct modem_args *private_args = args;

    private_args->ctrl[stage].stage = stage;
    if (private_args->nv_ver == NV_V2) {
        if (stage == NV_NORM_STAGE)
            private_args->ctrl[stage].fd = private_args->fds.fd_nv_norm;
        else if (stage == NV_PROT_STAGE)
            private_args->ctrl[stage].fd = private_args->fds.fd_nv_prot;
        else
            private_args->ctrl[stage].fd = private_args->fds.fd_bin;
    } else {
        if (stage == NV_STAGE)
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
    private_args->ctrl[stage].size = private_args->toc[image_type].size;
    if (security_check == CRC_CHECK)
        private_args->ctrl[stage].crc = private_args->toc[image_type].crc;
    else
        private_args->ctrl[stage].crc = 0;

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
    struct modem_args *private_args = args;
    int i;

    if (private_args->modem.type == MODEM_SS300)
        private_args->ops.send_bootloader = send_bootloader_ss300;

    args->ops.full_dump_from_kernel = full_dump_from_kernel;

    cbd_info("num_stages:%d\n", args->num_stages);
    cbd_info("nv_ver:%d\n", args->nv_ver);

    for (i = 0; i < args->num_stages; i++) {
        /* TOC */
        if (strcmp(args->toc[i].name, "TOC") == 0) {
            args->ctrl[TOC_STAGE].stage = TOC_STAGE;
            args->ctrl[TOC_STAGE].data_available = YES;
            args->ctrl[TOC_STAGE].start_req = YES;
            args->ctrl[TOC_STAGE].finish_req = YES;
            args->ctrl[TOC_STAGE].security_check = NO;
            args->ctrl[TOC_STAGE].fd = args->fds.fd_bin;
            args->ctrl[TOC_STAGE].offset = args->toc[i].b_offset;
            args->ctrl[TOC_STAGE].m_offset = args->toc[i].m_offset;
            args->ctrl[TOC_STAGE].size = args->toc[i].size;
            args->ctrl[TOC_STAGE].crc = args->toc[i].crc;
            continue;
        }

        /*
         * During CP download with PCIE interface,
         * AP should download CP boot binary via SPI and
         * AP should download the other binaries via PCIE.
         * So CP boot should be separated with others
         */
        /* BOOT */
        if (strcmp(args->toc[i].name, "BOOT") == 0) {
            args->ctrl[BOOT_STAGE].stage = BOOT_STAGE;
            args->ctrl[BOOT_STAGE].data_available = NO;
            args->ctrl[BOOT_STAGE].start_req = NO;
            args->ctrl[BOOT_STAGE].finish_req = YES;
            args->ctrl[BOOT_STAGE].security_check = SECURITY_CHECK;
            args->ctrl[BOOT_STAGE].fd = args->fds.fd_bin;
            args->ctrl[BOOT_STAGE].offset = args->toc[i].b_offset;
            args->ctrl[BOOT_STAGE].m_offset = args->toc[i].m_offset;
            args->ctrl[BOOT_STAGE].size = args->toc[i].size;
            args->ctrl[BOOT_STAGE].crc = args->toc[i].crc;
            continue;
        }

        /* MAIN */
        if (strcmp(args->toc[i].name, "MAIN") == 0)
            args->ctrl[i].security_check = CRC_CHECK;

	/* Set file descriptor */
        if (args->nv_ver == NV_V2) {
            if (strcmp(args->toc[i].name, "NV_NORM") == 0)
                args->ctrl[i].fd = args->fds.fd_nv_norm;
            else if (strcmp(args->toc[i].name, "NV_PROT") == 0)
                args->ctrl[i].fd = args->fds.fd_nv_prot;
            else
                args->ctrl[i].fd = args->fds.fd_bin;
        } else {
            if (strcmp(args->toc[i].name, "NV") == 0)
                args->ctrl[i].fd = args->fds.fd_nv;
            else
                args->ctrl[i].fd = args->fds.fd_bin;
        }

        args->ctrl[i].stage = i;
        args->ctrl[i].data_available = YES;
        args->ctrl[i].start_req = YES;
        args->ctrl[i].finish_req = YES;
        args->ctrl[i].offset = args->toc[i].b_offset;
        args->ctrl[i].m_offset = args->toc[i].m_offset;
        args->ctrl[i].size = args->toc[i].size;
        args->ctrl[i].crc = args->toc[i].crc;
    }
    args->ctrl[i].stage = FIN_STAGE;
    args->ctrl[i].start_req = YES;
    args->ctrl[i].finish_req = NO;

    return TRUE;
}

/******************************************************************************
 * send_bootloader_spi
 * description : read TOC[BOOT_STAGE] + data and send via SPI
 ******************************************************************************/

int send_bootloader_ss300(void *in_args)
{
    struct modem_args *args = (struct modem_args *)in_args;
    struct download_stage_control *ctrl = &args->ctrl[BOOT_STAGE];
    struct modem_data bootloader;
    int ret = 0;

    // 1. malloc bootload size
    bootloader.size = ctrl->size;
    bootloader.data = malloc(bootloader.size);
    if (bootloader.data == NULL) {
        cbd_err("1. bootloader malloc fail\n");
        goto exit;
    }

    cbd_info("size:%d\n", bootloader.size);

    ctrl->fd = args->fds.fd_bin;
    if (!bootloader.data) {
        cbd_err("1. bootloader malloc fail\n");
        goto exit;
    }

    // 2. lseek bootloader
    ret = lseek(ctrl->fd, ctrl->offset, SEEK_SET);
    if (ret < 0) {
        cbd_err("2. lseek fail (%s)\n", ERR2STR);
        goto exit;
    }

    // 3. read bootloader
    ret = read(ctrl->fd, bootloader.data, bootloader.size);
    if (ret < 0) {
        cbd_err("3. read fail (%s)\n", ERR2STR);
        goto exit;
    }

    // 4. check read size
    if ((u32)ret != bootloader.size) {
        cbd_err("4. read size fail!! want:%d real:%d\n", bootloader.size, ret);
        ret = -EFAULT;
        goto exit;
    }

    // 5. send bootloader
    ret = modem_send_data(args->fds.fd_spi, &bootloader);
    if (ret < 0) {
        cbd_err("5. IOCTL_MODEM_XMIT_BOOT fail (%s)\n", ERR2STR);
        goto exit;
    }

exit:
        free(bootloader.data);
    return ret;
}
