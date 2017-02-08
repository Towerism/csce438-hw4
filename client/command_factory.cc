#include "command_factory.h"

#include "join_command.h"
#include "list_command.h"
#include "null_command.h"

std::unique_ptr<Command>
CommandFactory::MakeCommand(FbClient &client, CommandStream &commandStream) {
  Command *joinCommand = new JoinCommand("JOIN", client, commandStream);
  Command *listCommand = new ListCommand("LIST", client);
  Command *nullCommand = new NullCommand(commandStream);
  joinCommand->SetNextCommand(listCommand)->SetNextCommand(nullCommand);
  return std::unique_ptr<Command>(joinCommand);
}
