#include "fb_client.h"

#include <grpc++/grpc++.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using fb::RegisterRequest;
using fb::JoinRequest;
using fb::BasicReply;
using fb::Fakebook;

// Register a user name with the server
bool FbClient::Register(const std::string &username) {
  // Data we are sending to the server.
  RegisterRequest request;
  request.set_username(username);

  BasicReply reply;
  ClientContext context;
  Status status = stub->Register(&context, request, &reply);

  if (status.ok()) {
    if (reply.success())
      return true;
    else {
      std::cout << "Failed: " << reply.message() << std::endl;
      return false;
    }
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return false;
  }
}

bool FbClient::Join(const std::string &username) {
  // Data we are sending to the server.
  JoinRequest request;
  request.set_username(username);

  BasicReply reply;
  ClientContext context;
  Status status = stub->Join(&context, request, &reply);

  if (status.ok()) {
    if (reply.success())
      return true;
    else {
      std::cout << "Failed: " << reply.message() << std::endl;
      return false;
    }
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return false;
  }
}
