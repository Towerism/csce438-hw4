#include "chat_command.h"

#include <thread>
#include <chrono>

void ChatCommand::Execute() {
  std::cout << "You are now in CHAT mode...\n";
  client.Chat();
}

