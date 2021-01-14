/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.oemril;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicInteger;

import static com.samsung.slsi.telephony.oem.OemRilConstants.*;
import com.samsung.slsi.telephony.oem.Am;
import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.io.DataWriter;
import com.samsung.slsi.telephony.oem.util.StringUtil;
import com.samsung.slsi.telephony.aidl.IOemRilService;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbManager;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

public class OemRilService extends Service {

    private static final String TAG = "OemRilService";
    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_SEND_RIL_DATA_DONE = 103;
    private static final int EVENT_CP_RESPONSE_TIMEOUT = 104;
    private static final int EVENT_AM = 105;
    private static final int EVENT_PIN_CONTROL = 106;
    private static final int EVENT_REQUEST_MODEM_LOG_DUMP = 107;
    private static final int EVENT_REQUEST_CANCEL_GET_AVAILABLE_NETWORK = 108;
    private static final int EVENT_REQUEST_RAW_DONE = 200;
    private static final int PIN_CTRL_SIGNAL_DTR = 2;

    private static final String CANCEL_GET_AVAILABLE_NETWORK_ACTION = "com.samsung.slsi.telephony.action.CANCEL_GET_AVAILABLE_NETWORK";
    private static final String CP_LOG_DUMP_ACTION = "com.samsung.slsi.telephony.action.MODEM_LOG_DUMP";
    private BroadcastReceiver sReceiver;

    /* RIL request */
    OemRil[] mOemRil = new OemRil[2];
    private static final int SIM1 = 0;
    private static final int SIM2 = 1;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            int error = 0;

