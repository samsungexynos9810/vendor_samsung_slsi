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

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;

import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemProperties;

import android.provider.Settings;
import android.telephony.CellLocation;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.CellInfo;
import android.telephony.CellInfoCdma;
import android.telephony.CellInfoGsm;
import android.telephony.CellInfoLte;
import android.telephony.CellInfoWcdma;
import android.telephony.CellIdentityCdma;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityWcdma;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;
import android.telephony.TelephonyManager;

import android.text.TextUtils;
import android.util.Log;

import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

public class SelfRegisterService extends Service {

    // Debug flag
    private static final Boolean DBG = true;
    // Constants
    private static final String TAG_SELFREGISTER_SVC = "SELF_REG_SVC";
    private static final String KEY_REGISTERED_ICCID = "ctc.selfregister.iccid";
    private static final String REBOOT_COMPLETE ="android.intent.action.BOOT_COMPLETED";
    private static final String RETRY_ACTION = "com.samsungslsi.ctcselfregister.RETRY";
    private static final String T305_TIMER = "com.samsung.slsi.CtcSelfRegister.T305_TIMER";
    private static final String T30_TIMER = "com.samsung.slsi.CtcSelfRegister.T30_TIMER";
    public static final int MAX_SIM_COUNT = 2;

    // Message types
    private static final int MSG_SELF_REG_REGISTER = 0;
    private static final int MSG_SELF_REG_RESULT = 1;
    private static final int MSG_SELF_REG_MONTHLY = 2;
    private static final int StatusforMonthlyAlarm  =  11;
    private static final int StatusforFunctional=  17;
    // JSON field in response
    private static final String RESULT_CODE = "resultCode";
    private static final String RESULT_DESC = "resultDesc";

    private static final int MULTIDAY_REGI_DONE =  24;
    private static final int FBOOT_DONE = 49;
    private static final int MULTIDAY_REGI_RESET =  6;
    private static final long tBootInterval = 732 * 60 * 60 * 1000L; //for mills (30.5days)
    private static final long tInterval = 30 * 24 * 60 * 60 * 1000L; //for mills (30days)

    private TelephonyManager mTelManager;

    private static SubscriptionManager mSubManager;
    private static ConnectivityManager mConnManager;
    private static AlarmManager mAlarmManager;
    private static WifiManager mWifiManager;
    private static PhoneStateListener mPhoneStateListener;
    private static Context mContext;
    private static SelfRegisterPreference mPref;
    private static ConnInfo mConnInfo;
    private static List<SubscriptionInfo> mSubList;

    private Intent intent_T30 = new Intent(T30_TIMER);
    private Intent intent_T305 = new Intent(T305_TIMER);
    private PendingIntent pi_T30;
    private PendingIntent pi_T305;

    private String[] mCurrIccId;

    public enum State {
        IDLE,
        STARTING
    }
    public static State mState;

    @Override
    public void onCreate() {

        Log.d (TAG_SELFREGISTER_SVC, "onCreate : BEGIN");
        // Check the condition whether Self registration is required.
        mContext = this;

        mPref = new SelfRegisterPreference();
        mPref.init(mContext);

        mConnInfo = new ConnInfo(this);


        mTelManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        mConnManager = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
        mAlarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        mSubManager =(SubscriptionManager) getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);

        if (mTelManager == null ||mSubManager ==null||mConnManager == null || mAlarmManager == null)
            Log.e(TAG_SELFREGISTER_SVC, "onCreate: mTelManager/mConnManager/mSubManager/mAlarmManager is NULL");

        mCurrIccId = new String[MAX_SIM_COUNT];

        registerPhoneStateListener();

        pi_T30 = PendingIntent.getBroadcast(mContext, 0, intent_T30, PendingIntent.FLAG_UPDATE_CURRENT);
        pi_T305 = PendingIntent.getBroadcast(mContext, 0, intent_T305, PendingIntent.FLAG_UPDATE_CURRENT);

        mState = State.IDLE;


    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    @Override
    public void onDestroy() {

        if (mTelManager != null) {
            mTelManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
            mTelManager = null;
        }
        super.onDestroy();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null) {
            Log.e(TAG_SELFREGISTER_SVC,
                "Intent is NULL, stop service" );
            return START_NOT_STICKY;
        }

        String action = intent.getAction();

        do{
          if(mPref!=null) break;
          Log.d (TAG_SELFREGISTER_SVC, "Waiting");
        }while(true);

        /*
         (1) Condition
            a. T30.5-Alarm trigger after first regi finished. (If not, it will be delayed every 30.5 days)
            b. In case of T30-Alarm, This will be set after T30.5-Alarm executed and registration success!!.
        */
        //Register reboot alarm
        setRBAlarm(action);

        //Check Registration should be happened
        boolean registatus = checkRegiTime(action);

