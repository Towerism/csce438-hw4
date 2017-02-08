#include "list_command.h"

void ListCommand::Execute(std::string commandMatch) {
  if (commandMatch != match) {
    nextCommand->Execute(commandMatch);
    return;
  }
  client.List();
}
