/**
 * Created by Jian Chen
 * @since  2016.09.21
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/inet/inet_adapter.hpp"
#include "catch.hpp"

using xio::inet_adapter;

TEST_CASE("InetAdapterTest")
{
    // the following codes have no practical meaning
    // because adapters are different on each system
    // we can't do unit test for this class
    std::uint8_t addr[16]{};

    CHECK(!inet_adapter::enumerate().empty());
    CHECK_NOTHROW(inet_adapter::scope(addr, "0"));
    CHECK_NOTHROW(inet_adapter::scope(addr, "lo0"));
    CHECK_NOTHROW(inet_adapter::scope(0));
}