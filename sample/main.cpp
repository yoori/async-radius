#include "version.h"
#include "server.h"
#include "error.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <cstdint>
#include <boost/asio.hpp>

using boost::system::error_code;

namespace
{
  void print_help(const std::string& program_name)
  {
    std::cout << "Usage: " << program_name << " -s/--secret <secret> [-p/--port <port>] [-h/--help] [-v/--version]\n" <<
      "  --secret, -s <secret> - shared secret for password encryption by client and server;" << std::endl <<
      "  --port, -p <port>     - port number for the socket;" << std::endl <<
      "  --help, -h            - print this help;" << std::endl <<
      "  --version, -v         - print version." << std::endl;
  }

  void print_version(const std::string& program_name)
  {
    std::cout << program_name << std::endl <<
      "radius-lite" <<  " " << RADIUSD::version << std::endl;
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
      print_help(argv[0]);
      return 0;
    }

    if (arg == "--version" || arg == "-v")
    {
      print_version(argv[0]);
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
