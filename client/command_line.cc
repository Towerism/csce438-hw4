#include "command_line.h"

#include "command_factory.h"
#include "command_stream.h"

namespace CommandLine {
  void Run(FbClient& client) {
    PrintAvailableCommands();
    CommandStream commandStream;
    std::string command;
    while (true) {
      auto commandExecutor = CommandFactory::MakeCommand(client, commandStream);
      if (!commandStream.ReadCommandLine())
        continue;
      command = commandStream.Command();
      commandExecutor->Execute(command);
    }
  }

  void PrintAvailableCommands() {
    std::cout << "Available commands:\n"
              << "\tJOIN <user>\n"
              << "\tLEAVE <user>\n"
              << "\tLIST\n"
              << "\tCHAT"
              << std::endl;
  }
}
