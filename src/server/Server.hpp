#ifndef SERVER_HPP
#define SERVER_HPP

#include <asio.hpp>
#include <iostream>
#include <map>
#include <thread>
#include <atomic>

using ClientId = uint32_t;

struct Client {
    ClientId id;
    asio::ip::udp::endpoint endpoint;
};

class Server
{
public:
    Server(uint16_t port = 4242);
    ~Server();

    void run();
    void stop();

private:
    void receive();
    ClientId addClient(const asio::ip::udp::endpoint& endpoint);
    Client* findClient(const asio::ip::udp::endpoint& endpoint);

    asio::io_context m_io;
    asio::ip::udp::socket m_socket;
    
    std::map<ClientId, Client> m_clients;
    ClientId m_nextId = 1;
    
    std::array<char, 1024> m_buffer;
    asio::ip::udp::endpoint m_senderEndpoint;
    
    std::atomic<bool> m_running{false};
};

#endif
