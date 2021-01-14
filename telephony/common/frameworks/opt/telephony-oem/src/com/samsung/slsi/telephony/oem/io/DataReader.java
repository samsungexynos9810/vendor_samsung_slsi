/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.oem.io;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;

import android.util.Log;

import com.samsung.slsi.telephony.oem.util.StringUtil;

public class DataReader {

    private static final String TAG = "DataReader";
    private ByteArrayInputStream mByteArray;
    private DataInputStream mIn;

    public DataReader(byte[] b) {
        this(b, 0, b.length);
    }

    public DataReader(byte[] b, int offset, int len) {
        mByteArray = new ByteArrayInputStream(b, 0, len);
        mIn = new DataInputStream(mByteArray);
    }

    public int getInt() throws IOException {
        byte[] buf = new byte[4];
        mIn.read(buf, 0, 4);
        return (buf[0] & 0xFF | ((buf[1] & 0xFF) << 8) | ((buf[2] & 0xFF) << 16) | ((buf[3] & 0xFF) << 24));
    }

    public short getShort() throws IOException {
        byte[] buf = new byte[2];
        mIn.read(buf, 0, 2);

        return (short)(buf[0] & 0xFF | (buf[1] & 0xFF) << 8);
    }

    public long getLong() throws IOException {
        byte[] buf = new byte[8];
        mIn.read(buf, 0, 8);

        Log.d(TAG, "getLong=" + StringUtil.bytesToHexString(buf));
        return
                ((long)(buf[7] & 0xff) << 56) |
                ((long)(buf[6] & 0xff) << 48) |
                ((long)(buf[5] & 0xff) << 40) |
                ((long)(buf[4] & 0xff) << 32) |
                ((long)(buf[3] & 0xff) << 24) |
                ((long)(buf[2] & 0xff) << 16) |
                ((long)(buf[1] & 0xff) << 8) |
                (buf[0] & 0xff);
    }

    public byte getByte() throws IOException {
        byte b = mIn.readByte();
        Log.d(TAG, "getByte=" + StringUtil.bytesToHexString(new byte[] {b}));
        return b;
    }

    public byte[] getBytes(int len) throws IOException {
        byte[] buf = new byte[len];
        mIn.read(buf, 0, len);
        Log.d(TAG, "getBytes=" + StringUtil.bytesToHexString(buf));
        return buf;
    }

    public long[] getLongs(int len) throws IOException {
        long[] buf = new long[len];
        for (int i = 0; i < len; i++) {
            buf[i] = getLong();
        }
        return buf;
    }
}
