#include "join_command.h"

void JoinCommand::Execute(std::string commandMatch) {
  if (ExecuteNextCommand(match, commandMatch))
    return;
  auto argument = commandStream.Argument();
  if (!commandStream.IsGood())
    return;
  client.Join(argument);
}
