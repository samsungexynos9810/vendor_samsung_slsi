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

import android.app.NotificationManager;
import android.app.Notification;
import android.app.PendingIntent;
//import android.support.v7.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.DialogInterface;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.TelephonyIntents;
import android.os.Bundle;
import android.os.Build;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;
import android.util.Log;

import java.lang.reflect.Method;

// ##!GTR-DATA-00256, 00302
// This is independent Process to DataControl Plane
public class SIMProvisioningDetectionReceiver extends BroadcastReceiver {
    static final String TAG="SIMProvDetect";
    static final int NOTIFICATION_ID=1538;
    static final String NOTIFICATION_STRING_ID="com.samsung.slsi.DATA_SETUP_FAILED";
    // ##!GTR-DATA-00253
    static final String notification="Unable to establish a wireless data connection.";
    static String additionalErrorCodes="";

    boolean isDataFailNotified=false;

    NotificationManager notificationManager;

    // ##!GTR-DATA-00255
    static public class ConnectivityMonitor extends BroadcastReceiver
    {
        static final String TAG="ConnMonitor";
        ConnectivityMonitor()
        {
            Log.d(TAG, "Called constructor");
        }
        SIMProvisioningDetectionReceiver mReceiver;

        public void setParent(SIMProvisioningDetectionReceiver mParent)
        {
            mReceiver = mParent;
        }

        public void onReceive(Context context, Intent intent)
        {
            // receving Intent will be config_mobile_hotspot_provision_response
            Log.d(TAG, "CONNECTIVITY_CHANGED Intent is received " + intent);

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

            Log.d(TAG, "isDataFailNotified = " + mReceiver.isDataFailNotified);

            if(mReceiver.isDataFailNotified)
            {
                // Check Airplane MODE
                if( Settings.Global.getInt(context.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0) != 0)
                {
                    Log.d(TAG, "AirplaneMode is triggered");
                    mReceiver.clearNotification(context);
                    mReceiver.isDataFailNotified=false;
                    return;
                }
                ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
                TelephonyManager tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
                NetworkInfo info = cm.getActiveNetworkInfo();
                if (info == null || !cm.getBackgroundDataSetting()) {
                    Log.d(TAG, "No NetworkInfo");
                    // Check User disable Mobile Data, then clear Notificaiton
                    boolean mobileDataEnabled = false;
                    try {
                        Class cmClass = Class.forName(cm.getClass().getName());
                        Method method = cmClass.getDeclaredMethod("getMobileDataEnabled");
                        method.setAccessible(true);
                        mobileDataEnabled = (Boolean)method.invoke(cm);
                    } catch (Exception e){

                    }
                    if(!mobileDataEnabled)
                    {
                        mReceiver.clearNotification(context);
                        mReceiver.isDataFailNotified=false;
                    }
                    return;
                }
                int netType = info.getType();
                int netSubtype = info.getSubtype();
                Log.d(TAG, "NetType: " + netType + ", netSubType: " +netSubtype);
                if(netType != ConnectivityManager.TYPE_MOBILE){
                    return;
                }
                if(info.isConnected()){
                    mReceiver.clearNotification(context);
                    mReceiver.isDataFailNotified=false;
                }
            }
        }
    }


