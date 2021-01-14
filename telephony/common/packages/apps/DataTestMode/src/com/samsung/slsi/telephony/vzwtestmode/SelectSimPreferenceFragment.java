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

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.samsung.slsi.telephony.datatestmode.R;

public class SelectSimPreferenceFragment extends PreferenceFragment {

    private static final String TAG = "SelectSimPreferenceFragment";
    private static final int SIM1 = 0;
    private static final int SIM2 = 1;
    private Preference mPrefSim1, mPrefSim2;
    private String mSimOperator1, mSimOperator2;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.preference_select_sim);
    }

    @Override
    public void onResume() {
        mPrefSim1 = findPreference(VzwTestModeConstants.KEY_SIM1);
        mPrefSim2 = findPreference(VzwTestModeConstants.KEY_SIM2);
        mSimOperator1 = TelephonyManager.from(getContext()).getSimOperatorNumericForPhone(SIM1);
        mSimOperator2 = TelephonyManager.from(getContext()).getSimOperatorNumericForPhone(SIM2);
        String operatorName1 = TelephonyManager.from(getContext()).getSimOperatorNameForPhone(SIM1);
        String operatorName2 = TelephonyManager.from(getContext()).getSimOperatorNameForPhone(SIM2);
        Log.d(TAG, "sim1: " + mSimOperator1 + ", sim2: " + mSimOperator2);
        if (mSimOperator1.equals("")) {
            mPrefSim1.setEnabled(false);
            mPrefSim1.setSummary("No SIM");
        } else {
            mPrefSim1.setEnabled(true);
            mPrefSim1.setSummary(operatorName1);
        }
        if (mSimOperator2.equals("")) {
            mPrefSim2.setEnabled(false);
            mPrefSim2.setSummary("No SIM");
        } else {
            mPrefSim2.setEnabled(true);
            mPrefSim2.setSummary(operatorName2);
        }
        super.onResume();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        VzwTestModeActivity activity = (VzwTestModeActivity) getActivity();
        if (preference.getKey().equals(VzwTestModeConstants.KEY_SIM1)) {
            activity.onNextFragment(mSimOperator1, SubscriptionManager.from(getContext()).getSubId(SIM1)[0]);
        } else if (preference.getKey().equals(VzwTestModeConstants.KEY_SIM2)) {
            activity.onNextFragment(mSimOperator2, SubscriptionManager.from(getContext()).getSubId(SIM2)[0]);
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }
    public static SelectSimPreferenceFragment newInstance() {
        SelectSimPreferenceFragment frag = new SelectSimPreferenceFragment();
        return frag;
    }
}
