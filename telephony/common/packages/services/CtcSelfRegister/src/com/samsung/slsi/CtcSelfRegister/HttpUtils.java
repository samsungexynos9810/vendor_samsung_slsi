/*
 * Copyright (c) 2017. Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

package com.samsung.slsi.CtcSelfRegister;

import android.util.Log;


import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;


public class HttpUtils {
    private static final String LOG_TAG = "SELF_REG_HTTP";
    private static final String SERVER_URL = "http://zzhc.vnet.cn";
    private static final int HTTP_TIMEOUT = 5000; // 5 seconds

    public static JSONObject httpSend (String data) {
        HttpURLConnection httpConn;
        URL url;

        JSONObject result = null;

        try {
            url = new URL(SERVER_URL);
            httpConn = (HttpURLConnection) url.openConnection();

            try {
                httpConn.setConnectTimeout(HTTP_TIMEOUT);
                httpConn.setUseCaches(false);

                httpConn.setRequestMethod("POST");

                httpConn.setDoInput(true);
                httpConn.setDoOutput(true);
                httpConn.setRequestProperty("Content-Type", "application/encrypted-json");

                OutputStream os = httpConn.getOutputStream();
                os.write(data.getBytes());
                os.flush();
                os.close();

                int respCode = httpConn.getResponseCode();
                if (respCode != HttpURLConnection.HTTP_OK) {
                    Log.e(LOG_TAG, "Http URL Connection send failed response code (" + respCode + ")");
                }

                InputStream is = httpConn.getInputStream();
                if (is != null) {
                    Log.v(LOG_TAG, "Input stream (response from the server) " + is.toString());

                    BufferedReader reader = new BufferedReader (new InputStreamReader(is));
                    StringBuilder sb = new StringBuilder();

                    String line = null;
                    try {
                        while ((line = reader.readLine()) != null) {
                            sb.append(line + "\n");
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    } finally {
                        try {
                            is.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                    Log.v(LOG_TAG, "String built (response from the server) " + sb.toString());
                    result = new JSONObject(sb.toString());
                    Log.v(LOG_TAG, "JSONObject result :" +result.toString());
                }
            } catch (IOException ex) {
                Log.e(LOG_TAG, "Http Send - IOException");
                ex.printStackTrace();
            } catch (JSONException ex) {
                Log.e(LOG_TAG, "JSON Object exception");
                ex.printStackTrace();
            } finally {
                httpConn.disconnect();
            }
        } catch (MalformedURLException e) {
            Log.e(LOG_TAG, "Malformed URL Exception, caused by new URL ("+ SERVER_URL +")");
            e.printStackTrace();
        } catch (IOException ex) {
            Log.e (LOG_TAG, "IO Exception caused by URL.openConnection()");
            ex.printStackTrace();
        }
        return result;


    }
}
