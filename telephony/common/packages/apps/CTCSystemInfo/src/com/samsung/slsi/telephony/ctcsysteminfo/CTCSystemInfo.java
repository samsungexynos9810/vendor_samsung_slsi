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

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.RuimRecords;

import android.os.Build;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;

public class CTCSystemInfo extends PreferenceActivity {

    Preference mPrefModelNum, mPrefSwVer, mPrefHwVer, mPrefPrlVer, mPrefUimid,
            mPrefEsnMeid, mPrefSid, mPrefNid;
    Button mBtnOk;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        addPreferencesFromResource(R.xml.preference);

        mBtnOk = (Button) findViewById(R.id.btn_ok);
        mBtnOk.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                finish();
            }
        });

        mPrefModelNum = findPreference(CTCSystemInfoConstants.KEY_MODEL_NUM);
        mPrefSwVer = findPreference(CTCSystemInfoConstants.KEY_SW_VER);
        mPrefHwVer = findPreference(CTCSystemInfoConstants.KEY_HW_VER);
        mPrefPrlVer = findPreference(CTCSystemInfoConstants.KEY_PRL_VER);
        mPrefUimid = findPreference(CTCSystemInfoConstants.KEY_UIMID);
        mPrefEsnMeid = findPreference(CTCSystemInfoConstants.KEY_ESN_MEID);
        mPrefSid = findPreference(CTCSystemInfoConstants.KEY_SID);
        mPrefNid = findPreference(CTCSystemInfoConstants.KEY_NID);

        // phone
        Phone phone = PhoneFactory.getDefaultPhone();

        // setting summary
        // terminal model number
        mPrefModelNum.setSummary(Build.MODEL);

        // hardware version
        mPrefHwVer.setSummary(Build.BOARD);
        // software version
        StringBuilder sb = new StringBuilder();
        sb.append("Android version: ").append(Build.VERSION.RELEASE).append("\n");
        sb.append("Baseband version: ").append(getProperty("gsm.version.baseband")).append("\n");
        sb.append("Build number: ").append(Build.DISPLAY);
        mPrefSwVer.setSummary(sb.toString());

        // currently used PRL file version
        mPrefPrlVer.setSummary(phone.getCdmaPrlVersion());

        // currently used UIMID
        mPrefUimid.setSummary(phone.getEsn());

        //currently used terminal ESN or MEID
        //mPrefEsnMeid.setSummary(phone.getEsn());
        mPrefEsnMeid.setSummary(phone.getEsn());

        // currently used terminal access network SID and NID
        IccRecords curIccRecords = phone.getIccRecords();
        if ((curIccRecords != null) && (curIccRecords instanceof RuimRecords)){
            RuimRecords csim = (RuimRecords) curIccRecords;
            if (csim.getSid().length() > 0) {
                String[] sid = csim.getSid().split("\\,");
                mPrefSid.setSummary(sid[0]);
            }
            if (csim.getNid().length() > 0) {
                String[] nid = csim.getNid().split("\\,");
                mPrefNid.setSummary(nid[0]);
            }
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    private String getProperty(String property) {
        return SystemProperties.get(property, "Unknown");
    }
}