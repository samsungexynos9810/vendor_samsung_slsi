/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.sgc;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.util.Log;
import android.view.WindowManager;

import com.samsung.slsi.telephony.testmode.R;

public class SgcTestPreferenceFragment extends PreferenceFragment implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "SgcTest";
    private static final String PROPERTY_SGC_TEST_CONFIG = "persist.vendor.radio.sgc";
    private ListPreference mSgcGlobalListPref, mSgcLatamListPref, mSgcNaListPref;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.preference_sgc);

        mSgcGlobalListPref = (ListPreference) findPreference("key_sgc_global");
        mSgcGlobalListPref.setOnPreferenceChangeListener(this);
        mSgcLatamListPref = (ListPreference) findPreference("key_sgc_latam_specific");
        mSgcLatamListPref.setOnPreferenceChangeListener(this);
        mSgcNaListPref = (ListPreference) findPreference("key_sgc_na");
        mSgcNaListPref.setOnPreferenceChangeListener(this);
    }
    public static SgcTestPreferenceFragment newInstance() {
        SgcTestPreferenceFragment frag = new SgcTestPreferenceFragment();
        return frag;
    }

    @Override
    public void onResume() {
        super.onResume();
        updateCurrentMode();
    }

    private void updateCurrentMode() {
        String mode = String.valueOf(SystemProperties.get(PROPERTY_SGC_TEST_CONFIG, ""));
        if (mode.equals("")) {
            mSgcGlobalListPref.setSummary("");
            mSgcLatamListPref.setSummary("");
            mSgcNaListPref.setSummary("");
        } else {
            int pos = mSgcGlobalListPref.findIndexOfValue(mode);
            if (pos >= 0 ) {
                mSgcGlobalListPref.setSummary(mSgcGlobalListPref.getEntries()[pos]);
                mSgcGlobalListPref.setValueIndex(pos);
            } else {
                mSgcGlobalListPref.setSummary("");
                mSgcGlobalListPref.setValueIndex(0);
            }

            pos = mSgcLatamListPref.findIndexOfValue(mode);
            if (pos >= 0 ) {
                mSgcLatamListPref.setSummary(mSgcLatamListPref.getEntries()[pos]);
                mSgcLatamListPref.setValueIndex(pos);
            } else {
                mSgcLatamListPref.setSummary("");
                mSgcLatamListPref.setValueIndex(0);
            }

            pos = mSgcNaListPref.findIndexOfValue(mode);
            if (pos >= 0 ) {
                mSgcNaListPref.setSummary(mSgcNaListPref.getEntries()[pos]);
                mSgcNaListPref.setValueIndex(pos);
            } else {
                mSgcNaListPref.setSummary("");
                mSgcNaListPref.setValueIndex(0);
            }
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "Changed to " + newValue);
        SystemProperties.set(PROPERTY_SGC_TEST_CONFIG, newValue.toString());
        updateCurrentMode();
        reboot();
        return true;
    }

    private void reboot() {
        final PowerManager pm = (PowerManager) getContext().getSystemService(Context.POWER_SERVICE);
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setMessage("Reboot the device")
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        pm.reboot(null);
                    }
                }).setNegativeButton("Cancel", null);
        final AlertDialog resetDialog = builder.create();
        resetDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        resetDialog.show();
    }
}
