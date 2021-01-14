package com.samsung.slsi.telephony.extservice;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.ArrayList;

import android.app.Service;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;

import com.samsung.slsi.telephony.aidl.IDMControlService;
import com.samsung.slsi.telephony.aidl.IDMControlServiceCallback;
import com.samsung.slsi.telephony.oemservice.OemService;
import com.samsung.slsi.telephony.oemservice.OemServiceConstants;

import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.io.DataReader;
import com.samsung.slsi.telephony.oem.io.DataWriter;
import com.samsung.slsi.telephony.oem.util.StringUtil;

import com.samsung.slsi.telephony.silentlogging.SilentLoggingConstant;
import com.samsung.slsi.telephony.silentlogging.SilentLoggingControlInterface;

public class DMControlService extends Service {

    private static final String TAG = "DMControlService";
    private static final int EVENT_SILENT_LOGGING_START = 0;
    private static final int EVENT_SILENT_LOGGING_START_DONE = 1;
    private static final int EVENT_SILENT_LOGGING_STOP = 2;
    private static final int EVENT_SILENT_LOGGING_STOP_DONE = 3;
    private static final int EVENT_AUTO_LOGGING_START = 4;
    private static final int EVENT_AUTO_LOGGING_START_DONE = 5;
    private static final int EVENT_AUTO_LOGGING_STOP = 6;
    private static final int EVENT_AUTO_LOGGING_STOP_DONE = 7;
    private static final int EVENT_SILENT_LOGGING_STARTED = 8;
    private static final int EVENT_GET_AUTOLOG_STATUS = 9;
    private static final int EVENT_GET_AUTOLOG_STATUS_DONE = 10;
    private static final int EVENT_AUTO_LOGGING_START_WITH_PROFILE = 11;
    private static final int EVENT_AUTO_LOGGING_START_WITH_PROFILE_DONE = 12;
    private static final int EVENT_NOTIFY_AUTOLOG_STATUS = 50;

    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;

    private static final int EVENT_STOP_SILENT_RESP = 200;
    private static final int EVENT_SAVE_AUTOLOG_RESP = 201;

    private static final int RILC_REQ_SET_SELFLOG = 600;
    private static final int RILC_REQ_GET_SELFLOG_STATUS = 601;
    private static final int RILC_REQ_SET_SELFLOG_PROFILE = 611;

    /* DMControl command type */
    public static final int SILENT_LOGGING_START = 0;
    public static final int SILENT_LOGGING_STOP = 1;
    public static final int AUTO_LOGGING_START = 2;
    public static final int AUTO_LOGGING_STOP = 3;
    public static final int AUTO_LOGGING_SAVE = 4;

    private static final int SIM1 = 0;
    private int mStartCheckCount;
    private String mDestProfile = "/data/vendor/rild/";
    private String mProfileName = "DM Trace Item Selection.msg";

