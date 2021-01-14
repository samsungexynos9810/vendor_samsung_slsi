/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.extservice;

import java.util.ArrayList;

import com.samsung.slsi.telephony.aidl.IDesenseService;
import com.samsung.slsi.telephony.aidl.IDesenseServiceCallback;
import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.util.StringUtil;

import android.app.Service;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

public class DesenseService extends Service {

    private static final String TAG = "DesenseService";
    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_SET_RSSI_SCAN_DONE = 102;
    private static final int EVENT_RSSI_SCAN_RESULT = 103;
    private static final int EVENT_AT_COMMAND_CALLBACK = 104;
    private static final int RILC_REQ_SCAN_RSSI = 96;
    private static final int RILC_REQ_FORWARDING_AT_COMMAND = 97;

    private static final String DESENSE_SERVICE_INTENT = "com.samsung.slsi.telephony.extservice.DesenseService";

    /* RIL request */
    OemRil[] mOemRil = new OemRil[2];
    IDesenseServiceCallback mCb;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int phoneId = 0, error = 0;
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case EVENT_RIL_CONNECTED:
                Log.d(TAG, "RIL connected");
                break;
            case EVENT_RIL_DISCONNECTED:
                Log.d(TAG, "RIL disconnected");
                stopSelf();
                break;
            case EVENT_SET_RSSI_SCAN_DONE:
                phoneId = msg.arg1;
                error = msg.arg2;
                Log.d(TAG, "EVENT_SET_RSSI_SCAN_DONE [SUB" + phoneId + "]");
                if (mCb != null && ar.exception == null) {
                    try {
                      mCb.onResults(error, phoneId);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
                break;
            case EVENT_RSSI_SCAN_RESULT:
                phoneId = (Integer)ar.userObj;
                Log.d(TAG, "EVENT_RSSI_SCAN_RESULT [SUB" + phoneId + "]");
                if (mCb != null && ar.exception == null) {
                    try {
                        byte[] data = StringUtil.arrayListToPrimitiveArray((ArrayList<Byte>)ar.result);
                        mCb.onRssiScanResult(data, phoneId);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
                break;
            case EVENT_AT_COMMAND_CALLBACK:
                phoneId = (Integer)ar.userObj;
                Log.d(TAG, "EVENT_AT_COMMAND_CALLBACK [SUB" + phoneId + "]");
                if (ar.exception == null) {
                    if (isValidPhoneId(phoneId) && mCb != null) {
                        try {
                            String command = (String)ar.result;
                            Log.d(TAG, "at_callback: " + StringUtil.bytesToHexString(command.getBytes()) + " length:" + command.length());
                            mCb.onATCommandCallback(command, phoneId);
                        } catch (RemoteException e) {
                            Log.w(TAG, "", e);
                        }
                    }
                    else {
                        Log.w(TAG, "invalid phoneId or callback");
                    }
                }
                break;
            }
        }
    };

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand()");
        return START_STICKY;
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mOemRil[0].detach();
        mOemRil[1].detach();
        for (int i = 0; i < mOemRil.length; ++i) {
            if (mOemRil[i] == null) {
                Log.d(TAG, "mOemRil is null");
            } else {
                mOemRil[i].unregisterForOemRilConnected(mHandler);
                mOemRil[i].unregisterForOemRilDisconnected(mHandler);
                mOemRil[i].unregisterRssiScanResult(mHandler);
                mOemRil[i].unregisterForATCommandListener(mHandler);
            }
        }
    }

    private void connectToOemRilService() {
        mOemRil[0] = OemRil.init(getApplicationContext(), 0); // SIM0
        mOemRil[1] = OemRil.init(getApplicationContext(), 1); // SIM1

        for (int i = 0; i < mOemRil.length; ++i) {
            if (mOemRil[i] == null) {
                Log.d(TAG, "connectToOemRilService mOemRil is null");
            } else {
                mOemRil[i].registerForOemRilConnected(mHandler, EVENT_RIL_CONNECTED);
                mOemRil[i].registerForOemRilDisconnected(mHandler, EVENT_RIL_DISCONNECTED);
                mOemRil[i].registerRssiScanResult(mHandler, EVENT_RSSI_SCAN_RESULT, i);
                mOemRil[i].registerForATCommandListener(mHandler, EVENT_AT_COMMAND_CALLBACK, i);
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()");

        if ("user".equals(Build.TYPE)) {
            Log.d(TAG, "Not allow to bind in user binary");
            return null;
        }

        if (intent == null || !DESENSE_SERVICE_INTENT.equals(intent.getAction())) {
            Log.d(TAG, "Unexpected intent");
            return null;
        }

        connectToOemRilService();
        if (mDesenseServiceBinder == null) {
            mDesenseServiceBinder = new DesenseServiceBinder();
        }
        return mDesenseServiceBinder;
    }

    public OemRil getOemRil(int phoneId) {
        if (phoneId < 0 || phoneId > mOemRil.length) {
            return null;
        }
        return mOemRil[phoneId];
    }

    private static boolean isValidPhoneId(int phoneId) {
        return (phoneId == 0 || phoneId == 1);
    }

    /********** RssiScanServiceBinder **********/
    private DesenseServiceBinder mDesenseServiceBinder;
    class DesenseServiceBinder extends IDesenseService.Stub {

        @Override
        public void registerATCommandCallback(IDesenseServiceCallback cb) throws RemoteException {
            Log.d(TAG, "registerATCommandCallback()");
            if (cb != null) {
                mCb = cb;
            }
        }

        @Override
        public void setScanRssi(byte[] data, int phoneId) throws RemoteException {
            Log.d(TAG, "setScanRssi()");
            if (!isValidPhoneId(phoneId)) {
                Log.w(TAG, "Invalid phoneId=" + phoneId);
                return;
            }
            mOemRil[phoneId].invokeRequestRaw(RILC_REQ_SCAN_RSSI, data, mHandler.obtainMessage(EVENT_SET_RSSI_SCAN_DONE));
        }

        @Override
        public void sendATCommand(String command, int phoneId) throws RemoteException {
            Log.d(TAG, "sendATCommand() phoneId=" + phoneId);
            if (!isValidPhoneId(phoneId)) {
                Log.w(TAG, "Invalid phoneId=" + phoneId);
                return;
            }

            if (TextUtils.isEmpty(command)) {
                Log.w(TAG, "Invalid command. phoneId=" + phoneId);
                return;
            }

            byte[] data = command.getBytes();
            mOemRil[phoneId].invokeRequestRaw(RILC_REQ_FORWARDING_AT_COMMAND, data, null);
        }
    }
}
