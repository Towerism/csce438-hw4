#pragma once

#include "command.h"

#include "fb_client.h"

class ChatTestCommand : public Command {
public:
  ChatTestCommand(FbClient& client)
    : client(client) {}

  bool ExecuteMainAction() override;
private:
  FbClient& client;
};