        if (action.equalsIgnoreCase("android.net.conn.CONNECTIVITY_CHANGE") ||
            action.equalsIgnoreCase("android.intent.action.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED") ||
            action.equalsIgnoreCase(T305_TIMER) ||
            action.equalsIgnoreCase(T30_TIMER) ||
            action.equalsIgnoreCase(RETRY_ACTION)){
            Log.d(TAG_SELFREGISTER_SVC, "Action : "+ action);

            if(action.equalsIgnoreCase("android.net.conn.CONNECTIVITY_CHANGE") &&
                intent.getBooleanExtra(ConnectivityManager.EXTRA_NO_CONNECTIVITY, false)){
                Log.d(TAG_SELFREGISTER_SVC, "NoConnectivity(return), stop starting service");
                return START_NOT_STICKY;
            }

            //boolean handover = checkRatInfo(0) || checkRatInfo(1);

            if (!action.equalsIgnoreCase(RETRY_ACTION)){
                // reset the retry count only when the SIM update
                //mPref.reset();
                Log.d(TAG_SELFREGISTER_SVC, "do not reset the retry count");
            }
            //(1)Timer event : ignore if registration already succeeded
            if(action.equalsIgnoreCase(T305_TIMER)||action.equalsIgnoreCase(T30_TIMER)){
                Log.d(TAG_SELFREGISTER_SVC, "Start MonthlyRegistration");
                Message msg = mHandler.obtainMessage(MSG_SELF_REG_MONTHLY, null);
                mHandler.sendMessage(msg);
                return START_NOT_STICKY;
            }
            //(2) Non-Timer event : send Forcely if (current - previous regiT) > 30 + 1min
            if(registatus == true){
                Log.d(TAG_SELFREGISTER_SVC, "Start MonthlyRegistration(Forcely)");
                Message msg = mHandler.obtainMessage(MSG_SELF_REG_MONTHLY, null);
                mHandler.sendMessage(msg);
                return START_NOT_STICKY;
            }
            /*
           if(action.equalsIgnoreCase("android.net.conn.CONNECTIVITY_CHANGE") && handover){
                Log.d(TAG_SELFREGISTER_SVC, "HandOver Case");
                Message msg = mHandler.obtainMessage(MSG_SELF_REG_REGISTER, null);
                mHandler.sendMessage(msg);
                return START_NOT_STICKY;
           }
           */
            Message msg = mHandler.obtainMessage(MSG_SELF_REG_REGISTER, null);
            mHandler.sendMessage(msg);
        }
        return START_NOT_STICKY;
    }

   public TelephonyManager getTelephonyManager() {return mTelManager;}
   public SubscriptionManager getSubscriptionManager() {return mSubManager;}
   public WifiManager getWifiManager() {
       mWifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
       return mWifiManager;
   }
   public Context getContext() {return mContext;}
   public ConnInfo getConnInfo() {return mConnInfo;}

/*
  private boolean checkRatInfo(int slot)
  {
       boolean result= mTelManager.hasIccCard(slot);
       mSubManager =(SubscriptionManager) getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
       if(!result||mSubManager==null) return false;

       SubscriptionInfo info = mSubManager.getActiveSubscriptionInfoForSimSlotIndex(slot);
       if(info!=null){
           int subId= info.getSubscriptionId();
           int ratType = mTelManager.getNetworkType(subId);
           int prevType = mPref.readRatInfo(slot);
           Log.d (TAG_SELFREGISTER_SVC, "checkRatInfo : " +ratType+" (cur) & "+prevType+" (prev) , slot : "+slot);
           if(ratType == prevType || prevType==0) return false;
           else return true;
       }
       return false;
  }


private void saveRatInfo(int slotIdx)
{
      if(mTelManager==null) return;
      boolean result1= mTelManager.hasIccCard(0);
      boolean result2= mTelManager.hasIccCard(1);

      try{
          SubscriptionInfo info = mSubManager.getActiveSubscriptionInfoForSimSlotIndex(slotIdx);
          int subId= info.getSubscriptionId();
          int rat_slot= mTelManager.getNetworkType(subId);
          //mPref.saveRatInfo(slotIdx,rat_slot);
          Log.d(TAG_SELFREGISTER_SVC, "[saveRatInfo]  saveRatInfo : " +rat_slot);
      } catch (Exception e) {
          Log.d(TAG_SELFREGISTER_SVC, "[saveRatInfo] Exception occured" + e);
      }
}
*/


/**
* Convert System Time to Date
* @since   2.0
*/
   private String convertTimetoDate(long systime){
        long currentTime =System.currentTimeMillis();
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        String convertedsysT = format.format(systime);
        //Log.d (TAG_SELFREGISTER_SVC, "convertedTime: "+convertedsysT);
        return convertedsysT;
   }
/**
* Set Register Alarm after reboot
* @since   2.0
*/
   private void setRBAlarm(String action){

      if(!action.equals(REBOOT_COMPLETE)) return;

      int  RegStatus = mPref.readRegStatus();
      Log.d (TAG_SELFREGISTER_SVC, "***setRBAlarm***");
      //Check Boot count
      int count =0;
      try{
          count = Settings.Global.getInt(getContext().getContentResolver(), Settings.Global.BOOT_COUNT);
          Log.d (TAG_SELFREGISTER_SVC, "BT cnt"+count);
      } catch (Settings.SettingNotFoundException e) {
       e.printStackTrace();
      }

    if(count>1){
      long RegiTime = mPref.readRegiTime();
      int RegCnt = mPref.readRegCnt();
       //if(RegiTime ==0||RegStatus ==StatusforFunctional){
       if(RegiTime == 0) {
        Log.d (TAG_SELFREGISTER_SVC, "[setRBAlarm] RegiTime == 0");
        return;
      }
       if(RegStatus ==StatusforFunctional){
            //cancel previous one
            mAlarmManager.cancel(pi_T30);
            mAlarmManager.cancel(pi_T305);
            //re-register new one
            mAlarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, RegiTime+ tBootInterval, pi_T305);
            Log.d (TAG_SELFREGISTER_SVC, "REBOOT_T305ExactTime: "+convertTimetoDate(RegiTime+ tBootInterval));
       }else if(RegCnt >=1 && RegStatus ==StatusforMonthlyAlarm){
            //cancel previous one
            mAlarmManager.cancel(pi_T30);
            mAlarmManager.cancel(pi_T305);
            //re-register new one
            mAlarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, RegiTime+ tInterval, pi_T30);
            Log.d (TAG_SELFREGISTER_SVC, "REBOOT_T30ExactTime: "+convertTimetoDate(RegiTime+ tInterval));
       }
    }
  }
