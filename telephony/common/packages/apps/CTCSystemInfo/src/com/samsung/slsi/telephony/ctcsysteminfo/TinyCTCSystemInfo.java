/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.ctcsysteminfo;

import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import android.app.ActivityManager;
import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.StatFs;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.telephony.CellLocation;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class TinyCTCSystemInfo extends PreferenceActivity {

    Preference mPrefModelNum, mPrefSwVer, mPrefHwVer, mPrefPrlVer, mPrefUimid,
            mPrefEsnMeid, mPrefSid, mPrefNid;
    Button mBtnOk;
    private static final String TAG = "TinyCTCSystemInfo";
    private static final String REG_VER = "7.0";
    private static final String HW_VER = "V1.0";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        addPreferencesFromResource(R.xml.tiny_preference);

        mBtnOk = (Button) findViewById(R.id.btn_ok);
        mBtnOk.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                finish();
            }
        });

        fillList();
    }

    private void fillList() {
        Log.d(TAG, "fillList()");

        PreferenceScreen root = (PreferenceScreen)findPreference("key_tiny_screen");

        if (root != null) {
            root.removeAll();

            // category
            PreferenceCategory category = new PreferenceCategory(getApplication());
            category.setTitle(R.string.str_title);
            root.addPreference(category);

            // terminal type
            Preference prefModelName = new Preference(getApplicationContext());
            prefModelName.setTitle("Type");
            prefModelName.setSummary(Build.MODEL);
            root.addPreference(prefModelName);

            // hw ver
            Preference prefHwVer = new Preference(getApplicationContext());
            prefHwVer.setTitle("HW ver");
            prefHwVer.setSummary(HW_VER);
            root.addPreference(prefHwVer);

            // sw ver
            Preference prefSwVer = new Preference(getApplicationContext());
            prefSwVer.setTitle("SW version");
            prefSwVer.setSummary(Build.DISPLAY);
            root.addPreference(prefSwVer);

            Phone[] phone = PhoneFactory.getPhones();
            // meid
            Preference prefMeid = new Preference(getApplicationContext());
            prefMeid.setTitle("MEID");
            prefMeid.setSummary(phone[0].getMeid());
            root.addPreference(prefMeid);

            // imei1
            Preference prefImei1 = new Preference(getApplicationContext());
            prefImei1.setTitle("IMEI1");
            prefImei1.setSummary(phone[0].getImei());
            root.addPreference(prefImei1);

            // imei2
            if (phone.length == 2) {
                Preference prefImei2 = new Preference(getApplicationContext());
                prefImei2.setTitle("IMEI2");
                prefImei2.setSummary(phone[1].getImei());
                root.addPreference(prefImei2);
            }

            // mac id
            Preference prefMacId = new Preference(getApplicationContext());
            prefMacId.setTitle("MAC ID");
            prefMacId.setSummary(getMacAddr());
            root.addPreference(prefMacId);

            // os ver
            Preference prefOsVer = new Preference(getApplicationContext());
            prefOsVer.setTitle("OS ver");
            prefOsVer.setSummary(Build.VERSION.RELEASE);
            root.addPreference(prefOsVer);

            // rom
            Preference prefRom = new Preference(getApplicationContext());
            prefRom.setTitle("ROM");
            prefRom.setSummary(getRom());
            root.addPreference(prefRom);

            // ram
            Preference prefRam = new Preference(getApplicationContext());
            prefRam.setTitle("RAM");
            prefRam.setSummary(getRam());
            root.addPreference(prefRam);

            // reg ver
            Preference prefRegVer = new Preference(getApplicationContext());
            prefRegVer.setTitle("REG ver");
            prefRegVer.setSummary(REG_VER);
            root.addPreference(prefRegVer);

            // sim1 cell id
            Preference prefCid1 = new Preference(getApplicationContext());
            prefCid1.setTitle("SIM1 Cell Id");
            prefCid1.setSummary(getCellId((GsmCdmaPhone) phone[0]));
            root.addPreference(prefCid1);

            // sim2 cell id
            if (phone.length == 2) {
                Preference prefCid2 = new Preference(getApplicationContext());
                prefCid2.setTitle("SIM2 Cell Id");
                prefCid2.setSummary(getCellId((GsmCdmaPhone) phone[1]));
                root.addPreference(prefCid2);
            }
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    private String getMacAddr() {
        try {
            boolean bWifiForcedOn = false;
            String mac = "00:00:00";
            WifiManager wifiManager = (WifiManager) getBaseContext().getSystemService(getBaseContext().WIFI_SERVICE);
            if (wifiManager.isWifiEnabled() == false) {
                wifiManager.setWifiEnabled(true);
                bWifiForcedOn = true;
            }

            // wait until the WIFI enabled at maximum 5s
            int waitCnt;
            WifiInfo info;
            for (waitCnt = 25; waitCnt > 0 ; waitCnt--) {
                if (wifiManager.isWifiEnabled() == true) {
                    info = wifiManager.getConnectionInfo();
                    mac = info.getMacAddress();
                    if (mac.indexOf("00:00:00") == -1) {
                        Log.d(TAG, "Read MAC address in device = , stop waiting" + mac);
                        break;
                    }
                }
                android.os.SystemClock.sleep(200);
            }
            if (waitCnt == 0) Log.e(TAG, "Enabling WIFI failed - timeout 5s");

            Log.d(TAG, "WIFI MAC ADDR = " + mac);
            if (bWifiForcedOn) wifiManager.setWifiEnabled(false);
            return mac;
        } catch (Exception e) {
            e.printStackTrace();
            return "";
        }
    }

    private String getRam() {
        ActivityManager.MemoryInfo memInfo = new ActivityManager.MemoryInfo();
        ((ActivityManager) this.getSystemService(Context.ACTIVITY_SERVICE)).getMemoryInfo(memInfo);
        double total = 0;
        String unit = "B";
        if (memInfo.totalMem > 1024) {
            total = (double) memInfo.totalMem / 1024; // KB
            unit = "KB";
            if (total > 1024) {
                total = total / 1024; // MB
                unit = "MB";
                if (total > 1024) {
                    total = total / 1024; //GB
                    unit = "GB";
                }
            }
        }
        total = Math.floor(total*10)/10.0;
        return (String.valueOf(total)+unit);
    }

    private String getRom() {
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

        return result;
   }

    private String getCellId(GsmCdmaPhone phone) {
        String cid = "0";
        CellLocation cellLocation = phone.getCellLocation();
        Log.d(TAG, "CellLocation: " + cellLocation.toString());
        if (phone.isPhoneTypeGsm()) {
            cid = String.valueOf(((GsmCellLocation) cellLocation).getCid());
        } else {
            cid = String.valueOf(((CdmaCellLocation) cellLocation).getBaseStationId());
        }
        return cid;
    }
}
