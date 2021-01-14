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
import android.util.Log;
import android.system.Os;

public class AndroidServiceStartOnBoot extends Service {
    static final String EXTRA_TETHER_TYPE="TETHER_TYPE";
    static final String TAG ="TetherProvService";
    private static final String ACTION="com.samsung.slsi.testcarrier.provisioning.ProvisioningRequestReceiver";
    private static final boolean DEBUG = Log.isLoggable(TAG, Log.DEBUG);
    private static final String PROV_APP_NO_UI = "config_mobile_hotspot_provision_app_no_ui";
    private static final String PROV_RESP = "config_mobile_hotspot_provision_response";
    ProvisioningRequestReceiver mReceiver;
    ProvisioningResponseReceiver mResponseReceiver;

    public class ProvisioningResponseReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent)
        {
            // receving Intent will be config_mobile_hotspot_provision_response
            Log.d(TAG, "Self-Checker: Intent is received " + intent);

            // Dump Result
            Bundle bundle = intent.getExtras();
            if (bundle != null) {
                for (String key : bundle.keySet()) {
                    Object value = bundle.get(key);
                    Log.d(TAG, String.format("%s %s (%s)", key,
                                             value.toString(), value.getClass().getName()));
                }
            }
            else
            {
                Log.d(TAG, "No Extras");
            }
        }
        public ProvisioningResponseReceiver()
        {
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "Binding Compelete");
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // Access to Internal Resource
        int id = Resources.getSystem().getIdentifier(PROV_APP_NO_UI, "string", "android");
        if(id == 0){
            Log.d(TAG, "No Valid Resource for '" + PROV_APP_NO_UI + "'. finish.");
            return;
        }
        String provisionAction = Resources.getSystem().getString(id);
        if(provisionAction == null){
            Log.d(TAG, "id=" + id + " No Valid Resource for '" + PROV_APP_NO_UI + "'. finish.");
            return;
        }
        if(!provisionAction.isEmpty() && !provisionAction.equals("null")){
            Log.d(TAG, "id=" + id + " Access Resource is  " + provisionAction);
        }else{
            Log.d(TAG, "id=" + id + " No Valid Resource for '" + PROV_APP_NO_UI + "' is '"+provisionAction+"'. finish.");
            return;
        }

        //Intent actionIntent = new Intent(provisionAction);
        final IntentFilter intentfilter = new IntentFilter();
        intentfilter.addAction(provisionAction);

        mReceiver = new ProvisioningRequestReceiver();

        Log.d(TAG, "Creating TetherProvisioning non UI Service");
        Intent si = registerReceiver(mReceiver, intentfilter);
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

        // Access to Internal Resource
        id = Resources.getSystem().getIdentifier(PROV_RESP, "string", "android");
        if(id == 0)
        {
            Log.d(TAG, "No Valid Resource for '" + PROV_RESP + "'. finish.");
            unregisterReceiver(mReceiver);
            return;
        }
        String provisionResponse = Resources.getSystem().getString(id);
        Log.d(TAG, "Access Resource is  " + provisionResponse);

        Intent responseIntent = new Intent(provisionResponse);
        final IntentFilter responseFilter = new IntentFilter();
        responseFilter.addAction(provisionResponse);
        mResponseReceiver = new ProvisioningResponseReceiver();
        registerReceiver(mResponseReceiver, responseFilter);

        // Test Sending Intent
        Intent intent = new Intent(provisionAction);
        intent.putExtra("TETHER_TYPE", 0);
        intent.setFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        sendBroadcast(intent);

    }
    @Override
    public void onDestroy()
    {
        if(mReceiver != null)
            unregisterReceiver(mReceiver);
        super.onDestroy();
    }
}
