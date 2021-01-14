/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
    RFS protocol definitions
*/
#ifndef _rfs_h
#define _rfs_h

/*
    Types used
*/
#ifndef s8
typedef char                s8;
#endif

#ifndef s16
typedef short                s16;
#endif

#ifndef s32
typedef int                s32;
#endif

#ifndef u8
typedef unsigned char            u8;
#endif

#ifndef u16
typedef unsigned short            u16;
#endif

#ifndef u32
typedef unsigned int            u32;
#endif

/* Data payload size */
#define RFS_FRAME_DATA_PAYLOAD_SIZE    2012
/* File name length */
#define RFS_MAX_NAME_LENGTH        255
#define RFS_BACKUP_RETRY           3
/* Path */
//#define RFS_FILE_PATH            "/mnt/vendor/efs/"
//#define RFS_BACKUP_FILE_PATH     "/mnt/vendor/efs/backup/nv_protected"

/* Predefined files */
enum rfs_nv_file_id {
    RFS_NV_NORMAL = 1,
    RFS_NV_USER,            /* not used */
    RFS_NV_PROTECTED
};

static const char *rfs_nv_file_names[] = {
    "nv_normal.bin",
    "nv_user.bin",            /* not used */
    "nv_protected.bin"
};

/* Commands */
enum rfs_command {
    RFS_NONE,
    /* defined in SIT document */
    RFS_READ,
    RFS_WRITE,
    RFS_OP_STATUS,
    RFS_OPEN,
    RFS_CLOSE,
    RFS_IO_REQUEST,
    RFS_NV_UNPROTECT,        /* a.k.a. NV_WRITE_NOTIFY */
    RFS_NV_PROTECT,            /* obsolete */
    RFS_NV_BACKUP = 257,
    RFS_NV_MAX
};

/* I/O operation */
enum rfs_io {
    RFS_IO_READ = 1,
    RFS_IO_WRITE
};

/* Error status */
enum rfs_error_status {
    RFS_STATUS_SUCCESS,
    RFS_STATUS_OPEN_FAIL,
    RFS_STATUS_CLOSE_FAIL,
    RFS_STATUS_READ_FAIL,
    RFS_STATUS_WRITE_FAIL,
    RFS_STATUS_TIMEOUT,
    RFS_STATUS_INVALID_PARAM
};

/* Header */
struct rfs_header {
    u16    cmd;    /* Command */
    u16    tid;    /* Transaction ID */
    u32    len;    /* Length */
} __packed;

/*
    Read file
*/
struct rfs_read {
    u32    file;
    u32    offset;
    u32    length;
    u8    data[0];
} __packed;

struct rfs_read_res {
    u32    status;
    u32    file;
} __packed;

/*
    Write file
*/
struct rfs_write {
    u32    file;
    u32    offset;
    u32    length;
} __packed;

struct rfs_write_res {
    u32    status;
    u32    file;
    u32    length;
    u8    data[0];
} __packed;

/*
    Operation status
*/
struct rfs_status {
    u32    status;
    u32    file;
} __packed;

/*
    Open file (one way - from modem to RIL)
*/
struct rfs_open_ind {
    u32    file;
    u32    name_len;
    /* name[RFS_MAX_NAME_LENGTH] */
} __packed;

/*
    Close file (one way - from modem to RIL)
*/
struct rfs_close_ind {
    u32    file;
} __packed;

/*
    I/O request (one way - from modem to RIL)
*/
struct rfs_request_ind {
    u32    file;
    u32    offset;
    u32    length;
    u32    io;
} __packed;

/*
    NV protect file (one way - from modem to RIL)
*/
struct rfs_protect_ind {
    u32    file;
} __packed;

/*
    NV unprotect file (one way - from modem to RIL)
*/
struct rfs_unprotect_ind {
    u32    file;
} __packed;

/*
    NV backup file (one way - from modem to RIL)
*/
struct rfs_backup_ind {
    u32    file;
} __packed;

/*
    Sequences

Open file:     RFS_OPEN (modem), RFS_OP_STATUS (RIL), RFS_OP_STATUS (modem)
Close file:    RFS_CLOSE (modem), RFS_OP_STATUS (RIL), RFS_OP_STATUS (modem)
Read file:    RFS_IO_REQUEST (modem), RFS_READ (RIL), RFS_READ (response - modem)
Write file:    RFS_IO_REQUEST (modem), RFS_WRITE (RIL), RFS_WRITE (response - modem), (optional: RFS_OP_STATUS (RIL), RFS_OP_STATUS (modem))
Unprotect file:    RFS_UNPROTECT (modem), RFS_OP_STATUS (RIL), RFS_OP_STATUS (modem)
Protect file:    RFS_PROTECT (modem), RFS_OP_STATUS (RIL), RFS_OP_STATUS (modem)

*/

#endif
