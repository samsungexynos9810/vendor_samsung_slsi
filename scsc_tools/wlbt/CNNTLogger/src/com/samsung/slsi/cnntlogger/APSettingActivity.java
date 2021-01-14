package com.samsung.slsi.cnntlogger;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.util.SparseBooleanArray;
import android.widget.AdapterView.OnItemClickListener;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class APSettingActivity extends Activity {

    SharedPreferences sharedPref;

    private ListView listView;

    private String apList[];

    CNNTUtils mUtils;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list_ap);

        sharedPref = this.getSharedPreferences(CmdDefine.KEY_AP_TYPE, Context.MODE_PRIVATE);
        apList = getResources().getStringArray(R.array.ap_logtype);
        mUtils = new CNNTUtils(getApplicationContext());

        ArrayAdapter<String> adapter = new ArrayAdapter<>(this,
                android.R.layout.simple_list_item_multiple_choice, apList);
        listView = findViewById(R.id.list_ap);
        listView.setAdapter(adapter);
        listView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);

        listView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                SparseBooleanArray sparse = listView.getCheckedItemPositions();
                if (sparse.get(0) && position == 0) {
                    for (int i = 1; i < apList.length; i++) {
                        listView.setItemChecked(i, true);
                    }
                } else if (!sparse.get(0) && position == 0) {
                    for (int i = 1; i < apList.length; i++) {
                        listView.setItemChecked(i, false);
                    }
                } else {
                    listView.setItemChecked(0, false);
                }
            }
        });

        checkInitValue();

        for (int i = 0; i < apList.length; i++) {
            listView.setItemChecked(i, mUtils.getPreference(apList[i], false));
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_ap_list, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        int id = item.getItemId();
        if (id == R.id.action_save) {
            SparseBooleanArray sparse = listView.getCheckedItemPositions();
            int cnt = 0;
            for (int i = 0; i < apList.length; i++) {
                if (sparse.get(i)) {
                    mUtils.setPreference(apList[i], true);
                    cnt++;
                } else {
                    mUtils.setPreference(apList[i], false);
                }
            }

            if (cnt == 0) {
                Toast.makeText(this, getString(R.string.nothing_select), Toast.LENGTH_SHORT).show();
                return super.onOptionsItemSelected(item);
            }
        }
        finish();   // exit activity after save ap logging type
        return super.onOptionsItemSelected(item);
    }

    public void checkInitValue() {
        boolean initial = true;
        for (int i = 0; i < apList.length; i++) {
            if (mUtils.getPreference(apList[i], false)) {
                initial = false;
                break;
            }
        }
        if (initial) {
            Log.d(CmdDefine.LOGTAG, "No previous setting data >> set to default");
            for (int i = 0; i < apList.length; i++) {
                mUtils.setPreference(apList[i], true);
            }
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }
}
