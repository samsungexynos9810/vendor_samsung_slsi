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
    RFSD main
*/

#include <sys/poll.h>
#include <unistd.h>

#include "rfsservice.h"
#include "modemstatemonitor.h"

extern CRfsService *m_pRfsSrv;
extern bool firstBoot;
