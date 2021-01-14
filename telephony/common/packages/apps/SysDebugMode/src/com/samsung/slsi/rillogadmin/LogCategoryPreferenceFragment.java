/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.rillogadmin;

import static com.samsung.slsi.rillogadmin.VendorRILConstants.*;

import java.util.HashMap;
import java.util.Iterator;

import com.samsung.slsi.sysdebugmode.R;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.Toast;

public class LogCategoryPreferenceFragment extends PreferenceFragment {

    public static String[] LOG_CATEGORY = { "Core", "Call", "SMS", "SIM",
            "Network", "Data", "Misc", "Sound", "OEM", "RFS", };

    public static int[] LOG_VALUE = { RIL_LOG_CAT_CORE, RIL_LOG_CAT_CALL,
            RIL_LOG_CAT_SMS, RIL_LOG_CAT_SIM, RIL_LOG_CAT_NETWORK,
            RIL_LOG_CAT_DATA, RIL_LOG_CAT_MISC, RIL_LOG_CAT_SOUND,
            RIL_LOG_CAT_OEM, RIL_LOG_CAT_RFS, };

    public static final String TAG = "LogLevelPreferenceFragment";
    private HashMap<String, LogCategoryCheckBoxPreference> mCheckBoxPreference = new HashMap<String, LogCategoryCheckBoxPreference>();
    private int mCurrentLogCategory = 0;
    private boolean mChanged = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.ril_log_level_preference);
        setHasOptionsMenu(true);

        mCurrentLogCategory = getLogCategory();
        if (mCurrentLogCategory < 0) {
            Toast.makeText(getActivity(), "Fail to read RIL Log Category.",
                    Toast.LENGTH_LONG).show();
        }

        final boolean isAll = isCategoryAll(mCurrentLogCategory);
        Log.d(TAG,
                "currentLogLevel=" + Integer.toHexString(mCurrentLogCategory)
                        + " isAll=" + isAll);
        PreferenceGroup preference = (PreferenceGroup) findPreference(getString(R.string.key_category_log_level_preferences));
        if (preference != null) {
            for (int i = 0; i < LOG_CATEGORY.length; i++) {
                LogCategoryCheckBoxPreference cb = new LogCategoryCheckBoxPreference(
                        getActivity(), LOG_CATEGORY[i], LOG_VALUE[i]);
                mCheckBoxPreference.put(cb.getName(), cb);
                preference.addPreference(cb);

                Log.d(TAG,
                        "" + cb.getName() + "("
                                + Integer.toHexString(cb.getValue())
                                + ") enabled="
                                + isEnabled(mCurrentLogCategory, cb.getValue()));
                if (isAll || isEnabled(mCurrentLogCategory, cb.getValue())) {
                    cb.setChecked(true);
                }
            }
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {

        Log.v(TAG, "onPreferenceTreeClick()");
        if (preference != null) {
            LogCategoryCheckBoxPreference p = (LogCategoryCheckBoxPreference) preference;
            Log.d(TAG, "" + p.getName() + " clicked. checked=" + p.isChecked());
            mChanged = true;
        }

        return true;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.menu_log_level, menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.apply:
            onApply();
            break;
        case R.id.select_all:
            onSelectAll();
            break;
        case R.id.deselect_all:
            onDeselectAll();
            break;
        case R.id.reboot:
            onReboot();
            break;
        } // end switch ~
        return super.onOptionsItemSelected(item);
    }

    private void onApply() {
        Log.v(TAG, "onApply()");
        if (mChanged) {
            int newLogLevel = 0;
            Iterator<String> iter = mCheckBoxPreference.keySet().iterator();
            while (iter.hasNext()) {
                String key = iter.next();
                LogCategoryCheckBoxPreference p = mCheckBoxPreference.get(key);
                if (p != null) {
                    Log.d(TAG, "" + p.getName() + " checked=" + p.isChecked());
                    if (p.isChecked()) {
                        newLogLevel |= p.getValue();
                    }
                }
            } // end while ~
            Log.d(TAG, "newLogLevel=" + newLogLevel);

            setLogCategory(newLogLevel);
        } else {
            Toast.makeText(getActivity(), "Log policy is not changed.",
                    Toast.LENGTH_LONG).show();
        }
        mChanged = false;
    }

    private void onSelectAll() {
        Log.v(TAG, "onSelectAll()");

        Iterator<String> iter = mCheckBoxPreference.keySet().iterator();
        while (iter.hasNext()) {
            String key = iter.next();
            LogCategoryCheckBoxPreference p = mCheckBoxPreference.get(key);
            if (p != null && !p.isChecked()) {
                p.setChecked(true);
                mChanged = true;
            }
        } // end while ~
    }

    private void onDeselectAll() {
        Log.v(TAG, "onDeselectAll()");
        Iterator<String> iter = mCheckBoxPreference.keySet().iterator();
        while (iter.hasNext()) {
            String key = iter.next();
            LogCategoryCheckBoxPreference p = mCheckBoxPreference.get(key);
            if (p != null && p.isChecked()) {
                p.setChecked(false);
                mChanged = true;
            }
        } // end while ~
    }

    public void onReboot() {
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        Fragment prev = getFragmentManager().findFragmentByTag(
                "MyAlertDialogFragment");
        if (prev != null) {
            ft.remove(prev);
        }
        ft.addToBackStack(null);
        ft.commit();

        MyAlertDialogFragment dialog = MyAlertDialogFragment.newInstance();
        dialog.show(getFragmentManager(), "MyAlertDialogFragment");
    }

    int getLogCategory() {
        // read from system property
        int logCategory = RIL_LOG_NONE;
        try {
            logCategory = Integer.parseInt(SystemProperties
                    .get(PROPERTY_RIL_LOG_CATEGORY));
            ;
            Log.d(TAG, "Load log category=" + Integer.toHexString(logCategory));
        } catch (Exception e) {
            // 1. no permission to read system property
            // 2. no valid value yet
            // consider all
            Log.w(TAG, "Fail to load log category");
            return RIL_LOG_ALL;
        }

        return logCategory;
    }

    void setLogCategory(int logCategory) {
        if (mCurrentLogCategory == logCategory) {
            Log.i(TAG, "Log Level is not changed.");
            return;
        }

        // write system property
        SystemProperties.set(PROPERTY_RIL_LOG_CATEGORY,
                Integer.toString(logCategory));
        try {
            Thread.sleep(100);
        } catch (Exception e) {
        }

        int result = getLogCategory();
        if (result == RIL_LOG_INVALID || (result != logCategory)) {
            Log.w(TAG, "Fail to write new log level");
            Toast.makeText(getActivity(),
                    "Changing RIL Log Category was failed.", Toast.LENGTH_LONG)
                    .show();
        } else {
            Log.d(TAG,
                    "Log category has been changed. Old= "
                            + Integer.toHexString(mCurrentLogCategory)
                            + " New log level="
                            + Integer.toHexString(logCategory));
            mCurrentLogCategory = logCategory;
            Toast.makeText(
                    getActivity(),
                    "Changing RIL Log Category has been successful.\nPlease restart your system to apply new policy.",
                    Toast.LENGTH_LONG).show();

            onReboot();
        }
    }

    boolean isEnabled(int current, int value) {
        if (current == RIL_LOG_INVALID)
            return false;
        return (current & value) != 0;
    }

    boolean isCategoryAll(int logCategory) {
        if (RIL_LOG_INVALID == logCategory) {
            return false;
        }

        if (RIL_LOG_ALL == logCategory) {
            return true;
        }

        for (int value : LOG_VALUE) {
            if ((logCategory & value) == 0)
                return false;
        }
        return true;
    }

    static class LogCategoryCheckBoxPreference extends CheckBoxPreference {

        private String mName;
        private int mValue;

        public LogCategoryCheckBoxPreference(Context context, String name,
                int value) {
            super(context);
            mName = name;
            mValue = value;

            setTitle(mName);
            setPersistent(false);
            setChecked(false);
        }

        public String getName() {
            return mName;
        }

        public int getValue() {
            return mValue;
        }
    }

    public static class MyAlertDialogFragment extends DialogFragment {
        public static MyAlertDialogFragment newInstance() {
            MyAlertDialogFragment frag = new MyAlertDialogFragment();
            Bundle args = new Bundle();
            args.putString("title", "Alert");
            args.putString("message", "Turn off and on the screen");
            frag.setArguments(args);
            return frag;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            String title = getArguments().getString("title");
            String message = getArguments().getString("message");

            return new AlertDialog.Builder(getActivity()).setTitle(title)
                    .setMessage(message).setPositiveButton("OK", null).create();
        }
    }
}