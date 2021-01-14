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

import java.io.IOException;

import com.samsung.slsi.telephony.aidl.ISensorService;
import com.samsung.slsi.telephony.aidl.ISensorServiceCallback;
import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.io.DataWriter;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

public class SensorService extends Service {

    private static final String TAG = "SensorService";
    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_PSENSOR_SET_STATUS_DONE = 102;
    private static final int RILC_REQ_PSENSOR_SET_STATUS = 401;

    private static final String SENSOR_SERVICE_INTENT = "com.samsung.slsi.telephony.extservice.SensorService";

    /* RIL request */
    OemRil[] mOemRil = new OemRil[2];
    ISensorServiceCallback mCb;

    private static final int SIM1 = 0;
    private static final int SIM2 = 1;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int phoneId = 0, error = 1;
            switch (msg.what) {
            case EVENT_RIL_CONNECTED:
                Log.d(TAG, "RIL connected");
                break;
            case EVENT_RIL_DISCONNECTED:
                Log.d(TAG, "RIL disconnected");
                stopSelf();
                break;
            case EVENT_PSENSOR_SET_STATUS_DONE:
                phoneId = msg.arg1;
                error = msg.arg2;
                Log.d(TAG, "EVENT_PSENSOR_SET_STATUS_DONE [SUB" + phoneId + "]");
                if (mCb != null) {
                    try {
                        mCb.setSensorStatusRsp(error, phoneId);
                    } catch (RemoteException e) {
                        e.printStackTrace();
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
            }
        }
    }

    private void connectToOemRilService() {
        mOemRil[0] = OemRil.init(getApplicationContext(), SIM1);
        mOemRil[1] = OemRil.init(getApplicationContext(), SIM2);

        for (int i = 0; i < mOemRil.length; ++i) {
            if (mOemRil[i] == null) {
                Log.d(TAG, "connectToOemRilService mOemRil is null");
            } else {
                mOemRil[i].registerForOemRilConnected(mHandler, EVENT_RIL_CONNECTED);
                mOemRil[i].registerForOemRilDisconnected(mHandler, EVENT_RIL_DISCONNECTED);
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()");
        if (intent == null || !SENSOR_SERVICE_INTENT.equals(intent.getAction())) {
            Log.d(TAG, "Unexpected intent");
            return null;
        }

        connectToOemRilService();

        if (mSensorServiceBinder == null) {
            mSensorServiceBinder = new SensorServiceBinder();
        }

        return mSensorServiceBinder;
    }

    public OemRil getOemRil(int phoneId) {
        if (phoneId < 0 || phoneId > mOemRil.length) {
            return null;
        }
        return mOemRil[phoneId];
    }

    /********** SensorServiceBinder **********/
    private SensorServiceBinder mSensorServiceBinder;
    class SensorServiceBinder extends ISensorService.Stub {

        @Override
        public void registerCallback(ISensorServiceCallback cb) throws RemoteException {
            Log.d(TAG, "registerCallback()");
            if (cb != null) {
                mCb = cb;
            }
        }

        @Override
        public void setSensorStatus(int status, int phoneId) throws RemoteException {
            Log.d(TAG, "setState() status=" + status);
            if (phoneId < 0 || phoneId > mOemRil.length) {
                return;
            }
            DataWriter out = new DataWriter();
            try {
                out.writeInt(status);
                mOemRil[phoneId].invokeRequestRaw(RILC_REQ_PSENSOR_SET_STATUS, out.toByteArray(), mHandler.obtainMessage(EVENT_PSENSOR_SET_STATUS_DONE));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
