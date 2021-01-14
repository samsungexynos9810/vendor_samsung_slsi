/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.apnsettingmode;

import static android.content.Context.TELEPHONY_SERVICE;

import android.preference.PreferenceActivity;
import android.preference.Preference;
import android.preference.MultiSelectListPreference;
import android.preference.EditTextPreference;
import android.preference.SwitchPreference;
import android.preference.ListPreference;
import android.preference.Preference.OnPreferenceChangeListener;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.provider.Telephony;

import android.telephony.CarrierConfigManager;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnKeyListener;


import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.util.ArrayUtils;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.samsung.slsi.telephony.datatestmode.R;

public class ApnEditorActivity extends PreferenceActivity {

private final static String TAG = "ApnEditorActivity";
private final static boolean VDBG = true;   // STOPSHIP if true

private final static String KEY_AUTH_TYPE = "auth_type";
private final static String KEY_PROTOCOL = "apn_protocol";
private final static String KEY_ROAMING_PROTOCOL = "apn_roaming_protocol";
private final static String KEY_CARRIER_ENABLED = "carrier_enabled";
private final static String KEY_BEARER_MULTI = "bearer_multi";
private final static String KEY_MVNO_TYPE = "mvno_type";
private final static String KEY_PASSWORD = "apn_password";

private final static String ACTION_VIEW="com.samsung.slsi.telephony.apnsettingmode.action.VIEW";
private final static String ACTION_EDIT="com.samsung.slsi.telephony.apnsettingmode.action.EDIT";
private final static String ACTION_INSERT="com.samsung.slsi.telephony.apnsettingmode.action.INSERT";


private static final String MENU_EDIT = "menu_edit";
private static final String MENU_SAVE = "menu_save";

static String sNotSet;
EditTextPreference mName;
EditTextPreference mApn;
EditTextPreference mProxy;
EditTextPreference mPort;
EditTextPreference mUser;
EditTextPreference mServer;
EditTextPreference mPassword;
EditTextPreference mMmsc;
EditTextPreference mMcc;
EditTextPreference mMnc;
EditTextPreference mMmsProxy;
EditTextPreference mMmsPort;
ListPreference mAuthType;
EditTextPreference mApnType;
ListPreference mProtocol;
ListPreference mRoamingProtocol;
SwitchPreference mCarrierEnabled;
MultiSelectListPreference mBearerMulti;
ListPreference mMvnoType;
EditTextPreference mMvnoMatchData;


ApnData mApnData;

private String mCurMnc;
private String mCurMcc;

private boolean mNewApn;
private int mSubId;
private TelephonyManager mTelephonyManager;
private int mBearerInitialVal = 0;
private String mMvnoTypeStr;
private String mMvnoMatchDataStr;
private String[] mReadOnlyApnTypes;
private String[] mReadOnlyApnFields;
private boolean mReadOnlyApn;
private Uri mCarrierUri;

/**
 * Standard projection for the interesting columns of a normal note.
 */
private static final String[] sProjection = new String[] {
        Telephony.Carriers._ID,     // 0
        Telephony.Carriers.NAME,    // 1
        Telephony.Carriers.APN,     // 2
        Telephony.Carriers.PROXY,   // 3
        Telephony.Carriers.PORT,    // 4
        Telephony.Carriers.USER,    // 5
        Telephony.Carriers.SERVER,  // 6
        Telephony.Carriers.PASSWORD, // 7
        Telephony.Carriers.MMSC, // 8
        Telephony.Carriers.MCC, // 9
        Telephony.Carriers.MNC, // 10
        Telephony.Carriers.NUMERIC, // 11
        Telephony.Carriers.MMSPROXY,// 12
        Telephony.Carriers.MMSPORT, // 13
        Telephony.Carriers.AUTH_TYPE, // 14
        Telephony.Carriers.TYPE, // 15
        Telephony.Carriers.PROTOCOL, // 16
        Telephony.Carriers.CARRIER_ENABLED, // 17
        Telephony.Carriers.BEARER, // 18
        Telephony.Carriers.BEARER_BITMASK, // 19
        Telephony.Carriers.ROAMING_PROTOCOL, // 20
        Telephony.Carriers.MVNO_TYPE,   // 21
        Telephony.Carriers.MVNO_MATCH_DATA,  // 22
        Telephony.Carriers.EDITED_STATUS,   // 23
        Telephony.Carriers.USER_EDITABLE    //24
};

private static final int ID_INDEX = 0;

static final int NAME_INDEX = 1;
static final int APN_INDEX = 2;
private static final int PROXY_INDEX = 3;
private static final int PORT_INDEX = 4;
private static final int USER_INDEX = 5;
private static final int SERVER_INDEX = 6;
private static final int PASSWORD_INDEX = 7;
private static final int MMSC_INDEX = 8;

static final int MCC_INDEX = 9;
static final int MNC_INDEX = 10;
private static final int MMSPROXY_INDEX = 12;
private static final int MMSPORT_INDEX = 13;
private static final int AUTH_TYPE_INDEX = 14;
private static final int TYPE_INDEX = 15;
private static final int PROTOCOL_INDEX = 16;
static final int CARRIER_ENABLED_INDEX = 17;
private static final int BEARER_INDEX = 18;
private static final int BEARER_BITMASK_INDEX = 19;
private static final int ROAMING_PROTOCOL_INDEX = 20;
private static final int MVNO_TYPE_INDEX = 21;
private static final int MVNO_MATCH_DATA_INDEX = 22;
private static final int EDITED_INDEX = 23;
private static final int USER_EDITABLE_INDEX = 24;

public static final String EXTRA_POSITION = "position";
public static final String RESTORE_CARRIERS_URI =
    "content://telephony/carriers/restore";
public static final String PREFERRED_APN_URI =
    "content://telephony/carriers/preferapn";

public static final String APN_ID = "apn_id";
public static final String SUB_ID = "sub_id";
public static final String MVNO_TYPE = "mvno_type";
public static final String MVNO_MATCH_DATA = "mvno_match_data";

private static final Uri DEFAULTAPN_URI = Uri.parse(RESTORE_CARRIERS_URI);
private static final Uri PREFERAPN_URI = Uri.parse(PREFERRED_APN_URI);

