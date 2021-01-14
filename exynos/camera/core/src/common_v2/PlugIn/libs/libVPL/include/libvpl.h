/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef VPL_H
#define VPL_H

#include <utils/Log.h>
#include <dlfcn.h>

#include "ExynosCameraPlugInUtils.h"
#include "VPL_API.hpp"

#define VPL_LIBRARY_PATH "libvpl.so"

using namespace android;


class VPL
{
public:
    VPL();

    virtual ~VPL();

    //! create
    virtual status_t create(void);

    //! destroy
    virtual status_t  destroy(void);

    //! start
    virtual status_t start(void);

    //! stop
    virtual status_t stop(void);

    //! execute
    virtual status_t  setup(Map_t *map);

    //! execute
    virtual status_t  execute(Map_t *map);

    //! init
    virtual status_t  init(Map_t *map);

    //! query
    virtual status_t  query(Map_t *map);


protected:

    /* Library APIs */
    void*   (*createVplLib)(void);
    void    (*destroyVplLib)(void *handle);
    VPL_RETURN_TYPE    (*handleVplArray)(void *handle, struct VPL_FrameStr *frame, size_t facesin, struct VPL_FacesStr * facesInOut, size_t & faceNumber);

    bool 	clipToRoi(VPL_FacesStr &face, RectangleStr &roi);

    void    *m_vplDl;
    void    *m_vplHandle;
};
#endif //VPL_H
