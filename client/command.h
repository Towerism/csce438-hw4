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

  // attempt to to execute next command based on command matching
  // if the next command is executed, returns true, otherwise returns false
  bool ExecuteNextCommand(std::string match, std::string commandMatch);

  Command *SetNextCommand(Command *command);

protected:
  Command *nextCommand = nullptr;
};
