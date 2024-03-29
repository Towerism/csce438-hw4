#pragma once

#include "command.h"

#include "command_stream.h"
#include "fb_client.h"

class ChatCommand : public Command {
public:
  ChatCommand(FbClient& client)
    : client(client) {}

  bool ExecuteMainAction() override;
private:
  FbClient& client;
};
