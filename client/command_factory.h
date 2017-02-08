#pragma once

#include <memory>

#include "command.h"
#include "command_stream.h"
#include "fb_client.h"

class CommandFactory {
public:
  static std::unique_ptr<Command> MakeCommand(FbClient &client, CommandStream &commandStream);
};
