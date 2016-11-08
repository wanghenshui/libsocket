/**
 * Created by Jian Chen
 * @since  2016.11.07
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#pragma once

#include <socket/bsd/basic_socket.hpp>
#include <socket/bsd/dgram_packet.hpp>
#include <string>

namespace chen
{
    class dgram_socket
    {
    public:
        dgram_socket();

    public:
        /**
         * Read a packet from remote
         * the read callback will be invoked if successful
         * @param size the desired read length, actual size will be less or equal than this value
         */
        void read(std::size_t size);

        /**
         * Write a packet to remote
         * the write callback will be invoked if successful
         */
        void write(const char *text, const basic_address &addr);
        void write(const std::string &text, const basic_address &addr);
        void write(const void *data, std::size_t size, const basic_address &addr);

    protected:
        /**
         * Internal helper for read & write
         */
        void recv();
        void send();
        bool send(const void *data, std::size_t size, const basic_address &addr);

    protected:
        basic_socket _socket;

        std::vector<std::uint8_t> _receive;  // receive buffer, avoid recreated every time
        std::vector<dgram_packet> _pending;  // pending packet, will be sent out when writable
    };
}