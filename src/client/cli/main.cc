#include <iostream>
#include "client/connector.h"
#include "common/network/connection.h"

int main(int argc, const char *argv[])
{
    Connector connector("127.0.0.1", DEFAULT_PORT);

    if(connector.Init())
    {
        std::cout << "Sever connected" << std::endl;

        const int bufSize = 1024;
        char buffer[bufSize];

        do{
            std::cout << "Spada: ";
            std::cin.getline(buffer, bufSize); 
        } while(connector.Send(buffer, bufSize));
    }

    return 0;
}
