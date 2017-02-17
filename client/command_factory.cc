#include "command_factory.h"

#include "chat_command.h"
#include "join_command.h"
#include "leave_command.h"
#include "list_command.h"
#include "null_command.h"

std::unique_ptr<Command>
CommandFactory::MakeCommand(std::string command, FbClient &client, CommandStream &commandStream) {
  if (command == "JOIN")
    return std::unique_ptr<Command>(new JoinCommand(client, commandStream));
  else if (command == "LEAVE")
    return std::unique_ptr<Command>(new LeaveCommand(client, commandStream));
  else if (command == "LIST")
    return std::unique_ptr<Command>(new ListCommand(client));
  else if (command == "CHAT")
    return std::unique_ptr<Command>(new ChatCommand(client));
  else
    return std::unique_ptr<Command>(new NullCommand(commandStream));
}
