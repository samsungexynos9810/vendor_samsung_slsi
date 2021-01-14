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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.RadioButton;
import android.widget.Toast;

import com.samsung.slsi.telephony.oem.io.DataWriter;
import com.samsung.slsi.telephony.oemservice.OemService;

/**
 * Created by jin-h.shin on 2015-12-07.
 */
public class SilentLoggingControlInterface {

    private static final String TAG = "SilentLoggingControlInterface";
    private ProgressDialog progressDialog;
    private static Context mContext = null;
    private static Toast mToast = null;
    private SilentLogRunnable mSLogRun = null;
    private Thread mSLogThread = null;
    static final int RETRY_MILLIS = 4 * 1000;
    private static final int COPY_TO_STORAGE_DONE = 0;
    private static boolean initFlag = false;
    private static String mSlogPath = SilentLoggingConstant.SLOG_BASE_DIR;

    private OemService mOemService;
    private static final int EVENT_OEM_COMMAND_RESP = 0;
    private static final int EVENT_SAVE_AUTOLOG_RESP = 1;

    protected Registrant mStopSilentRespRegistrant;
    protected Registrant mSaveAutoLogRespRegistrant;

    private static class ControlInterfaceLoader {
        public static SilentLoggingControlInterface sInstance = new SilentLoggingControlInterface();
    }

    public static SilentLoggingControlInterface getInstance() {
        return ControlInterfaceLoader.sInstance;
    }

    public static void setContext(Context context) {
        mContext = context;
    }

    public OemService getOemService() {
        return mOemService;
    }

    public static String getSlogPath() {
        return mSlogPath;
    }

    public void setSlogPath(String path) {
        mSlogPath = path;
    }

    public void startSilentLogging() {
        Log.d(TAG, "startSilentLogging() : Start Silent Logging");
        checkLoggingMode();
    }

    public void startAPTCPLogging() {
        Log.d(TAG, "startAPTCPLogging()");
        if (checkApMode()) {
            Log.d(TAG, "APLogging On!");
            startAPLogging(SilentLoggingConstant.LOG_MODE.SILENT);
        }
        if (checkTcpMode()) {
            if (Build.IS_ENG) {
                Log.d(TAG, "Stop existing TCP Logging");
                stopApTcpLogging(SilentLoggingConstant.COMMAND_TCP_DUMP);
                Log.d(TAG, "TCPLogging On!");
                startTCPLogging(mSlogPath + SilentLoggingConstant.TCPLOG_BASE_FILENAME,
                        SilentLoggingConstant.LOG_MODE.SILENT);
            }
            else {
                Log.w(TAG, "TCP dump is only allowed in ENG binary.");
            }
        }
    }

    public void stopSilentLogging() {
        Log.d(TAG, "stopSilentLogging() : Stop Silent Logging");
        deleteConfigFile();
        if (checkApMode()) {
            //stopApTcpLogging(SilentLoggingConstant.COMMAND_LOGCAT);
            stopAPLogging();
        }
        if (checkTcpMode()) {
            if (Build.IS_ENG) {
                stopApTcpLogging(SilentLoggingConstant.COMMAND_TCP_DUMP);
            }
            else {
                Log.w(TAG, "TCP dump is only allowed in ENG binary.");
            }
        }
        if (checkCpMode()) {
            mOemService.setDmMode(SilentLoggingConstant.MODE_EXTERNAL_DM);
            stopCpLogging();
        }
    }

    public void saveAutoLog() {
        Log.d(TAG, "saveAutoLog()");
        mOemService.saveAutoLog();
    }

    private void checkLoggingMode() {
        Log.d(TAG, "checkLoggingMode() ++");
        if (checkCpMode()) {
            Log.d(TAG, "CPLogging On!");
            startCPLogging();
            checkLoggingOption("CP");
        }
        if (checkApMode()) {
            Log.d(TAG, "APLogging On!");
//            stopApTcpLogging(SilentLoggingConstant.COMMAND_LOGCAT);
//            startAPLogging(mSlogPath + SilentLoggingConstant.APLOG_BASE_FILENAME,
//                    SilentLoggingConstant.LOG_MODE.SILENT);
            startAPLogging(SilentLoggingConstant.LOG_MODE.SILENT);
        }
        if (checkTcpMode()) {
            if (Build.IS_ENG) {
                Log.d(TAG, "Stop existing TCP Logging");
                stopApTcpLogging(SilentLoggingConstant.COMMAND_TCP_DUMP);
                Log.d(TAG, "TCPLogging On!");
                startTCPLogging(mSlogPath + SilentLoggingConstant.TCPLOG_BASE_FILENAME,
                        SilentLoggingConstant.LOG_MODE.SILENT);
            }
            else {
                Log.w(TAG, "TCP dump is only allowed in ENG binary.");
            }
        }
        Log.d(TAG, "checkLoggingMode() --");
    }

