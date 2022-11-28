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

/* #define LOG_NDEBUG 0 */
#define LOG_TAG "ExynosCameraFrameFactoryVision"
#include <log/log.h>

#include "ExynosCameraFrameFactoryVision.h"

namespace android {

ExynosCameraFrameFactoryVision::~ExynosCameraFrameFactoryVision()
{
    status_t ret = NO_ERROR;

    if (m_shot_ext != NULL) {
        delete m_shot_ext;
        m_shot_ext = NULL;
    }

    ret = destroy();
    if (ret != NO_ERROR)
        CLOGE("destroy fail");
}

status_t ExynosCameraFrameFactoryVision::create()
{
    Mutex::Autolock lock(ExynosCameraStreamMutex::getInstance()->getStreamMutex());
    CLOGI("");

    status_t ret = NO_ERROR;

    m_setupConfig();
    m_constructPipes();

    /* FLITE pipe initialize */
    ret = m_pipes[INDEX(PIPE_FLITE)]->create(m_sensorIds[PIPE_FLITE]);
    if (ret != NO_ERROR) {
        CLOGE("FLITE create fail, ret(%d)", ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    ret = m_pipes[INDEX(PIPE_SW_MCSC)]->create();
    if (ret != NO_ERROR) {
        CLOGE("PIPE_SW_MCSC create fail, ret(%d)", ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    CLOGD("%s(%d) created", m_pipes[PIPE_FLITE]->getPipeName(), PIPE_FLITE);

    /* s_ctrl HAL version for selecting dvfs table */
    ret = m_pipes[INDEX(PIPE_FLITE)]->setControl(V4L2_CID_IS_HAL_VERSION, IS_HAL_VER_3_2);
    if (ret < 0)
        CLOGW("V4L2_CID_IS_HAL_VERSION is fail");

    ret = m_transitState(FRAME_FACTORY_STATE_CREATE);

    return ret;
}

status_t ExynosCameraFrameFactoryVision::precreate(void)
{
    CLOGI("");

    status_t ret = NO_ERROR;

    m_setupConfig();
    m_constructPipes();

    /* flite pipe initialize */
    ret = m_pipes[INDEX(PIPE_FLITE)]->create(m_sensorIds[PIPE_FLITE]);
    if (ret != NO_ERROR) {
        CLOGE("FLITE create fail, ret(%d)", ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("%s(%d) created", m_pipes[INDEX(PIPE_FLITE)]->getPipeName(), PIPE_FLITE);

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryVision::postcreate(void)
{
    CLOGI("");
    status_t ret = NO_ERROR;

    /* s_ctrl HAL version for selecting dvfs table */
    ret = m_pipes[INDEX(PIPE_FLITE)]->setControl(V4L2_CID_IS_HAL_VERSION, IS_HAL_VER_3_2);
    if (ret < 0)
        CLOGW("V4L2_CID_IS_HAL_VERSION is fail");

    ret = m_transitState(FRAME_FACTORY_STATE_CREATE);

    return ret;
}

/* Override destroy() to use EOS lock.
 * Sequence for closing vision's video node must not be run at the same time with other's video node open sequence
 * because current driver has some limitation for vision instance.
 */
status_t ExynosCameraFrameFactoryVision::destroy(void)
{
    Mutex::Autolock lock(ExynosCameraStreamMutex::getInstance()->getStreamMutex());
    status_t ret = NO_ERROR;

    ret = ExynosCameraFrameFactoryBase::destroy();
    return ret;
}

status_t ExynosCameraFrameFactoryVision::m_fillNodeGroupInfo(ExynosCameraFrameSP_sptr_t frame)
{
    status_t ret = NO_ERROR;
    camera2_node_group node_group_info_flite;
    camera2_node_group *node_group_info_temp;
    ExynosRect sensorSize;

    int pipeId = -1;
    uint32_t perframePosition = 0;
    int nodePipeId = -1;

    int yuvFormat[ExynosCameraParameters::YUV_MAX] = {0};
    int yuvIndex = -1;

    for (int i = ExynosCameraParameters::YUV_0;
        i < ExynosCameraParameters::YUV_MAX; i++) {
        yuvFormat[i] = m_configurations->getYuvFormat(i);
    }
    int bayerFormat = V4L2_PIX_FMT_YVYU;

    memset(&node_group_info_flite, 0x0, sizeof(camera2_node_group));

     /* FLITE */
    pipeId = PIPE_FLITE;
    perframePosition = PERFRAME_BACK_VC0_POS;

    node_group_info_temp = &node_group_info_flite;
    node_group_info_temp->leader.request = 1;
    node_group_info_temp->leader.pixelformat = bayerFormat;

    node_group_info_temp->capture[perframePosition].request = m_request[PIPE_VC0];
    node_group_info_temp->capture[perframePosition].vid = m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_VC0)] - FIMC_IS_VIDEO_BAS_NUM;
    node_group_info_temp->capture[perframePosition].pixelformat = bayerFormat;

    sensorSize.x = 0;
    sensorSize.y = 0;
    {
        sensorSize.w = VISION_WIDTH;
        sensorSize.h = VISION_HEIGHT;
    }

    setLeaderSizeToNodeGroupInfo(&node_group_info_flite, sensorSize.x, sensorSize.y, sensorSize.w, sensorSize.h);
    setCaptureSizeToNodeGroupInfo(&node_group_info_flite, perframePosition, sensorSize.w, sensorSize.h);

    frame->storeNodeGroupInfo(&node_group_info_flite, PERFRAME_INFO_FLITE);
#if defined(USE_SW_MCSC) && (USE_SW_MCSC == true)
    nodePipeId = PIPE_SW_MCSC;
    if (m_request[nodePipeId]) {
      ExynosRect inputSize, outputSize, ratioCropSize;
      camera2_node_group node_group_info_sw_mcsc;
      memset(&node_group_info_sw_mcsc, 0x0, sizeof(camera2_node_group));

      perframePosition = 0;
      int count = 3;
      int swMCSCyuvFormat[FIMC_IS_VIDEO_M5P_NUM] = {0};
      int yuvWidth[ExynosCameraParameters::YUV_MAX] = {0};
      int yuvHeight[ExynosCameraParameters::YUV_MAX] = {0};
      int onePortId = m_configurations->getOnePortId();
      int secondPortId = m_configurations->getSecondPortId();

      node_group_info_sw_mcsc.leader.pixelformat = bayerFormat;
        //  yuvFormat[(onePortId % ExynosCameraParameters::YUV_MAX) +
        //            ExynosCameraParameters::YUV_0];

      for (int i = 0; i < count; i++) {
        swMCSCyuvFormat[i] = yuvFormat[i];
        m_parameters->getSize(HW_INFO_HW_YUV_SIZE, (uint32_t *)&yuvWidth[i],
                              (uint32_t *)&yuvHeight[i], i);
      }
      for (int i=0; i < 2; i++) {
        if (i == 0 && onePortId < 0) continue;
        if (i == 1 && secondPortId < 0) continue;
        node_group_info_sw_mcsc.capture[i].request = m_request[INDEX(nodePipeId)];
        CLOGV("node_group_info_sw_mcsc.capture[%d].request = %d", i,
                node_group_info_sw_mcsc.capture[i].request);
        node_group_info_sw_mcsc.capture[secondPortId].vid =
            (FIMC_IS_VIDEO_M0P_NUM + i) - FIMC_IS_VIDEO_BAS_NUM;
        node_group_info_sw_mcsc.capture[i].pixelformat =
            swMCSCyuvFormat[i];

        inputSize.x = 0;
        inputSize.y = 0;
        inputSize.w = VISION_WIDTH;
        inputSize.h = VISION_HEIGHT;
        outputSize.x = 0;
        outputSize.y = 0;
        outputSize.w = yuvWidth[i];
        outputSize.h = yuvHeight[i];

        ret = getCropRectAlign(inputSize.w, inputSize.h, outputSize.w, outputSize.h,
                            &ratioCropSize.x, &ratioCropSize.y, &ratioCropSize.w,
                            &ratioCropSize.h, CAMERA_MCSC_ALIGN, 2, 1.0f);
        if (ret != NO_ERROR) {
            CLOGE2("getCropRectAlign failed. MCSC in_crop %dx%d, MCSC%d out_size "
                "%dx%d",
                inputSize.w, inputSize.h, secondPortId, outputSize.w,
                outputSize.h);
            ratioCropSize.x = 0;
            ratioCropSize.y = 0;
            ratioCropSize.w = inputSize.w;
            ratioCropSize.h = inputSize.h;
        }
        node_group_info_sw_mcsc.capture[i].input.cropRegion[0] =
            ratioCropSize.x;
        node_group_info_sw_mcsc.capture[i].input.cropRegion[1] =
            ratioCropSize.y;
        node_group_info_sw_mcsc.capture[i].input.cropRegion[2] =
            ratioCropSize.w;
        node_group_info_sw_mcsc.capture[i].input.cropRegion[3] =
            ratioCropSize.h;
        node_group_info_sw_mcsc.capture[i].output.cropRegion[0] =
            outputSize.x;
        node_group_info_sw_mcsc.capture[i].output.cropRegion[1] =
            outputSize.y;
        node_group_info_sw_mcsc.capture[i].output.cropRegion[2] =
            outputSize.w;
        node_group_info_sw_mcsc.capture[i].output.cropRegion[3] =
            outputSize.h;
        CLOGV("PIPE_SW_MCSC intput cropRegion (%d,%d,%d,%d) output cropRegion "
                "(%d,%d,%d,%d)",
                ratioCropSize.x, ratioCropSize.y, ratioCropSize.w, ratioCropSize.h,
                outputSize.x, outputSize.y, outputSize.w, outputSize.h);
       }

      updateNodeGroupInfo(PIPE_SW_MCSC, frame, &node_group_info_sw_mcsc);
      frame->storeNodeGroupInfo(&node_group_info_sw_mcsc, PERFRAME_INFO_SWMCSC);
    }
#endif


    return NO_ERROR;
}

ExynosCameraFrameSP_sptr_t ExynosCameraFrameFactoryVision::createNewFrame(uint32_t frameCount, __unused bool useJpegFlag)
{
    status_t ret = NO_ERROR;
    ExynosCameraFrameEntity *newEntity[MAX_NUM_PIPES] = {0};
    ExynosCameraFrameSP_sptr_t frame = NULL;
    int requestEntityCount = 0;
    int pipeId = -1;

    if (frameCount <= 0) {
        frameCount = m_frameCount;
    }

    frame = m_frameMgr->createFrame(m_configurations, frameCount, FRAME_TYPE_PREVIEW);
    if (frame == NULL) {
        CLOGE("frame is NULL");
        return NULL;
    }

    m_setBaseInfoToFrame(frame);

    ret = m_initFrameMetadata(frame);
    if (ret != NO_ERROR)
        CLOGE("(%s[%d]):frame(%d) metadata initialize fail", __FUNCTION__, __LINE__, m_frameCount);

    /* set flite pipe to linkageList */
    newEntity[INDEX(PIPE_FLITE)] = new ExynosCameraFrameEntity(PIPE_FLITE, ENTITY_TYPE_INPUT_ONLY, ENTITY_BUFFER_FIXED);
    frame->addSiblingEntity(NULL, newEntity[INDEX(PIPE_FLITE)]);
    frame->setPipeIdForResultUpdate(ExynosCameraFrame::RESULT_UPDATE_TYPE_PARTIAL, (enum pipeline)PIPE_FLITE);
    frame->setPipeIdForResultUpdate(ExynosCameraFrame::RESULT_UPDATE_TYPE_BUFFER, (enum pipeline)PIPE_FLITE);
    frame->setPipeIdForResultUpdate(ExynosCameraFrame::RESULT_UPDATE_TYPE_ALL, (enum pipeline)PIPE_FLITE);
    requestEntityCount++;

#if defined(USE_SW_MCSC) && (USE_SW_MCSC == true)
        int flipHorizontal = 0;
        int flipVertical = 0;

#ifdef SUPPORT_PERFRAME_FLIP
        flipHorizontal = m_configurations->getModeValue(CONFIGURATION_PERFRAME_FLIP_H_PICTURE);
        flipVertical = m_configurations->getModeValue(CONFIGURATION_PERFRAME_FLIP_V_PICTURE);
#else
        flipHorizontal = m_configurations->getModeValue(CONFIGURATION_FLIP_HORIZONTAL);
        flipVertical = m_configurations->getModeValue(CONFIGURATION_FLIP_VERTICAL);
#endif

        pipeId = PIPE_SW_MCSC;
        if (m_request[pipeId] == true) {
            frame->setMode(FRAME_MODE_SWMCSC, true);

            newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_OUTPUT, ENTITY_BUFFER_FIXED);
            frame->addSiblingEntity(NULL, newEntity[pipeId]);
            frame->setPipeIdForResultUpdate(ExynosCameraFrame::RESULT_UPDATE_TYPE_BUFFER, (enum pipeline)pipeId);
            frame->setPipeIdForResultUpdate(ExynosCameraFrame::RESULT_UPDATE_TYPE_ALL, (enum pipeline)pipeId);
            frame->setFlipHorizontal(pipeId, flipHorizontal);
            frame->setFlipVertical(pipeId, flipVertical);
            requestEntityCount++;
            for(int i = PIPE_MCSC0 ; i < PIPE_MCSC5; i++) {
                pipeId = i;
                newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_OUTPUT, ENTITY_BUFFER_FIXED);
                frame->addSiblingEntity(NULL, newEntity[pipeId]);
                requestEntityCount++;
            }
        }
#endif


    ret = m_initPipelines(frame);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):m_initPipelines fail, ret(%d)", __FUNCTION__, __LINE__, ret);
    }

    /* TODO: make it dynamic */
    frame->setNumRequestPipe(requestEntityCount);

    m_fillNodeGroupInfo(frame);

    m_frameCount++;

    return frame;
}

status_t ExynosCameraFrameFactoryVision::initPipes(void)
{
    CLOGI("");

    status_t ret = NO_ERROR;
    camera_pipe_info_t pipeInfo[MAX_NODE];
    camera_pipe_info_t nullPipeInfo;
    enum NODE_TYPE nodeType = INVALID_NODE;
    enum NODE_TYPE leaderNodeType = OUTPUT_NODE;
    int pipeId = -1;
    int bufferCount = 0;
    uint32_t frameRate = 15;

    ExynosRect tempRect;
    int hwSensorW = 0, hwSensorH = 0;
    int bayerFormat = 0;
    int perFramePos = 0;

    struct ExynosConfigInfo *config = m_configurations->getConfig();

    pipeId = PIPE_FLITE;

    {
        hwSensorW = VISION_WIDTH;
        hwSensorH = VISION_HEIGHT;
        bayerFormat = V4L2_PIX_FMT_YVYU;
        bufferCount = NUM_BAYER_BUFFERS;
    }

    /* setParam for Frame rate : must after setInput on Flite */
    struct v4l2_streamparm streamParam;
    memset(&streamParam, 0x0, sizeof(v4l2_streamparm));

    streamParam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    streamParam.parm.capture.timeperframe.numerator   = 1;
    streamParam.parm.capture.timeperframe.denominator = frameRate;
    CLOGI("Set framerate (denominator=%d)", frameRate);

    //ret = setParam(&streamParam, pipeId);
    if (ret != NO_ERROR) {
        CLOGE("FLITE setParam(frameRate(%d), pipeId(%d)) fail", frameRate, pipeId);
        return INVALID_OPERATION;
    }

    ret = m_setSensorSize(pipeId, hwSensorW, hwSensorH);
    if (ret != NO_ERROR) {
        CLOGE("m_setSensorSize(pipeId(%d), hwSensorW(%d), hwSensorH(%d)) fail", pipeId, hwSensorW, hwSensorH);
        return ret;
    }

    /* FLITE */
    nodeType = getNodeType(PIPE_FLITE);

    /* set v4l2 buffer size */
    tempRect.fullW = 32;
    tempRect.fullH = 64;
    tempRect.colorFormat = bayerFormat;

    /* set v4l2 video node buffer count */
    pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_3aa_buffers;

    /* Set output node default info */
    SET_OUTPUT_DEVICE_BASIC_INFO(PERFRAME_INFO_FLITE);

    /* BAYER */
    nodeType = getNodeType(PIPE_VC0);
    perFramePos = PERFRAME_BACK_VC0_POS;

    /* set v4l2 buffer size */
    tempRect.fullW = hwSensorW;
    tempRect.fullH = hwSensorH;
    tempRect.colorFormat = bayerFormat;

    /* set v4l2 video node bytes per plane */
    pipeInfo[nodeType].bytesPerPlane[0] = getBayerLineSize(tempRect.fullW, bayerFormat);

    /* set v4l2 video node buffer count */
    pipeInfo[nodeType].bufInfo.count = bufferCount;

    /* Set capture node default info */
    SET_CAPTURE_DEVICE_BASIC_INFO();

    ret = m_pipes[pipeId]->setupPipe(pipeInfo, m_sensorIds[pipeId]);
    if (ret != NO_ERROR) {
        CLOGE("Flite setupPipe fail, ret(%d)", ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    m_frameCount = 0;

    ret = m_transitState(FRAME_FACTORY_STATE_INIT);

    return ret;
}

status_t ExynosCameraFrameFactoryVision::preparePipes(void)
{
    CLOGI("");

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryVision::startPipes(void)
{
    status_t ret = NO_ERROR;

    CLOGI("");

    ret = m_pipes[INDEX(PIPE_FLITE)]->start();
    if (ret < 0) {
        CLOGE("FLITE start fail, ret(%d)", ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

#if defined (USE_SW_MCSC) && (USE_SW_MCSC == true)
    if (m_pipes[PIPE_SW_MCSC] != NULL) {
        ret = m_pipes[PIPE_SW_MCSC]->start();
        if (ret < 0) {
            CLOGE("PIPE_SW_MCSC start fail, ret(%d)", ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }
#endif

    ret = m_pipes[INDEX(PIPE_FLITE)]->prepare();
    if (ret < 0) {
        CLOGE("FLITE prepare fail, ret(%d)", ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    ret = m_pipes[INDEX(PIPE_FLITE)]->sensorStream(true);
    if (ret < 0) {
        CLOGE("FLITE sensorStream on fail, ret(%d)", ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    ret = m_transitState(FRAME_FACTORY_STATE_RUN);
    if (ret != NO_ERROR) {
        CLOGE("Failed to transitState. ret %d", ret);
        return ret;
    }

    CLOGI("Starting Vision Success!");

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryVision::startInitialThreads(void)
{
    status_t ret = NO_ERROR;

    CLOGI("start pre-ordered initial pipe thread");

    ret = startThread(PIPE_FLITE);
    if (ret < 0)
        return ret;

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryVision::setStopFlag(void)
{
    CLOGI("");

    status_t ret = NO_ERROR;

    ret = m_pipes[PIPE_FLITE]->setStopFlag();

#if defined(USE_SW_MCSC) && (USE_SW_MCSC == true)
    if (m_pipes[PIPE_SW_MCSC] != NULL && m_pipes[PIPE_SW_MCSC]->flagStart() == true)
        ret = m_pipes[PIPE_SW_MCSC]->setStopFlag();
#endif


    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryVision::stopPipes(void)
{
    status_t ret = NO_ERROR;
    status_t funcRet = NO_ERROR;

    CLOGI("");

    if (m_pipes[PIPE_FLITE]->isThreadRunning() == true) {
        ret = m_pipes[PIPE_FLITE]->stopThread();
        if (ret != NO_ERROR) {
            CLOGE("PIPE_FLITE stopThread fail, ret(%d)", ret);
            /* TODO: exception handling */
            funcRet |= ret;
        }
    }

#if defined(USE_SW_MCSC) && (USE_SW_MCSC == true)
    if (m_pipes[PIPE_SW_MCSC]->isThreadRunning() == true) {
        ret = m_pipes[PIPE_SW_MCSC]->stopThread();
        if (ret != NO_ERROR) {
            CLOGE("SW_MCSC stopThread fail. ret(%d)", ret);
            funcRet |= ret;
        }
    }
#endif
    

    ret = m_pipes[INDEX(PIPE_FLITE)]->sensorStream(false);
    if (ret < 0) {
        CLOGE("FLITE sensorStream fail, ret(%d)", ret);
        /* TODO: exception handling */
        funcRet |= ret;
    }

    /* PIPE_FLITE force done */
    ret = m_pipes[INDEX(PIPE_FLITE)]->forceDone(V4L2_CID_IS_FORCE_DONE, 0x1000);
    if (ret != NO_ERROR) {
        CLOGE("PIPE_FLITE force done fail, ret(%d)", ret);
        /* TODO: exception handling */
        funcRet |= ret;
    }

#if defined(USE_SW_MCSC) && (USE_SW_MCSC == true)
    if (m_pipes[PIPE_SW_MCSC] != NULL) {
        ret = m_pipes[PIPE_SW_MCSC]->stop();
        CLOGV("previewVendor: m_pipes[PIPE_SW_MCSC]->stop() is called");
        if (ret != NO_ERROR) {
            CLOGE("SW_MCSC stop fail. ret(%d)", ret);
            funcRet |= ret;
        }
    }
#endif


    ret = m_pipes[INDEX(PIPE_FLITE)]->stop();
    if (ret < 0) {
        CLOGE("FLITE stop fail, ret(%d)", ret);
        /* TODO: exception handling */
        funcRet |= ret;
    }

    ret = m_transitState(FRAME_FACTORY_STATE_CREATE);
    if (ret != NO_ERROR) {
        CLOGE("Failed to transitState. ret %d", ret);
        funcRet |= ret;
    }

    CLOGI("Stopping Vsion Success!");

    return funcRet;
}

status_t ExynosCameraFrameFactoryVision::m_setupConfig()
{
    CLOGI("");

    bool enableSecure = false;
    int pipeId = -1;
    enum NODE_TYPE nodeType = INVALID_NODE;
    bool flagStreamLeader = true;
    bool flagReprocessing = false;
    int sensorScenario = 0;

    m_initDeviceInfo(PIPE_FLITE);

#if defined(USE_SW_MCSC) && (USE_SW_MCSC == true)
    m_initDeviceInfo(PIPE_SW_MCSC);
#endif


    pipeId = PIPE_FLITE;
    nodeType = getNodeType(PIPE_FLITE);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_FLITE;
    if (enableSecure == true) {
        m_deviceInfo[pipeId].nodeNum[nodeType] = SECURE_CAMERA_FLITE_NUM;
    } else {
        m_deviceInfo[pipeId].nodeNum[nodeType] = VISION_CAMERA_FLITE_NUM;
    }
    m_deviceInfo[pipeId].connectionMode[nodeType] = HW_CONNECTION_MODE_OTF;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "FLITE", EXYNOS_CAMERA_NAME_STR_SIZE - 1);

    sensorScenario = SENSOR_SCENARIO_VISION;

    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[pipeId].nodeNum[nodeType], false, flagStreamLeader, flagReprocessing, sensorScenario);

    flagStreamLeader = false;

    /* VC0 for bayer */
    nodeType = getNodeType(PIPE_VC0);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_VC0;
    m_deviceInfo[pipeId].bufferManagerType[nodeType] = BUFFER_MANAGER_ION_TYPE;
    m_deviceInfo[pipeId].connectionMode[nodeType] = HW_CONNECTION_MODE_OTF;
    m_deviceInfo[pipeId].nodeNum[nodeType] = getFliteCaptureNodenum(m_cameraId, m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_FLITE)]);
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "BAYER", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_FLITE)], false, flagStreamLeader, flagReprocessing, sensorScenario);

#if defined(USE_SW_MCSC) && (USE_SW_MCSC == true)
    pipeId = PIPE_SW_MCSC;

    nodeType = getNodeType(PIPE_SW_MCSC);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_SW_MCSC;
    m_deviceInfo[pipeId].nodeNum[nodeType] = PREVIEW_GSC_NODE_NUM;
    m_deviceInfo[pipeId].secondaryNodeNum[nodeType] = PREVIEW_GSC_NODE_NUM;
    m_deviceInfo[pipeId].bufferManagerType[nodeType] = BUFFER_MANAGER_ION_TYPE;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "SW_MCSC_OUTPUT", EXYNOS_CAMERA_NAME_STR_SIZE - 1);

    nodeType = getNodeType(PIPE_MCSC0);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_MCSC0;
    m_deviceInfo[pipeId].nodeNum[nodeType] = PREVIEW_GSC_NODE_NUM;
    m_deviceInfo[pipeId].secondaryNodeNum[nodeType] = PREVIEW_GSC_NODE_NUM;
    m_deviceInfo[pipeId].bufferManagerType[nodeType] = BUFFER_MANAGER_SERVICE_GRALLOC_TYPE;
    strncpy(m_deviceInfo[pipeId].secondaryNodeName[nodeType], "SW_MCSC_YUV0", EXYNOS_CAMERA_NAME_STR_SIZE - 1);

    nodeType = getNodeType(PIPE_MCSC1);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_MCSC1;
    m_deviceInfo[pipeId].nodeNum[nodeType] = PREVIEW_GSC_NODE_NUM;
    m_deviceInfo[pipeId].secondaryNodeNum[nodeType] = PREVIEW_GSC_NODE_NUM;
    m_deviceInfo[pipeId].bufferManagerType[nodeType] = BUFFER_MANAGER_SERVICE_GRALLOC_TYPE;
    strncpy(m_deviceInfo[pipeId].secondaryNodeName[nodeType], "SW_MCSC_YUV1", EXYNOS_CAMERA_NAME_STR_SIZE - 1);

    nodeType = getNodeType(PIPE_MCSC2);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_MCSC2;
    m_deviceInfo[pipeId].nodeNum[nodeType] = PREVIEW_GSC_NODE_NUM;
    m_deviceInfo[pipeId].secondaryNodeNum[nodeType] = PREVIEW_GSC_NODE_NUM;
    m_deviceInfo[pipeId].bufferManagerType[nodeType] = BUFFER_MANAGER_SERVICE_GRALLOC_TYPE;
    strncpy(m_deviceInfo[pipeId].secondaryNodeName[nodeType], "SW_MCSC_YUV2", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
#endif


    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryVision::m_constructPipes()
{
    CLOGI("");

    int pipeId = -1;

    /* FLITE */
    pipeId = PIPE_FLITE;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraMCPipe(m_cameraId, m_configurations, m_parameters, false, &m_deviceInfo[pipeId], m_camIdInfo);
    m_pipes[pipeId]->setPipeName("PIPE_FLITE");
    m_pipes[pipeId]->setPipeId(pipeId);

#if defined(USE_SW_MCSC) && (USE_SW_MCSC == true)
    pipeId = PIPE_SW_MCSC;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraPipeSWMCSC(
                m_cameraId,
                m_configurations,
                m_parameters,
                m_flagReprocessing,
                m_nodeNums[pipeId],
                m_camIdInfo,
                &m_deviceInfo[pipeId]);
    m_pipes[pipeId]->setPipeName("PIPE_SW_MCSC");
    m_pipes[pipeId]->setPipeId(PIPE_SW_MCSC);
#endif


    CLOGI("pipe ids for vision");
    for (int i = 0; i < MAX_NUM_PIPES; i++) {
        if (m_pipes[i] != NULL) {
            CLOGI("INFO(%s[%d]):-> m_pipes[%d] : PipeId(%d) - %s", __FUNCTION__, __LINE__ , i, m_pipes[i]->getPipeId(), m_pipes[i]->getPipeName());
        }
    }

    return NO_ERROR;
}

void ExynosCameraFrameFactoryVision::m_init()
{
    m_flagReprocessing = false;

    m_shot_ext = new struct camera2_shot_ext;
}

status_t ExynosCameraFrameFactoryVision::m_initFrameMetadata(ExynosCameraFrameSP_sptr_t frame)
{
    status_t ret = NO_ERROR;

    if (m_shot_ext == NULL) {
        CLOGE("new struct camera2_shot_ext fail");
        return INVALID_OPERATION;
    }

    memset(m_shot_ext, 0x0, sizeof(struct camera2_shot_ext));

    m_shot_ext->shot.magicNumber = SHOT_MAGIC_NUMBER;

    for (int i = 0; i < INTERFACE_TYPE_MAX; i++) {
        m_shot_ext->shot.uctl.scalerUd.mcsc_sub_blk_port[i] = MCSC_PORT_NONE;
    }

    frame->setRequest(PIPE_VC0, m_request[PIPE_VC0]);
#if defined (USE_SW_MCSC) && (USE_SW_MCSC == true)
    frame->setRequest(PIPE_SW_MCSC, m_request[PIPE_SW_MCSC]);
#endif
    frame->setRequest(PIPE_MCSC0, m_request[PIPE_MCSC0]);
    frame->setRequest(PIPE_MCSC1, m_request[PIPE_MCSC1]);
    frame->setRequest(PIPE_MCSC2, m_request[PIPE_MCSC2]);
    m_TNRMode = 0;
    m_bypassFD = 1;

    setMetaTnrMode(m_shot_ext, m_TNRMode);
    setMetaBypassFd(m_shot_ext, m_bypassFD);

    ret = frame->initMetaData(m_shot_ext);
    if (ret != NO_ERROR)
        CLOGE("initMetaData fail");

    return ret;
}

void ExynosCameraFrameFactoryVision::extControl(__unused int pipeId,
                                                 __unused int scenario,
                                                 __unused int controlType,
                                                 __unused void *data)
{
    CLOGV("pipeId(%d), scenario(%d)", pipeId, scenario);
}
}; /* namespace android */

