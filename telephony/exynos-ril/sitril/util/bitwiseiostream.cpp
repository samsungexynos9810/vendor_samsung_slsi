/**
 * @file    bitwiseiostream.cpp
 *
 * @brief   The copy of BitwiseInputStream.java
 *          in frameworks/base/core/java/com/android/internal/util
 *
 * @author  Taesu Lee (taesu82.lee@samsung.com)
 *
 * @version Unspecipied.
 */

#include "bitwiseiostream.h"

CBitwiseInputStream::CBitwiseInputStream(BYTE *buf, UINT8 len)
    :m_pbBuf(NULL), m_uPos(0), m_uEnd(0)
{
    if (buf != NULL && len != 0) {
        m_pbBuf = buf;
        m_uEnd = len << 3;
    }
}

BYTE CBitwiseInputStream::Read(UINT8 bits)
{
    UINT16 index = m_uPos >> 3;
    UINT16 offset = 16 - (m_uPos & 0x07) - bits;
    if ((bits > 8) || ((m_uPos + bits) > m_uEnd)) {
        return 0;   // Return 0 for invalid access.
    }
    UINT16 data = (m_pbBuf[index] & 0xFF) << 8;
    if (offset < 8) {
        data |= m_pbBuf[index + 1] & 0xFF;
    }
    data >>= offset;
    data &= (0XFFFF >> (16 - bits));
    m_uPos += bits;
    return (BYTE)(data & 0x00FF);
}

CBitwiseOutputStream::CBitwiseOutputStream(BYTE *buf, UINT8 len)
    :m_pbBuf(NULL), m_uPos(0), m_uEnd(0)
{
    if (buf != NULL && len !=0) {
        m_pbBuf = buf;
        m_uEnd = len << 3;
    }
}

void CBitwiseOutputStream::Write(UINT8 bits, BYTE data)
{
    UINT16 temp = (UINT16)data;
    if ((bits > 8) || ((m_uPos + bits) > m_uEnd)) {
        return;
    }
    temp &= (0xFFFF >> (16 - bits));
    UINT16 index = m_uPos >> 3;
    UINT16 offset = 16 - (m_uPos & 0x07) - bits;
    temp <<= offset;
    m_uPos += bits;
    m_pbBuf[index] |= temp >> 8;
    if (offset < 8) {
        m_pbBuf[index + 1] |= temp & 0xFF;
    }
}
