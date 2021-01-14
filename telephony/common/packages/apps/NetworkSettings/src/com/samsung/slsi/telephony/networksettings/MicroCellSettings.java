/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.samsung.slsi.telephony.networksettings;

import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.telephony.TelephonyManager;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.dataconnection.DcTracker;

public class MicroCellSettings extends PreferenceActivity {

    private static final boolean DBG = true;
    private static final int EVENT_SET_FEMTO_CELL_SEARCH = 100;
    private static final int DIALOG_NETWORK_SEARCH_FEMTO_CELL = 100;
    private static final int FEMTO_MANUAL_MODE = 0;
    private static final int EVENT_ALL_DATA_DISCONNECTED_FOR_FEMTO = 1;
    protected boolean mIsForeground = false;

    private DcTracker mDcTracker;
    private Phone mPhone;
    private boolean dataEnable = false;
    private boolean isDataForFemto = false;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_ALL_DATA_DISCONNECTED_FOR_FEMTO:
                    if (isDataForFemto) {
                        isDataForFemto = false;
                        log("EVENT_ALL_DATA_DISCONNECTED_FOR_FEMTO");
                        selectFemtoCell();
                    }
                    break;

                case EVENT_SET_FEMTO_CELL_SEARCH:
                    log("remove femto cell search dialog");
                    removeDialog(DIALOG_NETWORK_SEARCH_FEMTO_CELL);
                    getPreferenceScreen().setEnabled(true);

                    ar = (AsyncResult) msg.obj;
                    if(ar.exception != null){ //connect fail
                        log("femto cell connect fail");
                        Toast.makeText(getApplicationContext(), R.string.str_connect_error, Toast.LENGTH_SHORT).show();
                    } else { // connect success
                        log("femto cell connect success");
                        Toast.makeText(getApplicationContext(), R.string.str_conncet_complete, Toast.LENGTH_SHORT).show();
                    }

                    if(dataEnable) {
                        log("femto search done, enable data");
                        mPhone.setInternalDataEnabled(true, null);
                        dataEnable = false;
                    }

                    break;
            }
            return;
        }
    };

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference.getKey().equals("key_femto_cell_search")) {
            if (isFemtoCellSearchEnabled()) {
                if (!mDcTracker.isDisconnected()) {
                    //create the dialog to remind the user de-activate the data
                    AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle(R.string.str_setup_title);
                    builder.setMessage(R.string.str_setup_dialog);
                    builder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int whichButton) {
                            if (whichButton == DialogInterface.BUTTON_POSITIVE) {
                                mPhone.registerForAllDataDisconnected(mHandler, EVENT_ALL_DATA_DISCONNECTED_FOR_FEMTO, null);
                                if (DBG) log("disable data for femto search");
                                mPhone.setInternalDataEnabled(false, null);
                                dataEnable = true;
                                isDataForFemto = true;
                            }
                        }
                    });
                    builder.setNegativeButton(R.string.cancel, null);
                    builder.create().show();
                } else{
                    //if data is not on the current sim or data is disable or the connect type is wifi/bluetooth/wimax
                    selectFemtoCell();
                }
            } else {
                if (DBG) log("OCSGL is not available");
                Toast.makeText(getApplicationContext(), R.string.str_connect_error, Toast.LENGTH_SHORT).show();
            }
            return true;
        }

        return false;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.femto_cell_search);

        int mPhoneId = getIntent().getIntExtra("phoneId", -1);
        log("mPhoneId = " + mPhoneId);
        if (TelephonyManager.getDefault().isMultiSimEnabled()) {
            if (mPhoneId < 0) {
                return;
            }
            mPhone = (PhoneFactory.getPhones())[mPhoneId]; // 0 means that the
                                                           // SIM card 1, we can
                                                           // get phone from the
                                                           // array.
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        mIsForeground = true;
    }

    @Override
    public void onPause() {
        super.onPause();
        mIsForeground = false;
    }

    @Override
    protected void onDestroy() {
        //recover the data
        if(dataEnable) {
            mPhone.setInternalDataEnabled(true, null);
            dataEnable = false;
        }
        mPhone = null;
        super.onDestroy();
    }

    @Override
    protected Dialog onCreateDialog(int id) {

        if (id == DIALOG_NETWORK_SEARCH_FEMTO_CELL) {
            ProgressDialog dialog = new ProgressDialog(this);
            switch (id) {
                case DIALOG_NETWORK_SEARCH_FEMTO_CELL:
                    if (DBG) log("DIALOG_NETWORK_SEARCH_FEMTO_CELL Message : "
                            + getResources().getString(R.string.str_conncet_register));
                    dialog.setMessage(getResources().getString(R.string.str_conncet_register));
                    dialog.setCancelable(false);
                    dialog.setIndeterminate(true);
                    break;
                default:
                    // reinstate the cancelablity of the dialog.
                    dialog.setMessage(getResources().getString(R.string.load_networks_progress));
                    dialog.setCanceledOnTouchOutside(false);
                    //dialog.setOnCancelListener(this);
                    break;
            }
            return dialog;
        }
        return null;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {
        if (id == DIALOG_NETWORK_SEARCH_FEMTO_CELL) {
            // when the dialogs come up, we'll need to indicate that
            // we're in a busy state to dissallow further input.
            getPreferenceScreen().setEnabled(false);
        }
    }

    private void selectFemtoCell() {
        log("select femto cell search...");
        if (mIsForeground) {
            showDialog(DIALOG_NETWORK_SEARCH_FEMTO_CELL);
        }

        Message msg = mHandler.obtainMessage(EVENT_SET_FEMTO_CELL_SEARCH);
        if (mPhone != null) {
            mPhone.mCi.setFemtoCellSearch(FEMTO_MANUAL_MODE, msg);
        }
    }

    private boolean isFemtoCellSearchEnabled() {
        boolean OCSGLAvailable = false;
        boolean OCSGLListAvailable = false;
        if (mPhone != null) {
            IccRecords r = mPhone.getIccRecords();
            if (r != null) {
                // OCSGL supported or not
                OCSGLAvailable = r.isOCSGLAvailable();
                // valid OCSGL ID existed or not
                for (byte[] data : r.getOcsglInformation()) {
                    if (data != null && data.length > 2) {
                        OCSGLListAvailable = true;
                        break;
                    }
                } // end for ~
            }
        }
        return !OCSGLAvailable || (OCSGLAvailable && OCSGLListAvailable);
    }

    private void log(String msg) {
        if (DBG) Log.d("FemtoCellSearch", msg);
    }
}