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
import java.util.ArrayList;

import com.samsung.slsi.telephony.aidl.ISarService;
import com.samsung.slsi.telephony.aidl.ISarServiceCallback;
import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.io.DataReader;
import com.samsung.slsi.telephony.oem.io.DataWriter;
import com.samsung.slsi.telephony.oem.util.StringUtil;

import android.app.Service;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.util.Log;
import android.widget.Toast;

public class SarService extends Service {

    private static final String TAG = "SarService";
    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_SET_STATE_DONE = 102;
    private static final int EVENT_GET_STATE_DONE = 103;
    private static final int EVENT_NOTIFY_RF_CONNECTION = 104;
    private static final int RILC_REQ_SAR_SET_SAR_STATE = 501;
    private static final int RILC_REQ_SAR_GET_SAR_STATE = 502;

    private static final String SAR_SERVICE_INTENT = "com.samsung.slsi.telephony.extservice.SarService";
    private static final String RIL_VENDOR_RF_CONNECTION = "vendor.ril.rf.connection";

    /* RIL request */
    OemRil[] mOemRil = new OemRil[2];
    ISarServiceCallback mCb;

    private static final int SIM1 = 0;
    private static final int SIM2 = 1;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int phoneId = 0, error = 1;
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case EVENT_RIL_CONNECTED:
                Log.d(TAG, "RIL connected");
                break;
            case EVENT_RIL_DISCONNECTED:
                Log.d(TAG, "RIL disconnected");
                stopSelf();
                break;
            case EVENT_SET_STATE_DONE:
                phoneId = msg.arg1;
                error = msg.arg2;
                Log.d(TAG, "EVENT_SET_STATE_DONE [SUB" + phoneId + "]");
                if (mCb != null) {
                    try {
                        mCb.setSarStateRsp(error, phoneId);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
                break;
            case EVENT_GET_STATE_DONE:
                phoneId = msg.arg1;
                error = msg.arg2;
                Log.d(TAG, "EVENT_GET_STATE_DONE [SUB" + phoneId + "]");
                if (mCb != null && ar.exception == null) {
                    DataReader dr = new DataReader((byte[])ar.result);
                    try {
                        int state = dr.getInt();
                        mCb.getSarStateRsp(error, state, phoneId);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                break;
            case EVENT_NOTIFY_RF_CONNECTION:
                phoneId = (Integer)ar.userObj;
                Log.d(TAG, "EVENT_NOTIFY_RF_CONNECTION [SUB" + phoneId + "]");
                if (mCb != null && ar.exception == null) {
                    try {
                        int rfstate = (Integer)ar.result;
                        mCb.notifyRfConnection(rfstate, phoneId);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
                break;
            }
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
    }


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
                mOemRil[i].unregisterSarRfConnection(mHandler);
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
                mOemRil[i].registerSarRfConnection(mHandler, EVENT_NOTIFY_RF_CONNECTION, i);
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()");
        if (intent == null || !SAR_SERVICE_INTENT.equals(intent.getAction())) {
            Log.d(TAG, "Unexpected intent");
            return null;
        }

        connectToOemRilService();
        if (mSarServiceBinder == null) {
            mSarServiceBinder = new SarServiceBinder();
        }

        return mSarServiceBinder;
    }

    public OemRil getOemRil(int phoneId) {
        if (phoneId < 0 || phoneId > mOemRil.length) {
            return null;
        }
        return mOemRil[phoneId];
    }

    private void updateRfConnection() {
        Log.d(TAG, "updateRfConnection()");
        int rfConn = SystemProperties.getInt(RIL_VENDOR_RF_CONNECTION, 0);
        Log.d(TAG, "rfConn : " + rfConn);
        Message msg = mHandler.obtainMessage(EVENT_NOTIFY_RF_CONNECTION);
        msg.obj = 0;
        AsyncResult.forMessage(msg, rfConn, null);
        msg.sendToTarget();
    }

    /********** SarServiceBinder **********/
    private SarServiceBinder mSarServiceBinder;
    class SarServiceBinder extends ISarService.Stub {

        @Override
        public void registerCallback(ISarServiceCallback cb) throws RemoteException {
            Log.d(TAG, "registerCallback()");
            if (cb != null) {
                mCb = cb;
                updateRfConnection();
            }
        }

        @Override
        public void setState(int state, int phoneId) throws RemoteException {
            Log.d(TAG, "setState() state=" + state);
            if (phoneId < 0 || phoneId > mOemRil.length) {
                return;
            }
            DataWriter out = new DataWriter();
            try {
                out.writeInt(state);
                mOemRil[phoneId].invokeRequestRaw(RILC_REQ_SAR_SET_SAR_STATE, out.toByteArray(), mHandler.obtainMessage(EVENT_SET_STATE_DONE));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void getState(int phoneId) throws RemoteException {
            Log.d(TAG, "getState()");
            if (phoneId < 0 || phoneId > mOemRil.length) {
                return;
            }
            mOemRil[phoneId].invokeRequestRaw(RILC_REQ_SAR_GET_SAR_STATE, null, mHandler.obtainMessage(EVENT_GET_STATE_DONE));
        }
    }
}
