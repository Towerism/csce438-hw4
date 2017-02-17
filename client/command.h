#pragma once

#include <string>

// Chain of Command Pattern
class Command {
public:
  virtual ~Command() = default;
  virtual void Execute() = 0;
};
