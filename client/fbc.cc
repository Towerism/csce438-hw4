#include <cstdio>
#include <fb.grpc.pb.h>
#include <grpc++/grpc++.h>

#include "arguments_parser.h"
#include "command_line.h"
#include "fb_client.h"

void printUsage(const char *program_name) {
  printf("Usage: %s <host>:<port> <username>\n", program_name);
  std::exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  ArgumentsParser argsParser(argc, argv);
  auto arguments = argsParser.Parse();
  if (!arguments.Valid)
    printUsage(argv[0]);
  auto channel = grpc::CreateChannel(arguments.ConnectionString,
                                     grpc::InsecureChannelCredentials());
  try {
    FbClient client(arguments.Username, channel);
  if (!client.Register())
    return 1;
  CommandLine::Run(client);
  return 0;
  } catch (FbClient::BadMasterChannelException e) {
    std::cerr << e.what() << "\n";
  }
}
