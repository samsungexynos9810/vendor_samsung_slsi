/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.biptest;

import com.samsung.slsi.telephony.testmode.R;

import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.SwitchPreference;
import android.util.Log;

public class BipTestPreferenceFragment extends PreferenceFragment implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "BipTest";
    private static final String PROPERTY_BIPTEST_ENABLED = "persist.vendor.radio.biptest_enabled";
    private SwitchPreference mBipModePref;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.layout.bip_test_activity);
    }
    public static BipTestPreferenceFragment newInstance() {
        BipTestPreferenceFragment frag = new BipTestPreferenceFragment();
        return frag;
    }

    @Override
    public void onResume() {
        super.onResume();

        mBipModePref = (SwitchPreference) findPreference("key_bip_test");
        mBipModePref.setOnPreferenceChangeListener(this);

        updateCurrentMode();
    }

    private void updateCurrentMode() {
        String mode = String.valueOf(SystemProperties.get(PROPERTY_BIPTEST_ENABLED, "0"));
        if (mode.equals("1")) {
            mBipModePref.setChecked(true);
            mBipModePref.setSummary("on");
        } else {
            mBipModePref.setChecked(false);
            mBipModePref.setSummary("off");
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "Changed to " + newValue);
        if (preference.getKey().equals("key_bip_test")) {
            if (newValue.toString().equals("true")) {
                preference.setSummary("on");
                SystemProperties.set(PROPERTY_BIPTEST_ENABLED, "1");
            } else {
                preference.setSummary("off");
                SystemProperties.set(PROPERTY_BIPTEST_ENABLED, "0");
            }
            return true;
        }
        return false;
    }
}
