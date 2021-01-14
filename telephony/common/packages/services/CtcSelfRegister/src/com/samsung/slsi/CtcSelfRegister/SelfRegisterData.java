/*
 * Copyright (c) 2017. Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.CtcSelfRegister;

import android.content.Context;
import android.content.IntentFilter;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Environment;
import android.os.PersistableBundle;
import android.os.StatFs;
import android.provider.Settings;
import android.telephony.CellInfo;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.util.Base64;
import android.util.Log;

import com.android.internal.util.MemInfoReader;

import android.os.storage.StorageManager;
import android.os.storage.VolumeInfo;
import android.provider.DocumentsContract;
import android.os.SystemProperties;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.lang.Long;
import java.lang.System;
import java.net.NetworkInterface;
import java.util.Calendar;
import java.util.List;
import java.util.Date;
import java.util.Collections;
import java.text.SimpleDateFormat;

import org.json.JSONObject;


/**HISTORY
   Created by jongmin.son on 2017-02-06. (Version 1)
**/


public class SelfRegisterData {
    //Debug
    private static final Boolean DBG = true;
    //SLOT Number
    public static int SLOT1 =0;
    public static int SLOT2 =1;
    // Maximum length specification
    private static final int MAX_MODEL_LENGTH = 20;
    private static final int MAX_SWVER_LENGTH = 60;

    // Registration Version -- Fixed value
    private static final String REGVER = "8.0";
    private static final String TAG_SELFREGISTER_DATA = "SELF_REG_DATA";

    // Self-Registration param names
    private static final String PARAM_REG_VERSION = "REGVER";
    private static final String PARAM_MEID = "MEID";
    private static final String PARAM_MODEL = "MODEL";
    private static final String PARAM_SW_VER = "SWVER";
    private static final String PARAM_SIM1_CDMAIMSI = "SIM1CDMAIMSI";
    private static final String PARAM_UE_TYPE = "UETYPE";
    private static final String PARAM_SIM1_ICCID = "SIM1ICCID";
    private static final String PARAM_SIM1_LTEIMSI = "SIM1LTEIMSI";
    private static final String PARAM_SIM1_TYPE = "SIM1TYPE";
    private static final String PARAM_SIM2_CDMAIMSI = "SIM2CDMAIMSI";
    private static final String PARAM_SIM2_ICCID = "SIM2ICCID";
    private static final String PARAM_SIM2_LTEIMSI = "SIM2LTEIMSI";
    private static final String PARAM_SIM2_TYPE = "SIM2TYPE";
    private static final String PARAM_MACID = "MACID";
    private static final String PARAM_OS_VER = "OSVER";
    private static final String PARAM_ROM = "ROM";
    private static final String PARAM_RAM = "RAM";
    private static final String PARAM_IMEI1 = "IMEI1";
    private static final String PARAM_IMEI2 = "IMEI2";
    private static final String PARAM_SIM1_CELLID = "SIM1CELLID";
    private static final String PARAM_SIM2_CELLID = "SIM2CELLID";
    private static final String PARAM_DATASIM = "DATASIM";
    private static final String PARAM_SIM1VOLTESW = "SIM1VoLTESW";
    private static final String PARAM_SIM2VOLTESW = "SIM2VoLTESW";
    private static final String PARAM_REGDATE = "REGDATE";

    // Optional Params
    //private static final String PARAM_ACCESSTYPE = "ACCESSTYPE";
    private static final String PARAM_MLPL_VER = "MLPLVER";
    private static final String PARAM_MSPL_VER = "MSPLVER";
    private static final String PARAM_MME_ID = "MMEID";

    // JSON Object member
    private JSONObject mData;
    private static int IndforPrint =3;

    // Service Object
    static SelfRegisterService mService;
    static TelephonyManager tm;
    static SubscriptionManager sm;

    // Member attribute
    int mSimCount = 0;
    int Sim1Type = -1;
    int Sim2Type = -1;
    int mMainCardSlot = -1;
    int mSubCardSlot = -1;

    static final int type_CDMA =2;
    static final int type_LTE=3;

