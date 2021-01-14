/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.dataretrymode;

public class DataRetryModeConstants {
    // APN mode preference fragment
    public static final String DEFAULT_SORT_ORDER = "name ASC";
    public static final String KEY_APN_LIST = "key_dataretry_list";
    public static final String CARRIER_ENABLED = "carrier_enabled";
    public static final int ID_INDEX = 0;
    public static final int APN_INDEX = 2;
    public static final int TYPE_INDEX = 3;

    // Phone Constants
    public static final String APN_TYPE_ALL = "*";
    public static final String APN_TYPE_DEFAULT = "default";
    public static final String APN_TYPE_MMS = "mms";
    public static final String APN_TYPE_SUPL = "supl";
    public static final String APN_TYPE_DUN = "dun";
    public static final String APN_TYPE_HIPRI = "hipri";
    public static final String APN_TYPE_FOTA = "fota";
    public static final String APN_TYPE_IMS = "ims";
    public static final String APN_TYPE_CBS = "cbs";
    public static final String APN_TYPE_IA = "ia";

    // Select SIM preference fragment
    public static final String KEY_SIM1 = "key_sim1";
    public static final String KEY_SIM2 = "key_sim2";

   // Result-code for delay-option
    public static final String RESULT_CODE ="NOTSET";
    public static final String RESULT_OK ="OK";
    public static final String RESULT_CANCEL ="CANCEL";

    // String for Handler
    public static final int EVENT_START_RETRY = 11;
    public static final int EVENT_STOP_RETRY = 12;

}
