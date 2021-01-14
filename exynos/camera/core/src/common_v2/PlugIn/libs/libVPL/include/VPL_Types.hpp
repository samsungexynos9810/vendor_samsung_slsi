/*************************************************************************/
/*        Samsung Israel R&D Center Proprietary and Confidential         */
/*                                                                       */
/*       Copyright (C) 2017 Samsung Israel R&D Center (SIRC), Ltd        */
/*                                                                       */
/*  self document and the information herein are the exclusive property  */
/*  of SIRC Ltd. and shall not be disclosed, in whole or in part,        */
/*  to any third party or utilized for any purpose other than the        */
/*  express purpose for which it has been provided.                      */
/*                                                                       */
/*************************************************************************/
#pragma once


#include <cmath>
#include <string>

#include <vector>
#define FLM_VECTOR std::vector
#define EDIT_FLM_VECTOR_AT at
#define GET_FLM_VECTOR_AT at

#define FLM_MIN(a,b) (((a)<(b))?(a):(b))
#define FLM_MAX(a,b) (((a)>(b))?(a):(b))
#define FLM_CEIL_UINT(x,y) (((x)+(y)-1)/(y))
#define PREPROC_OUTP_ICON_WIDTH_HEIGHT_WITHOUT_MARGIN	142
#define PREPROCESSING_OUTPUT_ICON_WIDTH_HEIGHT		256
#define FLM_MARGIN_FACTOR	0.3

#define PI (3.14159265f)

#define MAX_PP_FACES_ALLOCATION 10
#define MAX_FACE_WIDTH PREPROCESSING_OUTPUT_ICON_WIDTH_HEIGHT
#define MAX_FACE_HEIGHT PREPROCESSING_OUTPUT_ICON_WIDTH_HEIGHT
#define PREPROCESSING_OUTPUT_ICON_SIZE (PREPROCESSING_OUTPUT_ICON_WIDTH_HEIGHT * PREPROCESSING_OUTPUT_ICON_WIDTH_HEIGHT * 3)
#define CNNPROCESS_INPUT_ICON_SIZE PREPROCESSING_OUTPUT_ICON_SIZE
#define CNNPROCESS_INPUT_ICON_WIDTH_HEIGHT PREPROCESSING_OUTPUT_ICON_WIDTH_HEIGHT

#define ASSERT_FALSE_RET(condition) { EXPECT_FALSE(condition); if (condition) return condition; }

typedef void* VPL_Handle;
enum VPL_RETURN_TYPE
{
	VPL_RETURN_SUCCESS,
	VPL_RETURN_FAILURE,
	VPL_RETURN_VPL_NOT_READY,
	VPL_RETURN_INIT_FAILED
};

enum VPL_INIT_STATE
{
	VPL_INIT_STATE_COMPLETED,
	VPL_INIT_STATE_FAILED,
	VPL_INIT_STATE_PRE_INIT,
	VPL_INIT_STATE_INITIALIZING
};
enum FLM_MODEL_TYPES
{
	FLM_MODEL_HS_35_POINTS,
	FLM_MODEL_HQ_101_POINTS_HFD_3D,
	FLM_MODEL_HQ_101_POINTS_HFD_2D,
	FLM_MODEL_HQ_101_POINTS_NFD_3D,
	FLM_MODEL_HQ_101_POINTS_NFD_2D,
	FLM_MODEL_HQ_165_POINTS_HFD_3D,
	FLM_MODEL_HQ_165_POINTS_HFD_2D,
	FLM_MODEL_LIGHTWEIGHT_HFD_3D,
	FLM_MODEL_LIGHTWEIGHT_HFD_2D,
	// to be added later :
	// FLM_MODEL_HQ_165_POINTS_NFD_3D,
	// FLM_MODEL_HQ_165_POINTS_NFD_2D,
	// FLM_MODEL_LIGHTWEIGHT_NFD_3D,
	// FLM_MODEL_LIGHTWEIGHT_NFD_2D,
	NUM_OF_FLM_MODELS
};

/*!
Point to represent a location on the image,
such as FLM, middle point, edges etc.
*/
struct Point {
	int x;		/*!< The point's x position. */
	int y;		/*!< The point's y position. */
};

struct PointFloat {
	float x;		/*!< The point's x position. */
	float y;		/*!< The point's y position. */
};

struct CnnDimensions {
	int batchSize;		/*!<4th dimension batch */
	int depth;			/*!<3rd dimension (Z axis , depth) */
	int height;			/*!< Y axis . */
	int width;  		/*!< X axis. */
};
/*!
Rectangle represented by two points, to define
an area of an image.
*/
struct RectangleStr {
	Point topLeft;		/*!< The rectangle's top left point.*/
	int width;			/*!< The rectangle's width.*/
	int height;			/*!< The rectangle's height.*/
};

struct RectangleFloatStr{
	PointFloat topLeft;		/*!< The rectangle's top left point.*/
	float width;			/*!< The rectangle's width.*/
	float height;			/*!< The rectangle's height.*/
};

//Quit has no meaning currently, as handle frame is blocking so "destroy_flm" cannot be called while handleFrame is working
//however this is a preparation for quit functionality in case multi threading is incorporated

