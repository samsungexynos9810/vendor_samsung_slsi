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
#include "rillog.h"
#include "rilproperty.h"
#include <map>
using namespace std;

/**
 * Utility function:
 * IMPORTANT: DO NOT call this function directly. Direct call would cause
 * memory leak. Must call via method of a RilData-child class.
 */
const char HEX_CHARS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


void ConvertToHexString(char **hex, char *data, int len)
{
    int n = 0;
    size_t buflen = (len * 2);

    *hex = new char[buflen + 1];
    if (*hex == NULL)
    {
        return;
    }

    memset(*hex, 0, len * 2 + 1);

    for (int i = 0; i < len; i++)
    {
        n += snprintf(*hex + n, buflen - n , "%02X", data[i]);
    }
}

UINT8 HexCharToInt(char c)
{
    if (c >= '0' && c <= '9')
    {
        return (c - '0');
    }
    else if (c >= 'A' && c <= 'F')
    {
        return (c - 'A' + 10);
    }
    else if (c >= 'a' && c <= 'f')
    {
        return (c - 'a' + 10);
    }
    else
    {
        return 0;
    }
}

void ConvertToRaw(char *hexStr, char *data, int *len)
{
    int hexLen = strlen(hexStr);

    for (int i = 0; i < hexLen; i += 2)
    {
        data[i/2] = (char)((::HexCharToInt(hexStr[i]) << 4) | ::HexCharToInt(hexStr[i + 1]));
    }

    *len = hexLen / 2;
}

int ConvertToRaw(char *hexStr, char *buf, int bufSize)
{
    if (TextUtils::IsEmpty(hexStr)) {
        return 0;
    }

    if (buf == NULL || bufSize <= 0) {
        return 0;
    }

    int hexLen = strlen(hexStr);
    if (hexLen % 2 != 0 || (hexLen / 2) > bufSize) {
        return 0;
    }

    for (int i = 0; i < hexLen; i += 2) {
        buf[i/2] = (char)((::HexCharToInt(hexStr[i]) << 4) | ::HexCharToInt(hexStr[i + 1]));
    } // end for i ~

    return hexLen / 2;
}


string GetSimOperatorNum(int phoneId)
{
    string numeric = GetSimOperatorNumericGsm(phoneId);
    //RilLogV("GetSimOperatorNum[%d] %s", phoneId, numeric.c_str());
    return numeric;
}

string GetSimOperatorNumericGsm(int phoneId)
{
    string prop = SystemProperty::Get(PROPERTY_ICC_OPERATOR_NUMERIC);
    string propVal = "";
    if (phoneId >= 0 && !TextUtils::IsEmpty(prop)) {
        string values[2];
        size_t pos = prop.find(',');
        if (pos != string::npos) {
            values[0] = prop.substr(0, pos);
            values[1] = prop.substr(pos + 1);
        }
        else {
            values[0] = prop;
            values[1] = "";
        }
        RilLogV("%s [0]=%s [1]=%s", PROPERTY_ICC_OPERATOR_NUMERIC, values[0].c_str(), values[1].c_str());

        if (phoneId == RIL_SOCKET_1
#if (SIM_COUNT >= 2)
        || phoneId == RIL_SOCKET_2
#endif
        ) {
            propVal = values[phoneId];
        }
    }
    return propVal;
}

string GetSimOperatorNumericCdma(int phoneId)
{
    return string("");
}

string GetSimSpn(int phoneId)
{
    string prop = SystemProperty::Get(PROPERTY_ICC_OPERATOR_ALPHA);
    string propVal = "";
    if (phoneId >= 0 && !TextUtils::IsEmpty(prop)) {
        string values[2];
        size_t pos = prop.find(',');
        if (pos != string::npos) {
            values[0] = prop.substr(0, pos);
            values[1] = prop.substr(pos + 1);
        }
        else {
            values[0] = prop;
            values[1] = "";
        }
        RilLogV("%s [0]=%s [1]=%s", PROPERTY_ICC_OPERATOR_ALPHA, values[0].c_str(), values[1].c_str());

        if (phoneId == RIL_SOCKET_1
#if (SIM_COUNT >= 2)
        || phoneId == RIL_SOCKET_2
#endif
        ) {
            propVal = values[phoneId];
        }
    }
    return propVal;
}

