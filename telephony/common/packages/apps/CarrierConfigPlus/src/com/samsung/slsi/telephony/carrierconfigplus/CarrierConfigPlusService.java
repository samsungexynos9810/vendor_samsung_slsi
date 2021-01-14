/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.telephony.carrierconfigplus;

import java.io.IOException;
import java.util.HashMap;
import java.util.HashSet;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import android.os.Build;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.service.carrier.CarrierIdentifier;
import android.service.carrier.CarrierService;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.android.carrierconfig.R;
import com.samsung.slsi.telephony.carrierconfigplus.CarrierConfigPlusConstants.NetworkGroup;

/**
 * CarrierConfigPlus
 *
 * Provides network overrides for carrier configuration.
 *
 *   - Based on CarrierConfig in AOSP, adding additional features
 *     1. load configuration for build_carrier
 *     2. load configuration for a network group
 *     3. load configuration for a current PLMN
 */
public class CarrierConfigPlusService extends CarrierService {

    private static final String SPN_EMPTY_MATCH = "null";

    private static final String TAG = "CarrierConfigPlus";

    private XmlPullParserFactory mFactory;
    private static final String BUILD_CARRIER = SystemProperties.get("ro.vendor.config.build_carrier");

    private static HashMap<String, NetworkGroup> sNetworkGroup = new HashMap<String, NetworkGroup>();
    static {
        sNetworkGroup.put("45002", NetworkGroup.KT);
        sNetworkGroup.put("45005", NetworkGroup.SKT);
        sNetworkGroup.put("45006", NetworkGroup.LGU);

        sNetworkGroup.put("46000", NetworkGroup.CMCC);
        sNetworkGroup.put("46001", NetworkGroup.CU);
        sNetworkGroup.put("46002", NetworkGroup.CMCC);
        sNetworkGroup.put("46003", NetworkGroup.CTC);
        sNetworkGroup.put("46005", NetworkGroup.CTC);
        sNetworkGroup.put("46006", NetworkGroup.CU);
        sNetworkGroup.put("46007", NetworkGroup.CMCC);
        sNetworkGroup.put("46009", NetworkGroup.CU);
        sNetworkGroup.put("46011", NetworkGroup.CTC);
        sNetworkGroup.put("45502", NetworkGroup.CTC);
        sNetworkGroup.put("45507", NetworkGroup.CTC);
        sNetworkGroup.put("45407", NetworkGroup.CU);
        sNetworkGroup.put("45412", NetworkGroup.CMCC);
        sNetworkGroup.put("45413", NetworkGroup.CMCC);

        sNetworkGroup.put("310120", NetworkGroup.SPR);
        sNetworkGroup.put("311480", NetworkGroup.VZW);
    }

    private static HashSet<String> sBuildCarrier = new HashSet<String>();
    static {
        // China
        sBuildCarrier.add("cmcc");
        sBuildCarrier.add("ctc");
        sBuildCarrier.add("cu");

        // US
        sBuildCarrier.add("vzw");
        sBuildCarrier.add("spr");
        sBuildCarrier.add("att");
        sBuildCarrier.add("tmo");

        // Korea
        sBuildCarrier.add("skt");
        sBuildCarrier.add("kt");
        sBuildCarrier.add("lgu");

        // Open
        sBuildCarrier.add("chnopen");
        sBuildCarrier.add("europen");
    }

    public CarrierConfigPlusService() {
        Log.d(TAG, "Service created");
        mFactory = null;
    }

