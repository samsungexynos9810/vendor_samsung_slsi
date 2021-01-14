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

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.Arrays;

public class DataWriter {
    static final int RIL_MAX_COMMAND_BYTES = (4 * 1024);

    private static final String TAG = "DataWriter";

    private int mSize = 0;
    private int mCapacity = 0;
    private DataOutputStream mOut;
    private ByteArrayOutputStream mByteArray;

    public DataWriter() {
        this(RIL_MAX_COMMAND_BYTES);
    }

    public DataWriter(int capacity) {
        mCapacity = capacity;
        reset();
    }

    public void reset() {
        mSize = 0;

        if (mOut != null) {
            try {
                mOut.close();
            } catch (IOException e) {
            }
        }

        mByteArray = new ByteArrayOutputStream(mCapacity);
        mOut = new DataOutputStream(mByteArray);
    }

    public int size() {
        return mSize;
    }

    public byte[] toByteArray() {
        return mByteArray.toByteArray();
    }

    public void writeInt(int i) throws IOException {
        byte[] ar = new byte[] { (byte) (i & 0xFF), (byte) (i >> 8 & 0xFF), (byte) (i >> 16 & 0xFF), (byte) (i >> 24 & 0xFF) };
        mOut.write(ar);
    }

    public void writeShort(short s) throws IOException {
        byte[] ar = new byte[] { (byte) (s & 0xFF), (byte) (s >> 8 & 0xFF) };
        mOut.write(ar);
    }

    public void writeLong(long l) throws IOException {
        byte[] ar = new byte[] { (byte) (l & 0xFF), (byte) (l >> 8 & 0xFF), (byte) (l >> 16 & 0xFF), (byte) (l >> 24 & 0xFF),
                                 (byte) (l >> 32 & 0xFF), (byte) (l >> 40 & 0xFF), (byte) (l >> 48 & 0xFF), (byte) (l >> 56 & 0xFF), };
        mOut.write(ar);
    }

    public void writeLongs(long[] l) throws IOException {
        for (int i = 0; i < l.length; i++) {
            writeLong(l[i]);
        }
    }

    public void writeByte(byte b) throws IOException {
        mOut.write(b);
    }

    public void writeBytes(byte[] b) throws IOException {
        if (b != null) {
            mOut.write(b);
        }
    }

    public void writeBytes(byte[] b, int offset, int len) throws IOException {
        if (b != null) {
            mOut.write(b, offset, len);
        }
    }

    public void fillBytes(int len) throws IOException {
        if (len > 0) {
            byte[] ar = new byte[len];
            Arrays.fill(ar, (byte)0);
            mOut.write(ar);
        }
    }
}
