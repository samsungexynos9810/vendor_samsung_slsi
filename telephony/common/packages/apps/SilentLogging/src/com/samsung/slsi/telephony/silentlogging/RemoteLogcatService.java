/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.silentlogging;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.attribute.PosixFilePermission;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

public class RemoteLogcatService extends Service {

    // intent action
    public static final String TAG = "RemoteLogcatService";
    public static final String ACTION_START_LOGCAT_SERVICE = "com.samsung.slsi.telephony.silentlogging.START_LOGCAT_SERVICE";
    public static final String ACTION_STOP_LOGCAT_SERVICE = "com.samsung.slsi.telephony.silentlogging.STOP_LOGCAT_SERVICE";
    public static final String ACTION_RENEW_LOGCAT_SERVICE = "com.samsung.slsi.telephony.silentlogging.RENEW_LOGCAT_SERVICE";
    public static final String ACTION_DUMP_LOGCAT_SERVICE = "com.samsung.slsi.telephony.silentlogging.DUMP_LOGCAT_SERVICE";

    // running state
    public static final int UNKOWN = -1;
    public static final int STOP = 0;
    public static final int START = 1;
    public static final int RESUME = 2;

    // handler event
    public static final int EVENT_START_LOGCAT_SERVICE = 1;
    public static final int EVENT_STOP_LOGCAT_SERVICE = 2;
    public static final int EVENT_RENEW_LOGCAT_SERVICE = 3;
    public static final int EVENT_FINISH_LOGCAT_SERVICE = 4;
    public static final int EVENT_LOGCAT_DUMP_COMPLETE = 5;

    public static final long ELAPSED_WAKEUP = (1000 * 60 * 60 * 3);    // wake-up in next 3 hour

    public static final String DEFAULT_DUMP_PATH = "/data/vendor/slog";

    static interface OnZipArchiveListener {
        public void onZipArchiveStarted();

        public void onZipArchiveComplete();
    }

    class LogcatContext implements OnZipArchiveListener {
        public static final String KEY_MAIN = "main";
        public static final String KEY_RADIO = "radio";
        public static final String KEY_KERNEL = "kernel";
        public static final String KEY_SYSTEM = "system";
        public static final String KEY_EVENTS = "events";
        public static final String KEY_CRASH = "crash";
        public static final String KEY_DEFAULT = "default";
        public static final String KEY_ALL = "all";

        protected int mStartId;
        protected String mDirectoryPath;
        protected HashMap<String, Process> mProcessMap;
        protected String mTimestamp;
        protected final String[] mSupportedTypes;
        protected boolean mFinishing;

        public LogcatContext(int startId, String directoryPath) {
            mStartId = startId;
            mProcessMap = new HashMap<String, Process>();
            mDirectoryPath = directoryPath;
            if (TextUtils.isEmpty(mDirectoryPath)) {
                mDirectoryPath = DEFAULT_DUMP_PATH;
            }

            SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
            mTimestamp = dateFormat.format(new Date()).toString();
            mFinishing = false;

            /*
             * -b <buffer>, --buffer=<buffer> Request alternate ring buffer,
             * 'main', 'system', 'radio', 'events', 'crash', 'default' or 'all'.
             * Multiple -b parameters or comma separated list of buffers are
             * allowed. Buffers interleaved. Default -b main,system,crash.
             */
            // can be extended
            mSupportedTypes = new String[] { KEY_ALL, KEY_KERNEL };
        }

        public String getDirectory() {
            return mDirectoryPath;
        }

        public String getTimestamp() {
            return mTimestamp;
        }

        public List<String> makeLogFileList() {
            ArrayList<String> fileList = new ArrayList<String>();
            Set<String> keys = mProcessMap.keySet();
            for (String key : keys) {
                Process p = mProcessMap.get(key);
                if (p != null) {
                    String filename = "logcat_" + key + "_" + mTimestamp + ".log";
                    fileList.add(filename);
                }
            } // end for ~
            return fileList;
        }

        public boolean isRunning() {
            boolean running = false;
            Set<String> keys = mProcessMap.keySet();
            for (String key : keys) {
                Process p = mProcessMap.get(key);
                if (p != null && p.isAlive()) {
                    running = true;
                    Log.d(TAG, "" + p + " is running.");
                }
            } // end for ~

            if (!running)
                Log.d(TAG, "no running process");

            return running;
        }