/**
* set Alarm for Monthly-registration before AutoLogin registration
* @since   2.0
*/
 private boolean checkRegiTime(String action){
    //return for REBOOT
    if(action.equals(REBOOT_COMPLETE) || action.equals(T305_TIMER) || action.equals(T30_TIMER)) return false;
     Log.d (TAG_SELFREGISTER_SVC, "[checkRegiTime]Check RegiTime(action : "+action+" )");

    long current = System.currentTimeMillis();
    long regiTime = mPref.readRegiTime();
    long margin = 60 * 1000L; //for mills (1min)
    int RegCnt = mPref.readRegCnt();
    int RegStatus = mPref.readRegStatus();

    if(RegStatus == StatusforFunctional){
           if(regiTime!=0 && (current-regiTime) >= tBootInterval+margin){
                Log.d (TAG_SELFREGISTER_SVC, "[checkRegiTime]Forcely Send due to Last duration(30.5)");
                return true;
           }
           else return false;
     }else if (RegCnt >=1 && RegStatus == StatusforMonthlyAlarm){
           if(regiTime!=0 && (current-regiTime) >= tInterval+margin){
                Log.d (TAG_SELFREGISTER_SVC, "[checkRegiTime]Forcely Send due to Last duration(30)");
                return true;
           }
           else return false;
     }

      return false;
  }
/**
* set Alarm for Monthly-registration after AutoLogin registration
* @since   2.0
*/
   private void setMonthlyAlarmforRegi(){
       mAlarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
       long current =mPref.saveRegiTime();
       int  RegStatus = mPref.readRegStatus();
       int  RegCnt    = mPref.readRegCnt();

        if(RegStatus == StatusforMonthlyAlarm){
            //cancel previous one
            mAlarmManager.cancel(pi_T30);
            mAlarmManager.cancel(pi_T305);
            //re-register new one
            mAlarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, current+ tInterval, pi_T30);
            Log.d(TAG_SELFREGISTER_SVC, "[REGI_DONE]T30 case(Second Monthly Alarm):"+convertTimetoDate(current+ tInterval));
            RegCnt++;
            saveRegiCnt(RegCnt);
            Log.d(TAG_SELFREGISTER_SVC, "[REGI_DONE]Save RegCnt:" + mPref.readRegCnt());
        }
        else if(RegStatus == StatusforFunctional){
            //cancel previous one
            mAlarmManager.cancel(pi_T30);
            mAlarmManager.cancel(pi_T305);
            //re-register new one
            mAlarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, current+ tBootInterval, pi_T305);
            Log.d(TAG_SELFREGISTER_SVC, "[REGI_DONE]T305 case(Functional Triggering):"+convertTimetoDate(current+ tBootInterval));
            saveRegiCnt(0);
            Log.d(TAG_SELFREGISTER_SVC, "[REGI_DONE]Go to Initial Status:" + mPref.readRegCnt());
        }
        //Save this for rebooting
        saveRegiStatus(RegStatus);
   }

/**
* Return Network status
* @return <code>true</code> Return true if the Network is connected
*         <code>""</code> Return false if the Network is not connected
* @since   1.0
*/
    private boolean isDataRegistered()
    {
         if(mTelManager.getDataState()!=TelephonyManager.DATA_CONNECTED){
             Log.d(TAG_SELFREGISTER_SVC, "[isDefaultDataRegistered]Data Not registered");
             return false;
         }
         return true;
    }

