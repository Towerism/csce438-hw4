#include "list_command.h"

void ListCommand::Execute() {
  client.List();
}
