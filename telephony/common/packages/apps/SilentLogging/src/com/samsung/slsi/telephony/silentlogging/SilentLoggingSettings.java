/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.silentlogging;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.widget.Toast;

/**
 * Created by jin-h.shin on 2016-01-12.
 */
public class SilentLoggingSettings extends PreferenceActivity {

    private static final String TAG = "SilentLoggingSettings";
    private static int ACTIVITY_CHOOSE_FILE = 9;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //getActionBar().setDisplayHomeAsUpEnabled(true);
        addPreferencesFromResource(R.xml.settings);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        Log.d(TAG, "selected preference key : " + preference.getKey());
        //Toast.makeText(this, "preference key : " + preference.getKey(), Toast.LENGTH_SHORT).show();
        if (preference.getKey().equalsIgnoreCase("key_profile_load")) {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType("*/*");
            startActivityForResult(intent, ACTIVITY_CHOOSE_FILE);
        } else if (preference.getKey().equalsIgnoreCase("key_ap_logtype_list")) {
            Intent intent = new Intent(this, APListSettingActivity.class);
            startActivity(intent);
        } else if (preference.getKey().equalsIgnoreCase("key_profile_reset")) {
            String result = SilentLoggingProfile.getInstance().resetProfile();
            ResultMessage(result);
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        //super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == ACTIVITY_CHOOSE_FILE && resultCode == RESULT_OK) {
            //Toast.makeText(this, "Uri : " + data.getData().toString(), Toast.LENGTH_SHORT).show();
            Uri uri = data.getData();
            if (uri.getLastPathSegment().endsWith("nprf")) {
                //String profilePath = uri.getPath();
                Log.i(TAG, "file path : " + uri.getPath());
                SilentLoggingProfile.getInstance().changeProfile(uri);
            } else
                Toast.makeText(this, "This file is not profile : " + data.getData().toString(), Toast.LENGTH_SHORT).show();
        }
    }

    public void ResultMessage(String message) {
        AlertDialog.Builder alert_builder = new AlertDialog.Builder(this);
        alert_builder
                .setMessage(message)
                .setCancelable(false)
                .setPositiveButton(android.R.string.yes,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int id) {
                                dialog.dismiss();
                            }
                        });

        AlertDialog alert_dialog = alert_builder.create();
        alert_dialog.setTitle("Result");
        alert_dialog.show();
    }
/*
    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        SharedPreferences.Editor editor = sharedPreferences.edit();
        if(key.equalsIgnoreCase("key_tcp_logging_type")) {
            editor.putString("key_tcp_logging_type", "any");
        }
        editor.commit();
    }*/
}
