#include "version.h"
#include "server.h"
#include "error.h"
#include <boost/asio.hpp>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <cstdint> //uint8_t, uint32_t

using boost::system::error_code;

namespace
{

    void printHelp(const std::string& programName)
    {
        std::cout << "Usage: " << programName << " -s/--secret <secret> [-p/--port <port>] [-h/--help] [-v/--version]\n"
                  << "\t --secret, -s <secret> - shared secret for password encryption by client and server;\n"
                  << "\t --port, -p <port>     - port number for the socket;\n"
                  << "\t --help, -h            - print this help;\n"
                  << "\t --version, -v         - print version." <<
            std::endl;
    }

    void printVersion(const std::string& programName)
    {
        std::cout << programName << "\n"
                  << "radproto" <<  " " << RADIUSD::version << "\n";
    }

}

int main(int argc, char* argv[])
{
    std::string secret;
    std::string dictionary;
    uint16_t port = 1812;

    if (argc < 2)
    {
        std::cerr << "Needs a parameter secret - shared secret "
          "for password encryption by client and server." << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg(argv[i]);
        if (arg == "--help" || arg == "-h")
        {
            printHelp(argv[0]);
            return 0;
        }

        if (arg == "--version" || arg == "-v")
        {
            printVersion(argv[0]);
            return 0;
        }

        if (arg == "--port" || arg == "-p")
        {
            if (i + 1 == argc)
            {
                std::cerr << arg << " required argument - port number." << std::endl;
                return 1;
            }
            port = std::stoul(argv[++i]);
        }
        else if (arg == "--secret" || arg == "-s")
        {
            if (i + 1 == argc)
            {
                std::cerr << arg << " needs an argument - a shared secret." << std::endl;
                return 1;
            }
            secret = argv[++i];
        }
        else if (arg == "--dict" || arg == "-d")
        {
            if (i + 1 == argc)
            {
                std::cerr << arg << " needs an argument - a dictionary path." << std::endl;
                return 1;
            }
            dictionary = argv[++i];
        }
        else
        {
            std::cerr << "Unknown command line argument: " << arg << std::endl;
            return 1;
        }
    }

    try
    {
        boost::asio::io_service io_service;
        Server server(io_service, secret, port, dictionary.c_str());
        io_service.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
