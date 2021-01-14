/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.usimusattest;

import java.util.ArrayList;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import com.samsung.slsi.telephony.testmode.R;

public class USIMUSATTestActivity extends Activity {

    private static final String TAG = "USIM_USAT_Test";

    private ArrayList<String> mUsimList, mUsatList;
    private ArrayAdapter<String> mUsimAdapter, mUsatAdapter;
    private ListView mUsimListView, mUsatListView;
    private Button mBtnUsim, mBtnUsat;
    private boolean USIM_MODE = true;
    private boolean USAT_MODE = true;
    private static final Uri CONTENT_URI = Uri.parse("content://telephony/carriers");
    private static final String DEFAULT_SORT_ORDER = "name ASC";
    private static final String TELEPHONY_CARRIERS_NAME = "name";
    private static final String TELEPHONY_CARRIERS_USER = "user";
    private static final String TELEPHONY_CARRIERS_PASSWORD = "password";
    private static final String TELEPHONY_CARRIERS_MCC = "mcc";
    private static final String TELEPHONY_CARRIERS_MNC = "mnc";
    private static final String TELEPHONY_CARRIERS_NUMERIC = "numeric";
    private static final String TELEPHONY_CARRIERS_APN = "apn";
    private static final String TELEPHONY_CARRIERS_TYPE = "type";
    private static final int ID_INDEX = 0;

