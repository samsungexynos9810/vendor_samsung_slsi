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

import com.samsung.slsi.telephony.aidl.IDesenseServiceCallback;

oneway interface IDesenseService {

    /*
     * registerATCommandCallback
     *
     */
    void registerATCommandCallback(in IDesenseServiceCallback callback);

    /*
     * setRssiScan
     *
     * data is byte[]
     *   rat - 4bytes
     *   band - 4bytes
     *   scan mode - 4bytes
     *   start frequency - 4bytes
     *   end frequency - 4bytes
     *   step(offset) - 4bytes
     *   antenna selection - 4bytes
     *   sampling count - 4bytes
     *   tx1 - 4bytes
     *   tx1 band - 4bytes
     *   tx1 bw - 4bytes
     *   tx1 freq - 4bytes
     *   tx1 power - 4bytes
     *   tx1 rb_num - 4bytes
     *   tx1 rb_offset - 4bytes
     *   tx1 mcs - 4bytes
     *   tx2 - 4bytes
     *   tx2 band - 4bytes
     *   tx2 bw - 4bytes
     *   tx2 freq - 4bytes
     *   tx2 power - 4bytes
     *   tx2 rb_num - 4bytes
     *   tx2 rb_offset - 4bytes
     *   tx2 mcs - 4bytes
     */
    void setScanRssi(in byte[] data, in int phoneId);

    /*
     * sendATCommand
     *
     */
    void sendATCommand(in String command, in int phoneId);
}