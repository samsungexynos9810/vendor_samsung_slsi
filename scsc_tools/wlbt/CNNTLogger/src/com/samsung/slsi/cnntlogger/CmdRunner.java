package com.samsung.slsi.cnntlogger;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.util.Log;

import org.json.JSONObject;

class CmdRunner {

    private final String TAG = "CmdRunner";

    private CNNTUtils mUtils;

    private LoggingJSONValue mLoggingJSONValue;

    private LocalSocket mWlandSocket;

    private boolean mIsLoggingSuccess = true;

    private static Process mProcess;

    void startScript(Context context, String scriptName, String time, String logType, String dirName) {
        BluetoothAdapter adapter;

        Log.d(TAG, "startScript " + scriptName);

        mUtils = new CNNTUtils(context);
        mLoggingJSONValue = new LoggingJSONValue();
        mIsLoggingSuccess = true;
        if (scriptName.equals(CmdDefine.StartScript)) {
            startLogging(time, logType, dirName);
            mUtils.sendWifiLoggingResult(mIsLoggingSuccess);
        } else if (scriptName.equals(CmdDefine.StopScript)) {
            killLogcatProcess();

            JSONObject json = mLoggingJSONValue.getJSonValue(CmdDefine.WIFI_LOG_ALL_STOP, "", null);
            execShellCmd(json);
            mUtils.sendWifiLoggingResult(mIsLoggingSuccess);
        } else if (scriptName.equals(CmdDefine.loggingStart)) {
            adapter = ((BluetoothManager)context.getSystemService(Context.BLUETOOTH_SERVICE)).getAdapter();
            if (CmdDefine.btAudioFilter.equals(mUtils.getPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btAudioFilter))) {
                adapter.cmdDBFW(CmdDefine.VSC_OPCODE_DBFW, CmdDefine.DBFW_SCO_DUMP, CmdDefine.SCO_PCM_TX_RX_DUMP, 65535);
            }

            adapter.cmdDBFW(CmdDefine.VSC_OPCODE_LINK_LAYER, 0, getLinkLayerMode(), 0);
            startLogging(time, logType, dirName);
            mUtils.sendBtLoggingResult(mIsLoggingSuccess);
        } else if (scriptName.equals(CmdDefine.loggingStop)) {
            adapter = ((BluetoothManager)context.getSystemService(Context.BLUETOOTH_SERVICE)).getAdapter();
            if (CmdDefine.btAudioFilter.equals(mUtils.getPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btAudioFilter))) {
                adapter.cmdDBFW(CmdDefine.VSC_OPCODE_DBFW, CmdDefine.DBFW_SCO_DUMP, CmdDefine.SCO_DUMP_DISABLE, 0);
            }

            adapter.cmdDBFW(CmdDefine.VSC_OPCODE_LINK_LAYER, 0, 0, 0);
            killLogcatProcess();

