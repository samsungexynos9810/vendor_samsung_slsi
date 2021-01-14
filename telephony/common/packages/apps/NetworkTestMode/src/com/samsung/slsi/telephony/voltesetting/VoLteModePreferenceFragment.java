/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.voltesetting;

import java.io.IOException;

import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.util.Log;
import android.widget.Toast;

import com.samsung.slsi.telephony.networktestmode.R;
import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.io.DataWriter;

public class VoLteModePreferenceFragment extends PreferenceFragment implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "VoLteModePreferenceFragment";
    private ListPreference mVoLteListPref;
    private static int mPhoneId = -1;
    private int mCurMode = 0;
    private OemRil mOemRil;

    /* Handle Message */
    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_GET_PREF_CALL_CAPA = 102;
    private static final int EVENT_SET_PREF_CALL_CAPA = 103;

    /* RIL Request */
    private static final int RILC_REQ_MISC_SET_PREFERRED_CALL_CAPA = 20;
    private static final int RILC_REQ_MISC_GET_PREFERRED_CALL_CAPA = 21;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case EVENT_RIL_CONNECTED:
                Log.d(TAG, "RIL connected");
                getVoLteMode();
                break;
            case EVENT_RIL_DISCONNECTED:
                Log.d(TAG, "RIL disconnected");
                getActivity().finish();
                break;
            case EVENT_GET_PREF_CALL_CAPA:
                if (ar.exception == null) {
                    mCurMode = ((byte []) ar.result)[0];
                    mVoLteListPref.setValueIndex(mCurMode);
                    mVoLteListPref.setSummary(mVoLteListPref.getEntry());
                    Log.d(TAG, "Success to GetVoLteMode: " + mCurMode);
                } else {
                    Log.d(TAG, "Fail: " + msg.what);
                }
                break;
            case EVENT_SET_PREF_CALL_CAPA:
                if (ar.exception == null) {
                    mVoLteListPref.setSummary(mVoLteListPref.getEntry());
                    mCurMode = mVoLteListPref.findIndexOfValue(mVoLteListPref.getValue());
                    Log.d(TAG, "Success to SetVoLteMode: " + mCurMode);
                } else {
                    mVoLteListPref.setValueIndex(mCurMode);
                    mVoLteListPref.setSummary(mVoLteListPref.getEntry());
                    Log.d(TAG, "Fail: " + msg.what);
                }
                break;
            }
        }
    };


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.preference_volte_mode);

        mVoLteListPref = (ListPreference) getPreferenceScreen().findPreference("key_volte_mode");
        mVoLteListPref.setOnPreferenceChangeListener(this);
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

    private void connectToOemRilService() {
        mOemRil = OemRil.init(getActivity(), mPhoneId);
        if (mOemRil == null) {
            Log.d(TAG, "connectToOemRilService mOemRil is null");
        } else {
            mOemRil.registerForOemRilConnected(mHandler, EVENT_RIL_CONNECTED);
            mOemRil.registerForOemRilDisconnected(mHandler, EVENT_RIL_DISCONNECTED);
        }
    }


    private void setVoLteMode(int mode) {
        DataWriter dr = new DataWriter();
        try {
            dr.writeByte((byte)(0xff & mode));
        } catch (IOException e) {
            Log.i(TAG, "SendData() IOException" + e);
        }
        mOemRil.invokeRequestRaw(RILC_REQ_MISC_SET_PREFERRED_CALL_CAPA, dr.toByteArray(), mHandler.obtainMessage(EVENT_SET_PREF_CALL_CAPA));
    }

    private void getVoLteMode() {
        mOemRil.invokeRequestRaw(RILC_REQ_MISC_GET_PREFERRED_CALL_CAPA, null, mHandler.obtainMessage(EVENT_GET_PREF_CALL_CAPA));
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "Changed to " + newValue);
        int newMode = Integer.parseInt(newValue.toString());
        if (newMode != mCurMode) {
            setVoLteMode(newMode);
        }
        return true;
    }

    public static VoLteModePreferenceFragment newInstance(int phoneId) {
        Log.d(TAG, "phoneId: " + phoneId);
        VoLteModePreferenceFragment frag = new VoLteModePreferenceFragment();
        mPhoneId = phoneId;
        return frag;
    }
}
