/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _SECURE_C_H_
#define _SECURE_C_H_

#include <sys/types.h>

class SECURELIB
{
public:
    static void * memcpy ( void * destination, size_t dest_size, const void * source, size_t num );

    static char * strncpy ( char * destination, size_t dest_size, const char * source, size_t num );
    static char * strncat( char *destination, size_t dest_size, const char *source, size_t num );

    //SIZE_MAX
    static size_t strlen ( const char * str );
    static size_t strlen ( const char * str, size_t deflen);


};

#endif //_SECURE_C_H_