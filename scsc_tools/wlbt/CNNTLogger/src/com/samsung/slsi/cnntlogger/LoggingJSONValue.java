package com.samsung.slsi.cnntlogger;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

class LoggingJSONValue {

    static final String NAME_KEY = "name";

    static final String EXEC_KEY = "exec";

    static final String DIR_KEY = "dir";

    static final String DATA_KEY = "data";

    private JSONArray mJSonArray;

    LoggingJSONValue() {

        String jsonValue = "[{\"name\":\"wifilog\", \"exec\":\"start\", \"option\":\"udilog\", \"dir\":\"\"},"
                + "{\"name\":\"wifilog\", \"exec\":\"start\", \"option\":\"mxlog\", \"dir\":\"\"},"
                + "{\"name\":\"wifilog\", \"exec\":\"start\", \"option\":\"all\", \"dir\":\"\"},"
                + "{\"name\":\"wifilog\", \"exec\":\"stop\", \"option\":\"udilog\", \"dir\":\"\"},"
                + "{\"name\":\"wifilog\", \"exec\":\"stop\", \"option\":\"mxlog\", \"dir\":\"\"},"
                + "{\"name\":\"wifilog\", \"exec\":\"stop\", \"option\":\"all\", \"dir\":\"\"},"

                + "{\"name\":\"btlog\", \"exec\":\"start\", \"option\":\"general\", \"dir\":\"\"},"
                + "{\"name\":\"btlog\", \"exec\":\"start\", \"option\":\"audio\", \"dir\":\"\"},"
                + "{\"name\":\"btlog\", \"exec\":\"start\", \"option\":\"custom\", \"dir\":\"\", \"data\":\"\"},"
                + "{\"name\":\"btlog\", \"exec\":\"stop\", \"option\":\"\", \"dir\":\"\"}]";

        try {
            mJSonArray = new JSONArray(jsonValue);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    JSONObject getJSonValue(int num, String filepath, String data) {

        JSONObject json = null;
        try {
            json = mJSonArray.getJSONObject(num);
            json.put(DIR_KEY, filepath);
            if (data != null) {
                json.put(DATA_KEY, data);
            }

        } catch(JSONException e) {
            e.printStackTrace();
        }

        return json;
    }
}
