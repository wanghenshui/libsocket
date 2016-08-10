/**
 * Created by Jian Chen
 * @since  2016.04.29
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#ifdef _WIN32

#include <chen/sys/sys.hpp>
#include <Windows.h>

using namespace chen;

// -----------------------------------------------------------------------------
// sys
std::string sys::error()
{
    if (!errno)
        return "No error";

    char buf[1024] = {0};
    return !::strerror_s(buf, sizeof(buf), errno) ? std::string(buf) : "Unknown error";
}

#endif