#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "client/connector.h"

Connector::Connector(const char* _ip, uint16_t _port)
: serverAddr({ AF_INET, htons(_port), { inet_addr(_ip) } })
, sock(0)
{ 
}

Connector::~Connector()
{
    if(sock > 0)
    {
        close(sock);
    }
}

bool Connector::Init()
{
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0)
    {
        std::cout << "Failed to create socket" << std::endl;
        return false;
    }

    if(connect(sock,(sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cout << "Failed to connect to server" << std::endl;
        return false;
    }
    
    return true;
}

bool Connector::Send(char* buffer, uint32_t bufSize)
{
    size_t sentSize = send(sock, buffer, bufSize, 0);
   
    return sentSize > 0;
}
