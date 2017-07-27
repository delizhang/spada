#include "server/connector.h"
#include "common/network/connection.h"

int main(int argc, const char *argv[])
{
    Connector connector(DEFAULT_PORT);

    if(connector.Init())
    {
        connector.Run();
    }

    return 0;
}