string bcdToString(const BYTE *src,int srcLen){

    char *buffer = (char *)calloc((2*srcLen+1),sizeof(char) );
    if (buffer == NULL) {
        string temp = "";
        return temp;
    }
    int j=0;
    for(int i=0;i<srcLen;i++){
        int v,w;
        v = src[i] & 0xf;
        if (v > 9){
            break;
        }
        buffer[j++] = (char)('0'+v);

        w = (src[i] >> 4) & 0xf;
        if (w == 0xf) continue;
        if (w > 9){
            break;
        }
        buffer[j++] = (char)('0'+w);
    }
    buffer[j]=0;
    string temp = buffer;
    free(buffer);
    return temp;
}

string bchToString(const BYTE *src,int srcLen){

    char *buffer = (char *)calloc((2*srcLen+1),sizeof(char));
    int j=0;
    for(int i=0;i<srcLen;i++){
       int v,w;
       v = src[i] & 0xf;
       buffer[j++] = HEX_CHARS[v];

       w = (src[i] >> 4) & 0xf;
       buffer[j++] = HEX_CHARS[w];
    }
    buffer[j]=0;
    string temp = buffer;
    free(buffer);
    return temp;
}

bool IsPlmnMatchedSimPlmn(const char *carrier, int phoneId)
{
    string simPlmn = GetSimOperatorNum(phoneId);
    return strcmp(carrier, simPlmn.c_str()) == 0;
}

// MOX
void PrintBufferDump(const char *pszLabel, const BYTE *pBuffer, int nLength)
{
    static const int nCountPerGroup = 8;
    static const int nCountPerLine = nCountPerGroup * 2;

    char szDumpString[128];
    char szAscii[nCountPerLine+2];
    memset(szDumpString, 0, sizeof(szDumpString));
    memset(szAscii, 0, sizeof(szAscii));
    if(pBuffer && nLength>0)
    {
        int nLineCount = 0;
        int nAsciiIdx = 0;
        for(int i=0; i<((nLength/nCountPerLine)+1)*nCountPerLine; i++, nAsciiIdx++)
        {
            char szChHex[4];
            memset(szChHex, 0, 4);

            char ch = ' ';
            if(i<nLength)
            {
                snprintf(szChHex, 4, "%02X ", pBuffer[i]);
                ch = pBuffer[i];
            }
            else snprintf(szChHex, 4, "   ");

            strncat(szDumpString, szChHex, 4);
            if(ch>=32 && ch<=126) szAscii[nAsciiIdx] = ch;
            else szAscii[nAsciiIdx] = '.';

            if(i>0 && (i+1)%nCountPerGroup==0)
            {
                szAscii[++nAsciiIdx] = ' ';
                snprintf(szChHex, 4, "   ");
                strncat(szDumpString, szChHex, 4);

                if((i+1)%nCountPerLine==0)
                {
                    szAscii[nAsciiIdx] = '\0';
                    RilLogV("%s() [%s] %04X: %s   %s", __FUNCTION__, pszLabel, nLineCount, szDumpString, szAscii);
                    nLineCount+=nCountPerLine;
                    nAsciiIdx = -1;
                    memset(szDumpString, 0, sizeof(szDumpString));
                    memset(szAscii, 0, sizeof(szAscii));
                }
            }
        }

        szAscii[nAsciiIdx] = '\0';
        if(strlen(szDumpString)>0) RilLogV("%s() [%s] %04X: %s   %s", __FUNCTION__, pszLabel, nLineCount, szDumpString, szAscii);
    }
}

int Value2HexString(char *pszHexStrOut, const BYTE *pHexDecIn, int nLength)
{
    int nResult = -1;
    if(pszHexStrOut && pHexDecIn && nLength>0)
    {
        int nSrcOffset = 0;
        for(int i=0; i<nLength; i++)
        {
            nSrcOffset += snprintf(&pszHexStrOut[nSrcOffset], 3, "%02X", pHexDecIn[i]);
        }

        pszHexStrOut[nSrcOffset] = '\0';
        nResult = nSrcOffset;
    }

    return nResult;
}

int HexChar2Value(char ch)
{
    if (ch >= '0' && ch <= '9')
        return (int)(ch - '0');
    else if (ch >= 'a' && ch <= 'f')
        return (int)(ch - 'a') + 10;
    else if (ch >= 'A' && ch <= 'F')
        return (int)(ch - 'A') + 10;
    return -1;
}

