/**
 * Created by Jian Chen
 * @since  2016.05.20
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/inet/inet_adapter.hpp"
#include "catch.hpp"

using xio::ip_address;
using xio::ip_version4;
using xio::ip_version6;
using xio::inet_adapter;

TEST_CASE("IPAddressTest", )
{
    SECTION("Base")
    {
        CHECK(ip_address::Type::IPv4 == ip_address::detect("127.0.0.1"));
        CHECK(ip_address::Type::IPv6 == ip_address::detect("2404:6800:4004:817::200e"));
    }

    SECTION("None")
    {
        CHECK(ip_address(nullptr).empty());
        CHECK_FALSE(ip_address(nullptr));

        ip_address empty("127");
        empty = nullptr;

        CHECK(empty.empty());
        CHECK_FALSE(empty.isUnspecified());

        CHECK(empty.network().empty());
        CHECK(empty.minhost().empty());
        CHECK(empty.maxhost().empty());

        CHECK_FALSE(empty.isLoopback());
        CHECK_FALSE(empty.isMulticast());

        CHECK("" == ip_address(nullptr).str());
        CHECK(0u == ip_address(nullptr).cidr());
        CHECK(0u == ip_address(nullptr).scope());
        CHECK(0u == ip_address(nullptr).bytes().size());

        CHECK(ip_address(ip_address::Type::None) == ip_address(nullptr));
        CHECK(ip_address(ip_address::Type::None) <= ip_address(nullptr));
        CHECK(ip_address(ip_address::Type::None) == ip_address::any(ip_address::Type::None));
        CHECK(ip_address(ip_address::Type::None) == ip_address::loopback(ip_address::Type::None));
        CHECK(ip_address(ip_address::Type::IPv4) != ip_address(ip_address::Type::IPv6));

        CHECK_FALSE(ip_address(ip_address::Type::None) < ip_address(nullptr));
        CHECK_FALSE(ip_address(ip_address::Type::IPv4) > ip_address(ip_address::Type::IPv6));
        CHECK_FALSE(ip_address(ip_address::Type::IPv4) < ip_address(ip_address::Type::IPv6));
    }

    SECTION("IPv4")
    {
        // assign
        CHECK(ip_address(ip_address::Type::IPv4) == ip_address("0.0.0.0"));

        CHECK(ip_address("127.0.0.1/8") == ip_address("127.0.0.1/8"));
        CHECK(ip_address("127.0.0.1/8") == ip_address("127.0.0.1", 8));
        CHECK(ip_address("127.0.0.1/8") == ip_address("127.0.0.1", "255.0.0.0"));

        CHECK(ip_address("127") == ip_address("127.0.0.0"));
        CHECK(ip_address("127.1") == ip_address("127.0.0.1"));
        CHECK(ip_address("192.168.1") == ip_address("192.168.0.1"));

        CHECK(ip_address(ip_version4(0x7f000001)) == ip_address("127.0.0.1"));
        CHECK(ip_address(ip_version4(0x7f000001, 8)) == ip_address("127.0.0.1/8"));
        CHECK(ip_address(ip_version4(0x7f000001, "255.0.0.0")) == ip_address("127.0.0.1/8"));

        CHECK_THROWS_AS(ip_address("127.0.0.1", 33), std::runtime_error);
        CHECK_THROWS_AS(ip_address(ip_version4(0x7f000001, 33)), std::runtime_error);

        ip_address v4("127");
        CHECK(v4.isIPv4());
        CHECK("127.0.0.0" == v4.str());
        CHECK(4u == v4.bytes().size());

        v4 = ip_address::Type::IPv4;
        CHECK("0.0.0.0" == v4.str());

        v4 = "192.168.1.1/24";
        CHECK("192.168.1.1/24" == v4.str(true));
        CHECK(24 == v4.cidr());

        v4 = ip_version4(0x7f000001);
        CHECK("127.0.0.1" == v4.str());
        CHECK(32 == v4.cidr());

        // representation
        CHECK("127.0.0.1" == ip_address("127.0.0.1").str());
        CHECK("127.0.0.1" == ip_address("127.0.1").str());
        CHECK("127.0.0.1" == ip_address("127.1").str());
        CHECK("127.0.0.1" == ip_address(ip_version4(0x7f000001)).str());

        CHECK("127.0.0.1" == ip_address("127.0.0.1").str());
        CHECK("127.0.0.1/8" == ip_address("127.0.0.1/8").str(true));
        CHECK("127.0.0.1/8" == ip_address(ip_version4(0x7f000001, 8)).str(true));
        CHECK("192.168.1.1/30" == ip_address("192.168.1.1", 30).str(true));

        // network
        CHECK(8 == ip_address("127.0.0.1/8").cidr());
        CHECK(16 == ip_address("180.166.92.36/16").cidr());
        CHECK(24 == ip_address("192.168.0.1/24").cidr());
        CHECK(26 == ip_address("192.168.0.1/26").cidr());

        CHECK(ip_address("255.0.0.0").v4().addr() == ip_address("127.0.0.1/8").v4().netmask());
        CHECK(ip_address("0.255.255.255").v4().addr() == ip_address("127.0.0.1/8").v4().wildcard());

        CHECK("127.0.0.0/8" == ip_address("127.0.0.1/8").network().str(true));
        CHECK("127.0.0.1/8" == ip_address("127.0.0.1/8").minhost().str(true));
        CHECK("127.255.255.254/8" == ip_address("127.0.0.1/8").maxhost().str(true));
        CHECK("127.255.255.255/8" == ip_address("127.0.0.1/8").v4().broadcast().str(true));
        CHECK(16777214u == ip_address("127.0.0.1/8").v4().hosts());

        // special
        CHECK(ip_address("0.0.0.0").isUnspecified());

        CHECK(ip_address("0.0.0.0").v4().isReserved());
        CHECK(ip_address("127.0.0.1").v4().isReserved());
        CHECK(ip_address("169.254.0.0").v4().isReserved());
        CHECK(ip_address("192.0.0.0").v4().isReserved());
        CHECK(ip_address("192.0.2.0").v4().isReserved());
        CHECK(ip_address("192.88.99.0").v4().isReserved());
        CHECK(ip_address("198.51.100.0").v4().isReserved());
        CHECK(ip_address("203.0.113.0").v4().isReserved());
        CHECK(ip_address("224.0.0.0").v4().isReserved());
        CHECK(ip_address("240.0.0.0").v4().isReserved());
        CHECK(ip_address("10.0.0.0").v4().isReserved());
        CHECK(ip_address("100.64.0.0").v4().isReserved());
        CHECK(ip_address("172.16.0.0").v4().isReserved());
        CHECK(ip_address("192.168.0.0").v4().isReserved());
        CHECK(ip_address("198.18.0.0").v4().isReserved());

        CHECK(ip_address("10.0.0.0").v4().isPrivate());
        CHECK(ip_address("100.64.0.0").v4().isPrivate());
        CHECK(ip_address("172.16.0.0").v4().isPrivate());
        CHECK(ip_address("192.168.0.0").v4().isPrivate());
        CHECK(ip_address("198.18.0.0").v4().isPrivate());

        CHECK(ip_address("127.0.0.1").isLoopback());
        CHECK_FALSE(ip_address("192.168.0.1").isLoopback());

        CHECK(ip_address("169.254.0.0").v4().isLinkLocal());

        CHECK(ip_address("224.0.0.115").isMulticast());
        CHECK_FALSE(ip_address("10.0.0.1").isMulticast());

        CHECK(ip_address("192.168.0.255/24").v4().isBroadcast());
        CHECK_FALSE(ip_address("192.168.0.1/24").v4().isBroadcast());

        // class A
        CHECK(ip_address("0.0.0.0").v4().isClassA());
        CHECK_FALSE(ip_address("0.0.0.0").v4().isClassB());
        CHECK_FALSE(ip_address("0.0.0.0").v4().isClassC());

        CHECK(ip_address("10.0.0.0").v4().isClassA());
        CHECK_FALSE(ip_address("10.0.0.0").v4().isClassB());
        CHECK_FALSE(ip_address("10.0.0.0").v4().isClassC());

        CHECK(ip_address("127.255.255.255").v4().isClassA());
        CHECK_FALSE(ip_address("127.255.255.255").v4().isClassB());
        CHECK_FALSE(ip_address("127.255.255.255").v4().isClassC());

        // class B
        CHECK_FALSE(ip_address("128.0.0.0").v4().isClassA());
        CHECK(ip_address("128.0.0.0").v4().isClassB());
        CHECK_FALSE(ip_address("128.0.0.0").v4().isClassC());

        CHECK_FALSE(ip_address("190.0.0.0").v4().isClassA());
        CHECK(ip_address("190.0.0.0").v4().isClassB());
        CHECK_FALSE(ip_address("190.0.0.0").v4().isClassC());

        CHECK_FALSE(ip_address("191.255.255.255").v4().isClassA());
        CHECK(ip_address("191.255.255.255").v4().isClassB());
        CHECK_FALSE(ip_address("191.255.255.255").v4().isClassC());

        // class C
        CHECK_FALSE(ip_address("192.0.0.0").v4().isClassA());
        CHECK_FALSE(ip_address("192.0.0.0").v4().isClassB());
        CHECK(ip_address("192.0.0.0").v4().isClassC());

        CHECK_FALSE(ip_address("192.168.1.1").v4().isClassA());
        CHECK_FALSE(ip_address("192.168.1.1").v4().isClassB());
        CHECK(ip_address("192.168.1.1").v4().isClassC());

        CHECK_FALSE(ip_address("223.255.255.255").v4().isClassA());
        CHECK_FALSE(ip_address("223.255.255.255").v4().isClassB());
        CHECK(ip_address("223.255.255.255").v4().isClassC());

        // class D & E
        CHECK(ip_address("224.0.0.0").v4().isClassD());
        CHECK(ip_address("239.255.255.255").v4().isClassD());

        CHECK(ip_address("240.0.0.0").v4().isClassE());
        CHECK(ip_address("255.255.255.255").v4().isClassE());

        // operator
        CHECK(ip_address("127.0.0.1") == ip_address("127.0.0.1"));
        CHECK(ip_address("127.0.0.1/8") == ip_address("127.0.0.1/8"));

        CHECK(ip_address("127.0.0.0") != ip_address("127.0.0.1"));
        CHECK(ip_address("127.0.0.1/8") != ip_address("127.0.0.1"));

        CHECK(ip_address("127.0.0.0") < ip_address("127.0.0.1"));
        CHECK(ip_address("127.0.0.1/8") < ip_address("127.0.0.1"));

        CHECK(ip_address("127.0.0.1") > ip_address("127.0.0.0"));
        CHECK(ip_address("127.0.0.1") > ip_address("127.0.0.1/8"));

        CHECK(ip_address("127.0.0.0") <= ip_address("127.0.0.1"));
        CHECK(ip_address("127.0.0.1") <= ip_address("127.0.0.1"));
        CHECK(ip_address("127.0.0.1/8") <= ip_address("127.0.0.1"));

        CHECK(ip_address("127.0.0.1") >= ip_address("127.0.0.0"));
        CHECK(ip_address("127.0.0.1") >= ip_address("127.0.0.1"));
        CHECK(ip_address("127.0.0.1") >= ip_address("127.0.0.1/8"));

        // common
        CHECK("0.0.0.0" == ip_address::any(ip_address::Type::IPv4).str());
        CHECK("127.0.0.1" == ip_address::loopback(ip_address::Type::IPv4).str());

        // invalid test
        std::uint8_t cidr = 0;

        CHECK_THROWS_AS(ip_version4::toInteger("127..1"), std::runtime_error);
        CHECK_THROWS_AS(ip_version4::toInteger("999.0.0.0"), std::runtime_error);
        CHECK_THROWS_AS(ip_version4::toInteger("127.0.0.1/99", &cidr), std::runtime_error);
    }

    SECTION("IPv6")
    {
        // assign
        CHECK(ip_address(ip_address::Type::IPv6) == ip_address("::"));
        CHECK(ip_address("::1/64") == ip_address("::1/64"));
        CHECK(ip_address("::1/64") == ip_address("::1", 64));
        CHECK(ip_address("::1/64") == ip_address("::1", "ffff:ffff:ffff:ffff::"));

        CHECK(ip_address("::") == ip_address("0000:0000:0000:0000:0000:0000:0000:0000"));
        CHECK(ip_address("::") == ip_address("0:0:0:0:0:0:0:0"));

        CHECK(ip_address("2404:6800:4004:817::200e") == ip_address("2404:6800:4004:817:0:0:0:200e"));
        CHECK(ip_address("2404:6800:4004:817::200e") == ip_address("2404:6800:4004:817:0000:0000:0000:200e"));
        CHECK(ip_address("2404:6800:4004:817::200e") == ip_address("2404:6800:4004:817::200e/128"));

        CHECK(ip_address("::c0a8:1") == ip_address("::192.168.0.1"));

        CHECK(ip_address("fe80::1%1") == ip_address("fe80::1%1"));
        CHECK(ip_address("fe80::1%1") == ip_address("fe80::1%1", 128));
        CHECK(ip_address("fe80::1%1") == ip_address("fe80::1", 128, 1));

        CHECK(ip_address("fe80::1%1/64") == ip_address("fe80::1%1", 64));
        CHECK(ip_address("fe80::1%1/64") == ip_address("fe80::1", 64, 1));
        CHECK(ip_address("fe80::1%1/64") == ip_address("fe80::1%1", "ffff:ffff:ffff:ffff::"));
        CHECK(ip_address("fe80::1%1/64") == ip_address("fe80::1", "ffff:ffff:ffff:ffff::", 1));

        std::uint8_t bytes[16] = {0x24, 0x04, 0x68, 0, 0x40, 0x04, 0x08, 0x17, 0, 0, 0, 0, 0, 0, 0x20, 0x0e};

        CHECK(ip_address("2404:6800:4004:817::200e") == ip_address(ip_version6(bytes)));
        CHECK(ip_address("2404:6800:4004:817::200e/64") == ip_address(ip_version6(bytes, 64)));
        CHECK(ip_address("2404:6800:4004:817::200e/64") == ip_address(ip_version6(bytes, "ffff:ffff:ffff:ffff::")));

        CHECK_THROWS_AS(ip_address("2404:6800:4004:817::200e/129"), std::runtime_error);
        CHECK_THROWS_AS(ip_address(ip_version6(bytes, 129)), std::runtime_error);

        ip_address v6("::1");
        CHECK(v6.isIPv6());
        CHECK("::1" == v6.str());
        CHECK(16u == v6.bytes().size());

        v6 = "2404:6800:4004:817::200e";
        CHECK("2404:6800:4004:817::200e" == v6.str());
        CHECK(128 == v6.cidr());

        v6 = ip_version6(bytes);
        CHECK("2404:6800:4004:817::200e" == v6.str());
        CHECK(128 == v6.cidr());

        // uppercase
        CHECK(ip_address("fe80::1%1") == ip_address("FE80::1%1"));

        // representation
        CHECK("::" == ip_address(ip_address::Type::IPv6).str());
        CHECK("2404:6800:4004:817::" == ip_address("2404:6800:4004:817:0000:0000:0000:0000").str());
        CHECK("2404::817:0:0:0:200e" == ip_address("2404:0000:0000:817:0000:0000:0000:200e").str());
        CHECK("2404:6800:4004:817::200e" == ip_address("2404:6800:4004:817:0000:0000:0000:200e").str());
        CHECK("2404:6800:4004:0817:0000:0000:0000:200e" == ip_address("2404:6800:4004:817:0000:0000:0000:200e").v6().expanded());
        CHECK("2404:6800:4004:817:0:0:0:200e" == ip_address("2404:6800:4004:817:0000:0000:0000:200e").v6().suppressed());
        CHECK("2404:6800:4004:817::200e" == ip_address("2404:6800:4004:817:0000:0000:0000:200e").v6().compressed());
        CHECK("::ffff:192.0.2.128" == ip_address("::ffff:c000:280").v6().mixed());

        auto scope = inet_adapter::scope(1);

        CHECK("fe80::1%" + scope == ip_address("fe80::1%1").v6().str(false, true));
        CHECK("fe80::1%" + scope + "/128" == ip_address("fe80::1%1").v6().str(true, true));

        CHECK(1u == ip_address("fe80::1%1").scope());

        CHECK(ip_address("192.0.2.128").v4() == ip_address("::ffff:c000:280").v6().embedded());
        CHECK(ip_address("192.0.2.33").v4() == ip_address("2001:db8:c000:221::/32").v6().embedded());
        CHECK(ip_address("192.0.2.33").v4() == ip_address("2001:db8:1c0:2:21::/40").v6().embedded());
        CHECK(ip_address("192.0.2.33").v4() == ip_address("2001:db8:122:c000:2:2100::/48").v6().embedded());
        CHECK(ip_address("192.0.2.33").v4() == ip_address("2001:db8:122:3c0:0:221::/56").v6().embedded());
        CHECK(ip_address("192.0.2.33").v4() == ip_address("2001:db8:122:344:c0:2:2100::/64").v6().embedded());
        CHECK(ip_address("192.0.2.33").v4() == ip_address("2001:db8:122:344::192.0.2.33/96").v6().embedded());
        CHECK(ip_address("192.0.2.33").v4() == ip_address("64:ff9b::192.0.2.33/96").v6().embedded());

        CHECK("::" == ip_address(ip_address::Type::IPv6).str());
        CHECK("2404:6800:4004:817::200e" == ip_address("2404:6800:4004:817:0000:0000:0000:200e").str());

        std::array<std::uint8_t, 16> tmp;
        ::memcpy(tmp.data(), bytes, 16);

        CHECK(tmp == ip_address("2404:6800:4004:817::200e").v6().addr());
        CHECK(128 == ip_address("2404:6800:4004:817::200e").cidr());
        CHECK(128 == ip_address("2404:6800:4004:817::200e/128").cidr());
        CHECK(64 == ip_address("2404:6800:4004:817::200e/64").cidr());

        // network
        std::array<std::uint8_t, 16> netmask = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
        std::array<std::uint8_t, 16> wildcard = {{0, 0, 0, 0, 0, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

        CHECK(netmask == ip_address("2404:6800:4004:817::200e/48").v6().netmask());
        CHECK(wildcard == ip_address("2404:6800:4004:817::200e/48").v6().wildcard());

        netmask  = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
        wildcard = {{0, 0, 0, 0, 0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

        CHECK(netmask == ip_address("2404:6800:4004:817::200e/46").v6().netmask());
        CHECK(wildcard == ip_address("2404:6800:4004:817::200e/46").v6().wildcard());

        CHECK("2404:6800:4004::/46" == ip_address("2404:6800:4004:817::200e/46").network().str(true));
        CHECK("2404:6800:4004:817::200e" == ip_address("2404:6800:4004:817::200e/128").network().str());

        CHECK("2404:6800:4004::/46" == ip_address("2404:6800:4004:817::200e/46").minhost().str(true));
        CHECK("2404:6800:4007:ffff:ffff:ffff:ffff:ffff/46" == ip_address("2404:6800:4004:817::200e/46").maxhost().str(true));

        CHECK("2404:6800:4004:817::200e" == ip_address("2404:6800:4004:817::200e/128").minhost().str());
        CHECK("2404:6800:4004:817::200e" == ip_address("2404:6800:4004:817::200e/128").maxhost().str());

        // special
        CHECK(ip_address("::").isUnspecified());
        CHECK_FALSE(ip_address("::1").isUnspecified());

        CHECK(ip_address("::1").isLoopback());
        CHECK_FALSE(ip_address("::").isLoopback());

        CHECK(ip_address("2404:6800:4004:817::200e").v6().isGlobalUnicast());
        CHECK_FALSE(ip_address("fe80::7a31:c1ff:fec2:b5aa").v6().isGlobalUnicast());

        CHECK(ip_address("fe80::7a31:c1ff:fec2:b5aa").v6().isLinkLocalUnicast());
        CHECK_FALSE(ip_address("2404:6800:4004:817::200e").v6().isLinkLocalUnicast());

        CHECK(ip_address("FEC0::1234:5678:9ABC").v6().isSiteLocalUnicast());
        CHECK_FALSE(ip_address("2404:6800:4004:817::200e").v6().isSiteLocalUnicast());

        CHECK(ip_address("::192.168.1.1").v6().isIPv4Compatible());
        CHECK_FALSE(ip_address("2404::192.168.1.1").v6().isIPv4Compatible());

        CHECK(ip_address("::ffff:192.168.1.1").v6().isIPv4Mapped());
        CHECK_FALSE(ip_address("::192.168.1.1").v6().isIPv4Mapped());

        CHECK(ip_address("::ffff:192.168.1.1").v6().isIPv4Mapped());
        CHECK_FALSE(ip_address("::192.168.1.1").v6().isIPv4Mapped());

        CHECK(ip_address("ff02::1").isMulticast());
        CHECK_FALSE(ip_address("2404::192.168.1.1").isMulticast());

        CHECK(ip_address("64:ff9b::73b6:405e").v6().isIPv4EmbeddedWellKnown());
        CHECK_FALSE(ip_address("2001:db8:122:344::").v6().isIPv4EmbeddedWellKnown());

        // operator
        CHECK(ip_address("2404:6800:4004:817::200e") == ip_address("2404:6800:4004:817::200e"));
        CHECK(ip_address("2404:6800:4004:817::200e/64") == ip_address("2404:6800:4004:817::200e/64"));

        CHECK(ip_address("2404:6800:4004:817::200e") != ip_address("2404:6800:4004:817::"));
        CHECK(ip_address("2404:6800:4004:817::200e/64") != ip_address("2404:6800:4004:817::200e"));

        CHECK(ip_address("2404:6800:4004:817::") < ip_address("2404:6800:4004:817::200e"));
        CHECK(ip_address("2404:6800:4004:817::200e/64") < ip_address("2404:6800:4004:817::200e"));

        CHECK(ip_address("2404:6800:4004:817::200e") > ip_address("2404:6800:4004:817::"));
        CHECK(ip_address("2404:6800:4004:817::200e") > ip_address("2404:6800:4004:817::200e/64"));

        CHECK(ip_address("2404:6800:4004:817::") <= ip_address("2404:6800:4004:817::200e"));
        CHECK(ip_address("2404:6800:4004:817::200e") <= ip_address("2404:6800:4004:817::200e"));
        CHECK(ip_address("2404:6800:4004:817::200e/64") <= ip_address("2404:6800:4004:817::200e"));

        CHECK(ip_address("2404:6800:4004:817::200e") >= ip_address("2404:6800:4004:817::"));
        CHECK(ip_address("2404:6800:4004:817::200e") >= ip_address("2404:6800:4004:817::200e"));
        CHECK(ip_address("2404:6800:4004:817::200e") >= ip_address("2404:6800:4004:817::200e/64"));

        // common
        CHECK("::" == ip_address::any(ip_address::Type::IPv6).str());
        CHECK("::1" == ip_address::loopback(ip_address::Type::IPv6).str());

        // invalid test
        std::uint8_t cidr = 0;

        CHECK_THROWS_AS(ip_address("2404:6800:4004:817::200e", 200), std::runtime_error);
        CHECK_THROWS_AS(ip_address(ip_version6(bytes, 200)), std::runtime_error);
        CHECK_THROWS_AS(ip_version6::toBytes("::1::1"), std::runtime_error);
        CHECK_THROWS_AS(ip_version6::toBytes("::192.fe:1:1"), std::runtime_error);
        CHECK_THROWS_AS(ip_version6::toBytes("::1^$"), std::runtime_error);
        CHECK_THROWS_AS(ip_version6::toBytes("::1/200", &cidr), std::runtime_error);
    }
}