#include "fb_client.h"

#include <grpc++/grpc++.h>
#include <iostream>

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

bool FbClient::Leave(std::string channelname) {
  // Data we are sending to the server.
  LeaveRequest request;
  request.set_username(username);
  request.set_channelname(channelname);

  ClientContext context;
  status = stub->Leave(&context, request, &basicReply);

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

void FbClient::WhatsNew() {
  WhatsNewRequest request;
  request.set_username(username);
  request.clear_username();

  SendWhatsNewRequest(request);
}

bool FbClient::Chat(std::string text) {
  Message request;
  request.set_username(username);
  request.set_message(text);

  ClientContext context;
  status = stub->Chat(&context, request, &basicReply);

  return PrintPossibleStatusFailuresForBasicReply();
}

void FbClient::SendWhatsNewRequest(WhatsNewRequest request) {
  MessageList messageList;
  ClientContext context;
  status = stub->WhatsNew(&context, request, &messageList);
  if (status.ok()) {
    if (messageList.messages_size() == 0) {
      std::cout << "No new messages\n";
      return;
    }
    for (auto message : messageList.messages()) {
      printf("%s [%s]: %s\n", message.username().c_str(),
             message.date().c_str(), message.message().c_str());
    }
  } else {
    PrintStatusError();
  }
}