/**
* Return Network status
* @return <code>true</code> Return true if the Network is connected
*         <code>""</code> Return false if the Network is not connected
* @since   1.0
*/
    private Boolean isNetworkConnected() {
        if (mConnManager == null || mTelManager == null || mSubManager== null) return false;
        final android.net.NetworkInfo mobile = mConnManager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        final int voiceConnected=2;

        List<CellInfo> cellList;
        int bssid,cid;
        bssid = mConnInfo.getBasestationId();
        cid = mConnInfo.getCid();

        Log.d(TAG_SELFREGISTER_SVC,
                "Cell INFO ID = " + cid +", BSSID = " + bssid + ", Network is available: WIFI["+isWiFiConnected()+
                "], Mobile["+mobile.isConnected()+"]");
        // Check whether CTC SIM connected (NID, SID, BSSID should be valid)
        int slot;
        int[] subIds;
        boolean bCtcInserted = false;

       // WIFI or Cellular should be connected
        if(!isDataRegistered() && !isWiFiConnected()){
            Log.d(TAG_SELFREGISTER_SVC, "[isNetworkConnected]DataNotRegistered(WIFI & Cellular)");
            setRetryAlarm("NOT CONNECTED");
            return false;
        }

       if(isWiFiConnected()) Log.d(TAG_SELFREGISTER_SVC, "[isNetworkConnected]WIFI ON");



       //Even though WIFI connected at the network , we should wait till the mobile
       for (slot=0 ; slot < MAX_SIM_COUNT ; slot++) {
           Log.d(TAG_SELFREGISTER_SVC, "[SLOT"+ slot+"] case");
           if(!mTelManager.hasIccCard(slot)) continue;

             int defaultDataPhoneId = mSubManager.getDefaultDataPhoneId();
             Log.d(TAG_SELFREGISTER_SVC, "[isNetworkConnected]defaultDataPhoneId : " +defaultDataPhoneId);
             if(!checkCtcSimfromphoneId(slot)==true){
                  if(slot == defaultDataPhoneId){
                       Log.d(TAG_SELFREGISTER_SVC, "[isNetworkConnected]DataRegistered at Non-CTCsim");
                       setRetryAlarm("NOT CONNECTED");
                       return false;
                   }
             }

            int voiceRegstatus = mTelManager.getVoiceActivationState(slot);
            if((!checkCtcSimfromphoneId(slot)) && (voiceRegstatus!=voiceConnected)){
                Log.d(TAG_SELFREGISTER_SVC, "[isNetworkConnected]VoiceNotRegistered at Non-CTCsim");
                setRetryAlarm("NOT CONNECTED");
                return false;
            }

            if(isBothCtcSim() && slot != defaultDataPhoneId &&  voiceRegstatus!=voiceConnected){
                 Log.d(TAG_SELFREGISTER_SVC, "[isNetworkConnected]Dual-CTC Sim case, Not registered at voice CTC sim");
                 setRetryAlarm("NOT CONNECTED");
                 return false;
            }

            if (checkCtcSimfromphoneId(slot)){
                bCtcInserted = true;
                Log.d(TAG_SELFREGISTER_SVC, "[isNetworkConnected] bCtcInserted:" +slot);
            }
        }


        if (!bCtcInserted) {
            if (DBG) Log.d(TAG_SELFREGISTER_SVC, "[isNetworkConnected]Could not find CTC SIM");
        }

        if ( bCtcInserted && bssid < 0 ) {
            Log.e(TAG_SELFREGISTER_SVC, "[isNetworkConnected] CDMA is not connected");
            setRetryAlarm("NOT CONNECTED");
            return false;
        }

        if ( ((cid > 0) || (bssid > 0)) && (isWiFiConnected() || mobile.isConnected()) ) {
            return true;
        }

        setRetryAlarm("NOT CONNECTED");
        return false;
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (DBG) Log.d (TAG_SELFREGISTER_SVC, "handleMessage : BEGIN w/ msg [" + msg.toString() +"]" );

            switch (msg.what) {
                case MSG_SELF_REG_REGISTER:
                case MSG_SELF_REG_MONTHLY:
                    if(mState == State.IDLE){
                        mState = State.STARTING;
                        Log.d(TAG_SELFREGISTER_SVC, "state is Started");
                    }else{
                        Log.d(TAG_SELFREGISTER_SVC, "state is Already Started");
                        return;
                    }
                    mTelManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
                    mConnManager = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
                    mSubManager = (SubscriptionManager)getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
                    if (mTelManager == null || mConnManager == null || mSubManager == null) {
                        Log.e(TAG_SELFREGISTER_SVC,
                                "Failed to get Telephony manager, SubscriptionManager  or Connectivity Manager handle" );
                        stopSelf();
                        return;
                    }

                    setSubInfoList();
                    //Read ICCID
                    readIccIdFromCard();
                    //DataReg-50004 :if two card slots of the current dual-card terminal are not inserted subscriber's cards, the self-registration message does not need to be reported
                    if(isIccEmpty()){
                        Log.e(TAG_SELFREGISTER_SVC, "[STOP]Self-registration is not required (Both are empty)");
                        stopSelf();
                        return;
                    }
                    boolean dataSimChanged = isDataSimChanged();
                    boolean storedSIM = isIccidStored();
                    boolean compSwVer = compareSwVer();
                    //boolean ratChanged = (checkRatInfo(0) || checkRatInfo(1));

                    //if (msg.what != MSG_SELF_REG_MONTHLY && !dataSimChanged && storedSIM && compSwVer && !ratChanged) {
                    if (msg.what != MSG_SELF_REG_MONTHLY && !dataSimChanged && storedSIM && compSwVer) {
                        Log.e(TAG_SELFREGISTER_SVC, "[STOP]Self-registration is not required DataSimChanged:" + dataSimChanged + " isIccidStored:" + storedSIM + " compareSwVer:" + compSwVer);
                        stopSelf();
                        return;
                    } else {
                        Log.d(TAG_SELFREGISTER_SVC, "Trigger reason msg: " + msg.what);
                        if(dataSimChanged) Log.d(TAG_SELFREGISTER_SVC, "Trigger reason : Data SIM changed");
                        if(!storedSIM) Log.d(TAG_SELFREGISTER_SVC, "Trigger reason : SIM changed");
                        if(!compSwVer) Log.d(TAG_SELFREGISTER_SVC, "Trigger reason : SW Ver changed");
                        //if(ratChanged) Log.d(TAG_SELFREGISTER_SVC, "Trigger reason : HO happened");
                    }

                    if (isNetworkConnected() == false) {
                        Log.e(TAG_SELFREGISTER_SVC, "[STOP]Not in service, stop service");
                        stopSelf();
                        return;
                    }
                    // Reset retry count increased because of connection failed
                    mPref.resetRetryCount(mPref.RETRY_COUNTER_CONN);
                    if (mTelManager.isNetworkRoaming()) {
                        Log.e(TAG_SELFREGISTER_SVC, "[STOP]The device is in a roaming network");
                        stopSelf();
                        return;
                    }

                    // is WIFI connected?
                    if ( isWiFiConnected() == false) {
                        if (isMainSimCTC() == false) {
                            Log.e(TAG_SELFREGISTER_SVC, "[STOP]WiFi is not connected and the Main SIM is not CTC SIM, stop");
                            stopSelf();
                            return;
                        } else{
                            if(!isDataRegistered()){
                                  Log.e(TAG_SELFREGISTER_SVC, "[STOP]The default PDN is not connected on the CTC SIM, stop");
                                  stopSelf();
                                  return;
                             }
                        }
                    }
                    if(msg.what == MSG_SELF_REG_MONTHLY) saveRegiStatus(StatusforMonthlyAlarm);
                    else saveRegiStatus(StatusforFunctional);
                    // Make thread to send register data
                    new Thread(mSendTask).start();
                    break;
                case MSG_SELF_REG_RESULT:
                    JSONObject resp = null;
                    int resultCode = -1;
                    String resultDesc;
                    if (msg.obj != null) {
                        try {
                            resp = (JSONObject) msg.obj;
                            resultCode = resp.getInt(RESULT_CODE);
                            resultDesc = resp.getString(RESULT_DESC);
                            Log.d(TAG_SELFREGISTER_SVC, "Register response : Status code [" + resultCode + "], Result Desc [" + resultDesc +"]");
                        } catch (JSONException ex) {
                            Log.e(TAG_SELFREGISTER_SVC, "JSON Exception - " + ex.getMessage());
                            ex.printStackTrace();
                        }
                    } else {
                        Log.e(TAG_SELFREGISTER_SVC, "Response message has NULL data");
                    }

                    if (resultCode == 0) {
                        // Success - save ICCIDs and terminate this service
                        Log.d(TAG_SELFREGISTER_SVC, "Registration success");
                        saveDataSim();
                        saveIccId();
                        setMonthlyAlarmforRegi();
                        mPref.resetRetryCnt();
                        //saveRatInfo(0);
                        //saveRatInfo(1);
                        stopSelf();
                    } else {
                        Log.e(TAG_SELFREGISTER_SVC, "Registration failed, set retry alarm and terminate");
                        setRetryAlarm("ERROR Response");
                        stopSelf();
                    }
                    mState = State.IDLE;
                    Log.d(TAG_SELFREGISTER_SVC, "state is IDLE");

                    break;
                default:
                    super.handleMessage(msg);
            }
        Log.d (TAG_SELFREGISTER_SVC, "handleMessage : END w/ msg [" + msg.toString() +"]" );
        }
    };
  private boolean compareSwVer(){
    String currSwVer = SystemProperties.get("ro.build.id");
    String prevSwVer = mPref.readLastSwVer();
     Log.d (TAG_SELFREGISTER_SVC, "compareSwVer : " +prevSwVer +" & "+ currSwVer);
     if(currSwVer.equals(prevSwVer)){
           Log.d (TAG_SELFREGISTER_SVC, "compareSwVer :Same");
           return true;
     }
     else{
           Log.d (TAG_SELFREGISTER_SVC, "compareSwVer :Diff");
           return false;
     }
  }

  private void saveSwVer(String ver){
  if (mPref.saveLastSwVer(ver) == null)
         Log.e(TAG_SELFREGISTER_SVC, "saveSwVer : null");
   Log.d(TAG_SELFREGISTER_SVC, "saveSwVer : "+ver);
  }
 /**
 * Check whether status of DataSim is changed or not
 * @return <code>true</code> Return true if MainDataSim is changed
 *         <code>false</code> Return true if MainDataSim is not changed
 * @since   1.0
 */
    private Boolean isDataSimChanged() {
        int subId = getMainCardSubId();
        int slotId = getMainCardSlotId();
        String mccmnc = mTelManager.getSimOperatorNumeric(subId);

        if(slotId<0 || mccmnc.equals("")){
         Log.d (TAG_SELFREGISTER_SVC, "slotID is invalid : "+ slotId);
         return false;
        }
        if (getMainCardSlotId() != mPref.readDataSim()){
            Log.d (TAG_SELFREGISTER_SVC, "Changed(DataSIM) : "+subId+" /"+ mccmnc);
            return true;
        }
        else{
            Log.d (TAG_SELFREGISTER_SVC, "NotChanged(DataSIM) : "+subId+" /"+ mccmnc);
            return false;
        }
    }
 /**
  * Check whether both slot are empty or not
  * @since   2.0
 */
    private Boolean isIccEmpty(){
        // are they null-slot?
        if((mCurrIccId[0]==null || mCurrIccId[0].isEmpty()) && (mCurrIccId[1]==null || mCurrIccId[1].isEmpty())){
          Log.d(TAG_SELFREGISTER_SVC, "[isIccEmpty] Both are empty , Self-register is not required");
          return true;
        }
        return false;
    }
