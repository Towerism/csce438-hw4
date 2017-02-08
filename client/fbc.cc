#include <cstdio>
#include <fb.grpc.pb.h>
#include <grpc++/grpc++.h>
#include <sstream>

#include "arguments_parser.h"
#include "command_stream.h"
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
  FbClient client(arguments.Username, channel);
  if (!client.Register())
    return 1;
  Mode mode = Mode::Command;
  std::cout << "Available commands:\n"
            << "JOIN <user>\n"
            << "LIST"
            << std::endl;
  CommandStream commandStream;
  std::string command, argument;
  while (true) {
    switch (mode) {
    case Mode::Command:
      if (!commandStream.ReadCommandLine())
        continue;
      command = commandStream.Command();
      if (command == "JOIN") {
        argument = commandStream.Argument();
        if (!commandStream.IsGood())
          continue;
        client.Join(argument);
      } else if (command == "LIST") {
        client.List();
      } else {
        std::cout << "Unfamiliar command line: " << commandStream.CommandLine() << std::endl;
      }
      break;
    case Mode::Chat:
      break;
    }
  }
  return 0;
}
