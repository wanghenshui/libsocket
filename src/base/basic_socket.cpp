/**
 * Created by Jian Chen
 * @since  2016.09.11
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/base/basic_socket.hpp"
#include "xio/core/reactor.hpp"
#include "xio/core/ioctl.hpp"
#include "chen/sys/sys.hpp"

// -----------------------------------------------------------------------------
// basic_socket
const int xio::basic_socket::FlagOutOfBand  = MSG_OOB;
const int xio::basic_socket::FlagPeek       = MSG_PEEK;
const int xio::basic_socket::FlagDoNotRoute = MSG_DONTROUTE;
const int xio::basic_socket::FlagWaitAll    = MSG_WAITALL;

xio::basic_socket::basic_socket(std::nullptr_t) noexcept
{
}

xio::basic_socket::basic_socket(int family, int type, int protocol)
{
    this->reset(family, type, protocol);
}

xio::basic_socket::basic_socket(handle_t fd) noexcept
{
    this->reset(fd);
}

xio::basic_socket::basic_socket(handle_t fd, int family, int type, int protocol) noexcept
{
    this->reset(fd, family, type, protocol);
}

xio::basic_socket::~basic_socket() noexcept
{
    this->shutdown();
}

// reset
void xio::basic_socket::reset()
{
    if (!this->_family)
        throw std::runtime_error("socket: reset failed because family is unknown");

#ifdef __linux__
    this->change(::socket(this->_family, this->_type | SOCK_CLOEXEC, this->_protocol));
#else
    this->change(::socket(this->_family, this->_type, this->_protocol));
    ioctl::cloexec(this->native(), true);
#endif

    if (!this->valid())
        throw std::system_error(chen::sys::error(), "socket: failed to create socket");

#ifdef SO_NOSIGPIPE
    // this macro is defined on Unix to prevent SIGPIPE on this socket
    basic_option::set(this->native(), SOL_SOCKET, SO_NOSIGPIPE, 1);
#endif
}

void xio::basic_socket::reset(int family, int type, int protocol)
{
    this->_family   = family;
    this->_type     = type;
    this->_protocol = protocol;

    this->reset();
}

void xio::basic_socket::reset(handle_t fd) noexcept
{
    this->reset(fd, 0, 0, 0);
    this->_type = basic_option::type(fd);
}

void xio::basic_socket::reset(handle_t fd, int family, int type, int protocol) noexcept
{
    this->change(fd);

    this->_family   = family;
    this->_type     = type;
    this->_protocol = protocol;

#ifdef SO_NOSIGPIPE
    // this macro is defined on Unix to prevent SIGPIPE on this socket
    basic_option::set(fd, SOL_SOCKET, SO_NOSIGPIPE, 1);
#endif
}

// connection
std::error_code xio::basic_socket::connect(const basic_address &addr) noexcept
{
    auto storage = addr.sockaddr();
    return !::connect(this->native(), (::sockaddr*)&storage, addr.socklen()) ? std::error_code() : chen::sys::error();
}

std::error_code xio::basic_socket::bind(const basic_address &addr) noexcept
{
    auto storage = addr.sockaddr();
    return !::bind(this->native(), (::sockaddr*)&storage, addr.socklen()) ? std::error_code() : chen::sys::error();
}

std::error_code xio::basic_socket::listen(int backlog) noexcept
{
    return !::listen(this->native(), backlog) ? std::error_code() : chen::sys::error();
}

std::error_code xio::basic_socket::listen() noexcept
{
    return this->listen(SOMAXCONN);
}

std::error_code xio::basic_socket::accept(basic_socket &s) noexcept
{
    handle_t fd = invalid_handle;

    if ((fd = ::accept(this->native(), nullptr, nullptr)) == invalid_handle)
        return chen::sys::error();

    s.reset(fd);
    ioctl::cloexec(fd, true);

    return {};
}

// transmission
xio::ssize_t xio::basic_socket::recv(void *data, std::size_t size) noexcept
{
    return this->recv(data, size, 0);
}

xio::ssize_t xio::basic_socket::recv(void *data, std::size_t size, int flags) noexcept
{
#ifdef MSG_NOSIGNAL
    // this macro is defined on Linux to prevent SIGPIPE on this socket
    flags |= MSG_NOSIGNAL;
#endif

#ifdef _WIN32
    return ::recv(this->native(), (char*)data, static_cast<int>(size), flags);
#else
    return ::recv(this->native(), (char*)data, size, flags);
#endif
}

xio::ssize_t xio::basic_socket::recvfrom(void *data, std::size_t size) noexcept
{
    int flags = 0;

#ifdef MSG_NOSIGNAL
    // this macro is defined on Linux to prevent SIGPIPE on this socket
    flags |= MSG_NOSIGNAL;
#endif

#ifdef _WIN32
    return ::recvfrom(this->native(), (char*)data, static_cast<int>(size), flags, nullptr, nullptr);
#else
    return ::recvfrom(this->native(), (char*)data, size, flags, nullptr, nullptr);
#endif
}

xio::ssize_t xio::basic_socket::recvfrom(void *data, std::size_t size, basic_address &addr) noexcept
{
    return this->recvfrom(data, size, addr, 0);
}

xio::ssize_t xio::basic_socket::recvfrom(void *data, std::size_t size, basic_address &addr, int flags) noexcept
{
#ifdef MSG_NOSIGNAL
    // this macro is defined on Linux to prevent SIGPIPE on this socket
    flags |= MSG_NOSIGNAL;
#endif

    ::sockaddr_storage tmp{};
    socklen_t len = sizeof(tmp);

#ifdef _WIN32
    auto ret = ::recvfrom(this->native(), (char*)data, static_cast<int>(size), flags, (::sockaddr*)&tmp, &len);
#else
    auto ret = ::recvfrom(this->native(), (char*)data, size, flags, (::sockaddr*)&tmp, &len);
#endif

    if (ret >= 0)
        addr.sockaddr((::sockaddr*)&tmp);

    return ret;
}

xio::ssize_t xio::basic_socket::send(const void *data, std::size_t size) noexcept
{
    return this->send(data, size, 0);
}

xio::ssize_t xio::basic_socket::send(const void *data, std::size_t size, int flags) noexcept
{
#ifdef MSG_NOSIGNAL
    // this macro is defined on Linux to prevent SIGPIPE on this socket
    flags |= MSG_NOSIGNAL;
#endif

#ifdef _WIN32
    return ::send(this->native(), (char*)data, static_cast<int>(size), flags);
#else
    return ::send(this->native(), (char*)data, size, flags);
#endif
}

xio::ssize_t xio::basic_socket::sendto(const void *data, std::size_t size, const basic_address &addr) noexcept
{
    return this->sendto(data, size, addr, 0);
}

xio::ssize_t xio::basic_socket::sendto(const void *data, std::size_t size, const basic_address &addr, int flags) noexcept
{
#ifdef MSG_NOSIGNAL
    // this macro is defined on Linux to prevent SIGPIPE on this socket
    flags |= MSG_NOSIGNAL;
#endif

    auto storage = addr.sockaddr();

#ifdef _WIN32
    return ::sendto(this->native(), (char*)data, static_cast<int>(size), flags, (::sockaddr*)&storage, addr.socklen());
#else
    return ::sendto(this->native(), (char*)data, size, flags, (::sockaddr*)&storage, addr.socklen());
#endif
}

// cleanup
void xio::basic_socket::shutdown(Shutdown type) noexcept
{
#ifdef _WIN32
    #define SHUT_RD   SD_RECEIVE
    #define SHUT_WR   SD_SEND
    #define SHUT_RDWR SD_BOTH
#endif

    switch (type)
    {
        case Shutdown::Read:
            ::shutdown(this->native(), SHUT_RD);
            break;

        case Shutdown::Write:
            ::shutdown(this->native(), SHUT_WR);
            break;

        case Shutdown::Both:
            ::shutdown(this->native(), SHUT_RDWR);
            break;
    }
}

void xio::basic_socket::close() noexcept
{
    ev_handle::close();
}

// property
std::error_code xio::basic_socket::peer(basic_address &addr) const noexcept
{
    ::sockaddr_storage tmp{};
    socklen_t len = sizeof(tmp);

    if (::getpeername(this->native(), (::sockaddr*)&tmp, &len) < 0)
        return chen::sys::error();

    addr.sockaddr((::sockaddr*)&tmp);
    return {};
}

std::error_code xio::basic_socket::sock(basic_address &addr) const noexcept
{
    ::sockaddr_storage tmp{};
    socklen_t len = sizeof(tmp);

    if (::getsockname(this->native(), (::sockaddr*)&tmp, &len) < 0)
        return chen::sys::error();

    addr.sockaddr((::sockaddr*)&tmp);
    return {};
}

std::error_code xio::basic_socket::nonblocking(bool enable) noexcept
{
    return ioctl::nonblocking(this->native(), enable);
}

bool xio::basic_socket::valid() const noexcept
{
    return this->native() != invalid_handle;
}

xio::basic_socket::operator bool() const noexcept
{
    return this->valid();
}

std::size_t xio::basic_socket::available() const noexcept
{
    return ioctl::available(this->native());
}

int xio::basic_socket::family() const noexcept
{
    return this->_family;
}

int xio::basic_socket::type() const noexcept
{
    return this->_type;
}

int xio::basic_socket::protocol() const noexcept
{
    return this->_protocol;
}

// notify
void xio::basic_socket::attach(std::function<void (int type)> cb) noexcept
{
    this->_notify = std::move(cb);
}

// event
void xio::basic_socket::onEvent(int type)
{
    auto loop = this->evLoop();
    auto func = this->_notify;

    if (loop && ((type & Closed) || (this->evFlag() & reactor::FlagOnce)))
        loop->del(this);

    if (func)
        func(type);
}
