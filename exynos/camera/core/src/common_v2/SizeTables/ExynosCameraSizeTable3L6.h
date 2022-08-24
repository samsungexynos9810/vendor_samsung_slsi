/*
**
** Copyright 2013, Samsung Electronics Co. LTD
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

#ifndef EXYNOS_CAMERA_LUT_3L6_H
#define EXYNOS_CAMERA_LUT_3L6_H

/* -------------------------
    SIZE_RATIO_16_9 = 0,
    SIZE_RATIO_4_3,
    SIZE_RATIO_1_1,
    SIZE_RATIO_3_2,
    SIZE_RATIO_5_4,
    SIZE_RATIO_5_3,
    SIZE_RATIO_11_9,
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

static int PREVIEW_SIZE_LUT_3L6[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = 1080p */
    /*  16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    },
    /* 4:3 (VT_Call) */
    { SIZE_RATIO_4_3,
     (4000 + 0) ,(3000 + 0) ,   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1440      , 1080      ,   /* [bcrop  ] */
      1440      , 1080      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) , (1080 + 0),   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int PREVIEW_SIZE_LUT_3L6_BNS[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 2.0
       BDS       = OFF */
     /*  16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    },
    /* 4:3 (VT_Call) */
    { SIZE_RATIO_4_3,
     (4000 + 0) ,(3000 + 0) ,   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1440      , 1080      ,   /* [bcrop  ] */
      1440      , 1080      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) , (1080 + 0),   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int PICTURE_SIZE_LUT_3L6[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF */
    /*  16:9 (Single, Dual) */
    { SIZE_RATIO_4_3,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      4000      , 3000      ,   /* [bcrop  ] */
      4000      , 3000      ,   /* [bds  ] */
      4000      , 3000      ,   /* [target ] */
    },

    { SIZE_RATIO_16_9,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      4000      , 2250      ,   /* [bcrop  ] */
      4000      , 2250      ,   /* [bds  ] */
      4000      , 2250      ,   /* [target ] */
    },
     /*  16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    },
    /* 4:3 (VT_Call) */
    { SIZE_RATIO_4_3,
     (4000 + 0) ,(3000 + 0) ,   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1440      , 1080      ,   /* [bcrop  ] */
      1440      , 1080      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) , (1080 + 0),   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_3L6[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = 1080p */
    /*  16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    },
    
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) , (1080 + 0),   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_3L6_BNS[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 2.0
       BDS       = 1080p */
    /*  16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) , (1080 + 0),   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int PREVIEW_FULL_SIZE_LUT_3L6[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = 1080p */
    /*  16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    },
    /* 4:3 (VT_Call) */
    { SIZE_RATIO_4_3,
     (4000 + 0) ,(3000 + 0) ,   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1440      , 1080      ,   /* [bcrop  ] */
      1440      , 1080      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) , (1080 + 0),   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    }
};

static int PICTURE_FULL_SIZE_LUT_3L6[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF */
    /*  16:9 (Single, Dual) */
    { SIZE_RATIO_4_3,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      4000      , 3000      ,   /* [bcrop  ] */
      4000      , 3000      ,   /* [bds  ] */
      4000      , 3000      ,   /* [target ] */
    },
    { SIZE_RATIO_16_9,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      4000      , 2250      ,   /* [bcrop  ] */
      4000      , 2250      ,   /* [bds  ] */
      4000      , 2250      ,   /* [target ] */
    },
    { SIZE_RATIO_16_9,
     (4000 + 0) , (3000 + 0),   /* [sensor ] */
      4000      , 3000      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    },
    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (1920 + 0) , (1080 + 0),   /* [sensor ] */
      1920      , 1080      ,   /* [bns    ] */
      1920      , 1080      ,   /* [bcrop  ] */
      1920      , 1080      ,   /* [bds  ] */
      1920      , 1080      ,   /* [target ] */
    }
};



