package com.samsung.slsi.telephony.uartswitch;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.util.Log;
import android.widget.Toast;

public class UartSwitchPreferenceFragment extends PreferenceFragment implements Preference.OnPreferenceChangeListener {

    private Toast mToast = null;
    private String mUartSelPath = "";
    private static final String UART_PATH_UARTSEL_FOR_DEFAULT = "/sys/class/sec/switch/uart_sel";
    //private static final String UART_PATH_UARTSEL_FOR_ESPRESSO8890 = "/sys/class/muic/switch/uart_sel";
    //private static final String UART_PATH_UARTSEL_FOR_ESPRESSO7420 = "/sys/class/sec_class/switch/uart_sel";
    private static final String TAG = "UartSwitch";
    private static final String PROPERTY_DUN_SETTINGS = "persist.vendor.radio.dun_settings";
    private static final String KEY_UART_SWITCH_MODE = "key_uart_switch_mode";
    private static final String KEY_DUN_SETTINGS="key_dun_settings";
    private ListPreference mUartListPref, mDunListPref;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.layout.preferences);

        // default UART path
        mUartSelPath = UART_PATH_UARTSEL_FOR_DEFAULT;

        mUartListPref = (ListPreference) findPreference(KEY_UART_SWITCH_MODE);
        mUartListPref.setOnPreferenceChangeListener(this);
        mDunListPref = (ListPreference) findPreference(KEY_DUN_SETTINGS);
        mDunListPref.setOnPreferenceChangeListener(this);

    }
    public static UartSwitchPreferenceFragment newInstance() {
        UartSwitchPreferenceFragment frag = new UartSwitchPreferenceFragment();
        return frag;
    }

    @Override
    public void onResume() {
        super.onResume();
        updateUi();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "onPreferenceChange() : " + preference.getKey());
        if (KEY_UART_SWITCH_MODE.equals(preference.getKey())) {
            Log.d(TAG, "Changed to " + newValue);
            preference.setSummary(newValue.toString());
            if (newValue.toString().contains("AP")) writeParametersToUART(mUartSelPath, "AP");
            else writeParametersToUART(mUartSelPath, "CP");
            showToastMessage(newValue.toString());
            return true;
        } else if (KEY_DUN_SETTINGS.equals(preference.getKey())) {
            SystemProperties.set(PROPERTY_DUN_SETTINGS, mDunListPref.findIndexOfValue(newValue.toString())+"");
            preference.setSummary(newValue.toString());
            showToastMessage(newValue.toString());
            Reboot();
            return true;
        }
        return false;
    }
    public void updateUi() {
        Log.d(TAG, "update UI");
        File uartSysfs = new File(mUartSelPath);
        if(uartSysfs.exists()==false) {
            mUartListPref.setEnabled(false);
            Log.e(TAG, "UART sysfs does not exist.");
            return;
        }
        String path = readParametersFromUART(mUartSelPath);
        if (path.contains("AP")) {
            mUartListPref.setValueIndex(0);
            mUartListPref.setSummary(mUartListPref.getValue());
        }
        else if (path.contains("CP")) {
            mUartListPref.setValueIndex(1);
            mUartListPref.setSummary(mUartListPref.getValue());
        }
        else {
            Log.e(TAG, "The read value is niether AP nor CP.");
            Log.e(TAG, "AP UART is set by default.");
            mUartListPref.setValueIndex(0);
            mUartListPref.setSummary(mUartListPref.getValue());
        }

        // DUN settings
        int dunSettings = Integer.parseInt(String.valueOf(SystemProperties.get(PROPERTY_DUN_SETTINGS, "0")));
        mDunListPref.setValueIndex(dunSettings);
        mDunListPref.setSummary(mDunListPref.getValue());
    }

    public void showToastMessage(String str) {
        if (mToast == null)
            mToast = Toast.makeText(getContext(), str, Toast.LENGTH_SHORT);
        else
            mToast.setText(str);
        mToast.show();
    }

    public void writeParametersToUART(String path, String str) {
        FileWriter fw = null;
        BufferedWriter bw = null;
        try {
            fw = new FileWriter(path);
            bw = new BufferedWriter(fw);
            bw.write(str);
        } catch (IOException e) {
            Log.e(TAG, "IOException Write file path : " + path + " value : " + str + " err : " + e.getMessage());
        } finally {
            if(fw != null && bw != null) {
                try {
                    bw.close();
                    fw.close();
                } catch (IOException e) {
                    Log.e(TAG, "IOException in closing classes:  path = " + path + ", string = " + str + ", err = " + e.getMessage());
                }
            }
        }
    }

    public String readParametersFromUART(String path) {
        FileReader fr = null;
        BufferedReader br = null;
        String str = "";
        try {
            fr = new FileReader(path);
            br = new BufferedReader(fr, 4096);
            if (br != null)
                str = br.readLine();
        } catch (IOException e) {
            Log.e(TAG, "IOException Read file path : " + path + " value : " + str + " err : " + e.getMessage());
        } finally {
            if(fr != null && br != null) {
                try {
                    fr.close();
                    br.close();
                } catch (IOException e) {
                    Log.e(TAG, "IOException in closing classes:  path = " + path + ", string = " + str + ", err = " + e.getMessage());
                }
            }
        }
        return str;
    }

    private void Reboot() {
        AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
        builder.setMessage("Reboot the device").setCancelable(false)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        PowerManager pm = (PowerManager) getContext().getSystemService(Context.POWER_SERVICE);
                        pm.reboot(null);
                    }
                }).setNegativeButton("No", null);
        AlertDialog dialog = builder.create();
        dialog.show();
    }
}