    /**
     * Returns carrier configuration.
     */
    @Override
    public PersistableBundle onLoadConfig(CarrierIdentifier id) {
        Log.d(TAG, "Config being fetched");

        if (id == null) {
            Log.w(TAG, "id is null");
            return null;
        }

        PersistableBundle config = new PersistableBundle();
        Log.d(TAG, "Loading config for SIM operator");
        if (!TextUtils.isEmpty(id.getMcc()) && !TextUtils.isEmpty(id.getMnc())) {
            if ("001".equals(id.getMcc()) && "01".equals(id.getMnc()) &&
                !TextUtils.isEmpty(BUILD_CARRIER) && sBuildCarrier.contains(BUILD_CARRIER)) {
                Log.d(TAG, "Loading config for SIM operator read test SIM");
                String fileName = "equipment/carrier_config_00101_" + BUILD_CARRIER + ".xml";
                readConfigFromAsset(config, fileName, id);
            } else {
                String fileName = "carrier/carrier_config_" + id.getMcc() + id.getMnc() + ".xml";
                readConfigFromAsset(config, fileName, id);
            }
        }
        else {
            Log.w(TAG, "invalid mcc and mnc");
        }

        // Treat vendor.xml as if it were appended to the carrier config file we read.
        Log.d(TAG, "Loading config for device common");
        XmlPullParser vendorInput = getApplicationContext().getResources().getXml(R.xml.vendor);
        try {
            PersistableBundle vendorConfig = readConfigFromXml(vendorInput, id);
            config.putAll(vendorConfig);
        }
        catch (IOException | XmlPullParserException e) {
            Log.e(TAG, e.toString());
        }

        // build_carrier
        Log.d(TAG, "Loading config for build_carrier");
        if (!TextUtils.isEmpty(BUILD_CARRIER) && sBuildCarrier.contains(BUILD_CARRIER)) {
            String fileName = "build_carrier/build_carrier_config_" + BUILD_CARRIER + ".xml";
            readConfigFromAsset(config, fileName, id);
        }
        else {
            Log.w(TAG, "Unsupported build_carrier: " + BUILD_CARRIER);
        }

        // find phoneId using imsi
        String imsi = id.getImsi();
        int phoneId = CarrierConfigPlusConstants.INVALID_PHONE_INDEX;
        if (!TextUtils.isEmpty(imsi)) {
            for (int i = 0; i < 2; i++) {
                int[] subIds = SubscriptionManager.getSubId(i);
                if (subIds != null && subIds.length >= 1) {
                    int subId = subIds[0];
                    if (SubscriptionManager.isValidSubscriptionId(subId)) {
                        TelephonyManager tm = TelephonyManager.from(this);
                        String subscriberId = tm.getSubscriberId(subId);
                        if (subscriberId != null && subscriberId.equals(imsi)) {
                            StringBuilder sb = new StringBuilder();
                            sb.append(" found matched imsi value {");
                            sb.append(" subId=" + subId);
                            sb.append(" phoneId=" + i + " }");
                            Log.v(TAG, sb.toString());
                            phoneId = i;
                            break;
                        }
                    }
                }
            } // end for i ~
        }

        // Current Network
        if (SubscriptionManager.isValidPhoneId(phoneId)) {
            TelephonyManager tm = TelephonyManager.from(this);
            final String operatorNumeric = tm.getNetworkOperatorForPhone(phoneId);
            Log.d(TAG, "phoneId: " + phoneId + " operator-numeric: " + operatorNumeric);

            CarrierConfigPlusProperties.setOperatorNumeric(this, operatorNumeric, phoneId);

            // Network Group
            // Grouping based on the current network numeric (VZW, Sprint, AT&T, T-Mobile, CMCC, CTC, CU and so on)
            if (!TextUtils.isEmpty(operatorNumeric) && sNetworkGroup.containsKey(operatorNumeric)) {
                NetworkGroup group = sNetworkGroup.get(operatorNumeric);
                String fileName = "network_group/network_group_config_" + group.name().toLowerCase() + ".xml";
                readConfigFromAsset(config, fileName, id);
            }
            else {
                Log.w(TAG, "Unsupported Network Group. Operator Numeric: " + operatorNumeric);
            }

            Log.d(TAG, "Loading config for current PLMN");
            if (!(TextUtils.isEmpty(operatorNumeric) ||
                  TextUtils.equals(operatorNumeric, "00000") || TextUtils.equals(operatorNumeric, "000000"))) {
                String fileName = "plmn/plmn_config_" + operatorNumeric + ".xml";
                readConfigFromAsset(config, fileName, id);
            }
        }
        else {
            // SIM is not LOADED or ABSENT
            Log.w(TAG, "invalid phoneId.");
        }

        return config;
    }

    void readConfigFromAsset(PersistableBundle config, String fileName, CarrierIdentifier id) {
        Log.d(TAG, "loading from " + fileName);

        try {
            synchronized (this) {
                if (mFactory == null) {
                    mFactory = XmlPullParserFactory.newInstance();
                }
            }

            XmlPullParser parser = mFactory.newPullParser();
            parser.setInput(getApplicationContext().getAssets().open(fileName), "utf-8");
            printAssets(parser);
            parser.setInput(getApplicationContext().getAssets().open(fileName), "utf-8");
            PersistableBundle newConfig  = readConfigFromXml(parser, id);
            config.putAll(newConfig);
        }
        catch (IOException | XmlPullParserException e) {
            Log.d(TAG, e.toString());
        }
        catch (Exception e) {
            Log.d(TAG, e.toString());
        }
    }

