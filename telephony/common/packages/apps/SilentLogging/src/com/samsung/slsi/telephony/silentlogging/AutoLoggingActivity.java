/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.silentlogging;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.samsung.slsi.telephony.aidl.IDMControlService;
import com.samsung.slsi.telephony.aidl.IDMControlServiceCallback;

public class AutoLoggingActivity extends Activity {

    private static final String TAG = "AutoLoggingActivity";

    private static final String TARGET_PACKAGE = "com.samsung.slsi.telephony.silentlogging";
    private static final String TARGET_COMPONENT = "com.samsung.slsi.telephony.extservice.DMControlService";
    private IDMControlService mDMControlService;
    private boolean mIsBind = false;
    private DMServiceConnection mServiceConnection;

    private Button btnConn, btnUnbind, btnStartSilent, btnStopSilent, btnStartAuto, btnStopAuto;
    private Button btnStartAutoProfile;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_autologging);

        btnConn = (Button) findViewById(R.id.btnConn);
        btnConn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick - bind service");
                if (mServiceConnection == null) {
                    mServiceConnection = new DMServiceConnection();
                    Intent intent = new Intent();
                    intent.setComponent(new ComponentName(TARGET_PACKAGE, TARGET_COMPONENT));
//                    startService(intent);
                    if (!bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE)) {
                        Log.w(TAG, "bind failure");
                    }
                }
            }
        });

        btnUnbind = (Button) findViewById(R.id.btnUnbind);
        btnUnbind.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick - unbind service");
                unbindService();
            }
        });

        btnStartSilent = (Button) findViewById(R.id.btnStartSilent);
        btnStartSilent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick - start silent");
                try {
                    mDMControlService.startSilentLogging();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        });

        btnStopSilent = (Button) findViewById(R.id.btnStopSilent);
        btnStopSilent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick - stop silent");
                try {
                    mDMControlService.stopSilentLogging();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        });

        btnStartAuto = (Button) findViewById(R.id.btnStartAuto);
        btnStartAuto.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick - start auto");
                try {
                    mDMControlService.startAutoLogging(100);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        });

        btnStopAuto = (Button) findViewById(R.id.btnStopAuto);
        btnStopAuto.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick - stop silent");
                try {
                    mDMControlService.stopAutoLogging();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        });

        btnStartAutoProfile = (Button) findViewById(R.id.btnStartAutoProfile);
        btnStartAutoProfile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick - start auto with profile");
                try {
                    mDMControlService.startAutoLoggingWithProfile(32, "/sdcard/DCIM/sample.prx");
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        });
    }

    private final IDMControlServiceCallback.Stub mCallback = new IDMControlServiceCallback.Stub() {

        @Override
        public void onResults(int CommandId, int error) throws RemoteException {
            Log.d(TAG, "onResults : [" + CommandId + "] " + error);
        }
    };

    private void unbindService() {
        if (mIsBind && mDMControlService != null) {
            unbindService(mServiceConnection);
        }
        else {
            Log.w(TAG, "service not bound");
        }
        mIsBind = false;
        mDMControlService = null;
        mServiceConnection = null;
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
        unbindService();
    }

    class DMServiceConnection implements ServiceConnection {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected");
            Log.d(TAG, "name=" + name + " service=" + service);
            if (service != null) {
                mIsBind = true;
                mDMControlService = IDMControlService.Stub.asInterface(service);
                Log.d(TAG, "IDMControlService=" + mDMControlService);
            }

            if (mDMControlService != null) {
                try {
                    mDMControlService.registerCallback(mCallback);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected");
            Log.d(TAG, "name=" + name);
        }
    }
}
