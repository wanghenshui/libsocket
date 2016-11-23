/**
 * Created by Jian Chen
 * @since  2016.11.22
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include <socket/base/reactor_poller.hpp>
#include <chen/sys/sys.hpp>
#include <algorithm>

// -----------------------------------------------------------------------------
// define
#ifdef _WIN32
namespace
{
	inline int poll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout)
	{
		return ::WSAPoll(fdArray, fds, timeout);
	}
}
#endif

// only Linux defines POLLRDHUP, you must
// use it to detect close event on Linux
#ifndef POLLRDHUP
#define POLLRDHUP 0
#endif


// -----------------------------------------------------------------------------
// poller
chen::reactor_poller::reactor_poller()
{
}

chen::reactor_poller::~reactor_poller()
{
}

// modify
void chen::reactor_poller::set(handle_t fd, int opcode, int flag)
{
    auto find = std::find_if(this->_fds.begin(), this->_fds.end(), [&] (::pollfd &item) {
        return item.fd == fd;
    });

    if (find == this->_fds.end())
        find = this->_fds.insert(this->_fds.end(), ::pollfd());

    find->fd = fd;
    find->events = POLLRDHUP;

    if (opcode & OpcodeRead)
        find->events |= POLLIN;

    if (opcode & OpcodeWrite)
        find->events |= POLLOUT;

    this->_flags[fd] = flag;
}

void chen::reactor_poller::del(handle_t fd)
{
    std::remove_if(this->_fds.begin(), this->_fds.end(), [&] (::pollfd &item) {
        return item.fd == fd;
    });

    this->_flags.erase(fd);
}

std::size_t chen::reactor_poller::poll(std::vector<Data> &cache, std::size_t count, double timeout)
{
    // ignore if it's empty or just a wakeup socket in it
    if (!count || this->_fds.empty() || (this->_fds.front().fd == this->_wake.native()))
        return 0;

    // reset wake event
    if (!this->_wake)
    {
        this->_wake.reset(AF_INET, SOCK_DGRAM);
        this->set(this->_wake.native(), OpcodeRead);
    }

    // temporary use only
    auto fds   = this->_fds;
    auto flags = this->_flags;

    // poll next events
    auto result = ::poll(fds.data(), static_cast<unsigned>(fds.size()), timeout < 0 ? -1 : static_cast<int>(timeout * 1000));

    if (result <= 0)
    {
        // EINTR maybe triggered by debugger, treat it as user request to stop
        if ((errno == EINTR) || !result)
            return 0;
        else
            throw std::system_error(sys::error(), "poller: failed to poll event");
    }

    // user request to stop
    if (!this->_wake)
        return 0;

    // collect poll data
    auto origin = cache.size();

    for (std::size_t i = 0, l = fds.size(); i < l; ++i)
    {
        auto &event = fds[i];
        if (event.revents == 0)
            continue;

        // check events, multiple events maybe occur
        auto insert = [&] (Event code) {
            // remove fd if Ended event occurs or flag is once
            if ((code == Event::Ended) || (flags[event.fd] & FlagOnce))
                this->del(event.fd);

            if (i < origin)
                cache[i] = Data(event.fd, code);
            else
                cache.emplace_back(Data(event.fd, code));
        };

        if ((event.revents & POLLRDHUP) || (event.revents & POLLERR) || (event.revents & POLLHUP))
        {
            insert(Event::Ended);
        }
        else
        {
            if (event.revents & POLLIN)
                insert(Event::Readable);

            if (event.revents & POLLOUT)
                insert(Event::Writable);
        }
    }

    return static_cast<std::size_t>(cache.size() - origin);
}

std::vector<chen::reactor_poller::Data> chen::reactor_poller::poll(std::size_t count, double timeout)
{
    std::vector<chen::reactor_poller::Data> ret;
    this->poll(ret, count, timeout);
    return ret;
}

void chen::reactor_poller::stop()
{
    // notify wake message via socket's close event
    this->_wake.close();
}