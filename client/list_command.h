#pragma once

#include "command.h"

#include "command_stream.h"
#include "fb_client.h"

class ListCommand : public Command {
public:
  ListCommand(std::string match, FbClient& client)
    : match(match), client(client) {}

  void Execute(std::string commandMatch) override;
private:
  std::string match;
  FbClient& client;
};