  Preference.OnPreferenceChangeListener onPreferenceChangeListener = new Preference.OnPreferenceChangeListener() {
      public boolean onPreferenceChange(Preference preference, Object newValue) {
       String key = preference.getKey();
              if (KEY_AUTH_TYPE.equals(key)) {
                  try {
                      int index = Integer.parseInt((String) newValue);
                      mAuthType.setValueIndex(index);

                      String[] values = getResources().getStringArray(R.array.apn_auth_entries);
                      mAuthType.setSummary(values[index]);
                  } catch (NumberFormatException e) {
                      return false;
                  }
              } else if (KEY_PROTOCOL.equals(key)) {
                  String protocol = protocolDescription((String) newValue, mProtocol);
                  if (protocol == null) {
                      return false;
                  }
                  mProtocol.setSummary(protocol);
                  mProtocol.setValue((String) newValue);
              } else if (KEY_ROAMING_PROTOCOL.equals(key)) {
                  String protocol = protocolDescription((String) newValue, mRoamingProtocol);
                  if (protocol == null) {
                      return false;
                  }
                  mRoamingProtocol.setSummary(protocol);
                  mRoamingProtocol.setValue((String) newValue);
              } else if (KEY_BEARER_MULTI.equals(key)) {
                  String bearer = bearerMultiDescription((Set<String>) newValue);
                  if (bearer == null) {
                      return false;
                  }
                  mBearerMulti.setValues((Set<String>) newValue);
                  mBearerMulti.setSummary(bearer);
              } else if (KEY_MVNO_TYPE.equals(key)) {
                  String mvno = mvnoDescription((String) newValue);
                  if (mvno == null) {
                      return false;
                  }
                  mMvnoType.setValue((String) newValue);
                  mMvnoType.setSummary(mvno);
              } else if (KEY_PASSWORD.equals(key)) {
                  mPassword.setSummary(starify(newValue != null ? String.valueOf(newValue) : ""));
              } else if (KEY_CARRIER_ENABLED.equals(key)) {
                  // do nothing
              } else {
                  preference.setSummary(checkNull(newValue != null ? String.valueOf(newValue) : null));
              }

              return true;

      }
  };

@Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

@Override
public boolean onOptionsItemSelected(MenuItem item) {
     int id = item.getItemId();
         if (id == R.id.menu_save) {
              Log.d(TAG, "menu_save ");
              if (validateAndSaveApnData()) finish();
        }else if (id == R.id.menu_delete){
              Log.d(TAG, "menu_delete ");
              deleteApn();
              finish();
              return true;
        }else if (id == R.id.menu_cancel){
              Log.d(TAG, "menu_cancel ");
              finish();
              return true;
        }
     return super.onOptionsItemSelected(item);
}

@Override
protected void onCreate(Bundle saveInstance) {
    super.onCreate(saveInstance);
    addPreferencesFromResource(R.xml.apn_setting_editor);

    sNotSet = getResources().getString(R.string.apn_not_set);
    mName = (EditTextPreference) findPreference("apn_name");
    mApn = (EditTextPreference) findPreference("apn_apn");
    mProxy = (EditTextPreference) findPreference("apn_http_proxy");
    mPort = (EditTextPreference) findPreference("apn_http_port");
    mUser = (EditTextPreference) findPreference("apn_user");
    mServer = (EditTextPreference) findPreference("apn_server");
    mPassword = (EditTextPreference) findPreference(KEY_PASSWORD);
    mMmsProxy = (EditTextPreference) findPreference("apn_mms_proxy");
    mMmsPort = (EditTextPreference) findPreference("apn_mms_port");
    mMmsc = (EditTextPreference) findPreference("apn_mmsc");
    mMcc = (EditTextPreference) findPreference("apn_mcc");
    mMnc = (EditTextPreference) findPreference("apn_mnc");
    mApnType = (EditTextPreference) findPreference("apn_type");
    mAuthType = (ListPreference) findPreference(KEY_AUTH_TYPE);
    mProtocol = (ListPreference) findPreference(KEY_PROTOCOL);
    mRoamingProtocol = (ListPreference) findPreference(KEY_ROAMING_PROTOCOL);
    mCarrierEnabled = (SwitchPreference) findPreference(KEY_CARRIER_ENABLED);
    mBearerMulti = (MultiSelectListPreference) findPreference(KEY_BEARER_MULTI);
    mMvnoType = (ListPreference) findPreference(KEY_MVNO_TYPE);
    mMvnoMatchData = (EditTextPreference) findPreference("mvno_match_data");


    final Intent intent = getIntent();
    final String action = intent.getAction();
    mSubId = intent.getIntExtra(SUB_ID,
            SubscriptionManager.INVALID_SUBSCRIPTION_ID);

    Uri uri = null;

    if (action.equals(ACTION_EDIT)) {
        uri = intent.getData();
        if (!uri.isPathPrefixMatch(Telephony.Carriers.CONTENT_URI)) {
            Log.e(TAG, "Edit request not for carrier table. Uri: " + uri);
            finish();
            return;
        }
    } else if (action.equals(ACTION_INSERT)) {
        mCarrierUri = intent.getData();
        if (!mCarrierUri.isPathPrefixMatch(Telephony.Carriers.CONTENT_URI)) {
            Log.e(TAG, "Insert request not for carrier table. Uri: " + mCarrierUri);
            finish();
            return;
        }
        mNewApn = true;
        mMvnoTypeStr = intent.getStringExtra(MVNO_TYPE);
        mMvnoMatchDataStr = intent.getStringExtra(MVNO_MATCH_DATA);
    } else {
        finish();
        return;
    }

        // Creates an ApnData to store the apn data temporary, so that we don't need the cursor to
        // get the apn data. The uri is null if the action is ACTION_INSERT, that mean there is no
        // record in the database, so create a empty ApnData to represent a empty row of database.
        if (uri != null) {
            mApnData = getApnDataFromUri(uri);
        } else {
            mApnData = new ApnData(sProjection.length);
        }

        for (int i = 0; i < getPreferenceScreen().getPreferenceCount(); i++) {
            getPreferenceScreen().getPreference(i).setOnPreferenceChangeListener(onPreferenceChangeListener);
        }

      fillUI(saveInstance == null);

}

void fillUI(boolean firstTime) {
        if (firstTime) {
            // Fill in all the values from the db in both text editor and summary
            mName.setText(mApnData.getString(NAME_INDEX));
            mApn.setText(mApnData.getString(APN_INDEX));
            mProxy.setText(mApnData.getString(PROXY_INDEX));
            mPort.setText(mApnData.getString(PORT_INDEX));
            mUser.setText(mApnData.getString(USER_INDEX));
            mServer.setText(mApnData.getString(SERVER_INDEX));
            mPassword.setText(mApnData.getString(PASSWORD_INDEX));
            mMmsProxy.setText(mApnData.getString(MMSPROXY_INDEX));
            mMmsPort.setText(mApnData.getString(MMSPORT_INDEX));
            mMmsc.setText(mApnData.getString(MMSC_INDEX));
            mMcc.setText(mApnData.getString(MCC_INDEX));
            mMnc.setText(mApnData.getString(MNC_INDEX));
            mApnType.setText(mApnData.getString(TYPE_INDEX));
            if (mNewApn) {
                String numeric = mTelephonyManager.getSimOperator(mSubId);
                // MCC is first 3 chars and then in 2 - 3 chars of MNC
                if (numeric != null && numeric.length() > 4) {
                    // Country code
                    String mcc = numeric.substring(0, 3);
                    // Network code
                    String mnc = numeric.substring(3);
                    // Auto populate MNC and MCC for new entries, based on what SIM reports
                    mMcc.setText(mcc);
                    mMnc.setText(mnc);
                    mCurMnc = mnc;
                    mCurMcc = mcc;
                }
            }
            int authVal = mApnData.getInteger(AUTH_TYPE_INDEX, -1);
            if (authVal != -1) {
                mAuthType.setValueIndex(authVal);
            } else {
                mAuthType.setValue(null);
            }

            mProtocol.setValue(mApnData.getString(PROTOCOL_INDEX));
            mRoamingProtocol.setValue(mApnData.getString(ROAMING_PROTOCOL_INDEX));
            mCarrierEnabled.setChecked(mApnData.getInteger(CARRIER_ENABLED_INDEX, 1) == 1);
            mBearerInitialVal = mApnData.getInteger(BEARER_INDEX, 0);

            HashSet<String> bearers = new HashSet<String>();
            int bearerBitmask = mApnData.getInteger(BEARER_BITMASK_INDEX, 0);
            if (bearerBitmask == 0) {
                if (mBearerInitialVal == 0) {
                    bearers.add("" + 0);
                }
            } else {
                int i = 1;
                while (bearerBitmask != 0) {
                    if ((bearerBitmask & 1) == 1) {
                        bearers.add("" + i);
                    }
                    bearerBitmask >>= 1;
                    i++;
                }
            }

            if (mBearerInitialVal != 0 && bearers.contains("" + mBearerInitialVal) == false) {
                // add mBearerInitialVal to bearers
                bearers.add("" + mBearerInitialVal);
            }
            mBearerMulti.setValues(bearers);

            mMvnoType.setValue(mApnData.getString(MVNO_TYPE_INDEX));
            mMvnoMatchData.setEnabled(false);
            mMvnoMatchData.setText(mApnData.getString(MVNO_MATCH_DATA_INDEX));
            if (mNewApn && mMvnoTypeStr != null && mMvnoMatchDataStr != null) {
                mMvnoType.setValue(mMvnoTypeStr);
                mMvnoMatchData.setText(mMvnoMatchDataStr);
            }
        }

        mName.setSummary(checkNull(mName.getText()));
        mApn.setSummary(checkNull(mApn.getText()));
        mProxy.setSummary(checkNull(mProxy.getText()));
        mPort.setSummary(checkNull(mPort.getText()));
        mUser.setSummary(checkNull(mUser.getText()));
        mServer.setSummary(checkNull(mServer.getText()));
        mPassword.setSummary(starify(mPassword.getText()));
        mMmsProxy.setSummary(checkNull(mMmsProxy.getText()));
        mMmsPort.setSummary(checkNull(mMmsPort.getText()));
        mMmsc.setSummary(checkNull(mMmsc.getText()));
        mMcc.setSummary(formatInteger(checkNull(mMcc.getText())));
        mMnc.setSummary(formatInteger(checkNull(mMnc.getText())));
        mApnType.setSummary(checkNull(mApnType.getText()));

        String authVal = mAuthType.getValue();
        if (authVal != null) {
            int authValIndex = Integer.parseInt(authVal);
            mAuthType.setValueIndex(authValIndex);

            String[] values = getResources().getStringArray(R.array.apn_auth_entries);
            mAuthType.setSummary(values[authValIndex]);
        } else {
            mAuthType.setSummary(sNotSet);
        }

        mProtocol.setSummary(checkNull(protocolDescription(mProtocol.getValue(), mProtocol)));
        mRoamingProtocol.setSummary(
                checkNull(protocolDescription(mRoamingProtocol.getValue(), mRoamingProtocol)));
        mBearerMulti.setSummary(
                checkNull(bearerMultiDescription(mBearerMulti.getValues())));
        mMvnoType.setSummary(
                checkNull(mvnoDescription(mMvnoType.getValue())));
        mMvnoMatchData.setSummary(checkNull(mMvnoMatchData.getText()));

        mCarrierEnabled.setEnabled(true);

    }

/**
  * Returns the UI choice (e.g., "IPv4/IPv6") corresponding to the given
  * raw value of the protocol preference (e.g., "IPV4V6"). If unknown,
  * return null.
  */
 private String protocolDescription(String raw, ListPreference protocol) {
     int protocolIndex = protocol.findIndexOfValue(raw);
     if (protocolIndex == -1) {
         return null;
     } else {
         String[] values = getResources().getStringArray(R.array.apn_protocol_entries);
         try {
             return values[protocolIndex];
         } catch (ArrayIndexOutOfBoundsException e) {
             return null;
         }
     }
 }


private String bearerMultiDescription(Set<String> raw) {
    String[] values = getResources().getStringArray(R.array.bearer_entries);
    StringBuilder retVal = new StringBuilder();
    boolean first = true;
    for (String bearer : raw) {
        int bearerIndex = mBearerMulti.findIndexOfValue(bearer);
        try {
            if (first) {
                retVal.append(values[bearerIndex]);
                first = false;
            } else {
                retVal.append(", " + values[bearerIndex]);
            }
        } catch (ArrayIndexOutOfBoundsException e) {
            // ignore
        }
    }
    String val = retVal.toString();
    if (!TextUtils.isEmpty(val)) {
        return val;
    }
    return null;
}

private String mvnoDescription(String newValue) {
        int mvnoIndex = mMvnoType.findIndexOfValue(newValue);
        String oldValue = mMvnoType.getValue();

        if (mvnoIndex == -1) {
            return null;
        } else {
            String[] values = getResources().getStringArray(R.array.mvno_type_entries);
            boolean mvnoMatchDataUneditable =
                    mReadOnlyApn || (mReadOnlyApnFields != null
                            && Arrays.asList(mReadOnlyApnFields)
                            .contains(Telephony.Carriers.MVNO_MATCH_DATA));
            mMvnoMatchData.setEnabled(!mvnoMatchDataUneditable && mvnoIndex != 0);
            if (newValue != null && newValue.equals(oldValue) == false) {
                if (values[mvnoIndex].equals("SPN")) {
                    mMvnoMatchData.setText(mTelephonyManager.getSimOperatorName());
                } else if (values[mvnoIndex].equals("IMSI")) {
                    String numeric = mTelephonyManager.getSimOperator(mSubId);
                    mMvnoMatchData.setText(numeric + "x");
                } else if (values[mvnoIndex].equals("GID")) {
                    mMvnoMatchData.setText(mTelephonyManager.getGroupIdLevel1());
                }
            }

            try {
                return values[mvnoIndex];
            } catch (ArrayIndexOutOfBoundsException e) {
                return null;
            }
        }
    }

