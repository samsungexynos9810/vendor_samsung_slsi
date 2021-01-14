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

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.StatFs;
import android.os.SystemProperties;
import android.text.format.Formatter;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.Switch;

public class SilentLoggingActivity extends Activity {

    private static final String TAG = "SilentLoggingActivity";
    Switch switchOnOff;
    CheckBox apCheckbox;
    CheckBox cpCheckbox;
    CheckBox tcpCheckbox;
    Button btnCopy;
    Button btnDelete;
    Button btnSnapshot;
    private NotificationManager nm;
    private Notification.Builder mBuilder;
    private Notification noti;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_silent_logging);

        SilentLoggingConstant.mStartCommand = SilentLoggingConstant.DM_START;

        //Initialize UI
        initUI();
        initNotification();
        SilentLoggingControlInterface.getInstance().setContext(this);
        SilentLoggingProfile.getInstance().setContext(this);
        SilentLoggingControlInterface.getInstance().initialize();
    }

    private void initUI() {

        switchOnOff = (Switch) findViewById(R.id.switchOnOff);
        apCheckbox = (CheckBox) findViewById(R.id.apCheckbox);
        cpCheckbox = (CheckBox) findViewById(R.id.cpCheckbox);
        tcpCheckbox = (CheckBox) findViewById(R.id.tcpCheckbox);
        btnCopy = (Button) findViewById(R.id.btnCopy);
        btnDelete = (Button) findViewById(R.id.btnDelete);
        btnSnapshot = (Button) findViewById(R.id.btnSnapshot);

        String prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_MODE));
        if (prop.equalsIgnoreCase("On")) {
            switchOnOff.setChecked(true);
            btnCopy.setEnabled(false);

            prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_APLOG_MODE));
            if (prop.equalsIgnoreCase("On")) {
                apCheckbox.setChecked(true);
            } else {
                apCheckbox.setChecked(false);
            }

            prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_CPLOG_MODE));
            if (prop.equalsIgnoreCase("On")) {
                cpCheckbox.setChecked(true);
            } else {
                cpCheckbox.setChecked(false);
            }

            prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_TCPLOG_MODE));
            if (prop.equalsIgnoreCase("On")) {
                tcpCheckbox.setChecked(true);
            } else {
                tcpCheckbox.setChecked(false);
            }
            //disable log type settings
            disableUI();
        } else {
            switchOnOff.setChecked(false);

            prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_APLOG_MODE));
            if (prop.equalsIgnoreCase("On")) {
                apCheckbox.setChecked(true);
            } else {
                apCheckbox.setChecked(false);
            }

            prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_CPLOG_MODE));
            if (prop.equalsIgnoreCase("On")) {
                cpCheckbox.setChecked(true);
            } else {
                cpCheckbox.setChecked(false);
            }

            prop = String.valueOf(SystemProperties.get(SilentLoggingConstant.PROPERTY_TCPLOG_MODE));
            if (prop.equalsIgnoreCase("On")) {
                tcpCheckbox.setChecked(true);
            } else {
                tcpCheckbox.setChecked(false);
            }
            //enable log type settings
            enableUI();
        }

        switchOnOff.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    Log.d(TAG, "SwitchOnOff : isChecked(On)");
                    if (SilentLoggingControlInterface.getInstance().checkCpMode()) {
                        SilentLoggingControlInterface.getInstance().waitdialog();
                    }
                    SystemProperties.set(SilentLoggingConstant.PROPERTY_SLOG_MODE, "On");
                    disableUI();
                    new Thread() {
                        @Override
                        public void run() {
                            SilentLoggingControlInterface.getInstance().startSilentLogging();
                        }
                    }.start();
                    mBuilder.setContentTitle("Silent Logging On");
                    String logtype = "Log type : ";
                    if (SilentLoggingControlInterface.getInstance().checkApMode()) {
                        logtype += "AP ";
                    }
                    if (SilentLoggingControlInterface.getInstance().checkCpMode()) {
                        logtype += "CP ";
                    }
                    if (SilentLoggingControlInterface.getInstance().checkTcpMode()) {
                        logtype += "TCP ";
                    }
                    mBuilder.setContentText(logtype);
                    mBuilder.setPriority(Notification.PRIORITY_MAX);
                    noti = mBuilder.build();
                    nm.notify(1234, noti);
                } else {
                    Log.d(TAG, "SwitchOnOff : isChecked(Off)");
                    SystemProperties.set(SilentLoggingConstant.PROPERTY_SLOG_MODE, "Off");
                    SilentLoggingControlInterface.getInstance().stopSilentLogging();
                    if (SilentLoggingControlInterface.getInstance().checkCpMode()) {
                        SilentLoggingControlInterface.getInstance().showProgressDialog("Wait for Stop CP Logging");
                    }
                    enableUI();
                    mBuilder.setContentTitle("Silent Logging Off");
                    mBuilder.setContentText("Log type : ^^?");
                    mBuilder.setPriority(Notification.PRIORITY_MIN);
                    noti = mBuilder.build();
                    nm.notify(1234, noti);
                }
                invalidateOptionsMenu();    // update setting menu enable/disable
            }
        });

        apCheckbox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    Log.d(TAG, "apCheckbox (On)");
                    SystemProperties.set(SilentLoggingConstant.PROPERTY_APLOG_MODE, "On");
                } else {
                    Log.d(TAG, "apCheckbox (Off)");
                    SystemProperties.set(SilentLoggingConstant.PROPERTY_APLOG_MODE, "Off");
                }
            }
        });

        cpCheckbox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    Log.d(TAG, "cpCheckbox (On)");
                    SystemProperties.set(SilentLoggingConstant.PROPERTY_CPLOG_MODE, "On");
                } else {
                    Log.d(TAG, "cpCheckbox (Off)");
                    SystemProperties.set(SilentLoggingConstant.PROPERTY_CPLOG_MODE, "Off");
                }
            }
        });

        tcpCheckbox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    Log.d(TAG, "tcpCheckbox (On)");
                    SystemProperties.set(SilentLoggingConstant.PROPERTY_TCPLOG_MODE, "On");
                } else {
                    Log.d(TAG, "tcpCheckbox (Off)");
                    SystemProperties.set(SilentLoggingConstant.PROPERTY_TCPLOG_MODE, "Off");
                }
            }
        });

        btnCopy.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Copy Button is clicked.");
                SilentLoggingControlInterface.getInstance().copyLog();
            }
        });

        btnDelete.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "deleteLog");
                SilentLoggingControlInterface.getInstance().SearchAndDeleteSegmentFiles();
            }
        });

        btnSnapshot.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "snapshot");
                SilentLoggingControlInterface.getInstance().snapshotLog();
            }
        });

        SilentLoggingControlInterface.setContext(this);
        // set max file num
        File path = Environment.getDataDirectory();
        StatFs stat = new StatFs(path.getPath());
        Log.d(TAG, "used storage : " + formatSize(stat.getBlockSize() * ((long)stat.getBlockCount()-(long)stat.getAvailableBlocks())));
        Log.d(TAG, "available storage : " + formatSize((long)stat.getBlockSize() * (long)stat.getAvailableBlocks()));
        Log.d(TAG, "total storage : " + formatSize((long)stat.getBlockSize() * (long)stat.getBlockCount()));
    }

    private String formatSize(long size) {
        return Formatter.formatFileSize(this, size);
    }

    private void disableUI() {
        apCheckbox = (CheckBox) findViewById(R.id.apCheckbox);
        cpCheckbox = (CheckBox) findViewById(R.id.cpCheckbox);
        tcpCheckbox = (CheckBox) findViewById(R.id.tcpCheckbox);
        btnCopy = (Button) findViewById(R.id.btnCopy);
        btnDelete = (Button) findViewById(R.id.btnDelete);
        btnSnapshot = (Button) findViewById(R.id.btnSnapshot);

        apCheckbox.setEnabled(false);
        cpCheckbox.setEnabled(false);
        tcpCheckbox.setEnabled(false);
        btnCopy.setEnabled(false);
        btnDelete.setEnabled(false);
        btnSnapshot.setEnabled(true);
    }

    private void enableUI() {
        apCheckbox = (CheckBox) findViewById(R.id.apCheckbox);
        cpCheckbox = (CheckBox) findViewById(R.id.cpCheckbox);
        tcpCheckbox = (CheckBox) findViewById(R.id.tcpCheckbox);
        btnCopy = (Button) findViewById(R.id.btnCopy);
        btnDelete = (Button) findViewById(R.id.btnDelete);
        btnSnapshot = (Button) findViewById(R.id.btnSnapshot);

        apCheckbox.setEnabled(true);
        cpCheckbox.setEnabled(true);
        if (!Build.IS_ENG) {
            Log.d(TAG, "Not an ENG mode. TCP UI disabled forcibly");
        }
        tcpCheckbox.setEnabled(true && Build.IS_ENG);
        btnCopy.setEnabled(true);
        btnDelete.setEnabled(true);
        btnSnapshot.setEnabled(false);
    }

    private void initNotification() {
        // Get Notification Service
        nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, new Intent(this, SilentLoggingActivity.class), PendingIntent.FLAG_UPDATE_CURRENT);

        String channelId = "com.samsung.slsi.telephony.silentlogging";
        String channelName = "SilentLogNoti";
        int importance = NotificationManager.IMPORTANCE_LOW;
        NotificationChannel mChannel = new NotificationChannel(channelId, channelName, importance);
        mChannel.enableLights(true);
        nm.createNotificationChannel(mChannel);

        mBuilder = new Notification.Builder(getApplicationContext());
        mBuilder.setChannelId(channelId);
        mBuilder.setSmallIcon(R.drawable.logging);
        mBuilder.setTicker("Notification.Builder");

        String prop = SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_MODE);
        if(prop.equalsIgnoreCase("On")) {
            mBuilder.setContentTitle("Silent Logging On");
            String logtype = "Log type : ";
            if (SilentLoggingControlInterface.getInstance().checkApMode()) {
                logtype += "AP ";
            }
            if (SilentLoggingControlInterface.getInstance().checkCpMode()) {
                logtype += "CP ";
            }
            if (SilentLoggingControlInterface.getInstance().checkTcpMode()) {
                logtype += "TCP ";
            }
            mBuilder.setContentText(logtype);
            mBuilder.setPriority(Notification.PRIORITY_MAX);
        } else {
            mBuilder.setContentTitle("Silent Logging Off");
            String logtype = "Log type : ^^?";
            mBuilder.setContentText(logtype);
            mBuilder.setPriority(Notification.PRIORITY_MIN);
        }
        //mBuilder.setDefaults(Notification.DEFAULT_SOUND | Notification.DEFAULT_VIBRATE);
        mBuilder.setContentIntent(pendingIntent);
        mBuilder.setAutoCancel(false);
        //mBuilder.setPriority(Notification.PRIORITY_MAX);
        //mBuilder.setPriority(Notification.PRIORITY_MIN);
        mBuilder.setOngoing(true);

        noti = mBuilder.build();
        nm.notify(1234, noti);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_silent_logging, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            Intent SettingActivity = new Intent(this, SilentLoggingSettings.class);
            startActivity(SettingActivity);
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem item = menu.findItem(R.id.action_settings);
        String prop = SystemProperties.get(SilentLoggingConstant.PROPERTY_SLOG_MODE);
        if(prop.equalsIgnoreCase("On")) {
            item.setEnabled(false);
        }
        else
        {
            item.setEnabled(true);
        }
        return super.onPrepareOptionsMenu(menu);
    }
//
//    @Override
//    protected void onDestroy() {
//        Log.d(TAG, "SilentLogging onDestroy()++");
//        SilentLoggingControlInterface.getInstance().stopApTcpLogging(SilentLoggingConstant.LOG_MODE.SILENT);
//        super.onDestroy();
//    }
}
