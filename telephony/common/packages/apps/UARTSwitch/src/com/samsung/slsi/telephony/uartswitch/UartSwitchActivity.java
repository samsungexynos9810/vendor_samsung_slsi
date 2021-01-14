package com.samsung.slsi.telephony.uartswitch;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.os.Bundle;

public class UartSwitchActivity extends Activity {
    @Override
    protected void onCreate(Bundle saveInstance) {
        super.onCreate(saveInstance);

        if (saveInstance  == null) {
            FragmentTransaction ft = getFragmentManager().beginTransaction();
            Fragment newFrag = UartSwitchPreferenceFragment.newInstance();
            ft.add(android.R.id.content,  newFrag);
            ft.commit();
        }
    }
}
