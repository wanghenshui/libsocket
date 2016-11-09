/**
 * Created by Jian Chen
 * @since  2016.09.02
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include <socket/tcp/tcp_option.hpp>

// -----------------------------------------------------------------------------
// option
chen::tcp::option::option(bsd_socket &s) : ip_option(s)
{
}

// nodelay
bool chen::tcp::option::nodelay() const
{
    return bsd_option::get(IPPROTO_TCP, TCP_NODELAY) != 0;
}

bool chen::tcp::option::nodelay(bool val)
{
    return option::set(IPPROTO_TCP, TCP_NODELAY, val);
}