        public void start() {
            for (String type : mSupportedTypes) {
                makeProcess(type);
            } // end for ~

            // send next wakeup alarm
            if (mAlarmManager != null) {
                long elapsed = ELAPSED_WAKEUP;
                mAlarmManager.set(AlarmManager.ELAPSED_REALTIME_WAKEUP,
                        SystemClock.elapsedRealtime() + elapsed, mAlarmIntent);
                Log.d(TAG, "Alarm reserved after " + elapsed);
            }
        }

        public void destroyFocibly(boolean makeArchive, boolean finishing) {
            Set<String> keys = mProcessMap.keySet();
            for (String key : keys) {
                Process p = mProcessMap.get(key);
                if (p != null && p.isAlive()) {
                    Log.d(TAG, "kill " + p);
                    p.destroyForcibly();
                }
            } // end for ~

            mFinishing = finishing;
            if (makeArchive) {
                ZipArchiveThread thread = new ZipArchiveThread(mDirectoryPath, "logcat_ap_" + mTimestamp, makeLogFileList(), this);
                thread.start();
            }
            else if (finishing) {
                sendMessage(EVENT_FINISH_LOGCAT_SERVICE);
            }

            mProcessMap.clear();
        }

        private void makeProcess(String key) {
            String filepath = mDirectoryPath + "/logcat_" + key + "_" + mTimestamp + ".log";
            String command = "logcat -b " + key + " -f " + filepath;
            Log.d(TAG, "key=" + key + " command=" + command);
            try {
                Process process = Runtime.getRuntime().exec(command);
                if (process != null) {
                    Log.d(TAG, "execute " + process);
                    mProcessMap.put(key, process);
                }
                else {
                    Log.d(TAG, "failed to execute sub process. key=" + key);
                }
            } catch (IOException e) {
                Log.d(TAG, "", e);
            } catch (Exception e) {
                Log.d(TAG, "", e);
            }
        }

        public void putEnvironment(SharedPreferences.Editor prefs) {
            if (prefs != null) {
                prefs.putInt("startId", mStartId);
                prefs.putString("timestamp", mTimestamp);
                prefs.putStringSet("keyset", mProcessMap.keySet());
                prefs.putString("directoryPath", mDirectoryPath);
            }
        }

        @Override
        public void onZipArchiveStarted() {
            Log.d(TAG, "LogcatContext.onZipArchiveStarted");
        }

        @Override
        public void onZipArchiveComplete() {
            Log.d(TAG, "LogcatContext.onZipArchiveComplete");
            // delete a source file
            for (String type : mSupportedTypes) {
                String filepath = mDirectoryPath + "/logcat_" + type + "_" + mTimestamp + ".log";
                try {
                    File f = new File(filepath);
                    if (f.exists()) {
                        Log.d(TAG, "delete file " + filepath);
                        f.delete();
                    }
                    else {
                        Log.d(TAG, filepath + " not existed.");
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Failed to delete.", e);
                }
            } // end for ~

            if (mFinishing) {
                sendMessage(EVENT_FINISH_LOGCAT_SERVICE);
            }
        }
    }

    class LogcatDumpContext extends LogcatContext implements Runnable, OnZipArchiveListener {

        String mFilename;
        public LogcatDumpContext(int startId, String directoryPath) {
            super(startId, directoryPath);
            mFilename = "logcat_dump_" + mTimestamp + ".log";
        }

        @Override
        public void start() {
            new Thread(this).start();
        }

        @Override
        public void run() {
            Log.d(TAG, "run LogcatDumpContext thread");
            String filepath = mDirectoryPath + "/" + mFilename;
            String command = "logcat -b all -d -f " + filepath;
            Log.d(TAG, "command=" + command);
            try {
                Process process = Runtime.getRuntime().exec(command);
                if (process != null) {
                    Log.d(TAG, "execute " + process);
                    process.waitFor();
                }
                else {
                    Log.d(TAG, "failed to execute sub process.");
                }
            } catch (IOException e) {
                Log.d(TAG, "", e);
            } catch (Exception e) {
                Log.d(TAG, "", e);
            }

            ArrayList<String> fileList = new ArrayList<String>();
            fileList.add(mFilename);
            new ZipArchiveThread(getDirectory(), "logcat_dump_" + mTimestamp, fileList, this).start();
        }

        @Override
        public void onZipArchiveStarted() {
            Log.d(TAG, "LogcatDumpContext.onZipArchiveStarted");
        }

