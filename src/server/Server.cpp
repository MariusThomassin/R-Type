#include "Server.hpp"

Server::Server(uint16_t port)
    : m_socket(m_io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
{
    std::cout << "[Server] Ready on port " << port << std::endl;
}

Server::~Server()
{
    stop();
}

void Server::run()
{
    m_running = true;
    std::cout << "[Server] Running..." << std::endl;

    while (m_running) {
        receive();
    }
}

void Server::stop()
{
    m_running = false;
    m_socket.close();
    std::cout << "[Server] Stopped" << std::endl;
}

void Server::receive()
{
    m_socket.non_blocking(true);
    asio::error_code ec;

    size_t len = m_socket.receive_from(
        asio::buffer(m_buffer), m_senderEndpoint, 0, ec);

    if (ec == asio::error::would_block) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return;
    }

    if (ec) {
        return;
    }

    Client* client = findClient(m_senderEndpoint);
    if (!client) {
        ClientId id = addClient(m_senderEndpoint);
        std::cout << "[Server] Client #" << id << " connected" << std::endl;
    }

    std::string msg(m_buffer.data(), len);
    std::cout << "[Server] Received: " << msg << std::endl;
}

ClientId Server::addClient(const asio::ip::udp::endpoint& endpoint)
{
    ClientId id = m_nextId++;
    m_clients[id] = {id, endpoint};
    return id;
}

Client* Server::findClient(const asio::ip::udp::endpoint& endpoint)
{
    for (auto& [id, client] : m_clients) {
        if (client.endpoint == endpoint) {
            return &client;
        }
    }
    return nullptr;
}
