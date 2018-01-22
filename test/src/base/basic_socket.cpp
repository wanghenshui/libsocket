/**
 * Created by Jian Chen
 * @since  2016.11.29
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/base/basic_socket.hpp"
#include "xio/inet/inet_address.hpp"
#include "chen/mt/semaphore.hpp"
#include "chen/base/num.hpp"
#include "catch.hpp"
#include <cstring>
#include <thread>

using xio::basic_socket;
using xio::inet_address;
using xio::basic_address;
using xio::invalid_handle;

TEST_CASE("BasicSocketTest")
{
    SECTION("Empty")
    {
        CHECK_FALSE(basic_socket());
        CHECK_FALSE(basic_socket().valid());

        CHECK_FALSE(basic_socket(nullptr));
        CHECK_FALSE(basic_socket(nullptr).valid());

        CHECK_FALSE(basic_socket(invalid_handle));
        CHECK_FALSE(basic_socket(invalid_handle, 0, 0, 0));

        CHECK_THROWS(basic_socket(AF_INET, SOCK_DGRAM, IPPROTO_TCP));  // wrong protocol
    }

    SECTION("Create")
    {
        // create via family
        basic_socket s1(AF_INET, SOCK_DGRAM);
        basic_socket s2(AF_INET, SOCK_STREAM);

        CHECK(s1);
        CHECK(s2);
        CHECK(!s2.nonblocking(true));

        CHECK(s1.native() != s2.native());  // handle is unique

        // create via handle
        basic_socket s3(s1.transfer());
        basic_socket s4(s2.transfer(), s2.family(), s2.type(), s2.protocol());

        CHECK_FALSE(s1);  // s1, s2 already detached
        CHECK_FALSE(s2);

        CHECK(s3);
        CHECK(s4);

        CHECK_THROWS(s3.reset());  // because no family info
        CHECK_NOTHROW(s4.reset());   // because family info is provided when construct
    }

    SECTION("TCP")
    {
        chen::semaphore sem;
        std::uint16_t port = 0;

        std::thread thread_s([&] () {
            // a simple echo server
            basic_socket server(AF_INET, SOCK_STREAM);

            CHECK(!server.bind(inet_address("127.0.0.1:0")));  // bind on a random port
            CHECK(!server.listen());
            CHECK((port = server.sock<inet_address>().port()) > 0);  // retrieve random port number

            // notify client to connect
            sem.post();

            // wait for new connections
            while (true)
            {
                basic_socket conn;
                CHECK(!server.accept(conn));

                // remote address
                CHECK("127.0.0.1" == conn.peer<inet_address>().addr().str());

                // retrieve available bytes to read
                CHECK_NOTHROW(conn.available());

                // receive message
                char buff[512]{};
                auto size = conn.recv(buff, 511);

                CHECK(size > 0);
                CHECK(size == conn.send(buff, size));

                // receive shutdown
                size = conn.recv(buff, 511);
                CHECK(size <= 0);

                if (::strcmp(buff, "\n") == 0)
                    break;
            }
        });

        std::thread thread_c([&] () {
            sem.wait();  // wait for server become ready

            inet_address addr("127.0.0.1", port);

            // a simple echo client
            for (int i = 0; i < 10; ++i)
            {
                basic_socket client(AF_INET, SOCK_STREAM);

                std::string text("hello, " + chen::num::str(i));

                CHECK(!client.connect(addr));
                CHECK(static_cast<xio::ssize_t>(text.size()) == client.send(text.data(), text.size()));

                char buff[512]{};
                auto size = client.recv(buff, 511);

                CHECK(text == buff);
                CHECK(static_cast<xio::ssize_t>(text.size()) == size);

                // the following code is just for demonstration purpose
                client.shutdown(basic_socket::Shutdown::Read);   // close read channel
                client.shutdown(basic_socket::Shutdown::Write);  // close write channel
                client.shutdown(basic_socket::Shutdown::Both);   // close read & write
                client.shutdown();  // the same to Shutdown::Both
            }

            // stop it
            basic_socket stop(AF_INET, SOCK_STREAM);

            CHECK(!stop.connect(addr));
            CHECK(1 == stop.send("\n", 1));
        });

        thread_s.join();
        thread_c.join();
    }

    SECTION("UDP")
    {
        chen::semaphore sem;
        std::uint16_t port = 0;

        std::thread thread_s([&] () {
            // a simple echo server
            basic_socket server(AF_INET, SOCK_DGRAM);

            CHECK(!server.bind(inet_address("127.0.0.1:0")));  // bind on a random port
            CHECK((port = server.sock<inet_address>().port()) > 0);  // retrieve random port number

            // notify client to send message
            sem.post();

            // wait for new messages to arrive
            while (true)
            {
                inet_address addr;

                char buff[512]{};
                auto size = server.recvfrom(buff, 511, addr);

                CHECK(size > 0);
                CHECK(size == server.sendto(buff, size, addr));

                // remote address
                CHECK("127.0.0.1" == inet_address(addr).addr().str());

                if (::strcmp(buff, "\n") == 0)
                    break;
            }
        });

        std::thread thread_c([&] () {
            sem.wait();  // wait for server become ready

            inet_address addr("127.0.0.1", port);

            // a simple echo client
            for (int i = 0; i < 10; ++i)
            {
                basic_socket client(AF_INET, SOCK_DGRAM);

                // bind on a fixed port, otherwise you may not receive the
                // server message because the random port may be released
                CHECK(!client.bind(inet_address("127.0.0.1:0")));

                std::string text("hello, " + chen::num::str(i));
                CHECK(static_cast<xio::ssize_t>(text.size()) == client.sendto(text.data(), text.size(), addr));

                char buff[512]{};
                auto size = client.recvfrom(buff, 511);

                CHECK(text == buff);
                CHECK(static_cast<xio::ssize_t>(text.size()) == size);
            }

            // stop it
            basic_socket stop(AF_INET, SOCK_DGRAM);
            CHECK(1 == stop.sendto("\n", 1, addr));
        });

        thread_s.join();
        thread_c.join();
    }
}
