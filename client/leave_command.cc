#include "leave_command.h"

bool LeaveCommand::Parse() {
  argument = commandStream.Argument();
  return commandStream.IsGood();
}

bool LeaveCommand::ExecuteMainAction() {
  return client.Leave(argument);
}
