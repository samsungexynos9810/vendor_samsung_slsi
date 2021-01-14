package com.samsung.slsi.cnntlogger;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.SystemProperties;
import android.util.Log;

import java.io.File;
import android.support.v4.content.LocalBroadcastManager;

public class LoggingService extends Service {

    private final String TAG = "LoggingService";

    private LoggingManager mLoggingManager;

    private ConcurrentTask.ConcurrentTaskListener mConcurrentTask = new ConcurrentTask.ConcurrentTaskListener() {
        @Override
        public void taskCompleted() {
        }
    };

    private BroadcastReceiver mLoggingStartReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            String action = intent.getAction();
            Log.d(TAG, "onReceive : " + action);

            if (action == null) {
                return;
            }

            CNNTUtils utils = new CNNTUtils(context);
            String time = utils.getCurrentTimeString();

            switch(action) {
                case CmdDefine.WIFI_LOGGING_START_INTENT:
                    // logging started status
                    if (!utils.getPreference(CmdDefine.KEY_BTN_STATUS, true)) {
                        new ConcurrentTask(context, mConcurrentTask).execute(
                                CmdDefine.StopScript, "", "", "");
                    }

                    boolean isCheckedUdiLog = intent.getBooleanExtra("wifi_udilog", true);
                    boolean isCheckedMxLog = intent.getBooleanExtra("wifi_mxlog", true);

                    String logType = "";
                    if (isCheckedUdiLog) {
                        logType += "udilog ";
                    }

                    if (isCheckedMxLog) {
                        logType += "mxlog ";
                    }
                    new ConcurrentTask(context, mConcurrentTask).execute(
                            CmdDefine.StartScript, time, logType, "");

                    utils.setPreference(CmdDefine.KEY_BTN_STATUS, false);
                    utils.setPreference(CmdDefine.KEY_CHECK_LOGCAT, false);
                    utils.setPreference(CmdDefine.KEY_CHECK_MXLOG, isCheckedMxLog);
                    utils.setPreference(CmdDefine.KEY_CHECK_UDILOG, isCheckedUdiLog);
                    utils.setPreference(CmdDefine.KEY_IS_WIFI_LOG, true);
                    mLoggingManager.updateLoggingNotification(true, logType);
                    sendLocalBroadcast(context, CmdDefine.UPDATE_LOGGING_OPTION);
                    break;
                case CmdDefine.WIFI_LOGGING_STOP_INTENT:
                    new ConcurrentTask(context, mConcurrentTask).execute(
                            CmdDefine.StopScript, "", "", "");
                    utils.setPreference(CmdDefine.KEY_BTN_STATUS, true);
                    mLoggingManager.updateLoggingNotification(false, "");
                    sendLocalBroadcast(context, CmdDefine.UPDATE_LOGGING_OPTION);
                    break;
                case CmdDefine.BT_LOGGING_START_INTENT:
                    if (!utils.getPreference(CmdDefine.KEY_BTN_STATUS, true)) {
                        new ConcurrentTask(context, mConcurrentTask).execute(
                                CmdDefine.loggingStop, "", "", "");
                    }

                    String filterType = intent.getStringExtra("filterType");

                    new ConcurrentTask(context, mConcurrentTask).execute(
                            CmdDefine.loggingStart, time, filterType, "");

                    utils.setPreference(CmdDefine.KEY_BTN_STATUS, false);
                    utils.setPreference(CmdDefine.KEY_CHECK_LOGCAT, false);
                    utils.setPreference(CmdDefine.KEY_CHECK_MXLOG, true);
                    utils.setPreference(CmdDefine.KEY_CHECK_UDILOG, true);
                    utils.setPreference(CmdDefine.KEY_IS_WIFI_LOG, false);
                    utils.setPreference(CmdDefine.KEY_BT_FILTER, filterType);
                    mLoggingManager.updateLoggingNotification(true, filterType);
                    sendLocalBroadcast(context, CmdDefine.UPDATE_LOGGING_OPTION);
                    break;
                case CmdDefine.BT_LOGGING_STOP_INTENT:
                    new ConcurrentTask(context, mConcurrentTask).execute(
                            CmdDefine.loggingStop, time, "", "");
                    utils.setPreference(CmdDefine.KEY_BTN_STATUS, true);
                    mLoggingManager.updateLoggingNotification(false, "");
                    sendLocalBroadcast(context, CmdDefine.UPDATE_LOGGING_OPTION);
                    break;
                case CmdDefine.COPY_LOG_INTENT:
                    String loggingPath = utils.getSystemLoggingPath();
                    String sdcardPath = utils.getSystemSdcardPath();
                    final File loggingDirectory = new File(loggingPath);
                    final File sdcardDirectory = new File(sdcardPath + "/wlbt/");
                    boolean loggingCopyResult = true;
                    if (loggingDirectory.exists()) {
                        loggingCopyResult = utils.copyFile(loggingDirectory, sdcardDirectory);
                    } else {
                        Log.d(TAG, loggingDirectory + " doesn't exist.");
                    }

                    final File sableLogDirectory = new File(CmdDefine.SABLE_LOG_DIR);
                    final File sableSdcardDirectory = new File(sdcardPath + "/wifi/");
                    boolean sableCopyResult = true;
                    if (sableLogDirectory.exists()) {
                        sableCopyResult = utils.copyFile(sableLogDirectory, sableSdcardDirectory);
                    } else {
                        Log.d(TAG, sableLogDirectory + " doesn't exist.");
                    }

                    utils.sendLoggingResultBroadcast(CmdDefine.COPY_LOG_RESULT_INTENT, loggingCopyResult && sableCopyResult);
                    break;
                case CmdDefine.CLEAR_LOG_INTENT:
                    new ConcurrentTask(context, mConcurrentTask).execute(
                            CmdDefine.DeleteLog, "", "", "");
                    /* sendBroadcast clear result Code in CmdRunner.class */
                    break;
                default :
                    break;
            }
        }
    };

    private void sendLocalBroadcast(Context context, String intentName) {
        Intent intent = new Intent(intentName);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mLoggingManager = new LoggingManager(getApplicationContext(), false);

        IntentFilter filter = new IntentFilter();
        filter.addAction(CmdDefine.WIFI_LOGGING_START_INTENT);
        filter.addAction(CmdDefine.WIFI_LOGGING_STOP_INTENT);
        filter.addAction(CmdDefine.BT_LOGGING_START_INTENT);
        filter.addAction(CmdDefine.BT_LOGGING_STOP_INTENT);
        filter.addAction(CmdDefine.COPY_LOG_INTENT);
        filter.addAction(CmdDefine.CLEAR_LOG_INTENT);
        registerReceiver(mLoggingStartReceiver, filter);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mLoggingStartReceiver);
    }
}
