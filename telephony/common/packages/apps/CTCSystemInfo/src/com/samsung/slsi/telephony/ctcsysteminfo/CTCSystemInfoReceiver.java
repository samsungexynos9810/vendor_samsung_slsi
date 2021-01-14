/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.ctcsysteminfo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.SystemProperties;

public class CTCSystemInfoReceiver extends BroadcastReceiver {

    public static final String TAG = "CTCSystemInfoReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "carrier: " + intent.getStringExtra("build_carrier"));
        if (intent != null
                && "android.provider.Telephony.SECRET_CODE".equals(intent.getAction())) {
            String carrier = SystemProperties.get("ro.vendor.config.build_carrier", "");
            Log.d(TAG, "build carrier: " + carrier);
            if (carrier.equals("ctc") || carrier.equals("chnopen")) {
                Intent i = new Intent(Intent.ACTION_MAIN);
                i.setClass(context, TinyCTCSystemInfo.class);
                i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(i);
            }
        }
    }
}
