/*
** R-Type Server - Main Entry Point
** Headless game server with fixed 60 Hz simulation
** Network synchronization will be added in future iterations
**
** Usage: ./r-type_server [OPTIONS]
**   -p, --port PORT    Server port (default: 4242)
**   -h, --help         Show this help message
*/

#include "GameServer.hpp"
#include <csignal>
#include <iostream>
#include <string>
#include <cstring>

// Default configuration
constexpr uint16_t DEFAULT_PORT = 4242;

// Global pointer for signal handling
rtype::server::GameServer* g_gameServer = nullptr;

/**
 * @brief Print usage information
 */
void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " [OPTIONS]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -p, --port PORT    Server port (default: " << DEFAULT_PORT << ")" << std::endl;
    std::cout << "  -h, --help         Show this help message" << std::endl;
}

/**
 * @brief Parse command-line arguments
 * @param argc Argument count
 * @param argv Argument values
 * @param port Output: server port
 * @return true if arguments are valid, false otherwise
 */
bool parseArguments(int argc, char* argv[], uint16_t& port)
{
    port = DEFAULT_PORT;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return false;
        }
        else if (arg == "-p" || arg == "--port") {
            if (i + 1 >= argc) {
                std::cerr << "[Error] Missing port value for " << arg << std::endl;
                printUsage(argv[0]);
                return false;
            }
            try {
                int portValue = std::stoi(argv[++i]);
                if (portValue < 1 || portValue > 65535) {
                    std::cerr << "[Error] Port must be between 1 and 65535" << std::endl;
                    return false;
                }
                port = static_cast<uint16_t>(portValue);
            } catch (const std::exception& e) {
                std::cerr << "[Error] Invalid port number: " << argv[i] << std::endl;
                return false;
            }
        }
        else {
            std::cerr << "[Error] Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return false;
        }
    }
    return true;
}

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
 *
 * @param argc Argument count
 * @param argv Argument values
 */
int main(int argc, char* argv[])
{
    // Parse command-line arguments
    uint16_t port;
    if (!parseArguments(argc, argv, port)) {
        return 0;  // --help was shown or error occurred
    }

    std::cout << "=====================================" << std::endl;
    std::cout << "   R-Type Multiplayer Server" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;
    std::cout << "[Main] Starting server on port " << port << std::endl;

    // Register signal handlers
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    try {
        // Create and initialize game server with specified port
        rtype::server::GameServer gameServer(port);
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
