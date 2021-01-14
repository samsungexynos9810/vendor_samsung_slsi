/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.testcarrier.provisioning;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.IBinder;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.TelephonyIntents;
import android.util.Log;
import android.system.Os;

// This service will run with TetherProvisioningService
// This service will be used in DcTracker by ACTION_REQUEST_NETWORK_FAILED
// But service package and activity should be registered in CarrierConfig like below
//
// Sample Configuration of /packages/apps/CarrierConfig
// diff --git a/assets/carrier_config_45005.xml b/assets/carrier_config_45005.xml
// index c9cef78..f4e76cb 100644
// --- a/assets/carrier_config_45005.xml
// +++ b/assets/carrier_config_45005.xml
// @@ -13,5 +13,9 @@
//   <string name="carrier_instant_lettering_encoding_string">EUC-KR</string>
//   <int name="carrier_instant_lettering_length_limit_int" value="16" />
//   <boolean name="support_conference_call_bool" value="false" />
//   +<string-array translatable="false" name="sim_state_detection_carrier_app_string_array">
//   +    <item>com.samsung.slsi.testcarrier.provisioning</item>
//   +    <item>com.samsung.slsi.testcarrier.provisioning.SIMProvisioningDetectionReceiver</item>
//   +</string-array>
//    </carrier_config>
//    </carrier_config_list>

public class SIMProvisioningService extends Service {
    static final String TAG ="SIMProvService";
    static final boolean VDBG=false;
    //private static final boolean DEBUG = Log.isLoggable(TAG, Log.DEBUG);
    SIMProvisioningDetectionReceiver mReceiver;
    SIMProvisioningDetectionReceiver.ConnectivityMonitor mConnMonitor;

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "Binding Compelete");
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        final IntentFilter intentfilter = new IntentFilter();
        //intentfilter.addAction(TelephonyIntents.ACTION_DATA_CONNECTION_REDIRECTED);
        intentfilter.addAction(TelephonyIntents.ACTION_CARRIER_SIGNAL_REQUEST_NETWORK_FAILED);

        mReceiver = new SIMProvisioningDetectionReceiver();
        Log.d(TAG, "Creating SIMProvisioningDetectionReceiver");
        Intent si = registerReceiver(mReceiver, intentfilter);

        final IntentFilter intentfilter2 = new IntentFilter();
        intentfilter2.addAction("android.net.conn.CONNECTIVITY_CHANGE");
        intentfilter2.addAction("android.intent.action.AIRPLANE_MODE");
        mConnMonitor = new SIMProvisioningDetectionReceiver.ConnectivityMonitor();
        Log.d(TAG, "Creating ConnecitivtyMonitor");
        mConnMonitor.setParent(mReceiver);
        Intent si2 = registerReceiver(mConnMonitor, intentfilter2);

        if(VDBG){
            // Check Receiver is register correctly
            if( mReceiver != null )
                Log.d(TAG, "Receiver is created, and stickyIntent is " + si + 
                      "\nIntent Filter is " + intentfilter.getAction(0));
            else
                Log.d(TAG, "Broadcast receiver reigstering is failed");

            try{
                unregisterReceiver(mReceiver);
            }
            catch(Exception e)
            {
                Log.d(TAG, "Something Wrong, unregister Failed");
            }
            // Again
            Log.d(TAG, "ReRegister");
            si = registerReceiver(mReceiver, intentfilter);
        }
        // Now Receiver will work
    }
    @Override
    public void onDestroy()
    {
        if(mReceiver != null)
            unregisterReceiver(mReceiver);
        super.onDestroy();
    }
}
