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
 * systemmessageid.h
 *
 *  Created on: 2014. 7. 14.
 *      Author: sungwoo48.choi
 */

#ifndef __SYSTEM_MESSAGE_ID_H__
#define __SYSTEM_MESSAGE_ID_H__

#define    MSG_SYSTEM_BASE        0x0100

enum {
    MSG_SYSTEM_RADIO_STATE_CHANGED    = MSG_SYSTEM_BASE + 1,
    MSG_SYSTEM_VOICE_REGISTRTION_STATE_CHANGED,
    MSG_SYSTEM_DATA_REGISTRTION_STATE_CHANGED,
    MSG_SYSTEM_SERVICE_TATE_CHANGED,
    MSG_SYSTEM_SIM_STATUS_CHANGED,
    MSG_SYSTEM_IMSI_UPDATED,
    MSG_SYSTEM_RESET,
    MSG_SYSTEM_DATA_CALL_STATE_CHANGED,
    MSG_SYSTEM_MODEM_STATE_CHANGED,
};

#endif /* __SYSTEM_MESSAGE_ID_H__ */
