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

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;

import com.samsung.slsi.telephony.networktestmode.R;

public class SelectSimPreferenceFragment extends PreferenceFragment {

    private static final int SIM1 = 0;
    private static final int SIM2 = 1;
    private Preference mPrefSim1, mPrefSim2;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.preference_select_sim);
    }

    @Override
    public void onResume() {
        PreferenceScreen root = (PreferenceScreen) findPreference("key_select_sim");
        mPrefSim1 = findPreference("key_sim1");
        mPrefSim2 = findPreference("key_sim2");
        String simOperator1 = TelephonyManager.from(getContext()).getSimOperatorNumericForPhone(SIM1);

        if (simOperator1.equals("")) {
            mPrefSim1.setEnabled(false);
            mPrefSim1.setSummary("No SIM");
        } else {
            mPrefSim1.setEnabled(true);
        }

        if (TelephonyManager.getDefault().isMultiSimEnabled()) {
            String simOperator2 = TelephonyManager.from(getContext()).getSimOperatorNumericForPhone(SIM2);
            if (simOperator2.equals("")) {
                mPrefSim2.setEnabled(false);
                mPrefSim2.setSummary("No SIM");
            } else {
                mPrefSim2.setEnabled(true);
            }
        } else {
            root.removePreference(mPrefSim2);
        }

        super.onResume();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        VoLteModeActivity activity = (VoLteModeActivity) getActivity();
        if (preference.equals(mPrefSim1)) {
            activity.onNextFragment(SIM1);
        } else {
            activity.onNextFragment(SIM2);
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }
    public static SelectSimPreferenceFragment newInstance() {
        SelectSimPreferenceFragment frag = new SelectSimPreferenceFragment();
        return frag;
    }
}
