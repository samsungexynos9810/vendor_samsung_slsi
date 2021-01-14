#ifndef __UTIL_SRINFO_H__
#define __UTIL_SRINFO_H__

#define SRINFO_READ_SIZE 0x400 /* 1KB*/

struct shmem_srinfo {
	unsigned size;
	char buf[0];
};

void store_srinfo(char *surfix, int fd);
void restore_srinfo(char *surfix, int fd);

#endif
