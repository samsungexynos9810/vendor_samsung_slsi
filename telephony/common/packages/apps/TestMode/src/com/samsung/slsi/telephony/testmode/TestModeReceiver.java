/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.testmode;

import android.app.AlertDialog;
import android.os.SystemProperties;
import android.os.SystemService;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Handler;
import android.os.PowerManager;
import android.text.TextUtils;
import android.util.Log;
import android.util.SparseArray;
import android.view.WindowManager;

public class TestModeReceiver extends BroadcastReceiver {

    private static final String TAG = "TestModeReceiver";
    private static final String ACTION_BOOT_COMPLETED = "android.intent.action.BOOT_COMPLETED";
    private static final String ACTION_REJECT_REG = "com.samsung.slsi.action.REJECT_REG";
    private static final String ACTION_POWER = "com.samsung.slsi.action.POWER";
    private static final String ACTION_DATA_STATE = "com.samsung.slsi.action.DATA_STATE";
    private static final String DATA_REJECT_CAUSE = "rej_cause";
    private static final String DATA_RESET_TYPE = "reset_type";
    private static final String DATA_DATA_STATE = "data_state";
    private static final String DATA_PHONE_ID = "phone_id";
    private static final String PROPERTY_RIL_REJECT_CAUSE = "vendor.ril.net.rejectcause";
    private static final String PROPERTY_RIL_RESET_TYPE = "vendor.ril.reset_type";
    private static final String PROPERTY_RIL_DATA_STATE = "vendor.ril.data_state";
    private static final int DISMISS_TIME = 10;
    private static final int REJECT_PERIOD = 120;
    static AlertDialog dialog;
    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        mContext = context;
        if (intent.getAction().equals(ACTION_BOOT_COMPLETED)) {
            Log.d(TAG, "BOOT_COMPLETE");
            // checking reject cause
            String rejCause = SystemProperties.get(PROPERTY_RIL_REJECT_CAUSE);
            if (!TextUtils.isEmpty(rejCause)) {
                showRejectCauseDialog(Integer.parseInt(rejCause));
            }

            // checking reset type
            int resetType = Integer.parseInt(SystemProperties.get(PROPERTY_RIL_RESET_TYPE, "0"));
            showPowerDialog(resetType);
        }
        else if (intent.getAction().equals(ACTION_REJECT_REG)) {
            // request to show notification to user according to reject cause
            showRejectCauseDialog(intent.getIntExtra(DATA_REJECT_CAUSE, 13));
        }
        else if (intent.getAction().equals(ACTION_POWER)) {
            // UNSOL_PHONE_RESET
            int resetType = intent.getIntExtra(DATA_RESET_TYPE, 0);
            Log.d(TAG, "reset type: " + resetType);
            showPowerDialog(resetType);
        }
        else if (intent.getAction().equals(ACTION_DATA_STATE)) {
            // UNSOL_DATA_STATE_CHANGE
            int dataState = intent.getIntExtra(DATA_DATA_STATE, 0);
            int phoneId = intent.getIntExtra(DATA_PHONE_ID, 0);
            SubscriptionManager sb = (SubscriptionManager) context.getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
            int subId = sb.getSubId(phoneId)[0];
            Log.d(TAG, "data state: " + dataState + ", sub id: " + subId + ", phone id: " + phoneId);
            switch(dataState) {
            case 0:
                setDataEnabled(subId, false); // disconnect
                break;
            case 1:
                setDataEnabled(subId, true); // connect
                break;
            default:
                break;
            }
        }
    }

    static SparseArray<String> sPowerMessage = new SparseArray<String>();
    static SparseArray<String> sRejectCause = new SparseArray<String>();
    static {
        sPowerMessage.put(2, "Reboot the device");
        sPowerMessage.put(3, "Shutdown the device");

        sRejectCause.put(2, "SIM not provisioned MM#2");
        sRejectCause.put(3, "SIM not allowed MM#3");
        sRejectCause.put(6, "Phone not allowed MM#6");
        sRejectCause.put(13, "SOS/Emergency calls only");
    }

    private void setDataEnabled(int subId, boolean enable) {
        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        tm.setDataEnabled(subId, enable);
    }

    private void showPowerDialog(final int resetType) {
        final String message = sPowerMessage.get(resetType);
        final PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        if (TextUtils.isEmpty(message)) return;

        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        builder.setMessage(message).setCancelable(false)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (resetType == 2) {
                            pm.reboot(null);
                        } else if (resetType == 3) {
                            pm.shutdown(false, null, false);
                        }
                    }
                });
        final AlertDialog resetDialog = builder.create();
        resetDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        resetDialog.show();

        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (resetDialog.isShowing()) {
                    resetDialog.dismiss();
                    if (resetType == 2) {
                        pm.reboot(null);
                    } else if (resetType == 3) {
                        pm.shutdown(false, null, false);
                    }
                }
            }
        }, (DISMISS_TIME*1000));
    }

    public void showRejectCauseDialog(int rejectCause) {
        Log.d("TestModeReceiver", DATA_REJECT_CAUSE  + " : "+ rejectCause + ", period: " + REJECT_PERIOD);
        String message = sRejectCause.get(rejectCause);
        if (!TextUtils.isEmpty(message)) {
            if (dialog != null && dialog.isShowing()) {
                dialog.dismiss();
            }
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setMessage(message);
            builder.setPositiveButton(android.R.string.ok, null);
            builder.setCancelable(true);
            dialog = builder.create();
            dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
            dialog.show();

            Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    if (dialog.isShowing()) {
                        dialog.dismiss();
                    }
                }
            }, (REJECT_PERIOD*1000));
        }
    }
}
