/*
**
** Copyright 2018, Samsung Electronics Co. LTD
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

/* #define LOG_NDEBUG 0 */
#define LOG_TAG "vpl"

#include "libvpl.h"

#define BGR_COLOR_CHANNELS 3
#define MAX_FACES          10


VPL::VPL()
{
    m_vplDl = NULL;
    m_vplHandle = NULL;
    createVplLib = NULL;
    destroyVplLib = NULL;
    handleVplArray = NULL;
}

VPL::~VPL()
{
}

status_t VPL::create(void)
{
    char vplLibPath[] = VPL_LIBRARY_PATH;
    PLUGIN_LOGD("%s[%d]: new VPL object", __FUNCTION__, __LINE__);
    m_vplHandle = NULL;

    m_vplDl = dlopen(vplLibPath, RTLD_NOW);
    if (m_vplDl == NULL) {
        PLUGIN_LOGE("Failed to open VPL Library. path %s, err %s", vplLibPath, dlerror());
        return INVALID_OPERATION;
    }

    createVplLib = (void* (*)(void)) dlsym(m_vplDl, "createVpl");
    if (dlerror() != NULL && createVplLib == NULL) {
        PLUGIN_LOGE("Failed to dlsym createVpl");
        goto err_exit;
    }

    destroyVplLib = (void (*)(void *)) dlsym(m_vplDl, "destroyVpl");
    if (dlerror() != NULL && destroyVplLib == NULL) {
        PLUGIN_LOGE("Failed to dlsym destroyVpl");
        goto err_exit;
    }

    handleVplArray = (VPL_RETURN_TYPE (*)(void*, VPL_FrameStr*, size_t, struct VPL_FacesStr*,size_t&)) dlsym(m_vplDl, "vplHandleArray");
    if (dlerror() != NULL && handleVplArray == NULL) {
        PLUGIN_LOGE("Failed to dlsym vplHandleArray");
        goto err_exit;
    }

    PLUGIN_LOGD("%s[%d]: completed successfully", __FUNCTION__, __LINE__);

    return NO_ERROR;
err_exit:
    if (m_vplDl != NULL) {
        dlclose(m_vplDl);
    }

    return INVALID_OPERATION;
}

status_t VPL::start(void)
{
    PLUGIN_LOGD("%s[%d]: started", __FUNCTION__, __LINE__);
    m_vplHandle = (*createVplLib)();
    if (m_vplHandle == NULL)
    {
        PLUGIN_LOGE("createVplLib failed to create m_vplHandle object");
        return UNKNOWN_ERROR;
    }
    PLUGIN_LOGD("%s[%d]: new VPL Handler %p created successfully", __FUNCTION__, __LINE__, m_vplHandle);
    return NO_ERROR;
}

status_t VPL::stop(void)
{
    PLUGIN_LOGD("%s[%d]: started", __FUNCTION__, __LINE__);
    if (m_vplHandle == NULL) {
        PLUGIN_LOGW("vplHandle is already destroyed");
    } else {
        (*destroyVplLib)(m_vplHandle);
        PLUGIN_LOGD("%s[%d]: VPL Handler %p was destroyed", __FUNCTION__, __LINE__, m_vplHandle);
        m_vplHandle = NULL;
    }

    return NO_ERROR;
}

status_t VPL::init(Map_t *map)
{
    return NO_ERROR;
}

status_t VPL::setup(Map_t *map)
{
    return NO_ERROR;
}


status_t VPL::query(Map_t *map)
{
    if (map != NULL) {
        (*map)[PLUGIN_VERSION]                = (Map_data_t)MAKE_VERSION(1, 0);
        (*map)[PLUGIN_LIB_NAME]               = (Map_data_t) "VPLLib";
        (*map)[PLUGIN_BUILD_DATE]             = (Map_data_t)__DATE__;
        (*map)[PLUGIN_BUILD_TIME]             = (Map_data_t)__TIME__;
        (*map)[PLUGIN_PLUGIN_CUR_SRC_BUF_CNT] = (Map_data_t)1;
        (*map)[PLUGIN_PLUGIN_CUR_DST_BUF_CNT] = (Map_data_t)0;
        (*map)[PLUGIN_PLUGIN_MAX_SRC_BUF_CNT] = (Map_data_t)1;
        (*map)[PLUGIN_PLUGIN_MAX_DST_BUF_CNT] = (Map_data_t)0;
    }

    return NO_ERROR;
}

status_t VPL::destroy(void)
{
    PLUGIN_LOGD("%s[%d]: started", __FUNCTION__, __LINE__);

    if (m_vplHandle) {
        (*destroyVplLib)(m_vplHandle);
        PLUGIN_LOGD("%s[%d]: VPL Handler %p was destroyed", __FUNCTION__, __LINE__, m_vplHandle);
        m_vplHandle = NULL;
    }

    if (m_vplDl != NULL) {
        dlclose(m_vplDl);
        m_vplDl = NULL;
    }

    return NO_ERROR;
}

