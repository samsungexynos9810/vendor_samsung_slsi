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
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.util.Log;
import android.view.ContextMenu;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.view.Window;
import android.widget.EditText;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.AdapterView.AdapterContextMenuInfo;

public class BlackListMainActivity extends Activity {

    private ImageButton mAddBtn;
    private ImageButton mLogBtn;
    private ImageButton mSettingsBtn;
    private EditText mNumberEditor;
    private EditText mNameEditor;
    private CheckBox mCallCheck;
    private CheckBox mMmsCheck;
    private View mDialogView;
    private ListView mListview;
    private BlackListAdapter mBlackListAdapter;
    private long itemId;
    private ImageButton mBatchbtn;
    private ImageButton mBack;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_black_list_main);
        mAddBtn = (ImageButton)findViewById(R.id.add_button);
        mLogBtn = (ImageButton)findViewById(R.id.call_log_button);
        mBatchbtn = (ImageButton)findViewById(R.id.title_batch_btn);
        mBack = (ImageButton)findViewById(R.id.title_back_btn);
        mListview = (ListView)findViewById(R.id.black_list_number);
        mBlackListAdapter = new BlackListAdapter(this,getCursor());
        mListview.setAdapter(mBlackListAdapter);
        this.registerForContextMenu(mListview);
        mAddBtn.setOnClickListener(new android.view.View.OnClickListener(){

            @Override
            public void onClick(View v){
                initDialogView();
                addBlacklistDialog();
            }
        });
        mLogBtn.setOnClickListener(new android.view.View.OnClickListener(){

            @Override
            public void onClick(View v){
                Intent intent = new Intent(BlackListMainActivity.this, CallFireLogActivity.class);
                startActivity(intent);
            }
        });
        mBatchbtn.setOnClickListener(new android.view.View.OnClickListener(){

            @Override
            public void onClick(View v){
                Intent intent = new Intent(BlackListMainActivity.this, BlackListBatchActivity.class);
                startActivity(intent);
            }
        });
        mBack.setOnClickListener(new android.view.View.OnClickListener(){

            @Override
            public void onClick(View v){
                finish();
            }
        });
    }

    private void initDialogView() {
        mDialogView= LayoutInflater.from(BlackListMainActivity.this).inflate(R.layout.add_blacklist_dialog, null);
        mNumberEditor = (EditText)mDialogView.findViewById(R.id.enter_number);
        mNameEditor = (EditText)mDialogView.findViewById(R.id.enter_name);
        mCallCheck = (CheckBox)mDialogView.findViewById(R.id.intercept_call_cb);
        mCallCheck.setChecked(true);
        mMmsCheck = (CheckBox)mDialogView.findViewById(R.id.intercept_sms_cb);
        mMmsCheck.setChecked(true);
    }

    private void addBlacklistDialog () {
        AlertDialog.Builder builder = new AlertDialog.Builder(BlackListMainActivity.this);
        builder.setTitle(R.string.add_blacklist_title);
        builder.setView(mDialogView);
        builder.setCancelable(false);
        builder.setPositiveButton(R.string.yes, new OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                String number = mNumberEditor.getText().toString();
                String name = mNameEditor.getText().toString();
                if (number == null || number.trim().length() == 0) {
                    Toast.makeText(BlackListMainActivity.this, R.string.add_invalid_toast, Toast.LENGTH_LONG).show();
                } else if (!mCallCheck.isChecked() && !mMmsCheck.isChecked()) {
                    Toast.makeText(BlackListMainActivity.this, R.string.set_intercept_toast, Toast.LENGTH_LONG).show();
                } else {
                    ContentValues values = new ContentValues();
                    if (name.length() > 0) {
                        values.put(DBConfig.BlackListInfo.NAME, name);
                    }
                    values.put(DBConfig.BlackListInfo.NUMBER, number);
                    //values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_CALL);
                    if (mCallCheck.isChecked() && mMmsCheck.isChecked()) {
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_ALL);
                    } else if (mCallCheck.isChecked() && !mMmsCheck.isChecked()) {
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_CALL);
                    } else {
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_SMS);
                    }
                    insertBlackList(values);
                }
            }
        });
        builder.setNegativeButton(R.string.no, null);
        builder.show();
    }

    private void editBlacklistDialog () {
        AlertDialog.Builder builder = new AlertDialog.Builder(BlackListMainActivity.this);
        builder.setTitle(R.string.add_blacklist_title);
        builder.setView(mDialogView);
        builder.setCancelable(false);
        ContentResolver resolver = getBaseContext().getContentResolver();
        Cursor cursor = resolver.query(DBConfig.BlackListInfo.CONTENT_URI,
                new String[]{DBConfig.BlackListInfo.NAME, DBConfig.BlackListInfo.NUMBER,
                DBConfig.BlackListInfo.INTERCEPT_TYPE}, DBConfig.BlackListInfo._ID + "=?",
                new String[] {itemId + ""}, null);
        try {
            while (cursor.moveToNext()) {
                mNumberEditor.setText(cursor.getString(cursor.getColumnIndex(DBConfig.BlackListInfo.NUMBER)));
                mNameEditor.setText(cursor.getString(cursor.getColumnIndex(DBConfig.BlackListInfo.NAME)));
                if (cursor.getInt(cursor.getColumnIndex(DBConfig.BlackListInfo.INTERCEPT_TYPE))
                        == DBConfig.BlackListInfo.BLOCK_TYPE_ALL) {
                    mCallCheck.setChecked(true);
                    mMmsCheck.setChecked(true);
                } else if (cursor.getInt(cursor.getColumnIndex(DBConfig.BlackListInfo.INTERCEPT_TYPE))
                        == DBConfig.BlackListInfo.BLOCK_TYPE_CALL) {
                    mCallCheck.setChecked(true);
                    mMmsCheck.setChecked(false);
                } else {
                    mCallCheck.setChecked(false);
                    mMmsCheck.setChecked(true);
                }
            }
        } finally {
            cursor.close();
        }
        builder.setPositiveButton(R.string.yes, new OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                String number = mNumberEditor.getText().toString();
                String name = mNameEditor.getText().toString();
                if (number == null || number.trim().length() == 0) {
                    Toast.makeText(BlackListMainActivity.this, R.string.add_invalid_toast, Toast.LENGTH_LONG).show();
                } else {
                    ContentValues values = new ContentValues();
                    if (name.length() > 0) {
                        values.put(DBConfig.BlackListInfo.NAME, name);
                    }
                    values.put(DBConfig.BlackListInfo.NUMBER, number);
                    //values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_CALL);
                    if (mCallCheck.isChecked() && mMmsCheck.isChecked()) {
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_ALL);
                    } else if (mCallCheck.isChecked() && !mMmsCheck.isChecked()) {
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_CALL);
                    } else if (!mCallCheck.isChecked() && mMmsCheck.isChecked()){
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_SMS);
                    } else {
                        values.put(DBConfig.BlackListInfo.INTERCEPT_TYPE, DBConfig.BlackListInfo.BLOCK_TYPE_CALL);
                    }
                    insertBlackList(values);
                }
            }
        });
        builder.setNegativeButton(R.string.no, null);
        builder.show();
    }
    //Judge if this number has been added to the blacklist, then need to update this record;
    //otherwise, add this number to the table of blacklistinfo.
    private void insertBlackList(ContentValues values) {
        ContentResolver cr = getBaseContext().getContentResolver();
        String number = values.getAsString(DBConfig.BlackListInfo.NUMBER);
        try {
            Cursor cursor = cr.query(DBConfig.BlackListInfo.CONTENT_URI, null, "number = ?", new String[] {number}, null);
            if (cursor.getCount() == 0) {
                cr.insert(DBConfig.BlackListInfo.CONTENT_URI, values);
            } else {
                cr.update(DBConfig.BlackListInfo.CONTENT_URI, values, "number = ?", new String[] {number});
            }
            cursor.close();
            mBlackListAdapter.swapCursor(getCursor());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public Cursor getCursor() {
        ContentResolver resolver = getContentResolver();
        Cursor cursor = resolver.query(DBConfig.BlackListInfo.CONTENT_URI,
                new String[]{DBConfig.BlackListInfo._ID, DBConfig.BlackListInfo.NAME,
                DBConfig.BlackListInfo.NUMBER,DBConfig.BlackListInfo.INTERCEPT_TYPE}, null, null, null);
        return cursor;
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        ContentResolver resolver = getContentResolver();
        switch(item.getItemId()) {
        case R.id.action_edit:
            initDialogView();
            editBlacklistDialog();
            break;
        case R.id.action_delete:
            resolver.delete(DBConfig.BlackListInfo.CONTENT_URI,
                    DBConfig.BlackListInfo._ID + "=?", new String[]{itemId+""});
            break;
        default:
            break;
        }
        return true;
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {
        // TODO Auto-generated method stub
        super.onCreateContextMenu(menu, v, menuInfo);
        AdapterContextMenuInfo menuInfoItem = (AdapterContextMenuInfo) menuInfo;
        itemId = menuInfoItem.id;
        MenuInflater inflater=BlackListMainActivity.this.getMenuInflater();
        inflater.inflate(R.menu.black_list_main, menu);
    }
}
