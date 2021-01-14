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

int modem_on(int fd_boot, int sim_slot_cnt)
{
    int ret = 0;
    int sim_val = sim_slot_cnt;

    if (sim_val) {
	ret = ioctl(fd_boot, IOCTL_MODEM_ON, &sim_val);
    } else {
	ret = ioctl(fd_boot, IOCTL_MODEM_ON, NULL);
    }

    if (ret < 0) {
        cbd_err("IOCTL_MODEM_ON\n");
    }

    return ret;
}

int modem_off(int fd_boot)
{
    int ret = 0;
    ret = ioctl(fd_boot, IOCTL_MODEM_OFF, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_OFF\n");
    }

    return ret;
}

int modem_reset(int fd_boot)
{
    int ret = 0;
    ret = ioctl(fd_boot, IOCTL_MODEM_RESET, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_RESET\n");
    }

    return ret;
}

int modem_boot_on(int fd_boot)
{
    int ret = 0;
    ret = ioctl(fd_boot, IOCTL_MODEM_BOOT_ON, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_BOOT_ON\n");
    }

    return ret;
}

int modem_boot_off(int fd_boot)
{
    int ret = 0;
    ret = ioctl(fd_boot, IOCTL_MODEM_BOOT_OFF, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_BOOT_OFF (%s)\n", ERR2STR);
    }

    return ret;
}

int modem_boot_done(int fd_boot)
{
    int ret = 0;
    ret = ioctl(fd_boot, IOCTL_MODEM_BOOT_DONE, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_BOOT_DONE\n");
    }

    return ret;
}

int modem_status(int fd_boot)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_MODEM_STATUS, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_STATUS\n");
    }

    return ret;
}

int modem_start_download(int fd_boot)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_MODEM_DL_START, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_DL_START\n");
    }

    return ret;
}

int modem_register_pcie(int fd_boot)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_REGISTER_PCIE, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_DL_START\n");
    }

    return ret;
}

int modem_dump_start(int fd_boot)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_MODEM_DUMP_START, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_DUMP_START\n");
    }

    return ret;
}

int modem_dump_reset(int fd_boot)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_MODEM_DUMP_RESET, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_DUMP_RESET\n");
    }

    return ret;
}

int modem_force_crash_exit(int fd_boot)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_MODEM_FORCE_CRASH_EXIT, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_FORCE_CRASH_EXIT\n");
    }

    return ret;
}

int modem_cp_upload(int fd_boot, const char *pStr)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_MODEM_CP_UPLOAD, pStr);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_CP_UPLOAD\n");
    }

    return ret;
}

int modem_send_data(int fd_spi, struct modem_data *pImage)
{
    int ret = 0;

    ret = ioctl(fd_spi, IOCTL_MODEM_XMIT_BOOT, pImage);
    if (ret < 0)
        cbd_err("IOCTL_MODEM_XMIT_BOOT\n");

    return ret;
}

int modem_shutdown(int fd_boot)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_MODEM_OFF, NULL);
    if (ret < 0) {
        cbd_err("IOCTL_MODEM_OFF\n");
    }

    return ret;
}

/*****************************************************************************
* CBD version2
******************************************************************************/
int sec_cp_init(int fd_boot)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_SEC_CP_INIT, BOOTMODE_COLD_RESET);
    if (ret < 0) {
        cbd_err("IOCTL_SEC_CP_INIT\n");
    }

    return ret;

}
int modem_request_security(int fd_boot, struct sec_info *info)
{
    int ret = 0;

    ret = ioctl(fd_boot, IOCTL_CHECK_SECURITY, info);
    if (ret < 0) {
        cbd_err("IOCTL_CHECK_SECURITY\n");
    }

    return ret;
}

int modem_xmit_bin(int fd_boot, struct data_info *datainfo)
{
    int ret = 0;
    struct data_info *info = datainfo;

    ret = ioctl(fd_boot, IOCTL_XMIT_BIN, info);
    if (ret < 0) {
        cbd_err("IOCTL_XMIT_BIN\n");
    }

    return ret;
}
