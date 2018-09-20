
#include "HttpServer.h"
#include <iostream>

int main(int argc, char * argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 6)
        {
            std::cerr << "Usage: http_server_fast <address> <port> <doc_root> <num_workers>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    http_server_fast 0.0.0.0 80 . 100 block\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    http_server_fast 0::0 80 . 100 block\n";
            return EXIT_FAILURE;
        }

        auto const address = boost::asio::ip::make_address(argv[1]);
        const auto port = static_cast<unsigned short>(std::atoi(argv[2]));
        std::string doc_root = argv[3];
        const auto num_workers = std::atoi(argv[4]);

        boost::asio::io_context ioc { 1 };
        tcp::acceptor acceptor { ioc, { address, port } };

        std::list<HttpWorker> workers;
        for (int i = 0; i < num_workers; ++i)
        {
            workers.emplace_back(acceptor, doc_root);
            workers.back().start();
        }

        ioc.run();
    }
    catch (const std::exception & e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
