#include "chat_test_command.h"

#include <thread>
#include <chrono>

bool ChatTestCommand::ExecuteMainAction() {
  return client.SendChatTest();
}
