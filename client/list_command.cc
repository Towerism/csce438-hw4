#include "list_command.h"

void ListCommand::Execute(std::string commandMatch) {
  if (ExecuteNextCommand(match, commandMatch))
    return;
  client.List();
}
