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
import java.util.concurrent.atomic.AtomicInteger;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.RemoteException;
import android.util.Log;
import vendor.samsung_slsi.telephony.hardware.oemservice.V1_0.*;

public class OemService {

	private Context mContext;
	private static OemService sInstance = null;
	private static final String TAG = "OemService";
    private static final int EVENT_RADIO_PROXY_DEAD = 100;
    private static final int IRADIO_GET_SERVICE_DELAY_MILLIS = 4 * 1000;

    protected Object mStateMonitor = new Object();
    //    static AtomicInteger sNextSerial = new AtomicInteger(0);

    private IOemService mOemServiceDmdProxy = null;
    private IOemService mOemServiceScedProxy = null;
    private IOemServiceCallback mOemServiceDmdCallback = null;
    private IOemServiceCallback mOemServiceScedCallback = null;
    private RegistrantList mOemServiceDmdConntectedRegistrants = new RegistrantList();
    private RegistrantList mOemServiceScedConntectedRegistrants = new RegistrantList();
    private RegistrantList mOemServiceDmdDisconntectedRegistrants = new RegistrantList();
    private RegistrantList mOemServiceScedDisconntectedRegistrants = new RegistrantList();

    protected Registrant mNotifyDmLogRegistrant;
    protected Registrant mNotifyCommandRespRegistrant;
    protected Registrant mNotifySaveAutoLogRegistrant;

    public static OemService init(Context context) {
        Log.i(TAG, "Create new OemService instance");
        return new OemService(context);
    }

    public static OemService getInstance() {
        return sInstance;
    }

    public Context getContext() {
        return mContext;
    }

    public OemService(Context context) {
        mContext = context;
        mOemServiceDmdCallback = new OemServiceDmdCallback(this);
        mOemServiceScedCallback = new OemServiceScedCallback(this);
        getOemServiceDmdProxy();
        getOemServiceScedProxy();
    }

    private IOemService getOemServiceDmdProxy() {

        if (mOemServiceDmdProxy != null) {
            return mOemServiceDmdProxy;
        }

        try {
            mOemServiceDmdProxy = IOemService.getService("dm0");
            // mOemServiceProxy = IOemService.getService("sced0");
            if (mOemServiceDmdProxy != null) {
                // not calling linkToDeath() as ril service runs in the same process and death
                // notification for that should be sufficient

                mOemServiceDmdProxy.setCallback(mOemServiceDmdCallback);
                mOemServiceDmdConntectedRegistrants.notifyRegistrants();
                //mOemServiceProxy.linkToDeath(mOemSlsiRadioExternalProxyDeathRecipient, mOemSlsiRadioExternalProxyCookie.incrementAndGet());
            } else {
                Log.e(TAG, "getOemServiceDmdProxy: mOemServiceDmdProxy == null");
            }
        } catch (RemoteException | RuntimeException e) {
            mOemServiceDmdProxy = null;
            Log.e(TAG, "OemServiceDmdProxy getService/setCallback: " + e);
        }
        return mOemServiceDmdProxy;
    }

    private IOemService getOemServiceScedProxy() {

        if (mOemServiceScedProxy != null) {
            return mOemServiceScedProxy;
        }

        try {
            mOemServiceScedProxy = IOemService.getService("sced0");
            // mOemServiceProxy = IOemService.getService("sced0");
            if (mOemServiceScedProxy != null) {
                // not calling linkToDeath() as ril service runs in the same process and death
                // notification for that should be sufficient

                mOemServiceScedProxy.setCallback(mOemServiceScedCallback);
                mOemServiceScedConntectedRegistrants.notifyRegistrants();
                //mOemServiceProxy.linkToDeath(mOemSlsiRadioExternalProxyDeathRecipient, mOemSlsiRadioExternalProxyCookie.incrementAndGet());
            } else {
                Log.e(TAG, "getOemServiceScedProxy: mOemServiceScedProxy == null");
            }
        } catch (RemoteException | RuntimeException e) {
            mOemServiceScedProxy = null;
            Log.e(TAG, "OemServiceScedProxy getService/setCallback: " + e);
        }
        return mOemServiceScedProxy;
    }

    public void registerForOemServiceDmdConnected(Handler h, int what) {
        Registrant r = new Registrant (h, what, null);
        synchronized (mStateMonitor) {
            mOemServiceDmdConntectedRegistrants.add(r);
        }
        if (mOemServiceDmdProxy != null) {
            r.notifyRegistrant();
        }
    }

    public void unregisterForOemServiceDmdConnected(Handler h) {
        synchronized (mStateMonitor) {
            mOemServiceDmdConntectedRegistrants.remove(h);
        }
    }

    public void registerForOemServiceDmdDisconnected(Handler h, int what) {
        Registrant r = new Registrant (h, what, null);
        synchronized (mStateMonitor) {
            mOemServiceDmdDisconntectedRegistrants.add(r);
        }
    }

    public void unregisterForOemServiceDmdDisconnected(Handler h) {
        synchronized (mStateMonitor) {
            mOemServiceDmdDisconntectedRegistrants.remove(h);
        }
    }

    void notifyForOemServiceDmdConnected() {
        mOemServiceDmdConntectedRegistrants.notifyRegistrants();
    }

    void notifyForOemServiceDmdDisconnected() {
        mOemServiceDmdDisconntectedRegistrants.notifyRegistrants();
    }

    public void registerNotifyDmLog(Handler h, int what) {
        mNotifyDmLogRegistrant = new Registrant(h, what, null);
    }

    public void unregisterNotifyDmLog(Handler h) {
        if (mNotifyDmLogRegistrant != null && mNotifyDmLogRegistrant.getHandler() == h) {
            mNotifyDmLogRegistrant.clear();
            mNotifyDmLogRegistrant = null;
        }
    }

