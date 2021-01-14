/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.dataretrymode;

import com.android.internal.R;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.util.Log;
import android.os.Bundle;

public class  DataRetryModeActivity extends Activity {
	 private static final String TAG = "DataRetryModeActivity";

    @Override
    protected void onCreate(Bundle saveInstance) {
        super.onCreate(saveInstance);
        if (saveInstance  == null) {
            FragmentTransaction ft = getFragmentManager().beginTransaction();
            Fragment newFrag = SelectSimPreferenceFragment.newInstance();
            ft.add(android.R.id.content,  newFrag);
            ft.commit();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    public void onNextFragment(String plmn, int subId) {
        getFragmentManager().beginTransaction().replace(R.id.content, DataRetryModePreferenceFragment.newInstance(plmn, subId)).commit();
    }
}
