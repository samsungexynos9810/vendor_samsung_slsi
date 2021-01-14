/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __CBD_IOCTL_H__
#define __CBD_IOCTL_H__

#define IOCTL_MODEM_ON                  _IO('o', 0x19)
#define IOCTL_MODEM_OFF                 _IO('o', 0x20)
#define IOCTL_MODEM_RESET               _IO('o', 0x21)
#define IOCTL_MODEM_BOOT_ON             _IO('o', 0x22)
#define IOCTL_MODEM_BOOT_OFF            _IO('o', 0x23)
#define IOCTL_MODEM_BOOT_DONE           _IO('o', 0x24)
#define IOCTL_MODEM_STATUS              _IO('o', 0x27)
#define IOCTL_MODEM_DL_START            _IO('o', 0x28)
#define IOCTL_MODEM_FW_UPDATE           _IO('o', 0x29)
#define IOCTL_MODEM_DUMP_START          _IO('o', 0x32)
#define IOCTL_MODEM_DUMP_UPDATE         _IO('o', 0x33)
#define IOCTL_MODEM_FORCE_CRASH_EXIT    _IO('o', 0x34)
#define IOCTL_MODEM_CP_UPLOAD           _IO('o', 0x35)
#define IOCTL_MODEM_DUMP_RESET          _IO('o', 0x36)
#define IOCTL_MODEM_SWITCH_MODEM        _IO('o', 0x37)
#define IOCTL_MODEM_SET_AP_STATE        _IO('o', 0x38)
#define IOCTL_MODEM_CLEAR_AP_STATE      _IO('o', 0x39)
#define IOCTL_MODEM_GET_CP_STATE        _IO('o', 0x3A)
#define IOCTL_LINK_CONNECTED            _IO('o', 0x33)
#define IOCTL_LINK_PORT_ON              _IO('o', 0x35)
#define IOCTL_LINK_PORT_OFF             _IO('o', 0x36)
#define IOCTL_MODEM_SET_TX_LINK         _IO('o', 0x37)
#define IOCTL_MODEM_XMIT_BOOT           _IO('o', 0x40)
#define IOCTL_DPRAM_INIT_STATUS         _IO('o', 0x43)
#define IOCTL_LINK_DEVICE_RESET         _IO('o', 0x44)
#define IOCTL_MODEM_GET_SHMEM_SRINFO	_IO('o', 0x45)
#define IOCTL_MODEM_SET_SHMEM_SRINFO	_IO('o', 0x46)
#define IOCTL_MIF_LOG_DUMP              _IO('o', 0x51)
#define IOCTL_MIF_DPRAM_DUMP            _IO('o', 0x52)
#define IOCTL_SHMEM_FULL_DUMP           _IO('o', 0x54)
#define IOCTL_VSS_FULL_DUMP             _IO('o', 0x57)
#define IOCTL_ACPM_FULL_DUMP            _IO('o', 0x58)
#define IOCTL_SEC_CP_INIT               _IO('o', 0x61)
#define IOCTL_CHECK_SECURITY            _IO('o', 0x62)
#define IOCTL_XMIT_BIN                  _IO('o', 0x63)
#define IOCTL_MODEM_CRASH_REASON        _IO('o', 0x64)  /* Get Crash Reason */
#define IOCTL_REGISTER_PCIE	_IO('o', 0x65)  /* Register PCIe device */

extern int modem_on(int fd_boot, int sim_slot_cnt);
extern int modem_off(int fd_boot);
extern int modem_reset(int fd_boot);
extern int modem_boot_on(int fd_boot);
extern int modem_boot_off(int fd_boot);
extern int modem_boot_done(int fd_boot);
extern int modem_status(int fd_boot);
extern int modem_start_download(int fd_boot);
extern int modem_dump_start(int fd_boot);
extern int modem_dump_reset(int fd_boot);
extern int modem_force_crash_exit(int fd_boot);
extern int modem_cp_upload(int fd_boot, const char *pStr);
extern int modem_send_data(int fd_boot, struct modem_data *pImage);
extern int modem_shutdown(int fd_boot);
extern int sec_cp_init(int fd_boot);
extern int modem_request_security(int fd_boot, struct sec_info *secinfo);
extern int modem_xmit_bin(int fd_boot, struct data_info *datainfo);
extern int modem_register_pcie(int fd_boot);

#endif //__CBD_IOCTL_H__
