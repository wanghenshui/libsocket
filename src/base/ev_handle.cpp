/**
 * Created by Jian Chen
 * @since  2017.02.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/base/ev_handle.hpp"
#include "xio/core/reactor.hpp"

// -----------------------------------------------------------------------------
// ev_handle
xio::ev_handle::~ev_handle()
{
    this->close();
}

// control
void xio::ev_handle::change(handle_t fd)
{
    this->close();
    this->_fd = fd;
}

void xio::ev_handle::close()
{
    if (this->_fd == invalid_handle)
        return;

#ifdef _WIN32
    // WSAPoll only support SOCKET, so use closesocket is correct
    ::closesocket(this->transfer());
#else
    ::close(this->transfer());
#endif
}

xio::handle_t xio::ev_handle::transfer()
{
    if (this->evLoop())
        this->evLoop()->del(this);

    auto temp = this->_fd;
    this->_fd = invalid_handle;
    return temp;
}

// event
void xio::ev_handle::onAttach(reactor *loop, int mode, int flag)
{
    if (this->evLoop())
        this->evLoop()->del(this);

    ev_base::onAttach(loop, mode, flag);
}