/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef __DIT_IOCTL_H__
#define __DIT_IOCTL_H__
#include <uapi/linux/if.h>
#define OFFLOAD_DEV_NAME "dit"
#define OFFLOAD_IOC_MAGIC ('D')
#define OFFLOAD_IOCTL_INIT_OFFLOAD _IO(OFFLOAD_IOC_MAGIC, 0x00)
#define OFFLOAD_IOCTL_STOP_OFFLOAD _IO(OFFLOAD_IOC_MAGIC, 0x01)
#define OFFLOAD_IOCTL_SET_LOCAL_PRFIX _IO(OFFLOAD_IOC_MAGIC, 0x02)
#define OFFLOAD_IOCTL_GET_FORWD_STATS _IO(OFFLOAD_IOC_MAGIC, 0x03)
#define OFFLOAD_IOCTL_SET_DATA_LIMIT _IO(OFFLOAD_IOC_MAGIC, 0x04)
#define OFFLOAD_IOCTL_SET_UPSTRM_PARAM _IO(OFFLOAD_IOC_MAGIC, 0x05)
#define OFFLOAD_IOCTL_ADD_DOWNSTREAM _IO(OFFLOAD_IOC_MAGIC, 0x06)
#define OFFLOAD_IOCTL_REMOVE_DOWNSTRM _IO(OFFLOAD_IOC_MAGIC, 0x07)
#define OFFLOAD_IOCTL_CONF_SET_HANDLES _IO(OFFLOAD_IOC_MAGIC, 0x10)
enum offload_cb_event {
  OFFLOAD_STARTED = 1,
  OFFLOAD_STOPPED_ERROR = 2,
  OFFLOAD_STOPPED_UNSUPPORTED = 3,
  OFFLOAD_SUPPORT_AVAILABLE = 4,
  OFFLOAD_STOPPED_LIMIT_REACHED = 5,
};
struct forward_stat {
  uint64_t rxBytes;
  uint64_t txBytes;
};
struct iface_info {
  char iface[IFNAMSIZ];
  char prefix[48];
};
struct dit_cb_event {
  int32_t cb_event;
};
#endif
