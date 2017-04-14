#pragma once

#include <string>

class Command {
public:
  virtual ~Command() = default;
  void Execute() {
    if (!Parse())
      return;
    bool result;
    do {
      result = ExecuteMainAction();
    } while (!result);
  }
protected:
  virtual bool ExecuteMainAction() = 0;
  virtual bool Parse() { return true; }
};
