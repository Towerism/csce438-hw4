#include <cstdio>
#include <fb.grpc.pb.h>
#include <grpc++/grpc++.h>

#include "arguments_parser.h"
#include "fb_client.h"

void print_usage(const char *program_name) {
  printf("Usage: %s <host>:<port>\n", program_name);
  std::exit(EXIT_FAILURE);
}

enum class Mode {
  Command, Chat
};

int main(int argc, char **argv) {
  ArgumentsParser argsParser(argc, argv);
  auto arguments = argsParser.Parse();
  if (!arguments.Valid)
    print_usage(argv[0]);
  auto channel = grpc::CreateChannel(arguments.ConnectionString,
                                     grpc::InsecureChannelCredentials());
  FbClient client("username", channel);
  if (!client.Register())
    return 1;
  std::string command;
  std::string argument;
  std::string message; // used for chat
  Mode mode = Mode::Command;
  std::cout << "Available commands:\n"
            << "JOIN <user>\n"
            << "LIST"
            << std::endl;
  while (true) {
    switch (mode) {
    case Mode::Command:
      std::cin >> command;
      if (command == "JOIN") {
        std::cin >> argument;
        client.Join(argument);
      } else if (command == "LIST") {
        client.List();
      }
      break;
    case Mode::Chat:
      break;
    }
  }
  return 0;
}
