/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.networkmode;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.preference.ListPreference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.WindowManager;
import android.widget.Toast;

import com.samsung.slsi.telephony.testmode.R;

public class NetworkModePreferenceFragment extends PreferenceFragment {

    private static final String TAG = "NetworkModeSetting";
    private ListPreference mPhoneIdPref, mNetworkModeListPref;
    private TelephonyManager mTelephonyManager;
    private SubscriptionManager mSubscriptionManager;
    private int mPreferredNetworkType;
    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.preference_networkmode);

        mTelephonyManager = (TelephonyManager) getActivity().getSystemService(Context.TELEPHONY_SERVICE);
        mSubscriptionManager = (SubscriptionManager) getActivity().getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);

        mPhoneIdPref = (ListPreference) findPreference("key_phoneid");
        mPhoneIdPref.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {
                preference.setSummary("");
                int pos = mPhoneIdPref.findIndexOfValue(newValue.toString());
                if (pos >= 0) {
                    mPhoneIdPref.setSummary(mPhoneIdPref.getEntries()[pos]);
                    mPhoneIdPref.setValueIndex(pos);
                }
                updateSubId(Integer.parseInt(newValue.toString()));
                return true;
            }
        });

        mNetworkModeListPref = (ListPreference) findPreference("key_networkmode");
        mNetworkModeListPref.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object newValue) {
                preference.setSummary("");
                int pos = mNetworkModeListPref.findIndexOfValue(newValue.toString());
                if (pos >= 0) {
                    mNetworkModeListPref.setSummary(mNetworkModeListPref.getEntries()[pos]);
                    mNetworkModeListPref.setValueIndex(pos);
                }
                mPreferredNetworkType = Integer.parseInt(newValue.toString());
                updateNetworkMode();
                return true;
            }
        });

        updateUI();
    }
    public static NetworkModePreferenceFragment newInstance() {
        NetworkModePreferenceFragment frag = new NetworkModePreferenceFragment();
        return frag;
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    private void updateUI() {
        String stringValue = mPhoneIdPref.getValue();
        int pos = mPhoneIdPref.findIndexOfValue(stringValue);
        mPhoneIdPref.setSummary(pos >= 0 ? mPhoneIdPref.getEntries()[pos] : null);
        if (!TextUtils.isEmpty(stringValue))
            updateSubId(Integer.parseInt(stringValue.toString()));

        stringValue = mNetworkModeListPref.getValue();
        pos = mNetworkModeListPref.findIndexOfValue(stringValue);
        mNetworkModeListPref.setSummary(pos >= 0 ? mNetworkModeListPref.getEntries()[pos] : null);
    }

    private void updateSubId(int phoneId) {
        Log.d(TAG, "updateSubId() : phone(" + phoneId + ")");
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        if (SubscriptionManager.isValidPhoneId(phoneId)) {
            int[] subIds = SubscriptionManager.getSubId(phoneId);
            if (subIds != null) {
                mSubId = subIds[0];
            }
        }
    }

    private void updateNetworkMode() {
        Log.d(TAG, "Calling setPreferredNetworkType(" + mPreferredNetworkType + ")");
        if (mSubId < 0) {
            Log.d(TAG, "Invalid SubId (" + mSubId + ")");
            Toast.makeText(getActivity().getApplicationContext(), "Invalid SubId(" + mSubId + "), Please check SIM status", Toast.LENGTH_LONG).show();
            return ;
        }
        mTelephonyManager.setPreferredNetworkType(mSubId, mPreferredNetworkType);
    }

}