/**
 * Check whether previous ICCID information exist for not
 * @return <code>true</code> Return true if ICCID previous information exist
 *         <code>false</code> Return true if previous information do not exist
 * @since   1.0
*/
    private Boolean isIccidStored() {
       // SIM ICCID stored?
       readIccIdFromCard();
       // Is there stored ICCIDs?
       String[] StoredIccIds = mPref.readIccIds();
       if((StoredIccIds[0] ==null && mCurrIccId[0]!= null) ||
          (StoredIccIds[0] !=null && mCurrIccId[0]== null) ||
          (StoredIccIds[1] ==null && mCurrIccId[1]!= null) ||
          (StoredIccIds[1]!=null && mCurrIccId[1]== null) ){
         Log.d(TAG_SELFREGISTER_SVC, "[isIccidStored] SIM Status changed ");
         return false;
       }
        //is SLOT1 same?
        if((mCurrIccId[0]!=null)&&(StoredIccIds[0]!=null)&&(mCurrIccId[0].equalsIgnoreCase(StoredIccIds[0])== false)){
          Log.d(TAG_SELFREGISTER_SVC, "[isIccidStored] ICCID_0 is diff, Self-register required ("+mCurrIccId[0]+" / "+StoredIccIds[0]);
          return false;
        }
        // is SLOT2 same?
        if((mCurrIccId[1]!=null)&&(StoredIccIds[1]!=null)&&(mCurrIccId[1].equalsIgnoreCase(StoredIccIds[1])== false)){
          Log.d(TAG_SELFREGISTER_SVC, "[isIccidStored] ICCID_1 is diff, Self-register required ("+mCurrIccId[1]+" / "+StoredIccIds[1]);
          return false;
        }
        return true;
    }
