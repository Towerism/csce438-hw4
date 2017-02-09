#include "chat_command.h"

void ChatCommand::Execute(std::string commandMatch) {
  if (ExecuteNextCommand(match, commandMatch))
    return;
  std::cout << "You are now in CHAT mode...\n";
  client.WhatsNew();
  CommandStream stream;
  std::string message;
  while (true) {
    message = stream.ReadLine();
    client.Chat(message);
  }
}
