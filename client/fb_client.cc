#include "fb_client.h"

#include <grpc++/grpc++.h>
#include <iostream>
#include <thread>
#include <stdexcept>

using grpc::Channel;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;

using namespace hw2;
FbClient::FbClient(std::string username, std::shared_ptr<ChannelInterface> channel)
  : username(username), masterClient(channel) {
  ConnectToServer();
}

void FbClient::ConnectToServer() {
  std::string worker_location;
  auto result = masterClient.ConnectionPoint(worker_location);
  if (!result.ok()) {
    if (result.error_code() == grpc::StatusCode::CANCELLED)
      throw std::runtime_error(result.error_message());
    else
      throw std::runtime_error("Unable to connect to master.");
  }
  auto client_worker_channel = grpc::CreateChannel(worker_location,
                                                   grpc::InsecureChannelCredentials());
  stub = hw2::MessengerServer::NewStub(client_worker_channel);
}

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
    return PrintReplyMessage();
  Reconnect();
  return false;
}

bool FbClient::PrintReplyMessage() {
  std::cout << reply.msg() << std::endl;
  reply.clear_msg();
  return true;
}

void FbClient::Reconnect() {
  std::cout << "Lost connection with server. Reconnecting... ";
  ConnectToServer();
  Register();
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
    Reconnect();
  }
}

bool FbClient::Chat() {
  bool request_whatsnew = true;
  std::string input;
  Message m;
  while (true) {
    ClientContext context;
    auto stream(stub->Chat(&context));
    std::thread writer([&] {
        if (request_whatsnew) {
          input  = "Set Stream";
          m = MakeMessage(username, input);
          stream->Write(m);
          //request_whatsnew = false;
        }
        while(getline(std::cin, input)){
          m = MakeMessage(username, input);
          try {
            if (!stream->Write(m))
              return;
          } catch (...) {
            return;
          }
        }
        stream->WritesDone();
      });

    std::thread reader([&] {
        Message m;
        try {
          while(stream->Read(&m)){
            std::cout << m.username() << ": " << m.msg() << std::endl;
          }
        }
        catch (...) {
          return;
        }
      });

    writer.join();
    reader.join();
    Reconnect();
  }

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

