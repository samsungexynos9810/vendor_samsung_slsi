/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.rillogadmin;

public class VendorRILConstants {
    public static final int RIL_LOG_INVALID = -1;
    public static final int RIL_LOG_NONE = 0;
    public static final int RIL_LOG_ALL = 0x7FFFFFFF;
    public static final int RIL_LOG_CAT_CORE = 1;
    public static final int RIL_LOG_CAT_CALL = 1 << 1;
    public static final int RIL_LOG_CAT_SMS = 1 << 2;
    public static final int RIL_LOG_CAT_SIM = 1 << 3;
    public static final int RIL_LOG_CAT_NETWORK = 1 << 4;
    public static final int RIL_LOG_CAT_DATA = 1 << 5;
    public static final int RIL_LOG_CAT_MISC = 1 << 6;
    public static final int RIL_LOG_CAT_SOUND = 1 << 7;
    public static final int RIL_LOG_CAT_OEM = 1 << 8;
    public static final int RIL_LOG_CAT_RFS = 1 << 9;

    public static final String PROPERTY_RIL_LOG_CATEGORY = "persist.vendor.radio.log.categorymask";
}
