/**
 * Created by Jian Chen
 * @since  2016.11.29
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/base/basic_socket.hpp"
#include "catch.hpp"

using xio::basic_socket;
using xio::basic_option;
using xio::basic_address;

TEST_CASE("BasicOptionTest")
{
    SECTION("TCP")
    {
        basic_socket s(AF_INET, SOCK_STREAM);

        // error
        CHECK(!basic_option::error(s.native()));

        // type
        CHECK(s.type() == basic_option::type(s.native()));

        // debug
        CHECK_FALSE(basic_option::debug(s.native()));
        CHECK_NOTHROW(basic_option::debug(s.native(), true));  // system may not allowed
        CHECK_NOTHROW(basic_option::debug(s.native()));

        // reuseaddr
        CHECK_FALSE(basic_option::reuseaddr(s.native()));
        CHECK(basic_option::reuseaddr(s.native(), true));
        CHECK(basic_option::reuseaddr(s.native()));

        // reuseport
#ifndef _WIN32
        CHECK_FALSE(basic_option::reuseport(s.native()));
        CHECK(basic_option::reuseport(s.native(), true));
        CHECK(basic_option::reuseport(s.native()));
#endif

        // keepalive
        CHECK_FALSE(basic_option::keepalive(s.native()));
        CHECK(basic_option::keepalive(s.native(), true));
        CHECK(basic_option::keepalive(s.native()));

        // dontroute
        CHECK_FALSE(basic_option::dontroute(s.native()));
        CHECK(basic_option::dontroute(s.native(), true));
        CHECK(basic_option::dontroute(s.native()));

        // linger
        CHECK(basic_option::linger(s.native(), 1, 100));
        CHECK_NOTHROW(basic_option::linger(s.native()));  // may not allowed

        // oobinline
        CHECK_FALSE(basic_option::oobinline(s.native()));
        CHECK(basic_option::oobinline(s.native(), true));
        CHECK(basic_option::oobinline(s.native()));

        // sndbuf
        CHECK(basic_option::sndbuf(s.native()) > 0);
        CHECK(basic_option::sndbuf(s.native(), 1024));  // just a hint

        // rcvbuf
        CHECK(basic_option::rcvbuf(s.native()) > 0);
        CHECK(basic_option::rcvbuf(s.native(), 1024));  // just a hint

        // sndlowat
        CHECK(basic_option::sndlowat(s.native()) >= 0);
        CHECK_NOTHROW(basic_option::sndlowat(s.native(), 1024));  // may not allowed

        // rcvlowat
        CHECK(basic_option::rcvlowat(s.native()) >= 0);
        CHECK_NOTHROW(basic_option::rcvlowat(s.native(), 1024));  // may not allowed

        // sndtimeo
        CHECK(basic_option::sndtimeo(s.native(), 100, 4000));  // just a hint
        CHECK_NOTHROW(basic_option::sndtimeo(s.native()));

        // rcvtimeo
        CHECK(basic_option::rcvtimeo(s.native(), 100, 4000));  // just a hint
        CHECK_NOTHROW(basic_option::rcvtimeo(s.native()));
    }

    SECTION("UDP")
    {
        basic_socket s(AF_INET, SOCK_DGRAM);

        // broadcast
        CHECK_FALSE(basic_option::broadcast(s.native()));
        CHECK(basic_option::broadcast(s.native(), true));
        CHECK(basic_option::broadcast(s.native()));
    }
}