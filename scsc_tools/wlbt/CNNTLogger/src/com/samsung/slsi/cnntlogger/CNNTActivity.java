package com.samsung.slsi.cnntlogger;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;

import android.support.v4.content.LocalBroadcastManager;

public class CNNTActivity extends Activity implements View.OnClickListener, CompoundButton.OnCheckedChangeListener,
        RadioGroup.OnCheckedChangeListener {

    private static final int COPY_TO_STORAGE_DONE = 0;

    private final String TAG = "CNNTActivity";

    private Button mButton;

    private CheckBox mLogcat;

    private CheckBox mMxLog;

    private CheckBox mUdiLog;

    private RadioGroup mWifiBtGroup;

    private RadioGroup mBtFilterGroup;

    private EditText mDirectoryName;

    private EditText mCustomFilter;

    private LinearLayout mLayoutCustomFilter;

    private LoggingManager mLoggingManager;

    private CNNTUtils mUtils;

    private ProgressDialog progressDialog;

    private MyReceiver myReceiver;

    public class MyReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {

            String action = intent.getAction();

            switch (action) {
                case CmdDefine.UPDATE_LOGGING_OPTION: {
                    Log.d(TAG, "UPDATE_LOGGING_OPTION");
                    updateOptionStatus();
                    break;
                }
                default:
                    break;
            }
        }
    }

    public Handler mCopyResultHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {

                case COPY_TO_STORAGE_DONE: {
                    hideProgressDialog();
                    if (msg.arg1 == 1) {
                        ResultMessage(getString(R.string.copy_success) + "\n" + msg.obj);
                        MediaScannerConnection.scanFile(CNNTActivity.this, new String[]{Environment.getExternalStorageDirectory().toString() + CmdDefine.copyDir}, null, new MediaScannerConnection.OnScanCompletedListener() {
                            @Override
                            public void onScanCompleted(String path, Uri uri) {
                                // TODO Auto-generated method stub
                            }
                        });
                    } else {
                        ResultMessage(getString(R.string.copy_failure));
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

    private ConcurrentTask.ConcurrentTaskListener mConcurrentTask = new ConcurrentTask.ConcurrentTaskListener() {
        @Override
        public void taskCompleted() {
            Log.d(TAG, "ConcurrentTask taskCompleted ");
            mButton.setEnabled(true);
            mButton.setTextColor(Color.DKGRAY);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mUtils = new CNNTUtils(getApplicationContext());
        initUI();

        mLoggingManager = new LoggingManager(getApplicationContext(), true);
        mButton.setOnClickListener(this);

        myReceiver = new MyReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(CmdDefine.UPDATE_LOGGING_OPTION);
        LocalBroadcastManager.getInstance(this).registerReceiver(myReceiver, intentFilter);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");

        setPreferenceValue();
    }

    private String getVersionName() {
        String version = null;
        try {
            Process process = Runtime.getRuntime().exec("cat /proc/driver/mxman_info/mx_release");
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));

            String line = null;
            while ((line = bufferedReader.readLine()) != null) {
                version = line;
                Log.e(TAG, "version : " + version);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return version;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");

        if (myReceiver != null) {
            LocalBroadcastManager.getInstance(this).unregisterReceiver(myReceiver);
            myReceiver = null;
        }

        if (mLoggingManager != null) {
            mLoggingManager.removeInstance();
            mLoggingManager = null;
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();

        if (id == R.id.copyAllLog) {
            if (mButton.getText().equals(getString(R.string.stop_log))) {
                AlertDialog.Builder alert = new AlertDialog.Builder(CNNTActivity.this);
                alert.setTitle(R.string.error_title);
                alert.setMessage(getString(R.string.unable_to_copy));
                alert.create();
                AlertDialog dialog = alert.show();
                TextView msgView = (TextView) dialog.findViewById(android.R.id.message);
                if (msgView != null) {
                    msgView.setTextSize(12);
                    msgView.setTextColor(Color.DKGRAY);
                }
                return true;
            } else {
                copyToStorage();
            }
        } else if (id == R.id.deleteAllLog) {
            if (mButton.getText().equals(getString(R.string.stop_log))) {
                AlertDialog.Builder alert = new AlertDialog.Builder(CNNTActivity.this);
                alert.setTitle(R.string.error_title);
                alert.setMessage(getString(R.string.unable_to_delete));
                alert.create();
                AlertDialog dialog = alert.show();
                TextView msgView = (TextView) dialog.findViewById(android.R.id.message);
                if (msgView != null) {
                    msgView.setTextSize(12);
                    msgView.setTextColor(Color.DKGRAY);
                }
                return true;
            }

            AlertDialog.Builder alert = new AlertDialog.Builder(CNNTActivity.this);
            alert.setTitle(R.string.alert_title);
            alert.setMessage(R.string.alert_delete_log);
            alert.setPositiveButton(R.string.oki, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    String[] params = {CmdDefine.DeleteLog, "", "", ""};
                    try {
                        new ConcurrentTask(getApplicationContext(), null).execute(params);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });
            alert.setNegativeButton(R.string.doki, null);
            alert.create().show();

        } else if (id == R.id.settings) {
            Intent SettingActivity = new Intent(this, CNNTSetting.class);
            startActivity(SettingActivity);

        } else if (id == R.id.about) {
            String message = getString(R.string.apk_version);
            try {
                message += (" " + CNNTActivity.this.getPackageManager().getPackageInfo(CmdDefine.PACKAGE_NAME, 0).versionName);
            } catch (Exception e) {
                e.getLocalizedMessage();
            }
            message += ("\n" + getVersionName());
            message += ("\n" + CmdDefine.SYSTEM_PROPERTY_LOGGING_PATH + " : " + mUtils.getSystemLoggingPath());
            message += ("\n" + CmdDefine.SYSTEM_PROPERTY_SDCARD_PATH + " : " + mUtils.getSystemSdcardPath());

            AlertDialog.Builder alert = new AlertDialog.Builder(CNNTActivity.this);
            alert.setTitle(R.string.info_title);
            alert.setMessage(message);
            alert.create();
            AlertDialog dialog = alert.show();
            TextView msgView = (TextView) dialog.findViewById(android.R.id.message);
            if (msgView != null) {
                msgView.setTextSize(12);
                msgView.setTextColor(Color.DKGRAY);
            }
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private void initUI() {
        mDirectoryName = findViewById(R.id.editDir);
        mCustomFilter = findViewById(R.id.editFilter);

        mButton = findViewById(R.id.btnLog);
        mButton.setEnabled(false);
        mButton.setTextColor(Color.GRAY);

        mLogcat = findViewById(R.id.logcat_checkbox);
        mMxLog = findViewById(R.id.mxlog);
        mUdiLog = findViewById(R.id.udilog);

        mLogcat.setOnCheckedChangeListener(this);
        mMxLog.setOnCheckedChangeListener(this);
        mUdiLog.setOnCheckedChangeListener(this);

        mWifiBtGroup = findViewById(R.id.wifi_bt_RadioGroup);
        mWifiBtGroup.setOnCheckedChangeListener(this);
        mBtFilterGroup = findViewById(R.id.btFilter_RadioGroup);
        mBtFilterGroup.setOnCheckedChangeListener(this);

        mLayoutCustomFilter = findViewById(R.id.layoutCustomFilter);

        setPreferenceValue();

        mButton.setEnabled(true);
        mButton.setTextColor(Color.DKGRAY);
    }

    public void setPreferenceValue() {
        int isLiveMxLog = Integer.parseInt(SystemProperties.get("persist.vendor.wlbtlog.livemxlog", "0"));
        int isFileMxLog = Integer.parseInt(SystemProperties.get("persist.vendor.wlbtlog.filemxlog", "0"));
        int isUdiLog = Integer.parseInt(SystemProperties.get("persist.vendor.wlbtlog.udilog", "0"));
        Log.d(TAG, "isLiveMxLog " + isLiveMxLog + ", isFileMxLog " + isFileMxLog + ". isUdiLog " + isUdiLog);

        //check valid
        if (isLiveMxLog < 0 || isLiveMxLog > 5) {
            isLiveMxLog = 0;
        }
        if (isFileMxLog < 0 || isFileMxLog > 5) {
            isFileMxLog = 0;
        }
        if (isUdiLog < 0 || isUdiLog > 1) {
            isUdiLog = 0;
        }

        if (isLiveMxLog != 0 || isFileMxLog != 0 || isUdiLog != 0) {
            mUtils.setPreference(CmdDefine.KEY_BTN_STATUS, false);

            boolean isBtLog = (isLiveMxLog > 1 || isFileMxLog > 1);

            mUtils.setPreference(CmdDefine.KEY_IS_WIFI_LOG, !isBtLog);
            if (isBtLog) {
                int filterValue = isLiveMxLog == 0 ? isFileMxLog : isLiveMxLog;
                switch (filterValue) {
                    case CmdDefine.MXLOG_STATUS_BT_NORMAL:
                        Log.d(TAG, "bt general");
                        mUtils.setPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btNormalFilter);
                        break;
                    case CmdDefine.MXLOG_STATUS_BT_AUDIO:
                        Log.d(TAG, "bt audio");
                        mUtils.setPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btAudioFilter);
                        break;
                    default :
                        break;
                }
            } else {
                mUtils.setPreference(CmdDefine.KEY_CHECK_MXLOG, (isLiveMxLog == 1 || isFileMxLog == 1));
                mUtils.setPreference(CmdDefine.KEY_CHECK_UDILOG, (isUdiLog == 1));
            }
        }
        updateOptionStatus();
    }

    private void updateOptionStatus() {
        boolean btnStatus = mUtils.getPreference(CmdDefine.KEY_BTN_STATUS, true);
        mButton.setText(btnStatus ? getString(R.string.start_log)
                : getString(R.string.stop_log));
        mLogcat.setChecked(mUtils.getPreference(CmdDefine.KEY_CHECK_LOGCAT, true));
        mMxLog.setChecked(mUtils.getPreference(CmdDefine.KEY_CHECK_MXLOG, true));
        mUdiLog.setChecked(mUtils.getPreference(CmdDefine.KEY_CHECK_UDILOG, true));

        boolean isWifiLogEnable = mUtils.getPreference(CmdDefine.KEY_IS_WIFI_LOG, true);
        Log.e(TAG, "isWifiLogEnable : " + isWifiLogEnable);
        ((RadioButton)findViewById(R.id.wifi_radioButton)).setChecked(isWifiLogEnable);
        ((RadioButton)findViewById(R.id.bt_radioButton)).setChecked(!isWifiLogEnable);
        setEnableWifiLog(isWifiLogEnable);
        setEnableOption(btnStatus);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnLog:
                String dirName = mDirectoryName.getText().toString();
                String regEx = "^[a-zA-Z0-9_]*$";
                if (!dirName.matches(regEx)) {
                    Toast.makeText(getApplicationContext(), R.string.info_dir_name, Toast.LENGTH_LONG).show();
                    mButton.setEnabled(true);
                    mButton.setTextColor(Color.DKGRAY);
                    break;
                }
                String logtype = "";

                if (mButton.getText().equals(getString(R.string.start_log))) {
                    if (mUtils.getPreference(CmdDefine.KEY_CHECK_LOGCAT, true)) {
                        logtype += "logcat ";
                    }
                    if (mUtils.getPreference(CmdDefine.KEY_IS_WIFI_LOG, true)) {
                        if (mUtils.getPreference(CmdDefine.KEY_CHECK_UDILOG, true)) {
                            logtype += "udilog ";
                        }
                        if (mUtils.getPreference(CmdDefine.KEY_CHECK_MXLOG, true)) {
                            logtype += "mxlog ";
                        }

                        start(CmdDefine.StartScript, logtype, dirName);
                    } else {
                        //Bt Log
                        String filter = mUtils.getPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btNormalFilter);
                        logtype += filter;
                        if (filter.equals(CmdDefine.btCustomFilter)) {
                            String customFilter = mCustomFilter.getText().toString();
                            if (customFilter.length() == 10) {
                                logtype += customFilter;
                            } else {
                                Toast.makeText(this, "Please provide proper filter value (ex, 0x00824007)", Toast.LENGTH_LONG).show();
                                break;
                            }
                        }

                        BluetoothAdapter adapter = ((BluetoothManager)getSystemService(
                                    Context.BLUETOOTH_SERVICE)).getAdapter();
                        if (adapter.getState() != BluetoothAdapter.STATE_ON) {
                            Toast.makeText(this,
                                    "SCO Dump, Link Layer Log will not be available - Turn on BT!",
                                            Toast.LENGTH_LONG).show();
                        }

                        start(CmdDefine.loggingStart, logtype, dirName);
                        Log.e(TAG, "filter : " + filter);
                    }

                    mUtils.setPreference(CmdDefine.KEY_BTN_STATUS, false);
                    mButton.setText(getString(R.string.stop_log));
                    mButton.setEnabled(false);
                    mButton.setTextColor(Color.GRAY);

                    Log.e(TAG, "logtype : " + logtype);
                } else {
                    mUtils.setPreference(CmdDefine.KEY_BTN_STATUS, true);
                    mButton.setText(getString(R.string.start_log));
                    if (mUtils.getPreference(CmdDefine.KEY_IS_WIFI_LOG, true)) {
                        stop(CmdDefine.StopScript, logtype, dirName);
                    } else {
                        stop(CmdDefine.loggingStop, logtype, dirName);
                    }
                }
                break;
            default :
                break;
        }
    }

    private void start(String cmd, String logtype, String dirName) {
        String[] params = {cmd, mUtils.getCurrentTimeString(), logtype, dirName};
        try {
            setEnableOption(false);
            new ConcurrentTask(getApplicationContext(), mConcurrentTask).execute(params);
            mLoggingManager.updateLoggingNotification(true, logtype);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void stop(String cmd, String logtype, String dirName) {
        String[] params = {cmd, mUtils.getCurrentTimeString(), logtype, dirName};
        try {
            setEnableOption(true);
            new ConcurrentTask(getApplicationContext(), mConcurrentTask).execute(params);
            mLoggingManager.updateLoggingNotification(false, "");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void setBtLoggingFilter(String type) {
        Log.d(TAG, "setBtLoggingFilter : " + type);

        switch (type) {
            case CmdDefine.btNormalFilter:
                mUtils.setPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btNormalFilter);
                ((RadioButton)findViewById(R.id.bt_normal)).setChecked(true);
                break;
            case CmdDefine.btAudioFilter:
                mUtils.setPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btAudioFilter);
                ((RadioButton)findViewById(R.id.bt_audio)).setChecked(true);
                break;
            case CmdDefine.btCustomFilter:
                mUtils.setPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btCustomFilter);
                ((RadioButton)findViewById(R.id.bt_custom)).setChecked(true);
                break;
            default:
                mUtils.setPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btNormalFilter);
                ((RadioButton)findViewById(R.id.bt_normal)).setChecked(true);
                break;
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

        switch (buttonView.getId()) {
            case R.id.logcat_checkbox:
                Log.d(TAG, "logcat check: " + isChecked);
                mUtils.setPreference(CmdDefine.KEY_CHECK_LOGCAT, isChecked);
                break;
            case R.id.mxlog:
                Log.d(TAG, "mMxLog check: " + isChecked);
                mUtils.setPreference(CmdDefine.KEY_CHECK_MXLOG, isChecked);
                break;
            case R.id.udilog:
                Log.d(TAG, "udilog check: " + isChecked);
                mUtils.setPreference(CmdDefine.KEY_CHECK_UDILOG, isChecked);
                break;
            default:
                break;
        }
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        switch (checkedId) {
            case R.id.wifi_radioButton:
                Log.d(TAG, "wifi enable");
                mUtils.setPreference(CmdDefine.KEY_IS_WIFI_LOG, true);
                setEnableWifiLog(true);
                break;
            case R.id.bt_radioButton:
                Log.d(TAG, "bt enable");
                mUtils.setPreference(CmdDefine.KEY_IS_WIFI_LOG, false);
                setEnableWifiLog(false);
                break;
            case R.id.bt_normal:
                Log.d(TAG, "bt general");
                mUtils.setPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btNormalFilter);
                mLayoutCustomFilter.setVisibility(View.INVISIBLE);
                break;
            case R.id.bt_audio:
                Log.d(TAG, "bt audio");
                mUtils.setPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btAudioFilter);
                mLayoutCustomFilter.setVisibility(View.INVISIBLE);
                break;
            case R.id.bt_custom:
                Log.d(TAG, "bt custom");
                mUtils.setPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btCustomFilter);
                mLayoutCustomFilter.setVisibility(View.VISIBLE);
                break;
            default :
                break;
        }
    }

    private void setEnableWifiLog(boolean isWifiEnable) {

        mUdiLog.setVisibility(isWifiEnable? View.VISIBLE : View.GONE);
        mMxLog.setVisibility(isWifiEnable? View.VISIBLE : View.GONE);
        mBtFilterGroup.setVisibility(isWifiEnable? View.GONE : View.VISIBLE);
        if (!isWifiEnable) {
            String filter = mUtils.getPreference(CmdDefine.KEY_BT_FILTER, CmdDefine.btNormalFilter);
            setBtLoggingFilter(filter);
        }
    }

    private void setEnableOption(boolean isEnable) {
        mDirectoryName.setEnabled(isEnable);
        if (isEnable) {
            mDirectoryName.setText("");
        }

        mLogcat.setEnabled(isEnable);
        mMxLog.setEnabled(isEnable);
        mUdiLog.setEnabled(isEnable);

        for (int i = 0; i < mWifiBtGroup.getChildCount(); i++) {
            mWifiBtGroup.getChildAt(i).setEnabled(isEnable);
        }

        for (int i = 0; i < mBtFilterGroup.getChildCount(); i++) {
            mBtFilterGroup.getChildAt(i).setEnabled(isEnable);
        }

        for (int i = 0; i < mLayoutCustomFilter.getChildCount(); i++) {
            mLayoutCustomFilter.getChildAt(i).setEnabled(isEnable);
        }
    }

    public void ResultMessage(String message) {
        AlertDialog.Builder alert_builder = new AlertDialog.Builder(this);
        alert_builder
                .setMessage(message)
                .setCancelable(false)
                .setPositiveButton(android.R.string.yes,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int id) {
                                dialog.dismiss();
                            }
                        });

        AlertDialog alert_dialog = alert_builder.create();
        alert_dialog.setTitle("Result");
        alert_dialog.show();
    }

    private void copyToStorage() {

        LayoutInflater inflater = LayoutInflater.from(CNNTActivity.this);
        final View dialogView = inflater.inflate(R.layout.dialog_storage, null);
        final RadioButton dialog_rb_internal = (RadioButton) dialogView.findViewById(R.id.dialog_rb_internal);
        final RadioButton dialog_rb_sdcard = (RadioButton) dialogView.findViewById(R.id.dialog_rb_sdcard);
        final File sdcardDirectory = new File(CmdDefine.SDCARD_DIR);
        final File dataDirectory = new File(CmdDefine.dirPath); //CmdDefine.logDir
        final File internalDirectory = new File(Environment.getExternalStorageDirectory().toString() + CmdDefine.copyDir);
        AlertDialog.Builder mBuilder;
        mBuilder = new AlertDialog.Builder(CNNTActivity.this);
        mBuilder.setTitle("Setting storage");
        mBuilder.setView(dialogView);
        mBuilder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub
                showProgressDialog(getString(R.string.lodding_copy));
                Thread thread = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        // TODO Auto-generated method stub
                        boolean result = true;
                        if (dialog_rb_internal.isChecked()) {
                            result = mUtils.copyFile(dataDirectory, internalDirectory);
                        } else {
                            result = mUtils.copyFile(dataDirectory, sdcardDirectory);
                        }
                        Message msg = mCopyResultHandler.obtainMessage();
                        msg.what = COPY_TO_STORAGE_DONE;
                        msg.arg1 = result ? 1 : 0;
                        msg.obj = dialog_rb_internal.isChecked() ? internalDirectory : sdcardDirectory;
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

    private void showProgressDialog(String msg) {
        if (progressDialog == null) {
            progressDialog = new ProgressDialog(CNNTActivity.this);
        }

        try {
            progressDialog.setMessage(msg);
            progressDialog.setIndeterminate(true);
            progressDialog.setCancelable(false);
            progressDialog.show();
        } catch (WindowManager.BadTokenException e) {
            Log.d(TAG, "BadTokenException: " + e);
        }
    }

    private void hideProgressDialog() {
        if (progressDialog == null) {
            return;
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
    }

}