    public void registerNotifyCommandResp(Handler h, int what) {
        mNotifyCommandRespRegistrant = new Registrant(h, what, null);
    }

    public void unregisterNotifyCommandResp(Handler h) {
        if (mNotifyCommandRespRegistrant != null && mNotifyCommandRespRegistrant.getHandler() == h) {
            mNotifyCommandRespRegistrant.clear();
            mNotifyCommandRespRegistrant = null;
        }
    }

    public void registerNotifySaveAutoLog(Handler h, int what) {
        mNotifySaveAutoLogRegistrant = new Registrant(h, what, null);
    }

    public void unregisterNotifySaveAutoLog(Handler h) {
        if (mNotifySaveAutoLogRegistrant != null && mNotifySaveAutoLogRegistrant.getHandler() == h) {
            mNotifySaveAutoLogRegistrant.clear();
            mNotifySaveAutoLogRegistrant = null;
        }
    }

    public void registerForOemServiceScedConnected(Handler h, int what) {
        Registrant r = new Registrant (h, what, null);
        synchronized (mStateMonitor) {
            mOemServiceScedConntectedRegistrants.add(r);
        }
        if (mOemServiceScedProxy != null) {
            r.notifyRegistrant();
        }
    }

    public void unregisterForOemServiceScedConnected(Handler h) {
        synchronized (mStateMonitor) {
            mOemServiceScedConntectedRegistrants.remove(h);
        }
    }

    public void registerForOemServiceScedDisconnected(Handler h, int what) {
        Registrant r = new Registrant (h, what, null);
        synchronized (mStateMonitor) {
            mOemServiceScedDisconntectedRegistrants.add(r);
        }
    }

    public void unregisterForOemServiceScedDisconnected(Handler h) {
        synchronized (mStateMonitor) {
            mOemServiceScedDisconntectedRegistrants.remove(h);
        }
    }

    void notifyForOemServiceScedConnected() {
        mOemServiceScedConntectedRegistrants.notifyRegistrants();
    }

    void notifyForOemServiceScedDisconnected() {
        mOemServiceScedDisconntectedRegistrants.notifyRegistrants();
    }

    public static ArrayList<Byte> primitiveArrayToArrayList(byte[] arr) {
        ArrayList<Byte> arrayList = new ArrayList<>(arr.length);
        for (byte b : arr) {
            arrayList.add(b);
        }
        return arrayList;
    }

    /* DMD */
    public void setDmMode(int mode) {
        byte[] data = new byte[] { (byte) (mode & 0xFF), (byte) (mode >> 8 & 0xFF), (byte) (mode >> 16 & 0xFF), (byte) (mode >> 24 & 0xFF) };

        IOemService oemServiceProxy = getOemServiceDmdProxy();
        if (mOemServiceDmdProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(OemServiceConstants.TYPE_COMMAND, OemServiceConstants.COMMAND_SET_DM_MODE, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }
    public void sendProfile(byte[] data) {
        IOemService oemServiceProxy = getOemServiceDmdProxy();
        if (mOemServiceDmdProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(OemServiceConstants.TYPE_COMMAND, OemServiceConstants.COMMAND_SEND_PROFILE, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void stopCpSilentLogging(byte[] data) {
        IOemService oemServiceProxy = getOemServiceDmdProxy();
        if (mOemServiceDmdProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(OemServiceConstants.TYPE_COMMAND, OemServiceConstants.COMMAND_STOP_DM, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void saveSnapshot(byte[] data) {
        IOemService oemServiceProxy = getOemServiceDmdProxy();
        if (mOemServiceDmdProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(OemServiceConstants.TYPE_COMMAND, OemServiceConstants.COMMAND_SAVE_SNAPSHOT, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void refreshFileList() {
        byte[] data = {0x00};
        IOemService oemServiceProxy = getOemServiceDmdProxy();
        if (mOemServiceDmdProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(OemServiceConstants.TYPE_COMMAND, OemServiceConstants.COMMAND_REFRESH_FILE_LIST, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void saveAutoLog() {
        byte[] data = {0x00};
        IOemService oemServiceProxy = getOemServiceDmdProxy();
        if (mOemServiceDmdProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(OemServiceConstants.TYPE_COMMAND, OemServiceConstants.COMMAND_SAVE_AUTOLOG, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void setDmMaxFileSize(int size) {
        byte[] data = new byte[] { (byte) (size & 0xFF), (byte) (size >> 8 & 0xFF), (byte) (size >> 16 & 0xFF), (byte) (size >> 24 & 0xFF) };

        IOemService oemServiceProxy = getOemServiceDmdProxy();
        if (mOemServiceDmdProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(OemServiceConstants.TYPE_COMMAND, OemServiceConstants.COMMAND_SET_DM_MAX_FILE_SIZE, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void sendToModem(byte[] data) {
        IOemService oemServiceProxy = getOemServiceDmdProxy();
        if (mOemServiceDmdProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(OemServiceConstants.TYPE_RAW, 0, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    /* SCED */
    public void startApSilentLogging(int id, byte[] data) {
        IOemService oemServiceProxy = getOemServiceScedProxy();
        if (mOemServiceScedProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(0, id, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void startTcpSilentLogging(int id, byte[] data) {
        IOemService oemServiceProxy = getOemServiceScedProxy();
        if (mOemServiceScedProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(0, id, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void stopApTcpSilentLogging(byte[] data) {
        IOemService oemServiceProxy = getOemServiceScedProxy();
        if (mOemServiceScedProxy != null) {
            try {
                oemServiceProxy.sendRequestRaw(0, OemServiceConstants.COMMAND_KILL, primitiveArrayToArrayList(data));
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }
}
