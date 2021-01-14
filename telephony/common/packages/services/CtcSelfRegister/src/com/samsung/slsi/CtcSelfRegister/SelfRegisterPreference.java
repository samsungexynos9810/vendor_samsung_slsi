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
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;
import java.text.SimpleDateFormat;


/**
 * Created by jongmin.son on 2017-01-18.
 */

public class SelfRegisterPreference {

    private final String TAG_SELFREGISTER_PREF = "SELF_REG_PREF";
    public final int MAX_RETRY_COUNT = 10;
    public final int RETRY_TIME = 60 * 60 * 1000; // One hour in Millisecond
    public final int CONNECTION_RETRY_TIME = 60 * 1000; // One minute
    public final int RETRY_COUNTER_FAIL = 1;
    public final int RETRY_COUNTER_CONN = 2;

    public final int FirstRegiAlarmDone =  7;

    private final String KEY_ICCID1 = "ICCID1";
    private final String KEY_ICCID2 = "ICCID2";
    private final String KEY_DATASIM = "DATASIM";
    private final String KEY_RETRY_COUNT = "RETRY_COUNT";
    private final String KEY_CONN_RETRY_COUNT = "CONN_RETRY_COUNT";
    private final String KEY_MAC = "MACADDR";
    private final String KEY_SWVERSION = "SW_VERSION";
    private final String KEY_SYSREGITIME = "SYSREGITIME";
    //added new preference
    private final String KEY_REGITIME = "REGITIME";
    private final String KEY_REGICNT = "REGICNT";
    private final String KEY_REGISTATUS = "REGISTATUS";
    //private final String KEY_RATINFO0 = "RATINFO0";
    //private final String KEY_RATINFO1 = "RATINFO1";

    private SharedPreferences mSharedPref = null;
    private Context mContext = null;
    private SharedPreferences.Editor mEditor = null;

    public void init(Context context) {
        mContext = context;
        if (mContext == null) {
            Log.e(TAG_SELFREGISTER_PREF, "Invalid context value");
            return;
        }

        mSharedPref = PreferenceManager.getDefaultSharedPreferences(context);
        if (mSharedPref == null) {
            Log.e(TAG_SELFREGISTER_PREF, "Failed on get the default shared preferences");
            return;
        }

        mEditor = mSharedPref.edit();
        if (mSharedPref == null) {
            Log.e(TAG_SELFREGISTER_PREF, "Failed on get the editor interface of the shared preferences");
            return;
        }
    }

    public boolean storeIccIds(String[] IccIds) {
        if (mSharedPref == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[storeIccIds] Shared preferences is not initialized yet");
            return false;
        }

       mEditor.putString(KEY_ICCID1, IccIds[0]);
       mEditor.putString(KEY_ICCID2, IccIds[1]);
       Log.d(TAG_SELFREGISTER_PREF, "[storeIccIds] 1."+IccIds[0]+"  2. "+IccIds[1]);
       mEditor.commit();

       return true;
    }


    public String[] readIccIds() {
        if (mSharedPref == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[readIccIds] Shared preferences is not initialized yet");
            return null;
        }

        String[] IccIds = new String[SelfRegisterService.MAX_SIM_COUNT];

        IccIds[0] = mSharedPref.getString(KEY_ICCID1, null);
        IccIds[1] = mSharedPref.getString(KEY_ICCID2, null);

        Log.d(TAG_SELFREGISTER_PREF, "Got stored ICCIDs [" + (IccIds[0]==null ? "null":IccIds[0]) + "], [" + (IccIds[1]==null ? "null":IccIds[1]) + "]");

        return IccIds;
    }

    public boolean storeDataSim(int dataSim) {
        if (mSharedPref == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[storeDataSim] Shared preferences is not initialized yet");
            return false;
        }

        Log.d(TAG_SELFREGISTER_PREF, "[storeDataSim] Store Data SIM ["+dataSim+"]");
        mEditor.putInt(KEY_DATASIM, dataSim);
        mEditor.commit();
        return true;
    }

