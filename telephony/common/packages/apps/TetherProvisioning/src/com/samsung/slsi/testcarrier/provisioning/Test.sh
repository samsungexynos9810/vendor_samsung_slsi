#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

#!/bin/bash
# This script is intended for testing Data Error UI in PC side

# This will test Error notification
adb shell am broadcast -a android.intent.action.REQUEST_NETWORK_FAILED --ei errorCode 35

# then try Celluar Data off -> Notification clear
# then try Airplane Mode on -> Notification clear
# If network has connected to mobile correctly -> Notification clear,
# we have no method to test this, because AirplaneMode on is already triggered
