#include "master_client.h"

using namespace hw2;
using grpc::ClientContext;
using grpc::Status;

Status MasterClient::ConnectionPoint(std::string& location) {
  Empty empty;

  ClientContext context;
  ConnectionReply reply;
  Status status = stub->ConnectionPoint(&context, empty, &reply);

  if (!status.ok())
    return status;

  location = reply.location();

  return status;
}
