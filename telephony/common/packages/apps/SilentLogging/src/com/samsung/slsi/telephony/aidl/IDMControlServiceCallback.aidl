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

oneway interface IDMControlServiceCallback {

    /*
     * onResults
     *
     * error is 0 for SUCCESS
     * error is 1 for FAILURE
     *
     */
    void onResults(in int commnadId, in int error);

}
