/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.oem;

import java.util.Arrays;

import android.os.Parcel;
import android.os.Parcelable;

public class OemRilResult implements Parcelable {
    private int mType = OemRilConstants.UNSOLICITED;
    private int mChannel;
    private int mId;
    private int mSerial;
    private int mStatus;
    private byte[] mData;

    OemRilResult(int channel, int id, int serial, int status, byte[] data, int dataLen) {
        mChannel = channel;
        mId = id;
        mSerial = serial;
        mStatus = status;
        if (serial != OemRilConstants.RILC_TRANSACTION_NONE) {
            mType = OemRilConstants.SOLICITED;
        }

        if (data != null) {
            mData = Arrays.copyOf(data, dataLen);
        }
    }

    OemRilResult(byte[] data, int dataLen) {

    }

    public int getType() { return mType; }
    public int getChannel() { return mChannel; }
    public int getId() { return mId; }
    public int getSerial() { return mSerial; }
    public int getStatus() { return mStatus; }
    public byte[] getData() { return mData; }
    public int getDataLength() { return (mData != null) ? mData.length : 0 ; }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("OemRilResult{");
        sb.append(" type=" + ((mType == OemRilConstants.SOLICITED) ? "SOLICITED" : "UNSOLICITED"));
        sb.append(" transaction=" + mSerial);
        sb.append(" id=" + mId);
        sb.append(" status=" + mStatus);
        sb.append(" dataLength=" + ((mData != null) ? mData.length : 0));
        sb.append(" }");
        return sb.toString();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mChannel);
        dest.writeInt(mSerial);
        dest.writeInt(mId);
        dest.writeInt(mStatus);
        if (mData != null && mData.length > 0) {
            dest.writeInt(mData.length);
            dest.writeByteArray(mData);
        }
        else {
            dest.writeInt(0);
        }
    }

    public static final Creator<OemRilResult> CREATOR = new Creator<OemRilResult>() {
        @Override
        public OemRilResult createFromParcel(Parcel in) {
            int channel = in.readInt();
            int serial = in.readInt();
            int id = in.readInt();
            int status = in.readInt();
            int dataLen = in.readInt();
            byte[] data = null;
            if (dataLen > 0) {
                data = new byte[dataLen];
                in.readByteArray(data);
            }

            return new OemRilResult(channel, id, serial, status, data, dataLen);
        }

        @Override
        public OemRilResult[] newArray(int size) {
            return new OemRilResult[size];
        }
    };
}
