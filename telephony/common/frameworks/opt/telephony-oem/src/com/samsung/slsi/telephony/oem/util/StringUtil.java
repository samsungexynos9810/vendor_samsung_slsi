/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.oem.util;

import java.util.ArrayList;
import java.util.List;

import android.os.Build;

public class StringUtil {
    private static final boolean VDBG = !Build.TYPE.equals("user");

    public static String bytesToHexString(byte[] data) {
        if (data != null) {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < data.length; ++i) {
                sb.append(String.format("%02X ", data[i]));
                if (i!=0 && (i%16 == 15)) {
                    sb.append("\n");
                }
            }

            return sb.toString();
        }

        return "";
    }

    public static String bytesToHexString(byte[] data, int dataLen) {
        if (data != null && dataLen <= data.length) {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < dataLen; i++) {
                sb.append(String.format("%02X ", data[i]));
                if (i != 0 && (i%16 == 15)) {
                    sb.append("\n");
                }
            }

            return sb.toString();
        }

        return "";
    }

    public static String byteArrayListToHexString(List<Byte> data) {
        if (data != null) {
            byte[] tmp = new byte[data.size()];
            int i = 0;
            for (Byte b : data) {
                tmp[i++] = b.byteValue();
            }

            return bytesToHexString(tmp);
        }
        return "";
    }

    public static ArrayList<Byte> primitiveArrayToArrayList(byte[] arr) {
        ArrayList<Byte> arrayList = new ArrayList<>(arr.length);
        for (byte b : arr) {
            arrayList.add(b);
        }
        return arrayList;
    }

    public static byte[] arrayListToPrimitiveArray(ArrayList<Byte> bytes) {
        if (bytes == null) {
            return null;
        }
        byte[] ret = new byte[bytes.size()];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = bytes.get(i);
        }
        return ret;
    }

    public static String retToString(int req, Object ret) {
        if (!VDBG) {
            return "";
        }

        if (ret == null) return "";

        StringBuilder sb;
        String s;
        int length;
        if (ret instanceof int[]) {
            int[] intArray = (int[]) ret;
            length = intArray.length;
            sb = new StringBuilder("{");
            if (length > 0) {
                int i = 0;
                sb.append(intArray[i++]);
                while (i < length) {
                    sb.append(", ").append(intArray[i++]);
                }
            }
            sb.append("}");
            s = sb.toString();
        } else if (ret instanceof String[]) {
            String[] strings = (String[]) ret;
            length = strings.length;
            sb = new StringBuilder("{");
            if (length > 0) {
                int i = 0;
                sb.append(strings[i++]);
                while (i < length) {
                    sb.append(", ").append(strings[i++]);
                }
            }
            sb.append("}");
            s = sb.toString();
        } else if (ret instanceof byte[]) {
            sb = new StringBuilder();
            if (((byte[])ret).length > 8) {
                sb.append("\n");
            }
            sb.append(StringUtil.bytesToHexString((byte[])ret));
            s = sb.toString();
        }
        else {
            s = ret.toString();
        }
        return s;
    }
}