static int S5K3L6_PREVIEW_LIST[][SIZE_OF_RESOLUTION] =
{
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1440, 1080, 33331760, SIZE_RATIO_4_3},
    { 1072, 1072, 33331760, SIZE_RATIO_1_1},
    { 1280,  720, 33331760, SIZE_RATIO_16_9},
    { 1056,  704, 33331760, SIZE_RATIO_3_2},
    {  960,  720, 33331760, SIZE_RATIO_4_3},
    {  880,  720, 33331760, SIZE_RATIO_11_9},
    {  720,  720, 33331760, SIZE_RATIO_1_1},
    {  720,  480, 33331760, SIZE_RATIO_3_2},
    {  640,  480, 33331760, SIZE_RATIO_4_3},
    {  352,  288, 33331760, SIZE_RATIO_11_9},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
};


static int S5K3L6_JPEG_LIST[][SIZE_OF_RESOLUTION] =
{
    { 4000, 2250, 33331760, SIZE_RATIO_16_9},
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1280,  720, 33331760, SIZE_RATIO_16_9},
    {  960,  720, 33331760, SIZE_RATIO_4_3},
    {  640,  480, 33331760, SIZE_RATIO_4_3},
};


static int S5K3L6_THUMBNAIL_LIST[][SIZE_OF_RESOLUTION] =
{
    {  512,  384, 33331760, SIZE_RATIO_4_3},
    {  512,  288, 33331760, SIZE_RATIO_16_9},
    {  384,  384, 33331760, SIZE_RATIO_1_1},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
    {    0,    0, 33331760, SIZE_RATIO_1_1}
};

static int S5K3L6_VIDEO_LIST[][SIZE_OF_RESOLUTION] =
{
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1440, 1080, 33331760, SIZE_RATIO_4_3},
    { 1280,  720, 33331760, SIZE_RATIO_16_9},
    {  960,  720, 33331760, SIZE_RATIO_4_3},
    {  800,  450, 33331760, SIZE_RATIO_16_9},
    {  720,  480, 33331760, SIZE_RATIO_3_2},
    {  640,  480, 33331760, SIZE_RATIO_4_3},
    {  480,  320, 33331760, SIZE_RATIO_3_2},
    {  352,  288, 33331760, SIZE_RATIO_11_9},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
    {  176,  144, 33331760, SIZE_RATIO_11_9}
};

static int S5K3L6_YUV_LIST[][SIZE_OF_RESOLUTION] =
{
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1280,  720, 33331760, SIZE_RATIO_16_9},
    {  960,  720, 33331760, SIZE_RATIO_4_3},
    {  720,  480, 33331760, SIZE_RATIO_3_2},
    {  640,  480, 33331760, SIZE_RATIO_4_3},
    {  352,  288, 33331760, SIZE_RATIO_11_9},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
    {  256,  144, 33331760, SIZE_RATIO_16_9},
};

static int S5K3L6_FPS_RANGE_LIST[][2] =
{
    {   5000,   5000},
    {   7000,   7000},
    {  15000,  15000},
    {  24000,  24000},
    {   4000,  30000},
    {  10000,  30000},
    {  15000,  30000},
    {  30000,  30000},
};

static camera_metadata_rational UNIT_MATRIX_3L6_3X3[] =
{
    {128, 128}, {0, 128}, {0, 128},
    {0, 128}, {128, 128}, {0, 128},
    {0, 128}, {0, 128}, {128, 128}
};

static camera_metadata_rational COLOR_MATRIX1_3L6_3X3[] = {
    {1094, 1024}, {-306, 1024}, {-146, 1024},
    {-442, 1024}, {1388, 1024}, {52, 1024},
    {-104, 1024}, {250, 1024}, {600, 1024}
};

static camera_metadata_rational COLOR_MATRIX2_3L6_3X3[] = {
    {2263, 1024}, {-1364, 1024}, {-145, 1024},
    {-194, 1024}, {1257, 1024}, {-56, 1024},
    {-24, 1024}, {187, 1024}, {618, 1024}
};

static camera_metadata_rational FORWARD_MATRIX1_3L6_3X3[] = {
    {612, 1024}, {233, 1024}, {139, 1024},
    {199, 1024}, {831, 1024}, {-6, 1024},
    {15, 1024}, {-224, 1024}, {1049, 1024}
};

static camera_metadata_rational FORWARD_MATRIX2_3L6_3X3[] = {
    {441, 1024}, {317, 1024}, {226, 1024},
    {29, 1024}, {908, 1024}, {87, 1024},
    {9, 1024}, {-655, 1024}, {1486, 1024}
};

#endif