int HexString2Value(BYTE *pHexDecOut, const char *pszHexStrIn)
{
    int nResult = 0;

    if (pszHexStrIn == NULL) {
        return 0;
    }

    int nLength = strlen(pszHexStrIn);
    if(pHexDecOut && pszHexStrIn)
    {
        int nSrcIdx, nDstIdx;
        for (nSrcIdx=0, nDstIdx=0; nSrcIdx<nLength && pszHexStrIn[nSrcIdx]!='\0'; nSrcIdx++)
        {
            // Even index is upper 4 bit
            if(nSrcIdx==0 || nSrcIdx%2==0) pHexDecOut[nDstIdx] = ((HexChar2Value(pszHexStrIn[nSrcIdx]) << 4) & 0xF0);
            else
            {
                pHexDecOut[nDstIdx] |= (HexChar2Value(pszHexStrIn[nSrcIdx]) & 0x0F);
                nDstIdx++;
            }
        }

        // Odd Length of Source Data
        if(nSrcIdx!=0 && nSrcIdx%2==1) nDstIdx++;
        nResult = nDstIdx;
    }

    return nResult;
}

/*
 * Base64
 */
char *Base64_Encode(const BYTE *pSource, int nSrcLength)
{
    static const char Base64EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static const char Base64Padding = '=';

    int nEncodeLength = (4*(nSrcLength/3)) + (nSrcLength%3? 4:0);
    char *pszEncoded = new char[nEncodeLength+1];
    memset(pszEncoded, 0, nEncodeLength+1);

    int nDstIdx = 0;
    BYTE *pSrc = (BYTE *) pSource;
    for(int i=0; i<nSrcLength-3; i+=3)
    {
        pszEncoded[nDstIdx++] = Base64EncodeTable[(pSrc[0] >> 2)];
        pszEncoded[nDstIdx++] = Base64EncodeTable[((pSrc[0] & 0x03) << 4) | (pSrc[1] >> 4)];
        pszEncoded[nDstIdx++] = Base64EncodeTable[((pSrc[1] & 0x0F) << 2) | (pSrc[2] >> 6)];
        pszEncoded[nDstIdx++] = Base64EncodeTable[pSrc[2] & 0x3F];
        pSrc += 3;
    }

    if(nSrcLength%3 == 1)
    {
        pszEncoded[nDstIdx++] = Base64EncodeTable[(pSrc[0] >> 2)];
        pszEncoded[nDstIdx++] = Base64EncodeTable[((pSrc[0] & 0x03) << 4)];
    }
    else if(nSrcLength%3 == 2)
    {
        pszEncoded[nDstIdx++] = Base64EncodeTable[(pSrc[0] >> 2)];
        pszEncoded[nDstIdx++] = Base64EncodeTable[((pSrc[0] & 0x03) << 4) | (pSrc[1] >> 4)];
        pszEncoded[nDstIdx++] = Base64EncodeTable[((pSrc[1] & 0x0F) << 2)];
    }

    while(nDstIdx<nEncodeLength) pszEncoded[nDstIdx++] = Base64Padding;
    pszEncoded[nEncodeLength] = '\0';

    return pszEncoded;
}

int Base64_Decode(BYTE **pDst, const char *pBase64)
{
    static const int Base64DecodeTable[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,        /* 00 - 0F */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,        /* 10 - 1F */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,        /* 20 - 2F */
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,        /* 30 - 3F */
        -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,        /* 40 - 4F */
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,        /* 50 - 5F */
        -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,        /* 60 - 6F */
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,        /* 70 - 7F */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,        /* 80 - 8F */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,        /* 90 - 9F */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,        /* A0 - AF */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,        /* B0 - BF */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,        /* C0 - CF */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,        /* D0 - DF */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,        /* E0 - EF */
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1        /* F0 - FF */
    };

    int nBase64Length = strlen(pBase64);
    int nLength = 3 * (nBase64Length / 4);
    int nPadding = 0;
    for(int i=nBase64Length-1; i>=0; i--) { if(pBase64[i]=='=') nPadding++; }
    nLength -= nPadding;

    *pDst = new BYTE[nLength+1];
    memset(*pDst, 0, nLength+1);

    int nDstIdx = 0;
    int nDecode=0, nPrevDecode=0;
    BYTE cCombined = 0;
    for(int i=0; i<nBase64Length; i++)
    {
        nDecode = Base64DecodeTable[(int)pBase64[i]];
        if(nDecode!=-1)
        {
            if(i!=0 && i%4>0)
            {
                switch(i%4)
                {
                case 1: cCombined = (((nPrevDecode & 0x3F) << 2) | ((nDecode & 0x30) >> 4)); break;
                case 2: cCombined = (((nPrevDecode & 0x0F) << 4) | ((nDecode & 0x3C) >> 2)); break;
                case 3: cCombined = (((nPrevDecode & 0x03) << 6) | (nDecode & 0x3F)); break;
                }

                if(nDstIdx < nLength) (*pDst)[nDstIdx++] = cCombined;
            }

            nPrevDecode = nDecode;

            /*
            pszString[nDstIdx++] = (acInput[0] & 0x3F << 2) | (acInput[1] & 0x30 >> 4);
            pszString[nDstIdx++] = (acInput[1] & 0x0F << 4) | (acInput[2] & 0x3C >> 2);
            pszString[nDstIdx++] = (acInput[2] & 0x03 << 6) | (acInput[3] & 0x3F);
            */
        }
    }

    (*pDst)[nLength] = '\0';

    return nLength;
}

