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

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.ListPreference;

import android.telephony.TelephonyManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

public class UplmnDetailSetting extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "UplmnDetailSetting";
    private static final int MENU_ADD = Menu.FIRST;
    private static final int MENU_EDIT = Menu.FIRST + 1;
    private static final int MENU_DELE = Menu.FIRST + 2;
    private static final int MENU_CANCEL = Menu.FIRST + 3;
    private static final int MENU_BACK = android.R.id.home;
    private static final int EVENT_SET_PREFER_UPLMN  = 1;
    private EditPlmnPreference mNetworkIdPreferece;
    private ListPreference mButtonSelectNetwork;
    private Phone mPhone;
    private String mPlmn, mRat;
    private int mPreferMode, mIndex;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SET_PREFER_UPLMN:
                    UplmnSetting.goUpToTopLevelSetting(UplmnDetailSetting.this, mPhone.getPhoneId());
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
         super.onCreate(savedInstanceState);
         getActionBar().setDisplayHomeAsUpEnabled(true);
         addPreferencesFromResource(R.xml.uplmn_detail_setting);
         mNetworkIdPreferece = (EditPlmnPreference)getPreferenceScreen().findPreference("button_network_type");
         mButtonSelectNetwork = (ListPreference)getPreferenceScreen().findPreference("select_networks_key");
         mNetworkIdPreferece.setOnPreferenceChangeListener(this);
         mButtonSelectNetwork.setOnPreferenceChangeListener(this);

         int phoneId = getIntent().getIntExtra("phoneId", -1);
         mIndex = getIntent().getIntExtra("index", 0xFF);
         mPlmn = getIntent().getStringExtra("uplmn");
         mRat = getIntent().getStringExtra("rat");

         if (TelephonyManager.getDefault().isMultiSimEnabled()) {
            if (phoneId < 0) {
                return;
            }
            mPhone = (PhoneFactory.getPhones())[phoneId];//0 means that the SIM card 1,we can get phone from the array.
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        if (mRat != null && mPlmn != null) {
            UpdatePreferredNetworkModeSummary(Integer.parseInt(mRat));
            mNetworkIdPreferece.setText(mPlmn);
            mNetworkIdPreferece.setSummary(mPlmn);
        } else {
            mButtonSelectNetwork.setValue("0");
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        String value = (String)newValue;
        Log.d(TAG, "onPreferenceChange() " + value);
        if (preference == mButtonSelectNetwork) {
            UpdatePreferredNetworkModeSummary(Integer.parseInt(value));
        } else if (preference == mNetworkIdPreferece && (value.length() < 5 || value.length() > 6)) {
            Toast.makeText(getApplicationContext(), "Invalid Network ID! Restore it", Toast.LENGTH_LONG).show();
            mNetworkIdPreferece.setText(mPlmn);
            mNetworkIdPreferece.setSummary(mPlmn);
            return false;
        }
        return true;
    }

    private void UpdatePreferredNetworkModeSummary(int NetworkMode) {
        mPreferMode = NetworkMode;
        mButtonSelectNetwork.setValue(Integer.toString(NetworkMode));
        mButtonSelectNetwork.setSummary(UplmnSetting.sRatToSummary.get(mPreferMode));
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, MENU_ADD, 0, R.string.add)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW);
        menu.add(0, MENU_EDIT, 0, R.string.edit)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW);
        menu.add(0, MENU_DELE, 0, R.string.delete)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW);
        menu.add(0, MENU_CANCEL, 0, R.string.cancel)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        String summary = (String)mButtonSelectNetwork.getSummary();
        String text = mNetworkIdPreferece.getText();
        if (mPlmn != null) {
            if (summary.equals("Unknown") || text == null) {
                menu.findItem(MENU_EDIT).setEnabled(false);
            } else {
                menu.findItem(MENU_EDIT).setEnabled(true);
            }
            menu.removeItem(MENU_ADD);
        } else {
            if (summary.equals("Unknown") || text == null) {
                menu.findItem(MENU_ADD).setEnabled(false);
            } else {
                menu.findItem(MENU_ADD).setEnabled(true);
            }
            menu.removeItem(MENU_DELE);
            menu.removeItem(MENU_EDIT);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_ADD:
            mPhone.setPreferredUplmn(mPreferMode, 1, mIndex, mNetworkIdPreferece.getText(), mHandler.obtainMessage(EVENT_SET_PREFER_UPLMN));
            return true;
        case MENU_EDIT:
            mPhone.setPreferredUplmn(mPreferMode, 2, mIndex, mNetworkIdPreferece.getText(), mHandler.obtainMessage(EVENT_SET_PREFER_UPLMN));
            return true;
        case MENU_DELE:
            mPhone.setPreferredUplmn(Integer.parseInt(mRat), 3, mIndex, mPlmn, mHandler.obtainMessage(EVENT_SET_PREFER_UPLMN));
            return true;
        case MENU_BACK:
        case MENU_CANCEL:
            UplmnSetting.goUpToTopLevelSetting(this, mPhone.getPhoneId());
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
