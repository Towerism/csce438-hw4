#include "arguments_parser.h"

#include <algorithm>

ArgumentsParser::Arguments ArgumentsParser::Parse() {
  if (!UnmarshallArguments())
    return InvalidArguments();
  return arguments;
}

bool ArgumentsParser::UnmarshallArguments() {
  return argc == 2 && UnmarshallConnectionString();
}

ArgumentsParser::Arguments ArgumentsParser::InvalidArguments() {
  arguments.Valid = false;
  return arguments;
}

bool ArgumentsParser::UnmarshallConnectionString() {
  std::string connectionString(argv[1]);
  size_t number_of_colons = std::count(connectionString.begin(), connectionString.end(), ':');
  arguments.ConnectionString = connectionString;
  return number_of_colons == 1;
}
