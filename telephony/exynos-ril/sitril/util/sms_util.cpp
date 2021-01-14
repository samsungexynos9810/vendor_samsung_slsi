/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include "sms_util.h"
#include "rillog.h"

int ConvertSmscBcdToNumber(const char *tpdu, int tpduLen, char *smsc, int smscLen)
{

    if (tpduLen <= 0) {
        return -1;
    }

    int len = (tpduLen - 1) * 2;
    char buf[MAX_GSM_SMS_SERVICE_CENTER_ADDR * 2] = {0, };

    //Check '+'
    int i = 0, j = 1;
    if (tpdu[j] == RIL_TOA_INTERNATIONAL) {
        buf[i] = '+';
        i++;
        j++;
        len--;
    } else if (tpdu[j] == RIL_TOA_UNKNOWN) {
        // TOA: National Numbering skip
        j++;
        len--;
    }

    //Convert BCD -> ASCII
    for (; j < tpduLen; i++, j++) {
        buf[i] = (tpdu[j] & 0x0F) + '0';
        if ((tpdu[j] >> 4) == 0x0F) {
            // odd count smsc
            len--;
            break;
        }
        else {
            buf[++i] = ((tpdu[j] >> 4) & 0x0F) + '0';
        }
    }

    if (len > smscLen) {
        return -1;
    }

    strncpy(smsc, buf, len);
    RilLogV("smsc=%s length=%d", smsc, len);

    return len;
}

int ConvertSmscNumberToBcd(const char *smsc, int smscLen, char *tpdu, int tpduLen)
{
    if (smscLen <= 0) {
        return -1;
    }

    //Consist of SMSC
    // 09  08  91  28  01  00  99  01  02  89
    // BYTE SCA_LEN => 09
    // BYTE SCA[MAX 12]='SCA array len => 08','SCA data => 91  28  01  00  99  01  02  89'
    int i=0, j=1, insert_point=0;
    char buf[MAX_GSM_SMS_SERVICE_CENTER_ADDR] = {0, };

    //Check '+'
    if (smsc[i] == '+') {
        // TOA: international numbering
        buf[j] = RIL_TOA_INTERNATIONAL;
        i++;
        j++;
        insert_point++;
    } else {
        // TOA: national numbering
        buf[j]  = RIL_TOA_UNKNOWN;
        j++;
    }

    //Convert ASCII -> BCD
    for(; i < smscLen; i++)
    {
        if (i % 2 == insert_point) {
            buf[j] = (smsc[i] & 0x0F);
        }
        else {
            buf[j] |= ((smsc[i] << 4) & 0xF0);
            j++;
        }
    }

    if ((insert_point && i % 2 == 0) || (!insert_point && i % 2 == 1)) {
        // odd count smsc
        buf[j] |= 0xF0;    // append 0xF to high nibble
        j++;
    }
    buf[0] = j - 1;

    if (j > tpduLen) {
        return -1;
    }

    memcpy(tpdu, buf, j);

    return j;
}

bool IsSmscLenValid(const char *smsc, int smscLen){
    // One byte for Length and one byte for TOA, remaing charcaters
    // will be ecnoded to BCD i.e. maximum 20 characters (10 bytes)
    int maxLen = (MAX_GSM_SMS_SERVICE_CENTER_ADDR -2) * 2;

    // check for + sign, maximum length will be 21 character in this case
    if (smsc[0] == '+' ) {
        maxLen++;
    }

    if ( smscLen > maxLen) {
        return false;
    }
    return true;
}
