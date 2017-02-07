#pragma once

#include "fb.grpc.pb.h"

#include <memory>

class FbClient {
public:
  FbClient(std::shared_ptr<grpc::ChannelInterface> channel)
    : stub(fb::Fakebook::NewStub(channel)) {}

  // Register a user name
  bool Register(const std::string& username);

  // Join a user's chat room
  bool Join(const std::string& username);

private:
  std::unique_ptr<fb::Fakebook::Stub> stub;
};
