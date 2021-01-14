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

int kmsg_fd = STDOUT_FILENO;
char *modem_partition_path;
#define kprintf(fmt) dprintf(kmsg_fd, fmt)

#ifdef DEBUG_KERNEL_MSG
int get_kmsg_fd(void)
{
    return kmsg_fd;
}

static void kprintf_init(void)
{
    char *name = "/dev/kmsg";
    kmsg_fd = open(name, O_RDWR);
}

static void kprintf_deinit(void)
{
    if (kmsg_fd != STDOUT_FILENO)
        close(kmsg_fd);
}
#else
#define kprintf_init() { do {} while(0); }
#define kprintf_deinit() { do {} while(0); }
#endif

#define LOG_BUF_SIZE 128
static char dprintf_buf[LOG_BUF_SIZE];
int dprintf(int fd, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vsnprintf(dprintf_buf, LOG_BUF_SIZE, format, ap);
    dprintf_buf[LOG_BUF_SIZE - 1] = 0;
    va_end(ap);
    return write(fd, dprintf_buf, strlen(dprintf_buf));
}

int main(int argc, char *argv[])
{
    int ret;
    int opt;
    kprintf_init();
    cbd_info("*** CBD start ***\n");

	while(1) {
		opt = getopt(argc, argv, "hdt:b:m:n:o:p:P:c:");

		if (opt == -1)
			break;

		switch (opt) {
		case 'p':
			node.path_bin = optarg;
			cbd_info("partition path : %s\n", node.path_bin);
			break;
		default:
			cbd_info("WARNING! invalid option %c\n", opt);
                        break;
		}

	}
    ret = modem_manager();
    if (ret < 0) {
        cbd_err("Exit CBD, BOOT failed!!!\n");
        kprintf_deinit();
    }
    return ret;
}
