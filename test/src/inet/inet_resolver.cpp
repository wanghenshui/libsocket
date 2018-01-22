/**
 * Created by Jian Chen
 * @since  2016.09.21
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/inet/inet_resolver.hpp"
#include "catch.hpp"

using xio::ip_address;
using xio::inet_resolver;

TEST_CASE("InetResolverTest")
{
    SECTION("Domain")
    {
        // resolve domain
        CHECK_NOTHROW(inet_resolver::resolve("example.com"));
        CHECK_NOTHROW(inet_resolver::resolve("example.com:80"));
        CHECK_NOTHROW(inet_resolver::resolve("example.com:http"));
    }

    SECTION("IPv4")
    {
        CHECK("0.0.0.0:80" == inet_resolver::resolve(":80")[0].str());
        CHECK("127.0.0.1:0" == inet_resolver::resolve("127.0.0.1")[0].str());
        CHECK("127.0.0.1:80" == inet_resolver::resolve("127.0.0.1:80")[0].str());
        CHECK("127.0.0.1:80" == inet_resolver::resolve("127.0.0.1:http")[0].str());

        CHECK("0.0.0.0:80" == inet_resolver::resolve(":80", AF_INET)[0].str());
        CHECK("127.0.0.1:0" == inet_resolver::resolve("127.0.0.1", AF_INET)[0].str());
        CHECK("127.0.0.1:80" == inet_resolver::resolve("127.0.0.1:80", AF_INET)[0].str());
        CHECK("127.0.0.1:80" == inet_resolver::resolve("127.0.0.1:http", AF_INET)[0].str());
    }

    SECTION("IPv6")
    {
        CHECK("[fe80::1]:0" == inet_resolver::resolve("fe80::1")[0].str());
        CHECK("[fe80::1]:0" == inet_resolver::resolve("[fe80::1]")[0].str());

        CHECK("[::]:80" == inet_resolver::resolve("[::]:80")[0].str());
        CHECK("[fe80::1]:80" == inet_resolver::resolve("[fe80::1]:80")[0].str());
        CHECK("[fe80::1]:80" == inet_resolver::resolve("[fe80::1]:http")[0].str());

        CHECK("[::]:80" == inet_resolver::resolve("[::]:80", AF_INET6)[0].str());
        CHECK("[fe80::1]:80" == inet_resolver::resolve("[fe80::1]:80", AF_INET6)[0].str());
        CHECK("[fe80::1]:80" == inet_resolver::resolve("[fe80::1]:http", AF_INET6)[0].str());
    }

    SECTION("Reverse")
    {
        // reverse resolve
        CHECK_NOTHROW(inet_resolver::reverse("127.0.0.1:80"));
        CHECK_NOTHROW(inet_resolver::reverse("127.0.0.1:443"));
    }

    SECTION("Service")
    {
        // service port and name
        CHECK(0 == inet_resolver::service(""));

        CHECK(80 == inet_resolver::service("80"));
        CHECK(80 == inet_resolver::service("http"));

        CHECK(80 == inet_resolver::service("80", "tcp"));
        CHECK(80 == inet_resolver::service("http", "tcp"));

        CHECK(53 == inet_resolver::service("domain", "tcp"));
        CHECK(53 == inet_resolver::service("domain", "udp"));

        CHECK(513 == inet_resolver::service("who"));  // only udp

        CHECK("http" == inet_resolver::service(80));
        CHECK("https" == inet_resolver::service(443));
    }
}