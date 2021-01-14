package com.android.server.wifi;

import com.android.server.SystemService;
import com.android.server.wifi.WifiInjector;
import com.android.server.wifi.scanner.WifiScanningServiceImpl;
import com.android.server.wifi.WifiMonitor;
import com.android.server.wifi.p2p.WifiP2pMonitor;
import com.android.server.wifi.p2p.WifiP2pServiceImpl;

import com.android.internal.util.State;
import com.android.internal.util.IState;
import com.android.internal.util.StateMachine;
import com.android.internal.util.Protocol;
import com.android.internal.util.AsyncChannel;

import android.os.MessageQueue;
import android.os.Messenger;
import android.os.Message;
import android.os.Handler;
import android.os.Looper;

import android.net.wifi.WifiScanner;
import android.net.wifi.WifiManager;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiSsid;
import android.net.wifi.WifiConfiguration;
import android.net.dhcp.DhcpClient;
import android.net.IpConfiguration;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CodingErrorAction;
import java.nio.CharBuffer;
import java.nio.charset.CoderResult;
import java.nio.ByteBuffer;

import android.util.Log;
import android.util.Printer;
import android.content.Context;
import android.util.SparseArray;
import java.lang.Thread;
import java.lang.String;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Proxy;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.InvocationTargetException;

import com.android.server.wifi.WifiConnectivityManager;

import java.io.UnsupportedEncodingException;

public final class SlsiWifiService extends SystemService {
    private static final String TAG = "SlsiWifiService";
    private boolean mVerboseLoggingEnabled = false;

    private Context mContext;
    private WifiInjector mWifiInjector;
    private WifiStateMachine mWifiStateMachine;
    private IState mPreProccessState;
    private IState mPostProccessState;
    private int mMiracastSate;

    public SlsiWifiService(Context context) {
        super(context);
        mContext = context;
        mWifiInjector = null;
        mWifiStateMachine = null;
        mPreProccessState = null;
        mPostProccessState = null;
        mMiracastSate = 0;
    }

    /** Field getter using reflection */
    private Object getPrivateField(final Class clazz, final Object obj, final String name)
    {
        try
        {
            Field f = clazz.getDeclaredField(name); //NoSuchFieldException
            f.setAccessible(true);
            return f.get(obj); //IllegalAccessException
        }
        catch(NoSuchFieldException | IllegalAccessException ex)
        {
            return null;
        }
    }

    /** Field setter using reflection */
    private boolean setPrivateField(final Class clazz, final Object obj, final String name, final Object value)
    {
        try
        {
            Field f = clazz.getDeclaredField(name); //NoSuchFieldException
            f.setAccessible(true);
            f.set(obj, value); //IllegalAccessException
        }
        catch(NoSuchFieldException | IllegalAccessException ex)
        {
            return false;
        }
        return true;
    }

    /** Method getter using reflection */
    private Method getMethod(final Class clazz, final String methodName, final Class[] paramTypes)
    {
        Method ret = null;
        try {
            ret = clazz.getDeclaredMethod(methodName, paramTypes);
            ret.setAccessible(true);
            return ret;
        } catch (NoSuchMethodException | NullPointerException | SecurityException ex) {
            return null;
        }
    }

    static boolean isUCNVString(byte[] str, int length) {
		char byte1;
		char byte2;
		boolean isAllASCII = true;

		for (int i = 0; i < length; i++) {
			byte1 = (char)(str[i]&0x00FF); //make unsigned

			if (byte1 >= 0x81 && byte1 < 0xFF && (i + 1) < length) {
				byte2 = (char)(str[i + 1]&0x00FF); //make unsigned
				if (byte2 >= 0x40 && byte2 < 0xFF && byte2 != 0x7F) {
					// GBK
					isAllASCII = false;
					i++;
					continue;
				} else {
					return false;
				}
			} else if (byte1 < 0x80) {
				// ASCII
				continue;
			} else {
				return false;
			}
		}

		if (isAllASCII)
			return false;

		return true;
	}

