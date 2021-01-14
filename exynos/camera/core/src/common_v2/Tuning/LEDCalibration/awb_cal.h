/*
 * AWB_M2M
 *
 * Copyright (c) 2015 SLSI, Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics.
 */

#ifndef __AWB_CAL_H
#define __AWB_CAL_H

//------------------------------------------------------------------------------
// Definition for c++ project
//------------------------------------------------------------------------------
#ifdef __cplusplus
    #define EXTERNC extern "C"
#else
    #define EXTERNC
#endif

//------------------------------------------------------------------------------
// Definitions for bayer order
//------------------------------------------------------------------------------
#define BAYER_R_FIRST 0
#define BAYER_GR_FIRST 1
#define BAYER_GB_FIRST 2
#define BAYER_B_FIRST 3

//------------------------------------------------------------------------------
// Definition for default value
//------------------------------------------------------------------------------
#define DEFAULT_BAYER_FIRST    (BAYER_GR_FIRST)
#define DEFAULT_PEDESTAL_VALUE (64)

//------------------------------------------------------------------------------
// Definition for return values
//------------------------------------------------------------------------------
#define SUCCESS	0
#define ERROR_COORDINATE -100
#define ERROR_FILE_OPEN -101

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
int ComputeAvgValue(
	const int bayerWidth,
	const int bayerHeight,
	const int bayerOrder,
          char* bayerMemory,
	const int parabolicCenterX,
	const int parabolicCenterY,
	const int pedestal, /* 64 */
	unsigned short* out
);

#endif