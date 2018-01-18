/**
 * libxio: A Modern C++ Network Library
 * @author    Jian Chen <admin@chensoft.com>
 * @link      http://chensoft.com
 * @license   Licensed under MIT license
 * @copyright Copyright 2016 Jian Chen
 */
#pragma once

// version
constexpr int SOCKET_VERSION_MAJOR = 0;
constexpr int SOCKET_VERSION_MINOR = 1;
constexpr int SOCKET_VERSION_PATCH = 0;

constexpr const char *SOCKET_VERSION = "0.1.0";

// libchen
#include "chen/chen.hpp"

// libxio
#include "xio/config.hpp"

#include "xio/base/basic_address.hpp"
#include "xio/base/basic_option.hpp"
#include "xio/base/basic_socket.hpp"
#include "xio/base/ev_base.hpp"
#include "xio/base/ev_event.hpp"
#include "xio/base/ev_handle.hpp"
#include "xio/base/ev_timer.hpp"

#include "xio/core/ioctl.hpp"
#include "xio/core/reactor.hpp"
#include "xio/core/startup.hpp"

#include "xio/inet/inet_adapter.hpp"
#include "xio/inet/inet_address.hpp"
#include "xio/inet/inet_resolver.hpp"

#include "xio/ip/ip_address.hpp"
#include "xio/ip/ip_option.hpp"
#include "xio/ip/ip_version.hpp"