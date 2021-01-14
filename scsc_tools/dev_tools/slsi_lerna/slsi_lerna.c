/****************************************************************************
 *
 * Copyright (c) 2019 Samsung Electronics Co., Ltd. All rights reserved
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdint.h>

#include <string.h>
#include <errno.h>

#include <getopt.h>
#include <stdlib.h>  /* For abort(), exit(), etc. */

#include <unistd.h>
#include <fcntl.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
static const char version_string[] = "v" STR(VERSION_MAJOR) "." STR(VERSION_MINOR);

static int is_verbose = 0;

struct lerna_cmd_header {
	uint8_t magic_number;
	uint8_t cid;
	uint16_t length;
	uint16_t psid;
	uint8_t row_index;
	uint8_t group_index;
};

struct lerna_cmd_header header;

struct lerna_vldata_block {
	uint16_t psid;
	uint16_t length;
	uint8_t data[1024];  /* Arbitrary maximum length, not likely to be overwritten. */
};

struct lerna_cmd {
	struct lerna_cmd_header header;
	struct lerna_vldata_block vldata;
};
struct lerna_cmd command;


struct lerna_cmd_response {
	struct lerna_cmd_header header;
	uint8_t payload[64];
};
struct lerna_cmd_response response;


struct option long_options[] = {
	/* Options that set a flag. */
	{"verbose",   no_argument, &is_verbose, 1},
	{"brief",     no_argument, &is_verbose, 0},

	/* Options that don't set a flag. */
	{"help",      no_argument,       0, 'h'},
	{"psid",      required_argument, 0, 'p'},
	{"group",     required_argument, 0, 'g'},
	{"row",       required_argument, 0, 'r'},
	{"version",   no_argument,       0, 'v'},
	{"subsystem", required_argument, 0, 's'},
	{NULL, 0, NULL, 0}
};

size_t unsigned_to_vldata(uint64_t value, struct lerna_vldata_block *vblock)
{
	size_t size = 0;
	uint8_t *data = &(vblock->data[vblock->length]);

	if (value <= 0x3F) {
		/* Single octet integer. */
		*data = 0x00 | /* Signed 7bit integer value represented in vldata. */
			0x00;  /* Unsigned value, so sign bit must be 0. If non-zero, it gets treated
				  as a 7bit 2's complement integer (hooray, fun). */
		*data |= (uint8_t)(value & 0x3F);
		size = 1;
	} else {
		*data = 0x80 | /* More bit set. */
			0x00 | /* Sign bit. Unsigned value. */
			0x00;  /* Type bit. 0 for integer value. */
		uint64_t check = 0;
		size_t shift = 0;
		do {
			check <<= 1;
			check |= 0x01;
			++shift;
		} while (check < value);
		/* Shift is now the number of bits needed to represent value. Find how many octets are needed. */
		size_t len = ((shift + 7) >> 3) & 0x1F;
		*data |= len;

		/* Finally add, big endian byte order, the original number. */
		++data;
		size_t index = len;
		do {
			*data++ = ((uint8_t *)(&value))[--index];
		} while (index);

		/* Total vldata block size. */
		size = 1 + len;
	}

	vblock->length += (uint16_t)(size);

	return vblock->length + 4;
}

size_t vldata_finalise(struct lerna_vldata_block *vblock)
{
	if (vblock->length & 0x01) {
		vblock->length++;
	}

	return vblock->length + 4;
}

uint8_t is_string_enabled(const char *s)
{
	uint8_t rval = 0;
	if (strcmp(s, "true") == 0) {
		rval = 1;
	} else if (strcmp(s, "enabled") == 0) {
		rval = 1;
	}

	return rval;
}

uint8_t is_string_disabled(const char *s)
{
	uint8_t rval = 0;
	if (strcmp(s, "false") == 0) {
		rval = 1;
	} else if (strcmp(s, "disabled") == 0) {
		rval = 1;
	}

	return rval;
}

