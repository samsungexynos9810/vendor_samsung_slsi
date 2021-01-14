/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.aidl;

import com.samsung.slsi.telephony.oem.util.StringUtil;
import static com.samsung.slsi.telephony.oem.OemRilConstants.requestToString;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.util.SparseArray;

public class OemRilBinderManager {

    public static final String TAG = "OemRilBinderManager";
    public static final int EVENT_REQUEST_RAW_RESULT = 200;
    private HandlerThread mHandlerThread;
    private Messenger mMessenger;
    private SparseArray<Message> mResultList;
    private Context mContext;
    private IOemRilService mOemRilService;
    private boolean mConnected;

    private ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected: name=" + name);
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected: name=" + name + " service=" + service);
            mConnected = true;
            mOemRilService = IOemRilService.Stub.asInterface(service);
        }

        @Override
        public void onBindingDied(ComponentName name) {
            Log.d(TAG, "onBindingDied: name=" + name);
        }
    };

    public static OemRilBinderManager from(Context context) {
        return new OemRilBinderManager(context);
    }

    OemRilBinderManager(Context context) {
        Log.d(TAG, "OemRilBinderManager()");
        mContext = context;

        mHandlerThread = new HandlerThread("oemril_bind_service");
        mHandlerThread.start();
        mMessenger = new Messenger(new Handler(mHandlerThread.getLooper()) {

            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case EVENT_REQUEST_RAW_RESULT:
                    Log.d(TAG, "EVENT_REQUEST_RAW_RESULT");
                    onRequestRawResult(msg);
                    break;
                } // end switch ~
            }
        });

        mResultList = new SparseArray<>();
        Intent intent = new Intent();
        intent.setComponent(new ComponentName("com.samsung.slsi.telephony.oemril","com.samsung.slsi.telephony.oemril.OemRilService"));
        mContext.bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
    }

    public void finish() {
        if (mConnected) {
            mContext.unbindService(mServiceConnection);
            mConnected = false;
            mOemRilService = null;
        }

        if (mHandlerThread != null) {
            mHandlerThread.quitSafely();
            mHandlerThread = null;
        }
    }

    protected void onRequestRawResult(Message msg) {
        Log.d(TAG, "onRequestRawResult");
        if (msg == null) {
            return ;
        }

        int errorCode = msg.arg1;
        int serial = msg.arg2;
        byte[] result = null;
        Exception ex = null;
        if (errorCode == 0) {
            Bundle b = msg.getData();
            int request = b.getInt("request");
            int phoneId = b.getInt("phoneId");
            result = b.getByteArray("result");
            Log.e(TAG, "request: " + request);
            Log.v(TAG, "[" + TAG + "_" + serial + "]< " + requestToString(request) + " " + StringUtil.bytesToHexString(result) + " [SUB" + phoneId + "]");
        }
        else {
            ex = new Exception();
            Log.d(TAG, "exception");
        }

        Message onComplete = mResultList.get(serial);
        mResultList.remove(serial);
        if (onComplete != null) {
            AsyncResult.forMessage(onComplete, result, ex);
            onComplete.sendToTarget();
        }
    }

    public void invokeOemRilRequestRaw(int request, byte[] data, Message onComplete, int phoneId) {
        try {
            int serial = mOemRilService.invokeOemRilRequestRaw(request, data, mMessenger, new Binder(), phoneId);
            mResultList.put(serial, onComplete);
            Log.v(TAG, "[" + TAG + "_" + serial + "]> " + requestToString(request) + " " + StringUtil.bytesToHexString(data) + " [SUB" + phoneId + "]");
        } catch (RemoteException e) {
            Log.d(TAG, "", e);
            e.printStackTrace();
        }
    }
}
