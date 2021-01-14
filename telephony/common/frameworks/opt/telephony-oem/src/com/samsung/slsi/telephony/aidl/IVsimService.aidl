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

import com.samsung.slsi.telephony.aidl.IVsimServiceCallback;

oneway interface IVsimService {

    /*
     * registerCallback
     * register callback for handling response or indication message
     */
    void registerCallback(in IVsimServiceCallback cb);

    /*
     * send VSIM notification
     */
    void sendVsimNotification(in int transactionId, in int eventId, in int simType, in int phoneId);

    /*
     * send VSIM operation
     */
    void sendVsimOperation(in int transactionId, in int eventId, in int result, in String data, in int phoneId);
}