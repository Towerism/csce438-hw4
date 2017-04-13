#include <cstdio>
#include <fb.grpc.pb.h>
#include <grpc++/grpc++.h>
#include <iostream>

#include "arguments_parser.h"
#include "command_line.h"
#include "fb_client.h"

void printUsage(const char *program_name) {
  std::cerr << "Usage: "<< program_name << " <username> <host>:<port>\n";
  std::exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  ArgumentsParser argsParser(argc, argv);
  auto arguments = argsParser.Parse();
  if (!arguments.Valid)
    printUsage(argv[0]);
  FbClient client(arguments.Username, arguments.ConnectionString);
  if (!client.Register())
    return 1;
  CommandLine::Run(client);
  return 0;
}
