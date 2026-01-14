/*
** R-Type Client - LocalServer Implementation
** Background server for solo play
*/

#include "LocalServer.hpp"
#include "../server/GameServer.hpp"

#include <chrono>

namespace rtype::client {

    LocalServer::LocalServer() = default;

    LocalServer::~LocalServer() {
        stop();
    }

    bool LocalServer::start(bool allowSinglePlayer, uint16_t port) {
        // Don't start if already running
        if (m_running.load()) {
            std::cerr << "[LocalServer] Already running!" << std::endl;
            return false;
        }

        m_running.store(true);
        m_ready.store(false);
        m_port = 0;

        // Start server thread
        m_serverThread = std::thread(&LocalServer::serverThreadFunc, this, allowSinglePlayer, port);

        std::cout << "[LocalServer] Starting local server thread..." << std::endl;
        return true;
    }

    void LocalServer::stop() {
        if (!m_running.load()) {
            return;
        }

        std::cout << "[LocalServer] Stopping local server..." << std::endl;

        // Signal server to stop
        if (m_server) {
            m_server->stop();
        }

        m_running.store(false);

        // Wake up any waiting threads
        {
            std::lock_guard<std::mutex> lock(m_readyMutex);
            m_ready.store(false);
        }
        m_readyCondition.notify_all();

        // Wait for thread to finish
        if (m_serverThread.joinable()) {
            m_serverThread.join();
        }

        m_server.reset();
        m_port = 0;

        std::cout << "[LocalServer] Local server stopped." << std::endl;
    }

    bool LocalServer::waitUntilReady(uint32_t timeoutMs) {
        std::unique_lock<std::mutex> lock(m_readyMutex);

        if (m_ready.load()) {
            return true;
        }

        if (timeoutMs == 0) {
            // Wait indefinitely
            m_readyCondition.wait(lock, [this] {
                return m_ready.load() || !m_running.load();
            });
        } else {
            // Wait with timeout
            bool result = m_readyCondition.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] {
                return m_ready.load() || !m_running.load();
            });
            if (!result) {
                std::cerr << "[LocalServer] Timeout waiting for server ready" << std::endl;
                return false;
            }
        }

        return m_ready.load();
    }

    void LocalServer::serverThreadFunc(bool allowSinglePlayer, uint16_t requestedPort) {
        std::cout << "[LocalServer] Server thread started" << std::endl;

        try {
            // Use requested port or default to 4243 for local server
            // (4242 is default for remote, 4243 avoids conflict)
            uint16_t port = (requestedPort == 0) ? 4243 : requestedPort;

            // Create server with single-player mode enabled
            m_server = std::make_unique<rtype::server::GameServer>(port, allowSinglePlayer);

            // Initialize (sets up systems, network)
            m_server->initialize();

            // Signal ready
            signalReady(port);

            // Run game loop (blocks until stop() called)
            m_server->run();

        } catch (const std::exception& e) {
            std::cerr << "[LocalServer] Server error: " << e.what() << std::endl;
        }

        std::cout << "[LocalServer] Server thread exiting" << std::endl;
    }

    void LocalServer::signalReady(uint16_t port) {
        {
            std::lock_guard<std::mutex> lock(m_readyMutex);
            m_port = port;
            m_ready.store(true);
        }
        m_readyCondition.notify_all();

        if (m_onReady) {
            m_onReady(port);
        }

        std::cout << "[LocalServer] Server ready on port " << port << std::endl;
    }

} // namespace rtype::client
