#pragma once

#include <sstream>
#include <string>

class CommandStream {
public:
  bool ReadCommandLine();
  std::string ReadLine();
  std::string CommandLine();
  std::string Command();
  std::string Argument();
  bool IsGood();
private:
  std::istringstream commandStream;
  std::string command, argument, commandline;
};
