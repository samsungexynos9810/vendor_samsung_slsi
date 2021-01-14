/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.testmode;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class TestModeActivity extends Activity {

    //private static final int USIM_USAT_TEST = 0;
    private static final int CHECKING_APN_LIST = 0;
    private static final int ADD_NEW_APN = 1;
    private static final int SETTING_PORT_FORWARDING = 2;
    private static final int SMS_PREF_MODE = 3;
    private static final int BIP_TEST_MODE = 4;
    private static final int PLAT_SUBNET_MODE = 5;
    private static final int SGC_TEST_MODE = 6;
    private static final int EMBMS_TEST_MODE = 7;
    private static final int NETWORK_MODE = 8;

    private static final String ACTION_USIM_USAT_TEST = "com.samsung.slsi.action.USIMUSATTEST";
    private static final String ACTION_APN_LIST = "com.samsung.slsi.action.APNLIST";
    private static final String ACTION_PORT_FORWARDING = "com.samsung.slsi.action.PORTFORWARDING";
    private static final String ACTION_SMS_PREF_MODE = "com.samsung.slsi.action.SMSPREFMODE";
    private static final String ACTION_BIP_TEST = "com.samsung.slsi.action.BIPTEST";
    private static final String ACTION_PLAT_SUBNET = "com.samsung.slsi.action.PLAT_SUBNET";
    private static final String ACTION_SGC_TEST = "com.samsung.slsi.action.SGC_TEST";
    private static final String ACTION_EMBMS_TEST = "com.samsung.slsi.action.EMBMSTEST";
    private static final String ACTION_NETWORK_MODE = "com.samsung.slsi.action.NETWORK_MODE";

    private ArrayList<String> menuList;
    private ArrayAdapter<String> adapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        menuList = new ArrayList<String>();
        //menuList.add("[1] USIM/USAT Test");
        menuList.add("[1] Checking APN List");
        menuList.add("[2] Add new APN");
        menuList.add("[3] NS Test - Port Forwarding");
        menuList.add("[4] SMS Preferred Mode");
        menuList.add("[5] BIP Test ");
        menuList.add("[6] Use Predefined PLAT_SUBNET");
        menuList.add("[7] SGC Config Setting");
        menuList.add("[8] eMBMS Test");
        menuList.add("[9] Network Mode Setting");

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
        //case USIM_USAT_TEST:
        //    intent.setAction(ACTION_USIM_USAT_TEST);
        //    intent.addCategory("android.intent.category.DEFAULT");
        //    startActivity(intent);
        //    break;

        case CHECKING_APN_LIST:
            intent.setAction(ACTION_APN_LIST);
            break;
        case ADD_NEW_APN:
            intent.setAction(Intent.ACTION_INSERT);
            intent.setData(Uri.parse("content://telephony/carriers"));
            break;
        case SETTING_PORT_FORWARDING:
            intent.setAction(ACTION_PORT_FORWARDING);
            break;
        case SMS_PREF_MODE:
            intent.setAction(ACTION_SMS_PREF_MODE);
            break;
        case BIP_TEST_MODE:
            intent.setAction(ACTION_BIP_TEST);
            break;
        case PLAT_SUBNET_MODE:
            intent.setAction(ACTION_PLAT_SUBNET);
            break;
        case SGC_TEST_MODE:
            intent.setAction(ACTION_SGC_TEST);
            break;
        case EMBMS_TEST_MODE:
            intent.setAction(ACTION_EMBMS_TEST);
            break;
        case NETWORK_MODE:
            intent.setAction(ACTION_NETWORK_MODE);
            break;
        }
        intent.addCategory("android.intent.category.DEFAULT");
        startActivity(intent);
    }
}
