#include "fb_client.h"

#include <grpc++/grpc++.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace fb;

// Register a user name with the server
bool FbClient::Register() {
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

bool FbClient::Join(std::string username) {
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

void FbClient::List() {
  ListRequest request;
  request.set_username(username);

  UserList userList;
  ClientContext context;
  Status status = stub->List(&context, request, &userList);
  if (status.ok()) {
    std::cout << "All chat rooms:\n";
    for (auto user : userList.all_users()) {
      std::cout << "\t" << user << std::endl;
    }
    std::cout << "\nChat rooms you've joined:\n";
    for (auto user : userList.joined_users()) {
      std::cout << "\t" << user << std::endl;
    }
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}
