/**
 * Created by Jian Chen
 * @since  2016.09.09
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include <socket/base/bsd_option.hpp>
#include <socket/base/bsd_socket.hpp>

// -----------------------------------------------------------------------------
// bsd_option
chen::bsd_option::bsd_option(bsd_socket &s) : _socket(s)
{
}

// debug
bool chen::bsd_option::debug() const
{
    return bsd_option::get(SOL_SOCKET, SO_DEBUG) != 0;
}

bool chen::bsd_option::debug(bool val)
{
    return bsd_option::set(SOL_SOCKET, SO_DEBUG, val);
}

// reuseaddr
bool chen::bsd_option::reuseaddr() const
{
    return bsd_option::get(SOL_SOCKET, SO_REUSEADDR) != 0;
}

bool chen::bsd_option::reuseaddr(bool val)
{
    return bsd_option::set(SOL_SOCKET, SO_REUSEADDR, val);
}

// reuseport
bool chen::bsd_option::reuseport() const
{
#ifdef SO_REUSEPORT
    return bsd_option::get(SOL_SOCKET, SO_REUSEPORT) != 0;
#else
    return false;
#endif
}

bool chen::bsd_option::reuseport(bool val)
{
#ifdef SO_REUSEPORT
    return bsd_option::set(SOL_SOCKET, SO_REUSEPORT, val);
#else
    return false;
#endif
}

// keepalive
bool chen::bsd_option::keepalive() const
{
    return bsd_option::get(SOL_SOCKET, SO_KEEPALIVE) != 0;
}

bool chen::bsd_option::keepalive(bool val)
{
    return bsd_option::set(SOL_SOCKET, SO_KEEPALIVE, val);
}

// dontroute
bool chen::bsd_option::dontroute() const
{
    return bsd_option::get(SOL_SOCKET, SO_DONTROUTE) != 0;
}

bool chen::bsd_option::dontroute(bool val)
{
    return bsd_option::set(SOL_SOCKET, SO_DONTROUTE, val);
}

// broadcast
bool chen::bsd_option::broadcast() const
{
    return bsd_option::get(SOL_SOCKET, SO_BROADCAST) != 0;
}

bool chen::bsd_option::broadcast(bool val)
{
    return bsd_option::set(SOL_SOCKET, SO_BROADCAST, val);
}

// linger
struct ::linger chen::bsd_option::linger() const
{
    struct ::linger val{};
    option_t len = sizeof(val);
    return ::getsockopt(this->_socket.native(), SOL_SOCKET, SO_LINGER, (char*)&val, &len), val;
}

bool chen::bsd_option::linger(int onoff, int value)
{
    struct ::linger val{};
    val.l_onoff  = onoff;
    val.l_linger = value;

    return this->linger(val);
}

bool chen::bsd_option::linger(const struct ::linger &val)
{
    return !::setsockopt(this->_socket.native(), SOL_SOCKET, SO_LINGER, (const char*)&val, sizeof(val));
}

// oobinline
bool chen::bsd_option::oobinline() const
{
    return bsd_option::get(SOL_SOCKET, SO_OOBINLINE) != 0;
}

bool chen::bsd_option::oobinline(bool val)
{
    return bsd_option::set(SOL_SOCKET, SO_OOBINLINE, val);
}

// sndbuf
int chen::bsd_option::sndbuf() const
{
    return bsd_option::get(SOL_SOCKET, SO_SNDBUF);
}

bool chen::bsd_option::sndbuf(int val)
{
    return bsd_option::set(SOL_SOCKET, SO_SNDBUF, val);
}

// rcvbuf
int chen::bsd_option::rcvbuf() const
{
    return bsd_option::get(SOL_SOCKET, SO_RCVBUF);
}

bool chen::bsd_option::rcvbuf(int val)
{
    return bsd_option::set(SOL_SOCKET, SO_RCVBUF, val);
}

// sndlowat
int chen::bsd_option::sndlowat() const
{
    return bsd_option::get(SOL_SOCKET, SO_SNDLOWAT);
}

bool chen::bsd_option::sndlowat(int val)
{
    return bsd_option::set(SOL_SOCKET, SO_SNDLOWAT, val);
}

// rcvlowat
int chen::bsd_option::rcvlowat() const
{
    return bsd_option::get(SOL_SOCKET, SO_RCVLOWAT);
}

bool chen::bsd_option::rcvlowat(int val)
{
    return bsd_option::set(SOL_SOCKET, SO_RCVLOWAT, val);
}

// sndtimeo
struct ::timeval chen::bsd_option::sndtimeo() const
{
    struct ::timeval val{};
    option_t len = sizeof(val);
    return ::getsockopt(this->_socket.native(), SOL_SOCKET, SO_SNDTIMEO, (char*)&val, &len), val;
}

bool chen::bsd_option::sndtimeo(int sec, int usec)
{
    struct ::timeval val{};
    val.tv_sec  = sec;
    val.tv_usec = usec;

    return this->sndtimeo(val);
}

bool chen::bsd_option::sndtimeo(const struct ::timeval &time)
{
    return !::setsockopt(this->_socket.native(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&time, sizeof(time));
}

// rcvtimeo
struct ::timeval chen::bsd_option::rcvtimeo() const
{
    struct ::timeval val{};
    option_t len = sizeof(val);
    return ::getsockopt(this->_socket.native(), SOL_SOCKET, SO_RCVTIMEO, (char*)&val, &len), val;
}

bool chen::bsd_option::rcvtimeo(int sec, int usec)
{
    struct ::timeval val{};
    val.tv_sec  = sec;
    val.tv_usec = usec;

    return this->rcvtimeo(val);
}

bool chen::bsd_option::rcvtimeo(const struct ::timeval &time)
{
    return !::setsockopt(this->_socket.native(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&time, sizeof(time));
}

// error
std::error_code chen::bsd_option::error() const
{
    return std::error_code(bsd_option::get(SOL_SOCKET, SO_ERROR), std::generic_category());
}

// type
int chen::bsd_option::type() const
{
    return bsd_option::get(SOL_SOCKET, SO_TYPE);
}

// helper
int chen::bsd_option::get(int level, int name) const
{
    int val = 0;
    option_t len = sizeof(val);
    return ::getsockopt(this->_socket.native(), level, name, (char*)&val, &len), val;
}

bool chen::bsd_option::set(int level, int name, int val)
{
    return !::setsockopt(this->_socket.native(), level, name, (const char*)&val, sizeof(val));
}