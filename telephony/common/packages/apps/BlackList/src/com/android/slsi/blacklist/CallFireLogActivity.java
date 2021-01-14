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

import java.util.ArrayList;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.util.Log;
import android.view.ContextMenu;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.ListView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.util.DisplayMetrics;
import android.graphics.Matrix;
import android.view.animation.Animation;
import android.view.animation.TranslateAnimation;
import android.widget.ImageView;
import android.os.Handler;
import android.os.Message;
import android.widget.CursorAdapter;




public class CallFireLogActivity extends Activity implements OnItemClickListener{

    private final static String LOG_TAG = "CallFireLogActivity";

    private static final int EVENT_QUERY_SUCCESS = 1;
    private static final int EVENT_DELETE_COMPLETE = 2;

    // tab index
    private static final int INDEX_TAB_SMS = 1;
    private static final int INDEX_TAB_CALL = 0;

    // tab var
    private ViewPager tabPager;
    private ImageView scrollBar;
    private ArrayList<View> listPagerContent;
    private int tabOffset = 0;
    private int scrollBarWidth = 0;
    private int curTabIndex = 0;

    // list view
    private ListView listPrivacySms;
    private ListView listPrivacyCall;
    private TextView txtSmsTab;
    private TextView txtCallTab;

    private ListView mListview;
    private CallFireLogAdapter mCallFireLogAdapter;
    private SmsFireLogAdapter mSmsFireLogAdapter;
    private long itemId;
    private ImageButton mBack;

