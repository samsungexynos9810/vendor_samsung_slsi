/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#ifndef __ICC_UTIL_H__
#define __ICC_UTIL_H__

#include "rildef.h"
#include "types.h"

class SimTlv {

private:
    BYTE *mRecord;
    int mTlvLength;
    int mCurOffset;
    int mCurDataOffset;
    int mCurDataLength;
    bool mHasValidTlvObject;

public:
    SimTlv();
    SimTlv(BYTE *record, int offset, int length);
    virtual ~SimTlv();

    bool nextObject();
    bool isValidObject() { return mHasValidTlvObject; }
    int getTag() {
        if (!mHasValidTlvObject) return 0;
        return mRecord[mCurOffset] & 0xff;
    }

    BYTE *getData();
    int getDataLength() { return mCurDataLength; }
    bool parseCurrentTlvObject();

protected:

};

class IccUtil {
private:
    static const char HEX_CHARS[16];

public:
    static int hexCharToInt(char ch);
    static String bytesToHexString(BYTE *data, int length);
    static String bcdPlmnToString(BYTE *data, int offset, int length);
    static int bytesToInt(BYTE *src, int srcLength, int offset, int length);
    static String networkNameToString(BYTE *data, int offset, int length);
};

#endif
