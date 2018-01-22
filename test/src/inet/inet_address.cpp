/**
 * Created by Jian Chen
 * @since  2016.09.21
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/inet/inet_address.hpp"
#include "chen/base/num.hpp"
#include "catch.hpp"
#include <cstring>

using xio::ip_address;
using xio::ip_version6;
using xio::inet_address;
using xio::basic_address;

TEST_CASE("InetAddressTest")
{
    SECTION("General")
    {
        // nullptr
        CHECK(inet_address(nullptr).empty());
        CHECK_FALSE(inet_address(nullptr));
        CHECK("" == inet_address().str());

        // one param: string
        CHECK("0.0.0.0:80" == inet_address(":80").str());
        CHECK("127.0.0.1:0" == inet_address("127.0.0.1").str());
        CHECK("127.0.0.1:80" == inet_address("127.0.0.1:80").str());
        CHECK("127.0.0.1:80" == inet_address("127.0.0.1:http").str());

        CHECK("[::]:80" == inet_address("[::]:80").str());
        CHECK("[fe80::1]:0" == inet_address("[fe80::1]").str());
        CHECK("[fe80::1]:80" == inet_address("[fe80::1]:80").str());
        CHECK("[fe80::1]:80" == inet_address("[fe80::1]:http").str());

        // addr + port|service
        CHECK("127.0.0.1:80" == inet_address("127.0.0.1", 80).str());
        CHECK("127.0.0.1:80" == inet_address("127.0.0.1", "80").str());
        CHECK("127.0.0.1:80" == inet_address("127.0.0.1", "http").str());

        CHECK("[::]:80" == inet_address("::", 80).str());
        CHECK("[fe80::1]:80" == inet_address("fe80::1", "80").str());
        CHECK("[fe80::1]:80" == inet_address("fe80::1", "http").str());

        // raw socket address
        ::sockaddr_storage v4{};
        ::sockaddr_storage v6{};

        ::sockaddr_in  *p4 = (::sockaddr_in*)&v4;
        ::sockaddr_in6 *p6 = (::sockaddr_in6*)&v6;

        p4->sin_family = AF_INET;
        p4->sin_port   = chen::num::swap(static_cast<std::uint16_t>(80));
        p4->sin_addr.s_addr = chen::num::swap(static_cast<std::uint32_t>(0x7f000001));

        CHECK("127.0.0.1:80" == inet_address((::sockaddr*)&v4).str());
        CHECK("127.0.0.1:443" == inet_address((::sockaddr*)&v4, 443).str());
        CHECK("127.0.0.1:443" == inet_address((::sockaddr*)&v4, "https").str());

        p6->sin6_family = AF_INET6;
        p6->sin6_port   = chen::num::swap(static_cast<std::uint16_t>(80));

        ip_version6 i6("fe80::1");
        ::memcpy(p6->sin6_addr.s6_addr, i6.addr().data(), 16);

        CHECK("[fe80::1]:80" == inet_address((::sockaddr*)&v6).str());
        CHECK("[fe80::1]:443" == inet_address((::sockaddr*)&v6, 443).str());
        CHECK("[fe80::1]:443" == inet_address((::sockaddr*)&v6, "https").str());

        // conversion
        auto c  = inet_address().sockaddr();
        auto c4 = inet_address((::sockaddr*)&v4).sockaddr();
        auto c6 = inet_address((::sockaddr*)&v6).sockaddr();

        CHECK(c.ss_family == 0);
        CHECK(::memcmp(&v4, &c4, sizeof(::sockaddr_in)) == 0);
        CHECK(::memcmp(&v6, &c6, sizeof(::sockaddr_in6)) == 0);
    }

    SECTION("Assignment")
    {
        inet_address addr;

        // normal
        addr.assign(nullptr);
        CHECK("" == addr.str());

        addr.assign("127.0.0.1", 80);
        CHECK("127.0.0.1:80" == addr.str());

        addr.assign("127.0.0.1", "http");
        CHECK("127.0.0.1:80" == addr.str());

        // operator
        ::sockaddr_storage empty{};  // family is 0
        CHECK_THROWS(addr.assign((::sockaddr*)&empty));

        addr = nullptr;
        CHECK("" == addr.str());

        addr = "[fe80::1]:80";
        CHECK("[fe80::1]:80" == addr.str());

        addr = std::string("[fe80::1]:80");
        CHECK("[fe80::1]:80" == addr.str());

        // raw address
        ::sockaddr_in *p4 = (::sockaddr_in*)&empty;

        p4->sin_family = AF_INET;
        p4->sin_port   = chen::num::swap(static_cast<std::uint16_t>(80));
        p4->sin_addr.s_addr = chen::num::swap(static_cast<std::uint32_t>(0x7f000001));

        addr = (::sockaddr*)&empty;
        CHECK("127.0.0.1:80" == addr.str());
    }

    SECTION("Property")
    {
        // modify port and address
        inet_address addr("127.0.0.1", 80);

        CHECK(80 == addr.port());
        CHECK("127.0.0.1" == addr.addr().str());

        addr.port(443);
        addr.addr(ip_address("192.168.1.1"));

        CHECK(443 == addr.port());
        CHECK("192.168.1.1" == addr.addr().str());

        // address with port
        CHECK("127.0.0.1:80" == inet_address("127.0.0.1", 80).str());
        CHECK("[::1]:80" == inet_address("::1", 80).str());
    }

    SECTION("Special")
    {
        // special ports
        CHECK(inet_address("127.0.0.1", 0).isWellKnownPort());
        CHECK_FALSE(inet_address("127.0.0.1", 0).isRegisteredPort());
        CHECK_FALSE(inet_address("127.0.0.1", 0).isDynamicPort());

        CHECK(inet_address("127.0.0.1", 80).isWellKnownPort());
        CHECK(inet_address("127.0.0.1", 1023).isWellKnownPort());

        CHECK(inet_address("127.0.0.1", 1024).isRegisteredPort());
        CHECK(inet_address("127.0.0.1", 3306).isRegisteredPort());
        CHECK(inet_address("127.0.0.1", 49151).isRegisteredPort());

        CHECK(inet_address("127.0.0.1", 49152).isDynamicPort());
        CHECK(inet_address("127.0.0.1", 50000).isDynamicPort());
        CHECK(inet_address("127.0.0.1", 65535).isDynamicPort());
    }

    SECTION("Compare")
    {
        // compare
        CHECK(inet_address("127.0.0.1", 80) == inet_address("127.0.0.1", 80));
        CHECK(inet_address("192.168.0.1", 80) != inet_address("127.0.0.1", 80));

        CHECK(inet_address("127.0.0.0", 80) < inet_address("127.0.0.1", 80));
        CHECK(inet_address("127.0.0.1", 53) < inet_address("127.0.0.1", 80));

        CHECK(inet_address("127.0.0.1", 80) <= inet_address("127.0.0.1", 80));
        CHECK(inet_address("127.0.0.1", 53) <= inet_address("127.0.0.1", 80));

        CHECK(inet_address("127.0.0.1", 80) > inet_address("127.0.0.0", 80));
        CHECK(inet_address("127.0.0.1", 80) > inet_address("127.0.0.1", 53));

        CHECK(inet_address("127.0.0.1", 80) >= inet_address("127.0.0.1", 80));
        CHECK(inet_address("127.0.0.1", 80) >= inet_address("127.0.0.1", 53));
    }
}
