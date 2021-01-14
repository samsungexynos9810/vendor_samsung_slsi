/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.carrierconfigplus;

import android.net.Uri;

public class CarrierConfigPlusConstants {

    public static final String AUTHORITY = "com.samsung.slsi.telephony.provider.carrierconfigplus";
    public static final Uri CONTENT_URI = Uri.parse("content://com.samsung.slsi.telephony.provider.carrierconfigplus/config");

    public static final String PREFS_CARRIER_CONFIG = "prefs_carrier_config_plus";
    public static final String KEY_OPERATOR_NUMERIC = "operator_numeric";
    public static final String KEY_OPERATOR_NUMERIC0 = "operator_numeric0";
    public static final String KEY_OPERATOR_NUMERIC1 = "operator_numeric1";

    public static final String PROP_KEY_OPERATOR_NUMERIC = "radio.ril.operator_numeric";

    public static final String ACTION_RELOAD_CARRIER_CONFIG = "com.samsung.slsi.telephony.action.RELOAD_CARRIER_CONFIG";

    public enum NetworkGroup {
       KT,
       SKT,
       LGU,
       CMCC,
       CTC,
       CU,
       VZW,
       SPR,
       USCC,
       ATT,
       TMO;
    }

    public static final int INVALID_PHONE_INDEX = -1;
    public static final int INVALID_SUBSCRIPTION_ID = -1;

    // @desc
    public static final String KEY_USE_CMCC_SS_POLICY_NW_BOOL = "use_cmcc_ss_policy_nw_bool";

}
