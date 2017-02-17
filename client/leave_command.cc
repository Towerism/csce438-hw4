#include "leave_command.h"

void LeaveCommand::Execute() {
  auto argument = commandStream.Argument();
  if (!commandStream.IsGood())
    return;
  client.Leave(argument);
}
