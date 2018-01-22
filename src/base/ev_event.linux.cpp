/**
 * Created by Jian Chen
 * @since  2017.02.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#ifdef __linux__

#include "xio/base/ev_event.hpp"
#include "xio/core/reactor.hpp"
#include "chen/sys/sys.hpp"
#include <sys/eventfd.h>

// -----------------------------------------------------------------------------
// ev_event
xio::ev_event::ev_event(std::function<void ()> cb) : _notify(std::move(cb))
{
    auto fd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (fd < 0)
        throw std::system_error(chen::sys::error(), "event: failed to create eventfd");

    this->change(fd);
}

xio::ev_event::~ev_event()
{
}

void xio::ev_event::set()
{
    ::eventfd_write(this->native(), 1);
}

void xio::ev_event::reset()
{
    ::eventfd_t dummy;
    ::eventfd_read(this->native(), &dummy);
}

// notify
void xio::ev_event::attach(std::function<void ()> cb)
{
    this->_notify = std::move(cb);
}

// event
void xio::ev_event::onEvent(int type)
{
    auto loop = this->evLoop();
    auto func = this->_notify;

    if (loop && ((type & Closed) || (this->evFlag() & reactor::FlagOnce)))
        loop->del(this);

    if (func)
        func();
}

#endif