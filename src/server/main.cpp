#include <asio.hpp>
#include <iostream>

int main()
{
    try {
        asio::io_context io_context;

        // Create UDP socket on port 4242
        asio::ip::udp::socket socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 4242));

        std::cout << "R-Type Server started on port 4242" << std::endl;

        // Simple test: wait for one message
        char recv_buffer[1024];
        asio::ip::udp::endpoint remote_endpoint;
        
        std::cout << "Waiting for connections..." << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
