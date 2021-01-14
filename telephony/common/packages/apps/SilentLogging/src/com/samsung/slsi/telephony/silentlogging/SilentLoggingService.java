package com.samsung.slsi.telephony.silentlogging;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;

public class SilentLoggingService extends Service {

    private static final String TAG = "SilentLoggingService";
    private static final int EVENT_SILENT_LOGGING_STARTED = 0;
    private static final int EVENT_SILENT_LOGGING_STOPED = 1;
    private static final int EVENT_SILENT_LOGGING_REQUEST_FINISHED = 2;
    private int mStartCheckCount;
    private int mStartId;
    private Intent mResultIntent;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_SILENT_LOGGING_STARTED:
                if (SilentLoggingControlInterface.getMode() == SilentLoggingConstant.DM_MODE.SILENT) {
                    mResultIntent.putExtra("SLOG_RESULT", true);
                    finishSilentLoggingRequest(1000);
                }
                else {
                    if (mStartCheckCount-- > 0) {
                        sendEmptyMessageDelayed(EVENT_SILENT_LOGGING_STARTED, 1000);
                    }
                    else {
                        // TODO fail
                        mResultIntent.putExtra("SLOG_RESULT", false);
                        finishSilentLoggingRequest(1000);
                    }
                }
                break;
            case EVENT_SILENT_LOGGING_STOPED:
                if (SilentLoggingControlInterface.getMode() == SilentLoggingConstant.DM_MODE.SHANNON) {
                    mResultIntent.putExtra("SLOG_RESULT", true);
                    finishSilentLoggingRequest(1000);
                }
                else {
                    mResultIntent.putExtra("SLOG_RESULT", false);
                    finishSilentLoggingRequest(1000);
                }
                break;
            case EVENT_SILENT_LOGGING_REQUEST_FINISHED:
                sendBroadcast(mResultIntent);
                stopSelf(mStartId);
                break;
            default:
                break;
            }
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        Log.d(TAG, "onCreate()");
        super.onCreate();

        mResultIntent = new Intent("com.samsung.slsi.telephony.action.SILENT_LOGGING_RESULT");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand()");

        if (intent == null || intent.getAction() == null) {
            Log.d(TAG, "Intent or Action is null!");
            stopSelf(startId);
            return START_NOT_STICKY;
        }
        mStartId = startId;

        if (intent.getAction().equals("com.samsung.slsi.telephony.action.START_SILENT_LOGGING")) {
            Log.d(TAG, "get intent : START_SILENT_LOGGING");
            final boolean dspLog = intent.getBooleanExtra("DSP_LOG", false);
            Log.d(TAG, "Get DSP Mode : " + dspLog);

            final int vendorId = intent.getIntExtra("VENDOR_ID", 0xFF);
            final int profileCommand = intent.getIntExtra("PROFILE_COMMAND", 0xFF);
            if (vendorId != 0xFF || profileCommand != 0xFF) {
                SilentLoggingControlInterface.getInstance().setDmStartCommand(vendorId, profileCommand);
            } else {
                SilentLoggingConstant.mStartCommand = SilentLoggingConstant.DM_START;
            }

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
                    if (dspLog) {
                        if (!SilentLoggingProfile.getInstance().sendRequest(SilentLoggingConstant.ENABLE_DSP_LOG))
                            Log.w(TAG, "Failed to request for DSP Logging");
                    } else {
                        if (!SilentLoggingProfile.getInstance().sendRequest(SilentLoggingConstant.DISABLE_DSP_LOG))
                            Log.w(TAG, "Failed to request for DSP Logging");
                    }

                    mHandler.sendEmptyMessageDelayed(EVENT_SILENT_LOGGING_STARTED, 1000);
                    mStartCheckCount = 10;
                }
            }.start();

        } else if (intent.getAction().equals("com.samsung.slsi.telephony.action.STOP_SILENT_LOGGING")) {
            Log.d(TAG, "get intent : STOP_SILENT_LOGGING");

            SilentLoggingControlInterface.getInstance().setContext(this);
            SilentLoggingControlInterface.getInstance().initialize();

            SystemProperties.set(SilentLoggingConstant.PROPERTY_SLOG_MODE, "Off");

            new Thread() {
                @Override
                public void run() {
                    SilentLoggingControlInterface.getInstance().stopSilentLogging();
                    mHandler.sendEmptyMessageDelayed(EVENT_SILENT_LOGGING_STOPED, 1000);
                }
            }.start();

        } else if (intent.getAction().equals("com.samsung.slsi.telephony.action.MODEM_LOG_DUMP")) {
            Log.d(TAG, "get intent : MODEM_LOG_DUMP");
            ComponentName comp = new ComponentName("com.samsung.slsi.telephony.oemril", "com.samsung.slsi.telephony.oemril.OemRilService");
            Intent mIntent = new Intent("com.samsung.slsi.telephony.action.MODEM_LOG_DUMP");
            mIntent.setComponent(comp);
            startService(mIntent);
        }
        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();
    }

    private void finishSilentLoggingRequest(long delayed) {
        mHandler.sendEmptyMessageDelayed(EVENT_SILENT_LOGGING_REQUEST_FINISHED, delayed);
    }

}