 private void deleteApn() {
     if (mApnData.getUri() != null) {
         getContentResolver().delete(mApnData.getUri(), null, null);
         mApnData = new ApnData(sProjection.length);
     }
 }

 private String starify(String value) {
     if (value == null || value.length() == 0) {
         return sNotSet;
     } else {
         char[] password = new char[value.length()];
         for (int i = 0; i < password.length; i++) {
             password[i] = '*';
         }
         return new String(password);
     }
 }

static String formatInteger(String value) {
    try {
        final int intValue = Integer.parseInt(value);
        return String.format("%d", intValue);
    } catch (NumberFormatException e) {
        return value;
    }
}



/**
 * Returns {@link #sNotSet} if the given string {@code value} is null or empty. The string
 * {@link #sNotSet} typically used as the default display when an entry in the preference is
 * null or empty.
 */
private String checkNull(String value) {
    return TextUtils.isEmpty(value) ? sNotSet : value;
}

/**
 * Returns null if the given string {@code value} equals to {@link #sNotSet}. This method
 * should be used when convert a string value from preference to database.
 */
private String checkNotSet(String value) {
    return sNotSet.equals(value) ? null : value;
}


/**
 * Add key, value to {@code cv} and compare the value against the value at index in
 * {@link #mApnData}.
 *
 * <p>
 * The key, value will not add to {@code cv} if value is null.
 *
 * @return true if values are different. {@code assumeDiff} indicates if values can be assumed
 * different in which case no comparison is needed.
 */
boolean setStringValueAndCheckIfDiff(
        ContentValues cv, String key, String value, boolean assumeDiff, int index) {
    String valueFromLocalCache = mApnData.getString(index);
    if (VDBG) {
        Log.d(TAG, "setStringValueAndCheckIfDiff: assumeDiff: " + assumeDiff
                + " key: " + key
                + " value: '" + value
                + "' valueFromDb: '" + valueFromLocalCache + "'");
    }
    boolean isDiff = assumeDiff
            || !((TextUtils.isEmpty(value) && TextUtils.isEmpty(valueFromLocalCache))
            || (value != null && value.equals(valueFromLocalCache)));

    if (isDiff && value != null) {
        cv.put(key, value);
    }
    return isDiff;
}

/**
 * Add key, value to {@code cv} and compare the value against the value at index in
 * {@link #mApnData}.
 *
 * @return true if values are different. {@code assumeDiff} indicates if values can be assumed
 * different in which case no comparison is needed.
 */
boolean setIntValueAndCheckIfDiff(
        ContentValues cv, String key, int value, boolean assumeDiff, int index) {
    Integer valueFromLocalCache = mApnData.getInteger(index);
    if (VDBG) {
        Log.d(TAG, "setIntValueAndCheckIfDiff: assumeDiff: " + assumeDiff
                + " key: " + key
                + " value: '" + value
                + "' valueFromDb: '" + valueFromLocalCache + "'");
    }

    boolean isDiff = assumeDiff || value != valueFromLocalCache;
    if (isDiff) {
        cv.put(key, value);
    }
    return isDiff;
}

private String checkTypeNotSet(String value) {
    if (value == null || value.equals(sNotSet)) {
        return "default";
    } else {
        return value;
    }
}


/**
   * Validates the apn data and save it to the database if it's valid.
   *
   * <p>
   * A dialog with error message will be displayed if the APN data is invalid.
   *
   * @return true if there is no error
   */

