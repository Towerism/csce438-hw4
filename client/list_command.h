#pragma once

#include "command.h"

#include "command_stream.h"
#include "fb_client.h"

class ListCommand : public Command {
public:
  ListCommand(FbClient& client)
    : client(client) {}

  void Execute() override;
private:
  FbClient& client;
};