        @Override
        public void onZipArchiveComplete() {
            Log.d(TAG, "LogcatDumpContext.onZipArchiveComplete");
            // delete a source file
            String filepath = mDirectoryPath + "/" + mFilename;
            try {
                File f = new File(filepath);
                if (f.exists()) {
                    Log.d(TAG, "delete file " + filepath);
                    f.delete();
                }
                else {
                    Log.d(TAG, filepath + " not existed.");
                }
            } catch (Exception e) {
                Log.e(TAG, "Failed to delete.", e);
            }

            String archiveName = mDirectoryPath+ "/logcat_dump_" + mTimestamp + ".log";
            String text = archiveName + " has been created.";
            sendMessage(EVENT_LOGCAT_DUMP_COMPLETE, text);
        }
    }

    static class ZipArchiveThread extends Thread {
        public static final String TAG = "ZipArchiveThread";
        private String directoryPath;
        private String archiveName;
        private List<String> fileList;
        private OnZipArchiveListener listener;

        ZipArchiveThread(String directoryPath, String archiveName, List<String> fileList, OnZipArchiveListener listener) {
            this.directoryPath = directoryPath;
            this.fileList = new ArrayList<String>(fileList);
            this.archiveName = archiveName;
            this.listener = listener;
        }

        @Override
        public void run() {
            Log.d(TAG, "ZipArchiveThread start");

            if (fileList == null || fileList.isEmpty()) {
                Log.d(TAG, "No log files.");
                return;
            }

            if (listener != null) {
                listener.onZipArchiveStarted();
            }

            String targetPath = directoryPath + "/" + archiveName + ".zip";
            ZipOutputStream zos = null;
            try {
                zos = new ZipOutputStream(new FileOutputStream(new File(targetPath)));
                Log.d(TAG, "target name:" + targetPath);
                for (String file : fileList) {
                    ZipEntry entry = new ZipEntry(file);
                    zos.putNextEntry(entry);
                    Log.d(TAG, "add ZipEntery: " + file);

                    String sourcePath = directoryPath + "/" + file;
                    FileInputStream fis = null;
                    try {
                        fis = new FileInputStream(sourcePath);
                        Log.d(TAG, "Open FileInputStream: " + sourcePath);
                        byte[] buffer = new byte[4096];
                        int ret = 0;
                        while ((ret = fis.read(buffer)) > 0) {
                            zos.write(buffer, 0, ret);
                        } // end while ~
                    } catch (IOException e) {
                        Log.e(TAG, "", e);
                    } finally {
                        try {
                            if (fis != null)
                                fis.close();
                        } catch (IOException e) {
                        }
                    }
                    zos.closeEntry();
                } // end for ~
                zos.close();

                File targetFile = new File(targetPath);
                Set<PosixFilePermission> perms = new HashSet<PosixFilePermission>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                perms.add(PosixFilePermission.GROUP_READ);
                perms.add(PosixFilePermission.GROUP_WRITE);
                perms.add(PosixFilePermission.OTHERS_READ);
                Files.setPosixFilePermissions(targetFile.toPath(), perms);

            } catch (FileNotFoundException e) {
                Log.e(TAG, "", e);
            } catch (IOException e) {
                Log.e(TAG, "", e);
            } finally {
                try {
                    if (zos != null)
                        zos.close();
                } catch (IOException e) {
                }
            }
            Log.d(TAG, "ZipArchiveThread finished");

            if (listener != null) {
                listener.onZipArchiveComplete();
            }
        }
    }

    private String mDirectoryPath = DEFAULT_DUMP_PATH;