  boolean validateAndSaveApnData() {

      String name = checkNotSet(mName.getText());
      String apn = checkNotSet(mApn.getText());
      String mcc = checkNotSet(mMcc.getText());
      String mnc = checkNotSet(mMnc.getText());
/*
      String errorMsg = validateApnData();
      if (errorMsg != null) {
          showError();
          return false;
      }
*/
      ContentValues values = new ContentValues();
      // call update() if it's a new APN. If not, check if any field differs from the db value;
      // if any diff is found update() should be called
      boolean callUpdate = mNewApn;
      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.NAME,
              name,
              callUpdate,
              NAME_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.APN,
              apn,
              callUpdate,
              APN_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.PROXY,
              checkNotSet(mProxy.getText()),
              callUpdate,
              PROXY_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.PORT,
              checkNotSet(mPort.getText()),
              callUpdate,
              PORT_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.MMSPROXY,
              checkNotSet(mMmsProxy.getText()),
              callUpdate,
              MMSPROXY_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.MMSPORT,
              checkNotSet(mMmsPort.getText()),
              callUpdate,
              MMSPORT_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.USER,
              checkNotSet(mUser.getText()),
              callUpdate,
              USER_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.SERVER,
              checkNotSet(mServer.getText()),
              callUpdate,
              SERVER_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.PASSWORD,
              checkNotSet(mPassword.getText()),
              callUpdate,
              PASSWORD_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.MMSC,
              checkNotSet(mMmsc.getText()),
              callUpdate,
              MMSC_INDEX);

