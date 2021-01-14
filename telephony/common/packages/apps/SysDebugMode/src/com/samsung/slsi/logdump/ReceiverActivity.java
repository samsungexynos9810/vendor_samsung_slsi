/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.logdump;

import com.samsung.slsi.sysdebugmode.R;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.view.KeyEvent;
import android.widget.TextView;

public class ReceiverActivity extends Activity {
    /*
     * (non-Javadoc)
     *
     * @see android.app.Activity#onCreate(android.os.Bundle)
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_receiver);
        String message = "Doing modem dumping now. Please don't touch anything for 10 mins...";

        TextView tv = new TextView(this);
        tv.setText(message);
        tv.setTextColor(Color.YELLOW);
        tv.setTextSize(30);
        tv.setBackgroundColor(Color.RED);
        tv.setGravity(Gravity.CENTER);
        setContentView(tv);

    }

    @Override
    protected void onUserLeaveHint() {
        super.onUserLeaveHint();
        finish();
        startActivity(getIntent());
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
        case KeyEvent.KEYCODE_HOME:
        case KeyEvent.KEYCODE_MENU:
        case KeyEvent.KEYCODE_BACK:
        case KeyEvent.KEYCODE_POWER:
        case KeyEvent.KEYCODE_SEARCH:
        case KeyEvent.KEYCODE_VOLUME_DOWN:
        case KeyEvent.KEYCODE_VOLUME_UP:
            return false;
        }
        return super.onKeyDown(keyCode, event);
    }
}