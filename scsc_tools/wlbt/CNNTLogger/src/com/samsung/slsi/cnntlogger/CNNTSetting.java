package com.samsung.slsi.cnntlogger;

import android.content.Intent;
import android.preference.Preference;
import android.preference.ListPreference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.os.Bundle;
import android.util.Log;

public class CNNTSetting extends PreferenceActivity {

    private static final String TAG = "CNNTSetting";

    private CNNTUtils mUtils;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.setting);
        mUtils = new CNNTUtils(getApplicationContext());
        setOnPreferenceChange(findPreference("key_ap_logtype_list"));
        setOnPreferenceChange(findPreference("key_tcp_logging_type"));
    }

    @Override
    public void onRestart() {
        super.onRestart();
        setOnPreferenceChange(findPreference("key_ap_logtype_list"));
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        Log.d(TAG, "selected preference key : " + preference.getKey());
        if (preference.getKey().equalsIgnoreCase("key_ap_logtype_list")) {
            Intent intent = new Intent(this, APSettingActivity.class);
            startActivity(intent);
        } else if (preference.getKey().equalsIgnoreCase("key_bt_logtype_list")) {
            Intent intent = new Intent(this, BTSettingActivity.class);
            startActivity(intent);
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    private Preference.OnPreferenceChangeListener onPreferenceChangeListener =
        new Preference.OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            String stringValue = newValue.toString();

            if (preference.getKey().equals("key_tcp_logging_type")) {
                ListPreference listPreference = (ListPreference) preference;
                int index = listPreference.findIndexOfValue(stringValue);
                preference.setSummary(index >= 0 ? listPreference.getEntries()[index] : null);
            } else if (preference.getKey().equals("key_ap_logtype_list")) {
                String apList[];
                String buffer = "";
                apList = mUtils.getStringArr(R.array.ap_logtype);
                for (int i = 0; i < apList.length; i++) {
                    if (mUtils.getPreference(apList[i], true)) {
                        buffer = buffer + ", " + apList[i];
                        if (i == 0)
                            break;
                    }
                }
                if (!buffer.equals("")) {
                    buffer = buffer.substring(2);
                }
                preference.setSummary(buffer);
            }
            return true;
        }
    };

    private void setOnPreferenceChange(Preference mPreference) {
        mPreference.setOnPreferenceChangeListener(onPreferenceChangeListener);
        onPreferenceChangeListener.onPreferenceChange(mPreference,
            PreferenceManager.getDefaultSharedPreferences(
                mPreference.getContext()).getString(mPreference.getKey(), ""));
    }
}