      String authVal = mAuthType.getValue();
      if (authVal != null) {
          callUpdate = setIntValueAndCheckIfDiff(values,
                  Telephony.Carriers.AUTH_TYPE,
                  Integer.parseInt(authVal),
                  callUpdate,
                  AUTH_TYPE_INDEX);
      }

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.PROTOCOL,
              checkNotSet(mProtocol.getValue()),
              callUpdate,
              PROTOCOL_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.ROAMING_PROTOCOL,
              checkNotSet(mRoamingProtocol.getValue()),
              callUpdate,
              ROAMING_PROTOCOL_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.TYPE,
              checkTypeNotSet(getUserEnteredApnType()),
              callUpdate,
              TYPE_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.MCC,
              mcc,
              callUpdate,
              MCC_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.MNC,
              mnc,
              callUpdate,
              MNC_INDEX);

      values.put(Telephony.Carriers.NUMERIC, mcc + mnc);

      if (mCurMnc != null && mCurMcc != null) {
          if (mCurMnc.equals(mnc) && mCurMcc.equals(mcc)) {
              values.put(Telephony.Carriers.CURRENT, 1);
          }
      }

      Set<String> bearerSet = mBearerMulti.getValues();
      int bearerBitmask = 0;
      for (String bearer : bearerSet) {
          if (Integer.parseInt(bearer) == 0) {
              bearerBitmask = 0;
              break;
          } else {
              bearerBitmask |= ServiceState.getBitmaskForTech(Integer.parseInt(bearer));
          }
      }
      callUpdate = setIntValueAndCheckIfDiff(values,
              Telephony.Carriers.BEARER_BITMASK,
              bearerBitmask,
              callUpdate,
              BEARER_BITMASK_INDEX);

      int bearerVal;
      if (bearerBitmask == 0 || mBearerInitialVal == 0) {
          bearerVal = 0;
      } else if (ServiceState.bitmaskHasTech(bearerBitmask, mBearerInitialVal)) {
          bearerVal = mBearerInitialVal;
      } else {
          // bearer field was being used but bitmask has changed now and does not include the
          // initial bearer value -- setting bearer to 0 but maybe better behavior is to choose a
          // random tech from the new bitmask??
          bearerVal = 0;
      }
      callUpdate = setIntValueAndCheckIfDiff(values,
              Telephony.Carriers.BEARER,
              bearerVal,
              callUpdate,
              BEARER_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.MVNO_TYPE,
              checkNotSet(mMvnoType.getValue()),
              callUpdate,
              MVNO_TYPE_INDEX);

      callUpdate = setStringValueAndCheckIfDiff(values,
              Telephony.Carriers.MVNO_MATCH_DATA,
              checkNotSet(mMvnoMatchData.getText()),
              callUpdate,
              MVNO_MATCH_DATA_INDEX);

      callUpdate = setIntValueAndCheckIfDiff(values,
              Telephony.Carriers.CARRIER_ENABLED,
              mCarrierEnabled.isChecked() ? 1 : 0,
              callUpdate,
              CARRIER_ENABLED_INDEX);

      values.put(Telephony.Carriers.EDITED_STATUS, Telephony.Carriers.USER_EDITED);

      if (callUpdate) {
          final Uri uri = mApnData.getUri() == null ? mCarrierUri : mApnData.getUri();
          updateApnDataToDatabase(uri, values);
      } else {
          if (VDBG) Log.d(TAG, "validateAndSaveApnData: not calling update()");
      }

      return true;
  }


