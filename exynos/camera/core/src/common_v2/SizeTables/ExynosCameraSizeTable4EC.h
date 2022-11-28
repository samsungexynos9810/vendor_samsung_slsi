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

#ifndef EXYNOS_CAMERA_LUT_4EC_H
#define EXYNOS_CAMERA_LUT_4EC_H

/* -------------------------
    SIZE_RATIO_16_9 = 0,
    SIZE_RATIO_16_9,
    SIZE_RATIO_1_1,
    SIZE_RATIO_3_2,
    SIZE_RATIO_5_4,
    SIZE_RATIO_5_3,
    SIZE_RATIO_11_9,
    SIZE_RATIO_9_16,
    SIZE_RATIO_18P5_9,
    SIZE_RATIO_END
----------------------------
    RATIO_ID,
    SENSOR_W   = 1,
    SENSOR_H,
    BNS_W,
    BNS_H,
    BCROP_W,
    BCROP_H,
    BDS_W,
    BDS_H,
    TARGET_W,
    TARGET_H,
-----------------------------
    Sensor Margin Width  = 0,
    Sensor Margin Height = 0
-----------------------------*/

static int PREVIEW_SIZE_LUT_4EC[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF  */

    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) ,(1080 + 0) ,   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds    ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int PICTURE_SIZE_LUT_4EC[][SIZE_OF_LUT] =
{
    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) ,(1080 + 0) ,   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds    ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_4EC[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF  */

    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) ,(1080 + 0) ,   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds    ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int PREVIEW_FULL_SIZE_LUT_4EC[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF  */

    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) ,(1080 + 0) ,   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds    ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int PICTURE_FULL_SIZE_LUT_4EC[][SIZE_OF_LUT] =
{
    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) ,(1080 + 0) ,   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds    ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int S5K4EC_YUV_LIST[][SIZE_OF_RESOLUTION] =
{
//    { 1920, 1920, 33331760, SIZE_RATIO_16_9},
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
//    { 1080, 1080, 33331760, SIZE_RATIO_16_9},
//   { 1920,  1080, 33331760, SIZE_RATIO_16_9},
//    { 1920,  1080, 33331760, SIZE_RATIO_16_9},
//    {  1080,  1080, 33331760, SIZE_RATIO_16_9},
//    {  1080,  480, 33331760, SIZE_RATIO_3_2},
//    {  640,  480, 33331760, SIZE_RATIO_16_9},
//    {  320,  240, 33331760, SIZE_RATIO_16_9},
//    {  256,  144, 33331760, SIZE_RATIO_16_9},
};

/* yuv reprocessing input stream size list */
static int S5K4EC_YUV_REPROCESSING_INPUT_LIST[][SIZE_OF_RESOLUTION] =
{
 //   { 1920, 1920, 50000000, SIZE_RATIO_16_9},
};

/* Raw output stream size list */
static int S5K4EC_RAW_OUTPUT_LIST[][SIZE_OF_RESOLUTION] =
{
//    { 1920, 1920, 50000000, SIZE_RATIO_16_9},
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
};

static int S5K4EC_JPEG_LIST[][SIZE_OF_RESOLUTION] =
{
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
};

static int S5K4EC_FPS_RANGE_LIST[][2] =
{
    {   7500,  15000},
    {   15000,  15000},
};

#endif
