/*
** R-Type Server - Main Entry Point
** Headless game server with fixed 60 Hz simulation
** Network synchronization will be added in future iterations
*/

#include "GameServer.hpp"
#include <csignal>
#include <iostream>

// Global pointer for signal handling
rtype::server::GameServer* g_gameServer = nullptr;

/**
 * @brief Signal handler for graceful shutdown (Ctrl+C)
 */
void onSignal(int signum)
{
    std::cout << "\n[Main] Received signal " << signum << ", shutting down..." << std::endl;
    if (g_gameServer) {
        g_gameServer->stop();
    }
}

/**
 * @brief Server entry point
 *
 * Initializes and runs the headless game simulation.
 * Press Ctrl+C to stop the server gracefully.
 */
int main()
{
    std::cout << "=====================================" << std::endl;
    std::cout << "   R-Type Multiplayer Server" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;

    // Register signal handlers
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    try {
        // Create and initialize game server
        rtype::server::GameServer gameServer;
        g_gameServer = &gameServer;

        gameServer.initialize();

        std::cout << std::endl;
        std::cout << "[Main] Server ready! Press Ctrl+C to stop." << std::endl;
        std::cout << std::endl;

        // Run game loop (blocks until stopped)
        gameServer.run();

        std::cout << std::endl;
        std::cout << "[Main] Final Statistics:" << std::endl;
        std::cout << "  - Total ticks: " << gameServer.getTickCount() << std::endl;
        std::cout << "  - Total runtime: " << gameServer.getGameTime() << "s" << std::endl;
        std::cout << "  - Active entities: " << gameServer.getEntityCount() << std::endl;
        std::cout << std::endl;
        std::cout << "[Main] Server shutdown complete." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[Main] Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
