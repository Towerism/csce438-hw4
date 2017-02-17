#include "command_line.h"

#include "command_factory.h"
#include "command_stream.h"

namespace CommandLine {
  void Run(FbClient& client) {
    PrintAvailableCommands();
    CommandStream commandStream;
    std::string commandString;
    std::unique_ptr<Command> command;
    while (true) {
      if (!commandStream.ReadCommandLine())
        continue;
      commandString = commandStream.Command();
      auto command = CommandFactory::MakeCommand(commandString, client, commandStream);
      command->Execute();
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