    public Boolean checkApMode() {
        String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_APLOG_MODE));
        if (prop.equalsIgnoreCase("On")) {
            return true;
        } else
            return false;
    }

    public Boolean checkCpMode() {
        String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_CPLOG_MODE));
        if (prop.equalsIgnoreCase("On")) {
            return true;
        } else
            return false;
    }

    public Boolean checkTcpMode() {
        String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_TCPLOG_MODE));
        if (prop.equalsIgnoreCase("On")) {
            return true;
        } else
            return false;
    }

    public void initialize() {
        Log.i(TAG, "initial flag : " + initFlag);
        String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_MODE));
        setSlogPath(String.valueOf(
                SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_PATH, SilentLoggingConstant.SLOG_BASE_DIR)));
        if (!initFlag) {
            connectToOemService();
            initFlag = true;
        }
        if (prop.equalsIgnoreCase("On")) {
            startSilentLogging();
        }
    }

    public void autoStartAPTCP() {
        Log.i(TAG, "initial flag : " + initFlag);
        String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_MODE));
        setSlogPath(String.valueOf(
                SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_PATH, SilentLoggingConstant.SLOG_BASE_DIR)));
        if (!initFlag) {
            connectToOemService();
            initFlag = true;
        }
        if (prop.equalsIgnoreCase("On")) {
            startAPTCPLogging();
        }
    }

    private void connectToOemService() {
        mOemService = OemService.init(mContext);
        if (mOemService == null) {
            Log.e(TAG, "connectToOemService : mOemService is null");
        }
        mOemService.registerNotifyCommandResp(mOemCommandRespHandler, EVENT_OEM_COMMAND_RESP);
        mOemService.registerNotifySaveAutoLog(mOemCommandRespHandler, EVENT_SAVE_AUTOLOG_RESP);
    }

    private Handler mOemCommandRespHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            Integer result = new Integer((int)ar.result);
            switch (msg.what) {
            case EVENT_OEM_COMMAND_RESP:
                hideProgressDialog();
                try {
                    mStopSilentRespRegistrant.notifyResult(result);
                } catch (Exception ex) {
                    Log.d(TAG, "mStopSilentRespRegistrant.notifyResult() ex : " + ex);
                }
                break;
            case EVENT_SAVE_AUTOLOG_RESP:
                try {
                    mSaveAutoLogRespRegistrant.notifyResult(result);
                } catch (Exception ex) {
                    Log.d(TAG, "mSaveAutoLogRespRegistrant.notifyResult() ex : " + ex);
                }
                break;
            default:
                break;
            }
        }
    };

    public void registerStopSilentResp(Handler h, int what) {
        mStopSilentRespRegistrant = new Registrant(h, what, null);
    }

    public void unregisterStopSilentResp(Handler h) {
        if (mStopSilentRespRegistrant != null && mStopSilentRespRegistrant.getHandler() == h) {
            mStopSilentRespRegistrant.clear();
            mStopSilentRespRegistrant = null;
        }
    }

    public void registerSaveAutoLogResp(Handler h, int what) {
        mSaveAutoLogRespRegistrant = new Registrant(h, what, null);
    }

    public void unregisterSaveAutoLogResp(Handler h) {
        if (mSaveAutoLogRespRegistrant != null && mSaveAutoLogRespRegistrant.getHandler() == h) {
            mSaveAutoLogRespRegistrant.clear();
            mSaveAutoLogRespRegistrant = null;
        }
    }

    private String checkLoggingOption(String type) {
        String buffer = "";
        SharedPreferences sharedPreferences = mContext.getSharedPreferences("SHARED_PREF", mContext.MODE_PRIVATE);
        SharedPreferences prefSettings = PreferenceManager.getDefaultSharedPreferences(mContext);

        if (type.equalsIgnoreCase("AP")) {
            String apList[];
            apList = mContext.getResources().getStringArray(R.array.ap_logtype);
            for (int i = 0; i < apList.length; i++) {
                if (sharedPreferences.getBoolean(apList[i], true)) {
                    buffer = buffer + " -b " + apList[i];
                    if (i == 0)
                        break;
                }
            }
        } else if (type.equalsIgnoreCase("TCP")) {
            /*
             * for (int i = 0; i < SilentLoggingConstant.tcpLogArray.length; i++) { if
             * (sharedPreferences.getBoolean(SilentLoggingConstant.tcpLogArray[i], false)) {
             * buffer = buffer + SilentLoggingConstant.tcpLogArray[i]; break; } }
             */
            buffer = prefSettings.getString("key_tcp_logging_type", "any");
            Log.i(TAG, "prefSettings tcp type : " + buffer);
        } else if (type.equalsIgnoreCase("CP")) {
            try {
                if (prefSettings.getBoolean("key_cp_tcp_dump", false)) {
                    Log.d(TAG, "CP TCP Request");
                    if (!SilentLoggingProfile.getInstance().sendRequest(SilentLoggingConstant.START_TCP_LOGS))
                        return "false";
                }
            } catch (Exception ex) {
                Log.d(TAG, "CP TCPRequest ex : " + ex);
            }
        }
        return buffer;
    }

    private void startAPLogging(String filePath, SilentLoggingConstant.LOG_MODE loggingType) {
        Log.d(TAG, "startAPLogging() ++");
        try {
            SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
            String currTime = dateFormat.format(new Date()).toString();

            String buffer;
            if (loggingType == SilentLoggingConstant.LOG_MODE.SILENT) {
                buffer = checkLoggingOption("AP");
            } else {
                buffer = "-b all";
            }

            String shellCmd = "logcat -v time " + buffer + " -f " + filePath;
            if (loggingType == SilentLoggingConstant.LOG_MODE.SILENT) {
                shellCmd += currTime + SilentLoggingConstant.LOG_FILE_POST_EX;
            }
            Log.i(TAG, "shellcmd : " + shellCmd);

            if (loggingType == SilentLoggingConstant.LOG_MODE.SILENT) {
                mOemService.startApSilentLogging(SilentLoggingConstant.COMMAND_LOGCAT, shellCmd.getBytes());
            } else if (loggingType == SilentLoggingConstant.LOG_MODE.SSHOT) {
                mOemService.startApSilentLogging(SilentLoggingConstant.COMMAND_LOGCAT_SNAPSHOT, shellCmd.getBytes());
            }
        } catch (Throwable tr) {
            Log.e(TAG, "Uncaught exception", tr);
        }
        Log.d(TAG, "startAPLogging() --");
    }

    private void startTCPLogging(String filePath, SilentLoggingConstant.LOG_MODE loggingType) {
        Log.d(TAG, "startTCPLogging() ++");
        try {
            SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
            String currTime = dateFormat.format(new Date()).toString();

            String buffer;
            if (loggingType == SilentLoggingConstant.LOG_MODE.SILENT) {
                buffer = checkLoggingOption("TCP");
            } else {
                buffer = "any";
            }
            String shellCmd = "tcpdump -i " + buffer + " -p -s 0 -B 32768 -n -w " + filePath;
            if (loggingType == SilentLoggingConstant.LOG_MODE.SILENT) {
                shellCmd += currTime + SilentLoggingConstant.TCP_FILE_POST_EX;
            }
            if (loggingType == SilentLoggingConstant.LOG_MODE.SILENT) {
                mOemService.startApSilentLogging(SilentLoggingConstant.COMMAND_TCP_DUMP, shellCmd.getBytes());
            } else if (loggingType == SilentLoggingConstant.LOG_MODE.SSHOT) {
                mOemService.startApSilentLogging(SilentLoggingConstant.COMMAND_TCP_DUMP_SNAPSHOT, shellCmd.getBytes());
            }
        } catch (Throwable tr) {
            Log.e(TAG, "Uncaught exception", tr);
        }
        Log.d(TAG, "startTCPLogging() --");
    }

    private void startAPLogging(SilentLoggingConstant.LOG_MODE loggingType) {
        if (mContext != null) {
            Intent i = new Intent(mContext, RemoteLogcatService.class);
            String direcotryPath = mSlogPath;
            if (loggingType == SilentLoggingConstant.LOG_MODE.SSHOT) {
                i.setAction(RemoteLogcatService.ACTION_DUMP_LOGCAT_SERVICE);
                direcotryPath += "/snapshot";
            }
            else {
                i.setAction(RemoteLogcatService.ACTION_START_LOGCAT_SERVICE);
            }
            i.putExtra("path", direcotryPath);
            mContext.startService(i);
        }
    }

    private void stopAPLogging() {
        if (mContext != null) {
            Intent i = new Intent(mContext, RemoteLogcatService.class);
            i.setAction(RemoteLogcatService.ACTION_STOP_LOGCAT_SERVICE);
            mContext.startService(i);
        }
    }

    public void saveLoggingPid(int id, int pid) {
        Log.d(TAG, "saveLoggingPid() ++");
        SharedPreferences sharedPreferences = mContext.getSharedPreferences("SHARED_PREF", mContext.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();

        if (id == SilentLoggingConstant.COMMAND_LOGCAT) {
            editor.putString("AP_PID", "" + pid);
        } else if (id == SilentLoggingConstant.COMMAND_LOGCAT_SNAPSHOT) {
            editor.putString("SS_AP_PID", "" + pid);
        } else if (id == SilentLoggingConstant.COMMAND_TCP_DUMP) {
            editor.putString("TCP_PID", "" + pid);
        } else if (id == SilentLoggingConstant.COMMAND_TCP_DUMP_SNAPSHOT) {
            editor.putString("SS_TCP_PID", "" + pid);
        }
        editor.commit();
        Log.i(TAG, "PID  = " + Integer.toString(pid));
        Log.d(TAG, "saveLoggingPid() --");
    }

    private void startCPLogging() {
        /*
         * if (isEnabled()) { //already return; }
         */
        if (isStarted() == false) {
            deleteSegmentFiles(); // delete all files before start
            if (checkFreeSpace() == true) {
                createHeaderfile(); // to store timezone information, dmd will save it
                silentModeRequest();
            }
        } else {
            Log.d(TAG, "Silent Log already running, isStarted = " + isStarted());
        }
    }

    private void stopCpLogging() {
        mOemService.stopCpSilentLogging(SilentLoggingConstant.SET_SDM_MODE_DM);
        Log.d(TAG, "ShannonDM ModeRequest : Complete");
    }

    public void setDmStartCommand(int vendorId, int profileCommand) {
        int val = vendorId << 8 | profileCommand;

        DataWriter dw = new DataWriter();
        try {
            dw.writeBytes(SilentLoggingConstant.DM_START_OPTION_HEAD);
            dw.writeInt(val);
            dw.writeByte(SilentLoggingConstant.DM_COMMAND_POST);
        } catch (IOException e) {
            Log.i(TAG, "setDmStartCommand() IOException" + e);
        }
        SilentLoggingConstant.mStartCommand = dw.toByteArray();
    }

    public static void showToastMessage(String str) {
        if (mToast == null)
            mToast = Toast.makeText(mContext, str, Toast.LENGTH_LONG);
        else
            mToast.setText(str);
        mToast.show();
    }

    public void stopApTcpLogging(int id) {
        Log.d(TAG, "stopApTcpLogging() ++");
        SharedPreferences sharedPreferences = mContext.getSharedPreferences("SHARED_PREF", mContext.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        Log.i(TAG, sharedPreferences.getString("AP_PID", "") + "   " + sharedPreferences.getString("TCP_PID", ""));
        Log.i(TAG, sharedPreferences.getString("SS_AP_PID", "") + "   " + sharedPreferences.getString("SS_TCP_PID", ""));

        if (mOemService != null) {
            String shellCmd = "";
            if (id == SilentLoggingConstant.COMMAND_LOGCAT) {
                shellCmd = "kill " + sharedPreferences.getString("AP_PID", "");
            } else if (id == SilentLoggingConstant.COMMAND_LOGCAT_SNAPSHOT) {
                shellCmd = "kill " + sharedPreferences.getString("SS_AP_PID", "");
            } else if (id == SilentLoggingConstant.COMMAND_TCP_DUMP) {
                shellCmd = "kill " + sharedPreferences.getString("TCP_PID", "");
            } else if (id == SilentLoggingConstant.COMMAND_TCP_DUMP_SNAPSHOT) {
                shellCmd = "kill " + sharedPreferences.getString("SS_TCP_PID", "");
            }
            mOemService.stopApTcpSilentLogging(shellCmd.getBytes());

            if (id == SilentLoggingConstant.COMMAND_LOGCAT) {
                editor.putString("AP_PID", "");
            } else if (id == SilentLoggingConstant.COMMAND_TCP_DUMP) {
                editor.putString("TCP_PID", "");
            } else if (id == SilentLoggingConstant.COMMAND_LOGCAT_SNAPSHOT) {
                editor.putString("SS_AP_PID", "");
            } else if  (id == SilentLoggingConstant.COMMAND_TCP_DUMP_SNAPSHOT) {
                editor.putString("SS_TCP_PID", "");
            }
            editor.commit();
        } else {
            Log.e(TAG, "mOemService is null");
        }
        Log.d(TAG, "stopApTcpLogging() --");
    }

    public void copyLog() {
        copyToStorage();
    }

    private void copyToStorage() {
        LayoutInflater inflater = LayoutInflater.from(mContext);
        final View dialogView = inflater.inflate(R.layout.dialog_storage, null);
        final RadioButton dialog_rb_internal = (RadioButton) dialogView.findViewById(R.id.dialog_rb_internal);
        final RadioButton dialog_rb_sdcard = (RadioButton) dialogView.findViewById(R.id.dialog_rb_sdcard);
        final File sdcardDirectory = new File(SilentLoggingConstant.SDCARD_DIR);
        final File dataDirectory = new File(mSlogPath);
        final File internalDirectory = new File(Environment.getExternalStorageDirectory().toString() + "/DMLog");
        AlertDialog.Builder mBuilder;
        mBuilder = new AlertDialog.Builder(mContext);
        mBuilder.setTitle("Setting storage");
        mBuilder.setView(dialogView);
        mBuilder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub
                showProgressDialog("Wait... Copying Log Files");
                Thread thread = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        // TODO Auto-generated method stub
                        String result = "";
                        if (dialog_rb_internal.isChecked()) {
                            Log.d(TAG, "internel storage is checked");
                            result = copyDirectory(dataDirectory, internalDirectory);
                        } else {
                            Log.d(TAG, "sdcard storage is checked");
                            result = copyDirectory(dataDirectory, sdcardDirectory);
                        }
                        Message msg = mCopyResultHandler.obtainMessage();
                        msg.what = COPY_TO_STORAGE_DONE;
                        msg.obj = result;
                        mCopyResultHandler.sendMessage(msg);
                    }
                });
                thread.start();
            }
        });
        mBuilder.setNegativeButton(android.R.string.cancel, null);
        mBuilder.show();

        if (!Environment.getExternalStorageState(sdcardDirectory).equals(android.os.Environment.MEDIA_MOUNTED)) {
            Log.d(TAG, "getExternalStorageState : no sdcard directory");
            dialog_rb_sdcard.setClickable(false);
            dialog_rb_sdcard.setTextColor(Color.GRAY);
        }
    }

    public Handler mCopyResultHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {

            case COPY_TO_STORAGE_DONE: {
                Log.i(TAG, "in mCopyResultHandler COPY_TO_STORAGE_DONE");
                hideProgressDialog();
                if ((String) msg.obj == "") {
                    ResultMessage("No Log files present");
                } else {
                    ResultMessage("Copy files Complete \n" + (String) msg.obj);
                }
                break;
            }

            default:
                break;
            }
        }
    };

    private String copyDirectory(File src, File dst) {
        Log.d(TAG, "Copy Directory : To " + dst + " from " + src);
        FileInputStream srcFile = null;
        FileOutputStream dstFile = null;
        FileChannel srcChannel = null;
        FileChannel dstChannel = null;

        String result = "";

        if (src.isDirectory()) {
            if (!dst.exists()) {
                dst.mkdir();
            }

            String[] fileList = src.list();

            if (fileList == null || fileList.length <= 0)
                return "No file in " + src.getPath().substring(mSlogPath.length()) + " folder.\n";

            for (int i = 0; i < fileList.length; i++) {
                if (fileList[i].endsWith(SilentLoggingConstant.HEADER_FILE_POST_EX)) // skip header
                {
                    Log.d(TAG, "skip header file : " + fileList[i]);
                    continue;
                }
                if (fileList[i] .endsWith(SilentLoggingConstant.GZIP_FILE_POST_EX)) // support // .gz
                {
                    Log.d(TAG, ".gz file found : " + fileList[i]);
                    result += copyDirectory(new File(src, fileList[i]), new File(dst, fileList[i]));
                }
                if (fileList[i] .endsWith(SilentLoggingConstant.ZIP_FILE_POST_EX)) // support // .zip
                {
                    Log.d(TAG, ".ZIP file found : " + fileList[i]);
                    result += copyDirectory(new File(src, fileList[i]), new File(dst, fileList[i]));
                }
                if (fileList[i].endsWith(SilentLoggingConstant.SDM_FILE_POST_EX)) // support .sdm
                {
                    Log.d(TAG, ".sdm file found : " + fileList[i]);
                    result += copyDirectory(new File(src, fileList[i]), new File(dst, fileList[i]));
                }
                if (fileList[i].endsWith(SilentLoggingConstant.LOG_FILE_POST_EX)) // support .log
                {
                    Log.d(TAG, ".log file found : " + fileList[i]);
                    result += copyDirectory(new File(src, fileList[i]), new File(dst, fileList[i]));
                }
                if (fileList[i].endsWith(SilentLoggingConstant.TCP_FILE_POST_EX)) // support .pcap
                {
                    Log.d(TAG, ".pcap file found : " + fileList[i]);
                    result += copyDirectory(new File(src, fileList[i]), new File(dst, fileList[i]));
                }
            }
        } else {
            try {
                if (src.exists() == false) {
                    return src.getName() + " doesn't exist!!\n";
                }

                srcFile = new FileInputStream(src);
                dstFile = new FileOutputStream(dst);
                srcChannel = srcFile.getChannel();
                dstChannel = dstFile.getChannel();
                long size = srcChannel.size();

                srcChannel.transferTo(0, size, dstChannel);
                result += src.getPath().substring(mSlogPath.length() + 1) + " was copied.\n";
            } catch (Exception e) {
                e.printStackTrace();
                Log.d(TAG, "Exception : " + e);
            } finally {
                try {
                    if (src.exists()) {
                        if (srcChannel != null)
                            srcChannel.close();
                        if (srcFile != null)
                            srcFile.close();
                        if (dstChannel != null)
                            dstChannel.close();
                        if (dstFile != null)
                            dstFile.close();
                    }
                    MediaScannerConnection.scanFile(mContext, new String[] { dst.toString() }, null,
                            new MediaScannerConnection.OnScanCompletedListener() {
                                @Override
                                public void onScanCompleted(String path, Uri uri) {
                                    // TODO Auto-generated method stub
                                    Log.i(TAG, "Scanned " + path + ":");
                                    Log.i(TAG, "-> uri=" + uri);
                                }
                            });
                } catch (IOException e) {
                    e.printStackTrace();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
        return result;
    }

    private void SearchandDeleteFile(String delfile) {
        String sourceDir = mSlogPath;
        File dir = new File(sourceDir);
        Log.d(TAG, "Delete File :" + delfile);

        for (File f : dir.listFiles()) {
            if (f.getName().startsWith((delfile))) {
                Log.d(TAG, "Searched file to delete :" + f.getName());
                f.delete();
                break;
            }
        }
    }

    public void SearchAndDeleteSegmentFiles() {
        // time stamp based file search and delete
        Log.d(TAG, "SearchAndDeleteSegmentFiles: ");
        String sourceDir = mSlogPath;
        File dir = new File(sourceDir);
        if (dir.isDirectory()) {
            int count = SearchAndDeleteSegmentFiles(dir);;
            ResultMessage("Total " + count + " files have been deleted.");
            if (count > 0) {
                mOemService.refreshFileList();
            }
        }
    }

    public int SearchAndDeleteSegmentFiles(File workingDirectory) {
        if (workingDirectory == null) {
            Log.d(TAG, "SearchAndDeleteSegmentFiles: invalid workingDirectory");
            return 0;
        }

        if (!workingDirectory.isDirectory()) {
            Log.d(TAG, "SearchAndDeleteSegmentFiles: " + workingDirectory + " not a directory path");
            return 0;
        }

        int count = 0;
        for (File f : workingDirectory.listFiles()) {
            if (f.isDirectory()) {
                count += SearchAndDeleteSegmentFiles(f);
            }
            else {
                if (f.getName().endsWith(SilentLoggingConstant.GZIP_FILE_POST_EX)) {
                    Log.d(TAG, "Delete gz file :" + f.getName());
                    if (f.delete())
                        count++;
                }
                else if (f.getName().endsWith(SilentLoggingConstant.ZIP_FILE_POST_EX)) {
                    Log.d(TAG, "Delete zip file :" + f.getName());
                    if (f.delete())
                        count++;
                }
                else if (f.getName().endsWith(SilentLoggingConstant.LOG_FILE_POST_EX)) {
                    Log.d(TAG, "Delete log file :" + f.getName());
                    if (f.delete())
                        count++;
                }
                else if (f.getName().endsWith(SilentLoggingConstant.TCP_FILE_POST_EX)) {
                    Log.d(TAG, "Delete pcap file :" + f.getName());
                    if (f.delete())
                        count++;
                }
                else if (f.getName().endsWith(SilentLoggingConstant.SEGMENT_FILE_EXT)) {
                    Log.d(TAG, "Delete sdm file :" + f.getName());
                    if (f.delete())
                        count++;
                }
            }

        } // end for ~
        return count;
    }

    public boolean showProgressDialog(String msg) {
        Log.d(TAG, "showProgressDialog()");

        if (progressDialog == null) {
            progressDialog = new ProgressDialog(SilentLoggingControlInterface.mContext);
        }

        try {
            progressDialog.setMessage(msg);
            progressDialog.setIndeterminate(true);
            progressDialog.setCancelable(false);
            progressDialog.show();
        } catch (WindowManager.BadTokenException e) {
            Log.d(TAG, "BadTokenException: " + e);
            progressDialog = null;
        } catch (Exception e) {
            Log.d(TAG, "showProgressDialog() Exception: " + e);
            progressDialog = null;
        }

        return true;
    }

    public static boolean isStarted() {
        String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_SILENTLOG_MODE));
        if (prop.equalsIgnoreCase("On")) {
            return true;
        } else {
            return false;
        }
    }

    public boolean hideProgressDialog() {
        Log.i(TAG, "hideProgressDialog()");
        if (progressDialog == null) {
            return false;
        }

        try {
            if (progressDialog.isShowing() && (progressDialog.getWindow() != null)) {
                progressDialog.dismiss();
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            progressDialog = null;
        }

        return true;
    }

    public static SilentLoggingConstant.DM_MODE getMode() {
        SilentLoggingConstant.DM_MODE mode = SilentLoggingConstant.DM_MODE.SHANNON;

        String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_SILENTLOG_MODE));
        if (prop.equalsIgnoreCase("On"))
            mode = SilentLoggingConstant.DM_MODE.SILENT;
        else if (prop.equalsIgnoreCase(""))
            mode = SilentLoggingConstant.DM_MODE.SHANNON;
        return mode;
    }

    private Handler startHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (getMode() == SilentLoggingConstant.DM_MODE.SILENT) {
                ResultMessage("Silent Logging Start : OK");
            } else {
                ResultMessage("Silent Logging Start : ERROR");
            }
        }
    };

    public void ResultMessage(String message) {
        try {
            AlertDialog.Builder alert_builder = new AlertDialog.Builder(SilentLoggingControlInterface.mContext);
            alert_builder.setMessage(message).setCancelable(false).setPositiveButton(android.R.string.yes,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int id) {
                            dialog.dismiss();
                        }
                    });

            AlertDialog alert_dialog = alert_builder.create();
            alert_dialog.setTitle("Result");
            alert_dialog.show();
        } catch (Exception e) {
            Log.w(TAG, "" + e);
            e.printStackTrace();
        }
    }

    public void waitdialog() {
        if (isStarted() == false) {
            showProgressDialog("Ready for CP Silent Logging....");
            new Thread() {
                @Override
                public void run() {
                    int count = 10;
                    while (count-- > 0) {
                        if (getMode() == SilentLoggingConstant.DM_MODE.SILENT) {
                            hideProgressDialog();
                            startHandler.sendEmptyMessage(0);
                            return;
                        }
                        delay(1000);
                    }
                    hideProgressDialog();
                    startHandler.sendEmptyMessage(0);
                }
            }.start();
        } else {
            Log.d(TAG, "skip showing waitdialog, already silent log enabled ");
        }
    }

    private void delay(int delay) {
        try {
            Thread.sleep(delay);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void deleteSegmentFiles() {
        try {
            String sourceDir = mSlogPath;
            File dir = new File(sourceDir);

            for (File f : dir.listFiles()) {
                if (f.getName().contains("profile")) {
                    Log.d(TAG, "Delete profile file :" + f.getName());
                    f.delete();
                }
            }
        } catch (Exception ex) {
            Log.d(TAG, "deleteSegmentFiles ex : " + ex);
        }
    }

    public boolean checkFreeSpace() {
        int totalSize = 0;
        int toMB = 1024 * 1024;
        long usableSpace = 0;
        File checkfile = new File(mSlogPath);
        Log.d(TAG, "getTotalSpace: " + checkfile.getTotalSpace() / toMB);
        Log.d(TAG, "getUsableSpace: " + checkfile.getUsableSpace() / toMB);
        Log.d(TAG, "getFreeSpace: " + checkfile.getFreeSpace() / toMB);
        /*
         * usableSpace = checkfile.getUsableSpace() / toMB; //convert to MB String
         * maxSize = String.valueOf(SystemProperties.get(DMSettingsConstant.
         * PROPERTY_SILENTLOG_LOG_SIZE)); if (maxSize.isEmpty()) { Log.d(TAG,
         * "checkFreeSpace " + DMSettingsConstant.PROPERTY_SILENTLOG_LOG_SIZE +
         * " is : EMPTY "); return true; // temporary fix for device bootup with silent
         * log } else { totalSize = Integer.parseInt(maxSize); if (totalSize == 500) {
         * if (usableSpace > totalSize) { return true; } else { Log.d(TAG,
         * "free space is less than 500 MB  :  " + usableSpace);
         * ResultMessage("Free space is less than 500 MB"); return false; } } else if
         * (usableSpace > (totalSize * 1000)) { // convert to MB and check return true;
         * } else {
         * ResultMessage("Free space not available. Please free more space or decrease silent log max size"
         * ); return false; } }
         */
        return true;
    }

    private void deleteConfigFile() {
        File configFile = new File(mSlogPath + "/" + SilentLoggingConstant.SLOG_FILENAME);

        if (configFile.isFile() && configFile.exists())
            configFile.delete();
        Log.d(TAG, "Delete config file done");
    }

    public static boolean isEnabled() {
        // Check enable status
        boolean isConfigFile = false;

        // Check weather the Silent Log Flag file exists
        File SLogConfigFile = new File(mSlogPath + "/" + SilentLoggingConstant.SLOG_FILENAME);
        if (SLogConfigFile.isFile() && SLogConfigFile.exists())
            isConfigFile = true;

        return (isConfigFile ? true : false);
    }

    private boolean createHeaderfile() {
        // Create file containing header information - to be merged inside dmd before
        // saving logs

        String strDmLogFile = mSlogPath + "/." + SilentLoggingConstant.SEGMENT_FILE_PREFIX + "_header" + "."
                + SilentLoggingConstant.SEGMENT_FILE_EXT;
        byte[] bTimeStmp = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        TimeZone tz = Calendar.getInstance().getTimeZone();
        String TimeZoneid = tz.getID();
        byte[] timeZbyte = TimeZoneid.getBytes();

        try {

            File checkfile = new File(strDmLogFile);
            if (checkfile.isFile() && checkfile.exists()) {
                checkfile.delete();
                Log.d(TAG, "header file exists, deleting Headerfile : " + checkfile);
            }

            FileOutputStream FileOut = new FileOutputStream(strDmLogFile);

            byte[] buffer = new byte[1024];
            int nRead = 0;

            // Start Change file header time stamp
            bTimeStmp[0] = 0x00;
            bTimeStmp[1] = 0x00;
            bTimeStmp[2] = 0x00;
            bTimeStmp[3] = 0x00;
            bTimeStmp[4] = 0x00;
            bTimeStmp[5] = 0x00;
            // End Change file header time stamp
            byte[] tmpSdmHead = { 0x00, 0x00, 0x39, (byte) 0xFD, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x53,
                    0x69, 0x6c, 0x65, 0x6e, 0x74, 0x20, 0x4c, 0x6f, 0x67, 0x20, 0x56, 0x65, 0x72, 0x2e, 0x20, 0x32,
                    0x20, 0x53, 0x65, 0x70, 0x2e, 0x20, 0x30, 0x36, 0x2c, 0x32, 0x30, 0x31, 0x32, 0x08, 0x00,
                    bTimeStmp[0], bTimeStmp[1], bTimeStmp[2], bTimeStmp[3], bTimeStmp[4], bTimeStmp[5], 0x00, 0x00,
                    (byte) timeZbyte.length, 0x00 };

            byte[] SdmLogVersionInfo = combinebytearray(tmpSdmHead, timeZbyte);
            SdmLogVersionInfo[0] = (byte) (SdmLogVersionInfo.length - 2);
            FileOut.write(SdmLogVersionInfo, 0, SdmLogVersionInfo.length);

            Log.i(TAG, "Header Operation finished");
            FileOut.close();
        } catch (IOException e) {
            Log.d(TAG, "IOException: " + e);
            return false;
        }
        return true;
    }

    private void silentModeRequest() {
        // mode change
        mOemService.setDmMode(SilentLoggingConstant.MODE_SILENT_LOGGING);
        if (!SilentLoggingProfile.getInstance().sendDefaultProfile()) {
            Log.d(TAG, "SilentModeRequest : sendDefaultProfile fail");
            return;
        }
    }

    public void snapshotLog() {
        // do on thread
        // if (!isSavefromIntent()) {
        showProgressDialog("Wait...");
        // }
        String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_MODE));
        if (prop.equalsIgnoreCase("On")) {
            if (mSLogRun == null) {
                mSLogRun = new SilentLogRunnable();
                mSLogThread = new Thread(mSLogRun);
                mSLogThread.setDaemon(true);
                mSLogThread.start();
            } else {
                Log.d(TAG, "Already saving a Silent Log...");
            }
        } else {
            Log.d(TAG, "silent log is not enabled now");
        }
    }

    class SilentLogRunnable implements Runnable {
        @Override
        public void run() {
            //Looper.prepare();
            String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_MODE));
            if (prop.equalsIgnoreCase("On")) {
                Log.d(TAG, "SilentLogRunnable thread started");

                if(checkApMode()) {
                    //startAPLogging(mSlogPath + SilentLoggingConstant.SNAPSHOT_TARGET_DIR + "/" + SilentLoggingConstant.SNAPSHOT_APLOG_FILENAME, SilentLoggingConstant.LOG_MODE.SSHOT);
                    startAPLogging(SilentLoggingConstant.LOG_MODE.SSHOT);
                }
                if (checkTcpMode()) {
                    startTCPLogging(
                            mSlogPath + SilentLoggingConstant.SNAPSHOT_TARGET_DIR + "/"
                                    + SilentLoggingConstant.SNAPSHOT_TCPLOG_FILENAME,
                            SilentLoggingConstant.LOG_MODE.SSHOT);
                }
                if (checkCpMode()) {
                    int counter = -1;

                    mOemService.saveSnapshot(SilentLoggingConstant.START_SAVING_LOGS);

                    // To do. use handler for snapshot logging
                    delay(3000); // add read delay , file is being created under slog dir by dmd module
                    makeLogFile();
                } else {
                    delay(3000);
                    makeLogFile();
                }
                stopApTcpLogging(SilentLoggingConstant.COMMAND_TCP_DUMP_SNAPSHOT);
                mSLogRun = null;
                Log.d(TAG, "SilentLogRunnable thread terminated");
            } else {
                Log.d(TAG, "Silent Log is disabled");
            }
            //Looper.loop();
        }
    }

    private void makeLogFile() {
        String mLogSaveTime = null;
        final int BUFFER = 4096;
        Runtime rt = Runtime.getRuntime();
        DateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
        Date date = new Date();
        mLogSaveTime = dateFormat.format(date);

        String strZipFile = mSlogPath + SilentLoggingConstant.SNAPSHOT_TARGET_DIR + "/"
                + SilentLoggingConstant.SNAPSHOT_FILE_PRE_EX + mLogSaveTime + SilentLoggingConstant.ZIP_FILE_POST_EX;

        Log.d(TAG, "ZIP filename: " + strZipFile + "\n");
        // Dump main and radio logs in a zip file with sdm file
        try {
            // Make zip file
            FileOutputStream fileOut = new FileOutputStream(strZipFile);
            ZipOutputStream zipOut = new ZipOutputStream(new BufferedOutputStream(fileOut));
            byte data[] = new byte[BUFFER];
            File[] fileTargets = new File[3];
            if (checkCpMode()) {
                fileTargets[0] = new File(mSlogPath + SilentLoggingConstant.SNAPSHOT_TARGET_DIR + "/"
                        + SilentLoggingConstant.SILENT_LOG_FILE_EX);
            } else {
                fileTargets[0] = null;
            }
            if (checkApMode()) {
                fileTargets[1] = new File(mSlogPath + SilentLoggingConstant.SNAPSHOT_TARGET_DIR + "/"
                        + SilentLoggingConstant.SNAPSHOT_APLOG_FILENAME);
            } else {
                fileTargets[1] = null;
            }
            if (checkTcpMode()) {
                fileTargets[2] = new File(mSlogPath + SilentLoggingConstant.SNAPSHOT_TARGET_DIR + "/"
                        + SilentLoggingConstant.SNAPSHOT_TCPLOG_FILENAME);
            } else {
                fileTargets[2] = null;
            }
            // Start Zip process
            for (File target : fileTargets) {
                if (target != null) {
                    FileInputStream fileIn = new FileInputStream(target.getAbsolutePath());
                    BufferedInputStream buffIn = new BufferedInputStream(fileIn, BUFFER);
                    ZipEntry entry = new ZipEntry(target.getName());
                    zipOut.putNextEntry(entry);

                    int nRead;
                    while ((nRead = buffIn.read(data, 0, BUFFER)) != -1) {
                        zipOut.write(data, 0, nRead);
                    }
                    buffIn.close();
                    zipOut.closeEntry();
                    if (target.isFile() && target.exists() && target != null) {
                        Log.d(TAG, "delete target filename : " + target);
                        target.delete();
                    }
                }
            }
            zipOut.finish();
            zipOut.close();
        } catch (IOException e) {
            Log.d(TAG, "IOException" + e);
        }

        try {
            // Change permission
            Process prPermission = null;
            prPermission = rt.exec("chmod 755 " + strZipFile);
            // wait until processes finish
            try {
                prPermission.waitFor();
            } catch (InterruptedException ie) {
                Log.d(TAG, "Exception" + ie);
            }
        } catch (IOException e) {
            Log.d(TAG, "IOException" + e);
        }
        // if (!isSavefromIntent()) {
        hideProgressDialog();
        ResultMessage("Log saved to " + strZipFile);
        Log.d(TAG, "Log saved as.." + strZipFile);
        // }
    }

    private boolean appendtoFile(File src, File dst) {
        Log.d(TAG, "Append File : To " + dst + " from " + src);
        FileInputStream srcFile = null;
        FileOutputStream dstFile = null;
        FileChannel srcChannel = null;
        FileChannel dstChannel = null;
        try {
            if (src.exists() == false) {
                return false;
            }

            srcFile = new FileInputStream(src);
            dstFile = new FileOutputStream(dst, true);
            srcChannel = srcFile.getChannel();
            dstChannel = dstFile.getChannel();
            long size = srcChannel.size();
            Log.d(TAG, "dstChannel.size() : " + dstChannel.size());
            dstChannel.position(dstChannel.size());

            srcChannel.transferTo(0, size, dstChannel);
        } catch (Exception e) {
            e.printStackTrace();
            Log.d(TAG, "Exception : " + e);
        } finally {
            try {
                if (src.exists()) {
                    if (srcChannel != null)
                        srcChannel.close();
                    if (srcFile != null)
                        srcFile.close();
                    if (dstChannel != null)
                        dstChannel.close();
                    if (dstFile != null)
                        dstFile.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return true;
    }

    public static byte[] shortToByteArray(short value) {
        byte[] ret = new byte[2];
        ret[0] = (byte) (value & 0xFF);
        ret[1] = (byte) ((value >> 8) & 0xFF);

        return ret;
    }

    private byte[] combinebytearray(byte[] source1, byte[] source2) {
        byte[] destination = new byte[source1.length + source2.length];
        System.arraycopy(source1, 0, destination, 0, source1.length);
        System.arraycopy(source2, 0, destination, source1.length, source2.length);
        return destination;
    }
}