    OemRil mOemRil;
    private SilentLoggingControlInterface mSilentLoggingControlInterface;
    private IDMControlServiceCallback mCb;
    private Boolean mIsSilentMode = false;
    private int mAutoLogState = -1;
    private Boolean mAutoLogRequested = false;
    private Boolean mOnAutoLogging = false;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            int error = 0;
            switch (msg.what) {
            case EVENT_RIL_CONNECTED:
                Log.d(TAG, "EVENT_RIL_CONNECTED");
                mHandler.sendEmptyMessage(EVENT_GET_AUTOLOG_STATUS);
                break;
            case EVENT_GET_AUTOLOG_STATUS:
                Log.d(TAG, "EVENT_GET_AUTOLOG_STATUS");
                mOemRil.invokeRequestRaw(RILC_REQ_GET_SELFLOG_STATUS, null, mHandler.obtainMessage(EVENT_GET_AUTOLOG_STATUS_DONE));
                break;
            case EVENT_GET_AUTOLOG_STATUS_DONE:
                ar = (AsyncResult) msg.obj;
                Log.d(TAG, "EVENT_GET_AUTOLOG_STATUS_DONE");
                /*
                0x00 = DM is not running. Self-logging can be started
                0x01 = DM is running. Self-logging cannot be started
                0x02 = Self-logging already started
                0xFF = Self-logging feature is not supported
                */
                if (ar.exception == null) {
                    byte[] data = (byte [])ar.result;
                    if (data != null && data.length > 0)
                        mAutoLogState = data[0] & 0xFF;
                }

                if (mAutoLogRequested) {
                    mHandler.sendEmptyMessage(EVENT_AUTO_LOGGING_START);
                }
                break;
            case EVENT_SILENT_LOGGING_START:
                Log.d(TAG, "EVENT_SILENT_LOGGING_START");
                if (isSilentMode()) {
                    Log.d(TAG, "already start silent logging!");
                }

                if (mOnAutoLogging) {
                    Log.d(TAG, "Cannot start silent logging.");
                    Log.d(TAG, "CP is AutoLogging");
                    Message m = mHandler.obtainMessage(EVENT_SILENT_LOGGING_START_DONE);
                    m.arg1 = 1;
                    mHandler.sendMessage(m);
                    break;
                }
                startSilent();
                break;
            case EVENT_SILENT_LOGGING_STARTED:
                Log.d(TAG, "EVENT_SILENT_LOGGING_STARTED");
                if (SilentLoggingControlInterface.getMode() == SilentLoggingConstant.DM_MODE.SILENT) {
                    Message m = mHandler.obtainMessage(EVENT_SILENT_LOGGING_START_DONE);
                    m.arg1 = 0;
                    mHandler.sendMessage(m);
                }
                else {
                    if (mStartCheckCount-- > 0) {
                        mHandler.sendEmptyMessageDelayed(EVENT_SILENT_LOGGING_STARTED, 1000);
                    }
                    else {
                        // TODO fail
                        Message m = mHandler.obtainMessage(EVENT_SILENT_LOGGING_START_DONE);
                        m.arg1 = 1;
                        mHandler.sendMessage(m);
                    }
                }
                break;
            case EVENT_SILENT_LOGGING_START_DONE:
                if (msg.arg1 == 0) {
                    Log.d(TAG, "EVENT_SILENT_LOGGING_START_DONE, Success");
                } else if (msg.arg1 == 1) {
                    Log.d(TAG, "EVENT_SILENT_LOGGING_START_DONE, Fail");
                }
                sendResponse(SILENT_LOGGING_START, msg.arg1);
                break;
            case EVENT_SILENT_LOGGING_STOP:
                Log.d(TAG, "EVENT_SILENT_LOGGING_STOP");
                stopSilent();
                break;
            case EVENT_SILENT_LOGGING_STOP_DONE:
                Log.d(TAG, "EVENT_SILENT_LOGGING_STOP_DONE");
                sendResponse(SILENT_LOGGING_STOP, 0);
                if (mAutoLogRequested) {
                    mHandler.sendEmptyMessage(EVENT_GET_AUTOLOG_STATUS);
                }
                break;
            case EVENT_AUTO_LOGGING_START:
                Log.d(TAG, "EVENT_AUTO_LOGGING_START");
                Log.d(TAG, "mAutoLogState=" + mAutoLogState);
                if (!mAutoLogRequested) {
                    mAutoLogRequested = true;
                    mHandler.sendEmptyMessage(EVENT_GET_AUTOLOG_STATUS);
                } else {
                    mAutoLogRequested = false;
                    if (mAutoLogState == 0x00) {
                        int size = SystemProperties.getInt(SilentLoggingConstant.PROPERTY_AUTOLOG_SIZE, 32);
                        Log.d(TAG, "AutoLogSize=" + size);
                        DataWriter out = new DataWriter();
                        try {
                            out.writeInt(0);
                            out.writeInt(size);
                            mOemRil.invokeRequestRaw(RILC_REQ_SET_SELFLOG, out.toByteArray(), mHandler.obtainMessage(EVENT_AUTO_LOGGING_START_DONE));
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    } else if (mAutoLogState == 0x01) {
                        if (isSilentMode()) {
                            mAutoLogRequested = true;
                            mHandler.sendEmptyMessage(EVENT_SILENT_LOGGING_STOP);
                        } else {
                            Log.d(TAG, "AutoLogging Start Fail");
                            sendResponse(AUTO_LOGGING_START, mAutoLogState);
                        }
                    } else {
                        Log.d(TAG, "AutoLogging Start Fail");
                        sendResponse(AUTO_LOGGING_START, mAutoLogState);
                    }
                }
                break;
            case EVENT_AUTO_LOGGING_START_DONE:
                Log.d(TAG, "EVENT_AUTO_LOGGING_START_DONE");
                ar = (AsyncResult) msg.obj;

                if (ar.exception == null) {
                    byte[] data = (byte [])ar.result;
                    if (data != null && data.length > 0)
                        error = data[0] & 0xFF;
                }
                if (error == 0) {
                    mOnAutoLogging = true;
                }
                sendResponse(AUTO_LOGGING_START, error);
                break;
            case EVENT_AUTO_LOGGING_STOP:
                Log.d(TAG, "EVENT_AUTO_LOGGING_STOP");
                if (!mOnAutoLogging) {
                    Log.d(TAG, "AutoLogging is not running.");
                    sendResponse(AUTO_LOGGING_STOP, 0);
                    break;
                }
                DataWriter out = new DataWriter();
                try {
                    out.writeInt(1);
                    out.writeInt(0);
                    mOemRil.invokeRequestRaw(RILC_REQ_SET_SELFLOG, out.toByteArray(), mHandler.obtainMessage(EVENT_AUTO_LOGGING_STOP_DONE));
                } catch (IOException e) {
                    e.printStackTrace();
                }
                break;
            case EVENT_AUTO_LOGGING_STOP_DONE:
                ar = (AsyncResult) msg.obj;
                Log.d(TAG, "EVENT_AUTO_LOGGING_STOP_DONE");
                if (ar.exception == null) {
                    byte[] data = (byte [])ar.result;
                    if (data != null && data.length > 0)
                        error = data[0] & 0xFF;
                }
                sendResponse(AUTO_LOGGING_STOP, error);

                if (error == 0) {
                    mOnAutoLogging = false;
                    //To do. Save Auto Log
                    savaAutoLog();
                }
                break;
            case EVENT_NOTIFY_AUTOLOG_STATUS:
                Log.d(TAG, "EVENT_NOTIFY_AUTOLOG_STATUS");
                //ar = (AsyncResult) msg.obj;
                //just update AutoLogState
                mHandler.sendEmptyMessage(EVENT_GET_AUTOLOG_STATUS);
                /*
                if (ar.exception == null) {
                    byte[] data = arrayListToPrimitiveArray((ArrayList<Byte>)ar.result);
                    if (data != null && data.length > 1)
                        mAutoLogState = data[0];
                }
                */
                break;
            case EVENT_STOP_SILENT_RESP:
                Log.d(TAG, "EVENT_STOP_SILENT_RESP");
                Message m = mHandler.obtainMessage(EVENT_SILENT_LOGGING_STOP_DONE);
                ar = (AsyncResult) msg.obj;
                m.arg1 = (int)ar.result;
                mHandler.sendMessage(m);
                break;
            case EVENT_SAVE_AUTOLOG_RESP:
                Log.d(TAG, "EVENT_SAVE_AUTOLOG_RESP");
                ar = (AsyncResult) msg.obj;
                int result = (int)ar.result;
                if (result >= 0) {
                    Log.d(TAG, "Success Saving AutoLog");
                    error = 0;
                } else {
                    Log.d(TAG, "Fail Saving AutoLog");
                    error = 1;
                }
                sendResponse(AUTO_LOGGING_SAVE, error);
                break;
            case EVENT_AUTO_LOGGING_START_WITH_PROFILE:
                Log.d(TAG, "EVENT_AUTO_LOGGING_START_WITH_PROFILE");
                String profile = msg.getData().getString("path");
                unzip(profile);
                mOemRil.invokeRequestRaw(RILC_REQ_SET_SELFLOG_PROFILE, null, mHandler.obtainMessage(EVENT_AUTO_LOGGING_START_WITH_PROFILE_DONE));
                break;
            case EVENT_AUTO_LOGGING_START_WITH_PROFILE_DONE:
                Log.d(TAG, "EVENT_AUTO_LOGGING_START_WITH_PROFILE_DONE");
                mHandler.sendEmptyMessage(EVENT_AUTO_LOGGING_START);
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
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();

        mOemRil.detach();
        if (mOemRil == null) {
            Log.d(TAG, "mOemRil is null");
        } else {
            mOemRil.unregisterForOemRilConnected(mHandler);
            mOemRil.unregisterForOemRilDisconnected(mHandler);
            mOemRil.unregisterSelflogStatus(mHandler);
        }
    }

    private void connectToOemRilService() {
        mOemRil = OemRil.init(getApplicationContext(), SIM1);
        if (mOemRil == null) {
            Log.d(TAG, "connectToOemRilService mOemRil is null");
        } else {
            mOemRil.registerForOemRilConnected(mHandler, EVENT_RIL_CONNECTED);
            mOemRil.registerForOemRilDisconnected(mHandler, EVENT_RIL_DISCONNECTED);
            mOemRil.registerSelflogStatus(mHandler, EVENT_NOTIFY_AUTOLOG_STATUS);
        }
    }

    private void connectToControlInterface() {
        mSilentLoggingControlInterface = SilentLoggingControlInterface.getInstance();
        if (mSilentLoggingControlInterface == null) {
            Log.e(TAG, "connectToControlInterface : mSilentLoggingControlInterface is null");
        }
        mSilentLoggingControlInterface.registerStopSilentResp(mHandler, EVENT_STOP_SILENT_RESP);
        mSilentLoggingControlInterface.registerSaveAutoLogResp(mHandler, EVENT_SAVE_AUTOLOG_RESP);
    }

    private void sendResponse(int commandId, int error) {
        Log.d(TAG, "sendResponse()");
        try {
            mCb.onResults(commandId, error);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()");
        connectToOemRilService();
        connectToControlInterface();
        if (mDMControlServiceBinder == null) {
            mDMControlServiceBinder = new DMControlServiceBinder();
        }
        mIsSilentMode = isSilentMode();
        return mDMControlServiceBinder;
    }

    public static byte[] arrayListToPrimitiveArray(ArrayList<Byte> bytes) {
        byte[] ret = new byte[bytes.size()];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = bytes.get(i);
        }
        return ret;
    }

    private Boolean isSilentMode() {
        Boolean isSilentMode = false;
        SilentLoggingConstant.DM_MODE mode = SilentLoggingConstant.DM_MODE.SHANNON;

        String sLogProp = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_MODE));
        String cpLogProp = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_CPLOG_MODE));
        if (sLogProp.equalsIgnoreCase("On") && cpLogProp.equalsIgnoreCase("On"))
            isSilentMode = true;
        else
            isSilentMode = false;

        return isSilentMode;
    }

    private void startSilent() {
        if (isSilentMode()) {
            Log.d(TAG, "startSilent(), Silentlogging is already in process.");
            return;
        }

        SilentLoggingConstant.mStartCommand = SilentLoggingConstant.DM_START;
        SilentLoggingControlInterface.getInstance().setContext(this);
        SilentLoggingControlInterface.getInstance().initialize();

        SystemProperties.set(SilentLoggingConstant.PROPERTY_SLOG_MODE, "On");
        SystemProperties.set(SilentLoggingConstant.PROPERTY_CPLOG_MODE, "On");
        SystemProperties.set(SilentLoggingConstant.PROPERTY_APLOG_MODE, "Off");
        SystemProperties.set(SilentLoggingConstant.PROPERTY_TCPLOG_MODE, "Off");

        new Thread() {
            @Override
            public void run() {
                SilentLoggingControlInterface.getInstance().startSilentLogging();
                mHandler.sendEmptyMessageDelayed(EVENT_SILENT_LOGGING_STARTED, 1000);
                mStartCheckCount = 10;
            }
        }.start();
    }

    private void stopSilent() {
        if (!isSilentMode()) {
            Log.d(TAG, "stopSilent(), Silentlogging is not in process.");
            mHandler.sendEmptyMessage(EVENT_SILENT_LOGGING_STOP_DONE);
            return;
        }
        SilentLoggingControlInterface.getInstance().setContext(this);
        SilentLoggingControlInterface.getInstance().initialize();
        SystemProperties.set(SilentLoggingConstant.PROPERTY_SLOG_MODE, "Off");
        mIsSilentMode = true;
        new Thread() {
            @Override
            public void run() {
                SilentLoggingControlInterface.getInstance().stopSilentLogging();
            }
        }.start();
    }

    private void savaAutoLog() {
        SilentLoggingControlInterface.getInstance().setContext(this);
        SilentLoggingControlInterface.getInstance().initialize();
        SilentLoggingControlInterface.getInstance().saveAutoLog();
    }

    private void unzip(String unzipFile) {
        try {
            FileInputStream fin = new FileInputStream(unzipFile);
            ZipInputStream zin = new ZipInputStream(new BufferedInputStream(fin));
            ZipEntry ze = null;
            while ((ze = zin.getNextEntry()) != null) {
                if (mProfileName.equals(ze.getName())) {
                    Log.d(TAG, "Unzipping " + ze.getName());
                    byte b[] = new byte[1024];
                    int n;
                    FileOutputStream fout = new FileOutputStream(mDestProfile + "profile.hex");
                    BufferedOutputStream out = new BufferedOutputStream(fout, b.length);

                    while ((n = zin.read(b, 0, b.length)) >= 0) {
                        out.write(b, 0, n);
                    }
                    out.flush();
                    out.close();
                    zin.closeEntry();
                    fout.close();

                    File path = new File(mDestProfile + "profile.hex");
                    path.setReadable(true, false);
                } else {
                    Log.d(TAG, "Skip Unzipping " + ze.getName());
                }
            }
            zin.close();
        } catch(Exception e) {
            Log.e(TAG, "unzip", e);
        }
    }

    /* DMControlServiceBinder */
    private DMControlServiceBinder mDMControlServiceBinder;
    class DMControlServiceBinder extends IDMControlService.Stub {

        @Override
        public void registerCallback(IDMControlServiceCallback cb) throws RemoteException {
            Log.d(TAG, "registerCallback()");
            if (cb != null) {
                mCb = cb;
            }
        }

        @Override
        public void startSilentLogging() throws RemoteException {
            Log.d(TAG, "startSilentLogging()");
            // send to handler
            mHandler.sendEmptyMessage(EVENT_SILENT_LOGGING_START);
        }

        @Override
        public void stopSilentLogging() throws RemoteException {
            Log.d(TAG, "stopSilentLogging()");
            // send to handler
            mHandler.sendEmptyMessage(EVENT_SILENT_LOGGING_STOP);
        }

        @Override
        public void startAutoLogging(int size) throws RemoteException {
            Log.d(TAG, "startAutoLogging()");
            // 5 <= size <= 32
            if (size < 5) {
                size = 5;
            } else if (size > 32) {
                size = 32;
            }
            SystemProperties.set(SilentLoggingConstant.PROPERTY_AUTOLOG_SIZE, String.valueOf(size));
            // send to handler
            mHandler.sendEmptyMessage(EVENT_AUTO_LOGGING_START);
        }

        @Override
        public void startAutoLoggingWithProfile(int size, String profilePath) throws RemoteException {
            Log.d(TAG, "startAutoLogging() with profile=" + profilePath);
            // 5 <= size <= 32
            if (size < 5) {
                size = 5;
            } else if (size > 32) {
                size = 32;
            }
            SystemProperties.set(SilentLoggingConstant.PROPERTY_AUTOLOG_SIZE, String.valueOf(size));

            if (!TextUtils.isEmpty(profilePath)) {
                Bundle data = new Bundle();
                data.putString("path", profilePath);
                Message msg = mHandler.obtainMessage(EVENT_AUTO_LOGGING_START_WITH_PROFILE);
                msg.setData(data);
                mHandler.sendMessage(msg);
            } else {
                Log.w(TAG, "start using default profile");
                mHandler.sendEmptyMessage(EVENT_AUTO_LOGGING_START);
            }
        }

        @Override
        public void stopAutoLogging() throws RemoteException {
            Log.d(TAG, "stopAuto");
            // send to handler
            mHandler.sendEmptyMessage(EVENT_AUTO_LOGGING_STOP);
        }
    }

}
