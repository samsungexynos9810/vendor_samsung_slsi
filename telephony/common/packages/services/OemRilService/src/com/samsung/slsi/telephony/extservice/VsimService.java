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

import android.app.Service;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.samsung.slsi.telephony.aidl.IVsimService;
import com.samsung.slsi.telephony.aidl.IVsimServiceCallback;
import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.io.DataReader;
import com.samsung.slsi.telephony.oem.io.DataWriter;
import com.samsung.slsi.telephony.oem.util.StringUtil;

public class VsimService extends Service {

    private static final String TAG = "VsimService";
    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_VSIM_NOTIFICATION_DONE = 102;
    private static final int EVENT_VSIM_OPERATION_DONE = 103;
    private static final int EVENT_NOTIFY_VSIM_OPERATION = 104;
    private static final int RILC_REQ_VSIM_NOTIFICATION = 451;
    private static final int RILC_REQ_VSIM_OPERATION = 452;

    private static final String VSIM_SERVICE_INTENT = "com.samsung.slsi.telephony.extservice.VsimService";

    /* RIL request */
    OemRil[] mOemRil = new OemRil[2];
    IVsimServiceCallback mCb;

    private static final int SIM1 = 0;
    private static final int SIM2 = 1;

    private final Handler mHandler = new Handler() {
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
            case EVENT_VSIM_NOTIFICATION_DONE:
                phoneId = msg.arg1;
                error = msg.arg2;
                Log.d(TAG, "EVENT_VSIM_NOTIFICATION_DONE [SUB" + phoneId + "]");
                if (mCb != null) {
                    try {
                        mCb.sendVsimNotificationRsp(error, phoneId);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
                break;
            case EVENT_VSIM_OPERATION_DONE:
                phoneId = msg.arg1;
                error = msg.arg2;
                Log.d(TAG, "EVENT_VSIM_OPERATION_DONE [SUB" + phoneId + "]");
                if (mCb != null) {
                    try {
                        mCb.sendVsimNotificationRsp(error, phoneId);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
                break;
            case EVENT_NOTIFY_VSIM_OPERATION:
                phoneId = (Integer)ar.userObj;
                Log.d(TAG, "EVENT_NOTIFY_VSIM_OPERATION [SUB" + phoneId + "]");
                if (mCb != null && ar.exception == null && ar.result != null) {
                    byte[] res = StringUtil.arrayListToPrimitiveArray((ArrayList<Byte>)ar.result);
                    if (res.length < 16) {
                        Log.e(TAG, "Invalid EVENT_NOTIFY_VSIM_OPERATION response data. size=" + res.length);
                        return;
                    }

                    DataReader dr = new DataReader(res);
                    try {
                        int transactionId = dr.getInt();
                        int eventId = dr.getInt();
                        int result = dr.getInt();
                        int dataLength = dr.getInt();
                        Log.d(TAG, "data length=" + dataLength);
                        String data = "";
                        if (dataLength > 0) {
                            byte[] opData = dr.getBytes(dataLength);
                            Log.d(TAG, "data= " + StringUtil.bytesToHexString(opData));
                            data = new String(opData, "UTF-8").trim();
                        }
                        mCb.notifyVsimOpereation(transactionId, eventId, result, data, phoneId);
                    } catch (IOException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
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
                mOemRil[i].unregisterVsimOperation(mHandler);
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
                mOemRil[i].registerVsimOperation(mHandler, EVENT_NOTIFY_VSIM_OPERATION, i);
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()");
        if (intent == null || !VSIM_SERVICE_INTENT.equals(intent.getAction())) {
            Log.d(TAG, "Unexpected intent");
            return null;
        }

        connectToOemRilService();
        if (mVsimServiceBinder == null) {
            mVsimServiceBinder = new VsimServiceBinder();
        }
        return mVsimServiceBinder;
    }

    public OemRil getOemRil(int phoneId) {
        if (phoneId < 0 || phoneId > mOemRil.length) {
            return null;
        }
        return mOemRil[phoneId];
    }

    /********** VsimServiceBinder **********/
    private VsimServiceBinder mVsimServiceBinder;
    class VsimServiceBinder extends IVsimService.Stub {

        @Override
        public void registerCallback(IVsimServiceCallback cb) throws RemoteException {
            Log.d(TAG, "registerCallback()");
            if (cb != null) {
                mCb = cb;
            }
        }

        @Override
        public void sendVsimNotification(int transactionId, int eventId, int simType, int phoneId) throws RemoteException {
            Log.d(TAG, "sendVsimNotification() transactionId=" + transactionId + ", eventId=" + eventId + ", simType=" + simType);
            if (phoneId < 0 || phoneId > mOemRil.length) {
                return;
            }
            DataWriter out = new DataWriter();
            try {
                out.writeInt(transactionId);
                out.writeInt(eventId);
                out.writeInt(simType);
                mOemRil[phoneId].invokeRequestRaw(RILC_REQ_VSIM_NOTIFICATION, out.toByteArray(), mHandler.obtainMessage(EVENT_VSIM_NOTIFICATION_DONE));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void sendVsimOperation(int transactionId, int eventId, int result, String data, int phoneId) throws RemoteException {
            Log.d(TAG, "sendVsimOperation() transactionId=" + transactionId + ", eventId=" + eventId + ", result=" + result + ", data=" + data);
            DataWriter out = new DataWriter();
            try {
                out.writeInt(transactionId);
                out.writeInt(eventId);
                out.writeInt(result);
                if (TextUtils.isEmpty(data)) {
                     out.writeInt(0);
                } else {
                     out.writeInt(data.length());
                     out.writeBytes(data.getBytes());
                }
                mOemRil[phoneId].invokeRequestRaw(RILC_REQ_VSIM_OPERATION, out.toByteArray(), mHandler.obtainMessage(EVENT_VSIM_OPERATION_DONE));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
