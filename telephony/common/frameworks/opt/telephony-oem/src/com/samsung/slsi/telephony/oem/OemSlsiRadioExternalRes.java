/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.oem;

import vendor.samsung_slsi.telephony.hardware.radioExternal.V1_0.IOemSlsiRadioExternalRes;
import vendor.samsung_slsi.telephony.hardware.radioExternal.V1_0.RadioExternalError;
import vendor.samsung_slsi.telephony.hardware.radioExternal.V1_0.RadioExternalResponseInfo;

import java.util.ArrayList;

import com.samsung.slsi.telephony.oem.util.StringUtil;
import com.samsung.slsi.telephony.oem.OemRil.RILRequest;

import android.os.AsyncResult;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;

/**
 * {@hide}
 */
class OemSlsiRadioExternalRes extends IOemSlsiRadioExternalRes.Stub {

    public static final String TAG = "OemSlsiRadioExternalRsp";

    ArrayList<Byte> rspDataSeg;

    OemRil mRil;
    OemSlsiRadioExternalRes(OemRil ril) {
        mRil = ril;
    }

    /**
     * Helper function to send response msg
     * @param msg Response message to be sent
     * @param ret Return object to be included in the response message
     */
    static void sendMessageResponse(Message msg, Object ret) {
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, null);
            msg.sendToTarget();
        }
    }

    @Override
    public void sendRequestRawResponse(RadioExternalResponseInfo responseInfo, ArrayList<Byte> data) throws RemoteException {
        RILRequest rr = mRil.processResponse(responseInfo);

        if (rr != null) {
            Object ret = null;
            if (rr.mResult != null) {
                rr.mResult.arg1 = responseInfo.slotId;
                rr.mResult.arg2 = responseInfo.error;
                if (responseInfo.error == RadioExternalError.RADIO_EXTERNAL_NONE) {
                    ret = StringUtil.arrayListToPrimitiveArray(data);
                    sendMessageResponse(rr.mResult, ret);
                }
            }
            mRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    @Override
    public void sendRequestRawResponseSeg(RadioExternalResponseInfo responseInfo, ArrayList<Byte> data, int segIndex, int totalLen) throws RemoteException {
        Log.d(TAG, "Need to implement: segIndex(" + segIndex + ") totalLen(" + totalLen + ")");

        // It asumed that data always comes in sequence. There is no corruption in order.
        // If not, it should be redesinged.
        // If several rsp Msg shall be handled at the same time, code change is needed.
    }
}
