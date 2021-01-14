#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/stat.h>

#include "cbd_header.h"
#include "util_srinfo.h"

static char tochar(char x)
{
	return (x > 0x21 && x < 0x7E) ? x : '.';
}

void print_srinfo(struct shmem_srinfo *rdbuf, unsigned size) {
	char timestr[MAX_SUFFIX_LEN], ascii[17];
	time_t now;
	struct tm result;
	unsigned i, linelen = 16;

	time(&now);
	localtime_r(&now, &result);
	strftime(timestr, MAX_SUFFIX_LEN, "%Y-%m-%d %H:%M:%S", &result);
	cbd_info("[%s]\n", timestr);

	cbd_info("store to hex ascii text size=(0x%x)\n", (unsigned)size);

	for (i = 0; i < size; i += linelen) {
		unsigned j;
		char *rp = (rdbuf->buf) + i;
		int *rpr = (int *)rp;


		for (j = 0; j < linelen; j++)
			ascii[j] = tochar(*(rp + j));
		ascii[16] = '\0';

		cbd_info("%04x:%08x %08x %08x %08x %s\n", i, *rpr,
				*(rpr + 1), *(rpr + 2), *(rpr + 3), ascii);
	}
	cbd_info("\n");
	cbd_info("srinfo print done\n");
}

static struct shmem_srinfo *alloc_srinfo(unsigned *len, int boot_fd)
{
	struct shmem_srinfo *args;
	int ret;

	args = (struct shmem_srinfo *)malloc(SRINFO_READ_SIZE);
	if (!args) {
		cbd_info("read buf alloc(%d) fail\n", SRINFO_READ_SIZE);
		return NULL;
	}
	memset(args, 0x00, SRINFO_READ_SIZE);
	args->size = SRINFO_READ_SIZE - sizeof(unsigned);

	ret = ioctl(boot_fd, IOCTL_MODEM_GET_SHMEM_SRINFO, args);
	if (ret < 0) {
		cbd_err("ioctl fail - Get srinfo(%d)\n", ret);
		goto exit;
	}

	*len = args->size;

	return args;
exit:
	free(args);
	return NULL;
}

void store_srinfo(char *surfix, int dev_fd)
{
	unsigned size;
	struct shmem_srinfo *rdbuf;

	rdbuf = alloc_srinfo(&size, dev_fd);
	if (!rdbuf) {
		cbd_err("alloc srinfo fail\n");
		return;
	}

	print_srinfo(rdbuf, size);
}

void restore_srinfo(char *surfix, int dev_fd)
{
	int ret;
	char *rdbuf;
	struct shmem_srinfo *sr_args;

	rdbuf = malloc(SRINFO_READ_SIZE);
	if (!rdbuf) {
		cbd_err("read buf alloc(%d) fail\n", SRINFO_READ_SIZE);
		return;
	}
	memset(rdbuf, 0x00, SRINFO_READ_SIZE);

	sr_args = (struct shmem_srinfo *)rdbuf;
	sr_args->size = SRINFO_READ_SIZE;

	ret = ioctl(dev_fd, IOCTL_MODEM_SET_SHMEM_SRINFO, sr_args);
	if (ret < 0) {
		cbd_err("ioctl fail - Set srinfo(%d)\n", ret);
		goto exit_free;
	}

exit_free:
	free(rdbuf);
}
