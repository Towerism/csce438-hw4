#include "arguments_parser.h"

#include <algorithm>

ArgumentsParser::Arguments ArgumentsParser::Parse(int argc, char **argv) {
  Arguments result;
  if (argc != 2)
    return invalidArguments();
  if (!validateConnectionString(argv[1]))
    return invalidArguments();
  result.ConnectionString = argv[1];
  return result;
}

ArgumentsParser::Arguments ArgumentsParser::invalidArguments() {
  Arguments arguments;
  arguments.Valid = false;
  return arguments;
}

bool ArgumentsParser::validateConnectionString(std::string connectionString) {
  size_t number_of_colons = std::count(connectionString.begin(), connectionString.end(), ':');
  return number_of_colons == 1;
}
