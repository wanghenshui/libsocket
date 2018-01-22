/**
 * Created by Jian Chen
 * @since  2017.02.23
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/base/basic_socket.hpp"
#include "catch.hpp"

using xio::ip_option4;
using xio::ip_option6;
using xio::basic_socket;

TEST_CASE("IPOptionTest")
{
    basic_socket s4(AF_INET, SOCK_STREAM);
    basic_socket s6(AF_INET6, SOCK_STREAM);

    // ttl
    CHECK(ip_option4::ttl(s4.native()) >= 0);
    CHECK(ip_option4::ttl(s4.native(), 10));

    // v6only
    CHECK_NOTHROW(ip_option6::v6only(s6.native()));  // default is true on Windows, otherwise false
    CHECK(ip_option6::v6only(s6.native(), true));
}