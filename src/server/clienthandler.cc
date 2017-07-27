#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server/clienthandler.h"

ClientHandler::ClientHandler(int _sock, sockaddr_in _clientAddr)
: sock(_sock)
, clientAddr(_clientAddr)
, thread(&ClientHandler::Run, this)
{ 
    std::ostringstream clientNameStrm;
    clientNameStrm << "[" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "]";
    clientName = clientNameStrm.str();

    std::cout << "Initializing handlers for client " << clientName << std::endl;
}

ClientHandler::ClientHandler(ClientHandler&& moveFrom)
: sock(moveFrom.sock)
, clientAddr(moveFrom.clientAddr)
, thread(std::move(moveFrom.thread))
{
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::Run()
{
    socklen_t bufSize;
    socklen_t optSize = sizeof(bufSize);
    if(getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufSize, &optSize) != 0)
    {
        // default 64K recv buffer
        bufSize = 64 * 1024;
    }

    char buffer[bufSize];
    
    bool stopConn = false;
    do
    {
        int recvLen = recv(sock, buffer, bufSize, 0);
        
        if(recvLen > 0)
        {
            std::cout << buffer << std::endl;
        }
        else
        {
            std::cout << "Connect to client " << clientName << "is lost" << std::endl;
            stopConn = true;
        }

    }while(!stopConn);

    close(sock);
}
