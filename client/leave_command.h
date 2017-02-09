#pragma once

#include "command.h"

#include "command_stream.h"
#include "fb_client.h"

class LeaveCommand : public Command {
public:
  LeaveCommand(std::string match, FbClient& client, CommandStream& commandStream)
    : match(match), client(client), commandStream(commandStream) {}

  void Execute(std::string commandMatch) override;
private:
  std::string match;
  FbClient& client;
  CommandStream& commandStream;
};
