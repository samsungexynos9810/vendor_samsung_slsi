/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.oemservice;

import java.util.ArrayList;

import vendor.samsung_slsi.telephony.hardware.oemservice.V1_0.IOemServiceCallback;
import android.os.AsyncResult;
import android.os.Message;
import android.util.Log;

import com.samsung.slsi.telephony.silentlogging.SilentLoggingControlInterface;

class OemServiceScedCallback extends IOemServiceCallback.Stub {

    OemService mOemService;
    private static final String TAG = "OemServiceScedCallback";

    OemServiceScedCallback(OemService oemService) {
        mOemService = oemService;
    }

    static void sendMessageResponse(Message msg, Object ret) {
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, null);
            msg.sendToTarget();
        }
    }

    @Override
    public void onCallback(int type, int id, ArrayList<Byte> data) {
        Log.d(TAG, "onCallback: id= " + id);
        byte[] result = arrayListToPrimitiveArray(data);
        int pid = (result[0] & 0xFF) | ((result[1] & 0xFF) << 8) | ((result[2] & 0xFF) << 16) | ((result[3] & 0xFF) << 24);
		switch (id) {
		case OemServiceConstants.COMMAND_LOGCAT:
		case OemServiceConstants.COMMAND_LOGCAT_SNAPSHOT:
		case OemServiceConstants.COMMAND_TCP_DUMP:
		case OemServiceConstants.COMMAND_TCP_DUMP_SNAPSHOT:
		    try {
                SilentLoggingControlInterface.getInstance().saveLoggingPid(id, pid);
            } catch (Throwable e) {
                e.printStackTrace();
            }
            break;
		case OemServiceConstants.COMMAND_KILL:
		    SilentLoggingControlInterface.getInstance().stopApTcpLogging(id);
		    break;
		default:
			break;
		}
	}

    public static byte[] arrayListToPrimitiveArray(ArrayList<Byte> bytes) {
        byte[] ret = new byte[bytes.size()];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = bytes.get(i);
        }
        return ret;
    }

}