    private PendingIntent mAlarmIntent;
    private AlarmManager mAlarmManager;
    private Toast mToast;
    private LogcatContext mLogcatContext;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_START_LOGCAT_SERVICE:
                onStartLogcatServices((LogcatContext)msg.obj);
                break;
            case EVENT_STOP_LOGCAT_SERVICE:
                onStopLogcatService((LogcatContext)msg.obj, (msg.arg1 == 1));
                break;
            case EVENT_FINISH_LOGCAT_SERVICE:
                onFinishLogcatService();
                break;
            case EVENT_LOGCAT_DUMP_COMPLETE:
                onLogcatDumpComplete((String)msg.obj);
                break;
            } // end switch ~
        }
    };

    class MyReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "onReceive intent=" + intent);

            String action = intent.getAction();
            if (RemoteLogcatService.ACTION_RENEW_LOGCAT_SERVICE.equals(action)) {
                Intent i = new Intent(context, RemoteLogcatService.class);
                i.setAction(RemoteLogcatService.ACTION_RENEW_LOGCAT_SERVICE);
                context.startService(i);
            }
        }
    }

    private MyReceiver mReceiver;

    private void showMessage(String message) {
        if (mToast == null) {
            mToast = Toast.makeText(this, message, Toast.LENGTH_SHORT);
        }
        else {
            mToast.setText(message);
        }
        mToast.show();
    }

    void sendMessage(int what) {
        Message msg = mHandler.obtainMessage(what);
        msg.sendToTarget();
    }

    void sendMessage(int what, Object obj) {
        Message msg = mHandler.obtainMessage(what, obj);
        msg.sendToTarget();
    }

    void sendMessage(int what, int arg1, int arg2, Object obj) {
        Message msg = mHandler.obtainMessage(what, obj);
        msg.arg1 = arg1;
        msg.arg2 = arg2;
        msg.sendToTarget();
    }

    void sendMessage(int what, long delayMillis) {
        Message msg = mHandler.obtainMessage(what);
        if (delayMillis < 0) {
            delayMillis = 0;
        }
        mHandler.sendMessageDelayed(msg, delayMillis);
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        super.onCreate();

        mAlarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        Intent intent = new Intent(ACTION_RENEW_LOGCAT_SERVICE);
        mAlarmIntent = PendingIntent.getBroadcast(this, 1, intent, 0);

        mReceiver = new MyReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_RENEW_LOGCAT_SERVICE);

        registerReceiver(mReceiver, filter);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "Service=" + this);
        Log.d(TAG,"onStartCommand Service=" + this + " Intent=" + intent + " flags=" + flags + " startId=" + startId);
        if (intent != null) {
            String action = intent.getAction();
            if (ACTION_START_LOGCAT_SERVICE.equals(action)) {
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
                if (prefs != null) {
                    String tag = this.toString();
                    String prevTag = prefs.getString("tag", tag);
                    Log.d(TAG, "prevTag=" + prevTag + " curTag=" + tag);
                    if (!TextUtils.isEmpty(prevTag) && !prevTag.equals(tag)) {
                        recoveryIfNeed();
                    }
                }
                String directoryPath = intent.getStringExtra("path");
                startLogcatServices(startId, directoryPath);
            }
            else if (ACTION_STOP_LOGCAT_SERVICE.equals(action)) {
                stopLogcatServices(mLogcatContext, true);
            }
            else if (ACTION_RENEW_LOGCAT_SERVICE.equals(action)) {
                renewLogcatService(startId);
            }
            else if (ACTION_DUMP_LOGCAT_SERVICE.equals(action)) {
                String directoryPath = intent.getStringExtra("path");
                dumpLogcatService(startId, directoryPath);
            }
        }
        else {
            recoveryIfNeed();
            renewLogcatService(startId);
        }
        return START_STICKY;
    }

    private void recoveryIfNeed() {
        Log.d(TAG, "recoveryIfNeed");
        // make Zip archive before crash or power-off
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        String directoryPath = prefs.getString("directoryPath", DEFAULT_DUMP_PATH);
        setDirectoryPath(directoryPath);

        int startId = prefs.getInt("startId", 0);
        final String timestamp = prefs.getString("timestamp", "");
        Log.d(TAG, "directoryPath=" + directoryPath + " stardId=" + startId + " timestamp=" + timestamp);
        Set<String> keyset = prefs.getStringSet("keyset", null);
        final ArrayList<String> fileList = new ArrayList<String>();
        for (String key : keyset) {
            String filename = "logcat_" + key + "_" + timestamp + ".log";
            Log.d(TAG, "recovered filename: " + filename);
            fileList.add(filename);
        } // end for ~

        if (fileList.size() > 0) {
            OnZipArchiveListener listner = new OnZipArchiveListener() {
                @Override
                public void onZipArchiveStarted() {
                }

                @Override
                public void onZipArchiveComplete() {
                    Log.d(TAG, "Recovery: onZipArchiveComplete");
                    for (String file : fileList) {
                        String filepath = getDirectoryPath() + "/" + file;
                        Log.d(TAG, "Recovery: delete file " + filepath);
                        try {
                            File f = new File(filepath);
                            if (f.exists()) {
                                f.delete();
                            }
                        } catch (Exception e) {
                            Log.e(TAG, "Recovery: Failed to delete.", e);
                        }
                    } // end for ~
                }
            };

            new ZipArchiveThread(directoryPath, "logcat_ap_" + timestamp, fileList, listner).start();
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
        unregisterReceiver(mReceiver);

        if (mLogcatContext != null && mLogcatContext.isRunning()) {
            onStopLogcatService(mLogcatContext, false);
            mLogcatContext = null;
        }
        mAlarmIntent = null;
        mAlarmManager = null;
    }

    private void startLogcatServices(int startId, String directoryPath) {
        if (mLogcatContext == null) {
            mLogcatContext = new LogcatContext(startId, directoryPath);
        }

        sendMessage(EVENT_START_LOGCAT_SERVICE, mLogcatContext);
        setDirectoryPath(directoryPath);
    }

    private void stopLogcatServices(LogcatContext logcatContext, boolean finishing) {
        if (logcatContext != null) {
            sendMessage(EVENT_STOP_LOGCAT_SERVICE, finishing ? 1 : 0, 0, logcatContext);
        }
    }

    private void onStopLogcatService(LogcatContext logcatContext, boolean finishing) {
        Log.d(TAG, "onStopLogcatService");

        if (finishing) {
            showMessage("On stoping logcat service...");
        }

        if (logcatContext != null) {
            Log.d(TAG, "kill all processes forcibly");
            logcatContext.destroyFocibly(true, finishing);
        }

        if (mAlarmManager != null) {
            Log.d(TAG, "cancel alarm. Intent=" + mAlarmIntent);
            mAlarmManager.cancel(mAlarmIntent);
        }

        saveState(STOP);
    }

    private void onStartLogcatServices(LogcatContext logcatContext) {
        Log.d(TAG, "startLogcatServices");
        if (logcatContext == null) {
            Log.d(TAG, "LogcatContext is null. Stop service itself.");
            return;
        }

        if (logcatContext.isRunning()) {
            Log.d(TAG, "LogcatContext is already running.");
            return;
        }

        logcatContext.start();
        SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(this).edit();
        editor.putString("directoryPath", mDirectoryPath);
        logcatContext.putEnvironment(editor);
        editor.commit();

        saveState(START);
    }

    private void renewLogcatService(int startId) {
        Log.d(TAG, "renewLogcatService");
        // default directory path if mLogcatContext is null.
        String directoryPath = getDirectoryPath();
        if (mLogcatContext != null) {
            directoryPath = mLogcatContext.getDirectory();
            stopLogcatServices(mLogcatContext, false);
        }

        int storedState = getSavedState();
        Log.d(TAG, "storedState=" + storedState);
        if (storedState == START) {
            mLogcatContext = new LogcatContext(startId, directoryPath);
            sendMessage(EVENT_START_LOGCAT_SERVICE, mLogcatContext);
        }
        else {
            Log.d(TAG, "RemoteLogcatService had been already stoped. Stop renewing.");
        }
    }

    private void dumpLogcatService(int startId, String directoryPath) {
        Log.d(TAG, "dumpLogcatService");
        LogcatDumpContext logcatContext = new LogcatDumpContext(startId, directoryPath);
        logcatContext.start();
    }

    protected void onFinishLogcatService() {
        Log.d(TAG, "onFinishLogcatService");
        showMessage("Stoping logcat service...Complete.");
        clearEnvironment();
        stopSelf();
    }

    private void onLogcatDumpComplete(String text) {
        Log.d(TAG, "onLogcatDumpComplete");
        showMessage(text);
        int state = getSavedState();
        if (state != START) {
            stopSelf();
        }
    }

    void setDirectoryPath(String directoryPath) {
        if (TextUtils.isEmpty(directoryPath)) {
            directoryPath = DEFAULT_DUMP_PATH;
        }
        mDirectoryPath = directoryPath;
    }

    String getDirectoryPath() {
        if (TextUtils.isEmpty(mDirectoryPath)) {
            return DEFAULT_DUMP_PATH;
        }
        return mDirectoryPath;
    }

    void clearEnvironment() {
        SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(this).edit();
        editor.clear();
        editor.commit();
    }

    private void saveState(int state) {
        SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(this).edit();
        editor.putInt("state", state);
        editor.putString("tag", (state == START) ? RemoteLogcatService.this.toString() : "");
        editor.commit();
    }

    private int getSavedState() {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        return prefs.getInt("state", STOP);
    }
}
