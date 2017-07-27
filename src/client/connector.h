#ifndef CLIENT_CONNECTOR_H
#define CLIENT_CONNECTOR_H

#include <vector>
#include <netinet/in.h>

class Connector
{
public:
    Connector (const char* ip, uint16_t port);
    ~Connector ();

    bool Init();

    bool Send(char* buffer, uint32_t bufSize);

private:
    sockaddr_in serverAddr;
    int sock;
};

#endif /* end of include guard: CLIENT_CONNECTOR_H */