private String getUserEnteredApnType() {
    // if user has not specified a type, map it to "ALL APN TYPES THAT ARE NOT READ-ONLY"
    String userEnteredApnType = mApnType.getText();
    if (userEnteredApnType != null) userEnteredApnType = userEnteredApnType.trim();
    if ((TextUtils.isEmpty(userEnteredApnType)
            || PhoneConstants.APN_TYPE_ALL.equals(userEnteredApnType))
            && !ArrayUtils.isEmpty(mReadOnlyApnTypes)) {
        StringBuilder editableApnTypes = new StringBuilder();
        List<String> readOnlyApnTypes = Arrays.asList(mReadOnlyApnTypes);
        boolean first = true;
        for (String apnType : PhoneConstants.APN_TYPES) {
                if (first) {
                    first = false;
                } else {
                    editableApnTypes.append(",");
                }
                editableApnTypes.append(apnType);
        }
        userEnteredApnType = editableApnTypes.toString();
        Log.d(TAG, "getUserEnteredApnType: changed apn type to editable apn types: "
                + userEnteredApnType);
    }

    return userEnteredApnType;
}

private void updateApnDataToDatabase(Uri uri, ContentValues values) {

        if (uri.equals(mCarrierUri)) {
            // Add a new apn to the database
            final Uri newUri = getContentResolver().insert(mCarrierUri, values);
            if (newUri == null) {
                Log.e(TAG, "Can't add a new apn to database " + mCarrierUri);
            }
        } else {
            // Update the existing apn
            getContentResolver().update(
                    uri, values, null /* where */, null /* selection Args */);
        }

}//

