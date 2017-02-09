#pragma once

#include "command.h"

#include "command_stream.h"
#include "fb_client.h"

class ChatCommand : public Command {
public:
  ChatCommand(std::string match, FbClient& client)
    : match(match), client(client) {}

  void Execute(std::string commandMatch) override;
private:
  std::string match;
  FbClient& client;
};
