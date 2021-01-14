/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.autoanswer;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.telephony.PhoneStateListener;
import android.telecom.TelecomManager;
import android.telephony.TelephonyManager;
import android.util.Log;

public class CallService extends Service {

    private static final String TAG = "CallService";
    private SharedPreferences mSharedPref;
    private TelephonyManager mTelephonyManager;
    private TelecomManager mTelecomManager;
    private int mTime = 1;
    private boolean mIsChecked = false;
    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {

        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            super.onCallStateChanged(state, incomingNumber);
            if (state == TelephonyManager.CALL_STATE_RINGING) {
                Log.d(TAG, "CallService CALL_STATE_RINGING: " + incomingNumber);
                if (mIsChecked) AcceptCall();
            }
        }
    };

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand()");

        mTelephonyManager = (TelephonyManager) this.getSystemService(Context.TELEPHONY_SERVICE);
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
        mTelecomManager = (TelecomManager) this.getSystemService(Context.TELECOM_SERVICE);
        mSharedPref = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        mIsChecked = mSharedPref.getBoolean(Constants.KEY_AUTO_MODE, false);

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        super.onDestroy();
    }

    public void AcceptCall() {
        Log.d(TAG, "AcceptCall()");
        mTime = Integer.parseInt(mSharedPref.getString(Constants.KEY_ANSWER_TIME, "1"));
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    Thread.sleep(mTime * 1000);
                } catch (InterruptedException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                Log.d(TAG, "Accept in thread!!");
                mTelecomManager.acceptRingingCall();
            }
        }).start();
    }

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }
}
