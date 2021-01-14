/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#ifndef __MBD_SS300_H__
#define __MBD_SS300_H__

extern int send_bootloader_ss300(void *args);
extern int prepare_download_control(struct modem_args *args);
#endif //__MBD_SS300_H__
