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


#include <vector>
#include "VPL_Types.hpp"


#ifdef __cplusplus
extern "C"
{
#endif

	/*!
	\brief Convert vector of legacy HFD face structure to vector of VPL face struct
	\param hfdFacesIn vector of face to convert
	\param vplFaces vector of converted faces
	\return err-t according to success of function.
	*/
	err_t convertHfdFaceStrToVplFaceStr(const FLM_VECTOR<VPL_HfdFaceStr>& hfdFacesIn, FLM_VECTOR<VPL_FacesStr>& vplFaces);

	/*!
	\brief Creates an instance of vpl according to configuration provided
	\param cfgFilePath path for configuration file, if string is empty - function sets default value
	\return Handle to vision pipeline created. NULL if vpl not created.
	*/

	VPL_Handle	createVpl(void);


	VPL_Handle	createVplWithPath(std::string cfgFilePath);

	/*!
	\creates a vpl library instance and begins it's initialization.
	\a call to createVpl would return that instance (regardless of init stage)
	\ use this API in order to start vpl initialization prior to createVpl()
	*/
	void vplPreLoad(void);

	void vplUnload(void);

	void vplPreLoadWithPath(std::string cfgFilePath);


	/*!
	\brief Destroys an instance of vpl
	\param handle Handle to the vpl object
	*/
	void destroyVpl(VPL_Handle handle);

	/*!
	\brief Handle a processing of a single frame
	\param handle Handle to the vpl object
	\param frame pointer to information about frame to process
	\param inOutFaces vector of faces, used for input and output
	\return true if processing was done
	*/
	VPL_RETURN_TYPE vplHandleFrame(
		VPL_Handle handle,
		VPL_FrameStr *frame,
		FLM_VECTOR<VPL_FacesStr>& inOutFaces);

	/*!
	\brief Handle a processing of a single frame
	\param handle Handle to the vpl object
	\param frame pointer to information about frame to process
	\param inOutFaces array of faces, used for input and output
	\param faceNumber number of faces supported
	\return VPL_RETURN_SUCCESS if processing was done successfully
	\return VPL_RETURN_FAILURE if processing failed from any reason
	\return VPL_RETURN_VPL_NOT_READY if processing wasn't done since library is still initializing
	*/
	VPL_RETURN_TYPE vplHandleArray(
		VPL_Handle handle,
		VPL_FrameStr *frame,
		size_t inFaces,
		VPL_FacesStr* inOutFaces,
		size_t & faceNumber);
	/*!
	Init osTools if needed(not initialized by other SWIP)
	*/
	int	 vplOstoolsInit(void);

	/*!
	\brief Exit osTools if needed(not needed by other SWIP)
	*/
	int  vplOstoolsExit(void);


#ifdef __cplusplus
}
#endif