    SIMProvisioningDetectionReceiver()
    {
        Log.d(TAG, "Called constructor");
    }
    @Override
    public void onReceive(Context context, Intent intent)
    {
        // receving Intent will be config_mobile_hotspot_provision_response
        Log.d(TAG, "Intent is received " + intent);

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

        int apn_type = intent.getIntExtra("apnType", 0);
        // ?? Need to be implemented in DcTracker
        int subId = intent.getIntExtra("SubID", 0);

        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        TelephonyManager tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        NetworkInfo info = cm.getActiveNetworkInfo();
        boolean isMobile=false;
        boolean isLTE=false;
        boolean isEHPRD=false;
        boolean isMIP=false;
        if (info == null || !cm.getBackgroundDataSetting()) {
            Log.d(TAG, "No NetworkInfo");
            isMobile=false;
            return;
        }
        int netType = info.getType();
        int netSubtype = info.getSubtype();
        Log.d(TAG, "NetType: " + netType + ", netSubType: " +netSubtype);
        if(netType == ConnectivityManager.TYPE_MOBILE){
            isMobile=true;
            if( netSubtype == TelephonyManager.NETWORK_TYPE_LTE ||
                netSubtype == TelephonyManager.NETWORK_TYPE_LTE_CA  // SamsungSLSI Specific
                )
                isLTE=true;
            if( netSubtype == TelephonyManager.NETWORK_TYPE_EHRPD){
                isEHPRD=true;
                isMIP=true;
            }
            if( netSubtype == TelephonyManager.NETWORK_TYPE_1xRTT ||
                netSubtype == TelephonyManager.NETWORK_TYPE_CDMA ||
                netSubtype == TelephonyManager.NETWORK_TYPE_EVDO_0 ||
                netSubtype == TelephonyManager.NETWORK_TYPE_EVDO_A ||
                netSubtype == TelephonyManager.NETWORK_TYPE_EVDO_B )
                isMIP=true;
        }


        if(isMobile){
            // ##!GTR-DATA-00254, Currently just support ESM only that's AOSP default notification information
            int LTE_ESM_Code=intent.getIntExtra("errorCode", 0);
            int LTE_EMM_Code=0;
            int EHPRD_Code=0;
            int MIP_Code=0;

            additionalErrorCodes = "";
            additionalErrorCodes +="LTE: ";
            additionalErrorCodes +="ESM-";
            if(isLTE)
                additionalErrorCodes += LTE_ESM_Code;
            else
                additionalErrorCodes += "X";

            additionalErrorCodes += " ";
            additionalErrorCodes +="EMM-";
            if(isLTE)
                additionalErrorCodes += LTE_EMM_Code;
            else
                additionalErrorCodes += "X";

            additionalErrorCodes += "\n";
            additionalErrorCodes +="EHPRD: ";
            if(isEHPRD)
                additionalErrorCodes += EHPRD_Code;
            else
                additionalErrorCodes += "X";
            additionalErrorCodes += "\n";
            additionalErrorCodes +="MIP: ";
            if(isMIP)
                additionalErrorCodes += MIP_Code;
            else
                additionalErrorCodes += "X";
            additionalErrorCodes += "\n";

            // ##!GTR-DATA-00252
            // Launch Persistent NotificationAlarm
            makeNotification(context);

            // Set Flag Connectivity Monitor
            isDataFailNotified = true;
        }
    }

    private void clearNotification(Context context){
        notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.cancel(NOTIFICATION_ID);
    }


    private void makeNotification(Context context){
        Intent intent = new Intent(context, SIMProvisioningDetectionReceiver.class);

        PendingIntent pendingIntent = PendingIntent.getActivity(context, NOTIFICATION_ID, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        Notification.Builder builder = new Notification.Builder(context)
            .setContentTitle(notification)
            //.setContentText(notification + additionalErrorCodes)
            .setContentIntent(pendingIntent)
            .setOngoing(true)
            .setSmallIcon(R.drawable.ic_launcher)
            .setDefaults(Notification.DEFAULT_ALL)
            .setPriority(Notification.PRIORITY_MAX);
            //.setLargeIcon(BitmapFactory.decodeResource(getResources(), R.drawable.ic_launcher))
            ;

        Notification n;
        
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN)
        {
            n = new Notification.BigTextStyle(builder).bigText(additionalErrorCodes).build();
        }
        else {
            n = builder.getNotification();
        }
        n.flags |= Notification.FLAG_NO_CLEAR | Notification.FLAG_ONGOING_EVENT;

        notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(NOTIFICATION_ID, n);
    }

    private boolean checkLTE(Context context)
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
