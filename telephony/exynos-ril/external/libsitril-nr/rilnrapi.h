/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
 * rilnrapi.h
 */

#ifndef __RIL_NR_API_H__
#define __RIL_NR_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * enableNr
 * @description enable or disable NR mode
 * @param enable true if NR enabled false if NR disabled
 * @return 0 if success, otherwise error
 */
int enableNr(int enable);

#ifdef __cplusplus
};
#endif

#endif // __RIL_NR_API_H__
