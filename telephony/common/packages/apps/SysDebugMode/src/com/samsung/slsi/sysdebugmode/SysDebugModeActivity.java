/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.sysdebugmode;

import java.util.ArrayList;

import com.samsung.slsi.sysdebugmode.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.AdapterView.OnItemClickListener;

public class SysDebugModeActivity extends Activity {

    private static final int LOG_DUMP = 0;
    private static final int SET_DEBUG_LEVEL = 1;
    private static final int SET_RILLOG_LEVEL = 2;

    public static final String ACTION_LOG_DUMP = "com.samsung.slsi.action.LOGDUMP";
    public static final String ACTION_DEBUG_LEVEL = "com.samsung.slsi.action.DEBUGLEVEL";
    public static final String ACTION_RILLOG_LEVEL = "com.samsung.slsi.action.RILLOGLEVEL";

    private ArrayList<String> menuList;
    private ArrayAdapter<String> adapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        menuList = new ArrayList<String>();
        menuList.add("[1] Log DUMP(AP/CP)");
        menuList.add("[2] Set CP Debug Trace Mode");
        //menuList.add("[3] Set Ril Log Level");

        adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, menuList);
        ListView list = (ListView)findViewById(R.id.list);
        list.setAdapter(adapter);
        list.setChoiceMode(ListView.CHOICE_MODE_SINGLE);

        list.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
                selectMenuItem(position);
            }
        });
    }

    private void selectMenuItem(int position)
    {
        Intent intent = new Intent();
        switch(position)
        {
        case LOG_DUMP:
            intent.setAction(ACTION_LOG_DUMP);
            intent.addCategory("android.intent.category.DEFAULT");
            startActivity(intent);
            break;

        case SET_DEBUG_LEVEL:
            intent.setAction(ACTION_DEBUG_LEVEL);
            intent.addCategory("android.intent.category.DEFAULT");
            startActivity(intent);
            break;

        //case SET_RILLOG_LEVEL:
        //    intent.setAction(ACTION_RILLOG_LEVEL);
        //    intent.addCategory("android.intent.category.DEFAULT");
        //    startActivity(intent);
        //    break;
        }
    }
}
