/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.apnsettingmode;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;


import com.samsung.slsi.telephony.datatestmode.R;

public class ApnSettingModeActivity extends Activity {

    private static final Uri CONTENT_URI = Uri.parse("content://telephony/carriers");
    private static final String TAG = "ApnList";
    private static final String DEFAULT_SORT_ORDER = "name ASC";
    private static final String TELEPHONY_CARRIERS_NAME = "name";
    private static final String TELEPHONY_CARRIERS_APN = "apn";
    private static final int ID_INDEX = 0;
    private static final int NAME_INDEX = 1;
    private static final int APN_INDEX = 2;
    private static final int MCC_INDEX = 3;
    private static final int MNC_INDEX = 4;

    private ArrayList<Map<String, String>> mApnList;
    private ArrayList<String> mKeyList;
    private SimpleAdapter mApnAdapter;
    private ListView mApnListView;
    private Button mBtnSearch;
    private EditText mEditSelectedApn;


    private final static String ACTION_VIEW="com.samsung.slsi.telephony.apnsettingmode.action.VIEW";
    private final static String ACTION_EDIT="com.samsung.slsi.telephony.apnsettingmode.action.EDIT";
    private final static String ACTION_INSERT="com.samsung.slsi.telephony.apnsettingmode.action.INSERT";


    @Override
    protected void onCreate(Bundle saveInstance) {
        super.onCreate(saveInstance);
        setContentView(R.layout.apnlist_activity);

        mKeyList = new ArrayList<String>();
        mApnList = new ArrayList<Map<String, String>>();
        mApnAdapter = new SimpleAdapter(this, mApnList,
                android.R.layout.simple_list_item_2, new String[] {
                        TELEPHONY_CARRIERS_APN, TELEPHONY_CARRIERS_NAME },
                new int[] { android.R.id.text1, android.R.id.text2 });
        mApnListView = (ListView) findViewById(R.id.apnList);
        registerForContextMenu(mApnListView);
        mEditSelectedApn = (EditText) findViewById(R.id.edit_selected_apn);
        mBtnSearch = (Button) findViewById(R.id.btn_search);
        mBtnSearch.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                // TODO Auto-generated method stub
                getApnList(mEditSelectedApn.getText().toString());
                mEditSelectedApn.setText("");
                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditSelectedApn.getWindowToken(), 0);
            }
        });

    }

    @Override
    protected void onResume() {
        super.onResume();
        getApnList("");
        registerForContextMenu(mApnListView);

    }

    private void getApnList(String numeric) {
           Log.d(TAG, "getApnList()");
           mApnList.clear();
           mKeyList.clear();
           Cursor cursor;
           if (numeric.equals("")) {
               cursor = getContentResolver().query(CONTENT_URI,
                       new String[] { "_id", "name", "apn", "mcc", "mnc" }, null,
                       null, DEFAULT_SORT_ORDER);
           } else {
               String where = "numeric=\"" + numeric + "\"";
               cursor = getContentResolver().query(CONTENT_URI,
                       new String[] { "_id", "name", "apn", "mcc", "mnc" }, where,
                       null, DEFAULT_SORT_ORDER);
           }
           if (cursor != null) {
               cursor.moveToFirst();
               while (!cursor.isAfterLast()) {
                   String numericList = cursor.getString(MCC_INDEX) + cursor.getString(MNC_INDEX);
                   Map<String, String> apn = new HashMap<String, String>();
                   apn.put(TELEPHONY_CARRIERS_APN,
                           cursor.getString(NAME_INDEX) + "("
                                   + numericList + ")");
                   apn.put(TELEPHONY_CARRIERS_NAME, cursor.getString(APN_INDEX));
                   mApnList.add(apn);
                   // key list
                   mKeyList.add(cursor.getString(ID_INDEX));
                   cursor.moveToNext();
               }
               cursor.close();
               mApnListView.setAdapter(mApnAdapter);
           }
           checkApnList();
       }

    public void checkApnList() {
        if (mApnList.isEmpty()) {
            Toast.makeText(this, "No APN Result", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
     public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
         getMenuInflater().inflate(R.menu.context_menu, menu);
         super.onCreateContextMenu(menu, v, menuInfo);
     }

     @Override
     public boolean onContextItemSelected(MenuItem item) {
         AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
         Uri url = ContentUris.withAppendedId(CONTENT_URI, Integer.parseInt(mKeyList.get(info.position)));
         switch(item.getItemId()) {
         case R.id.menu_edit:
             Log.d(TAG, "Edit APN : " + mKeyList.get(info.position));
             Intent intent = new Intent(ACTION_EDIT,url);
             intent.setClassName("com.samsung.slsi.telephony.datatestmode", "com.samsung.slsi.telephony.apnsettingmode.ApnEditorActivity");
             intent.addCategory("android.intent.category.DEFAULT");
             startActivity(intent);
             break;
         }
         return super.onContextItemSelected(item);
     }

}
