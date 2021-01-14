/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "util.h"
#include "iccUtil.h"
#include "rillog.h"

#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/*
 * SimTlv
 */
SimTlv::SimTlv() {
    mRecord = NULL;
    mTlvLength = 0;
    mCurOffset = 0;
    mCurDataOffset = 0;
    mCurDataLength = 0;
    mHasValidTlvObject = false;
}

SimTlv::SimTlv(BYTE *record, int offset, int length) : SimTlv() {
    if(record != NULL && length > 0)
    {
        mRecord = new BYTE[length];
        memcpy(mRecord, record+offset, length);

        mTlvLength = length;
        mHasValidTlvObject = parseCurrentTlvObject();
    }
}

SimTlv::~SimTlv() {
    if(mRecord != NULL) delete[] mRecord;
}

bool SimTlv::nextObject() {
    if (!mHasValidTlvObject) return false;
    mCurOffset = mCurDataOffset + mCurDataLength;
    mHasValidTlvObject = parseCurrentTlvObject();
    return mHasValidTlvObject;
}

/**
 * Returns data associated with current TLV object
 * returns null if !isValidObject()
 */
BYTE *SimTlv::getData() {
    if (!mHasValidTlvObject) return NULL;
    if (mCurDataLength <= 0) return NULL;

    BYTE *ret = new BYTE[mCurDataLength];
    memcpy(ret, mRecord + mCurDataOffset, mCurDataLength);
    return ret;
}

/**
 * Updates curDataLength and curDataOffset
 * @return false on invalid record, true on valid record
*/
bool SimTlv::parseCurrentTlvObject() {

    if (mCurOffset >= mTlvLength) return false;

    // 0x00 and 0xff are invalid tag values
    if (mRecord[mCurOffset] == 0 || (mRecord[mCurOffset] & 0xff) == 0xff) {
        return false;
    }

    if (mCurOffset + 1 < mTlvLength) {
        if ((mRecord[mCurOffset + 1] & 0xff) < 0x80) {
            // one byte length 0 - 0x7f
            mCurDataLength = mRecord[mCurOffset + 1] & 0xff;
            mCurDataOffset = mCurOffset + 2;
        } else if ((mRecord[mCurOffset + 1] & 0xff) == 0x81) {
            // two byte length 0x80 - 0xff
            if ( mCurOffset + 2 <  mTlvLength) {
                mCurDataLength = mRecord[mCurOffset + 2] & 0xff;
                mCurDataOffset = mCurOffset + 3;
            }
            else {
                return false;
            }
        } else {
            return false;
        }
    }
    else {
        return false;
    }

    if (mCurDataLength + mCurDataOffset > mTlvLength) {
        return false;
    }

    return true;
}


/*
 * IccUtil
 */
const char IccUtil::HEX_CHARS[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

int IccUtil::hexCharToInt(char ch) {
    if (ch >= '0' && ch <= '9')
        return (int)(ch - '0');
    else if (ch >= 'a' && ch <= 'f')
        return (int)(ch - 'a') + 10;
    else if (ch >= 'A' && ch <= 'F')
        return (int)(ch - 'A') + 10;
    return -1;
}

/**
 * Converts a byte array into a String of hexadecimal characters.
 *
 * @param bytes an array of bytes
 *
 * @return hex string representation of bytes array
 */
String IccUtil::bytesToHexString(BYTE *data, int length) {
    if (data == NULL) return NULL;

    char *buf = new char[length*2];

    int j = 0;
    for (int i = 0 ; j < MAX_PLMN_LEN && i < length ; i++) {
        int b;
        b = 0x0f & (data[i] >> 4);
        buf[j++] = HEX_CHARS[b];

        b = 0x0f & data[i];
        buf[j++] = HEX_CHARS[b];
    }
    String ret(buf, j);

    delete [] buf;
    return ret;
}

/**
 * PLMN (MCC/MNC) is encoded as per 24.008 10.5.1.3
 * Returns a concatenated string of MCC+MNC, stripping
 * all invalid character 'F'
 */
String IccUtil::bcdPlmnToString(BYTE *data, int offset, int length) {
    if (offset + 3 > length) {
        return NULL;
    }
    BYTE *trans = new BYTE[3];
    trans[0] = (BYTE) ((data[0 + offset] << 4) | ((data[0 + offset] >> 4) & 0xF));
    trans[1] = (BYTE) ((data[1 + offset] << 4) | (data[2 + offset] & 0xF));
    trans[2] = (BYTE) ((data[2 + offset] & 0xF0) | ((data[1 + offset] >> 4) & 0xF));
    String ret = bytesToHexString(trans, 3);

    // For a valid plmn we trim all character 'F'
    for (int i = ret.size() - 1; i >= 0; --i) {
        if(ret.at(i) == 'F') ret.erase(i);
        else break;
    }

    delete [] trans;
    return ret;
}

/**
 * Converts a series of bytes to an integer. This method currently only supports positive 32-bit
 * integers.
 *
 * @param src The source bytes.
 * @param offset The position of the first byte of the data to be converted. The data is base
 *     256 with the most significant digit first.
 * @param length The length of the data to be converted. It must be <= 4.
 * @throws IllegalArgumentException If {@code length} is bigger than 4 or {@code src} cannot be
 *     parsed as a positive integer.
 * @throws IndexOutOfBoundsException If the range defined by {@code offset} and {@code length}
 *     exceeds the bounds of {@code src}.
 */
int IccUtil::bytesToInt(BYTE *src, int srcLength, int offset, int length) {
    if (length > 4) {
        // length must be <= 4 (only 32-bit integer supported):
        return 0;
    }

    if (offset < 0 || length < 0 || offset + length > srcLength) {
        //Out of the bounds
        return 0;
    }
    int result = 0;
    for (int i = 0; i < length; i++) {
        result = (result << 8) | (src[offset + i] & 0xFF);
    }
    if (result < 0) {
        //src cannot be parsed as a positive integer
        return 0;
    }
    return result;
}

/**
 * Convert a TS 24.008 Section 10.5.3.5a Network Name field to a string
 * "offset" points to "octet 3", the coding scheme byte
 * empty string returned on decode error
 */
String IccUtil::networkNameToString(BYTE *data, int offset, int length) {
    String ret = String("");
    char dstBuff[MAX_FULL_NAME_LEN] = { 0, };
    int decLen = 0;

    if ((data[offset] & 0x80) != 0x80 || length < 1) {
        return ret;
    }

    int countSeptets;
    int unusedBits =  data[offset] & 7;
    countSeptets = (((length - 1) * 8) - unusedBits) / 7 ;

    switch ((data[offset] >> 4) & 0x7) {
        case 0:
            decLen = DecodingNetworkNameFromSIM(0, data+offset+1, length-1, countSeptets, (unsigned char*)dstBuff, sizeof(dstBuff));
            if (decLen > 0) ret = String(dstBuff, decLen);
        break;
        case 1:
            decLen = DecodingNetworkNameFromSIM(1, data+offset+1, length-1, countSeptets, (unsigned char*)dstBuff, sizeof(dstBuff));
            if (decLen > 0) ret = String(dstBuff, decLen);
        break;

        // unsupported encoding
        default:
        break;
    }

    // "Add CI"
    // "The MS should add the letters for the Country's Initials and
    //  a separator (e.g. a space) to the text string"

    if ((data[offset] & 0x40) != 0) {
        // FIXME(mkf) add country initials here
    }

    return ret;
}
