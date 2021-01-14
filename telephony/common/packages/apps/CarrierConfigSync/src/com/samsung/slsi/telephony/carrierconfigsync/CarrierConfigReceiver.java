package com.samsung.slsi.telephony.carrierconfigsync;

import com.android.internal.telephony.PhoneConstants;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.CarrierConfigManager;
import android.util.Log;

public class CarrierConfigReceiver extends BroadcastReceiver {

    private static final String TAG = "CarrierConfigReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent.getAction().equals(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
            Log.d(TAG, "Action: " + intent.getAction());
            int subid = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, 0);
            int phoneid = intent.getIntExtra(PhoneConstants.PHONE_KEY, 0);

            Intent intentService = new Intent(context,
                    CarrierConfigService.class);
            intentService.putExtra("subid", subid);
            intentService.putExtra("phoneid", phoneid);
            context.startService(intentService);

        }
    }
}
