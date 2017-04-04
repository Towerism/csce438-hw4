#include "fb_client.h"

#include <grpc++/grpc++.h>
#include <iostream>
#include <thread>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace hw2;

// Register a user name with the server
bool FbClient::Register() {
  // Data we are sending to the server.
  Request request;
  request.set_username(username);

  ClientContext context;
  status = stub->Login(&context, request, &reply);

  return PrintPossibleStatusFailuresForBasicReply();
}

bool FbClient::PrintPossibleStatusFailuresForBasicReply() {
  if (status.ok())
    return PrintPossibleBasicReplyFailure();
  PrintStatusError();
  return false;
}

bool FbClient::PrintPossibleBasicReplyFailure() {
  if (true)
    return true;
  PrintBasicReplyError();
  return false;
}

void FbClient::PrintStatusError() {
  std::cout << status.error_code() << ": " << status.error_message()
            << std::endl;
}

void FbClient::PrintBasicReplyError() {
  std::cout << "Failed: " << reply.msg() << std::endl;
  reply.clear_msg();
}

bool FbClient::Join(std::string channelname) {
  // Data we are sending to the server.
  Request request;
  request.set_username(username);
  request.add_arguments(channelname);

  ClientContext context;
  status = stub->Join(&context, request, &reply);

  return PrintPossibleStatusFailuresForBasicReply();
}

bool FbClient::Leave(std::string channelname) {
  // Data we are sending to the server.
  Request request;
  request.set_username(username);
  request.add_arguments(channelname);

  ClientContext context;
  status = stub->Leave(&context, request, &reply);

  return PrintPossibleStatusFailuresForBasicReply();
}

void FbClient::List() {
  Request request;
  request.set_username(username);

  ListReply userList;
  ClientContext context;
  status = stub->List(&context, request, &userList);
  if (status.ok()) {
    std::cout << "All chat rooms:\n";
    for (auto user : userList.all_rooms()) {
      std::cout << "\t" << user << std::endl;
    }
    std::cout << "\nChat rooms you've joined:\n";
    for (auto user : userList.joined_rooms()) {
      std::cout << "\t" << user << std::endl;
    }
  } else {
    PrintStatusError();
  }
}

bool FbClient::Chat() {
  ClientContext context;
  auto stream(stub->Chat(&context));

  std::thread writer([&] {
      std::string input = "Set Stream";
      Message m = MakeMessage(username, input);
      stream->Write(m);
      while(getline(std::cin, input)){
        m = MakeMessage(username, input);
        stream->Write(m);
      }
      stream->WritesDone();
    });

  std::thread reader([&]() {
      Message m;
      while(stream->Read(&m)){
        std::cout << m.username() << " -- " << m.msg() << std::endl;
      }
    });

  return PrintPossibleStatusFailuresForBasicReply();
}

Message FbClient::MakeMessage(const std::string& username, const std::string& msg) {
  Message m;
  m.set_username(username);
  m.set_msg(msg);
  google::protobuf::Timestamp* timestamp = new google::protobuf::Timestamp();
  timestamp->set_seconds(time(NULL));
  timestamp->set_nanos(0);
  m.set_allocated_timestamp(timestamp);
  return m;
}

void FbClient::WhatsNew() {
  WhatsNewRequest request;
  request.set_username(username);
  request.clear_message();

  SendWhatsNewRequest(request);
}

void FbClient::SendWhatsNewRequest(WhatsNewRequest request) {
  MessageList messageList;
  ClientContext context;
  status = stub->WhatsNew(&context, request, &messageList);
}

void FbClient::WhatsNewPoll() {
  WhatsNewRequest request;
  request.set_username(username);
  request.set_allocated_message(new Message(mostRecentMessage));

  SendWhatsNewRequest(request);
}
