#pragma once

#include "fb_client.h"

namespace CommandLine {
  void PrintAvailableCommands();

  void Run(FbClient& client);
}
