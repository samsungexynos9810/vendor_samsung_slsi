/**
 * @file    bitwiseiostream.h
 *
 * @brief   The copy of BitwiseInputStream.java
 *          in frameworks/base/core/java/com/android/internal/util
 *
 * @author  Taesu Lee (taesu82.lee@samsung.com)
 *
 * @version Unspecipied.
 */

#ifndef __BITWISE_IO_STREAM_H__
#define __BITWISE_IO_STREAM_H__

/**
 * Bitwise Input Stream Class
 */
#include "types.h"

class CBitwiseInputStream
{
    private:
        BYTE *m_pbBuf;
        UINT16 m_uPos;
        UINT16 m_uEnd;

    public:
        CBitwiseInputStream(BYTE *buf, UINT8 len);
        virtual ~CBitwiseInputStream() {};

        BYTE Read(UINT8 bits);
};

class CBitwiseOutputStream
{
    private:
        BYTE *m_pbBuf;
        UINT16 m_uPos;
        UINT16 m_uEnd;

    public:
        CBitwiseOutputStream(BYTE *buf, UINT8 len);
        virtual ~CBitwiseOutputStream() {};

        void Write(UINT8 bits, BYTE data);
        UINT8 GetArrayIndex() { return m_uPos >> 3; };
};
#endif /*__BITWISE_IO_STREAM_H__*/
