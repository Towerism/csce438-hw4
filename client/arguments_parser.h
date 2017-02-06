#pragma once

#include <string>

class ArgumentsParser {
public:
  struct Arguments {
    bool Valid = true; // Whether or not the parse was successful
    std::string ConnectionString;
  };

  ArgumentsParser(int argc, char** argv) : argc(argc), argv(argv) {}

  Arguments Parse();

private:
  int argc;
  char** argv;
  Arguments arguments;

  bool UnmarshallArguments();
  Arguments InvalidArguments();
  bool UnmarshallConnectionString();
};
