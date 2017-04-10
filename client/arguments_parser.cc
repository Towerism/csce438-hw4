#include "arguments_parser.h"

#include <algorithm>

ArgumentsParser::Arguments ArgumentsParser::Parse() {
  if (!UnmarshallArguments())
    return InvalidArguments();
  return arguments;
}

bool ArgumentsParser::UnmarshallArguments() {
  return argc == 3 && UnmarshallUsername() && UnmarshallConnectionString();
}

ArgumentsParser::Arguments ArgumentsParser::InvalidArguments() {
  arguments.Valid = false;
  return arguments;
}

bool ArgumentsParser::UnmarshallUsername() {
  std::string username(argv[1]);
  arguments.Username = username;
  return true;
}

bool ArgumentsParser::UnmarshallConnectionString() {
  std::string connectionString(argv[2]);
  size_t number_of_colons = std::count(connectionString.begin(), connectionString.end(), ':');
  arguments.ConnectionString = connectionString;
  return number_of_colons == 1;
}
