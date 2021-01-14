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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.net.ConnectivityManager;
import android.util.Log;

import android.os.Bundle;
import android.os.SystemProperties;

public class ProvisioningRequestReceiver extends BroadcastReceiver {
    static final String TETHER_CHOICE="TETHER_TYPE";
    static final String TAG="ProvReqReceiver";
    static final String PROPERTY_TETHER_PROVISION_ALLOWED="vendor.config.tether_provision.allowed";
    static final String PROV_RESP="config_mobile_hotspot_provision_response";
    // Broadcasting intent defined by config_mobile_hotspot_provision_response,
    // will send to config_mobile_hotspot_provision_app including EntitlementResult extra
    // To See TetherService in Settings App: setprop log.tag.TetherService DEBUG
    // TetherService will created when Tether Triggering. And only restart by tethering success
    // To run Periodic provision Recheck

    ProvisioningRequestReceiver()
    {
        Log.d(TAG, "Called constructor");
    }
    @Override
    public void onReceive(Context context, Intent intent) {
        // receving Intent will be config_mobile_hotspot_provision_check
        Log.d(TAG, "Intent is received " + intent);
        Bundle bundle = intent.getExtras();
        if (bundle != null) {
            for (String key : bundle.keySet()) {
                Object value = bundle.get(key);
                Log.d(TAG, String.format("%s %s (%s)", key,
                        value.toString(), value.getClass().getName()));
            }
        }else
        {
            Log.d(TAG, "No Extras");
            return;
        }
        // Access to Internal Resource
        int id = Resources.getSystem().getIdentifier(PROV_RESP, "string", "android");
        if(id == 0)
        {
            Log.d(TAG, "No Valid Resource for '" + PROV_RESP + "'. finish.");
            return;
        }
        String provisionResponse = Resources.getSystem().getString(id);
        Log.d(TAG, "Access Resource is  " + provisionResponse);
        int tether_type = intent.getIntExtra(TETHER_CHOICE, 0);

        // TODO: Check Gid or others to check if Pre-paid SIM is present for Carrier
        // then determine allowing
        //

        // <CODE>


        // Emulated Result will be enableTether
        String isTetherAllowed = SystemProperties.get(PROPERTY_TETHER_PROVISION_ALLOWED);
        boolean enableTether=true;
        if(isTetherAllowed != null && isTetherAllowed.equals("false")) enableTether = false;

        Intent responseIntent = new Intent(provisionResponse);
        String notice="";

        if(enableTether) {
            notice += "Tethering is allowed\n";
            responseIntent.putExtra("EntitlementResult", Activity.RESULT_OK);
        }else {
            notice += notice + "Tethering is NOT allowed\n";
            responseIntent.putExtra("EntitlementResult", Activity.RESULT_CANCELED);
        }
        Log.d(TAG, notice);
        context.sendBroadcast(responseIntent);
        if(!enableTether){
            // If failed. Launch AlertDialog Then Exit.
            // This procedure can be initiated by intent receving from lower layer
            // to cover MIP error 67 from Sprint CDMA network error case
            Intent alertIntent = new Intent(context, AlertDialogActivity.class);
            alertIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
            alertIntent.putExtra(TETHER_CHOICE, tether_type);
            context.startActivity(alertIntent);
        }
    }
}
