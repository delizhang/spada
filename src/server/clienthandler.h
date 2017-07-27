#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <thread>
#include <netinet/in.h>

class ClientHandler
{
public:
    ClientHandler (int sock, sockaddr_in clientAddr);
    ClientHandler (ClientHandler&& moveFrom) noexcept;
    ~ClientHandler ();

    void Run();

private:
    int sock;
    sockaddr_in clientAddr;
    std::thread thread;
    std::string clientName;
};

#endif /* end of include guard: CLIENTHANDLER_H */
