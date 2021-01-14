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

oneway interface ISarServiceCallback {

    /*
     * onResults
     *
     * error: 0(success), 1(failure)
     *
     */
    void setSarStateRsp(in int error, in int phoneId);

    /*
     * onResults
     *
     * error: 0(success), 1(failure)
     * state: sar state index (0 ~ 8)
     *
     */
    void getSarStateRsp(in int error, in int state, in int phoneId);

    /*
     * notifyRfConnection
     *
     * rfstate is 0 for RF cable is disconnected
     * rfstate is 1 for RF cable is connected
     *
     */
    void notifyRfConnection(in int rfstate, in int phoneId);
}