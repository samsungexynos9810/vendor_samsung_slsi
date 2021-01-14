/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.uplmnsetting;

import android.preference.PreferenceScreen;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceGroup;

import android.telephony.TelephonyManager;
import android.app.Activity;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.UplmnInfoSet;
import com.android.internal.telephony.UplmnInfoSet.UplmnInfo;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.AsyncResult;
import android.util.Log;
import android.util.SparseArray;

import java.util.ArrayList;
import java.util.HashMap;

public class UplmnSetting extends PreferenceActivity {

    private static final String TAG = "UplmnSetting";
    private static final String UPLMN_LIST_KEY = "uplmn_list_key";
    private static final int EVENT_GET_PREFER_UPLMN = 1;
    private static final int MAX_INDEX_NUM = 16;
    private Phone mPhone;
    int mTotal;
    ArrayList<UplmnInfo> mUplmnInfo = new ArrayList<UplmnInfo>();
    private HashMap<Preference, UplmnInfo> mNetworkMap;
    private PreferenceGroup mUplmnList;

    static SparseArray<String> sRatToSummary = new SparseArray<String>();
    static {
        sRatToSummary.put(0, "UnKnown");
        sRatToSummary.put(1, "Gsm");
        sRatToSummary.put(2, "Gsm Compact");
        sRatToSummary.put(4, "UTRAN");
        sRatToSummary.put(8, "EUTRAN");
    }

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_GET_PREFER_UPLMN:
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    UplmnInfoSet res = (UplmnInfoSet) ar.result;
                    if (res == null) {
                        Log.d(TAG, "GET_PREFER_UPLMN is null");
                        break;
                    }
                    mUplmnInfo = res.getList();
                    if (res.getMaxPlmnNum() > 0) {
                        mTotal = res.getMaxPlmnNum();
                        Log.d(TAG, "total uplmn: " + mTotal);
                    }
                    updateUi();
                } else {
                    Log.d(TAG, "AsyncResult has exception " + ar.exception);
                    break;
                }
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.uplmn_setting);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        mUplmnList = (PreferenceGroup) getPreferenceScreen().findPreference(UPLMN_LIST_KEY);
        mNetworkMap = new HashMap<Preference, UplmnInfo>();
        mTotal = MAX_INDEX_NUM;
        int mPhoneId = getIntent().getIntExtra("phoneId", -1);
        Log.d(TAG, "mPhoneId = " + mPhoneId);
        if (TelephonyManager.getDefault().isMultiSimEnabled()) {
            if (mPhoneId < 0) {
                return;
            }
            mPhone = (PhoneFactory.getPhones())[mPhoneId]; // 0 means that the
                                                           // SIM card 1, we can
                                                           // get phone from the
                                                           // array.
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }

        mPhone.getPreferredUplmn(mHandler.obtainMessage(EVENT_GET_PREFER_UPLMN));
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        Intent intent = new Intent(this, UplmnDetailSetting.class);
        String plmn = (String) preference.getTitle();
        intent.putExtra("index", preference.getOrder());
        if (plmn.contains("Empty")) {
            // New PLMN
            intent.putExtra("phoneId", mPhone.getPhoneId());
        } else {
            plmn = mNetworkMap.get(preference).getPlmn();
            intent.putExtra("phoneId", mPhone.getPhoneId());
            intent.putExtra("uplmn", plmn);
            intent.putExtra("rat", mNetworkMap.get(preference).getRat());
        }
        startActivity(intent);
        return false;
    }

    private void updateUi() {
        for (int i = 0; i < mTotal; i++) {
            Preference pref = new Preference(UplmnSetting.this, null);
            pref.setTitle("Empty");
            pref.setSummary("priority: " + i);
            mUplmnList.addPreference(pref);
        }

        for (UplmnInfo ni : mUplmnInfo) {
            int index = Integer.parseInt(ni.getOperatorIndex());
            Preference pref = mUplmnList.getPreference(index);
            pref.setTitle(ni.getPlmn());
            pref.setPersistent(false);
            int rat = Integer.valueOf(ni.getRat()).intValue();
            pref.setSummary("priority: " + index + ", type: "+sRatToSummary.get(rat));
            Log.d(TAG, ni.toString());
            mNetworkMap.put(pref, ni);
        }
    }

    public static void goUpToTopLevelSetting(Activity activity, int phoneId) {

        Intent intent = new Intent(activity, UplmnSetting.class);
        intent.setAction(Intent.ACTION_MAIN);
        intent.putExtra("phoneId", phoneId);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        activity.startActivity(intent);
        activity.finish();
    }
}
