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
** Unless required by applicable law oR_Agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef EXYNOS_CAMERA_LED_CALIBRATION_MAP_H
#define EXYNOS_CAMERA_LED_CALIBRATION_MAP_H

struct __attribute__((__packed__)) ExynosCameraLEDCalibrationMap
{
    short CheckSum;
    char  Reserved[2];

    int   Cool_LED_Master_R_Avg;
    int   Cool_LED_Master_Gr_Avg;
    int   Cool_LED_Master_Gb_Avg;
    int   Cool_LED_Master_b_Avg;

    int   Cool_LED_Current_R_Avg; // round(R_measure  - BlackLevel(64))
    int   Cool_LED_Current_Gr_Avg;// round(Gr_measure - BlackLevel(64))
    int   Cool_LED_Current_Gb_Avg;// round(Gb_measure - BlackLevel(64))
    int   Cool_LED_Current_b_Avg; // round(b_measure  - BlackLevel(64))

    int   Cool_Warm_LED_Master_R_Avg;
    int   Cool_Warm_LED_Master_GrAvg;
    int   Cool_Warm_LED_Master_Gb_Avg;
    int   Cool_Warm_LED_Master_b_Avg;

    int   Cool_Warm_LED_Current_R_Avg;
    int   Cool_Warm_LED_Current_Gr_Avg;
    int   Cool_Warm_LED_Current_Gb_Avg;
    int   Cool_Warm_LED_Current_b_Avg;

    int   Warm_LED_Master_R_Avg;
    int   Warm_LED_Master_Gr_Avg;
    int   Warm_LED_Master_Gb_Avg;
    int   Warm_LED_Master_b_Avg;

    int   Warm_LED_Current_R_Avg;
    int   Warm_LED_Current_Gr_Avg;
    int   Warm_LED_Current_Gb_Avg;
    int   Warm_LED_Current_b_Avg;

    char  AWB_R_G_Min_limit_R1;
    char  AWB_R_G_Max_limit_R2;
    char  AWB_B_G_Min_limit_B1;
    char  AWB_B_G_Max_limit_B2;

    float Cool_LED_Master_R_G_Ratio;
    float Cool_LED_Master_B_G_Ratio;
    float Cool_LED_Master_Gr_Gb_Ratio;

    float Cool_LED_Current_R_G_Ratio;
    float Cool_LED_Current_B_G_Ratio;
    float Cool_LED_Current_Gr_Gb_Ratio;

    float Cool_Warm_LED_Master_R_G_Ratio;
    float Cool_Warm_LED_Master_B_G_Ratio;
    float Cool_Warm_LED_Master_Gr_Gb_Ratio;

    float Cool_Warm_LED_Current_R_G_Ratio;
    float Cool_Warm_LED_Current_B_G_Ratio;
    float Cool_Warm_LED_Current_Gr_Gb_Ratio;

    float Warm_LED_Master_R_G_Ratio;
    float Warm_LED_Master_B_G_Ratio;
    float Warm_LED_Master_Gr_Gb_Ratio;

    float Warm_LED_Current_R_G_Ratio;
    float Warm_LED_Current_B_G_Ratio;
    float Warm_LED_Current_Gr_Gb_Ratio;

    short Black_level;
};

#endif /* EXYNOS_CAMERA_LED_CALIBRATION_MAP_H */
