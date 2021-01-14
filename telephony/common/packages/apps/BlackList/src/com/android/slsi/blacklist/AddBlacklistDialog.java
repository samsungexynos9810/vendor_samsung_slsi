/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.android.slsi.blacklist;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.database.Cursor;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;
import android.content.Intent;
import android.util.Log;


public class AddBlacklistDialog extends Activity{

    private EditText mNumberEditor;
    private EditText mNameEditor;
    private CheckBox mCallCheck;
    private CheckBox mMmsCheck;
    private View mDialogView;
    private String mNumber = "";
    private String mName = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.blacklist_dialog_layout);
        initDialogView();
        Intent intent = this.getIntent();
        if (intent != null) {
            mNumber = intent.getStringExtra("add_blacklist_number");
            mName = intent.getStringExtra("add_blacklist_name");
            mNumberEditor.setText(mNumber);
            mNameEditor.setText(mName);
            addBlacklistDialog();
        }
    }

    private void initDialogView() {
        mDialogView= LayoutInflater.from(AddBlacklistDialog.this).inflate(R.layout.add_blacklist_dialog, null);
        mNumberEditor = (EditText)mDialogView.findViewById(R.id.enter_number);
        mNameEditor = (EditText)mDialogView.findViewById(R.id.enter_name);
        mCallCheck = (CheckBox)mDialogView.findViewById(R.id.intercept_call_cb);
        mCallCheck.setChecked(true);
        mMmsCheck = (CheckBox)mDialogView.findViewById(R.id.intercept_sms_cb);
        mMmsCheck.setChecked(true);
    }

    private void addBlacklistDialog () {
        AlertDialog.Builder builder = new AlertDialog.Builder(AddBlacklistDialog.this);
        builder.setTitle(R.string.add_blacklist_title);
        builder.setView(mDialogView);
        builder.setPositiveButton(R.string.yes, new OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                String number = mNumberEditor.getText().toString();
                String name = mNameEditor.getText().toString();
                if (number == null || number.trim().length() == 0) {
                    Toast.makeText(AddBlacklistDialog.this, R.string.add_invalid_toast, Toast.LENGTH_LONG).show();
                } else {
                    ContentValues values = new ContentValues();
                    if (name.length() > 0) {
                        values.put(DBConfig.BlackListInfo.NAME, name);
                    }
                    values.put(DBConfig.BlackListInfo.NUMBER, number);
                    if(mCallCheck.isChecked() && ! mMmsCheck.isChecked()){
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_CALL);
                    }else if(!mCallCheck.isChecked() && mMmsCheck.isChecked()){
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_SMS);
                    }else{
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_ALL);
                    }
                    insertBlackList(values);
                }
                AddBlacklistDialog.this.finish();
            }
        });
        builder.setNegativeButton(R.string.no, new OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                AddBlacklistDialog.this.finish();
            }
        });
        builder.show();
    }

    //Judge if this number has been added to the blacklist, then need to update this record;
    //otherwise, add this number to the table of blacklistinfo.
    private void insertBlackList(ContentValues values) {
        ContentResolver cr = getBaseContext().getContentResolver();
        String number = values.getAsString(DBConfig.BlackListInfo.NUMBER);
        int toastId ;
        try {
            Cursor cursor = cr.query(DBConfig.BlackListInfo.CONTENT_URI, null, "number = ?", new String[] {number}, null);
            if (cursor.getCount() == 0) {
                cr.insert(DBConfig.BlackListInfo.CONTENT_URI, values);
                toastId = R.string.add_success_toast;
            } else {
                toastId = R.string.add_repeat_toast;
            }
            cursor.close();
            Toast.makeText(AddBlacklistDialog.this, toastId, Toast.LENGTH_LONG).show();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
