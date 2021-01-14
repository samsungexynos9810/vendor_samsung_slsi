/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.portforwarding;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbManager;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.samsung.slsi.telephony.testmode.R;

import android.os.SystemProperties;
import android.provider.Settings;
import android.text.TextUtils;

public class PortForwardingActivity extends Activity {

    private static final String TAG = "PortForwarding";
    private static final String PROPERTY_NSTEST_ENABLED = "vendor.config.net.nstest_enabled";
    private static final String PROPERTY_NSTEST_UDPPORT = "vendor.config.net.nstest_udpport";
    private static final String PROPERTY_NSTEST_TCPPORT = "vendor.config.net.nstest_tcpport";
    private static final String PROPERTY_NSTEST_SYNPACKETDROP = "vendor.config.net.nstest_synpacketdrop";
    private static final String PROPERTY_NSTEST_PCADDR1 = "vendor.config.net.nstest_pcaddr1";
    private static final String PROPERTY_NSTEST_PCADDR2 = "vendor.config.net.nstest_pcaddr2";
    private static final String RESULT_BASE_DIR = "/data/testscript/runresult";
    private static final int MAX_PORT_NUM = 65535;
    private static final int MIN_PORT_NUM = 1;
    private static final String DEFAULT_UDP_PORT = "5013";
    private static final String DEFAULT_TCP_PORT = "5011";

    // Types of tethering.
    public static final int TETHERING_INVALID = -1;
    public static final int TETHERING_WIFI = 0;
    public static final int TETHERING_USB = 1;
    public static final int TETHERING_BLUETOOTH = 2;

    private boolean initEnabled = false;
    private boolean initPacketDrop = false;
    private boolean intentUsbTether = true;

