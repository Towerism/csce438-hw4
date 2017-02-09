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

  ClientContext context;
  status = stub->Register(&context, request, &basicReply);

  return PrintPossibleStatusFailuresForBasicReply();
}

bool FbClient::PrintPossibleStatusFailuresForBasicReply() {
  if (status.ok())
    return PrintPossibleBasicReplyFailure();
  PrintStatusError();
  return false;
}

bool FbClient::PrintPossibleBasicReplyFailure() {
  if (basicReply.success())
    return true;
  PrintBasicReplyError();
  return false;
}

void FbClient::PrintStatusError() {
  std::cout << status.error_code() << ": " << status.error_message()
            << std::endl;
}

void FbClient::PrintBasicReplyError() {
  std::cout << "Failed: " << basicReply.message() << std::endl;
  basicReply.clear_message();
}

bool FbClient::Join(std::string channelname) {
  // Data we are sending to the server.
  JoinRequest request;
  request.set_username(username);
  request.set_channelname(channelname);

  ClientContext context;
  status = stub->Join(&context, request, &basicReply);

  return PrintPossibleStatusFailuresForBasicReply();
}

void FbClient::List() {
  ListRequest request;
  request.set_username(username);

  UserList userList;
  ClientContext context;
  status = stub->List(&context, request, &userList);
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
    PrintStatusError();
  }
}
