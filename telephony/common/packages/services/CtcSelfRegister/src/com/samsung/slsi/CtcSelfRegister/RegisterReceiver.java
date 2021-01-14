/*
 * Copyright (c) 2017. Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.CtcSelfRegister;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

/**
 * Get Broadcast of BOOT_COMPLETED defined in AndroidManifest.xml
 * When the receive the event, it starts Self registration service by calling startService
 */

public class RegisterReceiver extends BroadcastReceiver {
    private static final String TAG_SELFREGISTER_RECEIVER = "CtcSelfRegisterReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent != null) {
            String action = intent.getAction();
            Log.d(TAG_SELFREGISTER_RECEIVER, "onReceive Action = " + action);
            intent.setClass(context, SelfRegisterService.class);
            context.startService(intent);
        }
    }
}
