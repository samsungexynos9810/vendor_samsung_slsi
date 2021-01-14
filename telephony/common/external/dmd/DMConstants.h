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
 * DMConstants.h
 *
 *  Created on: 2018. 5. 18.
 */

#ifndef __DM_CONSTANTS_H__
#define __DM_CONSTANTS_H__

enum {
    MODE_NONE,
    MODE_EXTERNAL_DM,
    MODE_SILENT_LOGGING,
    MODE_ON_BOARD_DM,
    MODE_EXT_APP_DM,
};

enum {
    COMMAND_SET_DM_MODE = 0,
    COMMAND_SEND_PROFILE = 1,   // Send start + profile
    COMMAND_STOP_DM = 2,        // Send stop
    COMMAND_SAVE_SNAPSHOT = 3,
    COMMAND_NOTIFY_DM_LOG = 4,
    COMMAND_REFRESH_FILE_LIST = 5,
    COMMAND_SAVE_AUTOLOG = 6,
    COMMAND_SET_DM_MAX_FILE_SIZE = 7,
    FACTORY_COMMAND_SET_DM_MODE = 100,
};

#endif /* __DM_CONSTANTS_H__ */
