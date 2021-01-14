/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

 #ifndef __OPEN_CARRIER_H_
#define __OPEN_CARRIER_H_

#include "types.h"

typedef enum {
    // Do not change order. The value should be the same to SMI spec
    OC_NONE = 0x0,

    // Aisa & Australia: 0x10000000
    // China mainland
    OC_CMCC = 0x10000000,
    OC_CUCC = 0x10000001,

    // HK
    OC_CSL = 0x10000003,
    OC_PCCW = 0x10000004,
    OC_3HK = 0x10000005,

    // Macau
    OC_CTC = 0x10000006,

    // Cambodia
    OC_CELLCARD = 0x10000007,
    OC_SEATEL = 0x10000008,

    // Thailand
    OC_DTAC = 0x10000009,

    // Malaysia
    OC_YES = 0x1000000A,

    //Singapore
    OC_STARHUB = 0x1000000B,
    OC_SINGTEL = 0x1000000C,
    OC_M1 = 0x1000000D,

    // Europ & Africa: 0x20000000
    // Isarel
    OC_PELEPHONE = 0x20000000,
    OC_CELLCOM = 0x20000001,
    OC_ORANGE  = 0x20000002,

    // Italy
    OC_VODAFONE = 0x20000003,

    // Rassia
    OC_MEGAFON = 0x20000004,

    // Spain
    OC_MASMOVIL = 0x20000005,
    OC_YOIGO = 0x20000006,

    // South Africa
    OC_TELKOM = 0x20000007,
    OC_VODACOM = 0x20000008,

    // America: 0x30000000
    // USA
    OC_ATT = 0x30000000,
    OC_TMO = 0x30000001,

    // Canada
    OC_ROGERS = 0x30000002,

    OC_UNKNOWN = 0xFFFFFFFF
}OPEN_CARRIER_NAME;

unsigned int GetOcNameByMccMnc(const char* mccmnc);


#endif  //__OPEN_CARRIER_H_
