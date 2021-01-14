/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __CBD_COMMON_H__
#define __CBD_COMMON_H__

//Modem type
#if defined(SS310)
#define DEFAULT_MODEM MODEM_SS310
#elif defined(SS300)
#define DEFAULT_MODEM MODEM_SS300
#else
#error "define modem type"
#endif

//BOOT link
#if defined(LINK_BOOT_SPI)
#define DEFAULT_BOOT_LINK LINKDEV_SPI
#elif defined(LINK_BOOT_SHMEM)
#define DEFAULT_BOOT_LINK LINKDEV_SHMEM
#else
#error "define link boot"
#endif

//Main link
#if defined(LINK_MAIN_LLI)
#define DEFAULT_MAIN_LINK LINKDEV_LLI
#elif defined(LINK_MAIN_SHMEM)
#define DEFAULT_MAIN_LINK LINKDEV_SHMEM
#elif defined(LINK_MAIN_PCIE)
#define DEFAULT_MAIN_LINK LINKDEV_PCIE
#else
#error "define link device"
#endif

extern int modem_manager();
extern int wait_event(int fd, short events, long timeout);
extern int send_cmd(struct modem_args *in_args, u32 cmd, u32 toc, u32 stat);
extern struct modem_node node;

#endif  //__CBD_COMMON_H__
