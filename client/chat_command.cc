#include "chat_command.h"

void ChatCommand::Execute(std::string commandMatch) {
  if (ExecuteNextCommand(match, commandMatch))
    return;
  std::cout << "You are now in CHAT mode...\n";
  while (true) {
  }
}
