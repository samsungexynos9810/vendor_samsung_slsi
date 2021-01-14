/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.networktestmode;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class NetworkTestModeActivity extends Activity {

    private static final int DNS_QUERY_BLOCK = 0;
    private static final int APN_MODE = 1;
    private static final int VOLTE_MODE = 2;
    private static final int DUAL_VOLTE_MODE = 3;

    private ArrayList<String> menuList;
    private ArrayAdapter<String> adapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        menuList = new ArrayList<String>();
        menuList.add("[1] DNS Query Block");
        menuList.add("[2] PDN Up/Down");
        menuList.add("[3] VoLTE mode setting");
        menuList.add("[4] Dual VoLTE mode setting");

        adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, menuList);
        ListView list = (ListView) findViewById(R.id.list);
        list.setAdapter(adapter);
        list.setChoiceMode(ListView.CHOICE_MODE_SINGLE);

        list.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View v,
                    int position, long id) {
                selectMenuItem(position);
            }
        });
    }

    private void selectMenuItem(int position) {
        Intent intent = new Intent();
        switch (position) {
        case DNS_QUERY_BLOCK:
            intent.setClassName("com.samsung.slsi.telephony.networktestmode", "com.samsung.slsi.telephony.dnsblock.DnsBlockActivity");
            break;
        case APN_MODE:
            intent.setClassName("com.samsung.slsi.telephony.networktestmode", "com.samsung.slsi.telephony.apnmode.ApnModeActivity");
            break;
        case VOLTE_MODE:
            intent.setClassName("com.samsung.slsi.telephony.networktestmode", "com.samsung.slsi.telephony.voltesetting.VoLteModeActivity");
            break;
        case DUAL_VOLTE_MODE:
            intent.setClassName("com.samsung.slsi.telephony.networktestmode", "com.samsung.slsi.telephony.dualvolte.DualVolteSettingActivity");
            break;
        }
        intent.addCategory("android.intent.category.DEFAULT");
        startActivity(intent);
    }
}