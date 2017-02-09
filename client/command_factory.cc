#include "command_factory.h"

#include "chat_command.h"
#include "join_command.h"
#include "leave_command.h"
#include "list_command.h"
#include "null_command.h"

std::unique_ptr<Command>
CommandFactory::MakeCommand(FbClient &client, CommandStream &commandStream) {
  Command *joinCommand = new JoinCommand("JOIN", client, commandStream);
  Command *leaveCommand = new LeaveCommand("LEAVE", client, commandStream);
  Command *listCommand = new ListCommand("LIST", client);
  Command *chatCommand = new ChatCommand("CHAT", client);
  Command *nullCommand = new NullCommand(commandStream);
  joinCommand->SetNextCommand(leaveCommand)
    ->SetNextCommand(listCommand)
    ->SetNextCommand(chatCommand)
    ->SetNextCommand(nullCommand);
  return std::unique_ptr<Command>(joinCommand);
}
