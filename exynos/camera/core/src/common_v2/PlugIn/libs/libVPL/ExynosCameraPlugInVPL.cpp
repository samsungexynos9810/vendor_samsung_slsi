/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/*#define LOG_NDEBUG 0 */
#define LOG_TAG "ExynosCameraPlugInVPL"
#include <log/log.h>

#include "libvpl.h"
#include "ExynosCameraPlugInVPL.h"

#define FRAMES_TO_SKIP 1

namespace android {

volatile int32_t ExynosCameraPlugInVPL::initCount = 0;

DECLARE_CREATE_PLUGIN_SYMBOL(ExynosCameraPlugInVPL);

/*********************************************/
/*  protected functions                      */
/*********************************************/
status_t ExynosCameraPlugInVPL::m_init(void)
{
    int count = android_atomic_inc(&initCount);

    CLOGD("count(%d)", count);

    if (count == 1) {
        /* do nothing */
    }

    return NO_ERROR;
}

status_t ExynosCameraPlugInVPL::m_deinit(void)
{
    int count = android_atomic_dec(&initCount);

    CLOGD("count(%d)", count);

    if (count == 0) {
        /* do nothing */
    }

    return NO_ERROR;
}

status_t ExynosCameraPlugInVPL::m_create(void)
{
    vpl = new VPL;
    vpl->create();
    strncpy(m_name, "VPL_PLUGIN", (PLUGIN_NAME_STR_SIZE - 1));
    frameCnt = 0;
    lastVplResultMap.clear();
    return NO_ERROR;
}

status_t ExynosCameraPlugInVPL::m_destroy(void)
{
    if (vpl) {
        vpl->destroy();
        delete vpl;
        vpl = NULL;
    }
    return NO_ERROR;
}

status_t ExynosCameraPlugInVPL::m_setup(Map_t *map)
{
    if (!vpl)
    {
        vpl = new VPL;
        vpl->create();
        strncpy(m_name, "VPL_PLUGIN", (PLUGIN_NAME_STR_SIZE - 1));
    }
    vpl->setup(map);

    return NO_ERROR;
}

static void copyVplResults(Map_t *srcMap, Map_t *dstMap)
{
    int face_num = (Data_int32_t)(*srcMap)[PLUGIN_NFD_DETECTED_FACES];

    (*dstMap)[PLUGIN_NFD_DETECTED_FACES] = face_num;

    if (face_num) {
        (*dstMap)[PLUGIN_NFD_FACE_ID] = (*srcMap)[PLUGIN_NFD_FACE_ID];
        (*dstMap)[PLUGIN_NFD_FACE_SCORE] = (*srcMap)[PLUGIN_NFD_FACE_SCORE];
        (*dstMap)[PLUGIN_NFD_FACE_RECT] = (*srcMap)[PLUGIN_NFD_FACE_RECT];
        (*dstMap)[PLUGIN_NFD_FACE_ROTATION] = (*srcMap)[PLUGIN_NFD_FACE_ROTATION];
        (*dstMap)[PLUGIN_NFD_FACE_YAW] = (*srcMap)[PLUGIN_NFD_FACE_YAW];
        (*dstMap)[PLUGIN_NFD_FACE_PITCH] = (*srcMap)[PLUGIN_NFD_FACE_PITCH];
    }
}

status_t ExynosCameraPlugInVPL::m_process(Map_t *map)
{
    CLOGV("");
    status_t ret = NO_ERROR;
    /* from map */
    Data_int32_t      mapSrcBufCnt      = (Data_int32_t     )(*map)[PLUGIN_SRC_BUF_CNT];
    Array_buf_t       mapSrcBufPlaneCnt = (Array_buf_t      )(*map)[PLUGIN_SRC_BUF_PLANE_CNT];
    Array_buf_plane_t mapSrcBufSize     = (Array_buf_plane_t)(*map)[PLUGIN_SRC_BUF_SIZE];
    Array_buf_rect_t  mapSrcRect        = (Array_buf_rect_t )(*map)[PLUGIN_SRC_BUF_RECT];
    Array_buf_t       mapSrcV4L2Format  = (Array_buf_t      )(*map)[PLUGIN_SRC_BUF_V4L2_FORMAT];
    Array_buf_addr_t  mapSrcBufAddr     = (Array_buf_addr_t )(*map)[PLUGIN_SRC_BUF_1];   //source;

    if (mapSrcBufCnt > 0)
            PLUGIN_LOGV("src[%d]::(adr:%p, P%d, S%d, [%d, %d, %d, %d, %d, %d] format :%d",
                    mapSrcBufCnt,
                    mapSrcBufAddr[0],
                    mapSrcBufPlaneCnt[0],
                    mapSrcBufSize[0][0],
                    mapSrcRect[0][PLUGIN_ARRAY_RECT_X],
                    mapSrcRect[0][PLUGIN_ARRAY_RECT_Y],
                    mapSrcRect[0][PLUGIN_ARRAY_RECT_W],
                    mapSrcRect[0][PLUGIN_ARRAY_RECT_H],
                    mapSrcRect[0][PLUGIN_ARRAY_RECT_FULL_W],
                    mapSrcRect[0][PLUGIN_ARRAY_RECT_FULL_H],
                    mapSrcV4L2Format[0]);

    if (vpl && frameCnt % FRAMES_TO_SKIP == 0)
    {
        ret =  vpl->execute(map);
        lastVplResultMap.clear();
        copyVplResults(map, &lastVplResultMap);
    }
    else {
        copyVplResults(&lastVplResultMap, map);
    }
    frameCnt++;

#if 0 //for debugging
    int face_num = (Data_int32_t)(*map)[PLUGIN_NFD_DETECTED_FACES];
    if (face_num > 0) {
        for (int index = 0; index < face_num; index++) {
            int id              = ((Array_vpl_int32_t)(*map)[PLUGIN_NFD_FACE_ID])[index];
            unsigned int score  = ((Array_vpl_float_t)(*map)[PLUGIN_NFD_FACE_SCORE])[index];
            float offset_x      = ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[index][0];
            float offset_y      = ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[index][1];
            float width         = ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[index][2] - ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[index][0];
            float height        = ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[index][3] - ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[index][1];
            float rotation      = ((Array_vpl_float_t)(*map)[PLUGIN_NFD_FACE_ROTATION])[index];
            float yaw           = ((Array_vpl_float_t)(*map)[PLUGIN_NFD_FACE_YAW])[index];
            float pitch         = ((Array_vpl_float_t)(*map)[PLUGIN_NFD_FACE_PITCH])[index];

            PLUGIN_LOGD("dst::ID%d, S%u, [%f, %f, %f, %f], R[%f], Y[%f], P[%f]",
                    id,
                    score,
                    offset_x,
                    offset_y,
                    width,
                    height,
                    rotation,
                    yaw,
                    pitch);
        }
    }
#endif

    return ret;
}

status_t ExynosCameraPlugInVPL::m_setParameter(int key, void *data)
{
    status_t ret = NO_ERROR;

    switch(key) {
    case PLUGIN_PARAMETER_KEY_START:
        ret = this->m_start();
        break;
    case PLUGIN_PARAMETER_KEY_STOP:
        ret = this->m_stop();
        break;
    default:
        CLOGW("Unknown key(%d)", key);
    }
    return ret;
}

status_t ExynosCameraPlugInVPL::m_getParameter(int key, void *data)
{
    return NO_ERROR;
}

void ExynosCameraPlugInVPL::m_dump(void)
{
    /* do nothing */
}

status_t ExynosCameraPlugInVPL::m_query(Map_t *map)
{
    CLOGV("");

    return vpl->query(map);
}

status_t ExynosCameraPlugInVPL::m_start(void)
{
    vpl->start();
    return NO_ERROR;
}

status_t ExynosCameraPlugInVPL::m_stop(void)
{
    vpl->stop();
    return NO_ERROR;
}
}
