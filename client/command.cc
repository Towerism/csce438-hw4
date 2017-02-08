#include "command.h"

bool Command::ExecuteNextCommand(std::string match, std::string commandMatch) {
  if (match != commandMatch) {
    nextCommand->Execute(commandMatch);
    return true;
  }
  return false;
}

Command *Command::SetNextCommand(Command *command) {
  nextCommand = command;
  return command;
}
