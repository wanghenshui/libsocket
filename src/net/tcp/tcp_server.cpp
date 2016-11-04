/**
 * Created by Jian Chen
 * @since  2016.11.02
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include <socket/net/inet/net_resolver.hpp>
#include <socket/net/tcp/tcp_server.hpp>
#include <chen/base/str.hpp>
#include <algorithm>

// -----------------------------------------------------------------------------
// server
chen::tcp::server::server(runloop &runloop, std::uint16_t port, ip::address::Type type) : _runloop(runloop)
{
    this->_local.port(port);
    this->_local.addr(ip::address(type));

    this->reset(this->_local.addr().type());
}

chen::tcp::server::server(runloop &runloop, const char *mixed) : server(runloop, mixed, ip::address::Type::None)
{
}

chen::tcp::server::server(runloop &runloop, const std::string &mixed, ip::address::Type type) : _runloop(runloop)
{
    auto ret = net::resolver::resolve(mixed, type);
    if (ret.empty())
        throw std::runtime_error(str::format("tcp: server resolve address '%s' fail", mixed.c_str()));

    this->_local = ret.front();

    this->reset(this->_local.addr().type());
}

chen::tcp::server::server(runloop &runloop, const std::string &host, std::uint16_t port, ip::address::Type type) : _runloop(runloop)
{
    auto ret = net::resolver::resolve(host, port, type);
    if (ret.empty())
        throw std::runtime_error(str::format("tcp: server resolve address '%s' fail", host.c_str()));

    this->_local = ret.front();

    this->reset(this->_local.addr().type());
}

chen::tcp::server::server(runloop &runloop, const std::string &host, const std::string &service, ip::address::Type type) : _runloop(runloop)
{
    auto ret = net::resolver::resolve(host, service, type);
    if (ret.empty())
        throw std::runtime_error(str::format("tcp: server resolve address '%s' or service '%s' fail", host.c_str(), service.c_str()));

    this->_local = ret.front();

    this->reset(this->_local.addr().type());
}

chen::tcp::server::server(runloop &runloop, const net::endpoint &ep) : _local(ep), _runloop(runloop)
{
    this->reset(this->_local.addr().type());
}

chen::tcp::server::~server()
{
    this->stop();
}

// control
void chen::tcp::server::stop()
{
    if (this->_socket)
    {
        this->_runloop.del(this->_socket.native());

        this->_socket.shutdown();
        this->_socket.close();
    }

    this->_running = false;
}

// property
bool chen::tcp::server::isRunning()
{
    return this->_running;
}

chen::net::endpoint chen::tcp::server::local() const
{
    return this->_local;
}

// serve
void chen::tcp::server::listen(int backlog)
{
    // stop if running
    if (this->isRunning())
        this->stop();

    // reset socket
    if (!this->_socket)
        this->reset(this->_local.addr().type());

    this->nonblocking(true);
    this->option().reuseaddr(true);

    // bind and listen
    auto err = this->_socket.bind(this->_local);
    if (err)
        throw std::system_error(err, "tcp: server bind error");

    err = this->_socket.listen(backlog);
    if (err)
        throw std::system_error(err, "tcp: server listen error");

    // listen events using runloop
    this->_runloop.set(this->_socket.native(),
                       runloop::OpcodeRead | runloop::OpcodeWrite,
                       runloop::FlagEdge,
                       std::bind(&server::onServerEvent, this, std::placeholders::_1));

    this->_running = true;
}

// server
void chen::tcp::server::onServerReadable()
{
    basic_socket s = this->_socket.accept();
    if (!s)
        return;

    auto &ptr = *this->_store.insert(this->_store.end(), std::unique_ptr<conn>(new conn(std::move(s), this->_factory())));
    this->_runloop.set(ptr->native(),
                       runloop::OpcodeRead | runloop::OpcodeWrite,
                       runloop::FlagEdge,
                       std::bind(&server::onConnEvent, this, std::ref(ptr), std::placeholders::_1));

    ptr->onAccepted();
}

void chen::tcp::server::onServerWritable()
{
}

void chen::tcp::server::onServerEnded()
{
}

void chen::tcp::server::onServerEvent(runloop::Event type)
{
    switch (type)
    {
        case runloop::Event::Read:
            return this->onServerReadable();

        case runloop::Event::Write:
            return this->onServerWritable();

        case runloop::Event::End:
            return this->onServerEnded();
    }
}

// connection
void chen::tcp::server::onConnReadable(std::unique_ptr<conn> &c)
{
    c->onReadable();
}

void chen::tcp::server::onConnWritable(std::unique_ptr<conn> &c)
{
    c->onWritable();
}

void chen::tcp::server::onConnEnded(std::unique_ptr<conn> &c)
{
    // check if the connection is disconnected passively
    if (!c->isDisconnect())
        c->onEnded();

    // delete connection from runloop
    this->_runloop.del(c->native());

    // delete connection from connection pool
    auto it = std::find(this->_store.begin(), this->_store.end(), c);
    this->_store.erase(it);
}

void chen::tcp::server::onConnEvent(std::unique_ptr<conn> &c, runloop::Event type)
{
    switch (type)
    {
        case runloop::Event::Read:
            return this->onConnReadable(c);

        case runloop::Event::Write:
            return this->onConnWritable(c);

        case runloop::Event::End:
            return this->onConnEnded(c);
    }
}