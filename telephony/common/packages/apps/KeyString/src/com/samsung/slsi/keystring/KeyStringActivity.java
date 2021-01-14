/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.keystring;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import com.android.internal.telephony.TelephonyIntents;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.FragmentTransaction;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.preference.Preference;
import android.util.Log;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;

public class KeyStringActivity extends Activity {

    private final String TAG = "KeyString";
    private final String mTitle = "Help";
    private AlertDialog.Builder mBuilder;
    private ArrayList<String> mList;
    private ArrayAdapter<String> mAdapter;
    private ListView mListView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final EditText editKeyStr = (EditText) findViewById(R.id.edit_keystring);
        Button btnOk = (Button) findViewById(R.id.btn_ok);
        btnOk.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View arg0) {
                String key = editKeyStr.getText().toString();
                editKeyStr.setText("");
                handleSecretCode(getApplication(), key);

                if (mList.contains(key)) {
                    mList.remove(key);
                }
                mList.add(0, key);
                mAdapter.notifyDataSetChanged();
            }
        });

        loadList();
        mAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, mList);
        mListView = (ListView) findViewById(R.id.list);
        mListView.setAdapter(mAdapter);
        mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int pos, long id) {
                String key = mList.get(pos);
                handleSecretCode(getApplication(), key);
            }
        });
        registerForContextMenu(mListView);
    }

    @Override
    protected void onPause() {
        saveList();
        super.onPause();
    }

    static private boolean handleSecretCode(Context context, String keystring) {
        int len = keystring.length();
        if (len > 8 && keystring.startsWith("*#*#") && keystring.endsWith("#*#*")) {
            Intent intent = new Intent(TelephonyIntents.SECRET_CODE_ACTION,
                    Uri.parse("android_secret_code://" + keystring.substring(4, len - 4)));
            intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
            context.sendBroadcast(intent);
            return true;
        }
        return false;
    }

    private void loadList() {
        SharedPreferences preferences = getSharedPreferences("list_keystring", Activity.MODE_PRIVATE);
        Map<String, String> map = new HashMap<String, String>();
        map = (Map<String, String>) preferences.getAll();
        mList = new ArrayList<String>(map.values());
    }

    private void saveList() {
        SharedPreferences preferences = getSharedPreferences("list_keystring", Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.clear();
        for (String key : mList) {
            editor.putString(key, key);
        }
        editor.commit();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "onCreateOptionsMenu occur");
        super.onCreateOptionsMenu(menu);
        getMenuInflater().inflate(R.menu.key_string, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if(item.getItemId() == R.id.action_help) {
            StringBuilder sb = new StringBuilder();
            sb.append("\t\t*#*# + Secret Number + #*#*\n");
            sb.append("\t---------------------------------------------------\n");
            sb.append("\t\t\t 5096 : SysDebugMode\n");
            sb.append("\t\t\t 0328 : DMSettings\n");
            sb.append("\t\t\t 0808 : USBSetting\n");
            sb.append("\t\t\t 1290 : UARTSetting\n");
            sb.append("\t\t\t 4412 : EngineerMode\n");
            sb.append("\t\t\t 7777 : TestMode\n");
            sb.append("\t\t\t 1214 : AutoAnswer\n");
            sb.append("\t---------------------------------------------------\n");
            printMessage(mTitle, sb.toString());
        }
        return true;
    }

    private void printMessage(String title, String msg) {
        mBuilder = new AlertDialog.Builder(this);
        mBuilder.setIcon(android.R.drawable.ic_dialog_info);
        mBuilder.setTitle(title);
        mBuilder.setMessage(msg);
        mBuilder.setPositiveButton(android.R.string.ok, null);
        mBuilder.setCancelable(true);
        mBuilder.show();
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        getMenuInflater().inflate(R.menu.context_menu, menu);

        super.onCreateContextMenu(menu, v, menuInfo);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
        switch(item.getItemId()) {
        case R.id.menu_delete:
            mList.remove(info.position);
            mAdapter.notifyDataSetChanged();
            break;
        }
        return super.onContextItemSelected(item);
    }
}
