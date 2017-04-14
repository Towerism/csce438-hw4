#pragma once

#include "command.h"

#include "command_stream.h"
#include "fb_client.h"

class LeaveCommand : public Command {
public:
  LeaveCommand(FbClient& client, CommandStream& commandStream)
    : client(client), commandStream(commandStream) {}

  bool Parse() override;
  bool ExecuteMainAction() override;
private:
  FbClient& client;
  CommandStream& commandStream;
  std::string argument;
};
