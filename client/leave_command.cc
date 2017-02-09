#include "leave_command.h"

void LeaveCommand::Execute(std::string commandMatch) {
  if (ExecuteNextCommand(match, commandMatch))
    return;
  auto argument = commandStream.Argument();
  if (!commandStream.IsGood())
    return;
  client.Leave(argument);
}
