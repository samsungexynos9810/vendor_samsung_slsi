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

public class UsbManagerProxy implements UsbManagerInterface {

    private UsbManagerInterface mUsbManager;
    private final Context mContext;

    private static UsbManagerProxy instance = null;
    public static UsbManagerProxy getInstance(Context context) {
        if (instance == null) {
            instance = new UsbManagerProxy(context);
            instance.init();
        }
        return instance;
    }

    UsbManagerProxy(Context context) {
        mContext = context;
    }

    private void init() {
        mUsbManager = new UsbManagerInterfaceImpl(mContext);
    }

    @Override
    public boolean setCurrentUsbFunctions(String functions, boolean makeDefault) {
        if (mUsbManager != null) {
            return mUsbManager.setCurrentUsbFunctions(functions, makeDefault);
        }
        return false;
    }

    @Override
    public String getCurrentUsbFunctions() {
        if (mUsbManager != null) {
            return mUsbManager.getCurrentUsbFunctions();
        }
        return "";
    }

    @Override
    public String getDefaultUsbFunctions() {
        if (mUsbManager != null) {
            return mUsbManager.getDefaultUsbFunctions();
        }
        return "";
    }
}