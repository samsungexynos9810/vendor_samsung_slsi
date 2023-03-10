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

#ifndef EXYNOS_CAMERA_LUT_4H5_H
#define EXYNOS_CAMERA_LUT_4H5_H

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
    Sensor Margin Width  = 16,
    Sensor Margin Height = 10
-----------------------------*/

static int PREVIEW_SIZE_LUT_4H5[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF */

    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (3264 + 16),(1836 + 10),   /* [sensor ] */
      3280      , 1846      ,   /* [bns    ] */
      3264      , 1836      ,   /* [bcrop  ] */
      3264      , 1836      ,   /* [bds    ] */
      1920      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_4_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2448      , 2448      ,   /* [bcrop  ] */
      2448      , 2448      ,   /* [bds    ] */
      1088      , 1088      ,   /* [target ] */
    },
    /*  3:2 (Single, Dual) */
    { SIZE_RATIO_3_2,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2176      ,   /* [bcrop  ] */
      3264      , 2176      ,   /* [bds    ] */
      1616      , 1080      ,   /* [target ] */
    },
    /*  5:4 (Single, Dual) */
    { SIZE_RATIO_5_4,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3060      , 2448      ,   /* [bcrop  ] */
      3056      , 2448      ,   /* [bds    ] *//* w=3060, Reduced for 16 pixel align */
      1344      , 1080      ,   /* [target ] */
    },
    /*  5:3 (Single, Dual) */
    { SIZE_RATIO_5_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 1958      ,   /* [bcrop  ] */
      3264      , 1958      ,   /* [bds    ] */
      1792      , 1080      ,   /* [target ] */
    },
    /*  11:9 (Single, Dual) */
    { SIZE_RATIO_11_9,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2992      , 2448      ,   /* [bcrop  ] */
      1312      , 1080      ,   /* [bds    ] *//* w=1320, Reduced for 16 pixel align */
      1312      , 1080      ,   /* [target ] */
    }
};

static int PREVIEW_SIZE_LUT_4H5_FULL_OTF[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = 1080p */

    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (3264 + 16),(1836 + 10),   /* [sensor ] */
      3280      , 1846      ,   /* [bns    ] */
      3264      , 1836      ,   /* [bcrop  ] */
      3264      , 1836      ,   /* [bds    ] */
      1920      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_4_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2448      , 2448      ,   /* [bcrop  ] */
      2448      , 2448      ,   /* [bds    ] *//* w=1080, Increased for 16 pixel align */
      1088      , 1088      ,   /* [target ] */
    },
    /*  3:2 (Single, Dual) */
    { SIZE_RATIO_3_2,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2176      ,   /* [bcrop  ] */
      3264      , 2176      ,   /* [bds    ] *//* w=1620, Reduced for 16 pixel align */
      1616      , 1080      ,   /* [target ] */
    },
    /*  5:4 (Single, Dual) */
    { SIZE_RATIO_5_4,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3060      , 2448      ,   /* [bcrop  ] */
      3060      , 2448      ,   /* [bds    ] *//* w=1350, Reduced for 16 pixel align */
      1344      , 1080      ,   /* [target ] */
    },
    /*  5:3 (Single, Dual) */
    { SIZE_RATIO_5_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 1958      ,   /* [bcrop  ] */
      3264      , 1958      ,   /* [bds    ] *//* w=1800, Reduced for 16 pixel align */
      1792      , 1080      ,   /* [target ] */
    },
    /*  11:9 (Single, Dual) */
    { SIZE_RATIO_11_9,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2992      , 2448      ,   /* [bcrop  ] */
      2992      , 2448      ,   /* [bds    ] *//* w=1320, Reduced for 16 pixel align */
      1312      , 1080      ,   /* [target ] */
    }
};

