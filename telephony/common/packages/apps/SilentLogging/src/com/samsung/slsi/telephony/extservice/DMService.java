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
import android.util.Log;

import com.samsung.slsi.telephony.aidl.IDMService;
import com.samsung.slsi.telephony.aidl.IDMServiceCallback;
import com.samsung.slsi.telephony.oemservice.OemService;
import com.samsung.slsi.telephony.oemservice.OemServiceConstants;

public class DMService extends Service {

    private static final String TAG = "DMService";
    private static final int EVENT_SILENT_LOGGING_STARTED = 0;
    private static final int EVENT_SILENT_LOGGING_STOPED = 1;
    private static final int EVENT_NOTIFY_DM_LOG = 2;

    private OemService mOemService;
    private IDMServiceCallback mCb;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case EVENT_SILENT_LOGGING_STARTED:
                break;
            case EVENT_SILENT_LOGGING_STOPED:
                break;
            case EVENT_NOTIFY_DM_LOG:
                if (mCb != null && ar.exception == null) {
                    try {
                        byte[] data = arrayListToPrimitiveArray((ArrayList<Byte>)ar.result);
                        mCb.notifyDmLog(data);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
                break;
            default:
                break;
            }
        }
    };

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        Log.d(TAG, "onCreate()");
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand()");
        connectToOemService();
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();
    }

    private void connectToOemService() {
        mOemService = OemService.init(this);
        if (mOemService == null) {
            Log.e(TAG, "connectToOemService : mOemService is null");
        } else {
            mOemService.registerNotifyDmLog(mHandler, EVENT_NOTIFY_DM_LOG);
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO Auto-generated method stub
        Log.d(TAG, "onCreate()");
        if (mDMServiceBinder == null) {
            mDMServiceBinder = new DMServiceBinder();
        }
        return mDMServiceBinder;
    }

    public OemService getOemService() {
        if (mOemService != null)
            return mOemService;
        return null;
    }

    public static byte[] arrayListToPrimitiveArray(ArrayList<Byte> bytes) {
        byte[] ret = new byte[bytes.size()];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = bytes.get(i);
        }
        return ret;
    }

    /* DMServiceBinder */
    private DMServiceBinder mDMServiceBinder;
    class DMServiceBinder extends IDMService.Stub {

        @Override
        public void registerCallback(IDMServiceCallback cb) throws RemoteException {
            Log.d(TAG, "registerCallback()");
            if (cb != null) {
                mCb = cb;
            }
        }

        @Override
        public void setExtAppMode() throws RemoteException {
            Log.d(TAG, "setExtAppMode()");
            mOemService.setDmMode(OemServiceConstants.MODE_EXT_APP_DM);
        }

        @Override
        public void sendToModem(byte[] data) throws RemoteException {
            Log.d(TAG, "sendToModem");
            mOemService.sendToModem(data);
        }
    }

}