            switch (msg.what) {
            case EVENT_RIL_CONNECTED:
                Log.d(TAG, "RIL connected");
                break;
            case EVENT_RIL_DISCONNECTED:
                Log.d(TAG, "RIL disconnected");
                stopSelf();
                break;
            case EVENT_SEND_RIL_DATA_DONE:
                Log.i(TAG, "EVENT_SEND_RIL_DATA_DONE");
                break;
            case EVENT_CP_RESPONSE_TIMEOUT:
                Log.e(TAG, "CP response timeout!");
                break;
            case EVENT_AM:
                Log.d(TAG, "Receive AM indication");
                doAm(ar);
                break;
            case EVENT_PIN_CONTROL:
                if (ar.exception == null) {
                    Log.d(TAG, "Success to SET_PIN_CONTROL");
                } else {
                    Log.d(TAG, "Fail: " + msg.what);
                }
                break;
            case EVENT_REQUEST_MODEM_LOG_DUMP:
                Log.d(TAG, "EVENT_REQUEST_MODEM_LOG_DUMP");
                error = 0;
                if (ar.exception == null) {
                    byte[] data = (byte [])ar.result;
                    if (data != null && data.length > 0)
                        error = data[0] & 0xFF;
                }
                if (error == 0) {
                    Log.d(TAG, "Success to MODEM_LOG_DUMP");
                } else {
                    Log.d(TAG, "Fail to MODEM_LOG_DUMP");
                }
                break;
            case EVENT_REQUEST_CANCEL_GET_AVAILABLE_NETWORK:
                Log.d(TAG, "EVENT_REQUEST_CANCEL_GET_AVAILABLE_NETWORK");
                error = 0;
                if (ar.exception == null) {
                    byte[] data = (byte [])ar.result;
                    if (data != null && data.length > 0)
                        error = data[0] & 0xFF;
                }
                if (error == 0) {
                    Log.d(TAG, "Success");
                } else {
                    Log.d(TAG, "Fail");
                }
                break;
            case EVENT_REQUEST_RAW_DONE:
                Log.d(TAG, "EVENT_REQUEST_RAW_DONE");
                notifyRequestRawResult(ar);
                break;
            }
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        connectToOemRilService();
        final IntentFilter mFilter = new IntentFilter();
        mFilter.addAction(UsbManager.ACTION_USB_STATE);
        this.sReceiver = new BroadcastReceiver() {

            @Override
            public void onReceive(Context context, Intent intent) {
                if (intent.getAction().equals(UsbManager.ACTION_USB_STATE)) {
                    boolean usbEnabled = intent.getBooleanExtra(UsbManager.USB_CONNECTED, false);
                    int status =  (usbEnabled ? 1 : 0);
                    if (mOemRil[0] != null && status != 2) {
                        setPinControl(PIN_CTRL_SIGNAL_DTR, status,
                                mHandler.obtainMessage(EVENT_PIN_CONTROL));
                    }
                }
            }
        };
        this.registerReceiver(this.sReceiver, mFilter);
    }

    private void connectToOemRilService() {
        mOemRil[0] = OemRil.init(getApplicationContext(), SIM1);
        mOemRil[1] = OemRil.init(getApplicationContext(), SIM2);

        for (int i = 0; i < mOemRil.length; ++i) {
            if (mOemRil[i] == null) {
                Log.d(TAG, "connectToOemRilService mOemRil is null");
            } else {
                mOemRil[i].registerForOemRilConnected(mHandler, EVENT_RIL_CONNECTED);
                mOemRil[i].registerForOemRilDisconnected(mHandler,
                        EVENT_RIL_DISCONNECTED);
                mOemRil[i].registerAm(mHandler, EVENT_AM, null);
            }
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand()");
        if (intent != null) {
            if (CP_LOG_DUMP_ACTION.equals(intent.getAction())) {
                setCPLogDump(mHandler.obtainMessage(EVENT_REQUEST_MODEM_LOG_DUMP));
            } else if (CANCEL_GET_AVAILABLE_NETWORK_ACTION.equals(intent.getAction())) {
                int phoneId = intent.getIntExtra("phoneId", 0);
                Log.d(TAG, "kano in OemRilService phoneId("+phoneId+")");
                cancelGetAvailableNetwork(phoneId, mHandler.obtainMessage(EVENT_REQUEST_CANCEL_GET_AVAILABLE_NETWORK));
            }
        }

        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mOemRil[0].detach();
        mOemRil[1].detach();
    }

    @Override
    public IBinder onBind(Intent arg0) {
        Log.d(TAG, "onBind()");
        return null;
    }

    public OemRil getOemRil(int phoneId) {
        if (phoneId < 0 || phoneId > mOemRil.length) {
            return null;
        }
        return mOemRil[phoneId];
    }

    public void doAm(AsyncResult ar) {
        byte[] data = StringUtil
                .arrayListToPrimitiveArray((ArrayList<Byte>) ar.result);
        try {
            String str = new String(data, "UTF-8").trim();
            Log.d(TAG, "doAm(): " + str);
            (new Am()).run(getApplicationContext(), str);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    public void setPinControl(int signal, int status, Message response) {
        Log.d(TAG, "setPinControl() status: " + status);
        DataWriter out = new DataWriter();
        try {
            out.writeByte((byte) signal);
            out.writeByte((byte) status);
            mOemRil[0].invokeRequestRaw(RILC_REQ_MISC_SET_PIN_CONTROL,
                    out.toByteArray(), response);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void setCPLogDump(Message response) {
        Log.d(TAG, "setCPLogDump()");
        mOemRil[0].invokeRequestRaw(RILC_REQ_SET_MODEM_LOG_DUMP, null, response);
    }

    public void cancelGetAvailableNetwork(int phoneId, Message response) {
        Log.d(TAG, "cancelGetAvailableNetwork()");
        mOemRil[phoneId].invokeRequestRaw(RILC_REQ_CANCEL_GET_AVAILABLE_NETWORK, null, response);
    }

    /********** OemRilServiceBinder **********/
    private OemRilServiceBinder mOemRilServiceBinder;
    private AtomicInteger mNextRequestId = new AtomicInteger(1);

    class OemRilServiceBinder extends IOemRilService.Stub {

        @Override
        public int invokeOemRilRequestRaw(int request, byte[] data, Messenger messenger, IBinder binder, int phoneId) throws RemoteException {
            int serial = mNextRequestId.getAndIncrement();
            Log.d(TAG, "request= " + request + " ,serial=" + serial + " ,raw=" + StringUtil.bytesToHexString(data));

            RequestInfo requestInfo = new RequestInfo(request, serial, messenger, binder, phoneId);
            Message message = Message.obtain(mHandler, EVENT_REQUEST_RAW_DONE, requestInfo);

            if (getOemRil(phoneId) != null) {
                getOemRil(phoneId).invokeRequestRaw(request, data, message);
            } else {
                Log.e(TAG, "OemRil is null. Fail: invokeRequestRaw");
                AsyncResult.forMessage(message, null, new Exception());
                mHandler.sendMessageDelayed(message, 100);
            }
            return serial;
        }
    }

    private void notifyRequestRawResult(AsyncResult ar) {
        Log.d(TAG, "notifyRequestRawResult()");

        if (ar == null) {
            return;
        }

        RequestInfo request = (RequestInfo) ar.userObj;
        byte[] result = (byte[]) ar.result;

        if (request.getIsBinderDead()) {
            Log.d(TAG, "Binder is already dead");
            return;
        }

        Messenger messenger = request.mMessenger;
        Message message = Message.obtain();
        message.what = EVENT_REQUEST_RAW_DONE;
        message.arg1 = (ar.exception == null) ? 0 : 1;
        message.arg2 = request.mSerial;

        Log.d(TAG, request.toString());
        if (result != null) {
            Bundle b = new Bundle();
            b.putInt("request", request.mRequest);
            b.putInt("phoneId", request.mPhoneId);
            b.putByteArray("result", result);
            message.setData(b);
        } else {
            message.obj = null;
        }

        try {
            messenger.send(message);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    class RequestInfo implements IBinder.DeathRecipient {
        int mRequest;
        int mSerial;
        Messenger mMessenger;
        IBinder mBinder;
        int mUid;
        int mPid;
        int mPhoneId;
        boolean mIsBinderDead;

        RequestInfo(int request, int serial, Messenger messenger, IBinder binder, int phoneId) {
            mRequest = request;
            mSerial = serial;
            mMessenger = messenger;
            mBinder = binder;
            mUid = Binder.getCallingUid();
            mPid = Binder.getCallingPid();
            mPhoneId = phoneId;
            mIsBinderDead = false;

            try {
                mBinder.linkToDeath(this, 0);
            } catch (RemoteException e) {
                Log.d(TAG, "", e);
                binderDied();
            }
        }

        synchronized void setIsBinderDead(boolean val) {
            mIsBinderDead = val;
        }

        synchronized boolean getIsBinderDead() {
            return mIsBinderDead;
        }

        @Override
        public void binderDied() {
            setIsBinderDead(true);
        }

        void unlinkDeathRecipient() {
            if (mBinder != null) {
                mBinder.unlinkToDeath(this, 0);
            }
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("RequestInfo {");
            sb.append(" mRequest=" + mRequest);
            sb.append(" mSerial=" + mSerial);
            sb.append(" mMessenger=" + mMessenger);
            sb.append(" mBinder=" + mBinder);
            sb.append(" mUid=" + mUid);
            sb.append(" mPid=" + mPid);
            sb.append(" mPhoneId=" + mPhoneId);
            sb.append(" mIsBinderDead=" + mIsBinderDead);
            sb.append(" }");
            return sb.toString();
        }
    }
}
