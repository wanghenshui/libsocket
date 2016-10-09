/**
 * Created by Jian Chen
 * @since  2016.09.19
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#ifndef _WIN32

#include <socket/net/net_proactor.hpp>
#include <chen/sys/sys.hpp>
#include <sys/event.h>

// -----------------------------------------------------------------------------
// helper
namespace
{
    // todo didn't EV_DELETE events
    void register_write(int k, int fd, void *data)
    {
        struct ::kevent event{};
        EV_SET(&event, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, data);

        if (::kevent(k, &event, 1, nullptr, 0, nullptr) < 0)
            throw std::system_error(chen::sys::error(), "proactor: failed to add write event");
    }

    void register_read(int k, int fd, void *data)
    {
        struct ::kevent event{};
        EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, data);

        if (::kevent(k, &event, 1, nullptr, 0, nullptr) < 0)
            throw std::system_error(chen::sys::error(), "proactor: failed to add read event");
    }
}


// -----------------------------------------------------------------------------
// proactor
chen::net::proactor::proactor()
{
    if ((this->_fd = ::kqueue()) < 0)
        throw std::system_error(sys::error(), "proactor: failed to create kqueue");

    if (::pipe(this->_pp) != 0)
    {
        ::close(this->_fd);
        throw std::system_error(sys::error(), "proactor: failed to create pipe");
    }

    // register pipe to receive stop message
    ::register_read(this->_fd, this->_pp[0], nullptr);
}

chen::net::proactor::~proactor()
{
    ::close(this->_fd);
    ::close(this->_pp[0]);
    ::close(this->_pp[1]);
}

void chen::net::proactor::read(net::socket *ptr, std::size_t size)
{
    this->_read[ptr].push(chunk(size));
    ::register_read(this->_fd, ptr->native(), ptr);
}

void chen::net::proactor::write(net::socket *ptr, std::vector<std::uint8_t> &&data)
{
    this->_write[ptr].push(std::move(data));
    ::register_write(this->_fd, ptr->native(), ptr);
}

void chen::net::proactor::remove(net::socket *ptr)
{
    this->_write.erase(ptr);
    this->_read.erase(ptr);
}

void chen::net::proactor::start()
{
    struct ::kevent event{};

    while (true)
    {
        if (::kevent(this->_fd, nullptr, 0, &event, 1, nullptr) != 1)
            throw std::system_error(sys::error(), "proactor: failed to wait event");

        // check pipe to see if user want to stop the proactor
        if (event.ident == static_cast<uintptr_t>(this->_pp[0]))
        {
            char dummy;
            ::read(this->_pp[0], &dummy, 1);
            break;
        }

        // retrieve the socket pointer associated with event
        auto ptr = static_cast<net::socket*>(event.udata);
        if (!ptr)
            throw std::system_error(sys::error(), "proactor: event happened but no related socket");

        // connection refused, disconnect or other error
        if (event.flags & EV_EOF)
        {
            // todo clear cache first?
            ptr->onEnd({});
            continue;
        }

        // simulate proactor, notify callback after read or write data
        if (event.filter == EVFILT_WRITE)
        {
            auto &list = this->_write[ptr];
            if (list.empty())
                continue;

            // todo record original size
            auto &chunk = list.front();
            auto length = ptr->handle().send(chunk.data(), chunk.size());

            if (length >= 0)
            {
                chunk.resize(chunk.size() - length);

                if (chunk.empty())
                {
                    // all data have been sent
                    list.pop();
                    ptr->onWrite(static_cast<std::size_t>(length), {});
                }
                else
                {
                    // wait for next write time point
                    ::register_write(this->_fd, ptr->native(), ptr);
                }
            }
            else
            {
                ptr->onWrite(0, sys::error());
            }
        }
        else if (event.filter == EVFILT_READ)
        {
            auto &list = this->_read[ptr];
            if (list.empty())
                continue;

            auto chunk = std::move(list.front());
            list.pop();

            if (!chunk.empty())
            {
                auto len = ptr->handle().recv(chunk.data(), chunk.size());
                ptr->onRead(std::move(chunk), len < 0 ? sys::error() : std::error_code());
            }
            else
            {
                ptr->onRead(std::move(chunk), {});
            }
        }
        else
        {
            throw std::system_error(sys::error(), "proactor: event happened but flags and filter are unknown");
        }
    }
}

void chen::net::proactor::stop()
{
    // notify stop message via pipe
    if (::write(this->_pp[1], "\n", 1) != 1)
        throw std::system_error(sys::error(), "proactor: failed to stop the proactor");
}

#endif