    public int readDataSim() {
        if (mSharedPref == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[readDataSim] Shared preferences is not initialized yet");
            return 0;
        }
        int dataSim = mSharedPref.getInt(KEY_DATASIM, 0);
        Log.d(TAG_SELFREGISTER_PREF, "Stored Data SIM [" + dataSim + "]");

        return dataSim;
    }

    public boolean storeMac(String mac) {
        if (mSharedPref == null || mac == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[storeMac] Shared preferences is not initialized yet");
            return false;
        }

        mEditor.putString(KEY_MAC, mac);
        mEditor.commit();
        return true;
    }

    public String readMac() {
        if (mSharedPref == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[readMac] Shared preferences is not initialized yet");
            return "02:00:00";
        }
        String mac = mSharedPref.getString(KEY_MAC, "02:00:00:00:00:00");
        Log.d(TAG_SELFREGISTER_PREF, "Got MAC [" + mac + "]");

        return mac;
    }

    public void resetRetryCnt() {
        if (mSharedPref == null || mEditor == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[resetRetryCnt] Shared preferences is not initialized yet");
            return;
        }

        // Don't remove ICCIDs stored, only retry count should be reset to 0
        mEditor.putInt(KEY_RETRY_COUNT, 0);
        mEditor.putInt(KEY_CONN_RETRY_COUNT, 0);
        mEditor.commit();

    }

    public void increaseRetryCount(int CounterType) {
        if (mSharedPref == null || mEditor == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[reset] Shared preferences is not initialized yet");
            return;
        }
        String counterKey;
        if (CounterType == RETRY_COUNTER_FAIL) counterKey = KEY_RETRY_COUNT;
        else counterKey = KEY_CONN_RETRY_COUNT;

        int retryCount = readRetryCount(CounterType);
        mEditor.putInt(counterKey, ++retryCount);
        mEditor.commit();
    }
    public int readRetryCount(int CounterType) {
        if (mSharedPref == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[readRetryCount] Shared preferences is not initialized yet");
            return 0;
        }

        String counterKey;
        if (CounterType == RETRY_COUNTER_FAIL) counterKey = KEY_RETRY_COUNT;
        else counterKey = KEY_CONN_RETRY_COUNT;

        return mSharedPref.getInt(counterKey, 0);
    }

    public void resetRetryCount(int CounterType) {
        if (mSharedPref == null || mEditor == null) {
            Log.e(TAG_SELFREGISTER_PREF, "[reset] Shared preferences is not initialized yet");
            return;
        }
        String counterKey;
        if (CounterType == RETRY_COUNTER_FAIL) counterKey = KEY_RETRY_COUNT;
        else counterKey = KEY_CONN_RETRY_COUNT;

        mEditor.putInt(counterKey, 0);
        mEditor.commit();
    }

    public String readLastSwVer() {
       if (mSharedPref == null) {
           Log.e(TAG_SELFREGISTER_PREF, "[readLastSwVer] Shared preferences is not initialized yet");
           return null;
       }

       String LastVersion = null;
       LastVersion= mSharedPref.getString(KEY_SWVERSION, null);
       return LastVersion;
   }

    public String saveLastSwVer(String version) {
    if (mSharedPref == null) {
        Log.e(TAG_SELFREGISTER_PREF, "[saveLastSwVer] Shared preferences is not initialized yet");
        return null;
    }
    Log.d(TAG_SELFREGISTER_PREF, "[saveLastSwVern] Store SW version ["+version+"]");
    mEditor.putString(KEY_SWVERSION, version);
    mEditor.commit();
    return version;
 }

    public long readSystemTime(){
      if (mSharedPref == null) {
       Log.e(TAG_SELFREGISTER_PREF, "[readSystemTime] Shared preferences is not initialized yet");
       return 0;
      }
     long cregTime= mSharedPref.getLong(KEY_SYSREGITIME, 0);
     Log.d(TAG_SELFREGISTER_PREF, "[readSystemTime] readTime : " +cregTime );
     return cregTime;
    }
    public void saveSystemTime() {
        if (mSharedPref == null) {
         Log.e(TAG_SELFREGISTER_PREF, "[saveSystemTime] Shared preferences is not initialized yet");
         return;
       }
       long cregTime = System.currentTimeMillis();
       mEditor.putLong(KEY_SYSREGITIME, cregTime);
       mEditor.commit();
       Log.d(TAG_SELFREGISTER_PREF, "[saveSystemTime] saveTime ["+cregTime+"]");
      }


// Reading Registration time & Saving Registration time

