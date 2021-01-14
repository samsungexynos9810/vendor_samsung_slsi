#ifndef __SEVA_UTILS_HPP
#define __SEVA_UTILS_HPP

#include <sys/stat.h>
#include <sys/types.h>

/**
 * @file utils.hpp
 * @brief Utils class header file
 */

namespace seva
{
namespace graph
{
/**
 * @brief represents a Utils object.
 *
 * Used for utility functions like CreateDirectory
 */
class Utils {
public:
    /**
     * @brief CreateDirectory
     * @param [in] path directory path which is included file name
     */
    static bool CreateDirectory(const char* path);

private:
    static const char* TAG;
};
}
}
#endif
