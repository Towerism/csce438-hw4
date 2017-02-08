#pragma once

#include "fb.grpc.pb.h"

#include <memory>

class FbClient {
public:
  FbClient(std::string username, std::shared_ptr<grpc::ChannelInterface> channel)
    : username(username), stub(fb::Fakebook::NewStub(channel)) {}

  // Register a user name
  bool Register();

  // Join a user's chat room
  bool Join(std::string username);

  // List users and joined users
  void List();

private:
  std::string username;

  std::unique_ptr<fb::Fakebook::Stub> stub;
};
