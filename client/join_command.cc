#include "join_command.h"

void JoinCommand::Execute() {
  auto argument = commandStream.Argument();
  if (!commandStream.IsGood())
    return;
  client.Join(argument);
}
