/**
 * Created by Jian Chen
 * @since  2016.09.09
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/base/basic_option.hpp"

// -----------------------------------------------------------------------------
// basic_option

// debug
bool xio::basic_option::debug(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_DEBUG) != 0;
}

bool xio::basic_option::debug(handle_t fd, bool val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_DEBUG, val);
}

// reuseaddr
bool xio::basic_option::reuseaddr(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_REUSEADDR) != 0;
}

bool xio::basic_option::reuseaddr(handle_t fd, bool val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_REUSEADDR, val);
}

// reuseport
bool xio::basic_option::reuseport(handle_t fd)
{
#ifdef SO_REUSEPORT
    return basic_option::get(fd, SOL_SOCKET, SO_REUSEPORT) != 0;
#else
    return false;
#endif
}

bool xio::basic_option::reuseport(handle_t fd, bool val)
{
#ifdef SO_REUSEPORT
    return basic_option::set(fd, SOL_SOCKET, SO_REUSEPORT, val);
#else
    return false;
#endif
}

// keepalive
bool xio::basic_option::keepalive(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_KEEPALIVE) != 0;
}

bool xio::basic_option::keepalive(handle_t fd, bool val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_KEEPALIVE, val);
}

// dontroute
bool xio::basic_option::dontroute(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_DONTROUTE) != 0;
}

bool xio::basic_option::dontroute(handle_t fd, bool val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_DONTROUTE, val);
}

// broadcast
bool xio::basic_option::broadcast(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_BROADCAST) != 0;
}

bool xio::basic_option::broadcast(handle_t fd, bool val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_BROADCAST, val);
}

// linger
struct ::linger xio::basic_option::linger(handle_t fd)
{
    struct ::linger val{};
    option_t len = sizeof(val);
    ::getsockopt(fd, SOL_SOCKET, SO_LINGER, (char*)&val, &len);
    return val;
}

bool xio::basic_option::linger(handle_t fd, int onoff, int value)
{
    struct ::linger val{};
    val.l_onoff  = onoff;
    val.l_linger = value;

    return basic_option::linger(fd, val);
}

bool xio::basic_option::linger(handle_t fd, const struct ::linger &val)
{
    return !::setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&val, sizeof(val));
}

// oobinline
bool xio::basic_option::oobinline(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_OOBINLINE) != 0;
}

bool xio::basic_option::oobinline(handle_t fd, bool val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_OOBINLINE, val);
}

// sndbuf
int xio::basic_option::sndbuf(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_SNDBUF);
}

bool xio::basic_option::sndbuf(handle_t fd, int val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_SNDBUF, val);
}

// rcvbuf
int xio::basic_option::rcvbuf(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_RCVBUF);
}

bool xio::basic_option::rcvbuf(handle_t fd, int val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_RCVBUF, val);
}

// sndlowat
int xio::basic_option::sndlowat(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_SNDLOWAT);
}

bool xio::basic_option::sndlowat(handle_t fd, int val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_SNDLOWAT, val);
}

// rcvlowat
int xio::basic_option::rcvlowat(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_RCVLOWAT);
}

bool xio::basic_option::rcvlowat(handle_t fd, int val)
{
    return basic_option::set(fd, SOL_SOCKET, SO_RCVLOWAT, val);
}

// sndtimeo
struct ::timeval xio::basic_option::sndtimeo(handle_t fd)
{
    ::timeval val{};
    option_t len = sizeof(val);
    ::getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&val, &len);
    return val;
}

bool xio::basic_option::sndtimeo(handle_t fd, int sec, int usec)
{
    ::timeval val{};
    val.tv_sec  = sec;
    val.tv_usec = usec;

    return basic_option::sndtimeo(fd, val);
}

bool xio::basic_option::sndtimeo(handle_t fd, const struct ::timeval &time)
{
    return !::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&time, sizeof(time));
}

// rcvtimeo
struct ::timeval xio::basic_option::rcvtimeo(handle_t fd)
{
    ::timeval val{};
    option_t len = sizeof(val);
    ::getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&val, &len);
    return val;
}

bool xio::basic_option::rcvtimeo(handle_t fd, int sec, int usec)
{
    ::timeval val{};
    val.tv_sec  = sec;
    val.tv_usec = usec;

    return basic_option::rcvtimeo(fd, val);
}

bool xio::basic_option::rcvtimeo(handle_t fd, const struct ::timeval &time)
{
    return !::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time, sizeof(time));
}

// error
std::error_code xio::basic_option::error(handle_t fd)
{
    return std::error_code(basic_option::get(fd, SOL_SOCKET, SO_ERROR), std::generic_category());
}

// type
int xio::basic_option::type(handle_t fd)
{
    return basic_option::get(fd, SOL_SOCKET, SO_TYPE);
}

// helper
int xio::basic_option::get(handle_t fd, int level, int name)
{
    int val = 0;
    option_t len = sizeof(val);
    ::getsockopt(fd, level, name, (char*)&val, &len);
    return val;
}

bool xio::basic_option::set(handle_t fd, int level, int name, int val)
{
    return !::setsockopt(fd, level, name, (const char*)&val, sizeof(val));
}