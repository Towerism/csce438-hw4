#pragma once

#include "command.h"
#include "command_stream.h"

#include <iostream>

class NullCommand : public Command {
public:
  NullCommand(CommandStream& commandStream) : commandStream(commandStream) {}

  void Execute() override {
    std::cout << "Unfamiliar command line: " << commandStream.CommandLine() << std::endl;
  }
private:
  CommandStream& commandStream;
};