bool VPL::clipToRoi(VPL_FacesStr &face, RectangleStr &roi) {

	int leftClipSize = FLM_MAX(roi.topLeft.x - face.rectangle.topLeft.x, 0);
	int rightClipSize = FLM_MAX(face.rectangle.topLeft.x + face.rectangle.width - (roi.topLeft.x + roi.width), 0);

	int topClipSize = FLM_MAX(roi.topLeft.y - face.rectangle.topLeft.y, 0);
	int bottomClipSize = FLM_MAX(face.rectangle.topLeft.y + face.rectangle.height - (roi.topLeft.y + roi.height), 0);

	face.rectangle.topLeft.x += leftClipSize;
	face.rectangle.width -= leftClipSize - rightClipSize;
	face.rectangle.topLeft.y += topClipSize;
	face.rectangle.height -= topClipSize - bottomClipSize;

    return face.rectangle.width > 0 && face.rectangle.height;
}

status_t VPL::execute(Map_t *map)
{
    PLUGIN_LOGD("%s[%d]: started", __FUNCTION__, __LINE__);

    VPL_RETURN_TYPE vplRet = VPL_RETURN_SUCCESS;
    struct VPL_FacesStr inOutFaces[MAX_FACES];
    size_t faceNumber = MAX_FACES;
    size_t numFacesInRoi = 0;

    Array_buf_addr_t sourceBuf = (Array_buf_addr_t)(*map)[PLUGIN_SRC_BUF_1];
    Array_buf_t sourceBufSize = (Array_buf_t)(*map)[PLUGIN_SRC_BUF_SIZE];
    Array_buf_t rectDimensions = (Array_buf_t)(*map)[PLUGIN_SRC_BUF_RECT];

    unsigned char *buf = (unsigned char *)(sourceBuf[0]);
    unsigned int channelSize = sourceBufSize[0] / BGR_COLOR_CHANNELS;

    // prepare VPL frame structure
    VPL_FrameStr frameStr;
    frameStr.bBuffer = buf;
    frameStr.bLength = channelSize;
    frameStr.uvOrGBuffer = buf + channelSize;
    frameStr.uvOrGLength = channelSize;
    frameStr.yOrRBuffer = buf + 2 * channelSize;
    frameStr.yOrRLength = channelSize;
    frameStr.frameSize.width =  ((Array_buf_t)(*map)[PLUGIN_SRC_BUF_WSTRIDE])[0];
    frameStr.frameSize.height = channelSize / frameStr.frameSize.width;
    frameStr.frameFormat = VPL_FORMAT_BGR;

    // run VPL FD
    vplRet = (*handleVplArray)(m_vplHandle, &frameStr, 0, inOutFaces, faceNumber);

    if (vplRet != VPL_RETURN_SUCCESS) {
        return UNKNOWN_ERROR;
    }

    // prepare ROI rect
    RectangleStr roi_boundaries;
    roi_boundaries.topLeft.x = (unsigned int)(rectDimensions[PLUGIN_ARRAY_RECT_X]);
    roi_boundaries.topLeft.y = (unsigned int)(rectDimensions[PLUGIN_ARRAY_RECT_Y]);
    roi_boundaries.width = (unsigned int)(rectDimensions[PLUGIN_ARRAY_RECT_W]);
    roi_boundaries.height = (unsigned int)(rectDimensions[PLUGIN_ARRAY_RECT_H]);

    // find faces in ROI
    for (size_t index = 0; index < faceNumber; index++) {
        struct VPL_FacesStr facesStr = inOutFaces[index];
        if (clipToRoi(facesStr, roi_boundaries)) {
            PLUGIN_LOGV("[F0 B0]param facesIn:ID %d Score %f rectangle [(%02f,%02f) height=%02f,width=%02f]"
                        "yaw %f pitch %f processType %02f rotation %d",
                        facesStr.id, facesStr.score, facesStr.rectangle.topLeft.x,
                        facesStr.rectangle.topLeft.y, facesStr.rectangle.height, facesStr.rectangle.width,
                        facesStr.yaw, facesStr.pitch, facesStr.rotation, facesStr.processType);
            ((Array_vpl_int32_t)(*map)[PLUGIN_NFD_FACE_ID])[numFacesInRoi]          = facesStr.id;                                              // ID
            ((Array_vpl_float_t)(*map)[PLUGIN_NFD_FACE_SCORE])[numFacesInRoi]       = facesStr.score;                                           // SCORE0
            ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[numFacesInRoi][0]      = facesStr.rectangle.topLeft.x;                             // X1
            ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[numFacesInRoi][1]      = facesStr.rectangle.topLeft.y;                             // Y1
            ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[numFacesInRoi][2]      = facesStr.rectangle.topLeft.x + facesStr.rectangle.width;  // X2
            ((Array_buf_rect_t)(*map)[PLUGIN_NFD_FACE_RECT])[numFacesInRoi][3]      = facesStr.rectangle.topLeft.y + facesStr.rectangle.height; // Y2
            ((Array_vpl_float_t)(*map)[PLUGIN_NFD_FACE_ROTATION])[numFacesInRoi]    = facesStr.rotation;                                        // ROTATION
            ((Array_vpl_float_t)(*map)[PLUGIN_NFD_FACE_YAW])[numFacesInRoi]         = facesStr.yaw;                                             // YAW
            ((Array_vpl_float_t)(*map)[PLUGIN_NFD_FACE_PITCH])[numFacesInRoi]       = facesStr.pitch;                                       // PITCH
            numFacesInRoi++;
        }
    }

    (*map)[PLUGIN_NFD_DETECTED_FACES] = numFacesInRoi;

    return NO_ERROR;
}

