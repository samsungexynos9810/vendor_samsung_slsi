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

import java.io.EOFException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.graphics.Color;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.samsung.slsi.telephony.oem.OemRil;
import com.samsung.slsi.telephony.oem.io.DataReader;
import com.samsung.slsi.telephony.oem.io.DataWriter;
import com.samsung.slsi.telephony.oem.util.StringUtil;


public class EngDetailActivity extends Activity {

    private static final String TAG = "EngDetailActivity";
    private static final int EVENT_RIL_CONNECTED = 100;
    private static final int EVENT_RIL_DISCONNECTED = 101;
    private static final int EVENT_DISPLAY_ENG = 102;
    private static final int EVENT_SEND_RIL_DATA_DONE = 103;
    private static final int EVENT_CP_RESPONSE_TIMEOUT = 104;
    private static final int EVENT_TITLE_CHANGED = 105;

    /* RIL request */
    private static final int RILC_REQ_MISC_SET_ENG_MODE = 2;    // Engineer Mode Set Command
    private static final int RILC_REQ_MISC_SCREEN_LINE = 3;     // Screen Line Select Command
    private static final int RILC_REQ_MISC_SET_STR_DATA = 6;   // Input Key Value Command

    private Timer mTimer;
    private ListView mList;
    private ProgressBar mProgressBar;
    private TextView mTxtMsg;
    private String mTopMenu = null;
    private OemRil mOemRil;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_RIL_CONNECTED: {
                Log.d(TAG, "RIL connected");
                Intent intent = getIntent();
                int engMode = intent.getIntExtra("eng_mode", 0);
                String title = intent.getStringExtra("eng_title");
                getActionBar().setTitle(title);
                sendRilData(RILC_REQ_MISC_SET_ENG_MODE, engMode);
                mDefaultTitle = title;
                break;
            }
            case EVENT_RIL_DISCONNECTED: {
                Log.d(TAG, "RIL disconnected");
                finish();
                break;
            }
            case EVENT_DISPLAY_ENG: {
                Log.d(TAG, "Receive display eng indication");
                AsyncResult ar = (AsyncResult) msg.obj;
                byte[] data = StringUtil.arrayListToPrimitiveArray((ArrayList<Byte>)ar.result);
                displayEngList2(data);
                break;
            }
            case EVENT_TITLE_CHANGED: {
                Log.d(TAG, "EVENT_TITLE_CHANGED");
                String title = (String)msg.obj;
                if (!TextUtils.isEmpty(title)) {
                    getActionBar().setTitle(title);
                }
                break;
            }
            case EVENT_SEND_RIL_DATA_DONE:
                Log.i(TAG, "EVENT_SEND_RIL_DATA_DONE");
                break;
            case EVENT_CP_RESPONSE_TIMEOUT:
                Log.e(TAG, "CP response timeout!");
                endServiceMode();
                break;

            }
        }
    };

    private String mDefaultTitle;
    private EngModeScreen mScreen;


    public static final int BLACK = 0;
    public static final int GRAY = 1;
    public static final int SILVER = 2;
    public static final int WHITE = 3;
    public static final int RED = 4;
    public static final int MARON = 5;
    public static final int YELLOW = 6;
    public static final int OLIVE = 7;
    public static final int LIME = 8;
    public static final int GREEN = 9;
    public static final int CYAN = 10;
    public static final int TEAL = 11;
    public static final int BLUE = 12;
    public static final int NAVY = 13;
    public static final int MAGENTA = 14;
    public static final int PURPLE = 15;

    public static int convertAndroidColor(int colorIndex) {
        // TODO convert colorIndex to Android color
        switch (colorIndex) {
        case BLACK:
            return Color.BLACK;
        case GRAY:
        case SILVER:
            return Color.GRAY;
        case WHITE:
            return Color.WHITE;
        case RED:
            return Color.RED;
        case MARON:
            return Color.DKGRAY;
        case YELLOW:
            return Color.YELLOW;
        case OLIVE:
        case GREEN:
            return Color.GREEN;
        case LIME:
            return Color.YELLOW;
        case CYAN:
            return Color.CYAN;
        case TEAL:
            return Color.BLACK;
        case BLUE:
        case NAVY:
            return Color.BLUE;
        case MAGENTA:
        case PURPLE:
            return Color.MAGENTA;
        } // end switch ~
        return Color.BLACK;
    }

    class Line {
        private int mPos;
        private int mIndex;
        private String mText;
        private int mTextColor;

        Line(int pos, int index, String text) {
            mPos = pos;
            mIndex = index;
            mText = text;
        }

        int getPosition() { return mPos; }
        int getIndex() { return mIndex; }

        void setText(String text) {
            mText = text;
        }
        String getText() { return mText; }
        void setTextColor(int colorIndex) {
            mTextColor = colorIndex;
        }
        int getTextColor() { return mTextColor; }

        @Override
        public String toString() {
            return "Line:{pos=" + mPos + " text=" + mText + " color=" + mTextColor + "}";
        }
    }

    class EngModeScreen {
        private String mTitle;  // use in further
        private boolean mMark;
        private int mLevel;
        private ArrayList<Line> mLines;

        EngModeScreen() {
            this(null);
        }

        EngModeScreen(EngModeScreen screen) {
            if (screen != null) {
                mTitle = screen.mTitle;
                mMark = screen.mMark;
            }
            else {
                setTitle(mDefaultTitle);
                mMark = false;
            }
            mLevel = -1;
            mLines = new ArrayList<>();
        }

        public int getCount() {
            return (mLines == null) ? 0 : mLines.size();
        }

        public void setTitle(String title) {
            if (!TextUtils.isEmpty(title)) {
                mTitle = title;
            }
        }

        public String getTitle() {
            // a title with mark
            String title = mTitle;
            if (mMark) {
                title += " [!]";
            }
            return title;
        }

        public String getFirstLineText() {
            if (mLines != null && mLines.size() > 0) {
                return mLines.get(0).getText();
            }
            return null;
        }

        public void setMark(boolean mark) {
            mMark = mark;
        }

        public List<Line> getLines() {
            return mLines;
        }

        public Line getLine(int pos) {
            if (mLines == null) {
                return null;
            }

            return mLines.get(pos);
        }

        public int getLevel() { return mLevel; }
        public void setLevel(int level) { mLevel = level; }

        public void setLines(List<Line> lines) {
            mLines = new ArrayList<>(lines);
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("EngModeScreen:{");
            sb.append(" mTitle=" + mTitle);
            sb.append(" mMark=" + mMark);
            sb.append(" mLevel=" + mLevel);
            sb.append("\n");
            for (Line line : mLines) {
                sb.append(" " + line + "\n");
            }
            sb.append("}");
            return sb.toString();
        }
    }

    class EngModeScreenAdapter extends BaseAdapter {

        @Override
        public int getCount() {
            if (mScreen == null)
                return 0;
            return mScreen.getCount();
        }

        @Override
        public Line getItem(int position) {
            if (mScreen == null)
                return null;
            return mScreen.getLine(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                LayoutInflater inflater = LayoutInflater.from(EngDetailActivity.this);
                convertView = inflater.inflate(R.layout.list_eng_mode, null);
            }
            TextView msg = (TextView)convertView.findViewById(R.id.txt_item);
            if (msg != null) {
                Line item = getItem(position);
                if (item != null) {
                    msg.setText(item.getText());
                    msg.setTextColor(convertAndroidColor(item.getTextColor()));
                }
                else {
                    Log.d(TAG, "pos is " + position + " but no item");
                }
            }
            return convertView;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_eng_mode);

        StringBuilder sb = new StringBuilder();
        sb.append("Please! Wait...\n");
        sb.append("If there is no response for 5 seconds, main page will be opened.\n");
        sb.append("Then, try again!\n");
        mTxtMsg = (TextView) findViewById(R.id.txt_msg);
        mTxtMsg.setText(sb.toString());
        mProgressBar = (ProgressBar) findViewById(R.id.progress_bar);
        mList = (ListView) findViewById(R.id.list);
        mList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView parent, View v, int position, long id) {
                Log.i(TAG, "setOnItemClickListener!!");
                Log.i(TAG, "position is :" + position);
                Log.i(TAG, "id is : " + id);

                if (mScreen != null) {
                    Line line = mScreen.getLine(position);
                    if (line != null) {
                        sendRilData(RILC_REQ_MISC_SCREEN_LINE, line.getIndex());
                    }
                }
                else {
                    Log.d(TAG, "No EngModeScreen instance yet");
                }
            }
        });
        connectToOemRilService();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mOemRil != null) {
            mOemRil.unregisterForOemRilConnected(mHandler);
            mOemRil.unregisterForOemRilDisconnected(mHandler);
            mOemRil.unregisterDisplayEng(mHandler);
            mOemRil.detach();
        }
    }

    private void connectToOemRilService() {
        mOemRil = OemRil.init(getApplicationContext(), 0); // default SIM1
        if (mOemRil == null) {
            Log.d(TAG, "connectToOemRilService mOemRil is null");
        } else {
            mOemRil.registerForOemRilConnected(mHandler, EVENT_RIL_CONNECTED);
            mOemRil.registerForOemRilDisconnected(mHandler, EVENT_RIL_DISCONNECTED);
            mOemRil.registerDisplayEng(mHandler, EVENT_DISPLAY_ENG, null);
        }
    }

    private void sendRilData(int requestId, int data) {
        mTimer = new Timer(true);
        mTimer.schedule(new UpdateTimeTask(), 5000);
        mProgressBar.setVisibility(View.VISIBLE);
        mTxtMsg.setVisibility(View.VISIBLE);
        mList.setVisibility(View.GONE);

        DataWriter dr = new DataWriter();
        try {
            dr.writeByte((byte)(0xff & data));
        } catch (IOException e) {
            Log.i(TAG, "SendData() IOException" + e);
        }
        mOemRil.invokeRequestRaw(requestId, dr.toByteArray(), mHandler.obtainMessage(EVENT_SEND_RIL_DATA_DONE));
    }

    private void sendRilData(int requestId, String data) {
        mTimer = new Timer(true);
        mTimer.schedule(new UpdateTimeTask(), 5000);
        mProgressBar.setVisibility(View.VISIBLE);
        mTxtMsg.setVisibility(View.VISIBLE);
        mList.setVisibility(View.GONE);

        DataWriter dr = new DataWriter();
        try {
            dr.writeBytes(data.getBytes());
        } catch (IOException e) {
            Log.i(TAG, "SendData() IOException" + e);
        }
        mOemRil.invokeRequestRaw(requestId, dr.toByteArray(), mHandler.obtainMessage(EVENT_SEND_RIL_DATA_DONE));
    }

    protected void displayEngList2(byte[] data) {
        Log.d(TAG, "displayEngList2");
        if (mTimer != null) mTimer.cancel();
        else {
            Log.e(TAG, "Unexcepted operation, so stop engineer mode !!");
            sendRilData(RILC_REQ_MISC_SET_ENG_MODE, 255);
            return;
        }

        Log.i(TAG, "size of result : " + data.length);
        Log.v(TAG, "onReceive processResponseUnsol: " + data[0]);

        mProgressBar.setVisibility(View.GONE);
        mTxtMsg.setVisibility(View.GONE);
        mList.setVisibility(View.VISIBLE);

        mScreen = new EngModeScreen(mScreen);
        ArrayList<Line> lines = new ArrayList<>();
        DataReader dr = new DataReader(data);
        int numOfLine = 0;
        try {
            numOfLine = dr.getByte();
            Log.i(TAG, "numOfLine : " + numOfLine);

            // legacy data
            // numOfLine + lines
            for (int i = 0; i < numOfLine; i++) {
                int index = dr.getByte();
                byte[] message = dr.getBytes(32);
                String text = new String(message, "UTF-8").trim();
                Line line = new Line(i, index, text);
                lines.add(line);

                if (mTopMenu == null) {
                    mTopMenu = text;
                }
            }
        } catch (IOException e) {
            Log.w(TAG, "Failed to decode ENG MODE by exception", e);
            return ;
        }

        boolean advanced = false;
        String title = "";
        boolean marks = false;
        int level = -1;
        byte[] colors = null;
        try {
            title = new String(dr.getBytes(32), "UTF-8").trim();
            marks = (dr.getByte() == 0) ? false : true;
            level = dr.getByte() & 0xFF;
            colors = dr.getBytes(numOfLine);  // color info for each line
            advanced = true;
        }
        catch (IOException e) {
            Log.w(TAG, "No advanced information by exception");
        }

        boolean test = false;
        // test only
        if (test) {
            advanced = true;
            int[] COLORS = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            colors = new byte[numOfLine];
            Random rand = new Random();
            marks = rand.nextBoolean();
            for (int i = 0; i < numOfLine; i++) {
                colors[i] = (byte)COLORS[rand.nextInt(COLORS.length)];
            }
        }

        // a legacy line information
        mScreen.setLines(lines);

        // advanced information
        if (advanced) {
            String oldTitle = mScreen.getTitle();
            //mScreen.setTitle(title);
            mScreen.setMark(marks);
            mScreen.setLevel(level);
            String newTitle = mScreen.getTitle();
            for (int i = 0; i < numOfLine; i++) {
                lines.get(i).setTextColor(colors[i] & 0xFF);
            } // end for i ~

            Log.d(TAG, "oldTitle=" + oldTitle + " newTitle=" + newTitle);
            if (!TextUtils.equals(oldTitle, newTitle)) {
                Message message = mHandler.obtainMessage(EVENT_TITLE_CHANGED, newTitle);
                message.sendToTarget();
            }
        }
        Log.d(TAG, "" + mScreen);
        EngModeScreenAdapter adapter = new EngModeScreenAdapter();
        mList.setAdapter(adapter);
    }

    public class UpdateTimeTask extends TimerTask {
        @Override
        public void run() {
            mHandler.sendEmptyMessageDelayed(EVENT_CP_RESPONSE_TIMEOUT, 100);
            mTimer.cancel();
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        Log.i(TAG, "onKeyDown!");
        switch (keyCode) {
        case KeyEvent.KEYCODE_HOME:
            endServiceMode();
            break;
        case KeyEvent.KEYCODE_DPAD_RIGHT:
            Log.i(TAG, "KEYCODE_RIGHT");
            sendRilData(RILC_REQ_MISC_SCREEN_LINE, 255); // select back
            break;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onBackPressed() {
        Log.i(TAG, "onBackPressed!");
        Log.i(TAG, "Back Button hard key clicked");

        // default END if an invalid or unknown status
        boolean isTopeMenu = true;
        if (mScreen != null) {
            int currentLevel = mScreen.getLevel();
            Log.d(TAG, "onBackPressed: currentLevel=" + currentLevel);
            if (currentLevel < 0) {
                // legacy logic. Compare with the first line
                String topMenu = mScreen.getFirstLineText();
                if ((mTopMenu != null && mTopMenu.equals(topMenu)) ||
                        (mTopMenu == null && topMenu == null)) {
                    isTopeMenu = true;
                }
                else {
                    isTopeMenu = false;
                }
            }
            else {
                isTopeMenu = (currentLevel == 0);
            }
        }

        if (isTopeMenu) {
            endServiceMode();
        }
        else {
            backScreen();
        }
    }

    private void endServiceMode() {
        Log.i(TAG, "End Service Mode!");
        mTopMenu = null;
        sendRilData(RILC_REQ_MISC_SET_ENG_MODE, 255); //End service mode
        if (mTimer != null) mTimer.cancel();
        finish();
    }

    private void backScreen() {
        Log.i(TAG, "back to the previous Screen");
        sendRilData(RILC_REQ_MISC_SCREEN_LINE, 255); // select back
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "onCreateOptionsMenu occur");

        super.onCreateOptionsMenu(menu);
        getMenuInflater().inflate(R.menu.engineer_mode, menu);

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.i(TAG, "onOptionsItemSelected!");
        if (item.getItemId() == R.id.action_key) {

            LayoutInflater inflater = (LayoutInflater) EngDetailActivity.this.getSystemService(LAYOUT_INFLATER_SERVICE);
            View layout = inflater.inflate(R.layout.dialog_key, (ViewGroup) findViewById(R.id.layout_customdialog));

            final AlertDialog.Builder builder;
            final AlertDialog mDialog;
            builder = new AlertDialog.Builder(EngDetailActivity.this);
            builder.setView(layout);
            mDialog = builder.create();
            mDialog.setTitle("Input String Data");
            mDialog.show();
            final EditText editInputKey = (EditText) layout.findViewById(R.id.edit_key);
            Button btnOk = (Button) layout.findViewById(R.id.btn_ok);
            Button btnCancel = (Button) layout.findViewById(R.id.btn_cancel);
            btnOk.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.d(TAG, "Button is clicked");
                    String data = editInputKey.getText().toString();
                    if (!data.equals("")) {
                        data = data.toLowerCase();
                        sendRilData(RILC_REQ_MISC_SET_STR_DATA, data);
                    }
                    mDialog.cancel();
                }
            });
            btnCancel = (Button) layout.findViewById(R.id.btn_cancel);
            btnCancel.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mDialog.cancel();
                }
            });
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
