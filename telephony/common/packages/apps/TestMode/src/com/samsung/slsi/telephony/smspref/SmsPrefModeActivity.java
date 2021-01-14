/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.smspref;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.samsung.slsi.telephony.testmode.R;

import android.os.SystemProperties;
public class SmsPrefModeActivity extends Activity {

    private static final String TAG = "SmsPrefModeActivity";
    private static final String PROPERTY_SMS_DOMAIN = "vendor.radio.smsdomain";
    private static final String PROPERTY_CS_PREF = "0";
    private static final String PROPERTY_PS_PREF = "1";
    private static final String PROPERTY_CS_ONLY = "2";
    private static final String PROPERTY_PS_ONLY = "3";
    private ArrayList<RadioButton> mRadioButtons;
    private RadioGroup.OnCheckedChangeListener mCheckedChangedListener = new RadioGroup.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(RadioGroup group, int checkedId) {
            String setProp = PROPERTY_CS_PREF;
            switch (checkedId) {
                case R.id.rb_cs_pref:
                    setProp = PROPERTY_CS_PREF;
                    break;
                case R.id.rb_ps_pref:
                    setProp = PROPERTY_PS_PREF;
                    break;
                case R.id.rb_cs_only:
                    setProp = PROPERTY_CS_ONLY;
                    break;
                case R.id.rb_ps_only:
                    setProp = PROPERTY_PS_ONLY;
                    break;
            }
            SystemProperties.set(PROPERTY_SMS_DOMAIN, setProp);
            Toast.makeText(getApplicationContext(), "SMS Domain Property : " + setProp, Toast.LENGTH_SHORT).show();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sms_mode_activity);

        mRadioButtons = new ArrayList<RadioButton>();
        registerRadioButton(R.id.rb_cs_pref);
        registerRadioButton(R.id.rb_ps_pref);
        registerRadioButton(R.id.rb_cs_only);
        registerRadioButton(R.id.rb_ps_only);

        RadioGroup radioGroup = (RadioGroup) findViewById(R.id.rg_sms_mode);
        if (radioGroup != null) {
            radioGroup.setOnCheckedChangeListener(mCheckedChangedListener);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateCurrentMode();
    }

    private void registerRadioButton(int id) {
        RadioButton button = (RadioButton)findViewById(id);
        if (button != null) {
            mRadioButtons.add(button);
        }
    }

    private void updateCurrentMode() {
        String currentMode = String.valueOf(SystemProperties.get(PROPERTY_SMS_DOMAIN, "0"));
        int position = Integer.parseInt(currentMode);
        mRadioButtons.get(position).setChecked(true);
    }

}
