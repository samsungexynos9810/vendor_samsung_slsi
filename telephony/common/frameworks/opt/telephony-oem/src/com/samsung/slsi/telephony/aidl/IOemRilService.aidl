/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.aidl;

import android.os.IBinder;
import android.os.Messenger;

interface IOemRilService {
    int invokeOemRilRequestRaw(in int request, in byte[] data, in Messenger messenger, in IBinder binder, in int phoneId);
}