/*
    TS23.038 5 CBS Data Coding Scheme
*/
static DcsType GetUssdDcs(unsigned char Octet)
{
    DcsType dcs = RIL_DCS_UNSPECIFIED;

    unsigned char CodingGroupBits = Octet & 0xF0;

    switch(CodingGroupBits)
    {
    case 0x00:    /* 0000.... */
        /*
        Language using the GSM 7 bit default alphabet
            Bits 3..0 indicate the language:
            0000 ...
        */
        dcs = RIL_DCS_GSM7BIT_DA;
        break;
    case 0x10:    /* 0001.... */
        /* 0000 GSM 7 bit default alphabet; message preceded by language indication. */
        if ( (Octet & 0x0F) == 0x00 )
        {
            dcs = RIL_DCS_GSM7BIT_DA;
        }
        /* 0001 UCS2; message preceded by language indication */
        else if ( (Octet & 0x0F) == 0x01 )
        {
            dcs = RIL_DCS_UCS2;
        }
        /* 0010..1111 Reserved */
        else
        {
            dcs = RIL_DCS_RESERVED;
        }
        break;
    case 0x20:    /* 0010.... */
        /*
        0000 ....
        0101...1111 Reserved for other languages using the GSM 7 bit default alphabet, with
                    unspecified handling at the MS
        */
        dcs = RIL_DCS_GSM7BIT_DA;
        break;
    case 0x30: /* 0011.... */
        /*
        0000..1111 Reserved for other languages using the GSM 7 bit default alphabet, with
                    unspecified handling at the MS
        */
        dcs = RIL_DCS_GSM7BIT_DA;
        break;
    case 0x40: /* 01xx.... */
    case 0x50:
    case 0x60:
    case 0x70:
        /*
        Bits 3 and 2 indicate the character set being used, as follows:
        Bit 3 Bit 2 Character set:
        0 0 GSM 7 bit default alphabet
        0 1 8 bit data
        1 0 UCS2 (16 bit)
        1 1 Reserved
        */
        if ( (Octet & 0x0C) == 0x00 )    /* 0 0 */
        {
            dcs = RIL_DCS_GSM7BIT_DA;
        }
        else if ( (Octet & 0x0C) == 0x04 )    /* 0 1 */
        {
            dcs = RIL_DCS_8BIT_DATA;
        }
        else if ( (Octet & 0x0C) == 0x08 )    /* 1 0 */
        {
            dcs = RIL_DCS_UCS2;
        }
        else     /* if ( (Octet & 0x0C) == 0x0C ) */    /* 1 1 */
        {
            dcs = RIL_DCS_RESERVED;
        }
        break;
    case 0x80:    /* 1000.... */
        /* Reserved coding groups */
        dcs = RIL_DCS_RESERVED;
        break;
    case 0x90:    /* 1001.... */
        /*
        Bits 3 and 2 indicate the alphabet being used, as follows:
        Bit 3 Bit 2 Alphabet:
        0 0 GSM 7 bit default alphabet
        0 1 8 bit data
        1 0 USC2 (16 bit)
        1 1 Reserved
        */
        if ( (Octet & 0x0C) == 0x00 )    /* 0 0 */
        {
            dcs = RIL_DCS_GSM7BIT_DA;
        }
        else if ( (Octet & 0x0C) == 0x04 )    /* 0 1 */
        {
            //dcs = RIL_DCS_8BIT_DATA;
            dcs = RIL_DCS_EUC_KR;
        }
        else if ( (Octet & 0x0C) == 0x08 )    /* 1 0 */
        {
            dcs = RIL_DCS_UCS2;
        }
        else     /* if ( (Octet & 0x0C) == 0x0C ) */    /* 1 1 */
        {
            dcs = RIL_DCS_RESERVED;
        }
        break;
    case 0xA0:    /* 1010...1100 */
    case 0xB0:
    case 0xC0:
        /* Reserved coding groups */
        dcs = RIL_DCS_RESERVED;
        break;
    case 0xD0:     /* 1101.... */
        /* I1 protocol message defined in 3GPP TS 24.294 */
        //dcs = RIL_DCS_I1_PROTO;
        ///TODO: how to decode?
        dcs = RIL_DCS_UNSPECIFIED;
        break;
    case 0xE0:    /* 1110.... */
        /* Defined by the WAP Forum */
        //dcs = RIL_DCS_WAP_FORUM;
        ///TODO: how to decode?
        dcs = RIL_DCS_UNSPECIFIED;
        break;
    case 0xF0:    /* 1111.... */
        /* Data coding / message handling */
        /*
        Bit 2 Message coding:
        0 GSM 7 bit default alphabet
        1 8 bit data
        */
        if ( (Octet & 0x04) == 0x00 )    /* 0 */
        {
            dcs = RIL_DCS_GSM7BIT_DATA;
        }
        else /* if ( (Octet & 0x04) == 0x04 ) */    /* 1 */
        {
            dcs = RIL_DCS_8BIT_DATA;
        }
        break;
    }
    return dcs;
}

