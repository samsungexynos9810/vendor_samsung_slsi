/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.cnntlogger;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class BootReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        if (intent != null && intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
            Log.d(CmdDefine.LOGTAG, "receive boot completed message..");
            CNNTUtils utils = new CNNTUtils(context);
            utils.setPreference(CmdDefine.KEY_BTN_STATUS, true);
            utils.setPreference(CmdDefine.KEY_CHECK_LOGCAT, true);
            utils.setPreference(CmdDefine.KEY_CHECK_MXLOG, true);

            context.startService(new Intent(context, LoggingService.class));
        }
    }
}
