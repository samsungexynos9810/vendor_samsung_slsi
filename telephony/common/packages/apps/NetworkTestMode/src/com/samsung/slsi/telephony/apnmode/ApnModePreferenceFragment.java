/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.apnmode;

import java.util.ArrayList;

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.Uri;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.samsung.slsi.telephony.networktestmode.R;

public class ApnModePreferenceFragment extends PreferenceFragment {

    private static final String TAG = "ApnModePreferenceFragment";
    private static final Uri CONTENT_URI = Uri.parse("content://telephony/carriers");
    private ArrayList<NetworkCallback> mNetworkCallback = new ArrayList<NetworkCallback>();
    private ArrayList<SwitchPreference> mPrefList = new ArrayList<SwitchPreference>();
    private static String mPlmn = "";
    private static int mSubId = -1;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.layout.preference_apn_mode);
        fillList();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    private void fillList() {
        PreferenceGroup root = (PreferenceGroup) findPreference(ApnModeConstants.KEY_APN_LIST);

        if (root != null) {
            root.removeAll();
            mNetworkCallback.clear();
            mPrefList.clear();
            String where = "numeric=\"" + mPlmn + "\"";
            Cursor cursor = getContext().getContentResolver().query(
                    CONTENT_URI, new String[] { "_id", "name", "apn", "type", "numeric" },
                    where, null, ApnModeConstants.DEFAULT_SORT_ORDER);
            String numeric = "";
            if (cursor != null) {
                cursor.moveToFirst();
                while (!cursor.isAfterLast()) {
                    String key = cursor.getString(ApnModeConstants.ID_INDEX);
                    String type = cursor.getString(ApnModeConstants.TYPE_INDEX);
                    String apn = cursor.getString(ApnModeConstants.APN_INDEX);
                    numeric= cursor.getString(4);
                    SwitchPreference pref = new SwitchPreference(getContext());
                    if (pref != null) {
                        pref.setTitle(apn);
                        pref.setSummary(type);
                        pref.setKey(key);
                        root.addPreference(pref);
                        mNetworkCallback.add(new NetworkCallback());
                        mPrefList.add(pref);
                    }
                    cursor.moveToNext();
                }
                cursor.close();
                Toast.makeText(getContext(), "numeric = "+numeric, Toast.LENGTH_LONG).show();
            }
        }
    }

    private int convertApnTypeToNetworkCapability(String apnType) {
        if (!TextUtils.isEmpty(apnType)) {
            if (apnType.equals(ApnModeConstants.APN_TYPE_DEFAULT)) {
                return NetworkCapabilities.NET_CAPABILITY_INTERNET;
            } else if (apnType.equals(ApnModeConstants.APN_TYPE_MMS)) {
                return NetworkCapabilities.NET_CAPABILITY_MMS;
            } else if (apnType.equals(ApnModeConstants.APN_TYPE_DUN)) {
                return NetworkCapabilities.NET_CAPABILITY_DUN;
            } else if (apnType.equals(ApnModeConstants.APN_TYPE_FOTA)) {
                return NetworkCapabilities.NET_CAPABILITY_FOTA;
            } else if (apnType.equals(ApnModeConstants.APN_TYPE_IMS)) {
                return NetworkCapabilities.NET_CAPABILITY_IMS;
            } else if (apnType.equals(ApnModeConstants.APN_TYPE_CBS)) {
                return NetworkCapabilities.NET_CAPABILITY_CBS;
            } else if (apnType.equals(ApnModeConstants.APN_TYPE_SUPL)) {
                return NetworkCapabilities.NET_CAPABILITY_SUPL;
            }
        }
        return NetworkCapabilities.NET_CAPABILITY_INTERNET;
    }

    private NetworkRequest getNetworkRequest(String apnType) {
        NetworkRequest.Builder builder = new NetworkRequest.Builder();
        builder.addCapability(convertApnTypeToNetworkCapability(apnType));
        builder.addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED);
        builder.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        builder.setNetworkSpecifier(Integer.toString(mSubId));
        return builder.build();
    }

    private String checkType(String type) {
        if (type.contains("ims")) {
            return "ims";
        } else if (type.contains("cbs")) {
            return "cbs";
        }
        return type;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        Log.d(TAG, "onPreferenceTreeClick()");
        SwitchPreference switch_pref = (SwitchPreference) preference;
        Uri uri = ContentUris.withAppendedId(CONTENT_URI, Integer.parseInt(switch_pref.getKey()));
        ConnectivityManager cm = (ConnectivityManager) getContext().getSystemService(Context.CONNECTIVITY_SERVICE);
        String type = switch_pref.getSummary().toString();
        if (type.contains("default")) {
        } else {
            type = checkType(type);
            Log.d(TAG, "Click: " + type + ", " + switch_pref.getOrder());
            if (switch_pref.isChecked()) {
                cm.requestNetwork(getNetworkRequest(type), mNetworkCallback.get(switch_pref.getOrder()));
            } else {
                try {
                    cm.unregisterNetworkCallback(mNetworkCallback.get(switch_pref.getOrder()));
                } catch (Exception e) {
                    Log.e(TAG, "Exception: mNetworkCallback is unstable.");
                }
            }
        }
        //ContentValues values = new ContentValues();
        //values.put(ApnModeConstants.CARRIER_ENABLED, switch_pref.isChecked() ? 1 : 0);
        //getContext().getContentResolver().update(uri, values, null, null);
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    public static ApnModePreferenceFragment newInstance(String plmn, int subId) {
        Log.d(TAG, "plmn: " + plmn + ", subId: " +subId);
        ApnModePreferenceFragment frag = new ApnModePreferenceFragment();
        mPlmn = plmn;
        mSubId = subId;
        return frag;
    }

}
