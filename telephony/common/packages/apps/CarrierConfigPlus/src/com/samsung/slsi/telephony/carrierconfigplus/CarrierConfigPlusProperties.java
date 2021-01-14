package com.samsung.slsi.telephony.carrierconfigplus;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

public class CarrierConfigPlusProperties {

    public static String getOperatorNumeric(Context context, int phoneId) {
        String operatorNumeric = null;
        Context directBootContext = context.createDeviceProtectedStorageContext();
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(directBootContext);
        if (sp != null) {
            String key = CarrierConfigPlusConstants.KEY_OPERATOR_NUMERIC + phoneId;
            operatorNumeric = sp.getString(key, null);
        }

        return operatorNumeric;
    }

    public static boolean setOperatorNumeric(Context context, String operatorNumeric, int phoneId) {
        Context directBootContext = context.createDeviceProtectedStorageContext();
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(directBootContext);
        if (sp != null) {
            SharedPreferences.Editor editor = sp.edit();
            String key = CarrierConfigPlusConstants.KEY_OPERATOR_NUMERIC + phoneId;
            editor.putString(key, operatorNumeric);
            editor.commit();
            return true;
        }
        return false;
    }
}
