#include <cstdio>
#include <fb.grpc.pb.h>
#include <grpc++/grpc++.h>

#include "arguments_parser.h"
#include "fb_client.h"

void print_usage(const char *program_name) {
  printf("Usage: %s <host>:<port>\n", program_name);
  std::exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  ArgumentsParser argsParser(argc, argv);
  auto arguments = argsParser.Parse();
  if (!arguments.Valid)
    print_usage(argv[0]);
  auto channel = grpc::CreateChannel(arguments.ConnectionString,
                                     grpc::InsecureChannelCredentials());
  FbClient client(channel);
  if (!client.Register("username"))
    return 1;
  return 0;
}
