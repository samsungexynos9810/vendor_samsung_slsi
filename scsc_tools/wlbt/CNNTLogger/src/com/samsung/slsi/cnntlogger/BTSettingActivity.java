package com.samsung.slsi.cnntlogger;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.widget.AdapterView.OnItemClickListener;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.CheckedTextView;

public class BTSettingActivity extends Activity {

    private ListView mListView;
    private String mModeList[];
    private CNNTUtils mUtils;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list_bt);

        mUtils = new CNNTUtils(getApplicationContext());
        mModeList = getResources().getStringArray(R.array.bt_ll_modes);
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this,
                android.R.layout.simple_list_item_multiple_choice, mModeList);

        mListView = findViewById(R.id.list_bt);
        mListView.setAdapter(adapter);
        mListView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
        mListView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                CheckedTextView ctv = (CheckedTextView)view;
                mUtils.setPreference(CmdDefine.KEY_BT_USE_DEFAULT_MODE, false);
                if (ctv.isChecked()) {
                    mUtils.setPreference(mModeList[position], true);
                } else {
                    mUtils.setPreference(mModeList[position], false);
                }
            }
        });

        refreshListView();
    }

    private void refreshListView() {
        if (mUtils.getPreference(CmdDefine.KEY_BT_USE_DEFAULT_MODE, true)) {
            // Link Manager Protocol
            mUtils.setPreference(mModeList[0], true);
            // Link Layer Control Packets
            mUtils.setPreference(mModeList[4], true);
        }

        for (int i = 0; i < mModeList.length; i++) {
            mListView.setItemChecked(i, mUtils.getPreference(mModeList[i], false));
        }
    }
}
