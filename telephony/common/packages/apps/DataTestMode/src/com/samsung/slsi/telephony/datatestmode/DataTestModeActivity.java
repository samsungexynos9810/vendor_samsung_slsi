/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.datatestmode;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class DataTestModeActivity extends Activity {

    private static final int APN_SETTING_MODE =0;
    private static final int VZW_TEST_MODE = 1;
    private static final int DATA_RETRY_MODE = 2;

    private static final int APN_MODE = 3;
    private static final int DUAL_VOLTE_MODE = 4;
    private static final int DNS_QUERY_BLOCK = 5;
    private static final int VOLTE_MODE =6;

    private ArrayList<String> menuList;
    private ArrayAdapter<String> adapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        menuList = new ArrayList<String>();
        menuList.add("[1] APN setting");
        menuList.add("[2] Enable Test mode setting (VZW)");
        menuList.add("[3] Data Retry mode setting");
        //menuList.add("[4] PDN Up/Down");
        //menuList.add("[5] Dual VoLTE mode setting");
        //menuList.add("[6] DNS Query block");
        //menuList.add("[7] VoLTE mode setting");

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
        case APN_SETTING_MODE:
            intent.setClassName("com.samsung.slsi.telephony.datatestmode", "com.samsung.slsi.telephony.apnsettingmode.ApnSettingModeActivity");
            break;
        case VZW_TEST_MODE:
            intent.setClassName("com.samsung.slsi.telephony.datatestmode", "com.samsung.slsi.telephony.vzwtestmode.VzwTestModeActivity");
            break;
        case DATA_RETRY_MODE:
            intent.setClassName("com.samsung.slsi.telephony.datatestmode", "com.samsung.slsi.telephony.dataretrymode.DataRetryModeActivity");
            break;
/*
        case APN_MODE:
            intent.setClassName("com.samsung.slsi.telephony.datatestmode", "com.samsung.slsi.telephony.apnmode.ApnModeActivity");
            break;
        case DUAL_VOLTE_MODE:
            intent.setClassName("com.samsung.slsi.telephony.datatestmode", "com.samsung.slsi.telephony.dualvolte.DualVolteSettingActivity");
            break;
         case DNS_QUERY_BLOCK:
            intent.setClassName("com.samsung.slsi.telephony.datatestmode", "com.samsung.slsi.telephony.dnsblock.DnsBlockActivity");
            break;
         case VOLTE_MODE:
            intent.setClassName("com.samsung.slsi.telephony.datatestmode", "com.samsung.slsi.telephony.voltesetting.VoLteModeActivity");
            break;
*/
        }
        intent.addCategory("android.intent.category.DEFAULT");
        startActivity(intent);
    }
}
