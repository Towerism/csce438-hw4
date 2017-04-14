#include "list_command.h"

bool ListCommand::ExecuteMainAction() {
  return client.List();
}
