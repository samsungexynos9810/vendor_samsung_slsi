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

oneway interface IVsimServiceCallback {

    /*
     * onResults
     *
     * error: 0(success), 1(failure)
     *
     */
    void sendVsimNotificationRsp(in int error, in int phoneId);

    /*
     * onResults
     *
     * error: 0(success), 1(failure)
     *
     */
    void sendVsimOperationRsp(in int error, in int phoneId);

    /*
     * notify VSIM operation
     *
     */
    void notifyVsimOpereation(in int transactionId, in int eventId, in int result, in String data, in int phoneId);
}