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

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.util.SparseBooleanArray;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class APListSettingActivity extends Activity {

    SharedPreferences sharedPref;
    SharedPreferences.Editor editPref;
    private static final String TAG = "SilentLoggingAPList";
    private static final String SHARED_PREF = "SHARED_PREF";
    private ListView listView;
    private String apList[];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list_ap);

        sharedPref = this.getSharedPreferences(SHARED_PREF, Context.MODE_PRIVATE);
        editPref = sharedPref.edit();

        apList = getResources().getStringArray(R.array.ap_logtype);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_multiple_choice, apList);
        listView = (ListView) findViewById(R.id.list_ap);
        listView.setAdapter(adapter);
        listView.setChoiceMode(listView.CHOICE_MODE_MULTIPLE);

        checkInitValue();

        listView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view,
                                    int position, long id) {
                // TODO Auto-generated method stub
                SparseBooleanArray sparse = listView.getCheckedItemPositions();
                if (sparse.get(0) && position == 0) {
                    for (int i = 1; i < apList.length; i++) {
                        listView.setItemChecked(i, true);
                    }
                } else if (!sparse.get(0) && position == 0) {
                    for (int i = 1; i < apList.length; i++) {
                        listView.setItemChecked(i, false);
                    }
                } else {
                    listView.setItemChecked(0, false);
                }
            }
        });

        for (int i = 0; i < apList.length; i++) {
            listView.setItemChecked(i, sharedPref.getBoolean(apList[i], false));
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_ap_list, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_save) {
            SparseBooleanArray sparse = listView.getCheckedItemPositions();
            int cnt = 0;
            for (int i = 0; i < apList.length; i++) {
                if (sparse.get(i)) {
                    Log.d(TAG, "result : " + apList[i] + " is clicked.");
                    editPref.putBoolean(apList[i], true);
                    cnt++;
                } else {
                    editPref.putBoolean(apList[i], false);
                }
            }
            editPref.commit();

            if (cnt == 0) {
                Log.d(TAG, "AP List is not checked");
                Toast.makeText(this, "There is no checked AP List.", Toast.LENGTH_SHORT).show();
            }
        }
        finish();   // exit activity after save ap logging type
        return super.onOptionsItemSelected(item);
    }

    public void checkInitValue() {
        if (!sharedPref.contains("all")) {
            Log.d(TAG, "AP List is empty");
            for (int i = 0; i < apList.length; i++) {
                editPref.putBoolean(apList[i], true);
            }
        }
        editPref.commit();
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }
}