    static void printAssets(XmlPullParser parser) throws IOException, XmlPullParserException {
        if (parser == null) {
            Log.d(TAG, "parser is null");
        }
        int event;
        while (((event = parser.next()) != XmlPullParser.END_DOCUMENT)) {
            if ("boolean".equals(parser.getName())) {
                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < parser.getAttributeCount(); ++i) {
                    String attribute = parser.getAttributeName(i);
                    String value = parser.getAttributeValue(i);
                    sb.append(attribute + "=" + value + " ");
                }
                Log.d(TAG, sb.toString());
            }
        }
    }


    /**
     * Parses an XML document and returns a PersistableBundle.
     */
    static PersistableBundle readConfigFromXml(XmlPullParser parser, CarrierIdentifier id)
            throws IOException, XmlPullParserException {
        PersistableBundle config = new PersistableBundle();

        if (parser == null) {
          return config;
        }

        // Iterate over each <carrier_config> node in the document and add it to the returned
        // bundle if its filters match.
        int event;
        while (((event = parser.next()) != XmlPullParser.END_DOCUMENT)) {
            if (event == XmlPullParser.START_TAG && "carrier_config".equals(parser.getName())) {
                // Skip this fragment if it has filters that don't match.
                if (!checkFilters(parser, id)) {
                    continue;
                }
                PersistableBundle configFragment = PersistableBundle.restoreFromXml(parser);
                config.putAll(configFragment);
            }
        }

        return config;
    }

    /**
     * Checks to see if an XML node matches carrier filters.
     */
    static boolean checkFilters(XmlPullParser parser, CarrierIdentifier id) {
        boolean result = true;
        for (int i = 0; i < parser.getAttributeCount(); ++i) {
            String attribute = parser.getAttributeName(i);
            String value = parser.getAttributeValue(i);
            switch (attribute) {
                case "mcc":
                    result = result && value.equals(id.getMcc());
                    break;
                case "mnc":
                    result = result && value.equals(id.getMnc());
                    break;
                case "gid1":
                    result = result && value.equalsIgnoreCase(id.getGid1());
                    break;
                case "gid2":
                    result = result && value.equalsIgnoreCase(id.getGid2());
                    break;
                case "spn":
                    result = result && matchOnSP(value, id);
                    break;
                case "imsi":
                    result = result && matchOnImsi(value, id);
                    break;
                case "device":
                    result = result && value.equals(Build.DEVICE);
                    break;
                default:
                    Log.e(TAG, "Unknown attribute " + attribute + "=" + value);
                    result = false;
                    break;
            }
        }
        return result;
    }

    /**
     * Check to see if the IMSI expression from the XML matches the IMSI of the
     * Carrier.
     *
     * @param xmlImsi IMSI expression fetched from the resource XML
     * @param id Id of the evaluated CarrierIdentifier
     * @return true if the XML IMSI matches the IMSI of CarrierIdentifier, false
     *         otherwise.
     */
    static boolean matchOnImsi(String xmlImsi, CarrierIdentifier id) {
        boolean matchFound = false;

        String currentImsi = id.getImsi();
        // If we were able to retrieve current IMSI, see if it matches.
        if (currentImsi != null) {
            Pattern imsiPattern = Pattern.compile(xmlImsi, Pattern.CASE_INSENSITIVE);
            Matcher matcher = imsiPattern.matcher(currentImsi);
            matchFound = matcher.matches();
        }
        return matchFound;
    }

    /**
     * Check to see if the service provider name expression from the XML matches the
     * CarrierIdentifier.
     *
     * @param xmlSP SP expression fetched from the resource XML
     * @param id Id of the evaluated CarrierIdentifier
     * @return true if the XML SP matches the phone's SP, false otherwise.
     */
    static boolean matchOnSP(String xmlSP, CarrierIdentifier id) {
        boolean matchFound = false;

        String currentSP = id.getSpn();
        if (SPN_EMPTY_MATCH.equalsIgnoreCase(xmlSP)) {
            if (TextUtils.isEmpty(currentSP)) {
                matchFound = true;
            }
        } else if (currentSP != null) {
            Pattern spPattern = Pattern.compile(xmlSP, Pattern.CASE_INSENSITIVE);
            Matcher matcher = spPattern.matcher(currentSP);
            matchFound = matcher.matches();
        }
        return matchFound;
    }
}