unsigned int convertAsciiToGsm7bitBasicCharSet(unsigned char* ascii, unsigned int len, unsigned char* gsm7)
{
    unsigned int id = 0;

    for (unsigned int i = 0; i < len; i++)
    {
        switch (ascii[i] )
        {
            /* Basic Character Set */
        case '@': gsm7[id++] = 0x00;
            break;
        case '$': gsm7[id++] = 0x02;
            break;
            /* Basic Character Set Extension */
        case '^': gsm7[id++] = 0x1B; gsm7[id++] = 0x14;
            break;
        case '{': gsm7[id++] = 0x1B; gsm7[id++] = 0x28;
            break;
        case '}': gsm7[id++] = 0x1B; gsm7[id++] = 0x29;
            break;
        case '\\': gsm7[id++] = 0x1B; gsm7[id++] = 0x2F;
            break;
        case '[': gsm7[id++] = 0x1B; gsm7[id++] = 0x3C;
            break;
        case '~': gsm7[id++] = 0x1B; gsm7[id++] = 0x3D;
            break;
        case ']': gsm7[id++] = 0x1B; gsm7[id++] = 0x3E;
            break;
        case '|': gsm7[id++] = 0x1B; gsm7[id++] = 0x40;
            break;
        default:
            gsm7[id++] = ascii[i];
            break;
        }
    }

    return id;
}

/*
unsigned int convertGsm7bitBasicCharSetToAscii(unsigned char* gsm7bit, unsigned int len, unsigned char* ascii)
{
    unsigned int id = 0;

    for (unsigned int i = 0; i < len; i++)
    {
        switch (gsm7bit[i])
        {
        case 0x00: ascii[id++] = '@';
            break;
        case 0x02: ascii[id++] = '$';
            break;
        case 0x1B:
            {
                if (i + 1 < len)
                {
                    switch (gsm7bit[i+1])
                    {
                    case 0x14: ascii[id++] = '^'; i++;
                        break;
                    case 0x28: ascii[id++] = '{'; i++;
                        break;
                    case 0x29: ascii[id++] = '}'; i++;
                        break;
                    case 0x2F: ascii[id++] = '\\'; i++;
                        break;
                    case 0x3C: ascii[id++] = '['; i++;
                        break;
                    case 0x3D: ascii[id++] = '~'; i++;
                        break;
                    case 0x3E: ascii[id++] = ']'; i++;
                        break;
                    case 0x40: ascii[id++] = '|'; i++;
                        break;
                    default: ascii[id++] = ' ';    //NBSP
                        break;
                    }
                }
                else
                {
                    ascii[id++] = ' ';    //NBSP
                }
            }
            break;
        default:
            ascii[id++] = gsm7bit[i];
            break;
        }
    }
    return id;
}
*/

int packing_gsm7bit(const unsigned char* src, const int src_len, unsigned char* dest)
{
    // if src_len is not multiple of 8 bytes, there will be always left bit which can be remained 1 byte.
    int enc_len = (src_len%8==0)?(src_len*7/8):(src_len*7/8)+1;

    int shift = 0, src_idx = 0, dest_idx = 0;
    for (src_idx = 0; src_idx < src_len; src_idx++, dest_idx++)
    {
        //every 8th packet(which can be left) is moved all to 7th packet's high bit
        //so there's no left packet in 8th packet
        if (src_idx % 8 == 7)
        {
            src_idx++;
        }

        shift = src_idx%8;

        // move high-left bits to low
        dest[dest_idx] = src[src_idx] >> shift;

        // if next packet is exist,
        if (src_idx+1 < src_len)
        {
            // move next packet's low bit to destination's high bits
            dest[dest_idx] |= ((src[src_idx + 1] << (7-shift))&0xFF);
        }
    }

    if (src_len%8==7)
    {
        // last shift == 6 -> there was only 1 bit left
        // all 0 padding can be considered as '@' (code value=0x00 in GSM-7bit) at decoding,
        // if 7 bit padding should be added with CR (0x0D)
        // (0x0d << 1) = 0x1A
        dest[dest_idx - 1] |= 0x1A;
    }

    return enc_len;
}

