
//#include "HttpServer.h"
#include "stdafx.h"

#include "CrowServer.h"

#include "PppServer.h"

#include <iostream>

#define USE_CROW_SERVER 0


int main(int argc, char * argv[])
{
    uint16_t serverPort = 4000;
    auto idx = 0;
    while (idx < argc)
    {
        if (std::strcmp(argv[idx], "-p") == 0 || std::strcmp(argv[idx], "--port") == 0)
            serverPort = std::atoi(argv[++idx]);
        idx++;
    }

#if USE_CROW_SERVER
    CrowServer server;
    server.run(serverPort);
#else
    PppServer server;

    server.run();
#endif
}