static int PICTURE_SIZE_LUT_4H5[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF */

    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (3264 + 16),(1836 + 10),   /* [sensor ] */
      3280      , 1846      ,   /* [bns    ] */
      3264      , 1836      ,   /* [bcrop  ] */
      3264      , 1836      ,   /* [bds    ] */
      3264      , 1836      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_4_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      3264      , 2448      ,   /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2448      , 2448      ,   /* [bcrop  ] */
      2448      , 2448      ,   /* [bds    ] */
      2448      , 2448      ,   /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_4H5[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = 1080p */

    /*  16:9 (Single) */
    { SIZE_RATIO_16_9,
     (3264 + 16),(1836 + 10),   /* [sensor ] */
      3280      , 1846      ,   /* [bns    ] */
      3264      , 1836      ,   /* [bcrop  ] */
      3264      , 1836      ,   /* [bds    ] */
      1920      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single) */
    { SIZE_RATIO_4_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2448      , 2448      ,   /* [bcrop  ] */
      2448      , 2448      ,   /* [bds    ] */
      1088      , 1088      ,   /* [target ] */
    },
    /*  3:2 (Single, Dual) */
    { SIZE_RATIO_3_2,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2176      ,   /* [bcrop  ] */
      3264      , 2176      ,   /* [bds    ] */
      1616      , 1080      ,   /* [target ] */
    },
    /*  5:4 (Single, Dual) */
    { SIZE_RATIO_5_4,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3060      , 2448      ,   /* [bcrop  ] */
      3056      , 2448      ,   /* [bds    ] *//* w=3060, Reduced for 16 pixel align */
      1344      , 1080      ,   /* [target ] */
    },
    /*  5:3 (Single, Dual) */
    { SIZE_RATIO_5_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 1958      ,   /* [bcrop  ] */
      3264      , 1958      ,   /* [bds    ] */
      1792      , 1080      ,   /* [target ] */
    },
    /*  11:9 (Single, Dual) */
    { SIZE_RATIO_11_9,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2992      , 2448      ,   /* [bcrop  ] */
      1312      , 1080      ,   /* [bds    ] *//* w=1320, Reduced for 16 pixel align */
      1312      , 1080      ,   /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_4H5_FULL_OTF[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = 1080p */

    /*  16:9 (Single) */
    { SIZE_RATIO_16_9,
     (3264 + 16),(1836 + 10),   /* [sensor ] */
      3280      , 1846      ,   /* [bns    ] */
      3264      , 1836      ,   /* [bcrop  ] */
      3264      , 1836      ,   /* [bds    ] */
      1920      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single) */
    { SIZE_RATIO_4_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2448      , 2448      ,   /* [bcrop  ] */
      2448      , 2448      ,   /* [bds    ] *//* w=1080, Increased for 16 pixel align */
      1088      , 1088      ,   /* [target ] */
    },
    /*  3:2 (Single, Dual) */
    { SIZE_RATIO_3_2,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2176      ,   /* [bcrop  ] */
      3264      , 2176      ,   /* [bds    ] *//* w=1620, Reduced for 16 pixel align */
      1616      , 1080      ,   /* [target ] */
    },
    /*  5:4 (Single, Dual) */
    { SIZE_RATIO_5_4,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3060      , 2448      ,   /* [bcrop  ] */
      3060      , 2448      ,   /* [bds    ] *//* w=1350, Reduced for 16 pixel align */
      1344      , 1080      ,   /* [target ] */
    },
    /*  5:3 (Single, Dual) */
    { SIZE_RATIO_5_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 1958      ,   /* [bcrop  ] */
      3264      , 1958      ,   /* [bds    ] *//* w=1800, Reduced for 16 pixel align */
      1792      , 1080      ,   /* [target ] */
    },
    /*  11:9 (Single, Dual) */
    { SIZE_RATIO_11_9,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2992      , 2448      ,   /* [bcrop  ] */
      2992      , 2448      ,   /* [bds    ] *//* w=1320, Reduced for 16 pixel align */
      1312      , 1080      ,   /* [target ] */
    }
};

static int PREVIEW_FULL_SIZE_LUT_4H5[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF */

    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_4_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  3:2 (Single, Dual) */
    { SIZE_RATIO_3_2,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  5:4 (Single, Dual) */
    { SIZE_RATIO_5_4,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  5:3 (Single, Dual) */
    { SIZE_RATIO_5_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  11:9 (Single, Dual) */
    { SIZE_RATIO_11_9,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    }
};

static int PICTURE_FULL_SIZE_LUT_4H5[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF */

    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_4_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      1440      , 1080      ,   /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_60FPS_HIGH_SPEED_4H5[][SIZE_OF_LUT] =
{
    /* Binning   = ON
       BNS ratio = 1.0
       BDS       = ON */

    /*   HD_60  16:9 (Single) */
    { SIZE_RATIO_16_9,
     (1624 + 16),( 914 + 10),   /* [sensor ] *//* Sensor binning ratio = 2 */
      1640      ,  924      ,   /* [bns    ] */
      1624      ,  914      ,   /* [bcrop  ] */
      1280      ,  720      ,   /* [bds    ] */
      1280      ,  720      ,   /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_60FPS_HIGH_SPEED_4H5_FULL_OTF[][SIZE_OF_LUT] =
{
    /* Binning   = ON
       BNS ratio = 1.0
       BDS       = ON */

    /*   HD_60  16:9 (Single) */
    { SIZE_RATIO_16_9,
     (1624 + 16),( 914 + 10),   /* [sensor ] *//* Sensor binning ratio = 2 */
      1640      ,  924      ,   /* [bns    ] */
      1632      ,  918      ,   /* [bcrop  ] */
      1632      ,  918      ,   /* [bds    ] */
      1280      ,  720      ,   /* [target ] */
    },
};

static int VIDEO_SIZE_LUT_120FPS_HIGH_SPEED_4H5[][SIZE_OF_LUT] =
{
    /* Binning   = ON
       BNS ratio = 1.0
       BDS       = ON */

    /*  4:3 (Single) */
    { SIZE_RATIO_4_3,
     ( 800 + 16),( 594 + 10),   /* [sensor ] */
       816      ,  604      ,   /* [bns    ] */
       800      ,  594      ,   /* [bcrop  ] */
       800      ,  594      ,   /* [bds    ] */
       800      ,  594      ,   /* [target ] */
    },
    /*  16:9 (Single) */
    { SIZE_RATIO_16_9,
     ( 800 + 16),( 450 + 10),   /* [sensor ] *//* Sensor binning ratio = 4 */
       816      ,  460      ,   /* [bns    ] */
       800      ,  450      ,   /* [bcrop  ] */
       800      ,  450      ,   /* [bds    ] */
       800      ,  450      ,   /* [target ] */
    }
};

static int S5K4H5_PREVIEW_LIST[][SIZE_OF_RESOLUTION] =
{
#if defined(CAMERA_LCD_SIZE) && (CAMERA_LCD_SIZE >= LCD_SIZE_1920_1080)
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1440, 1080, 33331760, SIZE_RATIO_4_3},
    { 1088, 1088, 33331760, SIZE_RATIO_1_1},
#endif
    { 1280,  720, 33331760, SIZE_RATIO_16_9},
    { 1056,  704, 33331760, SIZE_RATIO_3_2},
    {  960,  720, 33331760, SIZE_RATIO_4_3},
    {  736,  736, 33331760, SIZE_RATIO_1_1},
    {  720,  480, 33331760, SIZE_RATIO_3_2},
    {  640,  480, 33331760, SIZE_RATIO_4_3},
    {  352,  288, 33331760, SIZE_RATIO_11_9},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
};

static int S5K4H5_HIDDEN_PREVIEW_LIST[][SIZE_OF_RESOLUTION] =
{
#if !(defined(CAMERA_LCD_SIZE) && (CAMERA_LCD_SIZE >= LCD_SIZE_1920_1080))
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1440, 1080, 33331760, SIZE_RATIO_4_3},
    { 1088, 1088, 33331760, SIZE_RATIO_1_1},
#endif
    { 3840, 2160, 33331760, SIZE_RATIO_16_9},
#ifdef USE_WQHD_RECORDING
    { 2560, 1440, 33331760, SIZE_RATIO_16_9},
#endif
    { 1600, 1200, 33331760, SIZE_RATIO_4_3},
    { 1280,  960, 33331760, SIZE_RATIO_4_3},
    { 1056,  864, 33331760, SIZE_RATIO_11_9},
    {  720,  720, 33331760, SIZE_RATIO_1_1},
    {  528,  432, 33331760, SIZE_RATIO_11_9},
    {  800,  480, 33331760, SIZE_RATIO_5_3},
    {  672,  448, 33331760, SIZE_RATIO_3_2},
    {  480,  320, 33331760, SIZE_RATIO_3_2},
    {  480,  270, 33331760, SIZE_RATIO_16_9},
};

static int S5K4H5_PICTURE_LIST[][SIZE_OF_RESOLUTION] =
{
    { 3264, 2448, 33331760, SIZE_RATIO_4_3},
    { 3264, 1836, 33331760, SIZE_RATIO_16_9},
    { 2448, 2448, 33331760, SIZE_RATIO_1_1},
    { 2560, 1920, 33331760, SIZE_RATIO_4_3},
    { 2560, 1440, 33331760, SIZE_RATIO_16_9},
    { 2048, 1536, 33331760, SIZE_RATIO_4_3},
    { 2048, 1152, 33331760, SIZE_RATIO_16_9},
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1280,  720, 33331760, SIZE_RATIO_16_9},
    {  960,  720, 33331760, SIZE_RATIO_4_3},
    {  640,  480, 33331760, SIZE_RATIO_4_3},
};

static int S5K4H5_HIDDEN_PICTURE_LIST[][SIZE_OF_RESOLUTION] =
{
    { 3200, 2400, 33331760, SIZE_RATIO_4_3},
    { 3072, 1728, 33331760, SIZE_RATIO_16_9},
    { 2988, 2988, 33331760, SIZE_RATIO_1_1},
    { 2976, 2976, 33331760, SIZE_RATIO_1_1},
    { 2592, 1944, 33331760, SIZE_RATIO_4_3},
    { 2592, 1936, 33331760, SIZE_RATIO_4_3},  /* not exactly matched ratio */
    { 2448, 2448, 33331760, SIZE_RATIO_1_1},
    { 2048, 1536, 33331760, SIZE_RATIO_4_3},
    { 1440, 1080, 33331760, SIZE_RATIO_4_3},
};

static int S5K4H5_THUMBNAIL_LIST[][SIZE_OF_RESOLUTION] =
{
    {  512,  384, 33331760, SIZE_RATIO_4_3},
    {  512,  288, 33331760, SIZE_RATIO_16_9},
    {  384,  384, 33331760, SIZE_RATIO_1_1},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
    {    0,    0, 33331760, SIZE_RATIO_1_1}
};

static int S5K4H5_VIDEO_LIST[][SIZE_OF_RESOLUTION] =
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

static int S5K4H5_HIDDEN_VIDEO_LIST[][SIZE_OF_RESOLUTION] =
{
#ifdef USE_UHD_RECORDING
    { 3840, 2160, 33331760, SIZE_RATIO_16_9}
#endif
};

static int S5K4H5_FPS_RANGE_LIST[][2] =
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

static int S5K4H5_HIDDEN_FPS_RANGE_LIST[][2] =
{
    {  30000,  60000},
    {  60000,  60000},
    {  60000, 120000},
    { 120000, 120000},
};


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
    Sensor Margin Width  = 16,
    Sensor Margin Height = 10
-----------------------------*/

static int PICTURE_SIZE_LUT_4H5_YC[][SIZE_OF_LUT] =
{
    /* Binning   = OFF
       BNS ratio = 1.0
       BDS       = OFF */

    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
     (3264 + 16),(1836 + 10),   /* [sensor ] */
      3280      , 1846      ,   /* [bns    ] */
      3264      , 1836      ,   /* [bcrop  ] */
      3264      , 1836      ,   /* [bds    ] */
      3264      , 1836      ,   /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_4_3,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      3264      , 2448      ,   /* [bcrop  ] */
      3264      , 2448      ,   /* [bds    ] */
      3264      , 2448      ,   /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
     (3264 + 16),(2448 + 10),   /* [sensor ] */
      3280      , 2458      ,   /* [bns    ] */
      2448      , 2448      ,   /* [bcrop  ] */
      2448      , 2448      ,   /* [bds    ] */
      2448      , 2448      ,   /* [target ] */
    }
};

static int S5K4H5_YC_PREVIEW_LIST[][SIZE_OF_RESOLUTION] =
{
#if defined(CAMERA_LCD_SIZE) && (CAMERA_LCD_SIZE >= LCD_SIZE_1920_1080)
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1440, 1080, 33331760, SIZE_RATIO_4_3},
    { 1088, 1088, 33331760, SIZE_RATIO_1_1},
#endif
    { 1280,  720, 33331760, SIZE_RATIO_16_9},
    { 1056,  704, 33331760, SIZE_RATIO_3_2},
    {  960,  720, 33331760, SIZE_RATIO_4_3},
    {  800,  480, 33331760, SIZE_RATIO_5_3},
    {  736,  736, 33331760, SIZE_RATIO_1_1},
    {  720,  480, 33331760, SIZE_RATIO_3_2},
    {  640,  480, 33331760, SIZE_RATIO_4_3},
    {  352,  288, 33331760, SIZE_RATIO_11_9},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
    {  176,  144, 33331760, SIZE_RATIO_11_9},
};

static int S5K4H5_YC_HIDDEN_PREVIEW_LIST[][SIZE_OF_RESOLUTION] =
{
#if !(defined(CAMERA_LCD_SIZE) && (CAMERA_LCD_SIZE >= LCD_SIZE_1920_1080))
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1440, 1080, 33331760, SIZE_RATIO_4_3},
    { 1088, 1088, 33331760, SIZE_RATIO_1_1},
#endif
    { 3840, 2160, 33331760, SIZE_RATIO_16_9},
#ifdef USE_WQHD_RECORDING
    { 2560, 1440, 33331760, SIZE_RATIO_16_9},
#endif
    { 1600, 1200, 33331760, SIZE_RATIO_4_3},
    { 1280,  960, 33331760, SIZE_RATIO_4_3},
    { 1056,  864, 33331760, SIZE_RATIO_11_9},
    {  720,  720, 33331760, SIZE_RATIO_1_1},
    {  528,  432, 33331760, SIZE_RATIO_11_9},
    {  800,  480, 33331760, SIZE_RATIO_5_3},
    {  672,  448, 33331760, SIZE_RATIO_3_2},
    {  480,  320, 33331760, SIZE_RATIO_3_2},
    {  480,  270, 33331760, SIZE_RATIO_16_9},
};

static int S5K4H5_YC_PICTURE_LIST[][SIZE_OF_RESOLUTION] =
{
    { 3264, 2448, 33331760, SIZE_RATIO_4_3},
    { 3264, 1836, 33331760, SIZE_RATIO_16_9},
    { 2448, 2448, 33331760, SIZE_RATIO_1_1},
    { 2560, 1920, 33331760, SIZE_RATIO_4_3},
    { 2560, 1440, 33331760, SIZE_RATIO_16_9},
    { 2048, 1536, 33331760, SIZE_RATIO_4_3},
    { 2048, 1152, 33331760, SIZE_RATIO_16_9},
    { 1920, 1920, 33331760, SIZE_RATIO_1_1},
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1280,  720, 33331760, SIZE_RATIO_16_9},
    {  960,  720, 33331760, SIZE_RATIO_4_3},
    {  640,  480, 33331760, SIZE_RATIO_4_3},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
};

static int S5K4H5_YC_HIDDEN_PICTURE_LIST[][SIZE_OF_RESOLUTION] =
{
    { 3200, 2400, 33331760, SIZE_RATIO_4_3},
    { 3072, 1728, 33331760, SIZE_RATIO_16_9},
    { 2988, 2988, 33331760, SIZE_RATIO_1_1},
    { 2976, 2976, 33331760, SIZE_RATIO_1_1},
    { 2592, 1944, 33331760, SIZE_RATIO_4_3},
    { 2592, 1936, 33331760, SIZE_RATIO_4_3},  /* not exactly matched ratio */
    { 2560, 1920, 33331760, SIZE_RATIO_4_3},
    { 2448, 2448, 33331760, SIZE_RATIO_1_1},
    { 2048, 1536, 33331760, SIZE_RATIO_4_3},
    { 1280,  960, 33331760, SIZE_RATIO_4_3},
};

static int S5K4H5_YC_YUV_LIST[][SIZE_OF_RESOLUTION] =
{
    { 3264, 2448, 33331760, SIZE_RATIO_4_3},
    { 3264, 1836, 33331760, SIZE_RATIO_16_9},
    { 2560, 1920, 33331760, SIZE_RATIO_4_3},
    { 2560, 1440, 33331760, SIZE_RATIO_16_9},
    { 2448, 2448, 33331760, SIZE_RATIO_1_1},
    { 2048, 1536, 33331760, SIZE_RATIO_4_3},
    { 2048, 1152, 33331760, SIZE_RATIO_16_9},
    { 1920, 1920, 33331760, SIZE_RATIO_1_1},
    { 1920, 1080, 33331760, SIZE_RATIO_16_9},
    { 1440, 1080, 33331760, SIZE_RATIO_4_3},
    { 1280,  720, 33331760, SIZE_RATIO_16_9},
    { 1088, 1088, 33331760, SIZE_RATIO_1_1},
    { 1056,  704, 33331760, SIZE_RATIO_3_2},
    {  960,  720, 33331760, SIZE_RATIO_4_3},
    {  800,  480, 33331760, SIZE_RATIO_5_3},
    {  736,  736, 33331760, SIZE_RATIO_1_1},
    {  720,  480, 33331760, SIZE_RATIO_3_2},
    {  640,  480, 33331760, SIZE_RATIO_4_3},
    {  352,  288, 33331760, SIZE_RATIO_11_9},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
    {  176,  144, 33331760, SIZE_RATIO_11_9}
};

static int S5K4H5_YC_THUMBNAIL_LIST[][SIZE_OF_RESOLUTION] =
{
    {  512,  384, 33331760, SIZE_RATIO_4_3},
    {  512,  288, 33331760, SIZE_RATIO_16_9},
    {  384,  384, 33331760, SIZE_RATIO_1_1},
    {  320,  240, 33331760, SIZE_RATIO_4_3},
    {    0,    0, 33331760, SIZE_RATIO_1_1}
};

static int S5K4H5_YC_VIDEO_LIST[][SIZE_OF_RESOLUTION] =
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

static int S5K4H5_YC_HIDDEN_VIDEO_LIST[][SIZE_OF_RESOLUTION] =
{
#ifdef USE_UHD_RECORDING
    { 3840, 2160, 33331760, SIZE_RATIO_16_9},
#endif
#ifdef USE_WQHD_RECORDING
    { 2560, 1440, 33331760, SIZE_RATIO_16_9},
#endif
};

static int S5K4H5_YC_FPS_RANGE_LIST[][2] =
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

static int S5K4H5_YC_HIDDEN_FPS_RANGE_LIST[][2] =
{
    {  30000,  60000},
    {  60000,  60000},
    {  60000, 120000},
    { 120000, 120000},
};

static camera_metadata_rational UNIT_MATRIX_4H5_YC_3X3[] =
{
    {128, 128}, {0, 128}, {0, 128},
    {0, 128}, {128, 128}, {0, 128},
    {0, 128}, {0, 128}, {128, 128}
};

static camera_metadata_rational COLOR_MATRIX1_4H5_YC_3X3[] = {
    {1094, 1024}, {-306, 1024}, {-146, 1024},
    {-442, 1024}, {1388, 1024}, {52, 1024},
    {-104, 1024}, {250, 1024}, {600, 1024}
};

static camera_metadata_rational COLOR_MATRIX2_4H5_YC_3X3[] = {
    {2263, 1024}, {-1364, 1024}, {-145, 1024},
    {-194, 1024}, {1257, 1024}, {-56, 1024},
    {-24, 1024}, {187, 1024}, {618, 1024}
};

static camera_metadata_rational FORWARD_MATRIX1_4H5_YC_3X3[] = {
    {612, 1024}, {233, 1024}, {139, 1024},
    {199, 1024}, {831, 1024}, {-6, 1024},
    {15, 1024}, {-224, 1024}, {1049, 1024}
};

static camera_metadata_rational FORWARD_MATRIX2_4H5_YC_3X3[] = {
    {441, 1024}, {317, 1024}, {226, 1024},
    {29, 1024}, {908, 1024}, {87, 1024},
    {9, 1024}, {-655, 1024}, {1486, 1024}
};

#endif
