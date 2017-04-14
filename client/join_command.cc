#include "join_command.h"

bool JoinCommand::Parse() {
  argument = commandStream.Argument();
  return commandStream.IsGood();
}

bool JoinCommand::ExecuteMainAction() {
  return client.Join(argument);
}
