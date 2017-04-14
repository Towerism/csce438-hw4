#pragma once

#include "command.h"
#include "command_stream.h"

#include <iostream>

class NullCommand : public Command {
public:
  NullCommand(CommandStream& commandStream) : commandStream(commandStream) {}

  bool ExecuteMainAction() override {
    std::cout << "Unfamiliar command line: " << commandStream.CommandLine() << std::endl;
    return true;
  }
private:
  CommandStream& commandStream;
};
