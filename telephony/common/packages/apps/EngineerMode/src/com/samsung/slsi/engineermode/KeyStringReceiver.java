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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class KeyStringReceiver extends BroadcastReceiver {

    public static final String KEY_MENU_SETTING = "KEY_MENU_SETTING";
    public static final String ACTION_MAIN = "action.engineermode.main";
    public static final int URI_LENGTH = 22;
    public static final int MAIN_LENGTH = 4;
    public static final int SUB_LENGTH = 5;

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        if (intent != null && "android.provider.Telephony.SECRET_CODE".equals(intent.getAction())) {
            String str_intent = intent.getData().toString();
            int key_menu = 0;

            if (str_intent.length() == URI_LENGTH + SUB_LENGTH)
                key_menu = Integer.parseInt(str_intent.substring(URI_LENGTH + MAIN_LENGTH));

            Intent i = new Intent(ACTION_MAIN);
            i.setClass(context, EngModeActivity.class);
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            i.putExtra(KEY_MENU_SETTING, key_menu);
            context.startActivity(i);
        }
    }
}