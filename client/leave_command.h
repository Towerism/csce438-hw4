#pragma once

#include "command.h"

#include "command_stream.h"
#include "fb_client.h"

class LeaveCommand : public Command {
public:
  LeaveCommand(FbClient& client, CommandStream& commandStream)
    : client(client), commandStream(commandStream) {}

  void Execute() override;
private:
  FbClient& client;
  CommandStream& commandStream;
};
