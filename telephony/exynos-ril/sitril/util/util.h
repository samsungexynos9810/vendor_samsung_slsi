/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include "types.h"
#include "rildef.h"

#include <cutils/properties.h>

#define TAB_GSM_UNI_MAX     42
#define TAB_GSM_UNI_MAX2    9
#define BIT7_CODING         0x7F
#define BIT8_CODING         0xFE
#define BIT7                7
#define BIT8                8
#define MAX_ASCII           127


UINT8 HexCharToInt(char c);
void ConvertToHexString(char **hex, char *data, int len);
void ConvertToRaw(char *hexStr, char *data, int *len);
int ConvertToRaw(char *hexStr, char *buf, int bufSize);

string GetSimOperatorNum(int phoneId);
string GetSimOperatorNumericGsm(int phoneId);
string GetSimOperatorNumericCdma(int phoneId);
string GetSimSpn(int phoneId);
string bcdToString(const BYTE *src,int srcLen);
string bchToString(const BYTE *src,int srcLen);
bool IsPlmnMatchedSimPlmn(const char *carrier, int phoneId);

string bcdToString(const BYTE *src,int srcLen);

// MOX
void PrintBufferDump(const char *pszLabel, const BYTE *pBuffer, int nLength);
int Value2HexString(char *pszHexStrOut, const BYTE *pHexDecIn, int nLength);
int HexChar2Value(char ch);
int HexString2Value(BYTE *pHexDecOut, const char *pszHexStrIn);
char *Base64_Encode(const BYTE *pSource, int nSrcLength);
int Base64_Decode(BYTE **pDst, const char *pBase64);

unsigned int convertAsciiToGsm7bitBasicCharSet(unsigned char* ascii, unsigned int len, unsigned char* gsm7);
int packing_gsm7bit(const unsigned char* src, const int src_len, unsigned char* dest);
int unpacking_gsm7bit(const unsigned char* src, const int src_len, unsigned char* dest);
int DecodingUssd(const unsigned char dcs, const unsigned char *src, const unsigned int len, unsigned char *dst, size_t dst_size);
int DecodingNetworkNameFromSIM(const unsigned char type, const unsigned char *src, int srcLen, int countSeptets, unsigned char *dst, unsigned char dstLen);

int runAM(char *cmd);
int runcmd(char *cmd);

#endif /* __UTIL_H__ */

