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

import static com.samsung.slsi.telephony.oem.OemRilConstants.*;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

import vendor.samsung_slsi.telephony.hardware.radioExternal.V1_0.IOemSlsiRadioExternal;
import vendor.samsung_slsi.telephony.hardware.radioExternal.V1_0.IOemSlsiRadioExternalInd;
import vendor.samsung_slsi.telephony.hardware.radioExternal.V1_0.IOemSlsiRadioExternalRes;
import vendor.samsung_slsi.telephony.hardware.radioExternal.V1_0.RadioExternalResponseInfo;
import android.content.Context;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.HwBinder;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.RemoteException;
import android.util.Log;
import android.util.SparseArray;

import com.samsung.slsi.telephony.oem.io.DataWriter;
import com.samsung.slsi.telephony.oem.util.StringUtil;


public class OemRil implements OemCommandsInterface {

    private Context mContext;
    private int mPhoneId = RILC_SOCKET_ID_1;
    private int mClientId = -1;
    private static final String TAG = "OemRil";
    private static final boolean VDBG = !Build.TYPE.equals("user");
    private static final int EVENT_RADIO_PROXY_DEAD = 100;
    private static final int IRADIO_GET_SERVICE_DELAY_MILLIS = 4 * 1000;

    private static final int MAX_RADIO_DATA_SIZE = (1 << 20);    // set 2's multiplier
    private static final int MAX_RADIO_DATA_MOD = (MAX_RADIO_DATA_SIZE - 1);

    protected Object mStateMonitor = new Object();
    static AtomicInteger sNextSerial = new AtomicInteger(0);

    private static OemRil sInstance = null;
    private IOemSlsiRadioExternal mOemSlsiRadioExternalProxy = null;
    private IOemSlsiRadioExternalRes mOemSlsiRadioExternalRes = null;
    private IOemSlsiRadioExternalInd mOemSlsiRadioExternalInd = null;

    SparseArray<RILRequest> mRequestList = new SparseArray<RILRequest>();
    private RegistrantList mOemRilConntectedRegistrants = new RegistrantList();
    private RegistrantList mOemRilDisconntectedRegistrants = new RegistrantList();

    /* registrant for Indication */
    protected Registrant mDisplayEngRegistrant;
    protected Registrant mAmRegistrant;
    protected Registrant mRssiScanResultRegistrant;
    protected Registrant mATCommandListenerRegistrant;
    protected Registrant mSarRfConnectionRegistrant;
    protected Registrant mVsimOperationRegistrant;
    protected Registrant mModemInfoRegistrant;
    protected Registrant mSelflogStatusRegistrant;
    protected Registrant mCAInfoForPhoneRegistrant;
    protected Registrant mSrvccHoRegistrant;
    protected Registrant mSipMessageIndRegistrant;
    protected Registrant mAmbrReportRegistrant;
    protected Registrant mB2B1ConfigReportRegistrant;
    protected Registrant mFrequencyInfoRegistrant;

    public void registerAm(Handler h, int what, Object obj) {
        mAmRegistrant = new Registrant(h, what, obj);
    }

    public void registerDisplayEng(Handler h, int what, Object obj) {
        mDisplayEngRegistrant = new Registrant(h, what, obj);
    }

    public void registerRssiScanResult(Handler h, int what, Object obj) {
        mRssiScanResultRegistrant = new Registrant(h, what, obj);
    }

    public void registerForATCommandListener(Handler h, int what, Object obj) {
        mATCommandListenerRegistrant = new Registrant(h, what, obj);
    }

    public void registerSarRfConnection(Handler h, int what, Object obj) {
        mSarRfConnectionRegistrant = new Registrant(h, what, obj);
    }

    public void registerVsimOperation(Handler h, int what, Object obj) {
        mVsimOperationRegistrant = new Registrant(h, what, obj);
    }

    public void registerSelflogStatus(Handler h, int what) {
        mSelflogStatusRegistrant = new Registrant(h, what, null);
    }

    public void registerForCAInfoForPhone(Handler h, int what) {
        mCAInfoForPhoneRegistrant = new Registrant(h, what, null);
    }

    public void unregisterAm(Handler h) {
        if (mAmRegistrant != null && mAmRegistrant.getHandler() == h) {
            mAmRegistrant.clear();
            mAmRegistrant = null;
        }
    }

    public void unregisterDisplayEng(Handler h) {
        if (mDisplayEngRegistrant != null && mDisplayEngRegistrant.getHandler() == h) {
            mDisplayEngRegistrant.clear();
            mDisplayEngRegistrant = null;
        }
    }