    EditText mEditName, mEditApn, mEditUsername, mEditPwd, mEditMcc, mEditMnc,
            mEditApnType;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.usim_usat_activity);

        mUsimList = new ArrayList<String>();
        mUsimList.add("5.2.2 PartB (9)");

        mUsimAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mUsimList);
        mUsimListView = (ListView) findViewById(R.id.usimList);
        mUsimListView.setAdapter(mUsimAdapter);
        mUsimListView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);

        mUsimListView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View v,
                    int position, long id) {
                selectUSIMMenuItem(position);
            }
        });

        mUsatList = new ArrayList<String>();
        mUsatList.add("27.22.4.27.6/1");
        mUsatList.add("27.22.4.27.6/2");
        mUsatList.add("27.22.4.27.6/3");
        mUsatList.add("27.22.4.27.6/4");
        mUsatList.add("27.22.4.28.3/2");
        mUsatList.add("27.22.4.29.1/2");
        mUsatList.add("27.22.4.30.3/2");
        mUsatList.add("27.22.4.31.1/4");
        mUsatList.add("27.22.4.31.1/5");
        mUsatList.add("27.22.4.27.6/5");
        mUsatList.add("27.22.4.28.3/1");
        mUsatList.add("27.22.4.30.3/1");

        mUsatAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mUsatList);
        mUsatListView = (ListView) findViewById(R.id.usatList);
        mUsatListView.setAdapter(mUsatAdapter);
        mUsatListView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);

        mUsatListView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View v,
                    int position, long id) {
                selectUSATMenuItem(position);
            }
        });

        mBtnUsim = (Button) findViewById(R.id.btn_usim);
        mBtnUsim.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                if (USIM_MODE) {
                    mUsimListView.setVisibility(View.VISIBLE);
                    USIM_MODE = false;
                } else {
                    mUsimListView.setVisibility(View.GONE);
                    USIM_MODE = true;
                }
            }
        });

        mBtnUsat = (Button) findViewById(R.id.btn_usat);
        mBtnUsat.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                if (USAT_MODE) {
                    mUsatListView.setVisibility(View.VISIBLE);
                    USAT_MODE = false;
                } else {
                    mUsatListView.setVisibility(View.GONE);
                    USAT_MODE = true;
                }
            }
        });
    }

    private void selectUSIMMenuItem(int position) {
        apnInfoSetting("246", "081", "default");
    }

    private void selectUSATMenuItem(int position) {
        if (position < 9) {
            apnInfoSetting("001", "01", "bip");
        } else {
            apnInfoSetting("001", "01", "default");
        }
    }

    private void apnInfoSetting(final String mcc, final String mnc,
            final String apnType) {
        LayoutInflater inflater = LayoutInflater.from(this);
        final View dialogView = inflater
                .inflate(R.layout.dialog_apn_info, null);

        mEditName = (EditText) dialogView.findViewById(R.id.edit_name);
        mEditApn = (EditText) dialogView.findViewById(R.id.edit_apn);
        mEditUsername = (EditText) dialogView.findViewById(R.id.edit_user_name);
        mEditPwd = (EditText) dialogView.findViewById(R.id.edit_pwd);
        mEditMcc = (EditText) dialogView.findViewById(R.id.edit_mcc);
        mEditMnc = (EditText) dialogView.findViewById(R.id.edit_mnc);
        mEditApnType = (EditText) dialogView.findViewById(R.id.edit_apn_type);

        mEditMcc.setText(mcc);
        mEditMnc.setText(mnc);
        mEditApnType.setText(apnType);
        AlertDialog.Builder mBuilder;
        mBuilder = new AlertDialog.Builder(this);
        mBuilder.setTitle("APN Info Setting");
        mBuilder.setView(dialogView);
        mBuilder.setPositiveButton(android.R.string.ok, new OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub

                if (mEditApn.getText().toString().equals("")) {
                    Toast.makeText(getApplication(), "APN is empty.", Toast.LENGTH_SHORT).show();
                } else {
                    deleteSelectedApn(mcc, mnc);
                    createSelectedApn(mcc, mnc, apnType);
                }
            }
        });
        mBuilder.setNegativeButton(android.R.string.cancel, null);
        mBuilder.show();
    }

    private void deleteSelectedApn(String mcc, String mnc) {
        Log.d(TAG, "deleteSelectedApn()");
        String where = "numeric=\"" + mcc + mnc + "\"";
        Cursor cursor = getContentResolver().query(CONTENT_URI,
                new String[] { "_id", "name", "apn", "type", "sub_id" }, where,
                null, DEFAULT_SORT_ORDER);

        if (cursor != null) {
            cursor.moveToFirst();
            while (!cursor.isAfterLast()) {
                Log.d(TAG, "cursor key : " + cursor.getString(ID_INDEX));
                Uri uri = ContentUris.withAppendedId(CONTENT_URI,
                        Integer.parseInt(cursor.getString(ID_INDEX)));
                getContentResolver().delete(uri, null, null);
                cursor.moveToNext();
            }
            cursor.close();
        }
    }

    private boolean createSelectedApn(String mcc, String mnc, String apnType) {
        Log.d(TAG, "createSelectedApn()");
        Uri uri = getContentResolver().insert(CONTENT_URI, new ContentValues());
        Log.d(TAG, "new uri : " + uri.toString());
        String name = checkNotSet(mEditName.getText().toString());

        if (uri != null) {
            ContentValues values = new ContentValues();
            values.put(TELEPHONY_CARRIERS_NAME, name.length() < 1 ? getResources().getString(R.string.untitled_apn) : name);
            values.put(TELEPHONY_CARRIERS_APN, checkNotSet(mEditApn.getText().toString()));
            values.put(TELEPHONY_CARRIERS_USER, checkNotSet(mEditUsername.getText().toString()));
            values.put(TELEPHONY_CARRIERS_PASSWORD, checkNotSet(mEditPwd.getText().toString()));
            values.put(TELEPHONY_CARRIERS_TYPE, apnType);
            values.put(TELEPHONY_CARRIERS_MCC, mcc);
            values.put(TELEPHONY_CARRIERS_MNC, mnc);
            values.put(TELEPHONY_CARRIERS_NUMERIC, mcc + mnc);
            Log.d(TAG, values.toString());
            getContentResolver().update(uri, values, null, null);
        }
        return true;
    }

    private String checkNotSet(String value) {
        if (value == null || value.equals("")) {
            return "";
        } else {
            return value;
        }
    }
}
