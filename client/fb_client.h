#pragma once

#include <fb.grpc.pb.h>

#include <string>
#include <memory>
#include "master_client.h"
#include "chatter.h"

class FbClient {
public:
  FbClient(std::string username, std::string masterConnectionString);

  // Register a user name
  bool Register();

  // Join a user's chat room
  bool Join(std::string channelname);

  bool Leave(std::string channelname);

  // List users and joined users
  bool List();

  // Enter chat mode
  bool Chat();

  bool SendChatTest();

private:
  uint32_t tries = 0;
  std::string username;
  std::unique_ptr<hw2::MessengerServer::Stub> stub;
  grpc::Status status;
  hw2::Reply reply;
  MasterClient masterClient;
  std::string masterConnectionString;
  Chatter chatter;
  Chatter::stream_t stream;

  void ConnectToServer();
  void ResetMasterChannel();

  void Reconnect();
  bool PrintReplyMessageOrReconnect();
  bool PrintReplyMessage();
};
