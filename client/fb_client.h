#pragma once

#include <fb.grpc.pb.h>
#include <mutex>
#include <condition_variable>

#include <string>
#include <memory>
#include "master_client.h"

class FbClient {
public:
  FbClient(std::string username, std::string masterConnectionString);

  // Register a user name
  bool Register();

  // Join a user's chat room
  bool Join(std::string channelname);

  bool Leave(std::string channelname);

  // List users and joined users
  void List();

  // Enter chat mode
  bool Chat();

private:
  uint32_t tries = 0;
  std::string username;
  std::unique_ptr<hw2::MessengerServer::Stub> stub;
  grpc::Status status;
  hw2::Reply reply;
  hw2::Message mostRecentMessage;
  MasterClient masterClient;
  std::string masterConnectionString;
  void ConnectToServer();
  void ResetMasterChannel();

  void Reconnect();
  bool PrintPossibleStatusFailuresForBasicReply();
  bool PrintReplyMessage();
  hw2::Message MakeMessage(const std::string& username, const std::string& msg);
};
