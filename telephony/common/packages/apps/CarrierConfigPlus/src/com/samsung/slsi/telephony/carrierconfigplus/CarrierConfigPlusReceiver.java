/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.carrierconfigplus;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;


public class CarrierConfigPlusReceiver extends BroadcastReceiver {

    private static final String TAG = "CarrierConfigPlus";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        int phoneId = CarrierConfigPlusConstants.INVALID_PHONE_INDEX;
        int subId = CarrierConfigPlusConstants.INVALID_SUBSCRIPTION_ID;
        switch (action) {
        case TelephonyIntents.ACTION_SERVICE_STATE_CHANGED:
        case CarrierConfigPlusConstants.ACTION_RELOAD_CARRIER_CONFIG:
            Log.d(TAG, "action=" + action);
            phoneId = intent.getIntExtra(PhoneConstants.SLOT_KEY, CarrierConfigPlusConstants.INVALID_PHONE_INDEX);
            subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, CarrierConfigPlusConstants.INVALID_SUBSCRIPTION_ID);

            if (!SubscriptionManager.isValidPhoneId(phoneId)) {
                Log.w(TAG, "invalid phoneId=" + phoneId);
                return ;
            }

            if (!SubscriptionManager.isValidSubscriptionId(subId)) {
                int[] subIds = SubscriptionManager.getSubId(phoneId);
                if (subIds != null && subIds.length >= 1) {
                    subId = subIds[0];
                }

                if (!SubscriptionManager.isValidSubscriptionId(subId)) {
                    Log.w(TAG, "invalid subId for phoneId " + phoneId);
                    // SIM is not LOADED or ABSENT
                    // reset operator-numeric
                    CarrierConfigPlusProperties.setOperatorNumeric(context, "", phoneId);
                    return ;
                }
            }

            String simOperator = TelephonyManager.from(context).getSimOperatorNumericForPhone(phoneId);
            // A valid simOperator should be 5 or 6 digits, depending on the length of the MNC.
            if (simOperator == null || simOperator.length() < 5) {
                // IMSI may be loaded yet
                Log.w(TAG, "invalid SIM operator numeric for phoneId  " + phoneId);
                return ;
            }
            Log.d(TAG, "phoneId " + phoneId + " SIM operator numeric " + simOperator);

            String operatorNumeric = null;
            Bundle bundle = intent.getExtras();
            if (bundle != null) {
                operatorNumeric = bundle.getString("operator-numeric");
            }
            else {
                // for CarrierConfigConstants.ACTION_RELOAD_CARRIER_CONFIG (test only)
                operatorNumeric = intent.getStringExtra("operator-numeric");
            }

            String preOperatorNumeric = CarrierConfigPlusProperties.getOperatorNumeric(context, phoneId);
            Log.d(TAG, "phoneId=" + phoneId + " subId=" + subId +
                    " preOperatorNumeric=" + preOperatorNumeric + " operatorNumeric=" + operatorNumeric);

            if (TextUtils.isEmpty(preOperatorNumeric) && TextUtils.isEmpty(operatorNumeric)) {
                break;
            }

            if (!TextUtils.equals(preOperatorNumeric, operatorNumeric)) {
                Log.d(TAG, "set operatorNumeric : "+operatorNumeric+"for phoneId=" + phoneId);
                CarrierConfigPlusProperties.setOperatorNumeric(context, operatorNumeric, phoneId);
                CarrierConfigManager configLoader = (CarrierConfigManager)context.getSystemService(Context.CARRIER_CONFIG_SERVICE);
                if (configLoader != null) {
                    Log.d(TAG, "notifyConfigChangedForSubId(" + subId + ")");
                    configLoader.notifyConfigChangedForSubId(subId);
                }
            }

            break;
        }
    }
}