int unpacking_gsm7bit(const unsigned char* src, const int src_len, unsigned char* dest)
{
    int dest_idx = 0, src_idx = 0;
    int shift;
    int dec_len = 0;

    unsigned char down_shift_count = 0;
    for (src_idx = 0; src_idx < src_len; src_idx++, dest_idx++)
    {
        shift = src_idx % 7;

        // move low bits to destination's high bits
        dest[dest_idx] |= (src[src_idx] << shift)&0x7F;

        // move high bits to destination's next low bits
        down_shift_count = 7-shift;
        dest[dest_idx+1] = (src[src_idx] >> down_shift_count);

        // if 7 bit were moved, it can be a 1 valid gsm 7bit character by itself.
        // dest_idx+1 is valid, so jump to next destination
        if (down_shift_count == 1)
        {
            dest_idx++;
        }
    }
    dec_len = dest_idx;

    return dec_len;
}

typedef struct {
    unsigned char gsm7bit;
    unsigned short ucs2;
}Gsm7Ucs2Map;
Gsm7Ucs2Map Gsm7bitDefToUcs2[] = {
#include "gsm0338_def_map.dat"
};
Gsm7Ucs2Map Gsm7bitExtToUcs2[] = {
#include "gsm0338_ext_map.dat"
};

map<unsigned char, unsigned short>Gsm0338DefMap;
map<unsigned char, unsigned short>Gsm0338ExtMap;

static void InitGsm7bitUcs2Map()
{
    if ( Gsm0338DefMap.empty() == true )
    {
        int DefMapCount = sizeof(Gsm7bitDefToUcs2)/sizeof(Gsm7Ucs2Map);
        for ( int i = 0; i < DefMapCount; i++ )
        {
            Gsm0338DefMap.insert(pair<unsigned char, unsigned short>(Gsm7bitDefToUcs2[i].gsm7bit, Gsm7bitDefToUcs2[i].ucs2));
        }
        //RilLogV("[%s] Gsm0338 default character Map insert count = %d", __FUNCTION__, DefMapCount);
    }
    //RilLogV("[%s] Gsm0338 default character Map size = %d", __FUNCTION__, Gsm0338DefMap.size());
    if ( Gsm0338ExtMap.empty() == true )
    {
        int ExtMapCount = sizeof(Gsm7bitExtToUcs2)/sizeof(Gsm7Ucs2Map);
        for ( int i = 0; i < ExtMapCount; i++ )
        {
            Gsm0338ExtMap.insert(pair<unsigned char, unsigned short>(Gsm7bitExtToUcs2[i].gsm7bit, Gsm7bitExtToUcs2[i].ucs2));
        }
        //RilLogV("[%s] Gsm0338 extension character Map insert count = %d", __FUNCTION__, ExtMapCount);
    }
    //RilLogV("[%s] Gsm0338 extension character Map size = %d", __FUNCTION__, Gsm0338ExtMap.size());
}

const unsigned short ReplacmenteOfInvalidCode = 0x0020;        // SPACE(0x0020), QUESTION MARK(0x003F)
static int ConvertGsm7bitToUcs2(const unsigned char *src, const unsigned int len, unsigned short *dst)
{
    InitGsm7bitUcs2Map();

    int dec_len = 0;
    int dst_idx = 0;

    for ( unsigned int i = 0; i < len; i ++ )
    {
        if ( src[i] == 0x1B && i+1 < len )    // ESCAPE TO EXTENSION TABLE
        {
            // check extension table
            if ( Gsm0338ExtMap.find(src[i+1]) != Gsm0338ExtMap.end() )
            {
                i++;        // spend i+1 byte, move to next
                dst[dst_idx++] = Gsm0338ExtMap[src[i]];
            }
            else
            {
                /*
                #    The ESC character 0x1B is mapped to the no-break space character, unless it is part of a valid ESC sequence
                */
                dst[dst_idx++] = 0x0020;
            }
        }
        else
        {
            // check default table
            if ( Gsm0338DefMap.find(src[i]) != Gsm0338DefMap.end() )
            {
                dst[dst_idx++] = Gsm0338DefMap[src[i]];
            }
            else
            {
                /* no valid ucs2 code */
                dst[dst_idx++] = ReplacmenteOfInvalidCode;
            }
        }
    }

    dec_len = dst_idx;

    return dec_len;
}

