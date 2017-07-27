#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <vector>
#include <netinet/in.h>
#include "server/clienthandler.h"

class Connector
{
public:
    Connector (uint16_t port);
    ~Connector ();

    bool Init();

    void Run();

private:
    int port;
    int sock;
    sockaddr_in serverAddr;
    std::vector<ClientHandler> handlers;
};

#endif /* end of include guard: CONNECTOR_H */
