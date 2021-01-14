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

oneway interface IDesenseServiceCallback {

    /*
     * onResults
     *
     * error is 0 for SUCCESS
     * error is 1 for FAILURE
     *
     */
    void onResults(in int error, in int phoneId);

    /*
     * onRssiScanResult
     *
     * data is byte[]
     * total page - 4bytes
     * current page - 4bytes
     * start frequency - 4bytes
     * end frequency - 4bytes
     * step(offset) - 4bytes
     * result - variable, (2*N) bytes
     *
     */
    void onRssiScanResult(in byte[] data, in int phoneId);

    /*
     * onCallback
     *
     */
    void onATCommandCallback(in String command, in int phoneId);
}