    public SelfRegisterData(SelfRegisterService svc) {
        if (svc == null) {
            Log.e(TAG_SELFREGISTER_DATA, "Created with NULL SelfRegisterService object");
            return;
        }

        mService = svc;
        tm = mService.getTelephonyManager();
        sm = mService.getSubscriptionManager();
        if (tm == null || sm == null )
            Log.e(TAG_SELFREGISTER_DATA, "Constrcution failed due to the NULL Tel/Sub Manager object");

        mMainCardSlot = mService.getMainCardSlotId();
        mSimCount = mService.getSimCount();
        if (mSimCount >= 2)
            mSubCardSlot = (mService.getMainCardSlotId() + 1) % mSimCount;

        mData = new JSONObject();
    }

/**
* Return Encoded SelfRegisterData
* @since   1.0
*/
    public String getSelfRegisterData() {
        String data;
        byte[] encodedByte;

        generateSelfRegisterData();

        if (mData == null) {
            Log.e(TAG_SELFREGISTER_DATA, "[getSelfRegisterData] Self register data is not generated");
            return null;
        }

        data = mData.toString();
        Log.d(TAG_SELFREGISTER_DATA, "Self register data : " + data);

        encodedByte = Base64.encode(data.getBytes(), Base64.DEFAULT);

        return new String(encodedByte);
    }
/**
* Return SelfRegisterData for CTC auto registration
* @since   1.0
*/
  public void generateSelfRegisterData () {
        try {
            Log.d(TAG_SELFREGISTER_DATA, "[getSelfRegisterData] Started");
          mData.put(PARAM_REG_VERSION, REGVER);
          mData.put(PARAM_MEID, getMeid());
          mData.put(PARAM_MODEL, getModel());
          mData.put(PARAM_SW_VER, getSwVer());
          mData.put(PARAM_SIM1_CDMAIMSI, getSimImsi(SLOT1,type_CDMA));
          mData.put(PARAM_UE_TYPE, getUEType());
          mData.put(PARAM_SIM1_ICCID, getSimIccId(SLOT1));
          mData.put(PARAM_SIM1_LTEIMSI, getSimImsi(SLOT1,type_LTE));
          mData.put(PARAM_SIM1_TYPE, getSimType(SLOT1));
          mData.put(PARAM_SIM2_CDMAIMSI, getSimImsi(SLOT2,type_CDMA));
          mData.put(PARAM_SIM2_ICCID, getSimIccId(SLOT2));
          mData.put(PARAM_SIM2_LTEIMSI, getSimImsi(SLOT2,type_LTE));
          mData.put(PARAM_SIM2_TYPE, getSimType(SLOT2));
          mData.put(PARAM_MACID, getMacAddr());
          mData.put(PARAM_OS_VER, getOsVer());
          mData.put(PARAM_ROM, getROM());
          mData.put(PARAM_RAM, getRAM());
          mData.put(PARAM_IMEI1, getIMEI(SLOT1));
          mData.put(PARAM_IMEI2, getIMEI(SLOT2));
          mData.put(PARAM_SIM1_CELLID, getCELLID(SLOT1));
          mData.put(PARAM_SIM2_CELLID, getCELLID(SLOT2));
          if(isEmpty()==true)
          mData.put(PARAM_DATASIM, "");
          else
          mData.put(PARAM_DATASIM,Integer.toString(mMainCardSlot+1));
          mData.put(PARAM_SIM1VOLTESW, getVoLTESW(SLOT1));
          mData.put(PARAM_SIM2VOLTESW, getVoLTESW(SLOT2));
          /*
          if(isEmpty()==true)
          mData.put(PARAM_ACCESSTYPE,"");
          else
          mData.put(PARAM_ACCESSTYPE, getAccessType());
          */
          mData.put(PARAM_REGDATE, getRegDate());

          Log.d(TAG_SELFREGISTER_DATA, "[getSelfRegisterData]" +mData.toString(IndforPrint));
          Log.d(TAG_SELFREGISTER_DATA, "[getSelfRegisterData] Done");
        } catch (Exception ex) {
            Log.e(TAG_SELFREGISTER_DATA, "[generateSelfRegisterData] JSON Object Exception  " +ex );
            ex.printStackTrace();
        }

    }


/**
 * Return MEID values of the Terminal
 * @return <code>deviceId</code> Return MEID if the deviceId exist
 *         <code>""</code> if deviceId is null
 * @since   1.0
 */
 private String getMeid() {
         String deviceId=Build.getSerial();
         //deviceId = SystemProperties.get("ro.vendor.meizu.hardware.meid");
         if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[getMeid] MEID ="+deviceId);
         if (deviceId == null || deviceId.isEmpty()) {
             if (tm != null) {
                 deviceId = tm.getDeviceId();
                 return deviceId;
             } else {
                Log.e(TAG_SELFREGISTER_DATA,
                      "[getMeid]getMeid Cannot get the device ID due to the Telphony Manager object is null");
                 return "";
             }
         }
         return deviceId;
}

/**
 * Return Model Name of the Terminal
 * @return <code>Model</code> Return Model if the value of the Model field exist
 *         <code>""</code> if Model is null
 * @since   1.0
 */
    private String getModel() {
        //String Model = SystemProperties.get("ro.product.model"); //ro.product.model
        String Model =Build.MODEL;
        if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[getModel] Model =" + Model);

        if (Model == null) {
            return "";
        } else if (Model.length() > MAX_MODEL_LENGTH) {
            Log.e(TAG_SELFREGISTER_DATA, "[getModel]Model string is longer than allowed (" + Model.length());
            Model = Model.substring(0, MAX_MODEL_LENGTH);
            return Model;
        }

        return Model;
    }
/**
* Return SW Version of the Terminal
* @return          <code>SwVer</code> Return SwVer if the value of the SwVer field exist
* @since           1.0
*/
    public String getSwVer() {
        //String SwVer = Build.DISPLAY;
        String SwVer = SystemProperties.get("ro.build.id");
        if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[getSwVer] SW Version String = " + SwVer);
        if (SwVer.length() > MAX_SWVER_LENGTH) {
            Log.e (TAG_SELFREGISTER_DATA, "[getSwVer] SW Version string exceed the maximum length ("+SwVer.length()+"bytes");
            SwVer = SwVer.substring(0, MAX_SWVER_LENGTH);
        }
        if(SwVer ==null ||SwVer.length() ==0) return "";

        return SwVer;
    }

/**
* Return IMSI value of the each SIM card
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @param  type the type of the SIM card(3GPP or 3GPP2)
* @return <code>imsi</code> Return imsi if the value of the imsi field exist
*         <code>""</code> if imsi is null
* @since           2.0
*/
  private String getSimImsi(int slot, int type) {
      boolean result= tm.hasIccCard(slot);
      boolean isCTCSim =mService.checkCtcSimfromphoneId(slot);
      if(result == false) return "";
      if(!isCTCSim){
             if(mService.checkCMSimfromphoneId(slot)) return "222222222222222";
             else if(mService.checkCUSimfromphoneId(slot)) return "111111111111111";
      }

      String imsi =   getIMSIfromRil(slot,type);
      int slotId = slot;

      Log.d(TAG_SELFREGISTER_DATA, "[getSimImsi]getSimImsi( "+slot+") : "+imsi);

      if (imsi == null) return "";
      return imsi;

  }

/**
* Return ICCID value of the each SIM card
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @return          <code>Iccid</code> Return Iccid of specific slot if the value of the Iccid field exist
*                  <code>""</code> if imsi is null
* @since           1.0
*/
  private String getUEType(){
     boolean rSim1= tm.hasIccCard(SLOT1);
     boolean rSim2= tm.hasIccCard(SLOT2);
     if(rSim1 == false &&  rSim2 == false) return "";
      return "1";
    }
/**
* Return ICCID value of the each SIM card
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @return          <code>Iccid</code> Return Iccid of specific slot if the value of the Iccid field exist
*                  <code>""</code> if imsi is null
* @since           1.0
*/
  private String getSimIccId(int slot){
        // NO-SIM case
        boolean result= tm.hasIccCard(slot);
        if(result == false) return "";

       String[] Iccid;
       Iccid = mService.getCurrIccId();
       if(Iccid[slot]==null) return "";

       if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[getSimIccId]SIM [" + slot + "] ICCID [" + Iccid[slot] + "]");
       try {
           return Iccid[slot];
       } catch (Exception e) {
           Log.e(TAG_SELFREGISTER_DATA, "[getSimIccId] Failed to get SIM1 ICCID - Exception " + e);
           return "";
       }
 }
/**
* Return type of the each SIM card
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @return          <code>1</code> if lteimsi is null
*                  <code>2</code> if lteimsi exist
* @since           1.0
*/
private String getSimType(int slotId) {
    boolean result= tm.hasIccCard(slotId);
    if(result == false) return "";
     String Imsi;
     Imsi = getSimImsi(slotId,type_LTE);
            if (Imsi.length() > 0){
                if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[getSimType]SIM TYPE of Slot["+slotId+"]:UICC");
                return "1";
             }
             else{
              if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[getSimType]SIM TYPE of Slot["+slotId+"]:ICC");
                return "2";
              }
        }

