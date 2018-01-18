/**
 * Created by Jian Chen
 * @since  2016.09.12
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/inet/inet_resolver.hpp"
#include "chen/base/num.hpp"
#include <cstring>

// -----------------------------------------------------------------------------
// address
xio::inet_address::inet_address(std::nullptr_t)
{
}

xio::inet_address::inet_address(const char *mixed) : inet_address(std::string(mixed))
{
}

xio::inet_address::inet_address(const std::string &mixed)
{
    this->assign(mixed);
}

xio::inet_address::inet_address(const ip_address &addr, std::uint16_t port) : _addr(addr), _port(port)
{
}

xio::inet_address::inet_address(const ip_address &addr, const std::string &service) : _addr(addr), _port(inet_resolver::service(service))
{
}

xio::inet_address::inet_address(const struct ::sockaddr *addr)
{
    this->assign(addr);
}

xio::inet_address::inet_address(const struct ::sockaddr *addr, std::uint16_t port)
{
    this->assign(addr, port);
}

xio::inet_address::inet_address(const struct ::sockaddr *addr, const std::string &service)
{
    this->assign(addr, service);
}

// property
std::string xio::inet_address::str(bool cidr, bool scope) const
{
    switch (this->_addr.type())
    {
        case ip_address::Type::IPv4:
            return this->_addr.v4().str(cidr) + ":" + chen::num::str(this->_port);

        case ip_address::Type::IPv6:
            return "[" + this->_addr.v6().str(cidr, scope) + "]:" + chen::num::str(this->_port);

        default:
            return "";
    }
}

bool xio::inet_address::empty() const
{
    return this->_addr.empty();
}

xio::inet_address::operator bool() const
{
    return !this->empty();
}

const xio::ip_address& xio::inet_address::addr() const
{
    return this->_addr;
}

void xio::inet_address::addr(const ip_address &value)
{
    this->_addr = value;
}

std::uint16_t xio::inet_address::port() const
{
    return this->_port;
}

void xio::inet_address::port(std::uint16_t value)
{
    this->_port = value;
}

// special
bool xio::inet_address::isWellKnownPort() const
{
    // from 0 through 1023
    return this->_port <= 1023;
}

bool xio::inet_address::isRegisteredPort() const
{
    // from 1024 through 49151
    return (this->_port >= 1024) && (this->_port <= 49151);
}

bool xio::inet_address::isDynamicPort() const
{
    // from 49152 through 65535
    return this->_port >= 49152;
}

// assignment
void xio::inet_address::assign(std::nullptr_t)
{
    this->_addr = nullptr;
    this->_port = 0;
}

void xio::inet_address::assign(const std::string &mixed)
{
    auto pair = inet_resolver::extract(mixed);
    this->_addr = pair.first;
    this->_port = inet_resolver::service(pair.second);
}

void xio::inet_address::assign(const ip_address &addr, std::uint16_t port)
{
    this->_addr = addr;
    this->_port = port;
}

void xio::inet_address::assign(const ip_address &addr, const std::string &service)
{
    this->_addr = addr;
    this->_port = inet_resolver::service(service);
}

void xio::inet_address::assign(const struct ::sockaddr *addr)
{
    this->sockaddr(addr);
}

void xio::inet_address::assign(const struct ::sockaddr *addr, std::uint16_t port)
{
    this->assign(addr);
    this->_port = port;
}

void xio::inet_address::assign(const struct ::sockaddr *addr, const std::string &service)
{
    this->assign(addr);
    this->_port = inet_resolver::service(service);
}

xio::inet_address& xio::inet_address::operator=(std::nullptr_t)
{
    this->assign(nullptr);
    return *this;
}

xio::inet_address& xio::inet_address::operator=(const char *mixed)
{
    this->assign(mixed);
    return *this;
}

xio::inet_address& xio::inet_address::operator=(const std::string &mixed)
{
    this->assign(mixed);
    return *this;
}

xio::inet_address& xio::inet_address::operator=(const struct ::sockaddr *addr)
{
    this->assign(addr);
    return *this;
}

// comparison
bool xio::inet_address::operator==(const inet_address &o) const
{
    return (this->_addr == o._addr) && (this->_port == o._port);
}

bool xio::inet_address::operator!=(const inet_address &o) const
{
    return !(*this == o);
}

bool xio::inet_address::operator<(const inet_address &o) const
{
    return (this->_addr == o._addr) ? this->_port < o._port : this->_addr < o._addr;
}

bool xio::inet_address::operator>(const inet_address &o) const
{
    return o < *this;
}

bool xio::inet_address::operator<=(const inet_address &o) const
{
    return (this->_addr == o._addr) ? this->_port <= o._port : this->_addr <= o._addr;
}

bool xio::inet_address::operator>=(const inet_address &o) const
{
    return o <= *this;
}

// override
socklen_t xio::inet_address::socklen() const
{
    switch (this->_addr.type())
    {
        case ip_address::Type::IPv4:
            return sizeof(::sockaddr_in);

        case ip_address::Type::IPv6:
            return sizeof(::sockaddr_in6);

        default:
            return 0;
    }
}

struct ::sockaddr_storage xio::inet_address::sockaddr() const
{
    ::sockaddr_storage ret{};

    switch (this->_addr.type())
    {
        case ip_address::Type::IPv4:
        {
            auto in = (::sockaddr_in*)&ret;

            in->sin_family      = AF_INET;
            in->sin_port        = chen::num::swap(this->_port);
            in->sin_addr.s_addr = chen::num::swap(this->_addr.v4().addr());
        }
            break;

        case ip_address::Type::IPv6:
        {
            auto in = (::sockaddr_in6*)&ret;

            in->sin6_family   = AF_INET6;
            in->sin6_port     = chen::num::swap(this->_port);
            in->sin6_scope_id = this->_addr.v6().scope();

            ::memcpy(in->sin6_addr.s6_addr, this->_addr.v6().addr().data(), 16);
        }
            break;

        default:
            break;
    }

    return ret;
}

void xio::inet_address::sockaddr(const struct ::sockaddr *addr)
{
    switch (addr->sa_family)
    {
        case AF_INET:
        {
            auto in = (::sockaddr_in*)addr;
            this->_addr = ip_version4(chen::num::swap(in->sin_addr.s_addr));
            this->_port = chen::num::swap(in->sin_port);
        }
            break;

        case AF_INET6:
        {
            auto in = (::sockaddr_in6*)addr;
            this->_addr = ip_version6(in->sin6_addr.s6_addr, 128, in->sin6_scope_id);
            this->_port = chen::num::swap(in->sin6_port);
        }
            break;

        default:
            throw std::runtime_error("address: unknown bsd address provided");
    }
}