/**
 * Created by Jian Chen
 * @since  2016.08.21
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#pragma once

#include <socket/tcp/tcp_basic.hpp>

namespace chen
{
    namespace tcp
    {
        class conn : public basic
        {
        public:
            conn(socket_t fd);

        protected:
            /**
             * Event callbacks
             */
            virtual void onEventSend(std::size_t size, std::error_code error) override;
            virtual void onEventRecv(std::vector<std::uint8_t> data, std::error_code error) override;
            virtual void onEventEOF() override;

//        public:
//            /**
//             * Send data to connected host
//             */
//            using basic::send;
//            ssize_t send(const std::vector<std::uint8_t> &data, int flags = 0);
//
//            /**
//             * Receive data from connected host
//             */
//            using basic::recv;
//            std::vector<std::uint8_t> recv(std::size_t size, int flags = 0);
//
//        public:
//            /**
//             * Close the connection
//             */
//            void disconnect();
        };
    }
}