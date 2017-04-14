#include "fb_client.h"

#include <grpc++/grpc++.h>
#include <iostream>
#include <stdexcept>
#include <thread>

#define MAX_TRIES 3

using grpc::Channel;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;

using namespace hw2;
FbClient::FbClient(std::string username, std::string masterConnectionString)
    : username(username), masterConnectionString(masterConnectionString) {
  ConnectToServer();
}

void FbClient::ConnectToServer() {
  ResetMasterChannel();
  std::string worker_location;
  grpc::Status result;
  try {
    result = masterClient.ConnectionPoint(worker_location);
  } catch(...) {
    result = grpc::Status(grpc::StatusCode::ABORTED, "");
  }
  if (!result.ok()) {
    if (result.error_code() == grpc::StatusCode::CANCELLED)
      throw std::runtime_error(result.error_message());
    else {
      if (tries == 0)
        std::cout << "\nMaster is unavailable. Attempting reconnect.";
      else
        std::cout << '.';
      std::cout << std::flush;
      std::this_thread::sleep_for(std::chrono::seconds(3));
      ++tries;
      if (tries >= MAX_TRIES) {
        throw std::runtime_error("Unable to connect to master");
      }
      ConnectToServer();
      return;
    }
  }
  tries = 0;
  auto client_worker_channel =
      grpc::CreateChannel(worker_location, grpc::InsecureChannelCredentials());
  stub = hw2::MessengerServer::NewStub(client_worker_channel);
}

void FbClient::ResetMasterChannel() {
  auto channel = grpc::CreateChannel(masterConnectionString,
                                     grpc::InsecureChannelCredentials());
  masterClient = MasterClient(channel);
}

// Register a user name with the server
bool FbClient::Register() {
  // Data we are sending to the server.
  Request request;
  request.set_username(username);

  ClientContext context;
  status = stub->Login(&context, request, &reply);

  std::cout << " " << std::endl;

  return PrintReplyMessageOrReconnect();
}

bool FbClient::PrintReplyMessageOrReconnect() {
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
  std::cout << "Lost connection with server. Reconnecting...";
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

  return PrintReplyMessageOrReconnect();
}

bool FbClient::Leave(std::string channelname) {
  // Data we are sending to the server.
  Request request;
  request.set_username(username);
  request.add_arguments(channelname);

  ClientContext context;
  status = stub->Leave(&context, request, &reply);

  return PrintReplyMessageOrReconnect();
}

bool FbClient::List() {
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
    return false;
  }
  return true;
}

bool FbClient::Chat() {
  ClientContext context;
  auto stream = stub->Chat(&context);
  Chatter chatter(username, std::move(stream));

  chatter.Chat();

  Reconnect();

  return false;
}