	private String Hex_To_UTF8 (WifiSsid input) {
		final String UTF8_CHARSET = "UTF-8";
		byte[] ssidByte = input.getOctets();

		if (input.octets.size() <= 0 || input.isHidden()) return "";

		Charset charset = Charset.forName(UTF8_CHARSET);
		CharsetDecoder decoder = charset.newDecoder()
			.onMalformedInput(CodingErrorAction.REPLACE)
			.onUnmappableCharacter(CodingErrorAction.REPLACE);
		CharBuffer out = CharBuffer.allocate(32);

		CoderResult result = decoder.decode(ByteBuffer.wrap(ssidByte), out, true);
		out.flip();

		if ( result.isError() )
			return "<unknown ssid>";
		else
			return out.toString();
	}

	/*
	   GB2312
	   Upper 2 Hex : A1xx ~ FExx
	   Lower 2 Hex : xxA0 ~ xxFF
	 */
	private String Hex_To_GB2312 (WifiSsid input){
		final String GB2312_CHARSET = "GB2312";
		byte[] ssidByte = input.getOctets();

		if (input.octets.size() <= 0 || input.isHidden()) return "";

		Charset charset = Charset.forName(GB2312_CHARSET);
		CharsetDecoder decoder = charset.newDecoder()
			.onMalformedInput(CodingErrorAction.REPLACE)
			.onUnmappableCharacter(CodingErrorAction.REPLACE);
		CharBuffer out = CharBuffer.allocate(32);

		CoderResult result = decoder.decode(ByteBuffer.wrap(ssidByte), out, true);
		out.flip();

		if ( result.isError() )
			return "<unknown ssid>";
		else
			return out.toString();
	}