/**
 * Save ICCID information read from SIM Card
 * @since   1.0
*/
    private void readIccIdFromCard() {

    int slotIdx, simCount = 0;
    boolean result1= mTelManager.hasIccCard(0); //SLOT1
    boolean result2= mTelManager.hasIccCard(1); //SLOT2
    mSubManager =(SubscriptionManager) getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);

    for (slotIdx = 0; slotIdx < MAX_SIM_COUNT ; slotIdx++ ) {
           boolean result = mTelManager.hasIccCard(slotIdx);
           Log.d(TAG_SELFREGISTER_SVC, "[readIccIdFromCard] result : "+result+"  slot :" +slotIdx);
           if(!result) continue;
              try{
                  SubscriptionInfo info = mSubManager.getActiveSubscriptionInfoForSimSlotIndex(slotIdx);
                  String IccId= info.getIccId();
                  Log.d(TAG_SELFREGISTER_SVC, "[readIccIdFromCard]  IccId : " +IccId);
                  if (IccId != null && IccId.length() >0) {
                          simCount++;
                          mCurrIccId[slotIdx] = IccId;
                  } else {
                         Log.e(TAG_SELFREGISTER_SVC, "Cannot get SUB ID  for SLOT " + slotIdx);
                         mCurrIccId[slotIdx] = null;
                  }
              } catch (Exception e) {
                  Log.d(TAG_SELFREGISTER_SVC, "[readIccIdFromCard] Exception occured" + e);
              }
          }
    }

