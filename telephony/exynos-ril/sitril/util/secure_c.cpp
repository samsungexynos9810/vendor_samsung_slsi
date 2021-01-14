/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "secure_c.h"

#ifndef MIN
#define MIN(a, b)       ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#endif

char * SECURELIB::strncpy ( char * destination, size_t dest_size, const char * source, size_t num )
{
    if ( destination == NULL || source == NULL )
    {
        return destination;
    }

    if ( dest_size == 0 || num == 0 )
    {
        return destination;
    }

    size_t len = ::strnlen(source, num);
    if (len <= num && len < dest_size) {
        ::strncpy(destination, source, len);
        *(destination + len) = 0;
    }

    return destination;
}

void * SECURELIB::memcpy ( void * destination, size_t dest_size, const void * source, size_t num )
{
    if (destination == NULL || source == NULL) {
        return destination;
    }

    if (dest_size == 0 || num == 0 ) {
        return destination;
    }

    if ( num <= dest_size )
    {
        ::memcpy(destination, source, num);
    }

    return destination;
}

//SIZE_MAX
size_t SECURELIB::strlen ( const char * str )
{
    return SECURELIB::strlen(str, SIZE_MAX / sizeof(char));
}

size_t SECURELIB::strlen ( const char * str, size_t deflen)
{
    size_t len = 0;
    if (str != NULL && *str != 0) {
        len = ::strnlen(str, deflen);
    }
    return len;
}

char * SECURELIB::strncat( char *destination, size_t dest_size, const char *source, size_t num )
{
    if (destination == NULL || source == NULL) {
        return destination;
    }

    if (dest_size == 0 || num == 0) {
        return destination;
    }

    size_t dlen = ::strnlen(destination, dest_size);
    size_t slen = ::strnlen(source, num);

    if (dest_size > dlen + slen) {
        ::strncpy(destination + dlen, source, slen);
        *(destination + dlen + slen) = 0;
    }

    return destination;
}