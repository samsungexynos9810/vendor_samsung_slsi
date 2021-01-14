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
//import android.support.v7.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;
import android.util.Log;

public class AlertDialogActivity extends Activity {

    static final String EXTRA_TETHER_TYPE="TETHER_TYPE";
    static final String TAG="TetherAlert";

    final Context context = this;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        Bundle bun = getIntent().getExtras();
        int tether_type = getIntent().getIntExtra(EXTRA_TETHER_TYPE, 0);

        setContentView(R.layout.alertdialog);

        Bundle bundle = getIntent().getExtras();
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

        // ##!GTR-DATA-00271
        //   Cover 'pam' PDN profile is not provisioned case
        String service_name="";
        String alert_title="";
        String alert_msg="";
        switch(tether_type)
        {
          case ConnectivityManager.TETHERING_WIFI:
                service_name += "Hotspot";
                break;
          default:
                service_name += "Data tethering";
        }

        // ##!GTR-DATA-00273
        alert_title = service_name + " error";

        boolean isLTE = checkLTE();
        if(isLTE)
        {
            // ##!GTR-DATA-00276
            alert_msg = "You are not subscribed to "+service_name+".";
        }
        else // or 3G only
        {
            alert_msg = "Unable to connect to data network, turning off "+service_name+". " +
                "Your plan may not have the " + service_name + " service.";
        }

        // Launch AlertDialog
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
        alertDialogBuilder.setTitle(alert_title);
        alertDialogBuilder
            .setMessage(alert_msg)
            .setCancelable(false)
            .setPositiveButton("Close",
                   new DialogInterface.OnClickListener(){
                       public void onClick(
                               DialogInterface dialog, int id){
                           AlertDialogActivity.this.finish();
                       }
                   });

        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    private boolean checkLTE()
    {
        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        TelephonyManager tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        NetworkInfo info = cm.getActiveNetworkInfo();
        if (info == null || !cm.getBackgroundDataSetting()) {
            Log.d(TAG, "No NetworkInfo");
            return false;
        }
        int netType = info.getType();
        int netSubtype = info.getSubtype();
        Log.d(TAG, "NetType: " + netType + ", netSubType: " +netSubtype);
        if (netType == ConnectivityManager.TYPE_WIFI) {
            Log.d(TAG, "WIFI TYPE");
            return false; // if connection state is needed. return info.isConnected();
        }
        else if(netType == ConnectivityManager.TYPE_MOBILE
                && netSubtype == TelephonyManager.NETWORK_TYPE_LTE
                && !tm.isNetworkRoaming()){
            Log.d(TAG, "LTE TYPE and not roaming");
            return true;
        }
        else
        {
            Log.d(TAG, "ELSE Roaming: "+ tm.isNetworkRoaming());
            if(netType == ConnectivityManager.TYPE_MOBILE)
                Log.d(TAG, "NetworkTypeName: "+ tm.getNetworkTypeName(netSubtype));
            return false;
        }
    }
}