//Add each and every message passed in message queue in this enum
enum FLM_MSG_TYPE {
	FLM_MSG_PP_FACE_TASK_REQUEST,
	FLM_MSG_PP_FACE_TASK_COMPLETED,
	FLM_MSG_PP_NEW_FRAME_TASK_REQUEST,
	FLM_MSG_PP_NEW_FRAME_TASK_COMPLETED,
	FLM_MSG_CNN_TASK_REQUEST,
	FLM_MSG_CNN_TASK_COMPLETED,
	FLM_MSG_NFD_PP_TASK_REQUEST,
	FLM_MSG_NFD_PP_TASK_COMPLETED,
	FLM_MSG_NFD_CNN_TASK_REQUEST,
	FLM_MSG_NFD_CNN_TASK_COMPLETED,
	FLM_MSG_NFD_POST_PROC_TASK_REQUEST,
	FLM_MSG_NFD_POST_PROC_TASK_COMPLETED,
	FLM_MSG_QUIT_PROCESSING,
	FLM_MSG_TYPE_ERR_OCCURED
};

inline float getValInRange(float val, float minVal, float maxVal)
{
	float val1 = fmaxf(val, minVal);
	return fminf(val1, maxVal);
}

#define CLAMP(x, minval, maxval) FLM_MIN(FLM_MAX(x, minval), maxval)

typedef enum
{
	FLM_RGB = 0,
	FLM_BGR = 1
} FLM_RgbType;

typedef enum
{
	FLM_V422 = 1,
	FLM_V420 = 2,
	FLM_V444 = 3
} VPL_YUVType;

typedef enum
{
	PreProcessCnnInputType_RGB,
	PreProcessCnnInputType_BGR,
	PreProcessCnnInputType_YUV420,
	PreProcessCnnInputType_YUV422,
	PreProcessCnnInputType_YUV444_FLOAT,
	PreProcessCnnInputType_YUV444_UCHAR
} PreProcessCnnInputType;

typedef enum
{
	FLM_Y_EXT_PYR = 0,
	FLM_Y_INT_PYR = 1,
	FLM_Y_NO_PYR = 2
} FLM_YMode;

typedef enum
{
	PYRAMIDS_PER_ROI = 0,
	PYRAMIDS_PER_FRAME = 1
} FLM_PyramidMode;

enum cnnEngineModelType
{
	CNN_MODEL_TYPE_NFD,
	CNN_MODEL_TYPE_FLM,
	CNN_MODEL_TYPE_FLM_4_MODELS
};

typedef int err_t;

/*!
This enumeration is used to indicate type of face processing done in FLM
*/
enum VPL_FlmFaceProcessType
{
	FLM_FULL_FACE_PROCESS = 0,	/*!< Full FLM processing  */
	FLM_VIDEO_FACE_PROCESS = 1, /*!< Video mode - not operative */
	FLM_INVALID_FACE_PROCESS
};

/*!
This enumeration is used to indicate type of face processing done in FLM
*/
enum VPL_FrameFormat
{
	VPL_FORMAT_YUV420 = 0,	/*!< YUV420 format  */
	VPL_FORMAT_YUV422 = 1,	/*!< YUV422 format  */
	VPL_FORMAT_RGB = 2,		/*!< RGB format */
	VPL_FORMAT_BGR = 3,		/*!< BGR format */
	VPL_FORMAT_INVALID
};


/*!
Legacy face structure. Provides information about face in terms of HFD/VRA.
Note: this type remains for backwards compatibility.
*/
struct VPL_HfdFaceStr {
	unsigned int	id;			/*!< The face's identification number.*/
	float			score;		/*!< The face's score, given by the FD engine.*/
	int             rotation;	/*!< The face's rotation theta.*/
	int             yaw;		/*!< The face's yaw level.*/
	RectangleStr    rectangle;	/*!< The face's area on the image.*/
	bool			mirrorX;	/*!< Is the face mirrored (true= left side of face turns camera. false=right*/

};
/*!
Face structure for VPL - includes needed information about face from face detection
*/
struct VPL_FacesStr {
	unsigned int			id;			/*!< The face's identification number.*/
	RectangleFloatStr		rectangle;	/*!< The face's area on the image.*/
	float					score;		/*!< The face's score, given by the FD engine.*/
	float					rotation;	/*!< The face's rotation theta.*/
	float					yaw;		/*!< The face's yaw level.*/
	float					pitch;		/*!< The face's pitch level.*/
	FLM_VECTOR<PointFloat>	FLM_points;

	FLM_VECTOR<float> intermediateData;			/*!< For FLM internal usage - Deprecated - will be removed */
	VPL_FlmFaceProcessType processType;			/*!< For FLM internal usage - Deprecated - will be removed */
	FLM_VECTOR<PointFloat> scaled_FLM_points;	/*!< For FLM internal usage - Deprecated - will be removed */
	uint32_t iterationsProcessed;				/*!< For FLM internal usage - Deprecated - will be removed */

};



/*!
VPL_FrameSize, holds the width and the height of a frame. (in pixels)
*/
struct VPL_FrameSize {
	unsigned int width;		/*!< The frame's width.*/
	unsigned int height;	/*!< The frame's height.*/
};

/*!
VPL_FrameStr, holds pointers to input buffers (Y and UV, or R, G, and B), and format (YUV / RGB)
*/
struct VPL_FrameStr {
	unsigned char*	yOrRBuffer;		/*!< The frame's Y or R values.*/
	unsigned int	yOrRLength;		/*!< The frame's Y or R buffer size.*/
	unsigned char*	uvOrGBuffer;	/*!< The frame's UV or G values.*/
	unsigned int	uvOrGLength;	/*!< The frame's UV or G buffer size.*/
	unsigned char*	bBuffer;		/*!< The frame's B values.*/
	unsigned int	bLength;		/*!< The frame's B buffer size.*/
	VPL_FrameSize	frameSize;		/*!< The frame size (in pixels) */
	VPL_FrameFormat	frameFormat;	/*!< Frame format */
};
