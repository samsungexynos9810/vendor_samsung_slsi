/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.dualvolte;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.util.Log;
import android.view.WindowManager;

import com.samsung.slsi.telephony.networktestmode.R;

public class DualVolteSettingActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "DualVolteSetting";
    private static final String PROPERTY_DUAL_VOLTE = "persist.vendor.radio.dual.volte";
    private SwitchPreference mDualVolteModePref;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.preference);

        mDualVolteModePref = new SwitchPreference(this);
        if (mDualVolteModePref != null) {
            mDualVolteModePref.setTitle("Dual VoLTE Mode");
            mDualVolteModePref.setKey("key_dual_volte");
            mDualVolteModePref.setOnPreferenceChangeListener(this);
        }

        fillList();
    }

    private void fillList() {
        PreferenceScreen root = (PreferenceScreen) findPreference("key_pref_screen");

        if (root != null) {
            root.removeAll();
            root.addPreference(mDualVolteModePref);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updateCurrentMode();
    }

    private void updateCurrentMode() {
        String mode = String.valueOf(SystemProperties.get(PROPERTY_DUAL_VOLTE, "0"));
        if (mode.equals("1")) {
            mDualVolteModePref.setChecked(true);
            mDualVolteModePref.setSummary("on");
        } else {
            mDualVolteModePref.setChecked(false);
            mDualVolteModePref.setSummary("off");
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "Changed to " + newValue);
        if (preference.getKey().equals("key_dual_volte")) {
            if (newValue.toString().equals("true")) {
                preference.setSummary("on");
                SystemProperties.set(PROPERTY_DUAL_VOLTE, "1");
            } else {
                preference.setSummary("off");
                SystemProperties.set(PROPERTY_DUAL_VOLTE, "0");
            }
            reboot();
            return true;
        }
        return false;
    }

    public void reboot() {
        final PowerManager pm = (PowerManager) getApplicationContext().getSystemService(Context.POWER_SERVICE);
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Reboot the device").setPositiveButton("OK", new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                pm.reboot(null);
            }
        }).setNegativeButton("Cancel", null);
        final AlertDialog resetDialog = builder.create();
        resetDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        resetDialog.show();
    }
}