static int Convert8bitdataToUTF8(const unsigned char *src, const unsigned int len, unsigned char *dst, size_t dst_size)
{
    int dec_len = len;

    if ( src == NULL || dst == NULL || len == 0 || dst_size == 0 )
    {
        return 0;
    }

    /* Cell Broadcast messages using 8-bit data have user-defined coding, and will be 82 octets in length. */
    /* no need to convert to UTF8 */
    if ( dst_size > len )
    {
        memcpy(dst, src, len);
        return dec_len;
    }
    else
    {
        return 0;
    }
}

static int ConvertToHexString(const unsigned char *src, const unsigned int len, unsigned char *dst, size_t dst_size)
{
    unsigned int dec_len = len*2;
    char hexstring[4];

    if ( src == NULL || dst == NULL || len == 0 || dst_size == 0 )
    {
        return 0;
    }

    if ( dec_len >= dst_size-1 )
    {
        return 0;
    }

    int dst_idx = 0;
    for ( unsigned int i = 0; i < len; i++, dst_idx+=2 )
    {
        memset(hexstring, 0x00, sizeof(hexstring));
        snprintf(hexstring, 3, "%02x", src[i]);
        strncpy((char*)&dst[dst_idx], hexstring, 2);
    }

    return dec_len;
}

static int BeByteStream2LeUcs2(const unsigned char *src, const unsigned int len, unsigned short *dst, size_t dst_size)
{
    if ( src == NULL || dst == NULL || len == 0 || dst_size == 0 )
    {
        return 0;
    }

    if ( len %2 != 0 )
    {
        // ucs2 length should be multiple of 2.
        RilLogW("[%s] invalud UCS2 length : %d", __FUNCTION__, len);
        return 0;
    }
    int ucs_len = len/2;
    unsigned char* pDst = (unsigned char*)dst;

    for ( unsigned int i = 0; i < len; i+=2 )
    {
        pDst[i+1] = src[i];
        pDst[i] = src[i+1];
    }

    return ucs_len;
}
static int ConvertUcs2ToUtf8(const unsigned short *src, const unsigned int len, unsigned char *dst, size_t dst_size)
{
    unsigned int dec_len = 0;
    unsigned int utf_idx = 0;

    for ( unsigned int i = 0; i < len; i++ )
    {
        if ( utf_idx >= dst_size -3 )
        {
            break;
        }

        if ( src[i] <= 0x7F )            /* 000000-00007F : 0xxxxxxx */        /* 127 : ASCII */
        {
            dst[utf_idx++] = (char)src[i];    /* 0xxxxxxx */
        }
        else if ( src[i] <= 0x7FF )        /* 000080-0007FF : 110yyyxx 10xxxxxx */ /* 2047 */
        {
            dst[utf_idx++] = (0xC0 | (src[i] >> 6));            /* 110yyyxx */
            dst[utf_idx++] = (0x80 | (src[i] & 0x3F));            /* 10xxxxxx */
        }
        else //if ( src[i] <= 0xFFFF)        /* 000800-00FFFF : 1110yyyy 10yyyyxx 10xxxxxx */ /* 65535 : UCS2 */
        {
            dst[utf_idx++] = (0xE0 | (src[i] >> 12));            /* 1110yyyy */
            dst[utf_idx++] = (0x80 | ((src[i] >> 6) & 0x3F));    /* 10yyyyxx */
            dst[utf_idx++] = (0x80 | (src[i] & 0x3F));            /* 10xxxxxx */
        }
#if 0    // ucs2(2bytes) cannot support below range
        else if ( src[i] <= 0x10FFFF )    /* 010000-10FFFF : 11110zzz 10zzyyyy 10yyyyxx 10xxxxxx */    /* 1114111 : Unicode*/
        {
            dst[utf_idx++] = (0xF0 | (src[i] >> 18));            /* 11110zzz */
            dst[utf_idx++] = (0x80 | ((src[i] >> 12) & 0x3F));    /* 10zzyyyy */
            dst[utf_idx++] = (0x80 | ((src[i] >> 6) & 0x3F));    /* 10yyyyxx */
            dst[utf_idx++] = (0x80 | (src[i] & 0x3F));            /* 10xxxxxx */
        }
#endif
    }
    dec_len = utf_idx;

    return dec_len;
}