    public void unregisterRssiScanResult(Handler h) {
        if (mRssiScanResultRegistrant != null && mRssiScanResultRegistrant.getHandler() == h) {
            mRssiScanResultRegistrant.clear();
            mRssiScanResultRegistrant = null;
        }
    }

    public void unregisterForATCommandListener(Handler h) {
        if (mATCommandListenerRegistrant != null && mATCommandListenerRegistrant.getHandler() == h) {
            mATCommandListenerRegistrant.clear();
            mATCommandListenerRegistrant = null;
        }
    }

    public void unregisterSarRfConnection(Handler h) {
        if (mSarRfConnectionRegistrant != null && mSarRfConnectionRegistrant.getHandler() == h) {
            mSarRfConnectionRegistrant.clear();
            mSarRfConnectionRegistrant = null;
        }
    }

    public void unregisterVsimOperation(Handler h) {
        if (mVsimOperationRegistrant != null && mVsimOperationRegistrant.getHandler() == h) {
            mVsimOperationRegistrant.clear();
            mVsimOperationRegistrant = null;
        }
    }

    public void registerForModemInfo(Handler h, int what, Object obj) {
        mModemInfoRegistrant = new Registrant(h, what, obj);
    }

    public void registerForFrequencyInfo(Handler h, int what, Object obj) {
        mFrequencyInfoRegistrant = new Registrant(h, what, obj);
    }

    public void unregisterForModemInfo(Handler h) {
        if (mModemInfoRegistrant != null && mModemInfoRegistrant.getHandler() == h) {
            mModemInfoRegistrant.clear();
            mModemInfoRegistrant = null;
        }
    }

    public void unregisterSelflogStatus(Handler h) {
        if (mSelflogStatusRegistrant != null && mSelflogStatusRegistrant.getHandler() == h) {
            mSelflogStatusRegistrant.clear();
            mSelflogStatusRegistrant = null;
        }
    }

    public void unregisterForCAInfoForPhone(Handler h) {
        if (mCAInfoForPhoneRegistrant != null && mCAInfoForPhoneRegistrant.getHandler() == h) {
            mCAInfoForPhoneRegistrant.clear();
            mCAInfoForPhoneRegistrant = null;
        }
    }

    public void registerForSrvccHoInd(Handler h, int what, Object obj) {
        mSrvccHoRegistrant = new Registrant(h, what, obj);
    }

    public void unregisterForSrvccHoInd(Handler h) {
        if (mSrvccHoRegistrant != null && mSrvccHoRegistrant.getHandler() == h) {
            mSrvccHoRegistrant.clear();
            mSrvccHoRegistrant = null;
        }
    }

    public void registerForSipMessageInd(Handler h, int what, Object obj) {
        mSipMessageIndRegistrant = new Registrant(h, what, obj);
    }

    public void unregisterForSipMessageInd(Handler h) {
        if (mSipMessageIndRegistrant != null && mSipMessageIndRegistrant.getHandler() == h) {
            mSipMessageIndRegistrant.clear();
            mSipMessageIndRegistrant = null;
        }
    }

    public void registerForAmbrReport(Handler h, int what, Object obj) {
        mAmbrReportRegistrant = new Registrant(h, what, obj);
    }

    public void unregisterForAmbrReport(Handler h) {
        if (mAmbrReportRegistrant != null && mAmbrReportRegistrant.getHandler() == h) {
            mAmbrReportRegistrant.clear();
            mAmbrReportRegistrant = null;
        }
    }

    public void registerForB2B1ConfigInd(Handler h, int what, Object obj) {
        mB2B1ConfigReportRegistrant = new Registrant(h, what, obj);
    }

    public void unregisterForB2B1ConfigReport(Handler h) {
        if (mB2B1ConfigReportRegistrant != null && mB2B1ConfigReportRegistrant.getHandler() == h) {
            mB2B1ConfigReportRegistrant.clear();
            mB2B1ConfigReportRegistrant = null;
        }
    }

    public void unregisterForFrequencyInfo(Handler h) {
        if (mFrequencyInfoRegistrant != null && mFrequencyInfoRegistrant.getHandler() == h) {
            mFrequencyInfoRegistrant.clear();
            mFrequencyInfoRegistrant = null;
        }
    }

    public static OemRil init(Context context, int instanceId) {
        Log.i(TAG, "create new OemRil instance: instanceId=" + instanceId);
        return new OemRil(context, instanceId);
    }

    public static OemRil getInstance() {
        return sInstance;
    }

    public Context getContext() {
        return mContext;
    }

    public int getPhoneId() {
        return mPhoneId;
    }

    RILRequest CreateRILRequest(int request, Message result) {
        return new RILRequest(request, result);
    }