    private Thread mQueryThread;
    private boolean bSmsQuery = false;
    private boolean bCallQuery = false;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(android.os.Message msg) {
           Log.d(LOG_TAG, "handleMessage()...msg: " + msg);
            switch (msg.what) {
            case EVENT_QUERY_SUCCESS:
                setListView();
                updateListView();
                break;
            case EVENT_DELETE_COMPLETE:
                updateListView();
                break;
            default:
                break;
            }
        }
    };
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.call_log_list);

        initTabText();
        initScrollBar();
        initViewPager();
        // query
        curTabIndex = INDEX_TAB_CALL;
        tabPager.setCurrentItem(curTabIndex);
        queryPrivacyInfo(curTabIndex);
        mBack = (ImageButton)findViewById(R.id.log_title_back_btn);
        mBack.setOnClickListener(new OnClickListener(){

            @Override
            public void onClick(View v){
                finish();
            }
        });
    }

    // init tab
    private void initTabText() {
        Log.d(LOG_TAG, "initTabText()...");
        txtSmsTab = (TextView)findViewById(R.id.txt_sms_tab);
        txtSmsTab.setOnClickListener(new TabClickLisener(INDEX_TAB_SMS));
        txtCallTab = (TextView)findViewById(R.id.txt_call_tab);
        txtCallTab.setOnClickListener(new TabClickLisener(INDEX_TAB_CALL));
    }
    // OnClickLisener: switch tab
    public class TabClickLisener implements View.OnClickListener {

        private int nIndex = -1;
        public TabClickLisener(int nIndex) {
            this.nIndex = nIndex;
        }
        @Override
        public void onClick(View v) {
            Log.d(LOG_TAG, "onClick()...index:" + nIndex);
            tabPager.setCurrentItem(nIndex);
        }
    }
    // init pager
    private void initViewPager() {
        Log.d(LOG_TAG, "initViewPager()...");
        tabPager = (ViewPager)findViewById(R.id.list_view_pager);
        listPagerContent = new ArrayList<View>();
        LayoutInflater inflater = getLayoutInflater();
        View viewCall = inflater.inflate(R.layout.layout_callsms_content, null);
        listPrivacyCall = (ListView)viewCall.findViewById(R.id.call_log_list_number);
        this.registerForContextMenu(listPrivacyCall);
        listPagerContent.add(viewCall);

        View viewSms = inflater.inflate(R.layout.layout_callsms_content, null);
        listPrivacySms = (ListView)viewSms.findViewById(R.id.call_log_list_number);
        listPrivacySms.setOnItemClickListener(this);
        this.registerForContextMenu(listPrivacySms);
        listPagerContent.add(viewSms);

        tabPager.setAdapter(new ContentPagerAdaper(listPagerContent));
        tabPager.setOnPageChangeListener(new ContentPagerChangeListener());
    }

 // init scrool bar
    private void initScrollBar() {
        Log.d(LOG_TAG, "initScrollBar()...");
        scrollBar = (ImageView) findViewById(R.id.img_scroll_bar);
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        // init pos offset
        scrollBarWidth = dm.widthPixels/2;
        tabOffset = 0;
        Matrix matrix = new Matrix();
        matrix.postTranslate(tabOffset, 0);
        scrollBar.setImageMatrix(matrix); // init pos
    }
    public class ContentPagerChangeListener implements OnPageChangeListener {
        private int firstTabPos = tabOffset;
        private int secondTabPos = scrollBarWidth ; // tab 1 -> tab 2
        @Override
        public void onPageScrollStateChanged(int arg0) {
            Log.d(LOG_TAG, "onPageScrollStateChanged()...");
        }

        @Override
        public void onPageScrolled(int arg0, float arg1, int arg2) {
            Log.d(LOG_TAG, "onPageScrolled()...");
        }

        @Override
        public void onPageSelected(int arg0) {
            Log.d(LOG_TAG, "onPageSelected()...pos:" + arg0);
            Log.d(LOG_TAG, "first:" + firstTabPos + " second:" + secondTabPos );
            Animation animation = null;
            TextView newSelTab = null;
            TextView oldSelTab = null;
            switch (arg0) {
            case 0:
                newSelTab =txtCallTab ;
                if (curTabIndex == 1) {
                    oldSelTab = txtSmsTab;
                    animation = new TranslateAnimation(secondTabPos, 0,  0, 0);
                }
                break;
            case 1:
                newSelTab = txtSmsTab;
                if (curTabIndex == 0) {
                    oldSelTab = txtCallTab;
                    animation = new TranslateAnimation(0, secondTabPos, 0, 0);
                }
                break;
            }
            // change tab title color
            newSelTab.setTextColor(getResources().getColorStateList(R.drawable.selector_tabtitle_color_reverse));
            oldSelTab.setTextColor(getResources().getColorStateList(R.drawable.selector_tabtitle_color));
            // start animation
            curTabIndex = arg0;
            queryPrivacyInfo(curTabIndex);
            animation.setFillAfter(true);
            animation.setDuration(300);
            scrollBar.startAnimation(animation);
        }
    }


 // ViewPager adapter
    public class ContentPagerAdaper extends PagerAdapter {
        public ArrayList<View> listViews;

        public ContentPagerAdaper(ArrayList<View> listViews) {
            this.listViews = listViews;
        }

        @Override
        public void destroyItem(ViewGroup container, int position, Object object) {
            container.removeView(listViews.get(position));
        }

        @Override
        public Object instantiateItem(ViewGroup container, int position) {
            container.addView(listViews.get(position), 0);
            return listViews.get(position);
        }

        @Override
        public int getCount() {
            return listViews.size();
        }

        @Override
        public boolean isViewFromObject(View arg0, Object arg1) {
            // TODO Auto-generated method stub
            return arg0 == (arg1);
        }
    }

    /*
     * query privacy info
     */
    private void queryPrivacyInfo(int index) {
        Log.d(LOG_TAG, "queryPrivacyInfo()...index:" + index);
        // check whether need to query
        if (bCallQuery && bSmsQuery) {
            updateListView();
            return;
        }
        switch (index) {
        case INDEX_TAB_SMS:
            if (!bSmsQuery) {
                queryPrivacyInfoThread(INDEX_TAB_SMS);
            }
            break;
        case INDEX_TAB_CALL:
            if (!bCallQuery) {
                queryPrivacyInfoThread(INDEX_TAB_CALL);
            }
            break;
        default:
            break;
        }
    }
    /*
     * query privacy info through thread
     */
    private void queryPrivacyInfoThread(final int index) {
       Log.d(LOG_TAG, "queryPrivacyInfoThread()...index:" + index);
        mQueryThread = new Thread(new Runnable() {
            @SuppressWarnings("deprecation")
            @Override
            public void run() {
                Log.d(LOG_TAG, "run()...");
                // query info
                try {
                    switch (index) {
                    case INDEX_TAB_SMS:
                       getCursorSms();
                        Log.d(LOG_TAG, "sms count:" + getCursorSms().getCount());
                        break;
                    case INDEX_TAB_CALL:
                        getCursorCall();
                        Log.d(LOG_TAG, "calllog count:" + getCursorCall().getCount());
                        break;
                    }
                }  catch(Exception e) {
                    e.printStackTrace();
                }
                mHandler.sendEmptyMessage(EVENT_QUERY_SUCCESS);
            }
        });

        // start the thread of query running app
        mQueryThread.start();
    }


    private void setListView() {
        Log.d(LOG_TAG, "setListView()...index:" + curTabIndex);
        switch (curTabIndex) {
        case INDEX_TAB_SMS:
            mSmsFireLogAdapter = new SmsFireLogAdapter(this,getCursorSms());
            listPrivacySms.setAdapter(mSmsFireLogAdapter);
            bSmsQuery = true;
            break;
        case INDEX_TAB_CALL:
            mCallFireLogAdapter = new CallFireLogAdapter(this,getCursorCall());
            listPrivacyCall.setAdapter(mCallFireLogAdapter);
            bCallQuery = true;
            break;
        }
    }
    /*
     * update list view
     */
    private void updateListView() {
        Log.d(LOG_TAG, "updateListView()...index:" + curTabIndex);
        CursorAdapter adapter = null;
        switch (curTabIndex) {
        case INDEX_TAB_SMS:
            adapter = mSmsFireLogAdapter;
            break;
        case INDEX_TAB_CALL:
            adapter = mCallFireLogAdapter;
            break;
        }
        if (null != adapter) {
            adapter.notifyDataSetChanged();
        }
    }

    public Cursor getCursorCall() {
        ContentResolver resolver = getContentResolver();
        Cursor cursor = resolver.query(DBConfig.CallLogInfo.CONTENT_URI,
                new String[]{DBConfig.CallLogInfo._ID, DBConfig.CallLogInfo.NUMBER, DBConfig.CallLogInfo.DATE,
                DBConfig.CallLogInfo.LOCATION}, null, null, null);
        return cursor;
    }

    public Cursor getCursorSms() {
        ContentResolver resolver = getContentResolver();
        Cursor cursor = resolver.query(DBConfig.SmsInfo.CONTENT_URI,
                new String[]{DBConfig.SmsInfo._ID, DBConfig.SmsInfo.NUMBER, DBConfig.SmsInfo.DATE,
                DBConfig.SmsInfo.BODY}, null, null, DBConfig.SmsInfo.NUMBER_DATE_SORT_ORDER);
        return cursor;
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        Log.d(LOG_TAG, "onItemClick()...");
        switch (curTabIndex) {
        case INDEX_TAB_SMS:
            Log.d(LOG_TAG, "onItemClick()...Sms list visible");
            mSmsFireLogAdapter.showOrHideSmsBody(arg1);
            break;
        case INDEX_TAB_CALL:
            break;
        default:
            break;
        }
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        Log.d(LOG_TAG, "onContextItemSelected()...index:" + curTabIndex);
        ContentResolver resolver = getContentResolver();
        if(curTabIndex == INDEX_TAB_CALL ){
            switch(item.getItemId()) {
            case R.id.action_batch_delete:
                Intent intent = new Intent(CallFireLogActivity.this, CallLogBatchActivity.class);
                startActivity(intent);
                break;
            case R.id.action_delete:
                resolver.delete(DBConfig.CallLogInfo.CONTENT_URI,
                        DBConfig.CallLogInfo._ID + "=?", new String[]{itemId+""});
                break;
            default:
                break;
            }

        }else if(curTabIndex == INDEX_TAB_SMS){
            switch(item.getItemId()) {
            case R.id.action_batch_delete:
                Intent intent = new Intent(CallFireLogActivity.this, SmsLogBatchActivity.class);
                startActivity(intent);
                break;
            case R.id.action_delete:
                resolver.delete(DBConfig.SmsInfo.CONTENT_URI,
                        DBConfig.SmsInfo._ID + "=?", new String[]{itemId+""});
                break;
            default:
                break;
            }
        }
        mHandler.sendEmptyMessage(EVENT_DELETE_COMPLETE);
        return true;

    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {
        // TODO Auto-generated method stub
        super.onCreateContextMenu(menu, v, menuInfo);
        AdapterContextMenuInfo menuInfoItem = (AdapterContextMenuInfo) menuInfo;
        itemId = menuInfoItem.id;
        MenuInflater inflater=CallFireLogActivity.this.getMenuInflater();
        inflater.inflate(R.menu.call_log_menu, menu);
    }
}
