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
 * oem_nv_items.h
 *
 *  Created on: 2015. 12. 22.
 */

#ifndef __OEM_NV_ITEMS_H__
#define __OEM_NV_ITEMS_H__

typedef enum {
    RIL_NV_OEM_SN          = 10000,     // Serial Number
    RIL_NV_OEM_GSN         = 10001,     // Global SN
    RIL_NV_OEM_HW          = 10002,
    RIL_NV_OEM_BT_MAC      = 10003,     // Bluetooth
    RIL_NV_OEM_WLAN_MAC    = 10004,     // WLAN
    RIL_NV_OEM_TEST_FLOW   = 10005,
    RIL_NV_OEM_MMI         = 10006,
    RIL_NV_OEM_IMEI1       = 10007,
    RIL_NV_OEM_IMEI2       = 10008,
} RIL_OEM_NV_Item;

#endif /* __OEM_NV_ITEMS_H__ */