int main(int argc, char **argv)
{
	ssize_t rval;

	uint16_t psid = 0;
	uint8_t group_index = 0;
	uint8_t row_index = 0;
	uint8_t subsystem_id = 0;  /* 0 = common, 1 = bt, 2 = wlan. */

        int has_row = 0;

	int c;
	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "hp:g:r:s:v", long_options, &option_index);

		if (c == -1) {
			break;  /* End of options. */
		}

		switch(c) {
		case 0:
			if (long_options[option_index].flag != 0) {
				break;
			}
			printf("option %s", long_options[option_index].name);
			if (optarg) {
				printf(" with arg %s\n", optarg);
			}
			printf("\n");
			break;
		case 'h':
			printf("scsc_lerna is a utility for setting runtime Maxwell firmware configuration.\n"
			       "\n"
			       "Usage:\n"
			       "\t" "scsc_lerna [<options>] --subsystem=<curator|bt|wlan> --psid=<id>\n"
			       "\t" "scsc_lerna [<options>] --subsystem=<curator|bt|wlan> --psid=<id> <write-value>\n"
			       "\n"
			       "Options:\n"
			       "\t" "-h --help      Show this screen.\n"
			       "\t" "-v --version   Show program version.\n"
			       "\t" "-g --group     Group index for configuration read/write. Defaults to zero.\n"
			       "\t" "-r --row       Row index for table lookups. Defaults to zero.\n"
			       "\t" "-s --subsystem Subsystem to which the provided PSID belongs. Valid values are\n"
                               "\t" "               curator, bt, or wlan.\n"
			       "\t" "-p --psid      PSID to query. Required option.\n"
			       "\n"
			       "For any boolean write, acceptable inputs are 0, 1, true, false, enable, or disable\n");
			exit(0);
			break;
		case 'g':
			group_index = (uint8_t)(strtoul(optarg, NULL, 10));
			break;
		case 'p':
			psid = (uint16_t)(strtol(optarg, NULL, 10));
			break;
		case 'r':
			row_index = (uint8_t)(strtoul(optarg, NULL, 10));
                        /* If a row index is specified, then assume the target psid is a table. */
                        has_row = 1;
			break;
		case 's':
			subsystem_id = (uint8_t)(strtoul(optarg, NULL, 10));
			break;
		case 'v':
			printf("scsc_lerna %s\n", version_string);
			exit(0);
			break;
		case '?':
			/* getopt_long already printed an error message apparently. */
			break;
		default:
			printf("Unrecognised option, use:\n\tslsi_lerna --help\nfor program usage.\n");
			abort();
			break;
		}
	}

	if (is_verbose) {
		printf("verbose mode engaged.\n");
	}

	command.header.magic_number = 8;
	command.header.cid = 0x00;  /* Top bit is used as an ack. Everything else as a version identifier. For now. */
	command.header.psid = psid;
	command.header.row_index = row_index;
	command.header.group_index = group_index;
	command.header.length = 0;  /* Overwritten if a write command is issued. If 0, this is a read command. */

	command.vldata.psid = psid; /* Ignored on a read command. */
	command.vldata.length = 0;  /* Set a sane initial value. */

	if (optind < argc) {
		if (has_row) {
			/* When riting to a table, hcf format requires first vldata block to be the row index. */
			(void)unsigned_to_vldata(row_index, &command.vldata);
		}

		char firstchar = argv[optind][0];
		if ((firstchar == '0') && (strlen(argv[optind]) > 1)) {
			if (argv[optind][1] == 'x' || argv[optind][1] == 'X') {
				/* Hex formatted unsigned integer value. */
				printf("hex formatting not yet implemented.\n");
			} else {
				/* Octal formatted unsigned integer value. */
				printf("octal formatting not yet implemented.\n");
			}
		} else if (firstchar == '-') {
			/* Negative integer value. */
			printf("negative integers not yet implemented.\n");
		} else if (firstchar == '[') {
			/* Array of hex values, LSB. */
			printf("arrays not yet implemented.\n");
		} else if (is_string_enabled(argv[optind])) {
			uint64_t value = 1;
			(void)(unsigned_to_vldata(value, &command.vldata));
			command.header.length = vldata_finalise(&command.vldata);
		} else if (is_string_disabled(argv[optind])) {
			uint64_t value = 0;
			(void)(unsigned_to_vldata(value, &command.vldata));
			command.header.length = vldata_finalise(&command.vldata);
		} else {
			/* Decimal unsigned integer. */
			uint64_t value = strtoull(argv[optind], NULL, 10);
			(void)(unsigned_to_vldata(value, &command.vldata));
			command.header.length = vldata_finalise(&command.vldata);
		}
	}


	/**
	 * Standard fopen, et. al, can't be used in this case because the size for fread
	 * isn't known, and fread will continue to attempt reads from the char device
	 * until the size is matched, or there's a timeout. It will usually be a timeout,
	 * making the program very slow to use inside of tests.
	 * Instead, use good old fashioned open/write/read from POSIX, which can return
	 * the actual size read immediately.
	 */

	int fd = open("/dev/lerna", O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "open() failure: %s\n", strerror(errno));
		return -1;
	}

	write(fd, &command, sizeof(struct lerna_cmd_header) + command.header.length);

	rval = read(fd, &response, sizeof(struct lerna_cmd_response));
	close(fd);
	if (rval < 0) {
		fprintf(stderr, "read() failure %zu: %s\n", rval, strerror(errno));
		return rval;
	}

	printf("read %zu bytes.\n", rval);
	printf("response:\n");
	printf("\t magic number: 0x%02X\n", response.header.magic_number);
	printf("\t cid: 0x%02X\n", response.header.cid);
	printf("\t length: %u\n", response.header.length);
	printf("\t psid: 0x%04X\n", response.header.psid);
	printf("\t row: %u\n", response.header.row_index);
	printf("\t group: %u\n", response.header.group_index);
	printf("\t payload:\n\t\t");
	uint16_t i;
	for (i = 0; i < response.header.length; ++i) {
		printf("%0X ", response.payload[i]);
	}
	printf("\n");

	return 0;
}
