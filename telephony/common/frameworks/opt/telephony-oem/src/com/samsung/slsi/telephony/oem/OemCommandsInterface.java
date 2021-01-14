/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.oem;

import android.os.Handler;
import android.os.Message;

public interface OemCommandsInterface {

    public void detach();
    public void invokeRequestRaw(int request, byte[] data, Message response);
    public void registerForOemRilConnected(Handler h, int what);
    public void unregisterForOemRilConnected(Handler h);
    public void registerForOemRilDisconnected(Handler h, int what);
    public void unregisterForOemRilDisconnected(Handler h);
}
