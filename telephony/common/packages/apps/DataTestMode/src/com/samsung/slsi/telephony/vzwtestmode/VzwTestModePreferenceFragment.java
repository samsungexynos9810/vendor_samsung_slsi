/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.vzwtestmode;

import java.io.IOException;

import android.os.AsyncResult;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.util.Log;
import android.view.WindowManager;

import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.io.DataWriter;

import com.samsung.slsi.telephony.datatestmode.R;

public class VzwTestModePreferenceFragment extends PreferenceFragment implements Preference.OnPreferenceChangeListener {


    private static final String TAG = "VzwTestModeActivity";
    private static final String PROPERTY_VZW_TEST_MODE = "persist.vendor.config.vzwtestmode";
    private SwitchPreference mVzwTestModePref;
    private OemRil mOemRil;
    private static int mPhoneId = -1;
    private static final int TEST_MODE_DISABLE = 0;
    private static final int TEST_MODE_ENABLE = 1;

    /* Handle Message */
    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_SET_VZW_TEST_MODE=102;

    /* RIL Request */
    //private static final int RILC_REQ_MISC_SET_PREFERRED_CALL_CAPA = 20;
    //private static final int RILC_REQ_MISC_GET_PREFERRED_CALL_CAPA = 21;
    private static final int RILC_REQ_SET_IMS_TEST_MODE=38;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;

            switch (msg.what) {
            case EVENT_RIL_CONNECTED:
                Log.d(TAG, "RIL connected");
                break;
            case EVENT_RIL_DISCONNECTED:
                Log.d(TAG, "RIL disconnected");
                getActivity().finish();
                break;
            case EVENT_SET_VZW_TEST_MODE:
                if (ar.exception == null)
                    Log.d(TAG, "Success to SET_VZW_TEST_MODE");
                else
                    Log.d(TAG, "Fail: " + msg.what);
                break;
            default:
                Log.d(TAG, "Unknown command");
                break;
            }
        }
    };


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.preference);

        mVzwTestModePref = new SwitchPreference(getContext());
        if (mVzwTestModePref != null) {
            mVzwTestModePref.setTitle("Vzw Test Mode Setting");
            mVzwTestModePref.setKey("key_vzwtest_mode");
            mVzwTestModePref.setOnPreferenceChangeListener(this);
        }
        fillList();
        connectToOemRilService();
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

    private void fillList() {
        PreferenceScreen root = (PreferenceScreen) findPreference("key_pref_screen");

        if (root != null) {
            root.removeAll();
            root.addPreference(mVzwTestModePref);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updateCurrentMode();
    }

    private void updateCurrentMode() {
        String mode = String.valueOf(SystemProperties.get(PROPERTY_VZW_TEST_MODE, "0"));
        if (mode.equals("1")) {
            mVzwTestModePref.setChecked(true);
            mVzwTestModePref.setSummary("on");
        } else {
            mVzwTestModePref.setChecked(false);
            mVzwTestModePref.setSummary("off");
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "Changed to " + newValue);
        if (preference.getKey().equals("key_vzwtest_mode")) {
            if (newValue.toString().equals("true")) {
                preference.setSummary("on");
                SystemProperties.set(PROPERTY_VZW_TEST_MODE, "1");
                sendVzwTestMode(TEST_MODE_ENABLE);
            } else {
                preference.setSummary("off");
                SystemProperties.set(PROPERTY_VZW_TEST_MODE, "0");
                sendVzwTestMode(TEST_MODE_DISABLE);
            }
            return true;
        }
        return false;
    }

    private void connectToOemRilService() {
        mOemRil = OemRil.init(getActivity(), mPhoneId); //send this for sub
        if (mOemRil == null) {
            Log.d(TAG, "connectToOemRilService mOemRil is null");
        } else {
            mOemRil.registerForOemRilConnected(mHandler, EVENT_RIL_CONNECTED);
            mOemRil.registerForOemRilDisconnected(mHandler, EVENT_RIL_DISCONNECTED);
        }
    }

    private void sendVzwTestMode(int mode) {

        DataWriter dr = new DataWriter();
        try {
            dr.writeByte((byte)mode);
        } catch (IOException e) {
            Log.i(TAG, "SendData() IOException" + e);
        }
        mOemRil.invokeRequestRaw(RILC_REQ_SET_IMS_TEST_MODE, dr.toByteArray(), mHandler.obtainMessage(EVENT_SET_VZW_TEST_MODE));
        //mOemRil.invokeRequestRaw(RILC_REQ_MISC_SET_PREFERRED_CALL_CAPA, dr.toByteArray(), mHandler.obtainMessage(EVENT_SET_VZW_TEST_MODE));
        //mOemRil.invokeRequestRaw(RILC_REQ_MISC_GET_PREFERRED_CALL_CAPA, dr.toByteArray(), mHandler.obtainMessage(EVENT_SET_VZW_TEST_MODE));
        Log.d(TAG, "sendVzwTestMode : " + mode);
    }

    public static VzwTestModePreferenceFragment newInstance(int phoneId) {
        Log.d(TAG, "phoneId: " + phoneId);
        VzwTestModePreferenceFragment frag = new VzwTestModePreferenceFragment();
        mPhoneId = phoneId;
        return frag;
    }


}