            JSONObject json = mLoggingJSONValue.getJSonValue(CmdDefine.BT_LOG_STOP, "", null);
            execShellCmd(json);
            mUtils.sendBtLoggingResult(mIsLoggingSuccess);
        } else { //CmdDefine.DeleteLog
            deleteAllLogs();
        }
        Log.d(CmdDefine.LOGTAG, "mIsLoggingSuccess " + mIsLoggingSuccess);
    }

    private int getLinkLayerMode() {
        String[] modes = mUtils.getStringArr(R.array.bt_ll_modes);
        int mode = 0;

        if (mUtils.getPreference(CmdDefine.KEY_BT_USE_DEFAULT_MODE, true)) {
            mode = 0x0011;
        } else {
            for (int i = 0; i < modes.length; i++) {
                if (mUtils.getPreference(modes[i], false)) {
                    if (i == 0) {
                        mode = 1;
                    } else {
                        mode += 1<<i;
                    }
                }
            }
        }

        Log.d(TAG, "link layer mode=" + mode);
        return mode;
    }

    private void killLogcatProcess() {
        if (mProcess != null) {
            Log.d(TAG, "process information " + mProcess.toString());

            if (mProcess.isAlive()) {
                mProcess.destroy();
                mProcess = null;
                Log.d(TAG, "kill process ");
            }
        }
    }

    private void cmdExecute(String cmd) throws RuntimeException {

        Runtime runtime = Runtime.getRuntime();
        try {
            mProcess = runtime.exec(cmd);

            Log.e(TAG, "pid : " + mProcess.toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void multiCmdExecute(String cmd[]) throws RuntimeException {

        Runtime runtime = Runtime.getRuntime();
        try {
            mProcess = runtime.exec(cmd);
            mProcess.waitFor();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private boolean isValidFile(String filePath) {
        if (filePath == null) return false;

        File file = new File(filePath);
        return file.exists();
    }

    private void doStartLogging(String type, String time, String dirName) {
        Log.d(CmdDefine.LOGTAG, type + " logging start >>>");

        String buffer;
        String logdir;
        String filePath = null;
        JSONObject jsonObject = null;

        if (dirName.equals("")) {
            logdir = mUtils.getSystemLoggingPath() + "Log_" + time + "/";
        } else {
            logdir = mUtils.getSystemLoggingPath() + dirName + "_" + time + "/";
        }

        File directory = new File(mUtils.getSystemLoggingPath());
        if (!directory.exists()) {
            directory.mkdirs();
        }

        File directory2 = new File(logdir);
        if (!directory2.exists()) {
            directory2.mkdirs();
        }

        if (type.equals(CmdDefine.TYPE_AP)) {
            buffer = checkLoggingOption("AP");
            String shellCmd = CmdDefine.lcLog + buffer + " -f " + logdir + type + "_" + time + ".log" + " -r 51200";
            cmdExecute(shellCmd);
            Log.d(CmdDefine.LOGTAG, "     [AP] shell CMD >>> " + shellCmd);
            return;
        } else if (type.equals(CmdDefine.TYPE_MX)) {
            filePath = logdir + "mxlog_" + time;
            jsonObject = mLoggingJSONValue.getJSonValue(CmdDefine.WIFI_LOG_MXLOG_START, filePath, null);
        } else if (type.equals(CmdDefine.TYPE_UDI)) {
            filePath = logdir + "udi_decode_" + time;
            jsonObject = mLoggingJSONValue.getJSonValue(CmdDefine.WIFI_LOG_UDILOG_START, filePath, null);
        } else {
            Log.e(CmdDefine.LOGTAG, "Not supported logtype: " + type);
            mIsLoggingSuccess = false;
            return;
        }

        execShellCmd(jsonObject);
    }

    private void startBTLogging(String cmd, String type, String time, String dirName) {
        Log.d(CmdDefine.LOGTAG, type + " startBTLogging " + cmd);

        String logdir;
        String filePath = null;
        JSONObject jsonObject = null;

        if (dirName.equals("")) {
            logdir = mUtils.getSystemLoggingPath() + "Log_" + time;
        } else {
            logdir = mUtils.getSystemLoggingPath() + dirName + "_" + time;
        }

        if (type.equals(CmdDefine.btNormalFilter)) {
            filePath = logdir + "/bt_general";
            jsonObject = mLoggingJSONValue.getJSonValue(CmdDefine.BT_LOG_NORMAL, filePath, null);
        } else if (type.equals(CmdDefine.btAudioFilter)) {
            filePath = logdir + "/bt_audio";
            jsonObject = mLoggingJSONValue.getJSonValue(CmdDefine.BT_LOG_AUDIO, filePath, null);
        } else if (type.startsWith(CmdDefine.btCustomFilter)) {
            String customFilter = type.substring(CmdDefine.btCustomFilter.length());
            filePath = logdir + "/bt_custom";
            jsonObject = mLoggingJSONValue.getJSonValue(CmdDefine.BT_LOG_CUSTOM, filePath, customFilter);
        } else {
            Log.e(CmdDefine.LOGTAG, "Not supported logtype: " + type);
        }

        execShellCmd(jsonObject);
    }

    private void startLogging(String timestamp, String logtype, String dirName) {
        String[] logs = logtype.split(" ");

        for (String type : logs) {
            if (isBtLoggingFilter(type)) {
                startBTLogging(CmdDefine.loggingStart, type, timestamp, dirName);
            } else {
                doStartLogging(type, timestamp, dirName);
            }
        }
    }

    private boolean isBtLoggingFilter(String filter) {
        return CmdDefine.btNormalFilter.equals(filter)
                || CmdDefine.btAudioFilter.equals(filter)
                || filter.startsWith(CmdDefine.btCustomFilter);
    }

    private void execShellCmd(JSONObject jsonObject) {
        if (jsonObject == null) {
            Log.e(CmdDefine.LOGTAG, "jsonObject is null");
            mIsLoggingSuccess = false;
            return;
        }

        try {
            mWlandSocket = new LocalSocket();
            mWlandSocket.connect(new LocalSocketAddress(CmdDefine.SOCKET_NAME, LocalSocketAddress.Namespace.ABSTRACT));

            Log.d(CmdDefine.LOGTAG, "<<< jsonObject: " + jsonObject);
            byte[] bytes = ByteBuffer.allocate(4).putInt(jsonObject.toString().length()).array();
            mWlandSocket.getOutputStream().write(bytes);
            mWlandSocket.getOutputStream().write(jsonObject.toString().getBytes());

            String execKey = (String) jsonObject.get(LoggingJSONValue.EXEC_KEY);
            boolean loggingStarted = CmdDefine.loggingStart.equals(execKey);
            if (loggingStarted) {
                byte[] recvBuf = new byte[10];
                int byteRead = mWlandSocket.getInputStream().read(recvBuf);
                byte[] response = new byte[byteRead];
                System.arraycopy(recvBuf, 0, response, 0, byteRead);
                String pid = new String(response, "US-ASCII");

                Log.d(CmdDefine.LOGTAG, ">>> pid: " + pid);
                if (Integer.parseInt(pid) == -1) {
                    mIsLoggingSuccess = false;
                }
            }

            Thread.sleep(100);
            mWlandSocket.close();

            if (loggingStarted) {
                String filePath = (String) jsonObject.get(LoggingJSONValue.DIR_KEY);

                if (CmdDefine.wifiCommand.equals(jsonObject.get(LoggingJSONValue.NAME_KEY))) {
                    filePath = filePath + "_0.log";
                } else if (CmdDefine.btCommand.equals(jsonObject.get(LoggingJSONValue.NAME_KEY))) {
                    filePath = filePath + "_mxlog_0.log";
                }

                if (filePath != null && !isValidFile(filePath)) {
                    mIsLoggingSuccess = false;
                }
            }
        } catch (IOException ex) {
            Log.e(CmdDefine.LOGTAG, "'" + CmdDefine.SOCKET_NAME + "' socket closed", ex);
            mIsLoggingSuccess = false;
        } catch (InterruptedException ie) {
            Log.e(CmdDefine.LOGTAG, ie.getLocalizedMessage());
            mIsLoggingSuccess = false;
        } catch (Throwable tr) {
            Log.e(CmdDefine.LOGTAG, "Uncaught exception", tr);
            mIsLoggingSuccess = false;
        }
    }

    private String checkLoggingOption(String type) {
        String buffer = "";

        if (type.equalsIgnoreCase("AP")) {
            String apList[];
            apList = mUtils.getStringArr(R.array.ap_logtype); //c.getResources().getStringArray(R.array.ap_logtype);

            for (int i = 0; i < apList.length; i++) {
                if (mUtils.getPreference(apList[i], true)) {
                    buffer = buffer + " -b " + apList[i];
                    if (i == 0) break;
                }
            }
        } else if (type.equalsIgnoreCase("TCP")) {
            buffer = mUtils.getSettingPreference("key_tcp_logging_type", "any");
            Log.i(CmdDefine.LOGTAG, "prefSettings tcp type : " + buffer);
        }

        return buffer;
    }

    private void deleteAllLogs() {
        File wlbtDir = new File(mUtils.getSystemLoggingPath());
        File sableDir = new File(CmdDefine.SABLE_LOG_DIR);
        boolean isSuccess = false;

        String shellCmd[] = { "/bin/sh", "-c", "rm -rf " + mUtils.getSystemLoggingPath() + "* " + CmdDefine.SABLE_LOG_DIR + "*" };
        multiCmdExecute(shellCmd);

        File[] wlbtList = wlbtDir.listFiles();
        File[] sableList = sableDir.listFiles();
        if ((wlbtList == null || (wlbtList != null && wlbtList.length == 0))
                && (sableList == null || (sableList != null && sableList.length == 0))) {
            isSuccess = true;
        }

        mUtils.sendLoggingResultBroadcast(CmdDefine.CLEAR_LOG_RESULT_INTENT, isSuccess);
        killLogcatProcess();
    }
}