    /**
    * Return MACID of the terminal
    * @return <code>ver</code> Terminal Mac address
    * @since  8.0
    */
    public String getMacAddrFromNI() {
         try {
                     List<NetworkInterface> allNetIF = Collections.list(NetworkInterface.getNetworkInterfaces());
                     for (NetworkInterface netIF : allNetIF) {
                         if (!netIF.getName().equalsIgnoreCase("wlan0")) continue;
                         byte[] rawData = netIF.getHardwareAddress();
                         if (rawData == null) return "";

                         StringBuilder modifiedMac = new StringBuilder();
                         for (byte b : rawData) {
                             modifiedMac.append(String.format("%02X:",b));
                         }

                         if (modifiedMac.length() > 0) {
                             modifiedMac.deleteCharAt(modifiedMac.length() - 1);
                         }
                         return modifiedMac.toString();
                     }
         } catch(Exception e){
                     e.printStackTrace();
                     return "";
         }
         return "";
    }

/**
* Return MACID of the terminal
* @return <code>ver</code> Terminal Mac address
* @since  1.0
*/

 public String getMacAddr() {
       try {
           String storedMac = mService.readMacAddress();
           if ( storedMac.indexOf("02:00:00") == -1 && (!storedMac.isEmpty())) {
            Log.d(TAG_SELFREGISTER_DATA,"storedMac : "+ storedMac);
            return storedMac;
           }
           boolean bWifiForcedOn = false;
           String mac = "02:00:00:00:00:00";
           WifiManager wifiManager = (WifiManager)mService.getSystemService(Context.WIFI_SERVICE);
           if (wifiManager.isWifiEnabled() == false) {
               wifiManager.setWifiEnabled(true);
               bWifiForcedOn = true;
           }

           // wait until the WIFI enabled at maximum 3s
           int waitCnt;
           WifiInfo info;
           for (waitCnt = 15; waitCnt > 0 ; waitCnt--) {
                     mac = getMacAddrFromNI();
                   if (mac.indexOf("02:00:00") == -1 && (!mac.isEmpty())) {
                       Log.d(TAG_SELFREGISTER_DATA, "[getMacAddr] Read MAC address in device = , stop waiting  : " + mac);
                       break;
                   }
                   android.os.SystemClock.sleep(200);
           }
           if (waitCnt == 0) Log.e(TAG_SELFREGISTER_DATA, "[getMacAddr]Enabling WIFI failed - timeout 5s");

           if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[getMacAddr] WIFI MAC ADDR = " + mac);

           if (bWifiForcedOn) wifiManager.setWifiEnabled(false);
           mService.storeMacAddress(mac);
           return mac;
       } catch (Exception e) {
           e.printStackTrace();
           return "";
       }

   }

/**
* Return Os version of the Android
* @return <code>ver</code> Os version of the Android
* @since  1.0
*/
  private String getOsVer() {
        String ver;
        ver = "Android" + android.os.Build.VERSION.RELEASE;
        if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[17] OS Version string ["+ver+"]");
        return ver;
    }
/**
* Return RAM size of the target
* @return <code>ver</code> H/W version of the target
* @since  1.0
*/
 private String getRAM(){
      MemInfoReader mem = new MemInfoReader();
      mem.readMemInfo();
      int total =(int)(mem.getTotalSize() /1024/1024/1024);
      Log.d(TAG_SELFREGISTER_DATA, "RAM ["+total+"] totalSize: "+mem.getTotalSize());
      return String.valueOf(total+1) +"G";
  }

/**
* Return ROM size of the target
* @return <code>ver</code> H/W version of the target
* @since  1.0
*/
  private String getROM() {
      StatFs fs = new StatFs(Environment.getExternalStorageDirectory().getPath());
      long blockSize = fs.getBlockSize();
      long totalSize = fs.getBlockCount() * blockSize;
      long total = totalSize / 1024 / 1024 / 1024;

      String result="";
      //Check Boundary
      if(total <= 16)
        result = "16G";
      else if(total <= 32)
        result = "32G";
      else if(total <= 64)
        result = "64G";
      else if(total <= 128)
        result = "128G";
      else
        result ="64G";

      if(DBG) Log.d(TAG_SELFREGISTER_DATA, "ROM ["+result+"]");
      return result;
 }
/**
* Return Current base station ID
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @return          <code>id</code> BaseStationId
* @since           1.0
*/
  private String getCELLID(int slot) {
     // NO-SIM case
     boolean result= tm.hasIccCard(slot);
     if(result == false){
      Log.d(TAG_SELFREGISTER_DATA, "No Sim card ["+slot+"]");
      return "";
     }
     ConnInfo conn;
     conn = mService.getConnInfo();
     int cs_cid = conn.getCid(slot);
     int ps_cid = conn.getCidforData(slot);
     int cs_bssid = conn.getBasestationId(slot);

      Log.d(TAG_SELFREGISTER_DATA, "getCellID["+ slot + "]: 1.ps_cid:"+ps_cid+ " 2.cs_cid:"+cs_cid+" 3.cs_bssid:"+cs_bssid);

     //Priority : LTE>WCDMA>GSM(CellId) > CDMA(BSSID)
       //[1] Check PS CID exist
       if(ps_cid!=-1 && ps_cid!=0) return Integer.toString(ps_cid);
       //[2] Check CS CID exist
       if(cs_cid!=-1 && cs_cid!=0) return Integer.toString(cs_cid);
       //[3] Check BSS ID exist
       if(cs_bssid!=-1 && cs_bssid!=0) return Integer.toString(cs_bssid);

       Log.d(TAG_SELFREGISTER_DATA, "Nothing exist");
       return "";
 }

/**
* Return IMEI of specific sim slot
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @return          <code>Imei</code> Imei of the specific sim
* @since           1.0
*/
  private String getIMEI(int slot) {
         if (tm != null) {
             String Imei = tm.getImei(slot);
             if (Imei == null) return "";
             if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[19/20]IMEI Slot ID ["+slot+"] IMEI ["+Imei+"]");
             return Imei;
         } else {
            Log.e(TAG_SELFREGISTER_DATA, "Cannot get the IMEI due to the Telphony Manager object is null");
             return "";
         }
     }

/**
* Return Current base station ID
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @return          <code>id</code> BaseStationId
* @since           1.0
*/
  private int getBasestationId() {
        int id;
        ConnInfo conn = mService.getConnInfo();
        id = conn.getBasestationId();
        if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[21]Base station ID = " + id);
        return id;
    }
/**
* Return VoLTE SW switch state of the card
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @return          <code>"1"</code> open
*                  <code>"2"</code> close
                   <code>"3"</code> Not supported
* @since           1.0
*/
  private String getVoLTESW(int slot) {
    boolean result= tm.hasIccCard(slot);
    if(result == false){
     Log.d(TAG_SELFREGISTER_DATA, "No Sim card ["+slot+"]");
     return "3";
    }
    SubscriptionInfo info = sm.getActiveSubscriptionInfoForSimSlotIndex(slot);
    int subId= info.getSubscriptionId();
    CarrierConfigManager configManager = (CarrierConfigManager)mService.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
    Boolean volteEnabled =false;

    if (configManager != null) {
        PersistableBundle bundle = configManager.getConfigForSubId(subId);
        if (bundle != null) {
            volteEnabled = bundle.getBoolean(
                    CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL);
        }
    }
    if(!volteEnabled) return "3";
    else {
          if ( slot == mMainCardSlot && mService.isMainSimCTC()==false)  //Main SIM - VoLTE capable
                return "1";
          else
                return "2";
    }

}

/**
* Return  Current AccessType
* @return 1 or 2
* @since  2.0
*/
   private String getAccessType(){
     if(mService.isWiFiConnected()==false) return "1";
     else return "2";
   }
/**
* Return  Current RegDate
* @return date
* @since  2.0
*/
 public String getRegDate(){
    Calendar calendar = Calendar.getInstance();
    SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    //format.setTimeZone(TimeZone.getTimeZone("GMT"));
    String regTime = format.format(calendar.getTimeInMillis());
    Log.d(TAG_SELFREGISTER_DATA, "getRegDate"+regTime);
    return regTime;
 }

// Optional Field

// Extra APIs for NetworkInformation

/**
* Return current CDMA network SID value
* @return          <code>sid</code> Return sid of the current CDMA network SID value
*                  <code>0</code> if connection information is null
* @since           1.0
*/
    private int getSid() {
        ConnInfo conn;
        int sid;
        conn = mService.getConnInfo();
        if (conn != null) {
            sid = conn.getSid();
             if (sid ==-1)
              if(DBG) Log.d(TAG_SELFREGISTER_DATA, "[14] Sid: SID is NULL");
            return sid;
        }
        Log.e(TAG_SELFREGISTER_DATA, "[14] getSid, Failed to get connection info");
        return 0;
    }

/* Return CID value of the termianl
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @return          <code>Iccid</code> Return Iccid of specific slot if the value of the Iccid field exist
*                  <code>""</code> if imsi is null
* @since           1.0
*/
   private int getCid(int slot) {
         int id;
         ConnInfo conn = mService.getConnInfo();
         id = conn.getCid(slot);
         Log.d(TAG_SELFREGISTER_DATA, "GSM Cell ID = " + id);
         return id;
      }
  private int getCid() {
      int id;
      ConnInfo conn = mService.getConnInfo();
      id = conn.getCid();
      Log.d(TAG_SELFREGISTER_DATA, "getCid = " + id);
      return id;
   }


/* Return CID value of the termianl
* @param  slot # of SIM Slot (SLOT1=0 SLOT2=1)
* @return          <code>Iccid</code> Return Iccid of specific slot if the value of the Iccid field exist
*                  <code>""</code> if imsi is null
* @since           1.0
*/
    private boolean isEmpty(){
      boolean result1= tm.hasIccCard(SLOT1);
      boolean result2= tm.hasIccCard(SLOT2);
      if((result1==false) && (result2==false)) return true;
      else return false;
    }

/* Return IMSI value of the each case (CDMA/LTE)
* @param  type of Network technology
* @return          <code>Iccid</code> Return Imsi of specific type if the value of the Imsi field exist
*                  <code>""</code> if imsi is null
* @since           8.0
*/
   public String getIMSIfromRil(int slot,int type) {
       String propKeyLTEIMSI      = "vendor.ril.gsm.sim.imsi_"+slot;
       String propKeyCDMAIMSI  =  "vendor.ril.cdma.sim.imsi_"+slot;
       String imsi="";

       if(type ==type_CDMA){
           imsi = SystemProperties.get(propKeyCDMAIMSI,"");
           Log.d(TAG_SELFREGISTER_DATA,"getIMSIfromRil : CDMA "+ propKeyCDMAIMSI+ " :  " + imsi);
       }else if (type ==type_LTE){
           imsi = SystemProperties.get(propKeyLTEIMSI,"");
           Log.d(TAG_SELFREGISTER_DATA, "getIMSIfromRil : LTE  "+ propKeyLTEIMSI + "  :  " + imsi);
       }

       return imsi;
   }

}

