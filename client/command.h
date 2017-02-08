#pragma once

#include <string>

// Chain of Command Pattern
class Command {
public:
  virtual ~Command() {
    if (nullptr != nextCommand)
      delete nextCommand;
  }
  virtual void Execute(std::string commandMatch) = 0;
  Command *SetNextCommand(Command *command) {
    nextCommand = command;
    return command;
  }

protected:
  Command *nextCommand = nullptr;
};
