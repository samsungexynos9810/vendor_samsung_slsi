#ifndef _SENSOR_TYPE_H_
#define _SENSOR_TYPE_H_

typedef enum {
    CAMERA_TYPE_REAR = 0,
    CAMERA_TYPE_FRONT,
    CAMERA_TYPE_WIDE,
    CAMERA_TYPE_TELE,
    CAMERA_TYPE_BOTH_WIDE_TELE,
    CAMERA_TYPE_FRONT_1,
    CAMERA_TYPE_END = 99
} CAMERA_TYPE;

typedef enum {
    SENSOR_NAME_S5K4E6 = 21,
    SENSOR_NAME_S5K3H1 = 31,
    SENSOR_NAME_S5K2L2 = 32,
    SENSOR_NAME_S5K3M3 = 33,
    SENSOR_NAME_IMX260 = 109,
    SENSOR_NAME_IMX320 = 111,
    SENSOR_NAME_IMX333 = 112,
    SENSOR_NAME_S5K3P8 = 44,
    SENSOR_NAME_SR846 = 208,
    SENSOR_NAME_S5K2P6 = 58,
    SENSOR_NAME_END = 999
} SENSOR_TYPE;

typedef enum {
    AP_TYPE_SLSI = 0,
    AP_TYPE_QC,
    AP_TYPE_END = 99
} AP_TYPE;


#endif /// _SENSOR_TYPE_H_