    private String convertTimetoDate(long systime){
         long currentTime =System.currentTimeMillis();
         SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
         String convertedsysT = format.format(systime);
         //Log.d (TAG_SELFREGISTER_SVC, "convertedTime: "+convertedsysT);
         return convertedsysT;
    }

    public long readRegiTime() {
       if (mSharedPref == null) {
       Log.e(TAG_SELFREGISTER_PREF, "[readRegiTime] Shared preferences is not initialized yet");
       return 0;
       }
       long Time = mSharedPref.getLong(KEY_REGITIME, 0);
       Log.d(TAG_SELFREGISTER_PREF, "[readRegiTime]:" + convertTimetoDate(Time));
       return Time;
    }
    public long saveRegiTime() {
       if (mSharedPref == null) {
       Log.e(TAG_SELFREGISTER_PREF, "[saveRegiTime] Shared preferences is not initialized yet");
       return 0;
       }
       long Time = System.currentTimeMillis();
       Log.d(TAG_SELFREGISTER_PREF, "[saveRegiTime] Store RegiTime:" + convertTimetoDate(Time));
       mEditor.putLong(KEY_REGITIME, Time);
       mEditor.commit();
       return Time;
    }
    public int readRegCnt() {
       if (mSharedPref == null) {
           Log.e(TAG_SELFREGISTER_PREF, "[readRegiCnt] Shared preferences is not initialized yet");
           return 0;
       }
       int cnt = mSharedPref.getInt(KEY_REGICNT, 0);
       Log.d(TAG_SELFREGISTER_PREF, "[readRegCnt]:" + cnt);
       return cnt;
     }

     public void saveRegCnt(int cnt) {
       if (mSharedPref == null) {
           Log.e(TAG_SELFREGISTER_PREF, "[saveRegCnt] Shared preferences is not initialized yet");
           return;
       }
       Log.d(TAG_SELFREGISTER_PREF, "[saveRegCnt] Store RegCnt ["+cnt+"]");
       mEditor.putInt(KEY_REGICNT, cnt);
       mEditor.commit();
       return;
     }
     public int readRegStatus() {
      if (mSharedPref == null) {
          Log.e(TAG_SELFREGISTER_PREF, "[readRegStatus] Shared preferences is not initialized yet");
          return 0;
      }
      int status = mSharedPref.getInt(KEY_REGISTATUS, 0);
      Log.d(TAG_SELFREGISTER_PREF, "[readRegStatus]:" + status);
      return status;
    }

    public void saveRegStatus(int status) {
      if (mSharedPref == null) {
          Log.e(TAG_SELFREGISTER_PREF, "[saveRegStatus] Shared preferences is not initialized yet");
          return;
      }
      Log.d(TAG_SELFREGISTER_PREF, "[saveRegStatus] Store status ["+status+"]");
      mEditor.putInt(KEY_REGISTATUS, status);
      mEditor.commit();
      return;
    }

/*
 public int readRatInfo(int slot) {
  if (mSharedPref == null) {
      Log.e(TAG_SELFREGISTER_PREF, "[readRatInfo] Shared preferences is not initialized yet");
      return 0;
  }

  String Key = "KEY_RATINFO"+slot;
  int status = mSharedPref.getInt(Key, 0);
  Log.d(TAG_SELFREGISTER_PREF, "[readRatInfo]:" + status);
  return status;
}

public void saveRatInfo(int slot, int status) {
  if (mSharedPref == null) {
      Log.e(TAG_SELFREGISTER_PREF, "[saveRatInfo] Shared preferences is not initialized yet");
      return;
  }

  String Key = "KEY_RATINFO"+slot;
  Log.d(TAG_SELFREGISTER_PREF, "[saveRatInfo] Store status ["+status+"] :" +slot);
  mEditor.putInt(Key, status);
  mEditor.commit();
  return;
}
*/

}

