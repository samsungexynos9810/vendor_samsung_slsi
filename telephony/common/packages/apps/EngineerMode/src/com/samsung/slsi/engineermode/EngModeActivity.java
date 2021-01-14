/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.engineermode;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class EngModeActivity extends Activity {

    private static final String TAG = "EngMode";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_eng_mode);

        String msg = "Back : Use back hard key or select option->back  ";
        Toast.makeText(EngModeActivity.this, msg, Toast.LENGTH_LONG).show();

        ListView listView = (ListView) findViewById(R.id.list);
        final String[] values = new String[] { "Menu", "Network Setting", "Monitoring" };
        ArrayAdapter<String> adapter  = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, android.R.id.text1, values);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView parent, View v, int position, long id) {

                Log.i(TAG, "setOnItemClickListener!!");
                Log.i(TAG, "position is :" + position);
                Log.i(TAG, "id is : " + id);

                // send request for displayed menu only
                if (position < values.length) {
                    Intent intent = new Intent(EngModeActivity.this, EngDetailActivity.class);
                    intent.putExtra("eng_mode", position);
                    intent.putExtra("eng_title", values[position]);
                    startActivity(intent);
                }
            }
        });
    }
}