/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.logdump;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.Toast;

import com.samsung.slsi.sysdebugmode.R;
import com.samsung.slsi.telephony.oem.OemRil;

public class LogDumpActivity extends Activity {

    private static final String TAG = "LogDump";

    private static final String DUMP_BASE_DIR = "/data/vendor/dump";
    private static final String SDCARD_DIR = "/storage/sdcard/log";
    private static final String DUMPSTATE_BASE_FILE_NAME = DUMP_BASE_DIR+"/dumpstate_";
    private static final String LOGCAT_MAIN_BASE_FILE_NAME = DUMP_BASE_DIR+"/main_";
    private static final String LOGCAT_RADIO_BASE_FILE_NAME = DUMP_BASE_DIR+"/radio_";
    private static final String DMESG_BASE_FILE_NAME = DUMP_BASE_DIR+"/dmesg_";
    private static final String PROPERTY_CRASH_MODE = "persist.vendor.ril.crash_handling_mode";

    /* Item List */
    private static final int AP_DUMP = 0;
    private static final int CP_DUMP = 1;
    private static final int DELETION_LOG_FILES = 2;
    private static final int COPY_TO_STORAGE = 3;
    private static final int CRASH_HANDLING_MODE = 4;
    private static final int TS25_TABLE_DUMP = 5;

    /* Handle Message */
    private static final int EVENT_RIL_CONNECTED_FOR_CP_DUMP = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_AP_DUMP_DONE = 102;
    private static final int EVENT_SYSTEM_MODEM_DUMP_DONE = 103;
    private static final int EVEMT_COPY_TO_STORAGE_DONE = 104;
    private static final int EVENT_RIL_CONNECTED_FOR_TS25_TABLE_DUMP = 105;
    private static final int EVENT_TS25_TABLE_DUMP_DONE = 106;

    /* RIL Request */
    private static final int RILC_REQ_SYSTEM_MODEM_DUMP = 1;
    private static final int RILC_REQ_TS25_TABLE_DUMP = 807;

    private OemRil mOemRil;
    private ProgressDialog mLoadingDialog;
    private AlertDialog.Builder mbuilder;

