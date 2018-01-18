/**
 * Created by Jian Chen
 * @since  2017.02.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "xio/base/ev_base.hpp"

// -----------------------------------------------------------------------------
// ev_base
const int xio::ev_base::Readable = 1 << 0;
const int xio::ev_base::Writable = 1 << 1;
const int xio::ev_base::Closed   = 1 << 2;

void xio::ev_base::onAttach(reactor *loop, int mode, int flag)
{
    this->_ev_loop = loop;
    this->_ev_mode = mode;
    this->_ev_flag = flag;
}

void xio::ev_base::onDetach()
{
    this->_ev_loop = nullptr;
    this->_ev_mode = 0;
    this->_ev_flag = 0;
}