    public OemRil(Context context, int phoneId) {
        mContext = context;
        mPhoneId = phoneId;
        mOemSlsiRadioExternalRes = new OemSlsiRadioExternalRes(this);
        mOemSlsiRadioExternalInd = new OemSlsiRadioExternalInd(this);
        mOemSlsiRadioExternalProxyDeathRecipient = new RadioProxyDeathRecipient();
        getOemSlsiRadioExternalProxy(null);

    }

    final AtomicLong mOemSlsiRadioExternalProxyCookie = new AtomicLong(0);
    final RadioProxyDeathRecipient mOemSlsiRadioExternalProxyDeathRecipient;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_RADIO_PROXY_DEAD:
                Log.e(TAG, "handleMessage: EVENT_RADIO_PROXY_DEAD cookie = " + msg.obj + " mRadioProxyCookie = " + mOemSlsiRadioExternalProxyCookie.get());
                if ((long) msg.obj == mOemSlsiRadioExternalProxyCookie.get()) {
                    resetProxyAndRequestList();
                    getOemSlsiRadioExternalProxy(null);
                }
                break;
            }
        }
    };

    private void resetProxyAndRequestList() {
        mOemSlsiRadioExternalProxy = null;
        // increment the cookie so that death notification can be ignored
        mOemSlsiRadioExternalProxyCookie.incrementAndGet();
    }

    private void handleRadioProxyExceptionForRR(RILRequest rr, String caller, Exception e) {
        Log.e(TAG, caller + ": " + e);
        resetProxyAndRequestList();

        // service most likely died, handle exception like death notification to try to get service
        // again
        mHandler.sendMessageDelayed(
                mHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD, mOemSlsiRadioExternalProxyCookie.incrementAndGet()), IRADIO_GET_SERVICE_DELAY_MILLIS);
    }

    class RILRequest {
        int mChannel;
        int mSerial;
        int mRequest;
        Message mResult;
        DataWriter mOut;

        RILRequest(int request, Message result) {
            mChannel = mPhoneId;
            mSerial = sNextSerial.getAndIncrement();
            mOut = new DataWriter();
            mRequest = request;
            mResult = result;
        }

        void release() {
            mSerial = RILC_TRANSACTION_NONE;
            mRequest = 0;
            mResult = null;
            mOut.reset();
        }

        void onError(int error, Object ret) {
            Exception ex = OemRilConstants.fromRilErrno(error);

            Log.e(TAG, "Error: " + ex + " ret=" + StringUtil.retToString(mRequest, ret));
            if (mResult != null) {
                AsyncResult.forMessage(mResult, ret, ex);
                mResult.sendToTarget();
            }
            release();
        }

        @Override
        public String toString() {
            return String.format("RILRequest{channel:%d request:%d serial:%d data:%s}",
                    mChannel, mRequest, mSerial, StringUtil.bytesToHexString(mOut.toByteArray()));
        }
    }

    private RILRequest findAndRemoveRequestFromList(int serial) {
        RILRequest rr = null;
        synchronized (mRequestList) {
            rr = mRequestList.get(serial);
            if (rr != null) {
                mRequestList.remove(serial);
            }
        }

        return rr;
    }

    @Override
    public void detach() {
        Log.d(TAG, "detach()");
        try {
            mOemSlsiRadioExternalProxy.clearResponseFunctions(mClientId);
            mOemSlsiRadioExternalProxy.unlinkToDeath(mOemSlsiRadioExternalProxyDeathRecipient);
            mOemSlsiRadioExternalProxy = null;
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private void send(RILRequest rr) {
        IOemSlsiRadioExternal oemSlsiRadioExternalProxy = getOemSlsiRadioExternalProxy(rr.mResult);
        if (oemSlsiRadioExternalProxy != null) {

            synchronized (mRequestList) {
                mRequestList.append(rr.mSerial, rr);
            }
            try {
                oemSlsiRadioExternalProxy.sendRequestRaw(rr.mSerial, mClientId, rr.mRequest, mPhoneId, rr.mOut.size(), StringUtil.primitiveArrayToArrayList(rr.mOut.toByteArray()));
            } catch (RemoteException | RuntimeException e) {
                handleRadioProxyExceptionForRR(rr, "send", e);
            }
        }
    }

    private void sendMultiFrame(RILRequest rr) {
        Log.d(TAG, "need to implement: multi frame");
    }

    @Override
    public void invokeRequestRaw(int request, byte[] data, Message response) {
        RILRequest rr = CreateRILRequest(request, response);
        try {
            rr.mOut.writeBytes(data);
            Log.v(TAG, "[" + TAG + "_" + rr.mSerial + "]> " +
                   requestToString(rr.mRequest) + " " + StringUtil.retToString(rr.mRequest, data) +
                   " [SUB" + rr.mChannel + "]");

            // size() always returns 0 and RIL_MAX_COMMAND_BYTES is limited 4KB. DataWriter shall be modified.
            if ( rr.mOut.size() > MAX_RADIO_DATA_SIZE ) sendMultiFrame(rr);
            else send(rr);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private IOemSlsiRadioExternal getOemSlsiRadioExternalProxy(Message result) {

        if (mOemSlsiRadioExternalProxy != null) {
            return mOemSlsiRadioExternalProxy;
        }

        try {
            mOemSlsiRadioExternalProxy = IOemSlsiRadioExternal.getService("rilExternal");
            if (mOemSlsiRadioExternalProxy != null) {
                // not calling linkToDeath() as ril service runs in the same process and death
                // notification for that should be sufficient
                mClientId = mOemSlsiRadioExternalProxy.setResponseFunctions(mOemSlsiRadioExternalRes, mOemSlsiRadioExternalInd);
                mOemRilConntectedRegistrants.notifyRegistrants();
                mOemSlsiRadioExternalProxy.linkToDeath(mOemSlsiRadioExternalProxyDeathRecipient, mOemSlsiRadioExternalProxyCookie.incrementAndGet());
            } else {
                Log.e(TAG, "getOemSlsiRadioExternalProxy: mOemSlsiRadioExternalProxy == null");
            }
        } catch (RemoteException | RuntimeException e) {
            mOemSlsiRadioExternalProxy = null;
            Log.e(TAG, "OemSamsungslsiProxy getService/setResponseFunctions: " + e);
        }

        if (mOemSlsiRadioExternalProxy == null) {
            if (result != null) {
                AsyncResult.forMessage(result, null,
                        OemRilConstants.fromRilErrno(RILC_STATUS_FAIL));
                result.sendToTarget();
            }

            // if service is not up, treat it like death notification to try to get service again
            mHandler.sendMessageDelayed(mHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD, mOemSlsiRadioExternalProxyCookie.incrementAndGet()), IRADIO_GET_SERVICE_DELAY_MILLIS);
        }
        return mOemSlsiRadioExternalProxy;
    }

    RILRequest processResponse(RadioExternalResponseInfo responseInfo) {
        int serial = responseInfo.serial;
        int error = responseInfo.error;

        RILRequest rr = null;
        rr = findAndRemoveRequestFromList(serial);

        if (rr == null) {
            Log.e(TAG, "processResponse: Unexpected response! serial: " + serial
                    + " error: " + error);
            return null;
        }
        return rr;
    }

    void processResponseDone(RILRequest rr,
            RadioExternalResponseInfo responseInfo, Object ret) {
        if (responseInfo.error == 0) {
            Log.v(TAG, "[" + TAG + "_" + rr.mSerial + "]< " + requestToString(rr.mRequest) + " "
                    + StringUtil.retToString(rr.mRequest, ret) + " [SUB" + rr.mChannel + "]" );
        } else {
            Log.v(TAG, "[" + TAG + "_" + rr.mSerial + "]< " + requestToString(rr.mRequest)
                    + " error " + responseInfo.error);
            rr.onError(responseInfo.error, ret);
        }
    }

    @Override
    public void registerForOemRilConnected(Handler h, int what) {
        Registrant r = new Registrant (h, what, null);
        synchronized (mStateMonitor) {
            mOemRilConntectedRegistrants.add(r);
        }
        if (mOemSlsiRadioExternalProxy != null) {
            r.notifyRegistrant();
        }
    }

    /* OEM RIL connection/disconnection */
    @Override
    public void registerForOemRilDisconnected(Handler h, int what) {
        Registrant r = new Registrant (h, what, null);
        synchronized (mStateMonitor) {
            mOemRilDisconntectedRegistrants.add(r);
        }
    }

    @Override
    public void unregisterForOemRilConnected(Handler h) {
        synchronized (mStateMonitor) {
            mOemRilConntectedRegistrants.remove(h);
        }
    }

    @Override
    public void unregisterForOemRilDisconnected(Handler h) {
        synchronized (mStateMonitor) {
            mOemRilDisconntectedRegistrants.remove(h);
        }
    }

    void notifyForOemRilConnected() {
        mOemRilConntectedRegistrants.notifyRegistrants();
    }

    void notifyForOemRilDisconnected() {
        mOemRilDisconntectedRegistrants.notifyRegistrants();
    }

    final class RadioProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            Log.e(TAG, "serviceDied");
            notifyForOemRilDisconnected();
            mHandler.sendMessageDelayed(mHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD, cookie), IRADIO_GET_SERVICE_DELAY_MILLIS);
        }
    }
}
