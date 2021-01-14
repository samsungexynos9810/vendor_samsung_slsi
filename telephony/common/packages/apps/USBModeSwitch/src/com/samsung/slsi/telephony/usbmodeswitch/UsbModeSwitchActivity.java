/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.usbmodeswitch;

import android.os.Bundle;
import android.os.Handler;
import android.os.SystemProperties;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

public class UsbModeSwitchActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "UsbModeSwitch";
    private static final String PROPERTY_USB_CONFIG = "sys.usb.config";
    private ListPreference mUsbModePref;

    private final Handler mHandler = new Handler() {};

    @Override
    protected void onCreate(Bundle saveInstance) {
        super.onCreate(saveInstance);
        addPreferencesFromResource(R.layout.preferences);
        mUsbModePref = (ListPreference) getPreferenceScreen().findPreference("key_usb_mode");
        mUsbModePref.setOnPreferenceChangeListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        updateCurrentUsbMode();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "onPreferenceChange() " + newValue.toString());
        if (preference == mUsbModePref) {
            if (!TextUtils.isEmpty(newValue.toString())) {
                setCurrentUsbFunction(newValue.toString(), true);
                preference.setSummary(newValue.toString());
            }
            return true;
        }
        return false;
    }

    public void updateCurrentUsbMode() {
        String currentFunctions = getCurrentUsbFunction();
        Log.d(TAG, "update current USB mode : " + currentFunctions);
        mUsbModePref.setValue(currentFunctions);
        mUsbModePref.setSummary(currentFunctions);
    }

    public boolean setCurrentUsbFunction(String functions, boolean makeDefault) {
        UsbManagerProxy usbManager = UsbManagerProxy.getInstance(this);
        if (usbManager == null) {
            return false;
        }

        if (functions.equals("dm,acm,uts,adb"))
        {
            SystemProperties.set(PROPERTY_USB_CONFIG, functions);
            return true;
        }
        return usbManager.setCurrentUsbFunctions(functions, makeDefault);
    }

    private String getCurrentUsbFunction() {
        UsbManagerProxy usbManager = UsbManagerProxy.getInstance(this);
        if (usbManager != null) {
            return usbManager.getCurrentUsbFunctions();
        }
        return "";
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {

        if (item.getItemId() == R.id.recover) {
            DoRecover();
            return true;
        }

        return super.onMenuItemSelected(featureId, item);
    }

    private void DoRecover() {
        Log.d(TAG, "DoRecover");

        // change default USB mode.
        setCurrentUsbFunction("adb", true);
        mHandler.postDelayed(new Runnable() {

            @Override
            public void run() {
                updateCurrentUsbMode();
                mUsbModePref.setValue("");
            }
        }, 300);
    }
}
