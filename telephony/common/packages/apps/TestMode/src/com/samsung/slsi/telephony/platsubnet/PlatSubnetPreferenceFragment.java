/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.platsubnet;

import com.samsung.slsi.telephony.testmode.R;

import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.SwitchPreference;
import android.util.Log;

public class PlatSubnetPreferenceFragment extends PreferenceFragment implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "PlatSubnet";
    private static final String PROPERTY_PLAT_SUBNET = "persist.vendor.radio.plat_subnet";
    private SwitchPreference mPlatSubnetModePref;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.layout.plat_subnet_activity);
    }
    public static PlatSubnetPreferenceFragment newInstance() {
        PlatSubnetPreferenceFragment frag = new PlatSubnetPreferenceFragment();
        return frag;
    }

    @Override
    public void onResume() {
        super.onResume();

        mPlatSubnetModePref = (SwitchPreference) findPreference("key_plat_subnet");
        mPlatSubnetModePref.setOnPreferenceChangeListener(this);

        updateCurrentMode();
    }

    private void updateCurrentMode() {
        String mode = String.valueOf(SystemProperties.get(PROPERTY_PLAT_SUBNET, "0"));
        if (mode.equals("1")) {
            mPlatSubnetModePref.setChecked(true);
            mPlatSubnetModePref.setSummary("on");
        } else {
            mPlatSubnetModePref.setChecked(false);
            mPlatSubnetModePref.setSummary("off");
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "Changed to " + newValue);
        if (preference.getKey().equals("key_plat_subnet")) {
            if (newValue.toString().equals("true")) {
                preference.setSummary("on");
                SystemProperties.set(PROPERTY_PLAT_SUBNET, "1");
            } else {
                preference.setSummary("off");
                SystemProperties.set(PROPERTY_PLAT_SUBNET, "0");
            }
            return true;
        }
        return false;
    }
}