    @Override
    public void onStart() {
        try
        {
            mWifiInjector = WifiInjector.getInstance();
            mWifiStateMachine = mWifiInjector.getWifiStateMachine();
            mWifiStateMachine.getHandler().getLooper().setHookInterface(new Looper.HookInterface(){
                @Override
                public boolean hook(boolean preprocess, Message msg)
                {
                    if(getPrivateField(Message.class, msg, "target") == null)
                    {
                        /** This is a barrier message 
                            Skip this message. */
                        return false;
                    }
                    if(preprocess)
                    {
                        mPreProccessState = mWifiStateMachine.getCurrentState();
                        switch(msg.what) {
                            case WifiP2pServiceImpl.SET_MIRACAST_MODE :
                            {
                                mMiracastSate = msg.arg1;
                                return false;
                            }
                            case WifiScanner.CMD_SCAN_RESULT :
                            {
                                WifiScanner.ParcelableScanData scanData = (WifiScanner.ParcelableScanData) msg.obj;
                                WifiScanner.ScanData[] results = scanData.getResults();
                                for (WifiScanner.ScanData sd : results) {
                                    for (ScanResult sr : sd.getResults()) {
                                        if(sr.wifiSsid != null) {
                                            String decoded = sr.SSID;
                                            if ( isUCNVString(sr.wifiSsid.getOctets(), sr.wifiSsid.octets.size()) ) {
                                                decoded = Hex_To_GB2312(sr.wifiSsid);
                                            } else {
                                                decoded = Hex_To_UTF8(sr.wifiSsid);
                                            }
                                            try{
                                                final byte[] UTF_8 = decoded.getBytes("UTF-8");
                                                sr.wifiSsid.octets.reset();
                                                sr.wifiSsid.octets.write(UTF_8, 0, UTF_8.length);
                                                sr.SSID = decoded;
                                            } catch (UnsupportedEncodingException ex) {
                                                Log.e(TAG, "Cannot change SSID encoding");
                                            }
                                        }
                                    }
                                }
                                return false;
                            }
                            case WifiScanner.CMD_FULL_SCAN_RESULT:
                            {
                                ScanResult result = (ScanResult) msg.obj;
                                if(result.wifiSsid != null) {
                                    String decoded = result.SSID;
                                    if ( isUCNVString(result.wifiSsid.getOctets(), result.wifiSsid.octets.size()) ) {
                                        decoded = Hex_To_GB2312(result.wifiSsid);
                                    } else {
                                        decoded = Hex_To_UTF8(result.wifiSsid);
                                    }
                                    try{
                                        final byte[] UTF_8 = decoded.getBytes("UTF-8");
                                        result.wifiSsid.octets.reset();
                                        result.wifiSsid.octets.write(UTF_8, 0, UTF_8.length);
                                        result.SSID = decoded;
                                    } catch (UnsupportedEncodingException ex) {
                                        Log.e(TAG, "Cannot change SSID encoding");
                                    }
                                }
                                return false;
                            }
    						default :
							{
    		                    return false; /* true: Do drop this event */
							}
                        }
                    }
                    else
                    {
                        mPostProccessState = mWifiStateMachine.getCurrentState();
                        if(mPreProccessState == null || mPostProccessState == null)
                        {
                            return false;
                        }
                        if( mPreProccessState.getName().equals("WpsRunningState") && 
                            mPostProccessState.getName().equals("DisconnectedState") && 
                            msg.what == WifiMonitor.NETWORK_CONNECTION_EVENT)
                        {
                            Method mConnectToNetwork = null;
                            String platformVersion = "";
                            if(mConnectToNetwork == null) {
                                mConnectToNetwork = getMethod(WifiNative.class, "connectToNetwork", new Class[]{WifiConfiguration.class});
                                if(mConnectToNetwork != null) {
                                    platformVersion = "O";
                                }
                            }
                            if(mConnectToNetwork == null) {
                                mConnectToNetwork = getMethod(WifiNative.class, "connectToNetwork", new Class[]{String.class, WifiConfiguration.class});
                                if(mConnectToNetwork != null) {
                                    platformVersion = "P";
                                }
                            }
                            if(platformVersion.equals("O") == true) {
                                try {
                                    mConnectToNetwork.invoke(mWifiInjector.getWifiNative(), mWifiInjector.getWifiConfigManager().getConfiguredNetworkWithPassword(msg.arg1));
                                    Log.d(TAG, "[Android O] KATMAI7872-1886[WPS Setup]: Connect to "+msg.arg1+".");
                                } catch (IllegalAccessException | InvocationTargetException ex) {
                                    Log.e(TAG, "Failed: KATMAI7872-1886[WPS Setup]");
                                    return false;
                                }
                            }
                            else if(platformVersion.equals("P") == true) {
                                String interfaceName = (String)getPrivateField(WifiStateMachine.class, mWifiStateMachine, "mInterfaceName");
                                if(interfaceName == null) {
                                    Log.e(TAG, "Failed: KATMAI7872-1886[WPS Setup]");
                                    return false;
                                }
                                try {
                                    mConnectToNetwork.invoke(mWifiInjector.getWifiNative(), interfaceName, mWifiInjector.getWifiConfigManager().getConfiguredNetworkWithPassword(msg.arg1));
                                    Log.d(TAG, "[Android P] KATMAI7872-1886[WPS Setup]: Connect to "+msg.arg1+".");
                                } catch (IllegalAccessException | InvocationTargetException ex) {
                                    Log.e(TAG, "Failed: KATMAI7872-1886[WPS Setup]");
                                    return false;
                                }
                            }
                            else {
                                Log.e(TAG, "Failed: KATMAI7872-1886[WPS Setup]");
                            }
                        }
                        else if(mPostProccessState.getName().equals("ObtainingIpState") && 
                                msg.what == WifiManager.SAVE_NETWORK)
                        {
                            final WifiConfiguration currentConfig = mWifiStateMachine.getCurrentWifiConfiguration();
                            if(currentConfig.getIpAssignment() == IpConfiguration.IpAssignment.STATIC)
                            {
                                Method mDisconnect = null;
                                Method mConnectToNetwork = null;
                                String platformVersion = "";
                                
                                if(mDisconnect == null) {
                                    // 1. Try android O
                                    mDisconnect = getMethod(WifiNative.class, "disconnect", null);
                                    if(mDisconnect != null) {
                                        mConnectToNetwork = getMethod(WifiNative.class, "connectToNetwork", new Class[]{WifiConfiguration.class});
                                        if(mConnectToNetwork != null) {
                                            platformVersion = "O";
                                        }
                                    }
                                }
                                if(mDisconnect == null) {
                                    // 2. Try android P
                                    mDisconnect = getMethod(WifiNative.class, "disconnect", new Class[]{String.class});
                                    if(mDisconnect != null) {
                                        mConnectToNetwork = getMethod(WifiNative.class, "connectToNetwork", new Class[]{String.class, WifiConfiguration.class});
                                        if(mConnectToNetwork != null) {
                                            platformVersion = "P";
                                        }
                                    }
                                }
                                WifiInfo info = (WifiInfo)getPrivateField(WifiStateMachine.class, mWifiStateMachine, "mWifiInfo");
                                if(info == null)
                                {
                                    Log.e(TAG, "Failed: KATMAI7872-2426[Static IP Setup]");
                                    return false;
                                }
                                WifiConnectivityManager mWifiConnectivityManager = (WifiConnectivityManager)getPrivateField(WifiStateMachine.class, mWifiStateMachine, "mWifiConnectivityManager");
                                if(mWifiConnectivityManager == null)
                                {
                                    Log.e(TAG, "Failed: KATMAI7872-2426[Static IP Setup]");
                                    return false;
                                }
                                if(platformVersion.equals("O") == true) {
                                    mWifiConnectivityManager.prepareForForcedConnection(info.getNetworkId());
                                    try{
                                        mDisconnect.invoke(mWifiInjector.getWifiNative());
                                        mConnectToNetwork.invoke(mWifiInjector.getWifiNative(), mWifiInjector.getWifiConfigManager().getConfiguredNetworkWithPassword(msg.arg1));
                                        Log.d(TAG, "[Android O] KATMAI7872-2426[Static IP Setup]: Connect to "+info.getNetworkId()+".");
                                        Log.d(TAG, "[Android O] Static IP setup logic. NetID="+info.getNetworkId()+".");
                                    } catch (IllegalAccessException | InvocationTargetException ex) {
                                        Log.e(TAG, "Failed: KATMAI7872-2426[Static IP Setup]");
                                        return false;
                                    }
                                }
                                else if(platformVersion.equals("P") == true) {
                                    String interfaceName = (String)getPrivateField(WifiStateMachine.class, mWifiStateMachine, "mInterfaceName");
                                    if(interfaceName == null) {
                                        Log.e(TAG, "Failed: KATMAI7872-2426[Static IP Setup]");
                                        return false;
                                    }
                                    mWifiConnectivityManager.prepareForForcedConnection(info.getNetworkId());
                                    try {
                                        mDisconnect.invoke(mWifiInjector.getWifiNative(), interfaceName);
                                        mConnectToNetwork.invoke(mWifiInjector.getWifiNative(), interfaceName, mWifiInjector.getWifiConfigManager().getConfiguredNetworkWithPassword(msg.arg1));
                                        Log.d(TAG, "[Android P] KATMAI7872-2426[Static IP Setup]: Connect to "+info.getNetworkId()+".");
                                        Log.d(TAG, "[Android P] Static IP setup logic. NetID="+info.getNetworkId()+".");
                                    } catch (IllegalAccessException | InvocationTargetException ex) {
                                        Log.e(TAG, "Failed: KATMAI7872-2426[Static IP Setup]");
                                        return false;
                                    }
                                }
                                else {
                                    Log.e(TAG, "Failed: KATMAI7872-2426[Static IP Setup]");
                                }
                            }
                        }
                    }
                    /** false: Do not drop this event 
                        true: Drop this event        */
                    return false;
                }
            });
            Log.e(TAG, "SlsiWifiService has been started\n");
        }
        catch (IllegalStateException ex)
        {
            Log.e(TAG, "Msg: "+ex);
        }
    }

    @Override
    public void onBootPhase(int phase) {
        
    }
}