ApnData getApnDataFromUri(Uri uri) {
    ApnData apnData = null;
    try (Cursor cursor = getContentResolver().query(
            uri,
            sProjection,
            null /* selection */,
            null /* selectionArgs */,
            null /* sortOrder */)) {
        if (cursor != null) {
            cursor.moveToFirst();
            apnData = new ApnData(uri, cursor);
        }
    }

    if (apnData == null) {
        Log.d(TAG, "Can't get apnData from Uri " + uri);
    }

    return apnData;
}

static class ApnData {
        /**
         * The uri correspond to a database row of the apn data. This should be null if the apn
         * is not in the database.
         */
        Uri mUri;

        /** Each element correspond to a column of the database row. */
        Object[] mData;

        ApnData(int numberOfField) {
            mData = new Object[numberOfField];
        }

        ApnData(Uri uri, Cursor cursor) {
            mUri = uri;
            mData = new Object[cursor.getColumnCount()];
            for (int i = 0; i < mData.length; i++) {
                switch (cursor.getType(i)) {
                    case Cursor.FIELD_TYPE_FLOAT:
                        mData[i] = cursor.getFloat(i);
                        break;
                    case Cursor.FIELD_TYPE_INTEGER:
                        mData[i] = cursor.getInt(i);
                        break;
                    case Cursor.FIELD_TYPE_STRING:
                        mData[i] = cursor.getString(i);
                        break;
                    case Cursor.FIELD_TYPE_BLOB:
                        mData[i] = cursor.getBlob(i);
                        break;
                    default:
                        mData[i] = null;
                }
            }
        }

        Uri getUri() {
            return mUri;
        }

        void setUri(Uri uri) {
            mUri = uri;
        }

        Integer getInteger(int index) {
            return (Integer) mData[index];
        }

        Integer getInteger(int index, Integer defaultValue) {
            Integer val = getInteger(index);
            return val == null ? defaultValue : val;
        }

        String getString(int index) {
            return (String) mData[index];
        }
    }



}
