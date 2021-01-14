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

import java.util.Set;

import com.android.slsi.blacklist.BatchLogAdapter.ViewHolder;

import android.app.Activity;
import android.content.ContentResolver;
import android.database.Cursor;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.MenuInflater;
import android.view.View;
import android.view.Window;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.Toast;
import android.util.Log;

public class CallLogBatchActivity extends Activity {

    private ImageButton mSelectAllBtn;
    private ImageButton mDeleteBtn;
    private ImageButton mNotSelectAllBtn;
    private ImageButton mBack;
    private ListView mListview;
    private BatchLogAdapter mBatchLogAdapter;
    private boolean isAllSelected = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.call_log_batch_list);
        mListview = (ListView)findViewById(R.id.log_batch_list_number);
        mBatchLogAdapter = new BatchLogAdapter(this,getCursor());
        mListview.setAdapter(mBatchLogAdapter);
        mSelectAllBtn = (ImageButton)findViewById(R.id.log_select_all_button);
        mNotSelectAllBtn = (ImageButton)findViewById(R.id.log_not_select_all_button);
        mDeleteBtn = (ImageButton)findViewById(R.id.log_delete_button);
        mBack = (ImageButton)findViewById(R.id.log_batch_title_back_btn);
        mSelectAllBtn.setVisibility(View.VISIBLE);
        mNotSelectAllBtn.setVisibility(View.GONE);
        mSelectAllBtn.setOnClickListener(new OnClickListener(){

            @Override
            public void onClick(View v){
                mSelectAllBtn.setVisibility(View.GONE);
                mNotSelectAllBtn.setVisibility(View.VISIBLE);
                Set<Integer> set = BatchLogAdapter.isSelected.keySet();
                for (Integer itemId : set) {
                    BatchLogAdapter.isSelected.put(itemId, true);
                }
                mBatchLogAdapter.notifyDataSetChanged();
                isAllSelected = true;
            }
        });
        mNotSelectAllBtn.setOnClickListener(new OnClickListener(){

            @Override
            public void onClick(View v){
                mSelectAllBtn.setVisibility(View.VISIBLE);
                mNotSelectAllBtn.setVisibility(View.GONE);
                Set<Integer> set = BatchLogAdapter.isSelected.keySet();
                for (Integer itemId : set) {
                    BatchLogAdapter.isSelected.put(itemId, false);
                }
                mBatchLogAdapter.notifyDataSetChanged();
                isAllSelected = false;
            }
        });
        mDeleteBtn.setOnClickListener(new OnClickListener(){

            @Override
            public void onClick(View v){
                ContentResolver resolver = getContentResolver();
                if (isAllSelected) {
                    resolver.delete(DBConfig.CallLogInfo.CONTENT_URI, null, null);
                    Set<Integer> set = BatchLogAdapter.isSelected.keySet();
                    for (Integer itemId : set) {
                        BatchLogAdapter.isSelected.put(itemId, false);
                    }
                } else {
                    Set<Integer> set = BatchLogAdapter.isSelected.keySet();
                    for (Integer itemId : set) {
                        if (BatchLogAdapter.isSelected.get(itemId)) {
                            resolver.delete(DBConfig.CallLogInfo.CONTENT_URI,
                                  DBConfig.CallLogInfo._ID + "=?", new String[]{itemId+""});
                          BatchLogAdapter.isSelected.put(itemId, false);
                      }
                    }
                }
                mSelectAllBtn.setVisibility(View.VISIBLE);
                mNotSelectAllBtn.setVisibility(View.GONE);
            }
        });
        mBack.setOnClickListener(new OnClickListener(){

            @Override
            public void onClick(View v){
                finish();
            }
        });
        mListview.setOnItemClickListener(new OnItemClickListener(){

            @Override
            public void onItemClick(AdapterView<?> parent, View view,
                    int position, long id) {
                // TODO Auto-generated method stub
                ViewHolder vHollder = (ViewHolder) view.getTag();
                vHollder.checkbox.toggle();
                BatchLogAdapter.isSelected.put((int)id,
                        vHollder.checkbox.isChecked());
                int mSelectCount =0;
                for (Boolean value : BatchLogAdapter.isSelected.values()) {
                    if (value) {
                        ++mSelectCount;
                    }
                }
                if (mListview.getCount() > 0 && mListview.getCount() == mSelectCount) {
                    isAllSelected = true;
                    mSelectAllBtn.setVisibility(View.GONE);
                    mNotSelectAllBtn.setVisibility(View.VISIBLE);
                } else {
                    isAllSelected = false;
                    mSelectAllBtn.setVisibility(View.VISIBLE);
                    mNotSelectAllBtn.setVisibility(View.GONE);
                }
            }
        });
    }

    public Cursor getCursor() {
        ContentResolver resolver = getContentResolver();
        Cursor cursor = resolver.query(DBConfig.CallLogInfo.CONTENT_URI,
                new String[]{DBConfig.CallLogInfo._ID, DBConfig.CallLogInfo.NUMBER, DBConfig.CallLogInfo.DATE,
                DBConfig.CallLogInfo.LOCATION}, null, null, null);
        return cursor;
    }
}
