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
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.util.Timer;
import java.util.TimerTask;

import android.os.SystemProperties;
import android.provider.Settings;


public class EmptyActivity extends Activity {
    static final String EXTRA_TETHER_TYPE="TETHER_TYPE";
    static final String EXTRA_TETHER_RESULT="result";
    static final String TAG ="TheterProvApp";
    static final String PROPERTY_TETHER_PROVISION_ALLOWED="vendor.config.tether_provision.allowed";

    final Context context = this;

    ProvisioningRequestReceiver mReceiver;

    void fireout(int seconds)
    {
        if(seconds<0) seconds = 0;
        Timer timer = new Timer();
        timer.schedule(new TimerTask()
        {
            public void run()
            {
                finish();
            }
        }, seconds*1000);
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_empty);
        // Intent will be delivered
        Intent intent = getIntent();
        Intent returnIntent = new Intent();
        mReceiver = new ProvisioningRequestReceiver();

        Bundle bundle = intent.getExtras();
        if (bundle != null) {
            for (String key : bundle.keySet()) {
                Object value = bundle.get(key);
                Log.d(TAG, String.format("%s %s (%s)", key,
                        value.toString(), value.getClass().getName()));
            }
        }else
        {
            Log.d(TAG, "Not Expected Launch. Finish.");
            setResult(Activity.RESULT_CANCELED, returnIntent);
            fireout(3);
            return;
        }
        int tether_type = intent.getIntExtra(EXTRA_TETHER_TYPE, 0);

        String notice="";
        final TextView txt = (TextView)findViewById(R.id.InfoText);
        notice = intent.toString()+"\n"+tether_type+"\n";

        // TODO: Check Gid or others to check if Pre-paid SIM is present for Carrier
        // then determine allowing
        //

        // <CODE>


        // Emulated Result will be enableTether
        String isTetherAllowed = SystemProperties.get(PROPERTY_TETHER_PROVISION_ALLOWED);
        boolean enableTether=true;
        if(isTetherAllowed == null || isTetherAllowed.equals("true") || isTetherAllowed.equals("")) enableTether = true;
        else enableTether = false;
        notice += "isTetherAllowed is (" + isTetherAllowed + ") and enableTether is ("+enableTether+")";


        if(enableTether) {
            returnIntent.putExtra(EXTRA_TETHER_RESULT, "GOGO_TETHERING");
            notice += "Tethering is allowed with " + tether_type + "\n";
            setResult(Activity.RESULT_OK, returnIntent);
        }else {
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
            returnIntent.putExtra(EXTRA_TETHER_RESULT, "DO NOT ALLOW TETHERING");
            notice += notice + "Tethering is NOT allowed\n";
            // This should be placed before Dialog. ##!GTR-DATA-00272
            setResult(Activity.RESULT_CANCELED, returnIntent);

            // Showing Alerting Then Exit.
            Intent alertIntent = new Intent();

            AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
            alertDialogBuilder.setTitle(alert_title);
            alertDialogBuilder
                .setMessage(alert_msg)
                .setCancelable(false)
                .setPositiveButton("Close",
                       new DialogInterface.OnClickListener(){
                           public void onClick(
                                   DialogInterface dialog, int id){
                               EmptyActivity.this.finish();
                           }
                       });

            AlertDialog alertDialog = alertDialogBuilder.create();
            alertDialog.show();
        }

        int timeout = 3;
        notice += "Exit in " + timeout + " seconds\n";
        txt.setText(notice);
        Log.d(TAG, notice);

        fireout(timeout);
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
