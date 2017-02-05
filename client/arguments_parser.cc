#include "arguments_parser.h"

std::regex ArgumentsParser::ConnectionStringRegex(R"(((\d{1,3}:){4})|[^\s:]+:\d*)");

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
  return std::regex_match(connectionString, ConnectionStringRegex);
}
