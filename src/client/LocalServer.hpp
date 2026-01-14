/*
** R-Type Client - LocalServer
** Wrapper for running a local GameServer instance for solo play
** 
** Provides Quake-style architecture where single-player games
** run on a local server with the same logic as multiplayer.
*/

#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <functional>

// Forward declaration to avoid including server headers in client
namespace rtype::server {
    class GameServer;
}

namespace rtype::client {

    /**
     * @brief Wrapper for running a local GameServer in a background thread
     * 
     * Usage:
     *   LocalServer localServer;
     *   localServer.start(true);  // allowSinglePlayer = true
     *   localServer.waitUntilReady();
     *   uint16_t port = localServer.getPort();
     *   // Connect NetworkClient to 127.0.0.1:port
     *   // ... play game ...
     *   localServer.stop();
     */
    class LocalServer {
    public:
        /**
         * @brief Construct a LocalServer (does not start server)
         */
        LocalServer();

        /**
         * @brief Destructor - stops server if running
         */
        ~LocalServer();

        // Non-copyable
        LocalServer(const LocalServer&) = delete;
        LocalServer& operator=(const LocalServer&) = delete;

        /**
         * @brief Start the local server in a background thread
         * 
         * @param allowSinglePlayer If true, game starts with 1 player
         * @param port Port to listen on (0 = auto-assign ephemeral port)
         * @return true if server started successfully
         */
        bool start(bool allowSinglePlayer = true, uint16_t port = 0);

        /**
         * @brief Stop the local server
         * 
         * Blocks until server thread has joined.
         */
        void stop();

        /**
         * @brief Wait until server is ready to accept connections
         * 
         * Blocks until the server has initialized and is listening.
         * Call this after start() before connecting.
         * 
         * @param timeoutMs Maximum time to wait in milliseconds (0 = infinite)
         * @return true if server is ready, false if timeout
         */
        bool waitUntilReady(uint32_t timeoutMs = 5000);

        /**
         * @brief Get the port the server is listening on
         * 
         * @return Port number (valid after waitUntilReady() returns true)
         */
        uint16_t getPort() const { return m_port; }

        /**
         * @brief Check if server is currently running
         */
        bool isRunning() const { return m_running.load(); }

        /**
         * @brief Check if server is ready for connections
         */
        bool isReady() const { return m_ready.load(); }

        /**
         * @brief Set callback for when server is ready
         * 
         * Called from server thread when initialization is complete.
         */
        void setOnReady(std::function<void(uint16_t port)> callback) {
            m_onReady = std::move(callback);
        }

    private:
        /**
         * @brief Server thread entry point
         */
        void serverThreadFunc(bool allowSinglePlayer, uint16_t requestedPort);

        /**
         * @brief Signal that server is ready
         */
        void signalReady(uint16_t port);

    private:
        // Server instance (created in thread)
        std::unique_ptr<rtype::server::GameServer> m_server;

        // Thread management
        std::thread m_serverThread;
        std::atomic<bool> m_running{false};
        std::atomic<bool> m_ready{false};

        // Synchronization for ready state
        std::mutex m_readyMutex;
        std::condition_variable m_readyCondition;

        // Server port (set when ready)
        uint16_t m_port{0};

        // Callbacks
        std::function<void(uint16_t port)> m_onReady;
    };

} // namespace rtype::client
