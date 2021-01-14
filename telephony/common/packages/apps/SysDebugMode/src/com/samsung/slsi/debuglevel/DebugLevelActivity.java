/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.debuglevel;

import java.io.IOException;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Toast;
import android.os.SystemProperties;
import android.util.Log;

import com.samsung.slsi.sysdebugmode.R;
import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.io.DataWriter;

public class DebugLevelActivity extends Activity {

    Button mBtnDebugTraceOff, mBtnDebugTraceOn;

    private Toast mToast = null;
    private static final String PROPERTY_DEBUG_TRACE = "vendor.config.debug.trace";
    private static final String PROPERTY_CP_DEBUG_OFF = "persist.vendor.ril.cpdebugoff.onboot";
    private static final String TAG = "DebugLevel";
    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_SET_DEBUG_TRACE = 102;

    /* RIL request */
    private static final int RILC_REQ_MISC_SET_DEBUG_TRACE = 4;    /* Debug Trace RIL Command */
    private static final int DEBUG_TRACE_DISABLE = 0;    /* Enable Debug Trace */
    private static final int DEBUG_TRACE_ENABLE = 1;    /* Disable Debug Trace */

    private CheckBox mBtnCPDebugOff;
    private OemRil mOemRil;

    public Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_RIL_CONNECTED:
                Log.d(TAG, "RIL connected");
                break;
            case EVENT_RIL_DISCONNECTED:
                Log.d(TAG, "RIL disconnected");
                finish();
                break;
            case EVENT_SET_DEBUG_TRACE:
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Log.d(TAG, "Success to SET_DEBUG_TRACE");
                } else {
                    Log.d(TAG, "Fail: " + msg.what);
                }
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.debug_level);

        mBtnCPDebugOff = (CheckBox) findViewById(R.id.btn_cpdebugoff);
        String strCPDebugOff = String.valueOf(SystemProperties.get(PROPERTY_CP_DEBUG_OFF));
        if (!strCPDebugOff.isEmpty() && strCPDebugOff.equals("1")) {
            mBtnCPDebugOff.setChecked(true);
        }

        mBtnCPDebugOff.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) SystemProperties.set(PROPERTY_CP_DEBUG_OFF, "1");
                else SystemProperties.set(PROPERTY_CP_DEBUG_OFF, "0");
            }
        });

        mBtnDebugTraceOff = (Button) findViewById(R.id.btn_debug_off);
        mBtnDebugTraceOff.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Changed to Debug Trace OFF");
                SystemProperties.set(PROPERTY_DEBUG_TRACE, "OFF");
                setDebugTrace(DEBUG_TRACE_DISABLE);
                showToastMessage("Debug Trace OFF");
            }
        });

        mBtnDebugTraceOn = (Button) findViewById(R.id.btn_debug_on);
        mBtnDebugTraceOn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Changed to Debug Trace ON");
                SystemProperties.set(PROPERTY_DEBUG_TRACE, "ON");
                setDebugTrace(DEBUG_TRACE_ENABLE);
                showToastMessage("Debug Trace ON");
            }
        });

        connectToOemRilService();
    }

    public void showToastMessage(String str) {
        if (mToast == null)
            mToast = Toast.makeText(this, str, Toast.LENGTH_SHORT);
        else
            mToast.setText(str);
        mToast.show();
    }

    private void setDebugTrace(int status) {
        Log.d(TAG, "setDebugTrace() status: " + status);
        DataWriter dr = new DataWriter();
        try {
            dr.writeByte((byte)status);
            mOemRil.invokeRequestRaw(RILC_REQ_MISC_SET_DEBUG_TRACE, dr.toByteArray(), mHandler.obtainMessage(EVENT_SET_DEBUG_TRACE));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void connectToOemRilService() {
        mOemRil = OemRil.init(getApplicationContext(), 0); // default SIM1
        if (mOemRil == null) {
            Log.d(TAG, "connectToOemRilService mOemRil is null");
        } else {
            mOemRil.registerForOemRilConnected(mHandler, EVENT_RIL_CONNECTED);
            mOemRil.registerForOemRilDisconnected(mHandler, EVENT_RIL_DISCONNECTED);
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mOemRil != null) {
            mOemRil.unregisterForOemRilConnected(mHandler);
            mOemRil.unregisterForOemRilDisconnected(mHandler);
            mOemRil.detach();
        }
    }
}
