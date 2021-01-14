/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.usbmodeswitch;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.util.Log;

public class UsbModeSwitchReceiver extends BroadcastReceiver {

    private static final String TAG = "UsbModeSwitchReceiver";
    private static final String ACTION_BOOT_COMPLETED = "android.intent.action.BOOT_COMPLETED";
    private static final String PROPERTY_USB_CONFIG = "sys.usb.config";

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        if (intent != null && "android.provider.Telephony.SECRET_CODE"
                .equals(intent.getAction())) {
            Intent i = new Intent(Intent.ACTION_MAIN);
            i.setClass(context, UsbModeSwitchActivity.class);
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(i);
        }
        else if (intent.getAction().equals(ACTION_BOOT_COMPLETED)) {
            SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
            String sharedMode = sharedPref.getString("key_usb_mode", "");
            String systemMode = SystemProperties.get(PROPERTY_USB_CONFIG, "");
            if (!sharedMode.equals(systemMode) && !sharedMode.equals("")) {
                Log.d(TAG, "Boot complete() usb mode: " + sharedMode);
                SystemProperties.set(PROPERTY_USB_CONFIG, sharedMode);
            }
            else {
                Log.d(TAG, "skip to set USB mode: sharedMode=" + sharedMode + " systemMode=" + systemMode);
            }
        }
    }
}
