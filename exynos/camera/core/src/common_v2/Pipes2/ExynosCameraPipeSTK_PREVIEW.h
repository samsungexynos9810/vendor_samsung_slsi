/*
**
** Copyright 2017, Samsung Electronics Co. LTD
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef EXYNOS_CAMERA_PIPE_STK_PREVIEW_H
#define EXYNOS_CAMERA_PIPE_STK_PREVIEW_H

#include <dlfcn.h>
#include "ExynosCameraSWPipe.h"
#include "ExynosCameraAutoTimer.h"
#include "ExynosCameraPipeSTK_PICTURE.h"

#define STK_PREVIEW_LIBRARY_PATH STK_LIBRARY_PATH

namespace android {

class ExynosCameraPipeSTK_PREVIEW : protected virtual ExynosCameraSWPipe {
public:
    ExynosCameraPipeSTK_PREVIEW()
    {
        m_init(NULL);
    }

    ExynosCameraPipeSTK_PREVIEW(
        int cameraId,
        ExynosCameraConfigurations *configurations,
        ExynosCameraParameters *obj_param,
        bool isReprocessing,
        cameraId_Info *camIdInfo,
        int32_t *nodeNums)  : ExynosCameraSWPipe(cameraId, configurations, obj_param, isReprocessing, nodeNums, camIdInfo)
    {
        ALOGD("ExynosCameraPipeSTK_PREVIEW Initialization Start!");
        if (m_init(nodeNums) < NO_ERROR) {
            bSTKInit = false;
            ALOGE("ExynosCameraPipeSTK_PREVIEW Initialization failed!");
        }
        else {
            bSTKInit = true;
            ALOGD("ExynosCameraPipeSTK_PREVIEW Initialization succeed!");
        }
    }

    virtual status_t        create(int32_t *sensorIds = NULL);

    virtual status_t        start(void);
    virtual status_t        stop(void);

protected:
    virtual status_t        m_destroy(void);
    virtual status_t        m_run(void);

private:
    status_t                m_init(int32_t *nodeNums);

private:
    int                     m_stkNum;
    void                    *m_stk;
    void                    *stkHandle;
    void                    *m_stk_handle;
    bool                    bSTKInit = false;
    void                    *hSTK_object;
    pthread_t               *m_thread_id;

    void* (*init_stk)(int, int, enum stain_killer_scenario);
    pthread_t* (*run_stk)(void*, char*, char*, int);
    int (*end_stk)(void*);

    STK_params             m_stkdynamicMeta;
    ExynosCameraDurationTimer      m_timer;

};

}; /* namespace android */

#endif
