#include "Server.hpp"
#include <csignal>

Server* g_server = nullptr;

void onSignal(int)
{
    if (g_server)
        g_server->stop();
}

int main()
{
    std::signal(SIGINT, onSignal);

    Server server(4242);
    g_server = &server;

    server.run();

    return 0;
}
