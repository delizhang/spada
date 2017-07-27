#include <iostream>
#include <sys/socket.h>
#include "server/connector.h"

Connector::Connector(uint16_t _port)
: port(_port)
, sock(0)
, serverAddr({ AF_INET, htons(port), { INADDR_ANY } })
{ 
}

Connector::~Connector()
{

}

bool Connector::Init()
{
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0)
    {
        std::cout << "Failed to create socket on port " << port << std::endl;
        return false;
    }

    if(bind(sock,(sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cout << "Failed to bind to socket address" << std::endl;
        return false;
    }
    
    if(listen(sock, SOMAXCONN) < 0)
    {
        std::cout << "Failed to listen on socket" << std::endl;
        return false;
    }
   
    return true;
}

void Connector::Run()
{
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSock;
    do
    {
        clientSock = accept(sock, (sockaddr*)&clientAddr, &clientAddrLen);

        if(clientSock > 0)
        {
            handlers.push_back(ClientHandler(clientSock, clientAddr));
        }
    }
    while(clientSock > 0);

    std::cout << "Failed to accept incoming client connections" << std::endl;
}
