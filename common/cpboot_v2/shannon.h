#ifndef __CBD_SHANNON_H__
#define __CBD_SHANNON_H__

enum shannon_image_type {
	IMG_TOC = 0,
	IMG_BOOT,
	IMG_MAIN,
	IMG_VSS,
	IMG_NV,
	MAX_IMAGE_TYPE
};


#ifdef CONFIG_PROTOCOL_SIT
#define SHANNON_LEGACY_MAX_DL_STAGE 4 /* BOOT, MAIN, NV, NV_PROT */
#else
#define SHANNON_LEGACY_MAX_DL_STAGE 3 /* BOOT, MAIN, NV */
#endif

struct shannon_boot_args {
	struct boot_args *cbd_args;
	struct std_boot_args *std_args;
	int load_fd;
	int bin_fd;
	int nv_fd;
	int nv_prot_fd;
} __packed;

struct modem_img {
	unsigned long long binary; 	/* Pointer to binary buffer */
	u32 size;			/* Binary size */
	u32 m_offset;
	u32 b_offset;
	u32 mode;
	u32 len;
} __packed;
#endif /* __CBD_SHANNON_H__ */