    public Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case EVENT_AP_DUMP_DONE :
                mLoadingDialog.dismiss();
                displayDumpResult((String)msg.obj);
                break;
            case EVEMT_COPY_TO_STORAGE_DONE :
                mLoadingDialog.dismiss();
                printMessage("Copy To Storage Result", (String)msg.obj);
                break;
            case EVENT_RIL_CONNECTED_FOR_CP_DUMP:
                Log.d(TAG, "RIL connected for CP_DUMP");
                doCPDump();
                break;
            case EVENT_RIL_DISCONNECTED:
                Log.d(TAG, "RIL disconnected");
                finish();
                break;
            case EVENT_SYSTEM_MODEM_DUMP_DONE:
                if (ar.exception == null) {
                    Log.d(TAG, "Success to SYSTEM_MODEM_DUMP");
                } else {
                    Log.d(TAG, "Fail: " + msg.what);
                }
                disconnectToOemRilService();
                break;
            case EVENT_RIL_CONNECTED_FOR_TS25_TABLE_DUMP:
                Log.d(TAG, "RIL connected for TS25_TABLE_DUMP");
                doTs25TableDump();
                break;
            case EVENT_TS25_TABLE_DUMP_DONE:
                if (ar.exception == null) {
                    Log.d(TAG, "Success to TS25_TABLE_DUMP");
                    Toast.makeText(getApplicationContext(), "Success TS.25 table dump", Toast.LENGTH_LONG).show();
                } else {
                    Log.d(TAG, "Fail: " + msg.what);
                    Toast.makeText(getApplicationContext(), "Fail TS.25 table dump", Toast.LENGTH_LONG).show();
                }
                disconnectToOemRilService();
                break;
            default:
                break;
            }
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ArrayList<String> menuList = new ArrayList<String>();
        menuList.add("[0] Dump AP Log(dumpstate & logcat & dmesg)");
        menuList.add("[1] Forced CP Crash Dump");
        menuList.add("[2] Delete Log Files("+DUMP_BASE_DIR+")");
        menuList.add("[3] Copy to Storage");
        menuList.add("[4] Set Crash Handling Mode");
        menuList.add("[5] Dump TS.25 table");

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, menuList);
        ListView list = (ListView)findViewById(R.id.list);
        list.setAdapter(adapter);
        list.setChoiceMode(ListView.CHOICE_MODE_SINGLE);

        list.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
                selectMenuItem(position);
            }
        });

        registerReceiver(mBroadcastReceiver, mIntentFilter);
    }

    private void selectMenuItem(int position) {
        switch (position) {
        case AP_DUMP:
            Log.i(TAG, "Start to dump AP log...");
            doAPLogDump();
            break;
        case CP_DUMP:
            Log.i(TAG, "Start to dump CP log...");
            connectToOemRilService(EVENT_RIL_CONNECTED_FOR_CP_DUMP);
            break;
        case DELETION_LOG_FILES:
            Log.i(TAG, "Start to delete log files...");
            printMessage("Deletion Result", deleteDumpFile(DUMP_BASE_DIR));
            break;
        case COPY_TO_STORAGE:
            Log.i(TAG, "Start to copy log files to SD Card...");
            copyToStorage();
            break;
        case CRASH_HANDLING_MODE:
            Log.i(TAG, "Start to select crash handling mode...");
            crashHandlingMode();
            break;
        case TS25_TABLE_DUMP:
            Log.i(TAG, "Start to dump TS.25 table...");
            connectToOemRilService(EVENT_RIL_CONNECTED_FOR_TS25_TABLE_DUMP);
            break;
        default:
            Log.w(TAG, "Position " + position + " is Restricted!");
            return;
        }
    }

    private final IntentFilter mIntentFilter = new IntentFilter("com.samsung.slsi.telephony.action.SILENT_RESET");
    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals("com.samsung.slsi.telephony.action.SILENT_RESET") && (mLoadingDialog != null)) {
                Toast.makeText(context, "Silent Reset", Toast.LENGTH_SHORT).show();
                mLoadingDialog.dismiss();
            }
        }
    };

    private void doAPLogDump() {
        showProgressDialog("Dumping now. Please wait...");

        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {

                File dir = new File(DUMP_BASE_DIR);
                if (!dir.exists()) {
                    dir.mkdir();
                }

                SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
                String currTime = dateFormat.format(new Date()).toString();

                runShellCMD("dumpstate > " + DUMPSTATE_BASE_FILE_NAME + currTime + ".log");
                runShellCMD("logcat -b main -d -f " + LOGCAT_MAIN_BASE_FILE_NAME + currTime + ".log");
                runShellCMD("logcat -b radio -d -f " + LOGCAT_RADIO_BASE_FILE_NAME + currTime + ".log");
                runShellCMD("dmesg > " + DMESG_BASE_FILE_NAME + currTime + ".log");
                Message msg = mHandler.obtainMessage();
                msg.what = EVENT_AP_DUMP_DONE;
                msg.obj = currTime+".log";
                mHandler.sendMessage(msg);
            }
        });
        thread.start();
    }

    private void runShellCMD(String cmd) {
        String[] shellCmd = {"/system/bin/sh", "-c", cmd};
        Log.i(TAG, "Execute shell command >> " + cmd);

        try {
            Process proc = Runtime.getRuntime().exec(shellCmd);
            proc.waitFor();
        } catch (IOException e) {
            Log.i(TAG, "runShellCMD() IOException" + e);
        } catch (SecurityException e) {
            Log.i(TAG, "runShellCMD() SecurityException" + e);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void displayDumpResult(String tail) {
        File dumpstateFile = new File(DUMPSTATE_BASE_FILE_NAME + tail);
        File mainFile = new File(LOGCAT_MAIN_BASE_FILE_NAME + tail);
        File radioFile = new File(LOGCAT_RADIO_BASE_FILE_NAME + tail);
        File dmesgFile = new File(DMESG_BASE_FILE_NAME + tail);

        String resultMsg = DUMP_BASE_DIR+" :\n";
        if (dumpstateFile.exists()) {
            resultMsg += dumpstateFile.getPath().substring(DUMP_BASE_DIR.length()+1) +"(" + dumpstateFile.length() / 1024 + "Kb) was saved.\n";
        } else {
            resultMsg += "dumpstate failed!\n";
        }
        if (mainFile.exists()) {
            resultMsg += mainFile.getPath().substring(DUMP_BASE_DIR.length()+1) +"(" + mainFile.length() / 1024 + "Kb) was saved.\n";
        } else {
            resultMsg += "logcat main failed!\n";
        }

        if (radioFile.exists()) {
            resultMsg += radioFile.getPath().substring(DUMP_BASE_DIR.length()+1) +"(" + radioFile.length() / 1024 + "Kb) was saved.\n";
        } else {
            resultMsg += "logcat radio failed!";
        }

        if (dmesgFile.exists()) {
            resultMsg += dmesgFile.getPath().substring(DUMP_BASE_DIR.length()+1) +"(" + dmesgFile.length() / 1024 + "Kb) was saved.";
        } else {
            resultMsg += "dmesg failed!";
        }
        printMessage("Dump Result", resultMsg);
    }

    private void printMessage(String title, String msg) {
        if(mbuilder == null)
            mbuilder = new AlertDialog.Builder(this);

        mbuilder.setIcon(android.R.drawable.ic_dialog_alert);
        mbuilder.setTitle(title);
        mbuilder.setMessage(msg);
        mbuilder.setPositiveButton(android.R.string.ok, null);
        mbuilder.setCancelable(true);

        AlertDialog dialog = mbuilder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        dialog.show();
    }

    private void showProgressDialog(String msg) {
        if (mLoadingDialog == null) {
            mLoadingDialog = new ProgressDialog(this);
        }
        mLoadingDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mLoadingDialog.setMessage(msg);
        mLoadingDialog.show();
    }

    private void connectToOemRilService(int event) {
        mOemRil = OemRil.init(getApplicationContext(), 0); // default SIM1
        if (mOemRil == null) {
            Log.d(TAG, "connectToOemRilService mOemRil is null");
        } else {
            mOemRil.registerForOemRilConnected(mHandler, event);
            mOemRil.registerForOemRilDisconnected(mHandler, EVENT_RIL_DISCONNECTED);
        }
    }

    private void disconnectToOemRilService() {
        Log.d(TAG, "disconnectToOemRilService()");
        if (mOemRil != null) {
            mOemRil.unregisterForOemRilConnected(mHandler);
            mOemRil.unregisterForOemRilDisconnected(mHandler);
            mOemRil.detach();
        }
    }

    private void doCPDump() {
        Log.d(TAG, "doCPDump()");
        if (mOemRil != null) {
            mOemRil.invokeRequestRaw(RILC_REQ_SYSTEM_MODEM_DUMP, null, mHandler.obtainMessage(EVENT_SYSTEM_MODEM_DUMP_DONE));
        }
    }

    private String deleteDumpFile(String path) {
        File dir = null;
        String[] fileList = null;
        String result = "";

        try {
            dir = new File(path);
            fileList = dir.list();
            if (fileList != null && fileList.length > 0) {
                for (int i = 0; i < fileList.length; i++) {
                    String filename = fileList[i];
                    File f = new File(path + File.separator + filename);

                    if (f.isDirectory()) {
                        result += deleteDumpFile(path + File.separator + f.getName());
                    }
                    if (f.exists()) {
                        result += f.getPath().substring(DUMP_BASE_DIR.length() + 1) + " was deleted.\n";
                        Log.i(TAG, "Delete file : " + f.getName());
                        f.delete();
                    }
                }
            } else {
                result += "No file in "
                        + dir.getPath().substring(DUMP_BASE_DIR.length())
                        + " folder.\n";
            }
            if (!path.equals(DUMP_BASE_DIR)) {
                result += dir.getPath().substring(DUMP_BASE_DIR.length() + 1)
                        + " was deleted.\n";
                Log.i(TAG, "Delete directory : " + dir.getName());
                dir.delete();
            }
        } catch (Exception e) {
            Log.i(TAG, "Unexpected r" + e);
        }
        return result;
    }

    private void copyToStorage() {
        LayoutInflater inflater = LayoutInflater.from(getApplicationContext());
        final View dialogView = inflater.inflate(R.layout.dialog_storage, null);
        final RadioButton dialog_rb_internal = (RadioButton) dialogView.findViewById(R.id.dialog_rb_internal);
        final RadioButton dialog_rb_sdcard = (RadioButton) dialogView.findViewById(R.id.dialog_rb_sdcard);
        final File sdcardDirectory = new File(SDCARD_DIR);
        final File dataDirectory = new File(DUMP_BASE_DIR);
        final File internalDirectory = new File(Environment.getExternalStorageDirectory().toString()+"/log");
        Log.d(TAG, "internalDirectory = " +Environment.getExternalStorageDirectory().toString()+"/log");
        Log.d(TAG, "getExternalStorageState = " +Environment.getExternalStorageState());
        AlertDialog.Builder builder;
        builder = new AlertDialog.Builder(this);
        builder.setTitle("Setting storage");
        builder.setView(dialogView);
        builder.setPositiveButton(android.R.string.ok, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub
                showProgressDialog("Copying now. Please wait...");
                Thread thread = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        // TODO Auto-generated method stub
                        String result = "";
                        if (dialog_rb_internal.isChecked()) {
                            Log.d(TAG, "sdcard storage is checked");
                            result = copyDirectory(dataDirectory, internalDirectory);
                        } else {
                            Log.d(TAG, "sdcard storage is checked");
                            result = copyDirectory(dataDirectory, sdcardDirectory);
                        }
                        Message msg = mHandler.obtainMessage();
                        msg.what = EVEMT_COPY_TO_STORAGE_DONE;
                        msg.obj = result;
                        mHandler.sendMessage(msg);
                    }
                });
                thread.start();
            }
        });
        builder.setNegativeButton(android.R.string.cancel, null);
        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        dialog.show();

        if (!Environment.getExternalStorageState(sdcardDirectory).equals(android.os.Environment.MEDIA_MOUNTED)) {
            Log.d(TAG, "getExternalStorageState : no sdcard directory");
            dialog_rb_sdcard.setClickable(false);
            dialog_rb_sdcard.setTextColor(Color.GRAY);
        }
    }

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
            return "No file in " + src.getPath().substring(DUMP_BASE_DIR.length()) + " folder.\n";

        for (int i = 0; i < fileList.length; i++)
            result += copyDirectory(new File(src, fileList[i]), new File(dst, fileList[i]));
        } else {
            try {
                if(src.exists() == false) {
                    return src.getName() + " doesn't exist!!\n";
                }
                result += src.getPath().substring(DUMP_BASE_DIR.length()+1) + " was copied.\n";
                srcFile = new FileInputStream(src);
                dstFile = new FileOutputStream(dst);
                srcChannel = srcFile.getChannel();
                dstChannel = dstFile.getChannel();
                long size = srcChannel.size();

                srcChannel.transferTo(0, size, dstChannel);
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                try {
                    if (src.exists()) {
                        srcChannel.close();
                        srcFile.close();
                        dstChannel.close();
                        dstFile.close();
                    }
                    MediaScannerConnection.scanFile(getApplicationContext(), new String[] { dst.toString() }, null, new MediaScannerConnection.OnScanCompletedListener() {
                        @Override
                        public void onScanCompleted(String path, Uri uri) {
                            // TODO Auto-generated method stub
                            Log.i(TAG, "Scanned " + path + ":");
                            Log.i(TAG, "-> uri=" +uri);
                        }
                    });
                } catch(IOException e) {
                    e.printStackTrace();
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }
        }
        return result;
    }

    private void crashHandlingMode() {
        final String[] modes = new String[]{"crash dump & kernel panic", "crash dump & silent reset", "silent reset only"};
        final int[] mode = {0};
        String property = String.valueOf(SystemProperties.get(PROPERTY_CRASH_MODE));
        if (!property.isEmpty()) {
            mode[0] = Integer.parseInt(property);
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Select Crash Handling Mode");
        builder.setCancelable(false);
        builder.setSingleChoiceItems(modes, mode[0], new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                mode[0] = which;
            }
        }).setPositiveButton("OK", new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                SystemProperties.set(PROPERTY_CRASH_MODE, mode[0]+"");
            }
        });

        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        dialog.show();
    }

    private void doTs25TableDump() {
        Log.d(TAG, "doTs25TableDump()");
        if (mOemRil != null) {
            mOemRil.invokeRequestRaw(RILC_REQ_TS25_TABLE_DUMP, null, mHandler.obtainMessage(EVENT_TS25_TABLE_DUMP_DONE));
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        disconnectToOemRilService();
    }
}