int DecodingUssd(const unsigned char dcs, const unsigned char *src, const unsigned int len, unsigned char *dst, size_t dst_size)
{
    DcsType dcsType = GetUssdDcs(dcs);
    int dec_len = 0;

    switch (dcsType)
    {
    case RIL_DCS_EUC_KR:
        ///TODO:check received data is euc-kr or not.  bEucKr is removed in order to clear prevent (dead code)
        {
            //bool bEucKr = true;

            //if ( bEucKr == true )
            //{
                dec_len = ConvertToHexString(src, len, dst, dst_size);
            //}
            //else
            //{
                //treat as 8 bit data
                // convert 8 bit data to Utf8
                //dec_len = Convert8bitdataToUTF8(src, len, dst, dst_size);
            //}
        }
        break;
    case RIL_DCS_8BIT_DATA:
        // convert 8 bit data to Utf8
        dec_len = Convert8bitdataToUTF8(src, len, dst, dst_size);
        break;
    case RIL_DCS_UCS2:
        {
            // convert received ucs2 binary stream to Ucs2 stream
            unsigned short Ucs2[MAX_USSD_DATA_LEN];
            memset(Ucs2, 0x00, sizeof(Ucs2));
            int Ucs2Len = BeByteStream2LeUcs2(src, len, Ucs2, sizeof(Ucs2));

            // convert Ucs2 to Utf8
            dec_len = ConvertUcs2ToUtf8(Ucs2, Ucs2Len, dst, dst_size);
        }
        break;
    case RIL_DCS_GSM7BIT_DA:
    case RIL_DCS_RESERVED:    /* Any reserved codings shall be assumed to be the GSM 7 bit default alphabet */
        {
            // unpacking Gsm7bit
            unsigned char Gsm7[MAX_USSD_DATA_LEN];
            memset(Gsm7, 0x00, sizeof(Gsm7));
            int Gsm7Len = unpacking_gsm7bit(src, len, Gsm7);

            // convert Utf8 to Ucs2
            unsigned short Ucs2[MAX_USSD_DATA_LEN];
            memset(Ucs2, 0x00, sizeof(Ucs2));
            int Ucs2Len = ConvertGsm7bitToUcs2(Gsm7, Gsm7Len, Ucs2);

            // convert Ucs2 to Utf8
            dec_len = ConvertUcs2ToUtf8(Ucs2, Ucs2Len, dst, dst_size);
        }
        break;
    case RIL_DCS_GSM7BIT_DATA:
        dec_len = ConvertToHexString(src, len, dst, dst_size);
        break;
    case RIL_DCS_UNSPECIFIED:
    default:
        RilLogW("[%s] invalud DCS  : %d", __FUNCTION__, dcs);
        break;
    }

    return dec_len;
}

int DecodingNetworkNameFromSIM(const unsigned char type,
                            const unsigned char *src,
                            int srcLen,
                            int countSeptets,
                            unsigned char *dst,
                            unsigned char dstLen)
{
    if (src == NULL || dst == NULL || srcLen < 0) return 0;

    int decLen = 0;
    if (type == 0) {
        // unpacking Gsm7bit
        decLen = unpacking_gsm7bit(src, srcLen, dst);
        if (decLen > countSeptets) {
            decLen = countSeptets;
            dst[countSeptets] = 0;
        }
    }
    else if (type == 1) {
        unsigned short Ucs2[MAX_FULL_NAME_LEN];
        memset(Ucs2, 0x00, MAX_FULL_NAME_LEN);
        int Ucs2Len = (MAX_FULL_NAME_LEN - 1) < srcLen ? (MAX_FULL_NAME_LEN - 1):srcLen;
        memcpy(Ucs2, src, Ucs2Len);

        // convert Ucs2 to Utf8
        decLen = ConvertUcs2ToUtf8(Ucs2, Ucs2Len, dst, dstLen);
    }
    else {
        // nothing to do
    }

    return decLen;
}


int runAM(char *cmd)
{
    char szCmd[512];
    sprintf(szCmd, "/system/bin/am %s", cmd);
    return runcmd(szCmd);
}

int runcmd(char *cmd)
{
#ifdef _USE_FORK_
    pid_t child_pid;
    int child_status;

    char* argv[MAX_ARGS];
    parsecmd(cmd,argv);

    child_pid = fork();
    if(child_pid == 0) {
        /* This is done by the child process. */
        execv(argv[0], argv);

        /* If execv returns, it must have failed. */
        printf("Unknown command\n");
        return 0;
    }
    else
    {
        /* This is run by the parent.  Wait for the child to terminate. */
        pid_t tpid = wait(&child_status);

        return child_status;
    }
#else
    system(cmd);
    return 0;
#endif
}
