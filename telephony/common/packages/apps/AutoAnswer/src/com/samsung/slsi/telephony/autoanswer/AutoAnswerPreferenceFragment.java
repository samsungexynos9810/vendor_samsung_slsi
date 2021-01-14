/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.autoanswer;

import android.content.Intent;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.SwitchPreference;
import android.util.Log;

public class AutoAnswerPreferenceFragment extends PreferenceFragment implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "AutoAnswer";
    private SwitchPreference mAutoModePref;
    private ListPreference mAnswerTimePref;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.layout.activity_main);
    }
    public static AutoAnswerPreferenceFragment newInstance() {
        AutoAnswerPreferenceFragment frag = new AutoAnswerPreferenceFragment();
        return frag;
    }

    @Override
    public void onResume() {
        super.onResume();

        mAnswerTimePref = (ListPreference) findPreference(Constants.KEY_ANSWER_TIME);
        mAnswerTimePref.setOnPreferenceChangeListener(this);
        mAnswerTimePref.setSummary(mAnswerTimePref.getValue());

        mAutoModePref = (SwitchPreference) findPreference(Constants.KEY_AUTO_MODE);
        mAutoModePref.setOnPreferenceChangeListener(this);
        mAutoModePref.setDefaultValue(false);
        if (mAutoModePref.isChecked()) mAutoModePref.setSummary("on");
        else mAutoModePref.setSummary("off");
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "onPreferenceChange() : " + preference.getKey());
        Log.d(TAG, "Changed to " + newValue);
        if (Constants.KEY_AUTO_MODE.equals(preference.getKey())) {
            if (newValue.toString().equals("true")) {
                preference.setSummary("on");
                getContext().startService(new Intent(getContext(), CallService.class));
            } else {
                preference.setSummary("off");
                getContext().stopService(new Intent(getContext(), CallService.class));
            }
            return true;
        } else if (Constants.KEY_ANSWER_TIME.equals(preference.getKey())) {
            mAnswerTimePref.setSummary(newValue.toString());
            return true;
        }
        return false;
    }
}