/**
 * Return current ICCID information
 * @return  mCurrIccId
 * @since   1.0
*/
    public String[] getCurrIccId() {
        return mCurrIccId;
    }

    private void saveIccId() {
        if (mPref.storeIccIds(mCurrIccId) == false) {
            Log.e(TAG_SELFREGISTER_SVC, "Failed to store ICCIDs");
        }
    }

    private void saveDataSim() {
        if (mPref.storeDataSim(getMainCardSlotId()) == false) {
            Log.e(TAG_SELFREGISTER_SVC, "Failed to store Data SIM");
        }
    }
    private void saveRegiStatus(int st) {
      int status = mPref.readRegStatus();
      Log.d(TAG_SELFREGISTER_SVC, "Previous RegiStatus: "+status);
      Log.d(TAG_SELFREGISTER_SVC, "Current RegiStatus: "+st);
      mPref.saveRegStatus(st);
    }
    private void saveRegiCnt(int cnt) {
      int status = mPref.readRegCnt();
      Log.d(TAG_SELFREGISTER_SVC, "Previous RegiCnt: "+status);
      if(cnt > 10000) cnt =100;
      Log.d(TAG_SELFREGISTER_SVC, "Current RegiCnt: "+cnt);
      mPref.saveRegCnt(cnt);
    }
    private void registerPhoneStateListener() {
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        int slot;
        int[] subIds;
        for (slot=0 ; slot < MAX_SIM_COUNT ; slot++) {
            subIds = SubscriptionManager.getSubId(slot);
            if (subIds != null && isCtcSim(subIds[0])){
                subId = subIds[0];
                break;
            }
        }
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            subId = SubscriptionManager.getDefaultDataSubscriptionId();
            if (DBG) Log.d(TAG_SELFREGISTER_SVC, "Could not find CTC SIM, use default SUB ID (" + subId + ")");
        }

        if (DBG) Log.d(TAG_SELFREGISTER_SVC, "registerPhoneStateListner : BEGIN subId (" + subId +")");
        mPhoneStateListener = new PhoneStateListener(subId) {
            @Override
            public void onServiceStateChanged(ServiceState serviceState) {
                mConnInfo.setServiceState(serviceState);
                Log.d(TAG_SELFREGISTER_SVC, "[PhoneStateListener] the state changed to " + mConnInfo.getServiceState().toString() );
            }

        };

        mTelManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_SERVICE_STATE);
        if (DBG) Log.d(TAG_SELFREGISTER_SVC, "registerPhoneStateListner : END");
    }

    private void setRetryAlarm(String reason) {
        Log.d(TAG_SELFREGISTER_SVC, "setRetryAlarm reason = " + reason);

        int counterType = mPref.RETRY_COUNTER_FAIL;

        if (reason.equalsIgnoreCase("NOT CONNECTED")) {
            counterType = mPref.RETRY_COUNTER_CONN;
        }

        int retryCount = mPref.readRetryCount(counterType);
        if (retryCount >= mPref.MAX_RETRY_COUNT) {
            if (counterType == mPref.RETRY_COUNTER_CONN) {
                Log.e(TAG_SELFREGISTER_SVC, "Exceed the maximum Connection retry count, retry after one HOUR");
                counterType = mPref.RETRY_COUNTER_FAIL;
                reason = "TRY LATER";
            } else {
                Log.e(TAG_SELFREGISTER_SVC, "Exceed the maximum retry count, Self-registration failed!");
                mPref.resetRetryCnt();
                return;
            }
        }

        mPref.increaseRetryCount(counterType);
        Log.d(TAG_SELFREGISTER_SVC, "[setRetryAlarm] Retry later .. (" + mPref.readRetryCount(counterType) + ")");

        AlarmManager alarm = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        Intent intent = new Intent(RETRY_ACTION, null, this, SelfRegisterService.class);
        PendingIntent pi = PendingIntent.getService(this, 0, intent, PendingIntent.FLAG_ONE_SHOT);

        if (reason.equalsIgnoreCase("NOT CONNECTED"))
            alarm.setExact(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + mPref.CONNECTION_RETRY_TIME, pi);
        else alarm.setExact(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + mPref.RETRY_TIME, pi);
    }

    public boolean isWiFiConnected() {
        boolean result = false;
        Network[]   networks;
        NetworkInfo ni;

        networks = mConnManager.getAllNetworks();
        for (Network net: networks) {
            ni = mConnManager.getNetworkInfo(net);
            if ((ni != null) && ((ni.getType() == ConnectivityManager.TYPE_WIFI) &&  ni.isConnected())) {
                Log.v(TAG_SELFREGISTER_SVC, "Connected WIFI connection found [" + ni.toString());
                result = true;
                if(result) {
                    Log.v(TAG_SELFREGISTER_SVC, "wait isWiFiConnected");
                    android.os.SystemClock.sleep(500);
                }
                break;
            }
        }
        return result;
    }

    public boolean isMainSimCTC() {
        return isCtcSim(getMainCardSubId());
    }

     public boolean checkCMSimfromphoneId(int slotId) {
        boolean result = false;
        final String MccMnc = mTelManager.getSimOperatorNumericForPhone(slotId);
        Log.d(TAG_SELFREGISTER_SVC, "MNCMCC from slotId ("+slotId+" [" + MccMnc +"]");

        if (MccMnc.equals("46000") || MccMnc.equals("46002") ||  MccMnc.equals("46007") ||
            MccMnc.equals("45513") || MccMnc.equals("45428")|| MccMnc.equals("45412")) {
            result = true;
        }
        return result;
     }
    public boolean checkCUSimfromphoneId(int slotId) {
        boolean result = false;
        final String MccMnc = mTelManager.getSimOperatorNumericForPhone(slotId);
        Log.d(TAG_SELFREGISTER_SVC, "MNCMCC from slotId ("+slotId+" [" + MccMnc +"]");

        if (MccMnc.equals("46001") || MccMnc.equals("46004") ||  MccMnc.equals("46006") ||MccMnc.equals("45407")) {
            result = true;
        }
        return result;
    }
    public boolean checkCtcSimfromphoneId(int slotId) {
        boolean result = false;
        final String MccMnc = mTelManager.getSimOperatorNumericForPhone(slotId);
        Log.d(TAG_SELFREGISTER_SVC, "MNCMCC from slotId ("+slotId+" [" + MccMnc +"]");

        if (MccMnc.equals("46003") || MccMnc.equals("46005") ||  MccMnc.equals("46011") ||MccMnc.equals("45403") ||
            MccMnc.equals("45502") || MccMnc.equals("45507")  ||  MccMnc.equals("20404") ) {
            result = true;
        }
        return result;
    }

    public boolean isCtcSim(int SubId) {
        boolean result = false;
        final String MccMnc = mTelManager.getSimOperatorNumeric(SubId);
        Log.d(TAG_SELFREGISTER_SVC, "MNCMCC from SubID ("+SubId+" [" + MccMnc +"]");

        if (MccMnc.equals("46003") || MccMnc.equals("46005")||MccMnc.equals("46011") ||MccMnc.equals("45403") ||
            MccMnc.equals("45502") || MccMnc.equals("45507") ||MccMnc.equals("20404") ) {
            Log.d(TAG_SELFREGISTER_SVC, "China Telecom SIM card");
            result = true;
        }
        return result;
    }


    // check Dual-SIM status and MainSimStatus
    public boolean isBothCtcSim(){
       final String slot1_MccMnc = mTelManager.getSimOperatorNumericForPhone(0);
       final String slot2_MccMnc = mTelManager.getSimOperatorNumericForPhone(1);
       boolean result1=false;
       boolean result2=false;
       Log.d(TAG_SELFREGISTER_SVC, "slot1_MccMnc" +slot1_MccMnc +" slot2_MccMnc: "+slot2_MccMnc);

    if (slot1_MccMnc.equals("46003") || slot1_MccMnc.equals("46005") ||slot1_MccMnc.equals("20404") ||slot1_MccMnc.equals("45403") ||
         slot1_MccMnc.equals("46011") || slot1_MccMnc.equals("45502") || slot1_MccMnc.equals("45507")) {
          Log.d(TAG_SELFREGISTER_SVC, "slot1_MccMnc China Telecom SIM card");
          result1 = true;
      }
      if (slot2_MccMnc.equals("46003") || slot2_MccMnc.equals("46005") ||slot2_MccMnc.equals("20404") ||slot2_MccMnc.equals("45403") ||
          slot2_MccMnc.equals("46011") || slot2_MccMnc.equals("45502") || slot2_MccMnc.equals("45507")) {
          Log.d(TAG_SELFREGISTER_SVC, "slot2_MccMnc China Telecom SIM card");
          result2 = true;
      }
      if(result1==true && result2 ==true) {
        Log.d(TAG_SELFREGISTER_SVC, "BothSim are CTC Sims");
        return true;
     }
     else{
        Log.d(TAG_SELFREGISTER_SVC, "BothSim aren't CTC Sims");
       return false;
       }
    }

    public int getMainCardSubId() {
        int subId = mSubManager.getDefaultDataSubscriptionId();
        Log.d(TAG_SELFREGISTER_SVC, "[getMainSimSubId] subId= " + subId);

        return subId;
    }

    public int getMainCardSlotId() {

        int slotId = 0;
        int subId = getMainCardSubId();

        Log.d(TAG_SELFREGISTER_SVC, "[getMainCardSlotId] Main card sub ID = " + subId);
        if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            slotId = SubscriptionManager.getSlotIndex(subId);
            Log.d(TAG_SELFREGISTER_SVC, "[getMainCardSlotId] Main card Slot ID = " + slotId );
            if (slotId < 0) {
                Log.e(TAG_SELFREGISTER_SVC, "[getMainCardSlotId] Slot IDX is invalid (-1)");
            }
            return slotId;
        }

        Log.e(TAG_SELFREGISTER_SVC, "[getMainCardSlotId] Main card sub ID is Invalid, return 0 for main slot ID");
        // No main card selected. return 0 for default value
        return 0;
    }

    public List<SubscriptionInfo> getSubInfoList() {
        return mSubList;
    }

    private void setSubInfoList() {
        try {
            mSubList = mSubManager.getActiveSubscriptionInfoList();
        } catch (Exception e) {
            Log.e(TAG_SELFREGISTER_SVC, "Exception at setSubInfList()");
            e.printStackTrace();
        }
    }

    public int getSimCount() {

       int simCount;
       try {
           simCount = mSubList.size();
       } catch (Exception e) {
           Log.e(TAG_SELFREGISTER_SVC, "Exception at getSimCount\n");
            e.printStackTrace();
           simCount = 0;
       }
       return simCount;
    }

    public void storeMacAddress(String mac) {
        Log.d(TAG_SELFREGISTER_SVC, "Got MAC ADDR =" + mac);
        if ( mac.indexOf("02:00:00") == -1 ){
            Log.d(TAG_SELFREGISTER_SVC, "Succeed normal MAC addr, store it to preference");
            if (mPref.storeMac(mac) == false) {
                Log.e(TAG_SELFREGISTER_SVC, "Failed to store MAC address");
            }
        }
    }

    public String readMacAddress() {
        String mac;
        mac = mPref.readMac();

        Log.d(TAG_SELFREGISTER_SVC, "Got MAC ADDR from Pref. =" + mac);
        return mac;
    }


    Runnable mSendTask = new Runnable() {
        @Override
        public void run() {
            // Self registration is required. Making self-registration data
            SelfRegisterData data = new SelfRegisterData((SelfRegisterService)mContext);
            String registerData = data.getSelfRegisterData();

            if (registerData == null) {
                Log.e(TAG_SELFREGISTER_SVC, "Self register data is not generated, setting retry alarm");
                setRetryAlarm("Data not generated");
                return;
            }
            saveSwVer(data.getSwVer());
            JSONObject resp = HttpUtils.httpSend(registerData);
            Message msg = mHandler.obtainMessage(MSG_SELF_REG_RESULT, resp);
            mHandler.sendMessage(msg);
        }
    };
}

