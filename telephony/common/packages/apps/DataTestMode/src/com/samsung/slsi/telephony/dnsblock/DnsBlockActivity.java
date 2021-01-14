/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.dnsblock;

import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import com.samsung.slsi.telephony.datatestmode.R;

public class DnsBlockActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "DnsBlockActivity";
    private static final String PROPERTY_DNS_BLOCK = "persist.vendor.config.dnsblock";
    private static final String PDP_WATCHDOG_TRIGGER_PACKET_COUNT = "pdp_watchdog_trigger_packet_count";
    protected static final int NUMBER_SENT_PACKETS_OF_HANG = 10;
    protected static final int NUMBER_SENT_PACKETS_OF_MAX = 100000;
    private SwitchPreference mDnsBlockPref;
    private boolean mFirstTime = true;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.preference);

        mDnsBlockPref = new SwitchPreference(this);
        if (mDnsBlockPref != null) {
            mDnsBlockPref.setTitle("DNS Query Block");
            mDnsBlockPref.setKey("key_dns_block");
            mDnsBlockPref.setOnPreferenceChangeListener(this);
        }

        fillList();
    }

    private void fillList() {
        PreferenceScreen root = (PreferenceScreen) findPreference("key_pref_screen");

        if (root != null) {
            root.removeAll();
            root.addPreference(mDnsBlockPref);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mFirstTime) {
            updateCurrentMode();
            mFirstTime = false;
        }
    }

    private void updateCurrentMode() {
        String mode = String.valueOf(SystemProperties.get(PROPERTY_DNS_BLOCK, "0"));
        if (mode.equals("1")) {
            Toast.makeText(getApplicationContext(), "DNS: Block", Toast.LENGTH_SHORT).show();
            mDnsBlockPref.setChecked(true);
            mDnsBlockPref.setSummary("on");
        } else {
            mDnsBlockPref.setChecked(false);
            mDnsBlockPref.setSummary("off");
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "Changed to " + newValue);
        if (preference.getKey().equals("key_dns_block")) {
            if (newValue.toString().equals("true")) {
                preference.setSummary("on");
                SystemProperties.set(PROPERTY_DNS_BLOCK, "1");
                Settings.Global.putInt(getApplicationContext().getContentResolver(), PDP_WATCHDOG_TRIGGER_PACKET_COUNT, NUMBER_SENT_PACKETS_OF_MAX);
            } else {
                preference.setSummary("off");
                SystemProperties.set(PROPERTY_DNS_BLOCK, "0");
                Settings.Global.putInt(getApplicationContext().getContentResolver(), PDP_WATCHDOG_TRIGGER_PACKET_COUNT, NUMBER_SENT_PACKETS_OF_HANG);
            }
            return true;
        }
        return false;
    }
}