    private RadioGroup mRgEnable, mRgPacketDrop;
    private RadioButton mRbEnableOn, mRbEnableOff, mRbPacketDropOn,
            mRbPacketDropOff;
    private Button mBtnUdpport, mBtnTcpport, mBtnReset, mBtnResult, mBtnPcAddr1, mBtnPcAddr2;
    private CheckBox mBtnTethering;
    private EditText mEditUdpport, mEditTcpport, mEditPcAddr1, mEditPcAddr2;
    private String mUdpport = DEFAULT_UDP_PORT;
    private String mTcpport = DEFAULT_TCP_PORT;
    private String mPcAddr1, mPcAddr2;
    private InputMethodManager imm;
    private AlertDialog.Builder mbuilder;
    private boolean mUsbConnected = false;
    private static IntentFilter mIntentFilter = new IntentFilter(UsbManager.ACTION_USB_STATE);
    private static BroadcastReceiver mBroadcastReceiver = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.portforwarding_activity);

        imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        mEditUdpport = (EditText) findViewById(R.id.edit_udpport);
        mEditTcpport = (EditText) findViewById(R.id.edit_tcpport);
        mEditPcAddr1 = (EditText) findViewById(R.id.edit_pcaddr1);
        mEditPcAddr2 = (EditText) findViewById(R.id.edit_pcaddr2);

        mRbEnableOn = (RadioButton) findViewById(R.id.rb_enable_on);
        mRbEnableOff = (RadioButton) findViewById(R.id.rb_enable_off);
        mRbPacketDropOn = (RadioButton) findViewById(R.id.rb_synpacketdrop_on);
        mRbPacketDropOff = (RadioButton) findViewById(R.id.rb_synpacketdrop_off);

        mRgEnable = (RadioGroup) findViewById(R.id.rg_enable);
        mRgEnable
                .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(RadioGroup group, int checkedId) {
                        // TODO Auto-generated method stub
                        imm.hideSoftInputFromWindow(
                                mEditUdpport.getWindowToken(), 0);
                        imm.hideSoftInputFromWindow(
                                mEditTcpport.getWindowToken(), 0);
                        if (initEnabled) {
                            if (checkedId == R.id.rb_enable_off) {
                                Toast.makeText(getApplicationContext(), "Changed to Enable OFF", Toast.LENGTH_SHORT).show();
                                SystemProperties.set(PROPERTY_NSTEST_ENABLED, "0");
                            } else if (checkedId == R.id.rb_enable_on) {
                                Toast.makeText(getApplicationContext(), "Changed to Enable ON", Toast.LENGTH_SHORT).show();
                                SystemProperties.set(PROPERTY_NSTEST_ENABLED, "1");
                            }
                        } else {
                            initEnabled = true;
                        }
                    }
                });

        mRgPacketDrop = (RadioGroup) findViewById(R.id.rg_synpacketdrop);
        mRgPacketDrop
                .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(RadioGroup group, int checkedId) {
                        // TODO Auto-generated method stub
                        imm.hideSoftInputFromWindow(
                                mEditUdpport.getWindowToken(), 0);
                        imm.hideSoftInputFromWindow(
                                mEditTcpport.getWindowToken(), 0);
                        if (initPacketDrop) {
                            if (checkedId == R.id.rb_synpacketdrop_off) {
                                Toast.makeText(getApplicationContext(), "Changed to SYN packet drop OFF", Toast.LENGTH_SHORT).show();
                                SystemProperties.set(PROPERTY_NSTEST_SYNPACKETDROP, "0");
                            } else if (checkedId == R.id.rb_synpacketdrop_on) {
                                Toast.makeText(getApplicationContext(), "Changed to SYN packet drop ON", Toast.LENGTH_SHORT).show();
                                SystemProperties.set(PROPERTY_NSTEST_SYNPACKETDROP, "1");
                            }
                        } else {
                            initPacketDrop = true;
                        }
                    }
                });

        mBtnUdpport = (Button) findViewById(R.id.btn_udpport);
        mBtnUdpport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                imm.hideSoftInputFromWindow(mEditUdpport.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditTcpport.getWindowToken(), 0);
                if (checkPortSize(mEditUdpport.getText().toString())) {
                    mUdpport = mEditUdpport.getText().toString();
                    Toast.makeText(getApplicationContext(), "Changed to UDP port : " + mUdpport, Toast.LENGTH_SHORT).show();
                    SystemProperties.set(PROPERTY_NSTEST_UDPPORT, mUdpport);
                } else {
                    Toast.makeText(getApplicationContext(), "Invalid UDP Port Size", Toast.LENGTH_SHORT).show();
                    mEditUdpport.setText(mUdpport);
                }
            }
        });

        mBtnTcpport = (Button) findViewById(R.id.btn_tcpport);
        mBtnTcpport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                imm.hideSoftInputFromWindow(mEditUdpport.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditTcpport.getWindowToken(), 0);
                // TODO Auto-generated method stub
                if (checkPortSize(mEditTcpport.getText().toString())) {
                    mTcpport = mEditTcpport.getText().toString();
                    Toast.makeText(getApplicationContext(), "Changed to TCP port : " + mTcpport, Toast.LENGTH_SHORT).show();
                    SystemProperties.set(PROPERTY_NSTEST_TCPPORT, mTcpport);
                } else {
                    Toast.makeText(getApplicationContext(), "Invalid TCP Port Size", Toast.LENGTH_SHORT).show();
                    mEditTcpport.setText(mTcpport);
                }
            }
        });

        mBtnPcAddr1 = (Button) findViewById(R.id.btn_pcaddr1);
        mBtnPcAddr1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View arg0) {
                if (!TextUtils.isEmpty(mEditPcAddr1.getText().toString())) {
                    mPcAddr1 = mEditPcAddr1.getText().toString();
                    Toast.makeText(getApplicationContext(), "Changed to PC ADDR1 : " + mPcAddr1, Toast.LENGTH_SHORT).show();
                    SystemProperties.set(PROPERTY_NSTEST_PCADDR1, mPcAddr1);
                } else {
                    Toast.makeText(getApplicationContext(), "Invalid PC ADDR1", Toast.LENGTH_SHORT).show();
                    mEditPcAddr1.setText(mPcAddr1);
                }
            }
        });

        mBtnPcAddr2 = (Button) findViewById(R.id.btn_pcaddr2);
        mBtnPcAddr2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View arg0) {
                if (!TextUtils.isEmpty(mEditPcAddr2.getText().toString())) {
                    mPcAddr2 = mEditPcAddr2.getText().toString();
                    Toast.makeText(getApplicationContext(), "Changed to PC ADDR2 : " + mPcAddr2, Toast.LENGTH_SHORT).show();
                    SystemProperties.set(PROPERTY_NSTEST_PCADDR2, mPcAddr2);
                } else {
                    Toast.makeText(getApplicationContext(), "Invalid PC ADDR2", Toast.LENGTH_SHORT).show();
                    mEditPcAddr2.setText(mPcAddr2);
                }
            }
        });

        mBtnReset = (Button) findViewById(R.id.btn_reset);
        mBtnReset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                imm.hideSoftInputFromWindow(mEditUdpport.getWindowToken(), 0);
                imm.hideSoftInputFromWindow(mEditTcpport.getWindowToken(), 0);
                mRbEnableOff.setChecked(true);
                mRbPacketDropOff.setChecked(true);
                mUdpport = DEFAULT_UDP_PORT;
                mTcpport = DEFAULT_TCP_PORT;
                mEditUdpport.setText(mUdpport);
                mEditTcpport.setText(mTcpport);
                SystemProperties.set(PROPERTY_NSTEST_ENABLED, "0");
                SystemProperties.set(PROPERTY_NSTEST_SYNPACKETDROP, "0");
                SystemProperties.set(PROPERTY_NSTEST_UDPPORT, mUdpport);
                SystemProperties.set(PROPERTY_NSTEST_TCPPORT, mTcpport);

                Toast.makeText(getApplicationContext(), "Reset all properties.", Toast.LENGTH_SHORT).show();
            }
        });

        mBtnResult = (Button) findViewById(R.id.btn_result);
        mBtnResult.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                String strResult = "No Result Data.";
                try {
                    StringBuffer data = new StringBuffer();
                    File file = new File(RESULT_BASE_DIR);
                    FileInputStream fis = new FileInputStream(file);
                    BufferedReader buffer = new BufferedReader(new InputStreamReader(fis));
                    String str = buffer.readLine();
                    while (str != null) {
                        data.append(str);
                        str = buffer.readLine();
                    }
                    strResult = data.toString();
                    buffer.close();
                } catch (Exception e) {
                }
                printMessage("Result", strResult);
            }
        });

        mBtnTethering = (CheckBox) findViewById(R.id.btn_tethering);
        mBtnTethering.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                // TODO Auto-generated method stub
                ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
                if (mUsbConnected) {
                    // setting USB tethering
                    intentUsbTether = false;
                    mBtnTethering.setEnabled(false);
                    cm.setUsbTethering(isChecked);
                }
            }
        });

        mBroadcastReceiver = new BroadcastReceiver() {

            @Override
            public void onReceive(Context context, Intent intent) {
                // TODO Auto-generated method stub
                if (intent.getAction().equals(UsbManager.ACTION_USB_STATE)) {
                    mUsbConnected = intent.getBooleanExtra(UsbManager.USB_CONNECTED, false);
                    if (mUsbConnected) {
                        mBtnTethering.setEnabled(true);
                    } else {
                        mBtnTethering.setEnabled(false);
                    }
                } else if (intent.getAction().equals(ConnectivityManager.ACTION_TETHER_STATE_CHANGED)) {
                    ArrayList<String> available = intent.getStringArrayListExtra(ConnectivityManager.EXTRA_AVAILABLE_TETHER);
                    ArrayList<String> active = intent.getStringArrayListExtra(ConnectivityManager.EXTRA_ACTIVE_TETHER);
                    ArrayList<String> errored = intent.getStringArrayListExtra(ConnectivityManager.EXTRA_ERRORED_TETHER);
                    int disableTether = available.size() + active.size() + errored.size();
                    if (intentUsbTether) {
                        if (active.size() == 1 && !mBtnTethering.isChecked()) {
                            mBtnTethering.setChecked(true);
                        } else if (disableTether == 0 && mBtnTethering.isChecked()) {
                            mBtnTethering.setChecked(false);
                        }
                        intentUsbTether = true;
                    }
                    mBtnTethering.setEnabled(true);
                }
            }
        };

        mIntentFilter.addAction(ConnectivityManager.ACTION_TETHER_STATE_CHANGED);
        registerReceiver(mBroadcastReceiver, mIntentFilter);
        Initialize();
    }

    public void Initialize() {
        Log.d(TAG, "Initialize()");

        String strRgEnable = String.valueOf(SystemProperties.get(PROPERTY_NSTEST_ENABLED));
        String strPacketDrop = String.valueOf(SystemProperties.get(PROPERTY_NSTEST_SYNPACKETDROP));
        String strUdpport = String.valueOf(SystemProperties.get(PROPERTY_NSTEST_UDPPORT));
        String strTcpport = String.valueOf(SystemProperties.get(PROPERTY_NSTEST_TCPPORT));
        String strPcAddr1 = String.valueOf(SystemProperties.get(PROPERTY_NSTEST_PCADDR1));
        String strPcAddr2 = String.valueOf(SystemProperties.get(PROPERTY_NSTEST_PCADDR2));

        if (strRgEnable.isEmpty() || strRgEnable.equals("0")) {
            mRbEnableOff.setChecked(true);
        } else if (strRgEnable.equals("1")) {
            mRbEnableOn.setChecked(true);
        }

        if (strPacketDrop.isEmpty() || strPacketDrop.equals("0")) {
            mRbPacketDropOff.setChecked(true);
        } else if (strPacketDrop.equals("1")) {
            mRbPacketDropOn.setChecked(true);
        }

        if (strUdpport.isEmpty()) {
            mEditUdpport.setText(DEFAULT_UDP_PORT);
        } else {
            mEditUdpport.setText(strUdpport);
            mUdpport = strUdpport;
        }

        if (strTcpport.isEmpty()) {
            mEditTcpport.setText(DEFAULT_TCP_PORT);
        } else {
            mEditTcpport.setText(strTcpport);
            mTcpport = strTcpport;
        }

        if (!TextUtils.isEmpty(strPcAddr1)) {
            mEditPcAddr1.setText(strPcAddr1);
            mPcAddr1 = strPcAddr1;
        }

        if (!TextUtils.isEmpty(strPcAddr2)) {
            mEditPcAddr2.setText(strPcAddr2);
            mPcAddr2 = strPcAddr2;
        }
    }

    private boolean checkPortSize(String txt) {
        if (!txt.equals("")) {
            int port = Integer.parseInt(txt);
            if ((port >= MIN_PORT_NUM) && (port <= MAX_PORT_NUM))
                return true;
        }
        Log.e(TAG, "Invalid port size");
        return false;
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        intentUsbTether = true;
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();
        unregisterReceiver(mBroadcastReceiver);
    }

    private void printMessage(String title, String msg) {
        if (mbuilder == null)
            mbuilder = new AlertDialog.Builder(this);

        mbuilder.setIcon(android.R.drawable.ic_dialog_alert);
        mbuilder.setTitle(title);
        mbuilder.setMessage(msg);
        mbuilder.setPositiveButton(android.R.string.ok, null);
        mbuilder.setCancelable(true);
        mbuilder.show();
    }
}
