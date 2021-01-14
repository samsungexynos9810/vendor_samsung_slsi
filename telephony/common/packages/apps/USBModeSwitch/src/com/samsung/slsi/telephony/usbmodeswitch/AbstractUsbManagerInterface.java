/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.usbmodeswitch;

import android.content.Context;
import android.os.SystemProperties;

import com.samsung.slsi.telephony.usbmodeswitch.UsbManagerInterface;

public abstract class AbstractUsbManagerInterface implements UsbManagerInterface {

    protected Context mContext;

    AbstractUsbManagerInterface(Context context) {
        mContext = context;
    }

    @Override
    public abstract boolean setCurrentUsbFunctions(String functions, boolean makeDefault);

    @Override
    public String getCurrentUsbFunctions() {
        String currentFunctions = SystemProperties.get(SYS_USB_STATE, "");
        return currentFunctions;
    }

    @Override
    public String getDefaultUsbFunctions() {
        String defaultFunctions = SystemProperties.get(SYS_USB_CONFIG, "");
        return defaultFunctions;
    }
}
