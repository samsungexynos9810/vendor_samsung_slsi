/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.silentlogging;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.os.SystemProperties;
import android.util.Log;

public class LogdConfigService extends Service {
    private static final String TAG = "LogdConfigService";

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        // increase a larget size of logcat buffer for eng/userdebug
        Log.d(TAG, "onStartCommand: intent=" + intent);
        if (!Build.TYPE.equals("user")) {
            Log.d(TAG, "buildtype=" + Build.TYPE);
            int newSize = 16777216;
            int size = SystemProperties.getInt("persist.logd.size", 0);
            Log.d(TAG, "size=" + size + " expected size=" + newSize);
            if (size < newSize) {
                Log.d(TAG, "Incease logcat buffer size to " + newSize);
                SystemProperties.set("persist.logd.size", "" + newSize);
                SystemProperties.set("ctl.start", "logd-reinit");
            }
        }
        else {
            Log.d(TAG, "buildtype=" + Build.TYPE + " not debuggable");
        }

        stopSelf(startId);

        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}

