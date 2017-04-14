#include "chat_command.h"

#include <thread>
#include <chrono>

bool ChatCommand::ExecuteMainAction() {
  std::cout << "You are now in CHAT mode...\n";
  return client.Chat();
}

