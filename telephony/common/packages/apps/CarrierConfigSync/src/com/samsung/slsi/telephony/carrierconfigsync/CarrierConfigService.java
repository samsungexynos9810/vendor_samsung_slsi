package com.samsung.slsi.telephony.carrierconfigsync;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.telephony.CarrierConfigManager;
import android.util.Log;

public class CarrierConfigService extends Service {

    private static final String TAG = "CarrierConfigService";
    private CarrierConfigManager mCarrierConfigManager = null;

    private Messenger mServiceMessenger = null;
    private static boolean mBindserviceFlag = false;
    private String mData = "";
    private int mPhoneId = 0, mSubId = 0;
    private static WakeLock mWakeLock;

    private static final int RILC_REQ_MISC_SET_CARRIER_CONFIG = 5;    /*Set Carrier Config Command */
    private static final int SET_CARRIER_CONFIG_DONE = 0;
    private static final int MSG_CARRIER_CONFIG_DONE = 1;

    public Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case SET_CARRIER_CONFIG_DONE:
                    Log.d(TAG, "SET_CARRIER_CONFIG_DONE");
                    break;
                case MSG_CARRIER_CONFIG_DONE:
                    Log.d(TAG, "MSG_CARRIER_CONFIG_DONE");
                    stopSelf();
                    break;
                default:
                    break;
            }
        }
    };
    private Messenger mResponseMessenger = new Messenger(mHandler);

    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate()");
        PowerManager pm = (PowerManager) getSystemService(POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        mWakeLock.setReferenceCounted(false);
        mCarrierConfigManager = (CarrierConfigManager) this.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        mBindserviceFlag = false;
        connectToJniRilService();
    }

    @Override
    public void onStart(Intent intent, int startId) {
        Log.i(TAG, "onStart()");

        mPhoneId = intent.getIntExtra("phoneid", 0);
        mSubId = intent.getIntExtra("subid", 0);
        Log.d(TAG, "phoneid: " + mPhoneId + ", subid: " + mSubId);
        if (mServiceMessenger != null) {
            doCheckCarrierConfig();
        }
    }

    private void doCheckCarrierConfig() {
        if (mServiceMessenger != null) {
            mData = "";
            for (String key : mCarrierConfigManager.getConfigForSubId(mSubId).keySet()) {
                Object value = mCarrierConfigManager.getConfigForSubId(mSubId).get(key);
                if (value != null && value.toString().trim().length() > 0) {
                    if (value instanceof String[]) {
                        StringBuilder sb = new StringBuilder();

                        for (String s : (String[])value) {
                            sb.append(s);
                            sb.append(",");
                        }
                        value = sb.substring(0, sb.length()-1);
                    }

                    mData += key;
                    mData += "|";
                    mData += value;
                    mData += "|";

                    if (mData.length() > 60) {
                        Log.e(TAG, mData);
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                        SendRilData(RILC_REQ_MISC_SET_CARRIER_CONFIG, mData.length(), mData);
                        mData = "";
                    }
                }
            }

            if (mData.length() > 0) {
                Log.e(TAG, mData);
                SendRilData(RILC_REQ_MISC_SET_CARRIER_CONFIG, mData.length(), mData);
                mHandler.sendEmptyMessageDelayed(MSG_CARRIER_CONFIG_DONE, 100);
            }
        }
    }

    private void SendRilData(int cmd, int len, String writedata) {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(bos);

        byte[] data = null;

        try {
            dos.writeByte(cmd);
            dos.writeByte(0);
            dos.writeByte(0);
            dos.writeByte(0);

            //dos.writeInt(0);
            dos.writeByte(len);
            dos.writeByte(0);
            dos.writeByte(0);
            dos.writeByte(0);

            dos.writeBytes(writedata);
            //dos.writeByte(0);
            //dos.writeByte(0);
            //dos.writeByte(0);

            data = bos.toByteArray();
        } catch (IOException e) {
            Log.i(TAG, "SendData() IOException" + e);
        }
        invokeOemRilRequest(data, mHandler.obtainMessage(SET_CARRIER_CONFIG_DONE));
    }


    private ServiceConnection mPhoneServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            Log.d(TAG, "Connect to SecJniRilService");
            mServiceMessenger = new Messenger(service);
            mBindserviceFlag = true;

            doCheckCarrierConfig();
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            Log.d(TAG, "Disconnected from SecJniRilService");
            mServiceMessenger = null;
        }
    };

    private void connectToJniRilService() {
        Intent intent = new Intent();
        intent.setClassName("com.sec.jniril", "com.sec.jniril.SecJniRilService");
        bindService(intent, mPhoneServiceConnection, BIND_AUTO_CREATE);
        Log.d(TAG, "connectToJniRilService");
        if (mServiceMessenger == null){
            Log.d(TAG, "connectToJniRilService mServiceMessenger is null");
        }
    }

    private void invokeOemRilRequest(byte[] data, Message response) {

        Bundle req = response.getData();
        req.putByteArray("request", data);
        req.putInt("phoneid", mPhoneId);
        response.setData(req);
        response.replyTo = mResponseMessenger;

        try {
            if (mServiceMessenger != null)
                mServiceMessenger.send(response);
            else
                Log.d(TAG, "mServiceMessenger is null. Do nothing.");
        } catch (RemoteException e) {
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy()");
        unbindService(mPhoneServiceConnection);
        mPhoneServiceConnection = null;
        mBindserviceFlag = false;
    }

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }
}
