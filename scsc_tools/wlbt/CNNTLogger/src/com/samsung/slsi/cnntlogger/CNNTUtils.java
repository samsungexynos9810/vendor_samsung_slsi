package com.samsung.slsi.cnntlogger;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.channels.FileChannel;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.os.SystemProperties;
import android.util.Log;

public class CNNTUtils {

    private Context mContext;

    CNNTUtils(Context context) {
        mContext = context;
    }

    protected boolean removeDirectory(final String path, boolean inclroot) {
        File file = new File(path);
        File[] subFileList = file.listFiles();
        for (File subFile : subFileList) {
            if (subFile.isDirectory()) {
                if (!removeDirectory(subFile.getAbsolutePath(), true)) return false;
            } else {
                if (!subFile.delete()) return false;
            }
        }

        if (inclroot) if (!file.delete()) return false;
        return true;
    }

    protected boolean removeFile(final String path) {
        File file = new File(path);
        if (file.exists()) {
            if (file.delete()) return true;
        }

        return false;
    }

    protected boolean copyFile(File src, File dst) {
        Log.d(CmdDefine.LOGTAG, "Copy to " + dst + " from " + src);
        FileInputStream srcFile = null;
        FileOutputStream dstFile = null;
        FileChannel srcChannel = null;
        FileChannel dstChannel = null;

        boolean result = true;

        if (src.isDirectory()) {
            if (!dst.exists()) {
                dst.mkdirs();
            }

            String[] fileList = src.list();
            if (fileList == null) {
                Log.d(CmdDefine.LOGTAG, "Log file dosn't exist!");
                return true;
            }

            for (int i = 0; i < fileList.length; i++) {
                if (result)
                    result = copyFile(new File(src, fileList[i]), new File(dst, fileList[i]));
            }

        } else {
            try {
                if (!src.exists()) {
                    result = false;
                }

                if (!dst.exists()) {
                    srcFile = new FileInputStream(src);
                    dstFile = new FileOutputStream(dst);
                    srcChannel = srcFile.getChannel();
                    dstChannel = dstFile.getChannel();
                    long size = srcChannel.size();
                    srcChannel.transferTo(0, size, dstChannel);
                }
            } catch (Exception e) {
                result = false;
                Log.e(CmdDefine.LOGTAG, e.toString());
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
                } catch (Exception e) {
                    Log.e(CmdDefine.LOGTAG, e.toString());
                }
            }
        }
        return result;

    }

    protected String getSystemLoggingPath() {
        String path = String.valueOf(SystemProperties.get(CmdDefine.SYSTEM_PROPERTY_LOGGING_PATH));
        return path.isEmpty() ? CmdDefine.dirPath : path;
    }

    protected String getSystemSdcardPath() {
        String path = String.valueOf(SystemProperties.get(CmdDefine.SYSTEM_PROPERTY_SDCARD_PATH));
        return path.isEmpty() ? CmdDefine.SDCARD_DEFAULT_DIR : path;
    }

    protected void setPreference(String key, boolean value) {
        SharedPreferences pref = mContext.getSharedPreferences(key, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
        editor.putBoolean(key, value).apply();
    }

    protected boolean getPreference(String key, boolean value) {
        SharedPreferences pref = mContext.getSharedPreferences(key, Context.MODE_PRIVATE);
        return pref.getBoolean(key, value);
    }

    protected void setPreference(String key, String value) {
        SharedPreferences pref = mContext.getSharedPreferences(key, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
        editor.putString(key, value).apply();
    }

    protected String getPreference(String key, String value) {
        SharedPreferences pref = mContext.getSharedPreferences(key, Context.MODE_PRIVATE);
        return pref.getString(key, value);
    }

    protected String getSettingPreference(String key, String value) {
        SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(mContext);
        return pref.getString(key, value);
    }

    protected String[] getStringArr(int value) {
        return mContext.getResources().getStringArray(value);
    }

    public String getCurrentTimeString() {
        SimpleDateFormat sdfNow = new SimpleDateFormat("MMddHHmmss");
        String time = sdfNow.format(new Date(System.currentTimeMillis()));
        return time;
    }

    public void sendWifiLoggingResult(boolean isSuccess) {
        Intent intent = new Intent("com.samsung.slsi.wifi.action.LOGGING_RESULT");
        intent.putExtra("result", isSuccess);
        mContext.sendBroadcast(intent);
    }

    public void sendBtLoggingResult(boolean isSuccess) {
        Intent intent = new Intent("com.samsung.slsi.bt.action.LOGGING_RESULT");
        intent.putExtra("result", isSuccess);
        mContext.sendBroadcast(intent);
    }

    public void sendLoggingResultBroadcast(String action, boolean isSuccess) {
        Log.d(CmdDefine.LOGTAG, "action : " + action + ", " + isSuccess + ", " + mContext.getPackageName());
        Intent intent = new Intent(action);
        /* pass : 1, fail : 0 */
        intent.putExtra("result", isSuccess? "1" : "0");
        intent.putExtra("from", mContext.getPackageName());
        mContext.sendBroadcast(intent);
    }

    public void sendBroadcast(String action) {
        Intent intent = new Intent(action);
        mContext.sendBroadcast(intent);
    }
}
