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


import java.util.ArrayList;

import android.app.AlertDialog;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;

import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.Uri;
import android.os.Bundle;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;

import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import android.view.inputmethod.InputMethodManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;

import android.widget.Toast;
import android.widget.Button;
import android.widget.EditText;

import com.samsung.slsi.telephony.datatestmode.R;

public class  DataRetryModePreferenceFragment extends PreferenceFragment {

    private static final String TAG = "DataRetryModePreferenceFragment";
    private static final Uri CONTENT_URI = Uri.parse("content://telephony/carriers");
    private ArrayList<NetworkCallback> mNetworkCallback = new ArrayList<NetworkCallback>();
    private ArrayList<SwitchPreference> mPrefList = new ArrayList<SwitchPreference>();
    private Button mBtnSearch;
    private static String mPlmn = "";
    private static int mSubId = -1;
    private static final String TYPE = "type";
    private static long delayoption =10 * 1000L; //default : 10 sec
    private static String status =DataRetryModeConstants.RESULT_CODE; // default value

    private static TelephonyManager mT;
    private static ConnectivityManager mC;
    private static NetworkRequest.Builder builder = new NetworkRequest.Builder();
    private static NetworkRequest mNetRequest;
    private static NetworkCallback mNetCallback;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case DataRetryModeConstants.EVENT_START_RETRY:
                //Send Network request every XX seconds
                startTask.run();
                Log.d(TAG, "EVENT_START_RETRY Action");
                break;
            case DataRetryModeConstants.EVENT_STOP_RETRY:
                Log.d(TAG, "EVENT_STOP_RETRY Action");
                mHandler.removeCallbacks(startTask);
                stopTask.run();
                //getActivity().finish();
                break;
            default:
                Log.d(TAG, "Wrong Action");
                break;
            }
        }
    };

    private Runnable startTask = new Runnable(){
        @Override
         public void run() {
              try{
                    Log.d(TAG, "Send Network Request(start)");
                    mC.requestNetwork(mNetRequest, mNetCallback);
               } catch (Exception e){
                    Log.e(TAG, "Exception: mNetworkCallback is unstable.");
               }
           mHandler.postDelayed(this, getDelayopt()*1000L);
       }
   };

   private Runnable stopTask = new Runnable(){
       @Override
        public void run() {
             try{
                   Log.d(TAG, "Send Network Request(stop) ");
                   mC.unregisterNetworkCallback(mNetCallback);
             } catch (Exception e){
                   Log.e(TAG, "Exception: mNetworkCallback is unstable.");
           }
      }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.preference_dataretry_mode);
        mT = (TelephonyManager) getContext().getSystemService(Context.TELEPHONY_SERVICE);
        fillList();
        getInputDelay();
        Log.d(TAG, "onCreate");
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if(startTask!=null){
            mHandler.removeCallbacks(startTask);
            mHandler.removeCallbacks(stopTask);
        }
        Log.d(TAG, "onDestroy");
    }

    private String[] parseTypes(String types) {
        String[] result;
        // If unset, set to DEFAULT.
        if (types == null || types.equals("")) {
            result = new String[1];
            result[0] = DataRetryModeConstants.APN_TYPE_ALL;
        } else {
            result = types.split(",");
        }
        return result;
    }

    private void fillList() {
        PreferenceGroup root = (PreferenceGroup) findPreference(DataRetryModeConstants.KEY_APN_LIST);

        if (root != null) {
            root.removeAll();
            mNetworkCallback.clear();
            mPrefList.clear();
            String where = "numeric=\"" + mPlmn + "\"";
            Cursor cursor = getContext().getContentResolver().query(
                    CONTENT_URI, new String[] { "_id", "name", "apn", "type", "numeric" },
                    where, null, DataRetryModeConstants.DEFAULT_SORT_ORDER);
            String numeric = "";
            if (cursor != null) {
                cursor.moveToFirst();
                while (!cursor.isAfterLast()) {
                    String key = cursor.getString(DataRetryModeConstants.ID_INDEX);
                    String[] types = parseTypes(cursor.getString(cursor.getColumnIndexOrThrow(TYPE)));
                    String apn = cursor.getString(DataRetryModeConstants.APN_INDEX);
                    numeric= cursor.getString(4);
                    for (int i = 0; i < types.length; i++) {
                        SwitchPreference pref = new SwitchPreference(getContext());
                            if (pref != null) {
                                pref.setTitle(types[i]);
                                pref.setSummary(apn);
                                pref.setKey(key);
                                root.addPreference(pref);
                                pref.setChecked(false);
                                mNetworkCallback.add(new NetworkCallback());
                                mPrefList.add(pref);
                            }
                    }
                    cursor.moveToNext();
                }
                cursor.close();
                Toast.makeText(getContext(), "numeric = "+numeric, Toast.LENGTH_LONG).show();
            }
        }
    }

    private int convertApnTypeToNetworkCapability(String apnType) {
        if (!TextUtils.isEmpty(apnType)) {
            if (apnType.equals(DataRetryModeConstants.APN_TYPE_DEFAULT)) {
                return NetworkCapabilities.NET_CAPABILITY_INTERNET;
            } else if (apnType.equals(DataRetryModeConstants.APN_TYPE_MMS)) {
                return NetworkCapabilities.NET_CAPABILITY_MMS;
            } else if (apnType.equals(DataRetryModeConstants.APN_TYPE_DUN)) {
                return NetworkCapabilities.NET_CAPABILITY_DUN;
            } else if (apnType.equals(DataRetryModeConstants.APN_TYPE_FOTA)) {
                return NetworkCapabilities.NET_CAPABILITY_FOTA;
            } else if (apnType.equals(DataRetryModeConstants.APN_TYPE_IMS)) {
                return NetworkCapabilities.NET_CAPABILITY_IMS;
            } else if (apnType.equals(DataRetryModeConstants.APN_TYPE_CBS)) {
                return NetworkCapabilities.NET_CAPABILITY_CBS;
            } else if (apnType.equals(DataRetryModeConstants.APN_TYPE_SUPL)) {
                return NetworkCapabilities.NET_CAPABILITY_SUPL;
            }
        }
        return NetworkCapabilities.NET_CAPABILITY_INTERNET;
    }

    private NetworkRequest getNetworkRequest(String apnType) {
        builder.addCapability(convertApnTypeToNetworkCapability(apnType));
        builder.addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED);
        builder.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);
        builder.setNetworkSpecifier(Integer.toString(mSubId));
        mNetRequest = builder.build();
        return mNetRequest;
   }

    private String checkType(String type) {
        if (type.contains("ims")) {
            return "ims";
        } else if (type.contains("cbs")) {
            return "cbs";
        }
        return type;
    }

    private void setDelayopt(long opt){
       delayoption =opt;
       Log.d(TAG, "setDelayopt : " +delayoption);
    }

    private long getDelayopt(){
        return delayoption;
    }

    private void setResultCode(String rs){
        status =rs;
    }

    private String getResultCode(){
        return status;
    }

    private long getInputDelay()
    {
        LayoutInflater li = LayoutInflater.from(getContext());
        View promptsView = li.inflate(R.layout.prompt, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getContext());
        alertDialogBuilder.setView(promptsView);
        final EditText userInput = (EditText) promptsView.findViewById(R.id.editTextDialogUserInput);

        setResultCode(DataRetryModeConstants.RESULT_CODE);
        alertDialogBuilder
         .setCancelable(false)
         .setPositiveButton("OK",
           new DialogInterface.OnClickListener() {
             public void onClick(DialogInterface dialog,int id) {
                 String delay = userInput.getText().toString();
                 try{
                      Log.d(TAG, "getInputDelay(ON) ");
                      setDelayopt(Long.parseLong(delay));
                      setResultCode(DataRetryModeConstants.RESULT_OK);
                 } catch(NumberFormatException nfe) {
                      Log.d(TAG, "Couldn't parse " +delay);
                      setResultCode(DataRetryModeConstants.RESULT_CANCEL);
                 }
             }
           })
         .setNegativeButton("Cancel",
           new DialogInterface.OnClickListener() {
               public void onClick(DialogInterface dialog,int id) {
                   Log.d(TAG, "getInputDelay(ON) ");
                   setResultCode(DataRetryModeConstants.RESULT_CANCEL);
                   dialog.cancel();
             }
           });
           AlertDialog alertDialog = alertDialogBuilder.create();
           alertDialog.show();

           return delayoption;
    }

    private void checkToggledOne(String type){
        for(int i=0;i<mPrefList.size();i++){
             String curtype =mPrefList.get(i).getTitle().toString();
             if(mPrefList.get(i).isChecked() && (!curtype.equals(type)))
                 mPrefList.get(i).setChecked(false);
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
         Log.d(TAG, "onPreferenceTreeClick()");
         SwitchPreference switch_pref = (SwitchPreference) preference;
         Uri uri = ContentUris.withAppendedId(CONTENT_URI, Integer.parseInt(switch_pref.getKey()));
         mC = (ConnectivityManager) getContext().getSystemService(Context.CONNECTIVITY_SERVICE);
         String type = switch_pref.getTitle().toString();
         Log.d(TAG, "Click: " + type + ", " + switch_pref.getOrder());
         type = checkType(type);

         while(!getResultCode().equals(DataRetryModeConstants.RESULT_CODE)){
              if (switch_pref.isChecked()) {
                    checkToggledOne(type);
                    getNetworkRequest(type);
                    mNetCallback= mNetworkCallback.get(switch_pref.getOrder());
                    Message msg = mHandler.obtainMessage(DataRetryModeConstants.EVENT_START_RETRY);
                    mHandler.sendMessage(msg);
                    break;
              } else {
                     mNetCallback= mNetworkCallback.get(switch_pref.getOrder());
                     Message msg = mHandler.obtainMessage(DataRetryModeConstants.EVENT_STOP_RETRY);
                     mHandler.sendMessage(msg);
                     break;
              }
         }
         return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    public static DataRetryModePreferenceFragment newInstance(String plmn, int subId) {
         Log.d(TAG, "plmn: " + plmn + ", subId: " +subId);
         DataRetryModePreferenceFragment frag = new DataRetryModePreferenceFragment();
         mPlmn = plmn;
         mSubId = subId;
         return frag;
    }

}
