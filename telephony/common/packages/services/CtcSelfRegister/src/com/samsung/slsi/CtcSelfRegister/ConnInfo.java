/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.CtcSelfRegister;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.net.ConnectivityManager;
import android.telephony.CellLocation;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.ServiceState;
import android.util.Log;
import android.os.SystemProperties;


public class ConnInfo {

    private final String TAG_SELFREG_CONN = "SELF_REG_CONN";

    private CellLocation mCellLocation;
    private ServiceState mServiceState;
    private TelephonyManager mTelManager;
    private SelfRegisterService mService;

    private int mSid, mNid, mBssid, mCid ,mCid_data;

    public ConnInfo () {
        mCellLocation = null;
        mServiceState = null;
        mTelManager = null;
        mSid = -1;
        mNid = -1;
        mBssid = -1;
        mCid = -1;
        mCid_data =-1;
        mService = null;
    }
    public ConnInfo (SelfRegisterService service) {
        mCellLocation = null;
        mServiceState = null;
        mTelManager = null;
        mSid = -1;
        mNid = -1;
        mBssid = -1;
        mCid = -1;
        mCid_data =-1;
        mService = service;
    }

    public void setServiceState(ServiceState serviceState) {
        if (serviceState == null) {
            Log.e (TAG_SELFREG_CONN, "Invalid service state");
            return;
        }
        mServiceState = serviceState;
    }

    public ServiceState getServiceState() {
        return mServiceState;
    }

    public int getState() {
        if (mServiceState == null) return ServiceState.STATE_OUT_OF_SERVICE;
        return mServiceState.getState();
    }

    public int getSid() {
        int [] sids = {-1, -1};
        sids[0] = SystemProperties.getInt("vendor.ril.cdma.sid0",-1);
        sids[1] = SystemProperties.getInt("vendor.ril.cdma.sid1",-1);

        for (int sid : sids) {
            if (sid > 0) mSid = sid;
        }
        return mSid;
    }
    public int getNid() {
        int [] nids = {-1, -1};
        nids[0] = SystemProperties.getInt("vendor.ril.cdma.nid0",-1);
        nids[1] = SystemProperties.getInt("vendor.ril.cdma.nid1",-1);

        for (int nid : nids) {
            if (nid > 0) mNid = nid;
        }
        return mNid;
    }
    public int getBasestationId() {
        int [] bssids = {-1, -1};
        bssids[0] = SystemProperties.getInt("vendor.ril.cdma.bssid0",-1);
        bssids[1] = SystemProperties.getInt("vendor.ril.cdma.bssid1",-1);
        for (int bssid : bssids) {
            if (bssid > 0) mBssid = bssid;
        }
        return mBssid;
    }

    public int getBasestationId(int slot) {
        int [] bssids = {-1, -1};
        bssids[0] = SystemProperties.getInt("vendor.ril.cdma.bssid0",-1);
        bssids[1] = SystemProperties.getInt("vendor.ril.cdma.bssid1",-1);
        Log.d(TAG_SELFREG_CONN, "BaseStationID for slot1: " + bssids[0] +"  slot2:"+bssids[1]);
        mBssid =bssids[slot];
        return mBssid;
    }

    public int getCid() {
        int slot = -1;
        if (mService != null) {
            slot = mService.getMainCardSlotId();
        }
        int [] cids = {-1, -1};
        cids[0] = SystemProperties.getInt("vendor.ril.gsm.cid0",-1);
        cids[1] = SystemProperties.getInt("vendor.ril.gsm.cid1",-1);
        Log.d(TAG_SELFREG_CONN, "CID for slot1: " + cids[0] +"  slot2:"+cids[1]);
        if (slot >= 0 && slot < 2) {
            mCid = cids[slot];
        } else {
            for (int cid : cids) {
                if (cid > 0) mCid = cid;
            }
        }
        return mCid;
    }
    public int getCid(int slotId) {
        int cid = -1;
        String propKey = "vendor.ril.gsm.cid"+slotId;
        cid= SystemProperties.getInt(propKey,-1);
        Log.d(TAG_SELFREG_CONN, "CID for slot("+slotId+") : "+cid);
        mCid =cid;
        return mCid;
    }

    public int getCidforData(int slotId) {
        int cid_data = -1;
        String propKey = "vendor.ril.gsm.data_cid"+slotId;
        cid_data = SystemProperties.getInt(propKey,-1);
        Log.d(TAG_SELFREG_CONN, "CID_data for slot( "+slotId+") " + cid_data);
        mCid_data = cid_data;

        return mCid_data;
    }

    public void setSid(int sid) {
        mSid = sid;
    }
    public void setNid(int nid) {
        mNid = nid;
    }
    public void setBasestationId(int bssid) {
        mBssid = bssid;
    }

    public void setCid(int cid) {
        mCid = cid;
    }

    public boolean getRoamingState() {
        if (mServiceState != null) {
            Log.d(TAG_SELFREG_CONN, "Roaming state is " + mServiceState.getRoaming() );
            return mServiceState.getRoaming();
        } else {
            Log.d(TAG_SELFREG_CONN, "Service state is not set");
            // in this service, roaming let stop the service.
            return true;
        }
    }

}


